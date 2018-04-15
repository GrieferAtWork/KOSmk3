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
#ifndef GUARD_HYBRID_EXCEPT_PRINT_C
#define GUARD_HYBRID_EXCEPT_PRINT_C 1
#define __EXPOSE_CPU_CONTEXT 1

#include "hybrid.h"

#ifdef __KERNEL__
#include <kernel/debug.h>
#include <sched/task.h>
#include <sched/pid.h>
#include <unwind/eh_frame.h>
#include <unwind/linker.h>
#include <sched/userstack.h>
#else
#include "../libs/libc/libc.h"
#include "../libs/libc/format.h"
#include "../libs/libc/rtl.h"
#include "../libs/libc/sched.h"
#include "../libs/libc/system.h"
#include "../libs/libc/stdio.h"
#endif

#include <hybrid/compiler.h>
#include <except.h>
#include <errno.h>
#include <syslog.h>

DECL_BEGIN


#ifdef __KERNEL__
#define GETCPUID()   THIS_CPU->cpu_id
#else
#define GETCPUID()   libc_sched_getcpu()
#endif


#ifdef __KERNEL__
#define PRINTF(...) debug_printf(__VA_ARGS__)
INTERN void KCALL kernel_print_exception(void)
#else
#if 1
#define VPRINTF(f,a) libc_vsyslog(LOG_ERROR,f,a)
#define PRINTF(...)  libc_syslog(LOG_ERROR,__VA_ARGS__)
#else
#define VPRINTF(f,a) libc_vfprintf(fp,f,a)
#define PRINTF(...)  libc_fprintf(fp,__VA_ARGS__)
#endif
INTERN void LIBCCALL
libc_error_vfprintf(FILE *fp, char const *reason, va_list args)
#endif
{
#undef INFO
#ifdef __KERNEL__
 u16 code;
 struct exception_info *INFO;
 PRINTF("Unhandled exception:\n");
#else
#define INFO   (&info_struct)
 struct exception_info info_struct; u16 code;
 bool is_first = true; uintptr_t last_ip = 0;
 if (!fp) fp = libc_stderr;
 if (reason)
  VPRINTF(reason,args);
 else {
  PRINTF("Unhandled exception:\n");
 }
#endif
 code = error_code();
 PRINTF("\tcode:  %I16u (%I16x)\n",code,code);
#ifdef __KERNEL__
 INFO = error_info();
#else
 libc_memcpy(&info_struct,libc_error_info(),sizeof(struct exception_info));
#endif
 PRINTF("\tflags: %I16x (%s,%s)\n",
        INFO->e_error.e_flag,
        INFO->e_error.e_flag&ERR_FRESUMABLE ? "resumable" : "-",
        INFO->e_error.e_flag&ERR_FRESUMENEXT ? "prefault" : "-");
 {
  cpuid_t cpuid = (cpuid_t)GETCPUID();
  PRINTF("\tCPU:   #%u%s\n",
        (unsigned int)cpuid,
         cpuid == 0 ? " (boot CPU)" : "");
 }
 /* Print exception-specific information. */
 switch (code) {
 case E_NONCONTINUABLE:
  PRINTF("\tAttempted to continue a non-continuable exception\n"
         "\t\tOriginal code:  %I16u (%I16x)\n"
         "\t\tOriginal flags: %I16x (%s,%s)\n"
         "%[vinfo:%f(%l,%c) : %n : %p] : Original IP : %p\n",
         INFO->e_error.e_noncont.nc_origcode,INFO->e_error.e_noncont.nc_origcode,
         INFO->e_error.e_noncont.nc_origflag,
         INFO->e_error.e_noncont.nc_origflag&ERR_FRESUMABLE ? "resumable" : "-",
         INFO->e_error.e_noncont.nc_origflag&ERR_FRESUMENEXT ? "prefault" : "-",
         INFO->e_error.e_noncont.nc_origip-((INFO->e_error.e_noncont.nc_origflag&ERR_FRESUMENEXT) ? 0 : 1),
         INFO->e_error.e_noncont.nc_origip);
  break;
 case E_BADALLOC:
  if (INFO->e_error.e_badalloc.ba_resource == ERROR_BADALLOC_VIRTMEMORY) {
   PRINTF("\tFailed locate space for %Iu (%#Ix) bytes of virtual memory\n",
                INFO->e_error.e_badalloc.ba_amount,INFO->e_error.e_badalloc.ba_amount);
  } else if (INFO->e_error.e_badalloc.ba_resource == ERROR_BADALLOC_PHYSMEMORY) {
   PRINTF("\tFailed to allocate %Iu (%#Ix) bytes of physical memory\n",
                INFO->e_error.e_badalloc.ba_amount,INFO->e_error.e_badalloc.ba_amount);
  } else {
   PRINTF("\tFailed to allocate %Iu (%#Ix) of resource %d\n",
                INFO->e_error.e_badalloc.ba_amount,INFO->e_error.e_badalloc.ba_amount,
                INFO->e_error.e_badalloc.ba_resource);
  }
  break;
 case E_STACK_OVERFLOW:
 case E_SEGFAULT:
  PRINTF(
#ifdef __KERNEL__
         "\t%s when %s%s %svirtual address %p\n",
#else
         "\t%s when %s %svirtual address %p\n",
#endif
         code == E_SEGFAULT ? "Segmentation Fault" : "Stack Overflow",
#if defined(__i386__) || defined(__x86_64__)
#ifdef __KERNEL__
         INFO->e_error.e_segfault.sf_reason&X86_SEGFAULT_FUSER ? "userspace was " : "",
#endif
         INFO->e_error.e_segfault.sf_reason&X86_SEGFAULT_FWRITE ? "writing" :
         INFO->e_error.e_segfault.sf_reason&X86_SEGFAULT_FEXEC ? "executing" : "reading",
         INFO->e_error.e_segfault.sf_reason&X86_SEGFAULT_FPRESENT ? "" : "unmapped ",
#else
         "accessing","",
#endif
         INFO->e_error.e_segfault.sf_vaddr);
  break;
 case E_INVALID_ALIGNMENT:
   PRINTF("\tThe alignment of an operand was not valid\n");
   break;
 case E_DIVIDE_BY_ZERO:
  switch (INFO->e_error.e_divide_by_zero.dz_type) {
  case ERROR_DIVIDE_BY_ZERO_INT:
   PRINTF("\tAttempted to divide signed integer %I64d by 0\n",
          INFO->e_error.e_divide_by_zero.dz_arg.da_int);
   break;
  case ERROR_DIVIDE_BY_ZERO_UINT:
   PRINTF("\tAttempted to divide unsigned integer %I64u by 0\n",
          INFO->e_error.e_divide_by_zero.dz_arg.da_uint);
   break;
  default:
   PRINTF("\tAttempted to divide by 0\n");
   break;
  }
  break;
 case E_OVERFLOW:
  PRINTF("\tArithmetic overflow\n");
  break;
 case E_ILLEGAL_INSTRUCTION:
  PRINTF("\tIllegal instruction\n");
  break;
 case E_NOT_IMPLEMENTED:
  PRINTF("\tFeature or function has not been implemented\n");
  break;
 case E_IOERROR:
  PRINTF("\tHardware error, or miss-behaving/miss-configured device\n");
  break;
 case E_PRIVILEGED_INSTRUCTION:
  PRINTF("\tPrivileged instruction\n");
  break;
 case E_UNHANDLED_INTERRUPT:
  PRINTF("\tUnhandled interrupt %#x (errcode %#x)\n",
         INFO->e_error.e_unhandled_interrupt.ui_intcode,
         INFO->e_error.e_unhandled_interrupt.ui_errcode);
  break;
 case E_UNKNOWN_SYSTEMCALL:
  PRINTF("\tUnknown system call %#Ix { %p, %p, %p, %p, %p, %p }\n",
         INFO->e_error.e_unknown_systemcall.us_sysno,
         INFO->e_error.e_unknown_systemcall.us_args[0],
         INFO->e_error.e_unknown_systemcall.us_args[1],
         INFO->e_error.e_unknown_systemcall.us_args[2],
         INFO->e_error.e_unknown_systemcall.us_args[3],
         INFO->e_error.e_unknown_systemcall.us_args[4],
         INFO->e_error.e_unknown_systemcall.us_args[5]);
  break;
 case E_INVALID_HANDLE:
  /* TODO: Include new information in error messages. */
  PRINTF("\tInvalid handle number %d\n",
         INFO->e_error.e_invalid_handle.h_handle);
  break;
 case E_NO_DATA:
  PRINTF("\tIndexed or named object or data block does not exist\n");
  break;
 case E_NOT_EXECUTABLE:
  PRINTF("\tThe named file was not recognized as a valid executable\n");
  break;
 case E_WOULDBLOCK:
  PRINTF("\tA blocking call was attempted when non-blocking behavior was requested\n");
  break;
 case E_INDEX_ERROR:
  PRINTF("\tIndex %I64u is out-of-bounds of %I64u...%I64u\n",
         INFO->e_error.e_index_error.b_index,
         INFO->e_error.e_index_error.b_boundmin,
         INFO->e_error.e_index_error.b_boundmax);
  break;
 case E_BREAKPOINT:
  PRINTF("\tBreakpoint\n");
  break;
 case E_INVALID_ARGUMENT:
  PRINTF("\tInvalid argument\n");
  break;
 case E_TOO_MANY_HANDLES:
  PRINTF("\tToo many open handles\n");
  break;
 case E_PROCESS_EXITED:
  PRINTF("\tProcess has exited\n");
  break;
 case E_NO_DEVICE:
  PRINTF("\tThe %s-device %[dev] doesn't exist\n",
         INFO->e_error.e_no_device.d_type == ERROR_NO_DEVICE_FBLOCKDEV ? "block" : "character",
         INFO->e_error.e_no_device.d_devno);
  break;
 case E_FILESYSTEM_ERROR:
  PRINTF("\tFilesystem error %u\n",
         INFO->e_error.e_filesystem_error.fs_errcode);
  switch (INFO->e_error.e_filesystem_error.fs_errcode) {
  case ERROR_FS_FILE_NOT_FOUND:        PRINTF("\t\tFile could not be found\n"); break;
  case ERROR_FS_PATH_NOT_FOUND:        PRINTF("\t\tDirectory could not be found\n"); break;
  case ERROR_FS_NOT_A_DIRECTORY:       PRINTF("\t\tExpected a directory when encountering a file while traversing a path\n"); break;
  case ERROR_FS_TOO_MANY_LINKS:        PRINTF("\t\tToo many symbolic links encountered while evaluating a path\n"); break;
  case ERROR_FS_DIRECTORY_NOT_EMPTY:   PRINTF("\t\tCannot remove a directory that isn't empty\n"); break;
  case ERROR_FS_ILLEGAL_PATH:          PRINTF("\t\tA portion of the path contains characters that are not supported by the hosting filesystem\n"); break;
  case ERROR_FS_CROSSDEVICE_LINK:      PRINTF("\t\tThe source and destination of a `rename()' or `link()' operation do not target the same device\n"); break;
  case ERROR_FS_FILENAME_TOO_LONG:     PRINTF("\t\tA single path segment is longer than the max supported length\n"); break;
  case ERROR_FS_FILE_ALREADY_EXISTS:   PRINTF("\t\tThe target of a `rename()' or `link()' operation already exists, or `O_EXCL' was passed to `open()', and the file already exists\n"); break;
  case ERROR_FS_UNSUPPORTED_FUNCTION:  PRINTF("\t\tSome function required for the current operation is not supported by the filesystem\n"); break;
  case ERROR_FS_READONLY_FILESYSTEM:   PRINTF("\t\tThe filesystem has been mounted as read-only, or associated block-device cannot be written to during some operation that either requires\n"); break;
  case ERROR_FS_TRUNCATE_GREATER_SIZE: PRINTF("\t\tThe `truncate()' or `ftruncate()' function was called with an argument greater than the file's current length\n"); break;
  case ERROR_FS_ACCESS_ERROR:          PRINTF("\t\tThe calling process does not have sufficient permissions for the operation\n"); break;
  case ERROR_FS_DISK_FULL:             PRINTF("\t\tThe associated disk is full or insufficient space is available to complete the operation\n"); break;
  case ERROR_FS_RMDIR_REGULAR:         PRINTF("\t\t`rmdir()' cannot be used to remove a non-directory file\n"); break;
  case ERROR_FS_UNLINK_DIRECTORY:      PRINTF("\t\tCannot unlink() a directory\n"); break;
  case ERROR_FS_REMOVE_MOUNTPOINT:     PRINTF("\t\t`rmdir()' or `unlink()' cannot be used to remove a mounting point\n"); break;
  case ERROR_FS_UNMOUNT_NOTAMOUNT:     PRINTF("\t\t`umount()' cannot be used to remove something that isn't a mounting point\n"); break;
  case ERROR_FS_RENAME_NOTAMOUNT:      PRINTF("\t\t`rename()' cannot be used to perform a re-mount operation\n"); break;
  case ERROR_FS_CORRUPTED_FILESYSTEM:  PRINTF("\t\tCorrupted, miss-configured, or otherwise not compatible filesystem\n"); break;
  default: break;
  }
  break;

 case E_BUFFER_TOO_SMALL:
  PRINTF("\tThe provided buffer size of %Iu bytes is smaller than the requirement of %Iu bytes\n",
         INFO->e_error.e_buffer_too_small.bs_bufsize,
         INFO->e_error.e_buffer_too_small.bs_reqsize);
  break;
 
 case E_UNICODE_ERROR:
  PRINTF("\tAn illegal sequence was encountered in a unicode string\n");
  break;

 case E_INTERRUPT:
  PRINTF("\tThe calling thread was interrupted\n");
  break;

#ifdef E_INVALID_SEGMENT
 case E_INVALID_SEGMENT: {
  PRIVATE char const segment_names[] = {
      [INVALID_SEGMENT_REGISTER_DS] = 'd',
      [INVALID_SEGMENT_REGISTER_ES] = 'e',
      [INVALID_SEGMENT_REGISTER_FS] = 'f',
      [INVALID_SEGMENT_REGISTER_GS] = 'g',
      [INVALID_SEGMENT_REGISTER_SS] = 's',
      [INVALID_SEGMENT_REGISTER_CS] = 'c',
  };
  PRINTF("\tAttempted to start a thread or return from a signal handler "
           "with an invalid segment index 0x%.4I16X for register %%%cs\n",
         INFO->e_error.e_invalid_segment.is_segment,
         segment_names[INFO->e_error.e_invalid_segment.is_register]);
 } break;
#endif /* E_INVALID_SEGMENT */

#ifdef __KERNEL__
 case E_DRIVER_CLOSED:
  PRINTF("\tAttempted to register a new global hook using a closed driver\n");
  break;
 case E_RETRY_RWLOCK:
  PRINTF("\tThe thread should re-attempt to acquire an R/W-lock at %p\n",
         INFO->e_error.e_retry_rwlock.e_rwlock_ptr);
  break;
#endif

 default: break;
 }
 /* Print an arch-specific register state. */
#if defined(__x86_64__) || defined(__i386__)
#ifdef __x86_64__
 PRINTF("RAX %p  RCX %p  RDX %p  RBX %p  RIP %p\n"
        "RSP %p  RBP %p  RSI %p  RDI %p  ---\n"
        "R8  %p  R9  %p  R10 %p  R11 %p  ---\n"
        "R12 %p  R13 %p  R14 %p  R15 %p  ---\n",
        INFO->e_context.c_gpregs.gp_rax,
        INFO->e_context.c_gpregs.gp_rcx,
        INFO->e_context.c_gpregs.gp_rdx,
        INFO->e_context.c_gpregs.gp_rbx,
        INFO->e_context.c_rip,
        INFO->e_context.c_rsp,
        INFO->e_context.c_gpregs.gp_rbp,
        INFO->e_context.c_gpregs.gp_rsi,
        INFO->e_context.c_gpregs.gp_rdi,
        INFO->e_context.c_gpregs.gp_r8,
        INFO->e_context.c_gpregs.gp_r9,
        INFO->e_context.c_gpregs.gp_r10,
        INFO->e_context.c_gpregs.gp_r11,
        INFO->e_context.c_gpregs.gp_r12,
        INFO->e_context.c_gpregs.gp_r13,
        INFO->e_context.c_gpregs.gp_r14,
        INFO->e_context.c_gpregs.gp_r15);
#else /* __x86_64__ */
 PRINTF("EAX %p  ECX %p  EDX %p  EBX %p  EIP %p\n"
        "ESP %p  EBP %p  ESI %p  EDI %p  ---\n",
        INFO->e_context.c_gpregs.gp_eax,
        INFO->e_context.c_gpregs.gp_ecx,
        INFO->e_context.c_gpregs.gp_edx,
        INFO->e_context.c_gpregs.gp_ebx,
        INFO->e_context.c_eip,
        INFO->e_context.c_esp,
        INFO->e_context.c_gpregs.gp_ebp,
        INFO->e_context.c_gpregs.gp_esi,
        INFO->e_context.c_gpregs.gp_edi);
#endif /* !__x86_64__ */
#ifdef __KERNEL__
#ifdef CONFIG_X86_SEGMENTATION
 PRINTF("CS %.4IX DS %.4IX ES %.4IX FS %.4IX GS %.4IX\n",
        INFO->e_context.c_iret.ir_cs,
        INFO->e_context.c_segments.sg_ds,
        INFO->e_context.c_segments.sg_es,
        INFO->e_context.c_segments.sg_fs,
        INFO->e_context.c_segments.sg_gs);
#else /* CONFIG_X86_SEGMENTATION */
#define GETREG(name) XBLOCK({ register register_t v; __asm__("movl %" name ", %0" : "=r" (v)); XRETURN v; })
 PRINTF("CS %.4IX DS %.4IX ES %.4IX FS %.4IX GS %.4IX\n",
        INFO->e_context.c_iret.ir_cs,
        GETREG("%ds"),GETREG("%es"),
        GETREG("%fs"),GETREG("%gs"));
#undef GETREG
#endif /* !CONFIG_X86_SEGMENTATION */
#else /* __KERNEL__ */
#ifdef CONFIG_X86_SEGMENTATION
 PRINTF("CS %.4IX DS %.4IX ES %.4IX FS %.4IX GS %.4IX\n",
        INFO->e_context.c_cs,
        INFO->e_context.c_segments.sg_ds,
        INFO->e_context.c_segments.sg_es,
        INFO->e_context.c_segments.sg_fs,
        INFO->e_context.c_segments.sg_gs);
#else /* CONFIG_X86_SEGMENTATION */
#define GETREG(name) XBLOCK({ register register_t v; __asm__("movl %" name ", %0" : "=r" (v)); XRETURN v; })
 PRINTF("CS %.4IX DS %.4IX ES %.4IX FS %.4IX GS %.4IX\n",
        GETREG("%cs"),GETREG("%ds"),
        GETREG("%es"),GETREG("%fs"),
        GETREG("%gs"));
#undef GETREG
#endif /* !CONFIG_X86_SEGMENTATION */
#endif /* !__KERNEL__ */

#else
 /* XXX: Other architectures */
#endif

#ifdef __KERNEL__
 PRINTF("THIS_TASK = %p (%u)\n",THIS_TASK,posix_gettid());
#endif

#if defined(__KERNEL__) && 1
 //INFO->e_context.c_esp = X86_ANYCONTEXT32_ESP(*INFO->e_context);
 if (INFO->e_context.c_iret.ir_cs & 3) {
  struct userstack *stack = PERTASK_GET(_this_user_stack);
  if (stack != NULL) {
   PRINTF("stack: %p...%p\n",
          VM_PAGE2ADDR(stack->us_pagemin),
          VM_PAGE2ADDR(stack->us_pageend)-1);
   if (INFO->e_context.c_esp >= VM_PAGE2ADDR(stack->us_pagemin) &&
       INFO->e_context.c_esp <  VM_PAGE2ADDR(stack->us_pageend)) {
    PRINTF("%$[hex]\n",
          (VM_PAGE2ADDR(stack->us_pageend)-INFO->e_context.c_esp)+16,
           INFO->e_context.c_esp-16);
   }
  } else {
   PRINTF("No stack\n");
  }
 } else {
  PRINTF("stack: %p...%p\n",
        (uintptr_t)PERTASK_GET(this_task.t_stackmin),
        (uintptr_t)PERTASK_GET(this_task.t_stackend)-1);
  if (INFO->e_context.c_esp >= (uintptr_t)PERTASK_GET(this_task.t_stackmin) &&
      INFO->e_context.c_esp <= (uintptr_t)PERTASK_GET(this_task.t_stackend)) {
   PRINTF("%$[hex]\n",
         (uintptr_t)PERTASK_GET(this_task.t_stackend)-INFO->e_context.c_esp,
                    INFO->e_context.c_esp);
  }
 }
#endif

#ifdef __KERNEL__
 TRY {
  bool is_first = true;
  uintptr_t last_eip = 0;
  for (;;) {
   struct fde_info finfo;
   if (last_eip != INFO->e_context.c_eip) {
    PRINTF("%[vinfo:%f(%l,%c) : %n] : %p : ESP %p, EBP %p\n",
           is_first && (INFO->e_error.e_flag&ERR_FRESUMENEXT) ?
           INFO->e_context.c_eip : INFO->e_context.c_eip-1,
           INFO->e_context.c_eip,
           INFO->e_context.c_esp,INFO->e_context.c_gpregs.gp_ebp);
   }
   last_eip = INFO->e_context.c_eip;
   if (!linker_findfde_consafe(is_first && (INFO->e_error.e_flag&ERR_FRESUMENEXT) ?
                               INFO->e_context.c_eip : INFO->e_context.c_eip-1,&finfo)) {
 #if 0
    /* For the first entry, assume a standard unwind which can be used
     * to properly display tracebacks when execution tries to call a
     * NULL-function pointer. */
    if (!is_first) break;
    if (INFO->e_context.c_esp >= (uintptr_t)PERTASK_GET(this_task.t_stackend)) {
     PRINTF("SP is out-of-bounds (%p not in %p...%p)\n",
            INFO->e_context.c_esp,
           (uintptr_t)PERTASK_GET(this_task.t_stackmin),
           (uintptr_t)PERTASK_GET(this_task.t_stackend)-1);
     break;
    }
    INFO->e_context.c_eip = *(u32 *)INFO->e_context.c_esp;
    INFO->e_context.c_esp += 4;
 #else
    break;
 #endif
   } else {
    if (!eh_return(&finfo,&INFO->e_context,EH_FNORMAL)) {
 #if 0
     struct frame {
         struct frame *f_caller;
         void         *f_return;
     };
     struct frame *f;
     TRY {
      f = (struct frame *)INFO->e_context.c_gpregs.gp_ebp;
      INFO->e_context.c_eip           = (uintptr_t)f->f_return;
      INFO->e_context.c_esp           = (uintptr_t)(f+1);
      INFO->e_context.c_gpregs.gp_ebp = (uintptr_t)f->f_caller;
     } CATCH (E_SEGFAULT) {
      break;
     }
 #else
     break;
 #endif
    }
   }
   is_first = false;
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
 }
#else
 /* Print a traceback. */
 for (;;) {
  if (last_ip != INFO->e_context.c_eip) {
   PRINTF("%[vinfo:%f(%l,%c) : %n] : %p : SP %p\n",
          is_first && (INFO->e_error.e_flag&ERR_FRESUMENEXT) ?
          CPU_CONTEXT_IP(INFO->e_context) : CPU_CONTEXT_IP(INFO->e_context)-1,
          CPU_CONTEXT_IP(INFO->e_context),
          CPU_CONTEXT_SP(INFO->e_context));
  }
  if (sys_xunwind(&INFO->e_context,code,
                 (!is_first || !(INFO->e_error.e_flag&ERR_FRESUMENEXT))))
      break;
  is_first = false;
 }
 /* Copy back exception information in case it got overwritten in the mean time. */
 libc_memcpy(libc_error_info(),INFO,sizeof(struct exception_info));
#endif
#undef INFO
}


#ifndef __KERNEL__
EXPORT(error_vfprintf,libc_error_vfprintf);
#endif


DECL_END

#endif /* !GUARD_HYBRID_EXCEPT_PRINT_C */
