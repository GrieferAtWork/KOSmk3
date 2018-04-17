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
#ifndef _KOS_I386_KOS_INTRIN_H
#define _KOS_I386_KOS_INTRIN_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>
#include <hybrid/host.h>

__SYSDECL_BEGIN

__FORCELOCAL __BOOL (__verr)(__UINT16_TYPE__ __seg) { __BOOL __result; __asm__("verr %w1" : "=@ccz" (__result) : "g" (__seg)); return __result; }
__FORCELOCAL __BOOL (__verw)(__UINT16_TYPE__ __seg) { __BOOL __result; __asm__("verw %w1" : "=@ccz" (__result) : "g" (__seg)); return __result; }
__FORCELOCAL __UINT16_TYPE__ (__bswapw)(__UINT16_TYPE__ __x) { __UINT16_TYPE__ __result; __asm__("xchgb %b0, %h0" : "=q" (__result) : "0" (__x)); return __result; }
__FORCELOCAL __UINT32_TYPE__ (__bswapl)(__UINT32_TYPE__ __x) { __UINT32_TYPE__ __result; __asm__("bswap %0" : "=r" (__result) : "0" (__x)); return __result; }
__FORCELOCAL __BOOL (__btw)(__UINT16_TYPE__ __x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__("btw %2, %w1" : "=@ccc" (__result) : "g" (__x), "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__btl)(__UINT32_TYPE__ __x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__("btl %2, %1" : "=@ccc" (__result) : "g" (__x), "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__btcw)(__UINT16_TYPE__ *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__("btcw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__btcl)(__UINT32_TYPE__ *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__("btcl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__btrw)(__UINT16_TYPE__ *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__("btrw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__btrl)(__UINT32_TYPE__ *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__("btrl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__btsw)(__UINT16_TYPE__ *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__("btsw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__btsl)(__UINT32_TYPE__ *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__("btsl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL void (__clc)(void) { __asm__("clc" : : : "cc"); }
__FORCELOCAL void (__cmc)(void) { __asm__("cmc" : : : "cc"); }
__FORCELOCAL void (__stc)(void) { __asm__("stc" : : : "cc"); }
__FORCELOCAL void (__cld)(void) { __asm__("cld" /*: : : "cc"*/); }
__FORCELOCAL void (__std)(void) { __asm__("std" /*: : : "cc"*/); }
__FORCELOCAL void (__cli)(void) { __asm__("cli" /*: : : "cc"*/); }
__FORCELOCAL void (__sti)(void) { __asm__("sti" /*: : : "cc"*/); }
__FORCELOCAL void (__clflush)(void *__p) { __asm__("clflush %0" : : "m" (*(int *)__p)); }
__FORCELOCAL void (__cpuid)(__UINT32_TYPE__ __leaf, __UINT32_TYPE__ *__peax, __UINT32_TYPE__ *__pecx, __UINT32_TYPE__ *__pedx, __UINT32_TYPE__ *__pebx) { __asm__("cpuid" : "=a" (*__peax), "=c" (*__pecx), "=d" (*__pedx), "=b" (*__pebx) : "a" (__leaf)); }
__FORCELOCAL __UINT8_TYPE__ (__daa)(__UINT8_TYPE__ __x) { __UINT8_TYPE__ __result; __asm__("daa" : "=a" (__result) : "0" (__x) : "cc"); return __result; }
__FORCELOCAL __UINT8_TYPE__ (__dal)(__UINT8_TYPE__ __x) { __UINT8_TYPE__ __result; __asm__("dal" : "=a" (__result) : "0" (__x) : "cc"); return __result; }
__FORCELOCAL void (__hlt)(void) { __asm__("hlt"); }
__FORCELOCAL void (__into)(void) { __asm__("into"); }
__FORCELOCAL void (__int3)(void) { __asm__("int {$}3"); }
__FORCELOCAL void (__int_)(__UINT8_TYPE__ __intno) { __asm__("int %0" : : "N" (__intno)); }
__FORCELOCAL void (__invd)(void) { __asm__("invd"); }
__FORCELOCAL void (__wbinvd)(void) { __asm__("wbinvd"); }
__FORCELOCAL void (__invlpg)(void *__p) { __asm__("invlpg" : : "m" (*(int *)__p)); }
__FORCELOCAL void (__ldmxcsr)(__UINT32_TYPE__ __val) { __asm__("ldmxcsr" : : "m" (__val)); }
__FORCELOCAL __UINT32_TYPE__ (__stmxcsr)(void) { __UINT32_TYPE__ __result; __asm__("stmxcsr" : "=m" (__result)); return __result; }
__FORCELOCAL void (__lfence)(void) { __asm__("lfence"); }
__FORCELOCAL void (__sfence)(void) { __asm__("sfence"); }
__FORCELOCAL void (__mfence)(void) { __asm__("mfence"); }
__FORCELOCAL void (__lgdt)(void *__p) { typedef struct { __UINT16_TYPE__ __x[3]; } __T; __asm__("lgdt %0" : : "m" (*(__T *)__p)); }
__FORCELOCAL void (__lidt)(void *__p) { typedef struct { __UINT16_TYPE__ __x[3]; } __T; __asm__("lidt %0" : : "m" (*(__T *)__p)); }
__FORCELOCAL void (__sgdt)(void *__p) { typedef struct { __UINT16_TYPE__ __x[3]; } __T; __asm__("sgdt %0" : "=m" (*(__T *)__p)); }
__FORCELOCAL void (__sidt)(void *__p) { typedef struct { __UINT16_TYPE__ __x[3]; } __T; __asm__("sidt %0" : "=m" (*(__T *)__p)); }
__FORCELOCAL void (__lldt)(__UINT16_TYPE__ __x) { __asm__("lldt %0" : : "g" (__x)); }
__FORCELOCAL __UINT16_TYPE__ (__sldt)(void) { __UINT16_TYPE__ __result; __asm__("sldt %0" : "=g" (__result)); return __result; }
__FORCELOCAL void (__ud2)(void) { __asm__("ud2"); }
__FORCELOCAL void (__nop)(void) { __asm__("nop"); }
__FORCELOCAL void (__pause)(void) { __asm__("pause"); }
__FORCELOCAL __UINT32_TYPE__ (__getfl)(void) { __UINT32_TYPE__ __result; __asm__("pushfl; popl %0" : "=g" (__result)); return __result; }
__FORCELOCAL void (__setfl)(__UINT32_TYPE__ __fl) { __asm__("pushl %0; popfl" : : "g" (__fl) : "cc"); }
__FORCELOCAL void (__lmsw)(__UINT16_TYPE__ __x) { __asm__("lmsw %0" : : "g" (__x)); }
__FORCELOCAL __UINT16_TYPE__ (__smsw)(void) { __UINT16_TYPE__ __result; __asm__("smsw %0" : "=g" (__result)); return __result; }
__FORCELOCAL __UINT8_TYPE__ (__lahf)(void) { __UINT8_TYPE__ __result; __asm__("lahf" : "=a" (__result)); return __result; }
__FORCELOCAL void (__sahf)(__UINT8_TYPE__ __fl) { __asm__("sahf" : : "a" (__fl) : "cc"); }
__FORCELOCAL __UINT16_TYPE__ (__rolw)(__UINT16_TYPE__ __x, __UINT8_TYPE__ __y) { __UINT16_TYPE__ __result; __asm__("rolw %1, %w0" : "=g" (__result) : "nc" (__y), "0" (__x) : "cc"); return __result; }
__FORCELOCAL __UINT32_TYPE__ (__roll)(__UINT32_TYPE__ __x, __UINT8_TYPE__ __y) { __UINT32_TYPE__ __result; __asm__("roll %1, %0" : "=g" (__result) : "nc" (__y), "0" (__x) : "cc"); return __result; }
__FORCELOCAL __UINT16_TYPE__ (__rorw)(__UINT16_TYPE__ __x, __UINT8_TYPE__ __y) { __UINT16_TYPE__ __result; __asm__("rorw %1, %w0" : "=g" (__result) : "nc" (__y), "0" (__x) : "cc"); return __result; }
__FORCELOCAL __UINT32_TYPE__ (__rorl)(__UINT32_TYPE__ __x, __UINT8_TYPE__ __y) { __UINT32_TYPE__ __result; __asm__("rorl %1, %0" : "=g" (__result) : "nc" (__y), "0" (__x) : "cc"); return __result; }
__FORCELOCAL __UINT64_TYPE__ (__rdmsr)(__UINT32_TYPE__ __id) { __UINT64_TYPE__ __result; __asm__("rdmsr" : "=A" (__result) : "c" (__id)); return __result; }
__FORCELOCAL __UINT64_TYPE__ (__rdpmc)(__UINT32_TYPE__ __id) { __UINT64_TYPE__ __result; __asm__("rdpmc" : "=A" (__result) : "c" (__id)); return __result; }
__FORCELOCAL __UINT64_TYPE__ (__rdtsc)(void) { __UINT64_TYPE__ __result; __asm__("rdtsc" : "=A" (__result)); return __result; }
__FORCELOCAL void (__wrmsr)(__UINT32_TYPE__ __id, __UINT64_TYPE__ __val) { __asm__("wrmsr" : : "c" (__id), "A" (__val)); }
__FORCELOCAL __ATTR_NORETURN void (__sysenter)(void) { __asm__("sysenter"); __builtin_unreachable(); }
__FORCELOCAL __ATTR_NORETURN void (__sysexit)(__UINT32_TYPE__ __uesp, __UINT32_TYPE__ __ueip) { __asm__("sysexit" : : "c" (__uesp), "d" (__ueip)); __builtin_unreachable(); }

#if !defined(__KERNEL__) || !defined(CONFIG_NO_SMP)
#define __X86_LOCK_PREFIX "lock;"
#else
#define __X86_LOCK_PREFIX /* nothing */
#endif
__FORCELOCAL __BOOL (__lock_btcw)(__UINT16_TYPE__ volatile *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btcw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__lock_btcl)(__UINT32_TYPE__ volatile *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btcl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__lock_btrw)(__UINT16_TYPE__ volatile *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btrw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__lock_btrl)(__UINT32_TYPE__ volatile *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btrl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__lock_btsw)(__UINT16_TYPE__ volatile *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btsw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__lock_btsl)(__UINT32_TYPE__ volatile *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btsl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL void (__lock_rolw)(__UINT16_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "rolw %1, %w0" : "+g" (*__x) : "nc" (__y) : "cc"); }
__FORCELOCAL void (__lock_roll)(__UINT32_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "roll %1, %0" : "+g" (*__x) : "nc" (__y) : "cc"); }
__FORCELOCAL void (__lock_rorw)(__UINT16_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "rorw %1, %w0" : "+g" (*__x) : "nc" (__y) : "cc"); }
__FORCELOCAL void (__lock_rorl)(__UINT32_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "rorl %1, %0" : "+g" (*__x) : "nc" (__y) : "cc"); }
#undef __X86_LOCK_PREFIX

__SYSDECL_END

#endif /* !_KOS_I386_KOS_INTRIN_H */
