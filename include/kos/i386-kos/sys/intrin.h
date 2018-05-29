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
#ifndef _KOS_I386_KOS_SYS_INTRIN_H
#define _KOS_I386_KOS_SYS_INTRIN_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>
#include <hybrid/host.h>

__SYSDECL_BEGIN

#ifdef __CC__
__FORCELOCAL __BOOL (__verr)(__UINT16_TYPE__ __seg) { __BOOL __result; __asm__("verr %w1" : "=@ccz" (__result) : "g" (__seg)); return __result; }
__FORCELOCAL __BOOL (__verw)(__UINT16_TYPE__ __seg) { __BOOL __result; __asm__("verw %w1" : "=@ccz" (__result) : "g" (__seg)); return __result; }
__FORCELOCAL __UINT16_TYPE__ (__bswapw)(__UINT16_TYPE__ __x) { register __UINT16_TYPE__ __result; __asm__("xchgb %b0, %h0" : "=q" (__result) : "0" (__x)); return __result; }
__FORCELOCAL __UINT32_TYPE__ (__bswapl)(__UINT32_TYPE__ __x) { register __UINT32_TYPE__ __result; __asm__("bswap %0" : "=r" (__result) : "0" (__x)); return __result; }
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
__FORCELOCAL void (__cld)(void) { __COMPILER_BARRIER(); __asm__ __volatile__("cld" /*: : : "cc"*/); __COMPILER_BARRIER(); }
__FORCELOCAL void (__std)(void) { __COMPILER_BARRIER(); __asm__ __volatile__("std" /*: : : "cc"*/); __COMPILER_BARRIER(); }
__FORCELOCAL void (__cli)(void) { __asm__ __volatile__("cli" : : : "memory"); }
__FORCELOCAL void (__sti)(void) { __asm__ __volatile__("sti" : : : "memory"); }
__FORCELOCAL void (__clflush)(void *__p) { struct __cl { __BYTE_TYPE__ __b[64]; }; __asm__ __volatile__("clflush %0" : : "m" (*(struct __cl *)__p)); }
__FORCELOCAL void (__cpuid)(__UINT32_TYPE__ __leaf, __UINT32_TYPE__ *__peax, __UINT32_TYPE__ *__pecx, __UINT32_TYPE__ *__pedx, __UINT32_TYPE__ *__pebx) { __asm__("cpuid" : "=a" (*__peax), "=c" (*__pecx), "=d" (*__pedx), "=b" (*__pebx) : "a" (__leaf)); }
__FORCELOCAL __UINT8_TYPE__ (__daa)(__UINT8_TYPE__ __x) { __UINT8_TYPE__ __result; __asm__("daa" : "=a" (__result) : "0" (__x) : "cc"); return __result; }
__FORCELOCAL __UINT8_TYPE__ (__dal)(__UINT8_TYPE__ __x) { __UINT8_TYPE__ __result; __asm__("dal" : "=a" (__result) : "0" (__x) : "cc"); return __result; }
__FORCELOCAL void (__hlt)(void) { __asm__ __volatile__("hlt" : : : "memory"); }
__FORCELOCAL void (__into)(void) { __asm__("into"); }
__FORCELOCAL void (__int3)(void) { __asm__("int {$}3"); }
#if defined(__INTELLISENSE__) || (defined(__OPTIMIZE__) && 0)
__FORCELOCAL void (__int)(__UINT8_TYPE__ __intno) { __asm__("int %0" : : "N" (__intno)); }
#else
#define __int(intno) __XBLOCK({ __asm__("int {$}" __PP_STR(intno)); (void)0; })
#endif
__FORCELOCAL void (__invd)(void) { __asm__ __volatile__("invd"); }
__FORCELOCAL void (__wbinvd)(void) { __asm__ __volatile__("wbinvd"); }
__FORCELOCAL void (__invlpg)(void *__p) { __asm__ __volatile__("invlpg" : : "m" (*(int *)__p)); }
__FORCELOCAL void (__lfence)(void) { __COMPILER_READ_BARRIER(); __asm__ __volatile__("lfence"); __COMPILER_READ_BARRIER(); }
__FORCELOCAL void (__sfence)(void) { __COMPILER_WRITE_BARRIER(); __asm__ __volatile__("sfence"); __COMPILER_WRITE_BARRIER(); }
__FORCELOCAL void (__mfence)(void) { __COMPILER_BARRIER(); __asm__ __volatile__("mfence"); __COMPILER_BARRIER(); }
__FORCELOCAL void (__lgdt)(void const *__p) { typedef struct { __UINT16_TYPE__ __x[3]; } __T; __asm__ __volatile__("lgdt %0" : : "m" (*(__T const *)__p)); }
__FORCELOCAL void (__lidt)(void const *__p) { typedef struct { __UINT16_TYPE__ __x[3]; } __T; __asm__ __volatile__("lidt %0" : : "m" (*(__T const *)__p)); }
__FORCELOCAL void (__sgdt)(void *__p) { typedef struct { __UINT16_TYPE__ __x[3]; } __T; __asm__ __volatile__("sgdt %0" : "=m" (*(__T *)__p)); }
__FORCELOCAL void (__sidt)(void *__p) { typedef struct { __UINT16_TYPE__ __x[3]; } __T; __asm__ __volatile__("sidt %0" : "=m" (*(__T *)__p)); }
__FORCELOCAL void (__lldt)(__UINT16_TYPE__ __x) { __asm__ __volatile__("lldt %0" : : "g" (__x)); }
__FORCELOCAL __UINT16_TYPE__ (__sldt)(void) { __UINT16_TYPE__ __result; __asm__ __volatile__("sldt %0" : "=g" (__result)); return __result; }
__FORCELOCAL void (__ud2)(void) { __asm__ __volatile__("ud2"); }
__FORCELOCAL void (__nop)(void) { __asm__("nop"); }
__FORCELOCAL void (__pause)(void) { __asm__("pause"); }
__FORCELOCAL __UINT32_TYPE__ (__getfl)(void) { __UINT32_TYPE__ __result; __asm__("pushfl; popl %0" : "=g" (__result)); return __result; }
__FORCELOCAL void (__setfl)(__UINT32_TYPE__ __fl) { __asm__("pushl %0; popfl" : : "g" (__fl) : "cc"); }
__FORCELOCAL void (__lmsw)(__UINT16_TYPE__ __x) { __asm__ __volatile__("lmsw %0" : : "g" (__x)); }
__FORCELOCAL __UINT16_TYPE__ (__smsw)(void) { __UINT16_TYPE__ __result; __asm__ __volatile__("smsw %0" : "=g" (__result)); return __result; }
__FORCELOCAL __UINT8_TYPE__ (__lahf)(void) { __UINT8_TYPE__ __result; __asm__("lahf" : "=a" (__result)); return __result; }
__FORCELOCAL void (__sahf)(__UINT8_TYPE__ __fl) { __asm__("sahf" : : "a" (__fl) : "cc"); }
__FORCELOCAL __UINT16_TYPE__ (__rolw)(__UINT16_TYPE__ __x, __UINT8_TYPE__ __y) { __UINT16_TYPE__ __result; __asm__("rolw %1, %w0" : "=g" (__result) : "nc" (__y), "0" (__x) : "cc"); return __result; }
__FORCELOCAL __UINT32_TYPE__ (__roll)(__UINT32_TYPE__ __x, __UINT8_TYPE__ __y) { __UINT32_TYPE__ __result; __asm__("roll %1, %0" : "=g" (__result) : "nc" (__y), "0" (__x) : "cc"); return __result; }
__FORCELOCAL __UINT16_TYPE__ (__rorw)(__UINT16_TYPE__ __x, __UINT8_TYPE__ __y) { __UINT16_TYPE__ __result; __asm__("rorw %1, %w0" : "=g" (__result) : "nc" (__y), "0" (__x) : "cc"); return __result; }
__FORCELOCAL __UINT32_TYPE__ (__rorl)(__UINT32_TYPE__ __x, __UINT8_TYPE__ __y) { __UINT32_TYPE__ __result; __asm__("rorl %1, %0" : "=g" (__result) : "nc" (__y), "0" (__x) : "cc"); return __result; }
__FORCELOCAL __UINT64_TYPE__ (__rdmsr)(__UINT32_TYPE__ __id) { __UINT64_TYPE__ __result; __asm__ __volatile__("rdmsr" : "=A" (__result) : "c" (__id)); return __result; }
__FORCELOCAL __UINT64_TYPE__ (__rdpmc)(__UINT32_TYPE__ __id) { __UINT64_TYPE__ __result; __asm__ __volatile__("rdpmc" : "=A" (__result) : "c" (__id)); return __result; }
__FORCELOCAL __UINT64_TYPE__ (__rdtsc)(void) { __UINT64_TYPE__ __result; __asm__ __volatile__("rdtsc" : "=A" (__result)); return __result; }
__FORCELOCAL void (__wrmsr)(__UINT32_TYPE__ __id, __UINT64_TYPE__ __val) { __asm__ __volatile__("wrmsr" : : "c" (__id), "A" (__val)); }
__FORCELOCAL __ATTR_NORETURN void (__sysenter)(void) { __asm__ __volatile__("sysenter"); __builtin_unreachable(); }
__FORCELOCAL __ATTR_NORETURN void (__sysexit)(__UINT32_TYPE__ __uesp, __UINT32_TYPE__ __ueip) { __asm__ __volatile__("sysexit" : : "c" (__uesp), "d" (__ueip)); __builtin_unreachable(); }
__FORCELOCAL void (__movsb)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __count) { __asm__("rep; movsb" : "+D" (__dst), "+S" (__src), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT8_TYPE__,__count,__dst)) : "m" (__COMPILER_ASM_BUFFER(__UINT8_TYPE__,__count,__src))); }
__FORCELOCAL void (__movsw)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __count) { __asm__("rep; movsw" : "+D" (__dst), "+S" (__src), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT16_TYPE__,__count,__dst)) : "m" (__COMPILER_ASM_BUFFER(__UINT16_TYPE__,__count,__src))); }
__FORCELOCAL void (__movsl)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __count) { __asm__("rep; movsl" : "+D" (__dst), "+S" (__src), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT32_TYPE__,__count,__dst)) : "m" (__COMPILER_ASM_BUFFER(__UINT32_TYPE__,__count,__src))); }
__FORCELOCAL void (__stosb)(void *__restrict __dst, __UINT8_TYPE__  __byte , __SIZE_TYPE__ __count) { __asm__("rep; stosb" : "+D" (__dst), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT8_TYPE__,__count,__dst)) : "a" (__byte)); }
__FORCELOCAL void (__stosw)(void *__restrict __dst, __UINT16_TYPE__ __word , __SIZE_TYPE__ __count) { __asm__("rep; stosw" : "+D" (__dst), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT16_TYPE__,__count,__dst)) : "a" (__word)); }
__FORCELOCAL void (__stosl)(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __count) { __asm__("rep; stosl" : "+D" (__dst), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT32_TYPE__,__count,__dst)) : "a" (__dword)); }
#ifdef __x86_64__
__FORCELOCAL void (__movsq)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __count) { __asm__("rep; movsq" : "+D" (__dst), "+S" (__src), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT64_TYPE__,__count,__dst)) : "m" (__COMPILER_ASM_BUFFER(__UINT64_TYPE__,__count,__src))); }
__FORCELOCAL void (__stosq)(void *__restrict __dst, __UINT64_TYPE__ __dword, __SIZE_TYPE__ __count) { __asm__("rep; stosq" : "+D" (__dst), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT64_TYPE__,__count,__dst)) : "a" (__dword)); }
#endif

