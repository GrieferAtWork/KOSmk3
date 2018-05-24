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
#ifndef __GUARD_HYBRID___BIT_H
#define __GUARD_HYBRID___BIT_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>

/* Bit-scanning / bit-manipulation functions. */

__DECL_BEGIN

#ifdef __CC__


/* Generic implementations. */
#define ____IMPL_DO_FFS(T,i) \
{ unsigned int __result; \
  if (!i) return 0; \
  for (__result = 1; !(((T)i)&1); ++__result) \
       i = ((T)i >> 1); \
  return __result; \
}
#define ____IMPL_DO_CLZ(T,i) \
{ unsigned int __result = 0; \
  T __mask = (T)1 << ((sizeof(T)*8)-1); \
  for (; !((T)(i) & __mask); __mask >>= 1,++__result); \
  return __result; \
}
#define ____IMPL_DO_CTZ(T,i) \
{ unsigned int __result = 0; \
  T __mask = 1; \
  for (; !((T)(i) & __mask); __mask <<= 1,++__result); \
  return __result; \
}
#define ____IMPL_DO_POPCOUNT(T,i) \
{ unsigned int __result = 0; \
  T __mask = 1; \
  for (; __mask < (T)i; __mask <<= 1) \
      if (((T)i) & __mask) ++__result; \
  return __result; \
}
#define ____IMPL_DO_PARITY(T,i) \
{ unsigned int __result = 0; \
  T __mask = 1; \
  for (; __mask < (T)i; __mask <<= 1) \
      if (((T)i) & __mask) __result ^= 1; \
  return __result; \
}
#define ____IMPL_DO_CLRSB(T,i) \
{ unsigned int __result = 0; \
  T __mask = (T)1 << ((sizeof(T)*8)-1); \
  T __sbit = (T)i & __mask; \
  for (;;) { \
      __mask >>= 1; \
      __sbit >>= 1; \
      if (((T)i & __mask) != __sbit) \
          break; \
      ++__result; \
  } \
  return __result; \
}






#ifndef __hybrid_ffs8
#if __has_builtin(__builtin_ffs) && __SIZEOF_INT__ >= 1
#define __hybrid_ffs8(i)   (unsigned int)__builtin_ffs(i)
#elif __has_builtin(__builtin_ffsl) && __SIZEOF_LONG__ >= 1
#define __hybrid_ffs8(i)   (unsigned int)__builtin_ffsl(i)
#elif __has_builtin(__builtin_ffsll) && __SIZEOF_LONG_LONG__ >= 1
#define __hybrid_ffs8(i)   (unsigned int)__builtin_ffsll(i)
#elif defined(__hybrid_ffs16)
#define __hybrid_ffs8(i)    __hybrid_ffs16(i)
#elif defined(__hybrid_ffs32)
#define __hybrid_ffs8(i)    __hybrid_ffs32(i)
#elif defined(__hybrid_ffs64)
#define __hybrid_ffs8(i)    __hybrid_ffs32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_ffs8)(__INT8_TYPE__ __i) ____IMPL_DO_FFS(__UINT8_TYPE__,__i)
#endif
#endif /* !__hybrid_ffs8 */
#ifndef __hybrid_ffs16
#if __has_builtin(__builtin_ffs) && __SIZEOF_INT__ >= 2
#define __hybrid_ffs16(i)   (unsigned int)__builtin_ffs(i)
#elif __has_builtin(__builtin_ffsl) && __SIZEOF_LONG__ >= 2
#define __hybrid_ffs16(i)   (unsigned int)__builtin_ffsl(i)
#elif __has_builtin(__builtin_ffsll) && __SIZEOF_LONG_LONG__ >= 2
#define __hybrid_ffs16(i)   (unsigned int)__builtin_ffsll(i)
#elif defined(__hybrid_ffs32)
#define __hybrid_ffs16(i)    __hybrid_ffs32(i)
#elif defined(__hybrid_ffs64)
#define __hybrid_ffs16(i)    __hybrid_ffs32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_ffs16)(__INT16_TYPE__ __i) ____IMPL_DO_FFS(__UINT16_TYPE__,__i)
#endif
#endif /* !__hybrid_ffs16 */
#ifndef __hybrid_ffs32
#if __has_builtin(__builtin_ffs) && __SIZEOF_INT__ >= 4
#define __hybrid_ffs32(i)   (unsigned int)__builtin_ffs(i)
#elif __has_builtin(__builtin_ffsl) && __SIZEOF_LONG__ >= 4
#define __hybrid_ffs32(i)   (unsigned int)__builtin_ffsl(i)
#elif __has_builtin(__builtin_ffsll) && __SIZEOF_LONG_LONG__ >= 4
#define __hybrid_ffs32(i)   (unsigned int)__builtin_ffsll(i)
#elif defined(__hybrid_ffs64)
#define __hybrid_ffs32(i)    __hybrid_ffs32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_ffs32)(__INT32_TYPE__ __i) ____IMPL_DO_FFS(__UINT32_TYPE__,__i)
#endif
#endif /* !__hybrid_ffs32 */
#ifndef __hybrid_ffs64
#if __has_builtin(__builtin_ffs) && __SIZEOF_INT__ >= 8
#define __hybrid_ffs64(i)   (unsigned int)__builtin_ffs(i)
#elif __has_builtin(__builtin_ffsl) && __SIZEOF_LONG__ >= 8
#define __hybrid_ffs64(i)   (unsigned int)__builtin_ffsl(i)
#elif __has_builtin(__builtin_ffsll) && __SIZEOF_LONG_LONG__ >= 8
#define __hybrid_ffs64(i)   (unsigned int)__builtin_ffsll(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_ffs64)(__INT64_TYPE__ __i) ____IMPL_DO_FFS(__UINT64_TYPE__,__i)
#endif
#endif /* !__hybrid_ffs64 */

