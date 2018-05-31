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
#ifndef GUARD_KERNEL_SRC_KERNEL_MALL_C
#define GUARD_KERNEL_SRC_KERNEL_MALL_C 1
#define _KOS_SOURCE 1
#define __OMIT_KMALLOC_CONSTANT_P_WRAPPERS 1
#define __OMIT_HEAP_TRACED_CONSTANT_P_WRAPPERS 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>
#include <kernel/malloc.h>
#if defined(CONFIG_DEBUG_MALLOC) && 1

#if defined(__i386__) || defined(__x86_64__)
#include "../i386-kos/scheduler.h"
#include <i386-kos/ipi.h>
#else
#error "GC-based leak detection is not supposed on this architecture"
#endif

#include <kos/types.h>
#include <hybrid/minmax.h>
#include <hybrid/align.h>
#include <hybrid/list/atree.h>
#include <sched/task.h>
#include <sched/mutex.h>
#include <sched/pid.h>
#include <kos/context.h>
#include <kos/context.h>
#include <unwind/eh_frame.h>
#include <unwind/linker.h>
#include <kernel/heap.h>
#include <except.h>
#include <string.h>
#include <kernel/debug.h>
#include <fs/linker.h>

DECL_BEGIN

/* MALL -- Debug MALLOC API.
 * This one differs from what can be seen in the old KOS significantly.
 * The most important change is that it uses a GC-based memory leak detector,
 * as well as doesn't store file-and-line debug information any more.
 * The traceback stored at the end of the pointer is kept however.
 * Another big change is that MALL information is no longer stored
 * alongside mall pointers themself, but rather accessed through an
 * address TREE with nodes allocated using an independent heap.
 * This way, debug information is even less likely to become corrupt
 * from invalid use of user-pointers.
 * Secondly, free-after-use debug checks are implemented as part of the
 * kernel heap API, rather than only applying to debug-allocated pointers.
 * Thirdly, memory pre-initializations are also done by the heap API,
 * which also does strict enforcing of the state of said data, filling
 * in newly allocated memory with `DEBUGHEAP_FRESH_MEMORY', and clearing
 * all data using `DEBUGHEAP_NO_MANS_LAND' during a call to `free()'. */

#ifndef CONFIG_MALL_HEAD_PATTERN
#define CONFIG_MALL_HEAD_PATTERN   0x33333333
#endif
#ifndef CONFIG_MALL_TAIL_PATTERN
#define CONFIG_MALL_TAIL_PATTERN   0x77777777
#endif
#ifndef CONFIG_MALL_HEAD_SIZE
#define CONFIG_MALL_HEAD_SIZE      HEAP_ALIGNMENT
#endif
#ifndef CONFIG_MALL_TAIL_SIZE
#define CONFIG_MALL_TAIL_SIZE      HEAP_ALIGNMENT
#endif

/* The minimum amount of traceback entries that MALL
 * should attempt to include in debug information of
 * allocated pointers. */
#ifndef CONFIG_MALL_TRACEMIN
#define CONFIG_MALL_TRACEMIN       4
#endif


#define MALL_HEAP_FLAGS  (GFP_SHARED|GFP_LOCKED)

/* Debug-heap used for allocating `struct mallnode' objects. */
PRIVATE struct heap mall_heap = {
    .h_lock       = ATOMIC_RWLOCK_INIT,
    .h_overalloc  = PAGESIZE*4,
    .h_freethresh = PAGESIZE*16,
    .h_hintpage   = VM_KERNEL_MALLHEAP_HINT,
    .h_hintmode   = VM_KERNEL_MALLHEAP_MODE
};

#if __SIZEOF_POINTER__ == 4
typedef u16 mall_ver_t;
#elif __SIZEOF_POINTER__ == 8
typedef u32 mall_ver_t;
#else
#error FIXME
#endif

struct mallnode {
    ATREE_NODE(struct mallnode,uintptr_t) m_tree;     /* Tree of mall nodes. */
    size_t                                m_size;     /* [const] Allocated heap-size of this node. */

    mall_ver_t                            m_reach;    /* Last leak-check iteration when this node was reached. */
    mall_ver_t                            m_visit;    /* Last leak-check iteration when this node was visited. */
#define MALLNODE_FNORMAL                  0x0000      /* Normal MALLNODE flags. */
#define MALLNODE_FLEAKLESS                0x8000      /* This node not being reachable isn't a leak. */
#define MALLNODE_FUSERNODE                0x4000      /* This node describes a custom tracing point. */
    u32                                   m_flags;    /* Set of `MALLNODE_F*' */
    pid_t                                 m_tracepid; /* Traceback process id (in the root namespace). */
    VIRT void                            *m_trace[1]; /* [1..1][0..MALLNODE_TRACESZ(self)]
                                                       * Traceback of where the pointer was originally allocated. */
};
#define MALLNODE_MIN(x)      ((x)->m_tree.a_vmin)
#define MALLNODE_MAX(x)      ((x)->m_tree.a_vmax)
#define MALLNODE_BEGIN(x)    ((x)->m_tree.a_vmin)
#define MALLNODE_END(x)      ((x)->m_tree.a_vmax + 1)
#define MALLNODE_SIZE(x)    (((x)->m_tree.a_vmax - (x)->m_tree.a_vmin) + 1)
#define MALLNODE_TRACESZ(x) (((x)->m_size - offsetof(struct mallnode,m_trace)) / sizeof(void *))

struct mallheap {
    atomic_rwlock_t                  m_lock;   /* Lock for accessing this mall heap. */
    VIRT ATREE_HEAD(struct mallnode) m_tree;   /* Tree of MALL Nodes allocated within this heap. */
};

PRIVATE ATTR_MALL_UNTRACKED
struct mallheap mall_heaps[__GFP_HEAPCOUNT] = {
    [0 ... __GFP_HEAPCOUNT - 1] = {
        .m_lock = ATOMIC_RWLOCK_INIT,
        .m_tree = NULL
    }
};

/* Custom MALL heap for tracing points
 * defined using the `mall_trace()' API. */
PRIVATE ATTR_MALL_UNTRACKED
struct mallheap mall_usertrace = {
    .m_lock = ATOMIC_RWLOCK_INIT,
    .m_tree = NULL
};


DECL_END

/* Define the ABI for the address tree used by mall nodes. */
#define ATREE(x)            mallnode_tree_##x
#define Tkey                VIRT uintptr_t
#define T                   struct mallnode
#define path                m_tree
#include <hybrid/list/atree-abi.h>

DECL_BEGIN

/* The current version number used when searching for memory leaks.
 * This value is used to identity pointers that haven't been reached/search yet. */
PRIVATE mall_ver_t mall_leak_version = 0;
/* The max heap that should be searched for leaks. */
PRIVATE gfp_t mall_leak_heapmax;

/* Set to true to disable lazy memory allocation. */
INTDEF ATTR_PERTASK bool mall_leak_nocore;


/* Analyze a pointer, or data block for reachable pointers,
 * returning the number of reachable mall pointers. */
FORCELOCAL ATTR_NOTHROW size_t KCALL mall_reachable_pointer(void *ptr);
PRIVATE ATTR_NOTHROW size_t KCALL mall_reachable_data(void *base, size_t num_bytes);


