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
#ifndef GUARD_KERNEL_SRC_VM_REGION_TLS_C
#define GUARD_KERNEL_SRC_VM_REGION_TLS_C 1
#define _KOS_SOURCE 1
#define _NOSERVE_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/align.h>
#include <kos/thread.h>
#include <kernel/vm.h>
#include <kernel/tls.h>
#include <kernel/environ.h>
#include <kernel/sections.h>
#include <kernel/debug.h>
#include <kernel/bind.h>
#include <kernel/malloc.h>
#include <sched/pid.h>
#include <sched/group.h>
#include <sched/userstack.h>
#include <sched/task.h>
#include <assert.h>
#include <string.h>
#include <except.h>

DECL_BEGIN

/* The allocation gap to try and preserve between TLS allocations */
#if PAGESIZE >= 256
#define TLS_GAP       1
#define TLS_ALIGN     1
#else
#define TLS_GAP       CEILDIV(256,PAGESIZE)
#endif

#define USERSEG_SIZE  CEILDIV(sizeof(struct user_task_segment),PAGESIZE)
#define SIZEOF_USER_TASK_SEGMENT  sizeof(struct user_task_segment)
#define ALIGNOF_USER_TASK_SEGMENT 16 /* alignof(struct user_task_segment) */



#define VM_TLS_PREALLOC 2
struct vm_tls_entry {
    uintptr_t          te_base;     /* Base address */
    size_t             te_size;     /* Total segment size. */
    size_t             te_align;    /* [!0][!(. & (. - 1))] Segment alignment. */
    USER CHECKED void *te_template; /* Template base address. */
    size_t             te_size_tpl; /* [<= te_size] Template size. */
};
struct vm_tls {
    size_t               vt_total;  /* Total size from 0 to the end of the greatest segment. */
    size_t               vt_align;  /* [!0] Greatest alignment required by any segment. */
    size_t               vt_cnt;    /* The number of existing TLS segments. */
    struct vm_tls_entry *vt_vec;    /* [owned_if(!= vt_sbuf)][0..vt_cnt][sort(ASCENDING(.te_base))]
                                     * Vector of TLS segments. */
#if VM_TLS_PREALLOC != 0
    struct vm_tls_entry  vt_sbuf[VM_TLS_PREALLOC]; /* Static TLS segment buffer. */
#endif
};

/* [lock(:vm_lock)] Per-VM TLS descriptors. */
PRIVATE ATTR_PERVM struct vm_tls my_tls = {
    .vt_cnt   = 0,
    .vt_vec   = NULL,
    .vt_align = 1
};

PRIVATE ATTR_USED void KCALL vm_tls_fini(struct vm *__restrict self) {
#if VM_TLS_PREALLOC != 0
 if (FORVM(self,my_tls).vt_vec != FORVM(self,my_tls).vt_sbuf)
#endif
 {
  kfree(FORVM(self,my_tls).vt_vec);
 }
}



/* Return the total size of all static TLS segments. */
LOCAL size_t KCALL sizeof_static_tls_segment(void) {
 assert(vm_holding_read(THIS_VM));
 return PERVM(my_tls).vt_total;
}

/* Return the MAX alignment of all static TLS segments. */
LOCAL size_t KCALL alignof_static_tls_segment(void) {
 assert(vm_holding_read(THIS_VM));
 assert(PERVM(my_tls).vt_align != 0);
 return PERVM(my_tls).vt_align;
}

