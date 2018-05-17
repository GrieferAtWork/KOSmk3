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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_TASK_H
#define GUARD_KERNEL_INCLUDE_SCHED_TASK_H 1

#include <hybrid/atomic.h>
#include <hybrid/compiler.h>
#include <hybrid/timespec.h>
#include <hybrid/list/list.h>
#include <hybrid/list/ring.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kos/types.h>
#include <kernel/paging.h>
#include <kernel/sections.h>
#include <kos/context.h>
#include <kos/thread.h>
#include <bits/sched.h>
#include <sched/pertask.h>

#include <errno.h>
#include <stdbool.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/vm86.h>
#include <i386-kos/scheduler.h>
#endif

DECL_BEGIN

#ifdef __CC__
struct vm;
struct cpu;
#endif /* __CC__ */



#ifdef CONFIG_NO_SMP
#   undef CONFIG_MAX_CPU_COUNT
#   define CONFIG_MAX_CPU_COUNT 1
#else /* CONFIG_NO_SMP */
/* Configuration option: The max number of CPUs supported by KOS. */
#   ifndef CONFIG_MAX_CPU_COUNT
#      define CONFIG_MAX_CPU_COUNT 8
#   elif (CONFIG_MAX_CPU_COUNT+0) <= 1
#      undef CONFIG_MAX_CPU_COUNT
#      define CONFIG_MAX_CPU_COUNT 1
#      define CONFIG_NO_SMP 1
#   endif
#endif /* !CONFIG_NO_SMP */


#ifdef __CC__
#if CONFIG_MAX_CPU_COUNT >= 16
#   define KERNEL_CPUSET_ITEMTYPE  u16
#   define KERNEL_CPUSET_ITEMSIZE  2
#   define KERNEL_CPUSET_ITEMBITS  16
#else
#   define KERNEL_CPUSET_ITEMTYPE  u8
#   define KERNEL_CPUSET_ITEMSIZE  1
#   define KERNEL_CPUSET_ITEMBITS  8
#endif
#define KERNEL_CPUSET_NUMITEMS   \
      ((CONFIG_MAX_CPU_COUNT+(KERNEL_CPUSET_ITEMBITS-1))/KERNEL_CPUSET_ITEMBITS)
#if KERNEL_CPUSET_NUMITEMS == 1
#define KERNEL_CPUSET_INIT       {0}
#define KERNEL_CPUSET_INIT_FULL  {(KERNEL_CPUSET_ITEMTYPE)-1}
#elif KERNEL_CPUSET_NUMITEMS == 2
#define KERNEL_CPUSET_INIT       {0,0}
#define KERNEL_CPUSET_INIT_FULL  {(KERNEL_CPUSET_ITEMTYPE)-1,(KERNEL_CPUSET_ITEMTYPE)-1}
#else
#define KERNEL_CPUSET_INIT       { [0 ... KERNEL_CPUSET_NUMITEMS-1] = 0 }
#define KERNEL_CPUSET_INIT_FULL  { [0 ... KERNEL_CPUSET_NUMITEMS-1] = (KERNEL_CPUSET_ITEMTYPE)-1 }
#endif
#if KERNEL_CPUSET_NUMITEMS == 1
#define KERNEL_CPUSET_FOREACH(set,cpuid) \
 if (!(set)[0]); else for ((cpuid) = 0; (cpuid) < KERNEL_CPUSET_ITEMBITS; ++(cpuid)) \
 if (!((set)[0] & 1 << (cpuid))); else
#else
#define KERNEL_CPUSET_FOREACH(set,cpuid) \
 for (unsigned int _index = 0; _index < KERNEL_CPUSET_NUMITEMS; ++_index) \
 if (!(set)[_index]); else for (unsigned int _bit = 0; _bit < KERNEL_CPUSET_ITEMBITS; ++_bit) \
 if (!((set)[_index] & 1 << _bit)); else if (((cpuid) = _index*KERNEL_CPUSET_ITEMBITS+_bit,0)); else
#endif

/* Helper functions for working with kernel CPU sets. */
typedef KERNEL_CPUSET_ITEMTYPE kernel_cpuset_t[KERNEL_CPUSET_NUMITEMS];
FORCELOCAL void KCALL kernel_cpuset_add(kernel_cpuset_t self, cpuid_t id) { self[id/KERNEL_CPUSET_ITEMBITS] |= (KERNEL_CPUSET_ITEMTYPE)1 << (id % KERNEL_CPUSET_ITEMBITS); }
FORCELOCAL void KCALL kernel_cpuset_del(kernel_cpuset_t self, cpuid_t id) { self[id/KERNEL_CPUSET_ITEMBITS] &= ~((KERNEL_CPUSET_ITEMTYPE)1 << (id % KERNEL_CPUSET_ITEMBITS)); }
FORCELOCAL bool KCALL kernel_cpuset_has(kernel_cpuset_t const self, cpuid_t id) { return (self[id/KERNEL_CPUSET_ITEMBITS] & ((KERNEL_CPUSET_ITEMTYPE)1 << (id % KERNEL_CPUSET_ITEMBITS))) != 0; }

#endif


#define CPU_OFFSETOF_ID      0
#define CPU_OFFSETOF_ARCH    __SIZEOF_POINTER__

#ifdef __CC__
#ifndef STRUCT_CPU_DEFINED
#define STRUCT_CPU_DEFINED 1
struct cpu {
    /* This is what `struct task::t_cpu' points to. */
    union PACKED {
        cpuid_t           cpu_id;    /* [const][< :cpu_count] CPU Id of this CPU. */
        uintptr_t       __cpu_pad;   /* ... */
    };
    /* ... Additional, arch-specific scheduling fields go here. */
    /* ... Additional, per-cpu variables go here. */
};
#endif /* !STRUCT_CPU_DEFINED */

/* CPU Enumeration globals. */

/* [!0][<= CONFIG_MAX_CPU_COUNT][const]
 *  The total number of known CPUs. */
DATDEF cpuid_t const cpu_count;

/* [1..1][cpu_count] Vector of CPU descriptors.
 *  NOTE: The index in this vector is the
 *       `cpu_id' of the associated CPU.
 *  NOTE: `_boot_cpu' always has the CPUID ZERO(0) */
DATDEF struct cpu *const cpu_vector[CONFIG_MAX_CPU_COUNT];


/* TODO: Per-CPU RPC function calls:
 *     - Executed on any random thread running on that CPU.
 *     - Executed with the `TASK_FKEEPCORE' flag set.
 *     - If the calling thread was already running on that CPU, execute RPC immediately.
 *     - Queue of per-CPU RPC functions is polled similarly to how `task_serve()' is.
 *     - To schedule:
 *       - Add callback to queue.
 *       - Send IPI to CPU that asks it to keep wake threads sporadically
 *         until one of them has served all CPU RPC functions.
 * TODO: Reserve and unmap the page immediately below the boot thread's stack.
 *       Currently a stack overflow occurring in the boot thread will just keep
 *       on overwriting kernel .bss / .data, corrupting everything in the process.
 *       We've got a stack-overflow handler. We're just not using it right now...
 * TODO: Use a gap of 1 page when allocating kernel stacks.
 * TODO: Implement enforcing of gaps in `vm_getfree()'
 */

#endif /* __CC__ */