FORCELOCAL ATTR_NOTHROW size_t KCALL
mall_reachable_pointer(void *ptr) {
 unsigned int i; struct mallnode *node;
 if likely(!ptr) return 0; /* Optimization: NULL pointer */
#if __SIZEOF_POINTER__ == 4
#ifdef CONFIG_DEBUG_HEAP
 if likely((uintptr_t)ptr == DEBUGHEAP_NO_MANS_LAND) return 0; /* Optimization: No mans land */
 if likely((uintptr_t)ptr == DEBUGHEAP_FRESH_MEMORY) return 0; /* Optimization: Fresh memory. */
#endif /* CONFIG_DEBUG_HEAP */
 if likely((uintptr_t)ptr == 0xcccccccc) return 0; /* Optimization: Stack pre-initialization. */
#endif
 if (((uintptr_t)ptr & (sizeof(void *)-1)) != 0)
     return 0; /* Unaligned pointer -> not a heap pointer */
 if ((uintptr_t)ptr < (uintptr_t)kernel_end &&
     mall_leak_heapmax < GFP_KERNEL)
     return 0; /* There shouldn't be heap mappings between 3Gb and the kernel start... */
 for (i = 0; i <= mall_leak_heapmax; ++i) {
  node = mallnode_tree_locate(mall_heaps[i].m_tree,(uintptr_t)ptr);
  if (!node) continue;
  /* Found a reachable node! */
  if (node->m_reach == mall_leak_version)
      return 0; /* Already reached. */
  node->m_reach = mall_leak_version;
  return 1;
 }
 node = mallnode_tree_locate(mall_usertrace.m_tree,(uintptr_t)ptr);
 if (!node) return 0;
 if (node->m_reach == mall_leak_version)
     return 0; /* Already reached. */
 node->m_reach = mall_leak_version;
 return 1;
}

PRIVATE ATTR_NOTHROW size_t KCALL
mall_reachable_data(void *base, size_t num_bytes) {
 size_t result = 0;
 byte_t *EXCEPT_VAR iter;
 byte_t *EXCEPT_VAR end;
 if unlikely((uintptr_t)base & (sizeof(void *)-1)) {
  /* Align the base pointer. */
  size_t offset;
  offset = sizeof(void *)-((uintptr_t)base & (sizeof(void *)-1));
  if unlikely(offset >= num_bytes) goto done;
  num_bytes -= offset;
 }
 end = (iter = (byte_t *)base)+num_bytes;
continue_search:
 for (; iter < end; iter += sizeof(void *)) {
  TRY {
   void *ptr;
   /* Read a pointer from this location. */
   COMPILER_READ_BARRIER();
   ptr = *(void **)iter;
   COMPILER_READ_BARRIER();
   /* Check if this is a heap pointer, and if so: mark it as reachable. */
   result += mall_reachable_pointer(ptr);
  } CATCH_HANDLED (E_SEGFAULT) {
   /* Page isn't allocated. -> Just skip it! */
   uintptr_t skip_bytes;
   skip_bytes = PAGESIZE-((uintptr_t)iter & (PAGESIZE-1));
   iter += skip_bytes;
   /* Quickly skip consecutive, unmapped pages. */
   while (iter < end && !pagedir_ismapped(VM_ADDR2PAGE((uintptr_t)iter)))
          iter += PAGESIZE;
   goto continue_search;
  }
 }
done:
 return result;
}



INTDEF byte_t mall_tracked_start[];
INTDEF byte_t mall_tracked_end[];

PRIVATE void KCALL
mall_search_task(struct task *__restrict thread) {
 struct cpu_schedcontext *context;
 /* Search the registers of this thread. */
 context = thread->t_context;
 if (X86_ANYCONTEXT_ISUSER(*context)) {
  /* Thread is currently in user-space (meaning its kernel stack is unused) */
 } else {
  unsigned int i;
  /* Search general-purpose registers. */
  for (i = 0; i < (sizeof(struct x86_gpregs)/sizeof(void *)); ++i)
       mall_reachable_pointer(((void **)&context->c_gpregs)[i]);

  if (X86_ANYCONTEXT_USERSP(*context) >  (uintptr_t)thread->t_stackmin &&
      X86_ANYCONTEXT_USERSP(*context) <= (uintptr_t)thread->t_stackend) {
   /* Search the used portion of the kernel stack. */
   mall_reachable_data((void *)X86_ANYCONTEXT_USERSP(*context),
                       (uintptr_t)thread->t_stackend-
                       (uintptr_t)X86_ANYCONTEXT_USERSP(*context));
  } else {
   /* Stack pointer is out-of-bounds (no idea what this is
    * about, but let's just assume the entire stack is allocated) */
   mall_reachable_data(thread->t_stackmin,
                      (uintptr_t)thread->t_stackend-
                      (uintptr_t)thread->t_stackmin);
  }
 }
}

PRIVATE size_t KCALL
scan_reachable(struct mallnode *__restrict node) {
 size_t result = 0;
again:
 if (node->m_reach == mall_leak_version &&
     node->m_visit != mall_leak_version) {
  /* Scan the user-data block of this node. */
  if (node->m_flags & MALLNODE_FUSERNODE) {
   result += mall_reachable_data((void *)node->m_tree.a_vmin,
                                 (node->m_tree.a_vmax-node->m_tree.a_vmin)+1);
  } else {
   result += mall_reachable_data((void *)(node->m_tree.a_vmin + CONFIG_MALL_HEAD_SIZE),
                                 (node->m_tree.a_vmax-node->m_tree.a_vmin)-
                                ((CONFIG_MALL_HEAD_SIZE+CONFIG_MALL_TAIL_SIZE)-1));
  }
  node->m_visit = mall_leak_version;
 }
 if (node->m_tree.a_min) {
  if (node->m_tree.a_max)
      result += scan_reachable(node->m_tree.a_max);
  node = node->m_tree.a_min;
  goto again;
 }
 if (node->m_tree.a_max) {
  node = node->m_tree.a_max;
  goto again;
 }
 return result;
}

#undef CONFIG_PRINT_LEAKS_SEARCH_PHASES
//#define CONFIG_PRINT_LEAKS_SEARCH_PHASES 1

struct vm_corebase {
    /* The corebase system is the base allocator for
     * creating virtual memory mappings with the kernel. */
    ATOMIC_DATA size_t  cb_inuse; /* The number of nodes + regions using this corebase block.
                                   * (+2 for the reserved pair when this is the last
                                   *     block in the internal `corebase_head' chain)
                                   * This field acts as a sort-of reference counter for
                                   * used/valid core slots. */
    LIST_NODE(struct vm_corebase)
                        cb_chain; /* [lock(vm_kernel.vm_lock)] Chain of core-base segments. */
};
/* The initial corebase data block. */
INTDEF struct vm_corebase corebase_initial;



