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
#ifndef GUARD_KERNEL_SRC_FS_NODE_C
#define GUARD_KERNEL_SRC_FS_NODE_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/align.h>
#include <kernel/vm.h>
#include <kernel/malloc.h>
#include <kernel/debug.h>
#include <fs/path.h>
#include <fs/device.h>
#include <bits/stat.h>
#include <linux/limits.h>
#include <limits.h>
#include <fs/node.h>
#include <dev/wall.h>
#include <fs/driver.h>
#include <string.h>
#include <except.h>
#include <assert.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <kernel/user.h>

DECL_BEGIN

PRIVATE ATTR_NORETURN void KCALL
throw_fs_error(u16 fs_error_code) {
 struct exception_info *info;
 info = error_info();
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_code                        = E_FILESYSTEM_ERROR;
 info->e_error.e_flag                        = ERR_FNORMAL;
 info->e_error.e_filesystem_error.fs_errcode = fs_error_code;
 error_throw_current();
 __builtin_unreachable();
}


PUBLIC WUNUSED uintptr_t KCALL
directory_entry_hash(CHECKED USER char const *__restrict name,
                     u16 namelen) {
 size_t const *iter,*end;
 size_t hash = DIRECTORY_ENTRY_EMPTY_HASH;
 end = (iter = (size_t const *)name)+(namelen/sizeof(size_t));
 while (iter != end) hash += *iter++,hash *= 9;
 switch (namelen & (sizeof(size_t)-1)) {
#if __SIZEOF_SIZE_T__ > 4
 case 7:  hash += (size_t)((byte_t *)iter)[6] << 48;
 case 6:  hash += (size_t)((byte_t *)iter)[5] << 40;
 case 5:  hash += (size_t)((byte_t *)iter)[4] << 32;
 case 4:  hash += (size_t)((byte_t *)iter)[3] << 24;
#endif
 case 3:  hash += (size_t)((byte_t *)iter)[2] << 16;
 case 2:  hash += (size_t)((byte_t *)iter)[1] << 8;
 case 1:  hash += (size_t)((byte_t *)iter)[0];
 default: break;
 }
 return hash;
}


PUBLIC ATTR_NOTHROW void KCALL
directory_entry_destroy(struct directory_entry *__restrict self) {
 /* Drop references from mounting points. */
 if unlikely(self->de_type = DT_WHT)
    inode_decref(self->de_virtual);
 kfree(self);
}


/* Destroy a previously allocated INode. */
PUBLIC ATTR_NOTHROW void KCALL
inode_destroy(struct inode *__restrict self) {
 /* NOTE: `i_ops' may be NULL for partially constructed nodes. */
 if (self->i_ops && self->i_ops->io_fini)
   (*self->i_ops->io_fini)(self);
 /* NOTE: `i_super' may be NULL for partially constructed nodes. */
 if (self->i_super) {
  /* Unlink from the superblock inodes list. */
  if (self->i_nodes.le_pself) {
   atomic_rwlock_write(&self->i_super->s_nodes_lock);
   LIST_REMOVE(self,i_nodes);
   assert(self->i_super->s_nodesc);
   --self->i_super->s_nodesc;
   atomic_rwlock_endwrite(&self->i_super->s_nodes_lock);
  }
  /* Unlink from the changed inodes list. */
  if ((self->i_flags & INODE_FCHANGED) &&
      (self->i_changed.le_pself != NULL)) {
   atomic_rwlock_write(&self->i_super->s_changed_lock);
   LIST_REMOVE(self,i_changed);
   atomic_rwlock_endwrite(&self->i_super->s_changed_lock);
  }
  if (self != (struct inode *)self->i_super->s_root)
      superblock_decref(self->i_super);
 }
 switch (self->i_attr.a_mode & S_IFMT) {

 {
  struct regular_node *me;
 case S_IFREG:
  me = (struct regular_node *)self;
  assert(!me->re_module.m_module);
 } break;

 {
  struct symlink_node *me;
 case S_IFLNK:
  me = (struct symlink_node *)self;
  kfree(me->sl_text);
 } break;

 {
  struct directory_node *me;
  struct directory_entry *iter,*next;
 case S_IFDIR:
  me = (struct directory_node *)self;
  assert((me->d_bypos != NULL) ==
         (me->d_bypos_end != NULL));
  iter = me->d_bypos;
  while (iter) {
   next = iter->de_next;
   /* Drop the saved reference from the directory entry. */
   directory_entry_decref(iter);
   iter = next;
  }
  kfree(me->d_map);
 } break;

 default: break;
 }
 kfree(self);
}


PUBLIC size_t KCALL
inode_read(struct inode *__restrict EXCEPT_VAR self,
           CHECKED USER void *buf, size_t bufsize,
           pos_t pos, iomode_t flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->i_lock,flags);
 TRY {
  if unlikely(INODE_ISCLOSED(self))
   result = 0; /* INode was closed. */
  else if likely(self->i_ops->io_file.f_pread)
   result = (*self->i_ops->io_file.f_pread)(self,buf,bufsize,pos,flags);
  else if (S_ISCHR(self->i_attr.a_mode)) {
   REF struct character_device *EXCEPT_VAR dev;
   result = 0; /* Try to read using the pread() operator of a pointed-to character device. */
   if ((dev = try_lookup_character_device(self->i_attr.a_rdev)) != NULL) {
    TRY {
     /* Invoke the pread() operator of the character device (if it exists) */
     if (dev->c_ops->c_file.f_pread)
         result = (*dev->c_ops->c_file.f_pread)(dev,buf,bufsize,pos,flags);
    } FINALLY {
     character_device_decref(dev);
    }
   }
  } else {
   result = 0; /* No way of reading data */
  }
 } FINALLY {
  if (rwlock_endread(&self->i_lock))
      goto again;
 }
 return result;
}
PUBLIC size_t KCALL
inode_write(struct inode *__restrict EXCEPT_VAR self,
            CHECKED USER void const *buf,
            size_t bufsize, pos_t pos, iomode_t flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 rwlock_writef(&self->i_lock,flags);
 TRY {
  if unlikely(INODE_ISCLOSED(self))
   result = 0; /* INode was closed. */
  else if likely(self->i_ops->io_file.f_pwrite)
   result = (*self->i_ops->io_file.f_pwrite)(self,buf,bufsize,pos,flags);
  else if (S_ISCHR(self->i_attr.a_mode)) {
   REF struct character_device *EXCEPT_VAR dev;
   result = 0; /* Try to write using the pwrite() operator of a pointed-to character device. */
   if ((dev = try_lookup_character_device(self->i_attr.a_rdev)) != NULL) {
    TRY {
     /* Invoke the pwrite() operator of the character device (if it exists) */
     if (dev->c_ops->c_file.f_pwrite)
         result = (*dev->c_ops->c_file.f_pwrite)(dev,buf,bufsize,pos,flags);
    } FINALLY {
     character_device_decref(dev);
    }
   }
  } else {
   result = 0; /* No way of writing data */
  }
 } FINALLY {
  rwlock_endwrite(&self->i_lock);
 }
 return result;
}

PUBLIC void KCALL
inode_kreadall(struct inode *__restrict self,
               void *__restrict buf, size_t bufsize,
               pos_t pos, iomode_t flags) {
 size_t temp;
 for (;;) {
  if (!bufsize) return;
  temp = inode_kread(self,buf,bufsize,pos,flags);
  assert(temp <= bufsize);
  if (!temp) break;
  bufsize            -= temp;
  *(uintptr_t *)&buf += temp;
 }
 error_throw(E_NO_DATA);
}
PUBLIC void KCALL
inode_kwriteall(struct inode *__restrict self,
                void const *__restrict buf, size_t bufsize,
                pos_t pos, iomode_t flags) {
 size_t temp;
 for (;;) {
  if (!bufsize) return;
  temp = inode_kwrite(self,buf,bufsize,pos,flags);
  assert(temp <= bufsize);
  if (!temp) break;
  bufsize            -= temp;
  *(uintptr_t *)&buf += temp;
 }
 throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
}


PUBLIC void KCALL
inode_access(struct inode *__restrict self, int how) {
 /* TODO */
 (void)self;
 (void)how;
}

PUBLIC WUNUSED int KCALL
inode_accessok(struct inode *__restrict self) {
 /* TODO */
 (void)self;
 return R_OK|W_OK|X_OK;
}



PUBLIC void KCALL
inode_changed(struct inode *__restrict self) {
 struct superblock *super;
 assertf(self->i_nlink != 0,"The node has been deallocated");
 assertf(self->i_flags & INODE_FATTRLOADED,
         "Attributes must be loaded before an INode can be marked as changed");
 assert(rwlock_writing(&self->i_lock));
 /* Update the last-modified timestamp. */
 if (!(self->i_super->s_flags&SUPERBLOCK_FNOMTIME))
       self->i_attr.a_mtime = wall_gettime(self->i_super->s_wall);
 assertf(self->i_ops->io_saveattr,
         "You must only change a node if it "
         "implements the `io_saveattr' operator");
 if (!(ATOMIC_FETCHOR(self->i_flags,INODE_FCHANGED) & INODE_FCHANGED)) {
  /* Do not register the INode if then superblock has been closed. */
  if (self->i_flags&INODE_FCLOSED)
      return;
  super = self->i_super;
  atomic_rwlock_write(&super->s_changed_lock);
  LIST_INSERT(super->s_changed,self,i_changed);
  atomic_rwlock_endwrite(&super->s_changed_lock);
 }
}

/* Ensure that attributes have been loaded for the given INode. */
PUBLIC void KCALL
inode_loadattr(struct inode *__restrict EXCEPT_VAR self) {
 /* Check attribute have already been loaded. */
 if (!(self->i_flags&INODE_FATTRLOADED)) {
  assert(self->i_ops->io_loadattr);
  rwlock_write(&self->i_lock);
  if unlikely(self->i_flags&INODE_FATTRLOADED) {
   rwlock_endwrite(&self->i_lock);
   return;
  }
  TRY {
   if unlikely(INODE_ISCLOSED(self))
      throw_fs_error(ERROR_FS_FILE_NOT_FOUND);
   (*self->i_ops->io_loadattr)(self);
  } FINALLY {
   /* Set the attributes-loaded flag on success. */
   if (!FINALLY_WILL_RETHROW)
        self->i_flags |= INODE_FATTRLOADED;
   rwlock_endwrite(&self->i_lock);
  }
 }
}



/* Truncate the length of the given INode.
 * @param: mode: Set of `DIRECTORY_REMOVE_F*'
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:          [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:   [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_TRUNCATE_GREATER_SIZE: [...] */
PUBLIC void KCALL
inode_truncate(struct inode *__restrict EXCEPT_VAR self,
               pos_t new_smaller_size) {
 inode_access(self,W_OK);
 if (!self->i_ops->io_file.f_truncate)
      throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
 rwlock_write(&self->i_lock);
 TRY {
  if unlikely(INODE_ISCLOSED(self))
     throw_fs_error(ERROR_FS_FILE_NOT_FOUND);
  if unlikely(new_smaller_size >= self->i_attr.a_size)
     throw_fs_error(ERROR_FS_TRUNCATE_GREATER_SIZE);
  (*self->i_ops->io_file.f_truncate)(self,new_smaller_size);
  /* Update the size attribute. */
  self->i_attr.a_size = new_smaller_size;
 } FINALLY {
  rwlock_endwrite(&self->i_lock);
 }
}

PUBLIC ssize_t KCALL
inode_ioctl(struct inode *__restrict self, unsigned long cmd,
            USER UNCHECKED void *arg, iomode_t mode) {
 assert(rwlock_writing(&self->i_lock));
 if (!self->i_ops->io_file.f_ioctl)
      error_throw(E_NOT_IMPLEMENTED);
 return (*self->i_ops->io_file.f_ioctl)(self,cmd,arg,mode);
}

PUBLIC WUNUSED long int KCALL
inode_pathconf(struct inode *__restrict self, int name) {
 long int result;
 struct inode *root;
 /* Try to invoke the pathconf operator. */
 if (self->i_ops->io_pathconf &&
    ((result = (*self->i_ops->io_pathconf)(self,name)) != -EINVAL))
    return result;
 /* Forward the command to the superblock's root INode. */
 root = &self->i_super->s_root->d_node;
 if (root->i_ops->io_pathconf &&
    ((result = (*root->i_ops->io_pathconf)(root,name)) != -EINVAL))
    return result;
 /* Default path configurations. */
 switch (name) {

 case _PC_LINK_MAX:
  return LINK_MAX;

 case _PC_MAX_CANON:
  return MAX_CANON;

 case _PC_MAX_INPUT:
  return MAX_INPUT;

 case _PC_NAME_MAX:
  return 65535;

 case _PC_PATH_MAX:
  return LONG_MAX;

 case _PC_PIPE_BUF:
  return PIPE_BUF;

 case _PC_CHOWN_RESTRICTED:
  return 0;

 case _PC_NO_TRUNC:
  return 1;

 case _PC_VDISABLE:
  return '\0';

 case _PC_FILESIZEBITS:
  return 64;

 case _PC_REC_MAX_XFER_SIZE:
  return self->i_super->s_device->b_blocksize*8;

 case _PC_REC_INCR_XFER_SIZE:
 case _PC_REC_MIN_XFER_SIZE:
 case _PC_REC_XFER_ALIGN:
 case _PC_ALLOC_SIZE_MIN:
  return self->i_super->s_device->b_blocksize;

 case _PC_SYMLINK_MAX:
  return self->i_ops->io_directory.d_symlink != NULL ? LONG_MAX : 0;

 case _PC_2_SYMLINKS:
  return self->i_ops->io_directory.d_symlink != NULL ? 1 : 0;

 default: break;
 }
 return -EINVAL;
}

