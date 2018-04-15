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
#ifndef GUARD_KERNEL_SRC_FS_HANDLE_C
#define GUARD_KERNEL_SRC_FS_HANDLE_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/align.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <kernel/sections.h>
#include <kernel/vm.h>
#include <sched/task.h>
#include <sched/taskref.h>
#include <bits/sched.h>
#include <kos/fcntl.h>
#include <fs/node.h>
#include <fs/iomode.h>
#include <fs/device.h>
#include <fs/file.h>
#include <fs/linker.h>
#include <fs/path.h>
#include <fs/pipe.h>
#include <fs/handle.h>
#include <string.h>
#include <except.h>
#include <fcntl.h>

DECL_BEGIN

/* Assert that IO_* flags match their user-space O_* counterparts. */
STATIC_ASSERT(IO_ACCMODE  == O_ACCMODE);
STATIC_ASSERT(IO_RDONLY   == O_RDONLY);
STATIC_ASSERT(IO_WRONLY   == O_WRONLY);
STATIC_ASSERT(IO_RDWR     == O_RDWR ||
              IO_RDWR_ALT == O_RDWR);
STATIC_ASSERT(IO_APPEND   == O_APPEND);
STATIC_ASSERT(IO_NONBLOCK == O_NONBLOCK);
STATIC_ASSERT(IO_SYNC     == O_SYNC);
STATIC_ASSERT(IO_ASYNC    == O_ASYNC);
STATIC_ASSERT(IO_DIRECT   == O_DIRECT);

/* Assert that symbol handle values match their user-space counterparts. */
STATIC_ASSERT(HANDLE_SYMBOLIC_CWD             == AT_FDCWD);
STATIC_ASSERT(HANDLE_SYMBOLIC_ROOT            == AT_FDROOT);
STATIC_ASSERT(HANDLE_SYMBOLIC_THIS_TASK       == AT_THIS_TASK);
STATIC_ASSERT(HANDLE_SYMBOLIC_DRIVE_CWD('A')  == AT_FDDRIVE_CWD('A'));
STATIC_ASSERT(HANDLE_SYMBOLIC_DRIVE_CWD('Z')  == AT_FDDRIVE_CWD('Z'));
STATIC_ASSERT(HANDLE_SYMBOLIC_DRIVE_ROOT('A') == AT_FDDRIVE_ROOT('A'));
STATIC_ASSERT(HANDLE_SYMBOLIC_DRIVE_ROOT('Z') == AT_FDDRIVE_ROOT('Z'));


/* Implement incref() / decref() operators for all handle types. */
#define DEFINE_HANDLE_REFERENCE_FUNCTIONS_EX(name,type,incref,decref) \
    INTERN ATTR_NOTHROW void KCALL handle_##name##_incref(type *__restrict self) { incref(self); } \
    INTERN ATTR_NOTHROW void KCALL handle_##name##_decref(type *__restrict self) { decref(self); }
