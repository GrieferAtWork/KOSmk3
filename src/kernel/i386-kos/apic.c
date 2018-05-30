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
#ifndef GUARD_KERNEL_I386_KOS_APIC_C
#define GUARD_KERNEL_I386_KOS_APIC_C 1

#include "scheduler.h"

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kos/types.h>
#include <hybrid/align.h>
#include <i386-kos/apic.h>
#include <i386-kos/pic.h>
#include <i386-kos/pit.h>
#include <i386-kos/scheduler.h>
#include <i386-kos/tss.h>
#include <kernel/malloc.h>
#include <kernel/debug.h>
#include <kernel/sections.h>
#include <kernel/memory.h>
#include <sched/task.h>
#include <sys/io.h>
#include <i386-kos/smp.h>
#include <string.h>

DECL_BEGIN

PUBLIC ATTR_PERCPU volatile u32 x86_lapic_timer_freq;

#ifndef CONFIG_NO_SMP
DATDEF cpuid_t _cpu_count ASMNAME("cpu_count");
DATDEF struct cpu *_cpu_vector[CONFIG_MAX_CPU_COUNT] ASMNAME("cpu_vector");

INTDEF INITCALL void KCALL x86_load_cpuid(void);
INTDEF INITCALL void KCALL x86_initialize_sysenter(void);

INTERN ATTR_FREETEXT void KCALL x86_percpu_initialize(void) {
 u32 num_ticks;

 /* Load CPUID information. */
 x86_load_cpuid();
 /* Enable support for sysenter on this CPU. */
 x86_initialize_sysenter();

 /* Enable the APIC of this CPU */
 lapic_write(APIC_SPURIOUS,APIC_SPURIOUS_FENABLED | 0xff);

 /* Same as what the BOOT processor does to configure its LAPIC. */
 lapic_write(APIC_TIMER_DIVIDE,APIC_TIMER_DIVIDE_F16);
 lapic_write(APIC_TIMER,0xff | APIC_TIMER_MODE_FONESHOT | APIC_TIMER_SOURCE_FDIV);
 outb(PIT_PCSPEAKER,
     (inb(PIT_PCSPEAKER) &
        ~(PIT_PCSPEAKER_FSYNCPIT|PIT_PCSPEAKER_FINOUT)) |
          PIT_PCSPEAKER_FSYNCPIT);
 outb(PIT_COMMAND,
      PIT_COMMAND_SELEFT_F2 |
      PIT_COMMAND_ACCESS_FLOHI |
      PIT_COMMAND_MODE_FONESHOT);
 outb_p(PIT_DATA2,(PIT_HZ_DIV(100) & 0xff));
 outb  (PIT_DATA2,(PIT_HZ_DIV(100) >> 8) & 0xff);
 { u8 temp = inb(PIT_PCSPEAKER) & ~PIT_PCSPEAKER_FINOUT;
   outb(PIT_PCSPEAKER,temp);
   outb(PIT_PCSPEAKER,temp | PIT_PCSPEAKER_OUT);
 }
 lapic_write(APIC_TIMER_INITIAL,(u32)-1);
 while (inb(PIT_PCSPEAKER) & PIT_PCSPEAKER_FPIT2OUT)
     __asm__("pause");
 lapic_write(APIC_TIMER,APIC_TIMER_FDISABLED);
 num_ticks = lapic_read(APIC_TIMER_CURRENT);
 num_ticks = (((u32)-1) - num_ticks)*100;
 PERCPU(x86_lapic_timer_freq) = num_ticks;
 debug_printf(FREESTR("[APIC] CPU #%u has LAPIC timing with %u ticks per second\n"),
              THIS_CPU->cpu_id,num_ticks);

 /* Enable the. */
 lapic_write(APIC_TIMER_DIVIDE,APIC_TIMER_DIVIDE_F16);
 lapic_write(APIC_TIMER,
             X86_INTNO_PIC1_PIT |
             APIC_TIMER_MODE_FPERIODIC |
             APIC_TIMER_SOURCE_FDIV);
 lapic_write(APIC_TIMER_INITIAL,num_ticks/HZ);
 PREEMPTION_ENABLE();
}



PRIVATE ATTR_FREETEXT void KCALL apic_send_init(u8 procid) {
 lapic_write(APIC_ICR1,APIC_ICR1_MKDEST(procid));
 lapic_write(APIC_ICR0,
             APIC_ICR0_TYPE_FINIT |
             APIC_ICR0_DEST_PHYSICAL |
             APIC_ICR0_FASSERT |
             APIC_ICR0_TARGET_FICR1);
 while (lapic_read(APIC_ICR0) & APIC_ICR0_FPENDING)
     __asm__ __volatile__("pause");
}
PRIVATE ATTR_FREETEXT void KCALL apic_send_startup(u8 procid, u8 pageno) {
 lapic_write(APIC_ICR1,APIC_ICR1_MKDEST(procid));
 lapic_write(APIC_ICR0,pageno |
             APIC_ICR0_TYPE_FSIPI |
             APIC_ICR0_DEST_PHYSICAL |
             APIC_ICR0_FASSERT |
             APIC_ICR0_TARGET_FICR1);
 while (lapic_read(APIC_ICR0) & APIC_ICR0_FPENDING)
     __asm__ __volatile__("pause");
}

