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
#ifndef _KOS_I386_KOS_EXCEPT_H
#define _KOS_I386_KOS_EXCEPT_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>
#include <kos/i386-kos/context.h>
#include <bits/siginfo.h>

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
#define E_INVALID_SEGMENT 0xfec0 /* [ERRNO(EINVAL)] Attempted to start a thread or return from a signal handler with an invalid segment register. */


#if defined(__PIC__) || defined(__PIE__)
#define __X86_PIC_FLAGS                 EXCEPTION_HANDLER_FRELATIVE
#ifdef __ASSEMBLER__
#ifdef __x86_64__
#   define __X86_PIC_IMAGE_RELATIVE(x)  .reloc .,R_X86_64_RELATIVE,x; .qword 0
#else
#   define __X86_PIC_IMAGE_RELATIVE(x)  .reloc .,R_386_RELATIVE,x; .long 0
#endif
#else /* __ASSEMBLER__ */
#ifdef __x86_64__
#   define __X86_PIC_IMAGE_RELATIVE(x) ".reloc .,R_X86_64_RELATIVE," x "; .qword 0"
#else
#   define __X86_PIC_IMAGE_RELATIVE(x) ".reloc .,R_386_RELATIVE," x "; .long 0"
#endif
#endif /* !__ASSEMBLER__ */
#else
#define __X86_PIC_FLAGS                 0
#ifdef __ASSEMBLER__
#ifdef __x86_64__
#   define __X86_PIC_IMAGE_RELATIVE(x)  .qword x
#else
#   define __X86_PIC_IMAGE_RELATIVE(x)  .long x
#endif
#else
#ifdef __x86_64__
#   define __X86_PIC_IMAGE_RELATIVE(x) ".qword " x
#else
#   define __X86_PIC_IMAGE_RELATIVE(x) ".long " x
#endif
#endif
#endif

#define __X86_EXCEPT_SECTION_NAME    .except, "a"
#define __X86_EXCEPT_SECTION_NAME_S ".except, \"a\""

#ifdef __ASSEMBLER__
/* Allow the use of these macros from assembly. */
#ifdef __x86_64__
#define __X86_DEFINE_EXCEPT_HANDLER(begin,end,entry,flags,mask) \
        .pushsection __X86_EXCEPT_SECTION_NAME; \
            __X86_PIC_IMAGE_RELATIVE(begin); \
            __X86_PIC_IMAGE_RELATIVE(end); \
            __X86_PIC_IMAGE_RELATIVE(entry); \
            .long  __X86_PIC_FLAGS|(flags); \
            .long  mask; \
        .popsection;
#else /* __x86_64__ */
#define __X86_DEFINE_EXCEPT_HANDLER(begin,end,entry,flags,mask) \
        .pushsection __X86_EXCEPT_SECTION_NAME; \
            __X86_PIC_IMAGE_RELATIVE(begin); \
            __X86_PIC_IMAGE_RELATIVE(end); \
            __X86_PIC_IMAGE_RELATIVE(entry); \
            .word __X86_PIC_FLAGS|(flags); \
            .word mask; \
        .popsection;
#endif /* !__x86_64__ */
#define __DEFINE_FINALLY_HANDLER(begin,end,entry) \
        __X86_DEFINE_EXCEPT_HANDLER(begin,end,entry,EXCEPTION_HANDLER_FFINALLY,0)
#define __DEFINE_CATCH_HANDLER(begin,end,entry,mask) \
        __X86_DEFINE_EXCEPT_HANDLER(begin,end,entry,EXCEPTION_HANDLER_FHASMASK,mask)
#define __DEFINE_EXCEPT_HANDLER(begin,end,entry) \
        __X86_DEFINE_EXCEPT_HANDLER(begin,end,entry,EXCEPTION_HANDLER_FINTERRUPT,0)
#else
#ifdef __x86_64__
#define __X86_DEFINE_EXCEPT_HANDLER(begin,end,entry,flags,mask) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l3") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l4") "\n" \
                                  ".long %a0\n" \
                                  ".long %a1\n" \
                                  ".popsection\n" : : \
                                  "i" (__X86_PIC_FLAGS|(flags)), "i" (mask) : : \
                                  begin, end, entry);
