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
#ifndef GUARD_KERNEL_I386_KOS_FPU_C
#define GUARD_KERNEL_I386_KOS_FPU_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/intrin.h>
#include <hybrid/section.h>
#include <kernel/sections.h>
#include <kernel/debug.h>
#include <kernel/bind.h>
#include <kernel/heap.h>
#include <kernel/malloc.h>
#include <i386-kos/fpu.h>
#include <sched/task.h>
#include <kos/context.h>
#include <asm/cpu-flags.h>
#include <assert.h>

#ifndef CONFIG_NO_FPU
DECL_BEGIN

STATIC_ASSERT(sizeof(struct fpu_context) == __X86_FPUCONTEXT_SIZE);

#define FPU_GFP  (GFP_SHARED|GFP_LOCKED)
#define FPU_ALLOC() \
   heap_align_untraced(&kernel_heaps[FPU_GFP & __GFP_HEAPMASK], \
                        __X86_FPUCONTEXT_ALIGN,0, \
                        __X86_FPUCONTEXT_SIZE,FPU_GFP)
#define FPU_FREE(p,s) \
   heap_free_untraced(&kernel_heaps[FPU_GFP & __GFP_HEAPMASK], \
                       p,s,FPU_GFP & ~GFP_CALLOC)

/* TODO: task_setcpu() must clear `x86_fpu_current' if
 *       the calling thread holds the active FPU context. */
PUBLIC ATTR_PERCPU struct task *x86_fpu_current = NULL;
PUBLIC ATTR_PERTASK struct fpu_context *x86_fpu_context = NULL;
PRIVATE ATTR_PERTASK size_t x86_fpu_size = 0;

#if 1

/* Reset the current FPU state and discard any saved state. */
PUBLIC void KCALL x86_fpu_reset(void) {
 pflag_t was; size_t size;
 was = PREEMPTION_PUSHOFF();
 if (PERCPU(x86_fpu_current) == THIS_TASK)
     PERCPU(x86_fpu_current) = NULL;
 PREEMPTION_POP(was);
 /* Free any saved FPU context. */
 size = PERTASK_GET(x86_fpu_size);
 if (size) {
  struct fpu_context *context;
  context = PERTASK_GET(x86_fpu_context);
  PERTASK_SET(x86_fpu_context,NULL);
  PERTASK_SET(x86_fpu_size,0);
  FPU_FREE(context,size);
 }
}

/* Save the current FPU state if it has been
 * modified and no longer matches `x86_fpu_context'.
 * If the FPU state has been initialized and `x86_fpu_context'
 * hasn't been allocated yet, allocate it now.
 * @return: false: The FPU wasn't initialized, and `x86_fpu_context' wasn't allocated.
 * @return: true: `x86_fpu_context' is allocated and is up-to-date. */
PUBLIC bool KCALL x86_fpu_save(void) {
 pflag_t was;
 was = PREEMPTION_PUSHOFF();
 if (PERCPU(x86_fpu_current) == THIS_TASK) {
  /* We're the ones using the FPU */
  assert(PERTASK_TEST(x86_fpu_size));
  /* Actually save the context. */
  __clts();
  COMPILER_BARRIER();
  __fxsave((byte_t *)PERTASK_GET(x86_fpu_context));
  COMPILER_BARRIER();
  PREEMPTION_POP(was);
  return true;
 }
 PREEMPTION_POP(was);
 /* The calling thread either hasn't used the FPU, or
  * it's FPU context is already up-to-date. */
 return PERTASK_TEST(x86_fpu_size);
}

PUBLIC NOIRQ ATTR_NOTHROW bool KCALL
x86_fpu_save_thread(struct task *__restrict thread) {
 assert(!PREEMPTION_ENABLED());
#ifndef CONFIG_NO_SMP
 assert(thread->t_cpu == THIS_CPU);
#endif
 if (PERCPU(x86_fpu_current) == thread) {
  /* We're the ones using the FPU */
  assert(FORTASK(thread,x86_fpu_size) != 0);
  /* Actually save the context. */
  __clts();
  COMPILER_BARRIER();
  __fxsave((byte_t *)FORTASK(thread,x86_fpu_context));
  COMPILER_BARRIER();
  if (thread != THIS_TASK) {
   /* Disable the FPU if the given thread isn't the one calling. */
   __wrcr0(__rdcr0() | CR0_TS);
  }
  return true;
 }
 return FORTASK(thread,x86_fpu_size) != 0;
}


