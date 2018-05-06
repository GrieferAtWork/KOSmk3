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
#ifndef GUARD_KERNEL_INCLUDE_NET_PACKETBUFFER_H
#define GUARD_KERNEL_INCLUDE_NET_PACKETBUFFER_H 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/list/list.h>
#include <kos/types.h>
#include <fs/iomode.h>
#include <bits/uio.h>
#include <sched/signal.h>
#include <assert.h>
#ifndef __INTELLISENSE__
#include <except.h>
#endif

DECL_BEGIN

struct packetbuffer;
struct iovec;

/* Min alignment of all packet buffer data. */
#define PACKET_BUFFER_ALIGNMENT __SIZEOF_POINTER__

struct packet_data {
    byte_t *pd_base;   /* [1..1] Base address of the buffer. */
    size_t  pd_mask;   /* Mask of wrapping addresses within the buffer (power-of-2 minus 1). */
    size_t  pd_addr;   /* Offset into the buffer where data starts. */
    size_t  pd_size;   /* [!0] Number of data bytes. */
};
#define PACKET_DATA_GETB(x,offset) \
     (*(u8 *)((x).pd_base + (((x).pd_addr + (offset)) & (x).pd_mask)))
#define PACKET_DATA_GETW(x,offset) \
     (*(u16 *)((x).pd_base + (((x).pd_addr + (offset)) & (x).pd_mask)))
#define PACKET_DATA_GETL(x,offset) \
     (*(u32 *)((x).pd_base + (((x).pd_addr + (offset)) & (x).pd_mask)))
#define PACKET_DATA_GETP(x,offset) \
     (*(uintptr_t *)((x).pd_base + (((x).pd_addr + (offset)) & (x).pd_mask)))




struct PACKED packet_header {
    u16       p_total;     /* [!0] Total size size of the packet (in bytes, including this header and
                            *      any trailing data used for padding, as well as ancillary data)
                            * NOTE: Aligned by `PACKET_BUFFER_ALIGNMENT'
                            */
    u16       p_payload;   /* Size of the packet's payload (located immediately after this instruction)
                            * NOTE: Allowed be ZERO(0) */
    u16       p_ancillary; /* Size of ancillary data (in bytes; located at the next `PACKET_BUFFER_ALIGNMENT'
                            * aligned address after the payload, and spans for `p_ancillary')
                            * With that in mind:
                            * >> `p_payload + p_ancillary + sizeof(struct packet_header) <= p_total' */
    u16     __p_pad;       /* ... */
};

union PACKED packetbuffer_state {
    ATOMIC_DATA u32     pbs_state; /* Packet buffer read state. */
    struct PACKED {
        ATOMIC_DATA u16 pbs_addr;  /* [valid_if(pbs_count != 0)]
                                    * Offset into `pb_base', where the next unread packet starts. */
        ATOMIC_DATA u16 pbs_count; /* The number of currently unread packet bytes (sum of all unread packet `p_total' values). */
    };
};


struct packetbuffer {
    atomic_rwlock_t          pb_lock;  /* Lock for accessing the buffer. */
    union packetbuffer_state pb_state; /* {lock(pb_lock)] The current read-state of the packet buffer.
                                        * NOTE: Writing this field only requires a read-lock on `pb_lock'! */
    byte_t                  *pb_base;  /* [1..pb_mask+1][owned] Base address of the packet buffer's packet data block.
                                        *  This is where all the packets, as well as their associated data is stored. */
    size_t                   pb_mask;  /* Buffer size mask (power-of-2 minus 1) */
#define PBUFFER_LIMT_FMASK   (~3ul)    /* Mask for the buffer limit. */
#define PBUFFER_LIMT_FSTATIC   0x1     /* [const] FLAG: Don't allocate a dynamic buffer. - `pb_base' is statically allocated and fixed-length. */
#define PBUFFER_LIMT_FCLOSED   0x2     /* [lock(WEAK(WRITE_ONCE))] The buffer was closed. */
    size_t                   pb_limt;  /* The max length to which the packet buffer is allowed to grow. */
    /* [0..1][const] An optional callback to destroy unused ancillary data.
     * NOTE: Only invoked if the ancillary data size is non-zero
     * NOTE: Invoked by `packetbuffer_read()' to discard unused ancillary data
     * NOTE: Invoked by `packetbuffer_fini()' on any remaining packets that were never sent.
     * WARNING: This operator may be invoked while `self->pb_lock' is held. */
    /*ATTR_NOTHROW*/void (KCALL *pb_ancillary_fini)(struct packet_data data);
    struct sig               pb_stat; /* Signal broadcast / send to notify the following state changes:
                                       *   - The buffer was closed -> BROADCAST(*)
                                       *   - Data was written to an empty buffer -> BROADCAST(*)
                                       *   - The buffer is empty -> BROADCAST(PBUF_STATE_CHANNEL_EMPTY)
                                       *   - The buffer is full -> BROADCAST(PBUF_STATE_CHANNEL_FULL) */
    struct sig               pb_read; /* Signal broadcast when a packet is read */
};

