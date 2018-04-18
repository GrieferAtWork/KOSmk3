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
#ifndef GUARD_KERNEL_SRC_DEV_PS2_C
#define GUARD_KERNEL_SRC_DEV_PS2_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kernel/debug.h>
#include <sched/async_signal.h>
#include <dev/ps2.h>
#include <kos/keyboard.h>
#include <dev/keyboard.h>
#include <dev/ps2-program.h>
#include <dev/ps2-mouse.h>
#include <sys/io.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/boot.h>
#include <kernel/malloc.h>
#include <except.h>
#include <assert.h>
#include <string.h>
#include <alloca.h>

#ifdef CONFIG_HAVE_DEV_PS2

DECL_BEGIN

#undef CONFIG_LOG_PS2_PROGRAM_COMMUNICATIONS
#define CONFIG_LOG_PS2_PROGRAM_COMMUNICATIONS 1

LOCAL void KCALL ps2_write_cmd(u8 cmd) {
 while (inb_p(PS2_STATUS) & PS2_STATUS_INFULL);
 outb_p(PS2_CMD,cmd);
 /*while (inb_p(PS2_STATUS) & PS2_STATUS_INFULL);*/
}
LOCAL void KCALL ps2_write_cmddata(u8 data) {
 while (inb_p(PS2_STATUS) & PS2_STATUS_INFULL);
 outb_p(PS2_DATA,data);
}
LOCAL void KCALL ps2_write_data(u8 port, u8 data) {
#ifdef CONFIG_LOG_PS2_PROGRAM_COMMUNICATIONS
 debug_printf("PROGRAM --> 0x%.2I8x (OUT) (port #%x)\n",data,port+1);
#endif
 if (port == PS2_PORT2) {
  while (inb_p(PS2_STATUS) & PS2_STATUS_INFULL);
  outb_p(PS2_CMD,PS2_CONTROLLER_WRITE_PORT2_INPUT);
 }
 while (inb_p(PS2_STATUS) & PS2_STATUS_INFULL);
 outb_p(PS2_DATA,data);
}


union ps2_state {
    struct {
        u8           ps_pc;       /* The %pc register */
        u8           ps_arg;      /* The %arg index register */
        u8           ps_res;      /* The %res index register */
        u8           ps_irr;      /* The %irr register */
    };
    ATOMIC_DATA u32  ps_word;     /* The register state control word (updated atomically) */
};


struct ps2_regs {
    u8              *pe_pc;       /* [const] Current program counter. */
    u8              *pe_arg;      /* [const] The `%arg' pointer register. */
    u8              *pe_res;      /* [const] The `%res' pointer register. */
    union ps2_state  pe_regs;     /* The current register state. */
    u8               pe_attempt;  /* The number of remaining attempts when `PS2_RESEND' is received. */
    u8               pe_port;     /* The port to which data is sent. */
    u8             __pe_pad[sizeof(void *)-2];
};

struct ps2_progreq {
    struct ps2_regs     pr_regs;   /* The current program register state */
    struct async_sig    pr_stop;   /* A signal that should be broadcast when the program stops. */
    struct ps2_progreq *pr_next;   /* The next program. */
    bool               *pr_ok;     /* Set to true/false indicative of success. */
};

#define PS2_PROGREQ_FINISHED   NULL                     /* Used to indicate no program running. */
#define PS2_PROGREQ_PENDING  ((struct ps2_progreq *)-1) /* Used to indicate the program being set up. */
/* The Program currently being executed. */
PRIVATE struct ps2_progreq *current_program[2] = { PS2_PROGREQ_FINISHED, PS2_PROGREQ_FINISHED };

/* Chain of pending programs. */
PRIVATE struct ps2_progreq *pending_programs[2] = { NULL, NULL };
/* List of free programs. */
PRIVATE struct ps2_progreq *free_programs = NULL;

PRIVATE jtime_t ps2_timeout = JIFFIES_FROM_MILLI(200);
PRIVATE u8      ps2_retries = 3; /* Number of times to restart a program. */