#ifndef __hybrid_clz8
#if __has_builtin(__builtin_clz) && __SIZEOF_INT__ >= 1
#define __hybrid_clz8(i)   (unsigned int)__builtin_clz(i)
#elif __has_builtin(__builtin_clzl) && __SIZEOF_LONG__ >= 1
#define __hybrid_clz8(i)   (unsigned int)__builtin_clzl(i)
#elif __has_builtin(__builtin_clzll) && __SIZEOF_LONG_LONG__ >= 1
#define __hybrid_clz8(i)   (unsigned int)__builtin_clzll(i)
#elif defined(__hybrid_clz16)
#define __hybrid_clz8(i)    __hybrid_clz16(i)
#elif defined(__hybrid_clz32)
#define __hybrid_clz8(i)    __hybrid_clz32(i)
#elif defined(__hybrid_clz64)
#define __hybrid_clz8(i)    __hybrid_clz32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_clz8)(__INT8_TYPE__ __i) ____IMPL_DO_CLZ(__UINT8_TYPE__,__i)
#endif
#endif /* !__hybrid_clz8 */
#ifndef __hybrid_clz16
#if __has_builtin(__builtin_clz) && __SIZEOF_INT__ >= 2
#define __hybrid_clz16(i)   (unsigned int)__builtin_clz(i)
#elif __has_builtin(__builtin_clzl) && __SIZEOF_LONG__ >= 2
#define __hybrid_clz16(i)   (unsigned int)__builtin_clzl(i)
#elif __has_builtin(__builtin_clzll) && __SIZEOF_LONG_LONG__ >= 2
#define __hybrid_clz16(i)   (unsigned int)__builtin_clzll(i)
#elif defined(__hybrid_clz32)
#define __hybrid_clz16(i)    __hybrid_clz32(i)
#elif defined(__hybrid_clz64)
#define __hybrid_clz16(i)    __hybrid_clz32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_clz16)(__INT16_TYPE__ __i) ____IMPL_DO_CLZ(__UINT16_TYPE__,__i)
#endif
#endif /* !__hybrid_clz16 */
#ifndef __hybrid_clz32
#if __has_builtin(__builtin_clz) && __SIZEOF_INT__ >= 4
#define __hybrid_clz32(i)   (unsigned int)__builtin_clz(i)
#elif __has_builtin(__builtin_clzl) && __SIZEOF_LONG__ >= 4
#define __hybrid_clz32(i)   (unsigned int)__builtin_clzl(i)
#elif __has_builtin(__builtin_clzll) && __SIZEOF_LONG_LONG__ >= 4
#define __hybrid_clz32(i)   (unsigned int)__builtin_clzll(i)
#elif defined(__hybrid_clz64)
#define __hybrid_clz32(i)    __hybrid_clz32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_clz32)(__INT32_TYPE__ __i) ____IMPL_DO_CLZ(__UINT32_TYPE__,__i)
#endif
#endif /* !__hybrid_clz32 */
#ifndef __hybrid_clz64
#if __has_builtin(__builtin_clz) && __SIZEOF_INT__ >= 8
#define __hybrid_clz64(i)   (unsigned int)__builtin_clz(i)
#elif __has_builtin(__builtin_clzl) && __SIZEOF_LONG__ >= 8
#define __hybrid_clz64(i)   (unsigned int)__builtin_clzl(i)
#elif __has_builtin(__builtin_clzll) && __SIZEOF_LONG_LONG__ >= 8
#define __hybrid_clz64(i)   (unsigned int)__builtin_clzll(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_clz64)(__INT64_TYPE__ __i) ____IMPL_DO_CLZ(__UINT64_TYPE__,__i)
#endif
#endif /* !__hybrid_clz64 */


