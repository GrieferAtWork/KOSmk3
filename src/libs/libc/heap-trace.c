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
#ifndef GUARD_LIBS_LIBC_HEAP_TRACE_C
#define GUARD_LIBS_LIBC_HEAP_TRACE_C 1
#define __EXPOSE_HEAP_INTERNALS 1
#define __OMIT_HEAP_CONSTANT_P_WRAPPERS 1
#define CONFIG_DEBUG_HEAP 1

#include "libc.h"
#include "format.h"
#include "system.h"
#include "heap.h"
#include "errno.h"
#include "sync.h"
#include "sched.h"
#include "unistd.h"

#include <kos/heap.h>
#include <kos/sched/mutex.h>
#include <kos/gc.h>
#include <syslog.h>
#include <format-printer.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/section.h>
#include <stddef.h>
#include <stdio.h>

/* The HEAP tracing API is very similar to the kernel's MALL API.
 * For a better explanation of what happens here,
 * take a look at `src/kernel/src/kernel/mall.c'. */

DECL_BEGIN

INTERN size_t LIBCCALL
libc_gc_search(struct gc_specs const *__restrict specs, unsigned int flags,
               struct gc_data *__restrict data, gc_ver_t current_version) {
 return FORWARD_SYSTEM_VALUE(sys_xgc_search(specs,flags,data,current_version));
}
EXPORT(gc_search,libc_gc_search);

INTERN size_t LIBCCALL
libc_capture_traceback(struct cpu_context *__restrict ctx,
                       unsigned int num_skipframes,
                       void **ptb, size_t max_entries) {
 ssize_t result;
 result = sys_xcapture_traceback(ctx,num_skipframes,ptb,max_entries*sizeof(void *));
 result = FORWARD_SYSTEM_VALUE(result);
 if unlikely(result < 0) result = 0;
 return result;
}

#ifndef CONFIG_MALL_TRACEMIN
#define CONFIG_MALL_TRACEMIN       4
#endif


#define LOCAL_HEAP_ALIGNMENT   __HEAP_GET_DEFAULT_ALIGNMENT(HEAP_TYPE_FNORMAL)
#if LOCAL_HEAP_ALIGNMENT == 4
#   define LOCAL_HEAP_BUCKET_OFFSET  3 /* FFS(LOCAL_HEAP_ALIGNMENT) */
#elif LOCAL_HEAP_ALIGNMENT == 8
#   define LOCAL_HEAP_BUCKET_OFFSET  4 /* FFS(LOCAL_HEAP_ALIGNMENT) */
#elif LOCAL_HEAP_ALIGNMENT == 16
#   define LOCAL_HEAP_BUCKET_OFFSET  5 /* FFS(LOCAL_HEAP_ALIGNMENT) */
#elif LOCAL_HEAP_ALIGNMENT == 32
#   define LOCAL_HEAP_BUCKET_OFFSET  6 /* FFS(LOCAL_HEAP_ALIGNMENT) */
#else
#error "Invalid default alignment"
#endif
#define LOCAL_HEAP_BUCKET_COUNT   ((__SIZEOF_SIZE_T__*8)-(LOCAL_HEAP_BUCKET_OFFSET-1))


struct local_heap {
    u16                       h_type;       /* [== HEAP_TYPE_FCURRENT][const] The type of heap. */
    u16                       h_flags;      /* Heap flags (Set of `HEAP_F*'). */
#if __SIZEOF_POINTER__ > 4
    u16                     __h_pad[(sizeof(void *)-4)/2]; /* ... */
#endif
    atomic_rwlock_t           h_lock;       /* Lock for this heap. */
    ATREE_HEAD(struct mfree)  h_addr;       /* [lock(h_lock)][0..1] Heap sorted by address. */
    LIST_HEAD(struct mfree)   h_size[LOCAL_HEAP_BUCKET_COUNT];
                                            /* [lock(h_lock)][0..1][*] Heap sorted by free range size. */
    WEAK size_t               h_overalloc;  /* Amount (in bytes) by which to over-allocate memory in heaps.
                                             * NOTE: Set to ZERO(0) to disable overallocation. */
    WEAK size_t               h_freethresh; /* Threshold that must be reached before any continuous block of free
                                             * data is unmapped from the kernel VM. (Should always be `>= PAGESIZE') */
    WEAK void                *h_corehint;   /* [valid_if(HEAP_FUSEHINTS)] Hint for where to allocate new memory. */
    ATOMIC_DATA size_t        h_dangle;     /* [lock(INCREMENT(h_lock),DECREMENT(atomic),READ(atomic))]
                                             * Amount of dangling bytes of memory (memory that was allocated, but may be
                                             * release again shortly) When new memory would have to be requested from the
                                             * core, this field is checked to see if it is likely that some large block
                                             * of memory will be released soon, preventing a race condition that would
                                             * unnecessarily allocate more memory when `heap_free()' is merging a data
                                             * block with another, larger data block, for which it must temporarily
                                             * allocate that larger data block. Another thread allocating memory at the
                                             * same time may then think that the cache has grown too small for the allocation
                                             * and unnecessarily request more memory from the core. */
};                            