#define PBUF_STATE_CHANNEL_EMPTY   0x2 /* Channel mask broadcast when the buffer becomes empty. */
#define PBUF_STATE_CHANNEL_FULL    0x4 /* Channel mask broadcast when the buffer becomes full. */


#define PACKETBUFFER_INIT(limt) \
    { ATOMIC_RWLOCK_INIT, { 0 }, NULL, \
     (limt) & PBUFFER_LIMT_FMASK, SIG_INIT }
#define packetbuffer_init(self,limt,anc_fini) \
    (atomic_rwlock_init(&(self)->pb_lock), \
    (self)->pb_state.pbs_state = 0, \
    (self)->pb_base = NULL,(self)->pb_mask = 0, \
    (self)->pb_limt = (limt) & PBUFFER_LIMT_FMASK, \
    (self)->pb_ancillary_fini = (anc_fini), \
     sig_init(&(self)->pb_stat))
#define packetbuffer_cinit(self,limt,anc_fini) \
    (atomic_rwlock_cinit(&(self)->pb_lock), \
     assert((self)->pb_state.pbs_state == 0), \
     assert((self)->pb_base == NULL), \
     assert((self)->pb_mask == 0), \
    (self)->pb_limt = (limt) & PBUFFER_LIMT_FMASK, \
    (self)->pb_ancillary_fini = (anc_fini), \
     sig_cinit(&(self)->pb_stat))

/* Finalize a given packet buffer. */
FUNDEF ATTR_NOTHROW void KCALL packetbuffer_fini(struct packetbuffer *__restrict self);

/* Close the buffer and wake all blocking readers/writers. */
FUNDEF ATTR_NOTHROW bool KCALL packetbuffer_close(struct packetbuffer *__restrict self);


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
 *                     meaning that it will not actually crash the kernel.
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was set and the operation would have blocked.
 * @throw: E_INTERRUPT:   The calling thread was interrupted.
 * @throw: E_BADALLOC:    Failed to (re-)allocate the internal packet buffer.
 * @throw: E_NET_ERROR.ERROR_NET_PACKET_TOO_LARGE:
 *                 [!PACKET_IO_FWRSPLIT] The total size of the packet to-be written exceeds `pb_limt',
 *                                     meaning that no matter how long the function would wait, there
 *                                     would never come a time when the buffer would be able to house
 *                                     the packet in its entirety.
 *                 [PACKET_IO_FWRSPLIT]  Even with all of the packet's buf-data except for a single byte
 *                                     placed into any number of packages, `anc_buffer ... ancsize' is
 *                                     still too large to put into a single packet. */
FUNDEF size_t KCALL
packetbuffer_writea_vio(struct packetbuffer *__restrict self,
                        USER CHECKED struct iovec const *iov,
                        size_t iov_offset, size_t num_bytes,
                        HOST void const *anc_buffer, size_t ancsize,
                        iomode_t mode, packet_iomode_t packet_mode);

