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

#ifndef __CPU_INTRIN_SYM
#define __CPU_INTRIN_SYM(name)              __##name
#define __CPU_INTRIN_FUNC(name) (__LIBCCALL __##name)
#endif


#ifdef __CC__
__FORCELOCAL __WUNUSED __BOOL __CPU_INTRIN_FUNC(verr)(__UINT16_TYPE__ __seg) { __BOOL __result; __asm__("verr %w1" : "=@ccz" (__result) : "g" (__seg)); return __result; }
__FORCELOCAL __WUNUSED __BOOL __CPU_INTRIN_FUNC(verw)(__UINT16_TYPE__ __seg) { __BOOL __result; __asm__("verw %w1" : "=@ccz" (__result) : "g" (__seg)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(clc)(void) { __asm__("clc" : : : "cc"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(cmc)(void) { __asm__("cmc" : : : "cc"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(stc)(void) { __asm__("stc" : : : "cc"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(cld)(void) { __COMPILER_BARRIER(); __asm__ __volatile__("cld" /*: : : "cc"*/); __COMPILER_BARRIER(); }
__FORCELOCAL void __CPU_INTRIN_FUNC(std)(void) { __COMPILER_BARRIER(); __asm__ __volatile__("std" /*: : : "cc"*/); __COMPILER_BARRIER(); }
__FORCELOCAL void __CPU_INTRIN_FUNC(cli)(void) { __asm__ __volatile__("cli" : : : "memory"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(sti)(void) { __asm__ __volatile__("sti" : : : "memory"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(clflush)(void *__p) { struct __cl { __BYTE_TYPE__ __b[64]; }; __asm__ __volatile__("clflush %0" : : "m" (*(struct __cl *)__p)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(cpuid)(__UINT32_TYPE__ __leaf, __UINT32_TYPE__ *__peax, __UINT32_TYPE__ *__pecx, __UINT32_TYPE__ *__pedx, __UINT32_TYPE__ *__pebx) { __asm__("cpuid" : "=a" (*__peax), "=c" (*__pecx), "=d" (*__pedx), "=b" (*__pebx) : "a" (__leaf)); }
__FORCELOCAL __WUNUSED __UINT8_TYPE__ __CPU_INTRIN_FUNC(daa)(__UINT8_TYPE__ __x) { __UINT8_TYPE__ __result; __asm__("daa" : "=a" (__result) : "0" (__x) : "cc"); return __result; }
__FORCELOCAL __WUNUSED __UINT8_TYPE__ __CPU_INTRIN_FUNC(dal)(__UINT8_TYPE__ __x) { __UINT8_TYPE__ __result; __asm__("dal" : "=a" (__result) : "0" (__x) : "cc"); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(hlt)(void) { __asm__ __volatile__("hlt" : : : "memory"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(into)(void) { __asm__("into"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(int3)(void) { __asm__("int {$}3"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(int)(__UINT8_TYPE__ __intno) { __asm__("int %0" : : "N" (__intno)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(invd)(void) { __asm__ __volatile__("invd"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(wbinvd)(void) { __asm__ __volatile__("wbinvd"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(invlpg)(void *__p) { __asm__ __volatile__("invlpg" : : "m" (*(int *)__p)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(lfence)(void) { __COMPILER_READ_BARRIER(); __asm__ __volatile__("lfence"); __COMPILER_READ_BARRIER(); }
__FORCELOCAL void __CPU_INTRIN_FUNC(sfence)(void) { __COMPILER_WRITE_BARRIER(); __asm__ __volatile__("sfence"); __COMPILER_WRITE_BARRIER(); }
__FORCELOCAL void __CPU_INTRIN_FUNC(mfence)(void) { __COMPILER_BARRIER(); __asm__ __volatile__("mfence"); __COMPILER_BARRIER(); }
__FORCELOCAL void __CPU_INTRIN_FUNC(lgdt)(__UINT16_TYPE__ __limit, void *__base) { struct __ATTR_PACKED { __UINT16_TYPE__ __limit; void *__base; } __arg; __arg.__limit = __limit; __arg.__base = __base; __asm__ __volatile__("lgdt %0" : : "m" (__arg)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(lidt)(__UINT16_TYPE__ __limit, void *__base) { struct __ATTR_PACKED { __UINT16_TYPE__ __limit; void *__base; } __arg; __arg.__limit = __limit; __arg.__base = __base; __asm__ __volatile__("lidt %0" : : "m" (__arg)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(sgdt)(void *__p) { typedef struct { __UINT16_TYPE__ __x[3]; } __T; __asm__ __volatile__("sgdt %0" : "=m" (*(__T *)__p)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(sidt)(void *__p) { typedef struct { __UINT16_TYPE__ __x[3]; } __T; __asm__ __volatile__("sidt %0" : "=m" (*(__T *)__p)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(lldt)(__UINT16_TYPE__ __x) { __asm__ __volatile__("lldt %0" : : "g" (__x)); }
__FORCELOCAL __WUNUSED __UINT16_TYPE__ __CPU_INTRIN_FUNC(sldt)(void) { __UINT16_TYPE__ __result; __asm__ __volatile__("sldt %0" : "=g" (__result)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(ud2)(void) { __asm__ __volatile__("ud2"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(nop)(void) { __asm__("nop"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(pause)(void) { __asm__("pause"); }
__FORCELOCAL __WUNUSED __UINT32_TYPE__ __CPU_INTRIN_FUNC(getfl)(void) { __UINT32_TYPE__ __result; __asm__("pushfl; popl %0" : "=g" (__result)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(setfl)(__UINT32_TYPE__ __fl) { __asm__("pushl %0; popfl" : : "g" (__fl) : "cc"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(lmsw)(__UINT16_TYPE__ __x) { __asm__ __volatile__("lmsw %0" : : "g" (__x)); }
__FORCELOCAL __WUNUSED __UINT16_TYPE__ __CPU_INTRIN_FUNC(smsw)(void) { __UINT16_TYPE__ __result; __asm__ __volatile__("smsw %0" : "=g" (__result)); return __result; }
__FORCELOCAL __WUNUSED __UINT8_TYPE__ __CPU_INTRIN_FUNC(lahf)(void) { __UINT8_TYPE__ __result; __asm__("lahf" : "=a" (__result)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(sahf)(__UINT8_TYPE__ __fl) { __asm__("sahf" : : "a" (__fl) : "cc"); }
__FORCELOCAL __WUNUSED __UINT16_TYPE__ __CPU_INTRIN_FUNC(rolw)(__UINT16_TYPE__ __x, __UINT8_TYPE__ __y) { __UINT16_TYPE__ __result; __asm__("rolw %1, %w0" : "=g" (__result) : "nc" (__y), "0" (__x) : "cc"); return __result; }
__FORCELOCAL __WUNUSED __UINT32_TYPE__ __CPU_INTRIN_FUNC(roll)(__UINT32_TYPE__ __x, __UINT8_TYPE__ __y) { __UINT32_TYPE__ __result; __asm__("roll %1, %0" : "=g" (__result) : "nc" (__y), "0" (__x) : "cc"); return __result; }
__FORCELOCAL __WUNUSED __UINT16_TYPE__ __CPU_INTRIN_FUNC(rorw)(__UINT16_TYPE__ __x, __UINT8_TYPE__ __y) { __UINT16_TYPE__ __result; __asm__("rorw %1, %w0" : "=g" (__result) : "nc" (__y), "0" (__x) : "cc"); return __result; }
__FORCELOCAL __WUNUSED __UINT32_TYPE__ __CPU_INTRIN_FUNC(rorl)(__UINT32_TYPE__ __x, __UINT8_TYPE__ __y) { __UINT32_TYPE__ __result; __asm__("rorl %1, %0" : "=g" (__result) : "nc" (__y), "0" (__x) : "cc"); return __result; }
__FORCELOCAL __ATTR_NORETURN void __CPU_INTRIN_FUNC(sysenter)(void) { __asm__ __volatile__("sysenter"); __builtin_unreachable(); }
__FORCELOCAL __ATTR_NORETURN void __CPU_INTRIN_FUNC(sysexit)(__UINT32_TYPE__ __uesp, __UINT32_TYPE__ __ueip) { __asm__ __volatile__("sysexit" : : "c" (__uesp), "d" (__ueip)); __builtin_unreachable(); }
__FORCELOCAL void __CPU_INTRIN_FUNC(movsb)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __count) { __asm__("rep; movsb" : "+D" (__dst), "+S" (__src), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT8_TYPE__,__count,__dst)) : "m" (__COMPILER_ASM_BUFFER(__UINT8_TYPE__,__count,__src))); }
__FORCELOCAL void __CPU_INTRIN_FUNC(movsw)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __count) { __asm__("rep; movsw" : "+D" (__dst), "+S" (__src), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT16_TYPE__,__count,__dst)) : "m" (__COMPILER_ASM_BUFFER(__UINT16_TYPE__,__count,__src))); }
__FORCELOCAL void __CPU_INTRIN_FUNC(movsl)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __count) { __asm__("rep; movsl" : "+D" (__dst), "+S" (__src), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT32_TYPE__,__count,__dst)) : "m" (__COMPILER_ASM_BUFFER(__UINT32_TYPE__,__count,__src))); }
__FORCELOCAL void __CPU_INTRIN_FUNC(stosb)(void *__restrict __dst, __UINT8_TYPE__  __byte , __SIZE_TYPE__ __count) { __asm__("rep; stosb" : "+D" (__dst), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT8_TYPE__,__count,__dst)) : "a" (__byte)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(stosw)(void *__restrict __dst, __UINT16_TYPE__ __word , __SIZE_TYPE__ __count) { __asm__("rep; stosw" : "+D" (__dst), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT16_TYPE__,__count,__dst)) : "a" (__word)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(stosl)(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __count) { __asm__("rep; stosl" : "+D" (__dst), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT32_TYPE__,__count,__dst)) : "a" (__dword)); }
#ifdef __x86_64__
__FORCELOCAL void __CPU_INTRIN_FUNC(movsq)(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __count) { __asm__("rep; movsq" : "+D" (__dst), "+S" (__src), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT64_TYPE__,__count,__dst)) : "m" (__COMPILER_ASM_BUFFER(__UINT64_TYPE__,__count,__src))); }
__FORCELOCAL void __CPU_INTRIN_FUNC(stosq)(void *__restrict __dst, __UINT64_TYPE__ __dword, __SIZE_TYPE__ __count) { __asm__("rep; stosq" : "+D" (__dst), "+c" (__count), "=m" (__COMPILER_ASM_BUFFER(__UINT64_TYPE__,__count,__dst)) : "a" (__dword)); }
#endif

/* Floating point intrinsics. */
__FORCELOCAL void __CPU_INTRIN_FUNC(ldmxcsr)(__UINT32_TYPE__ __val) { __asm__ __volatile__("ldmxcsr" : : "m" (__val)); }
__FORCELOCAL __WUNUSED __UINT32_TYPE__ __CPU_INTRIN_FUNC(stmxcsr)(void) { __UINT32_TYPE__ __result; __asm__ __volatile__("stmxcsr" : "=m" (__result)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(clts)(void) { __asm__ __volatile__("clts"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(fldcw)(__UINT16_TYPE__ __cw) { __asm__ __volatile__("fldcw %0" : : "m" (__cw)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(fxsave)(__BYTE_TYPE__ __data[512]) { __asm__ __volatile__("fxsave %0" : "=m" (*__data)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(fxrstor)(__BYTE_TYPE__ const __data[512]) { __asm__ __volatile__("fxrstor %0" : : "m" (*__data)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(fninit)(void) { __asm__ __volatile__("fninit"); }

/* Read/Write control registers. */
__FORCELOCAL __WUNUSED __REGISTER_TYPE__ __CPU_INTRIN_FUNC(rdcr0)(void) { register __REGISTER_TYPE__ __result; __asm__("mov %%cr0, %0" : "=r" (__result)); return __result; }
__FORCELOCAL __WUNUSED __REGISTER_TYPE__ __CPU_INTRIN_FUNC(rdcr2)(void) { register __REGISTER_TYPE__ __result; __asm__("mov %%cr2, %0" : "=r" (__result)); return __result; }
__FORCELOCAL __WUNUSED __REGISTER_TYPE__ __CPU_INTRIN_FUNC(rdcr3)(void) { register __REGISTER_TYPE__ __result; __asm__("mov %%cr3, %0" : "=r" (__result)); return __result; }
__FORCELOCAL __WUNUSED __REGISTER_TYPE__ __CPU_INTRIN_FUNC(rdcr4)(void) { register __REGISTER_TYPE__ __result; __asm__("mov %%cr4, %0" : "=r" (__result)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrcr0)(__REGISTER_TYPE__ __val) { __asm__ __volatile__("mov %0, %%cr0" : : "r" (__val)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrcr2)(__REGISTER_TYPE__ __val) { __asm__ __volatile__("mov %0, %%cr2" : : "r" (__val)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrcr3)(__REGISTER_TYPE__ __val) { __asm__ __volatile__("mov %0, %%cr3" : : "r" (__val)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrcr4)(__REGISTER_TYPE__ __val) { __asm__ __volatile__("mov %0, %%cr4" : : "r" (__val)); }

/* Read/Write segment registers. */
__FORCELOCAL __WUNUSED __UINT16_TYPE__ __CPU_INTRIN_FUNC(rdes)(void) { register __UINT16_TYPE__ __result; __asm__("movw %%es, %w0" : "=rm" (__result)); return __result; }
__FORCELOCAL __WUNUSED __UINT16_TYPE__ __CPU_INTRIN_FUNC(rdcs)(void) { register __UINT16_TYPE__ __result; __asm__("movw %%cs, %w0" : "=rm" (__result)); return __result; }
__FORCELOCAL __WUNUSED __UINT16_TYPE__ __CPU_INTRIN_FUNC(rdss)(void) { register __UINT16_TYPE__ __result; __asm__("movw %%ss, %w0" : "=rm" (__result)); return __result; }
__FORCELOCAL __WUNUSED __UINT16_TYPE__ __CPU_INTRIN_FUNC(rdds)(void) { register __UINT16_TYPE__ __result; __asm__("movw %%ds, %w0" : "=rm" (__result)); return __result; }
__FORCELOCAL __WUNUSED __UINT16_TYPE__ __CPU_INTRIN_FUNC(rdfs)(void) { register __UINT16_TYPE__ __result; __asm__("movw %%fs, %w0" : "=rm" (__result)); return __result; }
__FORCELOCAL __WUNUSED __UINT16_TYPE__ __CPU_INTRIN_FUNC(rdgs)(void) { register __UINT16_TYPE__ __result; __asm__("movw %%gs, %w0" : "=rm" (__result)); return __result; }
__FORCELOCAL __WUNUSED __UINT16_TYPE__ __CPU_INTRIN_FUNC(str)(void) { register __UINT16_TYPE__ __result; __asm__("str %w0" : "=rm" (__result)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(wres)(__UINT16_TYPE__ __val) { __asm__ __volatile__("movw %w0, %%es" : : "rm" (__val) : "memory"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrss)(__UINT16_TYPE__ __val) { __asm__ __volatile__("movw %w0, %%ss" : : "rm" (__val) : "memory"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrds)(__UINT16_TYPE__ __val) { __asm__ __volatile__("movw %w0, %%ds" : : "rm" (__val) : "memory"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrfs)(__UINT16_TYPE__ __val) { __asm__ __volatile__("movw %w0, %%fs" : : "rm" (__val) : "memory"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrgs)(__UINT16_TYPE__ __val) { __asm__ __volatile__("movw %w0, %%gs" : : "rm" (__val) : "memory"); }
__FORCELOCAL void __CPU_INTRIN_FUNC(ltr)(__UINT16_TYPE__ __val) { __asm__ __volatile__("ltr %w0" : : "rm" (__val) : "memory"); }


/* Segment read/write operators. */
__FORCELOCAL __UINT8_TYPE__ __CPU_INTRIN_FUNC(readfsb)(__UINTPTR_TYPE__ __offset) { register __UINT8_TYPE__ __result; __asm__("movb %%fs:%1, %0" : "=q" (__result) : "m" (*(__UINT8_TYPE__ *)__offset)); return __result; }
__FORCELOCAL __UINT8_TYPE__ __CPU_INTRIN_FUNC(readgsb)(__UINTPTR_TYPE__ __offset) { register __UINT8_TYPE__ __result; __asm__("movb %%gs:%1, %0" : "=q" (__result) : "m" (*(__UINT8_TYPE__ *)__offset)); return __result; }
__FORCELOCAL __UINT16_TYPE__ __CPU_INTRIN_FUNC(readfsw)(__UINTPTR_TYPE__ __offset) { register __UINT16_TYPE__ __result; __asm__("movw %%fs:%1, %0" : "=r" (__result) : "m" (*(__UINT16_TYPE__ *)__offset)); return __result; }
__FORCELOCAL __UINT16_TYPE__ __CPU_INTRIN_FUNC(readgsw)(__UINTPTR_TYPE__ __offset) { register __UINT16_TYPE__ __result; __asm__("movw %%gs:%1, %0" : "=r" (__result) : "m" (*(__UINT16_TYPE__ *)__offset)); return __result; }
__FORCELOCAL __UINT32_TYPE__ __CPU_INTRIN_FUNC(readfsl)(__UINTPTR_TYPE__ __offset) { register __UINT32_TYPE__ __result; __asm__("movl %%fs:%1, %0" : "=r" (__result) : "m" (*(__UINT32_TYPE__ *)__offset)); return __result; }
__FORCELOCAL __UINT32_TYPE__ __CPU_INTRIN_FUNC(readgsl)(__UINTPTR_TYPE__ __offset) { register __UINT32_TYPE__ __result; __asm__("movl %%gs:%1, %0" : "=r" (__result) : "m" (*(__UINT32_TYPE__ *)__offset)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(writefsb)(__UINTPTR_TYPE__ __offset, __UINT8_TYPE__ __val) { __asm__("movb %1, %%fs:%0" : : "m" (*(__UINT8_TYPE__ *)__offset), "q" (__val)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(writegsb)(__UINTPTR_TYPE__ __offset, __UINT8_TYPE__ __val) { __asm__("movb %1, %%gs:%0" : : "m" (*(__UINT8_TYPE__ *)__offset), "q" (__val)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(writefsw)(__UINTPTR_TYPE__ __offset, __UINT16_TYPE__ __val) { __asm__("movw %w1, %%fs:%0" : : "m" (*(__UINT16_TYPE__ *)__offset), "r" (__val)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(writegsw)(__UINTPTR_TYPE__ __offset, __UINT16_TYPE__ __val) { __asm__("movw %w1, %%gs:%0" : : "m" (*(__UINT16_TYPE__ *)__offset), "r" (__val)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(writefsl)(__UINTPTR_TYPE__ __offset, __UINT32_TYPE__ __val) { __asm__("movl %d1, %%fs:%0" : : "m" (*(__UINT32_TYPE__ *)__offset), "r" (__val)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(writegsl)(__UINTPTR_TYPE__ __offset, __UINT32_TYPE__ __val) { __asm__("movl %d1, %%gs:%0" : : "m" (*(__UINT32_TYPE__ *)__offset), "r" (__val)); }
#ifdef __x86_64__
__FORCELOCAL __UINT64_TYPE__ __CPU_INTRIN_FUNC(readfsq)(__UINTPTR_TYPE__ __offset) { register __UINT64_TYPE__ __result; __asm__("movq %%fs:%1, %0" : "=r" (__result) : "m" (*(__UINT64_TYPE__ *)__offset)); return __result; }
__FORCELOCAL __UINT64_TYPE__ __CPU_INTRIN_FUNC(readgsq)(__UINTPTR_TYPE__ __offset) { register __UINT64_TYPE__ __result; __asm__("movq %%gs:%1, %0" : "=r" (__result) : "m" (*(__UINT64_TYPE__ *)__offset)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(writefsq)(__UINTPTR_TYPE__ __offset, __UINT64_TYPE__ __val) { __asm__("movq %1, %%fs:%0" : : "m" (*(__UINT64_TYPE__ *)__offset), "r" (__val)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(writegsq)(__UINTPTR_TYPE__ __offset, __UINT64_TYPE__ __val) { __asm__("movq %1, %%gs:%0" : : "m" (*(__UINT64_TYPE__ *)__offset), "r" (__val)); }
__FORCELOCAL void *__CPU_INTRIN_FUNC(readfsptr)(__UINTPTR_TYPE__ __offset) { return (void *)__CPU_INTRIN_SYM(readfsq)(__offset); }
__FORCELOCAL void *__CPU_INTRIN_FUNC(readgsptr)(__UINTPTR_TYPE__ __offset) { return (void *)__CPU_INTRIN_SYM(readgsq)(__offset); }
__FORCELOCAL void __CPU_INTRIN_FUNC(writefsptr)(__UINTPTR_TYPE__ __offset, void *__val) { __CPU_INTRIN_SYM(writefsq)(__offset,(__UINT64_TYPE__)__val); }
__FORCELOCAL void __CPU_INTRIN_FUNC(writegsptr)(__UINTPTR_TYPE__ __offset, void *__val) { __CPU_INTRIN_SYM(writegsq)(__offset,(__UINT64_TYPE__)__val); }
#else /* __x86_64__ */
__FORCELOCAL void *__CPU_INTRIN_FUNC(readfsptr)(__UINTPTR_TYPE__ __offset) { return (void *)__readfsl(__offset); }
__FORCELOCAL void *__CPU_INTRIN_FUNC(readgsptr)(__UINTPTR_TYPE__ __offset) { return (void *)__readgsl(__offset); }
__FORCELOCAL void __CPU_INTRIN_FUNC(writefsptr)(__UINTPTR_TYPE__ __offset, void *__val) { __writefsl(__offset,(__UINT32_TYPE__)__val); }
__FORCELOCAL void __CPU_INTRIN_FUNC(writegsptr)(__UINTPTR_TYPE__ __offset, void *__val) { __writegsl(__offset,(__UINT32_TYPE__)__val); }
#endif /* !__x86_64__ */

/* FS/GS-base registers. */
#ifdef __x86_64__
#ifdef __KERNEL__
#include <i386-kos/segment.h>
__FORCELOCAL __WUNUSED __UINT32_TYPE__ __CPU_INTRIN_FUNC(rdfsbasel)(void) { register __UINT64_TYPE__ __result; __asm__("safe_rdfsbase %0" : "=r" (__result)); return (__UINT32_TYPE__)__result; }
__FORCELOCAL __WUNUSED __UINT32_TYPE__ __CPU_INTRIN_FUNC(rdgsbasel)(void) { register __UINT64_TYPE__ __result; __asm__("safe_rdgsbase %0" : "=r" (__result)); return (__UINT32_TYPE__)__result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrfsbasel)(__UINT32_TYPE__ __val) { __asm__("safe_wrfsbase %0" : : "r" ((__UINT64_TYPE__)__val)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrgsbasel)(__UINT32_TYPE__ __val) { __asm__("safe_wrgsbase %0" : : "r" ((__UINT64_TYPE__)__val)); }
__FORCELOCAL __WUNUSED __UINT64_TYPE__ __CPU_INTRIN_FUNC(rdfsbaseq)(void) { register __UINT64_TYPE__ __result; __asm__("safe_rdfsbase %0" : "=r" (__result)); return __result; }
__FORCELOCAL __WUNUSED __UINT64_TYPE__ __CPU_INTRIN_FUNC(rdgsbaseq)(void) { register __UINT64_TYPE__ __result; __asm__("safe_rdgsbase %0" : "=r" (__result)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrfsbaseq)(__UINT64_TYPE__ __val) { __asm__("safe_wrfsbase %0" : : "r" (__val)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrgsbaseq)(__UINT64_TYPE__ __val) { __asm__("safe_wrgsbase %0" : : "r" (__val)); }
#else
__FORCELOCAL __WUNUSED __UINT32_TYPE__ __CPU_INTRIN_FUNC(rdfsbasel)(void) { register __UINT32_TYPE__ __result; __asm__("rdfsbase %d0" : "=r" (__result)); return __result; }
__FORCELOCAL __WUNUSED __UINT32_TYPE__ __CPU_INTRIN_FUNC(rdgsbasel)(void) { register __UINT32_TYPE__ __result; __asm__("rdgsbase %d0" : "=r" (__result)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrfsbasel)(__UINT32_TYPE__ __val) { __asm__("wrfsbase %d0" : : "r" (__val)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrgsbasel)(__UINT32_TYPE__ __val) { __asm__("wrgsbase %d0" : : "r" (__val)); }
__FORCELOCAL __WUNUSED __UINT64_TYPE__ __CPU_INTRIN_FUNC(rdfsbaseq)(void) { register __UINT64_TYPE__ __result; __asm__("rdfsbase %0" : "=r" (__result)); return __result; }
__FORCELOCAL __WUNUSED __UINT64_TYPE__ __CPU_INTRIN_FUNC(rdgsbaseq)(void) { register __UINT64_TYPE__ __result; __asm__("rdgsbase %0" : "=r" (__result)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrfsbaseq)(__UINT64_TYPE__ __val) { __asm__("wrfsbase %0" : : "r" (__val)); }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrgsbaseq)(__UINT64_TYPE__ __val) { __asm__("wrgsbase %0" : : "r" (__val)); }
#endif
__FORCELOCAL void __CPU_INTRIN_FUNC(swapgs)(void) { __asm__("swapgs"); }
#endif

/* MachineSpecificRegisters (MSRs) */
#ifdef __x86_64__
__FORCELOCAL __WUNUSED __UINT64_TYPE__ __CPU_INTRIN_FUNC(rdmsr)(__UINT32_TYPE__ __id) { union __ATTR_PACKED { __UINT32_TYPE__ __lohi[2]; __UINT64_TYPE__ __result; } __res; __asm__ __volatile__("rdmsr" : "=a" (__res.__lohi[0]), "=d" (__res.__lohi[1]) : "c" (__id)); return __res.__result; }
__FORCELOCAL __WUNUSED __UINT64_TYPE__ __CPU_INTRIN_FUNC(rdpmc)(__UINT32_TYPE__ __id) { union __ATTR_PACKED { __UINT32_TYPE__ __lohi[2]; __UINT64_TYPE__ __result; } __res; __asm__ __volatile__("rdpmc" : "=a" (__res.__lohi[0]), "=d" (__res.__lohi[1]) : "c" (__id)); return __res.__result; }
__FORCELOCAL __WUNUSED __UINT64_TYPE__ __CPU_INTRIN_FUNC(rdtsc)(void) { union __ATTR_PACKED { __UINT32_TYPE__ __lohi[2]; __UINT64_TYPE__ __result; } __res; __asm__ __volatile__("rdtsc" : "=a" (__res.__lohi[0]), "=d" (__res.__lohi[1])); return __res.__result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrmsr)(__UINT32_TYPE__ __id, __UINT64_TYPE__ __val) { union __ATTR_PACKED { __UINT32_TYPE__ __lohi[2]; __UINT64_TYPE__ __val; } __arg; __arg.__val = __val; __asm__ __volatile__("wrmsr" : : "c" (__id), "a" (__arg.__lohi[0]), "d" (__arg.__lohi[1])); }
#else
__FORCELOCAL __WUNUSED __UINT64_TYPE__ __CPU_INTRIN_FUNC(rdmsr)(__UINT32_TYPE__ __id) { __UINT64_TYPE__ __result; __asm__ __volatile__("rdmsr" : "=A" (__result) : "c" (__id)); return __result; }
__FORCELOCAL __WUNUSED __UINT64_TYPE__ __CPU_INTRIN_FUNC(rdpmc)(__UINT32_TYPE__ __id) { __UINT64_TYPE__ __result; __asm__ __volatile__("rdpmc" : "=A" (__result) : "c" (__id)); return __result; }
__FORCELOCAL __WUNUSED __UINT64_TYPE__ __CPU_INTRIN_FUNC(rdtsc)(void) { __UINT64_TYPE__ __result; __asm__ __volatile__("rdtsc" : "=A" (__result)); return __result; }
__FORCELOCAL void __CPU_INTRIN_FUNC(wrmsr)(__UINT32_TYPE__ __id, __UINT64_TYPE__ __val) { __asm__ __volatile__("wrmsr" : : "c" (__id), "A" (__val)); }
#endif

#endif /* __CC__ */

__SYSDECL_END

#endif /* !_KOS_I386_KOS_SYS_INTRIN_H */