#ifdef __x86_64__
#error TODO
#elif defined(__i386__)
#define KERNEL_BASE      0xc0000000
#define HEAP_TRACE_HINT  0xbfac0000
#define HEAP_TRACE_FLAGS HEAP_FDOWNHINT
#else
#error "Unsuported architecture."
#endif


/* The heap used for allocating heap tracing points. */
INTERN struct local_heap libc_heap_trace_allocator = {
    .h_type       = HEAP_TYPE_FNORMAL,
    .h_flags      = HEAP_FUSEHINTS|HEAP_TRACE_FLAGS,
    .h_lock       = ATOMIC_RWLOCK_INIT,
    .h_addr       = NULL,
    .h_size       = { NULL, },
    .h_overalloc  = PAGESIZE*2,
    .h_freethresh = PAGESIZE*8,
    .h_corehint   = (void *)HEAP_TRACE_HINT,
    .h_dangle     = 0
};

INTDEF struct heap *libc_debug_heaps;
INTDEF atomic_rwlock_t libc_debug_heaps_lock;


#if __SIZEOF_POINTER__ == 4
typedef u16 heap_ver_t;
#elif __SIZEOF_POINTER__ == 8
typedef u32 heap_ver_t;
#else
#error FIXME
#endif

struct heapnode {
    ATREE_NODE(struct heapnode,uintptr_t) m_tree;     /* Tree of heap nodes. */
    size_t                                m_size;     /* [const] Allocated heap-size of this node. */
    heap_ver_t                            m_reach;    /* Last leak-check iteration when this node was reached. */
    heap_ver_t                            m_visit;    /* Last leak-check iteration when this node was visited. */
#define HEAPNODE_FNORMAL                  0x0000      /* Normal HEAPNODE flags. */
#define HEAPNODE_FROOT                    0x4000      /* This node is a GC root node. */
#define HEAPNODE_FLEAKLESS                0x8000      /* This node not being reachable isn't a leak. */
    u32                                   m_flags;    /* Set of `HEAPNODE_F*' */
    LIST_NODE(struct heapnode)            m_root;     /* [valid_if(HEAPNODE_FROOT)] Chain of root nodes. */
    pid_t                                 m_tid;      /* TID of the thread that allocated the memory. */
    void                                 *m_trace[1]; /* [1..1][0..HEAPNODE_TRACESZ(self)]
                                                       * Traceback of where the pointer was originally allocated. */
};
#define HEAPNODE_MIN(x)      ((x)->m_tree.a_vmin)
#define HEAPNODE_MAX(x)      ((x)->m_tree.a_vmax)
#define HEAPNODE_BEGIN(x)    ((x)->m_tree.a_vmin)
#define HEAPNODE_END(x)      ((x)->m_tree.a_vmax + 1)
#define HEAPNODE_SIZE(x)    (((x)->m_tree.a_vmax - (x)->m_tree.a_vmin) + 1)
#define HEAPNODE_TRACESZ(x) (((x)->m_size - offsetof(struct heapnode,m_trace)) / sizeof(void *))

DECL_END

/* Define the ABI for the address tree used by mall nodes. */
#define ATREE(x)            heapnode_tree_##x
#define Tkey                VIRT uintptr_t
#define T                   struct heapnode
#define path                m_tree
#include <hybrid/list/atree-abi.h>

DECL_BEGIN

struct trace_data_struct {
    /* NOTE: This structure _MUST_ be compatible with `struct gc_data' */
    ATREE_HEAD(struct heapnode) trace_tree;
    LIST_HEAD(struct heapnode)  trace_root;
};

/* Global variables implementing heap tracing. */
PRIVATE mutex_t trace_lock = MUTEX_INIT;

/* NOTE: The 2 pointers found in `trace_data' are special because
 *       they are not searched when scanning .data / .bss! */
PRIVATE struct trace_data_struct trace_data = { NULL, NULL };



/* GC Specifications for tracing data. */
PRIVATE struct gc_specs const trace_specs = {
    .gs_node = {
        .n_offsetof_atree = offsetof(struct heapnode,m_tree),
        .n_offsetof_reach = offsetof(struct heapnode,m_reach),
        .n_offsetof_visit = offsetof(struct heapnode,m_visit),
        .n_next_NULL      = NULL
    },
    .gs_root = {
        .r_offsetof_addrmin = offsetof(struct heapnode,m_tree.a_vmin),
        .r_offsetof_addrmax = offsetof(struct heapnode,m_tree.a_vmax),
        .r_offsetof_next    = offsetof(struct heapnode,m_root.le_next),
        .r_next_NULL        = NULL
    },
};