/* Construct and enqueue a new packet.
 * You may pass `ancsize' as ZERO(0) to create the packet without any ancillary data.
 * NOTE: Passing `num_bytes == 0' will enqueue an empty packet which will still
 *       have the same semantics any any other packet with a buffer size that
 *       wouldn't match ZERO.
 * @param: packet_mode: Either 0, or `PACKET_IO_FWRSPLIT'.
 *                      When `PACKET_IO_FWRSPLIT', allow the packet data (excluding
 *                      ancillary data) to be split across more than one packet.
 *                      Otherwise, or if ancillary data is too large, throw
 *                      an `E_NET_ERROR.ERROR_NET_PACKET_TOO_LARGE' error.
 * @return: * :         The total size of all written payloads.
 *                      Unless `PACKET_IO_FWRSPLIT' is set, this always equals `num_bytes'
 *                      Successfully enqueued a new packet.
 *                      This is the only exit state in which this function has
 *                      inherited passed ancillary data (if there is any)
 *                      In all other exit states, the caller must destroy ancillary
 *                      data manually before proceeding to handle the cause of failure.
 * @return: 0 :         The packet buffer was closed.
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was set and the operation would have blocked.
 * @throw: E_INTERRUPT:   The calling thread was interrupted.
 * @throw: E_BADALLOC:    Failed to (re-)allocate the internal packet buffer.
 * @throw: E_NET_ERROR.ERROR_NET_PACKET_TOO_LARGE:
 *                 [!PACKET_IO_FWRSPLIT] The total size of the packet to-be written exceeds `pb_limt',
 *                                       meaning that no matter how long the function would wait, there
 *                                       would never come a time when the buffer would be able to house
 *                                       the packet in its entirety.
 *                 [PACKET_IO_FWRSPLIT]  Even with all of the packet's buf-data except for a single byte
 *                                       placed into any number of packages, `anc_buffer ... ancsize' is
 *                                       still too large to put into a single packet. */
FORCELOCAL size_t KCALL
packetbuffer_writea(struct packetbuffer *__restrict self,
                    USER CHECKED void const *buf, size_t num_bytes,
                    HOST void const *anc_buffer, size_t ancsize,
                    iomode_t mode, packet_iomode_t packet_mode) {
 struct iovec iov[1];
 iov[0].iov_base = (void *)buf;
 iov[0].iov_len  = num_bytes;
 return packetbuffer_writea_vio(self,
                                iov,
                                0,
                                num_bytes,
                                anc_buffer,
                                ancsize,
                                mode|IO_NOIOVCHECK,
                                packet_mode);
}

FORCELOCAL size_t KCALL
packetbuffer_writeva(struct packetbuffer *__restrict self,
                     USER CHECKED struct iovec const *iov, size_t num_bytes,
                     HOST void const *anc_buffer, size_t ancsize,
                     iomode_t mode, packet_iomode_t packet_mode) {
 return packetbuffer_writea_vio(self,iov,0,num_bytes,anc_buffer,ancsize,mode,packet_mode);
}


FORCELOCAL size_t KCALL
packetbuffer_writev(struct packetbuffer *__restrict self,
                    USER CHECKED struct iovec const *iov, size_t num_bytes,
                    iomode_t mode, packet_iomode_t packet_mode) {
 return packetbuffer_writea_vio(self,iov,0,num_bytes,NULL,0,mode,packet_mode);
}

FORCELOCAL size_t KCALL
packetbuffer_write(struct packetbuffer *__restrict self,
                   USER CHECKED void const *buf, size_t num_bytes,
                   iomode_t mode, packet_iomode_t packet_mode) {
 return packetbuffer_writea(self,
                            buf,
                            num_bytes,
                            NULL,
                            0,
                            mode,
                            packet_mode);
}


