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
#ifndef GUARD_KERNEL_SRC_DEV_PTY_C
#define GUARD_KERNEL_SRC_DEV_PTY_C 1

#include <hybrid/compiler.h>
#include <dev/pty.h>
#include <except.h>
#include <string.h>
#include <stdio.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sys/stat.h>
#include <sys/poll.h>

DECL_BEGIN

#define MKDEV_PTY_MASTER(minor)  MKDEV(2,minor)
#define MKDEV_PTY_SLAVE(minor)   MKDEV(3,minor)


PRIVATE void KCALL
PtySlave_Fini(struct ptyslave *__restrict self) {
 ringbuffer_fini(&self->ps_display);
}
PRIVATE size_t KCALL
PtySlave_WriteDisplay(struct ptyslave *__restrict self,
                      USER CHECKED void const *buf,
                      size_t bufsize, iomode_t flags) {
 return ringbuffer_write(&self->ps_display,buf,bufsize,flags);
}
PRIVATE bool KCALL
PtySlave_PollDisplay(struct ptyslave *__restrict self) {
 return ringbuffer_poll_nonfull(&self->ps_display);
}
PRIVATE void KCALL
PtySlave_SyncDisplay(struct ptyslave *__restrict self) {
 ringbuffer_wait_empty(&self->ps_display,JTIME_INFINITE);
}
PRIVATE void KCALL
PtySlave_DiscardDisplay(struct ptyslave *__restrict self) {
 ringbuffer_discard(&self->ps_display,(size_t)-1);
}

PRIVATE struct tty_ops ptyslave_ops = {
    .t_fini            = (void(KCALL *)(struct tty *__restrict))&PtySlave_Fini,
    .t_write_display   = (size_t(KCALL *)(struct tty *__restrict,USER CHECKED void const *,size_t,iomode_t))&PtySlave_WriteDisplay,
    .t_poll_display    = (bool(KCALL *)(struct tty *__restrict))&PtySlave_PollDisplay,
    .t_sync_display    = (void(KCALL *)(struct tty *__restrict))&PtySlave_SyncDisplay,
    .t_discard_display = (void(KCALL *)(struct tty *__restrict))&PtySlave_DiscardDisplay,
};




PRIVATE void KCALL
PtyMaster_Fini(struct ptymaster *__restrict self) {
 ptyslave_decref(self->pm_slave);
}
PRIVATE ssize_t KCALL
PtyMaster_Ioctl(struct ptymaster *__restrict self,
                unsigned long cmd, USER UNCHECKED void *arg,
                iomode_t flags) {
 return tty_ioctl((struct tty *)self->pm_slave,cmd,arg,flags);
}
PRIVATE void KCALL
PtyMaster_Stat(struct ptymaster *__restrict self,
               USER CHECKED struct stat64 *result) {
 result->st_blocks  = 1;
 result->st_blksize = ATOMIC_READ(self->pm_slave->ps_display.r_limt) & RBUFFER_LIMT_FMASK;
 result->st_size    = ringbuffer_maxread(&self->pm_slave->ps_display);
}
PRIVATE size_t KCALL
PtyMaster_Read(struct ptymaster *__restrict self,
               USER CHECKED void *buf, size_t bufsize,
               iomode_t flags) {
 return ringbuffer_read(&self->pm_slave->ps_display,buf,bufsize,flags);
}
PRIVATE size_t KCALL
PtyMaster_Write(struct ptymaster *__restrict self,
                USER CHECKED void *buf, size_t bufsize,
                iomode_t flags) {
 return tty_write_input((struct tty *)self->pm_slave,buf,bufsize,flags);
}
PRIVATE void KCALL
PtyMaster_Sync(struct ptymaster *__restrict self) {
 uintptr_t old_mask;
 /* Wait until the slave application has read
  * all data, or until the display was closed. */
 old_mask = task_channelmask(RBUF_STATE_CHANNEL_EMPTY);
 TRY {
  for (;;) {
   /* Wait for the slave to consume all input. */
   if (ringbuffer_poll_empty(&self->pm_slave->ps_tty.t_input))
       break;
   /* Wait for the PTY display to get closed. */
   if (ringbuffer_poll_close(&self->pm_slave->ps_display)) {
    task_disconnect();
    break;
   }
   /* Wait for the first of the 2 events above to become set. */
   task_wait();
  }
 } FINALLY {
  task_channelmask(old_mask);
 }
}
PRIVATE unsigned int KCALL
PtyMaster_Poll(struct ptymaster *__restrict self,
               unsigned int mode) {
 unsigned int result = 0;
 if (ringbuffer_isclosed(&self->pm_slave->ps_display))
     result |= POLLHUP;
 if ((mode & POLLIN) &&
      ringbuffer_poll_nonempty(&self->pm_slave->ps_display))
      result |= POLLIN;
 if ((mode & POLLOUT) &&
      ringbuffer_poll_nonfull(&self->pm_slave->ps_tty.t_input))
      result |= POLLOUT;
 return result;
}
PRIVATE pos_t KCALL
PtyMaster_Seek(struct ptymaster *__restrict self, off_t offset, int whence) {
 if (whence != SEEK_CUR)
     error_throw(E_INVALID_ARGUMENT);
 /* EXTENSION: `seek(42,SEEK_CUR)' -> DISCARD(42) (returns the number of discarded bytes) */
 if (offset >= 0)
     return ringbuffer_discard(&self->pm_slave->ps_display,(size_t)offset);
 /* EXTENSION: `seek(-42,SEEK_CUR)' -> UNREAD(42) (returns the number of unread bytes) */
 return ringbuffer_unread(&self->pm_slave->ps_display,(size_t)-offset);
}