PRIVATE void LIBCCALL
register_node(struct heapnode *__restrict node) {
 while (libc_mutex_get_timed64(&trace_lock,NULL));
 /* Insert the node into the HEAPNODE tree. */
 heapnode_tree_insert(&trace_data.trace_tree,node);
 libc_mutex_put(&trace_lock);
}

PRIVATE void LIBCCALL
register_root_node(struct heapnode *__restrict node) {
 while (libc_mutex_get_timed64(&trace_lock,NULL));
 /* Insert the node into the HEAPNODE tree. */
 heapnode_tree_insert(&trace_data.trace_tree,node);
 /* Also add the node to the chain of root nodes. */
 LIST_INSERT(trace_data.trace_root,node,m_root);
 libc_mutex_put(&trace_lock);
}

PRIVATE ATTR_NOTHROW struct heapnode *
LIBCCALL pop_node(uintptr_t addr) {
 struct heapnode *result;
 TRY {
  while (libc_mutex_get_timed64(&trace_lock,NULL));
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  libc_error_printf("Failed to acquire TRACE lock for %p\n",addr);
  return NULL;
 }
 /* Insert the node into the HEAPNODE tree. */
 result = heapnode_tree_remove(&trace_data.trace_tree,addr);
 /* If it's a root node, also remove it from chain chain. */
 if (result && (result->m_flags & HEAPNODE_FROOT))
     LIST_REMOVE(result,m_root);
 libc_mutex_put(&trace_lock);
 return result;
}



INTERN void LIBCCALL
libc_heap_trace(void *base, size_t num_bytes,
                unsigned int num_skipframes) {
 struct cpu_context context;
 struct heapnode *node;
 struct heapptr node_ptr;
 size_t num_frames,max_frames;
 libc_cpu_getcontext(&context);
 node_ptr = libc_heap_alloc_untraced(&libc_heap_trace_allocator,
                                      offsetof(struct heapnode,m_trace)+
                                      CONFIG_MALL_TRACEMIN*sizeof(void *),0);
 LIBC_TRY {
  node = (struct heapnode *)node_ptr.hp_ptr;
  node->m_tree.a_vmin = (uintptr_t)base;
  node->m_tree.a_vmax = (uintptr_t)base+num_bytes-1;
  node->m_size        = node_ptr.hp_siz;
  node->m_reach       = 0;
  node->m_visit       = 0;
  node->m_flags       = HEAPNODE_FNORMAL;
  node->m_tid         = libc_gettid();
  /* Capture a traceback for this node. */
  max_frames = HEAPNODE_TRACESZ(node);
  num_frames = libc_capture_traceback(&context,num_skipframes+1,
                                       node->m_trace,max_frames);
  /* ZERO-initialize all unused frames IPs. */
  libc_memset(node->m_trace+num_frames,0,
             (max_frames-num_frames)*sizeof(char));

  /* Register the node in the TRACE GC tree. */
  register_node(node);
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Re-free the node on error. */
  libc_heap_free_untraced(&libc_heap_trace_allocator,
                 node_ptr.hp_ptr,node_ptr.hp_siz,0);
  libc_error_rethrow();
 }
}

INTERN void LIBCCALL
libc_heap_trace_root(void *base, size_t num_bytes) {
 struct heapnode *node;
 struct heapptr node_ptr;
 node_ptr = libc_heap_alloc_untraced(&libc_heap_trace_allocator,
                            offsetof(struct heapnode,m_tid),
                            0);
 node = (struct heapnode *)node_ptr.hp_ptr;
 node->m_tree.a_vmin = (uintptr_t)base;
 node->m_tree.a_vmax = (uintptr_t)base+num_bytes-1;
 node->m_size        = node_ptr.hp_siz;
 node->m_reach       = 0;
 node->m_visit       = 0;
 node->m_flags       = HEAPNODE_FROOT;
 /* Register the node in the TRACE GC tree. */
 register_root_node(node);
}

INTERN void LIBCCALL
libc_heap_trace_leakless(void *base, size_t num_bytes) {
 struct heapnode *node;
 struct heapptr node_ptr;
 node_ptr = libc_heap_alloc_untraced(&libc_heap_trace_allocator,
                            offsetof(struct heapnode,m_tid),
                            0);
 node = (struct heapnode *)node_ptr.hp_ptr;
 node->m_tree.a_vmin = (uintptr_t)base;
 node->m_tree.a_vmax = (uintptr_t)base+num_bytes-1;
 node->m_size        = node_ptr.hp_siz;
 node->m_reach       = 0;
 node->m_visit       = 0;
 node->m_flags       = HEAPNODE_FLEAKLESS;
 /* Register the node in the TRACE GC tree. */
 register_node(node);
}

