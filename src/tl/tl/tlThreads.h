
/*

  KLayout Layout Viewer
  Copyright (C) 2006-2026 Matthias Koefferlein

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef HDR_tlThreads
#define HDR_tlThreads

#include "tlCommon.h"
#include "tlCxxFeatures.h"

#include <limits>

#if defined(HAVE_QT) && !defined(HAVE_PTHREADS)
#  include <QMutex>
#  include <QWaitCondition>
#  include <QThread>
#  include <QThreadStorage>
#else
#  include <atomic>
#endif

#if TL_CXX17
#  include <mutex>
#  include <shared_mutex>
#endif

namespace tl
{

/**
 *  @brief A mutex implementation
 *  This class acts as an abstraction. Qt-based and other implementations are
 *  available.
 */

#if defined(HAVE_QT) && !defined(HAVE_PTHREADS)

class TL_PUBLIC Mutex
  : public QMutex
{
public:
  Mutex () : QMutex () { }

  //  Lockable "try_lock" spelling (lower case) so tl::Mutex satisfies the
  //  standard Lockable requirements and can be used with std::scoped_lock /
  //  std::lock for deadlock-free multi-mutex locking.
  bool try_lock () { return QMutex::tryLock (); }
};

#else

class TL_PUBLIC Mutex
{
public:
  Mutex () = default;

  void lock ()
  {
    while (flag.test_and_set (std::memory_order_acquire))
      ;
  }

  void unlock ()
  {
    flag.clear (std::memory_order_release);
  }

  //  Lockable "try_lock": returns true if the lock was acquired. test_and_set
  //  returns the previous value, so a false return means we acquired it.
  bool try_lock ()
  {
    return ! flag.test_and_set (std::memory_order_acquire);
  }

private:
  std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

#endif

/**
 *  @brief A wait condition implementation
 *  This class acts as an abstraction. Qt-based and other implementations are
 *  available.
 */

#if defined(HAVE_QT) && !defined(HAVE_PTHREADS) && !defined(__cpp_lib_atomic_wait)

class TL_PUBLIC WaitCondition
  : public QWaitCondition
{
public:
  WaitCondition () : QWaitCondition () { }

  bool wait (Mutex *mutex, unsigned long time = std::numeric_limits<unsigned long>::max ()) { return QWaitCondition::wait (mutex, time); }
};

#else

class WaitConditionPrivate;

class TL_PUBLIC WaitCondition
{
public:
  WaitCondition ();
  ~WaitCondition ();
  bool wait (Mutex * /*mutex*/, unsigned long /*time*/ = std::numeric_limits<unsigned long>::max ());
  void wakeAll ();
  void wakeOne ();

private:
  WaitConditionPrivate *mp_data;
};

#endif

/**
 *  @brief A RAII-based Mutex locker
 */
class TL_PUBLIC MutexLocker
{
public:
  MutexLocker (Mutex *mutex)
    : mp_mutex (mutex)
  {
    mp_mutex->lock ();
  }

  ~MutexLocker ()
  {
    mp_mutex->unlock ();
  }

private:
  Mutex *mp_mutex;
};

#if TL_CXX17

/**
 *  @brief A scoped lock supporting one or more mutexes (C++17 and later)
 *
 *  tl::ScopedLock<M...> aliases std::scoped_lock<M...>. With a single mutex it
 *  behaves like MutexLocker (but taking a reference, not a pointer). With
 *  several mutexes it uses std::lock's deadlock-avoidance algorithm, which is
 *  why tl::Mutex now provides try_lock(). Example:
 *
 *    tl::ScopedLock<tl::Mutex, tl::Mutex> lk (m1, m2);  // no fixed lock order
 *
 *  Only available when the compiler is building as C++17 or newer; the
 *  pointer-based MutexLocker above remains the portable default.
 */
template <class... Mutexes>
using ScopedLock = std::scoped_lock<Mutexes...>;

#endif

/**
 *  @brief A reader-writer mutex
 *
 *  On C++17 and later this is std::shared_mutex, which allows many concurrent
 *  readers but exclusive writers - ideal for read-mostly data (lookup tables,
 *  memoization caches). On C++11 there is no shared mutex, so it degrades to an
 *  ordinary exclusive Mutex; the SharedLocker below then simply takes an
 *  exclusive lock, which is still correct (just without reader parallelism).
 *
 *  Because the type is defined on every standard, members can be declared as
 *  tl::SharedMutex unconditionally and the read/write distinction is expressed
 *  through tl::SharedLocker / tl::UniqueLocker instead of #if at every call site.
 */
#if TL_CXX17
using SharedMutex = std::shared_mutex;
#else
typedef Mutex SharedMutex;
#endif

/**
 *  @brief RAII reader (shared) lock for a tl::SharedMutex
 *
 *  Takes a shared lock on C++17+ (concurrent readers allowed); takes an ordinary
 *  exclusive lock on C++11.
 */
class TL_PUBLIC SharedLocker
{
public:
  SharedLocker (SharedMutex &m) : mp_mutex (&m)
  {
#if TL_CXX17
    mp_mutex->lock_shared ();
#else
    mp_mutex->lock ();
#endif
  }