PRIVATE struct character_device_ops ptymaster_ops = {
    .c_fini = (void(KCALL *)(struct character_device *__restrict))&PtyMaster_Fini,
    .c_file = {
        .f_ioctl = (ssize_t(KCALL *)(struct character_device *__restrict,unsigned long,USER UNCHECKED void *,iomode_t))&PtyMaster_Ioctl,
        .f_stat  = (void(KCALL *)(struct character_device *__restrict,USER CHECKED struct stat64 *))&PtyMaster_Stat,
        .f_read  = (size_t(KCALL *)(struct character_device *__restrict,USER CHECKED void *,size_t,iomode_t))&PtyMaster_Read,
        .f_write = (size_t(KCALL *)(struct character_device *__restrict,USER CHECKED void const *,size_t,iomode_t))&PtyMaster_Write,
        .f_sync  = (void(KCALL *)(struct character_device *__restrict))&PtyMaster_Sync,
        .f_poll  = (unsigned int(KCALL *)(struct character_device *__restrict,unsigned int))&PtyMaster_Poll,
        .f_seek  = (pos_t(KCALL *)(struct character_device *__restrict,off_t,int))&PtyMaster_Seek,
    }
};


/* Allocate and pre/null/zero-initialize a new PTY master device.
 * The caller must initialize the following members:
 *    - t_dev.c_device.d_namebuf      (Unless `d_name' is set directly)
 *    - t_dev.c_device.d_name         (Optionally; pre-initialized to `d_namebuf')
 *    - t_dev.c_device.d_devno        (See explanation below)
 * Following this, a device ID should be allocated for, or assigned
 * to the device, either by directly setting `return->ps_tty.t_dev.d_device.d_devno',
 * or by calling `devno_alloc()', or `register_dynamic_device()' (see above.) */
PUBLIC ATTR_RETNONNULL ATTR_MALLOC
REF struct ptyslave *KCALL pty_allocslave(void) {
 struct ptyslave *result;
 result = TTY_ALLOC(struct ptyslave,&ptyslave_ops);
 ringbuffer_cinit(&result->ps_display,1024); /* XXX: Some kind of limit? */
 return result;
}
PUBLIC ATTR_RETNONNULL ATTR_MALLOC REF struct ptymaster *
KCALL pty_allocmaster(struct ptyslave *__restrict master) {
 REF struct ptymaster *result;
 result = CHARACTER_DEVICE_ALLOC(struct ptymaster);
 result->pm_slave = master;
 result->pm_dev.c_ops = &ptymaster_ops;
 ptyslave_incref(master);
 return result;
}