/* Same as `packetbuffer_write_stream()', but for IO vectors, as
 * well as able to deal with the `IO_NOIOVCHECK' flag.
 * @return: * : The total number of written bytes.
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was set and the operation would have blocked.
 * @throw: E_INTERRUPT:   The calling thread was interrupted.
 * @throw: E_BADALLOC:    Failed to (re-)allocate the internal packet buffer.
 * @throw: E_NET_ERROR.ERROR_NET_PACKET_TOO_LARGE:
 *                 [!PACKET_IO_FWRSPLIT] The total size of the packet to-be written exceeds `pb_limt',
 *                                     meaning that no matter how long the function would wait, there
 *                                     would never come a time when the buffer would be able to house
 *                                     the packet in its entirety.
 *                 [PACKET_IO_FWRSPLIT]  Even with all of the packet's buf-data except for a single byte
 *                                     placed into any number of packages, `anc_buffer ... ancsize' is
 *                                     still too large to put into a single packet. */
FORCELOCAL size_t KCALL
packetbuffer_writev_stream(struct packetbuffer *__restrict self,
                           USER CHECKED struct iovec const *iov,
                           size_t num_bytes, iomode_t mode) {
 if (!packetbuffer_writev(self,iov,num_bytes,mode,PACKET_IO_FWRSPLIT))
      return 0;
 return num_bytes;
}

/* A convenience function that uses `PACKET_IO_FWRSPLIT' to emulate
 * the behavior of a stream-based buffer, such as `struct ringbuffer'
 * @return: 0 : Either `num_bytes' is ZERO(0), an empty packet was received,
 *              or the packet buffer was closed.
 * @return: * : The actual number of written bytes.
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was set and the operation would have blocked.
 * @throw: E_INTERRUPT:   The calling thread was interrupted.
 * @throw: E_BADALLOC:    Failed to (re-)allocate the internal packet buffer.
 * @throw: E_NET_ERROR.ERROR_NET_PACKET_TOO_LARGE:
 *                 [!PACKET_IO_FWRSPLIT] The total size of the packet to-be written exceeds `pb_limt',
 *                                     meaning that no matter how long the function would wait, there
 *                                     would never come a time when the buffer would be able to house
 *                                     the packet in its entirety.
 *                 [PACKET_IO_FWRSPLIT]  Even with all of the packet's buf-data except for a single byte
 *                                     placed into any number of packages, `anc_buffer ... ancsize' is
 *                                     still too large to put into a single packet. */
FORCELOCAL size_t KCALL
packetbuffer_write_stream(struct packetbuffer *__restrict self,
                          USER CHECKED void *buf, size_t num_bytes,
                          iomode_t mode) {
 struct iovec iov[1];
 iov[0].iov_base = (void *)buf;
 iov[0].iov_len  = num_bytes;
 return packetbuffer_writev_stream(self,
                                   iov,
                                   num_bytes,
                                   mode|IO_NOIOVCHECK);
}



/* Just as `packetbuffer_readv' is for `packetbuffer_read', allow
 * the user-space target buffer to be located in more than one place.
 * @param: anc_buffer: When `NULL', operate the same way `packetbuffer_reada()' would.
 * @param: pbufsize:   Upon entry, contains the total sum of bytes found in `iov'
 * @param: pbufsize:   Upon exit, store the total amount of required buffer space here. */
FUNDEF bool KCALL
packetbuffer_readva(struct packetbuffer *__restrict self,
                    USER CHECKED struct iovec const *iov, size_t *__restrict pbufsize,
                    HOST void *anc_buffer, size_t *__restrict pancsize,
                    iomode_t mode, packet_iomode_t packet_mode);

