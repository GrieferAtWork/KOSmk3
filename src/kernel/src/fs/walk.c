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
#ifndef GUARD_KERNEL_SRC_FS_WALK_C
#define GUARD_KERNEL_SRC_FS_WALK_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/align.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <kernel/sections.h>
#include <fs/node.h>
#include <fs/path.h>
#include <fs/handle.h>
#include <assert.h>
#include <string.h>
#include <except.h>
#include <fcntl.h>
#include <ctype.h>

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












PRIVATE ATTR_RETNONNULL REF struct path *KCALL
dosfs_walk_path_all(struct path *__restrict root,
                    struct path *__restrict pwd,
                    USER CHECKED char const *path,
                    size_t pathlen,
                    u16 *__restrict premaining_links,
                    int flags);
PRIVATE ATTR_RETNONNULL REF struct path *KCALL
fs_walk_path_all(struct path *__restrict root,
                 struct path *__restrict pwd,
                 USER CHECKED char const *path,
                 size_t pathlen,
                 u16 *__restrict premaining_links,
                 int flags);
FORCELOCAL ATTR_RETNONNULL REF struct path *KCALL
dosfs_walk_path_one(struct path *__restrict root,
                    struct path *__restrict pwd,
                    USER CHECKED char const **premaining_path,
                    size_t *premaining_pathlen,
                    u16 *__restrict premaining_links,
                    int flags) {
 REF struct path *COMPILER_IGNORE_UNINITIALIZED(result);
 char const *remaining_path = *premaining_path;
 size_t remaining_pathlen = *premaining_pathlen;
 size_t segment_length,segment_relevant; char ch;
 /* Skip leading slashes and whitespace. */
 while (remaining_pathlen &&
       (ch = *remaining_path,COMPILER_READ_BARRIER(),
        ch == '/' || ch == '\\' || isspace(ch)))
        --remaining_pathlen,++remaining_path;
 for (segment_length = 0;
      segment_length < remaining_pathlen; ++segment_length) {
  ch = remaining_path[segment_length];
  COMPILER_READ_BARRIER();
  if (ch == '/' || ch == '\\') break;
 }
 segment_relevant = segment_length;
 /* Skip trailing whitespace. */
 while (segment_relevant && isspace(remaining_path[segment_relevant-1]))
      --segment_relevant;

 if (!segment_relevant) {
  /* Throw a path-not-found error if empty paths are not allowed. */
  if (!(flags & FS_MODE_FEMPTY_PATH))
        throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
return_this_directory:
  /* Ignore empty segments. */
  path_incref(pwd);
  result = pwd;
 } else {
  /* Check for special names for current/parent directory references. */
  switch (segment_relevant) {
  case 2:
   if (remaining_path[1] != '.') goto default_lookup;
   if (remaining_path[0] != '.') goto default_lookup;
   /* Parent directory. */
   result = pwd->p_parent;
   if (!result || pwd == root)
        result = root;
   path_incref(result);
   break;
  case 1:
   if (remaining_path[0] != '.') goto default_lookup;
   /* Self-directory. */
   goto return_this_directory;
default_lookup:
  default:
   /* Do a regular directory lookup. */
   if unlikely(segment_relevant > 0xffff)
      throw_fs_error(ERROR_FS_FILENAME_TOO_LONG);
   result = path_casechild(pwd,remaining_path,(u16)segment_relevant);
   atomic_rwlock_read(&result->p_lock);
   if (INODE_ISLNK(result->p_node)) {
    REF struct path *new_result;
    REF struct symlink_node *node;
    /* Check if we've got any remaining link tickets. */
    if unlikely(!*premaining_links) {
     atomic_rwlock_endread(&result->p_lock);
     throw_fs_error(ERROR_FS_TOO_MANY_LINKS);
    }
    node = (REF struct symlink_node *)result->p_node;
    inode_incref(&node->sl_node);
    atomic_rwlock_endread(&result->p_lock);
    --*premaining_links;
    TRY {
     struct path *symlink_path;
     /* Make sure that the symlink has been loaded. */
     symlink_node_load(node);
     /* Dereference a symbolic link relative to its directory. */
     symlink_path = result->p_parent;
     if unlikely(!symlink_path)
        symlink_path = result; /* Shouldn't happen, unless the filesystem root is a symlink? */
     new_result = dosfs_walk_path_all(root,
                                      symlink_path,
                                      node->sl_text,
                                     (size_t)node->sl_node.i_attr.a_size,
                                      premaining_links,
                                      flags);
    } FINALLY {
     path_decref(result);
     inode_decref(&node->sl_node);
    }
    /* Use the dereferenced symlink as the new resulting path. */
    result = new_result;
   } else {
    atomic_rwlock_endread(&result->p_lock);
   }
   break;
  }
 }
 /* Skip the slash that specified the scope of this segment (if there was one...). */
 if (remaining_pathlen != segment_length) ++segment_length;
 assert(segment_length != 0);
 remaining_path += segment_length;
 remaining_pathlen -= segment_length;

 *premaining_path = remaining_path;
 *premaining_pathlen = remaining_pathlen;
 return result;
}
FORCELOCAL ATTR_RETNONNULL REF struct path *KCALL
fs_walk_path_one(struct path *__restrict root,
                 struct path *__restrict pwd,
                 USER CHECKED char const **premaining_path,
                 size_t *premaining_pathlen,
                 u16 *__restrict premaining_links,
                 int flags) {
 REF struct path *COMPILER_IGNORE_UNINITIALIZED(result);
 char const *remaining_path = *premaining_path;
 size_t remaining_pathlen = *premaining_pathlen;
 size_t segment_length,segment_relevant; char ch;
 /* Skip leading slashes and whitespace. */
 while (remaining_pathlen &&
       (ch = *remaining_path,ch == '/' || isspace(ch)))
        --remaining_pathlen,++remaining_path;
 segment_length = memlen(remaining_path,'/',remaining_pathlen);
 assert(segment_length <= remaining_pathlen);
 segment_relevant = segment_length;
 /* Skip trailing whitespace. */
 while (segment_relevant && isspace(remaining_path[segment_relevant-1]))
      --segment_relevant;

 if (!segment_relevant) {
  /* Throw a path-not-found error if empty paths are not allowed. */
  if (!(flags & FS_MODE_FEMPTY_PATH))
        throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
return_this_directory:
  /* Ignore empty segments. */
  path_incref(pwd);
  result = pwd;
 } else {
  /* Check for special names for current/parent directory references. */
  switch (segment_relevant) {
  case 2:
   if (remaining_path[1] != '.') goto default_lookup;
   if (remaining_path[0] != '.') goto default_lookup;
   /* Parent directory. */
   result = pwd->p_parent;
   if (!result || pwd == root)
        result = root;
   path_incref(result);
   break;
  case 1:
   if (remaining_path[0] != '.') goto default_lookup;
   goto return_this_directory;
default_lookup:
  default:
   /* Do a regular directory lookup. */
   if unlikely(segment_relevant > 0xffff)
      throw_fs_error(ERROR_FS_FILENAME_TOO_LONG);
   result = path_child(pwd,remaining_path,(u16)segment_relevant);
   atomic_rwlock_read(&result->p_lock);
   if (INODE_ISLNK(result->p_node)) {
    REF struct path *new_result;
    REF struct symlink_node *node;
    /* Check if we've got any remaining link tickets. */
    if unlikely(!*premaining_links) {
     atomic_rwlock_endread(&result->p_lock);
     throw_fs_error(ERROR_FS_TOO_MANY_LINKS);
    }
    node = (REF struct symlink_node *)result->p_node;
    inode_incref(&node->sl_node);
    atomic_rwlock_endread(&result->p_lock);
    --*premaining_links;
    TRY {
     struct path *symlink_path;
     /* Make sure that the symlink has been loaded. */
     symlink_node_load(node);
     /* Dereference a symbolic link relative to its directory. */
     symlink_path = result->p_parent;
     if unlikely(!symlink_path)
        symlink_path = result; /* Shouldn't happen, unless the fileystem root is a symlink? */
     new_result = fs_walk_path_all(root,
                                   symlink_path,
                                   node->sl_text,
                                  (size_t)node->sl_node.i_attr.a_size,
                                   premaining_links,
                                   flags);
    } FINALLY {
     path_decref(result);
     inode_decref(&node->sl_node);
    }
    /* Use the dereferenced symlink as the new resulting path. */
    result = new_result;
   } else {
    atomic_rwlock_endread(&result->p_lock);
   }
   break;
  }
 }
 /* Skip the slash that specified the scope of this segment (if there was one...). */
 if (remaining_pathlen != segment_length) ++segment_length;
 assert(segment_length != 0);
 remaining_path += segment_length;
 remaining_pathlen -= segment_length;

 *premaining_path = remaining_path;
 *premaining_pathlen = remaining_pathlen;
 return result;
}


