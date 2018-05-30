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
#ifndef GUARD_KERNEL_I386_KOS_HW_EXCEPT_C
#define GUARD_KERNEL_I386_KOS_HW_EXCEPT_C 1
#define _KOS_SOURCE 1
#define _NOSERVE_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <asm/cpu-flags.h>
#include <except.h>
#include <i386-kos/interrupt.h>
#include <i386-kos/vm86.h>
#include <kernel/debug.h>
#include <kernel/interrupt.h>
#include <kernel/malloc.h>
#include <kernel/syscall.h>
#include <kernel/vm.h>
#include <kos/context.h>
#include <kos/i386-kos/asm/except.h>
#include <kos/i386-kos/asm/pf-syscall.h>
#include <kos/i386-kos/bits/cpu-context.h>
#include <kos/intrin.h>
#include <kos/types.h>
#include <sched/pid.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <syscall.h>
#include <unwind/eh_frame.h>
#include <unwind/linker.h>

#include "emulator.h"
#include "posix_signals.h"
#include <sched/userstack.h>

DECL_BEGIN

/* For all of this exception handling, we must be extremely careful
 * to always be able to properly determine the ERR_FRESUMENEXT bit.
 * If we fail to determine it properly, then exception handling
 * might not work properly, and error-continue will be broken as well! */


#ifdef CONFIG_DEBUG_MALLOC
INTERN ATTR_PERTASK bool mall_leak_nocore = false;
#endif

#ifndef CONFIG_NO_VIO
INTDEF bool KCALL
x86_handle_vio(struct cpu_anycontext *__restrict context,
               struct vm_region *__restrict region,
               uintptr_t addr, VIRT void *abs_addr);
#endif /* !CONFIG_NO_VIO */


INTERN void FCALL
error_rethrow_atuser(struct cpu_context *__restrict context) {
 if (!X86_ANYCONTEXT_ISUSER(*context)) {
  if (!(error_info()->e_error.e_flag & ERR_FRESUMENEXT))
        --CONTEXT_IP(*context);
  __error_rethrow_at(context);
 }
 task_propagate_user_exception((struct cpu_hostcontext_user *)context,
                                TASK_USERCTX_TYPE_INTR_INTERRUPT);
}