#define __DEFINE_FINALLY_HANDLER(begin,end,entry) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l0") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l1") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n" \
                                  ".qword " __PP_STR(__X86_PIC_FLAGS) "|2\n" \
                                  ".popsection\n" : : : : \
                                  begin, end, entry);
#define __DEFINE_EXCEPT_HANDLER(begin,end,entry) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l0") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l1") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n" \
                                  ".qword " __PP_STR(__X86_PIC_FLAGS) "\n" \
                                  ".popsection\n" : : : : \
                                  begin, end, entry);
#define __DEFINE_CATCH_HANDLER(begin,end,entry,mask) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l1") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l3") "\n" \
                                  ".long " __PP_STR(__X86_PIC_FLAGS) "|1\n" \
                                  ".long %a0\n" \
                                  ".popsection\n" : : "i" (mask) : : \
                                  begin, end, entry);
#else
#define __X86_DEFINE_EXCEPT_HANDLER(begin,end,entry,flags,mask) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l3") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l4") "\n" \
                                  ".word %a0\n" \
                                  ".word %a1\n" \
                                  ".popsection\n" : : \
                                  "i" (__X86_PIC_FLAGS|(flags)), "i" (mask) : : \
                                  begin, end, entry);
#define __DEFINE_FINALLY_HANDLER(begin,end,entry) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l0") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l1") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n" \
                                  ".long " __PP_STR(__X86_PIC_FLAGS) "|2\n" \
                                  ".popsection\n" : : : : \
                                  begin, end, entry);
#define __DEFINE_EXCEPT_HANDLER(begin,end,entry) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l0") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l1") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n" \
                                  ".long " __PP_STR(__X86_PIC_FLAGS) "\n" \
                                  ".popsection\n" : : : : \
                                  begin, end, entry);
#define __DEFINE_CATCH_HANDLER(begin,end,entry,mask) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l1") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n" \
                                  __X86_PIC_IMAGE_RELATIVE("%l3") "\n" \
                                  ".word " __PP_STR(__X86_PIC_FLAGS) "|1\n" \
                                  ".word %a0\n" \
                                  ".popsection\n" : : "i" (mask) : : \
                                  begin, end, entry);
#endif
#endif

#if defined(__USE_KOS) || defined(__ASSEMBLER__)
/* Define public versions of the private define-handler macros. */
#define X86_DEFINE_EXCEPT_HANDLER(begin,end,entry,flags,mask) \
      __X86_DEFINE_EXCEPT_HANDLER(begin,end,entry,flags,mask)
#endif


#if 1
#if 0
#define __EXCEPT_CLOBBER_REGS() \
 __XBLOCK({ __asm__ __volatile__("#%=" : : : "memory", "esp"); \
            __asm__ __volatile__("#%=" : : : "memory", "esp"); \
           (void)0; \
 })
#elif 1
/* I wish there was a way to get GCC to understand that I only
 * want to clobber registers it is currently using for variables.
 * -> All I'd need is for you to safe variables to EBP-offset stack slots...
 *    But alas, KOS's entire exception system is nothing but a big clutch
 *   (If only C++ had a `finally' statement and supported `x = { .foo = 42 }'
 *    initializers. Then I probably would have just went and done everything
 *    using it, rather than C.
 *    AND DONT TELL ME ABOUT RAII; Because you can't use that one in interlocked
 *    syntax, and it also has the problem of requiring you to use variables in
 *    order to get finally-blocks.
 *    Plus: with all those hidden calls it puts everything, debugging would
 *          be the worst. So no: I'm a C guy that's having an easier time
 *          hacking his C compiler to the point of it having $hit like this,
 *          just because I find C++ to be too distant from the actual hardware) */
#define __EXCEPT_CLOBBER_REGS() \
 __XBLOCK({ __asm__ __volatile__("#%=" : : : "memory", "cc" \
                                 , "esp", "eax", "ecx", "edx" \
                                 , "ebx", "esi", "edi"); \
           (void)0; \
 })