PUBLIC void KCALL
inode_stat(struct inode *__restrict EXCEPT_VAR self,
           USER CHECKED struct stat64 *result) {
 struct block_device *superdev;
again:
 rwlock_read(&self->i_lock);
 TRY {
  superdev = self->i_super->s_device;
  /* CAUTION: SEGFAULT */
  COMPILER_WRITE_BARRIER();
  result->st_dev      = superdev->b_device.d_devno;
  result->st_ino64    = self->i_attr.a_ino;
  result->st_mode     = self->i_attr.a_mode;
  result->st_nlink    = self->i_nlink;
  result->st_uid      = self->i_attr.a_uid;
  result->st_gid      = self->i_attr.a_gid;
  result->st_rdev     = self->i_attr.a_rdev;
  result->st_size64   = self->i_attr.a_size;
  result->st_blksize  = superdev->b_blocksize;
  result->st_blocks64 = (self->i_super->s_flags & SUPERBLOCK_FNOBLOCKS)
                      ? CEILDIV(self->i_attr.a_size,superdev->b_blocksize)
                      : self->i_attr.a_blocks;
  result->st_atim32.tv_sec = (time32_t)(result->st_atim64.tv_sec = self->i_attr.a_atime.tv_sec);
  result->st_mtim32.tv_sec = (time32_t)(result->st_mtim64.tv_sec = self->i_attr.a_mtime.tv_sec);
  result->st_ctim32.tv_sec = (time32_t)(result->st_ctim64.tv_sec = self->i_attr.a_ctime.tv_sec);
  result->st_atim32.tv_nsec = result->st_atim64.tv_nsec = self->i_attr.a_atime.tv_nsec;
  result->st_mtim32.tv_nsec = result->st_mtim64.tv_nsec = self->i_attr.a_mtime.tv_nsec;
  result->st_ctim32.tv_nsec = result->st_ctim64.tv_nsec = self->i_attr.a_ctime.tv_nsec;
  COMPILER_WRITE_BARRIER();
 } FINALLY {
  if (rwlock_endread(&self->i_lock))
      goto again;
 }
}

PUBLIC unsigned int KCALL
inode_poll(struct inode *__restrict self,
           unsigned int mode) {
 if (self->i_ops->io_file.f_poll)
     return (*self->i_ops->io_file.f_poll)(self,mode);
 /* Poll the INode's R/W lock. */
 return rwlock_poll(&self->i_lock,mode);
}



PUBLIC void KCALL
inode_chtime(struct inode *__restrict EXCEPT_VAR self,
             struct timespec *new_atime,
             struct timespec *new_mtime,
             struct timespec *new_ctime) {
 /* Check if attributes can be modified. */
 if unlikely(!self->i_ops->io_saveattr)
    throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
#if 0 /* TODO: Check for the `CAP_CHCTIME' capability. */
 if (new_ctime)
    throw_fs_error(ERROR_FS_ACCESS_ERROR);
#endif

 rwlock_write(&self->i_lock);
 TRY {
  inode_access(self,W_OK);
  /* Update INode attributes. */
  if (new_atime) self->i_attr.a_atime = *new_atime;
  if (new_mtime) self->i_attr.a_mtime = *new_mtime;
  if (new_ctime) self->i_attr.a_ctime = *new_ctime;
  /* Mask attributes. */
  if (self->i_ops->io_maskattr)
    (*self->i_ops->io_maskattr)(self);
  /* Mark the INode as having changed. */
  inode_changed(self);
 } FINALLY {
  rwlock_endwrite(&self->i_lock);
 }
}



PUBLIC mode_t KCALL
inode_chmod(struct inode *__restrict EXCEPT_VAR self,
            mode_t perm_mask, mode_t perm_flag) {
 mode_t COMPILER_IGNORE_UNINITIALIZED(result),old_mode;
 if (!self->i_ops->io_saveattr)
      throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
 rwlock_write(&self->i_lock);
 TRY {
  /* TODO: Check file access permissions. */

  /* Determine the unmasked new file mode. */
  old_mode = self->i_attr.a_mode & 07777;
  result   = (old_mode & perm_mask) | perm_flag;
  if (old_mode != result) {
   /* Set the new file mode. */
   self->i_attr.a_mode &= ~07777;
   self->i_attr.a_mode |= result;
   if (self->i_ops->io_maskattr)
     (*self->i_ops->io_maskattr)(self);
   result = self->i_attr.a_mode & 07777;
   if (result != old_mode) {
    /* The file access mode has changed.
     * -> Also mark the INode itself as changed. */
    inode_changed(self);
   }
  }
 } FINALLY {
  rwlock_endwrite(&self->i_lock);
 }
 return result;
}

PUBLIC void KCALL
inode_chown(struct inode *__restrict EXCEPT_VAR self,
            uid_t owner, gid_t group) {
 if (!self->i_ops->io_saveattr)
      throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
 rwlock_write(&self->i_lock);
 TRY {
  /* TODO: Check file access permissions. */
  uid_t old_owner,old_group;

  /* Set the new file mode. */
  old_owner = self->i_attr.a_uid;
  old_group = self->i_attr.a_gid;
  if (old_owner != owner || old_group != group) {
   self->i_attr.a_uid = owner;
   self->i_attr.a_gid = group;
   if (self->i_ops->io_maskattr)
     (*self->i_ops->io_maskattr)(self);
   if (self->i_attr.a_uid == old_owner &&
       self->i_attr.a_gid == old_group) {
    /* The owner and group were masked out by `io_maskattr()'
     * -> Assume that the INode cannot change ownership. */
    throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
   } else {
    inode_changed(self);
   }
  }
 } FINALLY {
  rwlock_endwrite(&self->i_lock);
 }
}






PUBLIC void KCALL
inode_sync(struct inode *__restrict self, bool data_only) {
 if (!data_only) inode_syncatttr(self);
 /* TODO: New INode operator that selectively synchronizes
  *       block-pages belonging to the affected INode. */
}

PUBLIC bool KCALL
inode_syncatttr(struct inode *__restrict self) {
 if (!(self->i_flags & INODE_FCHANGED))
       return false;
 /* TODO: Lock the node. */
 /* TODO: Remove the node from the superblock's changed-list. */
 /* TODO: Try to call `io_saveattr'. */
 /* TODO: If that fails, re-add the node to the superblock's changed-list. */
 return true;
}





/* Ensure that the given symbolic-link INode has been loaded.
 * This function is a no-op when `INODE_FLNKLOADED' is already set,
 * and will set `INODE_FLNKLOADED' after successfully loading the text.
 * @throw: E_IO_ERROR:              [...]
 * @throw: ERROR_FS_FILE_NOT_FOUND: [...] */
PUBLIC void KCALL
symlink_node_load(struct symlink_node *__restrict EXCEPT_VAR self) {
 assert(INODE_ISLNK(&self->sl_node));
 /* Check if the symbolic link has already been loaded. */
 if (self->sl_text == NULL) {
  assert(self->sl_node.i_ops->io_symlink.sl_readlink);
  rwlock_write(&self->sl_node.i_lock);
  TRY {
   COMPILER_READ_BARRIER();
   if likely(self->sl_text == NULL) {
    if unlikely(INODE_ISCLOSED(&self->sl_node))
       throw_fs_error(ERROR_FS_FILE_NOT_FOUND);
    (*self->sl_node.i_ops->io_symlink.sl_readlink)(self);
   }
  } FINALLY {
   rwlock_endwrite(&self->sl_node.i_lock);
  }
 }
}


PRIVATE void KCALL
directory_rehash(struct directory_node *__restrict self) {
 REF struct directory_entry **new_map;
 REF struct directory_entry *iter,*next;
 size_t i,new_mask;
 assert(rwlock_writing(&self->d_node.i_lock));
 new_mask = (self->d_mask << 1)|1;
 /* Allocate the new directory map. */
 new_map  = (REF struct directory_entry **)kmalloc((new_mask+1)*sizeof(REF struct directory_entry *),
                                                    GFP_SHARED|GFP_CALLOC);
 /* Rehash all the entries. */
 for (i = 0; i <= self->d_mask; ++i) {
  iter = self->d_map[i];
  while (iter) {
   next = iter->de_next;
   /* Insert the entry into the new map. */
   iter->de_next = new_map[iter->de_hash & new_mask];
   new_map[iter->de_hash & new_mask] = iter;
   iter = next;
  }
 }
 /* Free the old map and setup the new mask. */
 kfree(self->d_map);
 self->d_map  = new_map;
 self->d_mask = new_mask;
}



/* Read the next directory entry from
 * `self' that hasn't been loaded, yet.
 * Return NULL and set the `INODE_FDIRLOADED' flag
 * once the entirety of the directory has been loaded.
 * NOTE: The caller must be holding a read-lock on the directory INode. */
PUBLIC WUNUSED struct directory_entry *KCALL
directory_readnext(struct directory_node *__restrict EXCEPT_VAR self,
                   iomode_t flags) {
 struct directory_entry *COMPILER_IGNORE_UNINITIALIZED(result);
 pos_t last_directory_position;
#ifndef NDEBUG
 pos_t entry_start_position;
#endif
 /* Quick check: has the directory been fully loaded. */
 if (self->d_node.i_flags & INODE_FDIRLOADED)
     return NULL;
 /* Make sure that we've got a readdir() operator to work with. */
 if unlikely(!self->d_node.i_ops->io_directory.d_readdir) {
  ATOMIC_FETCHOR(self->d_node.i_flags,INODE_FDIRLOADED);
  return NULL;
 }
continue_reading:
 /* Read more entires. */
 last_directory_position = self->d_dirend;
#ifndef NDEBUG
 entry_start_position = last_directory_position;
#endif
 TRY {
  result = (*self->d_node.i_ops->io_directory.d_readdir)(self,&last_directory_position,flags);
 } CATCH (E_NO_DATA) {
  /* Catch E_NO_DATA and handle it as end-of-directory. */
  result = NULL;
 }
 assert(!result || result->de_namelen != 0);
 assertf(!result || !isspace(result->de_name[0]),
         "Directory entry %$q must not start with spaces",
        (size_t)result->de_namelen,result->de_name);
 assertf(!result || !isspace(result->de_name[result->de_namelen-1]),
         "Directory entry %$q must not end with spaces",
        (size_t)result->de_namelen,result->de_name);

 TRY {
  /* get a write lock to the directory. */
  rwlock_write(&self->d_node.i_lock);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Relase the reference returned by `d_readdir()' */
  if (result)
      directory_entry_decref(result);
  error_rethrow();
 }

 TRY {
  struct directory_entry **presult;
#ifndef NDEBUG
  /* `rwlock_write()' will have thrown an error if it didn't
   *  manage to upgrade the associated R/W-lock atomically.
   *  That error will be propagated until the point where
   *  the caller originally acquired the first read-lock
   *  to the node, which will be released, then re-acquired,
   *  before we try this whole thing again.
   *  In other words: If we get here, we should be able to
   *                  assume that the `rwlock_write()' above
   *                  didn't temporarily grant other threads
   *                  write-access to the `d_dirend' field. */
  assert(self->d_dirend == entry_start_position);
  assert(!result ||
        (result->de_pos >= entry_start_position &&
         result->de_pos <  last_directory_position));
#endif
  if (result) {
   /* assert(result->de_type != DT_WHT); // Actually, this is allowed... */
   /* Set the partially-loaded flag and the directory
    * end pointer up to which data has now been loaded. */
   self->d_dirend = last_directory_position;
   if (!self->d_bypos_end) {
    assert(!self->d_bypos);
    self->d_bypos             = result;
    self->d_bypos_end         = result;
    result->de_bypos.le_pself = &self->d_bypos;
    result->de_bypos.le_next  = NULL;
   } else if (result->de_pos < self->d_bypos_end->de_pos) {
    struct directory_entry *insert_after;
    /* When the directory is being re-loaded after a umount(),
     * an older entry for this directory entry may still exist,
     * or we just have to insert the new entry somewhere else. */
    insert_after = self->d_bypos_end;
    do insert_after = COMPILER_CONTAINER_OF(insert_after->de_bypos.le_pself,
                                            struct directory_entry,de_bypos.le_next);
    while (insert_after->de_pos > result->de_pos);
    if (insert_after->de_pos == result->de_pos) {
     /* Entry already exists! */
     rwlock_endwrite(&self->d_node.i_lock);
     goto continue_reading; /* XXX: Jump out of try-block skips execution of finally */
    } else {
     LIST_INSERT_AFTER(insert_after,result,de_bypos);
    }
   } else {
    /* Add the directory entry to the by-position chain of directory entries. */
    result->de_bypos.le_pself           = &self->d_bypos_end->de_bypos.le_next;
    self->d_bypos_end->de_bypos.le_next = result;
    result->de_bypos.le_next            = NULL;
    self->d_bypos_end                   = result;
   }
   /* Add the new entry to the hash-map. */
   presult = &self->d_map[result->de_hash & self->d_mask];
   result->de_next = *presult; /* Inherit reference. */
   *presult = result; /* Inherit reference. */
   ++self->d_size;

   /* Add the directory entry to the hash-map. */
   if (self->d_size >= (self->d_mask/3)*2) {
    assert(self->d_mask != 0);
    /* Since this is a hash-map, we can simply
     * ignore allocation failures during re-hashing. */
    TRY {
     directory_rehash(self);
    } CATCH(E_BADALLOC) {
    }
   }
  } else {
   /* End of directory. (Set the proper flag) */
   ATOMIC_FETCHOR(self->d_node.i_flags,
                  INODE_FDIRLOADED);
  }
 } FINALLY {
  /* Release the write-lock (the caller still has a read-lock,
   * so we don't have to return a reference, but can simply forward
   * the weakly referenced directory entry from the hash-map) */
  rwlock_endwrite(&self->d_node.i_lock);
 }
 assert(rwlock_reading(&self->d_node.i_lock));
 return result;
}




