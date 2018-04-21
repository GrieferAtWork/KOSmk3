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
#ifndef GUARD_KERNEL_SRC_FS_FILE_C
#define GUARD_KERNEL_SRC_FS_FILE_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/timespec.h>
#include <kernel/malloc.h>
#include <kernel/sections.h>
#include <kernel/debug.h>
#include <kos/safecall.h>
#include <fs/path.h>
#include <dev/wall.h>
#include <fs/node.h>
#include <fs/file.h>
#include <except.h>
#include <string.h>
#include <fcntl.h>

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



/* Destroy a previously allocated File. */
PUBLIC ATTR_NOTHROW void KCALL
file_destroy(struct file *__restrict self) {
 /* 1..1 members may be NULL if the file was only partially constructed. */
 if (self->f_ops && self->f_ops->f_fini)
    (*self->f_ops->f_fini)(self);
 if (self->f_node) inode_decref(self->f_node);
 if (self->f_path) path_decref(self->f_path);
 kfree(self);
}

PRIVATE void KCALL
StandardFile_Fini(struct file *__restrict self) {
 if (self->f_directory.d_curent)
     directory_entry_decref(self->f_directory.d_curent);
}

PRIVATE size_t KCALL
StandardFile_Read(struct file *__restrict self,
                  USER CHECKED void *buf,
                  size_t bufsize, iomode_t flags) {
 size_t result; pos_t pos;
 do {
  pos = ATOMIC_READ(self->f_pos);
  result = inode_read(self->f_node,buf,bufsize,pos,flags);
 } while (!ATOMIC_CMPXCH(self->f_pos,pos,pos+result));
 return result;
}

PRIVATE size_t KCALL
StandardFile_Write(struct file *__restrict self,
                   USER CHECKED void const *buf,
                   size_t bufsize, iomode_t flags) {
 size_t result;
 result = inode_write(self->f_node,buf,bufsize,self->f_pos,flags);
 self->f_pos += result;
 return result;
}

PRIVATE ssize_t KCALL
StandardFile_Ioctl(struct file *__restrict self, unsigned long cmd,
                   USER UNCHECKED void *arg, iomode_t flags) {
 return inode_ioctl(self->f_node,cmd,arg,flags);
}

PRIVATE unsigned int KCALL
StandardFile_Poll(struct file *__restrict self,
                  unsigned int mode) {
 return inode_poll(self->f_node,mode);
}

PRIVATE pos_t KCALL
StandardFile_Seek(struct file *__restrict self,
                  off_t offset, int whence) {
 pos_t result,oldpos;
 do {
  oldpos = ATOMIC_READ(self->f_pos);
  switch (whence) {

  case SEEK_SET:
   result = (pos_t)offset;
   break;

  case SEEK_CUR:
   if unlikely(__builtin_add_overflow(oldpos,offset,&result))
      throw_fs_error(ERROR_FS_NEGATIVE_SEEK);
   break;

  case SEEK_END:
   /* NOTE: I always though one was supposed to subtract the offset from
    *       this position, though as it turns out, the user must provide
    *       a negative offset if they want to seek into the file, offset
    *       from its end... */
   if unlikely(__builtin_add_overflow(self->f_node->i_attr.a_size,offset,&result))
      throw_fs_error(ERROR_FS_NEGATIVE_SEEK);
   break;

  default:
   error_throw(E_INVALID_ARGUMENT);
  }
  /* Atomically write the new position. */
 } while (!ATOMIC_CMPXCH_WEAK(self->f_pos,oldpos,result));
 return result;
}


