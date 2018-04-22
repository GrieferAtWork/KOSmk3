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
#ifndef GUARD_KERNEL_MODULES_PROCFS_ROOT_C
#define GUARD_KERNEL_MODULES_PROCFS_ROOT_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <fs/node.h>
#include <fs/path.h>
#include <string.h>

#include "inode.h"

DECL_BEGIN


/*[[[deemon STATIC_DIRECTORY("PRIVATE","root_directory",{
    "cmdline"     : [ "DT_REG", "PROCFS_INODE_CMDLINE" ],
    "self"        : [ "DT_LNK", "PROCFS_INODE_SELF" ],
    "thread-self" : [ "DT_LNK", "PROCFS_INODE_THREAD_SELF" ],
});]]]*/
#if __SIZEOF_POINTER__ == 4
PRIVATE DEFINE_DIRECTORY_ENTRY(root_directory_2,"thread-self",0x26320082ul,DT_LNK,PROCFS_INODE_THREAD_SELF);
PRIVATE DEFINE_DIRECTORY_ENTRY(root_directory_3,"self",0x99cf910bul,DT_LNK,PROCFS_INODE_SELF);
PRIVATE DEFINE_DIRECTORY_ENTRY(root_directory_4,"cmdline",0xcfed46e4ul,DT_REG,PROCFS_INODE_CMDLINE);
PRIVATE struct directory_entry *const root_directory[] = {
    NULL,
    NULL,
    (struct directory_entry *)&root_directory_2,
    (struct directory_entry *)&root_directory_3,
    (struct directory_entry *)&root_directory_4,
    NULL,
    NULL,
    NULL,
};
#else
PRIVATE DEFINE_DIRECTORY_ENTRY(root_directory_0,"self",0x666c6573ull,DT_LNK,PROCFS_INODE_SELF);
PRIVATE DEFINE_DIRECTORY_ENTRY(root_directory_1,"thread-self",0xc98876c916c1879ull,DT_LNK,PROCFS_INODE_THREAD_SELF);
PRIVATE DEFINE_DIRECTORY_ENTRY(root_directory_3,"cmdline",0x656e696c646d63ull,DT_REG,PROCFS_INODE_CMDLINE);
PRIVATE struct directory_entry *const root_directory[] = {
    (struct directory_entry *)&root_directory_0,
    (struct directory_entry *)&root_directory_1,
    NULL,
    (struct directory_entry *)&root_directory_3,
    NULL,
    NULL,
    NULL,
    NULL,
};
#endif
//[[[end]]]




PRIVATE REF struct directory_entry *KCALL
Root_Lookup(struct directory_node *__restrict self,
            CHECKED USER char const *__restrict name,
            u16 namelen, uintptr_t hash, unsigned int mode) {
 struct directory_entry *result;
 u32 perturb,i;
 perturb = i = hash & (COMPILER_LENOF(root_directory)-1);
 for (;; i = ((i << 2) + i + perturb + 1),perturb >>= 5) {
  result = root_directory[i & (COMPILER_LENOF(root_directory)-1)];
  if (!result) break;
  if (result->de_hash != hash) continue;
  if (result->de_namelen != namelen) continue;
  if (memcmp(result->de_name,name,namelen*sizeof(char)) != 0) continue;
found_it:
  /* Found it! */
  directory_entry_incref(result);
  return result;
 }
 /* TODO: Check for PID names. */
 {
  char ch = name[0];
  if (ch >= '1' && ch <= '9') {
   pid_t pid = name[0]-'0';
   for (i = 1; i < namelen; ++i) {
    ch = name[i];
    COMPILER_READ_BARRIER();
    if unlikely(ch < '0' || ch > '9') goto not_a_pid;
    if unlikely(__builtin_mul_overflow(pid,10,&pid))
       goto not_a_pid;
    if unlikely(__builtin_add_overflow(pid,ch-'0',&pid))
       goto not_a_pid;
   }
   /* TODO: Lookup the directory entry of the given PID.
    *       (Use the `struct thread_pid::tp_procfsent' cache) */
  }
 }
not_a_pid:
 if (mode & FS_MODE_FDOSPATH) {
  /* Do another (case-insensitive) search. */
  for (i = 0; i < COMPILER_LENOF(root_directory); ++i) {
   if ((result = root_directory[i]) == NULL) continue;
   if (result->de_namelen != namelen) continue;
   if (memcasecmp(result->de_name,name,namelen*sizeof(char)) != 0) continue;
   goto found_it;
  }
 }
 return NULL;
}


PRIVATE void KCALL
Root_Enum(struct directory_node *__restrict UNUSED(node),
          directory_enum_callback_t callback, void *arg) {
 unsigned int i;
 for (i = 0; i < COMPILER_LENOF(root_directory); ++i) {
  if (!root_directory[i]) continue;
  (*callback)(root_directory[i]->de_name,
              root_directory[i]->de_namelen,
              root_directory[i]->de_type,
              root_directory[i]->de_ino,
              arg);
 }
 /* TODO: Enumerate PIDs of running processes. */
}


INTERN struct inode_operations Iprocfs_root_dir = {
    /* /proc */
    .io_directory = {
        .d_oneshot = {
            .o_lookup = &Root_Lookup,
            .o_enum   = &Root_Enum,
        }
    }
};

DECL_END

#endif /* !GUARD_KERNEL_MODULES_PROCFS_ROOT_C */