#ifndef __hybrid_ctz8
#if __has_builtin(__builtin_ctz) && __SIZEOF_INT__ >= 1
#define __hybrid_ctz8(i)   (unsigned int)__builtin_ctz(i)
#elif __has_builtin(__builtin_ctzl) && __SIZEOF_LONG__ >= 1
#define __hybrid_ctz8(i)   (unsigned int)__builtin_ctzl(i)
#elif __has_builtin(__builtin_ctzll) && __SIZEOF_LONG_LONG__ >= 1
#define __hybrid_ctz8(i)   (unsigned int)__builtin_ctzll(i)
#elif defined(__hybrid_ctz16)
#define __hybrid_ctz8(i)    __hybrid_ctz16(i)
#elif defined(__hybrid_ctz32)
#define __hybrid_ctz8(i)    __hybrid_ctz32(i)
#elif defined(__hybrid_ctz64)
#define __hybrid_ctz8(i)    __hybrid_ctz32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_ctz8)(__INT8_TYPE__ __i) ____IMPL_DO_CTZ(__UINT8_TYPE__,__i)
#endif
#endif /* !__hybrid_ctz8 */
#ifndef __hybrid_ctz16
#if __has_builtin(__builtin_ctz) && __SIZEOF_INT__ >= 2
#define __hybrid_ctz16(i)   (unsigned int)__builtin_ctz(i)
#elif __has_builtin(__builtin_ctzl) && __SIZEOF_LONG__ >= 2
#define __hybrid_ctz16(i)   (unsigned int)__builtin_ctzl(i)
#elif __has_builtin(__builtin_ctzll) && __SIZEOF_LONG_LONG__ >= 2
#define __hybrid_ctz16(i)   (unsigned int)__builtin_ctzll(i)
#elif defined(__hybrid_ctz32)
#define __hybrid_ctz16(i)    __hybrid_ctz32(i)
#elif defined(__hybrid_ctz64)
#define __hybrid_ctz16(i)    __hybrid_ctz32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_ctz16)(__INT16_TYPE__ __i) ____IMPL_DO_CTZ(__UINT16_TYPE__,__i)
#endif
#endif /* !__hybrid_ctz16 */
#ifndef __hybrid_ctz32
#if __has_builtin(__builtin_ctz) && __SIZEOF_INT__ >= 4
#define __hybrid_ctz32(i)   (unsigned int)__builtin_ctz(i)
#elif __has_builtin(__builtin_ctzl) && __SIZEOF_LONG__ >= 4
#define __hybrid_ctz32(i)   (unsigned int)__builtin_ctzl(i)
#elif __has_builtin(__builtin_ctzll) && __SIZEOF_LONG_LONG__ >= 4
#define __hybrid_ctz32(i)   (unsigned int)__builtin_ctzll(i)
#elif defined(__hybrid_ctz64)
#define __hybrid_ctz32(i)    __hybrid_ctz32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_ctz32)(__INT32_TYPE__ __i) ____IMPL_DO_CTZ(__UINT32_TYPE__,__i)
#endif
#endif /* !__hybrid_ctz32 */
#ifndef __hybrid_ctz64
#if __has_builtin(__builtin_ctz) && __SIZEOF_INT__ >= 8
#define __hybrid_ctz64(i)   (unsigned int)__builtin_ctz(i)
#elif __has_builtin(__builtin_ctzl) && __SIZEOF_LONG__ >= 8
#define __hybrid_ctz64(i)   (unsigned int)__builtin_ctzl(i)
#elif __has_builtin(__builtin_ctzll) && __SIZEOF_LONG_LONG__ >= 8
#define __hybrid_ctz64(i)   (unsigned int)__builtin_ctzll(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_ctz64)(__INT64_TYPE__ __i) ____IMPL_DO_CTZ(__UINT64_TYPE__,__i)
#endif
#endif /* !__hybrid_ctz64 */