/* Floating point intrinsics. */
__FORCELOCAL void (__ldmxcsr)(__UINT32_TYPE__ __val) { __asm__ __volatile__("ldmxcsr" : : "m" (__val)); }
__FORCELOCAL __UINT32_TYPE__ (__stmxcsr)(void) { __UINT32_TYPE__ __result; __asm__ __volatile__("stmxcsr" : "=m" (__result)); return __result; }
__FORCELOCAL void (__clts)(void) { __asm__ __volatile__("clts"); }
__FORCELOCAL void (__fldcw)(__UINT16_TYPE__ __cw) { __asm__ __volatile__("fldcw %0" : : "m" (__cw)); }
__FORCELOCAL void (__fxsave)(__BYTE_TYPE__ __data[512]) { __asm__ __volatile__("fxsave %0" : "=m" (*__data)); }
__FORCELOCAL void (__fxrstor)(__BYTE_TYPE__ const __data[512]) { __asm__ __volatile__("fxrstor %0" : : "m" (*__data)); }
__FORCELOCAL void (__fninit)(void) { __asm__ __volatile__("fninit"); }

/* Read/Write control registers. */
__FORCELOCAL __REGISTER_TYPE__ (__rdcr0)(void) { register __REGISTER_TYPE__ __result; __asm__ __volatile__("mov %%cr0, %0" : "=r" (__result)); return __result; }
__FORCELOCAL __REGISTER_TYPE__ (__rdcr2)(void) { register __REGISTER_TYPE__ __result; __asm__ __volatile__("mov %%cr2, %0" : "=r" (__result)); return __result; }
__FORCELOCAL __REGISTER_TYPE__ (__rdcr3)(void) { register __REGISTER_TYPE__ __result; __asm__ __volatile__("mov %%cr3, %0" : "=r" (__result)); return __result; }
__FORCELOCAL __REGISTER_TYPE__ (__rdcr4)(void) { register __REGISTER_TYPE__ __result; __asm__ __volatile__("mov %%cr4, %0" : "=r" (__result)); return __result; }
__FORCELOCAL void (__wrcr0)(__REGISTER_TYPE__ __val) { __asm__ __volatile__("mov %0, %%cr0" : : "r" (__val)); }
__FORCELOCAL void (__wrcr2)(__REGISTER_TYPE__ __val) { __asm__ __volatile__("mov %0, %%cr2" : : "r" (__val)); }
__FORCELOCAL void (__wrcr3)(__REGISTER_TYPE__ __val) { __asm__ __volatile__("mov %0, %%cr3" : : "r" (__val)); }
__FORCELOCAL void (__wrcr4)(__REGISTER_TYPE__ __val) { __asm__ __volatile__("mov %0, %%cr4" : : "r" (__val)); }


