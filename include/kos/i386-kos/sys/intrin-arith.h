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
#ifndef _KOS_I386_KOS_SYS_INTRIN_ARITH_H
#define _KOS_I386_KOS_SYS_INTRIN_ARITH_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>
#include <hybrid/host.h>
#include "intrin.h"

__SYSDECL_BEGIN

#ifdef __CC__
#if !defined(__KERNEL__) || !defined(CONFIG_NO_SMP)
#define __X86_LOCK_PREFIX "lock;"
#else
#define __X86_LOCK_PREFIX /* nothing */
#endif

#ifdef __x86_64__
#define __X86_IF64(x) x
#else
#define __X86_IF64(x) /* nothing */
#endif
#define __X86_DEFINE_INPLACE_ARITH_UNARY(name) \
__FORCELOCAL void __CPU_INTRIN_FUNC(name##b)(__UINT8_TYPE__  *__restrict __field) { __asm__(#name "b %b0" : "+qm" (*__field) : : "cc"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(name##w)(__UINT16_TYPE__ *__restrict __field) { __asm__(#name "w %w0" : "+g" (*__field) : : "cc"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(name##l)(__UINT32_TYPE__ *__restrict __field) { __asm__(#name "l %0" : "+g" (*__field) : : "cc"); } \
__X86_IF64(__FORCELOCAL void __CPU_INTRIN_FUNC(name##q)(__UINT64_TYPE__ *__restrict __field) { __asm__(#name "q %0" : "+g" (*__field) : : "cc"); }) \
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##b)(__UINT8_TYPE__  *__restrict __field) { __asm__(__X86_LOCK_PREFIX #name "b %b0" : "+qm" (*__field) : : "cc", "memory"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##w)(__UINT16_TYPE__ *__restrict __field) { __asm__(__X86_LOCK_PREFIX #name "w %w0" : "+g" (*__field) : : "cc", "memory"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##l)(__UINT32_TYPE__ *__restrict __field) { __asm__(__X86_LOCK_PREFIX #name "l %0" : "+g" (*__field) : : "cc", "memory"); } \
__X86_IF64(__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##q)(__UINT64_TYPE__ *__restrict __field) { __asm__(__X86_LOCK_PREFIX #name "q %0" : "+g" (*__field) : : "cc", "memory"); })
#define __X86_DEFINE_INPLACE_ARITH_BINARY(name) \
__FORCELOCAL void __CPU_INTRIN_FUNC(name##b)(__UINT8_TYPE__  *__restrict __field, __UINT8_TYPE__  __val) { __asm__(#name "b %b1, %b0" : "+qm" (*__field) : "q" (__val) : "cc"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(name##w)(__UINT16_TYPE__ *__restrict __field, __UINT16_TYPE__ __val) { __asm__(#name "w %w1, %w0" : "+g" (*__field) : "r" (__val) : "cc"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(name##l)(__UINT32_TYPE__ *__restrict __field, __UINT32_TYPE__ __val) { __asm__(#name "l %1, %0" : "+g" (*__field) : "r" (__val) : "cc"); } \
__X86_IF64(__FORCELOCAL void __CPU_INTRIN_FUNC(name##q)(__UINT64_TYPE__ *__restrict __field, __UINT64_TYPE__ __val) { __asm__(#name "q %1, %0" : "+g" (*__field) : "r" (__val) : "cc"); }) \
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##b)(__UINT8_TYPE__  *__restrict __field, __UINT8_TYPE__  __val) { __asm__(__X86_LOCK_PREFIX #name "b %b1, %b0" : "+qm" (*__field) : "q" (__val) : "cc", "memory"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##w)(__UINT16_TYPE__ *__restrict __field, __UINT16_TYPE__ __val) { __asm__(__X86_LOCK_PREFIX #name "w %w1, %w0" : "+g" (*__field) : "r" (__val) : "cc", "memory"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##l)(__UINT32_TYPE__ *__restrict __field, __UINT32_TYPE__ __val) { __asm__(__X86_LOCK_PREFIX #name "l %1, %0" : "+g" (*__field) : "r" (__val) : "cc", "memory"); } \
__X86_IF64(__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##q)(__UINT64_TYPE__ *__restrict __field, __UINT64_TYPE__ __val) { __asm__(__X86_LOCK_PREFIX #name "q %1, %0" : "+g" (*__field) : "r" (__val) : "cc", "memory"); })
#define __X86_DEFINE_INPLACE_ARITH_BINARY_NOCC(name) \
__FORCELOCAL void __CPU_INTRIN_FUNC(name##b)(__UINT8_TYPE__  *__restrict __field, __UINT8_TYPE__  __val) { __asm__(#name "b %b1, %b0" : "+qm" (*__field) : "q" (__val)); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(name##w)(__UINT16_TYPE__ *__restrict __field, __UINT16_TYPE__ __val) { __asm__(#name "w %w1, %w0" : "+g" (*__field) : "r" (__val)); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(name##l)(__UINT32_TYPE__ *__restrict __field, __UINT32_TYPE__ __val) { __asm__(#name "l %1, %0" : "+g" (*__field) : "r" (__val)); } \
__X86_IF64(__FORCELOCAL void __CPU_INTRIN_FUNC(name##q)(__UINT64_TYPE__ *__restrict __field, __UINT64_TYPE__ __val) { __asm__(#name "q %1, %0" : "+g" (*__field) : "r" (__val)); }) \
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##b)(__UINT8_TYPE__  *__restrict __field, __UINT8_TYPE__  __val) { __asm__(__X86_LOCK_PREFIX #name "b %b1, %b0" : "+qm" (*__field) : "q" (__val) : "memory"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##w)(__UINT16_TYPE__ *__restrict __field, __UINT16_TYPE__ __val) { __asm__(__X86_LOCK_PREFIX #name "w %w1, %w0" : "+g" (*__field) : "r" (__val) : "memory"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##l)(__UINT32_TYPE__ *__restrict __field, __UINT32_TYPE__ __val) { __asm__(__X86_LOCK_PREFIX #name "l %1, %0" : "+g" (*__field) : "r" (__val) : "memory"); } \
__X86_IF64(__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##q)(__UINT64_TYPE__ *__restrict __field, __UINT64_TYPE__ __val) { __asm__(__X86_LOCK_PREFIX #name "q %1, %0" : "+g" (*__field) : "r" (__val) : "memory"); })
#define __X86_DEFINE_INPLACE_ARITH_SHIFT(name) \
__FORCELOCAL void __CPU_INTRIN_FUNC(name##b)(__UINT8_TYPE__  *__restrict __field, unsigned int __shift) { __asm__(#name "bb %1, %b0" : "+qm" (*__field) : "q" (__shift) : "cc"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(name##w)(__UINT16_TYPE__ *__restrict __field, unsigned int __shift) { __asm__(#name "ww %1, %w0" : "+g" (*__field) : "r" (__shift) : "cc"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(name##l)(__UINT32_TYPE__ *__restrict __field, unsigned int __shift) { __asm__(#name "l %1, %0" : "+g" (*__field) : "r" (__shift) : "cc"); } \
__X86_IF64(__FORCELOCAL void __CPU_INTRIN_FUNC(name##q)(__UINT64_TYPE__ *__restrict __field, unsigned int __shift) { __asm__(#name "q %1, %0" : "+g" (*__field) : "r" (__shift) : "cc"); }) \
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##b)(__UINT8_TYPE__  *__restrict __field, unsigned int __shift) { __asm__(__X86_LOCK_PREFIX #name "b %b1, %b0" : "+qm" (*__field) : "q" (__shift) : "cc", "memory"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##w)(__UINT16_TYPE__ *__restrict __field, unsigned int __shift) { __asm__(__X86_LOCK_PREFIX #name "w %w1, %w0" : "+g" (*__field) : "r" (__shift) : "cc", "memory"); } \
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##l)(__UINT32_TYPE__ *__restrict __field, unsigned int __shift) { __asm__(__X86_LOCK_PREFIX #name "l %1, %0" : "+g" (*__field) : "r" (__shift) : "cc", "memory"); } \
__X86_IF64(__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_##name##q)(__UINT64_TYPE__ *__restrict __field, unsigned int __shift) { __asm__(__X86_LOCK_PREFIX #name "q %1, %0" : "+g" (*__field) : "r" (__shift) : "cc", "memory"); })

__X86_DEFINE_INPLACE_ARITH_UNARY(inc)
__X86_DEFINE_INPLACE_ARITH_UNARY(dec)
__X86_DEFINE_INPLACE_ARITH_BINARY(or)
__X86_DEFINE_INPLACE_ARITH_BINARY(xor)
__X86_DEFINE_INPLACE_ARITH_BINARY(and)
__X86_DEFINE_INPLACE_ARITH_BINARY_NOCC(xchg)
__X86_DEFINE_INPLACE_ARITH_SHIFT(shl)
__X86_DEFINE_INPLACE_ARITH_SHIFT(shr)
__X86_DEFINE_INPLACE_ARITH_SHIFT(sal)
__X86_DEFINE_INPLACE_ARITH_SHIFT(sar)

__FORCELOCAL __UINT8_TYPE__ __CPU_INTRIN_FUNC(cmpxchgb)(__UINT8_TYPE__ *__restrict __field, __UINT8_TYPE__ __old_value, __UINT8_TYPE__ __new_value) { __UINT8_TYPE__ __oldval; __asm__("cmpxchgb %b1, %b2" : "=a" (__oldval) : "q" (__new_value), "m" (*__field), "0" (__old_value) : "cc"); return __oldval; }
__FORCELOCAL __UINT16_TYPE__ __CPU_INTRIN_FUNC(cmpxchgw)(__UINT16_TYPE__ *__restrict __field, __UINT16_TYPE__ __old_value, __UINT16_TYPE__ __new_value) { __UINT16_TYPE__ __oldval; __asm__("cmpxchgw %w1, %w2" : "=a" (__oldval) : "q" (__new_value), "m" (*__field), "0" (__old_value) : "cc"); return __oldval; }
__FORCELOCAL __UINT32_TYPE__ __CPU_INTRIN_FUNC(cmpxchgl)(__UINT32_TYPE__ *__restrict __field, __UINT32_TYPE__ __old_value, __UINT32_TYPE__ __new_value) { __UINT32_TYPE__ __oldval; __asm__("cmpxchgl %1, %2" : "=a" (__oldval) : "r" (__new_value), "m" (*__field), "0" (__old_value) : "cc"); return __oldval; }
__FORCELOCAL __UINT8_TYPE__ __CPU_INTRIN_FUNC(atomic_cmpxchgb)(__UINT8_TYPE__ *__restrict __field, __UINT8_TYPE__ __old_value, __UINT8_TYPE__ __new_value) { __UINT8_TYPE__ __oldval; __asm__(__X86_LOCK_PREFIX "cmpxchgb %b1, %b2" : "=a" (__oldval) : "q" (__new_value), "m" (*__field), "0" (__old_value) : "cc"); return __oldval; }
__FORCELOCAL __UINT16_TYPE__ __CPU_INTRIN_FUNC(atomic_cmpxchgw)(__UINT16_TYPE__ *__restrict __field, __UINT16_TYPE__ __old_value, __UINT16_TYPE__ __new_value) { __UINT16_TYPE__ __oldval; __asm__(__X86_LOCK_PREFIX "cmpxchgw %w1, %w2" : "=a" (__oldval) : "q" (__new_value), "m" (*__field), "0" (__old_value) : "cc"); return __oldval; }
__FORCELOCAL __UINT32_TYPE__ __CPU_INTRIN_FUNC(atomic_cmpxchgl)(__UINT32_TYPE__ *__restrict __field, __UINT32_TYPE__ __old_value, __UINT32_TYPE__ __new_value) { __UINT32_TYPE__ __oldval; __asm__(__X86_LOCK_PREFIX "cmpxchgl %1, %2" : "=a" (__oldval) : "r" (__new_value), "m" (*__field), "0" (__old_value) : "cc"); return __oldval; }
#ifdef __x86_64__
__FORCELOCAL __UINT64_TYPE__ __CPU_INTRIN_FUNC(cmpxchgq)(__UINT64_TYPE__ *__restrict __field, __UINT64_TYPE__ __old_value, __UINT64_TYPE__ __new_value) { __UINT64_TYPE__ __oldval; __asm__("cmpxchgq %1, %2" : "=a" (__oldval) : "r" (__new_value), "m" (*__field), "0" (__old_value) : "cc"); return __oldval; }
__FORCELOCAL __UINT64_TYPE__ __CPU_INTRIN_FUNC(atomic_cmpxchgq)(__UINT64_TYPE__ *__restrict __field, __UINT64_TYPE__ __old_value, __UINT64_TYPE__ __new_value) { __UINT64_TYPE__ __oldval; __asm__(__X86_LOCK_PREFIX "cmpxchgq %1, %2" : "=a" (__oldval) : "r" (__new_value), "m" (*__field), "0" (__old_value) : "cc"); return __oldval; }
#endif /* __x86_64__ */

__FORCELOCAL __WUNUSED __UINT16_TYPE__ __CPU_INTRIN_FUNC(bswapw)(__UINT16_TYPE__ __x) { register __UINT16_TYPE__ __result; __asm__("xchgb %b0, %h0" : "=q" (__result) : "0" (__x)); return __result; }
__FORCELOCAL __WUNUSED __UINT32_TYPE__ __CPU_INTRIN_FUNC(bswapl)(__UINT32_TYPE__ __x) { register __UINT32_TYPE__ __result; __asm__("bswap %0" : "=r" (__result) : "0" (__x)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(btw)(__UINT16_TYPE__ __x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__("btw %2, %w1" : "=@ccc" (__result) : "g" (__x), "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(btl)(__UINT32_TYPE__ __x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__("btl %2, %1" : "=@ccc" (__result) : "g" (__x), "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(btcw)(__UINT16_TYPE__ *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__("btcw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(btcl)(__UINT32_TYPE__ *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__("btcl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(btrw)(__UINT16_TYPE__ *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__("btrw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(btrl)(__UINT32_TYPE__ *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__("btrl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(btsw)(__UINT16_TYPE__ *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__("btsw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(btsl)(__UINT32_TYPE__ *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__("btsl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }

__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(atomic_btcw)(__UINT16_TYPE__ volatile *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btcw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(atomic_btcl)(__UINT32_TYPE__ volatile *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btcl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(atomic_btrw)(__UINT16_TYPE__ volatile *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btrw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(atomic_btrl)(__UINT32_TYPE__ volatile *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btrl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(atomic_btsw)(__UINT16_TYPE__ volatile *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btsw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(atomic_btsl)(__UINT32_TYPE__ volatile *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btsl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_rolw)(__UINT16_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "rolw %1, %w0" : "+g" (*__x) : "nc" (__y) : "cc", "memory"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_roll)(__UINT32_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "roll %1, %0" : "+g" (*__x) : "nc" (__y) : "cc", "memory"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_rorw)(__UINT16_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "rorw %1, %w0" : "+g" (*__x) : "nc" (__y) : "cc", "memory"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_rorl)(__UINT32_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "rorl %1, %0" : "+g" (*__x) : "nc" (__y) : "cc", "memory"); }

#ifdef __x86_64__
__FORCELOCAL __WUNUSED __UINT64_TYPE__ __CPU_INTRIN_FUNC(bswapq)(__UINT64_TYPE__ __x) { register __UINT64_TYPE__ __result; __asm__("bswap %0" : "=r" (__result) : "0" (__x)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(btq)(__UINT64_TYPE__ __x, __UINT64_TYPE__ __bitindex) { __BOOL __result; __asm__("btq %2, %1" : "=@ccc" (__result) : "g" (__x), "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(btcq)(__UINT64_TYPE__ *__x, __UINT64_TYPE__ __bitindex) { __BOOL __result; __asm__("btcq %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(btrq)(__UINT64_TYPE__ *__x, __UINT64_TYPE__ __bitindex) { __BOOL __result; __asm__("btrq %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(btsq)(__UINT64_TYPE__ *__x, __UINT64_TYPE__ __bitindex) { __BOOL __result; __asm__("btsq %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(atomic_btcq)(__UINT64_TYPE__ volatile *__x, __UINT64_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btcq %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(atomic_btrq)(__UINT64_TYPE__ volatile *__x, __UINT64_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btrq %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL __CPU_INTRIN_FUNC(atomic_btsq)(__UINT64_TYPE__ volatile *__x, __UINT64_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btsq %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_rolq)(__UINT64_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "rolq %1, %0" : "+g" (*__x) : "nc" (__y) : "cc", "memory"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(atomic_rorq)(__UINT64_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "rorq %1, %0" : "+g" (*__x) : "nc" (__y) : "cc", "memory"); }
#endif

#undef __X86_IF64
#undef __X86_DEFINE_INPLACE_ARITH_UNARY
#undef __X86_DEFINE_INPLACE_ARITH_BINARY
#undef __X86_DEFINE_INPLACE_ARITH_BINARY_NOCC
#undef __X86_DEFINE_INPLACE_ARITH_SHIFT
#undef __X86_LOCK_PREFIX
#endif /* __CC__ */

__SYSDECL_END

#endif /* !_KOS_I386_KOS_SYS_INTRIN_ARITH_H */
