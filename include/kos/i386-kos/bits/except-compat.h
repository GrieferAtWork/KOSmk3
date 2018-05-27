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
#ifndef _KOS_I386_KOS_BITS_EXCEPT_COMPAT_H
#define _KOS_I386_KOS_BITS_EXCEPT_COMPAT_H 1

#include <__stdinc.h>
#include <bits/types.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>
#include <kos/i386-kos/bits/compat.h>
#include <kos/i386-kos/bits/cpu-context.h>
#include <kos/__exception_data.h>

__SYSDECL_BEGIN

#ifdef __EXPOSE_CPU_COMPAT

#ifdef __x86_64__
#define __cpu_context_compat_defined 1
#define __CPU_CONTEXT_COMPAT_SIZE X86_CONTEXT32_SIZE
#define   cpu_context_compat x86_context32
#define __cpu_usercontext_compat_defined 1
#define __CPU_USERCONTEXT_COMPAT_SIZE X86_USERCONTEXT32_SIZE
#define   cpu_usercontext_compat x86_usercontext32
#define __exception_data_segfault32_defined 1
#define __exception_data_segfault64_defined 1
#define   exception_data_segfault32 exception_data_segfault_compat
#define   exception_data_segfault64 exception_data_segfault
#define __exception_data_system32_defined 1
#define __exception_data_system64_defined 1
#define   exception_data_system32 exception_data_system_compat
#define   exception_data_system64 exception_data_system
#define __exception_data_illegal_instruction32_defined 1
#define __exception_data_illegal_instruction64_defined 1
#define   exception_data_illegal_instruction32 exception_data_illegal_instruction_compat
#define   exception_data_illegal_instruction64 exception_data_illegal_instruction
#define __exception_data_invalid_segment32_defined 1
#define __exception_data_invalid_segment64_defined 1
#define   exception_data_invalid_segment32 exception_data_invalid_segment_compat
#define   exception_data_invalid_segment64 exception_data_invalid_segment
#define __exception_rt_data32_defined 1
#define __exception_rt_data64_defined 1
#define   exception_rt_data32 exception_rt_data_compat
#define   exception_rt_data64 exception_rt_data
#define __exception_data_noncontinuable32_defined 1
#define __exception_data_noncontinuable64_defined 1
#define   exception_data_noncontinuable32 exception_data_noncontinuable_compat
#define   exception_data_noncontinuable64 exception_data_noncontinuable
#define __exception_data_badalloc32_defined 1
#define __exception_data_badalloc64_defined 1
#define   exception_data_badalloc32 exception_data_badalloc_compat
#define   exception_data_badalloc64 exception_data_badalloc
#define __exception_data_invalid_handle32_defined 1
#define __exception_data_invalid_handle64_defined 1
#define   exception_data_invalid_handle32 exception_data_invalid_handle_compat
#define   exception_data_invalid_handle64 exception_data_invalid_handle
#define __exception_data_divide_by_zero32_defined 1
#define __exception_data_divide_by_zero64_defined 1
#define   exception_data_divide_by_zero32 exception_data_divide_by_zero_compat
#define   exception_data_divide_by_zero64 exception_data_divide_by_zero
#define __exception_data_index_error32_defined 1
#define __exception_data_index_error64_defined 1
#define   exception_data_index_error32 exception_data_index_error_compat
#define __exception_data_index_error64 exception_data_index_error
#define __exception_data_buffer_too_small32_defined 1
#define __exception_data_buffer_too_small64_defined 1
#define   exception_data_buffer_too_small32 exception_data_buffer_too_small_compat
#define   exception_data_buffer_too_small64 exception_data_buffer_too_small
#define __exception_data_filesystem_error32_defined 1
#define __exception_data_filesystem_error64_defined 1
#define   exception_data_filesystem_error32 exception_data_filesystem_error_compat
#define   exception_data_filesystem_error64 exception_data_filesystem_error
#define __exception_data_net_error32_defined 1
#define __exception_data_net_error64_defined 1
#define   exception_data_net_error32 exception_data_net_error_compat
#define   exception_data_net_error64 exception_data_net_error
#define __exception_data_no_device32_defined 1
#define __exception_data_no_device64_defined 1
#define   exception_data_no_device32 exception_data_no_device_compat
#define   exception_data_no_device64 exception_data_no_device
#define __exception_data_unhandled_interrupt32_defined 1
#define __exception_data_unhandled_interrupt64_defined 1
#define   exception_data_unhandled_interrupt32 exception_data_unhandled_interrupt_compat
#define   exception_data_unhandled_interrupt64 exception_data_unhandled_interrupt
#define __exception_data_unknown_systemcall32_defined 1
#define __exception_data_unknown_systemcall64_defined 1
#define   exception_data_unknown_systemcall32 exception_data_unknown_systemcall_compat
#define   exception_data_unknown_systemcall64 exception_data_unknown_systemcall
#define __exception_data_exit32_defined 1
#define __exception_data_exit64_defined 1
#define   exception_data_exit32 exception_data_exit_compat
#define   exception_data_exit64 exception_data_exit
#define __exception_data_retry_rwlock32_defined 1
#define __exception_data_retry_rwlock64_defined 1
#define   exception_data_retry_rwlock32 exception_data_retry_rwlock_compat
#define   exception_data_retry_rwlock64 exception_data_retry_rwlock
#define __exception_data32_defined 1
#define __exception_data64_defined 1
#define   exception_data32 exception_data_compat
#define   exception_data64 exception_data
#define __exception_info32_defined 1
#define __exception_info64_defined 1
#define   exception_info32 exception_info_compat
#define   exception_info64 exception_info
#ifdef __KERNEL__
#define __user_exception_data32_defined 1
#define __user_exception_data64_defined 1
#define   user_exception_data32 user_exception_data_compat
#define   user_exception_data64 user_exception_data
#define __user_exception_rt_data32_defined 1
#define __user_exception_rt_data64_defined 1
#define   user_exception_rt_data32 user_exception_rt_data_compat
#define   user_exception_rt_data64 user_exception_rt_data
#define __user_exception_info32_defined 1
#define __user_exception_info64_defined 1
#define   user_exception_info32 user_exception_info_compat
#define   user_exception_info64 user_exception_info
#endif /* __KERNEL__ */
#define __exception_descriptor32_defined 1
#define __exception_descriptor64_defined 1
#define   exception_descriptor32 exception_descriptor_compat
#define   exception_descriptor64 exception_descriptor
#define __exception_handler32_defined 1
#define __exception_handler64_defined 1
#define   exception_handler32 exception_handler_compat
#define   exception_handler64 exception_handler
#else
#define __cpu_context_compat_defined 1
#define __CPU_CONTEXT_COMPAT_SIZE X86_CONTEXT64_SIZE
#define   cpu_context_compat x86_context64
#define __cpu_usercontext_compat_defined 1
#define __CPU_USERCONTEXT_COMPAT_SIZE X86_USERCONTEXT64_SIZE
#define   cpu_usercontext_compat x86_usercontext64
#define __exception_data_segfault32_defined 1
#define __exception_data_segfault64_defined 1
#define   exception_data_segfault32 exception_data_segfault
#define   exception_data_segfault64 exception_data_segfault_compat
#define __exception_data_system32_defined 1
#define __exception_data_system64_defined 1
#define   exception_data_system32 exception_data_system
#define   exception_data_system64 exception_data_system_compat
#define __exception_data_illegal_instruction32_defined 1
#define __exception_data_illegal_instruction64_defined 1
#define   exception_data_illegal_instruction32 exception_data_illegal_instruction
#define   exception_data_illegal_instruction64 exception_data_illegal_instruction_compat
#define __exception_data_invalid_segment32_defined 1
#define   exception_data_invalid_segment32 exception_data_invalid_segment
#define __exception_data_invalid_segment64_defined 1
#define   exception_data_invalid_segment64 exception_data_invalid_segment_compat
#define __exception_rt_data32_defined 1
#define __exception_rt_data64_defined 1
#define   exception_rt_data32 exception_rt_data
#define   exception_rt_data64 exception_rt_data_compat
#define __exception_data_noncontinuable32_defined 1
#define __exception_data_noncontinuable64_defined 1
#define   exception_data_noncontinuable32 exception_data_noncontinuable
#define   exception_data_noncontinuable64 exception_data_noncontinuable_compat
#define __exception_data_badalloc32_defined 1
#define __exception_data_badalloc64_defined 1
#define   exception_data_badalloc32 exception_data_badalloc
#define   exception_data_badalloc64 exception_data_badalloc_compat
#define __exception_data_invalid_handle32_defined 1
#define __exception_data_invalid_handle64_defined 1
#define   exception_data_invalid_handle32 exception_data_invalid_handle
#define   exception_data_invalid_handle64 exception_data_invalid_handle_compat
#define __exception_data_divide_by_zero32_defined 1
#define __exception_data_divide_by_zero64_defined 1
#define   exception_data_divide_by_zero32 exception_data_divide_by_zero
#define   exception_data_divide_by_zero64 exception_data_divide_by_zero_compat
#define __exception_data_index_error32_defined 1
#define __exception_data_index_error64_defined 1
#define   exception_data_index_error32 exception_data_index_error
#define __exception_data_index_error64 exception_data_index_error_compat
#define __exception_data_buffer_too_small32_defined 1
#define __exception_data_buffer_too_small64_defined 1
#define   exception_data_buffer_too_small32 exception_data_buffer_too_small
#define   exception_data_buffer_too_small64 exception_data_buffer_too_small_compat
#define __exception_data_filesystem_error32_defined 1
#define __exception_data_filesystem_error64_defined 1
#define   exception_data_filesystem_error32 exception_data_filesystem_error
#define   exception_data_filesystem_error64 exception_data_filesystem_error_compat
#define __exception_data_net_error32_defined 1
#define __exception_data_net_error64_defined 1
#define   exception_data_net_error32 exception_data_net_error
#define   exception_data_net_error64 exception_data_net_error_compat
#define __exception_data_no_device32_defined 1
#define __exception_data_no_device64_defined 1
#define   exception_data_no_device32 exception_data_no_device
#define   exception_data_no_device64 exception_data_no_device_compat
#define __exception_data_unhandled_interrupt32_defined 1
#define __exception_data_unhandled_interrupt64_defined 1
#define   exception_data_unhandled_interrupt32 exception_data_unhandled_interrupt
#define   exception_data_unhandled_interrupt64 exception_data_unhandled_interrupt_compat
#define __exception_data_unknown_systemcall32_defined 1
#define __exception_data_unknown_systemcall64_defined 1
#define   exception_data_unknown_systemcall32 exception_data_unknown_systemcall
#define   exception_data_unknown_systemcall64 exception_data_unknown_systemcall_compat
#define __exception_data_exit32_defined 1
#define __exception_data_exit64_defined 1
#define   exception_data_exit32 exception_data_exit
#define   exception_data_exit64 exception_data_exit_compat
#define __exception_data_retry_rwlock32_defined 1
#define __exception_data_retry_rwlock64_defined 1
#define   exception_data_retry_rwlock32 exception_data_retry_rwlock
#define   exception_data_retry_rwlock64 exception_data_retry_rwlock_compat
#define __exception_data32_defined 1
#define __exception_data64_defined 1
#define   exception_data32 exception_data
#define   exception_data64 exception_data_compat
#define __exception_info32_defined 1
#define __exception_info64_defined 1
#define   exception_info32 exception_info
#define   exception_info64 exception_info_compat
#ifdef __KERNEL__
#define __user_exception_data32_defined 1
#define __user_exception_data64_defined 1
#define   user_exception_data32 user_exception_data
#define   user_exception_data64 user_exception_data_compat
#define __user_exception_rt_data32_defined 1
#define __user_exception_rt_data64_defined 1
#define   user_exception_rt_data32 user_exception_rt_data
#define   user_exception_rt_data64 user_exception_rt_data_compat
#define __user_exception_info32_defined 1
#define __user_exception_info64_defined 1
#define   user_exception_info32 user_exception_info
#define   user_exception_info64 user_exception_info_compat
#endif /* __KERNEL__ */
#define __exception_descriptor32_defined 1
#define __exception_descriptor64_defined 1
#define   exception_descriptor32 exception_descriptor
#define   exception_descriptor64 exception_descriptor_compat
#define __exception_handler32_defined 1
#define __exception_handler64_defined 1
#define   exception_handler32 exception_handler
#define   exception_handler64 exception_handler_compat
#endif


