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
#ifndef GUARD_KERNEL_SRC_DEV_MOUSE_C
#define GUARD_KERNEL_SRC_DEV_MOUSE_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <dev/mouse.h>
#include <string.h>
#include <except.h>

DECL_BEGIN

PRIVATE void KCALL
cd_mouse_fini(struct mouse *__restrict self) {
 if (self->m_ops->mo_fini)
   (*self->m_ops->mo_fini)(self);
}

PRIVATE ssize_t KCALL
cd_mouse_ioctl(struct mouse *__restrict self,
               unsigned long cmd,
               USER UNCHECKED void *arg,
               iomode_t flags) {
 /* TODO */
 error_throw(E_NOT_IMPLEMENTED);
}

PRIVATE size_t KCALL
cd_mouse_read(struct mouse *__restrict self,
              USER CHECKED void *buf,
              size_t bufsize, iomode_t flags) {
 struct mouse_packet packet;
 if (bufsize < sizeof(struct mouse_packet))
     error_throwf(E_BUFFER_TOO_SMALL,bufsize,sizeof(struct mouse_packet));
 if (!mouse_read(self,&packet,flags))
      return 0;
 COMPILER_READ_BARRIER();
 /* Copy the packet back into user-space. */
 memcpy(buf,&packet,sizeof(struct mouse_packet));
 /* Return the size of a mouse packet. */
 return sizeof(struct mouse_packet);
}

PRIVATE unsigned int KCALL
cd_mouse_poll(struct mouse *__restrict self,
              unsigned int mode) {
 /* TODO: Polling for asynchronous signals */
 return 0;
}


PRIVATE struct character_device_ops mouse_ops = {
    .c_fini = (void(KCALL *)(struct character_device *__restrict))&cd_mouse_fini,
    .c_file = {
        .f_ioctl = (ssize_t (KCALL *)(struct character_device *__restrict self, unsigned long cmd, USER UNCHECKED void *arg, iomode_t flags))&cd_mouse_ioctl,
        .f_read  = (size_t (KCALL *)(struct character_device *__restrict self, USER CHECKED void *buf, size_t bufsize, iomode_t flags))&cd_mouse_read,
        .f_poll  = (unsigned int (KCALL *)(struct character_device *__restrict self, unsigned int mode))&cd_mouse_poll
    }
};


PUBLIC ATTR_RETNONNULL REF struct mouse *KCALL
__mouse_alloc(size_t struct_size,
              struct mouse_ops *__restrict ops,
              struct driver *__restrict caller) {
 REF struct mouse *result;
 assert(struct_size >= sizeof(struct mouse));
 assert(ops != NULL);
 result = (REF struct mouse *)__character_device_alloc(struct_size,
                                                       caller);
 result->m_ops       = ops;
 result->m_dev.c_ops = &mouse_ops;
 return result;
}



/* Parse a single mouse action and update `state' accordingly.
 * @return: * : The number of read bytes. */
PRIVATE u8 KCALL
mouse_parse_action(byte_t act[MOUSE_ACTION_BACKLOG_SIZE],
                   u16 rptr,
                   struct mouse_overflow *__restrict state,
                   bool allow_time_actions) {
 u16 start = rptr;
 u8 opcode = act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
 if (opcode & MOUSE_ACTION_FMOVXY) {
  u8 b = act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
  state->ms_relx += MOUSE_ACTION_FMOVXY_GETX(opcode,b);
  state->ms_rely += MOUSE_ACTION_FMOVXY_GETY(opcode,b);
 } else if (opcode & MOUSE_ACTION_FMOVZ) {
  state->ms_relz += MOUSE_ACTION_FMOVZ_GET(opcode);
 } else if ((opcode & MOUSE_ACTION_FPCKT4) == MOUSE_ACTION_FPCKT4) {
  if (!allow_time_actions) return 0;
  state->ms_time += MOUSE_ACTION_FPCKT4_GET(opcode);
 } else if (opcode & MOUSE_ACTION_FUP) {
  ++state->ms_up[MOUSE_ACTION_FUP_GET(opcode)];
 } else if (opcode & MOUSE_ACTION_FDOWN) {
  ++state->ms_down[MOUSE_ACTION_FUP_GET(opcode)];
 } else {
  switch (opcode) {

  case MOUSE_ACTION_FMOVY:
   state->ms_rely += act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
   break;

  {
   u32 offset;
  case MOUSE_ACTION_FPCKT32:
   if (!allow_time_actions) return 0;
   offset   = 0,offset |= act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
   offset <<= 8,offset |= act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
   offset <<= 8,offset |= act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
   offset <<= 8,offset |= act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
   state->ms_time += offset;
  } break;

  {
   u64 offset;
  case MOUSE_ACTION_FPCKT64:
   if (!allow_time_actions) return 0;
   offset   = 0,offset |= act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
   offset <<= 8,offset |= act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
   offset <<= 8,offset |= act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
   offset <<= 8,offset |= act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
   offset <<= 8,offset |= act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
   offset <<= 8,offset |= act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
   offset <<= 8,offset |= act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
   offset <<= 8,offset |= act[rptr++ % MOUSE_ACTION_BACKLOG_SIZE];
   state->ms_time += offset;
  } break;

  default:
   assertf(0,"Invalid MOUSE opcode 0x%.2I8x",opcode);
  }
 }
 return rptr - start;
}