#define DEFINE_HANDLE_REFERENCE_FUNCTIONS(name) \
    DEFINE_HANDLE_REFERENCE_FUNCTIONS_EX(name,struct name,name##_incref,name##_decref)

DEFINE_HANDLE_REFERENCE_FUNCTIONS(device)
DEFINE_HANDLE_REFERENCE_FUNCTIONS(inode)
DEFINE_HANDLE_REFERENCE_FUNCTIONS(file)
DEFINE_HANDLE_REFERENCE_FUNCTIONS(superblock)
DEFINE_HANDLE_REFERENCE_FUNCTIONS(directory_entry)
DEFINE_HANDLE_REFERENCE_FUNCTIONS(path)
DEFINE_HANDLE_REFERENCE_FUNCTIONS(fs)
DEFINE_HANDLE_REFERENCE_FUNCTIONS(module)
DEFINE_HANDLE_REFERENCE_FUNCTIONS(application)
DEFINE_HANDLE_REFERENCE_FUNCTIONS_EX(thread,struct task_weakref,
                                     task_weakref_incref,task_weakref_decref)
DEFINE_HANDLE_REFERENCE_FUNCTIONS(vm)
DEFINE_HANDLE_REFERENCE_FUNCTIONS(vm_region)
DEFINE_HANDLE_REFERENCE_FUNCTIONS(pipe)
DEFINE_HANDLE_REFERENCE_FUNCTIONS(pipereader)
DEFINE_HANDLE_REFERENCE_FUNCTIONS(pipewriter)
#undef DEFINE_HANDLE_REFERENCE_FUNCTIONS
#undef DEFINE_HANDLE_REFERENCE_FUNCTIONS_EX






/* ==================== Handle manager implementation ==================== */
INTERN ATTR_NORETURN void KCALL
throw_invalid_handle(fd_t fd, u16 reason, u16 istype, u16 rqtype, u16 rqkind) {
 struct exception_info *info;
 info = error_info();
 info->e_error.e_code = E_INVALID_HANDLE;
 info->e_error.e_flag = ERR_FNORMAL;
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_invalid_handle.h_handle = fd;
 info->e_error.e_invalid_handle.h_reason = reason;
 info->e_error.e_invalid_handle.h_istype = istype;
 info->e_error.e_invalid_handle.h_rqtype = rqtype;
 info->e_error.e_invalid_handle.h_rqkind = rqkind;
 error_throw_current();
 __builtin_unreachable();
}

PRIVATE ATTR_NORETURN void KCALL throw_fs_error(u16 fs_error_code) {
 struct exception_info *info;
 info = error_info();
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_code                        = E_FILESYSTEM_ERROR;
 info->e_error.e_flag                        = ERR_FNORMAL;
 info->e_error.e_filesystem_error.fs_errcode = fs_error_code;
 error_throw_current();
 __builtin_unreachable();
}





/* Destroy a previously allocated handle_manager. */
PUBLIC void KCALL
handle_manager_destroy(struct handle_manager *__restrict self) {
 unsigned int i,count;
 struct handle *vec = self->hm_vector;
 count = self->hm_alloc;
 for (i = 0; i < count; ++i) {
  if (vec[i].h_type != HANDLE_TYPE_FNONE)
      handle_decref(vec[i]);
 }
 kfree(vec);
 kfree(self);
}

/* The handle manager of the kernel itself. */
PUBLIC struct handle_manager handle_manager_kernel = {
    .hm_refcnt = 1,
    .hm_limit  = CONFIG_HANDLE_MANAGER_DEFAULT_LIMIT,
    .hm_lock   = ATOMIC_RWLOCK_INIT,
    .hm_alloc  = 0,
    .hm_count  = 0,
    .hm_vector = NULL,
};

/* [1..1][const] Handle manager of the calling thread. */
PUBLIC ATTR_PERTASK struct handle_manager *_this_handle_manager = NULL;

DEFINE_PERTASK_FINI(this_handle_manager_fini);
INTERN void KCALL this_handle_manager_fini(struct task *__restrict thread) {
 struct handle_manager *man;
 man = FORTASK(thread,_this_handle_manager);
 /* NOTE: The reference may be NULL if the thread wasn't fully constructed. */
 if (man) handle_manager_decref(man);
}


/* Allocate a new, empty handle manager. */
PUBLIC ATTR_RETNONNULL ATTR_MALLOC
REF struct handle_manager *KCALL handle_manager_alloc(void) {
 REF struct handle_manager *result;
 result = (REF struct handle_manager *)kmalloc(sizeof(struct handle_manager),
                                               GFP_SHARED|GFP_CALLOC);
 result->hm_refcnt = 1;
 result->hm_limit  = CONFIG_HANDLE_MANAGER_DEFAULT_LIMIT;
 atomic_rwlock_cinit(&result->hm_lock);
 return result;
}

/* Clone the current handle manager, but omit all
 * handles with the `IO_HANDLE_FCLOFORK' flag set. */
PUBLIC ATTR_RETNONNULL ATTR_MALLOC
REF struct handle_manager *KCALL handle_manager_clone(void) {
 struct handle_manager *orig = THIS_HANDLE_MANAGER;
 REF struct handle_manager *result;
 unsigned int i,count; struct handle *vector;
 result = (REF struct handle_manager *)kmalloc(sizeof(struct handle_manager),
                                               GFP_SHARED);
 TRY {
  result->hm_refcnt = 1;
  atomic_rwlock_init(&result->hm_lock);
  atomic_rwlock_read(&orig->hm_lock);
  if unlikely(!orig->hm_count) {
   atomic_rwlock_endread(&orig->hm_lock);
   result->hm_limit  = orig->hm_limit;
   result->hm_alloc  = 0;
   result->hm_count  = 0;
   result->hm_vector = NULL;
   goto done;
  }
  vector = NULL;
  for (;;) {
   result->hm_alloc = orig->hm_alloc;
   atomic_rwlock_endread(&orig->hm_lock);
   COMPILER_BARRIER();
   vector = (struct handle *)krealloc(vector,
                                      result->hm_alloc*
                                      sizeof(struct handle),
                                      GFP_SHARED);
   COMPILER_BARRIER();
   atomic_rwlock_read(&orig->hm_lock);
   if (result->hm_alloc >= orig->hm_alloc)
       break;
  }
  memcpy(vector,orig->hm_vector,orig->hm_alloc*sizeof(struct handle));
  if unlikely(result->hm_alloc != orig->hm_alloc) {
   /* Clear handles that we've overallocated. */
   memset(vector+orig->hm_alloc,0,
         (result->hm_alloc-orig->hm_alloc)*sizeof(struct handle));
  }
  /* Now incref() all handles without the
   * `IO_HANDLE_FCLOFORK' flag set, and clear the reset. */
  result->hm_limit  = orig->hm_limit;
  result->hm_count  = orig->hm_count;
  result->hm_vector = vector;
  assert(orig->hm_count <= orig->hm_alloc);
  count = orig->hm_alloc;
  for (i = 0; i < count; ++i) {
   /* Skip unused entries. */
   if (vector[i].h_type == HANDLE_TYPE_FNONE)
       continue;
   if (vector[i].h_flag & IO_HANDLE_FCLOFORK) {
    /* Close this handle. */
    vector[i].h_type = HANDLE_TYPE_FNONE;

    /* Update the number of used handles. */
    assert(result->hm_count);
    --result->hm_count;
   } else {
    /* incref() this handle. */
    handle_incref(vector[i]);
   }
  }
  atomic_rwlock_endread(&orig->hm_lock);
done:
  result->hm_flags = HANDLE_MANAGER_FNORMAL;
  return result;
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(result);
  error_rethrow();
 }
}

DEFINE_PERTASK_CLONE(task_clone_handle_manager);
INTERN void KCALL
task_clone_handle_manager(struct task *__restrict new_thread, u32 flags) {
 if (flags & CLONE_FILES) {
  FORTASK(new_thread,_this_handle_manager) = THIS_HANDLE_MANAGER;
  handle_manager_incref(FORTASK(new_thread,_this_handle_manager));
 } else {
  /* Use a clone of the handler manager for the new thread. */
  FORTASK(new_thread,_this_handle_manager) = handle_manager_clone();
 }
}


/* Close all handles with the `IO_HANDLE_FCLOEXEC'
 * flag set in the current handle manager. */
PUBLIC void KCALL handle_manager_close_exec(void) {
 struct handle_manager *man = THIS_HANDLE_MANAGER;
 unsigned int i; bool did_change;
 do {
  did_change = false;
  atomic_rwlock_write(&man->hm_lock);
  for (i = 0; i < man->hm_alloc; ++i) {
   struct handle hnd;
   if (man->hm_vector[i].h_type == HANDLE_TYPE_FNONE)
       continue;
   if (!(man->hm_vector[i].h_flag & IO_HANDLE_FCLOEXEC))
       continue;
   /* Must close this handle. */
   hnd                      = man->hm_vector[i];
   man->hm_vector[i].h_type = HANDLE_TYPE_FNONE;
   atomic_rwlock_endwrite(&man->hm_lock);
   /* Drop a reference from the handle. */
   handle_decref(hnd);
   /* Remember that something changed.
    * This must be done to prevent some other thread
    * from opening new files until this function returns. */
   did_change = true;
   atomic_rwlock_write(&man->hm_lock);
  }
  atomic_rwlock_endwrite(&man->hm_lock);

 } while (did_change);
}

PRIVATE bool KCALL close_symbolic_handle(fd_t fd);

/* Try to close the handle associated with `fd'.
 * @return: true:  The handle under `fd' was closed.
 * @return: false: No handle was associated with `fd'. */
PUBLIC bool KCALL handle_close(fd_t fd) {
 struct handle_manager *man = THIS_HANDLE_MANAGER;
 struct handle hnd;
 if (fd < 0)
     return close_symbolic_handle(fd);
 atomic_rwlock_write(&man->hm_lock);
 if unlikely((unsigned int)fd >= man->hm_alloc) {
  atomic_rwlock_endwrite(&man->hm_lock);
  return false;
 }
 hnd = man->hm_vector[(unsigned int)fd];
 man->hm_vector[(unsigned int)fd].h_type = HANDLE_TYPE_FNONE;
 atomic_rwlock_endwrite(&man->hm_lock);
 /* NOTE: decref() is a noop for FNONE */
 handle_decref(hnd);
 return hnd.h_type != HANDLE_TYPE_FNONE;
}


/* Duplicate the handle `fd' and return a new descriptor for it.
 * @throw: E_INVALID_HANDLE:   The given `fd' is an invalid handle.
 * @throw: E_TOO_MANY_HANDLES: Too many open handles. */
PUBLIC unsigned int KCALL handle_dup(fd_t fd, iomode_t flags) {
 struct handle hnd;
 unsigned int result;
 hnd = handle_get(fd);
 hnd.h_flag &= ~IO_HANDLE_FMASK;
 hnd.h_flag |= flags;
 TRY {
  result = handle_put(hnd);
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}

/* Similar to `handle_dup()', but duplicate into a file
 * descriptor slot that is greater than, or equal to `hint'.
 * @throw: E_INVALID_HANDLE:   The given `fd' is an invalid handle.
 * @throw: E_TOO_MANY_HANDLES: `hint' is great than the max allowed handle number.
 * @throw: E_TOO_MANY_HANDLES: Too many open handles. */
PUBLIC unsigned int KCALL
handle_dupat(fd_t fd, unsigned int hint, iomode_t flags) {
 struct handle hnd;
 unsigned int result;
 hnd = handle_get(fd);
 hnd.h_flag &= ~IO_HANDLE_FMASK;
 hnd.h_flag |= flags;
 TRY {
  result = handle_putat(hnd,hint);
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}

/* Duplicate `fd' into `dfd'.
 * @throw: E_INVALID_HANDLE: The given `fd' is an invalid handle.
 * @throw: E_INVALID_HANDLE: The given `dfd' is too large. */
PUBLIC void KCALL
handle_dupinto(fd_t fd, fd_t dfd, iomode_t flags) {
 struct handle hnd;
 hnd = handle_get(fd);
 hnd.h_flag &= ~IO_HANDLE_FMASK;
 hnd.h_flag |= flags;
 TRY {
  handle_putinto(dfd,hnd);
 } FINALLY {
  handle_decref(hnd);
 }
}

/* Add the given handle to the handle manager and
 * return the handle number of where it was placed.
 * @throw: E_TOO_MANY_HANDLES: Too many open handles. */
PUBLIC unsigned int KCALL handle_put(struct handle hnd) {
 struct handle *vector; unsigned int result;
 struct handle_manager *man = THIS_HANDLE_MANAGER;
 assert(hnd.h_type != HANDLE_TYPE_FNONE);
 atomic_rwlock_write(&man->hm_lock);
 vector = man->hm_vector;
 if unlikely(man->hm_count == man->hm_alloc) {
  /* Must allocate more vector entries. */
  unsigned int new_alloc = man->hm_alloc * 2;
  if unlikely(!new_alloc) new_alloc = 16;
  if unlikely(new_alloc > man->hm_limit) {
   new_alloc = man->hm_limit;
   if unlikely(new_alloc <= man->hm_alloc) {
    atomic_rwlock_endwrite(&man->hm_lock);
    error_throw(E_TOO_MANY_HANDLES); /* Too many handles. */
   }
  }
#if HANDLE_TYPE_FNONE != 0
#error "ERROR: This krealloc() assumes that `HANDLE_TYPE_FNONE' is ZERO"
#endif
  TRY {
   vector = (struct handle *)krealloc(vector,new_alloc*
                                      sizeof(struct handle),
                                      GFP_SHARED|GFP_CALLOC);
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   /* Try once more time with a minimal increment. */
   new_alloc = man->hm_alloc+1;
   TRY {
    vector = (struct handle *)krealloc(vector,new_alloc*
                                       sizeof(struct handle),
                                       GFP_SHARED|GFP_CALLOC);
   } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
    atomic_rwlock_endwrite(&man->hm_lock);
    error_rethrow();
   }
  }
  /* Save the new vector. */
  man->hm_alloc  = new_alloc;
  man->hm_vector = vector;
 }
 /* Find a suitable location. */
 result = 0;
 /* NOTE: Since we're starting at ZERO(0), we know
  *       that there must be at least one free slot
  *       within the first `man->hm_count+1' entires.
  * NOTE: POSIX requires us to always use the lowest free index. */
 for (; result < man->hm_count; ++result)
     if (vector[result].h_type == HANDLE_TYPE_FNONE)
         break;
 assert(vector[result].h_type == HANDLE_TYPE_FNONE);

 /* Save the given handle into the vector. */
 vector[result] = hnd;
 handle_incref(hnd);

 /* Track the number of allocated handles. */
 ++man->hm_count;
 atomic_rwlock_endwrite(&man->hm_lock);
 return result;
}

PUBLIC unsigned int KCALL
handle_putat(struct handle hnd, unsigned int hint) {
 struct handle *vector; unsigned int result;
 struct handle_manager *man = THIS_HANDLE_MANAGER;
 assert(hnd.h_type != HANDLE_TYPE_FNONE);
 atomic_rwlock_write(&man->hm_lock);
 vector = man->hm_vector;
 if unlikely(hint >= man->hm_alloc) {
  /* Must allocate more vector entries. */
  unsigned int new_alloc = man->hm_alloc * 2;
  if unlikely(!new_alloc) new_alloc = 16;
  if unlikely(hint >= man->hm_limit) {
   /* Invalid hint position. */
   atomic_rwlock_endwrite(&man->hm_lock);
   error_throw(E_TOO_MANY_HANDLES); /* Too many handles. */
  }
  while (new_alloc <= hint) new_alloc *= 2;
  if unlikely(new_alloc > man->hm_limit)
     new_alloc = man->hm_limit;
  assert(new_alloc > hint);
#if HANDLE_TYPE_FNONE != 0
#error "ERROR: This krealloc() assumes that `HANDLE_TYPE_FNONE' is ZERO"
#endif
  TRY {
   vector = (struct handle *)krealloc(vector,new_alloc*
                                      sizeof(struct handle),
                                      GFP_SHARED|GFP_CALLOC);
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   /* Try once more time with a minimal increment. */
   new_alloc = hint+1;
   TRY {
    vector = (struct handle *)krealloc(vector,new_alloc*
                                       sizeof(struct handle),
                                       GFP_SHARED|GFP_CALLOC);
   } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
    atomic_rwlock_endwrite(&man->hm_lock);
    error_rethrow();
   }
  }
  /* Save the new vector. */
  man->hm_alloc  = new_alloc;
  man->hm_vector = vector;
 }
 /* Find a suitable location. */
 result = hint;
 /* NOTE: Since we're starting at ZERO(0), we know
  *       that there must be at least one free slot
  *       within the first `man->hm_count+1' entires.
  * NOTE: POSIX requires us to always use the lowest free index. */
 for (; result < man->hm_alloc; ++result)
     if (vector[result].h_type == HANDLE_TYPE_FNONE)
         break;
 assert(vector[result].h_type == HANDLE_TYPE_FNONE);

 /* Save the given handle into the vector. */
 vector[result] = hnd;
 handle_incref(hnd);

 /* Track the number of allocated handles. */
 ++man->hm_count;
 atomic_rwlock_endwrite(&man->hm_lock);
 return result;
}




/* Place the given handle into the specified handler slot.
 * @throw: E_INVALID_HANDLE: The given `dfd' is too large. */
PUBLIC void KCALL
handle_putinto(fd_t dfd, struct handle hnd) {
 struct handle *vector,old_hnd;
 struct handle_manager *man = THIS_HANDLE_MANAGER;
 assert(hnd.h_type != HANDLE_TYPE_FNONE);
 if unlikely(dfd < 0) {
  /* Special handling for symbolic handle numbers.
   * Using this, KOS implements fchdir(), as well
   * as a way of changing the root directory to
   * a path obtained from a file descriptor. */
  struct fs *my_fs = THIS_FS;
  REF struct path *COMPILER_IGNORE_UNINITIALIZED(old_path);
  REF struct path *new_path;
  if (hnd.h_type == HANDLE_TYPE_FPATH) {
   new_path = hnd.h_object.o_path;
  } else if (hnd.h_type == HANDLE_TYPE_FFILE) {
   new_path = hnd.h_object.o_file->f_path;
  } else {
   throw_invalid_handle(dfd,
                        ERROR_INVALID_HANDLE_FWRONGTYPE,
                        hnd.h_type,
                        HANDLE_TYPE_FPATH,
                        HANDLE_KIND_FANY);
  }

  path_incref(new_path);
  TRY {
   switch (dfd) {

   case HANDLE_SYMBOLIC_CWD:
    /* fchdir() - style */
    atomic_rwlock_write(&my_fs->fs_lock);
    old_path = my_fs->fs_cwd;
    my_fs->fs_cwd = new_path;
    atomic_rwlock_endwrite(&my_fs->fs_lock);
    return;

   case HANDLE_SYMBOLIC_ROOT:
    /* fchroot() - style */
    atomic_rwlock_write(&my_fs->fs_lock);
    old_path = my_fs->fs_root;
    my_fs->fs_root = new_path;
    atomic_rwlock_endwrite(&my_fs->fs_lock);
    return;

   {
    struct path *iter;
    REF struct path *drive_root;
   case HANDLE_SYMBOLIC_DRIVE_CWD('A') ... HANDLE_SYMBOLIC_DRIVE_CWD('Z'):
    /* Check if the new path is located within the associated drive's tree. */
    drive_root = vfs_drive((unsigned int)dfd-
                           (unsigned int)HANDLE_SYMBOLIC_DRIVE_CWD('A'));
    old_path   = new_path;
    iter       = new_path;
    while (iter != drive_root) {
     iter = iter->p_parent;
     if unlikely(!iter) {
      path_decref(drive_root);
      /* The new path wouldn't be reachable from the drive's root. */
      throw_fs_error(ERROR_FS_CROSSDEVICE_LINK);
     }
    }
    path_decref(drive_root);
    atomic_rwlock_write(&my_fs->fs_lock);
    /* Override the drive's current working directory. */
    old_path = my_fs->fs_dcwd[(unsigned int)dfd-
                              (unsigned int)HANDLE_SYMBOLIC_DRIVE_CWD('A')];
    my_fs->fs_dcwd[(unsigned int)dfd-
                   (unsigned int)HANDLE_SYMBOLIC_DRIVE_CWD('A')] = new_path;
    atomic_rwlock_endwrite(&my_fs->fs_lock);
   } break;

   case HANDLE_SYMBOLIC_DRIVE_ROOT('A') ... HANDLE_SYMBOLIC_DRIVE_ROOT('Z'):
    /* Rebind DOS drive roots. */
    old_path = new_path;
    vfs_bind_drive((unsigned int)dfd-
                   (unsigned int)HANDLE_SYMBOLIC_DRIVE_ROOT('A'),
                    new_path);
    break;

   default: break;
   }
  } FINALLY {
   path_decref(old_path);
  }
  {
   u16 reason = ERROR_INVALID_HANDLE_ILLHND_FNOSYM;
   u16 handle_type = HANDLE_TYPE_FPATH;
   switch (dfd) {
   case HANDLE_SYMBOLIC_THIS_TASK:
    reason = ERROR_INVALID_HANDLE_ILLHND_FROSYM;
    handle_type = HANDLE_TYPE_FTHREAD;
    break;
   default: break;
   }
   throw_invalid_handle(dfd,
                        ERROR_INVALID_HANDLE_FUNDEFINED,
                        HANDLE_TYPE_FNONE,
                        handle_type,
                        reason);
  }
 }
 atomic_rwlock_write(&man->hm_lock);
 vector = man->hm_vector;
 if unlikely((unsigned int)dfd >= man->hm_alloc) {
  /* Must allocate more vector entries. */
  unsigned int new_alloc = man->hm_alloc * 2;
  if unlikely(!new_alloc) new_alloc = 16;
  while (new_alloc <= (unsigned int)dfd) new_alloc *= 2;
  if unlikely(new_alloc > man->hm_limit) {
   new_alloc = (unsigned int)dfd+1;
   if unlikely(new_alloc >= man->hm_limit) {
    atomic_rwlock_endwrite(&man->hm_lock);
    /* Handle is too large. */
    throw_invalid_handle(dfd,
                         ERROR_INVALID_HANDLE_FUNDEFINED,
                         HANDLE_TYPE_FNONE,
                         HANDLE_TYPE_FNONE,
                         ERROR_INVALID_HANDLE_ILLHND_FBOUND);
   }
  }
#if HANDLE_TYPE_FNONE != 0
#error "ERROR: This krealloc() assumes that `HANDLE_TYPE_FNONE' is ZERO"
#endif
  TRY {
   assert(new_alloc > man->hm_alloc);
   vector = (struct handle *)krealloc(vector,new_alloc*
                                      sizeof(struct handle),
                                      GFP_SHARED|GFP_CALLOC);
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   /* Try once more time with the minimal size. */
   new_alloc = (unsigned int)dfd+1;
   assert(new_alloc > man->hm_alloc);
   TRY {
    vector = (struct handle *)krealloc(vector,new_alloc*
                                       sizeof(struct handle),
                                       GFP_SHARED|GFP_CALLOC);
   } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
    atomic_rwlock_endwrite(&man->hm_lock);
    error_rethrow();
   }
  }
  /* Save the new vector. */
  man->hm_alloc  = new_alloc;
  man->hm_vector = vector;
 }

 /* Save the given handle into the vector. */
 old_hnd = vector[(unsigned int)dfd];
 vector[(unsigned int)dfd] = hnd;
 handle_incref(hnd);

 /* Track the number of allocated handles. */
 if (old_hnd.h_type != HANDLE_TYPE_FNONE)
     ++man->hm_count;
 atomic_rwlock_endwrite(&man->hm_lock);

 /* Decref() the old handle. */
 handle_decref(old_hnd);
}



