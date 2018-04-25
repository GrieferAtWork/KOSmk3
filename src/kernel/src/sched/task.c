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
#ifndef GUARD_KERNEL_SRC_SCHED_TASK_C
#define GUARD_KERNEL_SRC_SCHED_TASK_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <kos/types.h>
#include <kernel/sections.h>
#include <kernel/malloc.h>
#include <kernel/debug.h>
#include <kernel/syscall.h>
#include <kernel/vm.h>
#include <kernel/user.h>
#include <kernel/heap.h>
#include <kernel/environ.h>
#include <kos/context.h>
#include <kos/thread.h>
#include <sched/task.h>
#include <sched/mutex.h>
#include <sched/pid.h>
#include <sched/stat.h>
#include <sched/pertask-arith.h>
#include <sched/userstack.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <except.h>
#include <hybrid/align.h>

DECL_BEGIN

#ifndef __INTELLISENSE__
STATIC_ASSERT(offsetof(struct task,t_segment) == TASK_OFFSETOF_SEGMENT);
STATIC_ASSERT(offsetof(struct task,t_refcnt) == TASK_OFFSETOF_REFCNT);
STATIC_ASSERT(offsetof(struct task,t_context) == TASK_OFFSETOF_CONTEXT);
#ifndef CONFIG_NO_SMP
STATIC_ASSERT(offsetof(struct task,t_cpu) == TASK_OFFSETOF_CPU);
#endif /* !CONFIG_NO_SMP */
STATIC_ASSERT(offsetof(struct task,t_vm_lock) == TASK_OFFSETOF_VM_LOCK);
STATIC_ASSERT(offsetof(struct task,t_vm) == TASK_OFFSETOF_VM);
STATIC_ASSERT(offsetof(struct task,t_vmtasks) == TASK_OFFSETOF_VMTASKS);
STATIC_ASSERT(offsetof(struct task,t_userseg) == TASK_OFFSETOF_USERSEG);
STATIC_ASSERT(offsetof(struct task,t_stackmin) == TASK_OFFSETOF_STACKMIN);
STATIC_ASSERT(offsetof(struct task,t_stackend) == TASK_OFFSETOF_STACKEND);
STATIC_ASSERT(offsetof(struct task,t_temppage) == TASK_OFFSETOF_TEMPPAGE);
STATIC_ASSERT(offsetof(struct task,t_sched) == TASK_OFFSETOF_SCHED);
STATIC_ASSERT(offsetof(struct task,t_addrlimit) == TASK_OFFSETOF_ADDR2LIMIT);
STATIC_ASSERT(offsetof(struct task,t_timeout) == TASK_OFFSETOF_TIMEOUT);
STATIC_ASSERT(offsetof(struct task,t_flags) == TASK_OFFSETOF_FLAGS);
STATIC_ASSERT(offsetof(struct task,t_state) == TASK_OFFSETOF_STATE);
STATIC_ASSERT(offsetof(struct task,t_nothrow_serve) == TASK_OFFSETOF_NOTHROW_SERVE);
STATIC_ASSERT(sizeof(struct task) == TASK_OFFSETOF_DATA);
#endif

#ifndef CONFIG_NO_TASKSTAT
STATIC_ASSERT(offsetof(struct taskstat,ts_hswitch) == TASKSTATE_OFFSETOF_HSWITCH);
STATIC_ASSERT(offsetof(struct taskstat,ts_uswitch) == TASKSTATE_OFFSETOF_USWITCH);
STATIC_ASSERT(offsetof(struct taskstat,ts_hyield)  == TASKSTATE_OFFSETOF_HYIELD);
STATIC_ASSERT(offsetof(struct taskstat,ts_uyield)  == TASKSTATE_OFFSETOF_UYIELD);
STATIC_ASSERT(offsetof(struct taskstat,ts_sleep)   == TASKSTATE_OFFSETOF_SLEEP);
STATIC_ASSERT(offsetof(struct taskstat,ts_xrpc)    == TASKSTATE_OFFSETOF_XRPC);
STATIC_ASSERT(offsetof(struct taskstat,ts_qrpc)    == TASKSTATE_OFFSETOF_QRPC);
STATIC_ASSERT(offsetof(struct taskstat,ts_started) == TASKSTATE_OFFSETOF_STARTED);
STATIC_ASSERT(sizeof(struct taskstat) == TASKSTATE_SIZE);