INTERN size_t KCALL
StandardFile_ReadDir(struct file *__restrict self,
                     USER CHECKED struct dirent *buf,
                     size_t bufsize, int mode, iomode_t flags) {
 struct directory_entry *entry,*orig_entry;
 size_t result; pos_t dirpos,req_index,entry_pos;
 if unlikely(!INODE_ISDIR(self->f_node))
    throw_fs_error(ERROR_FS_NOT_A_DIRECTORY);
again:
 for (;;) {
  dirpos = ATOMIC_READ(self->f_pos);
  if (dirpos < 2) {
   if (mode & READDIR_SKIPREL) {
    /* Skip special entries. */
    ATOMIC_CMPXCH_WEAK(self->f_pos,dirpos,2);
    continue;
   }
   /* Emit special entries. */
   COMPILER_BARRIER();
   req_index = dirpos+1;
   result = (offsetof(struct dirent,d_name)+
             2*sizeof(char)); /* 2: `.\0' */
   if (dirpos == 1) ++result; /* The second `.' of `..' */
   if (bufsize >= offsetof(struct dirent,d_name)) {
    u16 namelen; char name[3] = {'.',0,0};
    namelen = 1;
    if (dirpos == 1) ++namelen,name[1] = '.';
    /* Fill in basic members of the user-buffer.
     * CAUTION: E_SEGFAULT */
    if (dirpos == 0) {
     buf->d_ino = self->f_node->i_attr.a_ino; /* Self */
    } else {
     ino_t parent_node;
     struct path *parent = self->f_path->p_parent;
     if (!parent || self->f_path == THIS_FS->fs_root) {
      /* No parent, or parent is hidden from view.
       * >> Skip the parent directory. */
      assert(dirpos    == 1);
      assert(req_index == 2);
      ATOMIC_CMPXCH(self->f_pos,dirpos,req_index);
      goto again;
     }
     /*  Load the INode number of the parent directory. */
     atomic_rwlock_read(&parent->p_lock);
     parent_node = parent->p_node->i_attr.a_ino;
     atomic_rwlock_endread(&parent->p_lock);
     /* Barrier to prevent the dangerous
      * write that follows from being moved. */
     COMPILER_BARRIER();
     buf->d_ino = parent_node;
    }
    /* Save other fields. */
    buf->d_namlen = namelen;
    buf->d_type   = DT_DIR;
    bufsize      -= offsetof(struct dirent,d_name);
    if (bufsize >= (size_t)(namelen+1))
     bufsize = (size_t)(namelen+1);
    else if ((mode & READDIR_MODEMASK) == READDIR_DEFAULT) {
     req_index = dirpos;
    }
    /* Copy the name to user-space.
     * CAUTION: E_SEGFAULT */
    memcpy(buf->d_name,name,bufsize*sizeof(char));
    COMPILER_WRITE_BARRIER();
    if ((mode & READDIR_MODEMASK) == READDIR_PEEK)
        req_index = dirpos;
    COMPILER_WRITE_BARRIER();
   } else {
    if ((mode & READDIR_MODEMASK) != READDIR_CONTINUE)
        req_index = dirpos;
   }
   if (!ATOMIC_CMPXCH_WEAK(self->f_pos,dirpos,dirpos+1))
        continue;
   break;
  }
  req_index = dirpos-2;

  /* Read the current directory stream state. */
  atomic_rwlock_read(&self->f_directory.d_curlck);
  entry_pos  = self->f_directory.d_curidx;
  entry      = self->f_directory.d_curent;
  atomic_rwlock_endread(&self->f_directory.d_curlck);
  orig_entry = entry;

  if (!entry) {
read_entry_pos_0:
   assert(entry_pos == 0);
   entry = ((struct directory_node *)self->f_node)->d_bypos;
   /* Load the initial directory entry. */
   if (!entry) {
    entry = directory_readnext((struct directory_node *)self->f_node,flags);
    if (!entry) return 0; /* Empty directory */
    /* Make sure that this is the initial and only directory entry.
     * NOTE: Because the caller is holding a read-lock, we know that
     *       the `directory_readnext()' we just did must have created
     *       the first and only entry (see `E_RETRY_RWLOCK_') */
    assert(entry == ((struct directory_node *)self->f_node)->d_bypos &&
           entry == ((struct directory_node *)self->f_node)->d_bypos_end);
   }
  }
  /* Load the effective directory entry. */
  while (req_index != entry_pos) {
   if (req_index > entry_pos) {
    /* Advance forward. */
    if (!entry->de_bypos.le_next) {
     struct directory_node *dir;
     /* No successor.
      * Either we're on a dangling entry, the directory has ended,
      * or the remainder of the directory hasn't been loaded yet. */
     dir = (struct directory_node *)self->f_node;
     assert((dir->d_bypos != NULL) ==
            (dir->d_bypos_end != NULL));
     if (entry != dir->d_bypos_end) {
      /* Danging entry (rewind to find the proper position). */
#ifndef NDEBUG
      entry = dir->d_bypos;
      if (entry) {
       while (entry->de_bypos.le_next)
           entry = entry->de_bypos.le_next;
      }
      assertf(entry == dir->d_bypos_end,
              "Wrong directory end (%p != %p)",
              dir->d_bypos_end,entry);
#endif
      entry_pos = 0;
      goto read_entry_pos_0;
     }
     /* Last entry (either end of stream, or directory isn't fully loaded) */
     if (dir->d_node.i_flags & INODE_FDIRLOADED)
         return 0; /* End of directory... */
     /* Keep reading more entries. */
     entry = directory_readnext(dir,flags);
     if (!entry) return 0; /* End of directory (we just didn't know, yet) */
     assert(entry == dir->d_bypos_end);
    } else {
     /* Read the next entry that has already been cached. */
     entry = entry->de_bypos.le_next;
    }
    ++entry_pos;
    continue;
   }
   /* Move backwards through the directory. */
   assert(req_index < entry_pos);
   if (req_index <= (entry_pos-req_index)) {
    /* Quick solution for rewinding, or seeking close to the start. */
    entry     = ((struct directory_node *)self->f_node)->d_bypos;
    entry_pos = 0;
    if unlikely(!entry) goto read_entry_pos_0;
    continue;
   }
   assert(req_index < entry_pos);
   /* Move backwards one-at-a-time. */
   if unlikely(entry == ((struct directory_node *)self->f_node)->d_bypos) {
    /* Our directory index broken when files were added/removed.
     * We're not on the first entry, so set continue searching
     * for the one that the user requested. */
    entry_pos = 0;
    continue;
   }
   assert(req_index < entry_pos);
   /* Move backwards by one. */
   entry = COMPILER_CONTAINER_OF(entry->de_bypos.le_pself,
                                 struct directory_entry,
                                 de_bypos.le_next);
   --entry_pos;
  }

  /* Exchange the current entry. */
  atomic_rwlock_write(&self->f_directory.d_curlck);
  if unlikely(self->f_directory.d_curent != orig_entry) {
   /* Something changed (try again) */
   atomic_rwlock_endwrite(&self->f_directory.d_curlck);
   goto again;
  }
  directory_entry_incref(entry);
  /* Override the current position. */
  self->f_directory.d_curidx = req_index;
  self->f_directory.d_curent = entry;
  atomic_rwlock_endwrite(&self->f_directory.d_curlck);
  /* Drop a reference from the original entry. */
  if (orig_entry) directory_entry_decref(orig_entry);

  /* Emit the directory entry `entry' */
  COMPILER_BARRIER();
  req_index = dirpos+1;
  result = (offsetof(struct dirent,d_name)+
           (entry->de_namelen+1)*sizeof(char));
  if (bufsize >= offsetof(struct dirent,d_name)) {
   /* Fill in basic members of the user-buffer.
    * CAUTION: E_SEGFAULT */
   if unlikely(entry->de_type == DT_WHT) {
    /* Fixed mounting point. */
    struct inode *virt = entry->de_virtual;
    buf->d_ino  = virt->i_attr.a_ino;
    buf->d_type = IFTODT(virt->i_attr.a_mode);
   } else {
    /* Regular directory entry. */
    buf->d_ino  = entry->de_ino;
    buf->d_type = entry->de_type;
   }
   buf->d_namlen = entry->de_namelen;
   bufsize -= offsetof(struct dirent,d_name);
   if (bufsize >= (size_t)(entry->de_namelen+1))
    bufsize = (size_t)(entry->de_namelen+1);
   else if ((mode & READDIR_MODEMASK) == READDIR_DEFAULT) {
    req_index = dirpos;
   }
   /* Copy the entry name to user-space.
    * CAUTION: E_SEGFAULT */
   memcpy(buf->d_name,entry->de_name,
          bufsize*sizeof(char));
   COMPILER_WRITE_BARRIER();
   if ((mode & READDIR_MODEMASK) == READDIR_PEEK)
       req_index = dirpos;
  } else {
   if ((mode & READDIR_MODEMASK) != READDIR_CONTINUE)
       req_index = dirpos;
  }
  /* Save the new stream position.
   * NOTE: This would be allowed to be a weak access.
   *       However, after so much work, let's put some
   *       effort into confirming that we've copied data. */
  if (!ATOMIC_CMPXCH(self->f_pos,dirpos,req_index))
       continue;

  break;
 }
 return result;
}