PRIVATE REF struct handle KCALL
get_symbolic_handle(fd_t fd) {
 REF struct handle result;
 /* Special, symbolic handles. */
 switch (fd) {

 case HANDLE_SYMBOLIC_CWD:
  result.h_mode          = HANDLE_MODE(HANDLE_TYPE_FPATH,0);
  result.h_object.o_path = fs_getcwd(); /* Inherit reference. */
  break;

 case HANDLE_SYMBOLIC_ROOT:
  result.h_mode          = HANDLE_MODE(HANDLE_TYPE_FPATH,0);
  result.h_object.o_path = fs_getroot(); /* Inherit reference. */
  break;

 case HANDLE_SYMBOLIC_THIS_TASK:
  result.h_mode            = HANDLE_MODE(HANDLE_TYPE_FTHREAD,0);
  result.h_object.o_thread = task_getweakref(THIS_TASK);
  break;

 {
  struct fs *my_fs;
 case HANDLE_SYMBOLIC_DRIVE_CWD('A') ... HANDLE_SYMBOLIC_DRIVE_CWD('Z'):
  my_fs = THIS_FS;
  result.h_mode = HANDLE_MODE(HANDLE_TYPE_FPATH,0);
  atomic_rwlock_read(&my_fs->fs_lock);
  result.h_object.o_path = my_fs->fs_dcwd[(unsigned int)fd-
                                          (unsigned int)HANDLE_SYMBOLIC_DRIVE_CWD('A')];
  if unlikely(!result.h_object.o_path) {
   atomic_rwlock_endread(&my_fs->fs_lock);
unbound_handle:
   throw_invalid_handle(fd,
                        ERROR_INVALID_HANDLE_FUNDEFINED,
                        HANDLE_TYPE_FNONE,
                        HANDLE_TYPE_FPATH,
                        ERROR_INVALID_HANDLE_ILLHND_FRMSYM);
  }
  path_incref(result.h_object.o_path);
  atomic_rwlock_endread(&my_fs->fs_lock);
 } break;

 case HANDLE_SYMBOLIC_DRIVE_ROOT('A') ... HANDLE_SYMBOLIC_DRIVE_ROOT('Z'):
  result.h_mode          = HANDLE_MODE(HANDLE_TYPE_FPATH,0);
  TRY {
   result.h_object.o_path = vfs_drive((u8)((unsigned int)fd-
                                           (unsigned int)HANDLE_SYMBOLIC_DRIVE_ROOT('A')));
  } CATCH (E_FILESYSTEM_ERROR) {
   goto unbound_handle;
  }
  break;


 default:
  throw_invalid_handle(fd,
                       ERROR_INVALID_HANDLE_FUNDEFINED,
                       HANDLE_TYPE_FNONE,
                       HANDLE_TYPE_FNONE,
                       ERROR_INVALID_HANDLE_ILLHND_FNOSYM);
 }
 return result;
}

