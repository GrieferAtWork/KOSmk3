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
#ifndef GUARD_KERNEL_I386_KOS_BOOT_C
#define GUARD_KERNEL_I386_KOS_BOOT_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/section.h>
#include <i386-kos/gdt.h>
#include <i386-kos/idt_pointer.h>
#include <i386-kos/memory.h>
#include <kernel/debug.h>
#include <kernel/memory.h>
#include <kernel/sections.h>
#include <kernel/malloc.h>
#include <kernel/heap.h>
#include <kernel/vm.h>
#include <kernel/interrupt.h>
#include <i386-kos/driver.h>
#include <kos/i386-kos/bits/thread.h>
#include <proprietary/multiboot.h>
#include <proprietary/multiboot2.h>
#include <sched/task.h>
#include <fs/linker.h>
#include <fs/driver.h>
#include <sys/io.h>
#include <string.h>
#include <stdlib.h>
#include <fs/ramfs.h>

#undef CALL_OLD_KERNEL_MAIN
#if 0
#define CALL_OLD_KERNEL_MAIN 1
#endif

#ifdef CALL_OLD_KERNEL_MAIN
#include "../__ice__/main.c"
#endif

DECL_BEGIN

struct bootstack_init {
     struct meminfo is_predef[MEMINSTALL_EARLY_PREDEFINED];
     struct meminfo is_init[(MEMINSTALL_EARLY_BUFSIZE / sizeof(struct meminfo))-MEMINSTALL_EARLY_PREDEFINED];
};
INTDEF struct bootstack_init x86_boot_stack;
INTDEF struct meminfo const predefined_meminfo[MEMINSTALL_EARLY_PREDEFINED];

struct heap_hint {
    vm_vpage_t   hh_page;
    unsigned int hh_mode;
};
INTDEF struct heap_hint kernel_heap_hints[__GFP_HEAPCOUNT];



struct mb_tag {
    u32 mt_magic;
    u32 mt_flags;
    u32 mt_checksum;
};

/* Multiboot header */
#define MB_FLAGS       (MB_PAGE_ALIGN|MB_MEMORY_INFO)
/*      MB_FLAGS       (MB_PAGE_ALIGN|MB_MEMORY_INFO|MB_VIDEO_MODE)*/
PRIVATE ATTR_SECTION(".multiboot")
        ATTR_ALIGNED(MB_HEADER_ALIGN)
ATTR_USED struct mb_tag mb_multiboot = {
    .mt_magic    =   MB_HEADER_MAGIC,
    .mt_flags    =   MB_FLAGS,
    .mt_checksum = (u32)-(MB_HEADER_MAGIC+MB_FLAGS),
};

/* Multiboot2 header */
#define MB2_TAG   ATTR_USED ATTR_SECTION(".multiboot2.tag") ATTR_ALIGNED(MB2_TAG_ALIGN)

PRIVATE ATTR_SECTION(".multiboot2.tag.end")
        ATTR_ALIGNED(MB2_TAG_ALIGN)
ATTR_USED struct mb2_header_tag tag_empty = {
    .type  = MB2_HEADER_TAG_END,
    .size  = sizeof(struct mb2_header_tag),
};

#if __SIZEOF_POINTER__ >= 8
INTDEF byte_t kernel_multiboot2_hdrlen_chksum[];
#else
INTDEF byte_t kernel_multiboot2_hdrlen[];
INTDEF byte_t kernel_multiboot2_chksum[];
#endif
PRIVATE ATTR_SECTION(".multiboot2")
        ATTR_ALIGNED(MB2_HEADER_ALIGN)
ATTR_USED struct mb2_header mb_multiboot2 = {
    .magic         =  MB2_HEADER_MAGIC,
    .architecture  =  MB2_ARCHITECTURE,
    {
#if __SIZEOF_POINTER__ >= 8
        .header_length_and_checksum = (u64)kernel_multiboot2_hdrlen_chksum
#else
        .header_length = (u32)kernel_multiboot2_hdrlen,
        .checksum      = (u32)kernel_multiboot2_chksum
#endif
    }
};