/* Read a packet including ancillary data.
 * This function differs from `packetbuffer_read()', in that the
 * ancillary data buffer doesn't obey the rules set by `packet_mode':
 *   - PACKET_IO_FRDALWAYS:
 *               If the ancillary data buffer is too small to fit all
 *               ancillary data, return `false' and don't discard the
 *               packet in question, however do update `*pancsize' to
 *               contain the _required_ buffer size for ancillary data.
 *   - PACKET_IO_FRDNEVER:
 *               The ancillary data buffer is ignored and `*pancsize' is
 *               set to ZERO(0). Beyond this, the call will be identical
 *               to an equivalent call to `packetbuffer_read()'.
 *   - PACKET_IO_FRDIFFIT:
 *               Still functions as intended for the user-buffer, but
 *               unless `*pbufsize' was of sufficient size upon entry,
 *               no ancillary data will be written and `*pancsize' will
 *               be set to ZERO(0) before returning `false'.
 *               Additionally, even if the user-buffer is of sufficient
 *               size (and `packetbuffer_read()' would have returned true),
 *               return `false' if the ancillary buffer isn't large enough,
 *               after updating `*pancsize' to the required buffer size.
 *   - PACKET_IO_FRDIFFIT|PACKET_IO_FRDTRUNC:
 *               Still functions as intended for the user-buffer, but
 *               ancillary data is only read once per packet, and as
 *               part of a random, undefined invocation using this mode.
 *               It could be the first time a piece of the packet is
 *               taken away, but it could also be the call when the
 *               last piece is taken, causing the packet to actually
 *               be removed.
 *               If ancillary data exists and should be returned by
 *               the current call, behavior related to `anc_buffer'
 *               matches that described by `PACKET_IO_FRDIFFIT'.
 * @return: true:  Successfully read and dequeued, or truncated a packet.
 *                `*pbufsize' has been updated to the used buffer size (which is <= PRE(*pbufsize))
 *                `*pancsize' has been updated to the used ancillary buffer size (which is <= PRE(*pancsize))
 * @return: false: Failed to read a packet:
 *              - `PACKET_IO_FRDNEVER' was passed in `packet_mode'
 *                 In this case `*pbufsize' is set to the required packet size,
 *                 and as much data as possible (MIN(PRE(*pbufsize),PACKET_SIZE))
 *                 were copied into `buf'
 *                 Additionally, `*pancsize' will have been set to ZERO(0) in this case.
 *              - `PACKET_IO_FRDIFFIT' was passed in `packet_mode' and the
 *                 provided `PRE(*pbufsize)' was not of sufficient size.
 *                 In this case `POST(*pancsize)' is set to ZERO(0)
 *              -  Irregardless of `packet_mode' and `PRE(*pbufsize)' / `POST(*pbufsize)',
 *                 the ancillary data buffer is too small.
 *                 In this case `POST(*pancsize)' is updated to contain the required ancillary
 *                 data buffer size, meaning that `POST(*pancsize) > PRE(*pancsize)'
 *              -  The packet buffer was closed.
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was set and the operation would have blocked.
 * @throw: E_INTERRUPT:   The calling thread was interrupted.
 * @throw: E_SEGFAULT:    The provided user-buffer is faulty. */
FORCELOCAL bool KCALL
packetbuffer_reada(struct packetbuffer *__restrict self,
                   USER CHECKED void *buf, size_t *__restrict pbufsize,
                   HOST void *anc_buffer, size_t *__restrict pancsize,
                   iomode_t mode, packet_iomode_t packet_mode) {
 struct iovec iov[1];
 iov[0].iov_base = buf;
 iov[0].iov_len  = *pbufsize;
 return packetbuffer_readva(self,iov,pbufsize,
                            anc_buffer,pancsize,mode,
                            packet_mode|IO_NOIOVCHECK);
}

/* An extended variant of `packetbuffer_read()' which can be used
 * to store the data of a single packet in more than one location.
 * @param: pbufsize: Upon entry, contains the total sum of bytes found in `iov'
 * @param: pbufsize: Upon exit, store the total amount of required buffer space here. */
FORCELOCAL bool KCALL
packetbuffer_readv(struct packetbuffer *__restrict self,
                   USER CHECKED struct iovec const *iov,
                   size_t *__restrict pbufsize,
                   iomode_t mode, packet_iomode_t packet_mode) {
 return packetbuffer_readva(self,
                            iov,
                            pbufsize,
                            NULL,
                            NULL,
                            mode,
                            packet_mode);
}