PRIVATE bool KCALL
close_symbolic_handle(fd_t fd) {
 /* Special, symbolic handles. */
 switch (fd) {

 {
  REF struct path *old_cwd;
  struct fs *my_fs;
 case HANDLE_SYMBOLIC_DRIVE_CWD('A') ... HANDLE_SYMBOLIC_DRIVE_CWD('Z'):
  my_fs = THIS_FS;
  /* Reset the drive's current working directory. */
  atomic_rwlock_write(&my_fs->fs_lock);
  old_cwd = my_fs->fs_dcwd[(unsigned int)fd-
                           (unsigned int)HANDLE_SYMBOLIC_DRIVE_CWD('A')];
  my_fs->fs_dcwd[(unsigned int)fd-
                 (unsigned int)HANDLE_SYMBOLIC_DRIVE_CWD('A')] = NULL;
  atomic_rwlock_endwrite(&my_fs->fs_lock);
  if (!old_cwd) return false;
  path_decref(old_cwd);
  return true;
 } break;

 case HANDLE_SYMBOLIC_DRIVE_ROOT('A') ... HANDLE_SYMBOLIC_DRIVE_ROOT('Z'):
  /* Unbind a VFS drive root. */
  return vfs_unbind_drive(THIS_VFS,
                         (u8)((unsigned int)fd-
                              (unsigned int)HANDLE_SYMBOLIC_DRIVE_ROOT('A')));


 default: break;
 }
 return false;
}