/* Task flags. */
#define TASK_FNORMAL             0x0000     /* Normal task flags. */
#define TASK_FOWNUSERSEG         0x0001     /* [lock(THIS_TASK)] The task owns `t_userseg' within `t_vm' */
#define TASK_FNOSIGNALS          0x0002     /* [lock(THIS_TASK)] Quick toggle bit for enabling/disabling user-space signal delivery.
                                             * NOTE: When this bit is set, signals are not delivered during preemption.
                                             *       However, interrupt requests (such as generated by `task_queue_rpc()' with
                                             *       the `TASK_RPC_USER' flag set) and RPC function calls will still be executed.
                                             *  On X86, this bit can be controlled by the user-space emulation of the
                                             * `cli' / `sti' instructions (if that feature is enabled for this thread) */
#define TASK_FUSEREXCEPT         0x0004     /* [lock(THIS_TASK)] Userspace has defined a custom TLS address, but has promised
                                             *                   that the address is compatible with exception handling. */
#define TASK_FKEEPCORE           0x0010     /* [lock(THIS_TASK && !TASK_FALWAYSKEEPCORE)]
                                             *  Under no circumstances must the task be moved to a different core.
                                             *  While this flag is set, `THIS_CPU' is guarantied not to change. */
#define TASK_FALWAYSKEEPCORE     0x0100     /* [lock(THIS_TASK && WRITE_ONCE)]
                                             *  The task can never change its core.
                                             *  This flag is used to differentiate between situations where a thread
                                             *  is able to eventually change its core when the `TASK_FKEEPCORE' flag
                                             *  is unset, and thread that must not change core for the entirety of
                                             *  their lifetime. */
#define TASK_FRPCRECURSION       0x0020     /* [lock(THIS_TASK)] RPC functions should be served recursively.
                                             *                   Normally, RPC functions calling `task_serve()'
                                             *                   will simple have that function return immediately
                                             *                   without it doing anything. However if this flag
                                             *                   is set, RPC functions are served recursively.
                                             *                   However, this flag will once again be unset
                                             *                   within the context of those recursive handlers,
                                             *                   meaning that setting this flag within an RPC
                                             *                   function will only allow for 1 additional level
                                             *                   of immediate indirection.
                                             *             NOTE: An RPC function that sets this flag is also
                                             *                   responsible to unsetting it before returning. */
#define TASK_FNOHOSTSTACK        0x1000     /* [const] The task uses a custom host stack, which must not be freed during its destruction. */
#ifdef CONFIG_VM86
#define TASK_FVM86               0x4000     /* [lock(PRIVATE(THIS_TASK))] This task is running in VM86 mode. */
#endif
#define TASK_FKERNELJOB          0x8000     /* [const] This is a kernel-job task, meaning it never returns to user-space.
                                             *  When this flag is set, `task_queue_rpc()' cannot be used in `TASK_RPC_USER' mode. */

/* Task state flags. */
#define TASK_STATE_FINITIAL      0x0000     /* Initial state flags. */
#define TASK_STATE_FSTARTED      0x0001     /* [lock(WRITE_ONCE)] The task was started. */
#define TASK_STATE_FINTERRUPTED  0x0002     /* [atomic] Interrupts (user-space signals, or RPC callbacks)
                                             *          have been scheduled, to-be served by the task.
                                             *    NOTE: This flag may be set sporadically to temporarily
                                             *          wake the task.
                                             *    NOTE: Only the task itself is allowed to unset this bit. */
#define TASK_STATE_FSUSPENDED    0x0004     /* [lock(SET(THIS_TASK),UNSET(ATOMIC))] The thread has been suspended.
                                             *  To resume the thread, you must atomically unset this bit
                                             *  and use `task_wake()' to wake the thread. */
#define TASK_STATE_FTERMINATED   0x0008     /* [lock(WRITE_ONCE,SET(THIS_TASK))]
                                             *  Set before the join-signal for this task is broadcast.
                                             *  NOTE: This bit is set while a lock to the join-signal
                                             *        is already being held, meaning that in order to
                                             *        safely join a task, you should do this:
                                             * >> for (;;) {
                                             * >>     if (task->t_state & TASK_STATE_FTERMINATED)
                                             * >>         break;
                                             * >>     sig_write(&FORTASK(task,task_joinsig));
                                             * >>     if (task->t_state & TASK_STATE_FTERMINATED) {
                                             * >>         sig_endwrite(&FORTASK(task,task_joinsig));
                                             * >>     }
                                             * >>     task_connect(&FORTASK(task,task_joinsig));
                                             * >>     sig_endwrite(&FORTASK(task,task_joinsig));
                                             * >>     task_wait();
                                             * >> }
                                             */
#define TASK_STATE_FINTERRUPTING 0x0020     /* [atomic] Lock-bit for accessing task interrupt data.
                                             * WARNING: Any code holding this lock _must_ be async-safe and
                                             *          more importantly not be able to call `task_serve()'. */
#define TASK_STATE_FTERMINATING  0x0080     /* [lock(WRITE_ONCE,SET(THIS_TASK))]
                                             *  Set once the task starts shutting down.
                                             *  When this bit is set, signals and RPC callbacks
                                             *  can no longer be send to the thread.
                                             *  Additionally, this bit cannot be unset. */
#define TASK_STATE_FINRPC        0x0100     /* [lock(THIS_TASK)] The thread is currently serving RPC functions. */
#ifdef _NOSERVE_SOURCE
#define TASK_STATE_FDONTSERVE    0x0200     /* [lock(THIS_TASK)] Don't serve RPC functions.
                                             * Use of this flags should be kept to an absolute minimum. However the
                                             * existence of this flag is actually required to prevent a logic
                                             * recursion when code invoked by `error_throw()' (such as `linker_findfde')
                                             * would attempt to acquire a lock to the effective VM, potentially re-triggering
                                             * another exception being thrown (such as `E_INTERRUPT'), which would lead
                                             * back to `linker_findfde()' when _it_ tries to get unwound (it gets ugly from there on)
                                             * SO UNLESS YOUR REASON OF USING THIS FLAG ISN'T AT LEAST HALF AS VALID,
                                             * KEEP YOUR HANDS AWAY FROM IT! */
#endif
#define TASK_STATE_FHELPMETERM   0x0800     /* [lock(READ(!PREEMPTION_ENABLED() && THIS_CPU),
                                             *       WRITE(!PREEMPTION_ENABLED() && THIS_TASK))]
                                             *  This bit is set when a task is unable to terminate itself
                                             *  and allows other threads (on the same CPU) encountering this
                                             *  one to terminate the thread (by unscheduling and decref()-ing it).
                                             *  This flag is used to prevent an IDLE-loop when more than one thread
                                             *  exits at the same time, meaning that they won't be able to use RPC
                                             *  calls on each other in order to off-load the job of being unscheduled
                                             *  to another thread. */
#define TASK_STATE_FIDLETHREAD   0x1000     /* [lock(THIS_TASK)] The task's quantum may be skipped if other tasks are scheduled
                                             *                   without the `TASK_STATE_FIDLETHREAD' flag and no RPC functions
                                             *                   are scheduled for it (aka. `TASK_STATE_FINTERRUPTED' isn't set).
                                             *                   Note that this flag is merely a hint to the scheduler,
                                             *                   meaning that it does not guaranty that the task will
                                             *                   be alone, or together with only other IDLEING thread
                                             *                   while it is executing. */