  ~SharedLocker ()
  {
#if TL_CXX17
    mp_mutex->unlock_shared ();
#else
    mp_mutex->unlock ();
#endif
  }

private:
  SharedMutex *mp_mutex;

  SharedLocker (const SharedLocker &);
  SharedLocker &operator= (const SharedLocker &);
};

/**
 *  @brief RAII writer (exclusive) lock for a tl::SharedMutex
 *
 *  Takes an exclusive lock on every standard.
 */
class TL_PUBLIC UniqueLocker
{
public:
  UniqueLocker (SharedMutex &m) : mp_mutex (&m)
  {
    mp_mutex->lock ();
  }

  ~UniqueLocker ()
  {
    mp_mutex->unlock ();
  }

private:
  SharedMutex *mp_mutex;

  UniqueLocker (const UniqueLocker &);
  UniqueLocker &operator= (const UniqueLocker &);
};

/**
 *  @brief A thread implementation
 *  This class acts as an abstraction. Qt-based and other implementations are
 *  available.
 */

#if defined(HAVE_QT) && !defined(HAVE_PTHREADS)

class TL_PUBLIC Thread
  : public QThread
{
public:
  Thread ()
    : QThread ()
  { }
};

#else

class ThreadPrivateData;

class TL_PUBLIC Thread
{
public:
  Thread ();
  virtual ~Thread ();

  void exit (int /*returnCode*/ = 0);
  bool isFinished () const;
  bool isRunning () const;
  void quit ();
  void start ();
  void terminate ();
  bool wait (unsigned long /*time*/ = std::numeric_limits<unsigned long>::max ());

protected:
  virtual void run () { }

private:
  friend void *start_thread (void *);
  ThreadPrivateData *mp_data;
  void do_run ();
};

#endif

/**
 *  @brief A thread local storage implementation
 *  This class acts as an abstraction. Qt-based and other implementations are
 *  available.
 */

#if defined(HAVE_QT) && !defined(HAVE_PTHREADS)

template <class T>
class ThreadStorage
  : public QThreadStorage<T>
{
public:
  ThreadStorage () : QThreadStorage<T> () { }
};

#else

class TL_PUBLIC ThreadStorageHolderBase
{
public:
  ThreadStorageHolderBase (void *obj) : mp_obj (obj) { }
  virtual ~ThreadStorageHolderBase () = default;

protected:
  void *obj () { return mp_obj; }
  void *mp_obj;
};

template <class T>
class ThreadStorageHolder
  : public ThreadStorageHolderBase
{
public:
  ThreadStorageHolder (T *t) : ThreadStorageHolderBase ((void *) t) { }
  ~ThreadStorageHolder () { delete data (); }
  T *data () { return (T *) obj (); }
};

class TL_PUBLIC ThreadStorageBase
{
public:
  ThreadStorageBase ();

protected:
  void add (ThreadStorageHolderBase *holder);
  ThreadStorageHolderBase *holder ();

  const ThreadStorageHolderBase *holder () const
  {
    return (const_cast<ThreadStorageBase *> (this)->holder ());
  }
};

template <class T>
class ThreadStorage
  : public ThreadStorageBase
{
public:
  ThreadStorage () : ThreadStorageBase () { }

  bool hasLocalData () const
  {
    return dynamic_cast<const ThreadStorageHolder<T> *> (holder ()) != 0;
  }

  T &localData ()
  {
    return *(dynamic_cast<ThreadStorageHolder<T> *> (holder ())->data ());
  }

  T localData () const
  {
    return *(dynamic_cast<const ThreadStorageHolder<T> *> (holder ())->data ());
  }

  void setLocalData (const T &data)
  {
    if (hasLocalData ()) {
      localData () = data;
    } else {
      add (new ThreadStorageHolder<T> (new T (data)));
    }
  }
};

#endif

}

#endif
