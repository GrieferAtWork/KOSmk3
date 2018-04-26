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
#ifndef GUARD_KERNEL_MODULES_PROCFS_INODE_C
#define GUARD_KERNEL_MODULES_PROCFS_INODE_C 1

#include <hybrid/compiler.h>
#include <fs/node.h>
#include <fs/driver.h>
#include <fs/path.h>
#include <kernel/debug.h>
#include <except.h>
#include <sched/pid.h>

#include "inode.h"

DECL_BEGIN

INTERN ATTR_NORETURN void KCALL ProcFS_FileNotFound(void) {
 error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_FILE_NOT_FOUND);
}


INTERN ATTR_RETNONNULL REF struct thread_pid *KCALL
ProcFS_GetTaskPid(struct superblock *__restrict self, pid_t pid) {
 REF struct thread_pid *COMPILER_IGNORE_UNINITIALIZED(result);
 TRY {
  result = pidns_lookup(self->s_fsdata->pf_pidns,pid);
 } CATCH (E_PROCESS_EXITED) {
  error_info()->e_error.e_code                        = E_FILESYSTEM_ERROR;
  error_info()->e_error.e_filesystem_error.fs_errcode = ERROR_FS_FILE_NOT_FOUND;
  error_rethrow();
 }
 return result;
}
INTERN ATTR_RETNONNULL REF struct task *KCALL
ProcFS_GetTask(struct superblock *__restrict self, pid_t pid) {
 REF struct thread_pid *tpid;
 REF struct task *result;
 /* Lookup the thread PID descriptor for the given PID */
 tpid = ProcFS_GetTaskPid(self,pid);
 /* Extract the task pointer from the PID */
 result = thread_pid_get(tpid);
 thread_pid_decref(tpid);
 /* If the PID is a zombie, throw an error. */
 if unlikely(!result)
    ProcFS_FileNotFound();
 return result;
}

INTERN ATTR_RETNONNULL REF struct vm *KCALL
ProcFS_GetTaskVm(struct superblock *__restrict self, pid_t pid) {
 REF struct vm *COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct task *EXCEPT_VAR thread;
 thread = ProcFS_GetTask(self,pid);
 TRY {
  result = task_getvm(thread);
 } FINALLY {
  task_decref(thread);
 }
 return result;
}

INTERN ATTR_RETNONNULL REF struct application *KCALL
ProcFS_GetTaskApp(struct superblock *__restrict self, pid_t pid) {
 REF struct application *COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct vm *EXCEPT_VAR thread_vm;
 thread_vm = ProcFS_GetTaskVm(self,pid);
 TRY {
  result = vm_apps_primary(thread_vm);
 } FINALLY {
  vm_decref(thread_vm);
 }
 if unlikely(!result)
    ProcFS_FileNotFound();
 return result;
}


#ifndef CONFIG_NO_SMP
PRIVATE void KCALL
RPC_GetThreadFS(REF struct fs **__restrict pfs) {
 *pfs = THIS_FS;
 fs_incref(*pfs);
}
PRIVATE void KCALL
RPC_GetThreadHandleManager(REF struct handle_manager **__restrict pman) {
 *pman = THIS_HANDLE_MANAGER;
 handle_manager_incref(*pman);
}
#endif


INTERN ATTR_RETNONNULL REF struct fs *KCALL
ProcFS_GetTaskFs(struct superblock *__restrict self, pid_t pid) {
 REF struct fs *result;
#ifndef CONFIG_NO_SMP
 REF struct task *EXCEPT_VAR thread;
#else
 REF struct task *thread;
#endif
 pflag_t was;
 was = PREEMPTION_PUSHOFF();
#ifndef CONFIG_NO_SMP
 if (thread->t_cpu != THIS_CPU) {
  PREEMPTION_POP(was);
  TRY {
   /* Although seemingly overkill, this is the only way we
    * can safely ensure that the thread isn't in the middle
    * of changing its currently set FS. */
   if (!task_queue_rpc(thread,(task_rpc_t)&RPC_GetThreadFS,&result,TASK_RPC_SYNC))
        ProcFS_FileNotFound();
  } FINALLY {
   task_decref(thread);
  }
 } else
#endif
 {
  result = FORTASK(thread,_this_fs);
  fs_incref(result);
  PREEMPTION_POP(was);
  task_decref(thread);
 }
 return result;
}
INTERN ATTR_RETNONNULL REF struct handle_manager *KCALL
ProcFS_GetTaskHandleManager(struct superblock *__restrict self, pid_t pid) {
 REF struct handle_manager *result;
#ifndef CONFIG_NO_SMP
 REF struct task *EXCEPT_VAR thread;
#else
 REF struct task *thread;
#endif
 pflag_t was;
 was = PREEMPTION_PUSHOFF();
#ifndef CONFIG_NO_SMP
 if (thread->t_cpu != THIS_CPU) {
  PREEMPTION_POP(was);
  TRY {
   /* Although seemingly overkill, this is the only way we
    * can safely ensure that the thread isn't in the middle
    * of changing its currently set FS. */
   if (!task_queue_rpc(thread,(task_rpc_t)&RPC_GetThreadHandleManager,&result,TASK_RPC_SYNC))
        ProcFS_FileNotFound();
  } FINALLY {
   task_decref(thread);
  }
 } else
#endif
 {
  result = FORTASK(thread,_this_handle_manager);
  handle_manager_incref(result);
  PREEMPTION_POP(was);
  task_decref(thread);
 }
 return result;
}