/* Hardware exception handling. */
INTERN void FCALL
x86_handle_pagefault(struct cpu_anycontext *__restrict context,
                     register_t errcode) {
 struct cpu_anycontext *EXCEPT_VAR xcontext = context;
 struct exception_info *info;
 void *EXCEPT_VAR fault_address;
 /* Extract the fault address before re-enabling interrupts. */
 fault_address = (void *)__rdcr2();
 assert(!PREEMPTION_ENABLED());
#ifdef __x86_64__
 debug_printf("#PF at %p (from %p; errcode %Ix; gs_base: %p)\n",
              fault_address,context->c_pip,errcode,
              __rdgsbaseq());
#endif
 /* Re-enable interrupts if they were enabled before. */
 if (context->c_pflags & EFLAGS_IF)
     x86_interrupt_enable();

again:
#ifdef CONFIG_DEBUG_MALLOC
 if (!PERTASK_TEST(mall_leak_nocore))
#endif
 {
  /* TODO: Disable RPC serving in here. (An arbitrary memory access causing
   *       an RPC serve really isn't something that any sort of code expects,
   *       especially when you start considering LOA for memory-mapped files...) */
  TRY {
   bool EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(vm_ok);
   struct vm *EXCEPT_VAR effective_vm;
   struct task_connections old_connections;
   struct task_connections *EXCEPT_VAR pold_connections = &old_connections;
   vm_vpage_t fault_page = VM_ADDR2PAGE((uintptr_t)fault_address);
   u16 EXCEPT_VAR old_state;
   effective_vm = fault_page >= KERNEL_BASE_PAGE ? &vm_kernel : THIS_VM;

   /* Preserve the set of active task connections while faulting. */
   task_push_connections(&old_connections);
   COMPILER_BARRIER();
   /* This right here is a long-debated change:
    *    Don't serve RPCs while fetching the faulting page.
    *    Synchronous RPC users doesn't expect LOA/ALOA to serve RPCs.
    * While it is the right thing to do this, it does pose some problems,
    * since page loading _IS_ very much capable of blocking, meaning that
    * when it does, there's no way to interrupt it while it does so...
    * However if we didn't, then all hell would break loose, considering
    * that any access to memory that hasn't been locked could lead to
    * RPCs being served, which in turn have full access to _ANY_ thread-local
    * variable, at which point stuff starts getting ~real~ weird. */
   old_state = ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FDONTSERVE);
   TRY {
#ifndef CONFIG_NO_VIO
    bool EXCEPT_VAR has_vm_lock = true;
#endif
    vm_acquire(effective_vm);
    TRY {
     struct vm_node *node;
     /* Ensure that X86 SEGFAULT flags match our VM LOADCORE mode flags. */
     STATIC_ASSERT(X86_SEGFAULT_FWRITE == VM_LOADCORE_WRITE);
     STATIC_ASSERT(X86_SEGFAULT_FUSER  == VM_LOADCORE_USER);
     vm_ok = vm_loadcore(fault_page,1,errcode & (VM_LOADCORE_WRITE|VM_LOADCORE_USER));
     if (!vm_ok && (node = vm_getnode(fault_page)) != NULL) {
      /* Due to race conditions, the access might still be supposed to be OK. */
      if ((errcode & X86_SEGFAULT_FUSER) &&
          (node->vn_prot & PROT_NOUSER)); /* User-space access, but access isn't given to the user. */
      else if ((errcode & X86_SEGFAULT_FWRITE) &&
              !(node->vn_prot & PROT_WRITE)); /* Write-access isn't allowed */
      else if ((errcode & X86_SEGFAULT_FEXEC) &&
              !(node->vn_prot & PROT_EXEC)); /* Code execution isn't allowed */
      else if (errcode & X86_SEGFAULT_FRESWRITE); /* May happen for custom mappings: Reserved paging bit was set */
#ifndef CONFIG_NO_VIO
      else if (node->vn_region->vr_type == VM_REGION_VIO &&
              (uintptr_t)fault_address != CONTEXT_IP(*xcontext)) {
       REF struct vm_region *EXCEPT_VAR vio_region;
       uintptr_t vio_addr;
       vio_addr   = node->vn_start*PAGESIZE+
                  ((uintptr_t)fault_address-
                   (uintptr_t)VM_NODE_MINADDR(node));
       vio_region = node->vn_region;
       vm_region_incref(vio_region);
       TRY {
        vm_release(effective_vm);
        has_vm_lock = false;
        /* Perform a VIO memory access. */
        TRY {
         if (X86_ANYCONTEXT_ISUSER(*xcontext)) {
#ifdef __x86_64__
          vm_ok = x86_handle_vio(xcontext,vio_region,vio_addr,fault_address);
#else
          xcontext->c_hostesp = xcontext->c_useresp;
          vm_ok = x86_handle_vio(xcontext,vio_region,vio_addr,fault_address);
          xcontext->c_useresp = xcontext->c_hostesp;
#endif
         } else {
          vm_ok = x86_handle_vio(xcontext,vio_region,vio_addr,fault_address);
         }
        } CATCH_HANDLED (E_NOT_IMPLEMENTED) {
         vm_ok = false;
        }
       } FINALLY {
        vm_region_decref(vio_region);
       }
      }
#endif
      else if (node->vn_region->vr_type == VM_REGION_RESERVED) {
       /* The region is reserved and we must ask the
        * page directory itself if access is allowed.
        * This may still be a sporadic fault if an invtlb hasn't
        * reached use yet, or if the page is being updated lazily. */
#ifdef __x86_64__
       unsigned int vec4 = X86_PDIR_E4_INDEX(fault_page);
       unsigned int vec3 = X86_PDIR_E3_INDEX(fault_page);
       unsigned int vec2 = X86_PDIR_E2_INDEX(fault_page);
       unsigned int vec1 = X86_PDIR_E1_INDEX(fault_page);
       /* Extract data from the page table. */
       u64 data = X86_PDIR_E4_IDENTITY[vec4].p_flag;
       if (data & X86_PAGE_FPRESENT) {
        data = X86_PDIR_E3_IDENTITY[vec4][vec3].p_flag;
        if ((data & X86_PAGE_FPRESENT) && !(data & X86_PAGE_F1GIB)) {
         data = X86_PDIR_E2_IDENTITY[vec4][vec3][vec2].p_flag;
         if ((data & X86_PAGE_FPRESENT) && !(data & X86_PAGE_F2MIB))
              data = X86_PDIR_E1_IDENTITY[vec4][vec3][vec2][vec1].p_flag;
        }
       }
#else
       unsigned int vec2 = X86_PDIR_VEC2INDEX_VPAGE(fault_page);
       unsigned int vec1 = X86_PDIR_VEC1INDEX_VPAGE(fault_page);
       u32 data = X86_PDIR_E2_IDENTITY[vec2].p_flag;
       if ((data & X86_PAGE_FPRESENT) && !(data & X86_PAGE_F4MIB))
            data = X86_PDIR_E1_IDENTITY[vec2][vec1].p_flag;
#endif
       if (!(data & X86_PAGE_FPRESENT)); /* Not mapped */
       else if ((errcode & X86_SEGFAULT_FUSER) &&
               !(data & X86_PAGE_FUSER)); /* Not accessible to user-space. */
       else if ((errcode & X86_SEGFAULT_FWRITE) &&
               !(data & X86_PAGE_FWRITE)); /* Not writable. */
#if 0 /* Already checked above... */
       else if (errcode & X86_SEGFAULT_FRESWRITE); /* Reserved write. */
#endif
       else {
        /* Synchronize (invalidate) the affected page. */
        debug_printf("VM memory access race condition at %p\n",fault_address);
        pagedir_sync(fault_page,1);
        vm_ok = true;
       }
      } else {
       /* Manually force a remap with the updated permissions. */
       vm_map_node(node);
       /* The access should be OK now... */
       if (pagedir_ismapped(fault_page)) {
        vm_ok = !(errcode & X86_SEGFAULT_FWRITE) ||
                  pagedir_iswritable(fault_page);
        if (vm_ok) {
         /* Synchronize the freshly remapped node (in case something else was mapped there before) */
         pagedir_sync(VM_NODE_BEGIN(node),VM_NODE_SIZE(node));
         debug_printf("VM memory access race condition at %p (errcode %p)\n",
                      fault_address,errcode);
        }
       }
      }
     }
    } FINALLY {
#ifndef CONFIG_NO_VIO
     if (has_vm_lock)
#endif
         vm_release(effective_vm);
    }
   } FINALLY {
    /* Restore task connections. */
    task_pop_connections(pold_connections);
    /* Restore the old RPC configuration. */
    if (!(old_state & TASK_STATE_FDONTSERVE))
          ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FDONTSERVE);
   }
   /* If the VM says the error is OK, return without throwing a SEGFAULT. */
   if (vm_ok)
       return;
  } CATCH_HANDLED (E_INTERRUPT) {
   /* Deal with interrupt restarts. */
   task_restart_interrupt(xcontext);
   goto again;
  }
 }