PRIVATE struct ps2_progreq *KCALL pop_pending(u8 port) {
 struct ps2_progreq *result;
 do if ((result = ATOMIC_READ(pending_programs[port])) == NULL) break;
 while (!ATOMIC_CMPXCH_WEAK(pending_programs[port],result,result->pr_next));
 return result;
}

PRIVATE void KCALL push_pending(u8 port, struct ps2_progreq *__restrict self) {
 struct ps2_progreq *next;
 do self->pr_next = next = ATOMIC_READ(pending_programs[port]);
 while (!ATOMIC_CMPXCH_WEAK(pending_programs[port],next,self));
}


struct ps2_callback {
     ps2_callback_t c_func; /* [1..1] Function to invoke */
     void          *c_arg;  /* [?..?] Argument passed to `c_func' */
};

struct ps2_inputdata {
#define PS2_BUFFERVECTOR_LOCKED  ((struct ps2_callback *)-1)
    size_t               bv_bufc; /* Amount of buffers. */
    struct ps2_callback *bv_bufv; /* [1..1][0..bv_bufc][lock(== PS2_BUFFERVECTOR_LOCKED)][owned]
                                   * Vector of callbacks to invoke during a PS/2 interrupt. */
    atomic_rwlock_t      bv_lock; /* Lock for adding / removing elements. */
};

/* PS/2 input state for the primary and secondary ports. */
PRIVATE struct ps2_inputdata ps2_input[PS2_PORTCOUNT];
PUBLIC u8 ps2_packet_size[PS2_PORTCOUNT] = { [0 ... PS2_PORTCOUNT-1] = 1 };
PUBLIC u8 ps2_port_device[PS2_PORTCOUNT] = { [0 ... PS2_PORTCOUNT-1] = PS2_PORT_DEVICE_FUNKNOWN };

/* Operate with PS/2 input data. */
PRIVATE ASYNCSAFE void KCALL ps2_inputdata_putword(struct ps2_inputdata *__restrict self, byte_t *__restrict ps2_bytes);
PRIVATE void KCALL ps2_inputdata_addfunc(struct ps2_inputdata *__restrict self, ps2_callback_t func, void *arg);
PRIVATE bool KCALL ps2_inputdata_delfunc(struct ps2_inputdata *__restrict self, ps2_callback_t func, void *arg);


PRIVATE ASYNCSAFE void KCALL
ps2_inputdata_putword(struct ps2_inputdata *__restrict self,
                      byte_t *__restrict ps2_bytes) {
 struct ps2_callback *vec; size_t i,count;
 assert(!PREEMPTION_ENABLED());
#if 0
 debug_printf("PS2 %$q\n",ps2_packet_size[self-ps2_input],ps2_bytes);
#endif

again:
 count = ATOMIC_READ(self->bv_bufc);
#ifdef CONFIG_NO_SMP
 vec = ATOMIC_XCH(self->bv_bufv,PS2_BUFFERVECTOR_LOCKED);
 assert(vec != PS2_BUFFERVECTOR_LOCKED);
#else
 do vec = ATOMIC_XCH(self->bv_bufv,PS2_BUFFERVECTOR_LOCKED);
 while (vec == PS2_BUFFERVECTOR_LOCKED);
#endif
 if unlikely(count != ATOMIC_READ(self->bv_bufc)) {
  /* The number of connected buffers changed. */
  ATOMIC_WRITE(self->bv_bufv,vec);
  goto again;
 }
#if 1
 if unlikely(!count) {
  debug_printf("Untargeted interrupt: %$q (port #%u)\n",
               ps2_packet_size[self-ps2_input],ps2_bytes,
               1+(self-ps2_input));
 }
#endif

 /* Add the key to every connected buffer. */
 for (i = 0; i < count; ++i)
     (*vec[i].c_func)(vec[i].c_arg,ps2_bytes);
 ATOMIC_WRITE(self->bv_bufv,vec);
}

