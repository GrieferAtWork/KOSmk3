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
#ifndef GUARD_KERNEL_INCLUDE_NET_SOCKET_H
#define GUARD_KERNEL_INCLUDE_NET_SOCKET_H 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/list/list.h>
#include <kos/types.h>
#include <fs/iomode.h>
#include <sched/rwlock.h>
#include <stdbool.h>
#include <bits/socket.h>
#include <bits/socket_type.h>

DECL_BEGIN

struct socket;
struct sockaddr;

struct socket_ops {
    /* [0..1] An optional callback that is invoked during socket destruction.
     * NOTE: Only invoked during socket destruction if the any of
     *       the `SOCKET_STATE_FDESTROYED' flags haven't been set yet. */
    /*ATTR_NOTHROW*/void (KCALL *so_fini)(struct socket *__restrict self);

    /* [0..1][lock(WRITE(s_lock) &&
     *           ((s_state & SOCKET_STATE_FSHUTRD) ||
     *            (s_state & SOCKET_STATE_FSHUTWR))]
     * An optional callback that is invoked when the `shutdown(2)'
     * system call is invoked on this socket.
     * @param: new_flags: Set of `SOCKET_STATE_FSHUT*'
     *                    Flags of the shutdown modes that weren't set before.
     * WARNING: This operator may be invoked prior to complete initialization
     *          of the socket in the case of the implementing driver being
     *          unloaded. */
    /*ATTR_NOTHROW*/void (KCALL *so_shutdown)(struct socket *__restrict self,
                                              u16 new_flags);

    /* [0..1][lock(WRITE(s_lock) &&
     *           !(s_state & SOCKET_STATE_FSHUTRD) &&
     *           !(s_state & SOCKET_STATE_FSHUTWR) &&
     *           !(s_state & SOCKET_STATE_FCONNECTED))]
     * Connect the socket to the specified socket address `addr'.
     * Upon success, the caller will automatically set the `SOCKET_STATE_FCONNECTED' flag.
     * When not implemented (as done by connection-less) sockets, the specified `addr' is
     * saved in the `s_peeraddr' field, which is then passed in future calls to `socket_send()'
     * by forwarding the call to `so_sendto' (if `so_send' isn't implemented either).
     * Additionally, calls to `socket_recv()' will be forwarded to `so_recvfrom' (if `so_recv'
     * isn't implemented either), and packets received from an address other than `s_peeraddr'
     * will be discarded.
     * Also not that in this case, the socket can be ~re-connected~ (if you want to call it that) any number of times.
     * NOTE: Some address families will also generate an automatic local address for the
     *       socket within this operator, which is then stored in `s_sockaddr', following
     *       which the `SOCKET_STATE_FBOUND' flag is set.
     *       If this is done, this operator must check the `SOCKET_STATE_FBOUND' flag
     *       prior to performing the bind itself and act accordingly (either keep
     *       the pre-defined binding, or throw an error)
     *       If this isn't done, but the send() / sendto() operators still require a
     *       local socket address, the address must either be generated then, or require
     *       the socket user to call bind() manually before performing a send().
     * NOTE: This operator is allowed to make use of `s_sockaddr' and `s_sockaddr_len',
     *       as well as `s_peeraddr' and `s_peeraddr_len' to store the connected socket
     *       and peer addresses, however is not required to if the `so_getsockname()'
     *       and/or `so_getpeername()' operators are implemented explicitly.
     * @throw: * :                The connect() failed for some reason. (XXX: net errors?)
     * @throw: E_WOULDBLOCK:     `IO_NONBLOCK' was specified and the operation would have blocked. */
    void (KCALL *so_connect)(struct socket *__restrict self,
                             USER CHECKED struct sockaddr const *addr,
                             socklen_t addr_len, iomode_t mode);

    /* [0..1][lock(WRITE(s_lock) &&
     *           !(s_state & SOCKET_STATE_FSHUTRD) &&
     *           !(s_state & SOCKET_STATE_FSHUTWR) &&
     *           !(s_state & SOCKET_STATE_FBOUND) &&
     *           !(s_state & SOCKET_STATE_FCONNECTED) &&
     *           !(s_state & SOCKET_STATE_FLISTENING))]
     * Bind the socket to the specified socket address `addr'.
     * Upon success, the caller will automatically set the `SOCKET_STATE_FBOUND' flag.
     * When not implemented (as done by connection-less) sockets, the specified `addr' is
     * saved in the `s_sockaddr' field.
     * NOTE: This operator is allowed to make use of `s_sockaddr' and `s_sockaddr_len'
     *       to store the bound socket addresses, however is not required to if the
     *      `so_getsockname()' operator are implemented explicitly.
     * @throw: * :                The bind() failed for some reason. (XXX: net errors?)
     * @throw: E_WOULDBLOCK:     `IO_NONBLOCK' was specified and the operation would have blocked. */
    void (KCALL *so_bind)(struct socket *__restrict self,
                          USER CHECKED struct sockaddr const *addr,
                          socklen_t addr_len, iomode_t mode);