/* Lookup a directory item, given its name.
 * NOTE: The caller must be holding a read-lock on `self->d_node.i_lock'
 * @return: * :   A reference to the associated INode.
 * @return: NULL: No INode with the given `name' exists.
 *              (`ERROR_FS_FILE_NOT_FOUND' / `ERROR_FS_PATH_NOT_FOUND'-style)
 * @throw: E_SEGFAULT: Failed to access the given `name'.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_NOT_FOUND:       [...] */
PUBLIC WUNUSED struct directory_entry *KCALL
directory_getentry(struct directory_node *__restrict self,
                   CHECKED USER char const *__restrict name,
                   u16 namelen, uintptr_t hash) {
 REF struct directory_entry *result;
 assert(rwlock_reading(&self->d_node.i_lock));
 assert(self->d_mask != 0);
 if unlikely(!self->d_size) goto read_directory;
 inode_access(&self->d_node,R_OK|X_OK);
 result = self->d_map[hash & self->d_mask];
 for (; result; result = result->de_next) {
  /* Check hash and name-length. */
  if (result->de_hash    != hash) continue;
  if (result->de_namelen != namelen) continue;
  /* Highly likely that this is our guy... */
  if unlikely(memcmp(result->de_name,name,namelen*sizeof(char)) != 0)
     continue;
  return result;
 }
 /* Read more directory entries. */
read_directory:
 while ((result = directory_readnext(self,IO_RDONLY)) != NULL) {
  /* Check if this is the one. */
  if (result->de_hash    != hash) continue;
  if (result->de_namelen != namelen) continue;
  /* Highly likely that this is our guy... */
  if unlikely(memcmp(result->de_name,name,namelen*sizeof(char)) != 0)
     continue;
  break;
 }
 return result;
}
PUBLIC WUNUSED struct directory_entry *KCALL
directory_getcaseentry(struct directory_node *__restrict self,
                       CHECKED USER char const *__restrict name,
                       u16 namelen, uintptr_t hash) {
 REF struct directory_entry *result;
 assert(rwlock_reading(&self->d_node.i_lock));
 assert(self->d_mask != 0);
 if unlikely(!self->d_size) goto read_directory;
 inode_access(&self->d_node,R_OK|X_OK);
 result = self->d_map[hash & self->d_mask];
 for (; result; result = result->de_next) {
  /* Check hash and name-length. */
  if (result->de_hash    != hash) continue;
  if (result->de_namelen != namelen) continue;
  /* Highly likely that this is our guy... */
  if unlikely(memcmp(result->de_name,name,namelen*sizeof(char)) != 0)
     continue;
  return result;
 }
 if unlikely(!namelen)
    return NULL;
 /* Check for non-matching casing. */
 {
  char first_char = name[0];
  COMPILER_READ_BARRIER();
  first_char = tolower(first_char);
  for (hash = 0; hash <= self->d_mask; ++hash) {
   result = self->d_map[hash];
   for (; result; result = result->de_next) {
    /* Check hash and name-length. */
    if (result->de_namelen != namelen) continue;
    if (tolower(result->de_name[0]) != first_char) continue;
    /* Highly likely that this is our guy... */
    if unlikely(memcasecmp(result->de_name,name,
                           namelen*sizeof(char)) != 0)
       continue;
    return result;
   }
  }
 }

 /* Read more directory entries. */
read_directory:
 while ((result = directory_readnext(self,IO_RDONLY)) != NULL) {
  /* Check if this is the one. */
  if (result->de_hash    != hash) continue;
  if (result->de_namelen != namelen) continue;
  /* Highly likely that this is our guy... */
  if unlikely(memcasecmp(result->de_name,name,namelen*sizeof(char)) != 0)
     continue;
  break;
 }
 return result;
}

/* Same as `directory_getentry()', but automatically dereference
 * the directory entry to retrieve the associated INode. */
PUBLIC WUNUSED REF struct inode *KCALL
directory_getnode(struct directory_node *__restrict EXCEPT_VAR self,
                  CHECKED USER char const *__restrict name,
                  u16 namelen, uintptr_t hash) {
 struct directory_entry *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(entry);
 REF struct inode *COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_read(&self->d_node.i_lock);
 TRY {
  entry = directory_getentry(self,name,namelen,hash);
  /* Load a reference to the entry. */
  if (entry) directory_entry_incref(entry);
 } FINALLY {
  if (rwlock_endread(&self->d_node.i_lock))
      goto again;
 }
 if (!entry) return NULL;
 TRY {
  if unlikely(entry->de_type == DT_WHT) {
   /* Virtual link. */
   result = entry->de_virtual;
   inode_incref(result);
  } else {
   result = superblock_opennode(self->d_node.i_super,self,entry);
  }
 } FINALLY {
  directory_entry_decref(entry);
 }
 return result;
}
PUBLIC WUNUSED REF struct inode *KCALL
directory_getcasenode(struct directory_node *__restrict EXCEPT_VAR self,
                      CHECKED USER char const *__restrict name,
                      u16 namelen, uintptr_t hash) {
 struct directory_entry *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(entry);
 REF struct inode *COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_read(&self->d_node.i_lock);
 TRY {
  entry = directory_getcaseentry(self,name,namelen,hash);
  /* Load a reference to the entry. */
  if (entry) directory_entry_incref(entry);
 } FINALLY {
  if (rwlock_endread(&self->d_node.i_lock))
      goto again;
 }
 if (!entry) return NULL;
 TRY {
  if unlikely(entry->de_type == DT_WHT) {
   /* Virtual link. */
   result = entry->de_virtual;
   inode_incref(result);
  } else {
   result = superblock_opennode(self->d_node.i_super,self,entry);
  }
 } FINALLY {
  directory_entry_decref(entry);
 }
 return result;
}

#if __SIZEOF_INO_T__ <= __SIZEOF_POINTER__
#define INO_HASH(x) ((uintptr_t)(x))
#else
#define INO_HASH(x) ((uintptr_t)((x) >> (8*__SIZEOF_POINTER__)) ^ (uintptr_t)(x))
#endif

PRIVATE void KCALL
superblock_rehash_and_unlock(struct superblock *__restrict self) {
 REF LIST_HEAD(struct inode) *COMPILER_IGNORE_UNINITIALIZED(new_map);
 REF LIST_HEAD(struct inode) *old_map;
 struct inode *iter,*next,**pbucket;
 size_t i,new_mask = (self->s_nodesm << 1)|1;
 /* Try to allocate a bigger hash-map. */
 atomic_rwlock_endwrite(&self->s_nodes_lock);
 TRY {
  new_map = (REF LIST_HEAD(struct inode) *)kmalloc((new_mask+1)*
                                                    sizeof(LIST_HEAD(struct inode)),
                                                    GFP_SHARED|GFP_CALLOC);
 } CATCH (E_BADALLOC) {
  return;
 }
 /* @NOEXCEPT:begin */
 atomic_rwlock_write(&self->s_nodes_lock);
 /* Install the new map. */
 old_map = self->s_nodesv;
 for (i = 0; i <= self->s_nodesm; ++i) {
  iter = old_map[i];
  while (iter) {
   next = iter->i_nodes.le_next;
   /* Transfer the node into the new map. */
   pbucket = &new_map[INO_HASH(iter->i_attr.a_ino)];
   LIST_INSERT(*pbucket,iter,i_nodes);
   iter = next;
  }
 }
 /* Save the new map vector and its mask. */
 self->s_nodesv = new_map;
 self->s_nodesm = new_mask;
 atomic_rwlock_endwrite(&self->s_nodes_lock);
 /* @NOEXCEPT:end */
 kfree(old_map);
}

PUBLIC void KCALL
superblock_addnode(struct superblock *__restrict self,
                   struct inode *__restrict node) {
 REF struct inode **pbucket;
 uintptr_t ino_hash = INO_HASH(node->i_attr.a_ino);
 atomic_rwlock_write(&self->s_nodes_lock);
 pbucket = &self->s_nodesv[ino_hash & self->s_nodesm];
#ifndef NDEBUG
 /* Assert that the node isn't already cached. */
 {
  REF struct inode *old_node = *pbucket;
  for (; old_node; old_node = old_node->i_nodes.le_next) {
   assert(old_node->i_super == self);
   assertf(old_node->i_attr.a_ino != node->i_attr.a_ino,
           "Node %I64u has already been defined using %p (new node is %p)",
           old_node->i_attr.a_ino,old_node,node);
  }
 }
#endif

 if unlikely(node->i_flags & INODE_FCHANGED) {
  assert(node->i_ops->io_saveattr);
  /* Add the node the chain of changed nodes. */
  atomic_rwlock_write(&self->s_changed_lock);
  LIST_INSERT(self->s_changed,node,i_changed);
  atomic_rwlock_endwrite(&self->s_changed_lock);
 }

 /* Insert the new node into the hash-map. */
 inode_incref(node);
 LIST_INSERT(*pbucket,node,i_nodes);
 if (++self->s_nodesc >= (self->s_nodesm/3)*2) {
  superblock_rehash_and_unlock(self);
 } else {
  atomic_rwlock_endwrite(&self->s_nodes_lock);
 }
}


PRIVATE void KCALL
directory_addentry(struct directory_node *__restrict self,
               REF struct directory_entry **__restrict pentry) {
 struct directory_entry *insert_after,**pdirent;
 struct directory_entry *entry = *pentry;

 /* Add the directory entry to the by-position chain of directory entries. */
 insert_after = self->d_bypos_end;
 if unlikely(!insert_after) {
  /* Special case: first entry. */
  self->d_bypos     = entry;
  self->d_bypos_end = entry;
 } else {
  /* Find the entry after which we must insert this one. */
  while (entry->de_pos < insert_after->de_pos) {
   insert_after = COMPILER_CONTAINER_OF(insert_after->de_bypos.le_pself,
                                        struct directory_entry,
                                        de_bypos.le_next);
  }
  entry->de_bypos.le_pself = &insert_after->de_bypos.le_next;
  insert_after->de_bypos.le_next = entry;
  assert((insert_after->de_bypos.le_next != NULL) ==
         (self->d_bypos_end == insert_after));
  if likely(self->d_bypos_end == insert_after) {
   entry->de_bypos.le_next = NULL;
   self->d_bypos_end = entry;
  } else {
   /* Insert in-between two other entries. (Can happen if `d_creat' re-uses older slots) */
   assert(insert_after->de_bypos.le_next);
   entry->de_bypos.le_next = insert_after->de_bypos.le_next;
   entry->de_bypos.le_next->de_bypos.le_pself = &entry->de_bypos.le_next;
  }
 }

 /* Add the new entry to the hash-map. */
 pdirent = &self->d_map[entry->de_hash & self->d_mask];
 entry->de_next = *pdirent; /* Inherit reference. */
 *pdirent = entry; /* Inherit reference. */

 /* Set pentry to NULL now that we've inherited it. */
 *pentry = NULL;

 /* Track the total number of directory entires. */
 ++self->d_size;

 if (self->d_size >= (self->d_mask/3)*2) {
  assert(self->d_mask != 0);
  /* Since this is a hash-map, we can simply
   * ignore allocation failures during re-hashing. */
  TRY {
   directory_rehash(self);
  } CATCH(E_BADALLOC) {
  }
 }
}