PRIVATE void KCALL
ps2_inputdata_addfunc(struct ps2_inputdata *__restrict self,
                      ps2_callback_t func, void *arg) {
 size_t old_count;
 struct ps2_callback *old_vector;
 struct ps2_callback *new_vector;
again:
 old_count   = ATOMIC_READ(self->bv_bufc);
 old_vector  = ATOMIC_READ(self->bv_bufv);
 if (old_vector == PS2_BUFFERVECTOR_LOCKED) { task_yield(); goto again; }
 /* Allocate a new buffer that is a copy of the old. */
 new_vector  = (struct ps2_callback *)kmalloc((old_count+1)*sizeof(struct ps2_callback),
                                               GFP_SHARED);
 atomic_rwlock_read(&self->bv_lock);
 if (ATOMIC_READ(self->bv_bufv) != old_vector ||
     ATOMIC_READ(self->bv_bufc) != old_count)
     goto buffer_changed;
 /* Copy the old buffer vector. */
 memcpy(new_vector,old_vector,
        old_count*sizeof(struct ps2_callback));
 new_vector[old_count].c_func = func;
 new_vector[old_count].c_arg  = arg;
 if (!atomic_rwlock_tryupgrade(&self->bv_lock)) {
buffer_changed:
  atomic_rwlock_endread(&self->bv_lock);
  kfree(new_vector);
  task_yield();
  goto again;
 }
 if (!ATOMIC_CMPXCH(self->bv_bufv,old_vector,new_vector)) {
  atomic_rwlock_endwrite(&self->bv_lock);
  kfree(new_vector);
  goto again;
 }
 ATOMIC_WRITE(self->bv_bufc,old_count+1);
 atomic_rwlock_endwrite(&self->bv_lock);
 /* Delete the old buffer. */
 kfree(old_vector);
 /* Done! */
}

PRIVATE ASYNCSAFE void KCALL
sub_ps2_callback(byte_t UNUSED(ps2_byte), void *UNUSED(arg)) {
}


PRIVATE bool KCALL
ps2_inputdata_delfunc(struct ps2_inputdata *__restrict EXCEPT_VAR self,
                      ps2_callback_t func, void *arg) {
 size_t old_count,i;
 struct ps2_callback *old_vector;
 struct ps2_callback *COMPILER_IGNORE_UNINITIALIZED(new_vector);
 struct ps2_callback last_buffer;
again:
 old_count  = ATOMIC_READ(self->bv_bufc);
 old_vector = ATOMIC_READ(self->bv_bufv);
 if (old_vector == PS2_BUFFERVECTOR_LOCKED) { task_yield(); goto again; }
 if unlikely(!old_count) return false;
 /* Allocate a new buffer that is a copy of the old. */
 TRY {
  new_vector = (struct ps2_callback *)kmalloc((old_count-1)*sizeof(struct ps2_callback),
                                               GFP_SHARED);
 } CATCH (E_BADALLOC) {
  /* Replace the given `buffer' with `&stub_buffer' */
  atomic_rwlock_write(&self->bv_lock);
  if (ATOMIC_READ(self->bv_bufv) != old_vector ||
      ATOMIC_READ(self->bv_bufc) != old_count) {
   atomic_rwlock_endwrite(&self->bv_lock);
   goto again;
  }
  for (i = 0; i < old_count; ++i) {
   if (old_vector[i].c_arg != arg ||
      !ATOMIC_CMPXCH(old_vector[i].c_func,func,&sub_ps2_callback))
        continue;
   atomic_rwlock_endwrite(&self->bv_lock);
   /* Make sure that any currently ongoing boardcast operation has finished
    * before returning to the caller, thus ensuring that the interrupt driver
    * will only see valid word buffers while broadcasting.
    * If we returning without this check, we'd run a race condition where
    * the interrupt driver would use a word buffer which the caller may have
    * already deallocated.
    * Note however, that it is highly unlikely that this check ever comes true... */
   while unlikely(ATOMIC_READ(self->bv_bufv) == PS2_BUFFERVECTOR_LOCKED)
       task_yield();
   return true;
  }
  atomic_rwlock_endwrite(&self->bv_lock);
  return false;
 }
 /* Copy the old buffer. */
 atomic_rwlock_read(&self->bv_lock);
 if (ATOMIC_READ(self->bv_bufv) != old_vector ||
     ATOMIC_READ(self->bv_bufc) != old_count)
     goto buffer_changed;
 last_buffer = old_vector[old_count-1];
 memcpy(new_vector,old_vector,(old_count-1)*sizeof(struct ps2_callback));
 /* Remove the specified buffer from the new vector. */
 if (last_buffer.c_func == func &&
     last_buffer.c_arg  == arg)
     goto found_buffer;
 for (i = 0; i < old_count-1; ++i) {
  if (new_vector[i].c_func == func &&
      new_vector[i].c_arg  == arg) {
   memmove(&new_vector[i],&new_vector[i+1],
          ((old_count-2)-i)*sizeof(struct ps2_callback));
   goto found_buffer;
  }
 }
 /* The specified buffer wasn't found. */
 atomic_rwlock_endread(&self->bv_lock);
 kfree(new_vector);
 return false;
found_buffer:
 /* Upgrade our lock. */
 if (!atomic_rwlock_tryupgrade(&self->bv_lock)) {
buffer_changed:
  atomic_rwlock_endread(&self->bv_lock);
  kfree(new_vector);
  task_yield();
  goto again;
 }
 /* Update the new vector size. */
 ATOMIC_WRITE(self->bv_bufc,old_count-1);
 /* XXX: Minor race condition:
  *   The (now) last buffer may momentarily not receive
  *   keystrokes before the following atomic_cmpxch completes. */
 if (!ATOMIC_CMPXCH(self->bv_bufv,old_vector,new_vector)) {
  ATOMIC_WRITE(self->bv_bufc,old_count);
  atomic_rwlock_endwrite(&self->bv_lock);
  kfree(new_vector);
  goto again;
 }
 atomic_rwlock_endwrite(&self->bv_lock);
 /* Delete the old buffer. */
 kfree(old_vector);
 /* Done! */
 return true;
}