#if 1
 /* Special handling to deal with code like this:
  * >> (*(void(*)(int))17)(42);
  * ASM:
  * >>     pushl  $42
  * >>     call   17 // Fault here
  * >>     addl   $4, %esp
  * In this case, EIP will have already been broken to point to address 17.
  * However, we'd rather do exception handling at the call-site, meaning we'll
  * have to dereference the stack to try and get the proper return address.
  * Though because of how low-level all of this is, we have no guaranty that
  * memory at the call-site will actually be mapped, or that this really is
  * we ended up in a situation where the faulting address matches the return-ip.
  * For that reason, a number of additional checks are done to work out if
  * it was code like that above which caused the problem. */
 if ((void *)CONTEXT_IP(*xcontext) == fault_address &&
    !(errcode & X86_SEGFAULT_FWRITE)) {
  uintptr_t return_ip;
  if ((errcode & X86_SEGFAULT_FUSER) &&
       X86_ANYCONTEXT_ISUSER(*xcontext)) {
   uintptr_t sysno = PERTASK_GET(x86_sysbase);
   if ((uintptr_t)fault_address >= sysno &&
       (uintptr_t)fault_address <  sysno+X86_ENCODE_PFSYSCALL_SIZE) {
    sysno = (uintptr_t)fault_address-sysno;
    sysno = X86_DECODE_PFSYSCALL(sysno);
    /* User-space offers a special method of performing system calls that
     * involves jumping to a specific address offset from a random, per-thread
     * constant (to improve security by not using static addresses that would
     * allow for return-to-libc; or rather return-to-kernel; attacks). */
restart_syscall:
    xcontext->c_pip = xcontext->c_gpregs.gp_pax; /* #PF uses EAX as return address. */
    xcontext->c_gpregs.gp_pax = sysno; /* #PF encodes the sysno in EIP. */
    COMPILER_BARRIER();
    /* Execute the system calls. */
    TRY {
     x86_syscall_exec80();
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     /* Set the FSYSCALL flag so that exceptions are propagated accordingly. */
     error_info()->e_error.e_flag |= X86_INTERRUPT_GUARD_FSYSCALL;
     if (error_code() == E_INTERRUPT) {
#if 1
      COMPILER_WRITE_BARRIER();
      /* Deal with system call restarts. */
      task_restart_syscall((struct cpu_hostcontext_user *)xcontext,
                            TASK_USERCTX_TYPE_INTR_SYSCALL|
                            X86_SYSCALL_TYPE_FINT80,
                            sysno);
#else
      /* Restore the original user-space CPU xcontext. */
      xcontext->c_gpregs.gp_pax = xcontext->c_pip;
      xcontext->c_pip           = (uintptr_t)fault_address;
      COMPILER_WRITE_BARRIER();
      /* Deal with system call restarts. */
      if (!task_tryrestart_syscall(&xcontext->c_user,
                                    TASK_USERCTX_TYPE_INTR_SYSCALL|
                                    X86_SYSCALL_TYPE_FPF,
                                    sysno)) {
       xcontext->c_pip = xcontext->c_gpregs.gp_pax;
       xcontext->c_gpregs.gp_pax = sysno;
       error_rethrow();
      }
#endif
      COMPILER_BARRIER();
      goto restart_syscall;
     }
     error_rethrow();
    }
    return;
   }
  }
  /* Some CPUs don't set the FEXEC flag (QEMU?), but if the
   * fault address matches the faulting instruction, we can
   * pretty much assume that the exception was caused by some
   * sort of `jmp', or at least when the CPU was trying to
   * execute non-existent code. */
  errcode |= X86_SEGFAULT_FEXEC;

  /* NOTE: This try-block failing won't cause an infinite recursion,
   *       as any fault that could happen here wouldn't match the
   *       criteria of `CONTEXT_IP == fault_address' that is checked
   *       above. */
  TRY {
   uintptr_t **ppsp;
#ifdef __x86_64__
   ppsp = (uintptr_t **)&xcontext->c_psp;
#else
   ppsp = (uintptr_t **)&X86_ANYCONTEXT32_ESP(*xcontext);
#endif
   return_ip = **ppsp;
   /* Check if the supposed return-ip is mapped.
    * XXX: Maybe even check if it is executable? */
   if (pagedir_ismapped(VM_ADDR2PAGE(return_ip))) {
    ++*ppsp; /* Consume the addressed pushed by `call' */
    CONTEXT_IP(*xcontext) = return_ip;
   }
  } CATCH_HANDLED (E_SEGFAULT) {
  }
 }
