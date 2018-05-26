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
#ifndef _KOS_I386_KOS_ASM_EXCEPT_H
#define _KOS_I386_KOS_ASM_EXCEPT_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

/* System exception codes.
 * NOTE: Many of these are never actually thrown, and those that are also
 *       exist as `E_*' codes below. Also note that some exceptions may
 *       be merged with others, while allowing for differentiation using
 *      `error_info()'. */
#define X86_E_SYSTEM_DE   0xff00 /* Divide-by-zero. */
#define X86_E_SYSTEM_DB   0xff01 /* Debug. */
#define X86_E_SYSTEM_NMI  0xff02 /* Non-maskable Interrupt. */
#define X86_E_SYSTEM_BP   0xff03 /* Breakpoint. */
#define X86_E_SYSTEM_OF   0xff04 /* Overflow. */
#define X86_E_SYSTEM_BR   0xff05 /* Bound Range Exceeded. */
#define X86_E_SYSTEM_UD   0xff06 /* Invalid Opcode. */
#define X86_E_SYSTEM_NM   0xff07 /* Device Not Available. */
#define X86_E_SYSTEM_DF   0xff08 /* Double Fault. */
/*      X86_E_SYSTEM_     0xff09  */
#define X86_E_SYSTEM_TS   0xff0a /* Invalid TSS. */
#define X86_E_SYSTEM_NP   0xff0b /* Segment Not Present. */
#define X86_E_SYSTEM_SS   0xff0c /* Stack-Segment Fault. */
#define X86_E_SYSTEM_GP   0xff0d /* General Protection Fault. */
#define X86_E_SYSTEM_PF   0xff0e /* Page Fault. */
/*      X86_E_SYSTEM_     0xff0f  */
#define X86_E_SYSTEM_MF   0xff10 /* x87 Floating-Point Exception. */
#define X86_E_SYSTEM_AC   0xff11 /* Alignment Check. */
#define X86_E_SYSTEM_MC   0xff12 /* Machine Check. */
#define X86_E_SYSTEM_XM   0xff13 /* SIMD Floating-Point Exception. */
#define X86_E_SYSTEM_VE   0xff14 /* Virtualization Exception. */
/*      X86_E_SYSTEM_     0xff15  */
/*                        ....    */
/*      X86_E_SYSTEM_     0xff1d  */
#define X86_E_SYSTEM_SX   0xff1e /* Security Exception. */
/*      X86_E_SYSTEM_     0xff1f  */
#define X86_E_SYSTEM_XF   X86_E_SYSTEM_XM /* Alias */


/* CPU-specific exceptions. */
#define E_INVALID_SEGMENT 0xfec0 /* [ERRNO(EINVAL)] Attempted to start a thread or return
                                  * from a signal handler with an invalid segment register.
                                  * Also thrown if a NULL segment has been assigned to one
                                  * of the segment registers before that register was then
                                  * attempted to be used for something. */


/* System (Hardware-generated) exception codes. */
#define E_SEGFAULT               X86_E_SYSTEM_PF /* [ERRNO(EFAULT)] Segmentation fault (invalid memory access). */
#define E_BREAKPOINT             X86_E_SYSTEM_BP /* [ERRNO(EINVAL)] Breakpoint. */
#define E_DIVIDE_BY_ZERO         X86_E_SYSTEM_DE /* [ERRNO(EINVAL)] Divide by zero. */
#define E_OVERFLOW               X86_E_SYSTEM_OF /* [ERRNO(EOVERFLOW)] Overflow. */
#define E_ILLEGAL_INSTRUCTION    X86_E_SYSTEM_UD /* [ERRNO(EPERM)] Illegal/privileged/restricted  instruction/register/operand/addressing mode or trap encountered. */
#define E_STACK_OVERFLOW         X86_E_SYSTEM_DF /* [ERRNO(EFAULT)] Stack overflow.
                                                  * Thrown in kernel space when: #PF->#DF with ESP/RSP below stack_base.
                                                  * Thrown in user space when:   #PF ontop of a guard page with no remaining funds. */
#define E_INDEX_ERROR            X86_E_SYSTEM_BR /* [ERRNO(ERANGE)] The BOUND instruction was executed with an out-of-bounds index. */
#define E_INVALID_ALIGNMENT      X86_E_SYSTEM_AC /* [ERRNO(EFAULT)] The alignment of an operand was not valid. */