#else
#define __EXCEPT_CLOBBER_REGS() \
 __XBLOCK({ __asm__ __volatile__("" : : : "memory", "esp"); (void)0; })
#endif
#elif 1
#define __EXCEPT_CLOBBER_REGS() (void)0
#elif 1
#define __EXCEPT_CLOBBER_REGS() \
 __XBLOCK({ extern volatile unsigned int __some_undefined_symbol; \
            __asm__ __volatile__(".rept 0\n" : : : "memory"); \
            __asm__ __volatile__("" : : "g" (__builtin_alloca(__some_undefined_symbol)) : "memory"); \
            __asm__ __volatile__(".endr\n" : : : "memory"); \
            (void)0; })
#else
#define __EXCEPT_CLOBBER_REGS() \
 __XBLOCK({ extern volatile unsigned int __some_undefined_symbol; \
            __asm__ __volatile__(".pushsection .discard\n" : : : "memory"); \
            __asm__ __volatile__("" : : "g" (__builtin_alloca(__some_undefined_symbol)) : "memory"); \
            __asm__ __volatile__(".popsection\n" : : : "memory"); \
            (void)0; })
#endif

/* Defining `error_rethrow()' as inline assembly prevents GCC from optimizing
 * a regular call away by diverting a nearby jump to the call instruction
 * to another call to the same function, which would break exception handling:
 * >> TRY {
 * >>     TRY {
 * >>        printf("foo\n");
 * >>        error_throw(42);
 * >>     } FINALLY {
 * >>        printf("bar\n");
 * >>     }
 * >> } FINALLY {
 * >>     if (some_condition()) // Introduce a conditional jump just before `error_rethrow()' that GCC could then redirect
 * >>         printf("baz\n");
 * >>     // GCC may otherwise choose to optimize the call to `error_rethrow()'
 * >>     // that exists at the end of this scope into a jump to the call above.
 * >>     // Since it knows that `error_rethrow()' doesn't return, it thinks that
 * >>     // its return address wouldn't matter. However, it actually does, because
 * >>     // that return address is used to resolve unwind order of recursive exception
 * >>     // handlers like in this example.
 * >>     // However, that optimization may then generate a jump to the finally above,
 * >>     // causing us to falsely unwind the stack to re-execute the second finally,
 * >>     // leaving us to end up with an infinite loop...
 * >> }
 * ... By wrapping the call in inline assembly however, GCC will be forced to generate
 *     a _true_ call to the `error_rethrow()' function, whereever it appears.
 * NOTE: The same problem applies to the other functions also wrapped here in the same way.
 */
