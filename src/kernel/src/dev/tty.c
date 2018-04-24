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
#ifndef GUARD_KERNEL_SRC_DEV_TTY_C
#define GUARD_KERNEL_SRC_DEV_TTY_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/timespec.h>
#include <kernel/sections.h>
#include <kernel/user.h>
#include <dev/tty.h>
#include <sched/taskref.h>
#include <sched/group.h>
#include <sched/posix_signals.h>
#include <ctype.h>
#include <string.h>
#include <fs/iomode.h>
#include <except.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched/pid.h>

DECL_BEGIN

PRIVATE size_t KCALL
tty_dowrite_display_impl(struct tty *__restrict self,
                         USER CHECKED void const *buf,
                         size_t bufsize, iomode_t flags) {
 tcflag_t oflags = ATOMIC_READ(self->t_ios.c_oflag);
 if (!(oflags & OPOST)) {
  if unlikely(!(oflags & (OLCUC|ONLCR|OCRNL|ONLRET)))
   ATOMIC_CMPXCH(self->t_ios.c_oflag,oflags,oflags & ~OPOST);
  else {
   /* Process output. */
   char const *flush_start,*iter,*end;
   size_t temp2,temp,result = 0;
   flush_start = iter = (char const *)buf;
   end = iter+bufsize;
   for (; iter != end; ++iter) {
    char ch = *iter;
    switch (ch) {

    case '\n':
     if (oflags & ONLCR) {
      PRIVATE char const crlf[] = { '\r', '\n' };
      if (iter != flush_start) {
       temp    = (size_t)(iter-flush_start);
       temp2   = (*self->t_ops->t_write_display)(self,flush_start,temp,flags);
       result += temp2;
       if (temp2 != temp)
           goto done;
       flush_start = iter+1;
      }
      temp = (*self->t_ops->t_write_display)(self,crlf,2,flags);
      result += temp;
      if (temp < 2) goto done;
     }
     break;

    case '\r':
     if (oflags & ONLRET) {
      if (iter != flush_start) {
       temp    = (size_t)(iter-flush_start);
       temp2   = (*self->t_ops->t_write_display)(self,flush_start,temp,flags);
       result += temp2;
       if (temp2 != temp)
           goto done;
       flush_start = iter+1;
      }
      break;
     }
     if (oflags & OCRNL) {
      ch = '\n';
      goto print_ch;
     }
     break;

    default:
     if ((oflags & OLCUC) && islower(ch)) {
      /* Convert lower-case to uppercase */
      ch = toupper(ch);
print_ch:
      if (iter != flush_start) {
       temp    = (size_t)(iter-flush_start);
       temp2   = (*self->t_ops->t_write_display)(self,flush_start,temp,flags);
       result += temp2;
       if (temp2 != temp)
           goto done;
       flush_start = iter+1;
      }
      temp = (*self->t_ops->t_write_display)(self,&ch,1,flags);
      if (!temp) goto done;
      result += temp;
     }
     break;
    }
   }
   if (iter != flush_start) {
    result += (*self->t_ops->t_write_display)(self,flush_start,
                                             (size_t)(iter-flush_start),
                                              flags);
   }
done:
   return result;
  }
 }
 return (*self->t_ops->t_write_display)(self,buf,bufsize,flags);
}

/* Same as `tty_write_display()', but omit checks for process group
 * association and always grant write-permissions to the calling thread.
 * This function is used for implementing keyboard ECHO, and called
 * internally by `tty_write_display()' after checks have been done. */
PUBLIC size_t KCALL
tty_dowrite_display(struct tty *__restrict self,
                    USER CHECKED void const *buf,
                    size_t bufsize, iomode_t flags) {
 struct tty *EXCEPT_VAR xself = self;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->t_lock,flags);
 TRY {
  result = tty_dowrite_display_impl(self,buf,bufsize,flags);
 } FINALLY {
  if (rwlock_endread(&xself->t_lock))
      goto again;
 }
 return result;
}

PRIVATE size_t KCALL
tty_dowrite_echo2(struct tty *__restrict self,
                  USER CHECKED void const *buf,
                  size_t bufsize, iomode_t flags,
                  tcflag_t lflags) {
 size_t result = 0,temp,temp2;
 char *flush_start,*iter,*end;
 if (!(lflags & ECHOCTL))
     return tty_dowrite_display_impl(self,buf,bufsize,flags);
 /* Must encode CONTROL and/or ERASE characters. */
 flush_start = iter = (char *)buf;
 end = iter+bufsize;
 for (; iter < end; ++iter) {
  char ch = *iter;
  if (ch < 0x20) {
   char escape[2];
   temp2   = (size_t)(iter-flush_start);
   temp    = tty_dowrite_display_impl(self,flush_start,temp2,flags);
   result += temp;
   if (temp < temp2) goto done;
   /* Escape control characters. */
   escape[0] = '^';
   escape[1] = ch+0x40;
   temp    = tty_dowrite_display_impl(self,escape,2,flags);
   result += temp;
   if (temp < 2) goto done;
   flush_start = iter+1;
  }
 }
 /* Flush the remainder. */
 result += tty_dowrite_display_impl(self,flush_start,(size_t)(iter-flush_start),flags);
done:
 return result;
}

