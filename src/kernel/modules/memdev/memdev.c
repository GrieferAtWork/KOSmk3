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
#ifndef GUARD_KERNEL_MODULES_MEMDEV_MEMDEV_C
#define GUARD_KERNEL_MODULES_MEMDEV_MEMDEV_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <fs/driver.h>
#include <fs/device.h>
#include <kernel/vm.h>
#include <kernel/debug.h>
#include <kos/kdev_t.h>
#include <kos/types.h>
#include <string.h>
#include <assert.h>
#include <except.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/io.h>

DECL_BEGIN

PRIVATE REF struct vm_region *phys_memory = NULL;
DEFINE_DRIVER_FINI(decref_phys_memory);
PRIVATE ATTR_USED void KCALL decref_phys_memory(void) {
 if (phys_memory)
     vm_region_decref(phys_memory);
}

/* For /dev/mem */
PRIVATE REF struct vm_region *KCALL get_phys_memory(void) {
 REF struct vm_region *result,*new_result;
 if (this_driver.d_app.a_flags & APPLICATION_FCLOSING)
     error_throw(E_DRIVER_CLOSED);
 result = phys_memory;
 if (result) {
  vm_region_incref(result);
  return result;
 }
 /* Construct a physical memory region containing _everything_ */
 result = vm_region_alloc(1+(((uintptr_t)-1) / PAGESIZE));
 result->vr_type           = VM_REGION_PHYSICAL;
 result->vr_part0.vp_state = VM_PART_INCORE;
 result->vr_part0.vp_flags = VM_PART_FKEEP|VM_PART_FWEAKREF|VM_PART_FNOSWAP;
 result->vr_part0.vp_phys.py_num_scatter = 1;
 result->vr_part0.vp_phys.py_iscatter[0].ps_addr = 0;
 result->vr_part0.vp_phys.py_iscatter[0].ps_size = 1+(((uintptr_t)-1) / PAGESIZE);
 /* Cache the region. */
 new_result = ATOMIC_CMPXCH_VAL(phys_memory,NULL,result);
 if unlikely(new_result) {
  vm_region_decref(result);
  result = new_result;
 }
 vm_region_incref(result);
 return result;
}




PRIVATE void KCALL
Mem_Stat(struct character_device *__restrict UNUSED(self),
         USER CHECKED struct stat64 *result) {
#if __SIZEOF_POINTER__ >= 8
 result->st_size64 = (pos64_t)(uintptr_t)-1; /* Technically 1 more, but we can't represent that... */
#else
 result->st_size64 = (pos64_t)((uintptr_t)-1)+1;
#endif
 /* What better way to split physical memory, than into pages? */
 result->st_blocks = (blkcnt_t)(1+(((uintptr_t)-1) / PAGESIZE));
 result->st_blksize = PAGESIZE;
}

PRIVATE size_t KCALL
Mem_PRead(struct character_device *__restrict UNUSED(self),
          USER CHECKED void *buf, size_t bufsize,
          pos_t offset, iomode_t UNUSED(flags)) {
 vm_copyfromphys(buf,(vm_phys_t)offset,bufsize);
 return bufsize;
}
PRIVATE size_t KCALL
Mem_PWrite(struct character_device *__restrict UNUSED(self),
           USER CHECKED void const *buf, size_t bufsize,
           pos_t offset, iomode_t UNUSED(flags)) {
 vm_copytophys((vm_phys_t)offset,buf,bufsize);
 return bufsize;
}


PRIVATE REF struct vm_region *KCALL
Mem_Mmap(struct character_device *__restrict UNUSED(self),
         /*vm_raddr_t*/uintptr_t page_index,
         /*vm_raddr_t*/uintptr_t *__restrict pregion_start) {
 *pregion_start = page_index;
 return get_phys_memory();
}
PRIVATE void KCALL
Mem_Sync(struct character_device *__restrict UNUSED(self)) {
#if defined(__i386__) || defined(__x86_64__)
 /* This instruction should invalidate all
  * caches and write everything to memory. */
 __asm__ __volatile__("wbinvd" : : : "memory");
#endif
}



#if defined(__i386__) || defined(__x86_64__)
#define HAVE_DEV_PORT 1
PRIVATE void KCALL
Port_Stat(struct character_device *__restrict UNUSED(self),
          USER CHECKED struct stat64 *result) {
 result->st_size64  = 0x10000;
 result->st_blocks  = 0x10000 / 16;
 result->st_blksize = 16; /* ??? This isn't really split into blocks... */
}

