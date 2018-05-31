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
#ifndef GUARD_KERNEL_I386_KOS_SMP_C
#define GUARD_KERNEL_I386_KOS_SMP_C 1
#define _KOS_SOURCE 1

#include "scheduler.h"

#include <hybrid/align.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/wordbits.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kos/types.h>
#include <kernel/vm.h>
#include <kernel/malloc.h>
#include <kernel/debug.h>
#include <kernel/heap.h>
#include <sched/task.h>
#include <sched/signal.h>
#include <sched/pid.h>
#include <sched/group.h>
#include <fs/path.h>
#include <fs/handle.h>
#include <kos/context.h>
#include <i386-kos/apic.h>
#include <i386-kos/smp.h>
#include <i386-kos/tss.h>
#include <i386-kos/gdt.h>
#include <asm/cpu-flags.h>
#include <except.h>
#include <string.h>

DECL_BEGIN

PUBLIC PHYS u32 _x86_lapic_base_address_phys
    ASMNAME("x86_lapic_base_address_phys") = 0xcccccccc;
PUBLIC VIRT volatile byte_t *_x86_lapic_base_address
    ASMNAME("x86_lapic_base_address") = NULL;

#define MPFPS_ALIGN 16

#define MP_FLOATING_POINTER_SIGNATURE  \
         ENCODE_INT32('_','M','P','_')


PRIVATE void KCALL Mp_MapLAPICPhysicalBaseAddress(void);

PRIVATE ATTR_FREETEXT byte_t KCALL
smp_memsum(void const *__restrict p, size_t n_bytes) {
 byte_t result = 0;
 byte_t *iter,*end;
 end = (iter = (byte_t *)p)+n_bytes;
 for (; iter != end; ++iter) result += *iter;
 return result;
}


PRIVATE ATTR_FREETEXT MpFloatingPointerStructure *KCALL
Mp_LocateFloatingPointStructureInAddressRange(PHYS uintptr_t base, size_t bytes) {
 uintptr_t iter,end;
 end = (iter = (uintptr_t)base)+bytes;
 /* Clamp the search area to a 16-byte alignment. */
 iter = CEIL_ALIGN(iter,MPFPS_ALIGN);
 end  = FLOOR_ALIGN(end,MPFPS_ALIGN);
 if (iter < end) for (; iter != end; iter += MPFPS_ALIGN) {
  MpFloatingPointerStructure *result = (MpFloatingPointerStructure *)iter;
  if (*(u32 *)result->mp_sig != MP_FLOATING_POINTER_SIGNATURE) continue;
  /* When found, check the length and checksum. */
  if (result->mp_length >= sizeof(MpFloatingPointerStructure)/16 &&
     !smp_memsum(result,sizeof(MpFloatingPointerStructure)))
      return result;
 }
 return NULL;
}

PRIVATE ATTR_FREETEXT
MpFloatingPointerStructure *KCALL
Mp_LocateFloatingPointStructure(void) {
 MpFloatingPointerStructure *result;
 /* NOTE: No need to identity-map these, as they're all part of the
  *       first 1Gb of physical memory, which is fully mapped at this
  *       point, both in 32-bit and 64-bit mode. */
 /*        */ result = Mp_LocateFloatingPointStructureInAddressRange((uintptr_t)*(__u16 volatile *)X86_EARLY_PHYS2VIRT(0x40E),1024);
 if (!result) result = Mp_LocateFloatingPointStructureInAddressRange((uintptr_t)X86_EARLY_PHYS2VIRT((*(__u16 volatile *)X86_EARLY_PHYS2VIRT(0x413))*1024),1024);
 if (!result) result = Mp_LocateFloatingPointStructureInAddressRange((uintptr_t)X86_EARLY_PHYS2VIRT(0x0F0000),64*1024);
 return result;
}


PUBLIC ATTR_PERCPU u8 _x86_lapic_id ASMNAME("x86_lapic_id");
DATDEF cpuid_t _cpu_count ASMNAME("cpu_count");
DATDEF struct cpu *_cpu_vector[CONFIG_MAX_CPU_COUNT] ASMNAME("cpu_vector");

#ifndef CONFIG_NO_SMP
/* The task-level entry point of secondary CPUs. */
INTDEF void ASMCALL x86_smp_task0_entry(void);

