
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

Object::Object ()
  : mp_ptrs (nullptr)
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
  while ((ptrs = reinterpret_cast<WeakOrSharedPtr *> (size_t (mp_ptrs) & ~size_t (1))) != nullptr) {
    ptrs->reset_object ();
  }
}

Object::Object (const Object & /*other*/)
  : mp_ptrs (nullptr)
{
  //  .. nothing yet ..
}

Object &Object::operator= (const Object & /*other*/) // NOLINT(bugprone-unhandled-self-assignment)
{
  //  .. nothing yet ..
  return *this;
}

void Object::register_ptr (WeakOrSharedPtr *p)
{
  tl_assert (p->mp_next == nullptr);
  tl_assert (p->mp_prev == nullptr);

  WeakOrSharedPtr *ptrs = (WeakOrSharedPtr *)(size_t (mp_ptrs) & ~size_t (1));
  bool kept = (size_t (mp_ptrs) & size_t(1));

  p->mp_next = ptrs;
  if (ptrs) {
    ptrs->mp_prev = p;
  }

  mp_ptrs = (WeakOrSharedPtr *)(size_t (p) | (kept ? 1 : 0));
}

void Object::unregister_ptr (WeakOrSharedPtr *p)
{
  WeakOrSharedPtr *ptrs = (WeakOrSharedPtr *)(size_t (mp_ptrs) & ~size_t (1));
  bool kept = (size_t (mp_ptrs) & size_t(1));

  if (p == ptrs) {
    mp_ptrs = (WeakOrSharedPtr *)(size_t (p->mp_next) | (kept ? 1 : 0));
  } 
  if (p->mp_prev) {
    p->mp_prev->mp_next = p->mp_next;
  }
  if (p->mp_next) {
    p->mp_next->mp_prev = p->mp_prev;
  }
  p->mp_prev = p->mp_next = nullptr;
}

void Object::detach_from_all_events ()
{
  WeakOrSharedPtr *ptrs = (WeakOrSharedPtr *)(size_t (mp_ptrs) & ~size_t (1));

  for (WeakOrSharedPtr *p = ptrs; p; ) {
    WeakOrSharedPtr *pnext = p->mp_next;
    if (p->is_event ()) {
      p->reset_object ();
    }
    p = pnext;
  }
}

bool Object::has_strong_references () const
{
  WeakOrSharedPtr *ptrs = (WeakOrSharedPtr *)(size_t (mp_ptrs) & ~size_t (1));
  if (ptrs != mp_ptrs) {
    //  Object is kept
    return true;
  }

  for (WeakOrSharedPtr *p = ptrs; p; p = p->mp_next) {
    if (p->is_shared ()) {
      return true;
    }
  }
  return false;
}

void Object::keep_object ()
{
  mp_ptrs = (WeakOrSharedPtr *)(size_t (mp_ptrs) | size_t (1));
}

void Object::release_object ()
{
  mp_ptrs = (WeakOrSharedPtr *)(size_t (mp_ptrs) & ~size_t (1));

  //  If no more strong references are left, we have to delete ourselves
  if (! has_strong_references ()) {
    delete this;
  }
}

// ---------------------------------------------------------------------
//  WeakOrSharedPtr implementation

WeakOrSharedPtr::WeakOrSharedPtr ()
  : mp_next (nullptr), mp_prev (nullptr), mp_t (nullptr), m_is_shared (true), m_is_event (false)
{
}

WeakOrSharedPtr::WeakOrSharedPtr (const WeakOrSharedPtr &o)
  : mp_next (nullptr), mp_prev (nullptr), mp_t (nullptr), m_is_shared (true), m_is_event (false)
{
  operator= (o);
}

WeakOrSharedPtr::WeakOrSharedPtr (Object *t, bool shared, bool is_event)
  : mp_next (nullptr), mp_prev (nullptr), mp_t (nullptr), m_is_shared (true), m_is_event (false)
{
  reset (t, shared, is_event);
}

WeakOrSharedPtr::~WeakOrSharedPtr ()
{
  reset (nullptr, true, false);
}

WeakOrSharedPtr &WeakOrSharedPtr::operator= (const WeakOrSharedPtr &o)  // NOLINT(bugprone-unhandled-self-assignment)
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

  tl::Mutex *GlobalLockInitializer::sp_lock = nullptr;

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
    mp_t = nullptr;
  }

  tl_assert (mp_prev == nullptr);
  tl_assert (mp_next == nullptr);

  m_is_shared = true;
}

void WeakOrSharedPtr::reset (Object *t, bool is_shared, bool is_event)
{
  if (t == mp_t) {
    return;
  }

  Object *to_delete = nullptr;

  {
    tl::MutexLocker locker (&lock ());

    if (mp_t) {
      Object *told = mp_t;
      mp_t->unregister_ptr (this);
      mp_t = nullptr;
      if (m_is_shared && told && !told->has_strong_references ()) {
        to_delete = told;
      }
    }

    tl_assert (mp_prev == nullptr);
    tl_assert (mp_next == nullptr);

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
