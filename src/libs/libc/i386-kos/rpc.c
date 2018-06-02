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
#ifndef GUARD_LIBS_LIBC_I386_KOS_RPC_C
#define GUARD_LIBS_LIBC_I386_KOS_RPC_C 1
#define __EXPOSE_CPU_CONTEXT 1

#include "../rpc.h"
#include "../futex.h"
#include <hybrid/atomic.h>
#include <stdint.h>
#include <kos/rpc.h>
#include <kos/futex.h>

DECL_BEGIN

struct fast_rpc_regs {
#ifdef __x86_64__
    u64 mode;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rdi;
    u64 rsi;
    u64 rdx;
    u64 rcx;
    u64 rax;
    u64 rip;
    u64 rflags;
#else
    u32 mode;
    u32 edx;
    u32 ecx;
    u32 eax;
    u32 eip;
    u32 eflags;
#endif
};

CRT_KOS unsigned int FCALL
libc_invoke_rpc_fast(rpc_t func, void *arg,
                     struct fast_rpc_regs *__restrict regs) {
 unsigned int COMPILER_IGNORE_UNINITIALIZED(result);
 struct fast_rpc_regs *EXCEPT_VAR xregs;
 /* Only a noexcept system call needs a guard. */
 if (!(regs->mode & RPC_REASON_NOEXCEPT))
       return (*func)(arg);
 xregs = regs;
 LIBC_TRY {
  result = (*func)(arg);
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
#ifdef __x86_64__
  xregs->rax = (uintptr_t)-libc_except_errno();
#else
  xregs->eax = (uintptr_t)-libc_except_errno();
#endif
  error_handled();
  result = RPC_RETURN_RESUME;
 }
 return result;
}

INTDEF void ASMCALL x86_rpc_entry_fast(void);


struct fast_waitfor_rpc_data {
    rpc_t   func;
    void   *arg;
    futex_t pending;  /* Set to ZERO and broadcast when the RPC has finished. */
};

CRT_KOS unsigned int LIBCCALL
fast_waitfor_rpc_wrapper(struct fast_waitfor_rpc_data *__restrict data) {
 unsigned int COMPILER_IGNORE_UNINITIALIZED(result);
 struct fast_waitfor_rpc_data *EXCEPT_VAR xdata = data;
 TRY {
  result = (*data->func)(data->arg);
 } FINALLY {
  ATOMIC_WRITE(xdata->pending,0);
  libc_futex_wake(&xdata->pending,SIZE_MAX);
 }
 return result;
}