/* Lookup handles, given their FileDescriptor number.
 * The following implicit conversions are performed:
 *    - HANDLE_TYPE_FFILE        --> HANDLE_TYPE_FINODE
 *    - HANDLE_TYPE_FFILE        --> HANDLE_TYPE_FPATH
 *    - HANDLE_TYPE_FPATH        --> HANDLE_TYPE_FINODE
 *    - HANDLE_TYPE_FSUPERBLOCK  --> HANDLE_TYPE_FDEVICE
 *    - HANDLE_TYPE_FDEVICE      --> HANDLE_TYPE_FSUPERBLOCK
 *     (Only for block-devices that have superblock
 *      associated; uses `block_device_getsuper()')
 *    - HANDLE_TYPE_FTHREAD      --> HANDLE_TYPE_FVM
 *    - HANDLE_TYPE_FVM          --> HANDLE_TYPE_FAPPLICATION (The root application)
 *    - HANDLE_TYPE_FAPPLICATION --> HANDLE_TYPE_FMODULE
 * NOTE: Substitution is allowed to recurse, meaning that
 *      `HANDLE_TYPE_FTHREAD --> HANDLE_TYPE_FMODULE' can be done by
 *       going through `HANDLE_TYPE_FVM' and `HANDLE_TYPE_FAPPLICATION'
 * @throw: E_INVALID_HANDLE: The given file descriptor number does not match
 *                           the required type, or isn't associated with a handle.
 */
