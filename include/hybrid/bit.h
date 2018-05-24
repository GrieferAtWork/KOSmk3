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
#ifndef __GUARD_HYBRID_BIT_H
#define __GUARD_HYBRID_BIT_H 1

#include <hybrid/compiler.h>
#include <hybrid/__bit.h>

/* Bit-scanning / bit-manipulation functions. */

DECL_BEGIN

#ifdef __CC__

#ifndef __ffs_defined
#define __ffs_defined 1
/* unsigned int FFS(INTEGER i):
 *     FindFirstSet
 *     Returns the index (starting at 1 for 0x01) of the first
 *     1-bit in given value, or ZERO(0) if the given value is ZERO(0).
 *     >> assert(!x ||  (x &  (1 << (ffs(x)-1))));    // FFS-bit is set
 *     >> assert(!x || !(x & ((1 << (ffs(x)-1))-1))); // Less significant bits are clear */
#ifdef __INTELLISENSE__
extern "C++" template<class T> unsigned int ffs(T __i);
#else
#define ffs(i)  __hybrid_ffs(i)
#endif
#endif /* !__ffs_defined */

/* unsigned int CLZ(INTEGER i):
 *     CountLeadingZeroes
 *     Return the number of leading ZEROes in `i', starting at the most
 *     significant bit. When `i' is ZERO(0), then the result is undefined. */
#ifdef __INTELLISENSE__
extern "C++" template<class T> unsigned int clz(T __i);
#else
#define clz(i)  __hybrid_clz(i)
#endif

/* unsigned int CTZ(INTEGER i):
 *     CounTrailingZeroes
 *     Return the number of trailing ZEROes in `i', starting at the least
 *     significant bit. When `i' is ZERO(0), then the result is undefined. */
#ifdef __INTELLISENSE__
extern "C++" template<class T> unsigned int ctz(T __i);
#else
#define ctz(i)  __hybrid_ctz(i)
#endif

/* unsigned int POPCOUNT(INTEGER i):
 *     POPulationCOUNT
 *     Return the number of 1-bits in `i' */
#ifdef __INTELLISENSE__
extern "C++" template<class T> unsigned int popcount(T __i);
#else
#define popcount(i)  __hybrid_popcount(i)
#endif

/* unsigned int PARITY(INTEGER i):
 *     Return the parity of `i', that is `POPCOUNT(i) % 2' */
#ifdef __INTELLISENSE__
extern "C++" template<class T> unsigned int parity(T __i);
#else
#define parity(i) __hybrid_parity(i)
#endif

/* unsigned int CLRSB(INTEGER i):
 *     CoultLeadingRedundantSignBits
 *     Return the number of leading, redundant sign-bits,
 *     that is: The number of bits following the most
 *     significant bit that are identical to it. */
#ifdef __INTELLISENSE__
extern "C++" template<class T> unsigned int clrsb(T __i);
#else
#define clrsb(i) __hybrid_clrsb(i)
#endif

#endif /* __CC__ */


DECL_END

#endif /* !__GUARD_HYBRID_BIT_H */
