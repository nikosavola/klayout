
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


#include "tlObject.h"

#include <memory>

namespace tl
{

// ---------------------------------------------------------------------
//  Object implementation

namespace {
  /**
   *  @brief A high-performance, cache-friendly spinlock helper
   *  Based on the rigtorp spinlock design (https://rigtorp.se/spinlock/)
   */
  class SpinLocker {
  public:
    SpinLocker(std::atomic<bool>& lock) : m_lock(lock) {
      for (;;) {
        if (!m_lock.exchange(true, std::memory_order_acquire)) {
          return;
        }
        while (m_lock.load(std::memory_order_relaxed)) {
#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#  if defined(_MSC_VER)
          _mm_pause();
#  else
          __asm__ __volatile__("pause");
#  endif
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64)
#  if defined(_MSC_VER)
          __yield();
#  else
          __asm__ __volatile__("yield");
#  endif
#endif
        }
      }
    }

    ~SpinLocker() {
      m_lock.store(false, std::memory_order_release);
    }

  private:
    std::atomic<bool>& m_lock;
  };
}

Object::Object ()
  : mp_ptrs (0), m_list_lock (false)
{
  //  .. nothing yet ..
}

Object::~Object ()
{
  reset ();
}

void
Object::reset ()
{
  WeakOrSharedPtr *ptrs;

  //  NOTE: basically we'd need to lock the mutex here.
  //  But this will easily create deadlocks and the
  //  destructor should not be called while other threads
  //  are accessing this object anyway.
  while ((ptrs = reinterpret_cast<WeakOrSharedPtr *> (mp_ptrs.load(std::memory_order_relaxed) & ~uintptr_t (1))) != 0) {
    ptrs->reset_object ();
  }
}

Object::Object (const Object & /*other*/)
  : mp_ptrs (0), m_list_lock (false)
{
  //  .. nothing yet ..
}

Object &Object::operator= (const Object & /*other*/)
{
  //  .. nothing yet ..
  return *this;
}

void Object::register_ptr (WeakOrSharedPtr *p)
{
  tl_assert (p->mp_next == 0);
  tl_assert (p->mp_prev == 0);

  SpinLocker lock(m_list_lock);
  uintptr_t cur_ptrs = mp_ptrs.load(std::memory_order_relaxed);
  WeakOrSharedPtr *ptrs = reinterpret_cast<WeakOrSharedPtr*>(cur_ptrs & ~uintptr_t(1));
  bool kept = (cur_ptrs & uintptr_t(1));

  p->mp_next = ptrs;
  if (ptrs) {
    ptrs->mp_prev = p;
  }

  mp_ptrs.store(reinterpret_cast<uintptr_t>(p) | (kept ? 1 : 0), std::memory_order_relaxed);
}

void Object::unregister_ptr (WeakOrSharedPtr *p)
{
  SpinLocker lock(m_list_lock);
  uintptr_t cur_ptrs = mp_ptrs.load(std::memory_order_relaxed);
  WeakOrSharedPtr *ptrs = reinterpret_cast<WeakOrSharedPtr*>(cur_ptrs & ~uintptr_t(1));
  bool kept = (cur_ptrs & uintptr_t(1));

  if (p == ptrs) {
    mp_ptrs.store(reinterpret_cast<uintptr_t>(p->mp_next) | (kept ? 1 : 0), std::memory_order_relaxed);
  } 
  if (p->mp_prev) {
    p->mp_prev->mp_next = p->mp_next;
  }
  if (p->mp_next) {
    p->mp_next->mp_prev = p->mp_prev;
  }
  p->mp_prev = p->mp_next = 0;
}

void Object::detach_from_all_events ()
{
  tl::MutexLocker locker (&WeakOrSharedPtr::lock());
  SpinLocker slock(m_list_lock);
  
  WeakOrSharedPtr *ptrs = reinterpret_cast<WeakOrSharedPtr*>(mp_ptrs.load(std::memory_order_relaxed) & ~uintptr_t(1));

  for (WeakOrSharedPtr *p = ptrs; p; ) {
    WeakOrSharedPtr *pnext = p->mp_next;
    if (p->is_event ()) {
      if (p == ptrs) {
          uintptr_t cur_ptrs = mp_ptrs.load(std::memory_order_relaxed);
          bool kept = (cur_ptrs & uintptr_t(1));
          mp_ptrs.store(reinterpret_cast<uintptr_t>(p->mp_next) | (kept ? 1 : 0), std::memory_order_relaxed);
          ptrs = p->mp_next;
      } 
      if (p->mp_prev) {
          p->mp_prev->mp_next = p->mp_next;
      }
      if (p->mp_next) {
          p->mp_next->mp_prev = p->mp_prev;
      }
      p->mp_prev = p->mp_next = 0;
      
      p->mp_t = 0;
      p->m_is_shared = true;
    }
    p = pnext;
  }
}

