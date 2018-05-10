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
#ifndef GUARD_KERNEL_MODULES_UNIX_DOMAIN_DOMAIN_C
#define GUARD_KERNEL_MODULES_UNIX_DOMAIN_DOMAIN_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <fs/node.h>
#include <fs/path.h>
#include <net/socket.h>
#include <kernel/bind.h>
#include <string.h>
#include <hybrid/minmax.h>
#include <sched/task.h>
#include <sched/signal.h>
#include <except.h>
#include <sys/un.h>
#include <unistd.h>
#include <bits/poll.h>

#include "unix_socket.h"

DECL_BEGIN

#define UNIX_FD_MAX_DELIVERY  4 /* XXX: Arbitrary hard limit? */

PRIVATE size_t KCALL
unix_senda(struct packetbuffer *__restrict self,
           USER CHECKED struct iovec const *buf, size_t num_bytes,
           USER CHECKED struct cmsghdr const *anc, size_t anc_size,
           iomode_t mode, packet_iomode_t packet_mode) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct cmsghdr const *iter;
 REF struct handle fd_buffer[UNIX_FD_MAX_DELIVERY];
 REF struct handle EXCEPT_VAR *pfd_buffer = fd_buffer;
 unsigned int EXCEPT_VAR fd_count = 0;
 TRY {
  CMSGHDR_FOREACH(iter,anc,anc_size) {
   fd_t fdno;
   if (iter->cmsg_level != SOL_SOCKET)
       error_throw(E_INVALID_ALIGNMENT);
   if (iter->cmsg_type != SCM_RIGHTS)
       error_throw(E_INVALID_ALIGNMENT);
   if (iter->cmsg_len != CMSG_SPACE(sizeof(fd_t)))
       error_throw(E_INVALID_ALIGNMENT);
   if (fd_count >= COMPILER_LENOF(fd_buffer))
       error_throwf(E_NET_ERROR,ERROR_NET_PACKET_TOO_LARGE);
   memcpy(&fdno,CMSG_DATA(iter),sizeof(fd_t));
   COMPILER_READ_BARRIER();
   fd_buffer[fd_count] = handle_get(fdno);
   COMPILER_WRITE_BARRIER();
   ++fd_count;
   COMPILER_WRITE_BARRIER();
  }
  /* File descriptors can only be send alongside some other data.
   * Otherwise, we couldn't detect if the packet actually got delivered. */
  if (fd_count && !num_bytes)
      error_throw(E_INVALID_ARGUMENT);
  /* Send out the packet. */
  result = packetbuffer_writeva(self,
                                buf,
                                num_bytes,
                                fd_buffer,
                                fd_count*sizeof(struct handle),
                                mode,
                                packet_mode);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  while (fd_count--)
      handle_decref(pfd_buffer[fd_count]);
  error_rethrow();
 }
 if (result != num_bytes) {
  /* Packet wasn't delivered, or was only partially delivered.
   * Anyways, ancillary data wasn't send out, so destroy it now. */
  while (fd_count--)
      handle_decref(pfd_buffer[fd_count]);
 }
 return result;
}