#if !defined(__KERNEL__) || !defined(CONFIG_NO_SMP)
#define __X86_LOCK_PREFIX "lock;"
#else
#define __X86_LOCK_PREFIX /* nothing */
#endif
__FORCELOCAL __BOOL (__atomic_btcw)(__UINT16_TYPE__ volatile *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btcw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__atomic_btcl)(__UINT32_TYPE__ volatile *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btcl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__atomic_btrw)(__UINT16_TYPE__ volatile *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btrw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__atomic_btrl)(__UINT32_TYPE__ volatile *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btrl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__atomic_btsw)(__UINT16_TYPE__ volatile *__x, __UINT16_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btsw %2, %w1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL __BOOL (__atomic_btsl)(__UINT32_TYPE__ volatile *__x, __UINT32_TYPE__ __bitindex) { __BOOL __result; __asm__(__X86_LOCK_PREFIX "btsl %2, %1" : "=@ccc" (__result), "+g" (*__x) : "nr" (__bitindex)); return __result; }
__FORCELOCAL void (__atomic_rolw)(__UINT16_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "rolw %1, %w0" : "+g" (*__x) : "nc" (__y) : "cc", "memory"); }
__FORCELOCAL void (__atomic_roll)(__UINT32_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "roll %1, %0" : "+g" (*__x) : "nc" (__y) : "cc", "memory"); }
__FORCELOCAL void (__atomic_rorw)(__UINT16_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "rorw %1, %w0" : "+g" (*__x) : "nc" (__y) : "cc", "memory"); }
__FORCELOCAL void (__atomic_rorl)(__UINT32_TYPE__ volatile *__x, __UINT8_TYPE__ __y) { __asm__(__X86_LOCK_PREFIX "rorl %1, %0" : "+g" (*__x) : "nc" (__y) : "cc", "memory"); }
#undef __X86_LOCK_PREFIX


