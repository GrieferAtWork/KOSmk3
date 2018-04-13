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
#ifndef _PARTS_KOS2_ASSERT_H
#define _PARTS_KOS2_ASSERT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/debuginfo.h>

__SYSDECL_BEGIN

#ifdef __BUILDING_LIBC
__NAMESPACE_INT_BEGIN
__INTDEF __ATTR_NORETURN __ATTR_COLD void (__LIBCCALL libc_afail)(char const *__expr, __DEBUGINFO);
__INTDEF __ATTR_NORETURN __ATTR_COLD void (libc_afailf)(char const *__expr, __DEBUGINFO, char const *__format, ...);
__NAMESPACE_INT_END
#define __yes_assert(sexpr,expr)         (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM libc_afail(sexpr,__DEBUGINFO_GEN),0))
#define __yes_assertf(sexpr,expr,...)    (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM libc_afailf(sexpr,__DEBUGINFO_GEN,__VA_ARGS__),0))
#define __yes_asserte(sexpr,expr)        (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM libc_afail(sexpr,__DEBUGINFO_GEN),0))
#define __yes_assertef(sexpr,expr,...)   (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM libc_afailf(sexpr,__DEBUGINFO_GEN,__VA_ARGS__),0))
#define __yes_assert_d(sexpr,expr,...)   (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM libc_afail(sexpr,__VA_ARGS__),0))
#define __yes_assertf_d(sexpr,expr,...)  (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM libc_afailf(sexpr,__VA_ARGS__),0))
#define __yes_asserte_d(sexpr,expr,...)  (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM libc_afail(sexpr,__VA_ARGS__),0))
#define __yes_assertef_d(sexpr,expr,...) (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM libc_afailf(sexpr,__VA_ARGS__),0))
#else /* __BUILDING_LIBC */
__NAMESPACE_INT_BEGIN
__LIBC __ATTR_NORETURN __ATTR_COLD void (__LIBCCALL __afail)(char const *__expr, __DEBUGINFO);
__LIBC __ATTR_NORETURN __ATTR_COLD void (__afailf)(char const *__expr, __DEBUGINFO, char const *__format, ...);
__NAMESPACE_INT_END
#define __yes_assert(sexpr,expr)         (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afail(sexpr,__DEBUGINFO_GEN),0))
#define __yes_assertf(sexpr,expr,...)    (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afailf(sexpr,__DEBUGINFO_GEN,__VA_ARGS__),0))
#define __yes_asserte(sexpr,expr)        (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afail(sexpr,__DEBUGINFO_GEN),0))
#define __yes_assertef(sexpr,expr,...)   (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afailf(sexpr,__DEBUGINFO_GEN,__VA_ARGS__),0))
#define __yes_assert_d(sexpr,expr,...)   (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afail(sexpr,__VA_ARGS__),0))
#define __yes_assertf_d(sexpr,expr,...)  (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afailf(sexpr,__VA_ARGS__),0))
#define __yes_asserte_d(sexpr,expr,...)  (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afail(sexpr,__VA_ARGS__),0))
#define __yes_assertef_d(sexpr,expr,...) (void)(__ASSERT_LIKELY(expr) || (__NAMESPACE_INT_SYM __afailf(sexpr,__VA_ARGS__),0))
#endif /* !__BUILDING_LIBC */

__SYSDECL_END

#endif /* !_PARTS_KOS2_ASSERT_H */