/* The CPU context as it was when the KOS was started. */
INTERN ATTR_FREEBSS struct x86_context32 x86_boot_context;

INTDEF void KCALL x86_load_mb1info(PHYS mb_info_t *__restrict info);
INTDEF void KCALL x86_load_mb2info(PHYS u8 *__restrict info);

#ifndef CONFIG_NO_SMP
INTDEF byte_t boot_cpu_start[];
#endif /* !CONFIG_NO_SMP */
INTDEF byte_t boot_task_start[];
INTDEF byte_t boot_vm_start[];

INTDEF void KCALL x86_apic_initialize(void);
INTDEF void KCALL x86_interrupt_initialize(void);
INTDEF void KCALL x86_scheduler_initialize(void);
PRIVATE void KCALL x86_coredriver_initialize(void);
INTDEF void KCALL x86_smp_initialize(void);
#ifndef CONFIG_NO_SMP
INTDEF void KCALL x86_boot_secondary_cpus(void);
#endif

#ifndef CONFIG_NO_FPU
INTDEF INITCALL void KCALL x86_fpu_initialize(void);
#endif /* !CONFIG_NO_FPU */
#ifndef CONFIG_NO_X86_SYSENTER
INTDEF INITCALL void KCALL x86_initialize_sysenter(void);
#endif /* !CONFIG_NO_X86_SYSENTER */
#ifdef __x86_64__
INTDEF INITCALL void KCALL x86_fixup_fsgsbase(void);
#endif
INTDEF INITCALL void KCALL x86_load_cpuid(void);
INTDEF INITCALL void KCALL x86_mem_relocate_info(void);
INTDEF INITCALL void KCALL x86_mem_construct_zones(void);
INTDEF INITCALL void KCALL x86_mem_construct_zone_nodes(void);
INTDEF INITCALL void KCALL x86_initialize_kernel_vm_mappings(void);
INTDEF INITCALL void KCALL x86_delete_virtual_memory_identity(void);
INTDEF INITCALL void KCALL x86_mount_rootfs(void);
INTERN ATTR_NORETURN INITCALL void KCALL x86_switch_to_userspace(void);

INTDEF INITCALL void KCALL x86_configure_paging(void);
INTDEF INITCALL void KCALL kernel_relocate_commandline(void);
INTDEF INITCALL void KCALL kernel_eval_commandline(void);