/* Same as the buffer functions above, but used to install
 * asynchronous callbacks that are invoked from PS/2 interrupts. */
PUBLIC void KCALL
ps2_install_callback(u8 port, ps2_callback_t func, void *arg) {
 /* Install the given callback. */
 assert(port < PS2_PORTCOUNT);
 ps2_inputdata_addfunc(&ps2_input[port],func,arg);
}
PUBLIC bool KCALL
ps2_delete_callback(u8 port, ps2_callback_t func, void *arg) {
 /* Delete the given callback. */
 assert(port < PS2_PORTCOUNT);
 return ps2_inputdata_delfunc(&ps2_input[port],func,arg);
}



PRIVATE ATTR_RETNONNULL struct ps2_progreq *KCALL alloc_program(void) {
 struct ps2_progreq *result,*next;
 do {
  result = ATOMIC_READ(free_programs);
  if (!result) {
   result = (struct ps2_progreq *)kmalloc(sizeof(struct ps2_progreq),
                                          GFP_SHARED|GFP_LOCKED);
   break;
  }
  next = result->pr_next;
 } while (!ATOMIC_CMPXCH_WEAK(free_programs,result,next));
 return result;
}
PRIVATE void KCALL free_program(struct ps2_progreq *__restrict p) {
 struct ps2_progreq *next;
 do next = ATOMIC_READ(free_programs),
    p->pr_next = next;
 while (!ATOMIC_CMPXCH_WEAK(free_programs,next,p));
}



/* NOTE: wait-instructions must have the caller do the waiting _BEFOREHAND_
 * @return: true:  Execution should continue after waiting on an interrupt.
 * @return: false: Execution has finished. */