#ifndef __INTELLISENSE__
#if defined(__BUILDING_LIBC)
#define error_rethrow()       __XBLOCK({ __asm__ __volatile__("call libc_error_rethrow#%=\n" : : : "memory"); __builtin_unreachable(); (void)0; })
#define error_throw(code)     __XBLOCK({ __asm__ __volatile__("call libc_error_throw#%=\n" : : "c" ((__UINT16_TYPE__)(code)) : "memory"); __builtin_unreachable(); (void)0; })
#define error_continue(retry) __XBLOCK({ __asm__ __volatile__("call libc_error_continue#%=\n" : : "c" ((int)(retry)) : "memory"); __builtin_unreachable(); (void)0; })
#define error_except(mode)    __XBLOCK({ __asm__ __volatile__("call libc_error_except#%=\n" : : "c" ((int)(mode)) : "memory"); (void)0; })
#define error_throw_current() __XBLOCK({ __BOOL __etc_res; __asm__ __volatile__("call libc_error_throw_current#%=\n" : "=a" (__etc_res) : : "memory"); __XRETURN __etc_res; })
#elif defined(__KERNEL__) && defined(CONFIG_BUILDING_KERNEL_CORE)
#define error_rethrow()       __XBLOCK({ __asm__ __volatile__("call error_rethrow#%=\n" : : : "memory"); __builtin_unreachable(); (void)0; })
#define error_throw(code)     __XBLOCK({ __asm__ __volatile__("call error_throw#%=\n" : : "c" ((__UINT16_TYPE__)(code)) : "memory"); __builtin_unreachable(); (void)0; })
#define error_continue(retry) __XBLOCK({ __asm__ __volatile__("call error_continue#%=\n" : : "c" ((int)(retry)) : "memory"); __builtin_unreachable(); (void)0; })
#define error_except(mode)    __XBLOCK({ __asm__ __volatile__("call error_except#%=\n" : : "c" ((int)(mode)) : "memory"); (void)0; })
#define error_throw_current() __XBLOCK({ __BOOL __etc_res; __asm__ __volatile__("call error_throw_current#%=\n" : "=a" (__etc_res) : : "memory"); __XRETURN __etc_res; })
#else
#define error_rethrow()       __XBLOCK({ __asm__ __volatile__("call error_rethrow@PLT#%=\n" : : : "memory"); __builtin_unreachable(); (void)0; })
#define error_throw(code)     __XBLOCK({ __asm__ __volatile__("call error_throw@PLT#%=\n" : : "c" ((__UINT16_TYPE__)(code)) : "memory"); __builtin_unreachable(); (void)0; })
#define error_continue(retry) __XBLOCK({ __asm__ __volatile__("call error_continue@PLT#%=\n" : : "c" ((int)(retry)) : "memory"); __builtin_unreachable(); (void)0; })
#define error_except(mode)    __XBLOCK({ __asm__ __volatile__("call error_except@PLT#%=\n" : : "c" ((int)(mode)) : "memory"); (void)0; })
#define error_throw_current() __XBLOCK({ __BOOL __etc_res; __asm__ __volatile__("call error_throw_current@PLT#%=\n" : "=a" (__etc_res) : : "memory"); __XRETURN __etc_res; })
#endif
#endif /* __INTELLISENSE__ */



/* System (Hardware - generated) exception codes. */
#define E_SEGFAULT               X86_E_SYSTEM_PF /* [ERRNO(EFAULT)] Segmentation fault (invalid memory access). */
#define E_BREAKPOINT             X86_E_SYSTEM_BP /* [ERRNO(EINVAL)] Breakpoint. */
#define E_DIVIDE_BY_ZERO         X86_E_SYSTEM_DE /* [ERRNO(EINVAL)] Divide by zero. */
#define E_OVERFLOW               X86_E_SYSTEM_OF /* [ERRNO(EOVERFLOW)] Overflow. */
#define E_ILLEGAL_INSTRUCTION    X86_E_SYSTEM_UD /* [ERRNO(EPERM)] Illegal instruction / operand / addressing mode or trap encountered. */
#define E_STACK_OVERFLOW         X86_E_SYSTEM_DF /* [ERRNO(EFAULT)] Stack overflow.
                                                  * Thrown in kernel space when: #PF->#DF with ESP/RSP below stack_base.
                                                  * Thrown in user space when:   #PF ontop of a guard page with no remaining funds. */
#define E_PRIVILEGED_INSTRUCTION X86_E_SYSTEM_GP /* [ERRNO(EPERM)] Privileged instruction / register encountered. */
#define E_INDEX_ERROR            X86_E_SYSTEM_BR /* [ERRNO(ERANGE)] The BOUND instruction was executed with an out-of-bounds index. */
#define E_INVALID_ALIGNMENT      X86_E_SYSTEM_AC /* [ERRNO(EFAULT)] The alignment of an operand was not valid. */