PRIVATE ATTR_RETNONNULL REF struct path *KCALL
dosfs_walk_path_all(struct path *__restrict root,
                    struct path *__restrict pwd,
                    USER CHECKED char const *path,
                    size_t pathlen,
                    u16 *__restrict premaining_links,
                    int flags) {
 struct path *result = pwd;
 path_incref(result);
 TRY {
  /* Skip leading whitespace. */
  while (pathlen && isspace(path[0])) ++path,--pathlen;
  /* Enumerate the remainder of the path. */
  while (pathlen) {
   REF struct path *new_result;
   new_result = dosfs_walk_path_one(root,
                                    result,
                                   &path,
                                   &pathlen,
                                    premaining_links,
                                    flags);
   path_decref(result);
   result = new_result;
  }
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  path_decref(result);
  error_rethrow();
 }
 return result;
}

PRIVATE ATTR_RETNONNULL REF struct path *KCALL
fs_walk_path_all(struct path *__restrict root,
                 struct path *__restrict pwd,
                 USER CHECKED char const *path,
                 size_t pathlen,
                 u16 *__restrict premaining_links,
                 int flags) {
 struct path *result = pwd;
 path_incref(result);
 TRY {
  /* Skip leading whitespace. */
  while (pathlen && isspace(path[0])) ++path,--pathlen;
  if (pathlen) {
   char ch = path[0];
   COMPILER_READ_BARRIER();
   if (ch == ':' && pathlen >= 3 &&
       path[1] == ':' && path[2] == '/') {
    /* Compatibility with DOS-style universal absolute path:
     * >> open("::/foo/bar"); // Opens `/foo/bar', regardless of filesystem mode. */
    path += 2,pathlen -= 2;
    goto use_root;
   }
   if (ch == '/') {
use_root:
    /* Use the root directory as starting point, rather than the current. */
    path_decref(result);
    result = root;
    path_incref(result);

    /* Skip additional whitespace and slashes. */
    do ++path,--pathlen;
    while (pathlen && (path[0] == '/' || isspace(path[0])));
   }
  }
  /* Enumerate the remainder of the path. */
  while (pathlen) {
   REF struct path *new_result;
   new_result = fs_walk_path_one(root,
                                 result,
                                &path,
                                &pathlen,
                                 premaining_links,
                                 flags);
   path_decref(result);
   result = new_result;
  }
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  path_decref(result);
  error_rethrow();
 }
 return result;
}



