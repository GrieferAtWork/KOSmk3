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
#ifndef GUARD_KERNEL_SRC_FS_PATH_C
#define GUARD_KERNEL_SRC_FS_PATH_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/list/list.h>
#include <hybrid/align.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <kernel/sections.h>
#include <fs/node.h>
#include <fs/path.h>
#include <fs/handle.h>
#include <bits/sched.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <except.h>
#include <fcntl.h>
#include <ctype.h>

DECL_BEGIN

#if CONFIG_PATH_CHILD_MAPSIZE & (CONFIG_PATH_CHILD_MAPSIZE-1)
#error "`CONFIG_PATH_CHILD_MAPSIZE' must be a power-of-2"
#endif

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


PRIVATE void KCALL
path_clear_children(struct path *__restrict self) {
 unsigned int i;
 /* Clear all children. */
 atomic_rwlock_write(&self->p_directory.d_lock);
 for (i = 0; i < CONFIG_PATH_CHILD_MAPSIZE; ++i) {
  struct path *iter,*next;
  iter = self->p_directory.d_child[i];
  while (iter) {
   /* Unlink the sibling chain of paths. */
   next = iter->p_siblings.le_next;
   iter->p_siblings.le_pself = NULL;
   iter->p_siblings.le_next  = NULL;
   iter = next;
  }
 }
 atomic_rwlock_endwrite(&self->p_directory.d_lock);
}


PUBLIC void KCALL
path_mount(struct path *__restrict self,
           struct inode *__restrict node) {
 struct superblock *filesystem = node->i_super;
 atomic_rwlock_write(&self->p_lock);
 if unlikely(PATH_ISMOUNT(self)) {
  /* Already a mounting point. */
  atomic_rwlock_endwrite(&self->p_lock);
  throw_fs_error(ERROR_FS_FILE_ALREADY_EXISTS);
 }
 atomic_rwlock_write(&filesystem->s_mount_lock);
 /* Check if the node, superblock, path or vfs have been closed. */
 if unlikely((node->i_flags & INODE_FCLOSED) ||
             (filesystem->s_flags & SUPERBLOCK_FCLOSED) ||
             (self->p_flags & PATH_FCLOSED) ||
             (self->p_vfs->v_root.p_flags & PATH_FCLOSED)) {
  atomic_rwlock_endwrite(&filesystem->s_mount_lock);
  atomic_rwlock_endwrite(&self->p_lock);
  throw_fs_error(ERROR_FS_FILE_NOT_FOUND);
 }

 atomic_rwlock_write(&self->p_vfs->v_mount.m_lock);

 /* Start tracking this path as a mounting point. */
 LIST_INSERT(filesystem->s_mount,self,p_mount.m_fsmount);
 path_incref(self); /* Reference stored in `filesystem->s_mount' */
 LIST_INSERT(self->p_vfs->v_mount.m_chain,self,p_mount.m_mount);

 /* Implicit reference transfer: `p_node' --> `p_mount.m_rnode' */
 self->p_node = node;
 superblock_incref(node->i_super);
 inode_incref(node);

 /* Release locks. */
 atomic_rwlock_endwrite(&self->p_vfs->v_mount.m_lock);
 atomic_rwlock_endwrite(&filesystem->s_mount_lock);

 /* Clear all children. */
 path_clear_children(self);
 atomic_rwlock_endwrite(&self->p_lock);

 /* Clear the recently-used cache to get rid of
  * anything that was reachable from the old node. */
 vfs_clear_recent(self->p_vfs);
}


PUBLIC bool KCALL
path_umount(struct path *__restrict self) {
 struct superblock *filesystem;
 REF struct inode *mounted_node;
 atomic_rwlock_write(&self->p_lock);
 /* Check if we're really dealing with a mounting point. */
 if (!PATH_ISMOUNT(self)) {
  atomic_rwlock_endwrite(&self->p_lock);
  return false;
 }
 assertf(self->p_refcnt >= 2,
         "+1 for the caller, +1 for the "
         "mounting chain in `super->s_mount'");
 mounted_node = self->p_node;
 filesystem   = mounted_node->i_super;
 
 /* Acquire required locks. */
 atomic_rwlock_write(&filesystem->s_mount_lock);
 atomic_rwlock_write(&self->p_vfs->v_mount.m_lock);

 /* Restore the original mounted node. */
 self->p_node = self->p_mount.m_rnode; /* Implicit reference transfer. */
 
 /* Remove the path from mounting chains. */
 LIST_REMOVE(self,p_mount.m_fsmount); /* Inherit reference to `self' */
 LIST_REMOVE(self,p_mount.m_mount);

 atomic_rwlock_endwrite(&self->p_vfs->v_mount.m_lock);
 atomic_rwlock_endwrite(&filesystem->s_mount_lock);

 /* Clear all children. */
 path_clear_children(self);
 atomic_rwlock_endwrite(&self->p_lock);

 /* Drop references previously held by the mount binding. */
 inode_decref(mounted_node);    /* Inherited by restoring `p_mount.m_rnode' */
 superblock_decref(filesystem); /* Inherited by restoring `p_mount.m_rnode' */
 path_decref(self);             /* Inherited from `p_mount.m_fsmount' */

 return true;
}


PUBLIC size_t KCALL
vfs_umount_all(struct vfs *__restrict self) {
 size_t result = 0;
 while (ATOMIC_READ(self->v_mount.m_chain)) {
  REF struct path *p;
  atomic_rwlock_write(&self->v_mount.m_lock);
  COMPILER_READ_BARRIER();
  p = self->v_mount.m_chain;
  if (p) {
   assert(PATH_ISMOUNT(p));
   path_incref(p);
  }
  atomic_rwlock_endwrite(&self->v_mount.m_lock);
  if (!p) break;

  /* Unmount this path. */
  if (path_umount(p))
      ++result;

  path_decref(p);
 }
 return result;
}

PUBLIC size_t KCALL
superblock_umount_all(struct superblock *__restrict self) {
 size_t result = 0;
 while (ATOMIC_READ(self->s_mount)) {
  REF struct path *p;
  atomic_rwlock_write(&self->s_mount_lock);
  COMPILER_READ_BARRIER();
  p = self->s_mount;
  if (p) {
   assert(PATH_ISMOUNT(p));
   path_incref(p);
  }
  atomic_rwlock_endwrite(&self->s_mount_lock);
  if (!p) break;

  /* Unmount this path. */
  if (path_umount(p))
      ++result;

  path_decref(p);
 }
 return result;
}




