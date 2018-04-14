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
#ifndef GUARD_LIBS_LIBC_EXIT_C
#define GUARD_LIBS_LIBC_EXIT_C 1

#include "libc.h"
#include "malloc.h"
#include "errno.h"
#include "exit.h"
#include "stdio.h"

#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/xch.h>
#include <bits/dos-errno.h>
#include <errno.h>
#include <stdlib.h>

DECL_BEGIN

#define CONFIG_STATIC_ATEXIT_BUFFER  4
#define CONFIG_STATIC_ATQEXIT_BUFFER 2



#ifndef CONFIG_STATIC_ATEXIT_BUFFER
#define CONFIG_STATIC_ATEXIT_BUFFER  0
#endif
#ifndef CONFIG_STATIC_ATQEXIT_BUFFER
#define CONFIG_STATIC_ATQEXIT_BUFFER 0
#endif


struct atexit_callback {
    void (LIBCCALL *ac_func)(int status, void *arg);
    void           *ac_arg;
};
struct qatexit_callback { void (LIBCCALL *ac_func)(void); };

#if CONFIG_STATIC_ATEXIT_BUFFER != 0
PRIVATE struct atexit_callback  static_atexit_functions[CONFIG_STATIC_ATEXIT_BUFFER];
#endif
PRIVATE struct atexit_callback *atexit_vec = NULL;
PRIVATE size_t                  atexit_cnt = 0;
PRIVATE DEFINE_ATOMIC_RWLOCK(atexit_lock);


#if CONFIG_STATIC_ATQEXIT_BUFFER != 0
PRIVATE struct qatexit_callback  static_qatexit_functions[CONFIG_STATIC_ATQEXIT_BUFFER];
#endif
PRIVATE struct qatexit_callback *qatexit_vec = NULL;
PRIVATE size_t                   qatexit_cnt = 0;
PRIVATE DEFINE_ATOMIC_RWLOCK(qatexit_lock);

INTERN void LIBCCALL
libc_runexit(int status) {
 struct atexit_callback *vec; size_t i,cnt;
 atomic_rwlock_write(&atexit_lock);
 vec = atexit_vec,cnt = atexit_cnt;
 atexit_vec = 0,atexit_cnt = 0;
 atomic_rwlock_endwrite(&atexit_lock);
 for (i = 0; i < cnt; ++i)
    (*vec[i].ac_func)(status,vec[i].ac_arg);
#if CONFIG_STATIC_ATEXIT_BUFFER != 0
 if (vec != static_atexit_functions)
#endif
     libc_free(vec);
}
INTERN void LIBCCALL
libc_runquickexit(int status) {
 struct qatexit_callback *vec; size_t i,cnt;
 atomic_rwlock_write(&qatexit_lock);
 vec = qatexit_vec,cnt = qatexit_cnt;
 qatexit_vec = 0,qatexit_cnt = 0;
 atomic_rwlock_endwrite(&qatexit_lock);
 for (i = 0; i < cnt; ++i)
    (*vec[i].ac_func)();
#if CONFIG_STATIC_ATQEXIT_BUFFER != 0
 if (vec != static_qatexit_functions)
#endif
     libc_free(vec);
}

INTERN void LIBCCALL libc_internal_onexit(void) {
 /* Flush all file streams. */
#ifdef CONFIG_LIBC_USES_NEW_STDIO
 FileBuffer_FlushAllBuffers();
#endif /* CONFIG_LIBC_USES_NEW_STDIO */
}

#ifndef CONFIG_LIBC_HAVE_ARCH_EXIT
INTERN ATTR_NORETURN void LIBCCALL libc_exit(int status) {
 libc_runexit(status);
 libc_runquickexit(status);
 libc_internal_onexit();
 _libc_exit(status);
}
#endif /* !CONFIG_LIBC_HAVE_ARCH_EXIT */