    /* [0..1][lock(WRITE(s_lock) &&
     *           !(s_state & SOCKET_STATE_FSHUTRD) &&
     *           !(s_state & SOCKET_STATE_FSHUTWR) &&
     *            (s_state & SOCKET_STATE_FBOUND) &&
     *           !(s_state & SOCKET_STATE_FLISTENING))]
     * Start listening on a previously bound socket, setting up a buffer
     * for incoming connections with a max length of `max_backlog' entries.
     * @throw: * :                The listen() failed for some reason. (XXX: net errors?)
     * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator.
     * @throw: E_WOULDBLOCK:     `IO_NONBLOCK' was specified and the operation would have blocked. */
    void (KCALL *so_listen)(struct socket *__restrict self,
                            unsigned int max_backlog, iomode_t mode);

    /* [0..1][lock(READ(s_lock) &&
     *           !(s_state & SOCKET_STATE_FSHUTRD) &&
     *            (s_state & SOCKET_STATE_FBOUND) &&
     *            (s_state & SOCKET_STATE_FLISTENING))]
     * Accept a pending client connection and return its socket.
     * If no clients were waiting and `mode & IO_NONBLOCK' is set,
     * return `NULL' immediately, otherwise block until clients
     * become available.
     * @throw: * :                The accept() failed for some reason. (XXX: net errors?)
     * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator.
     * @return: * :    A new reference to the connected client socket.
     * @return: NULL: `IO_NONBLOCK' was set and no clients are available. */
    REF struct socket *(KCALL *so_accept)(struct socket *__restrict self, iomode_t mode);

    /* [0..1][lock(READ(s_lock) &&
     *           !(s_state & SOCKET_STATE_FSHUTRD) &&
     *            (s_state & SOCKET_STATE_FBOUND))]
     * Extract the socket (this end) address of the given socket
     * `self', and store it in the provided user-space buffer.
     * When not implemented, the socket name is taken from the `s_sockaddr' field.
     * @throw: * :                The getsockname() failed for some reason. (XXX: net errors?)
     * @throw: E_WOULDBLOCK:     `IO_NONBLOCK' was specified and the operation would have blocked.
     * @return: * : The required buffer size. (usual rules apply) */
    socklen_t (KCALL *so_getsockname)(struct socket *__restrict self,
                                      USER CHECKED struct sockaddr *buf,
                                      socklen_t buflen, iomode_t mode);

    /* [0..1][lock(READ(s_lock) &&
     *           !(s_state & SOCKET_STATE_FSHUTRD) &&
     *            (s_state & SOCKET_STATE_FCONNECTED))]
     * Extract the peer (other end) address of the given socket
     * `self', and store it in the provided user-space buffer.
     * When not implemented, the peer name is taken from the `s_peeraddr' field.
     * @throw: * :                The getpeername() failed for some reason. (XXX: net errors?)
     * @throw: E_WOULDBLOCK:     `IO_NONBLOCK' was specified and the operation would have blocked.
     * @return: * : The required buffer size. (usual rules apply) */
    socklen_t (KCALL *so_getpeername)(struct socket *__restrict self,
                                      USER CHECKED struct sockaddr *buf,
                                      socklen_t buflen, iomode_t mode);

    /* [0..1][lock(READ(s_lock) &&
     *           !(s_state & SOCKET_STATE_FSHUTRD))]
     * Read a socket option described by `level' and `optname'
     * @throw: * :                The getsockopt() failed for some reason. (XXX: net errors?)
     * @throw: E_NOT_IMPLEMENTED: The specified `level' and `optname' wasn't recognized.
     * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator.
     * @throw: E_WOULDBLOCK:     `IO_NONBLOCK' was specified and the operation would have blocked.
     * @return: * : The required buffer size. (usual rules apply) */
    socklen_t (KCALL *so_getsockopt)(struct socket *__restrict self,
                                     int level, int optname,
                                     USER CHECKED void *buf,
                                     socklen_t buflen, iomode_t mode);

    /* [0..1][lock(WRITE(s_lock) &&
     *           !(s_state & SOCKET_STATE_FSHUTWR))]
     * Set a socket option described by `level' and `optname'
     * @throw: * :                 The setsockopt() failed for some reason. (XXX: net errors?)
     * @throw: E_NOT_IMPLEMENTED:  The specified `level' and `optname' wasn't recognized.
     * @throw: E_NOT_IMPLEMENTED:  Same as not implementing this operator.
     * @throw: E_INVALID_ARGUMENT: The `buflen' argument isn't valid for the specified option.
     * @throw: E_WOULDBLOCK:      `IO_NONBLOCK' was specified and the operation would have blocked.
     * @return: * : The required buffer size. (usual rules apply) */
    void (KCALL *so_setsockopt)(struct socket *__restrict self,
                                int level, int optname,
                                USER CHECKED void const *buf,
                                socklen_t buflen, iomode_t mode);