PRIVATE size_t KCALL
Port_PRead(struct character_device *__restrict UNUSED(self),
           USER CHECKED void *buf, size_t bufsize,
           pos_t offset, iomode_t flags) {
 u16 i;
 if (offset >= 0x10000) return 0;
 bufsize = MIN(bufsize,(size_t)(0x10000-offset));
 if (flags & IO_NONBLOCK) {
  /* Use inb() */
  for (i = 0; i < bufsize; ++i)
      ((u8 *)buf)[i] = inb((u16)offset + i);
 } else {
  /* Use inb_p() */
  for (i = 0; i < bufsize; ++i)
      ((u8 *)buf)[i] = inb_p((u16)offset + i);
 }
 return bufsize;
}
PRIVATE size_t KCALL
Port_PWrite(struct character_device *__restrict UNUSED(self),
            USER CHECKED void const *buf, size_t bufsize,
            pos_t offset, iomode_t UNUSED(flags)) {
 u16 i;
 if (offset >= 0x10000) return 0;
 bufsize = MIN(bufsize,(size_t)(0x10000-offset));
 if (flags & IO_NONBLOCK) {
  /* Use outb() */
  for (i = 0; i < bufsize; ++i)
      outb((u16)offset + i,((u8 *)buf)[i]);
 } else {
  /* Use outb_p() */
  for (i = 0; i < bufsize; ++i)
      outb_p((u16)offset + i,((u8 *)buf)[i]);
 }
 return bufsize;
}
#endif




PRIVATE size_t KCALL
Zero_Read(struct character_device *__restrict UNUSED(self),
          USER void *__restrict buf, size_t bufsize,
          iomode_t UNUSED(flags)) {
 memset(buf,0,bufsize);
 return bufsize;
}
PRIVATE size_t KCALL
Zero_PRead(struct character_device *__restrict UNUSED(self),
           void *__restrict buf, size_t bufsize,
           pos_t UNUSED(pos), iomode_t UNUSED(flags)) {
 memset(buf,0,bufsize);
 return bufsize;
}


PRIVATE size_t KCALL
Null_Read(struct character_device *__restrict UNUSED(self),
          USER void *__restrict UNUSED(buf), size_t UNUSED(bufsize),
          iomode_t UNUSED(flags)) {
 return 0;
}
PRIVATE size_t KCALL
Null_PRead(struct character_device *__restrict UNUSED(self),
           USER void *__restrict UNUSED(buf), size_t UNUSED(bufsize),
           pos_t UNUSED(pos), iomode_t UNUSED(flags)) {
 return 0;
}
PRIVATE size_t KCALL
Null_Write(struct character_device *__restrict UNUSED(self),
           USER void const *__restrict UNUSED(buf), size_t bufsize,
           iomode_t UNUSED(flags)) {
 return bufsize;
}
PRIVATE size_t KCALL
Null_PWrite(struct character_device *__restrict UNUSED(self),
            USER void const *__restrict UNUSED(buf), size_t bufsize,
            pos_t UNUSED(pos), iomode_t UNUSED(flags)) {
 return bufsize;
}
PRIVATE size_t KCALL
Full_Write(struct character_device *__restrict UNUSED(self),
           USER void const *__restrict UNUSED(buf), size_t bufsize,
           iomode_t UNUSED(flags)) {
 error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_DISK_FULL);
}
PRIVATE size_t KCALL
Full_PWrite(struct character_device *__restrict UNUSED(self),
            USER void const *__restrict UNUSED(buf), size_t bufsize,
            pos_t UNUSED(pos), iomode_t UNUSED(flags)) {
 error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_DISK_FULL);
}
PRIVATE pos_t KCALL
Null_Seek(struct character_device *__restrict UNUSED(self),
          off_t UNUSED(off), int UNUSED(whence)) {
 return 0;
}


PRIVATE size_t KCALL
URandom_Read(struct character_device *__restrict UNUSED(self),
             USER void *__restrict buf, size_t bufsize,
             iomode_t UNUSED(flags)) {
 size_t result = bufsize;
 while (bufsize >= 4) {
  *(u32 *)buf = rand();
  *(uintptr_t *)&buf += 4;
  bufsize -= 4;
 }
 if (bufsize) {
  u32 rnd = rand();
  memcpy(buf,&rnd,bufsize);
 }
 return result;
}
PRIVATE size_t KCALL
URandom_PRead(struct character_device *__restrict UNUSED(self),
              USER void *__restrict buf, size_t bufsize,
              pos_t UNUSED(pos), iomode_t UNUSED(flags)) {
 size_t result = bufsize;
 while (bufsize >= 4) {
  *(u32 *)buf = rand();
  *(uintptr_t *)&buf += 4;
  bufsize -= 4;
 }
 if (bufsize) {
  u32 rnd = rand();
  memcpy(buf,&rnd,bufsize);
 }
 return result;
}
PRIVATE size_t KCALL
KMsg_Write(struct character_device *__restrict UNUSED(self),
           USER void const *__restrict buf, size_t bufsize,
           iomode_t UNUSED(flags)) {
 debug_printer((char *)buf,bufsize,NULL);
 return bufsize;
}