PRIVATE void KCALL mall_search_leaks_impl(void) {
 unsigned int i;
 /* Search for memory leaks.
  * The idea here is not to be able to find all memory blocks that were
  * leaked, but rather to find anything that ~might~ be referenced.
  * To do this, we search all places we can think of:
  *    - Kernel .data & .bss
  *    - Driver .data & .bss
  *    - Stacks and general purpose registers of all other threads
  *    - VMs reachable from those threads (XXX: maybe use kmalloc() for VMs in debug mode?)
  * For this purpose, any properly aligned data word is considered a
  * possible pointer and if directed at a known VM node, that node is
  * saved as reachable.
  * Following this first pass, we recursively analyze the user-data
  * blocks of all heap pointers that were reached thus far, until we're
  * no longer encountering any new ones.
  * Anything that still hasn't been reached is then considered a leak. */
#ifdef CONFIG_PRINT_LEAKS_SEARCH_PHASES
 debug_printf("[LEAK] Phase #1: Scan .data + .bss\n");
#endif /* CONFIG_PRINT_LEAKS_SEARCH_PHASES */
 mall_reachable_data(mall_tracked_start,
                    (size_t)(mall_tracked_end-mall_tracked_start));
 mall_reachable_data(&_boot_task,(size_t)kernel_pertask_size);
 /* Search all threads on all CPUs. */
#ifdef CONFIG_PRINT_LEAKS_SEARCH_PHASES
 debug_printf("[LEAK] Phase #2: Scan running threads\n");
#endif /* CONFIG_PRINT_LEAKS_SEARCH_PHASES */
 for (i = 0; i < cpu_count; ++i) {
  struct cpu *c = cpu_vector[i];
  struct task *iter,*chain = c->c_running;
  iter = chain;
  do {
   mall_reachable_pointer(iter);
   mall_search_task(iter);
  } while ((iter = iter->t_sched.sched_ring.re_next) != chain);
  chain = c->c_sleeping;
  for (; chain; chain = chain->t_sched.sched_list.le_next) {
   mall_reachable_pointer(chain);
   mall_search_task(chain);
  }
#ifndef CONFIG_NO_SMP
  chain = c->c_pending;
  for (; chain; chain = chain->t_sched.sched_slist.le_next) {
   mall_reachable_pointer(chain);
   mall_search_task(chain);
  }
#endif /* !CONFIG_NO_SMP */
 }

#if 0 /* Already search as part of .bss */
 /* Since the boot task isn't allowed using kmalloc(),
  * it will not have counted as a valid thread. */
 mall_search_task(&_boot_task);
#endif


 /* Scan all allocated COREBASE pointers from the kernel VM.
  * Since those are randomly sprinkled into the kernel VM tree,
  * they normally wouldn't be able to forward contained data
  * pointers, which would then result in us not realizing that
  * any dynamic node pointed to by them is actually reachable.
  * NOTE: COREBASE couldn't use `mall_trace()' because that
  *       could cause infinite recursion when `mall_trace()'
  *       tried to allocate a new recursion descriptor.
  */
 { struct vm_corebase *iter = &corebase_initial;
   for (; iter; iter = iter->cb_chain.le_next)
       mall_reachable_data(iter,PAGESIZE);
 }


#ifdef CONFIG_PRINT_LEAKS_SEARCH_PHASES
 debug_printf("[LEAK] Phase #3: Recursively scan reached pointers\n");
#endif /* CONFIG_PRINT_LEAKS_SEARCH_PHASES */
 {
  size_t num_found;
  /* With all data collected, recursively scan
   * the data blocks of all reachable nodes.
   * The recursion takes place because we keep
   * scanning until nothing new shows up.
   */
  do {
   num_found = 0;
   for (i = 0; i <= mall_leak_heapmax; ++i) {
    if (!mall_heaps[i].m_tree) continue;
    num_found += scan_reachable(mall_heaps[i].m_tree);
   }
   if (mall_usertrace.m_tree)
       num_found += scan_reachable(mall_usertrace.m_tree);
#ifdef CONFIG_PRINT_LEAKS_SEARCH_PHASES
   debug_printf("[LEAK] Phase #3: Reached %Iu pointers\n",num_found);
#endif /* CONFIG_PRINT_LEAKS_SEARCH_PHASES */
  } while (num_found);
 }
}


PRIVATE size_t KCALL
print_unreachable(struct mallnode *__restrict node) {
 size_t result = 0;
again:
 if (node->m_reach != mall_leak_version &&
   !(node->m_flags&MALLNODE_FLEAKLESS)) {
  size_t trace_size,i;
  /* This node wasn't reached. */
  debug_printf("%[vinfo:%f(%l,%c) : %n] : %p : Leaked %Iu bytes of memory at %p...%p (PID %p)\n",
              (uintptr_t)node->m_trace[0]-1,node->m_trace[0],
               MALLNODE_SIZE(node)-(CONFIG_MALL_HEAD_SIZE+CONFIG_MALL_TAIL_SIZE),
               MALLNODE_MIN(node)+CONFIG_MALL_HEAD_SIZE,
               MALLNODE_MAX(node)-CONFIG_MALL_TAIL_SIZE,
               node->m_tracepid);
  trace_size = MALLNODE_TRACESZ(node);
  for (i = 1; i < trace_size; ++i) {
   if (!node->m_trace[i]) break;
   debug_printf("%[vinfo:%f(%l,%c) : %n] : %p : Called from here\n",
               (uintptr_t)node->m_trace[i]-1,node->m_trace[i]);
  }
  debug_printf("\n");
  ++result;
 }
 if (node->m_tree.a_min) {
  if (node->m_tree.a_max)
      result += print_unreachable(node->m_tree.a_max);
  node = node->m_tree.a_min;
  goto again;
 }
 if (node->m_tree.a_max) {
  node = node->m_tree.a_max;
  goto again;
 }
 return result;
}

PRIVATE size_t KCALL mall_print_leaks_impl(void) {
 unsigned int i;
 size_t result = 0;
 /* Print all blocks that weren't reached. */
 for (i = 0; i <= mall_leak_heapmax; ++i) {
  if (!mall_heaps[i].m_tree) continue;
  result += print_unreachable(mall_heaps[i].m_tree);
 }
 if (mall_usertrace.m_tree) {
  result += print_unreachable(mall_usertrace.m_tree);
 }
 return result;
}



PUBLIC size_t KCALL mall_dump_leaks(gfp_t heap_max) {
 unsigned int EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(lock_count);
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 bool EXCEPT_VAR has_user_lock;
 bool EXCEPT_VAR has_vm_lock;
 u16 EXCEPT_VAR old_flags;
 assert(PREEMPTION_ENABLED());
 TRY {
  /* Step #1: Acquire locks to all MALL heaps, as well as the kernel VM. */
again_locks:
  lock_count = 0;
  has_vm_lock = false;
  has_user_lock = false;
  COMPILER_BARRIER();
  vm_acquire(&vm_kernel);
  has_vm_lock = true;
  COMPILER_BARRIER();
  /* Ensure that the entire kernel is loaded in-core. */
  if (THIS_VM == &vm_kernel) {
   /* When being hosted by the kernel VM, load
    * the entire address space into the core. */
   vm_unswap(0,VM_VPAGE_MAX+1);
  } else {
   vm_unswap(KERNEL_BASE_PAGE,KERNEL_NUM_PAGES);
  }
  COMPILER_BARRIER();
  for (; lock_count < heap_max; ++lock_count) {
   if (atomic_rwlock_tryread(&mall_heaps[lock_count].m_lock)) continue;
release_heap_locks:
   while (lock_count--)
       atomic_rwlock_endread(&mall_heaps[lock_count].m_lock);
   vm_release(&vm_kernel);
   task_yield();
   goto again_locks;
  }
  if (!atomic_rwlock_tryread(&mall_usertrace.m_lock))
      goto release_heap_locks;
  has_user_lock = true;
  /* Step #2: Set the keep-core flag in the calling thread. */
  old_flags = ATOMIC_FETCHOR(THIS_TASK->t_flags,TASK_FKEEPCORE);
  TRY {
   /* Step #3: Suspend all other CPUs. */
   if (!x86_unicore_begin()) {
    /* If this fails, we must release previously acquired
     * locks in order to prevent a deadlock scenario. */
    if (!(old_flags & TASK_FKEEPCORE))
          ATOMIC_FETCHAND(THIS_TASK->t_flags,~TASK_FKEEPCORE);
    atomic_rwlock_endread(&mall_usertrace.m_lock);
    while (lock_count--)
        atomic_rwlock_endread(&mall_heaps[lock_count].m_lock);
    vm_release(&vm_kernel);
    goto again_locks;
   }
   /* Step #4: Disable preemption (That way, we know we'll be the only thing still running) */
   PREEMPTION_DISABLE();
   TRY {

    /* When not running in the kernel VM, we can only check shared heaps. */
    if (heap_max > (GFP_SHARED|GFP_LOCKED) &&
        THIS_VM != &vm_kernel)
        heap_max = GFP_SHARED|GFP_LOCKED;

    /* Step #4: Actually do the search for memory leaks. */
    mall_leak_heapmax = heap_max; /* Set the max heap to search. */
    ++mall_leak_version;          /* Generate a new iteration version. */
    /* Skip invalid version numbers. */
    if (mall_leak_version == 0)
        mall_leak_version = 1;

    /* Disable loadcore() during #PF
     * Since we loaded the entire kernel into the core above,
     * any page fault that might happen from this point on
     * certainly implies an invalid pointer. */
    PERTASK_SET(mall_leak_nocore,true);
    TRY {
     /* Search for leaks. */
     mall_search_leaks_impl();
     /* Print all found leaks. */
     result = mall_print_leaks_impl();

    } FINALLY {
     /* Always re-enable loadcore() */
     PERTASK_SET(mall_leak_nocore,false);
    }
   } FINALLY {
    PREEMPTION_ENABLE();
    x86_unicore_end();
   }
  } FINALLY {
   if (!(old_flags & TASK_FKEEPCORE))
         ATOMIC_FETCHAND(THIS_TASK->t_flags,~TASK_FKEEPCORE);
  }
 } FINALLY {
  if (has_user_lock)
      atomic_rwlock_endread(&mall_usertrace.m_lock);
  while (lock_count--)
      atomic_rwlock_endread(&mall_heaps[lock_count].m_lock);
  if (has_vm_lock)
      vm_release(&vm_kernel);
 }
 assert(!result);
 return result;
}