/* Threading statistics for the calling thread.
 * NOTE: Statistics way be read by any thread with restriction to
 *       the fact that all contained data is weak, in the sense
 *       that no absolute meaning should be deciphered from it,
 *       other than its use for hints, or human-readable behavioral
 *       patterns.
 * NOTE: After creation of a thread using `task_alloc()', statistical
 *       tasking information is ZERO-initialized. */
/* Default pattern for statistical thread information (ZERO-initialized). */
PUBLIC ATTR_PERTASK struct taskstat _this_stat = { 0 };

#endif /* !CONFIG_NO_TASKSTAT */

INTERN ATTR_SECTION(".data.pertask.head")
struct task task_template = {
     .t_refcnt    = 2,
#ifndef CONFIG_NO_SMP
     .t_cpu       = &_boot_cpu,     /* Default CPU... */
#endif /* !CONFIG_NO_SMP */
     .t_addrlimit = KERNEL_BASE,
     .t_vm        = NULL,           /* Must be initialized by the user. */
     .t_temppage  = VM_VPAGE_MAX+1, /* Initialized to ~unallocated~ */
     .t_flags     = TASK_FNORMAL,
     .t_state     = TASK_STATE_FINITIAL,
};



typedef void (KCALL *task_func_t)(struct task *__restrict thread);
INTDEF task_func_t pertask_init_start[];
INTDEF task_func_t pertask_init_end[];
INTDEF task_func_t pertask_fini_start[];
INTDEF task_func_t pertask_fini_end[];


PUBLIC ATTR_RETNONNULL ATTR_MALLOC
REF struct task *KCALL task_alloc(void) {
 task_func_t *iter;
 REF struct task *result;
 result = (REF struct task *)kmalloc((size_t)kernel_pertask_size,
                                      GFP_SHARED);
 memcpy(result,kernel_pertask_start,(size_t)kernel_pertask_size);
 assert(result->t_cpu == &_boot_cpu);
 result->t_refcnt = 1;
 /* Setup the task segment self-pointer. */
 result->t_segment.ts_self = &result->t_segment;

 /* Execute initializers. */
 for (iter = pertask_init_start;
      iter < pertask_init_end; ++iter)
    (*iter)(result);

 return result;
}

PUBLIC void KCALL
task_alloc_stack(struct task *__restrict thread,
                 size_t num_pages) {
 REF struct vm_region *EXCEPT_VAR region;
 assert(!thread->t_stackmin);
 assert(!thread->t_stackend);
 assert(!(thread->t_state & TASK_STATE_FSTARTED));
 region = vm_region_alloc(num_pages);
 TRY {
  /* Use physical memory mappings for kernel stacks.
   * This way, we don't have to worry about having to
   * lock the stack in memory, as the VM will not be
   * allowed to swap the stack onto the disk. */
  region->vr_type = VM_REGION_PHYSICAL;
  vm_region_prefault(region);

  /* Map the stack a suitable location, using arch-specific hints. */
  thread->t_stackmin = vm_map(VM_KERNELSTACK_HINT,
                              num_pages,
                              1,
                              0,
                              VM_KERNELSTACK_MODE,
                              0,
                              region,
                              PROT_READ|PROT_WRITE|PROT_NOUSER,
                              NULL,
                              NULL);
  thread->t_stackend = (VIRT void *)((uintptr_t)thread->t_stackmin +
                                     (num_pages * PAGESIZE));
 } FINALLY {
  vm_region_decref(region);
 }
#ifndef NDEBUG
 /* Pre-initialize stack memory to debug-data. */
 memsetl(thread->t_stackmin,0xcccccccc,(num_pages * PAGESIZE) / 4);
#endif
}



#define USERSEG_SIZE  CEILDIV(sizeof(struct user_task_segment),PAGESIZE)


