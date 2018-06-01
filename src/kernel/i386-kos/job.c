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
#ifndef GUARD_KERNEL_I386_KOS_JOB_C
#define GUARD_KERNEL_I386_KOS_JOB_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kernel/syscall.h>
#include <sched/task.h>
#include <sched/pid.h>
#include <sched/group.h>
#include <kos/intrin.h>
#include <kernel/debug.h>
#include <kos/registers.h>
#include <i386-kos/gdt.h>
#include <asm/cpu-flags.h>
#include <except.h>
#include <string.h>

#include <kos/types.h>
#include <kos/rpc.h>

DECL_BEGIN


INTDEF ATTR_NORETURN void (KCALL throw_invalid_segment)(u16 segment_index, u16 segment_register);
#define throw_invalid_segment(segment_index,segment_register) \
       __EXCEPT_INVOKE_THROW_NORETURN(throw_invalid_segment(segment_index,segment_register))

INTDEF u8 KCALL restart_syscall_mode(syscall_ulong_t sysno);


struct urpc_data {
    unsigned int           ud_mode;    /* The `mode' argument passed to the `xqueue_job' system call. */
    struct x86_usercontext ud_context; /* The user-space context to load within the target thread. */
};

PRIVATE void KCALL urpc_validate(struct urpc_data *__restrict data) {
 /* Validate segment registers. */
 if (data->ud_mode & X86_JOB_FLOAD_SEGMENTS) {
#ifndef CONFIG_X86_FIXED_SEGMENTATION
#ifdef __x86_64__
  if (!data->ud_context.c_cs)
       data->ud_context.c_cs = interrupt_iscompat() ? X86_SEG_USER_CS32 : X86_SEG_USER_CS;
  if (!data->ud_context.c_ss)
       data->ud_context.c_ss = interrupt_iscompat() ? X86_SEG_USER_SS32 : X86_SEG_USER_SS;
#else
  if (!data->ud_context.c_cs)
       data->ud_context.c_cs = X86_SEG_USER_CS;
  if (!data->ud_context.c_ss)
       data->ud_context.c_ss = X86_SEG_USER_SS;
  if (!__verw(data->ud_context.c_ss))
       throw_invalid_segment(data->ud_context.c_ss,X86_REGISTER_SEGMENT_SS);
  if ((data->ud_context.c_cs & 3) != 3 || !__verw(data->ud_context.c_cs))
       throw_invalid_segment(data->ud_context.c_cs,X86_REGISTER_SEGMENT_CS);
#endif
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
#ifndef __x86_64__
  if (!data->ud_context.c_segments.sg_gs)
       data->ud_context.c_segments.sg_gs = X86_SEG_USER_GS;
  if (!data->ud_context.c_segments.sg_fs)
       data->ud_context.c_segments.sg_fs = X86_SEG_USER_FS;
#ifndef CONFIG_X86_FIXED_SEGMENTATION
  if (!data->ud_context.c_segments.sg_es)
       data->ud_context.c_segments.sg_es = X86_SEG_USER_ES;
  if (!data->ud_context.c_segments.sg_ds)
       data->ud_context.c_segments.sg_ds = X86_SEG_USER_DS;
  /* Verify user-space segment indices. */
  if (!__verw(data->ud_context.c_segments.sg_ds))
       throw_invalid_segment(data->ud_context.c_segments.sg_ds,X86_REGISTER_SEGMENT_DS);
  if (!__verw(data->ud_context.c_segments.sg_es))
       throw_invalid_segment(data->ud_context.c_segments.sg_es,X86_REGISTER_SEGMENT_ES);
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
  if (!__verw(data->ud_context.c_segments.sg_fs))
       throw_invalid_segment(data->ud_context.c_segments.sg_fs,X86_REGISTER_SEGMENT_FS);
  if (!__verw(data->ud_context.c_segments.sg_gs))
       throw_invalid_segment(data->ud_context.c_segments.sg_gs,X86_REGISTER_SEGMENT_GS);
#endif /* !__x86_64__ */
 }
 if (data->ud_mode & X86_JOB_FLOAD_STACK)
     validate_writable((byte_t *)CPU_CONTEXT_SP(data->ud_context),sizeof(uintptr_t));
}

