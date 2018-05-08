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
#ifndef GUARD_KERNEL_SRC_NET_PACKETBUFFER_C
#define GUARD_KERNEL_SRC_NET_PACKETBUFFER_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/atomic.h>
#include <hybrid/align.h>
#include <hybrid/xch.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/list/list.h>
#include <kernel/malloc.h>
#include <kernel/debug.h>
#include <kos/types.h>
#include <fs/iomode.h>
#include <kernel/user.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <net/packetbuffer.h>
#include <except.h>
#include <string.h>
#include <sys/uio.h>

DECL_BEGIN

#define GETB(addr) (*(u8 *)(self->pb_base + ((addr) & self->pb_mask)))
#define GETW(addr) (*(u16 *)(self->pb_base + ((addr) & self->pb_mask)))
#define GETL(addr) (*(u32 *)(self->pb_base + ((addr) & self->pb_mask)))

/* Finalize a given packet buffer. */
PUBLIC ATTR_NOTHROW void KCALL
packetbuffer_fini(struct packetbuffer *__restrict self) {
 if (self->pb_ancillary_fini) {
  union packetbuffer_state state;
  struct packet_data data;
  data.pd_base = self->pb_base;
  data.pd_mask = self->pb_mask;
  state.pbs_state = self->pb_state.pbs_state;
  while (state.pbs_count) {
   struct packet_header header;
   ((u32 *)&header)[0] = GETL(state.pbs_addr+0);
   ((u32 *)&header)[1] = GETL(state.pbs_addr+4);
   assert(header.p_total != 0);
   assert(header.p_total <= state.pbs_count);
   if (header.p_ancillary) {
    /* Cleanup ancillary data. */
    data.pd_size = header.p_ancillary;
    /* Figure out where ancillary data starts. */
    data.pd_addr = CEIL_ALIGN(state.pbs_addr +
                              header.p_payload +
                              sizeof(struct packet_header),
                              PACKET_BUFFER_ALIGNMENT);
    data.pd_addr &= data.pd_mask;
    /* Invoke the cleanup callback. */
    (*self->pb_ancillary_fini)(data);
   }
   state.pbs_count -= header.p_total;
   state.pbs_addr  += header.p_total;
   state.pbs_addr  &= self->pb_mask;
  }
 }
 if (!(self->pb_limt & PBUFFER_LIMT_FSTATIC))
       kfree(self->pb_base);
}


/* Close the buffer and wake all blocking readers/writers. */
PUBLIC ATTR_NOTHROW bool KCALL
packetbuffer_close(struct packetbuffer *__restrict self) {
 bool result;
 result = ATOMIC_FETCHOR(self->pb_limt,PBUFFER_LIMT_FCLOSED);
 if (result) sig_broadcast(&self->pb_stat);
 return result;
}




/* Just as `packetbuffer_readv' is for `packetbuffer_read', allow
 * the user-space target buffer to be located in more than one place.
 * @param: pbufsize: Upon entry, contains the total sum of bytes found in `iov'
 * @param: pbufsize: Upon exit, store the total amount of required buffer space here. */