/* Default file operations implementing read/write
 * operations using `f_pread' and `f_pwrite' */
PUBLIC struct file_operations default_file_operations = {
    .f_fini    = &StandardFile_Fini,
    .f_read    = &StandardFile_Read,
    .f_write   = &StandardFile_Write,
    .f_seek    = &StandardFile_Seek,
    .f_readdir = &StandardFile_ReadDir,
    .f_ioctl   = &StandardFile_Ioctl,
    .f_poll    = &StandardFile_Poll,
};

/* Allocate a new, NULL-initialized file with one reference. */
PUBLIC ATTR_RETNONNULL ATTR_MALLOC
REF struct file *KCALL file_alloc(void) {
 REF struct file *result;
 result = (REF struct file *)kmalloc(sizeof(struct file),
                                     GFP_SHARED|GFP_CALLOC);
 result->f_refcnt = 1;
 return result;
}


PRIVATE u8 const access_matrix[] = {
    [IO_RDONLY]   = R_OK,
    [IO_WRONLY]   = W_OK,
    [IO_RDWR]     = R_OK|W_OK,
    [IO_RDWR_ALT] = R_OK|W_OK
};


/* High-level open a given INode and path pair as a file stream. */
FUNDEF REF struct handle KCALL
file_open(struct inode *__restrict EXCEPT_VAR node,
          struct path *__restrict p, oflag_t flags) {
 REF struct handle result;
 if (flags & O_NOATIME) /* TODO: Implement permissions for this */
     throw_fs_error(ERROR_FS_ACCESS_ERROR);

 /* Check if the caller is allowed to access the file. */
 inode_access(node,access_matrix[flags & IO_ACCMODE]);

 /* Update last-accessed time if requested, to. */
 if ((node->i_super->s_flags&SUPERBLOCK_FDOATIME) &&
    !(flags & O_NOATIME) && node->i_ops->io_saveattr) {
  rwlock_write(&node->i_lock);
  TRY {
   /* Write the new last-accessed time. */
   node->i_attr.a_atime = wall_gettime(node->i_super->s_wall);
   if (node->i_ops->io_maskattr)
     (*node->i_ops->io_maskattr)(node);
   /* Mark the INode as changed. */
   inode_changed(node);
  } FINALLY {
   rwlock_endwrite(&node->i_lock);
  }
 }

 /* Forward to the open operator (if it exists) */
 if (node->i_ops->io_file.f_open)
     return (*node->i_ops->io_file.f_open)(node,p,flags);
 /* Special handling for block/character device nodes. */
 if (S_ISDEV(node->i_attr.a_mode)) {
  REF struct device *EXCEPT_VAR dev;
  if (S_ISBLK(node->i_attr.a_mode)) {
   dev = lookup_device(DEVICE_TYPE_FBLOCKDEV,
                       node->i_attr.a_rdev);
  } else {
   dev = lookup_device(DEVICE_TYPE_FCHARDEV,
                       node->i_attr.a_rdev);
  }
  TRY {
   result = device_open(dev,p,flags);
  } FINALLY {
   device_decref(dev);
  }
  goto done;
 }

 /* Open the file manually. */
 result.h_mode = HANDLE_MODE(HANDLE_TYPE_FFILE,IO_FROM_O(flags));
 result.h_object.o_file = (REF struct file *)kmalloc(sizeof(struct file),GFP_SHARED);
 /* Initialize fields. */
 result.h_object.o_file->f_refcnt = 1;
 result.h_object.o_file->f_node   = node;
 result.h_object.o_file->f_path   = p;
 result.h_object.o_file->f_ops    = &default_file_operations;
 result.h_object.o_file->f_pos    = 0;
 atomic_rwlock_init(&result.h_object.o_file->f_directory.d_curlck);
 result.h_object.o_file->f_directory.d_curidx = 0;
 result.h_object.o_file->f_directory.d_curent = NULL;
 inode_incref(node);
 path_incref(p);
done:
 return result;
}