#endif

 /* Construct and emit a SEGFAULT exception. */
 info                 = error_info();
 info->e_error.e_code = E_SEGFAULT;
 info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
 memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_segfault.sf_reason = errcode; /* The reason flags are defined to mirror the #PF error code. */
 info->e_error.e_segfault.sf_vaddr  = fault_address;
 /* Copy the CPU xcontext at the time of the exception. */
 fix_user_context(xcontext);
 memcpy(&info->e_context,xcontext,sizeof(struct cpu_context));
 /* Throw the error. */
 error_rethrow_atuser((struct cpu_context *)xcontext);
}


INTERN void FCALL
x86_handle_divide_by_zero(struct x86_anycontext *__restrict context) {
 struct exception_info *info; u64 arg = 0;
 u16 type = ERROR_DIVIDE_BY_ZERO_INT;
 TRY {
  /* Decode source instructions (differentiate between `div' and `idiv'). */
  byte_t *text = (byte_t *)context->c_pip;
  byte_t opcode = *text++;
  X86ModRm modrm;
  if (opcode == 0x66 && (*text == 0xf7)) {
   ++text;
   arg  = (u64)((u32)((u16)context->c_gpregs.gp_pdx) << 16);
   arg |= (u64)((u16)context->c_gpregs.gp_pax);
  } else if (opcode == 0xf6) {
   arg = (u64)(u16)context->c_gpregs.gp_pax;
  } else if (opcode == 0xf7) {
   arg  = (u64)((u64)context->c_gpregs.gp_edx << 32);
   arg |= (u64)(context->c_gpregs.gp_pax);
  } else {
   goto default_divide;
  }
  X86_ModRmDecode(text,&modrm,0);
  if (modrm.mi_rm == 6) {
   /* Unsigned divide */
   type = ERROR_DIVIDE_BY_ZERO_UINT;
  } else if (modrm.mi_rm == 7) {
   /* Signed divide (sign-extend the argument) */
   switch (opcode) {
   case 0x66: arg = (u64)(s64)(s8)(u8)arg; break;
   case 0xf6: arg = (u64)(s64)(s16)(u16)arg; break;
   case 0xf7: arg = (u64)(s64)(s32)(u32)arg; break;
   default: __builtin_unreachable();
   }
  } else {
   arg = 0;
  }
 } CATCH_HANDLED (E_SEGFAULT) {
 }
default_divide:
 /* Construct and emit an DIVIDE_BY_ZERO exception. */
 info                 = error_info();
 info->e_error.e_code = E_DIVIDE_BY_ZERO;
 info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
 memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_divide_by_zero.dz_type        = type;
 info->e_error.e_divide_by_zero.dz_flag        = ERROR_DIVIDE_BY_ZERO_FNORMAL;
 info->e_error.e_divide_by_zero.dz_arg.da_uint = arg;
 /* Copy the CPU context at the time of the exception. */
 fix_user_context(context);
 memcpy(&info->e_context,context,sizeof(struct cpu_context));
 /* Throw the error. */
 error_rethrow_atuser((struct cpu_context *)context);
}