/* Segment read/write operators. */
__FORCELOCAL __UINT8_TYPE__ (__readfsb)(__UINTPTR_TYPE__ __offset) { register __UINT8_TYPE__ __result; __asm__("movb %%fs:%1, %0" : "=q" (__result) : "m" (*(__UINT8_TYPE__ *)__offset)); return __result; }
__FORCELOCAL __UINT8_TYPE__ (__readgsb)(__UINTPTR_TYPE__ __offset) { register __UINT8_TYPE__ __result; __asm__("movb %%gs:%1, %0" : "=q" (__result) : "m" (*(__UINT8_TYPE__ *)__offset)); return __result; }
__FORCELOCAL __UINT16_TYPE__ (__readfsw)(__UINTPTR_TYPE__ __offset) { register __UINT16_TYPE__ __result; __asm__("movw %%fs:%1, %0" : "=r" (__result) : "m" (*(__UINT16_TYPE__ *)__offset)); return __result; }
__FORCELOCAL __UINT16_TYPE__ (__readgsw)(__UINTPTR_TYPE__ __offset) { register __UINT16_TYPE__ __result; __asm__("movw %%gs:%1, %0" : "=r" (__result) : "m" (*(__UINT16_TYPE__ *)__offset)); return __result; }
__FORCELOCAL __UINT32_TYPE__ (__readfsl)(__UINTPTR_TYPE__ __offset) { register __UINT32_TYPE__ __result; __asm__("movl %%fs:%1, %0" : "=r" (__result) : "m" (*(__UINT32_TYPE__ *)__offset)); return __result; }
__FORCELOCAL __UINT32_TYPE__ (__readgsl)(__UINTPTR_TYPE__ __offset) { register __UINT32_TYPE__ __result; __asm__("movl %%gs:%1, %0" : "=r" (__result) : "m" (*(__UINT32_TYPE__ *)__offset)); return __result; }
__FORCELOCAL void (__writefsb)(__UINTPTR_TYPE__ __offset, __UINT8_TYPE__ __val) { __asm__("movb %1, %%fs:%0" : : "m" (*(__UINT8_TYPE__ *)__offset), "q" (__val)); }
__FORCELOCAL void (__writegsb)(__UINTPTR_TYPE__ __offset, __UINT8_TYPE__ __val) { __asm__("movb %1, %%gs:%0" : : "m" (*(__UINT8_TYPE__ *)__offset), "q" (__val)); }
__FORCELOCAL void (__writefsw)(__UINTPTR_TYPE__ __offset, __UINT16_TYPE__ __val) { __asm__("movw %w1, %%fs:%0" : : "m" (*(__UINT16_TYPE__ *)__offset), "r" (__val)); }
__FORCELOCAL void (__writegsw)(__UINTPTR_TYPE__ __offset, __UINT16_TYPE__ __val) { __asm__("movw %w1, %%gs:%0" : : "m" (*(__UINT16_TYPE__ *)__offset), "r" (__val)); }
__FORCELOCAL void (__writefsl)(__UINTPTR_TYPE__ __offset, __UINT32_TYPE__ __val) { __asm__("movl %d1, %%fs:%0" : : "m" (*(__UINT32_TYPE__ *)__offset), "r" (__val)); }
__FORCELOCAL void (__writegsl)(__UINTPTR_TYPE__ __offset, __UINT32_TYPE__ __val) { __asm__("movl %d1, %%gs:%0" : : "m" (*(__UINT32_TYPE__ *)__offset), "r" (__val)); }
#ifdef __x86_64__
__FORCELOCAL __UINT64_TYPE__ (__readfsq)(__UINTPTR_TYPE__ __offset) { register __UINT64_TYPE__ __result; __asm__("movq %%fs:%1, %0" : "=r" (__result) : "m" (*(__UINT64_TYPE__ *)__offset)); return __result; }
__FORCELOCAL __UINT64_TYPE__ (__readgsq)(__UINTPTR_TYPE__ __offset) { register __UINT64_TYPE__ __result; __asm__("movq %%gs:%1, %0" : "=r" (__result) : "m" (*(__UINT64_TYPE__ *)__offset)); return __result; }
__FORCELOCAL void (__writefsq)(__UINTPTR_TYPE__ __offset, __UINT64_TYPE__ __val) { __asm__("movq %1, %%fs:%0" : : "m" (*(__UINT64_TYPE__ *)__offset), "r" (__val)); }
__FORCELOCAL void (__writegsq)(__UINTPTR_TYPE__ __offset, __UINT64_TYPE__ __val) { __asm__("movq %1, %%gs:%0" : : "m" (*(__UINT64_TYPE__ *)__offset), "r" (__val)); }
__FORCELOCAL void *(__readfsptr)(__UINTPTR_TYPE__ __offset) { return (void *)__readfsq(__offset); }
__FORCELOCAL void *(__readgsptr)(__UINTPTR_TYPE__ __offset) { return (void *)__readgsq(__offset); }
__FORCELOCAL void (__writefsptr)(__UINTPTR_TYPE__ __offset, void *__val) { __writefsq(__offset,(__UINT64_TYPE__)__val); }
__FORCELOCAL void (__writegsptr)(__UINTPTR_TYPE__ __offset, void *__val) { __writegsq(__offset,(__UINT64_TYPE__)__val); }
#else /* __x86_64__ */
__FORCELOCAL void *(__readfsptr)(__UINTPTR_TYPE__ __offset) { return (void *)__readfsl(__offset); }
__FORCELOCAL void *(__readgsptr)(__UINTPTR_TYPE__ __offset) { return (void *)__readgsl(__offset); }
__FORCELOCAL void (__writefsptr)(__UINTPTR_TYPE__ __offset, void *__val) { __writefsl(__offset,(__UINT32_TYPE__)__val); }
__FORCELOCAL void (__writegsptr)(__UINTPTR_TYPE__ __offset, void *__val) { __writegsl(__offset,(__UINT32_TYPE__)__val); }
#endif /* !__x86_64__ */