INTDEF byte_t x86_smp_entry[];
INTDEF byte_t x86_smp_entry_end[];

#define X86_SMP_ENTRY_IP   0x2000
INTERN ATTR_FREEBSS volatile u8 cpu_offline_mask[CEILDIV(CONFIG_MAX_CPU_COUNT,8)];
#if CEILDIV(CONFIG_MAX_CPU_COUNT,8) == 1
#define CPU_ALL_ONLINE  (ATOMIC_READ(*(u8 *)cpu_offline_mask) == 0)
#elif CEILDIV(CONFIG_MAX_CPU_COUNT,8) == 2
#define CPU_ALL_ONLINE  (ATOMIC_READ(*(u16 *)cpu_offline_mask) == 0)
#elif CEILDIV(CONFIG_MAX_CPU_COUNT,8) == 4
#define CPU_ALL_ONLINE  (ATOMIC_READ(*(u32 *)cpu_offline_mask) == 0)
#else
LOCAL bool KCALL all_all_cpus_online(void) {
 unsigned int i;
 for (i = 0; i < COMPILER_LENOF(cpu_offline_mask); ++i)
     if (ATOMIC_READ(cpu_offline_mask[i])) return false;
 return true;
}
#define CPU_ALL_ONLINE   all_all_cpus_online()
#endif

#endif


INTDEF byte_t x86_pic_acknowledge[];

/* NOTE: This code must match the text size in `x86_pic_acknowledge' */
#ifdef __x86_64__
PRIVATE ATTR_FREERODATA u8 const x86_ack_pic[18] = {
    0xb0, X86_PIC_CMD_EOI, /* movb $X86_PIC_CMD_EOI, %al */
    0xe6, X86_PIC1_CMD,    /* outb %al, $X86_PIC1_CMD */
    /* Fill the remaining space with NOPs. */
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90
};
#else
PRIVATE ATTR_FREERODATA u8 const x86_ack_pic[15] = {
    0xb0, X86_PIC_CMD_EOI, /* movb $X86_PIC_CMD_EOI, %al */
    0xe6, X86_PIC1_CMD,    /* outb %al, $X86_PIC1_CMD */
    /* Fill the remaining space with NOPs. */
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90
};
#endif


