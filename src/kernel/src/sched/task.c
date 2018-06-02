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
#include <kos/safecall.h>
#include <kernel/sections.h>
#include <kernel/cache.h>
#include <kernel/bind.h>
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
     .t_refcnt     = 2,
#ifndef CONFIG_NO_SMP
     .t_cpu        = &_boot_cpu,     /* Default CPU... */
#endif /* !CONFIG_NO_SMP */
     .t_addrlimit  = KERNEL_BASE,
     .t_userseg_sz = sizeof(struct user_task_segment),
     .t_vm         = NULL,           /* Must be initialized by the user. */
     .t_temppage   = VM_VPAGE_MAX+1, /* Initialized to ~unallocated~ */
     .t_flags      = TASK_FNORMAL,
     .t_state      = TASK_STATE_FINITIAL,
};



typedef void (KCALL *task_func_t)(struct task *__restrict thread);
INTDEF task_func_t pertask_init_start[];
INTDEF task_func_t pertask_init_end[];
INTDEF task_func_t pertask_fini_start[];
INTDEF task_func_t pertask_fini_end[];


/* List of all existing threads (including those that havn't
 * been started yet, as well as those that are about to terminate) */
INTERN DEFINE_ATOMIC_RWLOCK(all_threads_lock);
INTERN LIST_HEAD(struct task) all_threads_list = &_boot_task;
INTERN ATTR_PERTASK LIST_NODE(struct task) all_threads_link = { NULL, &all_threads_list };


/* Same as the functions above, but used to create a
 * snapshot of all threads currently in existence.
 * @param: buf:          A pointer to a vector of task pointers to-be filled with running threads upon success.
 * @param: buf_length:   The number of pointers that can be written into `buf' before it has filled up.
 * @return: * :          The required number of task buffer pointers.
 *                       When this value is greater than `buf_length', no reference will
 *                       have been stored in `*buf', and its contents are undefined.
 *                       When lower than, or equal to `buf_length', `buf' will have been
 *                       filled with the reference to all threads in existence matching
 *                       the given argument.
 * @throw: E_WOULDBLOCK: Preemption has been disabled. */
FUNDEF size_t KCALL
task_enumerate_ex(REF struct task **__restrict buf, size_t buf_length,
                  u16 state_mask, u16 state_flag) {
 size_t result = 0;
 REF struct task *iter;
 atomic_rwlock_read(&all_threads_lock);
 iter = all_threads_list;
 while (iter) {
  if ((ATOMIC_READ(iter->t_state) & state_mask) != state_flag ||
      !task_tryincref(iter)) {
   iter = FORTASK(iter,all_threads_link).le_next;
   continue; /* Ignored task, or dead task. */
  }
  /* Buffer isn't large enough. */
  if unlikely(result == buf_length) {
   /* Continue enumeration, then drop all references already collected. */
   struct task *iter2 = iter;
   do if ((ATOMIC_READ(iter2->t_state) & state_mask) == state_flag &&
          (ATOMIC_READ(iter2->t_refcnt) != 0)) ++result;
   while ((iter2 = FORTASK(iter2,all_threads_link).le_next) != NULL);
   atomic_rwlock_endread(&all_threads_lock);
   task_decref(iter);
   while (buf_length--) task_decref(buf[buf_length]);
   return result;
  }
  assert(result < buf_length);
  buf[result++] = iter; /* Inherit reference. */
  iter = FORTASK(iter,all_threads_link).le_next;
 }
 atomic_rwlock_endread(&all_threads_lock);
 return result;
}



/* Similar to `task_foreach()' above, but doesn't give the guaranty that any
 * thread is enumerated only once, or that no threads will be skipped during
 * enumeration, in trade for the ability to enumerate without the need of a
 * temporary buffer.
 * @return: * : The total number of enumerated threads.
 * @throw: E_WOULDBLOCK: Preemption has been disabled. */
PUBLIC size_t KCALL
task_foreach_weak_ex(bool (KCALL *func)(struct task *__restrict thread, void *arg),
                     void *arg, u16 state_mask, u16 state_flag) {
 REF struct task *EXCEPT_VAR iter;
 size_t result = 0;
again:
 atomic_rwlock_read(&all_threads_lock);
 iter = all_threads_list;
 while (iter) {
  if ((ATOMIC_READ(iter->t_state) & state_mask) != state_flag ||
      !task_tryincref(iter)) {
   iter = FORTASK(iter,all_threads_link).le_next;
   continue; /* Ignored task, or dead task. */
  }
  atomic_rwlock_endread(&all_threads_lock);
  ++result;
  /* Invoke the given callback. */
  TRY {
   if (!SAFECALL_KCALL_2(*func,iter,arg))
        goto done;
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   task_decref(iter);
   error_rethrow();
  }
  atomic_rwlock_read(&all_threads_lock);
  /* Must decref() _after_ having re-acquired the lock to ensure that
   * we can still safely read out the task's next-pointer below. */
  if (!ATOMIC_DECIFNOTONE(iter->t_refcnt)) {
   atomic_rwlock_endread(&all_threads_lock);
   task_decref(iter);
   goto again;
  }
  iter = FORTASK(iter,all_threads_link).le_next;
 }
 atomic_rwlock_endread(&all_threads_lock);
done:
 return result;
}