PRIVATE ATTR_RETNONNULL REF struct path *KCALL
dosfs_path(struct path *cwd, USER CHECKED char const *path,
           size_t pathlen, REF struct inode **pnode, int flags) {
 struct fs *f = THIS_FS; u16 max_links;
 REF struct path *root,*COMPILER_IGNORE_UNINITIALIZED(result);
 bool need_directory = !!(flags & FS_MODE_FDIRECTORY);
 /* Strip leading whitespace. */
 while (pathlen && isspace(path[0]))
    ++path,--pathlen;
 /* Handle drive letters. */
 if (pathlen >= 2 && path[1] == ':') {
  struct path *temp;
  char drive_letter;
  /* Strip trailing whitespace. */
  while (pathlen > 2 && isspace(path[pathlen-1]))
      --pathlen;
  /* The first character is a drive letter. */
  drive_letter = path[0];
  COMPILER_READ_BARRIER();
  drive_letter = toupper(drive_letter);
  /* Validate that it really is a drive letter. */
  if (drive_letter < 'A' || drive_letter > 'Z') {
   if (drive_letter == ':') {
    /* Posix-absolute path. (Used for universal path names) */
    path    += 2;
    pathlen -= 2;
    goto do_path;
   }
   throw_fs_error(ERROR_FS_PATH_NOT_FOUND);
  }
  if (pathlen == 2) {
   /* Special case: `C:' -- Return the current working directory of the specified drive. */
   atomic_rwlock_read(&THIS_FS->fs_lock);
   result = THIS_FS->fs_dcwd[(u8)(drive_letter - 'A')];
   if (result) {
    path_incref(result);
    atomic_rwlock_endread(&THIS_FS->fs_lock);
    goto got_result;
   }
   atomic_rwlock_endread(&THIS_FS->fs_lock);
   /* No CWD set for the drive. -> Fallback to using the drives ROOT. */
   result = vfs_drive((u8)(drive_letter - 'A'));
   goto got_result;
  }
  /* Skip the drive letter prefix. */
  path    += 2;
  pathlen -= 2;

  /* Lookup the drive root directory. */
  root = vfs_drive((u8)(drive_letter - 'A'));

  atomic_rwlock_read(&f->fs_lock);
validate_drive_root:
  max_links = f->fs_lnkmax;
  temp = root;
  /* Verify that the drive's root is located inside the POSIX-chroot tree. */
  while (temp != f->fs_root) {
   temp = temp->p_parent;
   if (!temp) {
    /* It isn't... (Force the posix chroot()-directory as path-relative root folder) */
    temp = f->fs_root;
    path_incref(temp);
    atomic_rwlock_endread(&f->fs_lock);
    path_decref(root);
    root = temp;
    goto use_root;
   }
  }
  atomic_rwlock_endread(&f->fs_lock);
use_root:
  cwd = root;
  path_incref(cwd);
 } else {
do_path:
  if (pathlen) {
   char ch = path[0];
   COMPILER_READ_BARRIER();
   if (ch == '/' || ch == '\\') {
    /* Skip additional leading slashes and whitespace. */
    for (;;) {
     ++path,--pathlen;
     ch = path[0];
     COMPILER_READ_BARRIER();
     if (ch != '/' && ch != '\\' && !isspace(ch)) break;
    }
    /* The path begins with a leading slash.
     * This must be interpreted as a path relative to the current drive's root. */
    atomic_rwlock_read(&f->fs_lock);
    root = f->fs_cwd;
    while (!(root->p_flags & PATH_FISDRIVE) &&
             root->p_parent) root = root->p_parent;
    path_incref(root);
    /* Verify that the drive's root is located inside the POSIX-chroot tree. */
    goto validate_drive_root;
   }
  }
  /* Just a regular, old relative path. */
  /* Extract lookup rules. */
  atomic_rwlock_read(&f->fs_lock);
  if (!cwd) cwd = f->fs_cwd;
  root = f->fs_root;
  max_links = f->fs_lnkmax;
  path_incref(root);
  path_incref(cwd);
  atomic_rwlock_endread(&f->fs_lock);
 }

 /* Don't allow any symbolic links when `FS_MODE_FSYMLINK_NOFOLLOW' is set. */
 if (flags & FS_MODE_FSYMLINK_NOFOLLOW)
     max_links = 0;

 /* Walk the path. */
 TRY {
  /* Strip trailing whitespace and check if the path ends with a slash.
   * If it does, the caller needs this path to be a directory. */
  while (pathlen) {
   char ch = path[pathlen-1];
   if (ch == '/' || ch == '\\') need_directory = true;
   else if (!isspace(ch)) break;
   --pathlen;
  }
  result = dosfs_walk_path_all(root,
                               cwd,
                               path,
                               pathlen,
                              &max_links,
                               flags);
 } FINALLY {
  path_decref(cwd);
  path_decref(root);
 }
got_result:
 if (need_directory) {
  atomic_rwlock_read(&result->p_lock);
  if (!INODE_ISDIR(result->p_node)) {
   atomic_rwlock_endread(&result->p_lock);
   path_decref(result);
   /* We needed a directory... */
   throw_fs_error(ERROR_FS_NOT_A_DIRECTORY);
  }
  if (pnode) {
   *pnode = result->p_node;
   inode_incref(*pnode);
  }
  atomic_rwlock_endread(&result->p_lock);
 } else if (pnode) {
  /* Always save the resulting node if the caller provides a buffer. */
  atomic_rwlock_read(&result->p_lock);
  *pnode = result->p_node;
  inode_incref(*pnode);
  atomic_rwlock_endread(&result->p_lock);
 }
 return result;
}