/* Copy TLS templates into `thread_base', thus initializing it. */
PRIVATE void KCALL
init_tls_segments(USER CHECKED byte_t *__restrict thread_base) {
 size_t i; struct vm_tls *me = &PERVM(my_tls);
 assert(vm_holding_read(THIS_VM));
 thread_base += me->vt_total;
 for (i = 0; i < me->vt_cnt; ++i) {
  struct vm_tls_entry *entry = &me->vt_vec[i];
  /* Copy template data. */
  memcpy(thread_base - (entry->te_base + entry->te_size),
         entry->te_template,entry->te_size_tpl);
#if 0 /* Not actually required. - Since this is only called from
       * `task_alloc_userseg()', the region pre-initialization
       * will have already pre-initialized memory to all ZEROes. */
  /* Zero out .bss memory. */
  memset(thread_base - (entry->te_base + entry->te_size_tpl),
         0,entry->te_size - entry->te_size_tpl);
#endif
 }
}

INTDEF void *KCALL
userseg_notify(void *closure, unsigned int code,
               vm_vpage_t addr, size_t num_pages,
               void *arg);


PUBLIC uintptr_t KCALL
tls_alloc_impl(USER CHECKED void *template_base,
               size_t template_size, size_t num_bytes,
               size_t tls_alignment) {
 size_t size_avail,i;
 uintptr_t result;
 struct vm_tls *me = &PERVM(my_tls);
 assert(num_bytes != 0);
 assert(vm_holding(THIS_VM));
 assert(template_size <= num_bytes);
 assert(tls_alignment);
 assert((tls_alignment & (tls_alignment -1 )) == 0);
#if VM_TLS_PREALLOC != 0
 if (!me->vt_vec) {
  me->vt_vec = me->vt_sbuf;
  size_avail = VM_TLS_PREALLOC;
 } else
#endif
 {
#if VM_TLS_PREALLOC != 0
  if (me->vt_vec == me->vt_sbuf)
      size_avail = VM_TLS_PREALLOC;
  else
#endif
  {
   size_avail = (kmalloc_usable_size(me->vt_vec) /
                 sizeof(*me->vt_vec));
  }
 }
 /* Allocate a new TLS descriptor. */
 assert(me->vt_cnt <= size_avail);
 if (me->vt_cnt == size_avail) {
  /* Allocate more vector memory. */
#if VM_TLS_PREALLOC != 0
  if (me->vt_vec == me->vt_sbuf) {
   me->vt_vec = (struct vm_tls_entry *)kmalloc((me->vt_cnt+1)*
                                                sizeof(struct vm_tls_entry),
                                                GFP_SHARED);
   memcpy(me->vt_vec,me->vt_sbuf,sizeof(me->vt_sbuf));
  } else
#endif
  {
   me->vt_vec = (struct vm_tls_entry *)krealloc(me->vt_vec,(me->vt_cnt+1)*
                                                sizeof(struct vm_tls_entry),
                                                GFP_SHARED);
  }
 }
 /* Find a suitable insert location. */
 i = 0;
 switch (me->vt_cnt) {
 case 0:
  result = 0;
  break;
 case 1:
  if (me->vt_vec[0].te_base >= num_bytes)
      result = 0;
  else {
   result = me->vt_vec[0].te_base+me->vt_vec[0].te_size;
   i      = 1;
   result = CEIL_ALIGN(result + num_bytes,tls_alignment) - num_bytes;
  }
  break;
 default:
  if (me->vt_vec[0].te_base >= num_bytes)
      result = 0;
  else {
   i = 1;
   for (;; ++i) {
    size_t avail;
    /* Determine the insert candidate location following the previous segment. */
    result = (me->vt_vec[i-1].te_base+me->vt_vec[i-1].te_size);
    result = CEIL_ALIGN(result + num_bytes,tls_alignment) - num_bytes;
    if (i >= me->vt_cnt) break;
    if (result < me->vt_vec[i].te_base) {
     avail = me->vt_vec[i].te_base - result;
     if (avail < num_bytes)
         continue; /* Too little space. */
     assert(i != me->vt_cnt);
     goto insert_in_gap;
    }
   }
   assert(i == me->vt_cnt);
  }
  break;
 }
 if (i == me->vt_cnt) {
  /* A new segment is appended at the end. -> Search for
   * threads which need to extend their TLS footprint. */
  struct vm *EXCEPT_VAR myvm = THIS_VM;
  struct task *EXCEPT_VAR thread;
  size_t needed_footprint;
  atomic_rwlock_read(&myvm->vm_tasklock);
  needed_footprint = result + num_bytes;
  assert(needed_footprint > me->vt_total);
  TRY {
   LIST_FOREACH(thread,myvm->vm_tasks,t_vmtasks) {
    size_t tls_footprint; vm_vpage_t expand_base;
    REF struct vm_region *EXCEPT_VAR extend_region;
    struct vm_node *EXCEPT_VAR extend_node;
    if (!(thread->t_flags & TASK_FOWNUSERSEG))
          continue; /* Custom, or no TLS segment. */
    assert(thread->t_userseg_sz >= sizeof(struct user_task_segment));
    assert(IS_ALIGNED((uintptr_t)TASK_USERSEG_BEGIN(thread),PAGESIZE));
    tls_footprint    = thread->t_userseg_sz - sizeof(struct user_task_segment);
    if (tls_footprint >= needed_footprint)
        continue; /* Thread already has sufficient memory. */
    /* Figure out how much is missing (in pages). */
    tls_footprint = needed_footprint - tls_footprint;
    tls_footprint = CEILDIV(tls_footprint,PAGESIZE);
    assert(tls_footprint != 0);
    /* Figure out where the expansion must be mapped. */
    expand_base = VM_ADDR2PAGE((uintptr_t)TASK_USERSEG_BEGIN(thread));
    if unlikely(expand_base < tls_footprint)
       goto memory_in_use; /* Underflow check (shouldn't really happen, but isn't impossible...) */
    expand_base -= tls_footprint;
    /* Check if there already is something mapped there. */
    if (vm_getanynode(expand_base,expand_base + tls_footprint - 1)) {
     /* Yes, there already is something there (fail the allocation) */
memory_in_use:
     error_throwf(E_BADALLOC,ERROR_BADALLOC_VIRTMEMORY,tls_footprint);
    }
    /* All right! we have the go-ahead and can happily map new memory! */
    extend_region = vm_region_alloc(tls_footprint);
    TRY {
     extend_region->vr_init           = VM_REGION_INIT_FFILLER;
     extend_region->vr_setup.s_filler = 0;

     /* Construct the node that will contain the new region. */
     extend_node = (struct vm_node *)kmalloc(sizeof(struct vm_node),
                                             GFP_SHARED|GFP_LOCKED);
     TRY {
      /* Bind the region to the node. */
      extend_node->vn_node.a_vmin = expand_base;
      extend_node->vn_node.a_vmax = expand_base + tls_footprint - 1;
      extend_node->vn_start       = 0;
      extend_node->vn_region      = extend_region;
      extend_node->vn_notify      = &userseg_notify;
      extend_node->vn_closure     = thread;
      extend_node->vn_prot        = PROT_READ|PROT_WRITE;
      extend_node->vn_flag        = VM_NODE_FNORMAL;
      extend_region->vr_part0.vp_refcnt = 1; /* The reference held by the new node. */

      /* Map the node into memory. */
      vm_insert_and_activate_node(myvm,extend_node);

      /* `extend_node' stole this reference. */
      extend_region = NULL;

     } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
      assert(extend_region->vr_part0.vp_refcnt == 1);
      extend_region->vr_part0.vp_refcnt = 0;
      kfree(extend_node);
      error_rethrow();
     }
    } FINALLY {
     if (extend_region)
         vm_region_decref(extend_region);
    }
    /* Try to merge the new segment with what was already there before. */
    vm_merge_before(myvm,expand_base + tls_footprint);

    /* With all that done and complete, update the thread's
     * user-space thread segment base address to indicate
     * the availability of the new memory. */
    thread->t_userseg_sz += tls_footprint * PAGESIZE;
   }
  } FINALLY {
   if (FINALLY_WILL_RETHROW) {
    /* XXX: Undo TLS expansions already performed? */
   }
   atomic_rwlock_endread(&myvm->vm_tasklock);
  }
  /* Adjust the total allocation. */
  me->vt_total = needed_footprint;
 }