PUBLIC WUNUSED ATTR_RETNONNULL REF struct inode *KCALL
directory_creatfile(struct directory_node *__restrict EXCEPT_VAR self,
                    CHECKED USER char const *__restrict name,
                    u16 namelen, oflag_t open_mode,
                    uid_t owner, gid_t group, mode_t mode,
                    REF struct directory_entry **EXCEPT_VAR pentry) {
 REF struct regular_node *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct directory_entry *EXCEPT_VAR result_dirent;
 assert(rwlock_reading(&self->d_node.i_lock));
 if unlikely(!namelen)
    throw_fs_error(ERROR_FS_ILLEGAL_PATH);
 if unlikely(INODE_ISCLOSED(&self->d_node))
    throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
 inode_access(&self->d_node,W_OK);
 result_dirent = (REF struct directory_entry *)kmalloc(offsetof(struct directory_entry,de_name)+
                                                     ((size_t)namelen+1)*sizeof(char),
                                                       GFP_SHARED);
 result_dirent->de_refcnt  = 1;
 result_dirent->de_namelen = namelen;
 result_dirent->de_type    = DT_REG;
 /* Copy the name from user-space. */
 TRY {
  memcpy(result_dirent->de_name,name,namelen*sizeof(char));
  /* Check if the file already exists.
   * NOTE: This can only (and must be) done now & using the
   *       resulting directory entry, simply because we can't
   *       trust a user-space string not to just change randomly.
   *       For that reason, we now need to use our new copy of it
   *       to do the final verification there there isn't already
   *       another entry loaded with the same name. */
  result_dirent->de_name[namelen] = 0;
  result_dirent->de_hash = directory_entry_hash(result_dirent->de_name,
                                                namelen);
  /* Search for an existing entry one last time. */
  if (open_mode & O_DOSPATH) {
   result = (REF struct regular_node *)directory_getcasenode(self,
                                                             result_dirent->de_name,
                                                             namelen,
                                                             result_dirent->de_hash);
  } else {
   result = (REF struct regular_node *)directory_getnode(self,
                                                         result_dirent->de_name,
                                                         namelen,
                                                         result_dirent->de_hash);
  }
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  kfree(result_dirent);
  error_rethrow();
 }
 if unlikely(result) {
  /* The thing already exists! */
  kfree(result_dirent);
  if (!(open_mode & O_EXCL))
        return (REF struct inode *)result;
  inode_decref((struct inode *)result);
  throw_fs_error(ERROR_FS_FILE_ALREADY_EXISTS);
 }
 /* All right. Now we've got our own, safe copy of the filename,
  *            and we've verified that the entry doesn't already
  *            exist.
  *            Now it's time to create the new file! */
 TRY {
  rwlock_write(&self->d_node.i_lock);
  TRY {
   /* ERROR: The directory doesn't support creation of files. */
   if unlikely(!self->d_node.i_ops->io_directory.d_creat)
      throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);

   /* Allocate a new INode destructor for the file. */
   result = (REF struct regular_node *)kmalloc(sizeof(struct regular_node),
                                               GFP_SHARED|GFP_CALLOC);
   /* Initialize the new INode */
   result->re_node.i_refcnt       = 1;
   result->re_node.i_super        = self->d_node.i_super;
   result->re_node.i_flags        = INODE_FATTRLOADED;
   result->re_node.i_attr.a_uid   = owner;
   result->re_node.i_attr.a_gid   = group;
   result->re_node.i_attr.a_mode  = S_IFREG | (mode & ~S_IFMT);
   rwlock_cinit(&result->re_node.i_lock);
   atomic_rwlock_cinit(&result->re_module.m_lock);
   superblock_incref(result->re_node.i_super);
   if (pentry) *pentry = NULL;
   TRY {
    /* Lookup the current WALL time for the new time. */
    result->re_node.i_attr.a_atime = wall_gettime(self->d_node.i_super->s_wall);
    result->re_node.i_attr.a_mtime = result->re_node.i_attr.a_atime;
    result->re_node.i_attr.a_ctime = result->re_node.i_attr.a_atime;
#ifndef NDEBUG
    memset(&result->re_node.i_attr.a_ino,0xcc,sizeof(ino_t));
    memset(&result_dirent->de_ino,0xdd,sizeof(ino_t));
#endif

    /* Create filesystem data for the new INode. */
    (*self->d_node.i_ops->io_directory.d_creat)(self,
                                                result_dirent,
                                                result);
    assert(result->re_node.i_ops != NULL);
    assert(result->re_node.i_nlink >= 1);
    assert(result->re_node.i_super == self->d_node.i_super);
    assert(result_dirent->de_ino == result->re_node.i_attr.a_ino);

    /* With the new INode now constructed, add it to the superblock. */
    superblock_addnode(self->d_node.i_super,(struct inode *)result);

    /* Now update our directory entry and add it to the directory node cache. */
    result_dirent->de_ino  = result->re_node.i_attr.a_ino;
    result_dirent->de_type = IFTODT(result->re_node.i_attr.a_mode); /* Shouldn't be different, but allow it to be. */
    if (pentry) {
     /* Pass a reference to the directory entry to the caller. */
     *pentry = result_dirent;
     directory_entry_incref(result_dirent);
    }
    /* assert(result->de_type != DT_WHT); // Actually, this is allowed... */
    directory_addentry(self,(struct directory_entry **)&result_dirent);
   } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
    if (pentry && *pentry)
        directory_entry_decref(*pentry);
    /* DECREF() the newly generated INode on error.
     * NOTE: Also unset the dir-loaded flag and rewind the
     *       directory load address, because now there is
     *       an unloaded directory entry still dangling
     *       within the directory. */
    self->d_dirend        = 0;
    self->d_node.i_flags &= ~(INODE_FDIRLOADED);
    inode_decref((struct inode *)result);
    if (error_code() == E_NO_DATA)
        error_info()->e_error.e_code = E_FILESYSTEM_ERROR,
        error_info()->e_error.e_code = ERROR_FS_DISK_FULL;
    error_rethrow();
   }
  } FINALLY {
   rwlock_endwrite(&self->d_node.i_lock);
  }
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  kfree(result_dirent);
  error_rethrow();
 }
 /* Return the newly constructed node. */
 return (REF struct inode *)result;
}


PUBLIC void KCALL
directory_remove(struct directory_node *__restrict EXCEPT_VAR self,
                 CHECKED USER char const *__restrict name,
                 u16 namelen, uintptr_t hash, unsigned int mode,
                 struct path *self_path) {
 REF struct inode *EXCEPT_VAR node;
 struct directory_entry *COMPILER_IGNORE_UNINITIALIZED(entry);
 struct directory_entry **pentry;
 rwlock_write(&self->d_node.i_lock);
 TRY {
  if unlikely(INODE_ISCLOSED(&self->d_node))
     throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
  entry = (mode & DIRECTORY_REMOVE_FNOCASE) ? directory_getcaseentry(self,name,namelen,hash)
                                            : directory_getentry(self,name,namelen,hash);
  if (!entry) throw_fs_error(ERROR_FS_FILE_NOT_FOUND);

  /* Ensure write permissions. */
  inode_access(&self->d_node,W_OK);
  pentry = &self->d_map[entry->de_hash & self->d_mask];
  while ((assert(*pentry),*pentry != entry)) pentry = &(*pentry)->de_next;
  if (entry->de_type == DT_WHT) {
   /* Special handling for virtual nodes. */
   if (!(mode&DIRECTORY_REMOVE_FVIRTUAL))
       throw_fs_error(ERROR_FS_REMOVE_MOUNTPOINT);
   /* Unlink the entry from the hash-map. */
   *pentry = entry->de_next;
   assert(self->d_size != 0);
   --self->d_size;
   /* Remove the entry from the by-position chain. */
   LIST_REMOVE(entry,de_bypos);
   /* Clear bypos pointers to not create
    * dangling pointers in directory streams. */
   entry->de_bypos.le_pself = NULL;
   entry->de_bypos.le_next  = NULL;
   /* With the virtual node now removed, we must re-load
    * the original entry associated with its location. */
   self->d_node.i_flags &= ~(INODE_FDIRLOADED);
   self->d_dirend        = 0;
  } else {
   node = superblock_opennode(self->d_node.i_super,self,entry);
   assert(node->i_nlink != 0);
   assert(node->i_super == self->d_node.i_super);
   TRY {
    if (INODE_ISDIR(node)) {
     struct directory_node *EXCEPT_VAR dir;
     if (!(mode&DIRECTORY_REMOVE_FDIRECTORY))
         throw_fs_error(ERROR_FS_UNLINK_DIRECTORY);
     dir = (struct directory_node *)node;
     if (dir->d_size != 0)
         throw_fs_error(ERROR_FS_DIRECTORY_NOT_EMPTY);
     /* If the directory hasn't been fully loaded,
      * read one entry to check that it's empty. */
     rwlock_write(&dir->d_node.i_lock);
     TRY {
      if (!(dir->d_node.i_flags&INODE_FDIRLOADED) &&
            directory_readnext(dir,IO_RDONLY) != NULL)
            throw_fs_error(ERROR_FS_DIRECTORY_NOT_EMPTY);
      /* The directory is empty except for its self and parent index.
       * Now to unlink it. */

      if (dir->d_node.i_nlink == 1) {
       /* Use the rmdir() operator. */
       if unlikely(!self->d_node.i_ops->io_directory.d_rmdir)
          throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
       (*self->d_node.i_ops->io_directory.d_rmdir)(self,entry,dir);
       assert(dir->d_node.i_nlink == 0);
      } else {
       /* Use the unlink() operator. */
       if unlikely(!self->d_node.i_ops->io_directory.d_unlink)
          throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
       (*self->d_node.i_ops->io_directory.d_unlink)(self,entry,(struct inode *)dir);
      }
     } FINALLY {
      rwlock_endwrite(&dir->d_node.i_lock);
     }
    } else {
     if (!self->d_node.i_ops->io_directory.d_unlink)
         throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
     /* Remove a regular file. (If this is the last link, truncate the file first) */
     rwlock_write(&node->i_lock);
     TRY {
      if (node->i_nlink == 1 &&
         (node->i_attr.a_size || node->i_attr.a_blocks)) {
       inode_access(node,W_OK);
       /* Deallocate file data of the node if this is the last link. */
       if unlikely(!node->i_ops->io_file.f_truncate)
          throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
       (*node->i_ops->io_file.f_truncate)(node,0);
       assert(node->i_attr.a_blocks == 0);
       node->i_attr.a_size = 0;
      }
      /* Do the actual unlink. */
#ifndef NDEBUG
      if (node->i_nlink == 1) {
       (*self->d_node.i_ops->io_directory.d_unlink)(self,entry,node);
       assert(node->i_nlink == 0);
      } else {
       (*self->d_node.i_ops->io_directory.d_unlink)(self,entry,node);
      }
#else
      (*self->d_node.i_ops->io_directory.d_unlink)(self,entry,node);
#endif
     } FINALLY {
      rwlock_endwrite(&node->i_lock);
     }
    }
   } FINALLY {
    inode_decref(node);
   }
   /* Unlink the entry from the hash-map. */
   *pentry = entry->de_next;
   assert(self->d_size >= 1);
   --self->d_size;
   /* Remove the entry from the by-position chain. */
   LIST_REMOVE(entry,de_bypos);
   /* Clear bypos pointers to not create
    * dangling pointers in directory streams. */
   entry->de_bypos.le_pself = NULL;
   entry->de_bypos.le_next  = NULL;
  }
  /* Remove the entry from the given path, too. */
  if (self_path)
      path_delchild(self_path,self,entry);
 } FINALLY {
  rwlock_endwrite(&self->d_node.i_lock);
 }
 /* Drop a reference from the directory entry we've just removed. */
 directory_entry_decref(entry);
}



