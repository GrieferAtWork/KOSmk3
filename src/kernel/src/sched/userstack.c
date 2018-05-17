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
#ifndef GUARD_KERNEL_SRC_SCHED_USERSTACK_C
#define GUARD_KERNEL_SRC_SCHED_USERSTACK_C 1

#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kos/types.h>
#include <hybrid/align.h>
#include <hybrid/atomic.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <kernel/vm.h>
#include <kernel/paging.h>
#include <kernel/memory.h>
#include <kernel/bind.h>
#include <sched/userstack.h>
#include <sched/task.h>
#include <except.h>

DECL_BEGIN



PUBLIC void *KCALL
userstack_notify(void *closure, unsigned int code,
                 vm_vpage_t addr, size_t num_pages,
                 void *arg) {
 struct userstack *me;
 me = (struct userstack *)closure;
 switch (code) {

 case VM_NOTIFY_INCREF:
  userstack_incref(me);
  break;

 case VM_NOTIFY_DECREF:
  userstack_decref(me);
  break;

 default:
  return NULL;
 }
 return closure;
}



/* [0..1][owned][lock(PRIVATE(THIS_TASK))]
 * The user-space stack descriptor of the calling thread.
 * This pointer is lazily allocated by `task_alloc_userstack()'
 * Finally, this stack will be unmapped in `task_exit()'. */
PUBLIC ATTR_PERTASK REF struct userstack *_this_user_stack = NULL;


DEFINE_PERTASK_CLEANUP(task_cleanup_user_stack);
INTERN void KCALL task_cleanup_user_stack(void) {
 REF struct userstack *EXCEPT_VAR ustack;
 /* Unmap and decref() this thread's user-space stack. */
 if ((ustack = PERTASK_GET(_this_user_stack)) != NULL) {
  TRY {
   assert(ustack->us_pageend >= ustack->us_pagemin);
   /* Unmap the user-space stack. */
   vm_unmap(ustack->us_pagemin,
            ustack->us_pageend-ustack->us_pagemin,
            VM_UNMAP_TAG|VM_UNMAP_NOEXCEPT|
            VM_UNMAP_SYNC,ustack);
  } FINALLY {
   /* Steal a reference from `_this_user_stack' */
   PERTASK_SET(_this_user_stack,NULL);
   userstack_decref(ustack);
  }
 }
}

DEFINE_PERTASK_FINI(task_fini_user_stack);
INTERN void KCALL task_fini_user_stack(struct task *__restrict thread) {
 REF struct userstack *ustack;
 if ((ustack = FORTASK(thread,_this_user_stack)) != NULL)
      userstack_decref(ustack);
}

DEFINE_PERTASK_CLONE(task_clone_user_stack);
INTERN void KCALL task_clone_user_stack(struct task *__restrict thread) {
 REF struct userstack *ustack;
 if ((ustack = FORTASK(thread,_this_user_stack)) != NULL)
      userstack_decref(ustack);
}


/* Allocate an automatically-sized user-space stack for the calling thread.
 * This stack is lazily allocated and mapped, meaning that this function
 * will return the same object after being called a second time. */
PUBLIC ATTR_RETNONNULL ATTR_CONST
struct userstack *KCALL task_alloc_userstack(void) {
 struct userstack *EXCEPT_VAR result;
 REF struct vm_region *EXCEPT_VAR stack_region;
 result = PERTASK_GET(_this_user_stack);
 /* Quick check: is the stack already allocated. */
 if (result) return result;
#define USER_STACK_PAGES   CEILDIV(CONFIG_USERSTACK_SIZE,PAGESIZE)
 /* TODO: Guard pages 'n stuff. */
 result = (struct userstack *)kmalloc(sizeof(struct userstack),
                                      GFP_SHARED);
 result->us_refcnt = 1;
 TRY {
  void *COMPILER_IGNORE_UNINITIALIZED(stack_base);
  stack_region = vm_region_alloc(USER_STACK_PAGES);

  /* Setup custom initialization of user-space stacks. */
  stack_region->vr_init           = VM_REGION_INIT_FFILLER;
  stack_region->vr_setup.s_filler = 0xcccccccc; /* XXX: Make this an option somewhere. */

  /* This initialization is required due to a
   * race condition further explained below. */
  result->us_pagemin = VM_VPAGE_MAX+1;
  result->us_pageend = 0;
  COMPILER_WRITE_BARRIER();
#if 0 /* TODO: For some reason, the VM is locked with 2 locks after a clone(),
       *       when there is no reason for it to be. The locks go away shortly
       *       after, but this might be some situation I though to be rare,
       *       actually being quite common... Investigate this. -> Performace! */
 debug_printf("TASK_ALLOC_USERSTACK #1.3 (%p,%p,%p)\n",
             ((u32 *)&THIS_VM->vm_lock)[0],
             ((u32 *)&THIS_VM->vm_lock)[1],
             ((u32 *)&THIS_VM->vm_lock)[2]);
#endif

  TRY {
   /* Map the stack region to a suitable location. */
   stack_base = vm_map(VM_USERSTACK_HINT,
                       USER_STACK_PAGES,
                       1,
                       VM_USERSTACK_GAP,
                       VM_USERSTACK_MODE,
                       0,
                       stack_region,
                       PROT_READ|PROT_WRITE,
                      &userstack_notify,
                       result);
  } FINALLY {
   vm_region_decref(stack_region);
  }
#if 0
 debug_printf("TASK_ALLOC_USERSTACK #1.4\n");
#endif
  /* Save the region location.
   * NOTE: The reason why this looks so weird is because of a race
   *       condition arising from the fact that we're no longer
   *       holding any locks on the VM, meaning that other threads
   *       may already be faulting the stack and updating its
   *       pointers. */
  {
   vm_vpage_t temp,newval;
   newval = VM_ADDR2PAGE((uintptr_t)stack_base);
   do if (newval > (temp = ATOMIC_READ(result->us_pagemin))) break;
   while (!ATOMIC_CMPXCH_WEAK(result->us_pagemin,temp,newval));
   newval += USER_STACK_PAGES;
   do if (newval < (temp = ATOMIC_READ(result->us_pageend))) break;
   while (!ATOMIC_CMPXCH_WEAK(result->us_pageend,temp,newval));
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  userstack_decref(result);
  error_rethrow();
 }
 PERTASK_SET(_this_user_stack,result); /* Inherit reference. */
 return result;
}


/* Destroy a previously allocated userstack. */
PUBLIC ATTR_NOTHROW void KCALL
userstack_destroy(struct userstack *__restrict self) {
 kfree(self);
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_USERSTACK_C */