PRIVATE ATTR_MALLOC ATTR_RETNONNULL
struct mallnode *KCALL mallnode_alloc(gfp_t gfp) {
 struct mallnode *result;
 struct heapptr ptr;
 ptr = heap_alloc_untraced(&mall_heap,
                           offsetof(struct mallnode,m_trace)+
                          (CONFIG_MALL_TRACEMIN*sizeof(void *)),
                           MALL_HEAP_FLAGS |
                          (gfp & (GFP_NOSWAP|GFP_NOFS|GFP_ATOMIC|
                                  GFP_NOOVER|GFP_NOMAP|GFP_INCORE)));
 result          = (struct mallnode *)ptr.hp_ptr;
 result->m_size  = ptr.hp_siz;
 result->m_flags = MALLNODE_FNORMAL;
 result->m_visit = 0;
 result->m_reach = 0;
 return result;
}

PRIVATE ATTR_NOTHROW void KCALL
mallnode_free(struct mallnode *__restrict self) {
 heap_free_untraced(&mall_heap,self,self->m_size,MALL_HEAP_FLAGS);
}

/* Assert that the header and tail are properly initialized. */
#if !CONFIG_MALL_HEAD_SIZE && !CONFIG_MALL_TAIL_SIZE
#define mallnode_verify_padding(self) (void)0
PUBLIC void KCALL mall_validate_padding(gfp_t UNUSED(heap_max)) { }
#else
PRIVATE void KCALL
mallnode_print_traceback(struct mallnode *__restrict self,
                         pformatprinter printer, void *closure) {
 size_t i,size;
 size = MALLNODE_TRACESZ(self);
 for (i = 0; i < size; ++i) {
  uintptr_t pc = (uintptr_t)self->m_trace[i];
  if (!pc) break;
  format_printf(printer,closure,
                "%[vinfo:%f(%l,%c) : %n] : %p : Allocated from here\n",
                pc-1,pc);
 }
}
PRIVATE void KCALL
mallnode_verify_padding(struct mallnode *__restrict self) {
 u32 *base; unsigned int i;
#if CONFIG_MALL_HEAD_SIZE
 base = (u32 *)self->m_tree.a_vmin;
 for (i = 0; i < CONFIG_MALL_HEAD_SIZE / 4; ++i) {
  if (base[i] != CONFIG_MALL_HEAD_PATTERN) {
   u32 word = CONFIG_MALL_HEAD_PATTERN; base += i;
   while (*(u8 *)base == ((u8 *)&word)[(uintptr_t)base & 3]) ++*(uintptr_t *)&base;
   debug_printf("\n\nCorrupted MALL header in at %p (offset %Id from %p...%p)\n",
                base,(uintptr_t)base - (self->m_tree.a_vmin + CONFIG_MALL_HEAD_SIZE),
                self->m_tree.a_vmin + CONFIG_MALL_HEAD_SIZE,
                self->m_tree.a_vmax+1 - CONFIG_MALL_TAIL_SIZE);
   goto fail;
  }
 }
#endif /* CONFIG_MALL_HEAD_SIZE */
#if CONFIG_MALL_TAIL_SIZE
 base = (u32 *)(self->m_tree.a_vmax+1 - CONFIG_MALL_TAIL_SIZE);
 for (i = 0; i < CONFIG_MALL_TAIL_SIZE / 4; ++i) {
  if (base[i] != CONFIG_MALL_TAIL_PATTERN) {
   u32 word = CONFIG_MALL_TAIL_PATTERN; base += i;
   while (*(u8 *)base == ((u8 *)&word)[(uintptr_t)base & 3]) ++*(uintptr_t *)&base;
   debug_printf("\n\nCorrupted MALL tail in at %p (offset %Id from %p...%p; offset %Iu from end of usable memory)\n",
                base,(uintptr_t)base - (self->m_tree.a_vmin + CONFIG_MALL_HEAD_SIZE),
                self->m_tree.a_vmin + CONFIG_MALL_HEAD_SIZE,
                self->m_tree.a_vmax+1 - CONFIG_MALL_TAIL_SIZE,
               (uintptr_t)base - (self->m_tree.a_vmax+1 - CONFIG_MALL_TAIL_SIZE));
   goto fail;
  }
 }
#endif /* CONFIG_MALL_TAIL_SIZE */
 return;
fail:
 format_hexdump(&debug_printer,NULL,
               (void *)MALLNODE_BEGIN(self),
                MALLNODE_SIZE(self),16,
                FORMAT_HEXDUMP_FLAG_ADDRESS);
 debug_printf("\n");
 mallnode_print_traceback(self,&debug_printer,NULL);
 assertf(0,"Corrupt MALL header or tail");
}
PRIVATE void KCALL
mall_validate_padding_impl(struct mallnode *__restrict self) {
again:
 mallnode_verify_padding(self);
 if (self->m_tree.a_min) {
  if (self->m_tree.a_max)
      mall_validate_padding_impl(self->m_tree.a_max);
  self = self->m_tree.a_min;
  goto again;
 }
 if (self->m_tree.a_max) {
  self = self->m_tree.a_max;
  goto again;
 }
}
PUBLIC void KCALL
mall_validate_padding(gfp_t heap_max) {
 unsigned int EXCEPT_VAR i;
 if (heap_max > (GFP_SHARED|GFP_LOCKED) &&
     THIS_VM != &vm_kernel)
     heap_max = GFP_SHARED|GFP_LOCKED;
 for (i = 0; i <= heap_max; ++i) {
  atomic_rwlock_read(&mall_heaps[i].m_lock);
  TRY {
   if (mall_heaps[i].m_tree)
       mall_validate_padding_impl(mall_heaps[i].m_tree);
  } FINALLY {
   atomic_rwlock_endread(&mall_heaps[i].m_lock);
  }
 }
}
#endif