/* Reconfigure the FPU to either load `x86_fpu_context' lazily, or
 * load `x86_fpu_context' now. Either way, a previously active FPU
 * state will be discarded.
 * @assume(WAS_ALLOCATED(x86_fpu_context)); */
PUBLIC void KCALL x86_fpu_load(void) {
 pflag_t was = PREEMPTION_PUSHOFF();
 assertf(PERTASK_GET(x86_fpu_size) != 0,"FPU state hasn't been allocated");
#if 1
 /* If we're holding the active FPU context, change
  * it so no-one is holding it, meaning that during
  * the next access, the saved context will be restored. */
 if (PERCPU(x86_fpu_current) == THIS_TASK)
     PERCPU(x86_fpu_current) = NULL;
 /* Configure for lazy loading. */
 __wrcr0(__rdcr0() | CR0_TS);
#else
 __clts();
 COMPILER_BARRIER();
 __fxrstor((byte_t *)PERTASK_GET(x86_fpu_context));
 COMPILER_BARRIER();
#endif
 PREEMPTION_POP(was);
}

/* Ensure that `x86_fpu_context' has been allocated.
 * NOTE: No-op when `x86_fpu_context' had already been allocated before. */
PUBLIC void KCALL x86_fpu_alloc(void) {
 struct heapptr fpu;
 assert(PREEMPTION_ENABLED());
 if (PERTASK_GET(x86_fpu_size)) return;
 fpu = FPU_ALLOC();
 COMPILER_BARRIER();
 /* Check if the FPU context got allocated in the mean time. */
 if unlikely(PERTASK_TEST(x86_fpu_size)) {
  FPU_FREE(fpu.hp_ptr,fpu.hp_siz);
  return;
 }
 PERTASK_SET(x86_fpu_context,(struct fpu_context *)fpu.hp_ptr);
 PERTASK_SET(x86_fpu_size,fpu.hp_siz);
}

DEFINE_PERTASK_FINI(x86_fpu_cleanup);
INTERN void KCALL x86_fpu_cleanup(struct task *__restrict thread) {
 if (FORTASK(thread,x86_fpu_size)) {
  FPU_FREE(FORTASK(thread,x86_fpu_context),
           FORTASK(thread,x86_fpu_size));
 }
}

/* Called as part of the #NM exception handler. */
INTERN int KCALL x86_fpu_interrupt_handler(void) {
 struct task *old_task,*new_task;
 struct fpu_context *state;
 assertf(PREEMPTION_ENABLED(),
         "Only user-space may use the FPU, and this being a TRAP, interrupts "
         "must still be enabled because userspace must have them enabled, too");
 PREEMPTION_DISABLE();
 if (!(__rdcr0() & CR0_TS)) {
  PREEMPTION_ENABLE();
  return 0;
 }
 /* If the task-switched flag is set, unset it because it is
  * possible that the calling task is the only one actually
  * using the FPU right now. */
 __clts();

 /* Check if a FPU context switch occurred. */
 old_task = PERCPU(x86_fpu_current);
 new_task = THIS_TASK;
 COMPILER_BARRIER();
 if (new_task != old_task) {
  debug_printf("[FPU] Switch context %p -> %p\n",old_task,new_task);
  asm("int3");
  if (old_task) {
   /* Save the old FPU register state. */
   state = FORTASK(old_task,x86_fpu_context);
   assert(FORTASK(old_task,x86_fpu_size));
   __fxsave((byte_t *)state);
  }
check_allocate_state:
  if (!PERTASK_TEST(x86_fpu_size)) {
   struct heapptr fpu;
#ifndef CONFIG_NO_SMP
   u16 old_flags; /* Make sure that we don't change cores while allocating the FPU state. */
   old_flags = ATOMIC_FETCHOR(THIS_TASK->t_flags,TASK_FKEEPCORE);
#endif /* !CONFIG_NO_SMP */
   PREEMPTION_ENABLE();
   /* Allocate a new FPU context. */
   fpu = FPU_ALLOC();
   PREEMPTION_DISABLE();
   assert(old_task == PERCPU(x86_fpu_current));
   COMPILER_READ_BARRIER();
   /* Check if the FPU context got allocated in the mean time. */
   if unlikely(PERTASK_TEST(x86_fpu_size)) {
    PREEMPTION_ENABLE();
    FPU_FREE(fpu.hp_ptr,fpu.hp_siz);
    PREEMPTION_DISABLE();
    COMPILER_BARRIER();
    assert(old_task == PERCPU(x86_fpu_current));
#ifndef CONFIG_NO_SMP
    if (!(old_flags & TASK_FKEEPCORE))
          ATOMIC_FETCHAND(THIS_TASK->t_flags,~TASK_FKEEPCORE);
#endif /* !CONFIG_NO_SMP */
    __clts();
    COMPILER_BARRIER();
    goto check_allocate_state;
   }
   PERTASK_SET(x86_fpu_context,(struct fpu_context *)fpu.hp_ptr);
   PERTASK_SET(x86_fpu_size,fpu.hp_siz);
#ifndef CONFIG_NO_SMP
   if (!(old_flags & TASK_FKEEPCORE))
         ATOMIC_FETCHAND(THIS_TASK->t_flags,~TASK_FKEEPCORE);
#endif /* !CONFIG_NO_SMP */
   /* Initialize the default FPU register state. */
   __clts();
   COMPILER_BARRIER();
   __fninit();
   COMPILER_BARRIER();
  } else {
   /* Load an existing FPU state. */
   state = PERTASK_GET(x86_fpu_context);
   __fxrstor((byte_t *)state);
  }
  PERCPU(x86_fpu_current) = new_task;
 }
 PREEMPTION_ENABLE();
 return 1;
}


