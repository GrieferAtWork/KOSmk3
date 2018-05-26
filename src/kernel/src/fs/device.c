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
#ifndef GUARD_KERNEL_SRC_FS_DEVICE_C
#define GUARD_KERNEL_SRC_FS_DEVICE_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/vm.h>
#include <kernel/heap.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <kernel/user.h>
#include <kos/kdev_t.h>
#include <fs/driver.h>
#include <fs/device.h>
#include <fs/node.h>
#include <fs/ramfs.h>
#include <sys/mount.h>
#include <string.h>
#include <except.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

DECL_BEGIN

PRIVATE void KCALL
NullDevice_Read(struct block_device *__restrict self,
                CHECKED USER void *buf, size_t num_blocks,
                blkaddr_t UNUSED(first_block)) {
 memset(buf,0,num_blocks*self->b_blocksize);
}
PRIVATE void KCALL
NullDevice_Write(struct block_device *__restrict UNUSED(self),
                 CHECKED USER void const *UNUSED(buf),
                 size_t UNUSED(num_blocks),
                 blkaddr_t UNUSED(first_block)) {
 /* nothing */
}


PUBLIC struct block_device null_device = {
    .b_device = {
        .d_refcnt  = 1,
        .d_driver  = &this_driver,
        .d_type    = DEVICE_TYPE_FBLOCKDEV,
        .d_flags   = DEVICE_FREADONLY,
        .d_name    = null_device.b_device.d_namebuf,
        .d_namebuf = "null_device"
    },
    .b_blocksize  = 512,
    .b_blockcount = (blkcnt_t)-1,
    .b_partstart  = 0,
    .b_fslock     = ATOMIC_RWLOCK_INIT,
    .b_filesystem = NULL,
    .b_master     = &null_device,
    .b_pagebuf = {
        .ps_lock     = RWLOCK_INIT,
        .ps_mapu     = 0,
        .ps_mapc     = 0,
        .ps_mapm     = 0,
        .ps_mapm_max = CONFIG_BLOCK_PAGES_MAXMASK,
        .ps_mapv     = NULL
    },
    .b_partlock   = ATOMIC_RWLOCK_INIT,
    .b_partitions = NULL,
    .b_partmaxcnt = 0,
    .b_io         = {
        .io_read  = &NullDevice_Read,
        .io_write = &NullDevice_Write
    }
};

#define throw_fs_error(fs_error_code) \
        __EXCEPT_INVOKE_THROW_NORETURN(throw_fs_error(fs_error_code))
PRIVATE __EXCEPT_NORETURN void
(KCALL throw_fs_error)(u16 fs_error_code) {
 struct exception_info *info;
 info = error_info();
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_code                        = E_FILESYSTEM_ERROR;
 info->e_error.e_flag                        = ERR_FNORMAL;
 info->e_error.e_filesystem_error.fs_errcode = fs_error_code;
 error_throw_current();
 __builtin_unreachable();
}


PUBLIC void KCALL
device_vsetnamef(struct device *__restrict dev,
                 char const *__restrict format,
                 va_list args) {
 size_t reqlen;
 reqlen = vsnprintf(dev->d_namebuf,
                    DEVICE_MAXNAME,
                    format,args);
 if (reqlen < DEVICE_MAXNAME)
  dev->d_name = dev->d_namebuf;
 else {
  /* Dynamically allocate the name buffer. */
  dev->d_name = (char *)kmalloc((reqlen+1)*sizeof(char),GFP_SHARED);
  vsprintf(dev->d_name,format,args);
 }
}

PUBLIC void ATTR_CDECL
device_setnamef(struct device *__restrict dev,
                char const *__restrict format, ...) {
 va_list args;
 va_start(args,format);
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__)
 device_vsetnamef(dev,format,args);
#else
 TRY {
  device_vsetnamef(dev,format,args);
 } FINALLY {
  va_end(args);
 }
#endif
}



/* Destroy a previously allocated device. */
PUBLIC ATTR_NOTHROW void KCALL
device_destroy(struct device *__restrict self) {
 minor_t num_devids = 1;
 assertf(!self->d_chain.le_pself,
         "self->d_chain.le_pself = %p",
          self->d_chain.le_pself);
 /* Cleanup block-device buffers. */
 if (self->d_type == DEVICE_TYPE_FBLOCKDEV) {
  struct block_device *me;
  size_t i;
  me = (struct block_device *)self;
  assertf(!me->b_filesystem,
          "The filesystem should have kept us alive through `->s_device'");
  if (me->b_master != me) {
   struct block_device *master = me->b_master;
   if (master) {
    /* Delete a partition. */
    atomic_rwlock_write(&master->b_partlock);
    /* Remove this partition from the master drive's chain. */
    if (me->b_partition.p_partitions.le_pself)
        LIST_REMOVE(me,b_partition.p_partitions);
    atomic_rwlock_endwrite(&master->b_partlock);
    /* Drop the reference that was stored in `b_master' */
    block_device_decref(master);
   }
  } else {
   assert(me->b_pagebuf.ps_mapc <= me->b_pagebuf.ps_mapm);
   assert(me->b_pagebuf.ps_mapu <= me->b_pagebuf.ps_mapc);
   if (me->b_io.io_fini)
      (*me->b_io.io_fini)(me);

   num_devids = me->b_partmaxcnt+1;
   if (me->b_pagebuf.ps_mapu) {
    assert(me->b_pagebuf.ps_mapv);
    for (i = 0; i <= me->b_pagebuf.ps_mapm; ++i) {
     struct block_page *page = &me->b_pagebuf.ps_mapv[i];
     /* Free all remaining page buffers.
      * NOTE: The caller was responsible to save any unwritten data.
      *       Now it's too late for that. */
     if (BLOCK_PAGE_ISDUMMYORNULL(page->bp_data))
         continue;
     if (page->bp_flags & BLOCK_PAGE_FCHANGED) {
      debug_printf("[BLOCK] Discarding unwritten page %I64u (%I64x) of block device %I64x\n",
                   page->bp_addr,page->bp_addr,self->d_devno);
     }
     heap_free_untraced(&kernel_heaps[GFP_SHARED|GFP_LOCKED],
                page->bp_data,page->bp_size,
                GFP_SHARED|GFP_LOCKED);
    }
   }
   kfree(me->b_pagebuf.ps_mapv);
  }
 } else {
  struct character_device *me;
  me = (struct character_device *)self;
  if (me->c_ops && me->c_ops->c_fini)
     (*me->c_ops->c_fini)(me);
 }
 if (self->d_driver)
     driver_decref(self->d_driver);
 /* Free dynamically allocated device IDs. */
 if (self->d_flags & DEVICE_FDYNDEVICE)
     devno_free(self->d_type,self->d_devno,num_devids);
 if (self->d_name != self->d_namebuf)
     kfree(self->d_name);
 kfree(self);
}

