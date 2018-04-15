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
#ifndef GUARD_KERNEL_SRC_I386_KOS_RTL_C
#define GUARD_KERNEL_SRC_I386_KOS_RTL_C 1
#define _KOS_SOURCE 1

#include "scheduler.h"

#include <hybrid/compiler.h>
#include <kernel/debug.h>
#include <kernel/interrupt.h>
#include <kernel/vm.h>
#include <unwind/eh_frame.h>
#include <sched/userstack.h>
#include <unwind/linker.h>
#include <kos/context.h>
#include <sched/task.h>
#include <sched/pid.h>
#include <except.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

/* Basic RTL support. */

DECL_BEGIN

INTDEF byte_t kernel_ehframe_start[];
INTDEF byte_t kernel_ehframe_end[];
INTDEF byte_t kernel_ehframe_size[];

INTDEF struct except_handler kernel_except_start[];
INTDEF struct except_handler kernel_except_end[];

INTERN struct except_handler *LIBCCALL
error_findhnd(uintptr_t ip) {
 struct except_handler *iter;
 /* Same as `linker_findfde()' - Eventually, we must search through modules here. */
 for (iter = kernel_except_start;
      iter < kernel_except_end; ++iter) {
  if (ip <  (uintptr_t)iter->eh_begin) continue;
  if (ip >= (uintptr_t)iter->eh_end) continue;
  if (iter->eh_flag&EXCEPTION_HANDLER_FHASMASK) {
   /* Check for the error code mask to match. */
   if (iter->eh_mask != error_code())
       continue;
  }
  if (iter->eh_entry >= (uintptr_t)iter->eh_begin &&
      iter->eh_entry <  (uintptr_t)iter->eh_end)
      debug_printf("WARNING: Overlapping handler %p in %p...%p\n",
                   iter->eh_entry,iter->eh_begin,iter->eh_end);
  return iter;
 }
 return NULL;
}

INTERN void KCALL print_tb(struct cpu_context *__restrict context) {
 struct cpu_context ctx = *context;
 bool is_first = true;
 uintptr_t last_eip = 0;
 for (;;) {
  struct fde_info finfo;
  if (last_eip != ctx.c_eip) {
   debug_printf("%[vinfo:%f(%l,%c) : %n] : %p : ESP %p, EBP %p\n",
                is_first ? ctx.c_eip : ctx.c_eip-1,
                ctx.c_eip,ctx.c_esp,ctx.c_gpregs.gp_ebp);
  }
  last_eip = ctx.c_eip;
  if (!linker_findfde_consafe(is_first ? ctx.c_eip : ctx.c_eip-1,&finfo)) {
#if 0
   /* For the first entry, assume a standard unwind which can be used
    * to properly display tracebacks when execution tries to call a
    * NULL-function pointer. */
   if (!is_first) break;
   if (info->e_context.c_esp >= (uintptr_t)PERTASK_GET(this_task.t_stackend)) {
    debug_printf("IP is out-of-bounds (%p not in %p...%p)\n",
                 info->e_context.c_esp,
                (uintptr_t)PERTASK_GET(this_task.t_stackmin),
                (uintptr_t)PERTASK_GET(this_task.t_stackend)-1);
    break;
   }
   info->e_context.c_eip = *(u32 *)info->e_context.c_esp;
   info->e_context.c_esp += 4;
#else
   break;
#endif
  } else {
   if (!eh_return(&finfo,&ctx,EH_FNORMAL)) {
#if 1
    struct frame {
        struct frame *f_caller;
        void         *f_return;
    };
    struct frame *f;
    f = (struct frame *)ctx.c_gpregs.gp_ebp;
    ctx.c_eip           = (uintptr_t)f->f_return;
    ctx.c_esp           = (uintptr_t)(f+1);
    ctx.c_gpregs.gp_ebp = (uintptr_t)f->f_caller;
#else
    break;
#endif
   }
  }
  is_first = false;
 }
}

INTDEF void KCALL kernel_print_exception(void);
INTERN void LIBCCALL
libc_error_vprintf(char const *__restrict reason, va_list args) {
 debug_vprintf(reason,args);
 kernel_print_exception();
}
INTERN void ATTR_CDECL
libc_error_printf(char const *__restrict reason, ...) {
 va_list args;
 va_start(args,reason);
 libc_error_vprintf(reason,args);
 va_end(args);
}
DEFINE_PUBLIC_ALIAS(error_vprintf,libc_error_vprintf);
DEFINE_PUBLIC_ALIAS(error_printf,libc_error_printf);


INTDEF ATTR_PERTASK
struct task_connections my_connections;

