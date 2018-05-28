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
#ifndef _KOS_BOUND_H
#define _KOS_BOUND_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifdef __CC__


struct boundb {
    __UINT8_TYPE__   b_min; /* Lower bound */
    __UINT8_TYPE__   b_max; /* Upper bound */
};
struct boundw {
    __UINT16_TYPE__  b_min; /* Lower bound */
    __UINT16_TYPE__  b_max; /* Upper bound */
};
struct boundl {
    __UINT32_TYPE__  b_min; /* Lower bound */
    __UINT32_TYPE__  b_max; /* Upper bound */
};
struct boundq {
    __UINT64_TYPE__  b_min; /* Lower bound */
    __UINT64_TYPE__  b_max; /* Upper bound */
};


#define BOUND(T) struct { T b_min,b_max; }
struct bound_ptr {
    __UINTPTR_TYPE__ b_min; /* Lower bound */
    __UINTPTR_TYPE__ b_max; /* Upper bound */
};


__LIBC void (__LIBCCALL __bound_chkb)(struct boundb const *__restrict __bnd, __UINT8_TYPE__ __index);
__LIBC void (__LIBCCALL __bound_chkw)(struct boundw const *__restrict __bnd, __UINT16_TYPE__ __index);
__LIBC void (__LIBCCALL __bound_chkl)(struct boundl const *__restrict __bnd, __UINT32_TYPE__ __index);
__LIBC void (__LIBCCALL __bound_chkq)(struct boundq const *__restrict __bnd, __UINT64_TYPE__ __index);
__LIBC __BOOL (__LIBCCALL __bound_chk_failb)(struct boundb const *__restrict __bnd, __UINT8_TYPE__ __index);
__LIBC __BOOL (__LIBCCALL __bound_chk_failw)(struct boundw const *__restrict __bnd, __UINT16_TYPE__ __index);
__LIBC __BOOL (__LIBCCALL __bound_chk_faill)(struct boundl const *__restrict __bnd, __UINT32_TYPE__ __index);
__LIBC __BOOL (__LIBCCALL __bound_chk_failq)(struct boundq const *__restrict __bnd, __UINT64_TYPE__ __index);

/* Check an integer `x' against the given bounds.
 * If possible, use hardware acceleration to do this, but fall back to
 * using portable code if the hosting architecture does not support this.
 * This functionality is highly useful for quickly checking pointer bounds
 * in places where many UNCHECKED USER-pointers often show up, such as in
 * the implementation of kernel-space linkers.
 * @throw: E_INDEX_ERROR: One of the following is true:
 *                       `x < bnd.b_min', `x > bnd.b_max'
 * NOTE: If `error_continue(true)' is used, the bounding check is
 *       repeated, however only `bnd' will be re-evaluated while
 *       `x' remains the same.
 * NOTE: If `error_continue(false)' is used, the check failure is ignored.
 * HINT: If an error is thrown, details about the faulting index
 *       and associated range can be retrieved from exception
 *       information found in `error_info()->e_error.e_index_error' */
#if (defined(__i386__) && !defined(__x86_64__)) && \
     defined(__COMPILER_HAVE_GCC_ASM)
/* Use hardware acceleration. */
#ifdef __OPTIMIZE_SIZE__
#define ASSERT_BOUNDS(bnd,x) \
 __XBLOCK({ if (sizeof((bnd).b_min) == 4) { \
                __asm__("boundl %1, %0" \
                        : \
                        : "m" (bnd) \
                        , "r" (x)); \
            } else if (sizeof((bnd).b_min) == 2) { \
                __asm__("boundw %w1, %0" \
                        : \
                        : "m" (bnd) \
                        , "q" (x)); \
            } else if (sizeof((bnd).b_min) == 8) { \
                __bound_chkq((struct boundq *)&(bnd),(__UINT64_TYPE__)(x)); \
            } else { \
                __bound_chkb((struct boundb *)&(bnd),(__UINT8_TYPE__)(x)); \
            } \
            (void)0; })
#else
#define ASSERT_BOUNDS(bnd,x) \
 __XBLOCK({ if (sizeof((bnd).b_min) == 4) { \
                __asm__("boundl %1, %0" \
                        : \
                        : "m" (bnd) \
                        , "r" (x)); \
            } else if (sizeof((bnd).b_min) == 2) { \
                __asm__("boundw %w1, %0" \
                        : \
                        : "m" (bnd) \
                        , "q" (x)); \
            } else if (sizeof((bnd).b_min) == 8) { \
                __UINT64_TYPE__ __index = (x); \
                struct boundq const *__bound = (struct boundq const *)&(bnd); \
                do if (__index >= __bound->b_min && __index <= __bound->b_max) break; \
                while (__bound_chk_failq(__bound,__index)); \
            } else { \
                __UINT8_TYPE__ __index = (x); \
                struct boundb const *__bound = (struct boundb const *)&(bnd); \
                do if (__index >= __bound->b_min && __index <= __bound->b_max) break; \
                while (__bound_chk_failb(__bound,__index)); \
            } \
            (void)0; })
#endif
#elif defined(__OPTIMIZE_SIZE__)
#define ASSERT_BOUNDS(bnd,x) \
 __XBLOCK({ if (sizeof((bnd).b_min) == 4) { \
                __bound_chkl((struct boundl *)&(bnd),(__UINT32_TYPE__)(x)); \
            } else if (sizeof((bnd).b_min) == 2) { \
                __bound_chkw((struct boundw *)&(bnd),(__UINT16_TYPE__)(x)); \
            } else if (sizeof((bnd).b_min) == 8) { \
                __bound_chkq((struct boundq *)&(bnd),(__UINT64_TYPE__)(x)); \
            } else { \
                __bound_chkb((struct boundb *)&(bnd),(__UINT8_TYPE__)(x)); \
            } \
            (void)0; })
#else
#define ASSERT_BOUNDS(bnd,x) \
 __XBLOCK({ if (sizeof((bnd).b_min) == 4) { \
                __UINT32_TYPE__ __index = (x); \
                struct boundl const *__bound = (struct boundl const *)&(bnd); \
                do if (__index >= __bound->b_min && __index <= __bound->b_max) break; \
                while (__bound_chk_faill(__bound,__index)); \
            } else if (sizeof((bnd).b_min) == 2) { \
                __UINT16_TYPE__ __index = (x); \
                struct boundw const *__bound = (struct boundw const *)&(bnd); \
                do if (__index >= __bound->b_min && __index <= __bound->b_max) break; \
                while (__bound_chk_failw(__bound,__index)); \
            } else if (sizeof((bnd).b_min) == 8) { \
                __UINT64_TYPE__ __index = (x); \
                struct boundq const *__bound = (struct boundq const *)&(bnd); \
                do if (__index >= __bound->b_min && __index <= __bound->b_max) break; \
                while (__bound_chk_failq(__bound,__index)); \
            } else { \
                __UINT8_TYPE__ __index = (x); \
                struct boundb const *__bound = (struct boundb const *)&(bnd); \
                do if (__index >= __bound->b_min && __index <= __bound->b_max) break; \
                while (__bound_chk_failb(__bound,__index)); \
            } \
            (void)0; })

#endif

#endif /* __CC__ */

__SYSDECL_END

#endif /* !_KOS_BOUND_H */