INTERN ATTR_FREETEXT void KCALL
x86_apic_initialize(void) {
 /* >> (re-)initialize the master & slave PICs.
  * Following this, each PIC will expect 3 additional "initialization words". */
 outb_p(X86_PIC1_CMD,X86_ICW1_INIT|X86_ICW1_ICW4);
 outb_p(X86_PIC2_CMD,X86_ICW1_INIT|X86_ICW1_ICW4);

 /* Word #1: Define the IRQ offsets.
  *          We map the master to 0x20..0x27,
  *          and the slave to 0x28..0x2f. */
 outb_p(X86_PIC1_DATA,X86_INTERRUPT_PIC1_BASE);
 outb_p(X86_PIC2_DATA,X86_INTERRUPT_PIC2_BASE);

 /* Word #2: Tell the master and slave how they are wired to each other. */
 outb_p(X86_PIC1_DATA,4);
 outb_p(X86_PIC2_DATA,2);

 /* Word #3: Define the environment mode. */
 outb_p(X86_PIC1_DATA,X86_ICW4_8086);
 outb_p(X86_PIC2_DATA,X86_ICW4_8086);

 if (X86_HAVE_LAPIC) {
#ifndef CONFIG_NO_SMP
  cpuid_t i;
  size_t entry_size = (size_t)(x86_smp_entry_end - x86_smp_entry);
#endif
  u32 num_ticks;

  debug_printf(FREESTR("[APIC] Enable LAPIC\n"));
  /* Disable the PIT interrupt if we're going to use the LAPIC timer. */
  outb_p(X86_PIC1_DATA,1);
  outb_p(X86_PIC2_DATA,0);

  lapic_write(APIC_SPURIOUS,
              APIC_SPURIOUS_FENABLED |
              X86_INTERRUPT_APIC_SPURIOUS);

#ifndef CONFIG_NO_SMP
  /* Copy AP entry code. */
  memcpy((void *)X86_SMP_ENTRY_IP,x86_smp_entry,entry_size);

  /* Send INIT commands to all CPUs. */
  for (i = 1; i < cpu_count; ++i) {
   cpu_offline_mask[i / 8] |= 1 << (i % 8); /* Mark the CPU as offline */
   apic_send_init(FORCPU(cpu_vector[i],x86_lapic_id));
  }
  /* NOTE: The APIC specs require us to wait for 10ms
   *       before we should send `APIC_ICR0_TYPE_FSIPI'
   *       And wouldn't you know, that's also the time that
   *       our LAPIC calibration code needs to determine the
   *       frequency of the BSP's LAPIC timer.
   *       So we just merge the two together to speed
   *       up boot time by just that time bit more. */
#endif


  /* Setup the first portion of an APIC timer. */
  lapic_write(APIC_TIMER_DIVIDE,APIC_TIMER_DIVIDE_F16);
  lapic_write(APIC_TIMER,
              X86_INTERRUPT_APIC_SPURIOUS |
              APIC_TIMER_MODE_FONESHOT |
              APIC_TIMER_SOURCE_FDIV);


  outb(PIT_PCSPEAKER,
      (inb(PIT_PCSPEAKER) &
         ~(PIT_PCSPEAKER_FSYNCPIT|PIT_PCSPEAKER_FINOUT)) |
           PIT_PCSPEAKER_FSYNCPIT);
  outb(PIT_COMMAND,
       PIT_COMMAND_SELEFT_F2 |
       PIT_COMMAND_ACCESS_FLOHI |
       PIT_COMMAND_MODE_FONESHOT);
  /* Configure the PIT to trigger after 1/100th of a second (10ms). */
  outb_p(PIT_DATA2,(PIT_HZ_DIV(100) & 0xff));
  outb  (PIT_DATA2,(PIT_HZ_DIV(100) >> 8) & 0xff);
  {
   /*  Move the speaker to trigger the one-shot timer.
    * (This is what I call a hack from the last century!) */
   u8 temp = inb(PIT_PCSPEAKER) & ~PIT_PCSPEAKER_FINOUT;
   outb(PIT_PCSPEAKER,temp);
   outb(PIT_PCSPEAKER,temp | PIT_PCSPEAKER_OUT);
  }
  /* The PIC timer is now running. */
  /* Set LAPIC counter to its maximum possible value. */
  lapic_write(APIC_TIMER_INITIAL,(u32)-1);

  /* Wait for our one-shot time to expire. */
  while (inb(PIT_PCSPEAKER) & PIT_PCSPEAKER_FPIT2OUT)
      __asm__("pause");

  /* Stop LAPIC counter */
  lapic_write(APIC_TIMER,APIC_TIMER_FDISABLED);
  num_ticks = lapic_read(APIC_TIMER_CURRENT);

#ifndef CONFIG_NO_SMP
  /* Send start IPIs to all APs. */
  for (i = 1; i < cpu_count; ++i) {
   apic_send_startup(FORCPU(cpu_vector[i],x86_lapic_id),
                    (u8)(X86_SMP_ENTRY_IP >> 12));
  }
#endif

  num_ticks = (((u32)-1) - num_ticks) * 100;
  debug_printf(FREESTR("[APIC] Boot CPU uses a LAPIC timing of %u ticks per second\n"),
               num_ticks);

  lapic_write(APIC_TIMER_DIVIDE,APIC_TIMER_DIVIDE_F16);
  lapic_write(APIC_TIMER,
              /* Set the PIT interrupt to the APIC timer. */
              X86_INTNO_PIC1_PIT |
              APIC_TIMER_MODE_FPERIODIC |
              APIC_TIMER_SOURCE_FDIV);
  lapic_write(APIC_TIMER_INITIAL,num_ticks/HZ);
  PREEMPTION_ENABLE();

#ifndef CONFIG_NO_SMP
  /* Wait for secondary CPUs to come online. */
  if (!CPU_ALL_ONLINE) {
   unsigned int timeout;
#if HZ >= 100
   /* Wait for more than a single jiffi. */
   timeout = ((HZ+1)/100);
   while (timeout--) {
    __asm__ __volatile__("hlt");
    if likely(CPU_ALL_ONLINE) goto all_online;
   }
#else
   __asm__ __volatile__("hlt"); /* XXX: Waiting for 1ms would be enough here... */
#endif

   if likely(CPU_ALL_ONLINE) goto all_online;
   /* Re-send start IPIs to all APs still not online. */
   for (i = 1; i < cpu_count; ++i) {
    if (!(ATOMIC_READ(cpu_offline_mask[i/8]) & (1 << (i % 8))))
        continue;
    debug_printf(FREESTR("[APIC] Re-attempting startup of processor #%u (LAPIC id %#.2I8x)\n"),
                 i,FORCPU(cpu_vector[i],x86_lapic_id));
    apic_send_startup(FORCPU(cpu_vector[i],x86_lapic_id),
                     (u8)(X86_SMP_ENTRY_IP >> 12));
   }
   /* Wait up to a full second for the (possibly slow?) CPUs to come online. */
   timeout = HZ;
   while (timeout--) {
    if (CPU_ALL_ONLINE) goto all_online;
    __asm__ __volatile__("hlt");
   }
   /* Check one last time? */
   if (CPU_ALL_ONLINE) goto all_online;
   /* Guess these CPUs are just broken... */
   for (i = 1; i < _cpu_count;) {
    struct cpu *discard_cpu;
    discard_cpu = _cpu_vector[i];
    discard_cpu->cpu_id = i; /* Re-assign CPU IDs to reflect removed entries. */
    if (!(ATOMIC_READ(cpu_offline_mask[i/8]) & (1 << (i % 8)))) { ++i; continue; }
    debug_printf(FREESTR("[APIC] CPU with LAPIC id %#.2I8x doesn't want to "
                         "come online (removing it from the configuration)\n"),
                 i,FORCPU(_cpu_vector[i],x86_lapic_id));
    task_destroy(discard_cpu->c_running); /* Destroy the IDLE tasks. */
#ifndef __x86_64__
    kfree((void *)(FORCPU(discard_cpu,x86_cputssdf).t_esp - CONFIG_X86_DFSTACK_SIZE));
#endif
    kfree(discard_cpu); /* Destroy the IDLE tasks. */

    /* Remove the CPU from the vector. */
    --_cpu_count;
    memmove(&_cpu_vector[i],&_cpu_vector[i+1],
            (_cpu_count-i)*sizeof(struct cpu *));
   }


  }
all_online:;
#endif
 } else {
  debug_printf(FREESTR("[APIC] LAPIC unavailable. Using PIC\n"));

  /* Enable all interrupts. */
  outb_p(X86_PIC1_DATA,0);
  outb_p(X86_PIC2_DATA,0);

  /* Re-write the preemption code to acknowledge PIC interrupts. */
  memcpy(x86_pic_acknowledge,x86_ack_pic,sizeof(x86_ack_pic));

  /* Set the PIC speed. */
  outb(PIT_COMMAND,
       PIT_COMMAND_SELEFT_F0 |
       PIT_COMMAND_ACCESS_FLOHI |
       PIT_COMMAND_MODE_FSQRWAVE |
       PIT_COMMAND_FBINARY);
  outb_p(PIT_DATA0,PIT_HZ_DIV(HZ) & 0xff);
  outb(PIT_DATA0,PIT_HZ_DIV(HZ) >> 8);

  PREEMPTION_ENABLE();
 }
}