#define TASK_STATE_FSERVEEXCEPT  0x2000     /* [lock(THIS_TASK)] An exception occurred when serving interrupts
                                             *                   while `t_nothrow_serve' was non-ZERO. */
#define TASK_STATE_FSLEEPING     0x4000     /* [lock(t_cpu == THIS_CPU)]
                                             *  Set while the task is sleeping (not part of the active task queue). */
#define TASK_STATE_FTIMEDOUT     0x8000     /* [lock(SET(t_cpu == THIS_CPU),UNSET(THIS_TASK))]
                                             *  NOTE: Only the CPU of the task is allowed to set this bit
                                             *        when the task is in a waiting-state, alongside a timeout. */



#define TASK_OFFSETOF_SEGMENT        0
#define TASK_OFFSETOF_REFCNT         TASK_SEGMENT_SIZE
#define TASK_OFFSETOF_CONTEXT       (TASK_SEGMENT_SIZE+__SIZEOF_REF_T__)
#ifndef CONFIG_NO_SMP
#define TASK_OFFSETOF_CPU           (TASK_SEGMENT_SIZE+__SIZEOF_REF_T__+__SIZEOF_POINTER__)
#define TASK_OFFSETOF_VM_LOCK       (TASK_SEGMENT_SIZE+__SIZEOF_REF_T__+__SIZEOF_POINTER__*2)
#else /* !CONFIG_NO_SMP */
#define TASK_OFFSETOF_VM_LOCK       (TASK_SEGMENT_SIZE+__SIZEOF_REF_T__+__SIZEOF_POINTER__)
#endif /* CONFIG_NO_SMP */
#define TASK_OFFSETOF_VM            (TASK_OFFSETOF_VM_LOCK+__SIZEOF_POINTER__)
#define TASK_OFFSETOF_VMTASKS       (TASK_OFFSETOF_VM_LOCK+__SIZEOF_POINTER__*2)
#define TASK_OFFSETOF_USERSEG       (TASK_OFFSETOF_VM_LOCK+__SIZEOF_POINTER__*4)
#define TASK_OFFSETOF_STACKMIN      (TASK_OFFSETOF_VM_LOCK+__SIZEOF_POINTER__*5)
#define TASK_OFFSETOF_STACKEND      (TASK_OFFSETOF_VM_LOCK+__SIZEOF_POINTER__*6)
#define TASK_OFFSETOF_TEMPPAGE      (TASK_OFFSETOF_VM_LOCK+__SIZEOF_POINTER__*7)
#define TASK_OFFSETOF_SCHED         (TASK_OFFSETOF_VM_LOCK+__SIZEOF_POINTER__*8)
#define TASK_OFFSETOF_ADDR2LIMIT    (TASK_OFFSETOF_VM_LOCK+__SIZEOF_POINTER__*10)
#define TASK_OFFSETOF_TIMEOUT       (TASK_OFFSETOF_VM_LOCK+__SIZEOF_POINTER__*11)
#define TASK_OFFSETOF_FLAGS         (TASK_OFFSETOF_VM_LOCK+__SIZEOF_JTIME_T__+__SIZEOF_POINTER__*11)
#define TASK_OFFSETOF_STATE         (TASK_OFFSETOF_VM_LOCK+__SIZEOF_JTIME_T__+__SIZEOF_POINTER__*11+2)
#define TASK_OFFSETOF_NOTHROW_SERVE (TASK_OFFSETOF_VM_LOCK+__SIZEOF_JTIME_T__+__SIZEOF_POINTER__*11+4)
#define TASK_OFFSETOF_DATA          (TASK_OFFSETOF_VM_LOCK+__SIZEOF_JTIME_T__+__SIZEOF_POINTER__*11+8)

#ifdef __CC__
struct task {
    /* This is the data structure pointed by the %FS / %GS register. */
    struct task_segment          t_segment;   /* Must be located at offset=0 */
    ATOMIC_DATA ref_t            t_refcnt;    /* Task reference counter. */
    struct cpu_anycontext       *t_context;   /* [lock(THIS_CPU == t_cpu)][valid_if(self != THIS_TASK)]
                                               * The CPU context to which this task will return
                                               * when scheduling resumes its execution. */
#ifndef CONFIG_NO_SMP
    struct cpu                  *t_cpu;       /* [1..1][lock(THIS_CPU == t_cpu)] The current CPU
                                               *  NOTE: This pointer is never invalid, and even when it changes (which is _very_ rare),
                                               *        there is no point in time where this field points somewhere else, but at a valid
                                               *        CPU descriptor. */
#endif /* !CONFIG_NO_SMP */
    atomic_rwlock_t              t_vm_lock;   /* Lock that must be held when the associated thread intending to modify the signal.
                                               * Similarly, this lock must be held when another thread wishes
                                               * to thread the thread's VM (without actually being that thread). */
    REF struct vm               *t_vm;        /* [1..1][lock(READ(THIS_TASK || t_vm_lock),WRITE(THIS_TASK && t_vm_lock))]
                                               *  The current VM. (NOTE: May only be NULL before the task was started) */
    LIST_NODE(struct task)       t_vmtasks;   /* [lock(t_vm->vm_lock)] Chain of tasks using `t_vm'.
                                               *  NOTE: This link is created by `task_start()'
                                               *  NOTE: Locking is done as follows when changing the current thread's VM:
                                               *  >> OLD_VM = t_vm;
                                               *  >> GET(t_vm_lock,OLD_VM->vm_lock,NEW_VM->vm_lock);
                                               *  >> PREEMPTION_PUSHOFF();
                                               *  >> t_vm = NEW_VM;
                                               *  >> PUT(t_vm_lock);
                                               *  >> LIST_REMOVE(OLD_VM,t_vmtasks);
                                               *  >> PUT(OLD_VM->vm_lock);
                                               *  >> LIST_INSERT(NEW_VM,t_vmtasks);
                                               *  >> ARCH_SET_CURRENT_VM(NEW_VM);
                                               *  >> PREEMPTION_POP();
                                               *  >> PUT(NEW_VM->vm_lock);
                                               */
    USER struct user_task_segment
                                *t_userseg;   /* [?..?][owned_if(TASK_FOWNUSERSEG)][lock(THIS_TASK)]
                                               *  A pointer to this task's user-segment.
                                               *  During a context switch, the user-space TLS mechanism
                                               *  is updated to use this pointer as base address. */
    HOST VIRT void              *t_stackmin;  /* Lowest address of the host (kernel) stack.
                                               * NOTE: When the `TASK_FNOHOSTSTACK' flag isn't set,
                                               *       this address must be page-aligned. */
    HOST VIRT void              *t_stackend;  /* End address of the host (kernel) stack.
                                               * NOTE: When the `TASK_FNOHOSTSTACK' flag isn't set,
                                               *       this address must be page-aligned. */
    HOST vm_vpage_t              t_temppage;  /* [const]
                                               * A single page of virtual memory that the thread (kernel-side)
                                               * may use for whatever it desires, that usually being temporary
                                               * memory mappings.
                                               * NOTE: This field maps to a singleton virtual memory region
                                               *       consisting of a single page of type `VM_REGION_RESERVED',
                                               *       meaning that the VM isn't used to manage which physical
                                               *       memory is actually mapped in this page.
                                               * The main idea behind this is to provide a way to quickly and
                                               * easily map a small portion of physical memory for use in only
                                               * the calling thread (this page is part still part of the
                                               * kernel-share segment), and may be used to implement functions
                                               * like `vm_read' or `vm_write' without the need of constructing
                                               * a true virtual memory mapping each time they are called.
                                               * Instead, this page should be passed to `pagedir_mapone()',
                                               * whilst any code using this page should be aware that other
                                               * code also using it is not require to document whether or not
                                               * it clobbers it.
                                               * Additionally, no code is required to restore the mapping of
                                               * the temporary page once it has finished operating.
                                               * The virtual page is unreserved once the associated thread terminates.
                                               * NOTE: This page is allocated lazily by `task_temppage()'
                                               *       and destroyed once `task_exit()' is called.
                                               *       Before then, it is initialized to `VM_VPAGE_MAX+1',
                                               *       indicative of an invalid page address. */
    union PACKED {
        RING_NODE(struct task)   sched_ring;  /* Ring node. */
        LIST_NODE(struct task)   sched_list;  /* List node. */
        SLIST_NODE(struct task)  sched_slist; /* Singly-linked list node. */
    }                            t_sched;     /* INTERNAL (Scheduler data) */
    VIRT vm_virt_t               t_addrlimit; /* [lock(THIS_TASK)] Arch-dependent address limit.
                                               *  In high-kernel configurations, this usually is the
                                               *  first address that user-space may not have access
                                               *  to when passed in system calls. */
    jtime_t                      t_timeout;   /* [lock(WRITE(THIS_TASK),READ(t_cpu == THIS_CPU))]
                                               *  Absolute timeout of the task.
                                               *  When expired during a wait operation, the task
                                               *  is resumed with the `TASK_STATE_FTIMEDOUT' flag
                                               *  set, which is then usually interpreted by throwing
                                               *  an `E_WOULDBLOCK' error. */
    u16                          t_flags;     /* Set of `TASK_F*' */
    u16                          t_state;     /* Set of `TASK_STATE_F*' */
#define TASK_ISTERMINATING(x) ((x)->t_state & (TASK_STATE_FTERMINATING|TASK_STATE_FTERMINATED))
#define TASK_ISTERMINATED(x)  ((x)->t_state & TASK_STATE_FTERMINATED)
    /* A way of delaying exceptions from RPC functions.
     * When `kfree()' locks the heap, it could potentially
     * serve RPC functions which could then throw an
     * exception that would prevent `kfree()' from actually
     * freeing the pointer.
     * However, the caller probably doesn't want that, yet
     * neither can we just ignore the error, in case it's
     * meant to terminate the calling thread.
     * >> There must be some way for cleanup code to not be
     *    re-interrupted and have the cleanup be aborted before
     *    it could finish (which would lead to resource leaks)
     * >> And that's where this comes into play.
     *    When non-ZERO, exceptions that are thrown by RPC
     *    functions are suspended and will only be propagated
     *    once this counter hits ZERO again. */
    u32                          t_nothrow_serve; /* Recursion counter for `task_serve()' to be NOTHROW. */
    /* PERTASK data goes here... */
};

/* Intended to be used like `PERTASK(this_task.foo)'
 * Same as `THIS_TASK->foo', but allows for use of `PERTASK_GET' and friends. */
INTDEF struct task this_task ASMNAME("NULL");

#endif /* __CC__ */


#ifdef __CC__
/* Allocate a new, unbound task descriptor.
 * PERTASK variables will have been initialized from the template,
 * and all other variables are default-initialized as unbound.
 * The reference counter is set to ONE(1)
 * @throw E_BADALLOC: Failed to allocate sufficient memory.
 * Before starting, the caller must initialize various fields. */
FUNDEF ATTR_RETNONNULL ATTR_MALLOC REF struct task *KCALL task_alloc(void);

/* Perform final linking and start scheduling the given task for execution.
 * NOTE: Execution of the task will commence at an undefined point in time,
 *       or even before this function returns. */
FUNDEF void KCALL task_start(struct task *__restrict self);

/* Destroy a previously allocated task. */
FUNDEF void KCALL task_destroy(struct task *__restrict self);

/* Safely set flags to indicate termination of a task
 * who's `TASK_STATE_FSTARTED' flag has never been set.
 * The intended use is as follows:
 * >> REF struct task *EXCEPT_VAR new_thread = task_alloc();
 * >> TRY {
 * >>     
 * >>     ... // Setup the thread's configuration.
 * >>     
 * >>     task_start(new_thread);
 * >> } FINALLY {
 * >>     if (FINALLY_WILL_RETHROW)
 * >>         task_failed(new_thread);
 * >>     task_decref(new_thread);
 * >> } */
FUNDEF void KCALL task_failed(struct task *__restrict self);

/* Increment/decrement the reference counter of the given task `x' */
#define task_tryincref(x)  ATOMIC_INCIFNONZERO((x)->t_refcnt)
#define task_incref(x)     ATOMIC_FETCHINC((x)->t_refcnt)
#define task_decref(x)    (ATOMIC_DECFETCH((x)->t_refcnt) || (task_destroy(x),0))


typedef void (KCALL *task_main_t)(void *arg);

/* Setup a newly constructed task as a kernel worker thread.
 * This function does _all_ the initialization between
 * `task_alloc()' and `task_start()'.
 * The intended use is as follows:
 * >> struct task *EXCEPT_VAR thread = task_alloc();
 * >> TRY {
 * >>     task_setup_kernel(thread,&my_thread_main,NULL);
 * >>     task_start(thread);
 * >>     // Potentially do some more stuff with `thread'
 * >> } FINALLY {
 * >>     task_decref(thread);
 * >> }
 * Creation of user-space threads requires platform-specific
 * code due to the number of undefined parameters.
 * NOTE: The size of the stack that is allocated is `CONFIG_KERNELSTACK_SIZE'
 * @throw: E_BADALLOC: Failed to allocate sufficient resources for the new thread. */
FUNDEF void KCALL
task_setup_kernel(struct task *__restrict thread,
                  task_main_t thread_main, void *arg);

/* Allocate, pre-fault, and lock a kernel-stack for the given thread.
 * @throw E_BADALLOC: Not enough available memory. */
FUNDEF void KCALL
task_alloc_stack(struct task *__restrict thread,
                 size_t num_pages);

/* Allocate and map a user-space thread segment for the calling thread.
 * No-op if the thread already has a user-space thread segment.
 * NOTE: This function will also pre-initialize the user-segment to its default values.
 * NOTE: If user-space should use an automatically allocated stack, this
 *       function must be called _AFTER_ `task_alloc_userstack()'
 * @return: * : The value of `THIS_TASK->t_userseg'
 * @throw E_BADALLOC: Not enough available memory.*/