    /* [0..1][lock(READ(s_lock) &&
     *          (!(s_state & SOCKET_STATE_FSHUTWR) || !(mode & (POLLOUT|POLLWRNORM|POLLWRBAND))) &&
     *          (!(s_state & SOCKET_STATE_FSHUTRD) || !(mode & (POLLIN|POLLPRI|POLLRDNORM|POLLRDBAND))))]
     * Poll to wait for data, or buffer space for data to become available.
     * This is the socket operator hook for the `poll(2)' system calls.
     * @return: 0 : Same as not implementing this operator.
     * @return: * : The set of socket poll conditions currently ready (set of `POLL*' masked by `mode') */
    unsigned int (KCALL *so_poll)(struct socket *__restrict self,
                                  unsigned int mode);

    /* [0..1][lock(READ(s_lock) &&
     *            (s_state & SOCKET_STATE_FCONNECTED) &&
     *           !(s_state & SOCKET_STATE_FSHUTRD))]
     * Read a packet from the socket.
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
     * @throw: * :            The recv() failed for some reason. (XXX: net errors?)
     * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was set and the operation would have blocked.
     * @throw: E_INTERRUPT:   The calling thread was interrupted.
     * @throw: E_SEGFAULT:    The provided user-buffer is faulty. */
    bool (KCALL *so_recv)(struct socket *__restrict self,
                          USER CHECKED struct iovec const *iov, size_t *__restrict pbufsize,
                          iomode_t mode, packet_iomode_t packet_mode);
    /* [0..1][lock(READ(s_lock) &&
     *           !(s_state & SOCKET_STATE_FSHUTRD))]
     * Same as `so_recv', but used for connection-less socket.
     * This is the socket operator hook for the `recvfrom(2)' system call.
     * @param: paddrbuflen: On entry, this pointer can be dereference to
     *                      determine the size of the `addrbuf' buffer.
     *                      On success, this pointer should be updated
     *                      to contain the _required_ size for the
     *                     `addrbuf' buffer. */
    bool (KCALL *so_recvfrom)(struct socket *__restrict self,
                              USER CHECKED struct iovec const *iov, size_t *__restrict pbufsize,
                              USER CHECKED struct sockaddr *addrbuf, socklen_t *__restrict paddrlen,
                              iomode_t mode, packet_iomode_t packet_mode);

    /* [0..1][lock(READ(s_lock) &&
     *            (s_state & SOCKET_STATE_FCONNECTED) &&
     *           !(s_state & SOCKET_STATE_FSHUTWR))]
     * Construct, enqueue and send a new packet.
     * NOTE: Passing `num_bytes == 0' will enqueue an empty packet which will still
     *       have the same semantics any any other packet with a buffer size that
     *       wouldn't match ZERO.
     * @param: packet_mode: Either 0, or `PACKET_IO_FWRSPLIT'.
     *                      When `PACKET_IO_FWRSPLIT', allow the packet data
     *                      to be split across more than one packet.
     * @return: * :         The total size of all written payloads.
     *                      Unless `PACKET_IO_FWRSPLIT' is set, this always equals `num_bytes'
     *                      Successfully enqueued a new packet.
     * @return: 0 :         The peer has closed the connection, or an empty packet was sent.
     * @throw: * :            The send() failed for some reason. (XXX: net errors?)
     * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was set and the operation would have blocked.
     * @throw: E_INTERRUPT:   The calling thread was interrupted.
     * @throw: E_BADALLOC:    Failed to (re-)allocate the internal packet buffer.
     * @throw: E_NET_ERROR.ERROR_NET_PACKET_TOO_LARGE:
     *                 [!PACKET_IO_FWRSPLIT] The total size of the packet to-be written exceeds `pb_limt',
     *                                       meaning that no matter how long the function would wait, there
     *                                       would never come a time when the buffer would be able to house
     *                                       the packet in its entirety. */
    size_t (KCALL *so_send)(struct socket *__restrict self,
                            USER CHECKED struct iovec const *iov, size_t num_bytes,
                            iomode_t mode, packet_iomode_t packet_mode);

    /* [0..1][lock(READ(s_lock) &&
     *           !(s_state & SOCKET_STATE_FSHUTWR))]
     * Same as `so_send', but used for connection-less socket.
     * This is the socket operator hook for the `sendto(2)' system call. */
    size_t (KCALL *so_sendto)(struct socket *__restrict self,
                              USER CHECKED struct iovec const *iov, size_t num_bytes,
                              USER CHECKED struct sockaddr const *addrbuf, socklen_t addrlen,
                              iomode_t mode, packet_iomode_t packet_mode);