/* Feed the mouse some action data.
 * `data' should be a tightly packed vector, as described by `MOUSE_ACTION_FPCKT32'. */
PUBLIC ATTR_NOTHROW ASYNCSAFE NOMP void KCALL
mouse_action(struct mouse *__restrict self,
             byte_t const *__restrict data,
             u16 datalen) {
 union mouse_action_state old_actst;
 union mouse_action_state actst;
again:
 old_actst.as_word = ATOMIC_READ(self->m_act_state.as_word);
 actst.as_word = old_actst.as_word;
 if (old_actst.as_rcnt+datalen >= MOUSE_ACTION_BACKLOG_SIZE) {
  /* The buffer isn't large enough. */
  if ((self->m_act[(u16)(old_actst.as_wptr - 1) % MOUSE_ACTION_BACKLOG_SIZE]) !=
       MOUSE_ACTION_FOVERFL) {
   self->m_act[actst.as_wptr++] = MOUSE_ACTION_FOVERFL;
   ++actst.as_rcnt;
   if (!ATOMIC_CMPXCH(self->m_act_state.as_word,
                      old_actst.as_word,actst.as_word))
        goto again;
   memset(&self->m_overflow,0,sizeof(struct mouse_overflow));
  }
  /* Parse action data as part of the mouse overflow. */
  while (datalen) {
   u8 length;
   length = mouse_parse_action((byte_t *)data,0,&self->m_overflow,true);
   assert(length != 0);
   assertf(length <= datalen,
           "length  = %u\n"
           "datalen = %u\n"
           "%$[hex]\n",
           (unsigned int)length,
           (unsigned int)datalen,
           (size_t)length,data);
   datalen -= length;
   data    += length;
  }
 } else {
  u8 i;
  for (i = 0; i < datalen; ++i) {
   self->m_act[actst.as_wptr++] = data[i];
   actst.as_wptr %= MOUSE_ACTION_BACKLOG_SIZE;
  }
  actst.as_rcnt += datalen;
  if (!ATOMIC_CMPXCH(self->m_act_state.as_word,
                     old_actst.as_word,actst.as_word))
       goto again;
 }
 /* If the action backlog was empty, indicate that new moust data has become available. */
 if (!old_actst.as_rcnt)
      async_sig_broadcast(&self->m_avail);
}


/* Read a single mouse packet from the given mouse.
 * @return: true:  Successfully read a mouse packet.
 * @return: false: `IO_NONBLOCK' was passed and no data was available immediately. */