PUBLIC ssize_t KCALL
device_ioctl(struct device *__restrict self,
             unsigned long cmd, USER UNCHECKED void *arg,
             iomode_t flags) {
 /* XXX: Standard device I/O-control commands? */
 if (self->d_type == DEVICE_TYPE_FBLOCKDEV) {
  struct block_device *me;
  me = (struct block_device *)self;
  switch (cmd) {

  {
   int new_roflag;
  case BLKROSET:
   validate_readable(arg,sizeof(int));
   new_roflag = *(int *)arg;
   COMPILER_READ_BARRIER();
   if (new_roflag) {
    ATOMIC_FETCHOR(self->d_flags,DEVICE_FREADONLY);
   } else {
    ATOMIC_FETCHAND(self->d_flags,~DEVICE_FREADONLY);
   }
  } break;

  case BLKROGET:
   validate_writable(arg,sizeof(int));
   *(int *)arg = ATOMIC_READ(self->d_flags) & DEVICE_FREADONLY ? 1 : 0;
   break;

  case BLKRRPART:
   /* TODO: Partitioning */
   break;

  case BLKGETSIZE:
   if (me->b_blockcount > ULONG_MAX)
       error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_FILE_TOO_LARGE);
   validate_writable(arg,sizeof(unsigned long));
   *(unsigned long *)arg = (unsigned long)me->b_blockcount;
   break;

  case BLKFLSBUF:
   block_device_sync(me);
   break;

  case BLKRASET:
  case BLKFRASET:
   /* TODO: Implement once DMA support is a thing */
   break;
  case BLKRAGET:
  case BLKFRAGET:
   /* TODO: Implement once DMA support is a thing */
   break;

  case BLKSSZGET:
  case BLKBSZGET:
   validate_writable(arg,sizeof(unsigned int));
   *(unsigned int *)arg = (unsigned int)me->b_blocksize;
   break;

  case BLKGETSIZE64:
   validate_writable(arg,sizeof(u64));
   *(u64 *)arg = me->b_blockcount * me->b_blocksize;
   break;

  default:
   if (me->b_io.io_ioctl)
       return (*me->b_io.io_ioctl)(me,cmd,arg,flags);
   goto unsupported;
  }
  return 0;
 } else {
  struct character_device *me;
  me = (struct character_device *)self;
  if (me->c_ops->c_file.f_ioctl)
      return (*me->c_ops->c_file.f_ioctl)(me,cmd,arg,flags);
 }
unsupported:
 error_throw(E_NOT_IMPLEMENTED);
}

/* @throw: E_NOT_IMPLEMENTED: Stat information cannot be gathered about the given device. */
PUBLIC void KCALL
device_stat(struct device *__restrict self,
            USER CHECKED struct stat64 *result) {
 /* Pre-initialize default fields. */
 memset(result,0,sizeof(struct stat64));
 result->st_dev  = self->d_devno;
 result->st_rdev = self->d_devno;
 if (self->d_type == DEVICE_TYPE_FBLOCKDEV) {
  struct block_device *me;
  me = (struct block_device *)self;
  result->st_blksize  = me->b_blocksize;
  result->st_blocks64 = me->b_blockcount;
  result->st_size64   = (pos_t)me->b_blocksize * (pos_t)me->b_blockcount;
 } else {
  struct character_device *me;
  me = (struct character_device *)self;
  if (!me->c_ops->c_file.f_stat)
      error_throw(E_NOT_IMPLEMENTED);
  (*me->c_ops->c_file.f_stat)(me,result);
 }
}

/* Synchronizes the page buffer of a block-device, or try to invoke the `c_sync'
 * operator of character devices, but ignore it if that operator is missing. */
FUNDEF void KCALL
device_sync(struct device *__restrict self) {
 if (self->d_type == DEVICE_TYPE_FBLOCKDEV)
  block_device_sync((struct block_device *)self);
 else {
  struct character_device *me;
  me = (struct character_device *)self;
  if (me->c_ops->c_file.f_sync)
     (*me->c_ops->c_file.f_sync)(me);
 }
}

/* Tries to invoke the `c_open' operator of character devices,
 * or simply re-returns a handle to the given device as-is. */
FUNDEF REF struct handle KCALL
device_open(struct device *__restrict self,
            struct path *__restrict p,
            oflag_t open_mode) {
 REF struct handle result;
 if (self->d_type == DEVICE_TYPE_FCHARDEV) {
  struct character_device *me;
  me = (struct character_device *)self;
  if (me->c_ops->c_open)
      return (*me->c_ops->c_open)(me,p,open_mode);
 } else {
  return block_device_open_stream((struct block_device *)self,p,open_mode);
 }
 /* Fallback: return a handle for the device itself. */
 result.h_mode = HANDLE_MODE(HANDLE_TYPE_FDEVICE,
                             IO_FROM_O(open_mode));
 result.h_object.o_device = self;
 device_incref(self);
 return result;
}


/* @throw: E_NOT_IMPLEMENTED: Current-position reading isn't supported (e.g.: block devices). */
FUNDEF size_t KCALL
device_read(struct device *__restrict self,
            USER CHECKED void *buf,
            size_t bufsize, iomode_t flags) {
 if (self->d_type == DEVICE_TYPE_FCHARDEV &&
   ((struct character_device *)self)->c_ops->c_file.f_read)
     return (*((struct character_device *)self)->c_ops->c_file.f_read)
              ((struct character_device *)self,buf,bufsize,flags);
 error_throw(E_NOT_IMPLEMENTED);
}

/* @throw: E_NOT_IMPLEMENTED: Current-position writing isn't supported (e.g.: block devices). */
FUNDEF size_t KCALL
device_write(struct device *__restrict self,
             USER CHECKED void const *buf,
             size_t bufsize, iomode_t flags) {
 if (self->d_type == DEVICE_TYPE_FCHARDEV &&
   ((struct character_device *)self)->c_ops->c_file.f_write)
     return (*((struct character_device *)self)->c_ops->c_file.f_write)
              ((struct character_device *)self,buf,bufsize,flags);
 error_throw(E_NOT_IMPLEMENTED);
}