#ifdef __x86_64__
#ifdef __KERNEL__
#include <i386-kos/segment.h>
__FORCELOCAL __UINT32_TYPE__ (__rdfsbasel)(void) { register __UINT32_TYPE__ __result; __asm__("safe_rdfsbase %d0" : "=r" (__result)); return __result; }
__FORCELOCAL __UINT32_TYPE__ (__rdgsbasel)(void) { register __UINT32_TYPE__ __result; __asm__("safe_rdgsbase %d0" : "=r" (__result)); return __result; }
__FORCELOCAL void (__wrfsbasel)(__UINT32_TYPE__ __val) { __asm__("safe_wrfsbase %d0" : : "r" (__val)); }
__FORCELOCAL void (__wrgsbasel)(__UINT32_TYPE__ __val) { __asm__("safe_wrgsbase %d0" : : "r" (__val)); }
__FORCELOCAL __UINT64_TYPE__ (__rdfsbaseq)(void) { register __UINT64_TYPE__ __result; __asm__("safe_rdfsbase %0" : "=r" (__result)); return __result; }
__FORCELOCAL __UINT64_TYPE__ (__rdgsbaseq)(void) { register __UINT64_TYPE__ __result; __asm__("safe_rdgsbase %0" : "=r" (__result)); return __result; }
__FORCELOCAL void (__wrfsbaseq)(__UINT64_TYPE__ __val) { __asm__("safe_wrfsbase %0" : : "r" (__val)); }
__FORCELOCAL void (__wrgsbaseq)(__UINT64_TYPE__ __val) { __asm__("safe_wrgsbase %0" : : "r" (__val)); }
#else
__FORCELOCAL __UINT32_TYPE__ (__rdfsbasel)(void) { register __UINT32_TYPE__ __result; __asm__("rdfsbase %d0" : "=r" (__result)); return __result; }
__FORCELOCAL __UINT32_TYPE__ (__rdgsbasel)(void) { register __UINT32_TYPE__ __result; __asm__("rdgsbase %d0" : "=r" (__result)); return __result; }
__FORCELOCAL void (__wrfsbasel)(__UINT32_TYPE__ __val) { __asm__("wrfsbase %d0" : : "r" (__val)); }
__FORCELOCAL void (__wrgsbasel)(__UINT32_TYPE__ __val) { __asm__("wrgsbase %d0" : : "r" (__val)); }
__FORCELOCAL __UINT64_TYPE__ (__rdfsbaseq)(void) { register __UINT64_TYPE__ __result; __asm__("rdfsbase %0" : "=r" (__result)); return __result; }
__FORCELOCAL __UINT64_TYPE__ (__rdgsbaseq)(void) { register __UINT64_TYPE__ __result; __asm__("rdgsbase %0" : "=r" (__result)); return __result; }
__FORCELOCAL void (__wrfsbaseq)(__UINT64_TYPE__ __val) { __asm__("wrfsbase %0" : : "r" (__val)); }
__FORCELOCAL void (__wrgsbaseq)(__UINT64_TYPE__ __val) { __asm__("wrgsbase %0" : : "r" (__val)); }
#endif
__FORCELOCAL void (__swapgs)(void) { __asm__("swapgs"); }
#endif
#endif /* __CC__ */

__SYSDECL_END

#endif /* !_KOS_I386_KOS_SYS_INTRIN_H */