/* Only unwind ~simple~ stack frames:
 *   - When allocating memory from #PF handlers, we'd otherwise run
 *     into problems when the following chain of events starts allocating
 *     non-locked memory that could cause the dreaded region-split
 *     recursion problems (s.a. `VM_PART_FSPLITTING')
 *     #-4: kmalloc()
 *     #-3: #PF
 *     #-2: loadcore()
 *     #-1: split_region()
 *     # 0: kmalloc(vm_part,GFP_LOCKED) -- This is us!
 *     # 1: linker_findfde()
 *     # 2: module_loadexcept()
 *     # 3: *m_section()
 *     # 4: kmalloc() // Lazy initialization of section names done by ELF (Doesn't use GFP_LOCKED)
 *     # 5: #PF
 *     # 6: loadcore()
 *     # 7: split_region()
 *     # 8: ERROR: There is a chance that the same region
 *                 gets split which isn't allowed.
 * Solution:
 *   - Whatever we do to unwind the stack here, we must
 *     not do it in a way that might trigger additional
 *     loadcore() operations!
 *   - With that, the problem lies in `module_loadexcept()'
 *     not being a function that can safely be called without
 *     causing additional #PF-s
 *
 * Another problem is the following loop that this prevents:
 *     #-2: module_loadexcept()
 *     #-1: *m_section()
 *     # 0: kmalloc()
 *     # 1: linker_findfde()
 *     # 2: module_loadexcept()
 *     # 3: *m_section()
 *     # 4: kmalloc()
 *     # 5: ...
 */
#undef CONFIG_ONLY_SIMPLE_TRACEBACKS
#define CONFIG_ONLY_SIMPLE_TRACEBACKS 1


PRIVATE void KCALL
mall_register(gfp_t flags, struct heapptr pointer,
              struct cpu_context *__restrict allocator_context) {
 /* Must propagate the GFP_LOCKED-flag to prevent an infinite loop. */
 struct mallnode *EXCEPT_VAR node = mallnode_alloc(flags);
 struct mallheap *heap = &mall_heaps[flags & __GFP_HEAPMASK];
#ifdef CONFIG_ONLY_SIMPLE_TRACEBACKS
 assert(!PERTASK_TEST(mall_leak_nocore));
 PERTASK_SET(mall_leak_nocore,true);
#endif
 TRY {
  unsigned int num_skip_frames = 1;
  size_t i,trace_size;
  struct exception_info info;
  memcpy(&info,error_info(),sizeof(struct exception_info));
  i = 0,trace_size = MALLNODE_TRACESZ(node);
  node->m_tracepid = THIS_THREAD_PID ? THIS_THREAD_PID->tp_pids[0] : 0;
#if 1 /* Generate a traceback. */
  TRY {
#ifndef CONFIG_ONLY_SIMPLE_TRACEBACKS
   struct fde_info fde;
#endif
   for (;;) {
    uintptr_t pc = CPU_CONTEXT_IP(*allocator_context);
    /* Skip the first couple of frames. */
    if (num_skip_frames) --num_skip_frames;
    else {
     node->m_trace[i++] = (void *)pc;
     if (i == trace_size) break;
    }
    assert(i < trace_size);
#if (defined(__i386__) || defined(__x86_64__)) && 1
    TRY {
     /* Optimization to walk the stack more quickly. */
     struct frame {
         struct frame *f_caller;
         void         *f_return;
     };
     struct frame *f;
#ifdef __x86_64__
     f = (struct frame *)allocator_context->c_gpregs.gp_rbp;
#else
     f = (struct frame *)allocator_context->c_gpregs.gp_ebp;
#endif
     if ((uintptr_t)f >= (uintptr_t)PERTASK_GET(this_task.t_stackmin) &&
         (uintptr_t)f <  (uintptr_t)PERTASK_GET(this_task.t_stackend) &&
         (uintptr_t)f->f_caller >= (uintptr_t)PERTASK_GET(this_task.t_stackmin) &&
         (uintptr_t)f->f_caller <  (uintptr_t)PERTASK_GET(this_task.t_stackend) &&
         (uintptr_t)f->f_return >= KERNEL_BASE) {
      register register_t temp;
      /* Make sure the supposed return address is valid by reading from it. */
      __asm__ __volatile__("mov %1, %0\n"
                           : "=r" (temp)
                           : "m" (f->f_return)
                           : "memory");
      allocator_context->c_pip           = (uintptr_t)f->f_return;
      allocator_context->c_psp           = (uintptr_t)(f+1);
      allocator_context->c_gpregs.gp_pbp = (uintptr_t)f->f_caller;
      continue; /* Skip the slow FINDFDE and EH_RETURN below. */
     }
    } CATCH_HANDLED (E_SEGFAULT) {
    }
#endif
#ifdef CONFIG_ONLY_SIMPLE_TRACEBACKS
    break;
#else
    if (!linker_findfde(pc - 1,&fde)) break;
    if (!eh_return(&fde,allocator_context,EH_FNORMAL)) break;
#endif
   }
  } CATCH_HANDLED (E_SEGFAULT) {
  }
#endif
  memcpy(error_info(),&info,sizeof(struct exception_info));

  /* NULL-initialize remaining traceback entries. */
  while (i < trace_size) node->m_trace[i++] = NULL;

  /* Setup min/max pointers. */
  node->m_tree.a_vmin = (uintptr_t)pointer.hp_ptr;
  node->m_tree.a_vmax = (uintptr_t)pointer.hp_ptr + pointer.hp_siz - 1;

  /* Insert the node into the heap. */
  atomic_rwlock_write(&heap->m_lock);
  mallnode_tree_insert(&heap->m_tree,node);
  atomic_rwlock_endwrite(&heap->m_lock);
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
#ifdef CONFIG_ONLY_SIMPLE_TRACEBACKS
  PERTASK_SET(mall_leak_nocore,false);
#endif
  /* Free the node on error. */
  mallnode_free(node);
  error_rethrow();
 }
#ifdef CONFIG_ONLY_SIMPLE_TRACEBACKS
 PERTASK_SET(mall_leak_nocore,false);
#endif
}

/* Lookup or remove a MALL node at `mall_pointer'.
 * When `plock' is non-NULL, store a pointer to the R/W-lock
 * that is read-held while accessing the node. Otherwise,
 * remove the node from its heap and return a pointer to it. */