PRIVATE size_t KCALL
tty_dowrite_echo(struct tty *__restrict self,
                 USER CHECKED void const *buf,
                 size_t bufsize, iomode_t flags,
                 tcflag_t lflags) {
 if (bufsize &&
    (ATOMIC_FETCHAND(self->t_ios.c_lflag,~__IERASING) & __IERASING))
    (*self->t_ops->t_write_display)(self,"/",1,flags);
 return tty_dowrite_echo2(self,buf,bufsize,flags,lflags);
}

#if 0
PRIVATE void KCALL
tty_do_echo_display(struct tty *__restrict self, char ch, tcflag_t iflags) {
 if ((iflags & PARMRK) && ch == '\377') {
  PRIVATE char const temp[] = { '\377', '\377' };
  tty_write_display(self,temp,2,0);
 } else if ((iflags & INLCR) && ch == '\n') {
  PRIVATE char const temp[] = { '\r' };
  tty_write_display(self,temp,1,0);
 } else if ((iflags & IGNCR) && ch == '\r') {
 } else if ((iflags & ICRNL) && ch == '\r') {
  PRIVATE char const temp[] = { '\n' };
  tty_write_display(self,temp,1,0);
 } else {
  if (iflags & ISTRIP) ch &= (char)0x7f;
  if (iflags & IUCLC) ch = tolower(ch);
  tty_write_display(self,&ch,1,0);
 }
}
#endif

PRIVATE void KCALL
tty_check_sigchar(struct tty *__restrict self, char ch) {
 siginfo_t info; struct task *EXCEPT_VAR t;
 if (ch == self->t_ios.c_cc[VINTR]) {
  info.si_signo = SIGINT+1;
 } else if (ch == self->t_ios.c_cc[VQUIT]) {
  info.si_signo = SIGQUIT+1;
 } else if (ch == self->t_ios.c_cc[VSUSP]) {
  info.si_signo = SIGTSTP+1;
 } else {
  return;
 }
 if (!self->t_fproc) return;
 t = thread_pid_get(self->t_fproc);
 if (!t) return;
 TRY {
  memset((byte_t *)&info+COMPILER_OFFSETAFTER(siginfo_t,si_signo),
         0,sizeof(siginfo_t)-COMPILER_OFFSETAFTER(siginfo_t,si_signo));
  info.si_code = SI_KERNEL;
  signal_raise_pgroup(t,&info);
 } FINALLY {
  task_decref(t);
 }
}

PRIVATE size_t KCALL
tty_dump_canon(struct tty *__restrict self, iomode_t flags) {
 struct canon EXCEPT_VAR can;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 can = canonbuffer_capture(&self->t_can);
 TRY {
  result = ringbuffer_write(&self->t_input,can.c_base,can.c_size,flags);
 } FINALLY {
  canonbuffer_release(&self->t_can,can);
 }
 return result;
}