#define TASK_STACK_BUFFER_SIZE  32
PRIVATE ATTR_NOINLINE size_t KCALL
task_foreach_stack_ex(bool (KCALL *func)(struct task *__restrict thread, void *arg),
                      void *arg, u16 state_mask, u16 state_flag) {
 REF struct task *buf[TASK_STACK_BUFFER_SIZE];
 size_t EXCEPT_VAR result;
 result = task_enumerate_ex(buf,
                            TASK_STACK_BUFFER_SIZE,
                            state_mask,
                            state_flag);
 if (result <= TASK_STACK_BUFFER_SIZE) {
  /* We already got all the thread! */
  size_t EXCEPT_VAR i = 0;
  TRY {
   for (; i < result; ++i) {
    if (!SAFECALL_KCALL_2(func,buf[i],arg)) {
     /* Stop enumeration. (drop references from all remaining threads) */
     for (; i < result; ++i) task_decref(buf[i]);
     break;
    }
    task_decref(buf[i]);
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* Drop references that hadn't been enumerated, yet. */
   for (; i < result; ++i)
       task_decref(buf[i]);
   error_rethrow();
  }
 }
 return result;
}


/* Enumerate all threads in existence, including those that haven't
 * been started yet, as well as those that have already terminated.
 * NOTES:
 *   - `task_foreach_running()' is similar to `task_foreach()', but
 *      excludes threads that haven't been started, or have already
 *      terminated.
 *     (NOTE: thread currently terminating are still enumerated, though)
 *   - `task_foreach_ex()' is an extended variant that only enumerates tasks
 *      for with the condition `(thread->t_state & state_mask) == state_flag'
 *      applies at the time when the task is enumerated.
 *   -  These functions are implemented using the `task_enumerate()'
 *      API below, which is why they need the ability of dynamically
 *      allocating heap memory when there are too many threads in
 *      existence so-as to allocate a buffer on the stack.
 * @return: * :          The total number of enumerated threads.
 * @throw: E_BADALLOC:   Too many threads are running and enumeration failed
 *                       to allocate a temporary buffer located on the heap.
 * @throw: E_WOULDBLOCK: Preemption has been disabled. */
PUBLIC size_t KCALL
task_foreach_ex(bool (KCALL *func)(struct task *__restrict thread, void *arg),
                void *arg, u16 state_mask, u16 state_flag) {
 size_t EXCEPT_VAR result;
 REF struct task **EXCEPT_VAR buffer;
 result = task_foreach_stack_ex(func,arg,state_mask,state_flag);
 if (result <= TASK_STACK_BUFFER_SIZE) goto done;
 /* Allocate a dynamic buffer. */
 buffer = (REF struct task **)kmalloc(result*sizeof(REF struct task *),
                                      GFP_SHARED);
 TRY {
  size_t EXCEPT_VAR i;
  size_t EXCEPT_VAR new_length;
retry_enumeration:
  new_length = task_enumerate_ex(buffer,result,state_mask,state_flag);
  if unlikely(new_length > result) {
   /* Allocate more memory and try again. */
   buffer = (REF struct task **)krealloc(buffer,
                                         new_length*sizeof(REF struct task *),
                                         GFP_SHARED);
   result = new_length;
   goto retry_enumeration;
  }
  /* Got all the threads. Now to enumerate them */
  i = 0;
  TRY {
   for (; i < result; ++i) {
    if (!SAFECALL_KCALL_2(func,buffer[i],arg)) {
     /* Stop enumeration. (drop references from all remaining threads) */
     for (; i < result; ++i) task_decref(buffer[i]);
     break;
    }
    task_decref(buffer[i]);
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* Drop references that hadn't been enumerated, yet. */
   for (; i < result; ++i)
       task_decref(buffer[i]);
   error_rethrow();
  }
 } FINALLY {
  kfree(buffer);
 }
done:
 return result;
}



typedef void (KCALL *pertask_clear_cache_t)(struct task *__restrict thread);
INTDEF pertask_clear_cache_t pertask_clear_caches_start[];
INTDEF pertask_clear_cache_t pertask_clear_caches_end[];

PRIVATE bool KCALL
clear_pertask_caches(struct task *__restrict thread,
                     void *UNUSED(arg)) {
 pertask_clear_cache_t *iter;
 for (iter = pertask_clear_caches_start;
      iter < pertask_clear_caches_end; ++iter) {
#if 1
  debug_printf("%[vinfo:%f(%l,%c) : %n : %p] : CC_INVOKE_PERTASK(%u)\n",
               *iter,posix_gettid_view(thread));
#endif
  SAFECALL_KCALL_VOID_1(**iter,thread);
  if (kernel_cc_done())
      return false; /* Stop if caching is done. */
 }
 /* Only continue enumeration when cache clearing isn't done, yet. */
 return !kernel_cc_done();
}

DEFINE_GLOBAL_CACHE_CLEAR(global_clear_pertask_caches);
PRIVATE ATTR_USED void KCALL global_clear_pertask_caches(void) {
 task_foreach_weak_running(&clear_pertask_caches,NULL);
}


typedef void (KCALL *pertask_unbind_driver_t)(struct task *__restrict thread, struct driver *__restrict d);
INTDEF pertask_unbind_driver_t pertask_unbind_driver_start[];
INTDEF pertask_unbind_driver_t pertask_unbind_driver_end[];
PRIVATE bool KCALL
unbind_pertask_drivers(struct task *__restrict thread,
                       void *arg) {
 pertask_unbind_driver_t *iter;
 for (iter = pertask_unbind_driver_start;
      iter < pertask_unbind_driver_end; ++iter)
      SAFECALL_KCALL_VOID_2(**iter,thread,(struct driver *)arg);
 return true;
}

DEFINE_GLOBAL_UNBIND_DRIVER(global_unbind_pertask_drivers);
PRIVATE ATTR_USED void KCALL
global_unbind_pertask_drivers(struct driver *__restrict d) {
 task_foreach(&unbind_pertask_drivers,d);
}







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

 TRY {
  /* Execute initializers. */
  for (iter = pertask_init_start;
       iter < pertask_init_end; ++iter)
       SAFECALL_KCALL_VOID_1(**iter,result);

  /* Register the task. */
  atomic_rwlock_write(&all_threads_lock);
  assertf(FORTASK(result,all_threads_link).le_pself == &all_threads_list,
          "This initialization should be part of the pertask template");
  FORTASK(result,all_threads_link).le_next = all_threads_list;
  if likely(all_threads_list)
     FORTASK(all_threads_list,all_threads_link).le_pself = &FORTASK(result,all_threads_link).le_next;
  all_threads_list = result;
  atomic_rwlock_endwrite(&all_threads_lock);

 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  iter = pertask_fini_end;
  while (iter-- != pertask_fini_start)
       SAFECALL_KCALL_VOID_1(**iter,result);
  kfree(result);
  error_rethrow();
 }

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

PUBLIC ATTR_NOTHROW void KCALL
task_destroy(struct task *__restrict self) {
 task_func_t *iter;
 assert(self != &_boot_task);

 /* Unregister the task. */
 atomic_rwlock_write(&all_threads_lock);
 {
  struct task *next_task = FORTASK(self,all_threads_link).le_next;
  if ((*FORTASK(self,all_threads_link).le_pself = next_task) != NULL)
        FORTASK(next_task,all_threads_link).le_pself = FORTASK(self,all_threads_link).le_pself;
 }
 atomic_rwlock_endwrite(&all_threads_lock);


 /* Execute finalizers. */
 iter = pertask_fini_end;
 while (iter-- != pertask_fini_start)
      SAFECALL_KCALL_VOID_1(**iter,self);

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
 return (task_tryyield)();
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


DEFINE_SYSCALL1(xnosignal,unsigned int,mode) {
 if (mode > 1)
     error_throw(E_INVALID_ARGUMENT);
 if (mode == 0)
     task_clear_nosignals();
 else {
  /* Set the nosignals flag. */
  ATOMIC_FETCHOR(THIS_TASK->t_flags,TASK_FNOSIGNALS);
 }
 return 0;
}


PUBLIC NOIRQ bool KCALL
task_qsleep(qtime_t abs_timeout) {
 qtime_t now;
 jtime_t jnow = ATOMIC_READ(jiffies);
 if (abs_timeout.qt_jiffies <= jnow) {
  /* The timeout has already passed. - Check the quantum time and
   * try to yield to another thread, hoping that it will yield again.
   * This way, we don't waste too much idle time doing nothing. */
  PREEMPTION_ENABLE();
  if (abs_timeout.qt_jiffies < jnow)
      return false; /* Timeout already expired. */
  now = qtime_now();
  if (QTIME_GREATER_EQUAL(now,abs_timeout))
      goto quantum_timeout;
  if (task_tryyield()) {
   now = qtime_now();
   if (QTIME_GREATER_EQUAL(now,abs_timeout))
       goto quantum_timeout;
  }
 } else {
  if (task_sleep(abs_timeout.qt_jiffies))
      return true;
  /* Spend the remainder of the sub-quantum doing arbitrary wake-ups. */
  now = qtime_now();
  if (QTIME_GREATER_EQUAL(now,abs_timeout))
      goto quantum_timeout;
 }
 return true;
quantum_timeout:
 return false;
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_TASK_C */