PRIVATE bool KCALL
unix_recva(struct packetbuffer *__restrict self,
           USER CHECKED struct iovec const *buf, size_t *__restrict pbufsize,
           USER CHECKED struct cmsghdr *anc, size_t *__restrict pancsize,
           iomode_t mode, packet_iomode_t packet_mode) {
 bool result;
 REF struct handle fd_buffer[UNIX_FD_MAX_DELIVERY];
 REF struct handle EXCEPT_VAR *pfd_buffer = fd_buffer;
 size_t EXCEPT_VAR kernel_anc_bufsize = sizeof(fd_buffer);
 if (!*pancsize) goto no_anc; /* Do not receive ancillary data. */
 result = packetbuffer_readva(self,
                              buf,
                              pbufsize,
                              fd_buffer,
                             (size_t *)&kernel_anc_bufsize,
                              mode,
                              packet_mode);
 if (!result) {
  if unlikely(kernel_anc_bufsize > sizeof(fd_buffer)) {
   /* Shouldn't happen unless some other piece
    * of code gained access to our buffers. */
   *pancsize = 0;
no_anc:
   return packetbuffer_readv(self,
                             buf,
                             pbufsize,
                             mode,
                             packet_mode);
  }
  return false;
 }
 if (kernel_anc_bufsize < sizeof(struct handle)) {
  /* No ancillary data received. */
  *pancsize = 0;
  return true;
 }
 kernel_anc_bufsize /= sizeof(struct handle);
 TRY {
  size_t buf_avail,buf_req;
  USER CHECKED struct cmsghdr *dst = anc;
  unsigned int i,num_handles;
  buf_req   = kernel_anc_bufsize * CMSG_SPACE(sizeof(fd_t));
  buf_avail = *pancsize;
  *pancsize = buf_req;
  /* Copy as many handles as can fit into the user-space buffer. */
  num_handles = buf_avail / CMSG_SPACE(sizeof(fd_t));
  num_handles = MIN(num_handles,kernel_anc_bufsize);
  for (i = 0; i < num_handles; ++i) {
   union {
      struct cmsghdr hdr;
      byte_t data[CMSG_SPACE(sizeof(fd_t))];
      fd_t   align;
   } buf;
   fd_t EXCEPT_VAR fd_no;
   /* Setup the ancillary data header. */
   buf.hdr.cmsg_len   = CMSG_SPACE(sizeof(fd_t));
   buf.hdr.cmsg_level = SOL_SOCKET;
   buf.hdr.cmsg_type  = SCM_RIGHTS;
   /* Setup handle flags. */
   assertf(fd_buffer[i].h_type != HANDLE_TYPE_FNONE &&
           fd_buffer[i].h_type <  HANDLE_TYPE_FCOUNT,
           "fd_buffer[%u/%u] = { %p, %p }",i,num_handles,
           fd_buffer[i].h_mode,
           fd_buffer[i].h_ptr);
   fd_buffer[i].h_flag &= ~IO_SETFD_MASK;
   if (packet_mode & PACKET_IO_FRCLOEXEC)
       fd_buffer[i].h_flag |= IO_HANDLE_FCLOEXEC;
   /* Register the new handle. */
   fd_no = handle_put(fd_buffer[i]);
   *(fd_t *)CMSG_DATA(&buf.hdr) = fd_no;
   TRY {
    /* Copy the entry to user-space. */
    memcpy(dst,&buf,CMSG_SPACE(sizeof(fd_t)));
    *(uintptr_t *)&dst += CMSG_SPACE(sizeof(fd_t));
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    /* Close the faulting handle on error.
     * If user-space pre-initialized the buffer, they at least get
     * a chance to recover from this (although that's unlikely...) */
    handle_close(fd_no);
    error_rethrow();
   }
  }
 } FINALLY {
  /* Drop all the inherited references. */
  while (kernel_anc_bufsize--)
     handle_decref(pfd_buffer[kernel_anc_bufsize]);
 }
 return true;
}





PRIVATE ATTR_NOTHROW void KCALL
UnixSocket_Fini(UnixSocket *__restrict self) {
 if (SOCKET_ISSERVER(self)) {
  AcceptSocket *pending,*next;
  pending = self->us_server.s_accept_list;
  /* Drop all unaccepted sockets that are still pending.
   * If they get destroyed, their finalizers will inform
   * any waiting client socket of the connection failure. */
  while (pending) {
   next = pending->as_pending.le_next;
   socket_decref(&pending->as_socket);
   pending = next;
  }
  /* Unbind the server socket from its filesystem location. */
  if (self->us_bind_node) {
   atomic_rwlock_write(&self->us_bind_node->re_unix.u_lock);
   if (self->us_bind_node->re_unix.u_server == self)
       self->us_bind_node->re_unix.u_server = NULL;
   atomic_rwlock_endwrite(&self->us_bind_node->re_unix.u_lock);
  }
 } else if (SOCKET_ISCLIENT(self)) {
  /* Drop a reference from the connection accept-socket,
   * and clear the ring buffers of all data that was never
   * transmitted. */
  socket_weak_decref(&self->us_client.c_accept->as_socket);
  packetbuffer_fini(&self->us_client.c_client2server);
  packetbuffer_fini(&self->us_client.c_server2client);
 }
 if (self->us_bind_node)
     inode_decref((struct inode *)self->us_bind_node);
 if (self->us_bind_path)
     path_decref(self->us_bind_path);
}


/* Since AF_UNIX doesn't implement a port system, sockets
 * created for it have the same for their socket, and their peer. */