PRIVATE size_t KCALL
tty_dowrite_input(struct tty *__restrict self,
                  USER CHECKED void const *buf,
                  size_t bufsize, iomode_t flags,
                  tcflag_t lflags) {
 struct tty *EXCEPT_VAR xself = self;
 iomode_t EXCEPT_VAR xflags = flags;
 tcflag_t EXCEPT_VAR xlflags = lflags;
 size_t EXCEPT_VAR result;
 if (lflags & ICANON) {
  char *EXCEPT_VAR flush_start;
  char *EXCEPT_VAR iter;
  char *end;
  /* Print directly to the canon. */
  result = 0;
  flush_start = iter = (char *)buf;
  end = iter+bufsize;
  TRY {
   for (; iter < end; ++iter) {
    int erase_mode = 0;
    char ch = *iter;
    if (lflags & ISIG)
        tty_check_sigchar(self,ch);
    if ((lflags & ECHOE) && (ch == self->t_ios.c_cc[VERASE])) {
     byte_t unput_ch;
erase_char:
     if (canonbuffer_unputc(&self->t_can,&unput_ch)) {
      if (lflags & ECHO) {
       if (flush_start != iter) {
        tty_dowrite_echo(self,flush_start,
                        (size_t)(iter-flush_start),
                         flags,lflags);
        flush_start = iter+1;
       }
       if ((lflags & ECHOPRT) && (erase_mode < 3 || (lflags & ECHOKE))) {
        char delete_sequence[2] = { '\\', (char)unput_ch };
        if (tty_dowrite_display_impl(self,delete_sequence,2,flags))
            ATOMIC_FETCHOR(self->t_ios.c_lflag,__IERASING);
       } else {
        PRIVATE char const delete_sequence[3] = { '\b', ' ', '\b' };
        tty_dowrite_echo2(self,delete_sequence,3,flags,lflags & ~(ECHOPRT|ECHOKE));
       }
      }
      if (erase_mode == 1) {
       /* while (isspace(last)) erase(); */
       if (isspace(ch)) goto erase_char;
       erase_mode = 2;
      }
      if (erase_mode == 2) {
       /* while (!isspace(last)) erase(); */
       if (!isspace(ch)) goto erase_char;
      }
      if (erase_mode == 3) /* Erase line. */
          goto erase_char;
     }
    } else if ((lflags & ECHOE) && (ch == self->t_ios.c_cc[VWERASE])) {
     erase_mode = 1; /* Erase word */
     goto erase_char;
    } else if ((lflags & ECHOK) && (ch == self->t_ios.c_cc[VKILL])) {
     erase_mode = 3; /* Erase line */
     goto erase_char;
    } else if (ch == '\n' ||
               ch == self->t_ios.c_cc[VEOL] ||
               ch == self->t_ios.c_cc[VEOL2]) {
     canonbuffer_putc(&self->t_can,(byte_t)ch);
     if (lflags & ECHO) {
      tty_dowrite_echo(self,flush_start,
                      (size_t)((iter+1)-flush_start),
                       flags,lflags);
      flush_start = iter+1;
     }
     tty_dump_canon(self,flags);
    } else if (ch == self->t_ios.c_cc[VEOF]) {
     /* We use the ring buffer's FCLOSED bit to implement
      * read() -> 0 interrupts of the TTY input stream. */
     if (lflags & ECHO) {
      tty_dowrite_echo(self,flush_start,
                      (size_t)(iter-flush_start),
                       flags,lflags);
      flush_start = iter+1;
     }
     if (!tty_dump_canon(self,flags))
          ringbuffer_close(&self->t_input);
    } else if ((lflags & IEXTEN) &&
               (ch == self->t_ios.c_cc[VREPRINT])) {
     struct canon EXCEPT_VAR can;
     if (lflags & ECHO) {
      tty_dowrite_echo(self,flush_start,
                      (size_t)(iter-flush_start),
                       flags,lflags);
      flush_start = iter+1;
     }
     can = canonbuffer_clone(&self->t_can);
     TRY {
      ringbuffer_write(&self->t_input,can.c_base,can.c_size,flags);
     } FINALLY {
      canonbuffer_release(&self->t_can,can);
     }
    } else if ((lflags & IEXTEN) &&
               (ch == self->t_ios.c_cc[VLNEXT])) {
     if (lflags & ECHO) {
      tty_dowrite_echo(self,flush_start,
                      (size_t)(iter-flush_start),
                       flags,lflags);
      flush_start = iter+1;
     }
     if (++iter >= end ||
        !canonbuffer_putc(&self->t_can,(byte_t)*iter))
         break;
    } else {
     if (!canonbuffer_putc(&self->t_can,(byte_t)ch))
          break;
    }
   }
  } FINALLY {
   if (!FINALLY_WILL_RETHROW || error_code() == E_INTERRUPT) {
    /* Echo characters that were printed. */
    if (xlflags & ECHO)
        tty_dowrite_echo(xself,flush_start,(size_t)(iter-flush_start),xflags,xlflags);
   }
  }
  result = (size_t)(iter-(char *)buf);
 } else if (lflags & (ISIG|IEXTEN)) {
  /* Deal with character escape and signal characters. */
  char *flush_start,*iter,*end;
  size_t temp,temp2;
  result = 0;
  flush_start = iter = (char *)buf;
  end = iter+bufsize;
  TRY {
   for (; iter < end; ++iter) {
    char ch = *iter;
    if (lflags & ISIG)
        tty_check_sigchar(self,ch);
    if ((lflags & IEXTEN) &&
        (ch == self->t_ios.c_cc[VLNEXT])) {
     /* Escape the next character. */
     temp2 = (size_t)(iter-flush_start);
     temp  = ringbuffer_write(&self->t_input,flush_start,temp2,flags);
     if (lflags & ECHO)
         tty_dowrite_echo(self,flush_start,temp,flags,lflags);
     result += temp;
     if (temp < temp2) break;
     flush_start = iter+1;
     ++iter;
    }
   }
  } FINALLY {
   if (!FINALLY_WILL_RETHROW || error_code() == E_INTERRUPT) {
    result += ringbuffer_write(&xself->t_input,flush_start,iter-flush_start,xflags);
    if (xlflags & ECHO)
        tty_dowrite_echo(xself,flush_start,iter-flush_start,xflags,xlflags);
   }
  }
 } else {
  /* Print directly to the input buffer. */
  result = ringbuffer_write(&self->t_input,buf,bufsize,flags);
  if (lflags & ECHO)
      tty_dowrite_echo(self,buf,result,flags,lflags);
 }
 return result;
}

