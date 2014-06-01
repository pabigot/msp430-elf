/* Threads compatibility routines for libgcc2.  */
/* Compile this one with gcc.  */
/* Copyright (C) 1998, 2001, 2009 Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef __gthr_qt_h
#define __gthr_qt_h

/* Cooperative threads package based on QuickThreads.  */

#define __GTHREADS 1

#include <coop.h>

typedef int     __gthread_key_t;
typedef void *  __gthread_once_t;
typedef coop_m  __gthread_mutex_t;

#define __GTHREAD_MUTEX_INIT_FUNCTION coop_mutex_init
#define __GTHREAD_ONCE_INIT 0

#if SUPPORTS_WEAK && GTHREAD_USE_WEAK

#pragma weak coop_once
#pragma weak coop_key_create
#pragma weak coop_key_destroy
#pragma weak coop_getspecific
#pragma weak coop_setspecific
#pragma weak coop_create

#pragma weak coop_mutex_init
#pragma weak coop_mutex_lock 
#pragma weak coop_mutex_trylock 
#pragma weak coop_mutex_unlock 

static void * __gthread_active_ptr = & coop_create;

static inline int
__gthread_active_p (void)
{
  return __gthread_active_ptr != 0;
}

#else /* not SUPPORTS_WEAK */

static inline int
__gthread_active_p (void)
{
  return 1;
}

#endif /* SUPPORTS_WEAK */

static inline int
__gthread_once (__gthread_once_t * once, void (* func) (void))
{
  if (__gthread_active_p ())
    {
      coop_once (once, func);
      return 0;
    }
  else
    return -1;
}

static inline int
__gthread_key_create (__gthread_key_t *key, void (*dtor) (void *))
{
  *key = coop_key_create (dtor);
  return 0;
}

static inline int
__gthread_key_dtor (__gthread_key_t key, void *ptr)
{
  /* Just reset the key value to zero. */
  if (ptr)
    coop_setspecific (0, key, 0);
  return 0;
}

static inline int
__gthread_key_delete (__gthread_key_t key)
{
  coop_key_destroy (key);
  return 0;
}

static inline void *
__gthread_getspecific (__gthread_key_t key)
{
  return coop_getspecific (key);
}

static inline int
__gthread_setspecific (__gthread_key_t key, const void *ptr)
{
  coop_setspecific (0, key, ptr);
  return 0;
}

static inline int
__gthread_mutex_lock (__gthread_mutex_t *mutex)
{
  if (__gthread_active_p ())
    coop_mutex_lock (mutex);
  return 0;
}

static inline int
__gthread_mutex_trylock (__gthread_mutex_t *mutex)
{
  if (__gthread_active_p ())
    return coop_mutex_trylock (mutex);
  else
    return 0;
}

static inline int
__gthread_mutex_unlock (__gthread_mutex_t *mutex)
{
  if (__gthread_active_p ())
    coop_mutex_unlock (mutex);
  return 0;
}

#endif /* __gthr_qt_h */