PRIVATE bool KCALL
ps2_exec(struct ps2_progreq *__restrict self, bool from_interrupt) {
 union ps2_state oldregs,regs;
 u8 op,action_word;
 bool orig_from_interrupt = from_interrupt;
#define ACTION_NONE      0 /* Do nothing as post-action */
#define ACTION_SAVE_WORD 1 /* Save `action_word' to the result vector. */
#define ACTION_SEND_WORD 2 /* Send `action_word' down the PS/2 cable. */
 int post_action;
nextop:
 oldregs.ps_word = ATOMIC_READ(self->pr_regs.pe_regs.ps_word);
restart_op:
 regs.ps_word = oldregs.ps_word;
 op = self->pr_regs.pe_pc[regs.ps_pc++];
 post_action = ACTION_NONE;
 switch (op) {

 case PS2_PROGOP_STOP:
  ATOMIC_WRITE(*self->pr_ok,true);
  return false;

 case PS2_PROGOP_FAIL:
program_failed:
  ATOMIC_WRITE(*self->pr_ok,false);
  return false;

 case PS2_PROGOP_SEND:
  action_word = self->pr_regs.pe_arg[regs.ps_arg++];
  post_action = ACTION_SEND_WORD;
  break;

 case PS2_PROGOP_DISP:
  action_word = self->pr_regs.pe_pc[regs.ps_pc++];
  post_action = ACTION_SEND_WORD;
  break;

 case PS2_PROGOP_WAIT:
 case PS2_PROGOP_WACK:
 case PS2_PROGOP_READ:
 case PS2_PROGOP_WIMM:
  /* If we didn't get here from an interrupt, this is where we must stop
   * execution and wait a connected PS/2 device to complete the operation. */
  if (!from_interrupt)
       return true;
  /* If we did get here from an interrupt, we must
   * continue execution until the next wait instruction,
   * or until the terminating stop / fail instruction. */
  from_interrupt = false;
  if (op == PS2_PROGOP_WACK) {
   if (regs.ps_irr != PS2_ACK)
       goto program_failed;
  } else if (op == PS2_PROGOP_WIMM) {
   if (regs.ps_irr != self->pr_regs.pe_pc[regs.ps_pc++])
       goto program_failed;
  } else if (op == PS2_PROGOP_READ) {
 case PS2_PROGOP_PIRR:
   action_word = regs.ps_irr;
   post_action = ACTION_SAVE_WORD;
   ++regs.ps_res;
  }
  break;

 case PS2_PROGOP_PRES:
  action_word = self->pr_regs.pe_pc[regs.ps_pc++];
  post_action = ACTION_SAVE_WORD;
  ++regs.ps_res;
  break;

 {
  s8 off;
 case PS2_PROGOP_JMP:
  off = (s8)self->pr_regs.pe_pc[regs.ps_pc++];
  regs.ps_pc += off;
 } break;

 {
  u8 l,r; s8 off;
 case PS2_PROGOP_IJIFE:
 case PS2_PROGOP_IJIFNE:
 case PS2_PROGOP_IJIFB:
 case PS2_PROGOP_IJIFBE:
 case PS2_PROGOP_IJIFA:
 case PS2_PROGOP_IJIFAE:
  l = regs.ps_irr;
  goto do_jcc;
 case PS2_PROGOP_AJIFE:
 case PS2_PROGOP_AJIFNE:
 case PS2_PROGOP_AJIFB:
 case PS2_PROGOP_AJIFBE:
 case PS2_PROGOP_AJIFA:
 case PS2_PROGOP_AJIFAE:
  l = self->pr_regs.pe_arg[regs.ps_arg++];
do_jcc:
  r = self->pr_regs.pe_pc[regs.ps_pc++];
  switch (op & 0x7) {
  case PS2_PROGOP_IJIFE  & 0x7: l = l == r; break;
  case PS2_PROGOP_IJIFNE & 0x7: l = l != r; break;
  case PS2_PROGOP_IJIFB  & 0x7: l = l < r; break;
  case PS2_PROGOP_IJIFBE & 0x7: l = l <= r; break;
  case PS2_PROGOP_IJIFA  & 0x7: l = l > r; break;
  case PS2_PROGOP_IJIFAE & 0x7: l = l >= r; break;
  default: __builtin_unreachable();
  }
  off = (s8)self->pr_regs.pe_pc[regs.ps_pc++];
  if (l) regs.ps_pc += off;
 } break;

 default: assertf(0,"Illegal opcode %.2I8x",op);
 }
 if (!ATOMIC_CMPXCH(self->pr_regs.pe_regs.ps_word,
                    oldregs.ps_word,regs.ps_word)) {
  /* If the register state changed before we got here, that can only mean
   * that some other piece of code has executed some part of the code.
   * If we got here from an interrupt, we must loop back and try to execute
   * whatever instruction it is that is selected now. Otherwise, we must
   * stop execution and wait for what can only be an interrupt to finish
   * execution, which will cause the finished-signal to be send. */
  if (!orig_from_interrupt)
       return true;
  goto restart_op;
 }
 COMPILER_BARRIER();
 /* Perform a post-action. */
 switch (post_action) {
 case ACTION_SAVE_WORD:
  self->pr_regs.pe_res[regs.ps_res-1] = action_word;
  break;
 case ACTION_SEND_WORD:
  ps2_write_data(self->pr_regs.pe_port,action_word);
  break;
 default: break;
 }
 goto nextop;
}



