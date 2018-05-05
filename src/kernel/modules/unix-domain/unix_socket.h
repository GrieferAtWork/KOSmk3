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
#ifndef GUARD_KERNEL_MODULES_UNIX_DOMAIN_UNIX_SOCKET_H
#define GUARD_KERNEL_MODULES_UNIX_DOMAIN_UNIX_SOCKET_H 1

#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <kos/types.h>
#include <net/socket.h>
#include <fs/node.h>
#include <fs/ringbuffer.h>

DECL_BEGIN


/* Unix domain sockets come in 3 variants:
 *   #1 Server socket (the one that called `bind()' + `listen()')
 *      - This socket is the one that holds the set of pending
 *        connections yet to be accept(2)-ed
 *   #2 A client socket (the one that called `connect()')
 *      - This one holds the two per-connection data buffers
 *        that are used to communicate between the server and
 *        a connected client.
 *        If also contains auxillery data that can be used to
 *        transfer file descriptors.
 *   #3 Accepted connection socket (the one returned by `accept(2)')
 *      - This is the simplest of them all, as all it really is, is
 *        a weak pointer to the connection `client socket' (see #2)
 */
typedef struct unix_socket UnixSocket;
typedef struct accept_socket AcceptSocket;
struct unix_socket {
    struct socket            us_socket;    /* The underlying socket. */
    REF struct regular_node *us_bind_node; /* [0..1][lock(WRITE_ONCE,us_socket.s_lock)] The bound filesystem node. */
    REF struct path         *us_bind_path; /* [0..1][lock(WRITE_ONCE,us_socket.s_lock)] The bound filesystem path.
                                            * This one's not actually used, except for implementation of
                                            * the `so_getsockname()' and `so_getpeername()' operator,
                                            * which returns this path. */
    union {
        struct {
            atomic_rwlock_t             s_accept_lock;   /* Lock used to guard the socket-accept queue. */
            REF LIST_HEAD(AcceptSocket) s_accept_list;   /* [lock(s_accept_lock)] Chain of pending, not-yet accepted connections. */
            unsigned int                s_accept_length; /* [lock(s_accept_lock)] The current number of queued sockets. */
            unsigned int                s_accept_limit;  /* [lock(s_accept_lock)] The maximum number of sockets that may be queued. */
            struct sig                  s_firstcon;      /* Signal broadcast when a new socket is added to a previously empty queue. */
        }             us_server;    /* [valid_if(SOCKET_ISSERVER(self))] Server data. */
        struct {
            WEAK REF AcceptSocket *c_accept;        /* [0..1][lock(us_socket.s_lock,WRITE_ONCE(in(connect)))]
                                                     * The socket that was accepted by the server.
                                                     * When this socket's reference counter drops to ZERO(0),
                                                     * the connection was closed by the server. */
            struct ringbuffer      c_client2server; /* Buffer of data being sent from the client to the server. */
            struct ringbuffer      c_server2client; /* Buffer of data being sent from the server to the client. */
        }             us_client;    /* [valid_if(SOCKET_ISCLIENT(self))] Client data. */
    };
};
#define SOCKET_ISSERVER(x)   ((x)->us_socket.s_state & SOCKET_STATE_FLISTENING)
#define SOCKET_ISCLIENT(x)   ((x)->us_socket.s_state & SOCKET_STATE_FCONNECTED)

struct accept_socket {
    struct socket               as_socket;  /* The underlying socket. */
    WEAK REF UnixSocket        *as_client;  /* [1..1][const] The accepted client socket.
                                             * When this socket's reference counter drops to ZERO(0),
                                             * the connection was closed by the client. */
    REF LIST_NODE(AcceptSocket) as_pending; /* Link in the chain of pending server connections, yet to-be accepted.
                                             * This field become invalid once the socket has been accepted. */
};


INTDEF struct socket_domain unix_domain;

DECL_END

#endif /* !GUARD_KERNEL_MODULES_UNIX_DOMAIN_UNIX_SOCKET_H */