/* Modify the user-space context to execute the job. */
PRIVATE void KCALL
urpc_callback(struct urpc_data *__restrict data,
              struct cpu_hostcontext_user *__restrict context,
              unsigned int mode) {
 unsigned int reason;
 unsigned int urpc_mode;
 byte_t *target_stack;

 /* Figure out the the reason for the RPC. */
 reason = RPC_REASON_ASYNC;
 switch (TASK_USERCTX_TYPE(mode)) {

 {
  syscall_ulong_t sysno;
  u8 restart_mode;
 case TASK_USERCTX_TYPE_INTR_SYSCALL:
  reason |= RPC_REASON_SYSCALL;
  if (!(mode & TASK_USERCTX_FTIMER))
        reason |= RPC_REASON_DIDBLOCK; /* Without a timer, we must have been blocking, or at least serving. */
  if (mode & X86_SYSCALL_TYPE_FPF) {
   sysno = context->c_pip - PERTASK_GET(x86_sysbase);
   sysno = X86_DECODE_PFSYSCALL(sysno);
   reason |= X86_RPC_REASON_FPF;
  }
#ifndef CONFIG_NO_X86_SYSENTER
  else if (mode & X86_SYSCALL_TYPE_FSYSENTER) {
   sysno = context->c_gpregs.gp_pax;
   reason |= X86_RPC_REASON_FSYSENTER;
  }
#endif /* !CONFIG_NO_X86_SYSENTER */
  else {
   sysno = context->c_gpregs.gp_pax;
   reason |= X86_RPC_REASON_FINT80;
  }
  if (!(sysno & 0x80000000))
        reason |= RPC_REASON_NOEXCEPT;
  /* Figure out the intended system call restart behavior. */
  restart_mode = restart_syscall_mode(sysno);
  switch (restart_mode) {
  case X86_SYSCALL_RESTART_FMUST:
   reason |= RPC_REASON_FMUST_RESTART;
   ATTR_FALLTHROUGH
  case X86_SYSCALL_RESTART_FAUTO:
   reason |= RPC_REASON_FSHOULD_RESTART;
   break;
  default: break;
  }
 } break;

 /*case TASK_USERCTX_TYPE_INTR_INTERRUPT:*/
 /*case TASK_USERCTX_TYPE_WITHINUSERCODE:*/
 default:
  break;
 }

 urpc_mode = data->ud_mode;

 /* Figure out where information is saved. */
 target_stack = (byte_t *)context->c_psp;
 if (urpc_mode & X86_JOB_FLOAD_STACK)
     target_stack = (byte_t *)data->ud_context.c_psp;
 else {
  validate_writable(target_stack,sizeof(uintptr_t));
 }
#define PUSH(x) (target_stack -= sizeof(uintptr_t),*(uintptr_t *)target_stack = (x))

 /* Construct the job frame, pushing only requested registers.
  * The user-space structure detailing this order is `struct x86_job_frame' */
#ifdef __x86_64__
 if (urpc_mode & X86_JOB_FSAVE_SEGMENTS)
     PUSH(context->c_iret.ir_ss);
 if (urpc_mode & X86_JOB_FSAVE_STACK)
     PUSH(context->c_rsp);
 if (urpc_mode & X86_JOB_FSAVE_CREGS)
     PUSH(context->c_rflags);
 if (urpc_mode & X86_JOB_FSAVE_SEGMENTS)
     PUSH(context->c_iret.ir_cs);
 PUSH(context->c_iret.ir_rip);
 if (urpc_mode & X86_JOB_FSAVE_SEGMENTS) {
  PUSH(RD_USER_FSBASE());
  PUSH(RD_USER_GSBASE());
 }
 if (urpc_mode & X86_JOB_FSAVE_CREGS) {
  PUSH(context->c_gpregs.gp_rax);
  PUSH(context->c_gpregs.gp_rcx);
  PUSH(context->c_gpregs.gp_rdx);
 }
 if (urpc_mode & X86_JOB_FSAVE_PREGS) {
  PUSH(context->c_gpregs.gp_rbx);
  PUSH(context->c_gpregs.gp_rbp);
 }
 if (urpc_mode & X86_JOB_FSAVE_CREGS) {
  PUSH(context->c_gpregs.gp_rsi);
  PUSH(context->c_gpregs.gp_rdi);
  PUSH(context->c_gpregs.gp_r8);
  PUSH(context->c_gpregs.gp_r9);
  PUSH(context->c_gpregs.gp_r10);
  PUSH(context->c_gpregs.gp_r11);
 }
 if (urpc_mode & X86_JOB_FSAVE_PREGS) {
  PUSH(context->c_gpregs.gp_r12);
  PUSH(context->c_gpregs.gp_r13);
  PUSH(context->c_gpregs.gp_r14);
  PUSH(context->c_gpregs.gp_r15);
 }
#else
 if (urpc_mode & X86_JOB_FSAVE_SEGMENTS)
     PUSH(context->c_iret.ir_ss),
     PUSH(context->c_iret.ir_cs);
 if (urpc_mode & X86_JOB_FSAVE_CREGS)
     PUSH(context->c_iret.ir_eflags);
 PUSH(context->c_iret.ir_eip);
 if (urpc_mode & X86_JOB_FSAVE_SEGMENTS) {
#ifndef CONFIG_X86_FIXED_SEGMENTATION
  PUSH(context->c_segments.sg_ds);
  PUSH(context->c_segments.sg_es);
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
  PUSH(context->c_segments.sg_fs);
  PUSH(context->c_segments.sg_gs);
 }
 if (urpc_mode & X86_JOB_FSAVE_CREGS) {
  PUSH(context->c_gpregs.gp_eax);
  PUSH(context->c_gpregs.gp_ecx);
  PUSH(context->c_gpregs.gp_edx);
 }
 if (urpc_mode & X86_JOB_FSAVE_STACK)
     PUSH(context->c_esp);
 if (urpc_mode & X86_JOB_FSAVE_PREGS) {
  PUSH(context->c_gpregs.gp_ebx);
  PUSH(context->c_gpregs.gp_esp);
  PUSH(context->c_gpregs.gp_ebp);
  PUSH(context->c_gpregs.gp_esi);
  PUSH(context->c_gpregs.gp_edi);
 }
#endif
 /* Finally, push the RPC reason */
 PUSH(reason);
#undef PUSH

 /* Save the updated user-space ESP */
 context->c_psp = (uintptr_t)target_stack;

 /* Now load those portions of the user-context which we should overwrite. */
#ifdef __x86_64__
 context->c_iret.ir_rip = data->ud_context.c_rip;
 if (urpc_mode & X86_JOB_FLOAD_SEGMENTS) {
  WR_USER_FSBASE(data->ud_context.c_segments.sg_fsbase);
  WR_USER_GSBASE(data->ud_context.c_segments.sg_gsbase);
 }
 if (urpc_mode & X86_JOB_FLOAD_CREGS) {
  context->c_iret.ir_rflags = data->ud_context.c_rflags;
  context->c_gpregs.gp_rax  = data->ud_context.c_gpregs.gp_rax;
  context->c_gpregs.gp_rcx  = data->ud_context.c_gpregs.gp_rcx;
  context->c_gpregs.gp_rdx  = data->ud_context.c_gpregs.gp_rdx;
  context->c_gpregs.gp_rsi  = data->ud_context.c_gpregs.gp_rsi;
  context->c_gpregs.gp_rdi  = data->ud_context.c_gpregs.gp_rdi;
  context->c_gpregs.gp_r8   = data->ud_context.c_gpregs.gp_r8;
  context->c_gpregs.gp_r9   = data->ud_context.c_gpregs.gp_r9;
  context->c_gpregs.gp_r10  = data->ud_context.c_gpregs.gp_r10;
  context->c_gpregs.gp_r11  = data->ud_context.c_gpregs.gp_r11;
  context->c_iret.ir_rflags |=  (EFLAGS_IF);
  context->c_iret.ir_rflags &= ~(EFLAGS_TF|EFLAGS_IOPL(3)|
                                 EFLAGS_NT|EFLAGS_RF|EFLAGS_VM|
                                 EFLAGS_AC|EFLAGS_VIF|EFLAGS_VIP|
                                 EFLAGS_ID);
 }
 if (urpc_mode & X86_JOB_FLOAD_PREGS) {
  context->c_gpregs.gp_rbp = data->ud_context.c_gpregs.gp_rbp;
  context->c_gpregs.gp_rbx = data->ud_context.c_gpregs.gp_rbx;
  context->c_gpregs.gp_r12 = data->ud_context.c_gpregs.gp_r12;
  context->c_gpregs.gp_r13 = data->ud_context.c_gpregs.gp_r13;
  context->c_gpregs.gp_r14 = data->ud_context.c_gpregs.gp_r14;
  context->c_gpregs.gp_r15 = data->ud_context.c_gpregs.gp_r15;
 }
#else
 context->c_iret.ir_eip = data->ud_context.c_eip;
 if (urpc_mode & X86_JOB_FLOAD_SEGMENTS) {
#ifndef CONFIG_X86_FIXED_SEGMENTATION
  context->c_iret.ir_cs     = data->ud_context.c_cs;
  context->c_iret.ir_ss     = data->ud_context.c_ss;
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
  context->c_segments.sg_gs = data->ud_context.c_segments.sg_gs;
  context->c_segments.sg_fs = data->ud_context.c_segments.sg_fs;
#ifndef CONFIG_X86_FIXED_SEGMENTATION
  context->c_segments.sg_es = data->ud_context.c_segments.sg_es;
  context->c_segments.sg_ds = data->ud_context.c_segments.sg_ds;
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
 }
 if (urpc_mode & X86_JOB_FLOAD_CREGS) {
  context->c_iret.ir_eflags = data->ud_context.c_eflags;
  context->c_gpregs.gp_eax  = data->ud_context.c_gpregs.gp_eax;
  context->c_gpregs.gp_ecx  = data->ud_context.c_gpregs.gp_ecx;
  context->c_gpregs.gp_edx  = data->ud_context.c_gpregs.gp_edx;
  context->c_iret.ir_eflags |=  (EFLAGS_IF);
  context->c_iret.ir_eflags &= ~(EFLAGS_TF|EFLAGS_IOPL(3)|
                                 EFLAGS_NT|EFLAGS_RF|EFLAGS_VM|
                                 EFLAGS_AC|EFLAGS_VIF|EFLAGS_VIP|
                                 EFLAGS_ID);
 }
 if (urpc_mode & X86_JOB_FLOAD_PREGS) {
  context->c_gpregs.gp_edi = data->ud_context.c_gpregs.gp_edi;
  context->c_gpregs.gp_esi = data->ud_context.c_gpregs.gp_esi;
  context->c_gpregs.gp_ebp = data->ud_context.c_gpregs.gp_ebp;
  context->c_gpregs.gp_ebx = data->ud_context.c_gpregs.gp_ebx;
 }
#endif
 /* All right! everything saved properly, and all registers have been updated.
  * All we need to do now, is return back to user-space! */
 error_info()->e_error.e_code = E_USER_RESUME;
}