INTERN void KCALL ps2_interrupt(u8 port) {
 struct ps2_progreq *prog;
 u8 data = inb(PS2_DATA);
 prog = ATOMIC_READ(current_program[port]);
 if (prog != PS2_PROGREQ_FINISHED &&
     prog != PS2_PROGREQ_PENDING) {
  struct ps2_progreq *next;
#ifdef CONFIG_LOG_PS2_PROGRAM_COMMUNICATIONS
  debug_printf("PROGRAM <-- 0x%.2I8x (IN) (port #%x)\n",data,port+1);
#endif
  if (data == PS2_RESEND) {
   union ps2_state reset_state;
   if (!prog->pr_regs.pe_attempt) {
    *prog->pr_ok = false;
    goto program_finished;
   }
   /* Reset the program. */
   --prog->pr_regs.pe_attempt;
   reset_state.ps_word = 0;
   reset_state.ps_irr  = PS2_RESEND;
   ATOMIC_WRITE(prog->pr_regs.pe_regs.ps_word,reset_state.ps_word);
  } else {
   ATOMIC_WRITE(prog->pr_regs.pe_regs.ps_irr,data);
  }
  if (ps2_exec(prog,true))
      return;
program_finished:
  /* Execution has finished. */
  ATOMIC_WRITE(current_program[port],PS2_PROGREQ_PENDING);
  /* Load the next program. */
  next = pop_pending(port);
  ATOMIC_WRITE(current_program[port],next);
  /* Signal that the program has finished. */
  async_sig_broadcast(&prog->pr_stop);
  /* Free the program. */
  free_program(prog);

  /* Start the next program. */
  if (next) {
   if (!ps2_exec(next,false))
        goto program_finished;
  }
 } else {
  u8 size = ps2_packet_size[port];
  if (size <= 1) {
   /* Broadcast the raw PS/2 data byte. */
   ps2_inputdata_putword(&ps2_input[port],&data);
  } else {
   /* Read the remainder of the packet. */
   byte_t *iter,*packet;
   iter = packet = (byte_t *)alloca(size);
   *iter++ = data,--size;
   do *iter++ = inb(PS2_DATA);
   while (--size);
   /* Broadcast the raw PS/2 packet. */
   ps2_inputdata_putword(&ps2_input[port],packet);
  }
 }
}


INTERN void KCALL ps2_irq_1(void) {
 ps2_interrupt(PS2_PORT1);
}
INTERN void KCALL ps2_irq_2(void) {
 ps2_interrupt(PS2_PORT2);
}