INTERN ATTR_NOTHROW void LIBCCALL
libc_heap_untrace(void *ptr) {
 struct heapnode *node;
 node = pop_node((uintptr_t)ptr);
 assertf(node,"No tracing point at %p",ptr);
 /* Also free the tracing point. */
 libc_heap_free_untraced(&libc_heap_trace_allocator,
                 node,node->m_size,0);
}



PRIVATE ATTR_COLDTEXT void LIBCCALL
reset_gc_versions(struct heapnode *__restrict node) {
again:
 node->m_reach = node->m_visit = 0;
 if (node->m_tree.a_min) {
  if (node->m_tree.a_max)
      reset_gc_versions(node->m_tree.a_max);
  node = node->m_tree.a_min;
  goto again;
 }
 if (node->m_tree.a_max) {
  node = node->m_tree.a_max;
  goto again;
 }
}

PRIVATE ATTR_COLDTEXT ssize_t LIBCCALL
enum_unreachable(struct heapnode *__restrict node, gc_ver_t version,
                 pfindleakscallback func, void *closure) {
 ssize_t temp,result = 0;
again:
 if (node->m_reach != version &&
   !(node->m_flags & (HEAPNODE_FROOT|HEAPNODE_FLEAKLESS))) {
  size_t trace_size;
  /* This one's not reachable. */
  trace_size = HEAPNODE_TRACESZ(node);
  while (trace_size && !node->m_trace[trace_size-1]) --trace_size;
  temp = (*func)((void *)node->m_tree.a_vmin,
                 (node->m_tree.a_vmax-node->m_tree.a_vmin)+1,
                  node->m_tid,trace_size,node->m_trace,closure);
  if (temp < 0) return temp;
  result += temp;
 }
 if (node->m_tree.a_min) {
  if (node->m_tree.a_max) {
   temp = enum_unreachable(node->m_tree.a_max,version,func,closure);
   if (temp < 0) return temp;
   result += temp;
  }
  node = node->m_tree.a_min;
  goto again;
 }
 if (node->m_tree.a_max) {
  node = node->m_tree.a_max;
  goto again;
 }
 return result;
}


INTERN ssize_t LIBCCALL
libc_heap_find_leaks(pfindleakscallback func, void *closure) {
 PRIVATE gc_ver_t gc_version = 0;
 ssize_t result = 0;
 while (libc_mutex_get_timed64(&trace_lock,NULL));
 LIBC_TRY {
  gc_ver_t next_version = ++gc_version;
  if unlikely(next_version == 0) {
   /* Reset version numbers of all traced data nodes. */
   if (trace_data.trace_tree)
       reset_gc_versions(trace_data.trace_tree);
  }
  /* Invoke the kernel to search for reachable data nodes. */
  libc_gc_search(&trace_specs,
                 GC_SEARCH_FDATABSS|
                 GC_SEARCH_FTHREADREGS|
                 GC_SEARCH_FTHREADSTCK,
                (struct gc_data *)&trace_data,
                 next_version);
  /* Enumerate nodes that couldn't be reached during the search. */
  if (trace_data.trace_tree)
      result = enum_unreachable(trace_data.trace_tree,
                                next_version,
                                func,closure);
 } LIBC_FINALLY {
  libc_mutex_put(&trace_lock);
 }
 return result;
}



PRIVATE ssize_t LIBCCALL
libc_print_leak(void *base, size_t num_bytes,
                pid_t leaker_tid, size_t frame_cnt,
                void *const frame_ip[], void *closure) {
 size_t i;
 /* This node wasn't reached. */
 if unlikely(!frame_cnt) {
  libc_fprintf((FILE *)closure,
               "???? -- Leaked %Iu bytes of memory at %p...%p (TID %p)'\n",
               num_bytes,base,(uintptr_t)base+num_bytes-1,leaker_tid);
 } else {
  libc_fprintf((FILE *)closure,
               "%[vinfo:%f(%l,%c) : %n] : %p : Leaked %Iu bytes of memory at %p...%p (TID %p)\n",
              (uintptr_t)frame_ip[0]-1,frame_ip[0],num_bytes,base,
              (uintptr_t)base+num_bytes-1,leaker_tid);
  for (i = 1; i < frame_cnt; ++i) {
   libc_fprintf((FILE *)closure,
                "%[vinfo:%f(%l,%c) : %n] : %p : Called from here\n",
               (uintptr_t)frame_ip[i]-1,frame_ip[i]);
  }
 }
 libc_fprintf((FILE *)closure,"\n");
 return 1;
}

INTERN size_t LIBCCALL libc_heap_dump_leaks(void) {
 return (size_t)libc_heap_find_leaks(&libc_print_leak,stderr);
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_HEAP_TRACE_C */