bool Object::has_strong_references () const
{
  uintptr_t cur_ptrs = mp_ptrs.load(std::memory_order_acquire);
  if (cur_ptrs & uintptr_t(1)) {
    //  Object is kept
    return true;
  }

  SpinLocker lock(m_list_lock);
  cur_ptrs = mp_ptrs.load(std::memory_order_relaxed);
  WeakOrSharedPtr *ptrs = reinterpret_cast<WeakOrSharedPtr*>(cur_ptrs & ~uintptr_t(1));
  for (WeakOrSharedPtr *p = ptrs; p; p = p->mp_next) {
    if (p->is_shared ()) {
      return true;
    }
  }
  return false;
}

void Object::keep_object ()
{
  mp_ptrs.fetch_or(1, std::memory_order_relaxed);
}

void Object::release_object ()
{
  bool do_delete = false;
  {
    tl::MutexLocker locker (&WeakOrSharedPtr::lock());
    mp_ptrs.fetch_and(~uintptr_t(1), std::memory_order_release);

    //  If no more strong references are left, we have to delete ourselves
    if (! has_strong_references ()) {
      do_delete = true;
    }
  }
  
  if (do_delete) {
    delete this;
  }
}

// ---------------------------------------------------------------------
//  WeakOrSharedPtr implementation

WeakOrSharedPtr::WeakOrSharedPtr ()
  : mp_next (0), mp_prev (0), mp_t (0), m_is_shared (true), m_is_event (false)
{
}

WeakOrSharedPtr::WeakOrSharedPtr (const WeakOrSharedPtr &o)
  : mp_next (0), mp_prev (0), mp_t (0), m_is_shared (true), m_is_event (false)
{
  operator= (o);
}

WeakOrSharedPtr::WeakOrSharedPtr (Object *t, bool shared, bool is_event)
  : mp_next (0), mp_prev (0), mp_t (0), m_is_shared (true), m_is_event (false)
{
  reset (t, shared, is_event);
}

WeakOrSharedPtr::~WeakOrSharedPtr ()
{
  reset (0, true, false);
}

WeakOrSharedPtr &WeakOrSharedPtr::operator= (const WeakOrSharedPtr &o) 
{
  reset (o.mp_t, o.m_is_shared, o.m_is_event);
  return *this;
}

namespace {

  /**
   *  @brief Provides the global lock instance
   */
  struct GlobalLockInitializer
  {
    GlobalLockInitializer ()
    {
      if (! sp_lock) {
        sp_lock = new tl::Mutex ();
      }
    }

    tl::Mutex &gl ()
    {
      return *sp_lock;
    }

  private:
    static tl::Mutex *sp_lock;
  };

  tl::Mutex *GlobalLockInitializer::sp_lock = 0;

  //  This ensures the instance is created in the initialization code
  static GlobalLockInitializer s_gl_init;

}

tl::Mutex &WeakOrSharedPtr::lock ()
{
  //  NOTE: to ensure proper function in static initialization code we cannot simply use
  //  a static QMutex instance - this may not be initialized. This is not entirely thread
  //  safe we make sure above that this initialization is guaranteed to happen in the
  //  static initialization which is single-threaded.
  return GlobalLockInitializer ().gl ();
}

Object *WeakOrSharedPtr::get () 
{
  //  NOTE: this assumes that the pointer access is an atomic operation. Hence no locking.
  return mp_t;
}

const Object *WeakOrSharedPtr::get () const
{
  //  NOTE: this assumes that the pointer access is an atomic operation. Hence no locking.
  return mp_t;
}

void WeakOrSharedPtr::unshare ()
{
  m_is_shared = false;
}

void WeakOrSharedPtr::reset_object ()
{
  tl::MutexLocker locker (&lock ());

  if (mp_t) {
    mp_t->unregister_ptr (this);
    mp_t = 0;
  }

  tl_assert (mp_prev == 0);
  tl_assert (mp_next == 0);

  m_is_shared = true;
}

void WeakOrSharedPtr::reset (Object *t, bool is_shared, bool is_event)
{
  if (t == mp_t) {
    return;
  }

  Object *to_delete = 0;

  {
    tl::MutexLocker locker (&lock ());

    if (mp_t) {
      Object *told = mp_t;
      mp_t->unregister_ptr (this);
      mp_t = 0;
      if (m_is_shared && told && !told->has_strong_references ()) {
        to_delete = told;
      }
    }

    tl_assert (mp_prev == 0);
    tl_assert (mp_next == 0);

    mp_t = t;
    m_is_shared = is_shared;
    m_is_event = is_event;

    if (mp_t) {
      mp_t->register_ptr (this);
    }
  }

  if (to_delete) {
    delete to_delete;
  }
}

}