/* HINT : This function calls `directory_creatfile()', using
 *        `!(flags & O_EXCL)' for `allow_open_existing'.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_PATH_NOT_FOUND:      [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:        [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:        [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:        `namelen' was ZERO(0)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_ALREADY_EXISTS: [...] (Only when `O_EXCL' is set)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:           [...] */
FUNDEF REF struct handle KCALL
file_creat(struct directory_node *__restrict path_node,
           struct path *__restrict path_desc,
           CHECKED USER char const *__restrict name,
           u16 namelen, oflag_t flags,
           uid_t owner, gid_t group, mode_t mode) {
 REF struct directory_entry *EXCEPT_VAR entry;
 REF struct inode *EXCEPT_VAR node;
 REF struct path *EXCEPT_VAR result_path;
 REF struct handle result;
 /* Create the node for the new file. */
 node = directory_creatfile(path_node,name,namelen,flags,
                            owner,group,mode,
                           (struct directory_entry **)&entry);
 TRY {
  /* Construct a path node for the new file. */
  result_path = path_newchild(path_desc,path_node,node,entry);
  TRY {
   /* Open the newly created node and associated path. */
   result = file_open(node,result_path,flags);
  } FINALLY {
   path_decref(result_path);
  }
 } FINALLY {
  directory_entry_decref(entry);
  inode_decref(node);
 }
 return result;
}