PUBLIC bool KCALL
packetbuffer_readva(struct packetbuffer *__restrict self,
                    USER CHECKED struct iovec const *iov, size_t *__restrict pbufsize,
                    HOST void *anc_buffer, size_t *__restrict pancsize,
                    iomode_t mode, packet_iomode_t packet_mode) {
 struct packetbuffer *EXCEPT_VAR xself = self;
 bool COMPILER_IGNORE_UNINITIALIZED(result);
 size_t buffer_mask,num_bytes;
 bool EXCEPT_VAR has_write_lock;
 struct packet_header header;
 union packetbuffer_state new_state;
 union packetbuffer_state state;
 num_bytes = *pbufsize;
again_full:
 has_write_lock = false;
 atomic_rwlock_read(&self->pb_lock);
 TRY {
again_locked:
  buffer_mask = self->pb_mask;
  do {
   state.pbs_state = ATOMIC_READ(self->pb_state.pbs_state);
   new_state.pbs_state = state.pbs_state;
   if unlikely(!state.pbs_count) {
    /* Check if the buffer was closed. */
    if (self->pb_limt & PBUFFER_LIMT_FCLOSED) {
     if (has_write_lock)
          atomic_rwlock_endwrite(&self->pb_lock);
     else atomic_rwlock_endread(&self->pb_lock);
     if (anc_buffer) *pancsize = 0;
     *pbufsize = 0;
     return false; /* XXX: Skip across finally is intentional! */
    }
    /* No data available for reading. */
    if (has_write_lock)
         atomic_rwlock_endwrite(&self->pb_lock);
    else atomic_rwlock_endread(&self->pb_lock);
    goto wait_for_data; /* XXX: Skip across finally is intentional! */
   }
   assert(IS_ALIGNED(state.pbs_addr,PACKET_BUFFER_ALIGNMENT));
   assert(state.pbs_count >= sizeof(struct packet_header));
   ((u32 *)&header)[0] = GETL(state.pbs_addr+0);
   ((u32 *)&header)[1] = GETL(state.pbs_addr+4);
   assert(state.pbs_count >= header.p_total);
   assertf(IS_ALIGNED(header.p_total,PACKET_BUFFER_ALIGNMENT),
           "header.p_total  = %u (%x)\n"
           "state.pbs_count = %u (%x)\n"
           ,header.p_total,header.p_total
           ,state.pbs_count,state.pbs_count);
   assert(CEIL_ALIGN(header.p_payload,PACKET_BUFFER_ALIGNMENT)+
          CEIL_ALIGN(header.p_ancillary,PACKET_BUFFER_ALIGNMENT)+
          sizeof(header) <= header.p_total);

   /* Save the payload size as the total required size. */
   *pbufsize = header.p_payload;

   /* Copy the packet's actual payload. */
   {
    size_t written_size = 0;
    size_t unwritten_size = header.p_payload;
    size_t iov_index;
    u16 payload_addr = (state.pbs_addr + sizeof(header)) & buffer_mask;
    for (iov_index = 0; written_size < num_bytes; ++iov_index) {
     struct iovec io;
     size_t copy_size;
     io = iov[iov_index];
     COMPILER_READ_BARRIER();
     if (!io.iov_len) continue;
     if (!(mode & IO_NOIOVCHECK))
           validate_writable(io.iov_base,io.iov_len);
     payload_addr &= buffer_mask;
     copy_size = (buffer_mask+1) - payload_addr;
     copy_size = MIN(copy_size,unwritten_size);
     if (copy_size >= io.iov_len) {
      /* Copy high memory only. */
      memcpy(io.iov_base,self->pb_base + payload_addr,io.iov_len);
      written_size   += io.iov_len;
      payload_addr   += io.iov_len;
      unwritten_size -= io.iov_len;
     } else {
      /* Copy high memory. */
      memcpy(io.iov_base,self->pb_base + payload_addr,copy_size);
      *(uintptr_t *)&io.iov_base += copy_size;
      written_size   += copy_size;
      payload_addr   += copy_size;
      unwritten_size -= copy_size;
      io.iov_len     -= copy_size;
      /* Must also read from low memory. */
      copy_size = MIN(unwritten_size,io.iov_len);
      memcpy(io.iov_base,self->pb_base,copy_size);
      written_size   += copy_size;
      payload_addr   += copy_size;
      unwritten_size -= copy_size;
     }
    }
    result = true;
    /* Figure out if we should actually indicate success. */
    if (packet_mode & PACKET_IO_FRDNEVER)
        result = false;
    else if ((packet_mode & PACKET_IO_FRDIFFIT) &&
             (written_size < header.p_payload)) {
     if (packet_mode & PACKET_IO_FRDTRUNC) {
      if (written_size != 0) {
       /* Special case: truncate the packet. */
       if (!has_write_lock) {
        has_write_lock = true;
        if (!atomic_rwlock_upgrade(&self->pb_lock))
             goto again_locked;
       }
       /* Adjust the payload size to trim the portion that was already read. */
       header.p_payload    -= written_size;
       /* Adjust the buffer state to point to the new header location. */
       new_state.pbs_addr  += written_size;
       new_state.pbs_count -= written_size;
       assert(new_state.pbs_count == unwritten_size);
       /* Write the updated packet header. */
       GETL(state.pbs_addr + 0) = ((u32 *)&header)[0];
       GETL(state.pbs_addr + 4) = ((u32 *)&header)[1];
       ATOMIC_WRITE(self->pb_state.pbs_state,new_state.pbs_state);
       /* Since we were allowed to truncate the buffer, the
        * required buffer size is only what we actually wrote. */
       *pbufsize = written_size;
      }
      /* Ancillary data is read alongside the packet being removed.
       * The documentation in the header would allow us to read it
       * here too, however doing it this way is a lot simpler. */
      if (anc_buffer) *pancsize = 0;
      /* Success. */
      goto read_finished;
     }
     /* Buffer isn't large enough, and `PACKET_IO_FRDIFFIT' is enabled. */
     result = false;
    }
   }

   if (result) {
    /* Adjust the packet buffer state to point to the next packet. */
    new_state.pbs_addr  += header.p_total;
    new_state.pbs_addr  &= buffer_mask;
    new_state.pbs_count -= header.p_total;
   }

   /* Deal with ancillary data. */
   if (result && header.p_ancillary) {
    struct packet_data adata;
    adata.pd_base = self->pb_base;
    adata.pd_addr = CEIL_ALIGN(state.pbs_addr +
                               header.p_payload +
                               sizeof(struct packet_header),
                               PACKET_BUFFER_ALIGNMENT);
    adata.pd_mask = buffer_mask;
    adata.pd_size = header.p_ancillary;
    if (anc_buffer) {
     size_t high_size;
     size_t bufavail = *pancsize;
     *pancsize = adata.pd_size;
     if (adata.pd_size > bufavail) {
      result = false;
      goto read_finished; /* Ancillary buffer is too small (always fail in reading the packet). */
     }
     /* Copy ancillary data into the caller's buffer.
      * HINT: This part cannot fail with E_SEGFAULT! */
     high_size = (buffer_mask+1)-adata.pd_addr;
     high_size = MIN(high_size,adata.pd_size);
     memcpy(anc_buffer,self->pb_base + adata.pd_addr,high_size);
     if (high_size < adata.pd_size) {
      /* Must also copy data from low memory. */
      memcpy((byte_t *)anc_buffer + high_size,self->pb_base,
              adata.pd_size-high_size);
     }
    } else if (self->pb_ancillary_fini) {
     /* Need a write-lock to prevent some other thread from
      * reading ancillary data while we're trying to destroy it. */
     if (!has_write_lock) {
      has_write_lock = true;
      if (!atomic_rwlock_upgrade(&self->pb_lock))
           goto again_locked;
     }
     /* Now that we have a write-lock, update the packet to
      * get rid of all traces of ancillary data ever being there. */
     GETW(state.pbs_addr + offsetof(struct packet_header,p_ancillary)) = 0;

     /* Simply destroy ancillary data. */
     (*self->pb_ancillary_fini)(adata);

    }
   } else if (anc_buffer) {
    /* No ancillary data in packet. */
    *pancsize = 0;
   }
  } while (!ATOMIC_CMPXCH(self->pb_state.pbs_state,
                          state.pbs_state,
                          new_state.pbs_state));
read_finished:
  ;
 } FINALLY {
  if (has_write_lock)
       atomic_rwlock_endwrite(&xself->pb_lock);
  else atomic_rwlock_endread(&xself->pb_lock);
 }
 /* Signal that something was read (causing writers to
  * re-evaluate their ability of writing their latest packet) */
 sig_broadcast(&self->pb_read);
 /* Signal special buffer state change: non_empty -> empty */
 if (state.pbs_count != 0 && new_state.pbs_count == 0)
     sig_broadcast_channel(&self->pb_stat,PBUF_STATE_CHANNEL_EMPTY);
 return result;
wait_for_data:
 if (mode & IO_NONBLOCK)
     error_throw(E_WOULDBLOCK); /* Non-blocking mode. */
 /* Wait for a not-empty to be signaled. */
 task_connect(&self->pb_stat);
 if (ATOMIC_READ(self->pb_state.pbs_count) != 0) {
  task_disconnect();
  goto again_full;
 }
 /* Do the actual wait. */
 task_wait();
 goto again_full;
}