/* Exception data structures for compatibility mode. */
#ifdef __CC__
#define __exception_data_segfault_compat_defined 1
struct __ATTR_PACKED exception_data_segfault_compat {
    __X86_INTPTRCC       sf_reason;
    __X86_PTRCC(void)    sf_vaddr;
};
#define __exception_data_system_compat_defined 1
struct __ATTR_PACKED exception_data_system_compat {
    __X86_INTPTRCC       s_errcode;
};
#define __exception_data_illegal_instruction_compat_defined 1
struct __ATTR_PACKED exception_data_illegal_instruction_compat {
    __X86_INTPTRCC       ii_errcode;
    __X86_INTPTRCC       ii_type;
    __X86_INTPTRCC       ii_register_type;
    __X86_INTPTRCC       ii_register_number;
    __UINT64_TYPE__      ii_value;
};
#define __exception_data_invalid_segment_compat_defined 1
struct __ATTR_PACKED exception_data_invalid_segment_compat {
    __X86_INTPTRCC     __is_errcode;
    __X86_INTPTRCC     __is_type;
    __X86_INTPTRCC     __is_register_type;
    __X86_INTPTRCC       is_register;
    union __ATTR_PACKED {
        __UINT64_TYPE__  is_segment;
        __UINT16_TYPE__  is_segment16;
    };
};
#endif /* __CC__ */