#define X86_SEGFAULT_FPRESENT    0x0001 /* Set if the page at `f_vaddr' was present (and the error was due to a protection violation) */
#define X86_SEGFAULT_FWRITE      0x0002 /* Set if the fault occurred when an attempt was made to write (Otherwise, it was caused by a read). */
#define X86_SEGFAULT_FUSER       0x0004 /* Set if the fault occurred in user-space. */
#define X86_SEGFAULT_FRESWRITE   0x0008 /* Set if the fault happened because a 1 was written in a reserved
                                         * page-directory bit (Should never be set unless there's a bug in KOS). */
#define X86_SEGFAULT_FEXEC       0x0010 /* Set if the fault happened when trying to fetch instructions. */
#define ERROR_SEGFAULT_ISMAPPED(x) ((x)&X86_SEGFAULT_FPRESENT)

#ifdef __CC__
struct __ATTR_PACKED exception_data_segfault {
    __UINTPTR_TYPE__     sf_reason;    /* Set of `X86_SEGFAULT_F*'. */
    void                *sf_vaddr;     /* Virtual address that was attempted to be accessed. */
};
#define __exception_data_system_defined 1
struct __ATTR_PACKED exception_data_system {
    __UINTPTR_TYPE__     s_errcode;    /* An interrupt-specific error code sometimes defined by system
                                        * exceptions. When not defined, this field is set to ZERO(0). */
};
#endif /* __CC__ */

#define INVALID_SEGMENT_REGISTER_DS 0
#define INVALID_SEGMENT_REGISTER_ES 1
#define INVALID_SEGMENT_REGISTER_FS 2
#define INVALID_SEGMENT_REGISTER_GS 3
#define INVALID_SEGMENT_REGISTER_SS 4
#define INVALID_SEGMENT_REGISTER_CS 5
#ifdef __CC__
struct __ATTR_PACKED exception_data_invalid_segment {
    __UINT16_TYPE__      is_register;  /* The segment register attempted to load with the invalid segment (One of `INVALID_SEGMENT_REGISTER_*') */
    __UINT16_TYPE__      is_segment;   /* The invalid segment index, including privilege bits (RPL) and the LDT bit. */
};
#endif /* __CC__ */


/* Platform-independent reason-value combinations for generating SEGFAULT reasons. */
#define SEGFAULT_BADUSERREAD    (X86_SEGFAULT_FUSER)
#define SEGFAULT_BADUSERWRITE   (X86_SEGFAULT_FUSER|X86_SEGFAULT_FWRITE)
#define SEGFAULT_BADUSEREXEC    (X86_SEGFAULT_FUSER|X86_SEGFAULT_FEXEC)
#ifdef __KERNEL__
#define SEGFAULT_BADREAD         0
#define SEGFAULT_BADWRITE       (X86_SEGFAULT_FWRITE)
#define SEGFAULT_BADEXEC        (X86_SEGFAULT_FEXEC)
#else
#define SEGFAULT_BADREAD        (X86_SEGFAULT_FUSER)
#define SEGFAULT_BADWRITE       (X86_SEGFAULT_FUSER|X86_SEGFAULT_FWRITE)
#define SEGFAULT_BADEXEC        (X86_SEGFAULT_FUSER|X86_SEGFAULT_FEXEC)
#endif



__SYSDECL_END

#if defined(__COMPILER_HAVE_GCC_ASM) && !defined(__NO_XBLOCK) && !defined(__INTELLISENSE__)
#include "tls.h"
#undef error_code
#undef error_info
#ifdef __x86_64__
#define error_code() \
 __XBLOCK({ __UINT16_TYPE__ __ecode; \
            __asm__("movw %%" __PP_STR(__ASM_TASK_SEGMENT) ":8, %w0" : "=g" (__ecode)); \
            __XRETURN __ecode; })
#define error_info() \
 __XBLOCK({ struct exception_info *__einfo; \
            __asm__("addq %%" __PP_STR(__ASM_TASK_SEGMENT) ":0, %0" : "=g" (__einfo) : "0" (8)); \
            __XRETURN __einfo; })
#else
#define error_code() \
 __XBLOCK({ __UINT16_TYPE__ __ecode; \
            __asm__("movw %%" __PP_STR(__ASM_TASK_SEGMENT) ":4, %w0" : "=g" (__ecode)); \
            __XRETURN __ecode; })
#define error_info() \
 __XBLOCK({ struct exception_info *__einfo; \
            __asm__("addl %%" __PP_STR(__ASM_TASK_SEGMENT) ":0, %0" : "=g" (__einfo) : "0" (4)); \
            __XRETURN __einfo; })
#endif
#endif


#endif /* !_KOS_I386_KOS_EXCEPT_H */
