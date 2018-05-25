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
#include <bits/siginfo.h>
#include "context.h"

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


#if defined(__PIC__) || defined(__PIE__)
#define __X86_PIC_FLAGS                 EXCEPTION_HANDLER_FRELATIVE
#ifdef __ASSEMBLER__
#ifdef __x86_64__
#   define __X86_PIC_IMAGE_RELATIVE(x)  .reloc .,R_X86_64_RELATIVE,x; .quad 0
#else
#   define __X86_PIC_IMAGE_RELATIVE(x)  .reloc .,R_386_RELATIVE,x; .long 0
#endif
#else /* __ASSEMBLER__ */
#ifdef __x86_64__
#   define __X86_PIC_IMAGE_RELATIVE(x) ".reloc .,R_X86_64_RELATIVE," x "; .quad 0"
#else
#   define __X86_PIC_IMAGE_RELATIVE(x) ".reloc .,R_386_RELATIVE," x "; .long 0"
#endif
#endif /* !__ASSEMBLER__ */
#else
#define __X86_PIC_FLAGS                 0
#ifdef __ASSEMBLER__
#ifdef __x86_64__
#   define __X86_PIC_IMAGE_RELATIVE(x)  .quad x
#else
#   define __X86_PIC_IMAGE_RELATIVE(x)  .long x
#endif
#else
#ifdef __x86_64__
#   define __X86_PIC_IMAGE_RELATIVE(x) "\t.quad " x
#else
#   define __X86_PIC_IMAGE_RELATIVE(x) "\t.long " x
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
            .int __X86_PIC_FLAGS|(flags); \
            .int mask; \
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
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l3") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l4") "\n\t" \
                                  "\t.int %a0\n\t" \
                                  "\t.int %a1\n\t" \
                                  ".popsection" : : \
                                  "i" (__X86_PIC_FLAGS|(flags)), "i" (mask) : : \
                                  begin, end, entry);
#define __DEFINE_FINALLY_HANDLER(begin,end,entry) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l0") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l1") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  "\t.quad " __PP_STR(__X86_PIC_FLAGS) "|2\n\t" \
                                  ".popsection" : : : : \
                                  begin, end, entry);
#define __DEFINE_EXCEPT_HANDLER(begin,end,entry) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l0") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l1") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  "\t.quad " __PP_STR(__X86_PIC_FLAGS) "\n\t" \
                                  ".popsection" : : : : \
                                  begin, end, entry);
#define __DEFINE_CATCH_HANDLER(begin,end,entry,mask) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l1") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l3") "\n\t" \
                                  "\t.long " __PP_STR(__X86_PIC_FLAGS) "|1\n\t" \
                                  "\t.long %a0\n\t" \
                                  ".popsection" : : "i" (mask) : : \
                                  begin, end, entry);
#else
#define __X86_DEFINE_EXCEPT_HANDLER(begin,end,entry,flags,mask) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l3") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l4") "\n\t" \
                                  "\t.word %a0\n\t" \
                                  "\t.word %a1\n\t" \
                                  ".popsection" : : \
                                  "i" (__X86_PIC_FLAGS|(flags)), "i" (mask) : : \
                                  begin, end, entry);
#define __DEFINE_FINALLY_HANDLER(begin,end,entry) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l0") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l1") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  "\t.long " __PP_STR(__X86_PIC_FLAGS) "|2\n\t" \
                                  ".popsection" : : : : \
                                  begin, end, entry);
#define __DEFINE_EXCEPT_HANDLER(begin,end,entry) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l0") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l1") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  "\t.long " __PP_STR(__X86_PIC_FLAGS) "\n\t" \
                                  ".popsection" : : : : \
                                  begin, end, entry);
#define __DEFINE_CATCH_HANDLER(begin,end,entry,mask) \
        __asm__ __volatile__ goto(".pushsection " __X86_EXCEPT_SECTION_NAME_S "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l1") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l2") "\n\t" \
                                  __X86_PIC_IMAGE_RELATIVE("%l3") "\n\t" \
                                  "\t.word " __PP_STR(__X86_PIC_FLAGS) "|1\n\t" \
                                  "\t.word %a0\n\t" \
                                  ".popsection" : : "i" (mask) : : \
                                  begin, end, entry);
#endif
#endif

#if defined(__USE_KOS) || defined(__ASSEMBLER__)
/* Define public versions of the private define-handler macros. */
#define X86_DEFINE_EXCEPT_HANDLER(begin,end,entry,flags,mask) \
      __X86_DEFINE_EXCEPT_HANDLER(begin,end,entry,flags,mask)
#endif


#ifdef __CC__
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



/* System (Hardware - generated) exception codes. */
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

/* Illegal instruction */
#define ERROR_ILLEGAL_INSTRUCTION_UNDEFINED    0x0000 /* The instruction/operand/flag is undefined. */
#define ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED   0x0001 /* The instruction/operand/flag is privileged and not available to the caller. */
#define ERROR_ILLEGAL_INSTRUCTION_RESTRICTED   0x0002 /* Execution of the instruction has been restricted for the caller. */
#define ERROR_ILLEGAL_INSTRUCTION_FINSTRUCTION 0x0000 /* The error was caused by the instruction itself being unknown/privileged or restricted. */
#define ERROR_ILLEGAL_INSTRUCTION_FADDRESS     0x0010 /* The error was caused by the addressing mode used/specified.
                                                       * (On X86 this flag is set when e.g. using a register operand in an instruction that requires a memory operand). */