/* Return a reference for a given path.
 * Traverse a given path, using the current CWD as starting point:
 * >> directory_traverse(my_directory,"foo /bar ///baz/../xy");
 * This function handles multiple consecutive slashes, as well as
 * whitespace surrounding them, as well as following symbolic links.
 * NOTE: This function also deals with root-directory
 *       referencing causing by leading slashes.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_PATH_NOT_FOUND:       [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_NOT_A_DIRECTORY:      [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_TOO_MANY_LINKS:       [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILENAME_TOO_LONG:    [...] */
PUBLIC ATTR_RETNONNULL REF struct path *KCALL
fs_path(struct path *cwd, USER CHECKED char const *path,
        size_t pathlen, REF struct inode **pnode, int flags) {
 struct fs *f = THIS_FS; u16 max_links;
 REF struct path *root,*COMPILER_IGNORE_UNINITIALIZED(result);
 bool need_directory;
 if (flags & FS_MODE_FDOSPATH)
     return dosfs_path(cwd,path,pathlen,pnode,flags);

 need_directory = !!(flags & FS_MODE_FDIRECTORY);

 /* Extract lookup rules. */
 atomic_rwlock_read(&f->fs_lock);
 if (!cwd) cwd = f->fs_cwd;
 root = f->fs_root;
 max_links = f->fs_lnkmax;
 path_incref(root);
 path_incref(cwd);
 atomic_rwlock_endread(&f->fs_lock);

 /* Don't allow any symbolic links when `FS_MODE_FSYMLINK_NOFOLLOW' is set. */
 if (flags & FS_MODE_FSYMLINK_NOFOLLOW)
     max_links = 0;

 /* Walk the path. */
 TRY {
  /* Strip trailing whitespace and check if the path ends with a slash.
   * If it does, the caller needs this path to be a directory. */
  while (pathlen) {
   char ch = path[pathlen-1];
   if (ch == '/') {
    /* Special case: root directory */
    while (pathlen > 1 && isspace(path[0]))
        ++path,--pathlen;
    if (pathlen == 1) {
     result = root;
     path_incref(result);
     goto got_result;
    }
    need_directory = true;
   } else {
    if (!isspace(ch)) break;
   }
   --pathlen;
  }
  result = fs_walk_path_all(root,
                            cwd,
                            path,
                            pathlen,
                           &max_links,
                            flags);
got_result:;
 } FINALLY {
  path_decref(cwd);
  path_decref(root);
 }
 if (need_directory) {
  atomic_rwlock_read(&result->p_lock);
  if (!INODE_ISDIR(result->p_node)) {
   atomic_rwlock_endread(&result->p_lock);
   path_decref(result);
   /* We needed a directory... */
   throw_fs_error(ERROR_FS_NOT_A_DIRECTORY);
  }
  if (pnode) {
   *pnode = result->p_node;
   inode_incref(*pnode);
  }
  atomic_rwlock_endread(&result->p_lock);
 } else if (pnode) {
  /* Always save the resulting node if the caller provides a buffer. */
  atomic_rwlock_read(&result->p_lock);
  *pnode = result->p_node;
  inode_incref(*pnode);
  atomic_rwlock_endread(&result->p_lock);
 }
 return result;
}


