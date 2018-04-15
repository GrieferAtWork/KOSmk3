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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_PERTASK_H
#define GUARD_KERNEL_INCLUDE_SCHED_PERTASK_H 1

#include <hybrid/compiler.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/pertask.h>
#endif
#ifndef PERTASK_XCH
#include <hybrid/xch.h>
#endif

DECL_BEGIN

#ifdef __CC__


#ifndef THIS_TASK
#ifdef CONFIG_NO_SMP
#   define THIS_TASK  (&_boot_task)
#else
#   error "No SMP support for the target arch. Try builtin with -DCONFIG_NO_SMP"
#endif
#endif
#ifndef PERTASK
#define PERTASK(x) (*(__typeof__(&(x)))(uintptr_t)THIS_TASK+(uintptr_t)&(x))
#endif

#ifdef __INTELLISENSE__
#undef THIS_TASK
#undef PERTASK
#undef PERTASK_GET
#undef PERTASK_SET
#undef PERTASK_XCH

struct task *THIS_TASK;
#define THIS_TASK THIS_TASK
#define PERTASK   PERTASK

#define PERTASK_GET   PERTASK_GET
#define PERTASK_TEST  PERTASK_TEST
#define PERTASK_TESTF PERTASK_TESTF
#define PERTASK_SET   PERTASK_SET
#define PERTASK_XCH   PERTASK_XCH

extern "C++" {
template<class T> T &PERTASK(T &object);
template<class T> T PERTASK_GET(T &object);
template<class T> bool PERTASK_TEST(T &object);
template<class T, class S> bool PERTASK_TESTF(T &object, S flags);
template<class T, class S> void PERTASK_SET(T &object, S value);
template<class T, class S> T PERTASK_XCH(T &object, S value);
}
#endif

#ifndef PERTASK_GET
#define PERTASK_GET(x)           ((__typeof__(x))PERTASK(x))
#endif
#ifndef PERTASK_TEST
#define PERTASK_TEST(x)        (!!PERTASK_GET(x))
#endif
#ifndef PERTASK_TESTF
#define PERTASK_TESTF(x,flags) (!!(PERTASK_GET(x) & (flags)))
#endif
#ifndef PERTASK_SET
#define PERTASK_SET(x,v)       (void)(PERTASK(x) = (v))
#endif
#ifndef PERTASK_XCH
#define PERTASK_XCH(x,v)        XCH(PERTASK(x),v)
#endif

#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_PERTASK_H */