#if defined(__PIC__) || defined(__PIE__)
#define __X86_EXCEPT_PIC_HFLAGS                 EXCEPTION_HANDLER_FRELATIVE
#define __X86_EXCEPT_PIC_DFLAGS                 EXCEPTION_DESCRIPTOR_FRELATIVE
#ifdef __ASSEMBLER__
#ifdef __x86_64__
#   define __X86_EXCEPT_PIC_IMAGE_RELATIVE(x)  .reloc .,R_X86_64_RELATIVE,x; .quad 0
#else
#   define __X86_EXCEPT_PIC_IMAGE_RELATIVE(x)  .reloc .,R_386_RELATIVE,x; .long 0
#endif
#else /* __ASSEMBLER__ */
#ifdef __x86_64__
#   define __X86_EXCEPT_PIC_IMAGE_RELATIVE(x) ".reloc .,R_X86_64_RELATIVE," x "; .quad 0"
#else
#   define __X86_EXCEPT_PIC_IMAGE_RELATIVE(x) ".reloc .,R_386_RELATIVE," x "; .long 0"
#endif
#endif /* !__ASSEMBLER__ */
#else
#define __X86_EXCEPT_PIC_HFLAGS                 0
#define __X86_EXCEPT_PIC_DFLAGS                 0
#ifdef __ASSEMBLER__
#ifdef __x86_64__
#   define __X86_EXCEPT_PIC_IMAGE_RELATIVE(x)  .quad x
#else
#   define __X86_EXCEPT_PIC_IMAGE_RELATIVE(x)  .long x
#endif
#else
#ifdef __x86_64__
#   define __X86_EXCEPT_PIC_IMAGE_RELATIVE(x) "\t.quad " x
#else
#   define __X86_EXCEPT_PIC_IMAGE_RELATIVE(x) "\t.long " x
#endif
#endif
#endif

#define __X86_EXCEPT_SECTION_NAME    .except, "a"
#define __X86_EXCEPT_SECTION_NAME_S ".except, \"a\""

#ifdef __ASSEMBLER__
/* Allow the use of these macros from assembly. */
#ifdef __x86_64__
#define __DEFINE_EXCEPTION_HANDLER(begin,end,entry,flags,mask) \
        .pushsection __X86_EXCEPT_SECTION_NAME; \
            __X86_EXCEPT_PIC_IMAGE_RELATIVE(begin); \
            __X86_EXCEPT_PIC_IMAGE_RELATIVE(end); \
            __X86_EXCEPT_PIC_IMAGE_RELATIVE(entry); \
            .int __X86_EXCEPT_PIC_HFLAGS|(flags); \
            .int mask; \
        .popsection;
#else /* __x86_64__ */
#define __DEFINE_EXCEPTION_HANDLER(begin,end,entry,flags,mask) \
        .pushsection __X86_EXCEPT_SECTION_NAME; \
            __X86_EXCEPT_PIC_IMAGE_RELATIVE(begin); \
            __X86_EXCEPT_PIC_IMAGE_RELATIVE(end); \
            __X86_EXCEPT_PIC_IMAGE_RELATIVE(entry); \
            .word __X86_EXCEPT_PIC_HFLAGS|(flags); \
            .word mask; \
        .popsection;
#endif /* !__x86_64__ */
#define __DEFINE_EXCEPTION_DESCRIPTOR(name,handler,type,flags,safe) \
        .set name, .; \
        __X86_EXCEPT_PIC_IMAGE_RELATIVE(handler); \
        .word (type); \
        .word __X86_EXCEPT_PIC_DFLAGS | (flags); \
        .word (safe); \
        .word 0; \
        .size name, . - name;
#else
#ifdef __x86_64__
#define __DEFINE_EXCEPTION_HANDLER(begin,end,entry,flags,mask) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l3") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l4") "\n\t" \
                                  "\t.int %a0\n\t" \
                                  "\t.int %a1\n\t" \
                                  ".popsection" : : \
                                  "i" (__X86_EXCEPT_PIC_HFLAGS|(flags)), "i" (mask) : : \
                                  begin, end, entry);
#define __DEFINE_FINALLY_BLOCK(begin,end,entry) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l0") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l1") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  "\t.quad " __PP_STR(__X86_EXCEPT_PIC_HFLAGS) "|2\n\t" \
                                  ".popsection" : : : : \
                                  begin, end, entry);
#define __DEFINE_EXCEPT_BLOCK(begin,end,entry) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l0") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l1") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  "\t.quad " __PP_STR(__X86_EXCEPT_PIC_HFLAGS) "\n\t" \
                                  ".popsection" : : : : \
                                  begin, end, entry);