PUBLIC void KCALL
pty_register(struct ptymaster *__restrict master,
             struct ptyslave *__restrict slave) {
 struct pty_devpair ids;
 /* Allocate a pair of device IDs. */
again:
 ids = devno_alloc_pty();
 TRY {
  char *name;
  minor_t num = MINOR(ids.dp_master);
  /* Generate names for the devices. */
  slave->ps_tty.t_dev.c_device.d_devno = ids.dp_slave;
  master->pm_dev.c_device.d_devno      = ids.dp_master;
  name = slave->ps_tty.t_dev.c_device.d_namebuf;
  if (num >= 256) {
   sprintf(name,"ptyX%.5I16x",(u16)num);
  } else {
   name[0] = 'p';
   name[1] = 't';
   name[2] = 'y';
   { char temp = 'p'+(num/16);
     if (temp > 'z') temp = 'a'+(temp-'z');
     name[3] = temp;
   }
   num %= 16;
   name[4] = num >= 10 ? 'a'+(num-10) : '0'+num;
   name[5] = '\0';
  }
  memcpy(master->pm_dev.c_device.d_namebuf,name,
         DEVICE_MAXNAME*sizeof(char));
  master->pm_dev.c_device.d_namebuf[0] = 't';
  /* If the device cannot be registers, loop back and allocate another ID pair. */
  if (!register_device_nodevfs((struct device *)master))
       goto again;
  TRY {
   if (!register_device_nodevfs((struct device *)slave)) {
    unregister_device_nodevfs((struct device *)master);
    goto again;
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   unregister_device_nodevfs((struct device *)master);
   error_rethrow();
  }
  master->pm_dev.c_device.d_flags      |= DEVICE_FDYNDEVICE;
  slave->ps_tty.t_dev.c_device.d_flags |= DEVICE_FDYNDEVICE;
  TRY {
   device_add_to_devfs((struct device *)master);
   device_add_to_devfs((struct device *)slave);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* NOTE: If the error happened while registering `slave',
    *       then we can't remove its devfs node, because
    *       that operation may cause another error... */
   unregister_device_nodevfs((struct device *)slave);
   unregister_device_nodevfs((struct device *)master);
   error_rethrow();
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  devno_free(DEVICE_TYPE_FCHARDEV,ids.dp_master,1);
  devno_free(DEVICE_TYPE_FCHARDEV,ids.dp_slave,1);
  error_rethrow();
 }
}


/* Allocate a device number pair for a PTY terminal. */
PUBLIC struct pty_devpair KCALL devno_alloc_pty(void) {
 struct pty_devpair result;
again:
 result.dp_master = devno_alloc_mask(DEVICE_TYPE_FCHARDEV,
                                     MKDEV_PTY_MASTER(0),
                                     MINORMASK,
                                     1);
 result.dp_slave = MKDEV_PTY_SLAVE(MINOR(result.dp_master));
 if (!devno_allocat(DEVICE_TYPE_FCHARDEV,
                    result.dp_slave,
                    1))
      goto again;
 return result;
}


DEFINE_SYSCALL3_64(xopenpty,USER char *,name,
                   USER UNCHECKED struct termios const *,termp,
                   USER UNCHECKED struct winsize const *,winp) {
 u32 fd_master,fd_slave;
 REF struct ptyslave *slave;
 REF struct ptymaster *master;
 validate_readable_opt(termp,sizeof(*termp));
 validate_readable_opt(winp,sizeof(*winp));

 /* Construct a new PTY slave and master. */
 slave = pty_allocslave();
 TRY {
  master = pty_allocmaster(slave);
  TRY {
   /* Copy optional user-space arguments. */
   if (termp) memcpy(&slave->ps_tty.t_ios,termp,sizeof(*termp));
   if (winp) memcpy(&slave->ps_tty.t_size,winp,sizeof(*winp));
   COMPILER_READ_BARRIER();

   /* Register both the master and its slave. */
   pty_register(master,
                slave);
   TRY {
    struct handle hmaster;
    struct handle hslave;
    /* Copy the PTY name if user-space provided a buffer. */
    if unlikely(name) { /* No one really uses this field, because no limit can be specified... */
#if 1 /* Oh yeah! Let's have a long discussion about which version is more correct here */
     sprintf(name,"/dev/%s",master->pm_dev.c_device.d_namebuf);
#else
     sprintf(name,"::/dev/%s",master->pm_dev.c_device.d_namebuf);
#endif
     COMPILER_WRITE_BARRIER();
    }
    /* Create file descriptors for the master and slave. */
    hmaster.h_mode = HANDLE_MODE(HANDLE_TYPE_FDEVICE,IO_RDWR);
    hslave.h_mode  = HANDLE_MODE(HANDLE_TYPE_FDEVICE,IO_RDWR);
    hmaster.h_object.o_device = (struct device *)master;
    hslave.h_object.o_device  = (struct device *)slave;
    fd_master = handle_put(hmaster);
    TRY {
     fd_slave = handle_put(hslave);
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     handle_close(fd_master);
     error_rethrow();
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    unregister_device((struct device *)master);
    unregister_device((struct device *)slave);
    error_rethrow();
   }
  } FINALLY {
   ptymaster_decref(master);
  }
 } FINALLY {
  ptyslave_decref(slave);
 }
 /* Return the packaged master and slave file descriptor handles. */
 return (u64)fd_master | (u64)fd_slave << 32;
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_DEV_PTY_C */