PUBLIC size_t KCALL
tty_write_input(struct tty *__restrict self,
                USER CHECKED void const *buf,
                size_t bufsize, iomode_t flags) {
 struct tty *EXCEPT_VAR xself = self;
 size_t COMPILER_IGNORE_UNINITIALIZED(result),temp,temp2;
 tcflag_t iflags,lflags;
again:
 rwlock_readf(&self->t_lock,flags);
 TRY {
  iflags = ATOMIC_READ(self->t_ios.c_iflag);
  lflags = ATOMIC_READ(self->t_ios.c_lflag);
  if (iflags & (PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IUCLC)) {
   char *iter,*end,*flush_start;
   /* Must perform some kind of input transformation,
    * or behave specially for certain characters. */
   result = 0;
   flush_start = iter = (char *)buf;
   end = iter+bufsize;
   for (; iter < end; ++iter) {
    char ch = *iter;
    switch (ch) {

    case '\377':
     if (iflags & PARMRK) {
      temp2   = (size_t)(iter-flush_start);
      temp    = tty_dowrite_input(self,flush_start,temp2,flags,lflags);
      result += temp;
      if (temp < temp2) goto done;
      /* Print the new character. */
      temp = tty_dowrite_input(self,"\377\377",2,flags,lflags);
      result += temp;
      if (temp < 2) goto done;
      flush_start = iter+1;
     }
     break;

    case '\n':
     if (iflags & INLCR) {
      temp2   = (size_t)(iter-flush_start);
      temp    = tty_dowrite_input(self,flush_start,temp2,flags,lflags);
      result += temp;
      if (temp < temp2) goto done;
      if (!(iflags & IGNCR)) {
       /* Print the new character. */
       temp = tty_dowrite_input(self,"\r",1,flags,lflags);
       result += temp;
       if (temp < 1) goto done;
      }
      flush_start = iter+1;
     }
     break;

    case '\r':
     if (iflags & (IGNCR|ICRNL)) {
      temp2   = (size_t)(iter-flush_start);
      temp    = tty_dowrite_input(self,flush_start,temp2,flags,lflags);
      result += temp;
      if (temp < temp2) goto done;
      if (!(iflags & IGNCR)) {
       /* Print the new character. */
       temp = tty_dowrite_input(self,"\n",1,flags,lflags);
       result += temp;
       if (temp < 1) goto done;
      }
      flush_start = iter+1;
     }
     break;

    {
     char new_ch;
    default:
     new_ch = ch;
     if (iflags & ISTRIP) new_ch &= 0x7f;
     if (iflags & IUCLC) new_ch = tolower(new_ch);
     if (new_ch != ch) {
      temp2   = (size_t)(iter-flush_start);
      /* Flush everything that hasn't been printed, yet. */
      temp    = tty_dowrite_input(self,flush_start,temp2,flags,lflags);
      result += temp;
      if (temp < temp2) goto done;
      /* Print the new character. */
      temp = tty_dowrite_input(self,&new_ch,1,flags,lflags);
      result += temp;
      if (temp < 1) goto done;
      flush_start = iter+1;
     }
    } break;
    }
   }
   if (flush_start != iter) {
    result += tty_dowrite_input(self,flush_start,
                               (size_t)(iter-flush_start),
                                flags,lflags);
   }
  } else {
   result = tty_dowrite_input(self,buf,bufsize,flags,lflags);
  }
done:
  if (result < bufsize && (iflags & IMAXBEL) &&
     (!result || !(flags & IO_NONBLOCK)))
     (void)0; /* TODO: Ring bell. */
 } FINALLY {
  if (rwlock_endread(&xself->t_lock))
      goto again;
 }
 return result;
}

PRIVATE size_t KCALL
tty_doread_input(struct tty *__restrict self,
                 USER CHECKED void *buf,
                 size_t bufsize, iomode_t flags) {
 size_t temp,result = 0;
 while (result < bufsize) {
  if (result >= ATOMIC_READ(self->t_ios.c_cc[VMIN])) {
   temp = ringbuffer_read_atomic(&self->t_input,
                                (byte_t *)buf+result,
                                 bufsize-result);
  } else {
   temp = ringbuffer_read(&self->t_input,
                         (byte_t *)buf+result,
                          bufsize-result,flags);
   /* We use the ring buffer's FCLOSED bit to implement
    * read() -> 0 interrupts of the TTY input stream. */
   if (!temp && !result &&
       !(ATOMIC_FETCHAND(self->t_input.r_limt,~RBUFFER_LIMT_FCLOSED) &
                                               RBUFFER_LIMT_FCLOSED))
       continue;
  }
  if (!temp) break;
  result += temp;
 }
 return result;
}


/* Read TTY input data, format it, then save it in the given `buf'.
 * NOTE: This function is used to implement the
 *       character-device `f_read' operator for TTYs.
 * @return: * : The actual number of bytes read. */