PUBLIC bool KCALL
ps2_runprogram(u8 port,
               u8 const *__restrict argv,
               u8 *__restrict resv,
               u8 const *__restrict program) {
 return ps2_runprogram_ex(port,argv,resv,program,ps2_timeout);
}
PUBLIC bool KCALL
ps2_runprogram_ex(u8 port,
                  u8 const *__restrict argv,
                  u8 *__restrict resv,
                  u8 const *__restrict program,
                  jtime_t rel_timeout) {
 bool result = true;
 struct ps2_progreq *req;
 struct async_task_connection con;
 req = alloc_program();
 async_sig_init(&req->pr_stop);
 req->pr_next                 = NULL;
 req->pr_ok                   = &result;
 req->pr_regs.pe_arg          = (u8 *)argv;
 req->pr_regs.pe_res          = (u8 *)resv;
 req->pr_regs.pe_pc           = (u8 *)program;
 req->pr_regs.pe_regs.ps_word = 0;
 req->pr_regs.pe_regs.ps_irr  = PS2_RESEND;
 req->pr_regs.pe_attempt      = ps2_retries;
 req->pr_regs.pe_port         = port;
 /* Connect to the stop-signal of the program request. */
 task_connect_async(&con,&req->pr_stop);
 task_nothrow_serve();
 TRY {
  struct ps2_progreq *next = NULL;
  /* Register the program. */
  if (ATOMIC_CMPXCH(current_program[port],
                    PS2_PROGREQ_FINISHED,
                    PS2_PROGREQ_PENDING)) {
   /* Start the initial program. */
start_initial:
   ATOMIC_WRITE(current_program[port],req);
   if (!ps2_exec(req,false)) {
    task_disconnect_async();
    next = pop_pending(port);
    ATOMIC_WRITE(current_program[port],next);
    free_program(req);
    goto start_next_program;
   }
  } else {
   /* Other programs are already running (queue as pending). */
   push_pending(port,req);
   /* Check if the current program finished while we were
    * registering ours as pending. If it did, we must unregister
    * our program, then start ours as the initial program.
    * However, we must made sure to find the proper spot
    * where to remove the program from. */
   if (ATOMIC_CMPXCH(current_program[port],
                     PS2_PROGREQ_FINISHED,
                     PS2_PROGREQ_PENDING)) {
    struct ps2_progreq **pself,*temp;
    for (;;) {
again_remove:
     pself = &pending_programs[port];
     for (;;) {
      temp = *pself;
      if likely(temp == req) break;
      if unlikely(!temp) goto again_remove;
      pself = &temp->pr_next;
     }
     if (ATOMIC_CMPXCH(*pself,req,next))
         break;
    }
    goto start_initial;
   }
  }
  /* Wait for the program to complete. */
  if (!task_waitfor_async(jiffies + rel_timeout)) {
   /* The program timed out. */
   result = false;
   /* Must start the next program. */
   if (ATOMIC_CMPXCH(current_program[port],req,next)) {
    free_program(req);
start_next_program:
    if (next && !ps2_exec(next,false)) {
     req = next;
     next = pop_pending(port);
     ATOMIC_WRITE(current_program[port],next);
     async_sig_broadcast(&req->pr_stop); /* Signal that the program has finished. */
     free_program(req);                  /* Free the program. */
     goto start_next_program;            /* Start the next program. */
    }
   }
  }
 } FINALLY {
  if (FINALLY_WILL_RETHROW)
      task_disconnect_async();
  task_nothrow_end();
 }
 return result;
}


INTDEF void KCALL ps2_register_keyboard(u8 port);
INTDEF void KCALL ps2_register_mouse(u8 port, u8 type);