INTERN ATTR_FREETEXT void KCALL x86_fpu_initialize(void) {
 register_t temp;
 __asm__ __volatile__("clts\n\t"
                      "mov %%cr0, %0\n\t"
                      "and $(" PP_STR(~CR0_EM) "), %0\n\t"
                      "or  $(" PP_STR(CR0_MP) "), %0\n\t"
                      "mov %0, %%cr0\n\t"
                      "mov %%cr4, %0\n\t"
                      "or  $(" PP_STR(CR4_OSFXSR) "), %0\n\t"
                      "mov %0, %%cr4"
                      : "=r" (temp));
}



#else

/* Reset the current FPU state and discard any saved state. */
PUBLIC void KCALL x86_fpu_reset(void) {
 pflag_t was; size_t size;
 was = PREEMPTION_PUSHOFF();
 if (PERCPU(x86_fpu_current) == THIS_TASK)
     PERCPU(x86_fpu_current) = NULL;
 PREEMPTION_POP(was);
 /* Free any saved FPU context. */
 size = PERTASK_GET(x86_fpu_size);
 if (size) {
  struct fpu_context *context;
  context = PERTASK_GET(x86_fpu_context);
  PERTASK_SET(x86_fpu_context,NULL);
  PERTASK_SET(x86_fpu_size,0);
  FPU_FREE(context,size);
 }
}

/* Save the current FPU state if it has been
 * modified and no longer matches `x86_fpu_context'.
 * If the FPU state has been initialized and `x86_fpu_context'
 * hasn't been allocated yet, allocate it now.
 * @return: false: The FPU wasn't initialized, and `x86_fpu_context' wasn't allocated.
 * @return: true: `x86_fpu_context' is allocated and is up-to-date. */
PUBLIC bool KCALL x86_fpu_save(void) {
 pflag_t was;
again:
 was = PREEMPTION_PUSHOFF();
 if (PERCPU(x86_fpu_current) == THIS_TASK) {
  /* We're the ones using the FPU */
  if (!PERTASK_TEST(x86_fpu_size)) {
   /* Make sure our context is allocated. */
   PREEMPTION_POP(was);
   x86_fpu_alloc();
   goto again;
  }
  /* Actually save the context. */
  __asm__ __volatile__("clts; fxsave %0\n"
                       :
                       : "m" (*PERTASK_GET(x86_fpu_context))
                       : "memory");
  PREEMPTION_POP(was);
  return true;
 }
 PREEMPTION_POP(was);
 /* The calling thread either hasn't used the FPU, or
  * it's FPU context is already up-to-date. */
 return PERTASK_TEST(x86_fpu_size);
}

/* Reconfigure the FPU to either load `x86_fpu_context' lazily, or
 * load `x86_fpu_context' now. Either way, a previously active FPU
 * state will be discarded.
 * @assume(WAS_ALLOCATED(x86_fpu_context)); */
PUBLIC void KCALL x86_fpu_load(void) {
 pflag_t was = PREEMPTION_PUSHOFF();
 assertf(PERTASK_GET(x86_fpu_size) != 0,"FPU state hasn't been allocated");
#if 1
 /* If we're holding the active FPU context, change
  * it so no-one is holding it, meaning that during
  * the next access, the saved context will be restored. */
 if (PERCPU(x86_fpu_current) == THIS_TASK)
     PERCPU(x86_fpu_current) = NULL;
#else
 __asm__ __volatile__("clts; fxrstor %0\n"
                      :
                      : "m" (*PERTASK_GET(x86_fpu_context))
                      : "memory");
#endif
 PREEMPTION_POP(was);
}