INTERN ATTR_FREETEXT void KCALL x86_kernel_main(void) {
 debug_printf(FREESTR("[INIT] Hello!\n"));

 /* Load the kernel binary into the addr2line cache of `magic.dee' */
 debug_printf(FREESTR("%%{vload:%s}"),
              FREESTR(KERNEL_BIN_FILENAME));

#ifndef __x86_64__ /* X86_64 does this from assembly. */
 /* Initialize the PER-context object segments for the BOOT process. */
#ifndef CONFIG_NO_SMP
 memcpy(boot_cpu_start,kernel_percpu_start,(size_t)kernel_percpu_size);
#endif /* !CONFIG_NO_SMP */
 memcpy(boot_task_start,kernel_pertask_start,(size_t)kernel_pertask_size);
 memcpy(boot_vm_start,kernel_pervm_start,(size_t)kernel_pervm_size);
#endif

 /* Mark the boot task as having started (which we couldn't do
  * until now because we still had to copy the pertask template
  * into the boot task TLS segment). */
 _boot_task.t_state |= TASK_STATE_FSTARTED;

#ifndef __x86_64__
 /* Load the initial (boot) GDT. (now that it's been copied) */
 {
  struct x86_idt_pointer gdt;
  gdt.ip_limit = sizeof(x86_cpugdt)-1;
  gdt.ip_gdt   = FORCPU(&_boot_cpu,x86_cpugdt);
  __asm__ __volatile__("lgdt %0\n" : : "m" (gdt) : "memory");
 }

 {
  register register_t temp;
  /* With the new GDT now loaded, load the new segments. */
  __asm__ __volatile__("    movw %[ds], %w[r]\n"
                       "    movw %w[r], %%ds\n"
                       "    movw %w[r], %%es\n"
                       "    movw %w[r], %%ss\n"
                       "    movw %[fs], %w[r]\n"
                       "    movw %w[r], %%fs\n"
                       "    movw %[gs], %w[r]\n"
                       "    movw %w[r], %%gs\n"
                       "    ljmp %[cs], $1f\n"
                       "1:\n"
                       "    movw %[tr], %w[r]\n"
                       "    ltr %w[r]\n"
                       : [r]  "=r" (temp)
                       : [ds] "i" (X86_KERNEL_DS)
                       , [fs] "i" (X86_SEG_FS)
                       , [gs] "i" (X86_SEG_GS)
                       , [cs] "i" (X86_KERNEL_CS)
                       , [tr] "i" (X86_SEG(X86_SEG_CPUTSS))
                       : "memory");
 }
#endif

 /* Initialize CPUID information. */
 x86_load_cpuid();

#ifdef __x86_64__
 /* Fixup use of the (rd|wr)(fs|gs)base instructions, so we can
  * start using them (or rather, their `safe_*' counterparts).
  * What this means is, that if the hosting CPU doesn't support
  * those instruction, we will replace all of their appearances
  * with function class to wrappers that emulate their behavior
  * using MSR instructions (which _must_ be supported to even
  * get into long mode). */
 x86_fixup_fsgsbase();
#endif

 /* Initialize the interrupt handling subsystem. */
 x86_interrupt_initialize();

 /* Initialize some basic boot template components. */
 x86_scheduler_initialize();

 /* Collect information about the host. */
 x86_configure_paging();

 /* Copy predefined memory information. */
 memcpy(x86_boot_stack.is_predef,
        predefined_meminfo,
        sizeof(predefined_meminfo));

 /* Load multiboot information.
  * NOTE: All information provided by the bootloader is assumed
  *       to be located within the first 1Gb of physical memory! */
 if (x86_boot_context.c_gpregs.gp_eax == MB_BOOTLOADER_MAGIC) {
  /* Multiboot Mk#1 */
  x86_load_mb1info((mb_info_t *)(uintptr_t)x86_boot_context.c_gpregs.gp_ebx);
 } else if (x86_boot_context.c_gpregs.gp_eax == MB2_BOOTLOADER_MAGIC) {
  /* Multiboot Mk#2 */
  x86_load_mb2info((u8 *)(uintptr_t)x86_boot_context.c_gpregs.gp_ebx);
 }

 /* Determine the total amount of usable memory (excluding pre-allocated data). */
 {
  size_t total_ram = 0;
  struct meminfo const *iter;
  MEMINFO_FOREACH(iter) {
   if (iter->mi_type == MEMTYPE_RAM)
       total_ram += MEMINFO_SIZE(iter);
  }
  /* If the Bootloader didn't locate enough memory (<= 10Mb), search for more ourself. */
  if (total_ram <= 0x00A00000) {
   /* TODO: Do all the bios interrupts ourself */
  }
 }

 /* Construct memory zones. */
 x86_mem_construct_zones();
 
 /* Since we're about to use randomization, we somehow
  * need to generate just a tiny bit of entropy first.
  * So we're just going to read the CMOS RTC state and
  * use it to set the initial kernel seed. */
 {
  PRIVATE ATTR_FREERODATA u8 const cmos_registers[] = {
      0x00, /* CMOS_SECOND  */
      0x02, /* CMOS_MINUTE  */
      0x04, /* CMOS_HOUR    */
      0x07, /* CMOS_DAY     */
      0x08, /* CMOS_MONTH   */
      0x09  /* CMOS_YEAR    */
  };
  unsigned int i;
  u8 entropy[COMPILER_LENOF(cmos_registers)];
  u32 boot_seed = 0;
  for (i = 0; i < COMPILER_LENOF(cmos_registers); ++i) {
   outb(0x70,cmos_registers[i]);
   entropy[i] = inb(0x71);
  }
  /* Combine entropy to generate our boot seed. */
  for (i = 0; i < COMPILER_LENOF(cmos_registers); ++i) {
   boot_seed <<= (32/COMPILER_LENOF(cmos_registers));
   boot_seed  ^= entropy[i];
  }
  /*boot_seed = 0x8c8b4498;*/

  /* Set the boot seed for our pseudo-random number generator. */
  srand(boot_seed);
  debug_printf(FREESTR("[BOOT] Set pseudo RNG seed to %.8I32x\n"),boot_seed);
 }

 /* Add a random twist on where exactly the kernel heap will be placed.
  * This counteracts faulty code that would otherwise only work because
  * `kmalloc()' always allocates its memory at the same location.
  * (which it doesn't because of this (as well as some other options unrelated to this)) */
 {
  unsigned int i;
  u32 random_data = rand();
  for (i = 0; i < COMPILER_LENOF(kernel_heaps); ++i) {
   u8 diff = random_data & 0xff;
   if (kernel_heaps[i].h_hintmode & VM_GETFREE_FBELOW)
        kernel_heaps[i].h_hintpage -= diff;
   else kernel_heaps[i].h_hintpage += diff;
   random_data >>= 8;
  }
 }

 /* Install VM nodes & regions describing the
  * kernel itself (use `VM_REGION_RESERVED' as type)
  * This is required to prevent the memory used by the
  * kernel core being allocated for other purposes,
  * such as for dynamic memory. */
 x86_initialize_kernel_vm_mappings();

 /* Create VM nodes for memory used by the
  * physical memory allocator for zoning. */
 x86_mem_construct_zone_nodes();

#ifndef CONFIG_NO_X86_SYSENTER
 /* Initialize support for `sysenter' if available. */
 x86_initialize_sysenter();
#endif /* !CONFIG_NO_X86_SYSENTER */

#ifndef CONFIG_NO_FPU
 /* Initialize the FPU. */
 x86_fpu_initialize();
#endif /* !CONFIG_NO_FPU */

 /* Initialize secondary processors.
  * NOTE: This is done in all configurations, as it includes
  *       initialization of APIC / LAPIC data structures. */
 x86_smp_initialize();

 /* Initialize the PIC/APIC to enable the PIC
  * interrupts driving our preemptive scheduler. */
 x86_apic_initialize();

 /* Relocate memory used for `mem_info_v' into GFP_SHARED memory. */
 x86_mem_relocate_info();

 /* Relocate the bootloader kernel commandline into GFP_SHARED memory. */
 kernel_relocate_commandline();

 /* TODO: Load drivers passed by the bootloader. */

 /* Unpreserve previously reserved memory. */
 mem_unpreserve();

 /* Unmap all virtual memory that isn't mapped by a VM node. */
 x86_delete_virtual_memory_identity();

 pagedir_syncall();

 /* Add the kernel driver itself to the kernel VM's app vector. */
 vm_acquire(&vm_kernel);
 vm_apps_append(&vm_kernel,&kernel_driver.d_app);
 vm_release(&vm_kernel);

 /* Evaluate the kernel commandline. */
 kernel_eval_commandline();

 /* Initialize core drivers. */
 x86_coredriver_initialize();

 /* Mount the initial VFS filesystem root. */
 x86_mount_rootfs();

 debug_printf(FREESTR("[INIT] Driver init done\n"));

#ifdef CALL_OLD_KERNEL_MAIN
 kernel_main();
#endif

#if 0
 /* Take away a whole bunch of physical memory to
  * simulate how KOS behaves in this situation. */
 page_malloc(0x4000,MZONE_ANY);
 page_malloc(0x3d20,MZONE_ANY);
 page_malloc(8,MZONE_ANY);
 page_malloc(8,MZONE_ANY);
 page_malloc(8,MZONE_ANY);
#endif

 /* Do the initial switch to user-space. */
 x86_switch_to_userspace();

}



typedef void (KCALL *coreinit_t)(void);
INTDEF coreinit_t kernel_coredriver_init_start[];
INTDEF coreinit_t kernel_coredriver_init_end[];

PRIVATE ATTR_FREETEXT void KCALL x86_coredriver_initialize(void) {
 coreinit_t *iter;
 for (iter = kernel_coredriver_init_start;
      iter < kernel_coredriver_init_end; ++iter)
      (**iter)();
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_BOOT_C */