INTERN void KCALL x86_cpu_idle_exception(void) {
 /* Called when an exception is propagated
  * to the root frame of an IDLE thread. */
 error_printf("Unhandled exception in IDLE thread\n");
}

INTDEF void FCALL
task_setup_kernel_environ(struct task *__restrict thread);

PRIVATE ATTR_FREETEXT ATTR_RETNONNULL struct cpu *
KCALL smp_allocate_processor(void) {
 struct cpu *result;
 struct task *idle_bootstrap;
 struct cpu_schedcontext *bootstrap_state;
 result = (struct cpu *)kmalloc((size_t)kernel_percpu_size,
                                 GFP_SHARED|GFP_LOCKED);
 /* Copy the per-cpu template into the newly allocated CPU. */
 memcpy(result,kernel_percpu_start,(size_t)kernel_percpu_size);

 /* Allocate the IDLE bootstrap task for the CPU. */
 idle_bootstrap = task_alloc();
 task_alloc_stack(idle_bootstrap,CONFIG_KERNELSTACK_SIZE/PAGESIZE);
 idle_bootstrap->t_cpu = result;
 idle_bootstrap->t_sched.sched_ring.re_prev = idle_bootstrap;
 idle_bootstrap->t_sched.sched_ring.re_next = idle_bootstrap;
 /* NOTE: `TASK_FKEEPCORE' is actually only required, because
  *        the task keeps a pointer to the CPU in a register. */
 idle_bootstrap->t_flags         = (TASK_FALWAYSKEEPCORE|TASK_FKEEPCORE|
                                    TASK_FNOSIGNALS|TASK_FKERNELJOB);
 idle_bootstrap->t_state         = TASK_STATE_FIDLETHREAD|TASK_STATE_FSTARTED;
 idle_bootstrap->t_nothrow_serve = 0x3fffffff;
 idle_bootstrap->t_vm            = &vm_kernel;
 LIST_INSERT(vm_kernel.vm_tasks,idle_bootstrap,t_vmtasks);
 vm_incref(&vm_kernel);
 bootstrap_state           = ((struct cpu_schedcontext *)idle_bootstrap->t_stackend)-1;
 idle_bootstrap->t_context = bootstrap_state;

 /* Setup environment components of the new thread. */
 task_setup_kernel_environ(idle_bootstrap);

 memset(bootstrap_state,0,sizeof(struct cpu_schedcontext));
 /* Setup the register state that is expected by `x86_secondary_cpu_idle_loop()'. */
 bootstrap_state->c_gpregs.gp_ebx  = (uintptr_t)result;          /* EBX: THIS_CPU */
 bootstrap_state->c_gpregs.gp_esi  = (uintptr_t)idle_bootstrap;  /* ESI: THIS_TASK */
 bootstrap_state->c_eip            = (uintptr_t)&x86_smp_task0_entry;
 bootstrap_state->c_iret.ir_cs     = X86_SEG_HOST_CS;
 bootstrap_state->c_iret.ir_eflags = EFLAGS_IF;
#if !defined(CONFIG_NO_X86_SEGMENTATION) && !defined(__x86_64__)
 bootstrap_state->c_segments.sg_ds = X86_SEG_HOST_DS;
 bootstrap_state->c_segments.sg_es = X86_SEG_HOST_ES;
 bootstrap_state->c_segments.sg_fs = X86_SEG_HOST_FS;
 bootstrap_state->c_segments.sg_gs = X86_SEG_HOST_GS;
#endif /* !CONFIG_NO_X86_SEGMENTATION */
 /* Also initialize the GDT segment base
  * addresses for the HOST_TLS and TSS segments. */
 { struct x86_segment *tls_segment; struct x86_tss *tss;
   tls_segment = &FORCPU(result,x86_cpugdt[X86_SEG_HOST_TLS]);
   X86_SEGMENT_STBASE(*tls_segment,(uintptr_t)idle_bootstrap);
   tls_segment = &FORCPU(result,x86_cpugdt[X86_SEG_CPUTSS]);
   tss         = &FORCPU(result,x86_cputss);
   X86_SEGMENT_STBASE(*tls_segment,(uintptr_t)tss);
   /* And finally, setup the ESP0 pointer in the TSS for the bootstrap task. */
#ifdef __x86_64__
   tss->t_rsp0 = (u64)idle_bootstrap->t_stackend;
#else
   tss->t_esp0 = (u32)idle_bootstrap->t_stackend;
#endif

#ifndef __x86_64__
   /* Also setup the TSS used for handling #DF exceptions. */
   tls_segment = &FORCPU(result,x86_cpugdt[X86_SEG_CPUTSS_DF]);
   tss         = &FORCPU(result,x86_cputssdf);
   X86_SEGMENT_STBASE(*tls_segment,(uintptr_t)tss);
   /* Allocate a small stack for the #DF stack */
   { struct heapptr df_stack;
     df_stack = heap_alloc_untraced(&kernel_heaps[GFP_SHARED|GFP_LOCKED],
                                     CONFIG_X86_DFSTACK_SIZE,
                                     GFP_SHARED|GFP_LOCKED);
     tss->t_esp = (uintptr_t)df_stack.hp_ptr+df_stack.hp_siz;
   }
   tss->t_esp0 = tss->t_esp;
   tss->t_esp1 = tss->t_esp;
   tss->t_esp2 = tss->t_esp;
#endif

 }

 /* With the bootstrap task now initialized, set it as the running task of the CPU. */
 result->c_running = idle_bootstrap;

 /* Register the CPU. */
 _cpu_vector[_cpu_count] = result;
 result->cpu_id          = _cpu_count;
 ++_cpu_count;
 return result;
}