/* Destroy a previously allocated path. */
PUBLIC void KCALL
path_destroy(struct path *__restrict self) {
 struct path *parent;
 assert(!PATH_ISVFS(self) ||
       (self == &self->p_vfs->v_root &&
       !((struct vfs *)self)->v_recent.r_size &&
       !((struct vfs *)self)->v_mount.m_chain));
 assert(!PATH_ISMOUNT(self));
 if ((parent = self->p_parent) != NULL) {
  assert(self->p_node);
  assert(self->p_dirent);
  if (self->p_siblings.le_pself) {
   atomic_rwlock_write(&parent->p_directory.d_lock);
   COMPILER_READ_BARRIER();
   if (self->p_siblings.le_pself)
       LIST_REMOVE(self,p_siblings);
   atomic_rwlock_endwrite(&parent->p_directory.d_lock);
  }

  directory_entry_decref(self->p_dirent);
#if 0 /* It really shouldn't cause any problems
       * if we don't clear the cache here. */
  if (PATH_ISVFS(parent))
      vfs_clear_recent((struct vfs *)parent);
#endif
  path_decref(parent);
 } else {
  assert(!self->p_dirent);
 }
 /* NOTE: `p_node' may be NULL if the path was only partially constructed. */
 if (self->p_node)
     inode_decref(self->p_node);
 if (self->p_vfs && self != &self->p_vfs->v_root) {
#if 1
  /* It really shouldn't cause any problems
   * if we don't clear the cache here. */
  path_decref(&self->p_vfs->v_root);
#else
  vfs_decref(self->p_vfs);
#endif
 }
 /* NOTE: We know that we can't have any children because they
  *       would be holding a reference to us through `p_parent' */
 kfree(self);
}

PRIVATE ssize_t KCALL
path_print_parents(struct path *__restrict self,
                   struct path *__restrict root,
                   pformatprinter printer,
                   void *closure, unsigned int type) {
 ssize_t result = 0,temp;
 if ((type & REALPATH_FDOSPATH) &&
     (self->p_flags & PATH_FISDRIVE)) {
  /* Stop at the first drive in DOS-mode. */
  struct vfs *v;
  if ((type & REALPATH_FTYPEMASK) == (REALPATH_FDRIVEPATH & REALPATH_FTYPEMASK))
      return 0; /* Don't print the drive itself! */
  v = self->p_vfs;
  atomic_rwlock_read(&v->v_drives.d_lock);
  COMPILER_READ_BARRIER();
  if (self->p_flags & PATH_FISDRIVE) {
   char buf[2];
   unsigned int i;
   for (i = 0;; ++i) {
    assert(i < VFS_DRIVECOUNT);
    if (v->v_drives.d_drives[i] == self)
        break;
   }
   atomic_rwlock_endread(&v->v_drives.d_lock);
   buf[0] = (char)('A'+i);
   buf[1] = ':';
   return (*printer)(buf,2,closure);
  }
  atomic_rwlock_endread(&v->v_drives.d_lock);
 }
 if (self == root) {
  /* Always stop at the POSIX root path. */
  if ((type & REALPATH_FDOSPATH) &&
      (type & REALPATH_FTYPEMASK) != (REALPATH_FDRIVEPATH & REALPATH_FTYPEMASK)) {
   char buf[2];
   buf[0] = ':';
   buf[1] = ':';
   return (*printer)(buf,2,closure);
  }
  return 0;
 }
 assert((self->p_dirent != NULL) ==
        (self->p_parent != NULL));
 if (!self->p_parent) {
  /* Check for the real root path. */
  if (type & REALPATH_FDOSPATH) {
   char buf[3];
   buf[0] = ':';
   buf[1] = ':';
   buf[2] = '\\';
   temp = (*printer)(buf,3,closure);
  } else {
   char buf[1];
   buf[0] = '/';
   temp = (*printer)(buf,1,closure);
  }
 } else {
  /* Print parent portion of the path. */
  temp = path_print_parents(self->p_parent,root,
                            printer,closure,type);
 }
 if unlikely(temp < 0) goto err;
 result += temp;
 /* Print a separator, followed by the name of this path. */
 { char sep[1];
   sep[0] = (type & REALPATH_FDOSPATH) ? '\\' : '/';
   temp = (*printer)(sep,1,closure);
   if unlikely(temp < 0) goto err;
   result += temp;
 }
 temp = (*printer)(self->p_dirent->de_name,
                   self->p_dirent->de_namelen,
                   closure);
 if unlikely(temp < 0) goto err;
 result += temp;
 return result;
err:
 return temp;
}

/* Print the name of the given path, according to the specified `type'.
 * @param: type: One of `REALPATH_F*', optionally or'd with `REALPATH_FDOSPATH'
 * @return: < 0: The first negative value returned by `printer'
 * @return: * :  The sum of all calls to `printer'. */
PUBLIC ssize_t KCALL
path_print(struct path *__restrict self, pformatprinter printer,
           void *closure, unsigned int type) {
 ssize_t result = 0,COMPILER_IGNORE_UNINITIALIZED(temp);
 REF struct path *EXCEPT_VAR root; struct fs *f;
 /* Check of unknown bits. */
 if (type & ~(REALPATH_FTYPEMASK|REALPATH_FFLAGMASK))
     error_throw(E_INVALID_ARGUMENT);
 switch (type & REALPATH_FTYPEMASK) {

 {
  char sep[1];
 case REALPATH_FHEAD & REALPATH_FTYPEMASK:
 case REALPATH_FPATH & REALPATH_FTYPEMASK:
 case REALPATH_FDRIVEPATH & REALPATH_FTYPEMASK:
  /* Print the path leading up to this point. */
  f = THIS_FS;
  atomic_rwlock_read(&f->fs_lock);
  root = f->fs_root;
  path_incref(root);
  atomic_rwlock_endread(&f->fs_lock);
  if (self->p_parent) {
   TRY {
    temp = path_print_parents(self->p_parent,root,printer,closure,type);
   } FINALLY {
    path_decref(root);
   }
  } else {
   path_decref(root);
   temp = 0;
   if ((type & REALPATH_FDOSPATH) &&
       (type & REALPATH_FTYPEMASK) != (REALPATH_FDRIVEPATH & REALPATH_FTYPEMASK)) {
    char buf[2];
    buf[0] = ':';
    buf[1] = ':';
    temp = (*printer)(buf,2,closure);
   }
  }
  if unlikely(temp < 0) goto err;
  result += temp;
  if ((type & REALPATH_FTYPEMASK) ==
      (REALPATH_FHEAD & REALPATH_FTYPEMASK))
       break; /* Only the head! */
  sep[0] = type & REALPATH_FDOSPATH ? '\\' : '/';
  temp = (*printer)(sep,1,closure);
  if unlikely(temp < 0) goto err;
  result += temp;
  ATTR_FALLTHROUGH
 case REALPATH_FTAIL & REALPATH_FTYPEMASK:
  /* Simply print the filename of this path entry. */
  if (self->p_dirent) {
   temp = (*printer)(self->p_dirent->de_name,
                     self->p_dirent->de_namelen,
                     closure);
   if unlikely(temp < 0) goto err;
   result += temp;
  }
 } break;

 {
  struct vfs *v;
  unsigned int i;
  char buf[2];
 case REALPATH_FDRIVE & REALPATH_FTYPEMASK:
  v = self->p_vfs;
  atomic_rwlock_read(&v->v_drives.d_lock);
  while (!(self->p_flags & PATH_FISDRIVE)) {
   self = self->p_parent;
   if (!self) {
    atomic_rwlock_endread(&v->v_drives.d_lock);
    /* Symbolic root drive. */
    return (*printer)("::",2,closure);
   }
  }
  /* Find the associated drive name. */
  for (i = 0;; ++i) {
   assert(i < VFS_DRIVECOUNT);
   if (v->v_drives.d_drives[i] == self)
       break;
  }
  atomic_rwlock_endread(&v->v_drives.d_lock);
  /* Return the name of the hosting DOS drive mounting point. */
  buf[0] = 'A'+i;
  buf[1] = ':';
  return (*printer)(buf,2,closure);
 } break;

 default:
  /* Unknown type. */
  error_throw(E_INVALID_ARGUMENT);
  break;
 }
 return result;
err:
 return temp;
}