#ifndef __hybrid_popcount8
#if __has_builtin(__builtin_popcount) && __SIZEOF_INT__ >= 1
#define __hybrid_popcount8(i)   (unsigned int)__builtin_popcount(i)
#elif __has_builtin(__builtin_popcountl) && __SIZEOF_LONG__ >= 1
#define __hybrid_popcount8(i)   (unsigned int)__builtin_popcountl(i)
#elif __has_builtin(__builtin_popcountll) && __SIZEOF_LONG_LONG__ >= 1
#define __hybrid_popcount8(i)   (unsigned int)__builtin_popcountll(i)
#elif defined(__hybrid_popcount16)
#define __hybrid_popcount8(i)    __hybrid_popcount16(i)
#elif defined(__hybrid_popcount32)
#define __hybrid_popcount8(i)    __hybrid_popcount32(i)
#elif defined(__hybrid_popcount64)
#define __hybrid_popcount8(i)    __hybrid_popcount32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_popcount8)(__INT8_TYPE__ __i) ____IMPL_DO_POPCOUNT(__UINT8_TYPE__,__i)
#endif
#endif /* !__hybrid_popcount8 */
#ifndef __hybrid_popcount16
#if __has_builtin(__builtin_popcount) && __SIZEOF_INT__ >= 2
#define __hybrid_popcount16(i)   (unsigned int)__builtin_popcount(i)
#elif __has_builtin(__builtin_popcountl) && __SIZEOF_LONG__ >= 2
#define __hybrid_popcount16(i)   (unsigned int)__builtin_popcountl(i)
#elif __has_builtin(__builtin_popcountll) && __SIZEOF_LONG_LONG__ >= 2
#define __hybrid_popcount16(i)   (unsigned int)__builtin_popcountll(i)
#elif defined(__hybrid_popcount32)
#define __hybrid_popcount16(i)    __hybrid_popcount32(i)
#elif defined(__hybrid_popcount64)
#define __hybrid_popcount16(i)    __hybrid_popcount32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_popcount16)(__INT16_TYPE__ __i) ____IMPL_DO_POPCOUNT(__UINT16_TYPE__,__i)
#endif
#endif /* !__hybrid_popcount16 */
#ifndef __hybrid_popcount32
#if __has_builtin(__builtin_popcount) && __SIZEOF_INT__ >= 4
#define __hybrid_popcount32(i)   (unsigned int)__builtin_popcount(i)
#elif __has_builtin(__builtin_popcountl) && __SIZEOF_LONG__ >= 4
#define __hybrid_popcount32(i)   (unsigned int)__builtin_popcountl(i)
#elif __has_builtin(__builtin_popcountll) && __SIZEOF_LONG_LONG__ >= 4
#define __hybrid_popcount32(i)   (unsigned int)__builtin_popcountll(i)
#elif defined(__hybrid_popcount64)
#define __hybrid_popcount32(i)    __hybrid_popcount32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_popcount32)(__INT32_TYPE__ __i) ____IMPL_DO_POPCOUNT(__UINT32_TYPE__,__i)
#endif
#endif /* !__hybrid_popcount32 */
#ifndef __hybrid_popcount64
#if __has_builtin(__builtin_popcount) && __SIZEOF_INT__ >= 8
#define __hybrid_popcount64(i)   (unsigned int)__builtin_popcount(i)
#elif __has_builtin(__builtin_popcountl) && __SIZEOF_LONG__ >= 8
#define __hybrid_popcount64(i)   (unsigned int)__builtin_popcountl(i)
#elif __has_builtin(__builtin_popcountll) && __SIZEOF_LONG_LONG__ >= 8
#define __hybrid_popcount64(i)   (unsigned int)__builtin_popcountll(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_popcount64)(__INT64_TYPE__ __i) ____IMPL_DO_POPCOUNT(__UINT64_TYPE__,__i)
#endif
#endif /* !__hybrid_popcount64 */