INTDEF void KCALL error_print_other_thread(void);
INTDEF void KCALL error_print_vm(void);
INTERN void FCALL
x86_handle_breakpoint(struct x86_anycontext *__restrict context) {
 debug_printf("Breakpoint at %p (LASTERROR = %x)\n",
               CONTEXT_IP(*context),error_code());
#ifdef __x86_64__
 debug_printf("RAX %p  RCX %p  RDX %p  RBX %p  RIP %p\n"
              "RSP %p  RBP %p  RSI %p  RDI %p  ---\n"
              "R8  %p  R9  %p  R10 %p  R11 %p  ---\n"
              "R12 %p  R13 %p  R14 %p  R15 %p  ---\n",
              context->c_gpregs.gp_rax,context->c_gpregs.gp_rcx,
              context->c_gpregs.gp_rdx,context->c_gpregs.gp_rbx,context->c_rip,
              context->c_rsp,context->c_gpregs.gp_rbp,
              context->c_gpregs.gp_rsi,context->c_gpregs.gp_rdi,
              context->c_gpregs.gp_r8,context->c_gpregs.gp_r9,
              context->c_gpregs.gp_r10,context->c_gpregs.gp_r11,
              context->c_gpregs.gp_r12,context->c_gpregs.gp_r13,
              context->c_gpregs.gp_r14,context->c_gpregs.gp_r15);
#else
 debug_printf("EAX %p  ECX %p  EDX %p  EBX %p  EIP %p\n"
              "ESP %p  EBP %p  ESI %p  EDI %p  EFL %p\n",
              context->c_gpregs.gp_pax,context->c_gpregs.gp_ecx,
              context->c_gpregs.gp_edx,context->c_gpregs.gp_ebx,context->c_pip,
              X86_ANYCONTEXT32_ESP(*context),context->c_gpregs.gp_pbp,
              context->c_gpregs.gp_esi,context->c_gpregs.gp_edi,
              context->c_pflags);
#ifndef CONFIG_NO_X86_SEGMENTATION
 debug_printf("CS %.4IX DS %.4IX ES %.4IX FS %.4IX GS %.4IX\n",
              context->c_iret.ir_cs,
              context->c_segments.sg_ds,
              context->c_segments.sg_es,
              context->c_segments.sg_fs,
              context->c_segments.sg_gs);
#else
#define GETREG(name) XBLOCK({ register register_t v; __asm__("movl %" name ", %0" : "=r" (v)); XRETURN v; })
 debug_printf("CS %.4IX DS %.4IX ES %.4IX FS %.4IX GS %.4IX\n",
              context->c_iret.ir_cs,
              GETREG("%ds"),GETREG("%es"),
              GETREG("%fs"),GETREG("%gs"));
#undef GETREG
#endif
 debug_printf("THIS_TASK = %p (%u)\n",THIS_TASK,posix_gettid());
#endif
#if defined(__KERNEL__) && 1
 //context->c_psp = X86_ANYCONTEXT32_ESP(*INFO->e_context);
 if (X86_ANYCONTEXT_ISUSER(*context)) {
  struct userstack *stack = PERTASK_GET(_this_user_stack);
  if (stack != NULL) {
   debug_printf("stack: %p...%p\n",
          VM_PAGE2ADDR(stack->us_pagemin),
          VM_PAGE2ADDR(stack->us_pageend)-1);
   if (X86_ANYCONTEXT_USERSP(*context) >= VM_PAGE2ADDR(stack->us_pagemin) &&
       X86_ANYCONTEXT_USERSP(*context) <  VM_PAGE2ADDR(stack->us_pageend)) {
    debug_printf("%$[hex]\n",
          (VM_PAGE2ADDR(stack->us_pageend)-X86_ANYCONTEXT_USERSP(*context))+16,
           X86_ANYCONTEXT_USERSP(*context)-16);
   }
  } else {
   debug_printf("No stack\n");
  }
 } else {
  debug_printf("stack: %p...%p\n",
        (uintptr_t)PERTASK_GET(this_task.t_stackmin),
        (uintptr_t)PERTASK_GET(this_task.t_stackend)-1);
  if (X86_ANYCONTEXT_HOSTSP(*context) >= (uintptr_t)PERTASK_GET(this_task.t_stackmin) &&
      X86_ANYCONTEXT_HOSTSP(*context) <= (uintptr_t)PERTASK_GET(this_task.t_stackend)) {
   debug_printf("%$[hex]\n",
         (uintptr_t)PERTASK_GET(this_task.t_stackend)-X86_ANYCONTEXT_HOSTSP(*context),
                    X86_ANYCONTEXT_HOSTSP(*context));
  }
 }
#endif
 /* Print a traceback. */
#if 1
 {
  struct cpu_context dup;
  struct fde_info unwind_info;
  struct exception_info old_info;
  struct task_connections cons;
  u16 EXCEPT_VAR old_state;
  bool is_first = true;
  task_push_connections(&cons);
  old_state = ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FDONTSERVE);
  memcpy(&old_info,error_info(),sizeof(struct exception_info));
  TRY {
   memcpy(&dup,context,sizeof(struct cpu_context));
#ifndef __x86_64__
   dup.c_psp = X86_ANYCONTEXT32_ESP(*context);
#endif
   for (;;) {
    debug_printf("%[vinfo:%f(%l,%c) : %n : %p] : ESP %p, EBP %p\n",
                is_first ? (uintptr_t)dup.c_pip : (uintptr_t)dup.c_pip-1,
                dup.c_psp,dup.c_gpregs.gp_pbp);
    if (!is_first) --dup.c_pip;
    if (!linker_findfde(dup.c_pip,&unwind_info)) {
     debug_printf("Cannot unwind at %p (%p)\n",dup.c_pip,is_first ? dup.c_pip : dup.c_pip+1);
     break;
    }
    if (!eh_return(&unwind_info,&dup,EH_FNORMAL)) {
#if 1
     struct frame {
         struct frame *f_caller;
         void         *f_return;
     };
     struct frame *f;
     f = (struct frame *)dup.c_gpregs.gp_pbp;
     dup.c_pip           = (uintptr_t)f->f_return;
     dup.c_psp           = (uintptr_t)(f+1);
     dup.c_gpregs.gp_pbp = (uintptr_t)f->f_caller;
#else
     break;
#endif
    }
    is_first = false;
   }
   //error_print_other_thread();
   error_print_vm();
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   //error_printf("Unwind failure\n");
   error_handled();
  }
  if (!(old_state & TASK_STATE_FDONTSERVE))
        ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FDONTSERVE);
  memcpy(error_info(),&old_info,sizeof(struct exception_info));
  task_pop_connections(&cons);
 }