insert_in_gap:
 /* Insert the new segment. */
 memmove(&me->vt_vec[i+1],
         &me->vt_vec[i],
         (me->vt_cnt-i)*sizeof(struct vm_tls_entry));
 ++me->vt_cnt;
 /* Fill in the new segment. */
 me->vt_vec[i].te_base     = result;
 me->vt_vec[i].te_size     = num_bytes;
 me->vt_vec[i].te_align    = tls_alignment;
 me->vt_vec[i].te_template = template_base;
 me->vt_vec[i].te_size_tpl = template_size;
 assert(me->vt_total >= result + num_bytes);
 if (me->vt_align < tls_alignment)
     me->vt_align = tls_alignment;
 return result;
}
PUBLIC ATTR_NOTHROW void KCALL tls_free_impl(uintptr_t base) {
 size_t i; u16 state;
 struct vm *myvm = THIS_VM;
 struct vm_tls *me = &FORVM(myvm,my_tls);
 state = ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FDONTSERVE);
 vm_acquire(myvm);
 for (i = 0;; ++i) {
  if (i == me->vt_cnt)
      return; /* Segment may have already been deleted.
               * This can happen due to static TLS expansion,
               * where a single thread can have multiple consecutive
               * TLS segments that aren't merged because merging
               * may have failed arbitrarily. */
  if (me->vt_vec[i].te_base == base) break;
 }
 if (me->vt_vec[i].te_align == me->vt_align) {
  /* This segment has the greatest alignment requirements.
   * Because of that, we must re-calculate the alignment
   * required by the greatest static TLS segment. */
  size_t min_alignment = 1;
  size_t j;
  for (j = 0; j < me->vt_cnt; ++j) {
   if (j == i) continue;
   if (min_alignment < me->vt_vec[j].te_align) {
    min_alignment = me->vt_vec[j].te_align;
    /* Optimization: If there is another segment that matches our
     *               alignment, then we know that the constraints
     *               didn't actually change! */
    if (min_alignment == me->vt_vec[j].te_align)
        goto alignment_updated;
   }
  }
  /* Set the new alignment. */
  me->vt_align = min_alignment;
 }