PRIVATE socklen_t KCALL
UnixSocket_GetSockName(UnixSocket *__restrict self,
                       USER CHECKED struct sockaddr_un *buf,
                       socklen_t num_bytes, iomode_t mode) {
 socklen_t result;
 if unlikely(!self->us_bind_path)
    error_throwf(E_NET_ERROR,ERROR_NET_NOT_BOUND);
 result = offsetof(struct sockaddr_un,sun_path);
 if likely(num_bytes >= result) {
  buf->sun_family = AF_UNIX;
  /* Copy the actual path to user-space. */
  result += path_getname(self->us_bind_path,
                         buf->sun_path,
                         num_bytes-result,
                         REALPATH_FPATH);
 } else {
  /* No space at all of the pathname. */
  result += path_getname(self->us_bind_path,
                         NULL,0,REALPATH_FPATH);
 }
 result += 1; /* The terminating NUL-character. */
 return result;
}


PRIVATE ATTR_NOTHROW void KCALL
AcceptSocket_Fini(AcceptSocket *__restrict self) {
 if (socket_tryincref(&self->as_client->us_socket)) {
  /* Close the communication buffers, thus interrupting any
   * recv() or send() operation that may still be in process. */
  packetbuffer_close(&self->as_client->us_client.c_client2server);
  packetbuffer_close(&self->as_client->us_client.c_server2client);
  socket_decref(&self->as_client->us_socket);
 }
 socket_weak_decref(&self->as_client->us_socket);
}
PRIVATE socklen_t KCALL
AcceptSocket_GetSockName(AcceptSocket *__restrict self,
                         USER CHECKED struct sockaddr_un *buf,
                         socklen_t num_bytes, iomode_t mode) {
 socklen_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF UnixSocket *client;
 client = self->as_client;
 if (!socket_tryincref(&client->us_socket))
      error_throwf(E_NET_ERROR,ERROR_NET_CONNECTION_REFUSED);
 TRY {
  result = UnixSocket_GetSockName(client,buf,num_bytes,mode);
 } FINALLY {
  socket_decref(&client->us_socket);
 }
 return result;
}

PRIVATE unsigned int KCALL
AcceptSocket_Poll(AcceptSocket *__restrict self,
                  unsigned int mode) {
 socklen_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF UnixSocket *client;
 client = self->as_client;
 if (!socket_tryincref(&client->us_socket))
      return mode; /* recv() and send() become non-blocking when the connection was closed. */
 TRY {
  /* Poll the ring buffer used to communicate data. */
  if ((mode & POLLIN) && packetbuffer_poll_nonempty(&client->us_client.c_client2server))
       mode |= POLLIN;
  if ((mode & POLLOUT) && packetbuffer_poll_empty(&client->us_client.c_server2client))
       mode |= POLLOUT;
 } FINALLY {
  socket_decref(&client->us_socket);
 }
 return result;
}

PRIVATE bool KCALL
AcceptSocket_Recv(AcceptSocket *__restrict self,
                  USER CHECKED struct iovec const *buf, size_t *__restrict pbufsize,
                  iomode_t mode, packet_iomode_t packet_mode) {
 bool COMPILER_IGNORE_UNINITIALIZED(result);
 REF UnixSocket *client;
 client = self->as_client;
 if (!socket_tryincref(&client->us_socket))
      return 0; /* Connection terminated. */
 TRY {
  result = packetbuffer_readv(&client->us_client.c_client2server,
                               buf,
                               pbufsize,
                               mode,
                               packet_mode);
 } FINALLY {
  socket_decref(&client->us_socket);
 }
 return result;
}

PRIVATE bool KCALL
AcceptSocket_Recva(AcceptSocket *__restrict self,
                   USER CHECKED struct iovec const *buf, size_t *__restrict pbufsize,
                   USER CHECKED struct cmsghdr *anc, size_t *__restrict pancsize,
                   iomode_t mode, packet_iomode_t packet_mode) {
 bool COMPILER_IGNORE_UNINITIALIZED(result);
 REF UnixSocket *client;
 client = self->as_client;
 if (!socket_tryincref(&client->us_socket))
      return 0; /* Connection terminated. */
 TRY {
  result = unix_recva(&client->us_client.c_client2server,
                       buf,
                       pbufsize,
                       anc,
                       pancsize,
                       mode,
                       packet_mode);
 } FINALLY {
  socket_decref(&client->us_socket);
 }
 return result;
}