struct memdev {
    char                        md_name[DEVICE_MAXNAME]; /* Name of the device. */
    dev_t                       md_devno;                /* Device number. */
    struct character_device_ops md_ops;                  /* Character device operators. */
};

PRIVATE struct memdev memdevs[] = {
    { "mem",     MKDEV(1,1),  { /* /dev/mem      Physical memory .access */
        .c_open = &character_device_open_stream,
        .c_file = {
            .f_stat   = &Mem_Stat,
            .f_sync   = &Mem_Sync,
            .f_pread  = &Mem_PRead,
            .f_pwrite = &Mem_PWrite,
            .f_mmap   = &Mem_Mmap,
        },
    } },
    { "kmem",    MKDEV(1,2),  { /* /dev/kmem     Kernel virtual memory access. */
        .c_open = &character_device_open_stream,
    } },
    { "null",    MKDEV(1,3),  { /* /dev/null     Null device. */
        .c_file = {
            .f_read   = &Null_Read,
            .f_write  = &Null_Write,
            .f_pread  = &Null_PRead,
            .f_pwrite = &Null_PWrite,
            .f_seek   = &Null_Seek,
        }
    } },
#ifdef HAVE_DEV_PORT
    { "port",    MKDEV(1,4),  { /* /dev/port     I/O port access. */
        .c_open = &character_device_open_stream,
        .c_file = {
            .f_stat   = &Port_Stat,
            .f_pread  = &Port_PRead,
            .f_pwrite = &Port_PWrite,
        },
    } },
#endif
    { "zero",    MKDEV(1,5),  { /* /dev/zero     Null byte source. */
        .c_file = {
            .f_read   = &Zero_Read,
            .f_write  = &Null_Write,
            .f_pread  = &Zero_PRead,
            .f_pwrite = &Null_PWrite,
            .f_seek   = &Null_Seek,
        }
    } },
    { "core",    MKDEV(1,6),  { /* /dev/core     OBSOLETE - replaced by /proc/kcore. */
        .c_open = &character_device_open_stream,
    } },
    { "full",    MKDEV(1,7),  { /* /dev/full     Returns ENOSPC on write. */
        .c_file = {
            .f_read   = &Zero_Read,
            .f_write  = &Full_Write,
            .f_pread  = &Zero_PRead,
            .f_pwrite = &Full_PWrite,
            .f_seek   = &Null_Seek,
        }
    } },
    { "random",  MKDEV(1,8),  { /* /dev/random   Nondeterministic random number gen. */
        .c_file = {
            /* XXX: Use hardware RNG */
            .f_read   = &URandom_Read,
            .f_pread  = &URandom_PRead,
        }
    } },
    { "urandom", MKDEV(1,9),  { /* /dev/urandom  Faster, less secure random number gen. */
        .c_file = {
            .f_read   = &URandom_Read,
            .f_pread  = &URandom_PRead,
        }
    } },
    { "aio",     MKDEV(1,10), { /* /dev/aio      Asynchronous I/O notification interface. */
    } },
    { "kmsg",    MKDEV(1,11), { /* /dev/kmsg     Writes to this come out as `syslog()'s. */
        .c_file = {
            .f_write  = &KMsg_Write,
        }
    } }
};

DEFINE_DRIVER_INIT(memdevs_init);
PRIVATE ATTR_USED void KCALL memdevs_init(void) {
 unsigned int i;
 for (i = 0; i < COMPILER_LENOF(memdevs); ++i) {
  struct character_device *EXCEPT_VAR cdev;
  /* Allocate a new character device. */
  cdev = CHARACTER_DEVICE_ALLOC(struct character_device);
  TRY {
   /* Fill in members of the character device. */
   memcpy(cdev->c_device.d_name,
          memdevs[i].md_name,
          sizeof(memdevs[i].md_name));
   cdev->c_device.d_devno = memdevs[i].md_devno;
   cdev->c_ops = &memdevs[i].md_ops;
   /* Register the character device. */
   register_device(&cdev->c_device);
  } FINALLY {
   character_device_decref(cdev);
  }
 }
}



DECL_END

#endif /* !GUARD_KERNEL_MODULES_MEMDEV_MEMDEV_C */