PUBLIC REF struct handle KCALL handle_get(fd_t fd) {
 REF struct handle result;
 struct handle_manager *man;
 /* Deal with symbolic handles. */
 if (fd < 0)
     return get_symbolic_handle(fd);
 man = THIS_HANDLE_MANAGER;
 atomic_rwlock_read(&man->hm_lock);
 if unlikely((unsigned int)fd >= man->hm_alloc) {
  atomic_rwlock_endread(&man->hm_lock);
  COMPILER_BARRIER();
  goto undefined_handle;
 }
 result = man->hm_vector[(unsigned int)fd];
 /* NOTE: incref() is a no-op on invalid handles. */
 handle_incref(result);
 atomic_rwlock_endread(&man->hm_lock);

 /* Check if this is an invalid handle. */
 if unlikely(result.h_type == HANDLE_TYPE_FNONE) {
undefined_handle:
  throw_invalid_handle(fd,
                       ERROR_INVALID_HANDLE_FUNDEFINED,
                       HANDLE_TYPE_FNONE,
                       HANDLE_TYPE_FNONE,
                       ERROR_INVALID_HANDLE_ILLHND_FUNSET);
 }
 return result;
}



PUBLIC ATTR_RETNONNULL REF struct device *
KCALL handle_get_device(fd_t fd) {
 REF struct device *result;
 struct handle hnd = handle_get(fd);
 if (hnd.h_type == HANDLE_TYPE_FDEVICE)
     return hnd.h_object.o_device;
 if (hnd.h_type == HANDLE_TYPE_FSUPERBLOCK) {
  result = &hnd.h_object.o_superblock->s_device->b_device;
  device_incref(result);
  superblock_decref(hnd.h_object.o_superblock);
  return result;
 }
 handle_decref(hnd);
 throw_invalid_handle(fd,
                      ERROR_INVALID_HANDLE_FWRONGTYPE,
                      hnd.h_type,
                      HANDLE_TYPE_FDEVICE,
                      HANDLE_KIND_FANY);
}
PUBLIC ATTR_RETNONNULL REF struct superblock *
KCALL handle_get_superblock(fd_t fd) {
 REF struct superblock *result;
 struct handle hnd = handle_get(fd);
 if (hnd.h_type == HANDLE_TYPE_FSUPERBLOCK)
     return hnd.h_object.o_superblock;
 if (hnd.h_type == HANDLE_TYPE_FDEVICE &&
     hnd.h_object.o_device->d_type == DEVICE_TYPE_FBLOCKDEV) {
  result = block_device_getsuper(hnd.h_object.o_block_device);
  device_decref(hnd.h_object.o_device);
  return result;
 }
 handle_decref(hnd);
 throw_invalid_handle(fd,
                      ERROR_INVALID_HANDLE_FWRONGTYPE,
                      hnd.h_type,
                      HANDLE_TYPE_FSUPERBLOCK,
                      HANDLE_KIND_FANY);
}
PUBLIC ATTR_RETNONNULL REF struct superblock *
KCALL handle_get_superblock_relaxed(fd_t fd) {
 REF struct superblock *result;
 struct handle hnd = handle_get(fd);
 if (hnd.h_type == HANDLE_TYPE_FSUPERBLOCK)
     return hnd.h_object.o_superblock;
 if (hnd.h_type == HANDLE_TYPE_FINODE) {
  result = hnd.h_object.o_inode->i_super;
  superblock_incref(result);
  inode_decref(hnd.h_object.o_inode);
  return result;
 }
 if (hnd.h_type == HANDLE_TYPE_FFILE) {
  result = hnd.h_object.o_file->f_node->i_super;
  superblock_incref(result);
  file_decref(hnd.h_object.o_file);
  return result;
 }
 if (hnd.h_type == HANDLE_TYPE_FPATH) {
  atomic_rwlock_read(&hnd.h_object.o_path->p_lock);
  result = hnd.h_object.o_path->p_node->i_super;
  superblock_incref(result);
  atomic_rwlock_endread(&hnd.h_object.o_path->p_lock);
  path_decref(hnd.h_object.o_path);
  return result;
 }
 if (hnd.h_type == HANDLE_TYPE_FDEVICE &&
     hnd.h_object.o_device->d_type == DEVICE_TYPE_FBLOCKDEV) {
  result = block_device_getsuper(hnd.h_object.o_block_device);
  device_decref(hnd.h_object.o_device);
  return result;
 }
 handle_decref(hnd);
 throw_invalid_handle(fd,
                      ERROR_INVALID_HANDLE_FWRONGTYPE,
                      hnd.h_type,
                      HANDLE_TYPE_FSUPERBLOCK,
                      HANDLE_KIND_FANY);
}


PUBLIC ATTR_RETNONNULL REF struct inode *
KCALL handle_get_inode(fd_t fd) {
 REF struct inode *result;
 struct handle hnd = handle_get(fd);
 if (hnd.h_type == HANDLE_TYPE_FINODE)
     return hnd.h_object.o_inode;
 if (hnd.h_type == HANDLE_TYPE_FFILE) {
  result = hnd.h_object.o_file->f_node;
  inode_incref(result);
  file_decref(hnd.h_object.o_file);
  return result;
 }
 if (hnd.h_type == HANDLE_TYPE_FPATH) {
  atomic_rwlock_read(&hnd.h_object.o_path->p_lock);
  result = hnd.h_object.o_path->p_node;
  inode_incref(result);
  atomic_rwlock_endread(&hnd.h_object.o_path->p_lock);
  path_decref(hnd.h_object.o_path);
  return result;
 }
 handle_decref(hnd);
 throw_invalid_handle(fd,
                      ERROR_INVALID_HANDLE_FWRONGTYPE,
                      hnd.h_type,
                      HANDLE_TYPE_FINODE,
                      HANDLE_KIND_FANY);
}

PUBLIC ATTR_RETNONNULL REF struct path *
KCALL handle_get_path(fd_t fd) {
 REF struct path *result;
 struct handle hnd = handle_get(fd);
 if (hnd.h_type == HANDLE_TYPE_FPATH)
     return hnd.h_object.o_path;
 if (hnd.h_type == HANDLE_TYPE_FFILE) {
  result = hnd.h_object.o_file->f_path;
  path_incref(result);
  file_decref(hnd.h_object.o_file);
  return result;
 }
 handle_decref(hnd);
 throw_invalid_handle(fd,
                      ERROR_INVALID_HANDLE_FWRONGTYPE,
                      hnd.h_type,
                      HANDLE_TYPE_FPATH,
                      HANDLE_KIND_FANY);
}

