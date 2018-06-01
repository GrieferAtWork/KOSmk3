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
#ifndef GUARD_KERNEL_I386_KOS_INIT_C
#define GUARD_KERNEL_I386_KOS_INIT_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/section.h>
#include <kos/kdev_t.h>
#include <kernel/interrupt.h>
#include <kernel/heap.h>
#include <kernel/debug.h>
#include <kernel/boot.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sched/task.h>
#include <sched/userstack.h>
#include <sched/pid.h>
#include <unwind/debug_line.h>
#include <fs/driver.h>
#include <fs/device.h>
#include <fs/linker.h>
#include <fs/node.h>
#include <fs/path.h>
#include <kernel/environ.h>
#include <unwind/eh_frame.h>
#include <unwind/linker.h>
#include <string.h>
#include <except.h>
#include <stdlib.h>
#include <elf.h>
#include <kos/context.h>
#include <i386-kos/gdt.h>
#include <asm/cpu-flags.h>

DECL_BEGIN

#ifndef KERNEL_CORE_BASE
#define KERNEL_CORE_BASE KERNEL_BASE
#endif


/* Mount the kernel VFS root filesystem. */
INTERN ATTR_FREETEXT
void KCALL x86_mount_rootfs(void) {
 REF struct block_device *EXCEPT_VAR dev;
 REF struct superblock *EXCEPT_VAR fs;
 /* XXX: Ask the BIOS for what the boot device was? */
 /* XXX: Bootloader drivers should have a word in this. */
 /* ... */ dev = (REF struct block_device *)try_lookup_device(DEVICE_TYPE_FBLOCKDEV,MKDEV(3,0));
 if (!dev) dev = (REF struct block_device *)try_lookup_device(DEVICE_TYPE_FBLOCKDEV,MKDEV(3,64));
 if (!dev) dev = (REF struct block_device *)try_lookup_device(DEVICE_TYPE_FBLOCKDEV,MKDEV(22,0));
 if (!dev) dev = (REF struct block_device *)try_lookup_device(DEVICE_TYPE_FBLOCKDEV,MKDEV(22,64));
 if (!dev) {
  /* TODO: Create a ramfs */
  debug_printf("[FS] Boot partition could not be located\n");
  return;
 }

 TRY {
  /* Automatically open the block-device.
   * Drivers passed via the bootloader can have a word in this. */
  fs = superblock_open(dev,NULL,NULL);
  TRY {
   /* Mount the filesystem in the root node. */
   path_mount(&THIS_VFS->v_root,(struct inode *)fs->s_root);
  } FINALLY {
   superblock_decref(fs);
  }
 } FINALLY {
  device_decref(&dev->b_device);
 }
}


DATDEF size_t          _mem_info_c ASMNAME("mem_info_c");
DATDEF struct meminfo *_mem_info_v ASMNAME("mem_info_v");
DATDEF struct meminfo *_mem_info_last ASMNAME("mem_info_last");

PRIVATE ATTR_FREERODATA char const default_init[] = "/bin/init";
PRIVATE ATTR_FREEDATA char const *init_exe = default_init;
DEFINE_DRIVER_STRING(init_exe,"init");