/* Read a packet from the buffer.
 * If a packet was read (`return == true'), and that packet contained
 * ancillary data, that data is destroyed using the `pb_ancillary_fini'
 * callback (if set), or simply discarded (if not set).
 * @param: packet_mode: Set of `PACKETBUFFER_IO_F*':
 *   - PACKET_IO_FRDALWAYS:
 *               Always discard the packet, irregardless of whether or
 *               not it actually fit into the provided user-buffer.
 *   - PACKET_IO_FRDNEVER:
 *               Never discard the packet and operate according to peek() semantics.
 *               Always return `false' (become the packet is never actually removed)
 *   - PACKET_IO_FRDIFFIT:
 *               Read as much of the packet as can fit into the provided
 *               user buffer, however in the event of the buffer being
 *               too small, update `*pbufsize' to contain the required
 *               buffer size of the packet next in line, and return `false'
 *   - PACKET_IO_FRDIFFIT|PACKET_IO_FRDTRUNC:
 *               Read as much as can be read into the provided user-buffer,
 *               and don't read more than one packet.
 *               However, if the provided user-buffer isn't large enough to
 *               hold the entire packet, rather than returning `false' and
 *               updating `*pbufsize' to contain the required buffer size,
 *               instead fill `*pbufsize' with the read buffer size before
 *               truncating the packet from which data was read to remove
 *               all the data that was read, before returning `true'.
 * @param: pbufsize: PRE(*)  The provided buffer size.
 * @param: pbufsize: POST(*) The required buffer size / size that would have been required.
 *                           Even set if `PACKET_IO_FRDALWAYS' is used and the packet
 *                           couldn't be read again, even if the caller provided a larger
 *                           buffer.
 * @return: true:  Successfully read and dequeued, or truncated a packet.
 *                `*pbufsize' has been updated to the used buffer size (which is <= PRE(*pbufsize))
 * @return: false: Failed to read a packet:
 *              - `PACKET_IO_FRDNEVER' was passed in `packet_mode'
 *                 In this case `*pbufsize' is set to the required packet size,
 *                 and as much data as possible (MIN(PRE(*pbufsize),PACKET_SIZE))
 *                 were copied into `buf'
 *              - `PACKET_IO_FRDIFFIT' was passed in `packet_mode' and the
 *                 provided `PRE(*pbufsize)' was not of sufficient size.
 *              -  The packet buffer was closed.
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was set and the operation would have blocked.
 * @throw: E_INTERRUPT:   The calling thread was interrupted.
 * @throw: E_SEGFAULT:    The provided user-buffer is faulty. */
FORCELOCAL bool KCALL
packetbuffer_read(struct packetbuffer *__restrict self,
                  USER CHECKED void *buf, size_t *__restrict pbufsize,
                  iomode_t mode, packet_iomode_t packet_mode) {
 struct iovec iov[1];
 iov[0].iov_base = buf;
 iov[0].iov_len  = *pbufsize;
 return packetbuffer_readv(self,iov,pbufsize,mode,
                           packet_mode|IO_NOIOVCHECK);
}

/* Same as `packetbuffer_read_stream()', but for IO vectors, as
 * well as able to deal with the `IO_NOIOVCHECK' flag. */
FORCELOCAL size_t KCALL
packetbuffer_readv_stream(struct packetbuffer *__restrict self,
                          USER CHECKED struct iovec const *iov, size_t num_bytes,
                          iomode_t mode, packet_iomode_t packet_mode) {
 size_t result,required_size;
 assert(packet_mode == (PACKET_IO_FRDNEVER) ||
        packet_mode == (PACKET_IO_FRDIFFIT|PACKET_IO_FRDTRUNC));
again:
 required_size = num_bytes;
 if (!packetbuffer_readv(self,iov,&required_size,mode,packet_mode))
      result = 0; /* Emulate EOF */
 else {
  if (!required_size)
       goto again; /* Ignore empty packets. */
  result = num_bytes;
 }
 return result;
}


/* A convenience function that uses `PACKET_IO_FRDIFFIT|PACKET_IO_FRDTRUNC'
 * to emulate the behavior of a stream-based buffer, such as `struct ringbuffer'
 * Note however that this function will only ever read at most a single packet,
 * and will truncate an existing packet is the provided buffer is too small, and
 * the specified mode doesn't equal `PACKET_IO_FRDNEVER'.
 * @param: packet_mode: Must be one of:
 *              - PACKET_IO_FRDNEVER:                  Emulate `ringbuffer_peek()'
 *              - PACKET_IO_FRDIFFIT|PACKET_IO_FRDTRUNC: Emulate `ringbuffer_read()'
 * @return: 0 : Either `num_bytes' is ZERO(0) or the packet buffer was closed.
 * @return: * : The actual number of read bytes.
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was set and the operation would have blocked.
 * @throw: E_INTERRUPT:   The calling thread was interrupted.
 * @throw: E_SEGFAULT:    The provided user-buffer is faulty. */