/* @return: 0: No signals are available, or polling isn't supported. */
FUNDEF unsigned int KCALL
device_poll(struct device *__restrict self, unsigned int mode) {
 /* Poll the lock for the page-buffer. */
 if (self->d_type == DEVICE_TYPE_FBLOCKDEV)
     return rwlock_poll(&((struct block_device *)self)->b_pagebuf.ps_lock,mode);
 if (((struct character_device *)self)->c_ops->c_file.f_poll)
       return (*((struct character_device *)self)->c_ops->c_file.f_poll)
                ((struct character_device *)self,mode);
 return 0;
}

/* @throw: E_NOT_IMPLEMENTED: Seeking is not supported. */
FUNDEF pos_t KCALL
device_seek(struct device *__restrict self, off_t offset, int whence) {
 if (((struct character_device *)self)->c_ops->c_file.f_seek)
       return (*((struct character_device *)self)->c_ops->c_file.f_seek)
                ((struct character_device *)self,offset,whence);
 error_throw(E_NOT_IMPLEMENTED);
}

/* @throw: E_NOT_IMPLEMENTED: Absolute-position reading isn't supported. */
FUNDEF size_t KCALL
device_pread(struct device *__restrict self,
             USER CHECKED void *buf, size_t bufsize,
             pos_t offset, iomode_t flags) {
 if (self->d_type == DEVICE_TYPE_FBLOCKDEV)
     return block_device_read((struct block_device *)self,
                               buf,bufsize,offset,flags);
 if (((struct character_device *)self)->c_ops->c_file.f_pread)
       return (*((struct character_device *)self)->c_ops->c_file.f_pread)
                ((struct character_device *)self,buf,bufsize,offset,flags);
 error_throw(E_NOT_IMPLEMENTED);
}

/* @throw: E_NOT_IMPLEMENTED: Absolute-position writing isn't supported. */
FUNDEF size_t KCALL
device_pwrite(struct device *__restrict self,
              USER CHECKED void const *buf,
              size_t bufsize, pos_t offset,
              iomode_t flags) {
 if (self->d_type == DEVICE_TYPE_FBLOCKDEV)
     return block_device_write((struct block_device *)self,
                                buf,bufsize,offset,flags);
 if (((struct character_device *)self)->c_ops->c_file.f_pwrite)
       return (*((struct character_device *)self)->c_ops->c_file.f_pwrite)
                ((struct character_device *)self,buf,bufsize,offset,flags);
 error_throw(E_NOT_IMPLEMENTED);
}



struct device_map {
    atomic_rwlock_t               dm_lock; /* Lock for this device map. */
    size_t                        dm_mapc; /* [lock(dm_lock)] Amount of tracked devices. */
    size_t                        dm_mapm; /* [lock(dm_lock)] Device map mask. */
    REF LIST_HEAD(struct device) *dm_mapv; /* [0..1][0..dm_mapm+1][lock(dm_lock)][owned] Map of devices. */
};

PRIVATE struct device_map device_maps[DEVICE_TYPE_COUNT];


#define DEVNO_HASH(x) (MINOR(x) ^ MAJOR(x))

STATIC_ASSERT(ERROR_NO_DEVICE_FBLOCKDEV == DEVICE_TYPE_FBLOCKDEV);
STATIC_ASSERT(ERROR_NO_DEVICE_FCHARDEV  == DEVICE_TYPE_FCHARDEV);

PUBLIC ATTR_RETNONNULL REF struct device *
KCALL lookup_device(u16 type, dev_t devno) {
 struct exception_info *info;
 REF struct device *result;
 result = try_lookup_device(type,devno);
 if likely(result) return result;
 info = error_info();
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_code              = E_NO_DEVICE;
 info->e_error.e_flag              = ERR_FNORMAL;
 info->e_error.e_no_device.d_devno = devno;
 info->e_error.e_no_device.d_type  = type;
 error_throw_current();
 __builtin_unreachable();
}

/* Lookup the device associated with a given type and device-number.
 * If the device doesn't exist, return `NULL'. */
PUBLIC ATTR_NOTHROW REF struct device *KCALL
try_lookup_device(u16 type, dev_t devno) {
 REF struct device *result;
 struct device_map *map = &device_maps[type];
 assert(type < DEVICE_TYPE_COUNT);
 atomic_rwlock_read(&map->dm_lock);
 if unlikely(!map->dm_mapv)
  result = NULL;
 else {
  /* Search the hash-map. */
  result = map->dm_mapv[DEVNO_HASH(devno) & map->dm_mapm];
  while (result && result->d_devno != devno)
         result = result->d_chain.le_next;
  if (result) {
   assert(result->d_type == type);
   device_incref(result);
  }
 }
 atomic_rwlock_endread(&map->dm_lock);
 return result;
}

/* Register / delete a given device. */
PUBLIC ATTR_NOTHROW void KCALL
unregister_device_nodevfs(struct device *__restrict dev) {
 struct device_map *map = &device_maps[dev->d_type];
 assert(dev->d_type < DEVICE_TYPE_COUNT);
 atomic_rwlock_write(&map->dm_lock);
 if (!dev->d_chain.le_pself) {
  atomic_rwlock_endwrite(&map->dm_lock);
  return; /* Device wasn't registered. */
 }
 assert(map->dm_mapc);
 LIST_REMOVE(dev,d_chain);
 dev->d_chain.le_pself = NULL;
 --map->dm_mapc;
 atomic_rwlock_endwrite(&map->dm_lock);
 /* Drop the reference saved in the map. */
 device_decref(dev);
}