PUBLIC size_t KCALL
tty_read_input(struct tty *__restrict self,
               USER CHECKED void *buf,
               size_t bufsize, iomode_t flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->t_lock,flags);
 TRY {
  /* Same deal as in `tty_write_display()':
   *  -> Assert that the calling process is
   *     part of the foregroup process group. */
  if (self->t_fproc &&
      thread_pid_weak(self->t_fproc) !=
      get_this_processgroup_weak()) {
   siginfo_t info;
   /* Calling thread isn't part of the foreground process group. */
   memset(&info,0,sizeof(siginfo_t));
   /* Posix says we should sent a `SIGTTIN' to the ensure process group. - So we do that. */
   info.si_signo = SIGTTIN+1;
   signal_raise_pgroup(THIS_TASK,&info);
   result = 0; /* XXX: Shouldn't get here? */
  } else {
   result = tty_doread_input(self,buf,bufsize,flags);
  }
 } FINALLY {
  if (rwlock_endread(&self->t_lock))
      goto again;
 }
 return result;
}

/* Write the given data to the TTY display adapter after
 * formating it according to the active TTY rules.
 * This function is called when a process using the TTY attempts to write data to it.
 * if `TOSTOP' is set, a `SIGTTOU' is sent to the calling thread if its process isn't part of
 * the TTY's foreground process group, causing `E_INTERRUPT' to be thrown by this function. */
PUBLIC size_t KCALL
tty_write_display(struct tty *__restrict self,
                  USER CHECKED void const *buf,
                  size_t bufsize, iomode_t flags) {
 struct tty *EXCEPT_VAR xself = self;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->t_lock,flags);
 TRY {
  if (self->t_fproc &&
      thread_pid_weak(self->t_fproc) !=
      get_this_processgroup_weak()) {
   siginfo_t info;
   /* Calling thread isn't part of the foreground process group. */
   memset(&info,0,sizeof(siginfo_t));
   /* Posix says we should sent a `SIGTTOU' to the ensure process group. - So we do that. */
   info.si_signo = SIGTTOU+1;
   signal_raise_pgroup(THIS_TASK,&info);
   result = 0; /* XXX: Shouldn't get here? */
  } else {
   /* Forward to the ~real~ writer function. */
   result = tty_dowrite_display_impl(self,buf,bufsize,flags);
  }
 } FINALLY {
  if (rwlock_endread(&xself->t_lock))
      goto again;
 }
 return result;
}



PRIVATE ATTR_NOTHROW void KCALL
tty_fini(struct character_device *__restrict self) {
 struct tty *me = (struct tty *)self;
 if (me->t_ops->t_fini)
   (*me->t_ops->t_fini)(me);
 canonbuffer_fini(&me->t_can);
 ringbuffer_fini(&me->t_input);
 if (me->t_cproc) thread_pid_decref(me->t_cproc);
 if (me->t_fproc) thread_pid_decref(me->t_fproc);
}