PUBLIC bool KCALL
mouse_read(struct mouse *__restrict self,
           struct mouse_packet *__restrict packet,
           iomode_t mode) {
 union mouse_action_state old_actst;
 union mouse_action_state actst;
 struct mouse_overflow data; u8 count;
 unsigned int i;
again_locked:
 atomic_rwlock_write(&self->m_readlock);
 if (self->m_flags & MOUSE_FHASLATEBUTTON) {
  u16 down_buttons = 0;
  u16 up_buttons = 0;
  /* Serve late button operations. */
  for (i = 0; i < MOUSE_BUTTON_COUNT; ++i) {
   u16 down,up;
   down = self->m_down_late[i];
   up   = self->m_up_late[i];
   if (down >= up) {
    if (!down) continue;
    /* Return a button-press packet. */
    if (up_buttons) break;
    --self->m_down_late[i];
    down_buttons |= 1 << i;
   } else {
    /* Return a button-release packet. */
    if (down_buttons) break;
    --self->m_up_late[i];
    up_buttons |= 1 << i;
   }
  }
  if (down_buttons || up_buttons) {
   packet->mp_time = self->m_lastread;
   atomic_rwlock_endwrite(&self->m_readlock);
   packet->mp_relx = 0;
   packet->mp_rely = 0;
   packet->mp_relz = 0;
   packet->mp_chng = down_buttons | up_buttons;
   packet->mp_keys = down_buttons;
   return true;
  }
  /* Done processing late button presses. */
  ATOMIC_FETCHAND(self->m_flags,~MOUSE_FHASLATEBUTTON);
 }
again:
 old_actst.as_word = ATOMIC_READ(self->m_act_state.as_word);
 actst.as_word = old_actst.as_word;
 if (!actst.as_rcnt) {
  struct async_task_connection acon;
  /* No data is available.
   * Either wait for data, or return `false' */
  atomic_rwlock_endwrite(&self->m_readlock);
  if (mode & IO_NONBLOCK) return false;
  task_connect_async(&acon,&self->m_avail);
  COMPILER_READ_BARRIER();
  /* Check for data one more time. */
  atomic_rwlock_read(&self->m_readlock);
  if (ATOMIC_READ(self->m_act_state.as_rcnt) != 0 ||
      ATOMIC_READ(self->m_flags) & MOUSE_FHASLATEBUTTON) {
   atomic_rwlock_endread(&self->m_readlock);
   task_disconnect_async();
   goto again_locked;
  }
  atomic_rwlock_endread(&self->m_readlock);
  /* Finally, wait for data to arrive. */
  task_waitfor_async(JTIME_INFINITE);
  goto again_locked;
 }
 memset(&data,0,sizeof(struct mouse_overflow));
 do {
  u16 rptr = (u16)(actst.as_wptr-actst.as_rcnt) % MOUSE_ACTION_BACKLOG_SIZE;
  if (self->m_act[rptr] == MOUSE_ACTION_FOVERFL) {
   if (actst.as_rcnt != old_actst.as_rcnt) break;
   /* Load the overflow packet. */
   memcpy(&data,
          &self->m_overflow,
           sizeof(struct mouse_overflow));
   /* Check if the overflow packet contains multiple down operations. */
   for (i = 0; i < MOUSE_BUTTON_COUNT; ++i) {
    if (data.ms_down[i]+data.ms_up[i] <= 1) continue;
    /* The overflow contains more than one operation on the same button. */
    if (!ATOMIC_CMPXCH(self->m_act_state.as_word,
                       old_actst.as_word,actst.as_word))
         goto again;
    /* Set up late button operations. */
    memcpy(self->m_down_late,data.ms_down,sizeof(data.ms_down));
    memcpy(self->m_up_late,data.ms_up,sizeof(data.ms_up));
    ATOMIC_FETCHOR(self->m_flags,MOUSE_FHASLATEBUTTON);
    /* Consume the late button operation which we will be returning. */
    if (self->m_down_late[i])
         --self->m_down_late[i];
    else --self->m_up_late[i];
    goto done_endread;
   }
   /* Consume the overflow instruction. */
   --actst.as_rcnt;
   break;
  }
  /* Process mouse packet data. */
  count = mouse_parse_action(self->m_act,
                             rptr,
                            &data,
                             data.ms_time == 0);
  actst.as_rcnt -= count;
 } while (actst.as_rcnt && count &&
         (self->m_flags & MOUSE_FMERGEONREAD));
 if (!ATOMIC_CMPXCH(self->m_act_state.as_word,
                    old_actst.as_word,actst.as_word))
      goto again;
done_endread:
 /* Update the current packet time. */
 self->m_lastread += data.ms_time;
 packet->mp_time   = self->m_lastread;
 atomic_rwlock_endwrite(&self->m_readlock);
 /* Translate mouse overflow data into a mouse packet. */
 packet->mp_relx = data.ms_relx;
 packet->mp_rely = data.ms_rely;
 packet->mp_relz = data.ms_relz;
 packet->mp_keys = 0;
 packet->mp_chng = 0;
 for (i = 0; i < MOUSE_BUTTON_COUNT; ++i) {
  if (data.ms_down[i])
      packet->mp_keys |= 1 << i;
  if (data.ms_down[i] || data.ms_up[i])
      packet->mp_chng |= 1 << i;
 }
 return true;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_DEV_MOUSE_C */