#ifndef __hybrid_parity8
#if __has_builtin(__builtin_parity) && __SIZEOF_INT__ >= 1
#define __hybrid_parity8(i)   (unsigned int)__builtin_parity(i)
#elif __has_builtin(__builtin_parityl) && __SIZEOF_LONG__ >= 1
#define __hybrid_parity8(i)   (unsigned int)__builtin_parityl(i)
#elif __has_builtin(__builtin_parityll) && __SIZEOF_LONG_LONG__ >= 1
#define __hybrid_parity8(i)   (unsigned int)__builtin_parityll(i)
#elif defined(__hybrid_parity16)
#define __hybrid_parity8(i)    __hybrid_parity16(i)
#elif defined(__hybrid_parity32)
#define __hybrid_parity8(i)    __hybrid_parity32(i)
#elif defined(__hybrid_parity64)
#define __hybrid_parity8(i)    __hybrid_parity32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_parity8)(__INT8_TYPE__ __i) ____IMPL_DO_PARITY(__UINT8_TYPE__,__i)
#endif
#endif /* !__hybrid_parity8 */
#ifndef __hybrid_parity16
#if __has_builtin(__builtin_parity) && __SIZEOF_INT__ >= 2
#define __hybrid_parity16(i)   (unsigned int)__builtin_parity(i)
#elif __has_builtin(__builtin_parityl) && __SIZEOF_LONG__ >= 2
#define __hybrid_parity16(i)   (unsigned int)__builtin_parityl(i)
#elif __has_builtin(__builtin_parityll) && __SIZEOF_LONG_LONG__ >= 2
#define __hybrid_parity16(i)   (unsigned int)__builtin_parityll(i)
#elif defined(__hybrid_parity32)
#define __hybrid_parity16(i)    __hybrid_parity32(i)
#elif defined(__hybrid_parity64)
#define __hybrid_parity16(i)    __hybrid_parity32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_parity16)(__INT16_TYPE__ __i) ____IMPL_DO_PARITY(__UINT16_TYPE__,__i)
#endif
#endif /* !__hybrid_parity16 */
#ifndef __hybrid_parity32
#if __has_builtin(__builtin_parity) && __SIZEOF_INT__ >= 4
#define __hybrid_parity32(i)   (unsigned int)__builtin_parity(i)
#elif __has_builtin(__builtin_parityl) && __SIZEOF_LONG__ >= 4
#define __hybrid_parity32(i)   (unsigned int)__builtin_parityl(i)
#elif __has_builtin(__builtin_parityll) && __SIZEOF_LONG_LONG__ >= 4
#define __hybrid_parity32(i)   (unsigned int)__builtin_parityll(i)
#elif defined(__hybrid_parity64)
#define __hybrid_parity32(i)    __hybrid_parity32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_parity32)(__INT32_TYPE__ __i) ____IMPL_DO_PARITY(__UINT32_TYPE__,__i)
#endif
#endif /* !__hybrid_parity32 */
#ifndef __hybrid_parity64
#if __has_builtin(__builtin_parity) && __SIZEOF_INT__ >= 8
#define __hybrid_parity64(i)   (unsigned int)__builtin_parity(i)
#elif __has_builtin(__builtin_parityl) && __SIZEOF_LONG__ >= 8
#define __hybrid_parity64(i)   (unsigned int)__builtin_parityl(i)
#elif __has_builtin(__builtin_parityll) && __SIZEOF_LONG_LONG__ >= 8
#define __hybrid_parity64(i)   (unsigned int)__builtin_parityll(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_parity64)(__INT64_TYPE__ __i) ____IMPL_DO_PARITY(__UINT64_TYPE__,__i)
#endif
#endif /* !__hybrid_parity64 */