#endif

#if 1
 /* TODO: Invoke debugger hooks. (An interactive kernel debugger would be nice...) */
 //for (;;) __asm__("hlt");
#else
 {
  struct exception_info *info;
  /* Construct and emit a BREAKPOINT exception. */
  info                 = error_info();
  info->e_error.e_code = E_BREAKPOINT;
  info->e_error.e_flag = ERR_FRESUMABLE;
  memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));

  /* Copy the CPU context at the time of the exception. */
  fix_user_context(context);
  memcpy(&info->e_context,context,sizeof(struct cpu_context));

  /* Throw the error. */
  error_rethrow_atuser((struct cpu_context *)context,true);
 }
#endif
}


INTERN void FCALL
x86_handle_overflow(struct x86_anycontext *__restrict context) {
 struct exception_info *info;
 /* Construct and emit an OVERFLOW exception. */
 info                 = error_info();
 info->e_error.e_code = E_OVERFLOW;
 info->e_error.e_flag = ERR_FRESUMABLE;
 memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));

 /* Copy the CPU context at the time of the exception. */
 fix_user_context(context);
 memcpy(&info->e_context,context,sizeof(struct cpu_context));

 /* The overflow error can happen for 3 different reason in KOS:
  *   --> `into'                      (IP = next_instr)
  *   --> `int $4'                    (IP = next_instr)
  *   --> `div' with too small target (IP = fault_instr)
  * With that in mind, we must check which case it is, in
  * order to properly determine the RESUME-NEXT flag. */
 {
  u8 instruction_byte;
  bool is_after_faulting = false;
  TRY {
   instruction_byte = ((u8 *)CONTEXT_IP(*context))[-1];
   /* */if (instruction_byte == 0xce) is_after_faulting = true; /* `into' */
   else if (instruction_byte == 4) {
    instruction_byte = ((u8 *)CONTEXT_IP(*context))[-2];
    if (instruction_byte == 0xcd)
        is_after_faulting = true; /* `int $4' */
   }
  } CATCH_HANDLED (E_SEGFAULT) {
  }
  /* Throw the error. */
  if (!is_after_faulting)
       info->e_error.e_flag |= ERR_FRESUMENEXT;
  error_rethrow_atuser((struct cpu_context *)context);
 }
}