EXPORT(Xqueue_rpc,libc_Xqueue_rpc);
CRT_KOS bool LIBCCALL
libc_Xqueue_rpc(pid_t pid, rpc_t func, void *arg,
                unsigned int mode) {
 struct cpu_context context;
 bool result;
 if (mode & ~(RPC_FSYNCHRONOUS|RPC_FASYNCHRONOUS|RPC_FWAITACK|RPC_FWAITFOR))
     error_throw(E_INVALID_ARGUMENT);
 context.c_eflags        = 0;
 if (mode & RPC_FWAITFOR) {
  struct fast_waitfor_rpc_data data;
  data.func               = func;
  data.arg                = arg;
  data.pending            = 1;
#ifdef __x86_64__
  context.c_rip           = (uintptr_t)&x86_rpc_entry_fast;
  context.c_gpregs.gp_rdi = (uintptr_t)&fast_waitfor_rpc_wrapper;
  context.c_gpregs.gp_rsi = (uintptr_t)&data;
#else
  context.c_eip           = (uintptr_t)&x86_rpc_entry_fast;
  context.c_gpregs.gp_ecx = (uintptr_t)&fast_waitfor_rpc_wrapper;
  context.c_gpregs.gp_edx = (uintptr_t)&data;
#endif
  result = libc_queue_job(pid,&context,
                         (mode & ~RPC_FWAITFOR)|
                          JOB_FWAITACK|
                          X86_JOB_FSAVE_RETURN|
                          X86_JOB_FLOAD_RETURN|
                          X86_JOB_FSAVE_CREGS|
                          X86_JOB_FLOAD_CREGS);
  if (result) {
   struct pollfutex poll_ftx[1];
   struct pollpid   poll_pid[1];
   /* Wait for the RPC to finish.
    * NOTE: Since the thread might terminate before it can
    *       do so, we must also wait for that to happen. */
   pollfutex_init_wait(&poll_ftx[0],&data.pending,1);
   pollpid_init(&poll_pid[0],P_PID,pid,WEXITED,NULL,NULL);
   libc_Xxppoll64(NULL,0,poll_ftx,1,poll_pid,1,NULL,NULL);
   /* Check if the RPC-pending field has been cleared. */
   result = ATOMIC_READ(data.pending) == 0;
  }
 } else {
#ifdef __x86_64__
  context.c_rip           = (uintptr_t)&x86_rpc_entry_fast;
  context.c_gpregs.gp_rdi = (uintptr_t)func;
  context.c_gpregs.gp_rsi = (uintptr_t)arg;
#else
  context.c_eip           = (uintptr_t)&x86_rpc_entry_fast;
  context.c_gpregs.gp_ecx = (uintptr_t)func;
  context.c_gpregs.gp_edx = (uintptr_t)arg;
#endif
  /* Invoke the RPC */
  result = libc_queue_job(pid,&context,mode|
                          X86_JOB_FSAVE_RETURN|
                          X86_JOB_FLOAD_RETURN|
                          X86_JOB_FSAVE_CREGS|
                          X86_JOB_FLOAD_CREGS);
 }
 return result;
}




struct full_rpc_regs {
    uintptr_t          mode;
    struct cpu_context ctx;
};

CRT_KOS unsigned int FCALL
libc_invoke_rpc_full(rpc_interrupt_t func, void *arg,
                     struct full_rpc_regs *__restrict regs) {
 unsigned int COMPILER_IGNORE_UNINITIALIZED(result);
 struct full_rpc_regs *EXCEPT_VAR xregs;
 /* Only a noexcept system call needs a guard. */
 if (!(regs->mode & RPC_REASON_NOEXCEPT))
       return (*func)(arg,&regs->ctx,regs->mode);
 xregs = regs;
 LIBC_TRY {
  result = (*func)(arg,&regs->ctx,regs->mode);
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  if (ERRORCODE_ISRTLPRIORITY(error_code()))
      error_rethrow();
  xregs->ctx.c_gpregs.gp_eax = -libc_except_errno();
  error_handled();
  result = RPC_RETURN_RESUME;
 }
 return result;
}

INTDEF void ASMCALL x86_rpc_entry_full(void);


struct full_waitfor_rpc_data {
    rpc_interrupt_t func;
    void           *arg;
    futex_t         pending;  /* Set to ZERO and broadcast when the RPC has finished. */
};

CRT_KOS unsigned int LIBCCALL
full_waitfor_rpc_wrapper(struct full_waitfor_rpc_data *__restrict data,
                         struct cpu_context *__restrict ctx, unsigned int reason) {
 unsigned int COMPILER_IGNORE_UNINITIALIZED(result);
 struct full_waitfor_rpc_data *EXCEPT_VAR xdata = data;
 TRY {
  result = (*data->func)(data->arg,ctx,reason);
 } FINALLY {
  ATOMIC_WRITE(xdata->pending,0);
  libc_futex_wake(&xdata->pending,SIZE_MAX);
 }
 return result;
}