#ifndef __hybrid_clrsb8
#if __has_builtin(__builtin_clrsb) && __SIZEOF_INT__ >= 1
#define __hybrid_clrsb8(i)   (unsigned int)__builtin_clrsb(i)
#elif __has_builtin(__builtin_clrsbl) && __SIZEOF_LONG__ >= 1
#define __hybrid_clrsb8(i)   (unsigned int)__builtin_clrsbl(i)
#elif __has_builtin(__builtin_clrsbll) && __SIZEOF_LONG_LONG__ >= 1
#define __hybrid_clrsb8(i)   (unsigned int)__builtin_clrsbll(i)
#elif defined(__hybrid_clrsb16)
#define __hybrid_clrsb8(i)    __hybrid_clrsb16(i)
#elif defined(__hybrid_clrsb32)
#define __hybrid_clrsb8(i)    __hybrid_clrsb32(i)
#elif defined(__hybrid_clrsb64)
#define __hybrid_clrsb8(i)    __hybrid_clrsb32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_clrsb8)(__INT8_TYPE__ __i) ____IMPL_DO_CLRSB(__UINT8_TYPE__,__i)
#endif
#endif /* !__hybrid_clrsb8 */
#ifndef __hybrid_clrsb16
#if __has_builtin(__builtin_clrsb) && __SIZEOF_INT__ >= 2
#define __hybrid_clrsb16(i)   (unsigned int)__builtin_clrsb(i)
#elif __has_builtin(__builtin_clrsbl) && __SIZEOF_LONG__ >= 2
#define __hybrid_clrsb16(i)   (unsigned int)__builtin_clrsbl(i)
#elif __has_builtin(__builtin_clrsbll) && __SIZEOF_LONG_LONG__ >= 2
#define __hybrid_clrsb16(i)   (unsigned int)__builtin_clrsbll(i)
#elif defined(__hybrid_clrsb32)
#define __hybrid_clrsb16(i)    __hybrid_clrsb32(i)
#elif defined(__hybrid_clrsb64)
#define __hybrid_clrsb16(i)    __hybrid_clrsb32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_clrsb16)(__INT16_TYPE__ __i) ____IMPL_DO_CLRSB(__UINT16_TYPE__,__i)
#endif
#endif /* !__hybrid_clrsb16 */
#ifndef __hybrid_clrsb32
#if __has_builtin(__builtin_clrsb) && __SIZEOF_INT__ >= 4
#define __hybrid_clrsb32(i)   (unsigned int)__builtin_clrsb(i)
#elif __has_builtin(__builtin_clrsbl) && __SIZEOF_LONG__ >= 4
#define __hybrid_clrsb32(i)   (unsigned int)__builtin_clrsbl(i)
#elif __has_builtin(__builtin_clrsbll) && __SIZEOF_LONG_LONG__ >= 4
#define __hybrid_clrsb32(i)   (unsigned int)__builtin_clrsbll(i)
#elif defined(__hybrid_clrsb64)
#define __hybrid_clrsb32(i)    __hybrid_clrsb32(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_clrsb32)(__INT32_TYPE__ __i) ____IMPL_DO_CLRSB(__UINT32_TYPE__,__i)
#endif
#endif /* !__hybrid_clrsb32 */
#ifndef __hybrid_clrsb64
#if __has_builtin(__builtin_clrsb) && __SIZEOF_INT__ >= 8
#define __hybrid_clrsb64(i)   (unsigned int)__builtin_clrsb(i)
#elif __has_builtin(__builtin_clrsbl) && __SIZEOF_LONG__ >= 8
#define __hybrid_clrsb64(i)   (unsigned int)__builtin_clrsbl(i)
#elif __has_builtin(__builtin_clrsbll) && __SIZEOF_LONG_LONG__ >= 8
#define __hybrid_clrsb64(i)   (unsigned int)__builtin_clrsbll(i)
#else
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL __hybrid_clrsb64)(__INT64_TYPE__ __i) ____IMPL_DO_CLRSB(__UINT64_TYPE__,__i)
#endif
#endif /* !__hybrid_clrsb64 */