    /* Extended variants of `so_recv' and `so_recvfrom' that
     * include information about incoming ancillary data.
     * @param: anc:      Ancillary data. Unless `so_recv' hasn't been implemented,
     *                   this pointer is always [1..1], however if `so_recv' is not
     *                   provided, but this operator is, then `anc' is passed as NULL
     *                   when data should be received without any ancillary information,
     *                   alongside `pancsize' pointing to a size_t with a value of ZERO.
     * @param: pancsize: Upon entry, the size of the ancillary data buffer.
     *                   Upon exit, the amount of data written to the ancillary data buffer.
     *             NOTE: In the event of the ancillary data buffer not being of sufficient
     *                   length, unused trailing data is truncated and `*pancsize' is set
     *                   the contain the size of all data buffers that were received.
     *                   There is no way of detecting that not all ancillary data could
     *                   actually be received! */
    bool (KCALL *so_recva)(struct socket *__restrict self,
                           USER CHECKED struct iovec const *iov, size_t *__restrict pbufsize,
                           USER CHECKED struct cmsghdr *anc, size_t *__restrict pancsize,
                           iomode_t mode, packet_iomode_t packet_mode);
    bool (KCALL *so_recvafrom)(struct socket *__restrict self,
                               USER CHECKED struct iovec const *iov, size_t *__restrict pbufsize,
                               USER CHECKED struct cmsghdr *anc, size_t *__restrict pancsize,
                               USER CHECKED struct sockaddr *addrbuf, socklen_t *__restrict paddrlen,
                               iomode_t mode, packet_iomode_t packet_mode);

    /* Extended variants of `so_send' and `so_sendto' that
     * include information about outgoing ancillary data.
     * @param: anc:     Ancillary data. Unless `so_send' hasn't been implemented,
     *                  this pointer is always [1..1], however if `so_send' is not
     *                  provided, but this operator is, then `anc' is passed as NULL
     *                  when data should be received without any ancillary information,
     *                  alongside `ancsize' having a value of ZERO(0).
     * @param: ancsize: The size of the ancillary data buffer. */
    size_t (KCALL *so_senda)(struct socket *__restrict self,
                             USER CHECKED struct iovec const *iov, size_t num_bytes,
                             USER CHECKED struct cmsghdr const *anc, size_t anc_size,
                             iomode_t mode, packet_iomode_t packet_mode);
    size_t (KCALL *so_sendato)(struct socket *__restrict self,
                               USER CHECKED struct iovec const *iov, size_t num_bytes,
                               USER CHECKED struct cmsghdr const *anc, size_t anc_size,
                               USER CHECKED struct sockaddr const *addrbuf, socklen_t addrlen,
                               iomode_t mode, packet_iomode_t packet_mode);

};

struct socket_domain {
    uintptr_half_t        sd_domain; /* Socket domain ID (One of `AF_*' from <bits/socket.h>) */
#define SOCKET_DOMAIN_FNORMAL 0x0000 /* Normal domain flags. */
    uintptr_half_t        sd_flags;  /* Socket domain flags (Set of `SOCKET_DOMAIN_F*') */
    REF struct driver    *sd_driver; /* [1..1][const][valid_if(!SOCKET_STATE_FDESTROYED)]
                                      * The driver implementing this socket's operators. */
    /* [1..1]
     * Create and return a reference to a new socket for `type' and `proto'.
     * @param: type:  One of `SOCK_*' from <bits/socket_type.h>
     * @param: proto: One of `PF_*' from <bits/socket.h>
     * @throw: E_BADALLOC:                                 [...]
     * @throw: E_NET_ERROR.ERROR_NET_UNSUPPORTED_TYPE:     [...]
     * @throw: E_NET_ERROR.ERROR_NET_UNSUPPORTED_PROTOCOL: [...] */
    REF struct socket *(KCALL *sd_socket)(struct socket_domain *__restrict domain,
                                          u16 type, u16 proto);
    struct {
        atomic_rwlock_t          s_lock;    /* Lock for the chain of created sockets. */
        LIST_HEAD(struct socket) s_sockets; /* [0..1][lock(s_lock)] Chain of created sockets. */
    }                     sd_sockets;
    LIST_NODE(struct socket_domain) sd_chain; /* [lock(INTERNAL(...))] Chain of registered socket domains. */
};

/* Register a new socket domain.
 * @throw: E_DRIVER_CLOSED: The calling driver is being closed. */
FUNDEF void KCALL register_socket_domain(struct socket_domain *__restrict domain);