PUBLIC void KCALL
directory_rename(struct directory_node *__restrict EXCEPT_VAR source_directory,
                 struct directory_entry *__restrict source_dirent,
                 struct directory_node *__restrict EXCEPT_VAR target_directory,
                 CHECKED USER char const *__restrict target_name,
                 u16 target_namelen) {
 REF struct directory_entry *target_dirent;
 bool inherit_source_dirent_reference = false;
 /* Disallow renaming of mounting points.
  * Technically we could do this, but then it would fall upon us to
  * deal with mounting permissions. And besides: I don't feel as though
  * mounting points should be something ~fragile~ enough to let you
  * just rename() to remount() them... ('Gotta be more compliate than that) */
 if unlikely(source_dirent->de_type == DT_WHT)
    throw_fs_error(ERROR_FS_RENAME_NOTAMOUNT);

 /* Check for cross-device links. */
 if unlikely(source_directory->d_node.i_super !=
             target_directory->d_node.i_super)
    throw_fs_error(ERROR_FS_CROSSDEVICE_LINK);
 if unlikely(!target_namelen)
    throw_fs_error(ERROR_FS_ILLEGAL_PATH);

 /* Construct a directory entry for the target filename. */
 target_dirent = (struct directory_entry *)kmalloc(offsetof(struct directory_entry,de_name)+
                                                 ((size_t)target_namelen+1)*sizeof(char),
                                                   GFP_SHARED);
 TRY {
  /* Fill in the new target directory entry. */
  target_dirent->de_refcnt  = 1;
  target_dirent->de_namelen = target_namelen;
  memcpy(target_dirent->de_name,target_name,
         target_namelen*sizeof(char));
  target_dirent->de_name[target_namelen] = 0;
  
  /* Generate the hash for the directory entry. */
  target_dirent->de_hash = directory_entry_hash(target_dirent->de_name,
                                                target_namelen);

  /* Acquire a write-lock to only the target directory at first. */
  rwlock_write(&target_directory->d_node.i_lock);
  TRY {
   struct directory_entry **psource_dirent;
   REF struct inode *EXCEPT_VAR source_node;
   if unlikely(INODE_ISCLOSED(&target_directory->d_node))
      throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
   /* Check if a directory entry matching the target name already exists. */
   if (directory_getentry(target_directory,
                          target_dirent->de_name,
                          target_namelen,
                          target_dirent->de_hash))
       throw_fs_error(ERROR_FS_FILE_ALREADY_EXISTS);
   inode_access(&target_directory->d_node,W_OK);

   rwlock_write(&source_directory->d_node.i_lock);
   TRY {
    /* Lookup the source directory entry pointer and ensure that it actually exists.
     * Between the time that the caller did the lookup for the source directory entry
     * and now (when we're holding a lock to `source_directory->d_node.i_lock'), the
     * directory that's supposed to be renamed may have been removed, meaning that it's
     * INO index could already point to another file, or just into the void.
     * But we can easily check this by looking for the hash-map pointer to source
     * directory entry (which we're going to need anyways as we'll have to remove
     * that entry upon success) */
    if unlikely(INODE_ISCLOSED(&source_directory->d_node))
       throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
    psource_dirent = &source_directory->d_map[source_dirent->de_hash & source_directory->d_mask];
    while ((*psource_dirent && *psource_dirent != source_dirent))
             psource_dirent = &(*psource_dirent)->de_next;
    if unlikely(!psource_dirent)
       throw_fs_error(ERROR_FS_FILE_NOT_FOUND);
    inode_access(&source_directory->d_node,W_OK);
    /* Lookup the source node. (NOTE: We've already checked for mounting points above) */
    source_node = superblock_opennode(source_directory->d_node.i_super,
                                      source_directory,
                                      source_dirent);
    TRY {
     assert(source_node->i_nlink != 0);
     /* Set the default target directory entry type according to the mode of the source node. */
     target_dirent->de_type = IFTODT(source_node->i_attr.a_mode);

#if 0
     /* Ensure that when moving a directory, the target directory
      * isn't located within the sub-tree of the source path. */
     if (S_ISDIR(source_node->i_attr.a_mode))
         check_inner_move((struct directory_node *)source_node,target_directory);
#endif

     if (source_directory->d_node.i_ops->io_directory.d_rename) {
      /* The source directory provides a rename() operator that we must use. */
#ifndef NDEBUG
      memset(&target_dirent->de_ino,0xcc,sizeof(ino_t));
#endif
      (*source_directory->d_node.i_ops->io_directory.d_rename)(source_directory,source_dirent,
                                                               target_directory,target_dirent,
                                                               source_node);
      /* NOTE: `d_rename()' will have filled in `de_pos' and `de_ino' in `target_dirent' */
     } else {
      if (!target_directory->d_node.i_ops->io_directory.d_link ||
          !source_directory->d_node.i_ops->io_directory.d_unlink)
           throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
      target_dirent->de_ino = source_node->i_attr.a_ino;

      rwlock_write(&source_node->i_lock);
      TRY {
       /* Without a rename, we can do a 2-step operation where we create
        * a new hardlink to the original file, before unlink()-ing it. */
       (*target_directory->d_node.i_ops->io_directory.d_link)(target_directory,
                                                              target_dirent,
                                                              source_node);
      } FINALLY {
       rwlock_endwrite(&source_node->i_lock);
      }
      /* Add the new target directory entry to the target directory. */
      directory_addentry(target_directory,&target_dirent);
      assert(source_node->i_nlink >= 2);
      (*source_directory->d_node.i_ops->io_directory.d_unlink)(source_directory,
                                                               source_dirent,
                                                               source_node);
      assert(source_node->i_nlink >= 1);
     }
     assert(source_directory->d_size >= 1);
     /* Remove the source node from the source directory. */
     *psource_dirent = source_dirent->de_next;
     --source_directory->d_size;
     LIST_REMOVE(source_dirent,de_bypos);
     /* Clear bypos pointers to not create
      * dangling pointers in directory streams. */
     source_dirent->de_bypos.le_pself = NULL;
     source_dirent->de_bypos.le_next  = NULL;
     /* Inherit the reference for the source directory entry. */
     inherit_source_dirent_reference = true;
    } FINALLY {
     inode_decref(source_node);
    }
   } FINALLY {
    rwlock_endwrite(&source_directory->d_node.i_lock);
   }
   /* Add the new target directory entry to the target directory (if we haven't already). */
   if (target_dirent)
       directory_addentry(target_directory,&target_dirent);
  } FINALLY {
   rwlock_endwrite(&target_directory->d_node.i_lock);
  }
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  if (inherit_source_dirent_reference)
      directory_entry_decref(source_dirent);
  kfree(target_dirent);
  if (error_code() == E_NO_DATA)
      error_info()->e_error.e_code = E_FILESYSTEM_ERROR,
      error_info()->e_error.e_code = ERROR_FS_DISK_FULL;
  error_rethrow();
 }
 /* Drop the inherited reference from the
  * source directory entry that got removed. */
 assert(inherit_source_dirent_reference);
 directory_entry_decref(source_dirent);
}

PUBLIC void KCALL
directory_link(struct directory_node *__restrict EXCEPT_VAR target_directory,
               CHECKED USER char const *__restrict target_name,
               u16 target_namelen, struct inode *__restrict EXCEPT_VAR link_target,
               bool ignore_casing) {
 REF struct directory_entry *EXCEPT_VAR target_dirent;
 /* Check for cross-device links. */
 if unlikely(link_target->i_super != target_directory->d_node.i_super)
    throw_fs_error(ERROR_FS_CROSSDEVICE_LINK);

 /* Check if the target directory even supports hardlinks. */
 if unlikely(!target_directory->d_node.i_ops->io_directory.d_link)
    throw_fs_error(ERROR_FS_UNSUPPORTED_FUNCTION);

 if unlikely(!target_namelen)
    throw_fs_error(ERROR_FS_ILLEGAL_PATH);

#if 0
 /* Ensure that when linking a directory, the target directory
  * isn't located within the sub-tree of the source path. */
 if (S_ISDIR(link_target->i_attr.a_mode))
     check_inner_move((struct directory_node *)link_target,target_directory);
#endif

 /* Construct a directory entry for the target filename. */
 target_dirent = (struct directory_entry *)kmalloc(offsetof(struct directory_entry,de_name)+
                                                 ((size_t)target_namelen+1)*sizeof(char),
                                                   GFP_SHARED);
 TRY { /* Required to guard against E_SEGFAULT from the memcpy() from userspace. */
  /* Fill in the new target directory entry. */
  target_dirent->de_refcnt  = 1;
  target_dirent->de_namelen = target_namelen;
  memcpy(target_dirent->de_name,target_name,
         target_namelen*sizeof(char));
  target_dirent->de_name[target_namelen] = 0;
  
  /* Generate the hash for the directory entry. */
  target_dirent->de_hash = directory_entry_hash(target_dirent->de_name,
                                                target_namelen);

  /* Acquire a write-lock to only the target directory at first. */
  rwlock_write(&target_directory->d_node.i_lock);
  TRY {
   if unlikely(INODE_ISCLOSED(&target_directory->d_node))
      throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
   /* Check if a directory entry matching the target name already exists. */
   if (ignore_casing ? directory_getcaseentry(target_directory,
                                              target_dirent->de_name,
                                              target_namelen,
                                              target_dirent->de_hash)
                     : directory_getentry(target_directory,
                                          target_dirent->de_name,
                                          target_namelen,
                                          target_dirent->de_hash))
       throw_fs_error(ERROR_FS_FILE_ALREADY_EXISTS);
   inode_access(&target_directory->d_node,W_OK);

   rwlock_write(&link_target->i_lock);
   TRY {
    target_dirent->de_ino  = link_target->i_attr.a_ino;
    target_dirent->de_type = IFTODT(link_target->i_attr.a_mode);
    (*target_directory->d_node.i_ops->io_directory.d_link)(target_directory,
                                                           target_dirent,
                                                           link_target);
    assert(target_dirent->de_ino == link_target->i_attr.a_ino);
   } FINALLY {
    rwlock_endwrite(&link_target->i_lock);
   }
   /* Add the new target directory entry to the target directory. */
   directory_addentry(target_directory,(struct directory_entry **)&target_dirent);
  } FINALLY {
   rwlock_endwrite(&target_directory->d_node.i_lock);
  }
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  kfree(target_dirent);
  if (error_code() == E_NO_DATA)
      error_info()->e_error.e_code = E_FILESYSTEM_ERROR,
      error_info()->e_error.e_code = ERROR_FS_DISK_FULL;
  error_rethrow();
 }
}

PUBLIC WUNUSED ATTR_RETNONNULL REF struct symlink_node *KCALL
directory_symlink(struct directory_node *__restrict EXCEPT_VAR target_directory,
                  CHECKED USER char const *__restrict target_name, u16 target_namelen,
                  CHECKED USER char const *__restrict link_text, size_t link_text_size,
                  uid_t owner, gid_t group, mode_t mode, bool ignore_casing) {
 REF struct directory_entry *EXCEPT_VAR target_dirent;
 REF struct symlink_node *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(link_node);
 /* Check if the target directory even supports symbolic links. */
 if unlikely(!target_directory->d_node.i_ops->io_directory.d_symlink)
    throw_fs_error(ERROR_FS_UNSUPPORTED_FUNCTION);
 if unlikely(!target_namelen)
    throw_fs_error(ERROR_FS_ILLEGAL_PATH);

 /* Construct a directory entry for the target filename. */
 target_dirent = (struct directory_entry *)kmalloc(offsetof(struct directory_entry,de_name)+
                                                 ((size_t)target_namelen+1)*sizeof(char),
                                                   GFP_SHARED);
 TRY { /* Required to guard against E_SEGFAULT from the memcpy() from userspace. */
  /* Fill in the new target directory entry. */
  target_dirent->de_refcnt  = 1;
  target_dirent->de_namelen = target_namelen;
  memcpy(target_dirent->de_name,target_name,
         target_namelen*sizeof(char));
  target_dirent->de_name[target_namelen] = 0;
  target_dirent->de_type = DT_LNK;
  
  /* Generate the hash for the directory entry. */
  target_dirent->de_hash = directory_entry_hash(target_dirent->de_name,
                                                target_namelen);
  COMPILER_WRITE_BARRIER();

  /* Construct the new link node that should be added. */
  link_node = (REF struct symlink_node *)kmalloc(sizeof(struct symlink_node),
                                                 GFP_SHARED|GFP_CALLOC);
  /* Initialize the new INode */
  link_node->sl_node.i_refcnt      = 1;
  link_node->sl_node.i_super       = target_directory->d_node.i_super;
  link_node->sl_node.i_flags       = INODE_FATTRLOADED;
  link_node->sl_node.i_attr.a_uid  = owner;
  link_node->sl_node.i_attr.a_gid  = group;
  link_node->sl_node.i_attr.a_mode = S_IFLNK | (mode & ~S_IFMT);
  link_node->sl_node.i_attr.a_size = link_text_size;
  rwlock_cinit(&link_node->sl_node.i_lock);
  superblock_incref(link_node->sl_node.i_super);

  TRY {
   /* Lookup the WALL time for the new symbolic link. */
   link_node->sl_node.i_attr.a_atime = wall_gettime(target_directory->d_node.i_super->s_wall);
   link_node->sl_node.i_attr.a_mtime = link_node->sl_node.i_attr.a_atime;
   link_node->sl_node.i_attr.a_ctime = link_node->sl_node.i_attr.a_atime;

   /* Copy the link text from user-space. */
   link_node->sl_text = (HOST char *)kmalloc((link_text_size+1)*sizeof(char),GFP_SHARED);
   memcpy(link_node->sl_text,link_text,link_text_size*sizeof(char));

   /* Acquire a write-lock to only the target directory at first. */
   rwlock_write(&target_directory->d_node.i_lock);
   TRY {
    if unlikely(INODE_ISCLOSED(&target_directory->d_node))
       throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
    /* Check if a directory entry matching the target name already exists. */
    if (ignore_casing ? directory_getcaseentry(target_directory,
                                               target_dirent->de_name,
                                               target_namelen,
                                               target_dirent->de_hash)
                      : directory_getentry(target_directory,
                                           target_dirent->de_name,
                                           target_namelen,
                                           target_dirent->de_hash))
        throw_fs_error(ERROR_FS_FILE_ALREADY_EXISTS);
    inode_access(&target_directory->d_node,W_OK);
#ifndef NDEBUG
    memset(&link_node->sl_node.i_attr.a_ino,0xcc,sizeof(ino_t));
    memset(&target_dirent->de_ino,0xdd,sizeof(ino_t));
#endif
    (*target_directory->d_node.i_ops->io_directory.d_symlink)(target_directory,
                                                              target_dirent,
                                                              link_node);
    assert(link_node->sl_node.i_ops   != NULL);
    assert(link_node->sl_node.i_nlink >= 1);
    assert(link_node->sl_node.i_super == target_directory->d_node.i_super);
    assert(target_dirent->de_ino == link_node->sl_node.i_attr.a_ino);

    /* Add the new target directory entry to the target directory. */
    directory_addentry(target_directory,(struct directory_entry **)&target_dirent);

    /* Add the new symlink INode to the associated superblock. */
    superblock_addnode(target_directory->d_node.i_super,
                      (struct inode *)link_node);
   } FINALLY {
    rwlock_endwrite(&target_directory->d_node.i_lock);
   }
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   inode_decref((struct inode *)link_node);
   if (error_code() == E_NO_DATA)
       error_info()->e_error.e_code = E_FILESYSTEM_ERROR,
       error_info()->e_error.e_code = ERROR_FS_DISK_FULL;
   error_rethrow();
  }
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  kfree(target_dirent);
  error_rethrow();
 }
 return link_node;
}