INTERN void FCALL
x86_handle_bound(struct x86_anycontext *__restrict context) {
 struct exception_info *info;
 /* Construct and emit an INDEX_ERROR exception. */
 info                 = error_info();
 info->e_error.e_code = E_INDEX_ERROR;
 info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
 memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 
 /* Try to retrieve information on the index that was out-of-bounds. */
 TRY {
  u8 *ptext = (u8 *)CONTEXT_IP(*context);
  X86ModRm modrm;
  uintptr_t bounds_struct;
  u16 flags = 0;
check_flag:
  switch (ptext[0]) {

   /* Prefix bytes */
  case 0x66: flags |= F_OP16; goto next_flag;
  case 0x67: flags |= F_AD16; goto next_flag;
  case 0xf0: flags |= F_LOCK; goto next_flag;
  case 0xf2: flags |= F_REPNE; goto next_flag;
  case 0xf3: flags |= F_REP; goto next_flag;
  case 0x26: flags = (flags & ~F_SEGMASK) | F_SEGES; goto next_flag;
  case 0x2e: flags = (flags & ~F_SEGMASK) | F_SEGCS; goto next_flag;
  case 0x36: flags = (flags & ~F_SEGMASK) | F_SEGSS; goto next_flag;
  case 0x3e: flags = (flags & ~F_SEGMASK) | F_SEGDS; goto next_flag;
  case 0x64: flags = (flags & ~F_SEGMASK) | F_SEGFS; goto next_flag;
  case 0x65: flags = (flags & ~F_SEGMASK) | F_SEGGS; /*goto next_flag;*/
next_flag: ++ptext; goto check_flag;

  default: break;
  }
  if unlikely(ptext[0] != 0x62)
     goto do_throw_error; /* Shouldn't happen (ensure `bound' instruction) */
  X86_ModRmDecode(ptext+1,&modrm,flags);
  if unlikely(modrm.mi_type != MODRM_MEMORY)
     goto do_throw_error; /* Shouldn't happen (ensure memory operand) */
  /* Determine the effective address of the bounds structure. */
  bounds_struct = X86_ModRmGetMem(context,&modrm,flags);
  /* Read data from the bounds structure. */
  if (flags & F_OP16) {
   u16 low,high;
   low  = ((u16 *)bounds_struct)[0];
   high = ((u16 *)bounds_struct)[1];
   COMPILER_BARRIER();
   info->e_error.e_index_error.b_boundmin = low;
   info->e_error.e_index_error.b_boundmax = high;
   info->e_error.e_index_error.b_index    = X86_GPREG16(context,modrm.mi_reg);
  } else {
   u32 low,high;
   low  = ((u32 *)bounds_struct)[0];
   high = ((u32 *)bounds_struct)[1];
   COMPILER_BARRIER();
   info->e_error.e_index_error.b_boundmin = low;
   info->e_error.e_index_error.b_boundmax = high;
   info->e_error.e_index_error.b_index    = X86_GPREG32(context,modrm.mi_reg);
  }
 } CATCH_HANDLED (E_SEGFAULT) {
 }

do_throw_error:
 /* Copy the CPU context at the time of the exception. */
 fix_user_context(context);
 memcpy(&info->e_context,context,sizeof(struct cpu_context));

 /* Throw the error. */
 error_rethrow_atuser((struct cpu_context *)context);
}



/* The default, fallback interrupt handler. */
INTERN void KCALL
x86_interrupt_handler(struct cpu_anycontext *__restrict context,
                      register_t intno, register_t errcode) {
 struct exception_info *info;
 assertf(intno <= 0xff,"intno = %p",intno);
 /* Re-enable interrupts if they were enabled before. */
 if (context->c_pflags & EFLAGS_IF)
     x86_interrupt_enable();

 /* Throw an unhandled-interrupt error. */
 info                 = error_info();
 info->e_error.e_code = E_UNHANDLED_INTERRUPT;
 info->e_error.e_flag = ERR_FRESUMABLE;
 memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_unhandled_interrupt.ui_intcode = intno;
 info->e_error.e_unhandled_interrupt.ui_errcode = errcode;

 /* Copy the CPU context at the time of the exception. */
 fix_user_context(context);
 memcpy(&info->e_context,context,sizeof(struct cpu_context));
 error_rethrow_atuser((struct cpu_context *)context);
}

DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_HW_EXCEPT_C */