/* Assert user-space task segment offsets. */
STATIC_ASSERT(offsetof(struct user_task_segment,ts_self) == TASK_SEGMENT_OFFSETOF_SELF);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_xcurrent) == TASK_SEGMENT_OFFSETOF_XCURRENT);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_state) == USERTASK_SEGMENT_OFFSETOF_STATE);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_eformat) == USERTASK_SEGMENT_OFFSETOF_EFORMAT);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_errno) == USERTASK_SEGMENT_OFFSETOF_ERRNO);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_dos_errno) == USERTASK_SEGMENT_OFFSETOF_DOS_ERRNO);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_tid) == USERTASK_SEGMENT_OFFSETOF_TID);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_process) == USERTASK_SEGMENT_OFFSETOF_PROCESS);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_ueh) == USERTASK_SEGMENT_OFFSETOF_UEH);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_ueh_sp) == USERTASK_SEGMENT_OFFSETOF_UEH_SP);
#if defined(__i386__) || defined(__x86_64__)
STATIC_ASSERT(offsetof(struct user_task_segment,ts_x86sysbase) == USERTASK_SEGMENT_OFFSETOF_X86SYSBASE);
#endif
#ifndef CONFIG_NO_DOS_COMPAT
STATIC_ASSERT(offsetof(struct user_task_segment,ts_tib) == USERTASK_SEGMENT_OFFSETOF_TIB);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_tib.nt_errno) == USERTASK_SEGMENT_OFFSETOF_NT_ERRNO);
#endif /* !CONFIG_NO_DOS_COMPAT */


PRIVATE void KCALL
setup_user_segment(USER CHECKED struct user_task_segment *__restrict self) {
 /* Setup default values of the user-space thread segment.
  * NOTE: All other values are pre-initialized to all ZEROes. */
 self->ts_self       = self;
 self->ts_tid        = posix_gettid();
 self->ts_process    = PERVM(vm_environ);
#if defined(__i386__) || defined(__x86_64__)
 self->ts_x86sysbase = PERTASK_GET(x86_sysbase);
#endif
#ifndef CONFIG_NO_DOS_COMPAT
 /* Fill in the DOS-compatible TIB data block */
 {
  struct userstack *stack = PERTASK_GET(_this_user_stack);
  if (stack) {
   self->ts_tib.nt_stack_top = (void *)VM_PAGE2ADDR(stack->us_pageend);
   self->ts_tib.nt_stack_min = (void *)VM_PAGE2ADDR(stack->us_pagemin);
  }
  self->ts_tib.nt_pid = posix_getpid();
  self->ts_tib.nt_tid = self->ts_tid;
  self->ts_tib.nt_teb = &self->ts_tib;
  self->ts_tib.nt_tls = self->ts_tib.nt_tlsdata;
 }
#endif
}

INTERN void *KCALL
userseg_notify(void *closure, unsigned int code,
               vm_vpage_t addr, size_t num_pages,
               void *arg) {
 switch (code) {

 case VM_NOTIFY_INCREF:
  break;
 case VM_NOTIFY_CLONE:
  if (closure != THIS_TASK)
      return VM_NOTIFY_CLONE_FLOOSE;
  break;

 default:
  return NULL;
 }
 return closure;
}

PUBLIC void KCALL task_alloc_userseg(void) {
 REF struct vm_region *EXCEPT_VAR region;
 assert(!PERTASK_TESTF(this_task.t_flags,TASK_FKERNELJOB));
 if (PERTASK_TESTF(this_task.t_flags,TASK_FOWNUSERSEG)) return;
 /* Allocate a new region for the user-space thread segment. */
 region = vm_region_alloc(USERSEG_SIZE);

 /* Setup user-space segments to be ZERO-initialized. */
 region->vr_init           = VM_REGION_INIT_FFILLER;
 region->vr_setup.s_filler = 0;

 TRY {
  /* Map the stack a suitable location, using arch-specific hints. */
  PERTASK_SET(this_task.t_userseg,
             (USER struct user_task_segment *)vm_map(VM_USERSEG_HINT,
                                                     USERSEG_SIZE,
                                                     1,
                                                     0,
                                                     VM_USERSEG_MODE,
                                                     0,
                                                     region,
                                                     PROT_READ|PROT_WRITE,
                                                    &userseg_notify,
                                                     THIS_TASK));
 } FINALLY {
  vm_region_decref(region);
 }
 ATOMIC_FETCHOR(THIS_TASK->t_flags,TASK_FOWNUSERSEG);
 COMPILER_WRITE_BARRIER();
 setup_user_segment(PERTASK_GET(this_task.t_userseg));
}