PUBLIC WUNUSED ATTR_RETNONNULL REF struct inode *KCALL
directory_mknod(struct directory_node *__restrict EXCEPT_VAR target_directory,
                CHECKED USER char const *__restrict target_name,
                u16 target_namelen, mode_t mode, uid_t owner,
                gid_t group, dev_t referenced_device, bool ignore_casing) {
 REF struct directory_entry *EXCEPT_VAR target_dirent;
 REF struct inode *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(device_node);
 if (S_ISREG(mode)) {
  /* Create a regular file when `mode' says so. */
  return directory_creatfile(target_directory,
                             target_name,
                             target_namelen,
                             ignore_casing ? O_EXCL|O_DOSPATH
                                           : O_EXCL,
                             owner,
                             group,
                             mode,
                             NULL);
 }
 /* XXX: We could easily add support for creating directories using this function... */
 assert(S_ISBLK(mode) || S_ISCHR(mode));

 /* Check if the target directory even supports symbolic links. */
 if unlikely(!target_directory->d_node.i_ops->io_directory.d_mknod)
    throw_fs_error(ERROR_FS_UNSUPPORTED_FUNCTION);
 if unlikely(!target_namelen)
    throw_fs_error(ERROR_FS_ILLEGAL_PATH);

 /* Construct a directory entry for the target filename. */
 target_dirent = (struct directory_entry *)kmalloc(offsetof(struct directory_entry,de_name)+
                                                 ((size_t)target_namelen+1)*sizeof(char),
                                                   GFP_SHARED);
 TRY { /* Required to guard against E_SEGFAULT from the memcpy() from userspace. */
  /* Fill in the new target directory entry. */
  target_dirent->de_refcnt  = 1;
  target_dirent->de_namelen = target_namelen;
  memcpy(target_dirent->de_name,target_name,
         target_namelen*sizeof(char));
  target_dirent->de_name[target_namelen] = 0;
  target_dirent->de_type = DT_LNK;
  
  /* Generate the hash for the directory entry. */
  target_dirent->de_hash = directory_entry_hash(target_dirent->de_name,
                                                target_namelen);
  COMPILER_WRITE_BARRIER();

  /* Construct the new link node that should be added. */
  device_node = (REF struct inode *)kmalloc(sizeof(struct inode),
                                            GFP_SHARED|GFP_CALLOC);
  /* Initialize the new INode */
  device_node->i_refcnt      = 1;
  device_node->i_super       = target_directory->d_node.i_super;
  device_node->i_flags       = INODE_FATTRLOADED;
  device_node->i_attr.a_uid  = owner;
  device_node->i_attr.a_gid  = group;
  device_node->i_attr.a_mode = mode;
  device_node->i_attr.a_rdev = referenced_device;
  rwlock_cinit(&device_node->i_lock);
  superblock_incref(device_node->i_super);

  TRY {
   /* Lookup the WALL time for the new symbolic link. */
   device_node->i_attr.a_atime = wall_gettime(target_directory->d_node.i_super->s_wall);
   device_node->i_attr.a_mtime = device_node->i_attr.a_atime;
   device_node->i_attr.a_ctime = device_node->i_attr.a_atime;

   /* Acquire a write-lock to only the target directory at first. */
   rwlock_write(&target_directory->d_node.i_lock);
   TRY {
    if unlikely(INODE_ISCLOSED(&target_directory->d_node))
       throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
    /* Check if a directory entry matching the target name already exists. */
    if (ignore_casing ? directory_getcaseentry(target_directory,
                                               target_dirent->de_name,
                                               target_namelen,
                                               target_dirent->de_hash)
                      : directory_getentry(target_directory,
                                           target_dirent->de_name,
                                           target_namelen,
                                           target_dirent->de_hash))
        throw_fs_error(ERROR_FS_FILE_ALREADY_EXISTS);
    inode_access(&target_directory->d_node,W_OK);
#ifndef NDEBUG
    memset(&device_node->i_attr.a_ino,0xcc,sizeof(ino_t));
    memset(&target_dirent->de_ino,0xdd,sizeof(ino_t));
#endif
    (*target_directory->d_node.i_ops->io_directory.d_mknod)(target_directory,
                                                            target_dirent,
                                                            device_node);
    assert(device_node->i_ops   != NULL);
    assert(device_node->i_nlink >= 1);
    assert(device_node->i_super == target_directory->d_node.i_super);
    assert(device_node->i_attr.a_mode == mode);
    assert(target_dirent->de_ino == device_node->i_attr.a_ino);

    /* Add the new target directory entry to the target directory. */
    directory_addentry(target_directory,(struct directory_entry **)&target_dirent);

    /* Add the new symlink INode to the associated superblock. */
    superblock_addnode(target_directory->d_node.i_super,
                      (struct inode *)device_node);
   } FINALLY {
    rwlock_endwrite(&target_directory->d_node.i_lock);
   }
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   inode_decref((struct inode *)device_node);
   if (error_code() == E_NO_DATA)
       error_info()->e_error.e_code = E_FILESYSTEM_ERROR,
       error_info()->e_error.e_code = ERROR_FS_DISK_FULL;
   error_rethrow();
  }
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  kfree(target_dirent);
  error_rethrow();
 }
 return device_node;
}



PUBLIC WUNUSED ATTR_RETNONNULL REF struct directory_node *
KCALL directory_node_alloc(void) {
 REF struct directory_node *EXCEPT_VAR result;
 result = (REF struct directory_node *)kmalloc(sizeof(struct directory_node),
                                               GFP_SHARED|GFP_CALLOC);
 /* Initialize the new INode */
 result->d_node.i_refcnt = 1;
 TRY {
  result->d_map = (REF struct directory_entry **)kmalloc((DIRECTORY_DEFAULT_MASK+1)*
                                                          sizeof(REF struct directory_entry *),
                                                          GFP_SHARED|GFP_CALLOC);
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  kfree(result);
  error_rethrow();
 }
 result->d_mask = DIRECTORY_DEFAULT_MASK;
 result->d_node.i_attr.a_mode = S_IFDIR;
 rwlock_cinit(&result->d_node.i_lock);
#ifndef NDEBUG
 /* Debug-initialize some must-have fields. */
 memset(&result->d_node.i_attr.a_ino,0xcc,sizeof(ino_t));
#endif
 return result;
}



PUBLIC REF struct directory_node *KCALL
directory_mkdir(struct directory_node *__restrict EXCEPT_VAR target_directory,
                CHECKED USER char const *__restrict target_name,
                u16 target_namelen, mode_t mode, uid_t owner,
                gid_t group, bool ignore_casing) {
 REF struct directory_entry *EXCEPT_VAR target_dirent;
 REF struct directory_node *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(dir_node);

 /* Check if the target directory even supports symbolic links. */
 if unlikely(!target_directory->d_node.i_ops->io_directory.d_mkdir)
    throw_fs_error(ERROR_FS_READONLY_FILESYSTEM);
 if unlikely(!target_namelen)
    throw_fs_error(ERROR_FS_ILLEGAL_PATH);

 /* Construct a directory entry for the target filename. */
 target_dirent = (struct directory_entry *)kmalloc(offsetof(struct directory_entry,de_name)+
                                                 ((size_t)target_namelen+1)*sizeof(char),
                                                   GFP_SHARED);
 TRY { /* Required to guard against E_SEGFAULT from the memcpy() from userspace. */
  /* Fill in the new target directory entry. */
  target_dirent->de_refcnt  = 1;
  target_dirent->de_namelen = target_namelen;
  memcpy(target_dirent->de_name,target_name,
         target_namelen*sizeof(char));
  target_dirent->de_name[target_namelen] = 0;
  target_dirent->de_type = DT_LNK;
  
  /* Generate the hash for the directory entry. */
  target_dirent->de_hash = directory_entry_hash(target_dirent->de_name,
                                                target_namelen);
  COMPILER_WRITE_BARRIER();

  /* Construct the new link node that should be added. */
  dir_node = directory_node_alloc();
  /* Initialize the new INode */
  dir_node->d_node.i_super       = target_directory->d_node.i_super;
  dir_node->d_node.i_attr.a_uid  = owner;
  dir_node->d_node.i_attr.a_gid  = group;
  dir_node->d_node.i_attr.a_mode = S_IFDIR | (mode & ~S_IFMT);
  dir_node->d_node.i_flags       = INODE_FATTRLOADED|INODE_FDIRLOADED;

  rwlock_cinit(&dir_node->d_node.i_lock);
  superblock_incref(dir_node->d_node.i_super);

  TRY {
   /* Lookup the WALL time for the new symbolic link. */
   dir_node->d_node.i_attr.a_atime = wall_gettime(target_directory->d_node.i_super->s_wall);
   dir_node->d_node.i_attr.a_mtime = dir_node->d_node.i_attr.a_atime;
   dir_node->d_node.i_attr.a_ctime = dir_node->d_node.i_attr.a_atime;

   /* Acquire a write-lock to only the target directory at first. */
   rwlock_write(&target_directory->d_node.i_lock);
   TRY {
    if unlikely(INODE_ISCLOSED(&target_directory->d_node))
       throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
    /* Check if a directory entry matching the target name already exists. */
    if (ignore_casing ? directory_getcaseentry(target_directory,
                                               target_dirent->de_name,
                                               target_namelen,
                                               target_dirent->de_hash)
                      : directory_getentry(target_directory,
                                           target_dirent->de_name,
                                           target_namelen,
                                           target_dirent->de_hash))
        throw_fs_error(ERROR_FS_FILE_ALREADY_EXISTS);
    inode_access(&target_directory->d_node,W_OK);
#ifndef NDEBUG
    memset(&dir_node->d_node.i_attr.a_ino,0xcc,sizeof(ino_t));
    memset(&target_dirent->de_ino,0xdd,sizeof(ino_t));
#endif
    (*target_directory->d_node.i_ops->io_directory.d_mkdir)(target_directory,
                                                            target_dirent,
                                                            dir_node);
    assert(dir_node->d_node.i_ops   != NULL);
    assert(dir_node->d_node.i_nlink >= 1);
    assert(dir_node->d_node.i_super == target_directory->d_node.i_super);
    assert(S_ISDIR(dir_node->d_node.i_attr.a_mode));
    assert(target_dirent->de_ino == dir_node->d_node.i_attr.a_ino);

    /* Add the new target directory entry to the target directory. */
    directory_addentry(target_directory,(struct directory_entry **)&target_dirent);

    /* Add the new symlink INode to the associated superblock. */
    superblock_addnode(target_directory->d_node.i_super,
                      (struct inode *)dir_node);
   } FINALLY {
    rwlock_endwrite(&target_directory->d_node.i_lock);
   }
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   inode_decref((struct inode *)dir_node);
   if (error_code() == E_NO_DATA)
       error_info()->e_error.e_code = E_FILESYSTEM_ERROR,
       error_info()->e_error.e_code = ERROR_FS_DISK_FULL;
   error_rethrow();
  }
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  kfree(target_dirent);
  error_rethrow();
 }
 return dir_node;
}