FUNDEF USER struct user_task_segment *KCALL task_alloc_userseg(void);

/* Check for pending RPC functions yet to be executed.
 * NOTE: If the task is currently running in user-space, this function
 *       is also called during interrupt-driven preemption, though the
 *       main idea is for tasks to call this function before entering
 *       a sleep-state, as is automatically done when `task_sleep()'
 *       is called.
 * NOTE: When the `t_nothrow_serve' counter of the calling thread is
 *       non-ZERO(0), this function doesn't propagate any exceptions,
 *       but instead delays them until the counter is decremented.
 * NOTE: In the event that RPC function calls are served, this function
 *       will automatically save the calling thread's signal connection
 *       set, meaning that the caller is not required to save it, nor are
 *       RPC functions that may use signal-based connections required to.
 * If an error is thrown, previously connected signals are disconnected after being restored.
 * @throw: E_INTERRUPT:   A user-space signal is pending delivery.
 * @throw: E_EXIT_THREAD: A scheduled RPC function threw an `E_EXIT_THREAD' error.
 * @throw: * :            A scheduled RPC function threw an error.
 * @return: true:  At least one RPC function was served.
 *                (Interrupts were enabled if the caller had disabled them)
 * @return: false: No RPC functions were served.
 *                (Interrupts are still disabled if the caller had disabled them) */
FUNDEF bool KCALL task_serve(void);

/* Recursively begin/end `task_serve()' being nothrow. */
FUNDEF ATTR_NOTHROW void (KCALL task_nothrow_serve)(void);
FUNDEF void (KCALL task_nothrow_end)(void);

#ifndef __INTELLISENSE__
#if defined(__i386__) || defined(__x86_64__)
#define task_nothrow_serve() \
 __asm__ __volatile__("incl %%" PP_STR(__ASM_TASK_SEGMENT) ":%a0\n" \
                      : : "i" (TASK_OFFSETOF_NOTHROW_SERVE) : "memory")
#endif
#endif


/* Return the virtual page index of a temporary, thread-local
 * virtual memory mapping that may be used for arbitrarily mapping
 * a single page of physical memory to a thread-private virtual
 * address (who's index is returned by this function).
 * NOTE: In order to map physical memory to the returned address,
 *       regular VM operations must be bypassed in favor of directly
 *       modify the underlying page directory using `pagedir_mapone()'
 * NOTE: While mapping memory at that address, the calling thread
 *       must be holding a write-lock to `vm_kernel.vm_lock'
 */
FUNDEF ATTR_CONST vm_vpage_t KCALL task_temppage(void);


/* Change execution order within the calling CPU to have
 * `next' execute when `prev's next, or current quantum ends.
 * This function is mean to-be used as follows:
 * >> REF struct task *iter,*prev = THIS_TASK;
 * >> REF struct task *high_priority;
 * >> // Schedule all tasks returned by `get_high_priority_task()' for
 * >> // immediate execution (even during times of high system loads)
 * >> task_incref(prev);
 * >> while ((high_priority = get_high_priority_task()) != NULL) {
 * >>     iter = task_runnext(prev,high_priority);
 * >>     task_decref(high_priority);
 * >>     task_decref(prev);
 * >>     prev = iter;
 * >> }
 * NOTE: If `prev' is currently sleeping, run `next' after `THIS_TASK' instead.
 * @return: prev:      `prev' and `next' are the same pointers (`prev == next'),
 *                      or either `next' are not hosted by the current CPU.
 * @return: THIS_TASK: `prev' isn't hosted by the current CPU.
 * @return: prev:      `next' is currently sleeping.
 * @return: next:       Execution order has been changed.
 */
FUNDEF REF struct task *KCALL
task_runnext(struct task *__restrict prev,
             struct task *__restrict next);



/* Task signaling API. (Low-level scheduling) */


/* Max number of signal connections guarantied to not invoke `kmalloc()'
 * and potentially throw exceptions, or serve RPC functions.
 * A value of ONE(1) would suffice, but a static buffer is mandatory
 * due to the fact that allocating a dynamic buffer (using `kmalloc()')
 * will do its own lock in the case that new memory must be vm_map()-ed
 * In that case, locking needs to be done, meaning that to prevent logic
 * recursion, it must be possible to have at least one signal slot that
 * is allocated statically (`task_connect()' uses `task_push_connections()'
 * to free up that static slot before calling `kmalloc()'). */
#ifndef CONFIG_TASK_STATIC_CONNECTIONS
#define CONFIG_TASK_STATIC_CONNECTIONS  3
#endif

struct sig;
struct task_connection {
    struct task_connections              *tc_conn;  /* [1..1][lock(tc_sig)][== :self] Pointer to the associated connection set.
                                                     *  NOTE: The least significant bit of this field is used to indicate ghost connections. */
#define TASK_CONNECTION_SIG_FGHOST        0x0001    /* Ghost connection (see explanation in `task_connect_ghost()') */
#define TASK_CONNECTION_SIG_FFLAGS        TASK_CONNECTION_SIG_FGHOST /* Mask of connection flags. */
#define TASK_CONNECTION_SIG_FMASK       ((uintptr_t)~1) /* Mask for the actual pointer. */
#define TASK_CONNECTION_GETSIG(x)       ((struct sig *)((uintptr_t)(x)->tc_sig & TASK_CONNECTION_SIG_FMASK))
    struct sig                           *tc_sig;   /* [0..1][lock(THIS_TASK)] When non-NULL, pointer to the signal to which this connection is bound. */
    union PACKED {
        struct task_connection           *tc_last;  /* [valid_if(tc_sig && INTERN(self == SIG_GETCON(tc_sig)))]
                                                     * [lock(tc_sig)][1..1] Pointer to the latest connection established with `tc_sig' */
        struct task_connection          **tc_pself; /* [valid_if(tc_sig && INTERN(self != SIG_GETCON(tc_sig)))]
                                                     * [lock(tc_sig)][== self][0..1] Self-pointer / pointer (in-)to the previous connect. */
    };
    struct task_connection               *tc_next;  /* [valid_if(tc_sig && tc_pself)][lock(tc_sig)][0..1]
                                                     * Pointer to the next connection. */
};

struct task_connections {
    struct task_connection  tcs_sbuf[CONFIG_TASK_STATIC_CONNECTIONS];
    size_t                  tcs_siz; /* [!0][lock(THIS_TASK)] Allocated number of connections. */
    size_t                  tcs_cnt; /* [lock(THIS_TASK)] Number of used connections. */
    struct task_connection *tcs_vec; /* [1..tcs_cnt|ALLOC(tcs_siz)][owned_if(!= tcs_sbuf)]
                                      * [lock(THIS_TASK)] Vector of connections. */
    struct sig             *tcs_sig; /* [atomic] The first signal that was received. */
    struct task            *tcs_tsk; /* The task to which this connection set is bound. */
    uintptr_t               tcs_chn; /* [lock(READ(atomic),WRITE(THIS_TASK))] Signal channel mask. */
};

/* Connect the calling task to the given signal.
 * If there is insufficient memory to establish the connection, the set of
 * already connected signals is disconnected and an `E_BADALLOC' error is thrown.
 * NOTE: Any task is guarantied to sustain at least `CONFIG_TASK_STATIC_CONNECTIONS'
 *       simultaneous connections before running the chance to not be able to
 *       connect to more due to lack of resources.
 * If the given `signal' was already connected, this function is a no-op.
 * Note however, that the implementation will still register the signal
 * a second time, knowing that when send, the calling thread will only
 * receive it once.
 * @throw: E_BADALLOC: There are already at least `CONFIG_TASK_STATIC_CONNECTIONS'
 *                     other connections, and not enough available memory to allocate
 *                     a new buffer for more. */
FUNDEF void KCALL task_connect(struct sig *__restrict signal);

/* Same as `task_connect()', but the connection counts as a so-called ghost connection.
 * Since many components of KOS make use `sig_send(...,1)' to optimize scheduling
 * in situations where only a single thread can actually hold a lock, a problem
 * arises when this mechanism is combined with polling.
 * Since polling a signal doesn't guaranty that the thread doing the polling will
 * ever actually acquire whatever lock may be associated with the signal, some thread
 * releasing the lock and signaling the thread that did the polling could then result
 * in a softlock scenario with the original lock holder assuming that some other thread
 * is going to acquire the lock after having signaled that thread. However, if more threads
 * are waiting to acquire the same lock, yet the thread that got signaled isn't actually
 * intending on acquiring the lock for certain, a deadlock can occur with threads still
 * waiting for a lock that has actually already been released.
 * The solution here is to somehow prevent threads that are only polling signals from
 * counting as ~true~ connections in the sense that they should be considered viable
 * signal receivers, like threads waiting for signals in order to acquire associated
 * locks.
 * For this purpose, ~ghost~ connections were introduced.
 * Ghost connections act identical to regular connections, however when `sig_send()'
 * is used to deliver the signal, although a ghost connection will still get the
 * signal delivered and be woken during the process, they however will not count to
 * the total sum of signaled threads, meaning that `sig_send()' will continue to wake
 * other threads until a sufficient number of non-ghostly (regularly) connected
 * threads have been signaled. */
FUNDEF void KCALL task_connect_ghost(struct sig *__restrict signal);


/* Check if the calling thread is currently connected to `signal'. */
FUNDEF ATTR_NOTHROW bool KCALL task_connected(struct sig *__restrict signal);

/* Check if the calling thread is connected to any signals. */
FUNDEF ATTR_NOTHROW bool KCALL task_isconnected(void);

