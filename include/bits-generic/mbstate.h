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
#ifndef _BITS_GENERIC_MBSTATE_H
#define _BITS_GENERIC_MBSTATE_H 1
#define _BITS_MBSTATE_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/byteorder.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifdef __CC__
#ifndef ____mbstate_t_defined
#define ____mbstate_t_defined 1
#if __KOS_VERSION__ >= 300
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define __MBSTATE_GETWORD24(x) ((x)->__word & 0xffffff)
#else
#define __MBSTATE_GETWORD24(x) ((x)->__word >> 8)
#endif
typedef struct __ATTR_PACKED __mbstate {
    /* This structure must not exceed 4 bytes, so we
     * can conform to DOS's 4-byte `mbstate_t' structure. */
    union __ATTR_PACKED {
        __UINT32_TYPE__    __word;
        __UINT16_TYPE__    __word16;
        struct __ATTR_PACKED {
            __UINT8_TYPE__ __chars[3];
            __UINT8_TYPE__ __count;
        }                  __state;
    };
} __mbstate_t;
#define __MBSTATE_INIT      {{0}}
#elif defined(__PE__) || defined(__DOS_COMAPT__)
typedef struct __mbstate { __INT32_TYPE__ __val; } __mbstate_t;
#define __MBSTATE_INIT     {0}
#elif defined(__DOS_COMAPT__)
typedef struct __mbstate {
    int                   __count;
    union { __WINT_TYPE__ __wch; char __wchb[4]; } __value;
} __mbstate_t;
#define __MBSTATE_INIT     {0,{0}}
#else
typedef struct __mbstate {
    int                   __count;
    union { __WINT_TYPE__ __wch; char __wchb[4]; } __value;
} __mbstate_t;
#define __MBSTATE_INIT     {0,{0}}
#endif
#endif /* !____mbstate_t_defined */

#ifndef __std_mbstate_t_defined
#define __std_mbstate_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __mbstate_t mbstate_t;
__NAMESPACE_STD_END
#endif /* !__std_mbstate_t_defined */

#if defined(__USE_GNU) || defined(__USE_XOPEN2K8)
#ifndef __CXX_SYSTEM_HEADER
#ifndef __mbstate_t_defined
#define __mbstate_t_defined 1
__NAMESPACE_STD_USING(mbstate_t)
#endif /* !__mbstate_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */
#endif /* __USE_GNU || __USE_XOPEN2K8 */

#ifdef __USE_KOS
#ifndef MBSTATE_INIT
#define MBSTATE_INIT     __MBSTATE_INIT
#endif /* !MBSTATE_INIT */
#endif /* __USE_KOS */
#endif /* __CC__ */

__SYSDECL_END

#endif /* !_BITS_GENERIC_MBSTATE_H */