#define __EXCEPTION_RT_DATA_COMPAT_OFFSETOF_FREE_SP 0
#define __EXCEPTION_RT_DATA_COMPAT_SIZE             __SIZEOF_X86_INTPTRCC__
#ifdef __CC__
#define __exception_rt_data_compat_defined 1
struct __ATTR_PACKED exception_rt_data_compat {
    __X86_INTPTRCC       xrt_free_sp;
};
#endif /* __CC__ */


#ifdef __CC__
#define __exception_data_noncontinuable_compat_defined 1
struct __ATTR_PACKED exception_data_noncontinuable_compat {
    __UINT16_TYPE__     nc_origcode;
    __UINT16_TYPE__     nc_origflag;
#if __SIZEOF_X86_INTPTRCC__ > 4
    __UINT16_TYPE__   __nc_pad[(__SIZEOF_X86_INTPTRCC__-4)/2];
#endif
    __X86_INTPTRCC      nc_origip;
};
#define __exception_data_badalloc_compat_defined 1
struct __ATTR_PACKED exception_data_badalloc_compat {
    __UINT16_TYPE__      ba_resource;
    __UINT16_TYPE__      ba_pad[(sizeof(__X86_INTPTRCC)-2)/2];
    __X86_INTPTRCC       ba_amount;
};
#define __exception_data_invalid_handle_compat_defined 1
struct __ATTR_PACKED exception_data_invalid_handle_compat {
    __INT32_TYPE__       h_handle;
    __UINT16_TYPE__      h_reason;
    __UINT16_TYPE__      h_istype;
    __UINT16_TYPE__      h_rqtype;
    union __ATTR_PACKED {
        __UINT16_TYPE__  h_illhnd;
        __UINT16_TYPE__  h_rqkind;
    };
};
#define __exception_data_divide_by_zero_compat_defined 1
struct __ATTR_PACKED exception_data_divide_by_zero_compat {
    __UINT16_TYPE__      dz_type;
    __UINT16_TYPE__      dz_flag;
    __UINT16_TYPE__    __dz_pad[2];
    union __ATTR_PACKED {
        __INT64_TYPE__   da_int;
        __UINT64_TYPE__  da_uint;
        float            da_flt;
        double           da_dbl;
        long double      da_ldbl;
    }                    dz_arg;
};
#define __exception_data_index_error_compat_defined 1
struct __ATTR_PACKED exception_data_index_error_compat {
    __UINT64_TYPE__    __b_pad;
    __UINT64_TYPE__      b_index;
    __UINT64_TYPE__      b_boundmin;
    __UINT64_TYPE__      b_boundmax;
};
#define __exception_data_buffer_too_small_compat_defined 1
struct __ATTR_PACKED exception_data_buffer_too_small_compat {
    __X86_INTPTRCC       bs_bufsize;
    __X86_INTPTRCC       bs_reqsize;
};
#define __exception_data_filesystem_error_compat_defined 1
struct __ATTR_PACKED exception_data_filesystem_error_compat {
    __UINT16_TYPE__      fs_errcode;
};
#define __exception_data_net_error_compat_defined 1
struct __ATTR_PACKED exception_data_net_error_compat {
    __UINT16_TYPE__      n_errcode;
};
#define __exception_data_no_device_compat_defined 1
struct __ATTR_PACKED exception_data_no_device_compat {
    __UINT16_TYPE__      d_type;
    __UINT16_TYPE__    __d_pad[(sizeof(__X86_PTRCC(void))-2)/2]; /* ... */
    __dev_t              d_devno;
};
#define __exception_data_unhandled_interrupt_compat_defined 1
struct __ATTR_PACKED exception_data_unhandled_interrupt_compat {
    __X86_INTPTRCC       ui_errcode;
    __UINT8_TYPE__       ui_intcode;
    __UINT8_TYPE__     __ui_pad[sizeof(__X86_PTRCC(void))-1];
};
#define __exception_data_unknown_systemcall_compat_defined 1
struct __ATTR_PACKED exception_data_unknown_systemcall_compat {
    __X86_INTPTRCC       us_sysno;
    __X86_INTPTRCC       us_args[__ARCH_SYSCALL_MAX_ARGC];
};
#define __exception_data_exit_compat_defined 1
struct __ATTR_PACKED exception_data_exit_compat {
    int                  e_status;
};
#define __exception_data_retry_rwlock_compat_defined 1
struct __ATTR_PACKED exception_data_retry_rwlock_compat {
    __X86_PTRCC(struct rwlock)   e_rwlock_ptr;
};
#endif /* __CC__ */