PRIVATE void KCALL
urpc_callback_inherit(struct urpc_data *__restrict data,
                      struct cpu_hostcontext_user *__restrict context,
                      unsigned int mode) {
 TRY {
  urpc_callback(data,context,mode);
 } FINALLY {
  kfree(data);
 }
}



DEFINE_SYSCALL3(xqueue_job,pid_t,upid,
                USER UNCHECKED struct x86_usercontext const *,job,
                unsigned int,mode) {
 REF struct thread_pid *thread;
 REF struct task *EXCEPT_VAR target;
 bool COMPILER_IGNORE_UNINITIALIZED(result);
 validate_readable(job,sizeof(struct x86_usercontext));
 if (mode & ~(JOB_FSYNCHRONOUS|JOB_FASYNCHRONOUS|JOB_FWAITACK|
              X86_JOB_FSAVE_RETURN|X86_JOB_FLOAD_RETURN|
              X86_JOB_FSAVE_STACK|X86_JOB_FLOAD_STACK|
              X86_JOB_FSAVE_SEGMENTS|X86_JOB_FLOAD_SEGMENTS|
              X86_JOB_FSAVE_CREGS|X86_JOB_FLOAD_CREGS|
              X86_JOB_FSAVE_PREGS|X86_JOB_FLOAD_PREGS))
     error_throw(E_INVALID_ARGUMENT);
 thread = pid_lookup(upid);
 target = thread_pid_get(thread);
 thread_pid_decref(thread);
 if (!target) return 0; /* Target thread has died. */
 TRY {
  unsigned int rpc_mode;
  rpc_mode = TASK_RPC_USER;
  if (mode & JOB_FASYNCHRONOUS)
      rpc_mode |= TASK_RPC_UASYNC;
  else {
      rpc_mode |= TASK_RPC_USYNC;
  }
  if unlikely(target == THIS_TASK) {
   struct urpc_data *EXCEPT_VAR data;
   /* Special case: Send an RPC to ourself. */
   data = (struct urpc_data *)kmalloc(sizeof(struct urpc_data),GFP_SHARED);
   TRY {
    data->ud_mode = mode;
    memcpy(&data->ud_context,job,sizeof(struct x86_usercontext));
    COMPILER_READ_BARRIER();
    urpc_validate(data);
    COMPILER_BARRIER();
    /* Upon success, `task_queue_rpc_user()' will have inherited `data' */
    task_queue_rpc_user(target,(task_user_rpc_t)&urpc_callback_inherit,
                        data,rpc_mode);
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    kfree(data);
    error_rethrow();
   }
   /* `task_queue_rpc_user()' will have thrown an exception to get
    * back to user-space so it can serve our RPC wrapper function. */
   __builtin_unreachable();
  } else if (mode & JOB_FWAITACK) {
   struct urpc_data data;
   /* We must wait for the job to be acknowledged,
    * so we can pass context data via our own stack! */
   data.ud_mode = mode;
   memcpy(&data.ud_context,job,sizeof(struct x86_usercontext));
   COMPILER_READ_BARRIER();
   urpc_validate(&data);
   COMPILER_BARRIER();
   result = task_queue_rpc_user(target,(task_user_rpc_t)&urpc_callback,
                               &data,rpc_mode|TASK_RPC_SYNC);
  } else {
   struct urpc_data *EXCEPT_VAR data;
   /* Special case: Send an RPC to ourself. */
   data = (struct urpc_data *)kmalloc(sizeof(struct urpc_data),GFP_SHARED);
   TRY {
    data->ud_mode = mode;
    memcpy(&data->ud_context,job,sizeof(struct x86_usercontext));
    COMPILER_READ_BARRIER();
    urpc_validate(data);
    COMPILER_BARRIER();
    /* Upon success, `task_queue_rpc_user()' will have inherited `data' */
    result = task_queue_rpc_user(target,(task_user_rpc_t)&urpc_callback_inherit,
                                 data,rpc_mode);
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    kfree(data);
    error_rethrow();
   }
  }
 } FINALLY {
  task_decref(target);
 }
 return result;
}


DEFINE_SYSCALL0(xserve_job) {
 /* Just serve RPC functions... */
 return task_serve();
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_JOB_C */