FORCELOCAL size_t KCALL
packetbuffer_read_stream(struct packetbuffer *__restrict self,
                         USER CHECKED void *buf, size_t num_bytes,
                         iomode_t mode, packet_iomode_t packet_mode) {
 struct iovec iov[1];
 assert(mode == (PACKET_IO_FRDNEVER) ||
        mode == (PACKET_IO_FRDIFFIT|PACKET_IO_FRDTRUNC));
 iov[0].iov_base = buf;
 iov[0].iov_len  = num_bytes;
 return packetbuffer_readv_stream(self,iov,num_bytes,
                                  mode,packet_mode|IO_NOIOVCHECK);
}



/* Poll for a specific buffer conditions.
 * @return: true:  The specified condition was met.
 * @return: false: The specified condition isn't met. */
LOCAL bool KCALL packetbuffer_poll_full(struct packetbuffer *__restrict self);
LOCAL bool KCALL packetbuffer_poll_empty(struct packetbuffer *__restrict self);
LOCAL bool KCALL packetbuffer_poll_nonempty(struct packetbuffer *__restrict self);
LOCAL bool KCALL packetbuffer_poll_close(struct packetbuffer *__restrict self);

/* Wait for a specific buffer conditions.
 * @return: true:  The specified condition was met.
 * @return: false: The given `abs_timeout' expired. */
LOCAL bool KCALL packetbuffer_wait_full(struct packetbuffer *__restrict self, jtime_t abs_timeout);
LOCAL bool KCALL packetbuffer_wait_empty(struct packetbuffer *__restrict self, jtime_t abs_timeout);
LOCAL bool KCALL packetbuffer_wait_nonempty(struct packetbuffer *__restrict self, jtime_t abs_timeout);
LOCAL bool KCALL packetbuffer_wait_close(struct packetbuffer *__restrict self, jtime_t abs_timeout);

/* Check if the given condition is currently met. */
LOCAL ATTR_NOTHROW WEAK bool KCALL packetbuffer_isfull(struct packetbuffer *__restrict self);
LOCAL ATTR_NOTHROW WEAK bool KCALL packetbuffer_isempty(struct packetbuffer *__restrict self);
LOCAL ATTR_NOTHROW WEAK bool KCALL packetbuffer_isnonempty(struct packetbuffer *__restrict self);
LOCAL ATTR_NOTHROW WEAK bool KCALL packetbuffer_isclosed(struct packetbuffer *__restrict self);


#ifndef __INTELLISENSE__
LOCAL ATTR_NOTHROW WEAK bool KCALL
packetbuffer_isfull(struct packetbuffer *__restrict self) {
 bool result;
 atomic_rwlock_read(&self->pb_lock);
 result = (!self->pb_mask || ATOMIC_READ(self->pb_state.pbs_count) >= self->pb_mask+1);
 atomic_rwlock_endread(&self->pb_lock);
 if (!result) result = packetbuffer_isclosed(self);
 return result;
}
LOCAL ATTR_NOTHROW WEAK bool KCALL
packetbuffer_isempty(struct packetbuffer *__restrict self) {
 bool result;
 atomic_rwlock_read(&self->pb_lock);
 result = ATOMIC_READ(self->pb_state.pbs_count) == 0;
 atomic_rwlock_endread(&self->pb_lock);
 if (!result) result = packetbuffer_isclosed(self);
 return result;
}

LOCAL ATTR_NOTHROW WEAK bool KCALL
packetbuffer_isnonempty(struct packetbuffer *__restrict self) {
 bool result;
 atomic_rwlock_read(&self->pb_lock);
 result = ATOMIC_READ(self->pb_state.pbs_count) != 0;
 atomic_rwlock_endread(&self->pb_lock);
 if (!result) result = packetbuffer_isclosed(self);
 return result;
}