PRIVATE struct vm_region singlepage_reserved_region = {
    .vr_refcnt = 1,
    .vr_lock   = MUTEX_INIT,
    .vr_futex  = ATOMIC_RWPTR_INIT(NULL),
    .vr_type   = VM_REGION_RESERVED,
    .vr_init   = VM_REGION_INIT_FNORMAL,
    .vr_flags  = VM_REGION_FDONTMERGE,
    .vr_funds  = 0,
    .vr_size   = 1, /* Single page */
    .vr_parts  = &singlepage_reserved_region.vr_part0,
    .vr_part0  = {
        .vp_refcnt = 1,
        .vp_chain  = {
            .le_next  = NULL,
            .le_pself = &singlepage_reserved_region.vr_parts,
        },
        .vp_start  = 0,
        .vp_state  = VM_PART_INCORE,
        .vp_flags  = VM_PART_FKEEP|VM_PART_FWEAKREF,
        .vp_locked = 0x3fff,
        .vp_phys   = {
            .py_num_scatter = 1,
            .py_iscatter    = {
                [0] = {
                    .ps_addr = 0,
                    .ps_size = 1
                }
            }
        }
    }
};

PUBLIC ATTR_CONST vm_vpage_t KCALL task_temppage(void) {
 struct vm_node *EXCEPT_VAR node;
 vm_vpage_t result = PERTASK_GET(this_task.t_temppage);
 if (result != VM_VPAGE_MAX+1)
     return result;
 /* Construct a new temporary page mapping. */
 node = (struct vm_node *)kmalloc(sizeof(struct vm_node),
                                  GFP_SHARED|GFP_LOCKED);
 TRY {
  vm_acquire(&vm_kernel);
  TRY {
   /* Determine a suitable free location for the temporary page. */
   result = vm_getfree(VM_TEMPPAGE_HINT,1,1,0,VM_TEMPPAGE_MODE);
   node->vn_node.a_vmin = result;
   node->vn_node.a_vmax = result;
   node->vn_region      = &singlepage_reserved_region;
   node->vn_start       = 0;
   node->vn_notify      = NULL;
   node->vn_prot        = PROT_EXEC|PROT_READ|PROT_WRITE|PROT_SHARED|PROT_NOUSER;
   node->vn_flag        = VM_NODE_FNORMAL;

   /* Increment reference counters of the affected pats of the reserved region. */
   ATOMIC_FETCHINC(singlepage_reserved_region.vr_refcnt);
   ATOMIC_FETCHINC(singlepage_reserved_region.vr_part0.vp_refcnt);

   /* Insert the node into the kernel VM (but don't map
    * anything. The caller is responsible to do that) */
   vm_insert_node(&vm_kernel,node);
  } FINALLY {
   vm_release(&vm_kernel);
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(node);
  error_rethrow();
 }
 /* Save the new temppage location. */
 PERTASK_SET(this_task.t_temppage,result);
 return result;
}


PUBLIC ATTR_NOTHROW
void (KCALL task_nothrow_serve)(void) {
 PERTASK_INC(this_task.t_nothrow_serve);
}
PUBLIC void (KCALL task_nothrow_end)(void) {
 struct task *t = THIS_TASK;
 assert(t->t_nothrow_serve);
 if (!--t->t_nothrow_serve &&
       (t->t_state&TASK_STATE_FSERVEEXCEPT)) {
  ATOMIC_FETCHAND(t->t_state,~TASK_STATE_FSERVEEXCEPT);
  /* Make the current error non-continuable. */
  error_info()->e_error.e_flag &= ~ERR_FRESUMABLE;
  /* Throw the last error (hopefully the one caused by the RPC function). */
  error_throw_current();
 }
}

PUBLIC void KCALL
task_failed(struct task *__restrict self) {
 u16 state;
 do {
  state = ATOMIC_READ(self->t_state);
  if (state & TASK_STATE_FSTARTED)
      break; /* Don't set if the thread was started. */
 } while (!ATOMIC_CMPXCH_WEAK(self->t_state,state,state|
                             (TASK_STATE_FTERMINATING|
                              TASK_STATE_FTERMINATED)));
}

PUBLIC void KCALL
task_destroy(struct task *__restrict self) {
 task_func_t *iter;

 /* Execute finalizers. */
 for (iter = pertask_fini_start;
      iter < pertask_fini_end; ++iter)
    (*iter)(self);

 if (self->t_vm) {
  /* NOTE: Because kernel stacks are allocated in the kernel-share segment,
   *       we are able to unmap them just as the thread was/would have been. */
  if (self->t_stackmin != self->t_stackend &&
    !(self->t_flags&TASK_FNOHOSTSTACK)) {
   vm_vpage_t stack_minpage = VM_ADDR2PAGE((uintptr_t)self->t_stackmin);
   vm_vpage_t stack_endpage = VM_ADDR2PAGE((uintptr_t)self->t_stackend);
   assert(IS_ALIGNED((uintptr_t)self->t_stackmin,PAGEALIGN));
   assert(IS_ALIGNED((uintptr_t)self->t_stackend,PAGEALIGN));
   /* Unmap the host stack of this task. */
   vm_unmap(stack_minpage,
            stack_endpage-stack_minpage,
            VM_UNMAP_NOEXCEPT|
            VM_UNMAP_SYNC,NULL);
  }
  atomic_rwlock_write(&self->t_vm->vm_tasklock);
  LIST_REMOVE(self,t_vmtasks);
  atomic_rwlock_endwrite(&self->t_vm->vm_tasklock);
  vm_decref(self->t_vm);
 }

 /* Finally, free the task structure itself. */
 kfree(self);
}


PUBLIC ATTR_RETNONNULL REF struct vm *
KCALL task_getvm(struct task *__restrict thread) {
 REF struct vm *result;
 atomic_rwlock_read(&thread->t_vm_lock);
 result = thread->t_vm;
 vm_incref(result);
 atomic_rwlock_endread(&thread->t_vm_lock);
 return result;
}

PUBLIC void KCALL
task_setvm(struct vm *__restrict new_vm) {
 pflag_t was;
 struct vm *old_vm = THIS_VM;
 if (new_vm == old_vm) return; /* No change */
again:
 vm_acquire(new_vm);
 if (!vm_tryacquire(old_vm)) {
again_newvm:
  vm_release(new_vm);
  task_yield();
  goto again;
 }
 if (!atomic_rwlock_trywrite(&PERTASK(this_task.t_vm_lock))) {
  vm_release(new_vm);
  goto again_newvm;
 }
 was = PREEMPTION_PUSHOFF();
 /* Set the new VM as active. */
 THIS_TASK->t_vm = new_vm;
 COMPILER_WRITE_BARRIER();
 atomic_rwlock_endwrite(&PERTASK(this_task.t_vm_lock));
 LIST_REMOVE(THIS_TASK,t_vmtasks);
 vm_release(old_vm);
 LIST_INSERT(new_vm->vm_tasks,THIS_TASK,t_vmtasks);
 COMPILER_BARRIER();
 /* Set the new physical page directory
  * pointer, switching context to it. */
 pagedir_set((pagedir_t *)(uintptr_t)new_vm->vm_physdir);
 PREEMPTION_POP(was);
 vm_release(new_vm);
 /* Update reference counters. */
 vm_incref(new_vm); /* The reference now saved in `t_vm' */
 vm_decref(old_vm); /* The reference inherited from `t_vm' */
}

DEFINE_SYSCALL_DONTRESTART(sched_yield);
DEFINE_SYSCALL0(sched_yield) {
 return task_tryyield() ? 0 : -EAGAIN;
}

#if 1
DEFINE_SYSCALL2(getcpu,
                USER UNCHECKED unsigned int *,pcpu,
                USER UNCHECKED unsigned int *,pnode)
#else
DEFINE_SYSCALL3(getcpu,
                USER UNCHECKED unsigned int *,pcpu,
                USER UNCHECKED unsigned int *,pnode,
                USER UNCHECKED void *,ignored)
#endif
{
 if (pcpu) {
  validate_writable(pcpu,sizeof(*pcpu));
  *pcpu = THIS_CPU->cpu_id;
 }
 if (pnode) {
  validate_writable(pnode,sizeof(*pnode));
  *pnode = THIS_CPU->cpu_id; /* XXX: CPU Node support? */
 }
 return 0;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_TASK_C */