#define ERROR_ILLEGAL_INSTRUCTION_FTRAP        0x0020 /* (Unused in X86) The error was caused by a trap. */
#define ERROR_ILLEGAL_INSTRUCTION_FOPERAND     0x0100 /* The error was caused by one of the instruction's operands. */
#define ERROR_ILLEGAL_INSTRUCTION_FREGISTER    0x0200 /* The error was thrown when attempting to modify a privileged/missing or restricted register. */
#define ERROR_ILLEGAL_INSTRUCTION_FVALUE       0x0800 /* The error was caused due to the value, or a flag contained in an operand. (Set alongside `ERROR_ILLEGAL_INSTRUCTION_FOPERAND')
                                                       * NOTE: When used alongside `ERROR_ILLEGAL_INSTRUCTION_FREGISTER', this flag indicates a write operation. */
#ifdef __CC__
struct __ATTR_PACKED exception_data_illegal_instruction {
    __UINTPTR_TYPE__     ii_errcode;         /* Interrupt-specific error code. */
    __UINTPTR_TYPE__     ii_type;            /* The type of illegal operation (One of `ERROR_ILLEGAL_INSTRUCTION_*',
                                              * optionally or'd with a set of `ERROR_ILLEGAL_INSTRUCTION_F*'). */
    __UINTPTR_TYPE__     ii_register_type;   /* [valid_if(ERROR_ILLEGAL_INSTRUCTION_FREGISTER)]
                                              *  The register to which an access was attempted (On X86, one of `X86_REGISTER_*').
                                              *  NOTE: Register indices can be found in <kos/registers.h> */
    __UINTPTR_TYPE__     ii_register_number; /* [valid_if(ERROR_ILLEGAL_INSTRUCTION_FREGISTER)]
                                              * The number of the register in question (On X86, one of `X86_REGISTER_[ii_register]*'). */
    __UINT64_TYPE__      ii_value;           /* [valid_if(ERROR_ILLEGAL_INSTRUCTION_FVALUE)]
                                              * The value of the illegal operand / value attempted
                                              * to write to illegal/privileged/restricted register. */
};
#endif /* __CC__ */

#ifdef __CC__
struct __ATTR_PACKED exception_data_invalid_segment {
    /* NOTE: This structure has binary compatibility with
     *      `exception_data_illegal_instruction', meaning
     *       that `E_INVALID_SEGMENT' errors can be handled
     *       the same way as `E_ILLEGAL_INSTRUCTION' errors. */
    __UINTPTR_TYPE__   __is_errcode;       /* Interrupt-specific error code. */
    __UINTPTR_TYPE__   __is_type;          /* [== ERROR_ILLEGAL_INSTRUCTION_UNDEFINED|
                                            *     ERROR_ILLEGAL_INSTRUCTION_FVALUE|
                                            *     ERROR_ILLEGAL_INSTRUCTION_FREGISTER] */
    __UINTPTR_TYPE__   __is_register_type; /* [== X86_REGISTER_SEGMENT] */
    __UINTPTR_TYPE__     is_register;      /* The segment register attempted to load with the invalid segment (One of `X86_REGISTER_SEGMENT_*') */
    union __ATTR_PACKED {
        __UINT64_TYPE__  is_segment;       /* The invalid segment index, including privilege bits (RPL) and the LDT bit. */
        __UINT16_TYPE__  is_segment16;     /* The invalid segment index, including privilege bits (RPL) and the LDT bit. */
    };
};
#endif /* __CC__ */



#define __EXCEPTION_RT_DATA_OFFSETOF_FREE_SP 0
#define __EXCEPTION_RT_DATA_SIZE             __SIZEOF_POINTER__
#ifdef __CC__
struct __ATTR_PACKED exception_rt_data {
    __UINTPTR_TYPE__     xrt_free_sp; /* The stack pointer up to which stack data
                                       * can be deallocated by an exception handler
                                       * that hasn't allocated additional stack memory.
                                       * This field is used to implement `error_dealloc_continue()',
                                       * which checks that the caller's stack-pointer matches the
                                       * SP-pointer described by the exception context, and overrides
                                       * the SP pointer with this value if they do match.
                                       * This field in return is set to the SP value calculated following
                                       * the last unwind operation completed prior to invocation of the
                                       * calling exception handler. */
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


__SYSDECL_END

#if defined(__COMPILER_HAVE_GCC_ASM) && !defined(__NO_XBLOCK) && !defined(__INTELLISENSE__)
#include "tls.h"
#undef error_code
#undef error_info
#ifdef __x86_64__
#define error_code() \
 __XBLOCK({ __UINT16_TYPE__ __ecode; \
            __asm__("movw %%" __PP_STR(__ASM_TASK_SEGMENT) ":8, %w0" : "=r" (__ecode)); \
            __XRETURN __ecode; })
#define error_info() \
 __XBLOCK({ struct exception_info *__einfo; \
            __asm__("addq %%" __PP_STR(__ASM_TASK_SEGMENT) ":0, %0" : "=r" (__einfo) : "0" (8)); \
            __XRETURN __einfo; })
#else
#define error_code() \
 __XBLOCK({ __UINT16_TYPE__ __ecode; \
            __asm__("movw %%" __PP_STR(__ASM_TASK_SEGMENT) ":4, %w0" : "=r" (__ecode)); \
            __XRETURN __ecode; })
#define error_info() \
 __XBLOCK({ struct exception_info *__einfo; \
            __asm__("addl %%" __PP_STR(__ASM_TASK_SEGMENT) ":0, %0" : "=r" (__einfo) : "0" (4)); \
            __XRETURN __einfo; })
#endif
#endif


#endif /* !_KOS_I386_KOS_EXCEPT_H */