FUNDEF ATTR_RETNONNULL REF struct task *KCALL handle_get_task(fd_t fd) {
 REF struct task *result;
 struct handle hnd;
 if (fd == HANDLE_SYMBOLIC_THIS_TASK) {
  /* Special optimization to work around the creation
   * of `task_getweakref()' for the calling thread. */
  result = THIS_TASK;
  task_incref(result);
  return result;
 }
 hnd = handle_get(fd);
 if (hnd.h_type == HANDLE_TYPE_FTHREAD) {
  result = task_weakref_lock(hnd.h_object.o_thread);
  task_weakref_decref(hnd.h_object.o_thread);
  if (!result) error_throw(E_PROCESS_EXITED);
  return result;
 }
 handle_decref(hnd);
 throw_invalid_handle(fd,
                      ERROR_INVALID_HANDLE_FWRONGTYPE,
                      hnd.h_type,
                      HANDLE_TYPE_FTHREAD,
                      HANDLE_KIND_FANY);
}

PUBLIC ATTR_RETNONNULL REF struct vm *
KCALL handle_get_vm(fd_t fd) {
 REF struct vm *result;
 REF struct task *thread;
 struct handle hnd;
 if (fd == HANDLE_SYMBOLIC_THIS_TASK) {
  /* Special optimization to work around the creation
   * of `task_getweakref()' for the calling thread. */
  result = THIS_VM;
  vm_incref(result);
  return result;
 }
 hnd = handle_get(fd);
 if (hnd.h_type == HANDLE_TYPE_FVM)
     return hnd.h_object.o_vm;
 TRY {
  if (hnd.h_type == HANDLE_TYPE_FTHREAD) {
   thread = task_weakref_lock(hnd.h_object.o_thread);
   task_weakref_decref(hnd.h_object.o_thread);
   if (!thread) error_throw(E_PROCESS_EXITED);
   /* Load the VM of the given thread. */
   result = task_getvm(thread);
   task_decref(thread);
   return result; /* XXX: Finally-skip is intended. */
  }
 } FINALLY {
  handle_decref(hnd);
 }
 throw_invalid_handle(fd,
                      ERROR_INVALID_HANDLE_FWRONGTYPE,
                      hnd.h_type,
                      HANDLE_TYPE_FVM,
                      HANDLE_KIND_FANY);
}

PRIVATE REF struct module *KCALL
vm_getmodule(struct vm *__restrict self) {
 REF struct application *app;
 REF struct module *result;
 app = vm_apps_primary(self);
 if (!app) error_throw(E_NO_DATA);
 /* Return a reference to the module. */
 result = app->a_module;
 module_incref(result);
 application_decref(app);
 return result;
}


PUBLIC ATTR_RETNONNULL REF struct application *
KCALL handle_get_application(fd_t fd) {
 REF struct application *result;
 REF struct task *thread;
 struct handle hnd;
 if (fd == HANDLE_SYMBOLIC_THIS_TASK) {
  /* Special optimization to work around the creation
   * of `task_getweakref()' for the calling thread. */
  return vm_apps_primary(THIS_VM);
 }
 hnd = handle_get(fd);
 if (hnd.h_type == HANDLE_TYPE_FAPPLICATION)
     return hnd.h_object.o_application;
 TRY {
  if (hnd.h_type == HANDLE_TYPE_FVM) {
   result = vm_apps_primary(hnd.h_object.o_vm);
   vm_decref(hnd.h_object.o_vm);
   return result; /* XXX: Finally-skip is intended. */
  }
  if (hnd.h_type == HANDLE_TYPE_FTHREAD) {
   REF struct vm *thread_vm;
   thread = task_weakref_lock(hnd.h_object.o_thread);
   task_weakref_decref(hnd.h_object.o_thread);
   if (!thread) error_throw(E_PROCESS_EXITED);
   /* Load the VM of the given thread. */
   thread_vm = task_getvm(thread);
   task_decref(thread);
   TRY {
    result = vm_apps_primary(thread_vm);
   } FINALLY {
    vm_decref(thread_vm);
   }
   return result; /* XXX: Finally-skip is intended. */
  }
 } FINALLY {
  handle_decref(hnd);
 }
 throw_invalid_handle(fd,
                      ERROR_INVALID_HANDLE_FWRONGTYPE,
                      hnd.h_type,
                      HANDLE_TYPE_FAPPLICATION,
                      HANDLE_KIND_FANY);
}
PUBLIC ATTR_RETNONNULL REF struct module *
KCALL handle_get_module(fd_t fd) {
 REF struct module *result;
 REF struct task *thread;
 struct handle hnd;
 if (fd == HANDLE_SYMBOLIC_THIS_TASK) {
  /* Special optimization to work around the creation
   * of `task_getweakref()' for the calling thread. */
  return vm_getmodule(THIS_VM);
 }
 hnd = handle_get(fd);
 if (hnd.h_type == HANDLE_TYPE_FMODULE)
     return hnd.h_object.o_module;
 if (hnd.h_type == HANDLE_TYPE_FAPPLICATION) {
  result = hnd.h_object.o_application->a_module;
  module_incref(result);
  application_decref(hnd.h_object.o_application);
  return result;
 }
 TRY {
  if (hnd.h_type == HANDLE_TYPE_FVM) {
   result = vm_getmodule(hnd.h_object.o_vm);
   vm_decref(hnd.h_object.o_vm);
   return result; /* XXX: Finally-skip is intended. */
  }
  if (hnd.h_type == HANDLE_TYPE_FTHREAD) {
   REF struct vm *thread_vm;
   thread = task_weakref_lock(hnd.h_object.o_thread);
   task_weakref_decref(hnd.h_object.o_thread);
   if (!thread) error_throw(E_PROCESS_EXITED);
   /* Load the VM of the given thread. */
   thread_vm = task_getvm(thread);
   TRY {
    result = vm_getmodule(thread_vm);
   } FINALLY {
    vm_decref(thread_vm);
   }
   task_decref(thread);
   return result; /* XXX: Finally-skip is intended. */
  }
 } FINALLY {
  handle_decref(hnd);
 }
 throw_invalid_handle(fd,
                      ERROR_INVALID_HANDLE_FWRONGTYPE,
                      hnd.h_type,
                      HANDLE_TYPE_FMODULE,
                      HANDLE_KIND_FANY);
}
PUBLIC ATTR_RETNONNULL REF struct pipe *
KCALL handle_get_pipe(fd_t fd) {
 REF struct pipe *result;
 struct handle hnd;
 hnd = handle_get(fd);
 if (hnd.h_type == HANDLE_TYPE_FPIPE)
     return hnd.h_object.o_pipe;
#ifdef CONFIG_PIPEWRITER_MATCHES_PIPEREADER
 if (hnd.h_type == HANDLE_TYPE_FPIPEREADER ||
     hnd.h_type == HANDLE_TYPE_FPIPEWRITER) {
  result = hnd.h_object.o_pipereader->pr_pipe;
  pipe_incref(result);
  pipereader_decref(hnd.h_object.o_pipereader);
  return result;
 }
#else
 if (hnd.h_type == HANDLE_TYPE_FPIPEREADER) {
  result = hnd.h_object.o_pipereader->pr_pipe;
  pipe_incref(result);
  pipereader_decref(hnd.h_object.o_pipereader);
  return result;
 }
 if (hnd.h_type == HANDLE_TYPE_FPIPEWRITER) {
  result = hnd.h_object.o_pipewriter->pw_pipe;
  pipe_incref(result);
  pipewriter_decref(hnd.h_object.o_pipewriter);
  return result;
 }
#endif
 handle_decref(hnd);
 throw_invalid_handle(fd,
                      ERROR_INVALID_HANDLE_FWRONGTYPE,
                      hnd.h_type,
                      HANDLE_TYPE_FPIPE,
                      HANDLE_KIND_FANY);
}