/* Define a socket domain from which a driver. */
#define DEFINE_SOCKET_DOMAIN(x) \
   DEFINE_DRIVER_INIT(_socket_domain_##x##_init); \
   PRIVATE ATTR_USED ATTR_FREETEXT void KCALL _socket_domain_##x##_init(void) \
   { \
     register_socket_domain(&x); \
   }




struct socket {
    ATOMIC_DATA ref_t        s_refcnt;  /* Socket reference counter. */
    ATOMIC_DATA ref_t        s_weakcnt; /* Socket weak reference counter. */
    struct socket_ops       *s_ops;     /* [1..1][const][valid_if(!SOCKET_STATE_FDESTROYED)] Socket operators. */
    struct socket_domain    *s_domain;  /* [1..1][const][valid_if(!SOCKET_STATE_FDESTROYED)] Socket domain controller. */
    LIST_NODE(struct socket) s_domain_chain; /* [valid_if(!SOCKET_STATE_FDESTROYED)][lock(s_domain->s_sockets.s_lock)] Chain of sockets created by this domain. */
    u16                      s_type;    /* [const] Socket type (One of `SOCK_*' from <bits/socket_type.h>) */
    u16                      s_proto;   /* [const] Socket protocol (One of `PF_*' from <bits/socket.h>) */
#define SOCKET_STATE_FNORMAL     0x0000 /* Normal socket state */
#define SOCKET_STATE_FCONNECTED  0x0001 /* [lock(WRITE_ONCE)] The socket has been connected (`connect(2)').
                                         * NOTE: The `WRITE_ONCE' tag does not apply to connection-less sockets,
                                         *       however since connections-less sockets shouldn't even be using
                                         *       this field, that detail shouldn't really matter, either. */
#define SOCKET_STATE_FBOUND      0x0002 /* [lock(WRITE_ONCE)] The socket has been bound (`bind(2)').
                                         * NOTE: Code is allowed to assume that this flag
                                         *       implies `SOCKET_STATE_FCONNECTED == 0' */
#define SOCKET_STATE_FLISTENING  0x0004 /* [lock(WRITE_ONCE)] The socket is listening for new connections (`listen(2)').
                                         * NOTE: Code is allowed to assume that this flag implies `SOCKET_STATE_FBOUND' */
#define SOCKET_STATE_FSHUTRD     0x0010 /* [lock(WRITE_ONCE)] The socket has been shut down for reading (`shutdown(2)') */
#define SOCKET_STATE_FSHUTWR     0x0020 /* [lock(WRITE_ONCE)] The socket has been shut down for writing (`shutdown(2)') */
#define SOCKET_STATE_FDESTROYED  0x0040 /* [lock(WRITE_ONCE)] Implies all shutdown flags.
                                         *  Set to close sockets controlled by a driver when that driver is unloaded. */
    u32                     s_state;    /* [lock(s_lock)] Socket state (Set of `SOCKET_STATE_F*') */
    struct sockaddr_storage s_sockaddr; /* [lock(s_lock)][valid_if(SOCKET_STATE_FBOUND)] The bound local socket address. */
    struct sockaddr_storage s_peeraddr; /* [lock(s_lock)][valid_if(SOCKET_STATE_FCONNECTED)] The connected peer socket address. */
    socklen_t               s_sockaddr_len; /* [lock(s_lock)][valid_if(SOCKET_STATE_FBOUND)] The length of the `s_sockaddrs' address. */
    socklen_t               s_peeraddr_len; /* [lock(s_lock)][valid_if(SOCKET_STATE_FCONNECTED)] The length of the `s_peeraddr' address. */
    rwlock_t                s_lock;     /* Lock for accessing this socket. */
    /* Additional domain, type and/or protocol-specific data is located here. */
};



/* Lookup the proper domain controller and construct a
 * new socket in accordance to the specified arguments.
 * @throw: E_BADALLOC:                                 [...]
 * @throw: E_NET_ERROR.ERROR_NET_UNSUPPORTED_DOMAIN:   [...]
 * @throw: E_NET_ERROR.ERROR_NET_UNSUPPORTED_TYPE:     [...]
 * @throw: E_NET_ERROR.ERROR_NET_UNSUPPORTED_PROTOCOL: [...] */
FUNDEF ATTR_RETNONNULL REF struct socket *
KCALL socket_create(u16 domain, u16 type, u16 proto);



/* Allocate a new socket structure of `struct_size' bytes.
 * The following fields will have already been initialized:
 *   - s_refcnt       (Initialized to ONE(1))
 *   - s_weakcnt      (Initialized to ONE(1))
 *   - s_ops          (Initialized to the provided `ops')
 *   - s_domain       (Initialized to the provided `domain')
 *   - s_domain_chain (Initialized to as part of the domain's socket instance chain)
 *   - s_type         (Initialized to the provided `type')
 *   - s_proto        (Initialized to the provided `proto')
 *   - s_state        (Initialized to `SOCKET_STATE_FNORMAL')
 *   - s_lock         (Initialized in accordance to `rwlock_(c)init()')
 *   - *              (All fields following the end of the `struct socket' structure,
 *                     which are located before `struct_size' are initialized to ZERO(0))
 * NOTE: This function is intended to be called from `struct socket_domain::sd_socket'
 * @throw: E_DRIVER_CLOSED: The driver reponsible for `domain' is being closed.
 * @return: * : An initial reference to the newly (and partially) constructed socket. */
FUNDEF ATTR_RETNONNULL REF struct socket *
KCALL socket_alloc(size_t struct_size,
                   struct socket_domain *__restrict domain,
                   u16 type, u16 proto,
                   struct socket_ops *__restrict ops);

/* Prior to being fully constructed, following a successful call to `socket_alloc()'
 * that was in itself followed by the failed calls to other initialization functions,
 * undo everything done by `sock_alloc()' and free the socket structure. */
FUNDEF void KCALL socket_free(struct socket *__restrict sock);


/* Increment/decrement the reference counter of the given socket `x' */
#define socket_tryincref(self) ATOMIC_INCIFNONZERO((self)->s_refcnt)
#define socket_incref(self)    ATOMIC_FETCHINC((self)->s_refcnt)
#define socket_decref(self)   (ATOMIC_DECFETCH((self)->s_refcnt) || (socket_destroy(self),0))
FUNDEF ATTR_NOTHROW void KCALL socket_destroy(struct socket *__restrict self);

#define socket_weak_incref(self)    ATOMIC_FETCHINC((self)->s_weakcnt)
#define socket_weak_decref(self)   (ATOMIC_DECFETCH((self)->s_weakcnt) || (socket_weak_destroy(self),0))
FUNDEF ATTR_NOTHROW void KCALL socket_weak_destroy(struct socket *__restrict self);



/* Socket operator invocation. */


/* Shut down a given socket for reading or writing.
 * @param: how: Set of `SOCKET_STATE_FSHUT*'. */
FUNDEF void KCALL socket_shutdown(struct socket *__restrict self, u16 how);

/* Connect the socket to the specified socket address `addr'.
 * Upon success, the caller will automatically set the `SOCKET_STATE_FCONNECTED' flag.
 * When not implemented (as done by connection-less) sockets, the specified `addr' is
 * saved in the `s_peeraddr' field, which is then passed in future calls to `socket_send()'
 * by forwarding the call to `so_sendto' (if `so_send' isn't implemented either).
 * Additionally, calls to `socket_recv()' will be forwarded to `so_recvfrom' (if `so_recv'
 * isn't implemented either), and packets received from an address other than `s_peeraddr'
 * will be discarded.
 * Also not that in this case, the socket can be ~re-connected~ (if you want to call it that) any number of times.
 * NOTE: Some address families will also generate an automatic local address for the
 *       socket within this operator, which is then stored in `s_sockaddr', following
 *       which the `SOCKET_STATE_FBOUND' flag is set.
 *       If this is done, this operator must check the `SOCKET_STATE_FBOUND' flag
 *       prior to performing the bind itself and act accordingly (either keep
 *       the pre-defined binding, or throw an error)
 *       If this isn't done, but the send() / sendto() operators still require a
 *       local socket address, the address must either be generated then, or require
 *       the socket user to call bind() manually before performing a send().
 * @throw: * :            The connect() failed for some reason. (XXX: net errors?)
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was specified and the operation would have blocked.
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN:         [...]
 * @throw: E_NET_ERROR.ERROR_NET_CANNOT_RECONNECT: [...] */
FUNDEF void KCALL socket_connect(struct socket *__restrict self,
                                 USER CHECKED struct sockaddr const *addr,
                                 socklen_t addr_len, iomode_t mode);

/* Bind the socket to the specified socket address `addr'.
 * Upon success, the caller will automatically set the `SOCKET_STATE_FBOUND' flag.
 * When not implemented (as done by connection-less) sockets, the specified `addr' is
 * saved in the `s_sockaddr' field.
 * @throw: * :            The bind() failed for some reason. (XXX: net errors?)
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was specified and the operation would have blocked.
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN:      [...]
 * @throw: E_NET_ERROR.ERROR_NET_CANNOT_REBIND: [...] */
FUNDEF void KCALL socket_bind(struct socket *__restrict self,
                              USER CHECKED struct sockaddr const *addr,
                              socklen_t addr_len, iomode_t mode);

/* Start listening on a previously bound socket, setting up a buffer
 * for incoming connections with a max length of `max_backlog' entries.
 * @throw: * :                The listen() failed for some reason. (XXX: net errors?)
 * @throw: E_WOULDBLOCK:     `IO_NONBLOCK' was specified and the operation would have blocked.
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN:          [...]
 * @throw: E_NET_ERROR.ERROR_NET_ALREADY_LISTENING: [...]
 * @throw: E_NET_ERROR.ERROR_NET_NOT_BOUND:         [...]
 * @throw: E_NET_ERROR.ERROR_NET_CANNOT_LISTEN:     [...] */
FUNDEF void KCALL socket_listen(struct socket *__restrict self,
                                unsigned int max_backlog, iomode_t mode);

/* Accept a pending client connection and return its socket.
 * If no clients were waiting and `mode & IO_NONBLOCK' is set,
 * return `NULL' immediately, otherwise block until clients
 * become available.
 * @throw: * :     The accept() failed for some reason. (XXX: net errors?)
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN:          [...]
 * @throw: E_NET_ERROR.ERROR_NET_NOT_BOUND:         [...]
 * @throw: E_NET_ERROR.ERROR_NET_NOT_LISTENING:     [...]
 * @throw: E_NET_ERROR.ERROR_NET_CANNOT_ACCEPT:     [...]
 * @return: * :    A new reference to the connected client socket.
 * @return: NULL: `IO_NONBLOCK' was set and no clients are available. */
FUNDEF REF struct socket *KCALL socket_accept(struct socket *__restrict self, iomode_t mode);

/* Extract the socket (this end) address of the given socket
 * `self', and store it in the provided user-space buffer.
 * When not implemented, the socket name is taken from the `s_sockaddr' field.
 * @throw: * :            The getsockname() failed for some reason. (XXX: net errors?)
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was specified and the operation would have blocked.
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN:  [...]
 * @throw: E_NET_ERROR.ERROR_NET_NOT_BOUND: [...]
 * @return: * : The required buffer size. (usual rules apply) */
FUNDEF socklen_t KCALL socket_getsockname(struct socket *__restrict self,
                                          USER CHECKED struct sockaddr *buf,
                                          socklen_t buflen, iomode_t mode);

/* Extract the peer (other end) address of the given socket
 * `self', and store it in the provided user-space buffer.
 * When not implemented, the peer name is taken from the `s_peeraddr' field.
 * @throw: * :            The getpeername() failed for some reason. (XXX: net errors?)
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was specified and the operation would have blocked.
 * @throw: E_NET_ERROR.ERROR_NET_NOT_CONNECTED: [...]
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN:      [...]
 * @return: * : The required buffer size. (usual rules apply) */
FUNDEF socklen_t KCALL socket_getpeername(struct socket *__restrict self,
                                          USER CHECKED struct sockaddr *buf,
                                          socklen_t buflen, iomode_t mode);

/* Read a socket option described by `level' and `optname'
 * @throw: * :                The getsockopt() failed for some reason. (XXX: net errors?)
 * @throw: E_NOT_IMPLEMENTED: The specified `level' and `optname' wasn't recognized.
 * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator.
 * @throw: E_WOULDBLOCK:     `IO_NONBLOCK' was specified and the operation would have blocked.
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN: [...]
 * @return: * : The required buffer size. (usual rules apply) */
FUNDEF socklen_t KCALL socket_getsockopt(struct socket *__restrict self,
                                         int level, int optname,
                                         USER CHECKED void *buf,
                                         socklen_t buflen, iomode_t mode);

/* Set a socket option described by `level' and `optname'
 * @throw: * :                 The setsockopt() failed for some reason. (XXX: net errors?)
 * @throw: E_NOT_IMPLEMENTED:  The specified `level' and `optname' wasn't recognized.
 * @throw: E_NOT_IMPLEMENTED:  Same as not implementing this operator.
 * @throw: E_INVALID_ARGUMENT: The `buflen' argument isn't valid for the specified option.
 * @throw: E_WOULDBLOCK:      `IO_NONBLOCK' was specified and the operation would have blocked.
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN: [...]
 * @return: * : The required buffer size. (usual rules apply) */
FUNDEF void KCALL socket_setsockopt(struct socket *__restrict self,
                                    int level, int optname,
                                    USER CHECKED void const *buf,
                                    socklen_t buflen, iomode_t mode);

/* Poll to wait for data, or buffer space for data to become available.
 * This is the socket operator hook for the `poll(2)' system calls.
 * @return: 0 :   Same as not implementing this operator.
 * @return: mode: The socket has been shut down (any operation will no longer block).
 * @return: * :   The set of socket poll conditions currently ready (set of `POLL*' masked by `mode') */
FUNDEF unsigned int KCALL socket_poll(struct socket *__restrict self,
                                      unsigned int mode);

/* Read a packet from the socket.
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
 * @throw: * :            The recv() failed for some reason. (XXX: net errors?)
 * @throw: E_NOT_IMPLEMENTED: The socket implements no way of receiving.
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN:      [...]
 * @throw: E_NET_ERROR.ERROR_NET_NOT_CONNECTED: [...]
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was set and the operation would have blocked.
 * @throw: E_INTERRUPT:   The calling thread was interrupted.
 * @throw: E_SEGFAULT:    The provided user-buffer is faulty. */
FUNDEF bool KCALL socket_recv(struct socket *__restrict self,
                              USER CHECKED struct iovec const *iov,
                              size_t *__restrict pbufsize,
                              iomode_t mode, packet_iomode_t packet_mode);

/* Same as `so_recv', but used for connection-less socket.
 * This is the socket operator hook for the `recvfrom(2)' system call.
 * @param: paddrbuflen: On entry, this pointer can be dereference to
 *                      determine the size of the `addrbuf' buffer.
 *                      On success, this pointer should be updated
 *                      to contain the _required_ size for the
 *                     `addrbuf' buffer. */
FUNDEF bool KCALL socket_recvfrom(struct socket *__restrict self,
                                  USER CHECKED struct iovec const *iov,
                                  size_t *__restrict pbufsize,
                                  USER CHECKED struct sockaddr *addrbuf,
                                  socklen_t *__restrict paddrlen,
                                  iomode_t mode, packet_iomode_t packet_mode);


/* Construct, enqueue and send a new packet.
 * NOTE: Passing `num_bytes == 0' will enqueue an empty packet which will still
 *       have the same semantics any any other packet with a buffer size that
 *       wouldn't match ZERO.
 * @param: packet_mode: Either 0, or `PACKET_IO_FWRSPLIT'.
 *                      When `PACKET_IO_FWRSPLIT', allow the packet
 *                      data to be split across more than one packet.
 * @return: * :         The total size of all written payloads.
 *                      Unless `PACKET_IO_FWRSPLIT' is set, this always equals `num_bytes'
 *                      Successfully enqueued a new packet.
 * @return: 0 :         The peer has closed the connection, or an empty packet was sent.
 * @throw: * :            The send() failed for some reason. (XXX: net errors?)
 * @throw: E_NOT_IMPLEMENTED: The socket provides no way of sending data.
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN:         [...]
 * @throw: E_NET_ERROR.ERROR_NET_CANNOT_RECONNECT: [...]
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was set and the operation would have blocked.
 * @throw: E_INTERRUPT:   The calling thread was interrupted.
 * @throw: E_BADALLOC:    Failed to (re-)allocate the internal packet buffer.
 * @throw: E_NET_ERROR.ERROR_NET_PACKET_TOO_LARGE:
 *                 [!PACKET_IO_FWRSPLIT] The total size of the packet to-be written exceeds `pb_limt',
 *                                       meaning that no matter how long the function would wait, there
 *                                       would never come a time when the buffer would be able to house
 *                                       the packet in its entirety. */
FUNDEF size_t KCALL socket_send(struct socket *__restrict self,
                                USER CHECKED struct iovec const *iov, size_t num_bytes,
                                iomode_t mode, packet_iomode_t packet_mode);

/* Same as `so_send', but used for connection-less socket.
 * This is the socket operator hook for the `sendto(2)' system call. */
FUNDEF size_t KCALL socket_sendto(struct socket *__restrict self,
                                  USER CHECKED struct iovec const *iov, size_t num_bytes,
                                  USER CHECKED struct sockaddr const *addrbuf, socklen_t addrlen,
                                  iomode_t mode, packet_iomode_t packet_mode);



/* Extended variants of `socket_recv' and `socket_recvfrom'
 * that include information about incoming ancillary data.
 * @param: anc:      Ancillary data.
 * @param: pancsize: Upon entry, the size of the ancillary data buffer.
 *                   Upon exit, the amount of data written to the ancillary data buffer.
 *             NOTE: In the event of the ancillary data buffer not being of sufficient
 *                   length, unused trailing data is truncated and `*pancsize' is set
 *                   the contain the size of all data buffers that were received.
 *                   There is no way of detecting that not all ancillary data could
 *                   actually be received! */
FUNDEF bool KCALL socket_recva(struct socket *__restrict self,
                               USER CHECKED struct iovec const *iov, size_t *__restrict pbufsize,
                               USER CHECKED struct cmsghdr *anc, size_t *__restrict pancsize,
                               iomode_t mode, packet_iomode_t packet_mode);
FUNDEF bool KCALL socket_recvafrom(struct socket *__restrict self,
                                   USER CHECKED struct iovec const *iov, size_t *__restrict pbufsize,
                                   USER CHECKED struct cmsghdr *anc, size_t *__restrict pancsize,
                                   USER CHECKED struct sockaddr *addrbuf, socklen_t *__restrict paddrlen,
                                   iomode_t mode, packet_iomode_t packet_mode);

/* Extended variants of `socket_send' and `socket_sendto' that
 * include information about outgoing ancillary data.
 * @param: anc:     Ancillary data.
 * @param: ancsize: The size of the ancillary data buffer. */
FUNDEF size_t KCALL socket_senda(struct socket *__restrict self,
                                 USER CHECKED struct iovec const *iov, size_t num_bytes,
                                 USER CHECKED struct cmsghdr const *anc, size_t anc_size,
                                 iomode_t mode, packet_iomode_t packet_mode);
FUNDEF size_t KCALL socket_sendato(struct socket *__restrict self,
                                   USER CHECKED struct iovec const *iov, size_t num_bytes,
                                   USER CHECKED struct cmsghdr const *anc, size_t anc_size,
                                   USER CHECKED struct sockaddr const *addrbuf, socklen_t addrlen,
                                   iomode_t mode, packet_iomode_t packet_mode);


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_NET_SOCKET_H */