PUBLIC ssize_t KCALL
tty_ioctl(struct tty *__restrict self,
          unsigned long cmd, USER UNCHECKED void *arg,
          iomode_t flags) {
 struct tty *EXCEPT_VAR xself = self;
 ssize_t result = 0;
 switch (cmd) {

 case TCGETS:
 case TCGETA:
  validate_writable(arg,sizeof(struct termios));
again_get:
  rwlock_readf(&self->t_lock,flags);
  TRY {
   memcpy(arg,&self->t_ios,sizeof(struct termios));
  } FINALLY {
   if (rwlock_endread(&xself->t_lock))
       goto again_get;
  }
  break;

 {
  struct termios new_ios;
 case TCSETS:
 case TCSETA:
 case TCSETSW:
 case TCSETAW:
 case TCSETSF:
 case TCSETAF:
  validate_readable(arg,sizeof(struct termios));
  memcpy(&new_ios,arg,sizeof(struct termios));
  COMPILER_BARRIER();
  rwlock_writef(&self->t_lock,flags);
  TRY {
   if (!(new_ios.c_lflag & ICANON) &&
        (self->t_ios.c_lflag & ICANON)) {
    /* When switching out of line-buffered mode, first dump the canon. */
    tty_dump_canon(self,0);
    memcpy(&self->t_ios,&new_ios,sizeof(struct termios));
    tty_dump_canon(self,0);
   } else {
    memcpy(&self->t_ios,&new_ios,sizeof(struct termios));
   }
  } FINALLY {
   rwlock_endwrite(&xself->t_lock);
  }
 } break;

 case TCFLSH:
  /* Discard data received, but not read (aka. keyboard input) */
  rwlock_writef(&self->t_lock,flags);
  TRY {
   if ((uintptr_t)arg != TCOFLUSH)
        ringbuffer_discard(&self->t_input,(size_t)-1);
   /* Discard data written, but not send (aka. display output). */
   if ((uintptr_t)arg != TCIFLUSH &&
        self->t_ops->t_discard_display)
      (*self->t_ops->t_discard_display)(self);
  } FINALLY {
   rwlock_endwrite(&xself->t_lock);
  }
  break;

 case TIOCGWINSZ:
  validate_writable(arg,sizeof(struct winsize));
  memcpy(arg,&self->t_size,sizeof(struct winsize));
  break;

 {
  struct winsize new_size;
 case TIOCSWINSZ:
  validate_readable(arg,sizeof(struct winsize));
  memcpy(&new_size,arg,sizeof(struct winsize));
  COMPILER_BARRIER();
  if (!self->t_ops->t_set_display_size)
       break;
  rwlock_writef(&self->t_lock,flags);
  TRY {
   /* Apply the new window size. */
   (*self->t_ops->t_set_display_size)(self,&new_size);
   /* Save the finalized window size. */
   if (memcmp(&self->t_size,&new_size,sizeof(struct winsize)) != 0) {
    REF struct task *EXCEPT_VAR t;
    memcpy(&self->t_size,&new_size,sizeof(struct winsize));
    if ((t = thread_pid_get(self->t_fproc)) != NULL) {
     TRY {
      siginfo_t info;
      /* Signal the window size change to the foreground process. */
      memset(&info,0,sizeof(siginfo_t));
      info.si_signo = SIGWINCH;
      info.si_code  = SI_KERNEL;
      signal_raise_pgroup(t,&info);
     } FINALLY {
      task_decref(t);
     }
    }
   }
  } FINALLY {
   rwlock_endwrite(&xself->t_lock);
  }
 } break;

 case TIOCGPGRP:
 case TIOCGSID: {
  pid_t COMPILER_IGNORE_UNINITIALIZED(respid);
  validate_writable(arg,sizeof(pid_t));
again_gpgrp:
  rwlock_readf(&self->t_lock,flags);
  TRY {
   if (!self->t_fproc)
       error_throw(E_PROCESS_EXITED);
   if (cmd == TIOCGSID) {
    REF struct task *t;
    t = thread_pid_get(self->t_fproc);
    if (!t) error_throw(E_PROCESS_EXITED);
    respid = posix_gettid_view(FORTASK(FORTASK(t,_this_group).tg_leader,_this_group).
                               tg_process.h_procgroup.pg_master.m_session);
    task_decref(t);
   } else {
    respid = posix_gettid_viewpid(self->t_fproc);
   }
  } FINALLY {
   if (rwlock_endread(&xself->t_lock))
       goto again_gpgrp;
  }
  COMPILER_BARRIER();
  *(pid_t *)arg = respid;
  COMPILER_BARRIER();
 } break;

 {
  REF struct task *EXCEPT_VAR new_task;
  REF struct thread_pid *new_pid;
  REF struct thread_pid *old_pid;
 case TIOCSPGRP:
  validate_readable(arg,sizeof(pid_t));
  new_task = pid_lookup_task(*(pid_t *)arg);
  TRY {
   REF struct task *new_group;
   new_group = get_processgroup_of(new_task);
   task_decref(new_task);
   new_task = new_group;
   new_pid = FORTASK(new_task,_this_pid);
   if (!new_pid) error_throw(E_PROCESS_EXITED);
   rwlock_writef(&self->t_lock,flags);
   thread_pid_incref(new_pid);
   old_pid = self->t_fproc;
   self->t_fproc = new_pid;
   rwlock_endwrite(&self->t_lock);
   if (old_pid)
       thread_pid_decref(old_pid);
  } FINALLY {
   task_decref(new_task);
  }
 } break;

#if 1
 case TIOCEXCL:
  ATOMIC_FETCHOR(self->t_flags,TTY_FEXCL);
  break;

 case TIOCNXCL:
  ATOMIC_FETCHAND(self->t_flags,~TTY_FEXCL);
  break;

 case TIOCGEXCL:
  validate_writable(arg,sizeof(int));
  *(int *)arg = (ATOMIC_READ(self->t_flags) & TTY_FEXCL) ? 1 : 0;
  break;

 case FIONREAD:
  validate_writable(arg,sizeof(unsigned int));
  *(unsigned int *)arg = (unsigned int)ringbuffer_maxread(&self->t_input);
  break;
 case FIOQSIZE:
  /* I guess this is right... */
  validate_writable(arg,sizeof(u64));
  *(u64 *)arg = (u64)ringbuffer_maxread(&self->t_input);
  break;

 case TIOCOUTQ:
  validate_writable(arg,sizeof(unsigned int));
  if (!self->t_ops->t_display_pending) {
   *(unsigned int *)arg = 0;
  } else {
   *(unsigned int *)arg = (unsigned int)(*self->t_ops->t_display_pending)(self);
  }
  break;

 case TIOCSTI:
  validate_readable(arg,sizeof(char));
  /* XXX: Are we supposed to bypass input modulation, or
   *      do we actually need to call `tty_write_input()'? */
  /* XXX: Tell user-space if the buffer is full? (`ringbuffer_write()' returns ZERO(0)) */
  ringbuffer_write(&self->t_input,arg,1,flags);
  break;

 case TIOCGSOFTCAR:
  validate_writable(arg,sizeof(int));
  *(int *)arg = (ATOMIC_READ(self->t_ios.c_cflag) & CLOCAL) ? 1 : 0;
  break;

 {
  int value;
 case TIOCSSOFTCAR:
  validate_readable(arg,sizeof(int));
  value = *(int *)arg;
  COMPILER_READ_BARRIER();
  /* Change the CLOCAL flag. */
  rwlock_writef(&self->t_lock,flags);
  self->t_ios.c_cflag &= ~CLOCAL;
  if (value)
      self->t_ios.c_cflag |= CLOCAL;
  rwlock_endwrite(&self->t_lock);
 } break;
#endif

#if 0
#define TCGETS2            _IOR('T',0x2a,struct termios2)
#define TCSETS2            _IOW('T',0x2b,struct termios2)
#define TCSETSW2           _IOW('T',0x2c,struct termios2)
#define TCSETSF2           _IOW('T',0x2d,struct termios2)
#define TIOCGRS485         _IO('T',0x2e)
#define TIOCSRS485         _IO('T',0x2f)
#define TIOCGPTN           _IOR('T',0x30,unsigned int) /* Get Pty Number (of pty-mux device) */
#define TIOCSPTLCK         _IOW('T',0x31,int)  /* Lock/unlock Pty */
#define TIOCGDEV           _IOR('T',0x32,unsigned int) /* Get primary device node of /dev/console */
#define TCGETX             _IO('T',0x32) /* SYS5 TCGETX compatibility */
#define TCSETX             _IO('T',0x33)
#define TCSETXF            _IO('T',0x34)
#define TCSETXW            _IO('T',0x35)
#define TIOCSIG            _IOW('T',0x36,int) /* pty: generate signal */
#define TIOCVHANGUP        _IO('T',0x37)
#define TIOCGPKT           _IOR('T',0x38,int) /* Get packet mode state */
#define TIOCGPTLCK         _IOR('T',0x39,int) /* Get Pty lock state */
#define FIONCLEX           _IO('T',0x50)
#define FIOCLEX            _IO('T',0x51)
#define FIOASYNC           _IO('T',0x52)
#define TIOCSERCONFIG      _IO('T',0x53)
#define TIOCSERGWILD       _IO('T',0x54)
#define TIOCSERSWILD       _IO('T',0x55)
#define TIOCGLCKTRMIOS     _IO('T',0x56)
#define TIOCSLCKTRMIOS     _IO('T',0x57)
#define TIOCSERGSTRUCT     _IO('T',0x58) /* For debugging only */
#define TIOCSERGETLSR      _IO('T',0x59) /* Get line status register */
#define TIOCSERGETMULTI    _IO('T',0x5a) /* Get multiport config  */
#define TIOCSERSETMULTI    _IO('T',0x5b) /* Set multiport config */
#define TIOCMIWAIT         _IO('T',0x5c) /* wait for a change on serial input line(s) */
#define TIOCGICOUNT        _IO('T',0x5d) /* read serial port inline interrupt counts */

#define TCSBRK             _IO('T',0x09)
#define TCSBRKP            _IO('T',0x25) /* Needed for POSIX tcsendbreak() */
#define TIOCSBRK           _IO('T',0x27) /* BSD compatibility */
#define TIOCCBRK           _IO('T',0x28) /* BSD compatibility */
#define TCXONC             _IO('T',0x0a)
#define TIOCSCTTY          _IO('T',0x0e)
#define TIOCNOTTY          _IO('T',0x22)

#define TIOCMGET           _IO('T',0x15)
#define TIOCMBIS           _IO('T',0x16)
#define TIOCMBIC           _IO('T',0x17)
#define TIOCMSET           _IO('T',0x18)

#define TIOCLINUX          _IO('T',0x1c)
#define TIOCCONS           _IO('T',0x1d)
#define TIOCGSERIAL        _IO('T',0x1e)
#define TIOCSSERIAL        _IO('T',0x1f)
#define TIOCPKT            _IO('T',0x20)
#define FIONBIO            _IO('T',0x21)

#define TIOCSETD           _IO('T',0x23)
#define TIOCGETD           _IO('T',0x24)
#endif

 default:
  error_throw(E_NOT_IMPLEMENTED);
  break;
 }
 return result;
}
PRIVATE unsigned int KCALL
tty_poll(struct tty *__restrict self,
         unsigned int mode) {
 unsigned int result = 0;
 /* Poll for input data to become available. */
 if ((mode & POLLIN) &&
      ringbuffer_poll_nonempty(&self->t_input))
      result |= POLLIN;
 /* Poll for data being possible to-be written to the display. */
 if ((mode & POLLOUT) &&
      self->t_ops->t_poll_display &&
    (*self->t_ops->t_poll_display)(self))
      result |= POLLOUT;
 return result;
}
PUBLIC void KCALL
tty_sync(struct tty *__restrict self) {
 /* Wait for data to appear on the display. */
 if (self->t_ops->t_sync_display)
   (*self->t_ops->t_sync_display)(self);
}
PUBLIC void KCALL
tty_stat(struct tty *__restrict self,
         USER CHECKED struct stat64 *result) {
 result->st_blocks  = 1;
 result->st_blksize = ATOMIC_READ(self->t_input.r_limt) & RBUFFER_LIMT_FMASK;
 result->st_size    = ringbuffer_maxread(&self->t_input);
}
PRIVATE pos_t KCALL
tty_seek(struct tty *__restrict self, off_t offset, int whence) {
 if (whence != SEEK_CUR)
     error_throw(E_INVALID_ARGUMENT);
 /* EXTENSION: `seek(42,SEEK_CUR)' -> DISCARD(42) (returns the number of discarded bytes) */
 if (offset >= 0)
     return ringbuffer_discard(&self->t_input,(size_t)offset);
 /* EXTENSION: `seek(-42,SEEK_CUR)' -> UNREAD(42) (returns the number of unread bytes) */
 return ringbuffer_unread(&self->t_input,(size_t)-offset);
}