PRIVATE size_t KCALL
AcceptSocket_Send(AcceptSocket *__restrict self,
                  USER CHECKED struct iovec const *buf, size_t num_bytes,
                  iomode_t mode, packet_iomode_t packet_mode) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF UnixSocket *client;
 client = self->as_client;
 if (!socket_tryincref(&client->us_socket))
      return 0; /* Connection terminated. */
 TRY {
  result = packetbuffer_writev(&client->us_client.c_server2client,
                                buf,
                                num_bytes,
                                mode,
                                packet_mode);
 } FINALLY {
  socket_decref(&client->us_socket);
 }
 return result;
}

PRIVATE size_t KCALL
AcceptSocket_Senda(AcceptSocket *__restrict self,
                   USER CHECKED struct iovec const *buf, size_t num_bytes,
                   USER CHECKED struct cmsghdr const *anc, size_t anc_size,
                   iomode_t mode, packet_iomode_t packet_mode) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF UnixSocket *client;
 client = self->as_client;
 if (!socket_tryincref(&client->us_socket))
      return 0; /* Connection terminated. */
 TRY {
  result = unix_senda(&client->us_client.c_server2client,
                       buf,num_bytes,
                       anc,anc_size,
                       mode,packet_mode);
 } FINALLY {
  socket_decref(&client->us_socket);
 }
 return result;
}



PRIVATE struct socket_ops AcceptSocket_Ops = {
    .so_fini        = (void(KCALL *)(struct socket *__restrict))&AcceptSocket_Fini,
    .so_getsockname = (socklen_t(KCALL *)(struct socket *__restrict,USER CHECKED struct sockaddr *,socklen_t,iomode_t))&AcceptSocket_GetSockName,
    .so_getpeername = (socklen_t(KCALL *)(struct socket *__restrict,USER CHECKED struct sockaddr *,socklen_t,iomode_t))&AcceptSocket_GetSockName,
    .so_poll        = (unsigned int(KCALL *)(struct socket *__restrict,unsigned int))&AcceptSocket_Poll,
    /* Assign some non-NULL pointers to bind() and connect() to make it look
     * like a connection-oriented socket. However, they are never invoked as
     * accept-sockets are constructed with the `SOCKET_STATE_FCONNECTED' and
     * `SOCKET_STATE_FBOUND' flags already pre-set. */
    .so_bind        = (void(KCALL *)(struct socket *__restrict,USER CHECKED struct sockaddr const *,socklen_t,iomode_t))(void *)(uintptr_t)1,
    .so_connect     = (void(KCALL *)(struct socket *__restrict,USER CHECKED struct sockaddr const *,socklen_t,iomode_t))(void *)(uintptr_t)1,
    .so_recv        = (bool(KCALL *)(struct socket *__restrict,USER CHECKED struct iovec const *,size_t *__restrict,iomode_t,packet_iomode_t))&AcceptSocket_Recv,
    .so_send        = (size_t(KCALL *)(struct socket *__restrict,USER CHECKED struct iovec const *,size_t,iomode_t,packet_iomode_t))&AcceptSocket_Send,
    .so_senda       = (size_t(KCALL *)(struct socket *__restrict,USER CHECKED struct iovec const *,size_t,USER CHECKED struct cmsghdr const *,size_t,iomode_t,packet_iomode_t))&AcceptSocket_Senda,
    .so_recva       = (bool(KCALL *)(struct socket *__restrict,USER CHECKED struct iovec const *,size_t *__restrict,USER CHECKED struct cmsghdr *,size_t *__restrict,iomode_t,packet_iomode_t))&AcceptSocket_Recva,
};

PRIVATE ATTR_NOTHROW void KCALL
UnixSocket_DestroyAncillaryData(struct packet_data data) {
 STATIC_ASSERT(sizeof(struct handle) == 2*sizeof(uintptr_t));
 while (data.pd_size >= sizeof(struct handle)) {
  struct handle hnd;
  ((uintptr_t *)&hnd)[0] = PACKET_DATA_GETP(data,data.pd_addr);
  ((uintptr_t *)&hnd)[1] = PACKET_DATA_GETP(data,data.pd_addr+sizeof(uintptr_t));
  handle_decref(hnd);
  data.pd_addr += sizeof(struct handle);
  data.pd_size -= sizeof(struct handle);
 }
}