PRIVATE ATTR_NOINLINE ATTR_RETNONNULL REF struct path *KCALL
dosfs_lastpath(struct path *cwd, USER CHECKED char const **__restrict ppath,
               size_t *__restrict ppathlen, REF struct inode **pnode, int flags) {
 size_t pathlen = *ppathlen;
 USER CHECKED char const *path = *ppath;
 char *path_end,temp;
search_end:
 path_end = (char *)(path+pathlen);
 for (;;) {
  /* Special case: Without a slash, the path is relative to the current working directory. */
  if (path_end == path) {
   /* Strip whitespace surrounding the filename. */
   while (pathlen && isspace(*path)) ++path,--pathlen;
   while (pathlen && isspace(path[pathlen-1])) --pathlen;
   /* Check that the last path segment isn't too long. */
   if unlikely(pathlen > 0xffff)
      throw_fs_error(ERROR_FS_FILENAME_TOO_LONG);
   *ppath    = path;
   *ppathlen = pathlen;
   /* Return the current working directory. */
   if (!cwd) return fs_getcwd();
   path_incref(cwd);
   return cwd;
  }
  temp = *--path_end;
  if (temp == '/' || temp == '\\') break;
 }
 ++path_end;
 {
  char  *last_segment = path_end;
  size_t last_segsize = (size_t)((path+pathlen)-path_end);
  /* Strip whitespace from the last segment. */
  while (last_segsize && isspace(*last_segment)) ++last_segment,--last_segsize;
  while (last_segsize && isspace(last_segment[last_segsize-1])) ++last_segment,--last_segsize;
  if (!last_segment && (flags&FS_MODE_FIGNORE_TRAILING_SLASHES)) {
   /* Empty last segment. - When trailing slashes should be ignored,
    * use the previous last segment instead. */
   pathlen = (size_t)(path_end-path);
   /* Skip additional spaces and slashes. */
   while (pathlen) {
    char ch = path[pathlen-1];
    if ((ch == '/' || ch == '\\') || isspace(ch)) --pathlen;
   }
   /* Search for the last slash within the remaining path. */
   goto search_end;
  }
  /* Check that the last path segment isn't too long. */
  if unlikely(last_segsize > 0xffff)
     throw_fs_error(ERROR_FS_FILENAME_TOO_LONG);
  /* Save the remaining path as the last segment. */
  *ppath    = last_segment;
  *ppathlen = last_segsize;
 }
 return fs_path(cwd,path,(size_t)(path_end-path),pnode,flags);
}