INTERN ATTR_NORETURN void LIBCCALL
libc_quick_exit(int status) {
 libc_runquickexit(status);
 libc_internal_onexit();
 _libc_exit(status);
}
INTERN int LIBCCALL
libc_on_exit(void (LIBCCALL *func)(int status, void *arg), void *arg) {
 struct atexit_callback *vector;
 size_t count;
again:
 atomic_rwlock_write(&atexit_lock);
 vector = atexit_vec;
#if CONFIG_STATIC_ATEXIT_BUFFER
 if (!vector) {
  assert(!atexit_cnt);
  vector = atexit_vec = static_atexit_functions;
 }
 if (atexit_cnt < CONFIG_STATIC_ATEXIT_BUFFER) {
  vector[atexit_cnt].ac_func = func;
  vector[atexit_cnt].ac_arg  = arg;
  ++atexit_cnt;
  atomic_rwlock_endwrite(&atexit_lock);
  return 0;
 }
#endif
 count = atexit_cnt;
 atomic_rwlock_endwrite(&atexit_lock);
 /* Dynamically allocate more. */
 vector = (struct atexit_callback *)libc_malloc((count+1)*
                                                sizeof(struct atexit_callback));
 if unlikely(!vector) return -1;
 vector[count].ac_func = func;
 vector[count].ac_arg  = arg;
 atomic_rwlock_write(&atexit_lock);
 if (atexit_cnt != count) {
  atomic_rwlock_endwrite(&atexit_lock);
  libc_free(vector);
  goto again;
 }
 /* Copy old data. */
 libc_memcpy(vector,atexit_vec,count*
             sizeof(struct atexit_callback));
 /* Install the new vector. */
 vector     = XCH(atexit_vec,vector);
 atexit_cnt = count+1;
 atomic_rwlock_endwrite(&atexit_lock);
 /* Free the old vector. */
#if CONFIG_STATIC_ATEXIT_BUFFER != 0
 if (vector != static_atexit_functions)
#endif
     libc_free(vector);
 return 0;
}
INTERN int LIBCCALL
libc_at_quick_exit(void (*LIBCCALL func)(void)) {
 struct qatexit_callback *vector;
 size_t count;
again:
 atomic_rwlock_write(&qatexit_lock);
 vector = qatexit_vec;
#if CONFIG_STATIC_ATEXIT_BUFFER
 if (!vector) {
  assert(!qatexit_cnt);
  vector = qatexit_vec = static_qatexit_functions;
 }
 if (qatexit_cnt < CONFIG_STATIC_ATEXIT_BUFFER) {
  vector[qatexit_cnt].ac_func = func;
  ++qatexit_cnt;
  atomic_rwlock_endwrite(&qatexit_lock);
  return 0;
 }
#endif
 count = qatexit_cnt;
 atomic_rwlock_endwrite(&qatexit_lock);
 /* Dynamically allocate more. */
 vector = (struct qatexit_callback *)libc_malloc((count+1)*
                                                 sizeof(struct qatexit_callback));
 if unlikely(!vector) return -1;
 vector[count].ac_func = func;
 atomic_rwlock_write(&qatexit_lock);
 if (qatexit_cnt != count) {
  atomic_rwlock_endwrite(&qatexit_lock);
  libc_free(vector);
  goto again;
 }
 /* Copy old data. */
 libc_memcpy(vector,qatexit_vec,count*
             sizeof(struct qatexit_callback));
 /* Install the new vector. */
 vector     = XCH(qatexit_vec,vector);
 qatexit_cnt = count+1;
 atomic_rwlock_endwrite(&qatexit_lock);
 /* Free the old vector. */
#if CONFIG_STATIC_ATEXIT_BUFFER != 0
 if (vector != static_qatexit_functions)
#endif
     libc_free(vector);
 return 0;
}

PRIVATE void LIBCCALL
run_atexit(int UNUSED(status), void *arg) {
 (*(void (*LIBCCALL)(void))arg)();
}

INTERN int LIBCCALL
libc_atexit(void (*LIBCCALL func)(void)) {
 return libc_on_exit(&run_atexit,(void *)func);
}
INTERN ATTR_NORETURN
void LIBCCALL libc_abort(void) {
 _libc_exit(EXIT_FAILURE);
}


EXPORT(abort,                      libc_abort);
#ifndef CONFIG_LIBC_HAVE_ARCH_EXIT
EXPORT(exit,                       libc_exit);
#endif
EXPORT(quick_exit,                 libc_quick_exit);
EXPORT(atexit,                     libc_atexit);
EXPORT(at_quick_exit,              libc_at_quick_exit);
EXPORT(on_exit,                    libc_on_exit);

EXPORT("?terminate%%YAXXZ",        libc_abort);
EXPORT(_onexit,                    libc_atexit);
EXPORT(__dllonexit,                libc_atexit);


/* GLibc Aliases */
EXPORT_STRONG(__cxa_atexit,libc_atexit);
EXPORT_STRONG(__cxa_at_quick_exit,libc_at_quick_exit);

DECL_END

#endif /* !GUARD_LIBS_LIBC_EXIT_C */