/* Do the initial switch to user-space by launching `/bin/init' */
INTERN ATTR_NORETURN ATTR_FREETEXT
void KCALL x86_switch_to_userspace(void) {
 REF struct module *init_mod;
 REF struct application *init_app;
 REF struct vm *init_vm;
 size_t init_exe_len;

 /* NOTE: No proper exception handling because failure here
  *       cannot be recovered from, as the kernel will be
  *       unable to transition to user-space. */

 {
  REF struct path *init_path;
  REF struct inode *init_node;
  init_exe_len = strlen(init_exe);
  init_path = fs_path(NULL,
                      init_exe,
                      init_exe_len,
                     &init_node,
                      FS_MODE_FNORMAL);
  /* Check if the initial executable is a regular file. */
  if unlikely(!INODE_ISREG(init_node))
     error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_NOTFILE);
  /* Open the init executable as a module. */
  init_mod = module_open((struct regular_node *)init_node,init_path);
  /* Check if the module has an entry point. */
  if unlikely(!(init_mod->m_flags & MODULE_FENTRY))
     error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_NOENTRY);
  inode_decref(init_node);
  path_decref(init_path);
 }

 /* Construct a new VM for the init application. */
 init_vm = vm_alloc();
 assert(THIS_TASK == &_boot_task);
 assert(_boot_task.t_vm == &vm_kernel);

 /* Switch to the new VM.
  * This is what is being warned about in the
  * documentation of the `t_vm' field for the
  * boot task. */
 PREEMPTION_DISABLE();
 atomic_rwlock_write(&vm_kernel.vm_tasklock);
 LIST_REMOVE(&_boot_task,t_vmtasks);
 LIST_INSERT(init_vm->vm_tasks,&_boot_task,t_vmtasks);
 atomic_rwlock_endwrite(&vm_kernel.vm_tasklock);
 _boot_task.t_vm = init_vm; /* Inherit reference. */

 {
  /* Assign PID #1 to the boot task. */
  REF struct thread_pid *pid;
  pid = pidns_newpid(&pidns_kernel,1);
  pid->tp_task = &_boot_task;
  FORTASK(&_boot_task,_this_pid) = pid; /* Inherit reference. */
 }

 /* Update the VM context.
  * NOTE: The kernel-share segment is already mapped in the new VM. */
 pagedir_set((pagedir_t *)(uintptr_t)init_vm->vm_physdir);

 /* With the new context now active, re-enable preemption. */
 PREEMPTION_ENABLE();
 COMPILER_BARRIER();

 /* Generate a random sysbase value for the boot task. */
 FORTASK(&_boot_task,x86_sysbase) = X86_SYSBASE_RAND();

 /* All right! Now create and load /bin/init as an application. */
 init_app = application_alloc(APPLICATION_TYPE_FUSERAPP);
 init_app->a_module = init_mod; /* Inherit reference. */

 /* Load the /bin/init application. */
 application_loadroot(init_app,
                      DL_OPEN_FGLOBAL,
                      "/lib:/usr/lib");

 assert(init_app->a_refcnt >= 2);
 application_decref(init_app);

 /* Construct the initial application environment for /bin/init */
 environ_create(init_exe,init_exe_len);

 {
  struct userstack *stack;
  struct cpu_hostcontext_user ctx;
  /* Allocate a stack for user-space. */
  stack = task_alloc_userstack();

  /* Allocate a user-space thread segment for the boot task. */
  task_alloc_userseg();
  memset(&ctx,0,sizeof(struct cpu_hostcontext_user));
#ifdef __x86_64__
  ctx.c_iret.ir_rip = init_app->a_loadaddr+init_mod->m_entry;
  if (init_app->a_module->m_machine == EM_386) {
   /* XXX: Use 32-bit user-task-segments. */
   WR_USER_FSBASE((u64)_boot_task.t_userseg);
#ifndef CONFIG_NO_DOS_COMPAT
   WR_USER_GSBASE((u64)&_boot_task.t_userseg->ts_tib);
#endif /* !CONFIG_NO_DOS_COMPAT */
   ctx.c_iret.ir_cs = X86_USER_CS32;
   ctx.c_iret.ir_ss = X86_SEG_USER_SS32;
  } else {
   WR_USER_GSBASE((u64)_boot_task.t_userseg);
#ifndef CONFIG_NO_DOS_COMPAT
   WR_USER_FSBASE((u64)&_boot_task.t_userseg->ts_tib);
#endif /* !CONFIG_NO_DOS_COMPAT */
   ctx.c_iret.ir_cs = X86_USER_CS;
   ctx.c_iret.ir_ss = X86_SEG_USER_SS;
  }
  ctx.c_iret.ir_rflags = EFLAGS_IF;
  ctx.c_iret.ir_rsp    = VM_PAGE2ADDR(stack->us_pageend);
#else
  set_user_tls_register(_boot_task.t_userseg);
#ifndef CONFIG_NO_DOS_COMPAT
  set_user_tib_register(&_boot_task.t_userseg->ts_tib);
#endif /* !CONFIG_NO_DOS_COMPAT */
  ctx.c_iret.ir_eip     = init_app->a_loadaddr+init_mod->m_entry;
  ctx.c_iret.ir_cs      = X86_USER_CS;
  ctx.c_iret.ir_eflags  = EFLAGS_IF;
  ctx.c_iret.ir_useresp = VM_PAGE2ADDR(stack->us_pageend);
  ctx.c_iret.ir_ss      = X86_SEG_USER_SS;
#ifndef CONFIG_NO_X86_SEGMENTATION
  ctx.c_segments.sg_gs  = X86_SEG_USER_GS;
  ctx.c_segments.sg_fs  = X86_SEG_USER_FS;
#ifndef CONFIG_X86_FIXED_SEGMENTATION
  ctx.c_segments.sg_es  = X86_SEG_USER_ES;
  ctx.c_segments.sg_ds  = X86_SEG_USER_DS;
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
#else /* !CONFIG_NO_X86_SEGMENTATION */
  /* Add user-space permissions to segments we're going to share with it. */
  __asm__ __volatile__("movw %w0, %%ds\n"
                       "movw %w0, %%es\n"
                       :
                       : "q" (X86_USER_DS)
                       : "memory");
#endif /* CONFIG_NO_X86_SEGMENTATION */
#endif

  /* Queue library initializers for all loaded user-space application. */
  vm_apps_initall(&ctx);

  /* Wait for all boot worker threads to terminate
   * These threads are allowed to run code apart of the .free section,
   * meaning they must all finish before we can delete such code.
   * Additionally, this way user-space is first launched once all
   * devices have been properly initialized (s.a. setup code in /bin/init). */
  kernel_join_bootworkers();

#if 0
  cpu_setcontext((struct cpu_context *)&ctx);
#else
  /* Free the kernel's .free section.
   * NOTE: Any race conditions that could normally cause our own
   *       code to become remapped as it is still running are
   *       negated by the fact that we're the only process there is
   *       right now (except for the IDLE processes of other CPUs).
   *       So with that in mind, we have nothing to fear from anyone! */
  {
   INTDEF INITDATA struct vm_node kernel_free_node;
   struct meminfo *iter = _mem_info_v;
   /* Update the kernel module range. */
   kernel_module.m_imageend = (uintptr_t)kernel_rwnf_end_raw;

#if 0 /* Not required, because the kernel's FDE cache is intended to keep
       * on including cache information for the kernel's .free section.
       * This is required so we can keep on using unwind information for
       * functions we relocated from `.free' (such as the sysenter segment) */
   /* Must clear the kernel's FDE cache, because if it was used, it will
    * have optimized itself for the kernel module's memory layout, which
    * we just changed. - Clearing the cache will cause it to re-calculate
    * those optimizations the next time it is used. */
   fde_cache_clear(&kernel_module.m_fde_cache);
#endif
   /* This one we do actually have to clear, because it does the same
    * optimizations as the kernel's FDE cache, but unlike it, all
    * exception information related to the kernel's .free section
    * is no longer required once that section has gone away. */
   except_cache_clear(&kernel_module.m_exc_cache);

   /* Remove the node for the .free section from the kernel VM. */
   asserte(vm_pop_nodes(&vm_kernel,
                       (vm_vpage_t)LOAD_FAR_POINTER(kernel_free_minpage),
                       (vm_vpage_t)LOAD_FAR_POINTER(kernel_free_maxpage),
                        VM_UNMAP_NORMAL|
                        VM_UNMAP_IMMUTABLE,
                        NULL) == &kernel_free_node);
   assert(!kernel_free_node.vn_byaddr.le_next);
   /* Find the KFREE memory information segment. */
   while (iter->mi_type != MEMTYPE_KFREE) {
    if (iter == _mem_info_last)
        goto no_free_segment; /* Shouldn't happen, unless the bootloader placed the kernel's
                               * .free segment in bad or NVS ram (which it really shouldn't have) */
    ++iter;
   }
   assert(iter[0].mi_addr == (uintptr_t)kernel_free_start - KERNEL_CORE_BASE);
   assert(iter[1].mi_addr == (uintptr_t)kernel_free_end - KERNEL_CORE_BASE);
   if (iter[1].mi_type == MEMTYPE_RAM) {
    /* Merge with the next segment. */
    iter[1].mi_addr = iter[0].mi_addr;
    --_mem_info_c,--_mem_info_last;
    memmove(iter,iter+1,
           (size_t)((_mem_info_v+_mem_info_c)-iter) *
                     sizeof(struct meminfo));
   } else {
    /* Simply change into a RAM segment. */
    iter[0].mi_type = MEMTYPE_RAM;
   }
no_free_segment:
   page_free(VM_ADDR2PAGE((uintptr_t)kernel_free_start - KERNEL_CORE_BASE),
            (uintptr_t)kernel_free_size / PAGESIZE);

   debug_printf("[BOOT] Commencing initial switch to user-space\n");

#ifdef __x86_64__
   /* On x86_64, we don't get rid of the identity mapping,
    * meaning that we can directly switch to user-space! */
   /* XXX: I think, fs/gs base registers are silently re-loaded by the CPU
    *      when entering compatibility mode, so us setting the user-space
    *      segment register bases above isn't being preserved in user-space.
    * Right now, everything is configured to start the 32-bit version of
    * /bin/init in compatibility mode. And don't get me wrong: it does start,
    * and works just as intended. However, the problems start when it tries
    * to access it's task segment to read the process environment pointer,
    * at which point it E_SEGFAULT-s because FS.BASE is NULL (contrary to
    * what we had set before)
    * The thing is: I'm not quite sure how to deal with this, as I really
    *               don't want to re-introduce segment indices in CPU
    *               contexts. (There has to be a better way...)
    * NOTE FOR THE FUTURE:
    *   - The reason was that the fs/gs segments weren't marked as
    *     available to user-space (DPL vs. CPL), and that was why
    *     they were getting reset! */
   enable_syscall_tracing();
   cpu_setcontext(&ctx);
#else
   /* With all memory descriptors now updated to view the
    * kernel's .free segment as unused RAM, the only thing
    * left for us to do, is to unmap() the segment in the
    * kernel page directory.
    * XXX: We also must send an IPI to other CPUs... But we'd
    *      need to do that _after_ unmapping the code that
    *      would control that operation... */
   __asm__ __volatile__("pushl %0\n"                     /* cpu_setcontext([ctx]) */
                        "pushl %1\n"                     /* pagedir_map(virt_page,num_pages,phys_page,[perm]); */
                        "pushl $0\n"                     /* pagedir_map(virt_page,num_pages,[phys_page],perm); */
                        "pushl $kernel_free_num_pages\n" /* pagedir_map(virt_page,[num_pages],phys_page,perm); */
                        "pushl $kernel_free_minpage\n"   /* pagedir_map([virt_page],num_pages,phys_page,perm); */
                        "pushl $cpu_setcontext_pop\n"    /* pagedir_map() -> RETURN_ADDRESS; */
                        "jmp   pagedir_map\n"            /* pagedir_map(...) */
                        :
                        : "r" (&ctx)
                        , "i" (PAGEDIR_MAP_FUNMAP)
                        : "memory");
#endif
   __builtin_unreachable();
  }
#endif
 }
}



DEFINE_SYSCALL_MUSTRESTART(xsyslog);
DEFINE_SYSCALL3(xsyslog,int,type,char *,str,size_t,len) {
 validate_readable(str,len);
 if unlikely(len > 0x1000) {
  debug_printf("xsyslog(%p,%p)",str,len);
 } else {
#if 0
  debug_printf("%#$q\n",len,str);
#else
  debug_print(str,len);
#endif
 }
 return 0;
}



DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_INIT_C */