PRIVATE ATTR_NOTHROW ATTR_RETNONNULL struct mallnode *
KCALL mall_lookup(uintptr_t mall_pointer, atomic_rwlock_t **plock,
                  bool exact, unsigned int *pheap) {
 struct mallnode *result;
 unsigned int heap_id,heap_count;
 /* Quick check: Is the pointer even aligned properly */
 if unlikely(!IS_ALIGNED(mall_pointer,HEAP_ALIGNMENT))
    goto bad_pointer;
 heap_count = __GFP_HEAPCOUNT;
 if (THIS_VM != &vm_kernel)
     heap_count = 2; /* Only the first 2 heaps are shared. */
 for (heap_id = 0; heap_id < heap_count; ++heap_id) {
  if (plock) {
   atomic_rwlock_read(&mall_heaps[heap_id].m_lock);
   result = mallnode_tree_locate(mall_heaps[heap_id].m_tree,
                                 mall_pointer);
   if (result) *plock = &mall_heaps[heap_id].m_lock;
   else atomic_rwlock_endread(&mall_heaps[heap_id].m_lock);
  } else {
   atomic_rwlock_write(&mall_heaps[heap_id].m_lock);
   result = mallnode_tree_remove(&mall_heaps[heap_id].m_tree,
                                  mall_pointer);
   atomic_rwlock_endwrite(&mall_heaps[heap_id].m_lock);
  }
  if (result) {
   if (pheap) *pheap = heap_id;
   if (exact && mall_pointer != (result->m_tree.a_vmin + CONFIG_MALL_HEAD_SIZE)) {
    assertf(0,"Malloc pointer %p does not point to the start of node %p...%p\n",
            mall_pointer,
            result->m_tree.a_vmin + CONFIG_MALL_HEAD_SIZE,
            result->m_tree.a_vmax - CONFIG_MALL_TAIL_SIZE);
   }
   return result;
  }
 }
bad_pointer:
 assertf(0,"Bad malloc pointer %p\n",mall_pointer);
}

PUBLIC void KCALL mall_print_traceback(void *ptr) {
 atomic_rwlock_t *lock;
 struct mallnode *node;
 node = mall_lookup((uintptr_t)ptr,&lock,false,NULL);
 mallnode_print_traceback(node,&debug_printer,NULL);
 atomic_rwlock_endread(lock);
}





PRIVATE WUNUSED ATTR_MALLOC ATTR_RETNONNULL
VIRT void *(KCALL mall_alloc)(size_t min_alignment, ptrdiff_t offset,
                              size_t n_bytes, gfp_t EXCEPT_VAR flags,
                              struct cpu_context *__restrict allocator_context) {
 struct heapptr EXCEPT_VAR result;
 result = heap_align_untraced(&kernel_heaps[flags & __GFP_HEAPMASK],
                               min_alignment,CONFIG_MALL_HEAD_SIZE+offset,
                               CONFIG_MALL_HEAD_SIZE+n_bytes+CONFIG_MALL_TAIL_SIZE,
                               flags);
 /* Initialize the debug header and tail. */
#if CONFIG_MALL_HEAD_SIZE != 0
 memsetl((byte_t *)result.hp_ptr,
         CONFIG_MALL_HEAD_PATTERN,
         CONFIG_MALL_HEAD_SIZE / 4);
#endif
#if CONFIG_MALL_TAIL_SIZE != 0
 memsetl((byte_t *)result.hp_ptr+(result.hp_siz-CONFIG_MALL_TAIL_SIZE),
         CONFIG_MALL_TAIL_PATTERN,
         CONFIG_MALL_TAIL_SIZE / 4);
#endif

 /* Register the new pointer. */
 TRY {
  mall_register(flags,result,allocator_context);
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  /* Free the allocated result pointer on error. */
  heap_free_untraced(&kernel_heaps[flags & __GFP_HEAPMASK],
                      result.hp_ptr,result.hp_siz,
                      flags & __GFP_HEAPMASK);
  error_rethrow();
 }
 return (byte_t *)result.hp_ptr + CONFIG_MALL_HEAD_SIZE;
}
PRIVATE WUNUSED ATTR_MALLOC ATTR_RETNONNULL
VIRT void *(KCALL mall_realloc)(VIRT void *ptr,
                                size_t min_alignment, ptrdiff_t offset,
                                size_t n_bytes, gfp_t flags) {
 size_t old_size,new_size;
 struct mallnode *EXCEPT_VAR node;
 unsigned int EXCEPT_VAR heap;
 /* Pop the node from its heap. */
 node = mall_lookup((uintptr_t)ptr,NULL,true,(unsigned int *)&heap);
 mallnode_verify_padding(node);
 new_size = n_bytes + (CONFIG_MALL_HEAD_SIZE + CONFIG_MALL_TAIL_SIZE);
 new_size = CEIL_ALIGN(new_size,HEAP_ALIGNMENT);
 old_size = MALLNODE_SIZE(node);

 TRY {
  if (new_size < old_size) {
   /* Reduce size. */
   old_size -= new_size;
   if (old_size >= HEAP_MINSIZE) {
    heap_free_untraced(&kernel_heaps[heap],
                       (void *)(MALLNODE_BEGIN(node) + new_size),
                        old_size,heap);
    /* Update the size of the node. */
    node->m_tree.a_vmax -= old_size;
    /* Initialize the new tail. */
#if CONFIG_MALL_TAIL_SIZE != 0
    memsetl((byte_t *)MALLNODE_END(node)-CONFIG_MALL_TAIL_SIZE,
            CONFIG_MALL_TAIL_PATTERN,
            CONFIG_MALL_TAIL_SIZE / 4);
#endif
   }
  } else if (new_size > old_size) {
   size_t alloc_size;
   /* Increase size. */
   new_size -= old_size;
   alloc_size = heap_allat_untraced(&kernel_heaps[heap],
                                    (void *)(MALLNODE_BEGIN(node) + old_size),
                                     new_size,(flags & ~__GFP_HEAPMASK) | heap);
   if (alloc_size) {
    if (flags & GFP_CALLOC) {
     /* Must ZERO-initialize the old tail (which is now part of the new pointer) */
#if CONFIG_MALL_TAIL_SIZE != 0
     memsetl((byte_t *)MALLNODE_END(node)-CONFIG_MALL_TAIL_SIZE,
              0,CONFIG_MALL_TAIL_SIZE / 4);
#endif
    } else {
#ifndef NDEBUG
#if CONFIG_MALL_TAIL_SIZE != 0
#ifdef CONFIG_DEBUG_HEAP
     /* The old tail now counts as freshly allocated memory. */
     memsetl((byte_t *)MALLNODE_END(node)-CONFIG_MALL_TAIL_SIZE,
              DEBUGHEAP_FRESH_MEMORY,CONFIG_MALL_TAIL_SIZE / 4);
#endif /* CONFIG_DEBUG_HEAP */
#endif
#endif
    }

    /* Inplace-realloc was successful. */
    node->m_tree.a_vmax += new_size;
#if CONFIG_MALL_TAIL_SIZE != 0
    memsetl((byte_t *)MALLNODE_END(node)-CONFIG_MALL_TAIL_SIZE,
            CONFIG_MALL_TAIL_PATTERN,
            CONFIG_MALL_TAIL_SIZE / 4);
#endif
   } else if (flags & GFP_NOMOVE) {
    /* Inplace re-alloc is not possible. */
    ptr = NULL;
   } else {
    struct heapptr new_block;
    /* Allocate a new memory block. */
    new_size += old_size;
    new_block = heap_alloc_untraced(&kernel_heaps[heap],
                                     new_size,
                                   (flags & ~__GFP_HEAPMASK) | heap);

    /* Transfer data to the new block. */
    memcpy(new_block.hp_ptr,
          (void *)MALLNODE_BEGIN(node),
           MALLNODE_SIZE(node) - CONFIG_MALL_TAIL_SIZE);

    /* Free the old memory block. */
    heap_free_untraced(&kernel_heaps[heap],
                      (void *)MALLNODE_BEGIN(node),
                       MALLNODE_SIZE(node),heap);

    ptr = (VIRT void *)((uintptr_t)new_block.hp_ptr + CONFIG_MALL_HEAD_SIZE);

    /* Set new node pointers according to the newly allocated block. */
    node->m_tree.a_vmin = (uintptr_t)new_block.hp_ptr;
    node->m_tree.a_vmax = (uintptr_t)new_block.hp_ptr + new_block.hp_siz - 1;

    /* Initialize the tail of the new node. */
#if CONFIG_MALL_TAIL_SIZE != 0
    memsetl((byte_t *)MALLNODE_END(node)-CONFIG_MALL_TAIL_SIZE,
            CONFIG_MALL_TAIL_PATTERN,
            CONFIG_MALL_TAIL_SIZE / 4);
#endif
   }
  }
 } FINALLY {
  /* Re-insert the node into its heap. */
  atomic_rwlock_write(&mall_heaps[heap].m_lock);
  mallnode_tree_insert(&mall_heaps[heap].m_tree,node);
  atomic_rwlock_endwrite(&mall_heaps[heap].m_lock);
 }
 return ptr;
}
PUBLIC ATTR_NOTHROW WUNUSED
size_t (KCALL kmalloc_usable_size)(VIRT void *ptr) {
 atomic_rwlock_t *plock;
 size_t result; struct mallnode *node;
 /* NULL-pointers have ZERO(0) usable size. */
 if (!ptr) return 0;
 node = mall_lookup((uintptr_t)ptr,&plock,true,NULL);
 mallnode_verify_padding(node);
 result = MALLNODE_SIZE(node) - (CONFIG_MALL_HEAD_SIZE + CONFIG_MALL_TAIL_SIZE);
 atomic_rwlock_endread(plock);
 return result;
}
PRIVATE void (KCALL mall_free)(VIRT void *ptr, gfp_t flags) {
 struct mallnode *node; unsigned int heap;
 /* Ignore NULL-pointers. */
 if (!ptr) return;
 /* Remove the node from its heap. */
 node = mall_lookup((uintptr_t)ptr,NULL,true,&heap);
 mallnode_verify_padding(node);

 if (flags & GFP_CALLOC) {
  /* Clear the header and tail. */
#if CONFIG_MALL_HEAD_SIZE != 0
  memsetl((byte_t *)MALLNODE_BEGIN(node),
           0,CONFIG_MALL_HEAD_SIZE / 4);
#endif
#if CONFIG_MALL_TAIL_SIZE != 0
  memsetl((byte_t *)MALLNODE_END(node)-CONFIG_MALL_TAIL_SIZE,
           0,CONFIG_MALL_TAIL_SIZE / 4);
#endif
 }

 /* Free the memory pointed to by the node. */
 heap_free_untraced(&kernel_heaps[heap],
                   (void *)MALLNODE_BEGIN(node),MALLNODE_SIZE(node),
                   (flags & ~__GFP_HEAPMASK) | (gfp_t)heap);
 /* Free the node itself. */
 mallnode_free(node);
}