/* SUPERBLOCK API */
PUBLIC size_t KCALL
superblock_clearcache(struct superblock *__restrict self,
                      size_t max_nodes) {
 u16 flags = ATOMIC_READ(self->s_flags);
 size_t i,result = 0;
again:
 while (result < max_nodes) {
  REF struct inode *node;
  atomic_rwlock_write(&self->s_nodes_lock);
  if (!self->s_nodesc) {
   atomic_rwlock_endwrite(&self->s_nodes_lock);
   break;
  }
  for (i = 0; i <= self->s_nodesm; ++i) {
   node = self->s_nodesv[i];
   /* Don't just simply discard INodes that have been marked as CHANGED. */
   while (node &&
        ((node->i_flags&INODE_FCLOSED) ||                      /* Node is closed... */
        !(node->i_flags&(INODE_FCHANGED|INODE_FPERSISTENT)) || /* or non-persistent and hasn't changed... */
         (flags&SUPERBLOCK_FCLOSED)))                          /* or the superblock was closed */
          node = node->i_nodes.le_next;
   if (!node) continue;
   /* Remove this node from the cache.
    * NOTE: Only get rid of it if this is the last reference.
    *       Otherwise, we need to keep it around to ensure
    *       that future requests for this INode yield the
    *       same underlying descriptor.
    *       However, if the superblock was closed, then
    *       we can safely get rid of _all_ nodes.
    */
   if (!(flags&SUPERBLOCK_FCLOSED)) {
    if (!ATOMIC_DECIFONE(node->i_refcnt)) continue;
   }
   LIST_REMOVE(node,i_nodes);
   --self->s_nodesc;
   atomic_rwlock_endwrite(&self->s_nodes_lock);
   /* Drop a reference to this node. */
   node->i_nodes.le_pself = NULL;
   if (flags&SUPERBLOCK_FCLOSED) {
    inode_decref(node);
   } else {
    /* Decrement to ZERO(0) above. */
    inode_destroy(node);
   }
   ++result;
   goto again;
  }
  atomic_rwlock_endwrite(&self->s_nodes_lock);
  break;
 }
 return result;
}

PUBLIC void KCALL
superblock_sync(struct superblock *__restrict EXCEPT_VAR self) {
 while (ATOMIC_READ(self->s_changed)) {
  struct inode *EXCEPT_VAR node;
  atomic_rwlock_write(&self->s_changed_lock);
  COMPILER_READ_BARRIER();
  node = self->s_changed;
  if unlikely(!node) {
   atomic_rwlock_endwrite(&self->s_changed_lock);
   break;
  }
  LIST_REMOVE(node,i_changed);
  inode_incref(node);
  atomic_rwlock_endwrite(&self->s_changed_lock);
  /* Save attributes for this INode. */
  rwlock_write(&node->i_lock);
  TRY {
   if likely(node->i_flags & INODE_FCHANGED) {
    assert(node->i_ops->io_saveattr);
    (*node->i_ops->io_saveattr)(node);
    ATOMIC_FETCHAND(node->i_flags,INODE_FCHANGED);
   }
  } FINALLY {
   rwlock_endwrite(&node->i_lock);
   if (FINALLY_WILL_RETHROW) {
    /* Re-add the node to the set of changed INodes after an error. */
    atomic_rwlock_write(&self->s_changed_lock);
    LIST_INSERT(self->s_changed,node,i_changed);
    atomic_rwlock_endwrite(&self->s_changed_lock);
   }
   inode_decref(node);
  }
 }
 /* With all changed nodes now saved, invoke
  * the optional filesystem sync() operator. */
 if (self->s_type->st_functions.f_sync)
   (*self->s_type->st_functions.f_sync)(self);

 /* With the superblock itself synchronized,
  * now synchronize the underlying block-device. */
 block_device_sync(self->s_device);
}


struct blocklist {
    size_t              bl_alloc;  /* Allocated number of pointers. */
    size_t              bl_count;  /* Number of pointers in use. */
    struct superblock **bl_blocks; /* [?..?][0..bl_count|ALLOC(bl_alloc)][owned] Vector of pointers. */
};

PRIVATE void KCALL
blocklist_insert(struct blocklist EXCEPT_VAR *__restrict EXCEPT_VAR self,
                 struct superblock *__restrict item) {
 if (self->bl_alloc == self->bl_count) {
  size_t EXCEPT_VAR new_alloc = self->bl_alloc*2;
  if (!new_alloc) new_alloc = 1;
  TRY {
   self->bl_blocks = (struct superblock **)krealloc(self->bl_blocks,new_alloc*
                                                    sizeof(struct superblock *),
                                                    GFP_SHARED);
  } CATCH (E_BADALLOC) {
   new_alloc = self->bl_count+1;
   self->bl_blocks = (struct superblock **)krealloc(self->bl_blocks,new_alloc*
                                                    sizeof(struct superblock *),
                                                    GFP_SHARED);
  }
  self->bl_alloc = new_alloc;
 }
 self->bl_blocks[self->bl_count++] = item;
}
PRIVATE ATTR_NOTHROW bool KCALL
blocklist_contains(struct blocklist EXCEPT_VAR *__restrict self,
                   struct superblock *__restrict item) {
 size_t i;
 for (i = 0; i < self->bl_count; ++i)
     if (self->bl_blocks[i] == item)
         return true;
 return false;
}


PUBLIC void KCALL superblock_syncall(void) {
 struct superblock *EXCEPT_VAR iter;
 struct blocklist EXCEPT_VAR blocks = { 0, 0, NULL };
 TRY {
again:
  atomic_rwlock_read(&fs_filesystems.f_superlock);
  /* Enumerate all filesystems. */
  iter = fs_filesystems.f_superblocks;
  for (; iter; iter = iter->s_filesystems.le_next) {
   if (blocklist_contains(&blocks,iter)) continue;
   if (!ATOMIC_INCIFNONZERO(iter->s_refcnt)) continue;
   atomic_rwlock_endread(&fs_filesystems.f_superlock);
   TRY {
    /* Do the synchronization operation. */
    superblock_sync(iter);

#if 1 /* Optimization to not create a heap-allocation
       * when only a single superblock exists. */
    if (iter == ATOMIC_READ(fs_filesystems.f_superblocks)) {
     atomic_rwlock_read(&fs_filesystems.f_superlock);
     COMPILER_READ_BARRIER();
     if (iter == ATOMIC_READ(fs_filesystems.f_superblocks) &&
         iter->s_filesystems.le_next == NULL) {
      atomic_rwlock_endread(&fs_filesystems.f_superlock);
      superblock_decref(iter);
      goto done; /* XXX: Skip across FINALLY is intended. */
     }
     atomic_rwlock_endread(&fs_filesystems.f_superlock);
    }
#endif

    /* Add the superblock to the set of ones that were already synchronized. */
    blocklist_insert(&blocks,iter);
   } FINALLY {
    /* Drop a reference from the superblock in question. */
    superblock_decref(iter);
   }
   goto again;
  }
  atomic_rwlock_endread(&fs_filesystems.f_superlock);
done:;
 } FINALLY {
  kfree(blocks.bl_blocks);
 }
}



PRIVATE void KCALL
superblock_close_nodes(struct superblock *__restrict self) {
 size_t i;
 atomic_rwlock_read(&self->s_nodes_lock);
 if (self->s_nodesc) {
  assert(self->s_nodesv);
  /* Set the changed-flag of all nodes associated with the superblock. */
  for (i = 0; i <= self->s_nodesm; ++i) {
   struct inode *iter = self->s_nodesv[i];
   for (; iter; iter = iter->i_nodes.le_next)
       ATOMIC_FETCHOR(iter->i_flags,INODE_FCLOSED);
  }
 }
 atomic_rwlock_endread(&self->s_nodes_lock);
}

PUBLIC bool KCALL
superblock_close(struct superblock *__restrict self) {
 u16 old_flags;
 /* Remove the superblock as the active mounting object of the associated device. */
 atomic_rwlock_write(&self->s_device->b_fslock);
 if (self == self->s_device->b_filesystem) /* NOTE: This check deals with null_device. */
     self->s_device->b_filesystem = NULL;
 /* Set the closed-flag while we're still holding on to the device's superblock lock.
  * That way, the superblock disappearing will be
  * atomically consistent with it being marked as closed. */
 old_flags = ATOMIC_FETCHOR(self->s_flags,SUPERBLOCK_FCLOSED);
 atomic_rwlock_endwrite(&self->s_device->b_fslock);
 if (old_flags & SUPERBLOCK_FCLOSED)
     return false;
 superblock_close_nodes(self);
 return true;
}


PUBLIC bool KCALL
superblock_shutdown(struct superblock *__restrict self) {
 bool result = superblock_close(self);
 superblock_umount_all(self);
 superblock_sync(self);
 superblock_clearcache(self,(size_t)-1);
 return result;
}

PUBLIC size_t KCALL
superblock_shutdownall(void) {
 size_t result = 0;
 struct superblock *EXCEPT_VAR iter;
again:
 atomic_rwlock_read(&fs_filesystems.f_superlock);
 iter = fs_filesystems.f_superblocks;
 for (; iter; iter = iter->s_filesystems.le_next) {
  if (ATOMIC_FETCHOR(iter->s_flags,SUPERBLOCK_FCLOSED) & SUPERBLOCK_FCLOSED)
      continue; /* Already closed. */
  ++result;
  superblock_incref(iter);
  atomic_rwlock_endread(&fs_filesystems.f_superlock);
  /* Close this superblock. */
  TRY {
   superblock_close_nodes(iter);
   superblock_umount_all(iter);
   superblock_sync(iter);
   superblock_clearcache(iter,(size_t)-1);
  } FINALLY {
   superblock_decref(iter);
  }
  goto again;
 }
 atomic_rwlock_endread(&fs_filesystems.f_superlock);
 return result;
}



PUBLIC ATTR_RETNONNULL
REF struct superblock *KCALL superblock_alloc(void) {
 REF struct superblock *EXCEPT_VAR result;
 result = (REF struct superblock *)kmalloc(sizeof(struct superblock),
                                           GFP_SHARED|GFP_CALLOC);
 result->s_refcnt = 1;
 result->s_nodesm = SUPERBLOCK_DEFAULT_NODES_MASK;
 result->s_wall   = &wall_kernel;
 atomic_rwlock_cinit(&result->s_nodes_lock);
 atomic_rwlock_cinit(&result->s_changed_lock);
 atomic_rwlock_cinit(&result->s_lock);
 TRY {
  result->s_nodesv = (REF LIST_HEAD(struct inode) *)kmalloc((SUPERBLOCK_DEFAULT_NODES_MASK+1)*
                                                             sizeof(LIST_HEAD(struct inode)),
                                                             GFP_SHARED|GFP_CALLOC);
  TRY {
   result->s_root = directory_node_alloc();
   device_incref((struct device *)&wall_kernel);
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   kfree(result->s_nodesv);
   error_rethrow();
  }
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  kfree(result);
  error_rethrow();
 }
 /* Grant full permissions to access the filesystem root directory by default. */
 result->s_root->d_node.i_attr.a_mode |= 0777;
 result->s_root->d_node.i_super        = result;

 return result;
}


PUBLIC ATTR_NOTHROW void KCALL
superblock_destroy(struct superblock *__restrict self) {
 /* NOTE: Superfluous NULL-checks are done in case the
  *       superblock was only partially constructed. */
 if (self->s_type && self->s_type->st_functions.f_fini)
    (*self->s_type->st_functions.f_fini)(self);
 /* Unbind the superblock from its device. */
 if (self->s_device) {
  atomic_rwlock_write(&self->s_device->b_fslock);
  if (self->s_device->b_filesystem == self)
      self->s_device->b_filesystem = NULL;
  atomic_rwlock_endwrite(&self->s_device->b_fslock);
 }
 /* Remove from the list of known filesystems. */
 if (self->s_filesystems.le_pself) {
  atomic_rwlock_write(&fs_filesystems.f_superlock);
  COMPILER_READ_BARRIER();
  if (self->s_filesystems.le_pself)
      LIST_REMOVE(self,s_filesystems);
  atomic_rwlock_endwrite(&fs_filesystems.f_superlock);
 }
 /* Drop owned references. */
 if (self->s_root)
     inode_decref((struct inode *)self->s_root);
 if (self->s_device)
     block_device_decref(self->s_device);
 if (self->s_driver)
     driver_decref(self->s_driver);
 if (self->s_wall)
     device_decref((struct device *)self->s_wall);
 /* Free buffers and the superblock itself. */
 kfree(self->s_nodesv);
 kfree(self);
}

PUBLIC void KCALL
superblock_load(struct superblock *__restrict self,
                UNCHECKED USER char *args) {
 assert(self->s_type);
 assert(self->s_device);
 assert(self->s_driver);
 assert(self->s_driver == self->s_type->st_driver);
 assert(self->s_wall);
 assert(self->s_root);
 assert(self->s_root->d_node.i_super == self);
 assert(self->s_refcnt >= 1);
 assert(self->s_nodesc == 0);
 assert(self->s_filesystems.le_pself == NULL);
 assert(self->s_filesystems.le_next == NULL);
 assert(!(self->s_type->st_flags & SUPERBLOCK_TYPE_FSINGLE));

 /* Invoke the open operator. */
 assert(self->s_type->st_open);
 TRY {
  (*self->s_type->st_open)(self,args);
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  except_t code = error_code();
  if (code == E_NO_DATA || code == E_INDEX_ERROR) {
   /* Translate corrupt-filesystem errors. */
   error_info()->e_error.e_code                        = E_FILESYSTEM_ERROR;
   error_info()->e_error.e_filesystem_error.fs_errcode = ERROR_FS_CORRUPTED_FILESYSTEM;
  }
  error_rethrow();
 }

 assert(self->s_root->d_node.i_ops != NULL);
 assert(self->s_root->d_node.i_nlink >= 1);

 /* Add the filesystem root node to the nodes tracked by the superblock. */
 superblock_addnode(self,(struct inode *)self->s_root);

 /* Add the filesystem to the chain of known filesystems. */
 superblock_register(self);
}