/* Ensure that `x86_fpu_context' has been allocated.
 * NOTE: No-op when `x86_fpu_context' had already been allocated before. */
PUBLIC void KCALL x86_fpu_alloc(void) {
 struct heapptr fpu;
 assert(PREEMPTION_ENABLED());
 if (PERTASK_GET(x86_fpu_size)) return;
 fpu = FPU_ALLOC();
 COMPILER_BARRIER();
 /* Check if the FPU context got allocated in the mean time. */
 if unlikely(PERTASK_TEST(x86_fpu_size)) {
  FPU_FREE(fpu.hp_ptr,fpu.hp_siz);
  return;
 }
 PERTASK_SET(x86_fpu_context,(struct fpu_context *)fpu.hp_ptr);
 PERTASK_SET(x86_fpu_size,fpu.hp_siz);
}


INTERN ATTR_FREETEXT void KCALL x86_fpu_initialize(void) {
 register_t temp;
 __asm__ __volatile__("clts\n"
                      "mov %%cr0, %0\n"
                      "and $(" __PP_STR(~CR0_EM) "), %0\n"
                      "or  $(" __PP_STR(CR0_MP) "), %0\n"
                      "mov %0, %%cr0\n"
                      "mov %%cr4, %0\n"
                      "or  $(" __PP_STR(CR4_OSFXSR) "), %0\n"
                      "mov %0, %%cr4\n"
                      : "=r" (temp));
}


DEFINE_PERTASK_FINI(x86_fpu_cleanup);
INTERN void KCALL x86_fpu_cleanup(struct task *__restrict thread) {
 if (FORTASK(thread,x86_fpu_size)) {
  FPU_FREE(FORTASK(thread,x86_fpu_context),
           FORTASK(thread,x86_fpu_size));
 }
}

/* Called as part of the #NM exception handler. */
INTERN int KCALL x86_fpu_interrupt_handler(void) {
 struct task *old_task,*new_task;
 struct fpu_context *state;
 /* If the task-switched flag is set, unset it because it is
  * possible that the calling task is the only one actually
  * using the FPU right now. */
 register register_t temp;
 __asm__ __volatile__("mov %%cr0, %0\n" : "=r" (temp));
 if (!(temp&CR0_TS)) return 0;
 __asm__ __volatile__("clts\n" : : : "memory");

 /* Check if a FPU context switch occurred. */
 old_task = PERCPU(x86_fpu_current);
 new_task = THIS_TASK;
 if (new_task != old_task) {
  debug_printf("[FPU] Switch context %p -> %p\n",old_task,new_task);
  asm("int3");
  if (old_task) {
   /* Save the old FPU register state. */
   PREEMPTION_DISABLE();
   state = FORTASK(old_task,x86_fpu_context);
   if (!FORTASK(old_task,x86_fpu_size)) {
    struct heapptr ptr = FPU_ALLOC();
    state = (struct fpu_context *)ptr.hp_ptr;
    COMPILER_READ_BARRIER();
    /* Check if the old task allocated its FPU context in the mean time. */
    if unlikely(FORTASK(old_task,x86_fpu_size)) {
     /* Race condition... (solve by a repeat-try-loop)
      * HINT: Because we know that `old_task' must have run in the mean time,
      *       we can also be certain that `CR0_TS' must have been set before
      *       we got back to this point. */
     PREEMPTION_ENABLE();
     FPU_FREE(ptr.hp_ptr,ptr.hp_siz);
     return 1;
    }
    FORTASK(old_task,x86_fpu_context) = state;
    FORTASK(old_task,x86_fpu_size)    = ptr.hp_siz;
    PREEMPTION_ENABLE();
   }
   __asm__ __volatile__("cli; clts; fxsave %0; sti\n"
                        : "=m" (*state)
                        :
                        : "memory");
  }
  state = FORTASK(new_task,x86_fpu_context);
  if (!state) {
   /* Initialize the default FPU register state. */
   __asm__ __volatile__("cli; clts; fninit; sti\n"
                        :
                        :
                        : "memory");
  } else {
   /* Load an existing FPU state. */
   __asm__ __volatile__("cli; clts; fxrstor %0; sti\n"
                        :
                        : "m" (*state)
                        : "memory");
  }
  PERCPU(x86_fpu_current) = new_task;
 }
 return 1;
}
#endif



DECL_END
#endif /* !CONFIG_NO_FPU */

#endif /* !GUARD_KERNEL_I386_KOS_FPU_C */