#endif


INTERN ATTR_FREETEXT void KCALL x86_smp_initialize(void) {
 MpFloatingPointerStructure *fps;
 MpConfigurationTable *table;
 TRY {
  fps = Mp_LocateFloatingPointStructure();
  if (!fps) return;
  debug_printf("[SMP] MPFPS structure at %p (v1.%I8u; defcfg %I8u)\n",
               fps,fps->mp_specrev,fps->mp_defcfg);
  if (fps->mp_defcfg) {
   /* TODO: Default configuration. */
   return;
  }
  /* Check pointer location. */
  if (fps->mp_cfgtab >= 0x40000000)
      return;
  table = (MpConfigurationTable *)X86_EARLY_PHYS2VIRT((uintptr_t)fps->mp_cfgtab);
  /* Check signature. */
  if (*(u32 *)table->tab_sig != ENCODE_INT32('P','C','M','P'))
      return;
  /* Check length. */
  if (table->tab_length < offsetafter(MpConfigurationTable,tab_chksum))
      return;
  /* Check checksum. */
  if (smp_memsum(table,table->tab_length))
      return;
  /* Load the LAPIC base address. */
  _x86_lapic_base_address_phys = table->tab_lapicaddr;
  Mp_MapLAPICPhysicalBaseAddress(); /* Map the LAPIC into physical memory. */
  /* Process configuration entries. */
  {
   MpConfigurationEntry *entry,*end; u16 i,count;
   entry = (MpConfigurationEntry *)(table+1);
   end   = (MpConfigurationEntry *)((uintptr_t)table+table->tab_length);
   count = table->tab_entryc;
   for (i = 0; i < count && entry < end; ++i) {
    switch (entry->mp_type) {

    case MPCFG_PROCESSOR:
#ifndef CONFIG_NO_SMP
     if (entry->mp_processor.p_cpuflag&
        (MP_PROCESSOR_FENABLED|MP_PROCESSOR_FBOOTPROCESSOR)) {
      struct cpu *that_cpu = &_boot_cpu;
      if (!(entry->mp_processor.p_cpuflag&MP_PROCESSOR_FBOOTPROCESSOR)) {
       /* If we've already loaded the max number of
        * configurable processors, don't load even more. */
       if unlikely(_cpu_count >= CONFIG_MAX_CPU_COUNT) {
        debug_printf(FREESTR("[SMP] Cannot configure additional Processor with LAPIC ID %#.2I8x\n"),
                     entry->mp_processor.p_lapicid);
        goto skip_processor;
       }
       /* Allocate a new processor descriptor. */
       that_cpu = smp_allocate_processor();
       debug_printf(FREESTR("[SMP] New processor #%u with LAPIC ID %#.2I8x\n"),
                    that_cpu->cpu_id,entry->mp_processor.p_lapicid);
      } else {
       debug_printf(FREESTR("[SMP] Boot processor has LAPIC ID %#.2I8x\n"),
                    entry->mp_processor.p_lapicid);
      }
      FORCPU(that_cpu,_x86_lapic_id) = entry->mp_processor.p_lapicid;
     }
skip_processor:
#endif
     *(uintptr_t *)&entry += 20;
     break;

    default: *(uintptr_t *)&entry += 8; break;
    }
   }
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
 }
}




PRIVATE struct vm_region lapic_region = {
    .vr_refcnt = 1,
    .vr_lock   = MUTEX_INIT,
    .vr_futex  = ATOMIC_RWPTR_INIT(NULL),
    .vr_type   = VM_REGION_PHYSICAL,
    .vr_init   = VM_REGION_INIT_FNORMAL,
    .vr_flags  = VM_REGION_FIMMUTABLE|VM_REGION_FDONTMERGE|VM_REGION_FLEAKINGPARTS,
    .vr_funds  = 0,
    .vr_size   = 0, /* Filled later. */
    .vr_parts  = &lapic_region.vr_part0,
    .vr_part0  = {
        .vp_refcnt = 2,
        .vp_chain = {
            .le_pself = &lapic_region.vr_parts
        },
        .vp_start  = 0,
        .vp_state  = VM_PART_INCORE,
        .vp_flags  = VM_PART_FKEEP|VM_PART_FWEAKREF,
        .vp_locked = 0x3fff,
        .vp_phys   = {
            .py_num_scatter = 1,
            .py_iscatter = {
                [0] = {
                    .ps_addr = 0, /* Filled later. */
                    .ps_size = 0  /* Filled later. */
                }
            }
        }
    }
};
PRIVATE struct vm_node lapic_node = {
    .vn_start   = 0,
    .vn_region  = &lapic_region,
    .vn_notify  = NULL,
    .vn_closure = NULL,
    .vn_prot    = PROT_READ|PROT_WRITE|PROT_NOUSER,
    .vn_flag    = VM_NODE_FNORMAL|VM_NODE_FIMMUTABLE
};


PRIVATE ATTR_FREETEXT void KCALL
Mp_MapLAPICPhysicalBaseAddress(void) {
 size_t lapic_num_pages;
 lapic_num_pages  = _x86_lapic_base_address_phys & (PAGESIZE-1);
 lapic_num_pages += APIC_SIZE;
 lapic_num_pages += (PAGESIZE-1);
 lapic_num_pages /= PAGESIZE;
 assertf(lapic_num_pages != 0 && lapic_num_pages <= 3,
         FREESTR("Shouldn't be more than 3 pages... (is %Iu)"),
         lapic_num_pages);
 /* Saved the effective page address of the LAPIC into the VM. */
 lapic_region.vr_size                                 = lapic_num_pages;
 lapic_region.vr_part0.vp_phys.py_iscatter[0].ps_addr = VM_ADDR2PAGE(_x86_lapic_base_address_phys);
 lapic_region.vr_part0.vp_phys.py_iscatter[0].ps_size = lapic_num_pages;

 vm_acquire(&vm_kernel);
 /* Find a suitable location where we can map the LAPIC.
  * NOTE: No TRY-FINALLY here because at this point we still know
  *       that the kernel VM will (must) have enough virtual address
  *       space. (Plus: if this fails, the kernel will just PANIC(),
  *                     which needs no cleanup...) */
 lapic_node.vn_node.a_vmin = vm_getfree(X86_VM_LAPIC_HINT,lapic_num_pages,1,0,X86_VM_LAPIC_MODE);
 lapic_node.vn_node.a_vmax = lapic_node.vn_node.a_vmin + lapic_num_pages-1;
 _x86_lapic_base_address = (byte_t *)(VM_PAGE2ADDR(lapic_node.vn_node.a_vmin) +
                                     (_x86_lapic_base_address_phys & (PAGESIZE-1)));
 debug_printf("[SMP] Mapping LAPIC to %p...%p (base at %p, phys at %.8I32X)\n",
              VM_PAGE2ADDR(lapic_node.vn_node.a_vmin),
              VM_PAGE2ADDR(lapic_node.vn_node.a_vmax+1)-1,
              _x86_lapic_base_address,
              _x86_lapic_base_address_phys);
 /* Insert and map the LAPIC node. */
 vm_insert_and_activate_node(&vm_kernel,&lapic_node);
 vm_release(&vm_kernel);
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_SMP_C */