struct buffer_printer {
    char *buf,*end;
};
PRIVATE ssize_t KCALL
buffer_callback(char const *__restrict data,
                size_t datalen,
                struct buffer_printer *__restrict buf) {
 if (buf->buf < buf->end) {
  size_t maxlen = (size_t)(buf->end-buf->buf);
  if (maxlen > datalen)
      maxlen = datalen;
  memcpy(buf->buf,data,maxlen*sizeof(char));
  buf->buf += maxlen;
 }
 return (ssize_t)datalen;
}



/* Using `path_print()', fill in the given user-space buffer before
 * appending a NUL-character and returning the number of required
 * characters (excluding the NUL-character).
 * @param: type: One of `REALPATH_F*', optionally or'd with `REALPATH_FDOSPATH' */
PUBLIC size_t KCALL
path_getname(struct path *__restrict self,
             USER CHECKED char *buf,
             size_t bufsize, unsigned int type) {
 size_t result;
 struct buffer_printer data;
 data.buf = buf;
 data.end = buf+bufsize;
 result = (size_t)path_print(self,
                            (pformatprinter)&buffer_callback,
                             &data,type);
 if (data.buf < data.end) *data.buf = '\0';
 return result;
}






PRIVATE void KCALL
vfs_add_recent(struct path *__restrict p) {
 struct vfs *v = p->p_vfs;
again:
 atomic_rwlock_write(&v->v_recent.r_lock);
 assert(v->v_recent.r_size <= CONFIG_VFS_RECENT_CACHESIZE);
 if (v->v_recent.r_size == CONFIG_VFS_RECENT_CACHESIZE) {
  /* Get rid of paths that haven't been used recently. */
  unsigned int i,old_refs_count = 0;
  REF struct path *old_refs[CONFIG_VFS_RECENT_CACHESIZE];
  u16 rmin = (u16)-1,rmax = 0,ravg;
  for (i = 0; i < v->v_recent.r_size; ++i) {
   struct path *rp = v->v_recent.r_recent[i];
   if (rmin > rp->p_recent)
       rmin = rp->p_recent;
   if (rmax < rp->p_recent)
       rmax = rp->p_recent;
  }
  ravg = rmin + CEILDIV(rmax-rmin,2);
  for (i = 0; i < v->v_recent.r_size;) {
   struct path *rp = v->v_recent.r_recent[i];
   if (rp->p_recent > ravg) {
    /* This page stays (Subtract a portion of the average use). */
    ATOMIC_FETCHSUB(rp->p_recent,CEILDIV(ravg,2));
    ++i;
   } else {
    rp->p_recent               = 0;
    old_refs[old_refs_count++] = rp;
    /* Delete this recent-entry. */
    --v->v_recent.r_size;
    memmove(&v->v_recent.r_recent[i],
            &v->v_recent.r_recent[i+1],
            (v->v_recent.r_size-i)*
             sizeof(REF struct path *));
    v->v_recent.r_recent[v->v_recent.r_size] = NULL;
   }
  }
  atomic_rwlock_endwrite(&v->v_recent.r_lock);
  /* Drop old path references. */
  while (old_refs_count--)
     path_decref(old_refs[old_refs_count]);
  goto again;
 }
 /* Add the given path to the cache of recently used paths. */
 if (p->p_recent != 0) {
  path_incref(p);
  v->v_recent.r_recent[v->v_recent.r_size++] = p;
 }
 atomic_rwlock_endwrite(&v->v_recent.r_lock);
}


PRIVATE void KCALL vfs_recent(struct path *__restrict p) {
 u16 count;
 do if ((count = ATOMIC_READ(p->p_recent)) == (u16)-1) return;
 while (!ATOMIC_CMPXCH_WEAK(p->p_recent,count,count+1));
 if (count != 0) return;
 /* Add to the recent-cache. */
 vfs_add_recent(p);
}