PRIVATE void KCALL
ProcFS_Open(struct superblock *__restrict self,
            UNCHECKED USER char *args) {
 /* Allocate ProcFS-specific superblock data. */
 self->s_fsdata = (struct superblock_data *)kmalloc(sizeof(struct superblock_data),
                                                    GFP_SHARED);
 self->s_fsdata->pf_pidns          = THIS_PIDNS;
 self->s_root->d_node.i_attr.a_ino = PROCFS_INODE;
 self->s_root->d_node.i_ops        = &Iprocfs_root_dir;
 self->s_root->d_node.i_nlink      = 1;
 self->s_root->d_node.i_flags     |= INODE_FATTRLOADED;
 pidns_incref(self->s_fsdata->pf_pidns);
}

PRIVATE void KCALL
ProcFS_Fini(struct superblock *__restrict self) {
 if (self->s_fsdata) {
  pidns_decref(self->s_fsdata->pf_pidns);
  kfree(self->s_fsdata);
 }
}



PRIVATE void KCALL
ProcFS_OpenNode(struct inode *__restrict node,
                struct directory_node *__restrict parent_directory,
                struct directory_entry *__restrict parent_directory_entry) {
 ino_t ino = node->i_attr.a_ino;
 pid_t pid = PROCFS_INODE_GTPID(ino);
 u8    cls = PROCFS_INODE_GTCLS(ino);
 u32   obj = PROCFS_INODE_GTOBJ(ino);
 node->i_nlink  = 1;
 /* Set special INode flags. */
 node->i_flags |= (INODE_FATTRLOADED);
 TRY {
  switch (cls) {

  case PROCFS_CLASS_FNORMAL:
   if (pid == 0) {
    switch (obj) {

    case PROCFS_INODE:
     node->i_ops = &Iprocfs_root_dir;
     break;

    case PROCFS_INODE_CMDLINE:
     node->i_fsdata = ProcFS_OpenRoText(kernel_driver.d_cmdline,
                                        kernel_driver.d_cmdsize);
     node->i_ops    = &Iprocfs_text_ro;
     break;

    case PROCFS_INODE_SELF:
     node->i_ops = &Iprocfs_self_link;
     break;

    case PROCFS_INODE_THREAD_SELF:
     node->i_ops = &Iprocfs_thread_self_link;
     break;

    default: goto invalid_pid;
    }
   } else {
    switch (obj) {

    case PROCFS_INODE_P:
     node->i_ops = &Iprocfs_p_root_dir;
     break;

    case PROCFS_INODE_P_CMDLINE:
     /* TODO: vm_read() */
     error_throw(E_NOT_IMPLEMENTED);
     break;

    {
     REF struct application *app;
    case PROCFS_INODE_P_EXE:
     app = ProcFS_GetTaskApp(parent_directory->d_node.i_super,
                             pid);
     node->i_fsdata = (struct inode_data *)app->a_module->m_path;
     path_incref((struct path *)node->i_fsdata);
     application_decref(app);
     node->i_ops = &Iprocfs_path_link;
    } break;

    {
     REF struct fs *thread_fs;
    case PROCFS_INODE_P_CWD:
    case PROCFS_INODE_P_ROOT:
     thread_fs = ProcFS_GetTaskFs(parent_directory->d_node.i_super,pid);
     atomic_rwlock_read(&thread_fs->fs_lock);
     node->i_fsdata = (obj == PROCFS_INODE_P_CWD)
                    ? (struct inode_data *)thread_fs->fs_cwd
                    : (struct inode_data *)thread_fs->fs_root;
     path_incref((struct path *)node->i_fsdata);
     atomic_rwlock_endread(&thread_fs->fs_lock);
     fs_decref(thread_fs);
     node->i_ops = &Iprocfs_path_link;
    } break;

    case PROCFS_INODE_P_ENVIRON:
     /* TODO: vm_read() */
     error_throw(E_NOT_IMPLEMENTED);
     break;

    case PROCFS_INODE_P_FD:
     node->i_ops = &Iprocfs_p_fd_dir;
     break;

    case PROCFS_INODE_P_TASK:
     node->i_ops = &Iprocfs_p_task_dir;
     break;

    default: goto invalid_pid;
    }
   }
   break;

  case PROCFS_CLASS_FFD:
   if (pid == 0)
       ProcFS_FileNotFound();
   node->i_ops = &Iprocfs_p_fd_link;
   break;

  default:
invalid_pid:
   error_throw(E_INVALID_ARGUMENT);
   break;
  }
 } CATCH (E_PROCESS_EXITED) {
  error_info()->e_error.e_code                        = E_FILESYSTEM_ERROR;
  error_info()->e_error.e_filesystem_error.fs_errcode = ERROR_FS_FILE_NOT_FOUND;
  error_rethrow();
 }
}


INTERN struct superblock_type procfs_type = {
    .st_name      = "procfs",
    .st_flags     = SUPERBLOCK_TYPE_FNODEV,
    .st_driver    = &this_driver,
    .st_open      = &ProcFS_Open,
    .st_functions = {
        .f_fini     = &ProcFS_Fini,
        .f_opennode = &ProcFS_OpenNode
    }
};

DEFINE_FILESYSTEM_TYPE(procfs_type);

DECL_END

#endif /* !GUARD_KERNEL_MODULES_PROCFS_INODE_C */