/* Return the number of signals connected to the calling thread. */
FUNDEF ATTR_NOTHROW size_t KCALL task_numconnected(void);

/* Set the active signal channel mask of the calling thread.
 * The signal channel mask can be used to better define certain
 * conditions for which to wait, rather that to simply wait for
 * any kind of signal.
 * This function goes hand in hand with `sig_send_channel' and
 * implements an extension to regular signal-based scheduling,
 * who's main purpose is to implement support for linux's futex
 * bitset operations:
 *   - When the regular `sig_send()' function is used, the channel
 *     mask is completely ignored (even when set to ZERO(0), the
 *     associated thread will still be woken)
 *   - When `sig_send_channel()' is used, only thread waiting for
 *     that signal and matching the following condition are woken
 *    (and therefor add to the number of woken threads returned):
 *     >> (GET_CHANNEL_MASK(CANDIDATE) & signal_mask) != 0
 *    (Where `GET_CHANNEL_MASK(CANDIDATE)' is the channel mask
 *     of a thread `CANDIDATE' that is waiting for the signal,
 *     as previously set by a call to this function)
 *   - With that in mind, the channel mask is a bitset of ~channels~
 *     that the calling thread is currently receiving.
 *     Also note that the channel mask is persistent, in that
 *     the only way to modify it, is by calling this function.
 *    (With the exception of it also being saved/restored
 *     by the `task_(push|pop)_connections' function pair)
 *   - Specifying a channel mask of ZERO(0) (as is the default,
 *     pre-initialized value of newly constructed threads) is
 *     the same as not waiting for any sub-channels (and only
 *     waiting for the master channel that is signaled when
 *    `sig_send()' is used)
 *   - Specifying a full mask (`(uintptr_t)-1') will have the
 *     calling thread wait for all sub-channels, as well as
 *     the master channel.
 *   - Calling `sig_send()' signals the master channel, as well
 *     as all sub-channels.
 *   - Calling `sig_send_channel()' only signals channels matching
 *     the passed mask (see the bitwise and in the pseudo code above.)
 *   - Changing the channel mask when already connected isn't allowed.
 * @return: * : The old channel mask. */
FUNDEF ATTR_NOTHROW uintptr_t KCALL task_channelmask(uintptr_t mask);

/* Same as `task_channelmask()', but rather than overwriting the
 * active channel mask, atomically start listening to all channels
 * masked by the given `mask', but keep listening for channels
 * already described by the previously channel mask.
 * This function should be used by poll()-style functions to ensure that
 * specific channels are open, before testing for signal conditions.
 * Also note that this function can be used to open new channels,
 * even when the calling thread is already connected.
 * @return: * : The old channel mask. */
FUNDEF ATTR_NOTHROW uintptr_t KCALL task_openchannel(uintptr_t mask);

/* Switching the calling task to a signaled-state with `signal'
 * being what was supposedly send. If the task already was set
 * to a signaled state, return `false', otherwise `true'.
 * Signaled state here means that the next call to `task_[try]wait[for]'
 * will return immediately, before serving RPC functions, and before
 * potentially changing the calling thread's state to sleeping.
 * WARNING: In order to become signaled, `signal' must be connected
 *          with, meaning that you must call this function following
 *          a previous call to `task_connect()' with the same signal. */
FUNDEF ATTR_NOTHROW bool KCALL task_setsignaled(struct sig *__restrict signal);

/* Disconnect all connected signals, returning `NULL' if none
 * had been sent yet, or a pointer to the signal that was sent. */
FUNDEF ATTR_NOTHROW struct sig *KCALL task_disconnect(void);

/* Push (safe the current, then load an empty set) / pop (restore the given set) of connections.
 * This function's main intent is to be used during pagefault handling for LOA/COW
 * while ensuring that connections established by the caller will not go lost, or
 * interfere with those then made by the handler.
 * Additionally, saved connections will continue to listen for signals,
 * and once restored using `task_pop_connections()', those signals will
 * immediately become available to the calling thread. */
FUNDEF ATTR_NOTHROW void KCALL task_push_connections(struct task_connections *__restrict safe);
FUNDEF ATTR_NOTHROW void KCALL task_pop_connections(struct task_connections *__restrict safe);

#define TASK_EVAL_CONSAFE(expr) \
 XBLOCK({ __typeof__(expr) COMPILER_IGNORE_UNINITIALIZED(__cs_result); \
          struct task_connections __cs_cons; \
          /* Clutch required to keep GCC from placing &__cs_cons in a register. */ \
          struct task_connections *EXCEPT_VAR __cs_pcons = &__cs_cons; \
          task_push_connections(&__cs_cons); \
          TRY { \
              __cs_result = (expr); \
          } EXCEPT (EXCEPT_EXECUTE_HANDLER) { \
              task_pop_connections(__cs_pcons); \
              error_rethrow(); \
          } \
          task_pop_connections(&__cs_cons); \
          XRETURN __cs_result; })



/* Without blocking, check if any signal has been sent.
 * If so, disconnect from all other connected signals and
 * return a pointer to the signal that was discovered to be sent. */
FUNDEF ATTR_NOTHROW struct sig *KCALL task_trywait(void);

/* Wait for any of the connected signals to be sent and
 * return a pointer to the first signal fulfilling this
 * requirement.
 * For the duration of waiting for signals, the calling
 * thread will be unscheduled, meaning that no CPU time
 * is used by this operation.
 * Only send operations executed after connecting to a
 * signal will trigger this function to return. However
 * when a signal was sent between connecting to it, and
 * these function being called, they will return immediately.
 * Otherwise, prior to being unscheduled, `task_serve()' is
 * called internally to serve any RPC functions that are
 * supposed to run in the calling thread.
 * If an RPC function throws an error, all connected signals are
 * disconnected and the function returns immediately by rethrowing
 * the previous error.
 * Function calling this function are called `synchronization points'
 * and all feature the same exception characteristics.
 * @throw: E_WOULDBLOCK: Preemption is currently disabled and
 *                      `abs_timeout' did not equal `JTIME_DONTWAIT'
 * @throw: E_INTERRUPT:  The calling thread was interrupt was a user-space signal
 *                       This only happens when the `TASK_FKERNELJOB' flag isn't set.
 * @throw: * :           A scheduled RPC callback threw this error.
 * NOTE: Passing `JTIME_DONTWAIT' for `abs_timeout' behaves the same as `task_trywait()'
 * NOTE: Passing `JTIME_INFINITE' for `abs_timeout' behaves the same as `task_wait()',
 *       meaning that the function will not return `NULL' in this case.
 * WARNING: These functions may forceably enable preemption and not disable it
 *          before returning (there would be no point to. These functions represent
 *          the core idea of what preemption even means!) */
FUNDEF ATTR_RETNONNULL struct sig *KCALL task_wait(void);
#ifdef _NOSERVE_SOURCE
FUNDEF ATTR_RETNONNULL struct sig *KCALL task_wait_noserve(void);
#endif
#ifdef __INTELLISENSE__
FUNDEF struct sig *KCALL task_waitfor(jtime_t abs_timeout);
#ifdef _NOSERVE_SOURCE
FUNDEF struct sig *KCALL task_waitfor_noserve(jtime_t abs_timeout);
#endif
#else
FUNDEF struct sig *KCALL __os_task_waitfor(jtime_t abs_timeout) ASMNAME("task_waitfor");
FORCELOCAL struct sig *KCALL task_waitfor(jtime_t abs_timeout) {
 COMPILER_BARRIER();
 if (__builtin_constant_p(abs_timeout)) {
  if (abs_timeout == JTIME_DONTWAIT)
      return task_trywait();
  if (abs_timeout == JTIME_INFINITE)
      return task_wait();
 }
 return __os_task_waitfor(abs_timeout);
}
#ifdef _NOSERVE_SOURCE
FUNDEF struct sig *KCALL __os_task_waitfor_noserve(jtime_t abs_timeout) ASMNAME("task_waitfor_noserve");
FORCELOCAL struct sig *KCALL task_waitfor_noserve(jtime_t abs_timeout) {
 COMPILER_BARRIER();
 if (__builtin_constant_p(abs_timeout)) {
  if (abs_timeout == JTIME_DONTWAIT)
      return task_trywait();
  if (abs_timeout == JTIME_INFINITE)
      return task_wait_noserve();
 }
 return __os_task_waitfor_noserve(abs_timeout);
}
#endif
#endif



/* Helper functions for user-space wait requests. */
FUNDEF struct sig *KCALL task_waitfor_tmrel(USER CHECKED struct timespec const *rel_timeout); /* Relative timeout */
FUNDEF struct sig *KCALL task_waitfor_tmabs(USER CHECKED struct timespec const *abs_timeout); /* Absolute timeout */


/* Connect the calling task to the join-signal of `other_task'.
 * If `other_task' has already been terminated, also switch
 * the calling thread to a signaled state for the join-signal
 * of `other_task'.
 * Despite the fact that this can be used to retrieve the internal
 * address of the signal used for joining, that address should
 * not be used for any purposes, much less to be connected with
 * without the use of this function.
 * The intended use of this function is as follows:
 * >> void task_join(struct task *__restrict other_task) {
 * >>     task_connect_join(other_task);
 * >>     task_wait();
 * >> } */
FUNDEF void KCALL task_connect_join(struct task *__restrict other_task);


/* Yield the remainder of the caller's quantum to the next
 * scheduled task (no-op if no task to switch to exists).
 * @throw: E_WOULDBLOCK: The caller has disabled preemption and
 *                       switching the current task context would
 *                       violate the no-interrupt rule. */