/* An extended variant of `packetbuffer_write()' which can be used to
 * construct a single packet with the data from more than one location.
 * @param: iov_offset: Offset from the start of the iov vector, describing
 *                     a number of bytes that should not be transferred.
 *                     This number of bytes should not be included in `num_bytes'
 * @param: num_bytes:  The sum of all iov entry lengths in bytes.
 *                     If user-space tinkers around with the IOV lengths
 *                     between the time this value is generated, and the
 *                     time data starts getting copied, the results are
 *                     undefined (truncation may cause a user-space segfault,
 *                     extension will cause trailing data to not be included)
 *                     However corruption here follows weak-undefined-behavior,
 *                     meaning that it will not actually crash the kernel. */
PUBLIC size_t KCALL
packetbuffer_writea_vio(struct packetbuffer *__restrict self,
                        USER CHECKED struct iovec const *iov,
                        size_t iov_offset, size_t num_bytes, HOST void const *anc_buffer,
                        size_t ancsize, iomode_t mode, packet_iomode_t packet_mode) {
 struct packet_header header;
 size_t size_avail;
 /* Put together a packet header. */
 if (__builtin_add_overflow(num_bytes,ancsize,&header.p_total))
     goto packet_too_large;
 if (__builtin_add_overflow(header.p_total,sizeof(header)+PACKET_BUFFER_ALIGNMENT-1,&header.p_total))
     goto packet_too_large;
 header.p_total    &= ~(PACKET_BUFFER_ALIGNMENT-1);
 header.p_payload   = (u16)num_bytes;
 header.p_ancillary = (u16)ancsize;
again:
 atomic_rwlock_write(&self->pb_lock);
 if unlikely(header.p_total >
            (self->pb_limt & PBUFFER_LIMT_FMASK)) {
  /* The packet is too big. */
  atomic_rwlock_endwrite(&self->pb_lock);
  goto packet_too_large;
 }
 size_avail  = (self->pb_mask+1);
 size_avail -= self->pb_state.pbs_count;
 if (header.p_total <= size_avail) {
  /* Can immediately write the packet. */
  bool was_empty;
  u16 packet_address,high_size;
  was_empty = self->pb_state.pbs_count == 0;
  packet_address  = self->pb_state.pbs_addr;
  packet_address += self->pb_state.pbs_count;
  packet_address &= self->pb_mask;

  high_size = (self->pb_mask+1)-packet_address;
  if unlikely(sizeof(header) > high_size) {
   /* The packet header itself is split across the edge. */
   memcpy(self->pb_base+packet_address,&header,high_size);
   packet_address = sizeof(header)-high_size;
   memcpy(self->pb_base,(byte_t *)&header+high_size,packet_address);
  } else {
   memcpy(self->pb_base+packet_address,&header,sizeof(header));
   packet_address += sizeof(header);
   packet_address &= self->pb_mask;
  }

  TRY { /* Try is required to guard against faulty user-space buffers. */
   /* Read data from the user-space IOV vector to fill the payload. */
   high_size = (self->pb_mask+1)-packet_address;
   if (high_size >= num_bytes) {
    iov_read(self->pb_base+packet_address,iov,iov_offset,num_bytes,mode);
    packet_address += num_bytes;
    packet_address &= self->pb_mask;
   } else {
    iov_read(self->pb_base+packet_address,iov,iov_offset,high_size,mode);
    packet_address = num_bytes-high_size;
    iov_read(self->pb_base,iov,iov_offset+high_size,packet_address,mode);
   }
   if (ancsize) {
    /* Align the packet address to the next valid address. */
    packet_address +=  (PACKET_BUFFER_ALIGNMENT-1);
    packet_address &= ~(PACKET_BUFFER_ALIGNMENT-1);

    /* Lastly, write ancillary data. */
    high_size = (self->pb_mask+1)-packet_address;
    if (high_size >= ancsize) {
     memcpy(self->pb_base+packet_address,anc_buffer,ancsize);
    } else {
     memcpy(self->pb_base+packet_address,anc_buffer,high_size);
     packet_address = ancsize-high_size;
     memcpy(self->pb_base,(byte_t *)anc_buffer+high_size,packet_address);
    }
   }
   COMPILER_BARRIER(),

   /* And finally, update the buffer state to include the new packet. */
   self->pb_state.pbs_count += header.p_total;
  } FINALLY {
   atomic_rwlock_endwrite(&self->pb_lock);
  }
  /* If the buffer was empty before, signal that data has now arrived. */
  if (was_empty)
      sig_broadcast(&self->pb_stat);
  return num_bytes;
 }
 /* Allocate a larger buffer & goto again */
 {
  size_t new_mask;
  new_mask = self->pb_mask;
  if ((new_mask+1) > (self->pb_limt & PBUFFER_LIMT_FMASK)) {
   /* The mask has already reached its limit */
   atomic_rwlock_endwrite(&self->pb_lock);
  } else {
   byte_t *new_buffer;
   assert(((new_mask+1) & new_mask) == 0);
   while (((new_mask+1) - self->pb_state.pbs_count) <= header.p_total) {
    if ((new_mask+1) >= (self->pb_limt & PBUFFER_LIMT_FMASK)) {
     /* Increase the mask one more time, allow for the limit to act
      * as a related rule when compared against the fact that the actual
      * mask always needs to be a power-of-2 minus 1. */
     new_mask = (new_mask << 1) | 1;
     break;
    }
    new_mask = (new_mask << 1) | 1;
   }
   assertf(new_mask > self->pb_mask,
           "new_mask      = %p\n"
           "self->pb_mask = %p\n",
           new_mask,self->pb_mask);
   atomic_rwlock_endwrite(&self->pb_lock);
   COMPILER_BARRIER();
   /* Allocate the new buffer. */
   new_buffer = (byte_t *)kmalloc(new_mask+1,GFP_SHARED);
   COMPILER_BARRIER();
   atomic_rwlock_write(&self->pb_lock);
   if (new_mask > self->pb_mask) {
    size_t wrap_count;
    if (self->pb_state.pbs_count) {
     /* Copy data from the old buffer. */
     memcpy(new_buffer,self->pb_base,self->pb_mask+1);
     /* Since the buffer is now larger than before, any packet data
      * that may have gotten wrapped around the buffer space must now
      * be copied into extended memory. */
     if (self->pb_state.pbs_addr) {
      size_t additional_memory;
      additional_memory = new_mask-self->pb_mask;
      wrap_count = self->pb_state.pbs_addr;
      if (wrap_count > additional_memory) {
       size_t wrap_offset;
       wrap_offset = wrap_count - additional_memory;
       /* Some memory must remain after the wrapping point. */
       memcpy(new_buffer+(self->pb_mask+1),
              self->pb_base + wrap_offset,
              additional_memory);
       memcpy(new_buffer,
              self->pb_base + additional_memory,
              wrap_offset);
      } else {
       /* All low memory must now be mapped in extended memory. */
       memcpy(new_buffer+(self->pb_mask+1),
              self->pb_base,wrap_count);
      }
     }
    }
    /* Install the new buffer. */
    new_buffer = XCH(self->pb_base,new_buffer);
    self->pb_mask = new_mask;
   }
   atomic_rwlock_endwrite(&self->pb_lock);
   COMPILER_BARRIER();
   kfree(new_buffer);
   COMPILER_BARRIER();
   /* With the new (larger) buffer now installed, loop back around and wait. */
   goto again;
  }
 }

 if (mode & IO_NONBLOCK)
     error_throw(E_WOULDBLOCK); /* We'd have to block to be able to do this... */
 /* Wait for readers to consume packets. */
 task_connect(&self->pb_read);
 atomic_rwlock_read(&self->pb_lock);
 COMPILER_READ_BARRIER();
 size_avail  = (self->pb_mask+1);
 size_avail -= self->pb_state.pbs_count;
 if unlikely(header.p_total <= size_avail) {
  /* See like things changed in the mean time... */
  atomic_rwlock_endread(&self->pb_lock);
  task_disconnect();
  goto again;
 }
 if unlikely(header.p_total >
            (self->pb_limt & PBUFFER_LIMT_FMASK)) {
  /* The buffer limit must have been lowered, and now our packet's too large... */
  atomic_rwlock_endwrite(&self->pb_lock);
  task_disconnect();
  goto packet_too_large;
 }
 atomic_rwlock_endread(&self->pb_lock);
 /* All right. Now wait for space to become available. */
 task_wait();
 goto again;

packet_too_large:
 if (packet_mode & PACKET_IO_FWRSPLIT) {
  size_t limit = ATOMIC_READ(self->pb_limt) & PBUFFER_LIMT_FMASK;
  size_t max_payload = limit - sizeof(struct packet_header);
  /* We're allowed to split the packet. */
  if (!ancsize) {
   size_t part_size;
   size_t written_size = 0;
   /* Without being allowed to block, we can't possible write a packet larger
    * than the buffer size, simply because we'd have to block in order to wait
    * for consumers to deal with data, freeing up memory for us to re-use. */
   if (mode & IO_NONBLOCK)
       error_throw(E_WOULDBLOCK);
   while (written_size < num_bytes) {
    part_size = MIN(max_payload,num_bytes-written_size);
    part_size = packetbuffer_writea_vio(self,iov,iov_offset+written_size,
                                        part_size,NULL,0,mode,packet_mode);
    if (!part_size)
         return written_size;
    written_size += part_size;
   }
   assert(written_size == num_bytes);
   return written_size;
  } else if ((ancsize+sizeof(struct packet_header)+PACKET_BUFFER_ALIGNMENT) > ancsize &&
             (ancsize+sizeof(struct packet_header)+PACKET_BUFFER_ALIGNMENT) <= limit) {
   size_t part_size;
   size_t written_size = 0;
   /* Without being allowed to block, we can't possible write a packet larger
    * than the buffer size, simply because we'd have to block in order to wait
    * for consumers to deal with data, freeing up memory for us to re-use. */
   if (mode & IO_NONBLOCK)
       error_throw(E_WOULDBLOCK);
   assert(num_bytes >= PACKET_BUFFER_ALIGNMENT);
   num_bytes -= PACKET_BUFFER_ALIGNMENT;
   while (written_size < num_bytes) {
    part_size = MIN(max_payload,num_bytes-written_size);
    part_size = packetbuffer_writea_vio(self,iov,iov_offset+written_size,part_size,
                                        NULL,0,mode,packet_mode);
    if (!part_size)
         return written_size;
    written_size += part_size;
   }
   assert(written_size == num_bytes);
   /* Write the last piece of the puzzle.
    * NOTE: This is the one in which we include ancillary data. */
   written_size += packetbuffer_writea_vio(self,iov,iov_offset+written_size,
                                           PACKET_BUFFER_ALIGNMENT,anc_buffer,
                                           ancsize,mode,packet_mode);
   return written_size;
  }
 }
 /* The packet would really be too large... */
 error_throwf(E_NET_ERROR,ERROR_NET_PACKET_TOO_LARGE);
}





DECL_END

#endif /* !GUARD_KERNEL_SRC_NET_PACKETBUFFER_C */