PUBLIC REF struct superblock *KCALL
superblock_open(struct block_device *__restrict EXCEPT_VAR device,
                struct superblock_type *EXCEPT_VAR type,
                UNCHECKED USER char *args) {
 REF struct superblock *EXCEPT_VAR result;
again:
 atomic_rwlock_read(&device->b_fslock);
 result = device->b_filesystem;
 if (result) {
  /* Device has already been opened. */
  if (!type || result->s_type == type) {
   superblock_incref(result);
   atomic_rwlock_endread(&device->b_fslock);
   TRY {
    /* Wait until the superblock has been loaded. */
    while (!SUPERBLOCK_LOADED(result)) {
     /* If the other thread trying to load the superblock
      * failed to do so, loop back and try again ourself. */
     if (ATOMIC_READ(device->b_filesystem) != result) {
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
  /* Different type. */
  atomic_rwlock_endread(&device->b_fslock);
  throw_fs_error(ERROR_FS_CORRUPTED_FILESYSTEM);
 }
 atomic_rwlock_endread(&device->b_fslock);
 /* Create a new filesystem. */
 if (!type) {
  /* Try all known types. */
  atomic_rwlock_read(&fs_filesystem_types.ft_typelock);
  type = fs_filesystem_types.ft_types;
  for (; type; type = type->st_chain.le_next) {
   REF struct driver *d;
   /* Device-less superblocks must be loaded explicitly. */
   if (type->st_flags & (SUPERBLOCK_TYPE_FNODEV|
                         SUPERBLOCK_TYPE_FSINGLE))
       continue;
   /* Save a reference to the driver in question.
    * Because drivers are not allowed to call `delete_filesystem_type()',
    * we can assume that a filesystem type will remain valid if we can
    * get a reference to the driver implementing it.
    * NOTE: Filesystem types registered by some driver are automatically
    *       deleted when the driver application's reference counter hits
    *       ZERO. */
   d = type->st_driver;
   COMPILER_READ_BARRIER();
   if (d->d_app.a_flags & APPLICATION_FCLOSING) continue;
   if (!driver_tryincref(d)) continue;
   atomic_rwlock_endread(&fs_filesystem_types.ft_typelock);
   /* Try this type. */
   TRY {
    result = superblock_open(device,type,args);
    driver_decref(d);
    return result;
   } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
    driver_decref(d);
    /* Propagate anything that isn't a corrupt-filesystem error. */
    if (error_code()                                != E_FILESYSTEM_ERROR ||
        error_info()->e_error.e_filesystem_error.fs_errcode != ERROR_FS_CORRUPTED_FILESYSTEM)
        error_rethrow();
   }
   atomic_rwlock_read(&fs_filesystem_types.ft_typelock);
  }
  atomic_rwlock_endread(&fs_filesystem_types.ft_typelock);
 }
 /* Deal with singleton superblocks */
 if (type->st_flags & SUPERBLOCK_TYPE_FSINGLE) {
  result = type->st_singleton;
  assert(result);
  assert(result->s_refcnt != 0);
  superblock_incref(result);
  return result;
 }
 /* Try to construct and open a superblock. */
 result = superblock_alloc();
 /* Get a reference to the driver implementing this type.
  * NOTE: This shouldn't be able to fail, unless the
  *       type has been allocated dynamically? */
 if unlikely((type->st_driver->d_app.a_flags & APPLICATION_FCLOSING) ||
             !driver_tryincref(type->st_driver)) {
  superblock_destroy(result);
  throw_fs_error(ERROR_FS_CORRUPTED_FILESYSTEM);
 }
 block_device_incref(device);
 result->s_type   = type;
 result->s_driver = type->st_driver; /* Inherit reference. */
 result->s_device = device;          /* Inherit reference. */

 /* Allow any number of superblocks to be bound to `null_device' */
 if (device != &null_device) {
  /* Set the superblock as active for the device. */
  atomic_rwlock_write(&device->b_fslock);
  /* Check if another superblock has appeared in the mean time. */
  if unlikely(device->b_filesystem) {
   atomic_rwlock_endwrite(&device->b_fslock);
   superblock_decref(result);
   goto again;
  }

  /* Set the resulting filesystem for this device. */
  device->b_filesystem = result;
  atomic_rwlock_endwrite(&device->b_fslock);
 }
 TRY {
  /* Actually load the superblock!
   * NOTE: Upon success, this function also registers
   *       the superblock as a globally known filesystem. */
  superblock_load(result,args);
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  /* Remove the filesystem from the block-device
   * after initialization failed. */
  if (device != &null_device) {
   atomic_rwlock_write(&device->b_fslock);
   assert(device->b_filesystem == result);
   device->b_filesystem = NULL;
   atomic_rwlock_endwrite(&device->b_fslock);
  }
  superblock_decref(result);
  error_rethrow();
 }

 /* Done! */
 return result;
}



PUBLIC ATTR_RETNONNULL REF struct inode *KCALL
superblock_opennode(struct superblock *__restrict self,
                    struct directory_node *__restrict parent_directory,
                    struct directory_entry *__restrict parent_directory_entry) {
 REF struct inode *EXCEPT_VAR result,*new_result,**pbucket;
 uintptr_t ino_hash = INO_HASH(parent_directory_entry->de_ino);
 atomic_rwlock_read(&self->s_nodes_lock);
 if unlikely(self->s_flags & SUPERBLOCK_FCLOSED) {
  atomic_rwlock_endread(&self->s_nodes_lock);
  throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
 }
 /* Search for an existing INode. */
 assert(self->s_nodesv);
 result = self->s_nodesv[ino_hash & self->s_nodesm];
 for (; result; result = result->i_nodes.le_next) {
  assert(result->i_super == self);
  if (result->i_attr.a_ino == parent_directory_entry->de_ino) {
   /* Found it! */
   assert(result->i_refcnt != 0);
   inode_incref(result);
   atomic_rwlock_endread(&self->s_nodes_lock);
   return result;
  }
 }
 atomic_rwlock_endread(&self->s_nodes_lock);

 /* Node wasn't in cache. Allocate a new descriptor. */
 switch (parent_directory_entry->de_type) {

 case DT_REG:
  result = (REF struct inode *)kmalloc(sizeof(struct regular_node),
                                       GFP_SHARED|GFP_CALLOC);
  break;

 case DT_LNK:
  result = (REF struct inode *)kmalloc(sizeof(struct symlink_node),
                                       GFP_SHARED|GFP_CALLOC);
  break;

 {
  REF struct directory_node *EXCEPT_VAR me;
 case DT_DIR:
  me = (REF struct directory_node *)kmalloc(sizeof(struct directory_node),
                                            GFP_SHARED|GFP_CALLOC);
  TRY {
   me->d_map = (REF struct directory_entry **)kmalloc((DIRECTORY_DEFAULT_MASK+1)*
                                                       sizeof(REF struct directory_entry *),
                                                       GFP_SHARED|GFP_CALLOC);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   kfree(me);
   error_rethrow();
  }
  me->d_mask = DIRECTORY_DEFAULT_MASK;
  result = (REF struct inode *)me;
 } break;

 default:
  break;

 }

 /* Initialize common INode members. */
 result->i_refcnt      = 1;
 result->i_super       = self;
 result->i_attr.a_ino  = parent_directory_entry->de_ino;
 result->i_attr.a_mode = DTTOIF(parent_directory_entry->de_type);
 rwlock_cinit(&result->i_lock);
 superblock_incref(self);

 TRY {
  /* Invoke the open-node operator. */
  (*self->s_type->st_functions.f_opennode)(result,
                                           parent_directory,
                                           parent_directory_entry);
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  assert(result->i_refcnt == 1);
  inode_destroy(result);
  error_rethrow();
 }
 assertf(result->i_ops != NULL,"`f_opennode' must fill in `i_ops'");
 assert(result->i_attr.a_ino == parent_directory_entry->de_ino);

 /* Insert the new INode into the hash-map, unless
  * its already been loaded in the mean time. */
 atomic_rwlock_write(&self->s_nodes_lock);
 pbucket = &self->s_nodesv[ino_hash & self->s_nodesm];
 new_result = *pbucket;
 for (; new_result; new_result = new_result->i_nodes.le_next) {
  assert(new_result->i_super == self);
  if unlikely(new_result->i_attr.a_ino == parent_directory_entry->de_ino) {
   /* Another thread was faster. */
   inode_incref(new_result);
   atomic_rwlock_endwrite(&self->s_nodes_lock);
   /* Destroy the newly constructed node and return the existing one. */
   inode_decref(result);
   return new_result;
  }
 }

 if unlikely(result->i_flags & INODE_FCHANGED) {
  assert(result->i_ops->io_saveattr);
  /* Add the node the chain of changed nodes. */
  atomic_rwlock_write(&self->s_changed_lock);
  LIST_INSERT(self->s_changed,result,i_changed);
  atomic_rwlock_endwrite(&self->s_changed_lock);
 }

 /* Insert the new node into the hash-map. */
 LIST_INSERT(*pbucket,result,i_nodes);
 assert(result->i_refcnt == 1);
 inode_incref(result); /* The reference stored in the bucket chain. */

 if (++self->s_nodesc >= (self->s_nodesm/3)*2) {
  TRY {
   superblock_rehash_and_unlock(self);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* Second handler is required because of interrupt exceptions. */
   inode_decref(result);
   error_rethrow();
  }
 } else {
  atomic_rwlock_endwrite(&self->s_nodes_lock);
 }
 return result;
}



INTERN void KCALL
delete_filesystem_type(struct superblock_type *__restrict type) {
 atomic_rwlock_write(&fs_filesystem_types.ft_typelock);
 LIST_REMOVE(type,st_chain);
 type->st_chain.le_pself = NULL;
 type->st_chain.le_next = NULL;
 atomic_rwlock_endwrite(&fs_filesystem_types.ft_typelock);
 driver_decref(type->st_driver);
}
PUBLIC void KCALL
register_filesystem_type(struct superblock_type *__restrict type) {
 assertf(type->st_driver,"No owner defined");
 driver_incref(type->st_driver);
 atomic_rwlock_write(&fs_filesystem_types.ft_typelock);
 LIST_INSERT(fs_filesystem_types.ft_types,type,st_chain);
 atomic_rwlock_endwrite(&fs_filesystem_types.ft_typelock);
}

PUBLIC REF struct superblock_type *KCALL
lookup_filesystem_type(USER CHECKED char const *name) {
 REF struct superblock_type *result;
 char kernel_name[SUPERBLOCK_TYPE_NAME_MAX];
 size_t namelen = user_strlen(name);
 if unlikely(namelen >= SUPERBLOCK_TYPE_NAME_MAX)
    return NULL;
 memcpy(kernel_name,name,(namelen+1)*sizeof(char));
 atomic_rwlock_read(&fs_filesystem_types.ft_typelock);
 result = fs_filesystem_types.ft_types;
 for (; result; result = result->st_chain.le_next) {
  if (strcmp(result->st_name,kernel_name) != 0) continue;
  if (result->st_driver->d_app.a_flags & APPLICATION_FCLOSING) continue;
  if (!driver_tryincref(result->st_driver)) continue;
  break;
 }
 atomic_rwlock_endread(&fs_filesystem_types.ft_typelock);
 return result;
}


PUBLIC struct filesystem_types fs_filesystem_types = {
    .ft_typelock = ATOMIC_RWLOCK_INIT,
    .ft_types    = NULL
};
PUBLIC struct filesystems fs_filesystems = {
    .f_superlock   = ATOMIC_RWLOCK_INIT,
    .f_superblocks = NULL
};



/* INode handle operators. */
DEFINE_INTERN_ALIAS(handle_inode_pread,inode_read);
DEFINE_INTERN_ALIAS(handle_inode_pwrite,inode_write);
DEFINE_INTERN_ALIAS(handle_inode_truncate,inode_truncate);
DEFINE_INTERN_ALIAS(handle_inode_ioctl,inode_ioctl);
DEFINE_INTERN_ALIAS(handle_inode_sync,inode_sync);
DEFINE_INTERN_ALIAS(handle_inode_stat,inode_stat);
DEFINE_INTERN_ALIAS(handle_inode_poll,inode_poll);


/* Directory entry handle operators. */
PUBLIC void KCALL
handle_directory_entry_stat(struct directory_entry *__restrict self,
                            USER CHECKED struct stat64 *result) {
 memset(result,0,sizeof(struct stat64));
 result->st_mode = DTTOIF(self->de_type);
 result->st_size = self->de_namelen;
 result->st_ino  = self->de_ino;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_NODE_C */