FUNDEF void (KCALL task_yield)(void);

/* Similar to `task_yield()', but rather than throwing an error if
 * yielding isn't possible (!PREEMPTION_ENABLED()), return `false' instead.
 * Additionally, if there is no other thread to yield to, return `false', too.
 * Otherwise, `true' is returned.
 * HINT: Because This function is a no-op when preemption is disabled,
 *       it is safe to be called from interrupt handlers (aka. ASYNCSAFE).
 * ---- ARCH-SPECIFIC ----
 * X86: When `true' is returned, EFLAGS_CF is cleared (`if (task_tryyield()) goto 1f' --> `call task_tryyield; jnc 1f')
 * X86: When `false' is returned, EFLAGS_CF is set    (`if (!task_tryyield()) goto 1f' --> `call task_tryyield; jc 1f') */
FUNDEF ASYNCSAFE ATTR_NOTHROW bool (KCALL task_tryyield)(void);

#if defined(__i386__) || defined(__x86_64__)
#ifdef CONFIG_BUILDING_KERNEL_CORE
#define task_tryyield() XBLOCK({ bool __ty_res; __asm__("call task_tryyield" : "=@ccnc" (__ty_res) : : "memory","eax","ecx","edx"); XRETURN __ty_res; })
#else
#define task_tryyield() XBLOCK({ bool __ty_res; __asm__("call task_tryyield@PLT" : "=@ccnc" (__ty_res) : : "memory","eax","ecx","edx"); XRETURN __ty_res; })
#endif
#endif

/* Re-schedule the given `thread' if it was unscheduled (entered a sleeping state).
 * Using this function, a ~sporadic interrupt~ is implemented.
 * If the thread hasn't been unscheduled, this function is a no-op.
 * NOTE: This function is used in the implementation of `sig_send'
 * @return: true:  The task was woken, or wasn't sleeping.
 * @return: false: The given task has terminated. */
FUNDEF ASYNCSAFE ATTR_NOTHROW bool KCALL task_wake(struct task *__restrict thread);

/* Similar to `task_wake()', but if the task isn't sleeping but was preempted
 * from user-space, redirect its control flow to serve RPC function calls 
 * before returning to user-space at the start of its next quantum. */
FUNDEF ASYNCSAFE ATTR_NOTHROW bool KCALL task_wake_for_rpc(struct task *__restrict thread);

#ifdef CONFIG_NO_SMP
#define task_wake_p(prev_threads,next)  task_wake(next)
#else
/* Same as `task_wake()', but try to schedule `next' for execution after
 * `prev_threads[next->t_cpu->cpu_id]', following which that vector entry
 * is updated to `next'.
 * If the vector entry in `prev_threads' was NULL before, `next' is re-scheduled
 * for execution following the thread currently being executed on the associated CPU.
 * This function is used to implement `sig_(send|boardcast)[_channel][_locked]_p'
 * @param: prev_threads: A vector of at least `cpu_count' entires
 *                      (and at most `CONFIG_MAX_CPU_COUNT')
 * @return: true:  The task was woken, or wasn't sleeping, and
 *                 may have been re-scheduled for early execution.
 * @return: false: The given task has terminated. */
FUNDEF ASYNCSAFE bool KCALL
task_wake_p(REF struct task *prev_threads[/*CONFIG_MAX_CPU_COUNT*/],
            struct task *__restrict next);
#endif


/* Enter a sleeping state and return once being woken (true),
 * or once the given `abs_timeout' expires (false)
 * WARNING: Even though the caller must first disable preemption,
 *          it will be re-enabled once this function returns.
 * NOTE: When `JTIME_DONTWAIT' is passed for `abs_timeout', `false'
 *       is returned immediately (after re-enabling preemption).
 * NOTE: This function is the bottom-most (arch-implemented) API
 *       for conserving CPU cycles using preemption, in that this
 *       function is even used to implement `task_wait()'.
 *       There _IS_ _NO_ _LOWER_ _LEVEL_ _API_!
 * The proper way of using this function is as follows:
 * >> while (SHOULD_WAIT()) { // Test some condition for which to wait
 * >>     PREEMPTION_DISABLE();
 * >>     // Test again now that interrupts are disabled
 * >>     // This test is required to prevent a race condition
 * >>     // where the condition is signaled between it being
 * >>     // changed and interrupts being disabled.
 * >>     COMPILER_READ_BARRIER();
 * >>     if (!SHOULD_WAIT()) { PREEMPTION_ENABLE(); break; }
 * >>     // Serve RPC functions (when TRUE is returned, preemption was re-enabled)
 * >>     if (task_serve()) continue;
 * >>     // Do the actual sleep.
 * >>     if (!task_sleep(TIMEOUT))
 * >>         return DID_TIME_OUT;
 * >> }
 * >> return WAS_SIGNALED;
 * The sleeping thread should then be woken as follows:
 * >> SET_SHOULD_WAIT(false);
 * >> task_wake(waiting_thread);
 */
FUNDEF NOIRQ bool KCALL task_sleep(jtime_t abs_timeout);


struct cpu_hostcontext_user;
typedef void (KCALL *task_rpc_t)(void *arg);

/* @param: arg:     The `arg' argument passed when the RPC was queued.
 * @param: context: The arch-dependent, user-space CPU context that will
 *                  be restored once this RPC, as well as all other queued
 *                  RPC callbacks have been served.
 * @param: mode:    One of `TASK_USERCTX_F*' explaining what state the current
 *                  thread is in, as far as user-space is concerned. (see below) */
typedef void (KCALL *task_user_rpc_t)(void *arg,
                                      struct cpu_hostcontext_user *__restrict context,
                                      unsigned int mode);

/* Queue a remote procedure call to-be executed the next time
 * `task_serve()' is called, or user-space is interrupted.
 * HINT: This function may throw an exception that will be propagated
 *       and can be used to terminate the thread using `E_EXIT_THREAD'.
 * Scheduled RPC functions are executed in order or being queued.
 * @param: func:    The function that should be executed in the context of `thread'.
 *                  The function will be run the next time `thread' calls `task_serve()',
 *                  or the next time the thread is interrupted while in user-space.
 * @param: mode:    Set of `TASK_RPC_*'
 * @throw: E_INTERRUPT: The given `thread' is the calling thread and `TASK_RPC_USER' is set.
 *                     (_ALWAYS_ thrown when `thread == THIS_TASK' and `TASK_RPC_USER' is set)
 *                      If the `TASK_RPC_USER' flag isn't set, the RPC is executed immediatly.
 * @throw: E_INTERRUPT: The calling thread was interrupted  (_NEVER_ thrown when `thread == THIS_TASK')
 *                      In this case no RPC is scheduled.
 * @throw: E_IOERROR:   Failed to communicate with the cpu of `thread'
 * @return: true:  Successfully queued the RPC (even when `TASK_RPC_SYNC'
 *                 isn't given, you can be sure that the RPC will be executed)
 * @return: false: Cannot queue RPC once the `TASK_STATE_FTERMINATING' flag is set.
 * @return: false: `TASK_RPC_USER' has been set, but the task is a kernel-job.
 * NOTE: When execution of the RPC is requested to be performed synchronously,
 *      `task_wait()' is used to wait for the function to complete, meaning
 *       that this function is a synchronization point.
 * NOTE: If `thread' is the calling thread, the behavior depends on the `TASK_RPC_USER'
 *       flag. When set, the `TASK_FKERNELJOB' flag is checked. If that is set, `false'
 *       is returned, and if it isn't an `E_INTERRUPT' error is thrown, instructing
 *       the thread to execute the given `func' before switching to user-space.
 *       However, when `TASK_RPC_USER' isn't set, `func' is executed immediately
 *       and so long as _it_ doesn't throw any errors, the functions simply
 *       returns `true'.
 * HINT: Unless the `TASK_RPC_SYNC' flag will be set, it is advised to allocate extended
 *       argument memory using heap functions, then pass a pointer to the data block as
 *       `arg' and have `func' free the data once it is done.
 *       However when `TASK_RPC_SYNC' is set, you are free to allocate `arg' on your
 *       stack. For the same reason why exceptions can be continued, seemingly by
 *       rewinding the stack to include unwound frames, `func' will be executed further
 *       up the stack, even when an exception must be thrown in order to get back to
 *       user-space when `TASK_RPC_USER' is set and `thread' is the calling thread.
 * NOTE: The only reason why `false' may be returned is because the thread is terminating/has
 *       terminated or because it has the `TASK_FKERNELJOB' flag set and `TASK_RPC_USER' was given. */
FUNDEF bool KCALL task_queue_rpc(struct task *__restrict thread,
                                 task_rpc_t func, void *arg,
                                 unsigned int mode);
/* Helper alias for `TASK_RPC_USER' */
FUNDEF bool KCALL
task_queue_rpc_user(struct task *__restrict thread,
                    task_user_rpc_t func, void *arg,
                    unsigned int mode) ASMNAME("task_queue_rpc");
#endif /* __CC__ */

#define TASK_RPC_NORMAL 0x0000 /* Normal RPC flags. */
#define TASK_RPC_SYNC   0x0001 /* Synchronize execution of the RPC.
                                * When set, `task_queue_rpc()' will not return until after
                                * the given `func()' has been executed and has returned. */