PRIVATE ATTR_FREETEXT void KCALL
ps2_detect_threadmain(void *UNUSED(arg)) {
 unsigned int i;
 /* Detect keyboard and mice. */
 for (i = 0; i < PS2_PORTCOUNT; ++i) {
  u8 resp[2] = { PS2_RESEND, PS2_RESEND },repc = 2;
  /* Identify the connected device. */
  if (!ps2_runprogram(i,NULL,resp,PS2_PROGRAM(
       ps2_send   0xf5; /* Disable scanning */
       ps2_wait   PS2_ACK;
       ps2_send   0xf2; /* Identify device */
       ps2_wait   PS2_ACK;
       /* Receive up to 2 response bytes */
       ps2_wait   %res;
       ps2_wait   %res;
       ps2_stop;
       ))) {
   repc = resp[0] != PS2_RESEND ? 1 : 0;
  }
  /* Interpret the response. */
  if (repc == 1 &&
     (resp[0] == PS2_MOUSE_TYPE_FNORMAL ||
      resp[0] == PS2_MOUSE_TYPE_FWHEEL ||
      resp[0] == PS2_MOUSE_TYPE_F5BUTTON)) {
   /* It's a mouse! */
   ps2_register_mouse(i,resp[0]);
   continue;
  }
  if (repc == 2 && resp[0] == 0xab &&
     (resp[1] == 0x41 ||
      resp[1] == 0xc1 ||
      resp[1] == 0x83)) {
   /* It's a keyboard! */
   ps2_register_keyboard(i);
   continue;
  }

  /* Fallback: Check if it's a keyboard
   *           by running an ECHO program. */
  if (ps2_runprogram(i,NULL,NULL,PS2_PROGRAM(
      ps2_send   0xee;
      ps2_wait   0xee;
      ps2_stop;
      ))) {
   /* It's a keyboard! */
   ps2_register_keyboard(i);
  }
 }
}


DEFINE_DRIVER_INIT(ps2_initialize);
PRIVATE ATTR_USED ATTR_FREETEXT void KCALL ps2_initialize(void) {
 REF struct task *EXCEPT_VAR worker;
 /* Make sure both ports are disabled while we configure PS/2. */
 ps2_write_cmd(PS2_CONTROLLER_DISABLE_PORT1);
 ps2_write_cmd(PS2_CONTROLLER_DISABLE_PORT2);

 /* Setup our initial PS/2 configuration. */
 ps2_write_cmd(PS2_CONTROLLER_WRAM(0)); /* Read RAM #0 */
 ps2_write_cmddata(PS2_CONTROLLER_CFG_PORT1_IRQ|
                   PS2_CONTROLLER_CFG_SYSTEMFLAG|
                   PS2_CONTROLLER_CFG_PORT2_IRQ);
 /* Enable both ports. */
 ps2_write_cmd(PS2_CONTROLLER_ENABLE_PORT1);
 ps2_write_cmd(PS2_CONTROLLER_ENABLE_PORT2);

 /* Spawn a worker thread for keyboard & mouse
  * initialization to deal with the timeouts
  * without holding up the entire boot process. */
 worker = task_alloc();
 TRY {
  task_setup_kernel(worker,
                   &ps2_detect_threadmain,
                    NULL);
#ifndef CONFIG_NO_SMP
#if 0 /* Although this does work, it is slow A.F. because of the enormous
       * number of IPIs that need to be sent to the secondary CPU.
       * (All the PS/2 interrupts happen on the boot CPU, but to wake
       *  a thread running on a secondary core, we must send in close
       *  to 50 additional interrupts, each coming with a slight delay)
       * And I'm not speaking about a couple of milli-seconds. No.
       * As far as I can tell, _not_ running this on a secondary core
       * reduces boot time by almost 1/2 of a second.
       * >> So just use same-core multithreading for this one... */
  /* Just to speed up booting a little bit more, have this
   * worker run on the second CPU if we have more than one. */
  if (cpu_count > 1)
      worker->t_cpu = cpu_vector[1];
#endif
#endif /* !CONFIG_NO_SMP */

  task_start(worker);
  /* Register the worker as a boot worker thread,
   * meaning it must be joined before .free is deleted,
   * and before the initial switch to user-space. */
  kernel_register_bootworker(worker);
 } FINALLY {
  task_decref(worker);
 }
 /* Yield once to kick-start the PS/2 detection function.
  * We're going to have to join it in init.c anyways, so
  * might as well cause it to start as soon as possible. */
 task_yield();
}



DECL_END

#endif /* CONFIG_HAVE_DEV_PS2 */

#endif /* !GUARD_KERNEL_SRC_DEV_PS2_C */