alignment_updated:
 --me->vt_cnt;
 memmove(&me->vt_vec[i],
         &me->vt_vec[i+1],
         (me->vt_cnt-i)*sizeof(struct vm_tls_entry));
 if (i == me->vt_cnt) {
  /* The greatest segment went away (the
   * required TLS memory footprint is now smaller) */
  if (!me->vt_cnt) {
   me->vt_total = 0;
   assert(me->vt_align == 1);
  }
#if VM_TLS_PREALLOC != 0
  if (me->vt_cnt == VM_TLS_PREALLOC) {
   assert(me->vt_vec != me->vt_sbuf);
   /* Copy data back into the static buffer. */
   memcpy(me->vt_sbuf,me->vt_vec,VM_TLS_PREALLOC*
          sizeof(struct vm_tls_entry));
   kfree(me->vt_vec);
   me->vt_vec = me->vt_sbuf;
  }
#endif
 }
 vm_release(myvm);
 if (!(state & TASK_STATE_FDONTSERVE))
       ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FDONTSERVE);
}


PUBLIC void KCALL
tls_init_impl(uintptr_t base, USER CHECKED void *template_base,
              size_t template_size, size_t num_bytes) {
 struct vm *EXCEPT_VAR myvm = THIS_VM;
 assert(vm_holding_read(myvm));
 assert(template_size <= num_bytes);
 atomic_rwlock_read(&myvm->vm_tasklock);
 TRY {
  struct task *thread;
  LIST_FOREACH(thread,myvm->vm_tasks,t_vmtasks) {
   if (!(thread->t_flags & TASK_FOWNUSERSEG))
         continue; /* Skip threads with a segment not managed by us. */
   /* Copy data from the template */
   memcpy((byte_t *)thread->t_userseg - (base+num_bytes),
          template_base,template_size);
   memset((byte_t *)thread->t_userseg - (base+template_size),
          0,num_bytes - template_size);
  }
 } FINALLY {
  atomic_rwlock_endread(&myvm->vm_tasklock);
 }
}

INTERN void KCALL tls_clear_allocations(void) {
 struct vm_tls *me = &PERVM(my_tls);
 assert(vm_holding_read(THIS_VM));
 /* Clear all allocated static TLS segments. */
#if VM_TLS_PREALLOC != 0
 if (me->vt_vec != me->vt_sbuf)
#endif
 {
  kfree(me->vt_vec);
  me->vt_vec = NULL;
 }
 me->vt_cnt   = 0;
 me->vt_total = 0;
 me->vt_align = 1;
}