#define TASK_RPC_SINGLE 0x0010 /* Optional, optimization flag:
                                * Check if the given `func' and `arg' have already been
                                * scheduled with the same `mode & TASK_RPC_USER' combination.
                                * If so, the implementation is allowed not to schedule the
                                * functions a second time, but instead return as a no-op
                                * (when `TASK_RPC_SYNC' isn't set), or wait until the existing
                                * RPC callback has finished.
                                * NOTE: This function can be ignored if the implementation
                                *       chooses not to support it (as KOS currently does). */
#define TASK_RPC_USYNC  0x4000 /* An extension to `TASK_RPC_USER': The RPC is not executed
                                * when the thread get preempted forceably at the end of its
                                * quantum. Rather, it only gets served when the thread performs
                                * some blocking operation.
                                * This flag is used to implement synchronous user-space RPCs
                                * that aren't executed arbitrarily, but rather only when the
                                * thread performs some blocking operation.
                                * Additionally, don't serve this type of RPC from an interrupt.
                                * Only serve it from a blocking system call. */
#define TASK_RPC_USER   0x8000 /* FLAG: Rather than execute the RPC immediately, interrupt the
                                *       current system call that is being executed by the task
                                *       by throwing an `E_INTERRUPT' error.
                                *       If the task is currently in user-space, temporarily
                                *       switch to kernel-space during the next preemption.
                                *       In either case, execute `func()' immediately prior
                                *       to the task returning to user-space.
                                * NOTE: If other RPCs are scheduled without this flag following
                                *       one with it, they are still executed in proper order,
                                *       meaning that they will run prior to returning to
                                *       user-space, too.
                                * NOTE: If the task has the `TASK_FKERNELJOB' flag set, this
                                *       flag cannot be used and `task_queue_rpc' will always
                                *       return `false'.
                                * WARNING: This flag changes the RPC callback prototype to
                                *          take an additional argument that describes the
                                *          modifiable CPU state as it will be loaded once
                                *          execution returns to user-space.
                                *          This allows RPC functions to be used to implement
                                *          more complicated user-space functionality such as
                                *          posix signal handlers, or something that would
                                *          actually be useful: RPC function calls in user-space. */

#ifdef __CC__
/* Terminate the calling thread immediately.
 * WARNING: Do not call this function to terminate a thread.
 *          It would work, but exception handlers would not
 *          get unwound and resources would be leaked.
 *          If you wish to exit your current thread, just
 *          throw an `E_EXIT_THREAD' error.
 * This function is called by the unhandled exception handler
 * when it encounters an `E_EXIT_THREAD' error, or when exception
 * data cannot be propagated to userspace in the event of an
 * interrupt throwing some error, whilst originating from user-space.
 * NOTE: The caller should fill in `error_info()->e_error.e_exit.e_reason'
 *       to pass information on why the exit is happening. */
FUNDEF ATTR_NORETURN void KCALL task_exit(void);

/* Thread exception handling and propagation to user-space.
 * This function is called when an unhandled exception
 * would be unwound into user-space.
 * Its job is to interpret some special exception (such as `E_EXIT_THREAD',
 * which will terminate the user-space thread by calling `task_exit'), before
 * copying exception data to the user-space task segment, using `context'
 * as the exception context where the error supposedly happened (we don't
 * expose the true origin of the exception to user-space when it originates
 * from within the kernel)
 * If this copy operation fails (E_SEGFAULT), `task_exit()' is invoked to
 * terminate the user-space thread immediately.
 * In the event of success, this function will update `context'
 * to refer to the appropriate user-space exception handler for the
 * error that occurred, before returning its caller who should activate
 * the new user-context.
 * NOTE: This function is called in the scheduling-context of the faulting thread. */
FUNDEF void FCALL
task_propagate_user_exception(struct cpu_hostcontext_user *__restrict context,
                              unsigned int mode);



/* Get/Set the used VM of the given / current thread.
 * NOTE: Although a thread is allowed to change VM any number of times,
 *       doing so is not encouraged, as it greatly confuses user-space.
 * NOTE: `task_unshare_vm()' (or alternatively `sys_unshare(CLONE_VM)')
 *       can be used to move the calling thread into a duplicate of its
 *       previous VM.
 * @throw: * :          [task_setvm] An RPC function threw this error (`new_vm' was not set)
 * @throw: E_INTERRUPT: [task_setvm] The calling thread was interrupted (`new_vm' was not set) */
FUNDEF ATTR_RETNONNULL REF struct vm *KCALL task_getvm(struct task *__restrict thread);
FUNDEF void KCALL task_setvm(struct vm *__restrict new_vm);


/* Unshare certain components of the calling thread
 * that were shared with other threads before.
 * @param: flags: Set of `CLONE_*' from <bits/sched.h> (unknown bits are ignored)
 * @return: * : The number of components unshared
 *             (Sum of `task_unshare_*' functions that returned `true').
 * HINT: This function is the kernel-equivalent of `sys_unshare()' */
FUNDEF size_t KCALL task_unshare(u32 flags);

/* Unshare certain components of the calling thread, returning
 * `true' if they were shared before, or false if the calling
 * thread already was the exclusive using thread. */