/* Similar to `fs_path()', but stop just before the last segment and
 * instead update `*ppath' and `**ppathlen' to point to said segment:
 * >> fs_lastpath("/foo/bar/foobar")
 * RETURN: fs_path("/foo/bar")
 * *ppath: "foobar" */
PUBLIC ATTR_RETNONNULL REF struct path *KCALL
fs_lastpath(struct path *cwd, USER CHECKED char const **__restrict ppath,
            size_t *__restrict ppathlen, REF struct inode **pnode, int flags) {
 size_t pathlen = *ppathlen;
 USER CHECKED char const *path = *ppath;
 char *path_end;
 if (flags & FS_MODE_FDOSPATH)
     return dosfs_lastpath(cwd,ppath,ppathlen,pnode,flags);

search_end:
 path_end = (char *)memrchr(path,'/',pathlen);
 /* Special case: Without a slash, the path is relative to the current working directory. */
 if (!path_end) {
  /* Strip whitespace surrounding the filename. */
  while (pathlen && isspace(*path)) ++path,--pathlen;
  while (pathlen && isspace(path[pathlen-1])) --pathlen;
  /* Check that the last path segment isn't too long. */
  if unlikely(pathlen > 0xffff)
     throw_fs_error(ERROR_FS_FILENAME_TOO_LONG);
  *ppath    = path;
  *ppathlen = pathlen;
  /* Return the current working directory. */
  if (!cwd) return fs_getcwd();
  path_incref(cwd);
  return cwd;
 }
 ++path_end;
 {
  char  *last_segment = path_end;
  size_t last_segsize = (size_t)((path+pathlen)-path_end);
  /* Strip whitespace from the last segment. */
  while (last_segsize && isspace(*last_segment)) ++last_segment,--last_segsize;
  while (last_segsize && isspace(last_segment[last_segsize-1])) ++last_segment,--last_segsize;
  if (!last_segment && (flags&FS_MODE_FIGNORE_TRAILING_SLASHES)) {
   /* Empty last segment. - When trailing slashes should be ignored,
    * use the previous last segment instead. */
   pathlen = (size_t)(path_end-path);
   /* Skip additional spaces and slashes. */
   while (pathlen) {
    char ch = path[pathlen-1];
    if (ch == '/' || isspace(ch)) --pathlen;
   }
   /* Search for the last slash within the remaining path. */
   goto search_end;
  }
  /* Check that the last path segment isn't too long. */
  if unlikely(last_segsize > 0xffff)
     throw_fs_error(ERROR_FS_FILENAME_TOO_LONG);
  /* Save the remaining path as the last segment. */
  *ppath    = last_segment;
  *ppathlen = last_segsize;
 }
 return fs_path(cwd,path,(size_t)(path_end-path),pnode,flags);
}


