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
#ifndef GUARD_KERNEL_INCLUDE_DEV_TTY_H
#define GUARD_KERNEL_INCLUDE_DEV_TTY_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/timespec.h>
#include <fs/device.h>
#include <fs/ringbuffer.h>
#include <fs/canonbuffer.h>
#include <fs/iomode.h>
#include <sched/rwlock.h>
#include <termios.h>

DECL_BEGIN

struct tty;
struct task_weakref;
struct tty_ops {
    /* [0..1] Finalizer. - Called during `device_destroy' */
    void (KCALL *t_fini)(struct tty *__restrict self);

    /* [1..1][locked(READ(self->t_lock))]
     *  Write data to the terminal (usually to a display)
     *  Data processing has already been done by the caller.
     *  @param: flags: Write mode flags (Usually used for `IO_NONBLOCK')
     *  @return: * : The number of written bytes.
     *  NOTE: This operator must implement the following flags itself:
     *    - ONOCR
     */
    size_t (KCALL *t_write_display)(struct tty *__restrict self,
                                    USER CHECKED void const *buf, size_t bufsize,
                                    iomode_t flags);
    /* [0..1]
     *  If existing, connect to a signal used by the TTY display to
     *  throttle / buffer outgoing data, as written by `t_write_display',
     *  and return true/false indicative of free memory being available right now.
     * (poll for `t_write_display' not blocking)
     *  When not implemented, connect to `self->t_lock' instead. */
    bool (KCALL *t_poll_display)(struct tty *__restrict self);
    /* [0..1][locked(READ(self->t_lock))]
     *  Wait for all data previously written by `t_write_display'
     *  to either finish being sent, or actually appear on-screen,
     *  or alternatively, do whatever is required to flush pending
     *  data to make it appear on-screen.
     *  This operator is called by `fsync()' when invoked on a TTY or PTY handle.
     *  NOTE: PTY terminals implement this by waiting for the SLAVE->MASTER buffer
     *        to become empty (essentially meaning that the master has read everything). */
    void (KCALL *t_sync_display)(struct tty *__restrict self);
    /* [0..1][locked(WRITE(self->t_lock))]
     *  Discard everything cached in the display buffer. */
    void (KCALL *t_discard_display)(struct tty *__restrict self);
    /* [0..1][locked(WRITE(self->t_lock))] Change the display size.
     *  This function may modify `new_size' to its liking
     *  before updating the actual, physical display size.
     *  If display resizing isn't supported, this function
     *  can be left as `NULL'.
     *  If any display size is allowed, this function should
     *  not modify `new_size'.
     *  After returning, `new_size' is saved as the active `t_size'. */
    void (KCALL *t_set_display_size)(struct tty *__restrict self,
                                     struct winsize *__restrict new_size);
};

struct thread_pid;
struct tty {
    struct character_device t_dev;    /* The underlying character device. */
    struct tty_ops         *t_ops;    /* [1..1] TTY Operators. */
    struct canonbuffer      t_can;    /* Canonical input line buffer. */
    struct ringbuffer       t_input;  /* Terminal input buffer. */
    rwlock_t                t_lock;   /* R/W lock for members below. */
    struct winsize          t_size;   /* [lock(t_lock)] Terminal window size. */
    struct termios          t_ios;    /* [lock(t_lock)] termios data. */
    REF struct thread_pid  *t_cproc;  /* [lock(t_lock)][0..1] Controlling process group. */
    REF struct thread_pid  *t_fproc;  /* [lock(t_lock)][0..1] Foreground process group. */
};


/* Increment/decrement the reference counter of the given tty `x' */
#define tty_incref(x) character_device_incref(&(x)->t_dev)
#define tty_decref(x) character_device_decref(&(x)->t_dev)


/* Allocate and pre/null/zero-initialize a new TTY device,
 * who's control structure has a size of `struct_size' bytes.
 * The caller must initialize the following members:
 *    - t_dev.c_device.d_namebuf      (Unless `d_name' is set directly)
 *    - t_dev.c_device.d_name         (Optionally; pre-initialized to `d_namebuf')
 *    - t_dev.c_device.d_devno        (See explanation below)
 *    - t_size                        (Optional; Initialized to POSIX-defined default values)
 *    - t_ios                         (Optional; Initialized to POSIX-defined default values)
 *    - t_cproc                       (Optional; Initialized to `NULL')
 *    - t_fproc                       (Optional; Initialized to `NULL')
 * @throw: E_DRIVER_CLOSED: The given driver has been closed/is closing.
 * Following this, a device ID should be allocated for, or assigned
 * to the device, either by directly setting `return->t_dev.d_device.d_devno',
 * or by calling `devno_alloc()', or `register_dynamic_device()' (see above.) */
FUNDEF ATTR_RETNONNULL REF struct tty *
KCALL __tty_alloc(size_t struct_size,
                  struct tty_ops *__restrict ops,
                  struct driver *__restrict caller)
                  ASMNAME("tty_alloc");
#define tty_alloc(struct_size,ops) \
      __tty_alloc(struct_size,ops,&this_driver)
#define TTY_ALLOC(T,ops) \
  ((T *)tty_alloc(sizeof(T),ops))



/* Read TTY input data, format it, then save it in the given `buf'.
 * NOTE: This function is used to implement the character-device `f_read' operator for TTYs.
 * @return: * : The actual number of bytes read. */
FUNDEF size_t KCALL
tty_read_input(struct tty *__restrict self,
               USER CHECKED void *buf,
               size_t bufsize, iomode_t flags);


/* Write data to the input buffer.
 * NOTE: When in canonical mode, input is written to the canon.
 *       When ECHO is enabled, data is also written to the display
 *       after being formatted according to input flags. */
FUNDEF size_t KCALL
tty_write_input(struct tty *__restrict self,
                USER CHECKED void const *buf,
                size_t bufsize, iomode_t flags);



/* Write the given data to the TTY display adapter after
 * formating it according to the active TTY rules.
 * This function is called when a process using the TTY attempts to write data to it.
 * if `TOSTOP' is set, a `SIGTTOU' is sent to the calling thread if its process isn't part of
 * the TTY's foreground process group, causing `E_INTERRUPT' to be thrown by this function. */
FUNDEF size_t KCALL
tty_write_display(struct tty *__restrict self,
                  USER CHECKED void const *buf,
                  size_t bufsize, iomode_t flags);

/* Same as `tty_write_display()', but omit checks for process group
 * association and always grant write-permissions to the calling thread.
 * This function is used for implementing keyboard ECHO, and called
 * internally by `tty_write_display()' after checks have been done. */
FUNDEF size_t KCALL
tty_dowrite_display(struct tty *__restrict self,
                    USER CHECKED void const *buf,
                    size_t bufsize, iomode_t flags);


FUNDEF ssize_t KCALL
tty_ioctl(struct tty *__restrict self,
          unsigned long cmd, USER UNCHECKED void *arg,
          iomode_t flags);


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_DEV_TTY_H */