FUNDEF bool KCALL task_unshare_files(void);    /* CLONE_FILES */
FUNDEF bool KCALL task_unshare_fs(void);       /* CLONE_FS */
FUNDEF bool KCALL task_unshare_sighand(void);  /* CLONE_SIGHAND */
FUNDEF bool KCALL task_unshare_vm(void);       /* CLONE_FS */



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
 *   -  If the given callback `func' returns `false', enumeration is aborted.
 * @return: * :          The total number of existing threads (when `func' only returned
 *                      `true', then this equals the number of enumerated threads).
 * @throw: E_BADALLOC:   Too many threads are running and enumeration failed
 *                       to allocate a temporary buffer located on the heap.
 * @throw: E_WOULDBLOCK: Preemption has been disabled. */
FORCELOCAL size_t KCALL task_foreach(bool (KCALL *func)(struct task *__restrict thread, void *arg), void *arg);
FORCELOCAL size_t KCALL task_foreach_running(bool (KCALL *func)(struct task *__restrict thread, void *arg), void *arg);
FUNDEF size_t KCALL task_foreach_ex(bool (KCALL *func)(struct task *__restrict thread, void *arg), void *arg, u16 state_mask, u16 state_flag);

/* Similar to `task_foreach()' above, but doesn't give the guaranty that any
 * thread is enumerated only once, or that no threads will be skipped during
 * enumeration, in trade for the ability to enumerate without the need of a
 * temporary buffer.
 * @return: * : The total number of enumerated threads.
 *              The behavior when `func' returns `false' differs slightly in this case,
 *              as the number of _enumerated_ threads is returned, not _existing_!
 * @throw: E_WOULDBLOCK: Preemption has been disabled. */
FORCELOCAL size_t KCALL task_foreach_weak(bool (KCALL *func)(struct task *__restrict thread, void *arg), void *arg);
FORCELOCAL size_t KCALL task_foreach_weak_running(bool (KCALL *func)(struct task *__restrict thread, void *arg), void *arg);
FUNDEF size_t KCALL task_foreach_weak_ex(bool (KCALL *func)(struct task *__restrict thread, void *arg), void *arg, u16 state_mask, u16 state_flag);

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
FORCELOCAL size_t KCALL task_enumerate(REF struct task **__restrict buf, size_t buf_length);
FORCELOCAL size_t KCALL task_enumerate_running(REF struct task **__restrict buf, size_t buf_length);
FUNDEF size_t KCALL task_enumerate_ex(REF struct task **__restrict buf, size_t buf_length, u16 state_mask, u16 state_flag);



#ifndef __INTELLISENSE__
FORCELOCAL size_t KCALL
task_foreach(bool (KCALL *func)(struct task *__restrict thread, void *arg), void *arg) {
 return task_foreach_ex(func,arg,0,0);
}
FORCELOCAL size_t KCALL
task_foreach_running(bool (KCALL *func)(struct task *__restrict thread, void *arg), void *arg) {
 return task_foreach_ex(func,arg,
                        TASK_STATE_FSTARTED|TASK_STATE_FTERMINATED,
                        TASK_STATE_FSTARTED);
}
FORCELOCAL size_t KCALL
task_foreach_weak(bool (KCALL *func)(struct task *__restrict thread, void *arg), void *arg) {
 return task_foreach_weak_ex(func,arg,0,0);
}
FORCELOCAL size_t KCALL
task_foreach_weak_running(bool (KCALL *func)(struct task *__restrict thread, void *arg), void *arg) {
 return task_foreach_weak_ex(func,arg,
                             TASK_STATE_FSTARTED|TASK_STATE_FTERMINATED,
                             TASK_STATE_FSTARTED);
}

FORCELOCAL size_t KCALL
task_enumerate(REF struct task **__restrict buf, size_t buf_length) {
 return task_enumerate_ex(buf,buf_length,0,0);
}
FORCELOCAL size_t KCALL
task_enumerate_running(REF struct task **__restrict buf, size_t buf_length) {
 return task_enumerate_ex(buf,buf_length,
                          TASK_STATE_FSTARTED|TASK_STATE_FTERMINATED,
                          TASK_STATE_FSTARTED);
}
#endif /* !__INTELLISENSE__ */


#endif /* __CC__ */


/* Mask for the arch-independent portion of `TASK_USERCTX_*'
 * Other bits are arch-specific and are usually used to identify
 * for format of registers.
 * On X86, that is one of `X86_SYSCALL_TYPE_F*' */
#define TASK_USERCTX_FMASK               0x00ff

#define TASK_USERCTX_FLAG_FMASK          0x00f0 /* Mask for context flags. */
#define TASK_USERCTX_FTIMER              0x0010 /* The interrupt happened because of a timed interrupt,
                                                 * rather than the thread performing a blocking operation. */
#define TASK_USERCTX_TYPE(x) ((x) & TASK_USERCTX_TYPE_FMASK)
#define TASK_USERCTX_TYPE_FMASK          0x000f /* Mask for the basic context type. (One of `TASK_USERCTX_TYPE_*') */
#define TASK_USERCTX_TYPE_WITHINUSERCODE 0x0000 /* Serve RPC functions with `context' referring to the CPU
                                                 * state as it was after user-space got forcefully preempted. */
#define TASK_USERCTX_TYPE_INTR_INTERRUPT 0x0001 /* Serve RPC functions with `context' referring to the
                                                 * CPU state following an interrupted interrupt.
                                                 * NOTE: An RPC may override `error_info()->e_error.e_code = E_USER_RESUME' to
                                                 *       continue execution in user-space normally. */
#define TASK_USERCTX_TYPE_INTR_SYSCALL   0x0002 /* Same as `TASK_USERCTX_TYPE_INTR_INTERRUPT', but the RPC
                                                 * is being served following an interrupted system call.
                                                 * NOTE: An RPC may override `error_info()->e_error.e_code = E_USER_RESUME' to
                                                 *       continue execution in user-space normally. */


#ifdef CONFIG_BUILDING_KERNEL_CORE
#ifdef __CC__
/* Serve RPC callbacks prior to switching to user-space.
 * Called before user-space exception propagation to serve RPC
 * functions that should be executed before returning to user-space.
 * NOTE: The regular `task_serve()' throws an `E_INTERRUPT' error in
 *       order to interrupt the current system call when RPC functions
 *       have been registered that should be executed prior to jumping
 *       back to user-space.
 * @param: context: The user-space CPU context as it will be restored during the switch.
 * @param: mode:    One of `TASK_USERCTX_F*' (see above)
 *            NOTE: May be or'd with arch-specific flags. */
INTDEF void FCALL
task_serve_before_user(struct cpu_hostcontext_user *__restrict context,
                       unsigned int mode);
#endif /* __CC__ */
#endif /* CONFIG_BUILDING_KERNEL_CORE */


#ifdef __CC__
/* Should be called from a CATCH(E_INTERRUPT) block within a
 * restartable interrupt to attempt to restart the system call.
 * If the given `context' isn't the final one that will be used to eventually
 * return to user-space, the previously thrown `E_INTERRUPT' will be re-thrown.
 * If these functions return normally, the caller should restart the current
 * system call / interrupt.
 * These functions will call `task_serve_before_user()' when `context' is
 * the associated (original) user-space CPU context to which to return.
 * NOTE: This function should only be called from interrupt handlers, such as
 *       the PAGEFAULT -> loadcore() handler in order to be restartable when
 *       some other thread schedules an RPC function with the `TASK_RPC_USER'
 *       flag set while that other thread is busy loading data into the core.
 * NOTE: This function also properly deals with redirected preemption, which
 *       should remind you that you can't just look at `context->CS & 3' to
 *       see if that context returns to user-space.
 *       _ALWAYS_ call this function when you're restartable interrupt
 *       handler catches an `E_INTERRUPT' exception!
 *       This function will then do the rest.
 * NOTE: This function must only be called from interrupt handlers that don't
 *       require async-safety (such as the #PF handler on X86). Additionally,
 *       the caller is responsible to re-enable preemption if the interrupt
 *       source had them enabled before, meaning that if the interrupt originated
 *       from user-space, preemption must be turned back on before this function
 *       should be called. */
FUNDEF void FCALL task_restart_interrupt(struct cpu_anycontext *__restrict context);
/* Same as `task_restart_interrupt()', but meant to be called from a
 * `CATCH(E_INTERRUPT)' block surrounding a system call invocation.
 * @param: mode: `TASK_USERCTX_TYPE_INTR_SYSCALL', optionally or'd with arch-
 *                dependent context flags (e.g.: on X86, `X86_SYSCALL_TYPE_F*') */
struct cpu_hostcontext_user;
FUNDEF void FCALL task_restart_syscall(struct cpu_hostcontext_user *__restrict context,
                                       unsigned int mode, syscall_ulong_t sysno);
/* A try-variant of `task_restart_syscall()' that will automatically propagate
 * exception to userspace, and return `false' is the caller should load the
 * updated `context', or `true' if the caller should re-attempt execution
 * of the current system call. */
FUNDEF ATTR_NOTHROW bool FCALL
task_tryrestart_syscall(struct cpu_hostcontext_user *__restrict context,
                        unsigned int mode, syscall_ulong_t sysno);
#endif /* __CC__ */


#ifdef __CC__
/* The current jiffies-time. */
DATDEF volatile jtime_t jiffies;
DATDEF volatile jtime32_t jiffies32[2] ASMNAME("jiffies");
#define JIFFIES_PER_SECOND        HZ
#define JIFFIES_PER_MINUTE       (HZ*60)
#define JIFFIES_PER_HOUR         (HZ*3600)
#define JIFFIES_FROM_HOURS(x)   ((x)*JIFFIES_PER_HOUR)
#define JIFFIES_FROM_MINUTES(x) ((x)*JIFFIES_PER_MINUTE)
#define JIFFIES_FROM_SECONDS(x) ((x)*JIFFIES_PER_SECOND)
#define JIFFIES_FROM_MILLI(x)   ((x)/(1000ul/JIFFIES_PER_SECOND))
#define JIFFIES_FROM_MICRO(x)   ((x)/(1000000ul/JIFFIES_PER_SECOND))
#define JIFFIES_FROM_NANO(x)    ((x)/(1000000000ul/JIFFIES_PER_SECOND))

LOCAL jtime_t KCALL
jiffies_from_timespec(struct timespec tmval) {
 return (JIFFIES_FROM_SECONDS(tmval.tv_sec) +
         JIFFIES_FROM_NANO(tmval.tv_nsec));
}

#endif /* __CC__ */


#ifdef __CC__

/* Boot scheduling objects. */
DATDEF struct cpu  _boot_cpu  ASMNAME("boot_cpu_start");
DATDEF struct task _boot_task ASMNAME("boot_task_start");
DATDEF struct vm   _boot_vm   ASMNAME("boot_vm_start");


#define THIS_CPU      PERTASK_GET(this_task.t_cpu)
#define PERCPU(x)  (*(__typeof__(&(x)))((uintptr_t)THIS_CPU+(uintptr_t)&(x)))

/* Address a symbol for a given object. */
#define FORTASK(self,x) (*(__typeof__(&(x)))((uintptr_t)(self)+(uintptr_t)&(x)))
#define FORCPU(self,x)  (*(__typeof__(&(x)))((uintptr_t)(self)+(uintptr_t)&(x)))

#ifdef CONFIG_NO_SMP
#undef THIS_CPU
#undef PERCPU
#undef FORCPU
#define THIS_CPU       _this_cpu
#define PERCPU(x)      x  /* Without SMP, our linker script maps percpu variables as one-on-one */
#define FORCPU(self,x) x
#endif /* CONFIG_NO_SMP */
#endif /* __CC__ */

/* Configuration option for standard synchronization primitives.
 * Before connecting to a signal, try to yield a couple of times
 * to try and get other threads to release some kind of lock,
 * as `task_yield()' is a much faster operation than connect()+wait().
 * Doing this may improve performance, especially on single-core machines.
 * Note however that this option does not affect the behavior of
 * low-level `struct sig' objects, but instead primitives found
 * in <sched/*.h>, such as `mutex_t', `rwlock_t' and `sem_t'
 * NOTE: The number that this is defined to describes how
 *       many times `task_yield()' should be attempted
 *      (implementing a kind-of spin-locking mechanism),
 *       before a signal is actually connected. */
#ifndef CONFIG_YIELD_BEFORE_CONNECT
#ifndef CONFIG_NO_YIELD_BEFORE_CONNECT
#define CONFIG_YIELD_BEFORE_CONNECT  4
#endif
#elif defined(CONFIG_NO_YIELD_BEFORE_CONNECT)
#undef CONFIG_YIELD_BEFORE_CONNECT
#endif

#ifdef CONFIG_YIELD_BEFORE_CONNECT
#if (CONFIG_YIELD_BEFORE_CONNECT+0) == 0
#undef CONFIG_YIELD_BEFORE_CONNECT
#define TASK_POLL_BEFORE_CONNECT(...) (void)0
#elif CONFIG_YIELD_BEFORE_CONNECT == 1
#define TASK_POLL_BEFORE_CONNECT(...) \
   do{ task_yield(); __VA_ARGS__; }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 2
#define TASK_POLL_BEFORE_CONNECT(...) \
   do{ task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
   }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 3
#define TASK_POLL_BEFORE_CONNECT(...) \
   do{ task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
   }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 4
#define TASK_POLL_BEFORE_CONNECT(...) \
   do{ task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
   }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 5
#define TASK_POLL_BEFORE_CONNECT(...) \
   do{ task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
   }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 6
#define TASK_POLL_BEFORE_CONNECT(...) \
   do{ task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
       task_yield(); __VA_ARGS__; \
   }__WHILE0
#else
#define TASK_POLL_BEFORE_CONNECT(...) \
   do{ unsigned int __poll_count = CONFIG_YIELD_BEFORE_CONNECT; \
       do { \
           task_yield(); \
           __VA_ARGS__; \
       } while (--__poll_count); \
   }__WHILE0
#endif
#else
#define TASK_POLL_BEFORE_CONNECT(...) (void)0
#endif


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_TASK_H */
