/* Copyright (c) 2018 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_LIBS_LIBPTHREAD_MISC_C
#define GUARD_LIBS_LIBPTHREAD_MISC_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1
#define _EXCEPT_SOURCE 1

#include "libpthread.h"
#include "misc.h"
#include <errno.h>
#include <sched.h>
#include <except.h>
#include <hybrid/atomic.h>
#include <kos/types.h>

DECL_BEGIN

#define PTHREAD_ONCE_INPROGRESS 1
#define PTHREAD_ONCE_DONE       2


EXPORT(pthread_once,thread_once);
INTERN errno_t LIBPCALL
thread_once(thread_once_t *__restrict once_control,
            void (*init_routine)(void)) {
 thread_once_t *EXCEPT_VAR xonce_control;
 thread_once_t old_state;
 for (;;) {
  old_state = ATOMIC_READ(*once_control);
  if (old_state & PTHREAD_ONCE_DONE)
      return 0; /* Already initialized. */
  if (old_state & PTHREAD_ONCE_INPROGRESS) {
   if (sched_yield())
       return GET_ERRNO();
   continue;
  }
  if (!ATOMIC_CMPXCH(*once_control,old_state,PTHREAD_ONCE_INPROGRESS))
       continue; /* The state changed... */
  xonce_control = once_control;
  TRY {
   /* Invoke the callback. */
   (*init_routine)();
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* Indicate that the callback failed... */
   ATOMIC_WRITE(*xonce_control,0);
   error_rethrow();
  }
  /* callback completed successfully! */
  ATOMIC_WRITE(*once_control,PTHREAD_ONCE_DONE);
 }
 return 0;
}


EXPORT(Xpthread_once,Xthread_once);
INTERN void LIBPCALL
Xthread_once(thread_once_t *__restrict once_control,
             void (*init_routine)(void)) {
 thread_once_t *EXCEPT_VAR xonce_control;
 thread_once_t old_state;
 for (;;) {
  old_state = ATOMIC_READ(*once_control);
  if (old_state & PTHREAD_ONCE_DONE)
      return; /* Already initialized. */
  if (old_state & PTHREAD_ONCE_INPROGRESS) {
   Xsched_yield();
   continue;
  }
  if (!ATOMIC_CMPXCH(*once_control,old_state,PTHREAD_ONCE_INPROGRESS))
       continue; /* The state changed... */
  xonce_control = once_control;
  TRY {
   /* Invoke the callback. */
   (*init_routine)();
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* Indicate that the callback failed... */
   ATOMIC_WRITE(*xonce_control,0);
   error_rethrow();
  }
  /* callback completed successfully! */
  ATOMIC_WRITE(*once_control,PTHREAD_ONCE_DONE);
 }
}


DECL_END

#endif /* !GUARD_LIBS_LIBPTHREAD_MISC_C */