#define __EXCEPTION_DATA_COMPAT_OFFSETOF_CODE     0
#define __EXCEPTION_DATA_COMPAT_OFFSETOF_FLAG     2
#define __EXCEPTION_DATA_COMPAT_OFFSETOF_POINTERS __SIZEOF_X86_INTPTRCC__
#define __EXCEPTION_DATA_COMPAT_SIZE             (__SIZEOF_X86_INTPTRCC__ * (__EXCEPTION_INFO_NUM_DATA_POINTERS+1))
#ifdef __CC__
#define __exception_data_compat_defined 1
struct exception_data_compat {
    __UINT16_TYPE__              e_code;
    __UINT16_TYPE__              e_flag;
#if __SIZEOF_X86_INTPTRCC__ > 4
    __UINT16_TYPE__            __e_pad[(__SIZEOF_X86_INTPTRCC__-4)/2];
#endif
    union __ATTR_PACKED {
        __X86_PTRCC(void) e_pointers[__EXCEPTION_INFO_NUM_DATA_POINTERS];
        __X86_INTPTRCC    e_words[__EXCEPTION_INFO_NUM_DATA_POINTERS];
#define __PRIVATE_DEFINE_EXCEPTION_DATA_MEMBER(name) \
  __IF_DEFINED(__exception_data_##name##_compat_defined,struct exception_data_##name##_compat e_##name;)
        __PRIVATE_FOREACH_EXCEPTION_DATA(__PRIVATE_DEFINE_EXCEPTION_DATA_MEMBER)
#undef __PRIVATE_DEFINE_EXCEPTION_DATA_MEMBER
    };
};
#endif /* __CC__ */


#define __EXCEPTION_INFO_COMPAT_OFFSETOF_ERROR    0
#define __EXCEPTION_INFO_COMPAT_OFFSETOF_RTDATA   __EXCEPTION_DATA_COMPAT_SIZE
#define __EXCEPTION_INFO_COMPAT_OFFSETOF_CONTEXT (__EXCEPTION_DATA_COMPAT_SIZE+__EXCEPTION_RT_DATA_COMPAT_SIZE)
#define __EXCEPTION_INFO_COMPAT_SIZE             (__EXCEPTION_DATA_COMPAT_SIZE+__EXCEPTION_RT_DATA_COMPAT_SIZE+__CPU_CONTEXT_COMPAT_SIZE)
#ifdef __CC__
#define __exception_info_compat_defined 1
struct __ATTR_PACKED exception_info_compat {
    struct exception_data_compat    e_error;
    struct exception_rt_data_compat e_rtdata;
    struct cpu_context_compat       e_context;
};
#endif /* __CC__ */

#ifdef __KERNEL__
#define __user_exception_data_compat_defined 1
#define __USER_EXCEPTION_DATA_COMPAT_SIZE __EXCEPTION_DATA_COMPAT_SIZE
#define   user_exception_data_compat exception_data_compat
#define __user_exception_rt_data_compat_defined 1
#define __USER_EXCEPTION_RT_DATA_COMPAT_SIZE __EXCEPTION_RT_DATA_COMPAT_SIZE
#define   user_exception_rt_data_compat exception_rt_data_compat
#define __user_exception_info_compat_defined 1
#define __USER_EXCEPTION_INFO_COMPAT_OFFSETOF_ERROR    0
#define __USER_EXCEPTION_INFO_COMPAT_OFFSETOF_RTDATA   __USER_EXCEPTION_DATA_COMPAT_SIZE
#define __USER_EXCEPTION_INFO_COMPAT_OFFSETOF_CONTEXT (__USER_EXCEPTION_DATA_COMPAT_SIZE+__USER_EXCEPTION_RT_DATA_COMPAT_SIZE)
#define __USER_EXCEPTION_INFO_COMPAT_SIZE             (__USER_EXCEPTION_DATA_COMPAT_SIZE+__USER_EXCEPTION_RT_DATA_COMPAT_SIZE+__CPU_USERCONTEXT_COMPAT_SIZE)
#ifdef __CC__
struct __ATTR_PACKED user_exception_info_compat {
    struct user_exception_data_compat    e_error;
    struct user_exception_rt_data_compat e_rtdata;
    struct cpu_usercontext_compat        e_context;
};
#endif /* __KERNEL__ */
#endif /* __CC__ */


#define __EXCEPTION_DESCRIPTOR_COMPAT_OFFSETOF_HANDLER 0
#define __EXCEPTION_DESCRIPTOR_COMPAT_OFFSETOF_TYPE    __SIZEOF_X86_INTPTRCC__
#define __EXCEPTION_DESCRIPTOR_COMPAT_OFFSETOF_FLAGS  (__SIZEOF_X86_INTPTRCC__+2)
#define __EXCEPTION_DESCRIPTOR_COMPAT_OFFSETOF_SAFE   (__SIZEOF_X86_INTPTRCC__+4)
#define __EXCEPTION_DESCRIPTOR_COMPAT_SIZE            (__SIZEOF_X86_INTPTRCC__+8)
#define __exception_descriptor_compat_defined 1
#ifdef __CC__
struct exception_descriptor_compat {
    __X86_PTRCC(void) ed_handler;
    __UINT16_TYPE__   ed_type;
    __UINT16_TYPE__   ed_flags;
    __UINT16_TYPE__   ed_safe;
    __UINT16_TYPE__ __ed_pad;
};
#endif /* __CC__ */


#define __EXCEPTION_HANDLER_COMPAT_OFFSETOF_BEGIN  0
#define __EXCEPTION_HANDLER_COMPAT_OFFSETOF_END    __SIZEOF_X86_INTPTRCC__
#define __EXCEPTION_HANDLER_COMPAT_OFFSETOF_ENTRY (__SIZEOF_X86_INTPTRCC__*2)
#define __EXCEPTION_HANDLER_COMPAT_OFFSETOF_FLAG  (__SIZEOF_X86_INTPTRCC__*3)
#define __EXCEPTION_HANDLER_COMPAT_OFFSETOF_MASK  (__SIZEOF_X86_INTPTRCC__*3+(__SIZEOF_X86_INTPTRCC__/2))
#define __EXCEPTION_HANDLER_COMPAT_SIZE           (__SIZEOF_X86_INTPTRCC__*4)
#define __exception_handler_compat_defined 1
#ifdef __CC__
struct __ATTR_PACKED exception_handler_compat {
    __X86_PTRCC(void)    eh_begin;
    __X86_PTRCC(void)    eh_end;
    union __ATTR_PACKED {
        __X86_PTRCC(struct exception_descriptor const)
                         eh_descr;
        __X86_INTPTRCC   eh_entry;
    };
    __X86_INTHALFPTRCC   eh_flag;
    __X86_INTHALFPTRCC   eh_mask;
};


#endif /* __CC__ */
#endif /* __EXPOSE_CPU_COMPAT */

__SYSDECL_END

#endif /* !_KOS_I386_KOS_BITS_EXCEPT_COMPAT_H */
