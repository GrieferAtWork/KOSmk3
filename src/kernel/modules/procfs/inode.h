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
#ifndef GUARD_KERNEL_MODULES_PROCFS_INODE_H
#define GUARD_KERNEL_MODULES_PROCFS_INODE_H 1

#include <hybrid/compiler.h>
#include <fs/node.h>

DECL_BEGIN

#ifdef __DEEMON__
#define DNAME_HASH32(x) \
({ local hash = uint32(0); \
   local temp = (uint32 *)(char *)x; \
   for (local i = 0; i < #(x) / 4; ++i) { hash += temp[i]; hash *= 9; } \
   local temp = x[#(x) - #(x) % 4 : #(x)]; \
   switch (#(temp)) { \
   case 3:  hash += (uint32)temp[2].ord() << 16; \
   case 2:  hash += (uint32)temp[1].ord() << 8; \
   case 1:  hash += (uint32)temp[0].ord(); \
   default: break; \
   } \
   hash; \
})
#define DNAME_HASH64(x) \
({ local hash = uint64(0); \
   local temp = (uint64 *)(char *)x; \
   for (local i = 0; i < #(x) / 8; ++i) { hash += temp[i]; hash *= 9; } \
   local temp = x[#(x) - #(x) % 8 : #(x)]; \
   switch (#(temp)) { \
   case 7:  hash += (uint64)temp[6].ord() << 48; \
   case 6:  hash += (uint64)temp[5].ord() << 40; \
   case 5:  hash += (uint64)temp[4].ord() << 32; \
   case 4:  hash += (uint64)temp[3].ord() << 24; \
   case 3:  hash += (uint64)temp[2].ord() << 16; \
   case 2:  hash += (uint64)temp[1].ord() << 8; \
   case 1:  hash += (uint64)temp[0].ord(); \
   default: break; \
   } \
   hash; \
})
#define STATIC_DIRECTORY(visibility,dirname,dents...) \
({  local entries = dict dents; \
    local hash_mask = 1; \
    while (hash_mask <= #entries) \
           hash_mask = (hash_mask << 1) | 1; \
    local hash_vector32 = [none] * (hash_mask + 1); \
    local hash_vector64 = [none] * (hash_mask + 1); \
    for (local name,args: entries) { \
        local perturb,i; \
        local hash32 = DNAME_HASH32(name); \
        local hash64 = DNAME_HASH64(name); \
        perturb = i = hash32 & hash_mask; \
        for (;;) { \
            local index = i & hash_mask; \
            if (hash_vector32[index] !is none) { \
                i = ((i << 2) + i + perturb + 1); \
                perturb = perturb >> 5; \
            } else { \
                hash_vector32[index] = pack(name,hash32,args...); \
                break; \
            } \
        } \
        perturb = i = hash64 & hash_mask; \
        for (;;) { \
            local index = i & hash_mask; \
            if (hash_vector64[index] !is none) { \
                i = ((i << 2) + i + perturb + 1); \
                perturb = perturb >> 5; \
            } else { \
                hash_vector64[index] = pack(name,hash64,args...); \
                break; \
            } \
        } \
    } \
    function print_hash_items(vector,hash_suffix) { \
        for (local i,data: __builtin_object(0x01F1)(vector)) { \
            if (data is none) continue; \
            local name,hash,tp,ino = data...; \
            print visibility,"DEFINE_DIRECTORY_ENTRY("+dirname+"_"+i+","+repr(name)+","+("0x%I64x" % hash)+hash_suffix+","+tp+","+ino+");"; \
        } \
    } \
    function print_hash_vector(vector) { \
        for (local i,data: __builtin_object(0x01F1)(vector)) { \
            if (data is none) { \
                print "    NULL,"; \
            } else { \
                print "    (struct directory_entry *)&"+dirname+"_"+i+","; \
            } \
        } \
    } \
    print "#if __SIZEOF_POINTER__ == 4"; \
    print_hash_items(hash_vector32,"ul"); \
    print visibility,"struct directory_entry *const "+dirname+"[] = {"; \
    print_hash_vector(hash_vector32); \
    print "};"; \
    print "#else"; \
    print_hash_items(hash_vector64,"ull"); \
    print visibility,"struct directory_entry *const "+dirname+"[] = {"; \
    print_hash_vector(hash_vector64); \
    print "};"; \
    print "#endif"; \
})

#endif /* __DEEMON__ */



/*
 *  PROCFS INode numbers:
 *  xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx 00000000 xxxxxxxx xxxxxxxx xxxxxxxx
 *  [PID..............................] [CLASS.] [PROCESS OBJECT..........]
 *
 *  PID=0 is used for system global file (e.g. `/proc/self' or `/proc/sys/...')
 *  NOTE: `PID' is relative to the PID namespace that was used to create the PROCFS
 *  
 *  CLASS is used to identify larger object ranges, such as file descriptors.
 *        CLASS=0 is used to generic objects. (It is one of `PROCFS_CLASS_F*')
 */
#define PROCFS_INODE_GTPID(ino)                __CCAST(pid_t)((ino) >> 32)
#define PROCFS_INODE_GTCLS(ino)   __CCAST(u8)((__CCAST(u32)(ino) >> 24) & 0xff)
#define PROCFS_INODE_GTOBJ(ino)                __CCAST(u32)((ino) & 0xffffff)
#define PROCFS_INODE_MKINO(pid,cls,obj)      ((__CCAST(ino_t)(pid) << 32) | (__CCAST(ino_t)(cls) << 24) | __CCAST(ino_t)(obj))

#define PROCFS_CLASS_FNORMAL       0x00   /* [*] Regular files (see object codes below) */
#define PROCFS_CLASS_FFD           0x01   /* [l] File descriptors (`obj' is a handle number; s.a. `/proc/[PID]/fd/*') */

#define PROCFS_INODE               0x0000 /* [d] /proc/ */
#define PROCFS_INODE_CMDLINE       0x0001 /* [-] /proc/cmdline */
#define PROCFS_INODE_SELF          0x0002 /* [l] /proc/self */
#define PROCFS_INODE_THREAD_SELF   0x0003 /* [l] /proc/thread-self */

#define PROCFS_INODE_P             0x0000 /* [d] /proc/[PID]/ */
#define PROCFS_INODE_P_CMDLINE     0x0001 /* [-] /proc/[PID]/cmdline */
#define PROCFS_INODE_P_EXE         0x0002 /* [l] /proc/[PID]/exe */
#define PROCFS_INODE_P_CWD         0x0003 /* [l] /proc/[PID]/cwd */
#define PROCFS_INODE_P_ROOT        0x0004 /* [l] /proc/[PID]/root */
#define PROCFS_INODE_P_ENVIRON     0x0005 /* [-] /proc/[PID]/environ */
#define PROCFS_INODE_P_FD          0x0006 /* [d] /proc/[PID]/fd/ */
#define PROCFS_INODE_P_TASK        0x0007 /* [d] /proc/[PID]/task/ */


struct pidns;
struct superblock_data {
    REF struct pidns  *pf_pidns; /* [1..1][const] The PID namespace within which `/proc' operates.
                                  *               While being mounted, this is set to the PID
                                  *               namespace of the calling thread. */
    /* TODO: Cache mapping `pid_t' to `REF struct directory_entry *' for /proc/[PID]/
     *       This cache is optional and makes use of `tp_procfsent'
     * TODO: Couldn't we just add some kind of callback mechanism to
     *      `struct thread_pid' or `struct task' that is invoked when the thread
     *       becomes a zombie? Then we could use that to delete (FCLOSED) Inodes
     *       of dying threads, as those threads die.
     *       If we put it in `struct task', that mechanism could even be used for other
     *       things: `bool task_queue_onexit(struct task *, void (KCALL *)(void *), void *)'.
     *       (Returns `false' is the thread has already terminated and won't execute onexit() anymore)
     */
};


INTDEF struct superblock_type procfs_type;

struct task;
struct thread_pid;
struct vm;
struct application;
struct fs;

/* Lookup a task or some of its sub-components, given it's PID.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_NOT_FOUND:
 *         Translated from `E_PROCESS_EXITED' */
INTDEF ATTR_RETNONNULL REF struct task *KCALL ProcFS_GetTask(struct superblock *__restrict self, pid_t pid);
INTDEF ATTR_RETNONNULL REF struct thread_pid *KCALL ProcFS_GetTaskPid(struct superblock *__restrict self, pid_t pid);
INTDEF ATTR_RETNONNULL REF struct vm *KCALL ProcFS_GetTaskVm(struct superblock *__restrict self, pid_t pid);
INTDEF ATTR_RETNONNULL REF struct application *KCALL ProcFS_GetTaskApp(struct superblock *__restrict self, pid_t pid);
INTDEF ATTR_RETNONNULL REF struct fs *KCALL ProcFS_GetTaskFs(struct superblock *__restrict self, pid_t pid);
INTDEF ATTR_RETNONNULL REF struct handle_manager *KCALL ProcFS_GetTaskHandleManager(struct superblock *__restrict self, pid_t pid);

INTDEF ATTR_NORETURN void KCALL ProcFS_FileNotFound(void);

/* Returns the `/proc/[PID]' directory entry for a given thread. */
INTDEF ATTR_RETNONNULL REF struct directory_entry *KCALL
ProcFS_GetThreadDirent(struct superblock *__restrict self,
                       struct thread_pid *__restrict thread);



INTDEF struct inode_operations Iprocfs_text_ro;
INTDEF struct inode_operations Iprocfs_text_rw;
struct procfs_text_ro_data {
    /* `struct inode_data' for `Iprocfs_text_ro' */
    byte_t const *td_base;  /* [const][1..td_size] Base address of data that can be read. */
    size_t        td_size;  /* [const] Total amount of readable data (in bytes). */
};
struct procfs_text_rw_data {
    /* `struct inode_data' for `Iprocfs_text_ro' */
    byte_t       *td_base; /* [const][1..td_size][owned] Base address of read-write data. */
    size_t        td_size; /* [const] Fixed number of read-write bytes. */
};

INTDEF ATTR_RETNONNULL struct inode_data *KCALL
ProcFS_OpenRoText(void const *data, size_t num_bytes);
/* NOTE: `data' is only inherited on success. */
INTDEF ATTR_RETNONNULL struct inode_data *KCALL
ProcFS_OpenRwText(/*inherit(kfree())*/void *data, size_t num_bytes);


INTDEF struct inode_operations Iprocfs_path_link;        /* [l] ... (`node->i_fsdata' is a `REF struct path *'; this link expands to the string of that path) */
INTDEF struct inode_operations Iprocfs_root_dir;         /* /proc/ */
INTDEF struct inode_operations Iprocfs_self_link;        /* /proc/self */
INTDEF struct inode_operations Iprocfs_thread_self_link; /* /proc/thread-self */
INTDEF struct inode_operations Iprocfs_p_root_dir;       /* /proc/[PID]/ */
INTDEF struct inode_operations Iprocfs_p_fd_dir;         /* /proc/[PID]/fd/ */
INTDEF struct inode_operations Iprocfs_p_fd_link;        /* /proc/[PID]/fd/xxx */
INTDEF struct inode_operations Iprocfs_p_task_dir;       /* /proc/[PID]/task/ */


DECL_END

#endif /* !GUARD_KERNEL_MODULES_PROCFS_INODE_H */
