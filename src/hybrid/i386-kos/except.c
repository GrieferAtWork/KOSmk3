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
#ifndef GUARD_HYBRID_I386_KOS_EXCEPT_C
#define GUARD_HYBRID_I386_KOS_EXCEPT_C 1
#define _KOS_SOURCE 2
#define __EXPOSE_CPU_CONTEXT 1

#include "../hybrid.h"
#include <hybrid/compiler.h>
#include <hybrid/asm.h>
#include <kos/types.h>
#include <kos/context.h>
#include <kos/handle.h>
#include <except.h>
#include <string.h>

DECL_BEGIN

INTERN void FCALL private_error_continue(int retry) {
 struct exception_info *EXCEPT_VAR info = libc_error_info();
 struct exception_info EXCEPT_VAR context;
 if (!(info->e_error.e_flag & ERR_FRESUMABLE))
       goto non_continuable;
 /* Deal with resume function calls.
  * In this situation, we act similar to `setjmp()' / `longjmp()',
  * in that we forward the given retry value to the associated
  * `error_throw_resumable()' or `error_throw_current()' */
 if (info->e_error.e_flag & ERR_FRESUMEFUNC) {
  info->e_context.c_gpregs.gp_eax = retry;
  libc_cpu_setcontext(&info->e_context);
 }
 /* Directly jump to the saved CPU context. */
 if (((info->e_error.e_flag&ERR_FRESUMENEXT) == 0) == (retry == 0)) {
  libc_cpu_setcontext(&info->e_context);
 }
 /* Copy the context. */
 libc_memcpy((void *)&context,info,sizeof(struct exception_info));
 /* Load the address of a neighboring instruction.
  * NOTE: In order to deal with problems that might arise
  *       when the instruction pointer is faulty, this
  *       part is guarded by a SEGFAULT exception handler. */
 TRY {
  context.e_context.c_eip = retry ? (uintptr_t)libc_prev_instruction((void *)context.e_context.c_eip)
                                  : (uintptr_t)libc_next_instruction((void *)context.e_context.c_eip);
 } CATCH_HANDLED (E_SEGFAULT) {
  /* Restore the saved context */
  libc_memcpy(info,(void *)&context,sizeof(struct exception_info));
  goto non_continuable;
 }
 if (!context.e_context.c_eip)
      goto non_continuable;
 /* Load the custom, new CPU context. */
 libc_cpu_setcontext((struct cpu_context *)&context.e_context);
non_continuable:
 /* Setup a non-continuable error. */
 context.e_error.e_noncont.nc_origcode = info->e_error.e_code;
 context.e_error.e_noncont.nc_origflag = info->e_error.e_flag;
 context.e_error.e_noncont.nc_origip   = info->e_context.c_eip;
 libc_memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 libc_memcpy(&info->e_error.e_noncont,(void *)&context.e_error.e_noncont,
              sizeof(context.e_error.e_noncont));
}