#undef ____IMPL_DO_FFS
#undef ____IMPL_DO_CLZ
#undef ____IMPL_DO_CTZ
#undef ____IMPL_DO_POPCOUNT
#undef ____IMPL_DO_PARITY
#undef ____IMPL_DO_CLRSB



#define __hybrid_ffs(i) \
   __builtin_choose_expr(sizeof(i) == 4,__hybrid_ffs32((__INT32_TYPE__)(i)), \
   __builtin_choose_expr(sizeof(i) == 8,__hybrid_ffs64((__INT64_TYPE__)(i)), \
   __builtin_choose_expr(sizeof(i) == 2,__hybrid_ffs16((__INT16_TYPE__)(i)), \
                                        __hybrid_ffs8((__INT8_TYPE__)(i)))))
#define __hybrid_clz(i) \
   __builtin_choose_expr(sizeof(i) == 4,__hybrid_clz32((__INT32_TYPE__)(i)), \
   __builtin_choose_expr(sizeof(i) == 8,__hybrid_clz64((__INT64_TYPE__)(i)), \
   __builtin_choose_expr(sizeof(i) == 2,__hybrid_clz16((__INT16_TYPE__)(i)), \
                                        __hybrid_clz8((__INT8_TYPE__)(i)))))
#define __hybrid_ctz(i) \
   __builtin_choose_expr(sizeof(i) == 4,__hybrid_ctz32((__INT32_TYPE__)(i)), \
   __builtin_choose_expr(sizeof(i) == 8,__hybrid_ctz64((__INT64_TYPE__)(i)), \
   __builtin_choose_expr(sizeof(i) == 2,__hybrid_ctz16((__INT16_TYPE__)(i)), \
                                        __hybrid_ctz8((__INT8_TYPE__)(i)))))
#define __hybrid_popcount(i) \
   __builtin_choose_expr(sizeof(i) == 4,__hybrid_popcount32((__INT32_TYPE__)(i)), \
   __builtin_choose_expr(sizeof(i) == 8,__hybrid_popcount64((__INT64_TYPE__)(i)), \
   __builtin_choose_expr(sizeof(i) == 2,__hybrid_popcount16((__INT16_TYPE__)(i)), \
                                        __hybrid_popcount8((__INT8_TYPE__)(i)))))
#define __hybrid_parity(i) \
   __builtin_choose_expr(sizeof(i) == 4,__hybrid_parity32((__INT32_TYPE__)(i)), \
   __builtin_choose_expr(sizeof(i) == 8,__hybrid_parity64((__INT64_TYPE__)(i)), \
   __builtin_choose_expr(sizeof(i) == 2,__hybrid_parity16((__INT16_TYPE__)(i)), \
                                        __hybrid_parity8((__INT8_TYPE__)(i)))))
#define __hybrid_clrsb(i) \
   __builtin_choose_expr(sizeof(i) == 4,__hybrid_clrsb32((__INT32_TYPE__)(i)), \
   __builtin_choose_expr(sizeof(i) == 8,__hybrid_clrsb64((__INT64_TYPE__)(i)), \
   __builtin_choose_expr(sizeof(i) == 2,__hybrid_clrsb16((__INT16_TYPE__)(i)), \
                                        __hybrid_clrsb8((__INT8_TYPE__)(i)))))


#endif /* __CC__ */

__DECL_END

#endif /* !__GUARD_HYBRID___BIT_H */