PUBLIC bool KCALL
register_device_nodevfs(struct device *__restrict dev) {
 struct device_map *EXCEPT_VAR map = &device_maps[dev->d_type];
 struct device **pbucket,*bucket;
 assert(dev->d_type < DEVICE_TYPE_COUNT);
again:
 atomic_rwlock_write(&map->dm_lock);
 assert(dev->d_chain.le_pself == NULL);
 if (map->dm_mapc >= (map->dm_mapm/3)*2) {
  size_t new_mask = (map->dm_mapm << 1) | 1;
  LIST_HEAD(struct device) *COMPILER_IGNORE_UNINITIALIZED(new_map);
  LIST_HEAD(struct device) *old_map;
  atomic_rwlock_endwrite(&map->dm_lock);
  if (new_mask < 15) new_mask = 15;
  TRY {
   new_map = (LIST_HEAD(struct device) *)kmalloc((new_mask+1)*sizeof(struct device *),
                                                  GFP_SHARED|GFP_CALLOC);
  } CATCH (E_BADALLOC) {
   atomic_rwlock_write(&map->dm_lock);
   if (map->dm_mapv != NULL) { error_handled(); goto use_map; }
   atomic_rwlock_endwrite(&map->dm_lock);
   error_rethrow();
  }
  atomic_rwlock_write(&map->dm_lock);
  if unlikely(new_mask <= map->dm_mapm && map->dm_mapv) {
   /* Keep the old map if its actually grown larger. */
   old_map = new_map;
  } else {
   struct device *iter,*next; size_t i;
   /* Rehash the old map to insert it into the new one. */
   old_map = map->dm_mapv;
   if (old_map != NULL) {
    for (i = 0; i <= map->dm_mapm; ++i) {
     iter = old_map[i];
     while (iter) {
      next = iter->d_chain.le_next;
      assert(iter->d_type == dev->d_type);
      /* Move the device into the new map. */
      pbucket = &new_map[DEVNO_HASH(iter->d_devno) & new_mask];
      LIST_INSERT(*pbucket,iter,d_chain);
      iter = next;
     }
    }
   }
   /* Install the new map. */
   map->dm_mapv = new_map;
   map->dm_mapm = new_mask;
  }
  atomic_rwlock_endwrite(&map->dm_lock);
  kfree(old_map);
  goto again;
 }
use_map:
 assert(map->dm_mapv);
 pbucket = &map->dm_mapv[DEVNO_HASH(dev->d_devno) & map->dm_mapm];
 for (bucket = *pbucket; bucket; bucket = bucket->d_chain.le_next) {
  /* Check for duplicate device numbers. */
  if unlikely(bucket->d_devno == dev->d_devno) {
   atomic_rwlock_endwrite(&map->dm_lock);
   return false;
  }
 }

 LIST_INSERT(*pbucket,dev,d_chain);
 ++map->dm_mapc;
 /* Create the reference now saved in the map. */
 device_incref(dev);
 atomic_rwlock_endwrite(&map->dm_lock);
 return true;
}

PUBLIC bool KCALL
register_device(struct device *__restrict dev) {
 struct device *EXCEPT_VAR xdev = dev;
 bool result;
 result = register_device_nodevfs(dev);
 if (result) {
  TRY {
   /* Add the device to /dev on success. */
   device_add_to_devfs(dev);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   unregister_device_nodevfs(xdev);
   error_rethrow();
  }
 }
 return result;
}

PUBLIC void KCALL
unregister_device(struct device *__restrict dev) {
 device_remove_from_devfs(dev);
 unregister_device_nodevfs(dev);
}




PUBLIC WUNUSED dev_t KCALL
devno_alloc_mask(u16 type,
                 dev_t start,
                 minor_t mask,
                 minor_t num_numbers) {
 assert(type < DEVICE_TYPE_COUNT);
 assert((MINOR(start) & mask) == 0);
 /* TODO */
 return start + (rand() & mask);
}
PUBLIC WUNUSED bool KCALL
devno_allocat(u16 type, dev_t start, minor_t num_numbers) {
 assert(type < DEVICE_TYPE_COUNT);
 /* TODO */
 return true;
}
PUBLIC ATTR_NOTHROW void KCALL
devno_free(u16 type,
           dev_t start,
           minor_t num_numbers) {
 assert(type < DEVICE_TYPE_COUNT);
 /* TODO */
}


PUBLIC void KCALL
register_dynamic_device_mask(struct device *__restrict dev,
                             dev_t start, minor_t mask) {
 struct device *EXCEPT_VAR xdev = dev;
 minor_t EXCEPT_VAR num_numbers = 1;
 if (dev->d_type == DEVICE_TYPE_FBLOCKDEV)
     num_numbers = ((struct block_device *)dev)->b_partmaxcnt;
again:
 dev->d_devno = devno_alloc_mask(dev->d_type,start,mask,num_numbers);
 TRY {
  if unlikely(!register_device(dev)) {
   /* Already in use??? */
   if (num_numbers > 1)
       devno_free(dev->d_type,dev->d_devno+1,num_numbers-1);
   goto again;
  }
  dev->d_flags |= DEVICE_FDYNDEVICE;
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  devno_free(xdev->d_type,
             xdev->d_devno,
             num_numbers);
  error_rethrow();
 }
}


PUBLIC void KCALL
register_dynamic_device(struct device *__restrict dev) {
 register_dynamic_device_mask(dev,
                              MKDEV(DEVNO_DYNAMIC_BEGIN_MAJOR,0),
                              MINORMASK);
}
PUBLIC WUNUSED dev_t KCALL
devno_alloc(u16 type, minor_t num_numbers) {
 return devno_alloc_mask(type,
                         MKDEV(DEVNO_DYNAMIC_BEGIN_MAJOR,0),
                         MINORMASK,
                         num_numbers);
}






PUBLIC ATTR_RETNONNULL REF struct device *
(KCALL device_alloc)(u16 device_type, size_t struct_size,
                     struct driver *__restrict caller) {
 REF struct device *result;
 assert(struct_size >= sizeof(struct device));
 assert(device_type < DEVICE_TYPE_COUNT);
 result = (REF struct device *)kmalloc(struct_size,GFP_SHARED|GFP_CALLOC);
 if (!driver_tryincref(caller)) goto procexit;
 if (caller->d_app.a_flags & APPLICATION_FCLOSING) {
  driver_decref(caller);
procexit:
  kfree(result);
  error_throw(E_DRIVER_CLOSED);
 }
 result->d_refcnt = 1;
 result->d_driver = caller; /* Inherit reference. */
 result->d_type   = device_type;
 result->d_name   = result->d_namebuf;
 return result;
}


PUBLIC ATTR_RETNONNULL REF struct block_device *
(KCALL block_device_alloc)(size_t struct_size, minor_t part_maxcount,
                           struct driver *__restrict caller) {
 REF struct block_device *result;
 result = (REF struct block_device *)(device_alloc)(DEVICE_TYPE_FBLOCKDEV,
                                                    struct_size,caller);
 result->b_master     = result;
 result->b_partmaxcnt = part_maxcount;
 atomic_rwlock_cinit(&result->b_fslock);
 atomic_rwlock_cinit(&result->b_partlock);
 rwlock_cinit(&result->b_pagebuf.ps_lock);
 result->b_pagebuf.ps_mapm_max = CONFIG_BLOCK_PAGES_MAXMASK;
 return result;
}