/* User-defined tracing points API */
PRIVATE void KCALL
define_user_traceing_point(void *base, size_t num_bytes, gfp_t gfp, u32 flags) {
 struct mallnode *node = mallnode_alloc(gfp);
 assertf(num_bytes,"Cannot define an empty tracing point");
 node->m_tree.a_vmin = (uintptr_t)base;
 node->m_tree.a_vmax = (uintptr_t)base + num_bytes-1;
 node->m_flags       = flags;
 /* Insert the new node into the tree of user-defined tracing points. */
 atomic_rwlock_write(&mall_usertrace.m_lock);
 mallnode_tree_insert(&mall_usertrace.m_tree,node);
 atomic_rwlock_endwrite(&mall_usertrace.m_lock);
}
PUBLIC ATTR_NOTHROW void KCALL
mall_untrace(void *ptr) {
 struct mallnode *node;
 atomic_rwlock_write(&mall_usertrace.m_lock);
 node = mallnode_tree_remove(&mall_usertrace.m_tree,(uintptr_t)ptr);
 atomic_rwlock_endwrite(&mall_usertrace.m_lock);
 assertf(node != NULL,"No tracing point at %p",ptr);
 mallnode_free(node);
}

PUBLIC void KCALL
mall_trace(void *base, size_t num_bytes, gfp_t flags) {
 define_user_traceing_point(base,num_bytes,flags,MALLNODE_FUSERNODE);
}
PUBLIC void KCALL
mall_trace_leakless(void *base, size_t num_bytes, gfp_t flags) {
 define_user_traceing_point(base,num_bytes,flags,
                            MALLNODE_FUSERNODE|
                            MALLNODE_FLEAKLESS);
}











PUBLIC WUNUSED ATTR_MALLOC ATTR_RETNONNULL
VIRT void *(KCALL kmalloc)(size_t n_bytes,
                           gfp_t flags) {
 struct cpu_context context;
 cpu_getcontext(&context);
 return mall_alloc(HEAP_ALIGNMENT,0,n_bytes,flags,&context);
}
PUBLIC WUNUSED ATTR_MALLOC ATTR_RETNONNULL
VIRT void *(KCALL kmemalign)(size_t min_alignment,
                             size_t n_bytes,
                             gfp_t flags) {
 struct cpu_context context;
 cpu_getcontext(&context);
 return mall_alloc(min_alignment,0,n_bytes,flags,&context);
}
PUBLIC WUNUSED ATTR_MALLOC ATTR_RETNONNULL
VIRT void *(KCALL kmemalign_offset)(size_t min_alignment, ptrdiff_t offset,
                                    size_t n_bytes, gfp_t flags) {
 struct cpu_context context;
 cpu_getcontext(&context);
 return mall_alloc(min_alignment,offset,n_bytes,flags,&context);
}
PUBLIC VIRT void *(KCALL krealloc)(VIRT void *ptr, size_t n_bytes, gfp_t flags) {
 /* Special case: When `ptr' is NULL, allocate new memory. */
 if (!ptr) {
  struct cpu_context context;
  if (flags & GFP_NOMOVE) return NULL;
  cpu_getcontext(&context);
  return mall_alloc(HEAP_ALIGNMENT,0,n_bytes,flags,&context);
 }
 return mall_realloc(ptr,HEAP_ALIGNMENT,0,n_bytes,flags);
}
PUBLIC VIRT void *(KCALL krealign)(VIRT void *ptr,
                                   size_t min_alignment,
                                   size_t n_bytes, gfp_t flags) {
 if (!ptr) {
  struct cpu_context context;
  if (flags & GFP_NOMOVE) return NULL;
  cpu_getcontext(&context);
  return mall_alloc(min_alignment,0,n_bytes,flags,&context);
 }
 return mall_realloc(ptr,min_alignment,0,n_bytes,flags);
}
PUBLIC VIRT void *(KCALL krealign_offset)(VIRT void *ptr, size_t min_alignment,
                                          ptrdiff_t offset, size_t n_bytes, gfp_t flags) {
 if (!ptr) {
  struct cpu_context context;
  if (flags & GFP_NOMOVE) return NULL;
  cpu_getcontext(&context);
  return mall_alloc(min_alignment,offset,n_bytes,flags,&context);
 }
 return mall_realloc(ptr,min_alignment,offset,n_bytes,flags);
}
PUBLIC ATTR_NOTHROW void (KCALL kfree)(VIRT void *ptr) {
 mall_free(ptr,GFP_NORMAL);
}
PUBLIC void (KCALL kffree)(VIRT void *ptr, gfp_t flags) {
 mall_free(ptr,flags);
}