/* Perform stream operations on the given file (see operators above)
 * These are the high-level wrappers that are exposed through system calls. */
PUBLIC pos_t KCALL
file_seek(struct file *__restrict self,
          off_t off, int whence) {
 return (*self->f_ops->f_seek)(self,off,whence);
}

PUBLIC size_t KCALL
file_read(struct file *__restrict EXCEPT_VAR self,
          USER CHECKED void *buf,
          size_t bufsize, iomode_t flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->f_node->i_lock,flags);
 TRY {
  result = SAFECALL_KCALL_4(self->f_ops->f_read,self,buf,bufsize,flags);
 } FINALLY {
  if (rwlock_endread(&self->f_node->i_lock))
      goto again;
 }
 return result;
}

PUBLIC size_t KCALL
file_write(struct file *__restrict EXCEPT_VAR self,
           USER CHECKED void const *buf,
           size_t bufsize, iomode_t flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 rwlock_writef(&self->f_node->i_lock,flags);
 TRY {
  if (flags & IO_APPEND) {
   /* Make sure that the size-attribute has been loaded. */
   inode_loadattr(self->f_node);
   /* Move the file pointer to the back of the file. */
   self->f_pos = self->f_node->i_attr.a_size;
  }
  result = SAFECALL_KCALL_4(self->f_ops->f_write,self,buf,bufsize,flags);
 } FINALLY {
  rwlock_endwrite(&self->f_node->i_lock);
 }
 return result;
}

PUBLIC size_t KCALL
file_readdir(struct file *__restrict EXCEPT_VAR self,
             USER CHECKED struct dirent *buf,
             size_t bufsize, int mode, iomode_t flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->f_node->i_lock,flags);
 TRY {
  result = SAFECALL_KCALL_5(self->f_ops->f_readdir,self,buf,bufsize,mode,flags);
 } FINALLY {
  if (rwlock_endread(&self->f_node->i_lock))
      goto again;
 }
 return result;
}

PUBLIC ssize_t KCALL
file_ioctl(struct file *__restrict EXCEPT_VAR self,
           unsigned long cmd,
           USER UNCHECKED void *arg, iomode_t flags) {
 ssize_t COMPILER_IGNORE_UNINITIALIZED(result);
 rwlock_writef(&self->f_node->i_lock,flags);
 TRY {
  result = SAFECALL_KCALL_4(self->f_ops->f_ioctl,self,cmd,arg,flags);
 } FINALLY {
  rwlock_endwrite(&self->f_node->i_lock);
 }
 return result;
}
PUBLIC unsigned int KCALL
file_poll(struct file *__restrict EXCEPT_VAR self, unsigned int mode) {
 return SAFECALL_KCALL_2(self->f_ops->f_poll,self,mode);
}





/* File-handle operators. */
DEFINE_INTERN_ALIAS(handle_file_read,file_read);
DEFINE_INTERN_ALIAS(handle_file_write,file_write);
DEFINE_INTERN_ALIAS(handle_file_seek,file_seek);
DEFINE_INTERN_ALIAS(handle_file_readdir,file_readdir);
DEFINE_INTERN_ALIAS(handle_file_ioctl,file_ioctl);
INTERN size_t KCALL
handle_file_pread(struct file *__restrict self,
                  USER CHECKED void *buf,
                  size_t bufsize, pos_t pos,
                  iomode_t flags) {
 return inode_read(self->f_node,buf,bufsize,pos,flags);
}
INTERN size_t KCALL
handle_file_pwrite(struct file *__restrict self,
                   USER CHECKED void const *buf,
                   size_t bufsize, pos_t pos,
                   iomode_t flags) {
 return inode_write(self->f_node,buf,bufsize,pos,flags);
}
INTERN void KCALL
handle_file_truncate(struct file *__restrict self,
                     pos_t new_smaller_size) {
 inode_truncate(self->f_node,new_smaller_size);
}
INTERN void KCALL
handle_file_sync(struct file *__restrict self,
                 bool data_only) {
 inode_sync(self->f_node,data_only);
}
INTERN void KCALL
handle_file_stat(struct file *__restrict self,
                 USER CHECKED struct stat64 *result) {
 inode_stat(self->f_node,result);
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_FILE_C */