INTERN void KCALL error_print_other_thread(void) {
#ifndef CONFIG_VM_USE_RWLOCK
 debug_printf("VM_HOLDER: %p(%p) x %Iu\n",
              THIS_VM->vm_lock.m_owner,
              THIS_VM->vm_lock.m_owner ? THIS_VM->vm_lock.m_owner->t_refcnt : 0,
              THIS_VM->vm_lock.m_ind);
 debug_printf("VM_KERNEL_HOLDER: %p(%p) x %Iu\n",
              vm_kernel.vm_lock.m_owner,
              vm_kernel.vm_lock.m_owner ? vm_kernel.vm_lock.m_owner->t_refcnt : 0,
              vm_kernel.vm_lock.m_ind);
#endif /* !CONFIG_VM_USE_RWLOCK */
 {
  struct task *iter = THIS_TASK->t_sched.sched_ring.re_next;
  for (; iter != THIS_TASK; iter = iter->t_sched.sched_ring.re_next) {
   debug_printf("RUNNING %p: (CTX %p; PID %p; SIGNAL %p:%Iu)\n",
                iter,iter->t_context,
                FORTASK(iter,_this_pid) ? FORTASK(iter,_this_pid)->tp_pids[0] : 0,
                FORTASK(iter,my_connections).tcs_sig,
                FORTASK(iter,my_connections).tcs_cnt);
   TRY {
    print_tb((struct cpu_context *)iter->t_context);
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   }
  }
 }
 if (THIS_TASK != &_boot_task) {
  debug_printf("BOOT_THREAD %p: (CTX %p)\n",&_boot_task,_boot_task.t_context);
  TRY {
   print_tb((struct cpu_context *)_boot_task.t_context);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  }
 }
 {
  struct task *iter = THIS_CPU->c_sleeping;
  for (; iter; iter = iter->t_sched.sched_list.le_next) {
   debug_printf("SLEEPER %p: (CTX %p; PID %p; SIGNAL %p:%Iu)\n",
                iter,iter->t_context,
                FORTASK(iter,_this_pid) ? FORTASK(iter,_this_pid)->tp_pids[0] : 0,
                FORTASK(iter,my_connections).tcs_sig,
                FORTASK(iter,my_connections).tcs_cnt);
   TRY {
    print_tb((struct cpu_context *)iter->t_context);
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   }
  }
 }
}

INTERN void KCALL error_print_vm(void) {
 struct vm_node *node;
 VM_FOREACH_NODE(node,THIS_VM) {
  debug_printf("%p...%p %c%c%c%c%c\n",
               VM_NODE_MINADDR(node),
               VM_NODE_MAXADDR(node),
               node->vn_prot & PROT_EXEC ? 'x' : '-',
               node->vn_prot & PROT_WRITE ? 'w' : '-',
               node->vn_prot & PROT_READ ? 'r' : '-',
               node->vn_prot & PROT_SHARED ? 's' : '-',
               node->vn_prot & PROT_NOUSER ? 'K' : 'u');
 }
}

INTDEF void KCALL take_down_process(unsigned int status);
INTERN ATTR_NORETURN void FCALL
x86_default_error_unhandled_exception(bool is_standalone) {
 struct exception_info *info;
 except_t code;
 code = error_code();
 /* The intended use of `E_EXIT_THREAD' and `E_EXIT_PROCESS'
  * is to either return to user-space (which we didn't if we got here),
  * or to terminate the thread after unwinding the stack. */
 if (code == E_EXIT_THREAD ||
     code == E_EXIT_PROCESS)
     goto done;
 debug_printf("\n\n\n");
 kernel_print_exception();
 error_print_other_thread();
 error_print_vm();

done:
 /* Terminate the task with the unhandled exception. */
 info = error_info();
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_code          = E_EXIT_THREAD;
 info->e_error.e_flag          = ERR_FNORMAL;
 info->e_error.e_exit.e_status = __W_EXITCODE(code,0);
 /*  */
 if (!is_standalone)
      take_down_process(info->e_error.e_exit.e_status);
 task_exit();
}

PUBLIC ATTR_NORETURN void FCALL
error_unhandled_exception(void) {
 x86_default_error_unhandled_exception(true);
}

PRIVATE ATTR_NOINLINE ATTR_NORETURN void KCALL
core_assertion_failure(char const *expr, DEBUGINFO,
                       char const *format, va_list args) {
 struct cpu_context context;
 unsigned int frame_id = 0;
 PREEMPTION_DISABLE();
 debug_printf("\n\n\n"
              "%s(%d) : %s : Assertion failed : %q\n",
              __file,__line,__func,expr);
 if (format) debug_vprintf(format,args),
             debug_printf("\n");
 cpu_getcontext(&context);
 task_disconnect();
 THIS_TASK->t_sched.sched_ring.re_prev = THIS_TASK;
 THIS_TASK->t_sched.sched_ring.re_next = THIS_TASK;
#ifdef CONFIG_VM_USE_RWLOCK
 vm_kernel.vm_lock.rw_state = 0;
 THIS_VM->vm_lock.rw_state  = 0;
#else
 vm_kernel.vm_lock.m_ind = 0;
 THIS_VM->vm_lock.m_ind  = 0;
#endif
 COMPILER_WRITE_BARRIER();
 PREEMPTION_ENABLE();
 for (;; ++frame_id) {
  struct fde_info finfo;
  if (frame_id >= 2) {
   debug_printf("%[vinfo:%f(%l,%c) : %n] : %p : ESP %p, EBP %p\n",
                context.c_eip-1,context.c_eip,
                context.c_esp,context.c_gpregs.gp_ebp);
  }
  if (!linker_findfde(context.c_eip-1,&finfo)) break;
  if (!eh_return(&finfo,&context,EH_FNORMAL)) break;
 }
 error_print_other_thread();
 error_print_vm();
 for (;;) __asm__("hlt");
}

INTERN uintptr_t __stack_chk_guard = 0x1246Ab1f;
INTERN ATTR_NORETURN void __stack_chk_fail(void) {
 error_throw(E_STACK_OVERFLOW);
}


PUBLIC ATTR_NORETURN ATTR_COLD
void (__afailf)(char const *expr, DEBUGINFO, char const *format, ...) {
 va_list args;
 va_start(args,format);
 core_assertion_failure(expr,DEBUGINFO_FWD,format,args);
}

PUBLIC ATTR_NORETURN ATTR_COLD
void (__LIBCCALL __afail)(char const *expr, DEBUGINFO) {
 va_list args;
 memset(&args,0,sizeof(va_list));
 core_assertion_failure(expr,DEBUGINFO_FWD,NULL,args);
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_RTL_C */