/* Return a reference to the child of a given path.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_PATH_NOT_FOUND:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_NOT_A_DIRECTORY: [...] */
PUBLIC ATTR_RETNONNULL REF struct path *KCALL
path_child(struct path *__restrict self,
           USER CHECKED char const *path, u16 pathlen) {
 uintptr_t hash;
 REF struct path *EXCEPT_VAR result;
 REF struct path *new_result,**pbucket;
 assertf(self->p_node,"Path hasn't been initialized");
 assertf(self->p_vfs,"Path hasn't been initialized");
 hash = directory_entry_hash(path,pathlen);
again:
 atomic_rwlock_read(&self->p_directory.d_lock);
 result = self->p_directory.d_child[hash & (CONFIG_PATH_CHILD_MAPSIZE-1)];
 for (; result; result = result->p_siblings.le_next) {
  struct directory_entry *entry = result->p_dirent;
  if (entry->de_hash != hash) continue;
  if (entry->de_namelen != pathlen) continue;
  /* This might the one... */
  path_incref(result);
  atomic_rwlock_endread(&self->p_directory.d_lock);
  TRY { /* Guard against E_SEGFAULT from user-space. */
   if (memcmp(entry->de_name,path,pathlen*sizeof(char)) == 0) {
    vfs_recent(result);
    return result;
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   path_decref(result);
   error_rethrow();
  }
  /* ... Nope */
  atomic_rwlock_read(&self->p_directory.d_lock);
  if (!ATOMIC_DECIFNOTONE(result->p_refcnt)) {
   atomic_rwlock_endread(&self->p_directory.d_lock);
   path_decref(result);
   goto again;
  }
 }
 atomic_rwlock_endread(&self->p_directory.d_lock);
 /* Check if our node even is a directory. */
 if (!INODE_ISDIR(self->p_node))
      throw_fs_error(ERROR_FS_NOT_A_DIRECTORY);

 /* Consult the filesystem for this directory entry. */
 {
  REF struct directory_entry *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(entry);
  REF struct inode *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(entry_node);
  struct directory_node *EXCEPT_VAR dir;
  dir = (struct directory_node *)self->p_node;
again_getentry:
  rwlock_read(&dir->d_node.i_lock);
  TRY {
   entry = directory_getentry(dir,path,pathlen,hash);
   if (!entry)
        throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
   /* Got the entry! */
   directory_entry_incref(entry);
  } FINALLY {
   if (rwlock_endread(&dir->d_node.i_lock))
       goto again_getentry;
  }
  /* Now lookup the INode of the entry. */
  TRY {
   entry_node = superblock_opennode(dir->d_node.i_super,dir,entry);
   TRY {
    /* Construct a new path node for this entry. */
    result = (REF struct path *)kmalloc(sizeof(struct path),
                                        GFP_SHARED|GFP_CALLOC);
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    inode_decref(entry_node);
    error_rethrow();
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   directory_entry_decref(entry);
   error_rethrow();
  }
  /* Construct the new path node. */
  path_incref(self); /* For `result->p_parent' */
  result->p_refcnt = 1;
  result->p_node   = entry_node;  /* Inherit reference. */
  result->p_dirent = entry;       /* Inherit reference. */
  result->p_parent = self;        /* Inherit reference. */
  vfs_incref(self->p_vfs);
  result->p_vfs           = self->p_vfs;
  result->p_mount.m_rnode = entry_node;
  atomic_rwlock_cinit(&result->p_lock);
  atomic_rwlock_cinit(&result->p_directory.d_lock);

  /* Add the new node to the directory. */
  atomic_rwlock_write(&self->p_directory.d_lock);
  pbucket = &self->p_directory.d_child[entry->de_hash & (CONFIG_PATH_CHILD_MAPSIZE-1)];
  new_result = *pbucket;
  for (; new_result; new_result = new_result->p_siblings.le_next) {
   if unlikely(new_result->p_dirent == entry ||
              (new_result->p_dirent->de_hash == entry->de_hash &&
               new_result->p_dirent->de_namelen == entry->de_namelen &&
               memcmp(new_result->p_dirent->de_name,entry->de_name,
                      entry->de_namelen*sizeof(char)) == 0)) {
    /* Node was loaded by another thread in the mean time. */
    path_incref(new_result);
    atomic_rwlock_endwrite(&self->p_directory.d_lock);
    /* Return the existing entry, rather than the next. */
    path_decref(result);
    vfs_recent(new_result);
    return new_result;
   }
  }
  /* Insert the newly generated node into the parent directory. */
  LIST_INSERT(*pbucket,result,p_siblings);
  atomic_rwlock_endwrite(&self->p_directory.d_lock);
 }
 vfs_recent(result);
 return result;
}
PUBLIC ATTR_RETNONNULL REF struct path *KCALL
path_casechild(struct path *__restrict self,
               USER CHECKED char const *path, u16 pathlen) {
 uintptr_t hash;
 REF struct path *EXCEPT_VAR result;
 REF struct path *new_result,**pbucket;
 assertf(self->p_node,"Path hasn't been initialized");
 assertf(self->p_vfs,"Path hasn't been initialized");
 hash = directory_entry_hash(path,pathlen);
again:
 atomic_rwlock_read(&self->p_directory.d_lock);
 result = self->p_directory.d_child[hash & (CONFIG_PATH_CHILD_MAPSIZE-1)];
 for (; result; result = result->p_siblings.le_next) {
  struct directory_entry *entry = result->p_dirent;
  if (entry->de_hash != hash) continue;
  if (entry->de_namelen != pathlen) continue;
  /* This might the one... */
  path_incref(result);
  atomic_rwlock_endread(&self->p_directory.d_lock);
  TRY { /* Guard against E_SEGFAULT from user-space. */
   if (memcmp(entry->de_name,path,pathlen*sizeof(char)) == 0) {
    vfs_recent(result);
    return result;
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   path_decref(result);
   error_rethrow();
  }
  /* ... Nope */
  atomic_rwlock_read(&self->p_directory.d_lock);
  if (!ATOMIC_DECIFNOTONE(result->p_refcnt)) {
   atomic_rwlock_endread(&self->p_directory.d_lock);
   path_decref(result);
   goto again;
  }
 }
 /* Either the file doesn't exist, or the given casing is incorrect. */
 if unlikely(!pathlen) {
  atomic_rwlock_endread(&self->p_directory.d_lock);
  throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
 }
 {
  char first_char = path[0];
  COMPILER_READ_BARRIER();
  first_char = tolower(first_char);
  for (hash = 0; hash < CONFIG_PATH_CHILD_MAPSIZE; ++hash) {
   result = self->p_directory.d_child[hash];
   for (; result; result = result->p_siblings.le_next) {
    struct directory_entry *entry = result->p_dirent;
    if (entry->de_namelen != pathlen) continue;
    if (tolower(entry->de_name[0]) != first_char) continue; /* Try to match the first character. */
    /* Try this one. */
    path_incref(result);
    atomic_rwlock_endread(&self->p_directory.d_lock);
    TRY { /* Guard against E_SEGFAULT from user-space. */
     if (memcasecmp(entry->de_name,path,pathlen*sizeof(char)) == 0) {
      vfs_recent(result);
      return result;
     }
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     path_decref(result);
     error_rethrow();
    }
    /* ... Nope */
    atomic_rwlock_read(&self->p_directory.d_lock);
    if (!ATOMIC_DECIFNOTONE(result->p_refcnt)) {
     atomic_rwlock_endread(&self->p_directory.d_lock);
     path_decref(result);
     goto again;
    }
   }
  }
 }
 atomic_rwlock_endread(&self->p_directory.d_lock);
 /* Check if our node even is a directory. */
 if (!INODE_ISDIR(self->p_node))
      throw_fs_error(ERROR_FS_NOT_A_DIRECTORY);

 /* Consult the filesystem for this directory entry. */
 {
  REF struct directory_entry *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(entry);
  REF struct inode *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(entry_node);
  struct directory_node *EXCEPT_VAR dir;
  dir = (struct directory_node *)self->p_node;
again_getentry:
  rwlock_read(&dir->d_node.i_lock);
  TRY {
   entry = directory_getcaseentry(dir,path,pathlen,hash);
   if (!entry) throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
   /* Got the entry! */
   directory_entry_incref(entry);
  } FINALLY {
   if (rwlock_endread(&dir->d_node.i_lock))
       goto again_getentry;
  }
  /* Now lookup the INode of the entry. */
  TRY {
   entry_node = superblock_opennode(dir->d_node.i_super,dir,entry);
   TRY {
    /* Construct a new path node for this entry. */
    result = (REF struct path *)kmalloc(sizeof(struct path),
                                        GFP_SHARED|GFP_CALLOC);
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    inode_decref(entry_node);
    error_rethrow();
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   directory_entry_decref(entry);
   error_rethrow();
  }
  /* Construct the new path node. */
  path_incref(self); /* For `result->p_parent' */
  result->p_refcnt        = 1;
  result->p_node          = entry_node;  /* Inherit reference. */
  result->p_dirent        = entry;       /* Inherit reference. */
  result->p_parent        = self;        /* Inherit reference. */
  vfs_incref(self->p_vfs);
  result->p_vfs           = self->p_vfs;
  result->p_mount.m_rnode = entry_node;
  atomic_rwlock_cinit(&result->p_lock);
  atomic_rwlock_cinit(&result->p_directory.d_lock);

  /* Add the new node to the directory. */
  atomic_rwlock_write(&self->p_directory.d_lock);
  pbucket = &self->p_directory.d_child[entry->de_hash & (CONFIG_PATH_CHILD_MAPSIZE-1)];
  new_result = *pbucket;
  for (; new_result; new_result = new_result->p_siblings.le_next) {
   if unlikely(new_result->p_dirent == entry ||
              (new_result->p_dirent->de_hash == entry->de_hash &&
               new_result->p_dirent->de_namelen == entry->de_namelen &&
               memcmp(new_result->p_dirent->de_name,entry->de_name,
                      entry->de_namelen*sizeof(char)) == 0)) {
    /* Node was loaded by another thread in the mean time. */
    path_incref(new_result);
    atomic_rwlock_endwrite(&self->p_directory.d_lock);
    /* Return the existing entry, rather than the next. */
    path_decref(result);
    vfs_recent(new_result);
    return new_result;
   }
  }
  /* Insert the newly generated node into the parent directory. */
  LIST_INSERT(*pbucket,result,p_siblings);
  atomic_rwlock_endwrite(&self->p_directory.d_lock);
 }
 vfs_recent(result);
 return result;
}


PUBLIC void KCALL
path_delchild(struct path *__restrict self,
              struct directory_node *__restrict self_node,
              struct directory_entry *__restrict child_entry) {
 REF struct path *child;
 /* Check if the path was remounted. */
 atomic_rwlock_read(&self->p_lock);
 if unlikely(self->p_node != (REF struct inode *)self_node) {
  atomic_rwlock_endread(&self->p_lock);
  return;
 }
 /* Interlocked change our lock to the directory */
 atomic_rwlock_write(&self->p_directory.d_lock);
 atomic_rwlock_endread(&self->p_lock);
 /* Search for a child-path for the given child_entry. */
 child = self->p_directory.d_child[child_entry->de_hash & (CONFIG_PATH_CHILD_MAPSIZE-1)];
 for (; child; child = child->p_siblings.le_next) {
  struct directory_entry *entry = child->p_dirent;
  if (entry == child_entry) goto child_found;
  /* Do a full name compare. */
  if likely(entry->de_hash != child_entry->de_hash) continue;
  if likely(entry->de_namelen != child_entry->de_namelen) continue;
  if likely(memcmp(entry->de_name,child_entry->de_name,
                   child_entry->de_namelen*sizeof(char)) != 0)
     continue;
child_found:
  /* Remove this child from the path cache. */
  path_incref(child); /* Get a reference for the `vfs_remove_recent()' below. */
  LIST_REMOVE(child,p_siblings);
  atomic_rwlock_endwrite(&self->p_directory.d_lock);
  vfs_remove_recent(child);
  /* Drop our temporary reference. */
  path_decref(child);
  return;
 }
 atomic_rwlock_endwrite(&self->p_directory.d_lock);
}

PUBLIC ATTR_RETNONNULL REF struct path *KCALL
path_newchild(struct path *__restrict self,
              struct directory_node *__restrict self_node,
              struct inode *__restrict child_node,
              struct directory_entry *__restrict child_entry) {
 REF struct path *result,*new_result,**pbucket;
 /* Construct the new path, optimizing for the greater
  * chance that the race condition of `path_child()'
  * having been called in the mean time (and the path
  * describing `child_entry' already exists) not having
  * happened. */
 result = (REF struct path *)kmalloc(sizeof(struct path),
                                     GFP_SHARED|GFP_CALLOC);
 result->p_refcnt        = 1;
 result->p_vfs           = self->p_vfs;
 result->p_parent        = self;
 result->p_dirent        = child_entry;
 result->p_node          = child_node;
 result->p_mount.m_rnode = child_node;
 atomic_rwlock_cinit(&result->p_lock);
 atomic_rwlock_cinit(&result->p_directory.d_lock);
 directory_entry_incref(child_entry);
 inode_incref(child_node);
 vfs_incref(self->p_vfs);
 path_incref(self);
 /* Check if the path was remounted. */
 atomic_rwlock_read(&self->p_lock);
 if unlikely(self->p_node != (REF struct inode *)self_node) {
  /* The node has changed. - We were remounted. */
  atomic_rwlock_endread(&self->p_lock);
  /* Return the resulting path as a ghost path
   * that isn't known as a child to the parent path.
   * XXX: Judging by their track record, Unix probably
   *      has some really gory term for this... ;)
   */
  goto done;
 }

 /* Interlocked change our lock to the directory */
 atomic_rwlock_write(&self->p_directory.d_lock);
 atomic_rwlock_endread(&self->p_lock);

 /* Check for the race condition where another
  * thread already created this entry. */
 pbucket = &self->p_directory.d_child[child_entry->de_hash & (CONFIG_PATH_CHILD_MAPSIZE-1)];
 new_result = *pbucket;
 for (; new_result; new_result = new_result->p_siblings.le_next) {
#if 0 /* XXX: Directory entries may change if files are deleted, then re-created... */
  if likely(new_result->p_dirent != child_entry) continue;
#else
  struct directory_entry *entry = new_result->p_dirent;
  if unlikely(entry == child_entry) goto already_exists;
  /* Do a full name-based compare. */
  if likely(entry->de_hash != child_entry->de_hash) continue;
  if likely(entry->de_namelen != child_entry->de_namelen) continue;
  if likely(memcmp(entry->de_name,child_entry->de_name,
                   child_entry->de_namelen*sizeof(char)) != 0)
     continue;
already_exists:
#endif
  /* Already exists... */
  path_incref(new_result);
  atomic_rwlock_endwrite(&self->p_directory.d_lock);
  /* Return the existing path, rather than the new one. */
  path_decref(result);
  vfs_recent(new_result);
  return new_result;
 }
 /* Add the path to the bucket list. */
 LIST_INSERT(*pbucket,result,p_siblings);
 atomic_rwlock_endwrite(&self->p_directory.d_lock);
 /* Cache the path as recently used. */
 vfs_recent(result);
done:
 return result;
}




/* Allocate a new VFS object and pre-initialize it to unmounted. */
PUBLIC ATTR_MALLOC ATTR_RETNONNULL
REF struct vfs *KCALL vfs_alloc(void) {
 REF struct vfs *result;
 result = (REF struct vfs *)kmalloc(sizeof(struct vfs),
                                    GFP_SHARED|GFP_CALLOC);
 result->v_root.p_refcnt = 1;
 result->v_root.p_vfs = result;
 atomic_rwlock_cinit(&result->v_mount.m_lock);
 atomic_rwlock_cinit(&result->v_recent.r_lock);
 atomic_rwlock_cinit(&result->v_root.p_lock);
 atomic_rwlock_cinit(&result->v_root.p_directory.d_lock);
 return result;
}



/* Create a deep copy of the current VFS (used when `CLONE_NEWNS' is set) */
PUBLIC ATTR_MALLOC ATTR_RETNONNULL
REF struct vfs *KCALL vfs_clone(void) {
 REF struct vfs *result = vfs_alloc();
 /* TODO: Copy mounting points. */
 //struct vfs *orig = THIS_VFS;

 return result;
}

PUBLIC ATTR_NOTHROW void KCALL
vfs_remove_recent(struct path *__restrict p) {
 u16 i; struct vfs *self = p->p_vfs;
 /* Quick check: Are there any recently used files. */
 if (!ATOMIC_READ(p->p_recent)) return;
 if (!ATOMIC_READ(self->v_recent.r_size)) return;
 atomic_rwlock_write(&self->v_recent.r_lock);
 COMPILER_READ_BARRIER();
 if (!p->p_recent) goto done;
 /* Search for the entry belonging to `p' */
 for (i = 0; i < self->v_recent.r_size;) {
  if (self->v_recent.r_recent[i] != p) { ++i; continue; }
  assertef(ATOMIC_DECFETCH(p->p_refcnt) != 0,
           "The caller must be holding a reference, too");
  /* Remove this recent-object cache entry. */
  --self->v_recent.r_size;
  memmove(&self->v_recent.r_recent[i],
          &self->v_recent.r_recent[i+1],
          (self->v_recent.r_size-i)*
           sizeof(REF struct path *));
 }
done:
 atomic_rwlock_endwrite(&self->v_recent.r_lock);
}

PUBLIC ATTR_NOTHROW void KCALL
vfs_clear_recent(struct vfs *__restrict self) {
 u16 count;
 REF struct path *temp[CONFIG_VFS_RECENT_CACHESIZE];
 /* Quick check: Are there any recently used files. */
 if (!ATOMIC_READ(self->v_recent.r_size)) return;
 atomic_rwlock_write(&self->v_recent.r_lock);
 COMPILER_READ_BARRIER();
 /* Extract the recently-used cache size. */
 count = self->v_recent.r_size;
 self->v_recent.r_size = 0;
 /* Extract the used portion of the recently-used cache. */
 memcpy(temp,self->v_recent.r_recent,count*sizeof(REF struct path *));
 atomic_rwlock_endwrite(&self->v_recent.r_lock);
 /* Drop references from all recently used path. */
 while (count--) path_decref(temp[count]);
}

/* Bind DOS drives.
 * If another drive had already been bound to the
 * same `drive_number', the old path is overwritten.
 * The caller is required to ensure that `location->p_vfs->v_fscount' is non-ZERO.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR: The calling this isn't allowed to bind or unbind DOS drives.
 * @param: drive_number: The drive number (0..VFS_DRIVECOUNT-1)
 * @param: location:     The path that should be bound to a DOS drive. */
PUBLIC void KCALL
vfs_bind_drive(u8 drive_number, struct path *__restrict location) {
 REF struct path *old_path;
 struct vfs *v = location->p_vfs;
 unsigned int i;
 /* TODO: Check if the calling thread is allowed to bind DOS drives. */
 assert(v->v_fscount != 0);
 assert(drive_number < VFS_DRIVECOUNT);
 /* Override the bound drive location. */
 path_incref(location);
 atomic_rwlock_write(&v->v_drives.d_lock);
 location->p_flags |= PATH_FISDRIVE;
 old_path = v->v_drives.d_drives[drive_number];
 v->v_drives.d_drives[drive_number] = location;
 for (i = 0; i < VFS_DRIVECOUNT; ++i)
     if (v->v_drives.d_drives[i] == old_path) goto done;
 /* Unset the ISDRIVE-flag. */
 old_path->p_flags &= ~PATH_FISDRIVE;
done:
 atomic_rwlock_endwrite(&v->v_drives.d_lock);
 path_decref(old_path);
}

/* Unbound a DOS drive.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR: The calling this isn't allowed to bind or unbind DOS drives.
 * @return: true:  The drive has been unbound.
 * @return: false: The drive was never bound to begin with. */
PUBLIC bool KCALL
vfs_unbind_drive(struct vfs *__restrict self, u8 drive_number) {
 unsigned int i;
 REF struct path *old_path;
 /* TODO: Check if the calling thread is allowed to bind DOS drives. */
 assert(drive_number < VFS_DRIVECOUNT);
 /* Override the drive binding with NULL. */
 atomic_rwlock_write(&self->v_drives.d_lock);
 old_path = self->v_drives.d_drives[drive_number];
 self->v_drives.d_drives[drive_number] = NULL;
 if (!old_path) {
  atomic_rwlock_endwrite(&self->v_drives.d_lock);
  return false;
 }
 for (i = 0; i < VFS_DRIVECOUNT; ++i)
     if (self->v_drives.d_drives[i] == old_path) goto done;
 /* Unset the ISDRIVE-flag. */
 old_path->p_flags &= ~PATH_FISDRIVE;
done:
 atomic_rwlock_endwrite(&self->v_drives.d_lock);
 /* Drop a reference from the old path. */
 path_decref(old_path);
 return true;
}

/* Return a reference to the path bound to the DOS drive `drive_number' in the current VFS.
 * @param: drive_number:                               The drive number (0..VFS_DRIVECOUNT-1)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_PATH_NOT_FOUND: The drive indexed by `drive_number' wasn't bound. */
PUBLIC ATTR_RETNONNULL REF struct path *KCALL vfs_drive(u8 drive_number) {
 REF struct path *result;
 struct vfs *v = THIS_VFS;
 assert(drive_number < VFS_DRIVECOUNT);
 assert(v->v_fscount != 0);
 atomic_rwlock_read(&v->v_drives.d_lock);
 result = v->v_drives.d_drives[drive_number];
 if unlikely(!result) {
  atomic_rwlock_endread(&v->v_drives.d_lock);
  throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
 }
 path_incref(result);
 atomic_rwlock_endread(&v->v_drives.d_lock);
 return result;
}

/* Unmount all DOS drives. */
PUBLIC ATTR_NOTHROW void KCALL
vfs_unbind_drives(struct vfs *__restrict self) {
 unsigned int i;
 REF struct path *old_paths[VFS_DRIVECOUNT];
 atomic_rwlock_write(&self->v_drives.d_lock);
 memcpy(old_paths,self->v_drives.d_drives,
        sizeof(REF struct path *)*VFS_DRIVECOUNT);
 memset(self->v_drives.d_drives,0,
        sizeof(REF struct path *)*VFS_DRIVECOUNT);
 /* Unset ISDRIVE flags for all paths. */
 for (i = 0; i < VFS_DRIVECOUNT; ++i) {
  if (!old_paths[i]) continue;
  old_paths[i]->p_flags &= ~PATH_FISDRIVE;
 }
 atomic_rwlock_endwrite(&self->v_drives.d_lock);
 /* Drop references from all paths. */
 for (i = 0; i < VFS_DRIVECOUNT; ++i) {
  if (!old_paths[i]) continue;
  path_decref(old_paths[i]);
 }
}





/* Allocate a new FS object and pre-initialize it to NULL, except for the following:
 *    - fs_umask  = CONFIG_FS_UMASK_DEFAULT
 *    - fs_lnkmax = CONFIG_FS_LNKMAX_DEFAULT */
PUBLIC ATTR_MALLOC ATTR_RETNONNULL
REF struct fs *KCALL fs_alloc(void) {
 REF struct fs *result;
 result = (REF struct fs *)kmalloc(sizeof(struct fs),
                                   GFP_SHARED|GFP_CALLOC);
 result->fs_refcnt = 1;
 atomic_rwlock_cinit(&result->fs_lock);
 result->fs_lnkmax = CONFIG_FS_LNKMAX_DEFAULT;
#if CONFIG_FS_UMASK_DEFAULT != 0
 result->fs_umask = CONFIG_FS_UMASK_DEFAULT;
#endif
 return result;
}

/* Create a deep copy of the current VFS (used when `CLONE_FS' isn't set) */
PUBLIC ATTR_MALLOC ATTR_RETNONNULL
REF struct fs *KCALL fs_clone(void) {
 struct fs *orig = THIS_FS;
 REF struct fs *result;
 unsigned int i;
 result = (REF struct fs *)kmalloc(sizeof(struct fs),
                                   GFP_SHARED|GFP_CALLOC);
 result->fs_refcnt = 1;
 atomic_rwlock_cinit(&result->fs_lock);
 /* Copy fields from the original filesystem context. */
 atomic_rwlock_read(&orig->fs_lock);
 result->fs_vfs    = orig->fs_vfs;
 result->fs_cwd    = orig->fs_cwd;
 result->fs_root   = orig->fs_root;
 memcpy(result->fs_dcwd,orig->fs_dcwd,
        sizeof(REF struct path *)*VFS_DRIVECOUNT);
 result->fs_umask  = orig->fs_umask;
 result->fs_lnkmax = orig->fs_lnkmax;
 result->fs_mode   = orig->fs_mode;
 vfs_incfscount(result->fs_vfs);
 vfs_incref(result->fs_vfs);
 path_incref(result->fs_cwd);
 path_incref(result->fs_root);
 for (i = 0; i < VFS_DRIVECOUNT; ++i) {
  if (!result->fs_dcwd[i]) continue;
  path_incref(result->fs_dcwd[i]);
 }
 atomic_rwlock_endread(&orig->fs_lock);
 return result;
}


/* Destroy a previously allocated FS. */
PUBLIC void KCALL
fs_destroy(struct fs *__restrict self) {
 unsigned int i;
 /* NOTE: Members shouldn't be NULL, but may be
  *       if the FS was only partially constructed. */
 if (self->fs_cwd) path_decref(self->fs_cwd);
 if (self->fs_root) path_decref(self->fs_root);
 for (i = 0; i < VFS_DRIVECOUNT; ++i) {
  if (!self->fs_dcwd[i]) continue;
  path_decref(self->fs_dcwd[i]);
 }
 if (self->fs_vfs) {
  vfs_decfscount(self->fs_vfs);
  vfs_decref(self->fs_vfs);
 }
 kfree(self);
}



/* The kernel's own filesystem / the filesystem used when running `/bin/init'. */
PUBLIC struct vfs vfs_kernel = {
    .v_root = {
        .p_refcnt = 4, /* +1: vfs_kernel
                        * +1: fs_kernel.fs_vfs
                        * +1: fs_kernel.fs_root
                        * +1: fs_kernel.fs_cwd */
        .p_vfs    = &vfs_kernel,
        .p_parent = NULL,
        .p_dirent = NULL,
        .p_lock   = ATOMIC_RWLOCK_INIT,
        .p_node   = NULL,
        .p_mount  = {
            .m_rnode = NULL
        },
        .p_directory = {
            .d_lock = ATOMIC_RWLOCK_INIT,
            .d_child = {
                [0 ... CONFIG_PATH_CHILD_MAPSIZE-1] = NULL
            }
        }
    },
    .v_fscount = 1, /* +1: fs_kernel.fs_vfs */
    .v_recent = {
        .r_lock = ATOMIC_RWLOCK_INIT,
        .r_recent = {
            [0 ... CONFIG_VFS_RECENT_CACHESIZE-1] = NULL
        },
        .r_size = 0
    },
    .v_mount = {
        .m_lock = ATOMIC_RWLOCK_INIT,
        .m_chain = NULL
    }
};
PUBLIC struct fs fs_kernel = {
    .fs_refcnt = 1,
    .fs_vfs    = &vfs_kernel,
    .fs_lock   = ATOMIC_RWLOCK_INIT,
    .fs_root   = &vfs_kernel.v_root,
    .fs_cwd    = &vfs_kernel.v_root,
    .fs_umask  = CONFIG_FS_UMASK_DEFAULT,
    .fs_lnkmax = CONFIG_FS_LNKMAX_DEFAULT,
    .fs_atmask = ~FS_MODE_FALWAYS0MASK,
    .fs_atflag = FS_MODE_FALWAYS1FLAG|FS_MODE_FEMPTY_PATH /* Allow empty paths everywhere by default. */
};


/* [1..1] Per-thread filesystem information. */
PUBLIC ATTR_PERTASK REF struct fs *_this_fs = NULL;

DEFINE_PERTASK_CLONE(clone_task_fs);
INTERN void KCALL
clone_task_fs(struct task *__restrict new_thread, u32 flags) {
 REF struct fs *new_fs;
 if (!(flags & CLONE_FS)) {
  new_fs = fs_clone();
  if (flags & CLONE_NEWNS) {
   /* TODO: Copy the mounting tree & DOS mounting points. */
  }
 } else {
  /* `CLONE_FS' cannot be used with `CLONE_NEWNS' */
  if (flags & CLONE_NEWNS)
      error_throw(E_INVALID_ARGUMENT);
  new_fs = THIS_FS;
  fs_incref(new_fs);
 }
 FORTASK(new_thread,_this_fs) = new_fs; /* Inherit reference. */
}

DEFINE_PERTASK_FINI(fini_task_fs);
INTERN void KCALL fini_task_fs(struct task *__restrict thread) {
 REF struct fs *f = FORTASK(thread,_this_fs);
 /* NOTE: May be NULL if the task wasn't fully constructed. */
 if (f) fs_decref(f);
}

/* Set the current working/root directory of the VFS of the calling thread.
 * NOTE: These functions invalidate certain caches that are
 *       automatically constructed in order to speed up `vfs_path()'. */
PUBLIC ATTR_NOTHROW void KCALL
fs_chdir(struct path *__restrict new_path) {
 REF struct path *old_path;
 struct fs *f = THIS_FS;
 path_incref(new_path);
 /* Exchange the path. */
 atomic_rwlock_write(&f->fs_lock);
 old_path  = f->fs_cwd;
 f->fs_cwd = new_path;
 atomic_rwlock_endwrite(&f->fs_lock);
 path_decref(old_path);
}
PUBLIC ATTR_NOTHROW void KCALL
fs_chroot(struct path *__restrict new_path) {
 REF struct path *old_path;
 struct fs *f = THIS_FS;
 path_incref(new_path);
 /* Exchange the path. */
 atomic_rwlock_write(&f->fs_lock);
 old_path   = f->fs_root;
 f->fs_root = new_path;
 atomic_rwlock_endwrite(&f->fs_lock);
 path_decref(old_path);
}
PUBLIC ATTR_NOTHROW ATTR_RETNONNULL
REF struct path *KCALL fs_getcwd(void) {
 struct fs *f = THIS_FS;
 REF struct path *result;
 atomic_rwlock_read(&f->fs_lock);
 result = f->fs_cwd;
 path_incref(result);
 atomic_rwlock_endread(&f->fs_lock);
 return result;
}
PUBLIC ATTR_NOTHROW ATTR_RETNONNULL
REF struct path *KCALL fs_getroot(void) {
 struct fs *f = THIS_FS;
 REF struct path *result;
 atomic_rwlock_read(&f->fs_lock);
 result = f->fs_root;
 path_incref(result);
 atomic_rwlock_endread(&f->fs_lock);
 return result;
}



/* Path handle operators. */
INTERN size_t KCALL
handle_path_pread(struct path *__restrict self,
                  CHECKED USER void *buf,
                  size_t bufsize, pos_t pos,
                  iomode_t flags) {
 REF struct inode *EXCEPT_VAR node;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 /* Invoke on the node of the path. */
 atomic_rwlock_read(&self->p_lock);
 node = self->p_node;
 inode_incref(node);
 atomic_rwlock_endread(&self->p_lock);
 TRY {
  inode_access(node,R_OK);
  result = inode_read(node,buf,bufsize,pos,flags);
 } FINALLY {
  inode_decref(node);
 }
 return result;
}
INTERN size_t KCALL
handle_path_pwrite(struct path *__restrict self,
                   CHECKED USER void const *buf,
                   size_t bufsize, pos_t pos,
                   iomode_t flags) {
 REF struct inode *EXCEPT_VAR node;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 /* Invoke on the node of the path. */
 atomic_rwlock_read(&self->p_lock);
 node = self->p_node;
 inode_incref(node);
 atomic_rwlock_endread(&self->p_lock);
 TRY {
  inode_access(node,W_OK);
  result = inode_write(node,buf,bufsize,pos,flags);
 } FINALLY {
  inode_decref(node);
 }
 return result;
}
INTERN void KCALL
handle_path_truncate(struct path *__restrict self,
                     pos_t new_smaller_size) {
 REF struct inode *EXCEPT_VAR node;
 /* Invoke on the node of the path. */
 atomic_rwlock_read(&self->p_lock);
 node = self->p_node;
 inode_incref(node);
 atomic_rwlock_endread(&self->p_lock);
 TRY {
  inode_access(node,W_OK);
  inode_truncate(node,new_smaller_size);
 } FINALLY {
  inode_decref(node);
 }
}

PRIVATE u8 const access_matrix[] = {
    [IO_RDONLY]   = R_OK,
    [IO_WRONLY]   = W_OK,
    [IO_RDWR]     = R_OK|W_OK,
    [IO_RDWR_ALT] = R_OK|W_OK
};

INTERN ssize_t KCALL
handle_path_ioctl(struct path *__restrict self,
                  unsigned long cmd,
                  USER UNCHECKED void *arg,
                  iomode_t flags) {
 REF struct inode *EXCEPT_VAR node;
 ssize_t COMPILER_IGNORE_UNINITIALIZED(result);
 /* Invoke on the node of the path. */
 atomic_rwlock_read(&self->p_lock);
 node = self->p_node;
 inode_incref(node);
 atomic_rwlock_endread(&self->p_lock);
 TRY {
  inode_access(node,access_matrix[flags & IO_ACCMODE]);
  result = inode_ioctl(node,cmd,arg,flags);
 } FINALLY {
  inode_decref(node);
 }
 return result;
}
INTERN void KCALL
handle_path_sync(struct path *__restrict self,
                 bool data_only) {
 REF struct inode *EXCEPT_VAR node;
 atomic_rwlock_read(&self->p_lock);
 node = self->p_node;
 inode_incref(node);
 atomic_rwlock_endread(&self->p_lock);
 TRY {
  inode_sync(node,data_only);
 } FINALLY {
  inode_decref(node);
 }
}
INTERN void KCALL
handle_path_stat(struct path *__restrict self,
                 USER CHECKED struct stat64 *result) {
 REF struct inode *EXCEPT_VAR node;
 atomic_rwlock_read(&self->p_lock);
 node = self->p_node;
 inode_incref(node);
 atomic_rwlock_endread(&self->p_lock);
 TRY {
  inode_stat(node,result);
 } FINALLY {
  inode_decref(node);
 }
}
INTERN unsigned int KCALL
handle_path_poll(struct path *__restrict self,
                 unsigned int mode) {
 unsigned int COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct inode *EXCEPT_VAR node;
 atomic_rwlock_read(&self->p_lock);
 node = self->p_node;
 inode_incref(node);
 atomic_rwlock_endread(&self->p_lock);
 TRY {
  result = inode_poll(node,mode);
 } FINALLY {
  inode_decref(node);
 }
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_PATH_C */