PUBLIC REF struct superblock *KCALL
block_device_getsuper(struct block_device *__restrict self) {
 REF struct superblock *EXCEPT_VAR result;
again:
 atomic_rwlock_read(&self->b_fslock);
 result = self->b_filesystem;
 if (result && !ATOMIC_INCIFNONZERO(result->s_refcnt))
     result = NULL;
 atomic_rwlock_endread(&self->b_fslock);
 TRY {
  /* Wait until the superblock has been loaded. */
  while unlikely(!SUPERBLOCK_LOADED(result)) {
   /* If the other thread trying to load the superblock
    * failed to do so, loop back and try again ourself. */
   if (ATOMIC_READ(self->b_filesystem) != result) {
    superblock_decref(result);
    goto again;
   }
   task_yield();
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  superblock_decref(result);
  error_rethrow();
 }
 return result;
}


#define BLKADDR_HASH(x)          ((u16)((x) >> 32) ^ (u16)((x) >> 16))
#define BLKADDR_NEXT(i,perturb)  (((i) << 2) + (i) + (perturb) + 1)
#define BLKADDR_SHFT(perturb)    ((perturb) >>= 5)

#if 0
PRIVATE struct block_page *KCALL
find_page(struct block_device *__restrict self,
          blkaddr_t addr) {
 struct block_page *entry; u16 counter;
 size_t i,perturb,hash = BLKADDR_HASH(addr);
 assert(self->b_pagebuf.ps_mapc <= self->b_pagebuf.ps_mapm);
 assert(self->b_pagebuf.ps_mapu <= self->b_pagebuf.ps_mapc);
 if unlikely(!self->b_pagebuf.ps_mapu)
    return NULL;
 perturb = i = hash & self->b_pagebuf.ps_mapm;
 for (;; i = BLKADDR_NEXT(i,perturb),BLKADDR_SHFT(perturb)) {
  entry = &self->b_pagebuf.ps_mapv[i & self->b_pagebuf.ps_mapm];
  if (!entry->bp_data) break;
  if (entry->bp_addr != addr) continue;
  if unlikely(entry->bp_data == BLOCK_PAGE_DUMMY) continue;
  /* Increment the usage counter. */
  do {
   counter = ATOMIC_READ(entry->bp_usage);
   if unlikely(counter == (u16)-1) break;
  } while (!ATOMIC_CMPXCH_WEAK(entry->bp_usage,counter,counter+1));
  return entry;
 }
 return NULL;
}
#endif

#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

/* Lookup or create a page for the given `addr'.
 * When the page doesn't exist, determine a suitable
 * new slot for one and allocate the associated buffer.
 * If a new slot is loaded, its data is initialized using
 * one of 2 methods.
 * Either `page_data' is non-NULL and data is copied from the given pointer.
 * Or `page_data' is NULL and data is read from disk.
 * Additionally, if the page already exists and `page_data'
 * is non-NULL, it's buffer is overwritten when `page_data'. */
PRIVATE ATTR_RETNONNULL struct block_page *KCALL
new_page(struct block_device *__restrict self, blkaddr_t addr,
         CHECKED USER void const *page_data, iomode_t flags) {
 struct block_device *EXCEPT_VAR xself = self;
 struct block_page *EXCEPT_VAR result;
 struct block_page *entry; size_t new_mask;
 size_t i,perturb,hash = BLKADDR_HASH(addr);
 assert(self->b_pagebuf.ps_mapc <= self->b_pagebuf.ps_mapm);
 assert(self->b_pagebuf.ps_mapu <= self->b_pagebuf.ps_mapc);
 assert(!(self->b_device.d_flags & DEVICE_BLOCK_FLINEAR));
 if unlikely(addr >= self->b_blockcount)
    error_throw(E_NO_DATA);
 if unlikely(!self->b_pagebuf.ps_mapu) {
  rwlock_writef(&self->b_pagebuf.ps_lock,flags);
  if (!self->b_pagebuf.ps_mapv) {
   /* Allocate the initial hash-mask. */
   new_mask = 7;
   goto do_rehash;
  }
  assert(self->b_pagebuf.ps_mapm);
  assert(self->b_pagebuf.ps_mapv);
  if unlikely(self->b_pagebuf.ps_mapc) {
   /* If there are dummy entries, delete them. */
   memset(self->b_pagebuf.ps_mapv,0,
         (self->b_pagebuf.ps_mapm+1)*
          sizeof(struct block_page));
   self->b_pagebuf.ps_mapc = 0;
  }
  result = &self->b_pagebuf.ps_mapv[BLKADDR_HASH(addr) & self->b_pagebuf.ps_mapm];
  assert(!result->bp_data);
  ++self->b_pagebuf.ps_mapc;
  ++self->b_pagebuf.ps_mapu;
  goto load_result;
 }
find_dummy:
 /* Find a dummy slot. */
 result = NULL;
 perturb = i = hash & self->b_pagebuf.ps_mapm;
 for (;; i = BLKADDR_NEXT(i,perturb),BLKADDR_SHFT(perturb)) {
  u16 counter;
  entry = &self->b_pagebuf.ps_mapv[i & self->b_pagebuf.ps_mapm];
  if (BLOCK_PAGE_ISDUMMYORNULL(entry->bp_data)) {
   /* Track the first dummy page. */
   if (!result) result = entry;
   if (entry->bp_data) continue;
   break;
  }
  if (entry->bp_addr != addr)
      continue;
  /* Increment the usage counter. */
  do {
   counter = ATOMIC_READ(entry->bp_usage);
   if unlikely(counter == (u16)-1) break;
  } while (!ATOMIC_CMPXCH_WEAK(entry->bp_usage,counter,counter+1));
  if (page_data)
      memcpy(entry->bp_data,page_data,self->b_blocksize);
  return entry;
 }
 assert(result);
 rwlock_writef(&self->b_pagebuf.ps_lock,flags);
 TRY {
  /* If our dummy slot was the end of a hash-chain, check
   * if we should rehash the page buffer, or potentially
   * get rid of older pages. */
  if (result->bp_data != BLOCK_PAGE_DUMMY) {
   if unlikely(self->b_pagebuf.ps_mapc >= (self->b_pagebuf.ps_mapm/2)) {
    new_mask = (self->b_pagebuf.ps_mapm << 1)|1;
    if (new_mask > self->b_pagebuf.ps_mapm_max)
        new_mask = self->b_pagebuf.ps_mapm_max;
    if (new_mask <= self->b_pagebuf.ps_mapm) {
     /* Buffer is full. Collect least used pages and delete them. */
     u16 min_use = (u16)-1,max_use = 0,avg_use;
     struct block_page *map = self->b_pagebuf.ps_mapv;
     /* If at least half of all block buffers are dummy entries,
      * don't delete barely used entries but rehash the buffer instead. */
     if (self->b_pagebuf.ps_mapu < self->b_pagebuf.ps_mapc/2) {
      new_mask = self->b_pagebuf.ps_mapm;
      goto do_rehash;
     }
     /* Figure out the least and most used buffers. */
     for (i = 0; i <= self->b_pagebuf.ps_mapm; ++i) {
      entry = &map[i];
      if (BLOCK_PAGE_ISDUMMYORNULL(entry->bp_data)) continue;
      if (min_use > entry->bp_usage)
          min_use = entry->bp_usage;
      if (max_use < entry->bp_usage)
          max_use = entry->bp_usage;
     }
     assertf(min_use <= max_use,
             "That would mean that no slots are being "
             "used, yet we've checked that case at the "
             "very beginning");
     /* Calculate the usage-threshold that determines with pages go away. */
     avg_use = min_use + CEILDIV(max_use - min_use,2);
     /* Delete all block pages with a usage counter `<= avg_use' */
     for (i = 0; i <= self->b_pagebuf.ps_mapm; ++i) {
      entry = &map[i];
      if (BLOCK_PAGE_ISDUMMYORNULL(entry->bp_data)) continue;
      if (entry->bp_usage > avg_use) {
       /* This page stays (Subtract a portion of the average use). */
       entry->bp_usage -= CEILDIV(avg_use,2);
      } else {
       /* This page goes away (If it was changed, write it to disk). */
       if (entry->bp_flags & BLOCK_PAGE_FCHANGED) {
        assert(entry->bp_addr < self->b_blockcount);
        if (flags & IO_NONBLOCK)
            error_throw(E_WOULDBLOCK);
        (*self->b_io.io_write)(self,entry->bp_data,1,entry->bp_addr);
       }
       /* Free the page buffer. */
       heap_free_untraced(&kernel_heaps[GFP_SHARED|GFP_LOCKED],
                  entry->bp_data,entry->bp_size,
                  GFP_SHARED|GFP_LOCKED);
       /* Turn the page into a dummy entry. */
       entry->bp_data = BLOCK_PAGE_DUMMY;
       assert(self->b_pagebuf.ps_mapu);
       --self->b_pagebuf.ps_mapu;
      }
     }
     rwlock_endwrite(&self->b_pagebuf.ps_lock);
     goto find_dummy; /* SKIP_FINALLY IS INTENTIONAL! */
    } else {
     struct block_page *new_map,*old_map;
do_rehash:
     /* Increase the mask size by allocating a new buffer. */
     TRY {
      new_map = (struct block_page *)kmalloc((new_mask+1)*sizeof(struct block_page),
                                              GFP_SHARED|GFP_LOCKED|
                                              GFP_NOSWAP|GFP_NOFS|GFP_CALLOC);
     } CATCH (E_BADALLOC) {
      /* If the buffer isn't completely full, then we can simply ignore a bad allocation. */
      if (xself->b_pagebuf.ps_mapc != xself->b_pagebuf.ps_mapm) {
       error_handled();
       goto increment_count;
      }
      error_rethrow();
     }

     /* Rehash the map. */
     if ((old_map = self->b_pagebuf.ps_mapv) != NULL) {
      size_t j;
      for (j = 0; j <= self->b_pagebuf.ps_mapm; ++j) {
       /* Skip DUMMY and NULL entries. */
       if (BLOCK_PAGE_ISDUMMYORNULL(old_map[j].bp_data))
           continue;
       perturb = i = BLKADDR_HASH(old_map[j].bp_addr) & new_mask;
       for (;; i = BLKADDR_NEXT(i,perturb),BLKADDR_SHFT(perturb)) {
        entry = &new_map[i & new_mask];
        if (entry->bp_data) continue;
        memcpy(entry,&old_map[j],sizeof(struct block_page));
        break;
       }
      }
      kfree(old_map);
     }
     self->b_pagebuf.ps_mapv = new_map;
     self->b_pagebuf.ps_mapm = new_mask;
     /* Dummy entries weren't remapped. */
     assert(self->b_pagebuf.ps_mapu <= self->b_pagebuf.ps_mapc);
     self->b_pagebuf.ps_mapc = self->b_pagebuf.ps_mapu;
     rwlock_endwrite(&self->b_pagebuf.ps_lock);
     goto find_dummy; /* SKIP_FINALLY IS INTENTIONAL! */
    }
   }
increment_count:
   ++self->b_pagebuf.ps_mapc;
  }

load_result:
  assert(addr < self->b_blockcount);
  result->bp_addr = addr;
  TRY {
   struct heapptr buf;
   /* Allocate the buffer for the resulting page.
    * NOTE: Since that buffer is for raw data only, we don't even track it. */
   buf = heap_alloc_untraced(&kernel_heaps[GFP_SHARED|GFP_LOCKED],
                     self->b_blocksize,GFP_SHARED|GFP_LOCKED);
   result->bp_data = (VIRT byte_t *)buf.hp_ptr;
   result->bp_size = buf.hp_siz;
   if (page_data) {
    memcpy(buf.hp_ptr,page_data,self->b_blocksize);
    result->bp_flags = BLOCK_PAGE_FCHANGED;
   } else {
    /* Load the page from disk. */
    if (flags & IO_NONBLOCK)
        error_throw(E_WOULDBLOCK);
    (*self->b_io.io_read)(self,buf.hp_ptr,1,addr);
    result->bp_flags = BLOCK_PAGE_FNORMAL;
   }
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   if (!BLOCK_PAGE_ISDUMMYORNULL(result->bp_data))
        heap_free_untraced(&kernel_heaps[GFP_SHARED|GFP_LOCKED],
                            result->bp_data,result->bp_size,
                            GFP_SHARED|GFP_LOCKED);
   /* Turn the resulting page into a dummy entry to keep the cache consistent. */
   result->bp_data = BLOCK_PAGE_DUMMY;
   error_rethrow();
  }
  /* Set the initial usage-counter of this page. (the first use counts as use #0) */
  result->bp_usage = 0;
  ++self->b_pagebuf.ps_mapu;
 } FINALLY {
  rwlock_endwrite(&xself->b_pagebuf.ps_lock);
 }
 return result;
}




/* Read/Write data to/from the given block-device.
 * @throw E_SEGFAULT: The given user-buffer is faulty.
 * @throw E_NO_DATA:  The given block range is overflowing, or out-of-bounds. */
PUBLIC size_t KCALL
block_device_read(struct block_device *__restrict self,
                  CHECKED USER void *buf, size_t num_bytes,
                  pos_t device_position, iomode_t flags) {
 struct block_device *EXCEPT_VAR xself;
 size_t EXCEPT_VAR xnum_bytes = num_bytes;
 size_t EXCEPT_VAR result = num_bytes;
 if unlikely(!num_bytes) goto done;
 /* Add the indirection associated with a partition. */
 if unlikely(__builtin_add_overflow(device_position,self->b_partstart,
                                   &device_position))
    error_throw(E_NO_DATA);
 self = self->b_master;
 assertf(self == self->b_master,
         "Recursive partitions must be resolved during creation");
 xself = self;
again:
 rwlock_readf(&self->b_pagebuf.ps_lock,flags);
 TRY {
  if (self->b_device.d_flags & (DEVICE_FCLOSED|DEVICE_BLOCK_FLINEAR)) {
   if (self->b_device.d_flags & DEVICE_FCLOSED)
       throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
   /* Use the linear read operator. */
   result = (*self->b_io.io_linear.l_read)(self,buf,xnum_bytes,device_position,flags);
   if unlikely(result < num_bytes && !(flags&IO_NONBLOCK))
      error_throw(E_NO_DATA);
  } else for (;;) {
   blkaddr_t pageno = (blkaddr_t)(device_position / self->b_blocksize);
   blksize_t pageof = (blksize_t)(device_position % self->b_blocksize);
   size_t max_read  = self->b_blocksize - (size_t)pageof;
   struct block_page *page;
   page = new_page(self,pageno,NULL,flags);
   if (max_read > xnum_bytes)
       max_read = xnum_bytes;
   memcpy(buf,(void *)((uintptr_t)page->bp_data + pageof),max_read);
   xnum_bytes         -= max_read;
   if (!xnum_bytes) break;
   device_position    += max_read;
   *(uintptr_t *)&buf += max_read;
  }
 } FINALLY {
  if (rwlock_endread(&xself->b_pagebuf.ps_lock))
      goto again;
  if (FINALLY_WILL_RETHROW && error_code() == E_WOULDBLOCK)
      return result-xnum_bytes;
 }
done:
 return result;
}

PUBLIC size_t KCALL
block_device_write(struct block_device *__restrict self,
                   CHECKED USER void const *buf, size_t num_bytes,
                   pos_t device_position, iomode_t flags) {
 struct block_device *EXCEPT_VAR xself;
 size_t EXCEPT_VAR xnum_bytes = num_bytes;
 size_t EXCEPT_VAR result = num_bytes;
 if unlikely(!num_bytes) goto done;
 /* Add the indirection associated with a partition. */
 if unlikely(__builtin_add_overflow(device_position,self->b_partstart,
                                   &device_position))
    error_throw(E_NO_DATA);
 self = self->b_master;
 assertf(self == self->b_master,
         "Recursive partitions must be resolved during creation");
 xself = self;
again:
 rwlock_readf(&self->b_pagebuf.ps_lock,flags);
 TRY {
  if (self->b_device.d_flags & (DEVICE_FREADONLY|DEVICE_FCLOSED|DEVICE_BLOCK_FLINEAR)) {
   if (self->b_device.d_flags & (DEVICE_FREADONLY|DEVICE_FCLOSED))
       throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
   /* Use the linear write operator. */
   result = (*self->b_io.io_linear.l_write)(self,buf,num_bytes,device_position,flags);
   if unlikely(result < num_bytes && !(flags&IO_NONBLOCK))
      error_throw(E_NO_DATA);
  } else for (;;) {
   blkaddr_t pageno = (blkaddr_t)(device_position / self->b_blocksize);
   blksize_t pageof = (blksize_t)(device_position % self->b_blocksize);
   size_t max_write  = self->b_blocksize - (size_t)pageof;
   struct block_page *page;
   if (max_write > xnum_bytes)
       max_write = xnum_bytes;
   if (max_write == self->b_blocksize) {
    assert(pageof == 0);
    /* Throw a segfault if the user-buffer is NULL. */
    if unlikely(!buf)
       error_throwf(E_SEGFAULT,0,(void *)0);
    /* Directly initialize new pages from the user-buffer (don't read them from disk). */
    page = new_page(self,pageno,buf,flags);
   } else {
    /* Do a partial write to a page. */
    page = new_page(self,pageno,NULL,flags);
    /* Mark the page as modified. */
    page->bp_flags |= BLOCK_PAGE_FCHANGED;
    memcpy((void *)((uintptr_t)page->bp_data + pageof),buf,max_write);
   }
   if (flags & IO_SYNC) {
    /* Sync changed pages immediately. */
    (*self->b_io.io_write)(self,
                           page->bp_data,
                           1,
                           page->bp_addr);
    COMPILER_BARRIER();
    page->bp_flags &= ~BLOCK_PAGE_FCHANGED;
   }
   xnum_bytes         -= max_write;
   if (!xnum_bytes) break;
   *(uintptr_t *)&buf += max_write;
   device_position    += max_write;
  }
 } FINALLY {
  if (rwlock_endread(&xself->b_pagebuf.ps_lock))
      goto again;
  if (FINALLY_WILL_RETHROW && error_code() == E_WOULDBLOCK)
      return result-xnum_bytes;
 }
done:
 return result;
}

PUBLIC void KCALL
block_device_sync(struct block_device *__restrict self) {
 struct block_device *EXCEPT_VAR xself;
 self = self->b_master;
 assertf(self == self->b_master,
         "Recursive partitions must be resolved during creation");
 xself = self;
 rwlock_write(&self->b_pagebuf.ps_lock);
 TRY {
  if (self->b_pagebuf.ps_mapv) {
   struct block_page *iter,*end;
   assertf(!(self->b_device.d_flags & DEVICE_BLOCK_FLINEAR),
           "Linear block device with non-empty page buffer");
   end = (iter = self->b_pagebuf.ps_mapv)+
                (self->b_pagebuf.ps_mapm+1);
   for (; iter != end; ++iter) {
    /* Search for changed pages. */
    if (BLOCK_PAGE_ISDUMMYORNULL(iter->bp_data)) continue;
    if (!(iter->bp_flags & BLOCK_PAGE_FCHANGED)) continue;
    /* Write the modified page to disk. */
    (*self->b_io.io_write)(self,iter->bp_data,1,iter->bp_addr);
    /* Unset the page-changed bit. */
    iter->bp_flags &= ~BLOCK_PAGE_FCHANGED;
   }
  }
  /* Invoke an optional, device-specific synchronization callback. */
  if (self->b_io.io_sync)
    (*self->b_io.io_sync)(self);
 } FINALLY {
  rwlock_endwrite(&xself->b_pagebuf.ps_lock);
 }
}


PRIVATE void KCALL
bad_partition_bounds(struct block_device *__restrict self,
                     pos_t partition_offset,
                     blkcnt_t partition_size) {
 /* TODO: Log warning */
}

PUBLIC REF struct block_device *KCALL
block_device_partition(struct block_device *__restrict self,
                       pos_t partition_offset,
                       blkcnt_t partition_size,
                       minor_t partition_number,
                       u8 const partition_guid[16]) {
 REF struct block_device *EXCEPT_VAR result;
 pos_t part_end;
 /* Validate partition bounds. */
 if unlikely(__builtin_mul_overflow(partition_size,self->b_blocksize,&part_end) ||
             __builtin_add_overflow(part_end,partition_offset,&part_end) ||
             part_end >= self->b_blocksize * self->b_blockcount) {
  blkcnt_t block_count;
  /* Partition is out-of-bounds! */
  bad_partition_bounds(self,partition_offset,partition_size);
  part_end = self->b_blockcount*self->b_blocksize;
  if (partition_offset >= part_end)
      error_throw(E_INVALID_ARGUMENT); /* Partition is fully out-ob-bounds */
  part_end   -= partition_offset;
  block_count = FLOORDIV(part_end,self->b_blocksize);
  if (partition_size > block_count)
      partition_size = block_count;
 }
 /* Adjust for the offset of another sub-partition. */
 partition_offset += self->b_partstart;
 self              = self->b_master;
 assert(self->b_master == self);
 /* Create a sub-partition. */
 if (!partition_size)
     error_throw(E_INVALID_ARGUMENT); /* Empty partition. */
 if (partition_number >= self->b_partmaxcnt)
     error_throw(E_INVALID_ARGUMENT); /* Bad partition number. */
 assert(CEILDIV(partition_offset,self->b_blocksize)+
        partition_size <= self->b_blockcount);
 result = (REF struct block_device *)device_alloc(DEVICE_TYPE_FBLOCKDEV,
                                                  offsetafter(struct block_device,
                                                              b_partition));
 result->b_blocksize       = self->b_blocksize;
 result->b_blockcount      = partition_size;
 result->b_partstart       = partition_offset;
 result->b_master          = self;
 result->b_device.d_devno  = self->b_device.d_devno;
 result->b_device.d_devno += partition_number+1;
 block_device_incref(self);
 TRY {
  /* Set the name of the part in the format of `hda1', `hda2', ... */
  device_setnamef(&result->b_device,"%s%u",
                  self->b_device.d_name,
                 (unsigned int)partition_number+1);
  /* Register the device globally. */
  if (!register_device(&result->b_device)) {
   /* Device already exists... */
   block_device_decref(result);
   return NULL;
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  block_device_decref(result);
  error_rethrow();
 }
 /* Add the device as a partition. */
 atomic_rwlock_write(&self->b_partlock);
 if (!DEVICE_ISREGISTERED(&self->b_device)) {
  atomic_rwlock_endwrite(&self->b_partlock);
  /* The master isn't registered (don't add new partitions) */
  TRY {
   unregister_device(&result->b_device);
  } FINALLY {
   block_device_decref(result);
  }
  error_throwf(E_NO_DEVICE,
               ERROR_NO_DEVICE_FBLOCKDEV,
               self->b_device.d_devno);
 }
 LIST_INSERT(self->b_partitions,result,
             b_partition.p_partitions);
 atomic_rwlock_endwrite(&self->b_partlock);
 return result;
}


#undef device_alloc
PUBLIC ATTR_RETNONNULL REF struct character_device *
KCALL __character_device_alloc(size_t struct_size,
                               struct driver *__restrict caller) {
 REF struct character_device *result;
 assert(struct_size >= sizeof(struct character_device));
 result = (REF struct character_device *)device_alloc(DEVICE_TYPE_FCHARDEV,
                                                      struct_size,
                                                      caller);
 return result;
}



/* Handle operators. */
DEFINE_INTERN_ALIAS(handle_device_read,device_read);
DEFINE_INTERN_ALIAS(handle_device_write,device_write);
DEFINE_INTERN_ALIAS(handle_device_seek,device_seek);
DEFINE_INTERN_ALIAS(handle_device_pread,device_pread);
DEFINE_INTERN_ALIAS(handle_device_pwrite,device_pwrite);
INTERN void KCALL
handle_device_sync(struct device *__restrict self,
                   bool UNUSED(only_data)) {
 device_sync(self);
}
DEFINE_INTERN_ALIAS(handle_device_ioctl,device_ioctl);
DEFINE_INTERN_ALIAS(handle_device_stat,device_stat);
DEFINE_INTERN_ALIAS(handle_device_poll,device_poll);



PUBLIC void KCALL
device_add_to_devfs(struct device *__restrict dev) {
 mode_t mode; REF struct inode *node;
 assertf(dev->d_name && *dev->d_name,
         "No device name set for %[dev]",dev->d_devno);
 mode = dev->d_type == DEVICE_TYPE_FBLOCKDEV ? (S_IFBLK|0755) : (S_IFCHR|0755);
 TRY {
  debug_printf("[DEV] Adding %s-device %q (dev_t = %[dev])\n",
               dev->d_type == DEVICE_TYPE_FBLOCKDEV ? "block" : "character",
               dev->d_name,dev->d_devno);
  node = directory_mknod(devfs.s_root,
                         dev->d_name,
                        (u16)strlen(dev->d_name),
                         mode,
                         0,
                         0,
                         dev->d_devno,
                         false);
  inode_decref(node);
 } CATCH_HANDLED (E_FILESYSTEM_ERROR) {
 }
}
PUBLIC void KCALL
device_remove_from_devfs(struct device *__restrict dev) {
 assertf(dev->d_name && *dev->d_name,
         "No device name set for %[dev]",dev->d_devno);
 TRY {
  u16 namlen = (u16)strlen(dev->d_name);
  debug_printf("[DEV] Removing %s-device %q (dev_t = %[dev])\n",
               dev->d_type == DEVICE_TYPE_FBLOCKDEV ? "block" : "character",
               dev->d_name,dev->d_devno);
  directory_remove(devfs.s_root,
                   dev->d_name,namlen,
                   directory_entry_hash(dev->d_name,namlen),
                   DIRECTORY_REMOVE_FREGULAR,
                   NULL);
 } CATCH_HANDLED (E_FILESYSTEM_ERROR) {
 }
}

DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_DEVICE_C */