PRIVATE void KCALL
UnixSocket_Connect(UnixSocket *__restrict self,
                   USER CHECKED struct sockaddr_un const *addr,
                   socklen_t addr_len, iomode_t mode) {
 size_t path_len;
 REF struct path *conn_path;
 REF struct regular_node *conn_node;
 assert(!SOCKET_ISSERVER(self));
 if unlikely(addr_len <= offsetof(struct sockaddr_un,sun_path))
     error_throwf(E_NET_ERROR,ERROR_NET_INVALID_SOCKET_ADDRESS);
 if (addr->sun_family != AF_UNIX)
     error_throwf(E_NET_ERROR,ERROR_NET_INVALID_ADDRESS_FAMILY);
 addr_len -= offsetof(struct sockaddr_un,sun_path);
 path_len = strnlen(addr->sun_path,addr_len);
 /* Lookup the described filesystem path. */
 conn_path = fs_path(NULL,addr->sun_path,path_len,
                    (struct inode **)&conn_node,
                     FS_DEFAULT_ATMODE);
 TRY {
  REF UnixSocket *server;
  if (self->us_socket.s_state & SOCKET_STATE_FBOUND) {
   /* If we've already been bound, check if we're connection to the same location! */
   if (self->us_bind_node != conn_node)
       error_throwf(E_NET_ERROR,ERROR_NET_CANNOT_REBIND);
  }
  if (!INODE_ISREG((struct inode *)conn_node))
       error_throwf(E_NET_ERROR,ERROR_NET_CONNECTION_REFUSED);
  /* Get a reference to the connected server socket. */
  atomic_rwlock_read(&conn_node->re_unix.u_lock);
  server = conn_node->re_unix.u_server;
  if (server && !socket_tryincref((struct socket *)server))
      server = NULL;
  atomic_rwlock_endread(&conn_node->re_unix.u_lock);
  if (!server) /* No active server bound to this port. */
       error_throwf(E_NET_ERROR,ERROR_NET_CONNECTION_REFUSED);
  TRY {
   REF AcceptSocket *accept;
   bool is_first_connection;
   /* Construct the accept-socket for the server. */
   accept = (REF AcceptSocket *)socket_alloc(sizeof(AcceptSocket),
                                            &unix_domain,
                                             self->us_socket.s_type,
                                             self->us_socket.s_proto,
                                            &AcceptSocket_Ops);
   /* The accept-socket comes in a pre-connection, and pre-bound state.
    * This way, we can prevent it from ever invoking the connect() or
    * bind() operator (which aren't actually implemented but just some
    * non-NULL pointers that point into the void to make it look like
    * a connection-oriented socket) */
   accept->as_socket.s_state |= (SOCKET_STATE_FCONNECTED|
                                 SOCKET_STATE_FBOUND);
   /* Create the references that will link the accept socket with the client socket. */
   socket_weak_incref(&accept->as_socket);
   socket_weak_incref(&self->us_socket);
   accept->as_client        = self;   /* Inherit reference. */
   self->us_client.c_accept = accept; /* Inherit reference. */
   /* Add the accept-socket to the server. */
   atomic_rwlock_write(&server->us_server.s_accept_lock);
   /* Check if the server is able to accept our new socket. */
   if unlikely(server->us_server.s_accept_length >= server->us_server.s_accept_limit ||
              (server->us_socket.s_state & SOCKET_STATE_FSHUTRD) ||
             !(server->us_socket.s_state & SOCKET_STATE_FLISTENING)) {
    /* No new connections can be accepted by the server, or the server isn't listening. */
    atomic_rwlock_endwrite(&server->us_server.s_accept_lock);
    ATOMIC_FETCHDEC(accept->as_socket.s_weakcnt);
    self->us_client.c_accept = NULL;
    socket_decref(&accept->as_socket);
    error_throwf(E_NET_ERROR,ERROR_NET_CONNECTION_REFUSED);
   }
   /* Insert the accept-socket into the server's pending chain,
    * and have _it_ inherit our reference to that socket. */
   LIST_INSERT(server->us_server.s_accept_list,
               accept,as_pending); /* Inherit reference. */
   is_first_connection = server->us_server.s_accept_length == 0;
   ++server->us_server.s_accept_length;
   atomic_rwlock_endwrite(&server->us_server.s_accept_lock);
   /* If it's the first connection, wake the server. */
   if (is_first_connection)
       sig_broadcast(&server->us_server.s_firstcon);

  } FINALLY {
   socket_decref(&server->us_socket);
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  inode_decref((struct inode *)conn_node);
  path_decref(conn_path);
  error_rethrow();
 }

 /* Initialize the client buffer.
  * XXX: Buffer limit configuration? */
 packetbuffer_cinit(&self->us_client.c_client2server,4096,&UnixSocket_DestroyAncillaryData);
 packetbuffer_cinit(&self->us_client.c_server2client,4096,&UnixSocket_DestroyAncillaryData);

 /* Deal with the binding of our own socket
  * to the specified filesystem location. */
 if (self->us_socket.s_state & SOCKET_STATE_FBOUND) {
  assert(self->us_bind_node == conn_node);
  inode_decref((struct inode *)conn_node);
  path_decref(conn_path);
 } else {
  self->us_bind_node = conn_node; /* Inherit reference. */
  self->us_bind_path = conn_path; /* Inherit reference. */
  self->us_socket.s_state |= SOCKET_STATE_FBOUND;
 }
}

PRIVATE void KCALL
UnixSocket_Bind(UnixSocket *__restrict self,
                USER CHECKED struct sockaddr_un const *addr,
                socklen_t addr_len, iomode_t mode) {
 USER CHECKED char const *path; size_t path_len;
 REF struct path *COMPILER_IGNORE_UNINITIALIZED(conn_path);
 REF struct path *conn_dirpath;
 REF struct directory_node *conn_dir;
 REF struct regular_node *COMPILER_IGNORE_UNINITIALIZED(conn_node);
 assert(!SOCKET_ISSERVER(self));
 if unlikely(addr_len <= offsetof(struct sockaddr_un,sun_path))
     error_throwf(E_NET_ERROR,ERROR_NET_INVALID_SOCKET_ADDRESS);
 if (addr->sun_family != AF_UNIX)
     error_throwf(E_NET_ERROR,ERROR_NET_INVALID_ADDRESS_FAMILY);
 addr_len -= offsetof(struct sockaddr_un,sun_path);
 path     = addr->sun_path;
 path_len = strnlen(path,addr_len);
 /* Lookup the described filesystem path. */
 conn_dirpath = fs_lastpath(NULL,&path,&path_len,
                           (struct inode **)&conn_dir,
                            FS_DEFAULT_ATMODE|
                            FS_MODE_FDIRECTORY);
 TRY {
  struct directory_entry *conn_entry;
  /* Create the file if it didn't exist before. */
  conn_node = (REF struct regular_node *)directory_creatfile(conn_dir,
                                                             path,
                                                            (u16)path_len,
                                                             0,
                                                             fs_getuid(),
                                                             fs_getgid(),
                                                            ~THIS_FS->fs_umask,
                                                            (struct directory_entry **)&conn_entry);
  TRY {
   /* Create the path for the connection.
    * NOTE: If the path already existed before,
    *       this function will deal with that, too. */
   conn_path = path_newchild(conn_dirpath,
                             conn_dir,
                            (struct inode *)conn_node,
                             conn_entry);
  } FINALLY {
   if (FINALLY_WILL_RETHROW)
       inode_decref((struct inode *)conn_node);
   directory_entry_decref(conn_entry);
  }
 } FINALLY {
  inode_decref((struct inode *)conn_dir);
  path_decref(conn_dirpath);
 }
 /* The filesystem location for the domain socket has been created.
  * Now to simply bind our server to it! */
 TRY {
  /* Literally bind ourself to this filesystem location! */
  atomic_rwlock_write(&conn_node->re_unix.u_lock);
  /* Check if another UNIX domain socket was already bound to this node. */
  if unlikely(conn_node->re_unix.u_server != NULL) {
   atomic_rwlock_endwrite(&conn_node->re_unix.u_lock);
   error_throwf(E_NET_ERROR,ERROR_NET_ADDRESS_IN_USE);
  }
  conn_node->re_unix.u_server = self; /* Weak reference */
  atomic_rwlock_endwrite(&conn_node->re_unix.u_lock);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  inode_decref((struct inode *)conn_node);
  path_decref(conn_path);
  error_rethrow();
 }
 self->us_bind_node = conn_node; /* Inherit reference. */
 self->us_bind_path = conn_path; /* Inherit reference. */
}

PRIVATE unsigned int KCALL
UnixSocket_Poll(UnixSocket *__restrict self,
                unsigned int mode) {
 unsigned int result = 0;
 if (SOCKET_ISSERVER(self)) {
  /* Poll for incoming connections. */
  if (mode & POLLIN) {
   if (self->us_server.s_accept_length)
       result |= POLLIN;
   else {
    /* Connect to the signal sent when new connections arrive. */
    task_connect_ghost(&self->us_server.s_firstcon);
    COMPILER_READ_BARRIER();
    if (self->us_server.s_accept_length)
        result |= POLLIN;
   }
  }
 } else if (SOCKET_ISCLIENT(self)) {
  /* Poll the buffers used to communicate between server and client. */
  if ((mode & POLLIN) && packetbuffer_poll_nonempty(&self->us_client.c_server2client))
       result |= POLLIN;
  if ((mode & POLLOUT) && packetbuffer_poll_empty(&self->us_client.c_client2server))
       result |= POLLOUT;
 }
 return result;
}

PRIVATE void KCALL
UnixSocket_Listen(UnixSocket *__restrict self,
                  unsigned int max_backlog, iomode_t UNUSED(mode)) {
 /* All the fields should still be ZERO-initialized from construction.
  * Since listen() can only be called once, and switches states permanently,
  * coupled with the fact that the caller will set the FLISTENING flag for
  * us, all _we_ really need to do, is to set the max backlog length. */
 assert(!self->us_server.s_accept_list);
 assert(!self->us_server.s_accept_length);
 assert(!self->us_server.s_accept_limit);
 self->us_server.s_accept_limit = max_backlog;
}

PRIVATE REF AcceptSocket *KCALL
UnixSocket_Accept(UnixSocket *__restrict self, iomode_t mode) {
 REF AcceptSocket *result;
 assert(SOCKET_ISSERVER(self));
 assert(!task_isconnected());
again:
 atomic_rwlock_write(&self->us_server.s_accept_lock);
 result = self->us_server.s_accept_list;
 if (result) {
  /* Remove the resulting socket from the chain. */
  LIST_REMOVE(result,as_pending); /* Inherit reference. */
  assert(self->us_server.s_accept_length);
  /* Update the number of remaining connection, yet to be accepted. */
  --self->us_server.s_accept_length;
  assert((self->us_server.s_accept_length != 0) ==
         (self->us_server.s_accept_list != NULL));
  atomic_rwlock_endwrite(&self->us_server.s_accept_lock);
  return result;
 }
 atomic_rwlock_endwrite(&self->us_server.s_accept_lock);
 if (mode & IO_NONBLOCK)
     return NULL; /* No connections available. */
 /* Connect to the signal used to indicate connection availability. */
 task_connect(&self->us_server.s_firstcon);
 COMPILER_READ_BARRIER();
 /* Check if connection have appeared in the mean time. */
 if (ATOMIC_READ(self->us_server.s_accept_list) != NULL) {
  task_disconnect();
  goto again;
 }
 /* Wait for connection to start appearing. */
 task_wait();
 goto again;
}

PRIVATE bool KCALL
UnixSocket_Recv(UnixSocket *__restrict self,
                USER CHECKED struct iovec const *iov, size_t *__restrict pbufsize,
                iomode_t mode, packet_iomode_t packet_mode) {
 assert(SOCKET_ISCLIENT(self));
 return packetbuffer_readv(&self->us_client.c_server2client,
                            iov,
                            pbufsize,
                            mode,
                            packet_mode);
}

PRIVATE bool KCALL
UnixSocket_Recva(UnixSocket *__restrict self,
                 USER CHECKED struct iovec const *iov, size_t *__restrict pbufsize,
                 USER CHECKED struct cmsghdr *anc, size_t *__restrict pancsize,
                 iomode_t mode, packet_iomode_t packet_mode) {
 assert(SOCKET_ISCLIENT(self));
 return unix_recva(&self->us_client.c_server2client,
                    iov,
                    pbufsize,
                    anc,
                    pancsize,
                    mode,
                    packet_mode);
}

PRIVATE size_t KCALL
UnixSocket_Send(UnixSocket *__restrict self,
                USER CHECKED struct iovec const *buf, size_t num_bytes,
                iomode_t mode, packet_iomode_t packet_mode) {
 assert(SOCKET_ISCLIENT(self));
 return packetbuffer_writev(&self->us_client.c_client2server,
                             buf,
                             num_bytes,
                             mode,
                             packet_mode);
}

PRIVATE size_t KCALL
UnixSocket_Senda(UnixSocket *__restrict self,
                 USER CHECKED struct iovec const *buf, size_t num_bytes,
                 USER CHECKED struct cmsghdr const *anc, size_t anc_size,
                 iomode_t mode, packet_iomode_t packet_mode) {
 assert(SOCKET_ISCLIENT(self));
 return unix_senda(&self->us_client.c_client2server,
                    buf,
                    num_bytes,
                    anc,
                    anc_size,
                    mode,
                    packet_mode);
}



PRIVATE struct socket_ops UnixSocket_Ops = {
    .so_fini        = (void(KCALL *)(struct socket *__restrict))&UnixSocket_Fini,
    .so_connect     = (void(KCALL *)(struct socket *__restrict,USER CHECKED struct sockaddr const *,socklen_t,iomode_t))&UnixSocket_Connect,
    .so_bind        = (void(KCALL *)(struct socket *__restrict,USER CHECKED struct sockaddr const *,socklen_t,iomode_t))&UnixSocket_Bind,
    .so_listen      = (void(KCALL *)(struct socket *__restrict,unsigned int,iomode_t))&UnixSocket_Listen,
    .so_accept      = (REF struct socket *(KCALL *)(struct socket *__restrict,iomode_t))&UnixSocket_Accept,
    .so_poll        = (unsigned int(KCALL *)(struct socket *__restrict,unsigned int))&UnixSocket_Poll,
    .so_getsockname = (socklen_t(KCALL *)(struct socket *__restrict,USER CHECKED struct sockaddr *,socklen_t,iomode_t))&UnixSocket_GetSockName,
    .so_getpeername = (socklen_t(KCALL *)(struct socket *__restrict,USER CHECKED struct sockaddr *,socklen_t,iomode_t))&UnixSocket_GetSockName,
    .so_recv        = (bool(KCALL *)(struct socket *__restrict,USER CHECKED struct iovec const *,size_t *__restrict,iomode_t,packet_iomode_t))&UnixSocket_Recv,
    .so_send        = (size_t(KCALL *)(struct socket *__restrict,USER CHECKED struct iovec const *,size_t,iomode_t,packet_iomode_t))&UnixSocket_Send,
    .so_senda       = (size_t(KCALL *)(struct socket *__restrict,USER CHECKED struct iovec const *,size_t,USER CHECKED struct cmsghdr const *,size_t,iomode_t,packet_iomode_t))&UnixSocket_Senda,
    .so_recva       = (bool(KCALL *)(struct socket *__restrict,USER CHECKED struct iovec const *,size_t *__restrict,USER CHECKED struct cmsghdr *,size_t *__restrict,iomode_t,packet_iomode_t))&UnixSocket_Recva,
};



PRIVATE REF struct socket *KCALL
UnixDomain_CreateSocket(struct socket_domain *__restrict domain,
                        u16 type, u16 proto) {
 /* Currently, unix domain sockets only support SOCK_STREAM with PF_UNIX */
 if (type != SOCK_STREAM)
     error_throwf(E_NET_ERROR,ERROR_NET_UNSUPPORTED_TYPE);
 if (proto != PF_UNIX)
     error_throwf(E_NET_ERROR,ERROR_NET_UNSUPPORTED_PROTOCOL);

 /* Allocate a new Unix domain socket.
  * NOTE: All the remaining fields are automatically ZERO-initialized.
  *       All of the magic then only happens in the callbacks of `UnixSocket_Ops' */
 return socket_alloc(sizeof(UnixSocket),
                     domain,
                     type,
                     proto,
                    &UnixSocket_Ops);
}


INTERN struct socket_domain unix_domain = {
    .sd_domain = AF_UNIX,
    .sd_flags  = SOCKET_DOMAIN_FNORMAL,
    .sd_driver = &this_driver,
    .sd_socket = &UnixDomain_CreateSocket,
};
DEFINE_SOCKET_DOMAIN(unix_domain);

DECL_END

#endif /* !GUARD_KERNEL_MODULES_UNIX_DOMAIN_DOMAIN_C */