LOCAL ATTR_NOTHROW WEAK bool KCALL
packetbuffer_isclosed(struct packetbuffer *__restrict self) {
 return ATOMIC_READ(self->pb_limt) & PBUFFER_LIMT_FCLOSED;
}

LOCAL bool KCALL
packetbuffer_poll_full(struct packetbuffer *__restrict self) {
 task_openchannel(PBUF_STATE_CHANNEL_FULL);
 task_connect_ghost(&self->pb_stat);
 return packetbuffer_isfull(self);
}
LOCAL bool KCALL
packetbuffer_poll_empty(struct packetbuffer *__restrict self) {
 task_openchannel(PBUF_STATE_CHANNEL_EMPTY);
 task_connect_ghost(&self->pb_stat);
 return packetbuffer_isempty(self);
}
LOCAL bool KCALL
packetbuffer_poll_nonempty(struct packetbuffer *__restrict self) {
 task_connect_ghost(&self->pb_stat);
 return packetbuffer_isnonempty(self);
}
LOCAL bool KCALL
packetbuffer_poll_close(struct packetbuffer *__restrict self) {
 task_connect_ghost(&self->pb_stat);
 return packetbuffer_isclosed(self);
}

LOCAL bool KCALL
packetbuffer_wait_full(struct packetbuffer *__restrict self, jtime_t abs_timeout) {
 bool result = true;
 uintptr_t EXCEPT_VAR old_mask;
 assert(!task_isconnected());
 old_mask = task_channelmask(PBUF_STATE_CHANNEL_FULL);
 TRY {
  while (!packetbuffer_isfull(self)) {
   task_connect_ghost(&self->pb_stat);
   if (packetbuffer_isfull(self))
       break;
   if (!task_waitfor(abs_timeout)) {
    result = false;
    break;
   }
  }
 } FINALLY {
  task_channelmask(old_mask);
 }
 return result;
}
LOCAL bool KCALL
packetbuffer_wait_empty(struct packetbuffer *__restrict self, jtime_t abs_timeout) {
 bool result = true;
 uintptr_t EXCEPT_VAR old_mask;
 assert(!task_isconnected());
 old_mask = task_channelmask(PBUF_STATE_CHANNEL_EMPTY);
 TRY {
  while (!packetbuffer_isempty(self)) {
   task_connect_ghost(&self->pb_stat);
   if (packetbuffer_isempty(self))
       break;
   if (!task_waitfor(abs_timeout)) {
    result = false;
    break;
   }
  }
 } FINALLY {
  task_channelmask(old_mask);
 }
 return result;
}
LOCAL bool KCALL
packetbuffer_wait_nonempty(struct packetbuffer *__restrict self, jtime_t abs_timeout) {
 bool result = true;
 uintptr_t EXCEPT_VAR old_mask;
 assert(!task_isconnected());
 old_mask = task_channelmask(0);
 TRY {
  while (!packetbuffer_isnonempty(self)) {
   task_connect_ghost(&self->pb_stat);
   if (packetbuffer_isnonempty(self))
       break;
   if (!task_waitfor(abs_timeout)) {
    result = false;
    break;
   }
  }
 } FINALLY {
  task_channelmask(old_mask);
 }
 return result;
}
LOCAL bool KCALL
packetbuffer_wait_close(struct packetbuffer *__restrict self, jtime_t abs_timeout) {
 bool result = true;
 uintptr_t EXCEPT_VAR old_mask;
 assert(!task_isconnected());
 old_mask = task_channelmask(0);
 TRY {
  while (!packetbuffer_isclosed(self)) {
   task_connect_ghost(&self->pb_stat);
   if (packetbuffer_isclosed(self))
       break;
   if (!task_waitfor(abs_timeout)) {
    result = false;
    break;
   }
  }
 } FINALLY {
  task_channelmask(old_mask);
 }
 return result;
}

#endif /* __INTELLISENSE__ */


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_NET_PACKETBUFFER_H */