INTERN void FCALL
libc_error_setf(except_t code, va_list args) {
 struct exception_info *info;
 info = libc_error_info();
 libc_memset(info->e_error.e_pointers,0,
             sizeof(info->e_error.e_pointers));
 switch (code) {

 case E_BADALLOC:
  info->e_error.e_badalloc.ba_resource = (u16)va_arg(args,unsigned int);
  info->e_error.e_badalloc.ba_amount   = va_arg(args,size_t);
  break;

 case E_INVALID_HANDLE:
  info->e_error.e_invalid_handle.h_handle = va_arg(args,int);
  info->e_error.e_invalid_handle.h_reason = (u16)va_arg(args,int);
  switch (info->e_error.e_invalid_handle.h_reason) {
  case ERROR_INVALID_HANDLE_FWRONGTYPE:
   info->e_error.e_invalid_handle.h_istype = (u16)va_arg(args,int);
   info->e_error.e_invalid_handle.h_rqtype = (u16)va_arg(args,int);
   break;
  case ERROR_INVALID_HANDLE_FWRONGKIND:
   info->e_error.e_invalid_handle.h_istype = (u16)va_arg(args,int);
   info->e_error.e_invalid_handle.h_rqtype = info->e_error.e_invalid_handle.h_istype;
   info->e_error.e_invalid_handle.h_rqkind = (u16)va_arg(args,int);
   break;
  default:
#if HANDLE_TYPE_FNONE != 0
   info->e_error.e_invalid_handle.h_istype = HANDLE_TYPE_FNONE;
   info->e_error.e_invalid_handle.h_rqtype = HANDLE_TYPE_FNONE;
#endif
   info->e_error.e_invalid_handle.h_illhnd = (u16)va_arg(args,int);
   break;
  }
  break;

 case E_SEGFAULT:
 case E_STACK_OVERFLOW:
  info->e_error.e_segfault.sf_reason = va_arg(args,unsigned int);
  info->e_error.e_segfault.sf_vaddr  = va_arg(args,void *);
  break;

 case E_DIVIDE_BY_ZERO:
  info->e_error.e_divide_by_zero.dz_type = (u16)va_arg(args,unsigned int);
  info->e_error.e_divide_by_zero.dz_flag = (u16)va_arg(args,unsigned int);
  switch (info->e_error.e_divide_by_zero.dz_type) {
  case ERROR_DIVIDE_BY_ZERO_INT:
  case ERROR_DIVIDE_BY_ZERO_UINT:
   info->e_error.e_divide_by_zero.dz_arg.da_uint = va_arg(args,u64);
   break;
#ifndef __KERNEL__
  case ERROR_DIVIDE_BY_ZERO_FLT:
   info->e_error.e_divide_by_zero.dz_arg.da_flt = (float)va_arg(args,double);
   break;
  case ERROR_DIVIDE_BY_ZERO_DBL:
   info->e_error.e_divide_by_zero.dz_arg.da_dbl = va_arg(args,double);
   break;
  case ERROR_DIVIDE_BY_ZERO_LDBL:
   info->e_error.e_divide_by_zero.dz_arg.da_ldbl = va_arg(args,long double);
   break;
#endif /* __KERNEL__ */
  default: break;
  }
  break;

 case E_INDEX_ERROR:
  info->e_error.e_index_error.b_index = va_arg(args,u64);
  info->e_error.e_index_error.b_boundmin = va_arg(args,u64);
  info->e_error.e_index_error.b_boundmax = va_arg(args,u64);
  break;

 case E_BUFFER_TOO_SMALL:
  info->e_error.e_buffer_too_small.bs_bufsize = va_arg(args,size_t);
  info->e_error.e_buffer_too_small.bs_reqsize = va_arg(args,size_t);
  break;

 case E_FILESYSTEM_ERROR:
 case E_NET_ERROR:
  info->e_error.e_filesystem_error.fs_errcode = (u16)va_arg(args,unsigned int);
  break;

 case E_NO_DEVICE:
  info->e_error.e_no_device.d_type  = (u16)va_arg(args,unsigned int);
  info->e_error.e_no_device.d_devno = va_arg(args,dev_t);
  break;

 case E_UNHANDLED_INTERRUPT:
  info->e_error.e_unhandled_interrupt.ui_intcode = (u8)va_arg(args,unsigned int);
  info->e_error.e_unhandled_interrupt.ui_errcode = va_arg(args,uintptr_t);
  break;

 {
  unsigned int i;
 case E_UNKNOWN_SYSTEMCALL:
  info->e_error.e_unknown_systemcall.us_sysno = va_arg(args,syscall_ulong_t);
  for (i = 0; i < __ARCH_SYSCALL_MAX_ARGC; ++i)
      info->e_error.e_unknown_systemcall.us_args[i] = va_arg(args,syscall_ulong_t);
 } break;

 case E_EXIT_THREAD:
 case E_EXIT_PROCESS:
  info->e_error.e_exit.e_status = va_arg(args,unsigned int);
  break;

#ifdef E_INVALID_SEGMENT
 case E_INVALID_SEGMENT:
  info->e_error.e_invalid_segment.is_register = (u16)va_arg(args,unsigned int);
  info->e_error.e_invalid_segment.is_segment  = (u16)va_arg(args,unsigned int);
  break;
#endif /* E_INVALID_SEGMENT */

 default: break;
 }
}


DECL_END

#endif /* !GUARD_HYBRID_I386_KOS_EXCEPT_C */