EXPORT(Xqueue_interrupt,libc_Xqueue_interrupt);
CRT_KOS bool LIBCCALL
libc_Xqueue_interrupt(pid_t pid, rpc_interrupt_t func,
                      void *arg, unsigned int mode) {
 struct cpu_context context;
 bool result;
 if (mode & ~(RPC_FSYNCHRONOUS|RPC_FASYNCHRONOUS|RPC_FWAITACK|RPC_FWAITFOR))
     error_throw(E_INVALID_ARGUMENT);
 context.c_eflags        = 0;
 if (mode & RPC_FWAITFOR) {
  struct full_waitfor_rpc_data data;
  data.func               = func;
  data.arg                = arg;
  data.pending            = 1;
#ifdef __x86_64__
  context.c_rip           = (uintptr_t)&x86_rpc_entry_full;
  context.c_gpregs.gp_rdi = (uintptr_t)&full_waitfor_rpc_wrapper;
  context.c_gpregs.gp_rsi = (uintptr_t)&data;
#else
  context.c_eip           = (uintptr_t)&x86_rpc_entry_full;
  context.c_gpregs.gp_ecx = (uintptr_t)&full_waitfor_rpc_wrapper;
  context.c_gpregs.gp_edx = (uintptr_t)&data;
#endif
  result = libc_queue_job(pid,&context,
                         (mode & ~RPC_FWAITFOR)|
                          JOB_FWAITACK|
                          X86_JOB_FSAVE_RETURN|
                          X86_JOB_FLOAD_RETURN|
                          X86_JOB_FSAVE_STACK|
                          X86_JOB_FSAVE_SEGMENTS|
                          X86_JOB_FSAVE_CREGS|
                          X86_JOB_FLOAD_CREGS|
                          X86_JOB_FSAVE_PREGS);
  if (result) {
   struct pollfutex poll_ftx[1];
   struct pollpid   poll_pid[1];
   /* Wait for the RPC to finish.
    * NOTE: Since the thread might terminate before it can
    *       do so, we must also wait for that to happen. */
   pollfutex_init_wait(&poll_ftx[0],&data.pending,1);
   pollpid_init(&poll_pid[0],P_PID,pid,WEXITED,NULL,NULL);
   libc_Xxppoll64(NULL,0,poll_ftx,1,poll_pid,1,NULL,NULL);
   /* Check if the RPC-pending field has been cleared. */
   result = ATOMIC_READ(data.pending) == 0;
  }
 } else {
#ifdef __x86_64__
  context.c_rip           = (uintptr_t)&x86_rpc_entry_full;
  context.c_gpregs.gp_rdi = (uintptr_t)func;
  context.c_gpregs.gp_rsi = (uintptr_t)arg;
#else
  context.c_eip           = (uintptr_t)&x86_rpc_entry_full;
  context.c_gpregs.gp_ecx = (uintptr_t)func;
  context.c_gpregs.gp_edx = (uintptr_t)arg;
#endif
  /* Invoke the RPC */
  result = libc_queue_job(pid,&context,mode|
                          X86_JOB_FSAVE_RETURN|
                          X86_JOB_FLOAD_RETURN|
                          X86_JOB_FSAVE_STACK|
                          X86_JOB_FSAVE_SEGMENTS|
                          X86_JOB_FSAVE_CREGS|
                          X86_JOB_FLOAD_CREGS|
                          X86_JOB_FSAVE_PREGS);
 }
 return result;
}


EXPORT(queue_rpc,libc_queue_rpc);
CRT_KOS int LIBCCALL
libc_queue_rpc(pid_t pid, rpc_t func, void *arg,
               unsigned int mode) {
 int COMPILER_IGNORE_UNINITIALIZED(result);
 LIBC_TRY {
  result = (int)libc_Xqueue_rpc(pid,func,arg,mode);
 } EXCEPT (libc_except_errno()) {
  result = -1;
 }
 return result;
}

EXPORT(queue_interrupt,libc_queue_interrupt);
CRT_KOS int LIBCCALL
libc_queue_interrupt(pid_t pid, rpc_interrupt_t func,
                     void *arg, unsigned int mode) {
 int COMPILER_IGNORE_UNINITIALIZED(result);
 LIBC_TRY {
  result = (int)libc_Xqueue_interrupt(pid,func,arg,mode);
 } EXCEPT (libc_except_errno()) {
  result = -1;
 }
 return result;
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_I386_KOS_RPC_C */
