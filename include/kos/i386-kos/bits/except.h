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
#ifndef _KOS_I386_KOS_BITS_EXCEPT_H
#define _KOS_I386_KOS_BITS_EXCEPT_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>
#include "compat.h"

__SYSDECL_BEGIN

#define X86_SEGFAULT_FPRESENT    0x0001 /* Set if the page at `f_vaddr' was present (and the error was due to a protection violation) */
#define X86_SEGFAULT_FWRITE      0x0002 /* Set if the fault occurred when an attempt was made to write (Otherwise, it was caused by a read). */
#define X86_SEGFAULT_FUSER       0x0004 /* Set if the fault occurred in user-space. */
#define X86_SEGFAULT_FRESWRITE   0x0008 /* Set if the fault happened because a 1 was written in a reserved
                                         * page-directory bit (Should never be set unless there's a bug in KOS). */
#define X86_SEGFAULT_FEXEC       0x0010 /* Set if the fault happened when trying to fetch instructions. */
#define ERROR_SEGFAULT_ISMAPPED(x) ((x)&X86_SEGFAULT_FPRESENT)
#ifdef __CC__
#define __exception_data_segfault_defined 1
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
#define __exception_data_illegal_instruction_defined 1
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
#define __exception_data_invalid_segment_defined 1
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
#define __exception_rt_data_defined 1
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

__SYSDECL_END

#ifdef __EXPOSE_CPU_COMPAT
#include "except-compat.h"
#endif

#if defined(__CC__) && \
    defined(__COMPILER_HAVE_GCC_ASM) && \
   !defined(__NO_XBLOCK) && \
   !defined(__INTELLISENSE__)
#include <kos/i386-kos/asm/tls.h>
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
#else /* __x86_64__ */
#define error_code() \
 __XBLOCK({ __UINT16_TYPE__ __ecode; \
            __asm__("movw %%" __PP_STR(__ASM_TASK_SEGMENT) ":4, %w0" : "=r" (__ecode)); \
            __XRETURN __ecode; })
#define error_info() \
 __XBLOCK({ struct exception_info *__einfo; \
            __asm__("addl %%" __PP_STR(__ASM_TASK_SEGMENT) ":0, %0" : "=r" (__einfo) : "0" (4)); \
            __XRETURN __einfo; })
#endif /* !__x86_64__ */
#endif /* ... */

#endif /* !_KOS_I386_KOS_ASM_EXCEPT_H */