/* Define traced versions of heap allocator functions. */
PUBLIC struct heapptr KCALL
heap_alloc(struct heap *__restrict self,
           size_t num_bytes, gfp_t flags) {
 struct heap *EXCEPT_VAR xself = self;
 gfp_t EXCEPT_VAR xflags = flags;
 struct heapptr EXCEPT_VAR result;
 /* Allocate the new data block. */
 result = heap_alloc_untraced(self,num_bytes,flags);
 TRY {
  /* Start tracing the new data block. */
  mall_trace(result.hp_ptr,result.hp_siz,flags);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Free the allocated data block on error. */
  heap_free_untraced(xself,
                     result.hp_ptr,
                     result.hp_siz,
                     xflags);
  error_rethrow();
 }
 return result;
}
PUBLIC struct heapptr KCALL
heap_align(struct heap *__restrict self,
           size_t min_alignment, ptrdiff_t offset,
           size_t num_bytes, gfp_t flags) {
 struct heap *EXCEPT_VAR xself = self;
 gfp_t EXCEPT_VAR xflags = flags;
 struct heapptr EXCEPT_VAR result;
 /* Allocate the new data block. */
 result = heap_align_untraced(self,min_alignment,
                              offset,num_bytes,flags);
 TRY {
  /* Start tracing the new data block. */
  mall_trace(result.hp_ptr,result.hp_siz,flags);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Free the allocated data block on error. */
  heap_free_untraced(xself,
                     result.hp_ptr,
                     result.hp_siz,
                     xflags);
  error_rethrow();
 }
 return result;
}
PUBLIC size_t KCALL
heap_allat(struct heap *__restrict self,
           VIRT void *__restrict ptr,
           size_t num_bytes, gfp_t flags) {
 struct heap *EXCEPT_VAR xself = self;
 VIRT void *EXCEPT_VAR xptr = ptr;
 gfp_t EXCEPT_VAR xflags = flags;
 size_t EXCEPT_VAR result;
 /* Allocate the new data block. */
 result = heap_allat_untraced(self,ptr,num_bytes,flags);
 TRY {
  /* Start tracing the new data block. */
  mall_trace(ptr,result,flags);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Free the allocated data block on error. */
  heap_free_untraced(xself,xptr,result,xflags);
  error_rethrow();
 }
 return result;
}
PUBLIC void KCALL
heap_free(struct heap *__restrict self,
          VIRT void *ptr, size_t num_bytes,
          gfp_t flags) {
 mall_untrace(ptr);
 heap_free_untraced(self,ptr,num_bytes,flags);
}



PUBLIC struct heapptr KCALL
heap_realloc(struct heap *__restrict self,
             VIRT void *old_ptr, size_t old_bytes,
             size_t new_bytes, gfp_t alloc_flags,
             gfp_t free_flags) {
 struct heap *EXCEPT_VAR xself = self;
 gfp_t EXCEPT_VAR xalloc_flags = alloc_flags;
 struct heapptr EXCEPT_VAR result; size_t missing_bytes;
 assert(IS_ALIGNED(old_bytes,HEAP_ALIGNMENT));
 assert(!old_bytes || IS_ALIGNED((uintptr_t)old_ptr,HEAP_ALIGNMENT));
 assert(!old_bytes || old_bytes >= HEAP_MINSIZE);
 if (old_bytes == 0) /* Special case: initial allocation */
     return heap_alloc(self,new_bytes,alloc_flags);
 new_bytes = CEIL_ALIGN(new_bytes,HEAP_ALIGNMENT);
 result.hp_ptr = old_ptr;
 result.hp_siz = old_bytes;
 if (new_bytes <= old_bytes) {
  size_t free_bytes;
  if unlikely(new_bytes < HEAP_MINSIZE)
              new_bytes = HEAP_MINSIZE;
  /* Free trailing memory. */
  free_bytes = old_bytes - new_bytes;
  if (free_bytes >= HEAP_MINSIZE) {
   heap_free(self,(void *)((uintptr_t)old_ptr+new_bytes),
             free_bytes,free_flags);
   result.hp_siz = new_bytes;
  }
  return result;
 }
 missing_bytes = new_bytes - old_bytes;
 missing_bytes = heap_allat(self,(void *)((uintptr_t)old_ptr+old_bytes),
                            missing_bytes,alloc_flags);
 if (missing_bytes) {
  /* Managed to extend the data block. */
  result.hp_siz += missing_bytes;
  return result;
 }
 /* Must allocate an entirely new data block and copy memory to it. */
 result = heap_alloc(self,new_bytes,alloc_flags);
 TRY {
  /* The try block is here because of the possibility of a LOA failure. */
  memcpy(result.hp_ptr,old_ptr,old_bytes);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  heap_free(xself,
            result.hp_ptr,
            result.hp_siz,
            xalloc_flags & ~GFP_CALLOC);
  error_rethrow();
 }
 /* Free the old data block. */
 heap_free(self,old_ptr,old_bytes,
           free_flags & ~GFP_CALLOC);
 return result;
}
PUBLIC struct heapptr KCALL
heap_realign(struct heap *__restrict self,
             VIRT void *old_ptr, size_t old_bytes,
             size_t min_alignment, ptrdiff_t offset,
             size_t new_bytes, gfp_t alloc_flags,
             gfp_t free_flags) {
 struct heap *EXCEPT_VAR xself = self;
 gfp_t EXCEPT_VAR xalloc_flags = alloc_flags;
 struct heapptr EXCEPT_VAR result; size_t missing_bytes;
 assert(IS_ALIGNED(old_bytes,HEAP_ALIGNMENT));
 assert(!old_bytes || IS_ALIGNED((uintptr_t)old_ptr,HEAP_ALIGNMENT));
 assert(!old_bytes || old_bytes >= HEAP_MINSIZE);
 if (old_bytes == 0) /* Special case: initial allocation */
     return heap_align(self,min_alignment,offset,new_bytes,alloc_flags);
 new_bytes = CEIL_ALIGN(new_bytes,HEAP_ALIGNMENT);
 result.hp_ptr = old_ptr;
 result.hp_siz = old_bytes;
 if (new_bytes <= old_bytes) {
  size_t free_bytes;
  if unlikely(new_bytes < HEAP_MINSIZE)
              new_bytes = HEAP_MINSIZE;
  /* Free trailing memory. */
  free_bytes = old_bytes - new_bytes;
  if (free_bytes >= HEAP_MINSIZE) {
   heap_free(self,(void *)((uintptr_t)old_ptr+new_bytes),
             free_bytes,free_flags);
   result.hp_siz = new_bytes;
  }
  return result;
 }
 missing_bytes = new_bytes - old_bytes;
 missing_bytes = heap_allat(self,(void *)((uintptr_t)old_ptr+old_bytes),
                            missing_bytes,alloc_flags);
 if (missing_bytes) {
  /* Managed to extend the data block. */
  result.hp_siz += missing_bytes;
  return result;
 }
 /* Must allocate an entirely new data block and copy memory to it. */
 result = heap_align(self,min_alignment,offset,new_bytes,alloc_flags);
 TRY {
  /* The try block is here because of the possibility of a LOA failure. */
  memcpy(result.hp_ptr,old_ptr,old_bytes);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  heap_free(xself,
            result.hp_ptr,
            result.hp_siz,
            xalloc_flags & ~GFP_CALLOC);
  error_rethrow();
 }
 /* Free the old data block. */
 heap_free(self,old_ptr,old_bytes,
           free_flags & ~GFP_CALLOC);
 return result;
}




DECL_END
#endif /* CONFIG_DEBUG_MALLOC */

#endif /* !GUARD_KERNEL_SRC_KERNEL_MALL_C */