PRIVATE REF struct handle KCALL
tty_open(struct tty *__restrict self,
         struct path *__restrict p, oflag_t open_mode) {
 REF struct handle result;
 /* Check the EXCL TTY flag. */
 if ((ATOMIC_READ(self->t_flags) & TTY_FEXCL)/* &&
     !capable(CAP_SYS_ADMIN)*/) /* TODO: Capabilities */
     error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_OBJECT_IS_BUSY);
 result.h_mode = HANDLE_MODE(HANDLE_TYPE_FDEVICE,
                             IO_FROM_O(open_mode));
 result.h_object.o_character_device = &self->t_dev;
 tty_incref(self);
 return result;
}


PRIVATE struct character_device_ops tty_character_ops = {
    .c_fini = &tty_fini,
    .c_open = (REF struct handle(KCALL *)(struct character_device *__restrict,struct path *__restrict,oflag_t))&tty_open,
    .c_file = {
        .f_ioctl = (ssize_t(KCALL *)(struct character_device *__restrict,unsigned long,USER UNCHECKED void *,iomode_t))&tty_ioctl,
        .f_read  = (size_t(KCALL *)(struct character_device *__restrict,USER CHECKED void *,size_t,iomode_t))&tty_read_input,
        .f_write = (size_t(KCALL *)(struct character_device *__restrict,USER CHECKED void const *,size_t,iomode_t))&tty_write_display,
        .f_poll  = (unsigned int(KCALL *)(struct character_device *__restrict,unsigned int))&tty_poll,
        .f_seek  = (pos_t(KCALL *)(struct character_device *__restrict,off_t,int))&tty_seek,
        .f_sync  = (void(KCALL *)(struct character_device *__restrict))&tty_sync,
        .f_stat  = (void(KCALL *)(struct character_device *__restrict,USER CHECKED struct stat64 *))&tty_stat,
    }
};