PUBLIC NOIRQ qtime_t KCALL qtime_now_noirq(void) {
 qtime_t result;
 result.qt_jiffies = jiffies;
 if (X86_HAVE_LAPIC) {
  result.qt_qlength = lapic_read(APIC_TIMER_INITIAL);
  result.qt_qoffset = lapic_read(APIC_TIMER_CURRENT);
  if (lapic_read(APIC_TIMER) & APIC_TIMER_FPENDING) {
   /* Special case: The current quantum should have already ended. */
   ++result.qt_jiffies;
   /* This check is somewhat questionable, but is required to
    * deal with the extremely rare scenario of the current
    * quantum having ended after we read `APIC_TIMER_CURRENT',
    * but before we read `APIC_TIMER'.
    * In that case, the quantum offset is still located extremely
    * close to its end, so we must set it to ZERO because we
    * already adjusted the full jiffies counter. */
   if (result.qt_qoffset > (result.qt_qlength/2))
       result.qt_qoffset = 0;
  }
  result.qt_qoffset = result.qt_qlength - result.qt_qoffset;
 } else {
  /* Read the current PIT counter position. */
  outb(PIT_COMMAND,
       PIT_COMMAND_SELEFT_F0 |
       PIT_COMMAND_ACCESS_FLATCH |
       PIT_COMMAND_MODE_FIRQONTERM |
       PIT_COMMAND_FBINARY);
  result.qt_qoffset  = (u16)inb(PIT_DATA0);
  result.qt_qoffset |= (u16)inb(PIT_DATA0) << 8;
  /* The PIT reload value is constant. */
  result.qt_qlength  = PIT_HZ_DIV(HZ);
  result.qt_qoffset  = PIT_HZ_DIV(HZ) - result.qt_qoffset;
 }
 return result;
}
PUBLIC qtime_t KCALL qtime_now(void) {
 qtime_t result;
 pflag_t was = PREEMPTION_PUSHOFF();
 result = qtime_now_noirq();
 PREEMPTION_POP(was);
 return result;
}



DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_APIC_C */