/* Assert user-space task segment offsets. */
STATIC_ASSERT(offsetof(struct user_task_segment,ts_self) == TASK_SEGMENT_OFFSETOF_SELF);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_xcurrent) == TASK_SEGMENT_OFFSETOF_XCURRENT);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_state) == USER_TASK_SEGMENT_OFFSETOF_STATE);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_eformat) == USER_TASK_SEGMENT_OFFSETOF_EFORMAT);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_errno) == USER_TASK_SEGMENT_OFFSETOF_ERRNO);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_dos_errno) == USER_TASK_SEGMENT_OFFSETOF_DOS_ERRNO);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_tid) == USER_TASK_SEGMENT_OFFSETOF_TID);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_process) == USER_TASK_SEGMENT_OFFSETOF_PROCESS);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_ueh) == USER_TASK_SEGMENT_OFFSETOF_UEH);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_ueh_sp) == USER_TASK_SEGMENT_OFFSETOF_UEH_SP);
#if defined(__i386__) || defined(__x86_64__)
STATIC_ASSERT(offsetof(struct user_task_segment,ts_x86sysbase) == USER_TASK_SEGMENT_OFFSETOF_X86SYSBASE);
#endif
#ifndef CONFIG_NO_DOS_COMPAT
STATIC_ASSERT(offsetof(struct user_task_segment,ts_tib) == USER_TASK_SEGMENT_OFFSETOF_TIB);
STATIC_ASSERT(offsetof(struct user_task_segment,ts_tib.nt_errno) == USER_TASK_SEGMENT_OFFSETOF_NT_ERRNO);
#endif /* !CONFIG_NO_DOS_COMPAT */


PRIVATE void KCALL
setup_user_segment(USER CHECKED struct user_task_segment *__restrict self) {
 /* Setup default values of the user-space thread segment.
  * NOTE: All other values are pre-initialized to all ZEROes. */
 self->ts_self       = self;
 self->ts_tid        = posix_gettid();
 self->ts_process    = PERVM(vm_environ);
 self->ts_type       = PERTASK_GET(_this_group.tg_leader) == THIS_TASK
                     ? THREAD_TYPE_MAINTHREAD
                     : THREAD_TYPE_WORKER;
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

PUBLIC USER struct user_task_segment *KCALL task_alloc_userseg(void) {
 REF struct vm_region *EXCEPT_VAR region = NULL;
 void *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(user_segment_base);
 size_t sizeof_segment;
 size_t EXCEPT_VAR total_segment_size;
 assert(!PERTASK_TESTF(this_task.t_flags,TASK_FKERNELJOB));
 if (PERTASK_TESTF(this_task.t_flags,TASK_FOWNUSERSEG))
     return PERTASK_GET(this_task.t_userseg);
again:
 vm_acquire_read(THIS_VM);
 TRY {
  sizeof_segment = sizeof_static_tls_segment();
  total_segment_size = sizeof_segment;
  total_segment_size += SIZEOF_USER_TASK_SEGMENT;

  /* Allocate a new region for the user-space thread segment. */
  region = vm_region_alloc(CEILDIV(total_segment_size,PAGESIZE));

  /* Setup user-space segments to be ZERO-initialized. */
  region->vr_init           = VM_REGION_INIT_FFILLER;
  region->vr_setup.s_filler = 0;

  /* Map the stack a suitable location, using arch-specific hints. */
  user_segment_base = vm_map(VM_USERSEG_HINT,
                             CEILDIV(total_segment_size,PAGESIZE),
                             CEILDIV(alignof_static_tls_segment(),PAGESIZE),
                             TLS_GAP,
                             VM_USERSEG_MODE,
                             0,
                             region,
                             PROT_READ|PROT_WRITE,
                            &userseg_notify,
                             THIS_TASK);
  TRY {
   init_tls_segments((byte_t *)user_segment_base);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* Unmap the segment if initialization failed. */
   vm_unmap(VM_ADDR2PAGE((uintptr_t)user_segment_base),
            CEILDIV(total_segment_size,PAGESIZE),
            VM_UNMAP_TAG|VM_UNMAP_NOEXCEPT|VM_UNMAP_SYNC,
            THIS_TASK);
   error_rethrow();
  }
  *(uintptr_t *)&user_segment_base += sizeof_segment;
  PERTASK_SET(this_task.t_userseg,user_segment_base);
  PERTASK_SET(this_task.t_userseg_sz,total_segment_size);
  ATOMIC_FETCHOR(THIS_TASK->t_flags,TASK_FOWNUSERSEG);
  COMPILER_WRITE_BARRIER();
 } FINALLY {
  if (region)
      vm_region_decref(region);
  if (vm_release_read(THIS_VM))
      goto again;
 }
 setup_user_segment((struct user_task_segment *)user_segment_base);
 return (struct user_task_segment *)user_segment_base;
}








DECL_END

#endif /* !GUARD_KERNEL_SRC_VM_REGION_TLS_C */