PUBLIC ATTR_RETNONNULL REF void *
KCALL handle_get_typed(fd_t fd, u16 type) {
 struct handle result;
 result = handle_get(fd);
 /* Check if the given type matches the handle's type. */
 if (result.h_type != type) {
  handle_decref(result);
  throw_invalid_handle(fd,
                       ERROR_INVALID_HANDLE_FWRONGTYPE,
                       result.h_type,
                       type,
                       HANDLE_KIND_FANY);
 }
 return result.h_ptr;
}


PUBLIC syscall_slong_t KCALL
handle_fcntl(fd_t fd, unsigned int cmd,
             UNCHECKED USER void *arg) {
 struct handle_manager *hman;
 syscall_slong_t COMPILER_IGNORE_UNINITIALIZED(result);
 switch (cmd) {

 {
  struct handle hnd;
 case FCNTL_KOS_TYPE:
  hnd = handle_get(fd);
  /* Simply return the type of this handle. */
  result = hnd.h_type;
  handle_decref(hnd);
 } break;
  
 case F_DUPFD:
  result = handle_dupat(fd,(unsigned int)(uintptr_t)arg,0);
  break;

 case F_DUPFD_CLOEXEC:
  result = handle_dupat(fd,(unsigned int)(uintptr_t)arg,0);
  break;

 case F_SETFD:
  /* Check if valid flags have been given. */
  arg = (void *)(uintptr_t)IO_HANDLE_FFROM_O((oflag_t)(uintptr_t)arg);
  if ((uintptr_t)arg & ~IO_SETFD_MASK)
      error_throw(E_INVALID_ARGUMENT);
 case F_GETFD:
  hman = THIS_HANDLE_MANAGER;
  atomic_rwlock_read(&hman->hm_lock);
  if unlikely((unsigned int)fd >= hman->hm_alloc ||
               hman->hm_vector[(unsigned int)fd].h_type == HANDLE_TYPE_FNONE) {
   atomic_rwlock_endread(&hman->hm_lock);
   throw_invalid_handle(fd,
                        ERROR_INVALID_HANDLE_FUNDEFINED,
                        HANDLE_TYPE_FNONE,
                        HANDLE_TYPE_FNONE,
                        ERROR_INVALID_HANDLE_ILLHND_FUNSET);
  }
  /* Read, write, or exchange the file flags. */
  if (cmd == F_GETFD) {
   result = IO_HANDLE_FTO_O(ATOMIC_READ(hman->hm_vector[(unsigned int)fd].h_flag));
  } else {
   ATOMIC_WRITE(hman->hm_vector[(unsigned int)fd].h_flag,(u16)(uintptr_t)arg);
  }
  atomic_rwlock_endread(&hman->hm_lock);
  break;

 {
  REF struct handle hnd;
 case F_GETFL:
  hnd = handle_get(fd);
  result = hnd.h_flag & IO_SETFL_MASK;
  handle_decref(hnd);
 } break;

 {
  iomode_t new_mode;
 case F_SETFL:
 case F_SETFL_XCH:
  /* Mask flags that are ignored. */
  new_mode = (iomode_t)IO_FROM_O((oflag_t)(uintptr_t)arg &
                               ~(O_CREAT|O_EXCL|O_NOCTTY|O_TRUNC|
                                 O_RDONLY|O_WRONLY|O_RDWR));
  /* Check for flags that are allowed. */
  if (new_mode & ~IO_SETFL_MASK)
      error_throw(E_INVALID_ARGUMENT);
  hman = THIS_HANDLE_MANAGER;
  atomic_rwlock_read(&hman->hm_lock);
  if unlikely((unsigned int)fd >= hman->hm_alloc ||
               hman->hm_vector[(unsigned int)fd].h_type == HANDLE_TYPE_FNONE) {
   atomic_rwlock_endread(&hman->hm_lock);
   throw_invalid_handle(fd,
                        ERROR_INVALID_HANDLE_FUNDEFINED,
                        HANDLE_TYPE_FNONE,
                        HANDLE_TYPE_FNONE,
                        ERROR_INVALID_HANDLE_ILLHND_FUNSET);
  }
  /* Exchange file flags. */
  result = hman->hm_vector[(unsigned int)fd].h_flag & IO_SETFL_MASK;
  result = IO_TO_O(result);
  hman->hm_vector[(unsigned int)fd].h_flag &= ~IO_SETFL_MASK;
  hman->hm_vector[(unsigned int)fd].h_flag |= new_mode;
  atomic_rwlock_endread(&hman->hm_lock);
  /* Only `F_SETFL_XCH' returns the old set of flags. */
  if (cmd != F_SETFL_XCH)
      result = 0;
 } break;

 {
  REF struct pipe *p;
  size_t old_limit,new_limit;
 case F_SETPIPE_SZ:
  p = handle_get_pipe(fd);
  /* XXX: Limit? */
  do {
   old_limit  = ATOMIC_READ(p->p_buffer.r_limt);
   new_limit  = old_limit & ~RBUFFER_LIMT_FMASK;
   new_limit |= CEIL_ALIGN((size_t)(uintptr_t)arg,(~RBUFFER_LIMT_FMASK)+1);
  } while (!ATOMIC_CMPXCH_WEAK(p->p_buffer.r_limt,old_limit,new_limit));
  pipe_decref(p);
  result = 0;
 } break;

 {
  REF struct pipe *p;
 case F_GETPIPE_SZ:
  p = handle_get_pipe(fd);
  result = (syscall_slong_t)((uintptr_t)p->p_buffer.r_limt &
                              RBUFFER_LIMT_FMASK);
  pipe_decref(p);
 } break;


 default:
  error_throw(E_INVALID_ARGUMENT);
 }
 return result;
}





DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_HANDLE_C */