FUNDEF ATTR_RETNONNULL REF struct tty *
(KCALL tty_alloc)(size_t struct_size,
                  struct tty_ops *__restrict ops,
                  struct driver *__restrict caller) {
 REF struct tty *result;
 assert(ops);
 assert(ops->t_write_display);
 assert(struct_size >= sizeof(struct tty));
 result = (REF struct tty *)__character_device_alloc(struct_size,
                                                     caller);
 result->t_dev.c_ops = &tty_character_ops;
 result->t_ops       = ops;
 canonbuffer_cinit(&result->t_can);
 ringbuffer_cinit(&result->t_input,MAX_INPUT);
 rwlock_cinit(&result->t_lock);
 /* Pre-initialize TTY configuration to ~sane~ values. */
 result->t_ios.c_iflag      = (ICRNL|BRKINT|IMAXBEL);
 result->t_ios.c_oflag      = (ONLCR|OPOST);
 result->t_ios.c_lflag      = (ECHO|ECHOE|ECHOK|ICANON|ISIG|IEXTEN);
 result->t_ios.c_cflag      = (CREAD);
#define CTRL_CODE(x) ((x)-64) /* ^x */
 result->t_ios.c_cc[VMIN]   = 1; /* Read at least one character by default. */
 result->t_ios.c_cc[VEOF]   = CTRL_CODE('D'); /* ^D. */
 result->t_ios.c_cc[VEOL]   = 0; /* Not set. */
 result->t_ios.c_cc[VERASE] = '\b';
 result->t_ios.c_cc[VINTR]  = CTRL_CODE('C'); /* ^C. */
 result->t_ios.c_cc[VKILL]  = CTRL_CODE('U'); /* ^U. */
 result->t_ios.c_cc[VQUIT]  = CTRL_CODE('^'); /* Might supposed to be '\\'? */
 result->t_ios.c_cc[VSTART] = CTRL_CODE('Q'); /* ^Q. */
 result->t_ios.c_cc[VSTOP]  = CTRL_CODE('S'); /* ^S. */
 result->t_ios.c_cc[VSUSP]  = CTRL_CODE('Z'); /* ^Z. */
 result->t_ios.c_cc[VTIME]  = 0;
#undef CTRL_CODE
 result->t_size.ws_xpixel = result->t_size.ws_col = 80;
 result->t_size.ws_ypixel = result->t_size.ws_row = 25;
 return result;
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_DEV_TTY_C */