#define __DEFINE_CATCH_BLOCK(begin,end,entry,mask) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l1") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l3") "\n\t" \
                                  "\t.long " __PP_STR(__X86_EXCEPT_PIC_HFLAGS) "|1\n\t" \
                                  "\t.long %a0\n\t" \
                                  ".popsection" : : "i" (mask) : : \
                                  begin, end, entry);
#else
#define __DEFINE_EXCEPTION_HANDLER(begin,end,entry,flags,mask) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l3") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l4") "\n\t" \
                                  "\t.word %a0\n\t" \
                                  "\t.word %a1\n\t" \
                                  ".popsection" : : \
                                  "i" (__X86_EXCEPT_PIC_HFLAGS|(flags)), "i" (mask) : : \
                                  begin, end, entry);
#define __DEFINE_FINALLY_BLOCK(begin,end,entry) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l0") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l1") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  "\t.long " __PP_STR(__X86_EXCEPT_PIC_HFLAGS) "|2\n\t" \
                                  ".popsection" : : : : \
                                  begin, end, entry);
#define __DEFINE_EXCEPT_BLOCK(begin,end,entry) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l0") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l1") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  "\t.long " __PP_STR(__X86_EXCEPT_PIC_HFLAGS) "\n\t" \
                                  ".popsection" : : : : \
                                  begin, end, entry);
#define __DEFINE_CATCH_BLOCK(begin,end,entry,mask) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l1") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  __X86_EXCEPT_PIC_IMAGE_RELATIVE("%l3") "\n\t" \
                                  "\t.word " __PP_STR(__X86_EXCEPT_PIC_HFLAGS) "|1\n\t" \
                                  "\t.word %a0\n\t" \
                                  ".popsection" : : "i" (mask) : : \
                                  begin, end, entry);
#endif
#define __DEFINE_EXCEPTION_DESCRIPTOR(name,handler,type,flags,safe) \
        __asm__(".set " #name ", .\n\t" \
                 __X86_EXCEPT_PIC_IMAGE_RELATIVE(handler) "\n\t" \
                ".word " __PP_PRIVATE_STR(type) "\n\t" \
                ".word " __PP_STR(__X86_EXCEPT_PIC_DFLAGS | (flags)) "\n\t" \
                ".word " __PP_PRIVATE_STR(safe) "\n\t" \
                ".word 0\n\t" \
                ".size name, . - name\n\t")
#endif




#ifdef __CC__
/* Just to be safe, surrounding these calls with memory barriers
 * and indicate to GCC that they will clobber (restore) ESP. */
#define __EXCEPT_INVOKE_HANDLED(x) \
   __XBLOCK({ __asm__ __volatile__("" : : : "memory", "esp"); \
              x; \
              __asm__ __volatile__("" : : : "memory"); \
   })
#define __EXCEPT_INVOKE_DEALLOC_CONTINUE(x) \
   __XBLOCK({ __SIZE_TYPE__ __edc_result; \
              __asm__ __volatile__("" : : : "memory", "esp"); \
              __edc_result = x; \
              __asm__ __volatile__("" : : : "memory"); \
              __XRETURN __edc_result; \
   })

#if 0
/* Sadly, GCC's -Wclobber warnings are totally broken, this is a no-go */
__PRIVATE __ATTR_UNUSED __ATTR_NOINLINE __ATTR_RETURNS_TWICE
void (__except_returns_twice)(void) { __asm__ __volatile__(""); }
#if 1
#define __EXCEPT_CLOBBER_REGS() \
 __XBLOCK({ __asm__ __volatile__(".pushsection .discard #%=" : : : "memory"); \
            __except_returns_twice(); \
            __asm__ __volatile__(".popsection #%=" : : : "memory", "esp"); \
           (void)0; })
#else
#define __EXCEPT_CLOBBER_REGS() \
 __XBLOCK({ __except_returns_twice(); \
            __asm__ __volatile__("#%=" : : : "memory", "esp"); \
           (void)0; })
#endif
#else
/* Mark ESP as clobber, because as far as GCC is concerned,
 * exception continue data is allocated the same way alloca
 * allocates memory (aka. on the stack) */
#define __EXCEPT_CLOBBER_REGS() \
        __XBLOCK({ __asm__ __volatile__("#%=" : : : "memory", "esp"); (void)0; })
#endif
#endif /* __CC__ */

__SYSDECL_END

#endif /* !_KOS_I386_KOS_ASM_EXCEPT_H */