/* Same as `fs_path()', but use the CWD associated with the handle `dfd' */
PUBLIC ATTR_RETNONNULL REF struct path *KCALL
fs_pathat(fd_t dfd, USER CHECKED char const *path,
          size_t pathlen, REF struct inode **pnode, int flags) {
 REF struct path *cwd,*result;
 /* Load the base directory that should be used.
  * NOTE: When `HANDLE_SYMBOLIC_CWD' is used for `dfd',
  *       let `fs_path()' load the workind directory so
  *       it can atomically ensure that CWD and ROOT have
  *       been loaded during the same lock instance.
  * Also: That way, this is optimized for the common case
  *       of user-space simply passing its current working
  *       directory as base path in *at system calls. */
 cwd = dfd == HANDLE_SYMBOLIC_CWD ? NULL : handle_get_path(dfd);
 TRY {
  result = fs_path(cwd,path,pathlen,pnode,flags);
 } FINALLY {
  if (cwd)
      path_decref(cwd);
 }
 return result;
}


PUBLIC ATTR_RETNONNULL REF struct path *KCALL
fs_lastpathat(fd_t dfd, USER CHECKED char const **__restrict ppath,
              size_t *__restrict ppathlen, REF struct inode **pnode, int flags) {
 REF struct path *cwd,*result;
 /* Load the base directory that should be used.
  * NOTE: When `HANDLE_SYMBOLIC_CWD' is used for `dfd',
  *       let `fs_path()' load the workind directory so
  *       it can atomically ensure that CWD and ROOT have
  *       been loaded during the same lock instance.
  * Also: That way, this is optimized for the common case
  *       of user-space simply passing its current working
  *       directory as base path in *at system calls. */
 cwd = dfd == HANDLE_SYMBOLIC_CWD ? NULL : handle_get_path(dfd);
 TRY {
  result = fs_lastpath(cwd,ppath,ppathlen,pnode,flags);
 } FINALLY {
  if (cwd)
      path_decref(cwd);
 }
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_WALK_C */
