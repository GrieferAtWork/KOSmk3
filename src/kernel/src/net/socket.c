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
#ifndef GUARD_KERNEL_SRC_NET_SOCKET_C
#define GUARD_KERNEL_SRC_NET_SOCKET_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <net/socket.h>
#include <except.h>
#include <string.h>
#include <hybrid/minmax.h>
#include <bits/poll.h>
#include <alloca.h>
#include <fs/linker.h>
#include <fs/driver.h>
#include <kernel/bind.h>
#include <sys/uio.h>

DECL_BEGIN

#define throw_net_error(code) error_throwf(E_NET_ERROR,code)



PRIVATE DEFINE_ATOMIC_RWLOCK(socket_domains_lock);
PRIVATE LIST_HEAD(struct socket_domain) socket_domains = NULL;

/* Register a new socket domain. */
PUBLIC void KCALL
register_socket_domain(struct socket_domain *__restrict domain) {
 assertf(domain->sd_driver != NULL,
         "Forgot to initialize `.sd_driver = &this_driver'");
 assertf(domain->sd_socket != NULL,
         "Forgot to initialize `.sd_socket = ...'");
 atomic_rwlock_write(&socket_domains_lock);
 /* NOTE: The reference created here is stored
  *       in the socket domain database. */
 if ((domain->sd_driver->d_app.a_flags & APPLICATION_FCLOSING) ||
     !driver_tryincref(domain->sd_driver)) {
  atomic_rwlock_endwrite(&socket_domains_lock);
  error_throw(E_DRIVER_CLOSED);
 }
 LIST_INSERT(socket_domains,domain,sd_chain);
 atomic_rwlock_endwrite(&socket_domains_lock);
}

PRIVATE void KCALL
clear_socket_domain(struct socket_domain *__restrict domain) {
 struct socket *sock;
again:
 atomic_rwlock_write(&domain->sd_sockets.s_lock);
 sock = domain->sd_sockets.s_sockets;
 if (sock) {
  assert(sock->s_domain == domain);
  /* Must acquire a lock to the socket, because socket's assume that holding
   * a lock to themself is enough to ensure that their driver is still loaded. */
  if (!rwlock_trywrite(&sock->s_lock)) {
   /* Prevent a deadlock scenario with the
    * socket's reference counter reaching ZERO(0). */
   atomic_rwlock_endwrite(&domain->sd_sockets.s_lock);
   task_yield();
   goto again;
  }
  atomic_rwlock_endwrite(&domain->sd_sockets.s_lock);
  if (!(ATOMIC_FETCHOR(sock->s_state,SOCKET_STATE_FDESTROYED) &
                                     SOCKET_STATE_FDESTROYED)) {
   if (sock->s_ops->so_fini)
     (*sock->s_ops->so_fini)(sock);
   atomic_rwlock_write(&domain->sd_sockets.s_lock);
   if (sock->s_domain_chain.le_pself) {
    LIST_REMOVE(sock,s_domain_chain);
    sock->s_domain_chain.le_pself = NULL;
   }
   atomic_rwlock_endwrite(&domain->sd_sockets.s_lock);
  }
  socket_decref(sock);
  goto again;
 }
 atomic_rwlock_endwrite(&domain->sd_sockets.s_lock);
}


DEFINE_GLOBAL_UNBIND_DRIVER(unbind_driver_socket_domains);
PRIVATE ATTR_USED void KCALL
unbind_driver_socket_domains(struct driver *__restrict d) {
 struct socket_domain *domain;
again:
 atomic_rwlock_write(&socket_domains_lock);
 domain = socket_domains;
 for (; domain; domain = domain->sd_chain.le_next) {
  if (domain->sd_driver != d) continue;
  /* Found one! (remove it) */
  LIST_REMOVE(domain,sd_chain);
  atomic_rwlock_endwrite(&socket_domains_lock);
  /* Clear the socket domain of all created sockets. */
  clear_socket_domain(domain);
  /* Drop the reference that was stored in the socket domain database. */
  driver_decref(d);
  /* Search for more domains registered by the given driver. */
  goto again;
 }
 atomic_rwlock_endwrite(&socket_domains_lock);
}




/* Lookup the proper domain controller and construct a
 * new socket in accordance to the specified arguments.
 * @throw: E_BADALLOC:                                 [...]
 * @throw: E_NET_ERROR.ERROR_NET_UNSUPPORTED_DOMAIN:   [...]
 * @throw: E_NET_ERROR.ERROR_NET_UNSUPPORTED_TYPE:     [...]
 * @throw: E_NET_ERROR.ERROR_NET_UNSUPPORTED_PROTOCOL: [...] */
PUBLIC ATTR_RETNONNULL REF struct socket *
KCALL socket_create(u16 domain_, u16 type_, u16 proto_) {
 u16 EXCEPT_VAR domain = domain_;
 u16 EXCEPT_VAR type   = type_;
 u16 EXCEPT_VAR proto  = proto_;
 struct socket_domain *dom;
 REF struct socket *COMPILER_IGNORE_UNINITIALIZED(result);
again:
 atomic_rwlock_read(&socket_domains_lock);
 dom = socket_domains;
 for (; dom; dom = dom->sd_chain.le_next) {
  if (dom->sd_domain != domain) continue;
  if (dom->sd_driver->d_app.a_flags & APPLICATION_FCLOSING) continue;
  if (!driver_tryincref(dom->sd_driver)) continue;
  atomic_rwlock_endread(&socket_domains_lock);
  TRY {
   /* Construct the socket. */
   result = (*dom->sd_socket)(dom,type,proto);
  } FINALLY {
   driver_decref(dom->sd_driver);
   /* Deal with driver-closed errors. */
   if (FINALLY_WILL_RETHROW &&
       error_code() == E_DRIVER_CLOSED) {
    error_handled();
    goto again;
   }
  }
  return result;
 }
 atomic_rwlock_endread(&socket_domains_lock);
 throw_net_error(ERROR_NET_UNSUPPORTED_DOMAIN);
}



/* Allocate a new socket structure of `struct_size' bytes.
 * The following fields will have already been initialized:
 *   - s_refcnt       (Initialized to ONE(1))
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
 * @return: * : An initial reference to the newly (and partially) constructed socket. */
PUBLIC ATTR_RETNONNULL REF struct socket *
KCALL socket_alloc(size_t struct_size,
                   struct socket_domain *__restrict domain,
                   u16 type, u16 proto,
                   struct socket_ops *__restrict ops) {
 REF struct socket *result;
 assert(struct_size >= sizeof(struct socket));
 /* Allocate the socket data structure. */
 result = (REF struct socket *)kmalloc(struct_size,GFP_SHARED|GFP_CALLOC);
 result->s_refcnt  = 1;
 result->s_weakcnt = 1;
 result->s_domain  = domain;
 result->s_ops     = ops;
 result->s_type    = type;
 result->s_proto   = proto;
#if SOCKET_STATE_FNORMAL != 0
 result->s_state   = SOCKET_STATE_FNORMAL;
#endif
 rwlock_cinit(&result->s_lock);

 /* Register the socket as part of the domain. */
 atomic_rwlock_write(&domain->sd_sockets.s_lock);
 if unlikely(domain->sd_driver->d_app.a_flags & APPLICATION_FCLOSING) {
  atomic_rwlock_endwrite(&domain->sd_sockets.s_lock);
  kfree(result);
  error_throw(E_DRIVER_CLOSED);
 }
 LIST_INSERT(domain->sd_sockets.s_sockets,result,s_domain_chain);
 atomic_rwlock_endwrite(&domain->sd_sockets.s_lock);
 return result;
}



/* Prior to being fully constructed, following a successful call to `socket_alloc()'
 * that was in itself followed by the failed calls to other initialization functions,
 * undo everything done by `sock_alloc()' and free the socket structure. */
PUBLIC void KCALL
socket_free(struct socket *__restrict self) {
 while (!rwlock_trywrite(&self->s_lock));
 if (!(ATOMIC_FETCHOR(self->s_state,SOCKET_STATE_FDESTROYED) &
                                    SOCKET_STATE_FDESTROYED)) {
  /* Literally the same as `socket_destroy()', but don't invoke `so_fini' */
  atomic_rwlock_write(&self->s_domain->sd_sockets.s_lock);
  if (self->s_domain_chain.le_pself)
      LIST_REMOVE(self,s_domain_chain);
  atomic_rwlock_endwrite(&self->s_domain->sd_sockets.s_lock);
 }
 kfree(self);
}


PUBLIC ATTR_NOTHROW void KCALL
socket_destroy(struct socket *__restrict self) {
 assert(self->s_refcnt == 0);
 while (!rwlock_trywrite(&self->s_lock));
 if (!(ATOMIC_FETCHOR(self->s_state,SOCKET_STATE_FDESTROYED) &
                                    SOCKET_STATE_FDESTROYED)) {
  if (self->s_ops->so_fini)
    (*self->s_ops->so_fini)(self);
  atomic_rwlock_write(&self->s_domain->sd_sockets.s_lock);
  if (self->s_domain_chain.le_pself)
      LIST_REMOVE(self,s_domain_chain);
  atomic_rwlock_endwrite(&self->s_domain->sd_sockets.s_lock);
 }
 socket_weak_decref(self);
}

PUBLIC ATTR_NOTHROW void KCALL
socket_weak_destroy(struct socket *__restrict self) {
 kfree(self);
}



/* Shut down a given socket for reading or writing.
 * @param: how: Set of `SOCKET_STATE_FSHUT*'. */
PUBLIC void KCALL
socket_shutdown(struct socket *__restrict self, u16 how) {
 u16 new_bits;
 assert((how & ~(SOCKET_STATE_FSHUTRD|SOCKET_STATE_FSHUTWR)) == 0);
 assert((how & (SOCKET_STATE_FSHUTRD|SOCKET_STATE_FSHUTWR)) != 0);
 /* TODO: Implement and use an aggressive-acquire-write function here:
  *       - Send RPC callbacks to all readers, which in turn will then
  *         cause an `E_RETRY_RWLOCK' to be thrown in their threads
  *         in order to get them to release their locks.
  *       - If we do this, we can use shutdown() to interrupt blocking
  *         operations such as recv(), send() or accept(), causing them
  *         to loop around and notice that the socket has been shut down! */
 rwlock_write(&self->s_lock);
 new_bits = ATOMIC_FETCHOR(self->s_state,how);
 /* Figure out which bits become enabled by this shutdown command. */
 new_bits = how & ~new_bits;
 if (new_bits && self->s_ops->so_shutdown)
   (*self->s_ops->so_shutdown)(self,new_bits);
 rwlock_endwrite(&self->s_lock);
}





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
PUBLIC void KCALL
socket_connect(struct socket *__restrict self,
               USER CHECKED struct sockaddr const *addr,
               socklen_t addr_len, iomode_t mode) {
 rwlock_writef(&self->s_lock,mode);
 TRY {
  if (self->s_state & (SOCKET_STATE_FSHUTRD|
                       SOCKET_STATE_FSHUTWR))
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (self->s_ops->so_connect) {
   if (self->s_state & SOCKET_STATE_FCONNECTED)
       throw_net_error(ERROR_NET_CANNOT_RECONNECT);
   (*self->s_ops->so_connect)(self,
                              addr,
                              addr_len,
                              mode);
   self->s_state |= SOCKET_STATE_FCONNECTED;
  } else if (!addr_len) {
   /* ~Disconnect~ a connection-less socket. */
   self->s_state &= ~SOCKET_STATE_FCONNECTED;
  } else {
   /* ~Connect~ a connection-less socket. */
   if (addr_len > sizeof(struct sockaddr_storage))
       error_throw(E_INVALID_ARGUMENT);
   memcpy(&self->s_peeraddr,addr,addr_len);
   self->s_peeraddr_len = addr_len;
   self->s_state |= SOCKET_STATE_FCONNECTED;
  }
 } FINALLY {
  rwlock_endwrite(&self->s_lock);
 }
}


/* Bind the socket to the specified socket address `addr'.
 * Upon success, the caller will automatically set the `SOCKET_STATE_FBOUND' flag.
 * When not implemented (as done by connection-less) sockets, the specified `addr' is
 * saved in the `s_sockaddr' field.
 * @throw: * :            The bind() failed for some reason. (XXX: net errors?)
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was specified and the operation would have blocked.
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN:      [...]
 * @throw: E_NET_ERROR.ERROR_NET_CANNOT_REBIND: [...] */
PUBLIC void KCALL
socket_bind(struct socket *__restrict self,
            USER CHECKED struct sockaddr const *addr,
            socklen_t addr_len, iomode_t mode) {
 rwlock_writef(&self->s_lock,mode);
 TRY {
  if (self->s_state & (SOCKET_STATE_FSHUTRD|
                       SOCKET_STATE_FSHUTWR))
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (self->s_ops->so_bind) {
   if (self->s_state & (SOCKET_STATE_FBOUND|
                        SOCKET_STATE_FCONNECTED|
                        SOCKET_STATE_FLISTENING))
       throw_net_error(ERROR_NET_CANNOT_REBIND);
   (*self->s_ops->so_bind)(self,addr,addr_len,mode);
   self->s_state |= SOCKET_STATE_FBOUND;
  } else if (!addr_len) {
   /* ~Unbind~ a connection-less socket. */
   self->s_state &= ~SOCKET_STATE_FBOUND;
  } else {
   /* ~Bind~ / ~Rebind~ a connection-less socket. */
   if (addr_len > sizeof(struct sockaddr_storage))
       error_throw(E_INVALID_ARGUMENT);
   memcpy(&self->s_sockaddr,addr,addr_len);
   self->s_sockaddr_len = addr_len;
   self->s_state |= SOCKET_STATE_FBOUND;
  }
 } FINALLY {
  rwlock_endwrite(&self->s_lock);
 }
}


/* Start listening on a previously bound socket, setting up a buffer
 * for incoming connections with a max length of `max_backlog' entries.
 * @throw: * :                The listen() failed for some reason. (XXX: net errors?)
 * @throw: E_WOULDBLOCK:     `IO_NONBLOCK' was specified and the operation would have blocked.
 * @throw: E_NET_ERROR.ERROR_NET_ALREADY_LISTENING: [...]
 * @throw: E_NET_ERROR.ERROR_NET_CANNOT_LISTEN:     [...]
 * @throw: E_NET_ERROR.ERROR_NET_NOT_BOUND:         [...]
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN:          [...] */
PUBLIC void KCALL
socket_listen(struct socket *__restrict self,
              unsigned int max_backlog, iomode_t mode) {
 rwlock_writef(&self->s_lock,mode);
 TRY {
  if (self->s_state & (SOCKET_STATE_FSHUTRD|SOCKET_STATE_FSHUTWR))
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (self->s_state & SOCKET_STATE_FLISTENING)
      throw_net_error(ERROR_NET_ALREADY_LISTENING);
  if (!(self->s_state & SOCKET_STATE_FBOUND))
      throw_net_error(ERROR_NET_NOT_BOUND);
  if (!self->s_ops->so_listen)
      throw_net_error(ERROR_NET_CANNOT_LISTEN);
  (*self->s_ops->so_listen)(self,max_backlog,mode);
  self->s_state |= SOCKET_STATE_FLISTENING;
 } FINALLY {
  rwlock_endwrite(&self->s_lock);
 }
}

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
PUBLIC REF struct socket *KCALL
socket_accept(struct socket *__restrict self_, iomode_t mode_) {
 struct socket *EXCEPT_VAR self = self_;
 iomode_t EXCEPT_VAR mode = mode_;
 REF struct socket *COMPILER_IGNORE_UNINITIALIZED(result);
again:
 if (!rwlock_tryread(&self->s_lock)) {
  if (mode & IO_NONBLOCK)
      return NULL;
  rwlock_read(&self->s_lock);
 }
 TRY {
  if (self->s_state & SOCKET_STATE_FSHUTRD)
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (!(self->s_state & SOCKET_STATE_FBOUND))
      throw_net_error(ERROR_NET_NOT_BOUND);
  if (!(self->s_state & SOCKET_STATE_FLISTENING))
      throw_net_error(ERROR_NET_NOT_LISTENING);
  if (!self->s_ops->so_accept)
      throw_net_error(ERROR_NET_CANNOT_ACCEPT);
  result = (*self->s_ops->so_accept)(self,mode);
  assert(result != NULL || (mode & IO_NONBLOCK));
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
 return result;
}


/* Extract the socket (this end) address of the given socket
 * `self', and store it in the provided user-space buffer.
 * When not implemented, the socket name is taken from the `s_sockaddr' field.
 * @throw: * :                The getsockname() failed for some reason. (XXX: net errors?)
 * @throw: E_WOULDBLOCK:     `IO_NONBLOCK' was specified and the operation would have blocked.
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN:  [...]
 * @throw: E_NET_ERROR.ERROR_NET_NOT_BOUND: [...]
 * @return: * : The required buffer size. (usual rules apply) */
PUBLIC socklen_t KCALL
socket_getsockname(struct socket *__restrict self_,
                   USER CHECKED struct sockaddr *buf_,
                   socklen_t buflen_, iomode_t mode_) {
 struct socket *EXCEPT_VAR self = self_;
 USER CHECKED struct sockaddr *EXCEPT_VAR buf = buf_;
 socklen_t EXCEPT_VAR buflen = buflen_;
 iomode_t EXCEPT_VAR mode = mode_;
 socklen_t COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->s_lock,mode);
 TRY {
  if (self->s_state & SOCKET_STATE_FSHUTRD)
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (!(self->s_state & SOCKET_STATE_FBOUND))
      throw_net_error(ERROR_NET_NOT_BOUND);
  if (self->s_ops->so_getsockname) {
   result = (*self->s_ops->so_getsockname)(self,buf,buflen,mode);
  } else {
   /* Copy the saved socket address. */
   result = self->s_sockaddr_len;
   memcpy(buf,&self->s_sockaddr,MIN(result,buflen));
  }
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
 return result;
}


/* Extract the peer (other end) address of the given socket
 * `self', and store it in the provided user-space buffer.
 * When not implemented, the peer name is taken from the `s_peeraddr' field.
 * @throw: * :            The getpeername() failed for some reason. (XXX: net errors?)
 * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was specified and the operation would have blocked.
 * @throw: E_NET_ERROR.ERROR_NET_NOT_CONNECTED: [...]
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN:      [...]
 * @return: * : The required buffer size. (usual rules apply) */
PUBLIC socklen_t KCALL
socket_getpeername(struct socket *__restrict self_,
                   USER CHECKED struct sockaddr *buf_,
                   socklen_t buflen_, iomode_t mode_) {
 struct socket *EXCEPT_VAR self = self_;
 USER CHECKED struct sockaddr *EXCEPT_VAR buf = buf_;
 socklen_t EXCEPT_VAR buflen = buflen_;
 iomode_t EXCEPT_VAR mode = mode_;
 socklen_t COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->s_lock,mode);
 TRY {
  if (self->s_state & SOCKET_STATE_FSHUTRD)
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (!(self->s_state & SOCKET_STATE_FCONNECTED))
      throw_net_error(ERROR_NET_NOT_CONNECTED);
  if (self->s_ops->so_getpeername) {
   result = (*self->s_ops->so_getpeername)(self,buf,buflen,mode);
  } else {
   /* Copy the saved peer address. */
   result = self->s_peeraddr_len;
   memcpy(buf,&self->s_peeraddr,MIN(result,buflen));
  }
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
 return result;
}



/* Read a socket option described by `level' and `optname'
 * @throw: * :                The getsockopt() failed for some reason. (XXX: net errors?)
 * @throw: E_NOT_IMPLEMENTED: The specified `level' and `optname' wasn't recognized.
 * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator.
 * @throw: E_WOULDBLOCK:     `IO_NONBLOCK' was specified and the operation would have blocked.
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN: [...]
 * @return: * : The required buffer size. (usual rules apply) */
PUBLIC socklen_t KCALL
socket_getsockopt(struct socket *__restrict self_,
                  int level_, int optname_,
                  USER CHECKED void *buf_,
                  socklen_t buflen_, iomode_t mode_) {
 struct socket *EXCEPT_VAR self = self_;
 int EXCEPT_VAR level = level_;
 int EXCEPT_VAR optname = optname_;
 USER CHECKED void *EXCEPT_VAR buf = buf_;
 socklen_t EXCEPT_VAR buflen = buflen_;
 iomode_t EXCEPT_VAR mode = mode_;
 socklen_t COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->s_lock,mode);
 TRY {
  if (self->s_state & SOCKET_STATE_FSHUTRD)
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (self->s_ops->so_getsockopt) {
   result = (*self->s_ops->so_getsockopt)(self,level,optname,buf,buflen,mode);
  } else {
   error_throw(E_NOT_IMPLEMENTED);
  }
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
 return result;
}


/* Set a socket option described by `level' and `optname'
 * @throw: * :                 The setsockopt() failed for some reason. (XXX: net errors?)
 * @throw: E_NOT_IMPLEMENTED:  The specified `level' and `optname' wasn't recognized.
 * @throw: E_NOT_IMPLEMENTED:  Same as not implementing this operator.
 * @throw: E_INVALID_ARGUMENT: The `buflen' argument isn't valid for the specified option.
 * @throw: E_WOULDBLOCK:      `IO_NONBLOCK' was specified and the operation would have blocked.
 * @throw: E_NET_ERROR.ERROR_NET_SHUTDOWN: [...]
 * @return: * : The required buffer size. (usual rules apply) */
PUBLIC void KCALL
socket_setsockopt(struct socket *__restrict self_,
                  int level_, int optname_,
                  USER CHECKED void const *buf_,
                  socklen_t buflen_, iomode_t mode_) {
 struct socket *EXCEPT_VAR self = self_;
 int EXCEPT_VAR level = level_;
 int EXCEPT_VAR optname = optname_;
 USER CHECKED void const *EXCEPT_VAR buf = buf_;
 socklen_t EXCEPT_VAR buflen = buflen_;
 iomode_t EXCEPT_VAR mode = mode_;
again:
 rwlock_readf(&self->s_lock,mode);
 TRY {
  if (self->s_state & SOCKET_STATE_FSHUTRD)
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (self->s_ops->so_setsockopt) {
   (*self->s_ops->so_setsockopt)(self,level,optname,buf,buflen,mode);
  } else {
   error_throw(E_NOT_IMPLEMENTED);
  }
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
}


/* Poll to wait for data, or buffer space for data to become available.
 * This is the socket operator hook for the `poll(2)' system calls.
 * @return: 0 :   Same as not implementing this operator.
 * @return: mode: The socket has been shut down (any operation will no longer block).
 * @return: * :   The set of socket poll conditions currently ready (set of `POLL*' masked by `mode') */
PUBLIC unsigned int KCALL
socket_poll(struct socket *__restrict self_,
            unsigned int mode_) {
 struct socket *EXCEPT_VAR self = self_;
 unsigned int EXCEPT_VAR mode = mode;
 unsigned int COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->s_lock,mode);
 TRY {
  result = 0;
  if (self->s_state & SOCKET_STATE_FSHUTRD)
      result |= mode & (POLLIN|POLLPRI|POLLRDNORM|POLLRDBAND);
  if (self->s_state & SOCKET_STATE_FSHUTWR)
      result |= mode & (POLLOUT|POLLWRNORM|POLLWRBAND);
  if (result != mode && self->s_ops->so_poll)
      result |= (*self->s_ops->so_poll)(self,mode & ~result);
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
 return result;
}



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
PUBLIC bool KCALL
socket_recv(struct socket *__restrict self_,
            USER CHECKED struct iovec const *iov_,
            size_t *__restrict pbufsize_,
            iomode_t mode_, packet_iomode_t packet_mode_) {
 struct socket *EXCEPT_VAR self = self_;
 USER CHECKED struct iovec const *EXCEPT_VAR iov = iov_;
 size_t *EXCEPT_VAR pbufsize = pbufsize_;
 iomode_t EXCEPT_VAR mode = mode_;
 packet_iomode_t EXCEPT_VAR packet_mode = packet_mode_;
 bool COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->s_lock,mode);
 TRY {
  if (self->s_state & SOCKET_STATE_FSHUTRD)
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (!(self->s_state & SOCKET_STATE_FCONNECTED))
      throw_net_error(ERROR_NET_NOT_CONNECTED);
  if (self->s_ops->so_recv) {
   result = (*self->s_ops->so_recv)(self,iov,pbufsize,mode,packet_mode);
  } else if (self->s_ops->so_recva) {
   size_t zero = 0;
   result = (*self->s_ops->so_recva)(self,iov,pbufsize,NULL,&zero,mode,packet_mode);
  } else if (self->s_ops->so_recvfrom) {
   struct sockaddr_storage dst;
   socklen_t length = self->s_peeraddr_len;
recv_again:
   result = (*self->s_ops->so_recvfrom)(self,iov,pbufsize,
                                       (struct sockaddr *)&dst,&length,
                                        mode,packet_mode);
   /* Check if the packet was received from the ~connected~ target. */
   if (length != self->s_peeraddr_len ||
       memcmp(&dst,&self->s_peeraddr,length) != 0)
       goto recv_again;
  } else if (self->s_ops->so_recvafrom) {
   struct sockaddr_storage dst;
   socklen_t length = self->s_peeraddr_len;
   size_t zero;
recva_again:
   zero = 0;
   result = (*self->s_ops->so_recvafrom)(self,iov,pbufsize,NULL,&zero,
                                        (struct sockaddr *)&dst,&length,
                                         mode,packet_mode);
   /* Check if the packet was received from the ~connected~ target. */
   if (length != self->s_peeraddr_len ||
       memcmp(&dst,&self->s_peeraddr,length) != 0)
       goto recva_again;
  } else {
   error_throw(E_NOT_IMPLEMENTED);
  }
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
 return result;
}


/* Same as `so_recv', but used for connection-less socket.
 * This is the socket operator hook for the `recvfrom(2)' system call.
 * @param: paddrbuflen: On entry, this pointer can be dereference to
 *                      determine the size of the `addrbuf' buffer.
 *                      On success, this pointer should be updated
 *                      to contain the _required_ size for the
 *                     `addrbuf' buffer. */
PUBLIC bool KCALL
socket_recvfrom(struct socket *__restrict self_,
                USER CHECKED struct iovec const *iov_,
                size_t *__restrict pbufsize_,
                USER CHECKED struct sockaddr *addrbuf_,
                socklen_t *__restrict paddrlen_,
                iomode_t mode_, packet_iomode_t packet_mode_) {
 struct socket *EXCEPT_VAR self = self_;
 USER CHECKED struct iovec const *EXCEPT_VAR iov = iov_;
 size_t *EXCEPT_VAR pbufsize = pbufsize_;
 USER CHECKED struct sockaddr *EXCEPT_VAR addrbuf = addrbuf_;
 socklen_t *EXCEPT_VAR paddrlen = paddrlen_;
 iomode_t EXCEPT_VAR mode = mode_;
 packet_iomode_t EXCEPT_VAR packet_mode = packet_mode_;
 bool COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->s_lock,mode);
 TRY {
  if (self->s_state & SOCKET_STATE_FSHUTRD)
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (self->s_ops->so_recvfrom) {
   result = (*self->s_ops->so_recvfrom)(self,iov,pbufsize,
                                        addrbuf,paddrlen,
                                        mode,packet_mode);
  } else if (self->s_ops->so_recvafrom) {
   size_t zero = 0;
   result = (*self->s_ops->so_recvafrom)(self,iov,pbufsize,
                                         NULL,&zero,
                                         addrbuf,paddrlen,
                                         mode,packet_mode);
  } else if (self->s_ops->so_recv) {
   if (!(self->s_state & SOCKET_STATE_FCONNECTED))
       throw_net_error(ERROR_NET_NOT_CONNECTED);
   if (self->s_ops->so_getpeername) {
    /* Lookup the peer address. */
    *paddrlen = (*self->s_ops->so_getpeername)(self,
                                               addrbuf,
                                              *paddrlen,
                                               mode);
   } else {
    /* Copy the saved peer address. */
    memcpy(addrbuf,&self->s_peeraddr,
           MIN(self->s_peeraddr_len,*paddrlen));
    *paddrlen = self->s_peeraddr_len;
   }
   /* Actually receive data. */
   result = (*self->s_ops->so_recv)(self,iov,pbufsize,mode,packet_mode);
  } else if (self->s_ops->so_recva) {
   size_t zero = 0;
   if (!(self->s_state & SOCKET_STATE_FCONNECTED))
       throw_net_error(ERROR_NET_NOT_CONNECTED);
   if (self->s_ops->so_getpeername) {
    /* Lookup the peer address. */
    *paddrlen = (*self->s_ops->so_getpeername)(self,
                                               addrbuf,
                                              *paddrlen,
                                               mode);
   } else {
    /* Copy the saved peer address. */
    memcpy(addrbuf,&self->s_peeraddr,
           MIN(self->s_peeraddr_len,*paddrlen));
    *paddrlen = self->s_peeraddr_len;
   }
   /* Actually receive data. */
   result = (*self->s_ops->so_recva)(self,iov,pbufsize,NULL,&zero,mode,packet_mode);
  } else {
   error_throw(E_NOT_IMPLEMENTED);
  }
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
 return result;
}


/* Construct, enqueue and send a new packet.
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
PUBLIC size_t KCALL
socket_send(struct socket *__restrict self_,
            USER CHECKED struct iovec const *iov_, size_t num_bytes_,
            iomode_t mode_, packet_iomode_t packet_mode_) {
 struct socket *EXCEPT_VAR self = self_;
 USER CHECKED struct iovec const *EXCEPT_VAR iov = iov_;
 size_t EXCEPT_VAR num_bytes = num_bytes_;
 iomode_t EXCEPT_VAR mode = mode_;
 packet_iomode_t EXCEPT_VAR packet_mode = packet_mode_;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->s_lock,mode);
 TRY {
  if (self->s_state & SOCKET_STATE_FSHUTWR)
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (!(self->s_state & SOCKET_STATE_FCONNECTED))
      throw_net_error(ERROR_NET_NOT_CONNECTED);
  if (self->s_ops->so_send) {
   result = (*self->s_ops->so_send)(self,iov,num_bytes,mode,packet_mode);
  } else if (self->s_ops->so_senda) {
   result = (*self->s_ops->so_senda)(self,iov,num_bytes,NULL,0,mode,packet_mode);
  } else if (self->s_ops->so_sendto) {
   /* Send data to the ~connected~ peer. */
   result = (*self->s_ops->so_sendto)(self,iov,num_bytes,
                                     (struct sockaddr *)&self->s_peeraddr,
                                      self->s_peeraddr_len,
                                      mode,packet_mode);
  } else if (self->s_ops->so_sendato) {
   /* Send data to the ~connected~ peer. */
   result = (*self->s_ops->so_sendato)(self,iov,num_bytes,NULL,0,
                                      (struct sockaddr *)&self->s_peeraddr,
                                       self->s_peeraddr_len,
                                       mode,packet_mode);
  } else {
   error_throw(E_NOT_IMPLEMENTED);
  }
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
 return result;
}


/* Same as `so_send', but used for connection-less socket.
 * This is the socket operator hook for the `sendto(2)' system call. */
PUBLIC size_t KCALL
socket_sendto(struct socket *__restrict self_,
              USER CHECKED struct iovec const *iov_, size_t num_bytes_,
              USER CHECKED struct sockaddr const *addrbuf_,
              socklen_t addrlen_, iomode_t mode_,
              packet_iomode_t packet_mode_) {
 struct socket *EXCEPT_VAR self = self_;
 USER CHECKED struct iovec const *EXCEPT_VAR iov = iov_;
 size_t EXCEPT_VAR num_bytes = num_bytes_;
 iomode_t EXCEPT_VAR mode = mode_;
 packet_iomode_t EXCEPT_VAR packet_mode = packet_mode_;
 USER CHECKED struct sockaddr const *EXCEPT_VAR addrbuf = addrbuf_;
 socklen_t EXCEPT_VAR addrlen = addrlen_;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->s_lock,mode);
 TRY {
  if (self->s_state & SOCKET_STATE_FSHUTWR)
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (self->s_ops->so_sendto) {
   result = (*self->s_ops->so_sendto)(self,iov,num_bytes,addrbuf,addrlen,mode,packet_mode);
  } else if (self->s_ops->so_sendato) {
   result = (*self->s_ops->so_sendato)(self,iov,num_bytes,NULL,0,addrbuf,addrlen,mode,packet_mode);
  } else if (self->s_ops->so_send || self->s_ops->so_senda) {
   if (!(self->s_state & SOCKET_STATE_FCONNECTED)) {
    rwlock_writef(&self->s_lock,mode);
    TRY {
     if unlikely(!self->s_ops->so_connect) {
      /* Without a connect callback, we can arbitrarily re-connect during each send. */
      if unlikely(addrlen > sizeof(struct sockaddr_storage))
         error_throw(E_INVALID_ARGUMENT);
      memcpy(&self->s_sockaddr,addrbuf,addrlen);
      self->s_sockaddr_len = addrlen;
      /* An empty address length is unsed to indicate a disconnect() */
      if unlikely(!addrlen)
         throw_net_error(ERROR_NET_NOT_CONNECTED);
     } else {
      (*self->s_ops->so_connect)(self,addrbuf,addrlen,mode);
     }
     self->s_state |= SOCKET_STATE_FCONNECTED;
    } FINALLY {
     rwlock_endwrite(&self->s_lock);
    }
   } else if (!self->s_ops->so_connect) {
    /* Already connected. - Check if it's connected to the same address. */
    if (self->s_peeraddr_len != addrlen ||
        memcmp(addrbuf,&self->s_peeraddr,addrlen) != 0) {
     /* Reconnect a connection-less socket if it changed targets. */
     rwlock_writef(&self->s_lock,mode);
     TRY {
      /* Without a connect callback, we can arbitrarily re-connect during each send. */
      if unlikely(addrlen > sizeof(struct sockaddr_storage))
         error_throw(E_INVALID_ARGUMENT);
      memcpy(&self->s_sockaddr,addrbuf,addrlen);
      self->s_sockaddr_len = addrlen;
      /* An empty address length is unsed to indicate a disconnect() */
      if unlikely(!addrlen)
         throw_net_error(ERROR_NET_NOT_CONNECTED);
      self->s_state |= SOCKET_STATE_FCONNECTED;
     } FINALLY {
      rwlock_endwrite(&self->s_lock);
     }
    }
   } else if (self->s_ops->so_getpeername) {
    /* Already connected. - Check if it's connected to the same address. */
    struct sockaddr *EXCEPT_VAR temp_addrbuf;
    socklen_t temp_addrbuf_len = MIN(addrlen,sizeof(struct sockaddr_storage));
    socklen_t req_addrbuf_len;
    temp_addrbuf = (struct sockaddr *)malloca(temp_addrbuf_len);
    TRY {
     req_addrbuf_len = (*self->s_ops->so_getpeername)(self,temp_addrbuf,temp_addrbuf_len,mode);
     if (req_addrbuf_len != addrlen)
         throw_net_error(ERROR_NET_CANNOT_RECONNECT); /* Different targets */
     if unlikely(req_addrbuf_len > temp_addrbuf_len) {
      struct sockaddr *EXCEPT_VAR dyn_buffer;
      COMPILER_WRITE_BARRIER();
      /* Need a larger socket address buffer. */
      dyn_buffer = (struct sockaddr *)kmalloc(req_addrbuf_len,GFP_SHARED);
      TRY {
       req_addrbuf_len = (*self->s_ops->so_getpeername)(self,temp_addrbuf,req_addrbuf_len,mode);
       if (req_addrbuf_len != addrlen ||
          (memcmp(temp_addrbuf,addrbuf,addrlen) != 0))
           throw_net_error(ERROR_NET_CANNOT_RECONNECT); /* Different targets */
      } FINALLY {
       kfree(dyn_buffer);
      }
     } else {
      if (memcmp(temp_addrbuf,addrbuf,addrlen) != 0)
          throw_net_error(ERROR_NET_CANNOT_RECONNECT); /* Different targets */
     }
    } FINALLY {
     freea(temp_addrbuf);
    }
   } else {
    /* Already connected. - Check if it's connected to the same address. */
    if (self->s_peeraddr_len != addrlen ||
        memcmp(addrbuf,&self->s_peeraddr,addrlen) != 0)
        throw_net_error(ERROR_NET_CANNOT_RECONNECT); /* Different targets */
   }
   /* The socket is now connected to the sendto() target. */
   if (self->s_ops->so_send) {
    result = (*self->s_ops->so_send)(self,iov,num_bytes,mode,packet_mode);
   } else {
    result = (*self->s_ops->so_senda)(self,iov,num_bytes,NULL,0,mode,packet_mode);
   }
  } else {
   error_throw(E_NOT_IMPLEMENTED);
  }
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
 return result;
}




/* Extended variants of `socket_recv' and `socket_recvfrom'
 * that include information about incoming ancillary data.
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
PUBLIC bool KCALL
socket_recva(struct socket *__restrict self_,
             USER CHECKED struct iovec const *iov_, size_t *__restrict pbufsize_,
             USER CHECKED struct cmsghdr *anc_, size_t *__restrict pancsize_,
             iomode_t mode_, packet_iomode_t packet_mode_) {
 struct socket *EXCEPT_VAR self = self_;
 USER CHECKED struct iovec const *EXCEPT_VAR iov = iov_;
 size_t *EXCEPT_VAR pbufsize = pbufsize_;
 USER CHECKED struct cmsghdr *EXCEPT_VAR anc = anc_;
 size_t *EXCEPT_VAR pancsize = pancsize_;
 iomode_t EXCEPT_VAR mode = mode_;
 packet_iomode_t EXCEPT_VAR packet_mode = packet_mode_;
 bool COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->s_lock,mode);
 TRY {
  if (self->s_state & SOCKET_STATE_FSHUTRD)
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (!(self->s_state & SOCKET_STATE_FCONNECTED))
      throw_net_error(ERROR_NET_NOT_CONNECTED);
  if (self->s_ops->so_recva) {
   result = (*self->s_ops->so_recva)(self,iov,pbufsize,anc,pancsize,mode,packet_mode);
  } else if (self->s_ops->so_recv) {
   result = (*self->s_ops->so_recv)(self,iov,pbufsize,mode,packet_mode);
  } else if (self->s_ops->so_recvafrom) {
   struct sockaddr_storage dst;
   socklen_t length = self->s_peeraddr_len;
   size_t orig_size = *pancsize;
recva_again:
   result = (*self->s_ops->so_recvafrom)(self,iov,pbufsize,anc,pancsize,
                                        (struct sockaddr *)&dst,&length,
                                         mode,packet_mode);
   /* Check if the packet was received from the ~connected~ target. */
   if (length != self->s_peeraddr_len ||
       memcmp(&dst,&self->s_peeraddr,length) != 0) {
    *pancsize = orig_size;
    goto recva_again;
   }
  } else if (self->s_ops->so_recvfrom) {
   struct sockaddr_storage dst;
   socklen_t length = self->s_peeraddr_len;
recv_again:
   result = (*self->s_ops->so_recvfrom)(self,iov,pbufsize,
                                       (struct sockaddr *)&dst,&length,
                                        mode,packet_mode);
   /* Check if the packet was received from the ~connected~ target. */
   if (length != self->s_peeraddr_len ||
       memcmp(&dst,&self->s_peeraddr,length) != 0)
       goto recv_again;
  } else {
   error_throw(E_NOT_IMPLEMENTED);
  }
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
 return result;
}
PUBLIC bool KCALL
socket_recvafrom(struct socket *__restrict self_,
                 USER CHECKED struct iovec const *iov_, size_t *__restrict pbufsize_,
                 USER CHECKED struct cmsghdr *anc_, size_t *__restrict pancsize_,
                 USER CHECKED struct sockaddr *addrbuf_, socklen_t *__restrict paddrlen_,
                 iomode_t mode_, packet_iomode_t packet_mode_) {
 struct socket *EXCEPT_VAR self = self_;
 USER CHECKED struct iovec const *EXCEPT_VAR iov = iov_;
 size_t *EXCEPT_VAR pbufsize = pbufsize_;
 USER CHECKED struct cmsghdr *EXCEPT_VAR anc = anc_;
 size_t *EXCEPT_VAR pancsize = pancsize_;
 USER CHECKED struct sockaddr *EXCEPT_VAR addrbuf = addrbuf_;
 socklen_t *EXCEPT_VAR paddrlen = paddrlen_;
 iomode_t EXCEPT_VAR mode = mode_;
 packet_iomode_t EXCEPT_VAR packet_mode = packet_mode_;
 bool COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->s_lock,mode);
 TRY {
  if (self->s_state & SOCKET_STATE_FSHUTRD)
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (self->s_ops->so_recvafrom) {
   result = (*self->s_ops->so_recvafrom)(self,iov,pbufsize,
                                         anc,pancsize,
                                         addrbuf,paddrlen,
                                         mode,packet_mode);
  } else if (self->s_ops->so_recvfrom) {
   result = (*self->s_ops->so_recvfrom)(self,iov,pbufsize,
                                        addrbuf,paddrlen,
                                        mode,packet_mode);
  } else if (self->s_ops->so_recva) {
   if (!(self->s_state & SOCKET_STATE_FCONNECTED))
       throw_net_error(ERROR_NET_NOT_CONNECTED);
   if (self->s_ops->so_getpeername) {
    /* Lookup the peer address. */
    *paddrlen = (*self->s_ops->so_getpeername)(self,
                                               addrbuf,
                                              *paddrlen,
                                               mode);
   } else {
    /* Copy the saved peer address. */
    memcpy(addrbuf,&self->s_peeraddr,
           MIN(self->s_peeraddr_len,*paddrlen));
    *paddrlen = self->s_peeraddr_len;
   }
   /* Actually receive data. */
   result = (*self->s_ops->so_recva)(self,iov,pbufsize,anc,pancsize,mode,packet_mode);
  } else if (self->s_ops->so_recv) {
   if (!(self->s_state & SOCKET_STATE_FCONNECTED))
       throw_net_error(ERROR_NET_NOT_CONNECTED);
   if (self->s_ops->so_getpeername) {
    /* Lookup the peer address. */
    *paddrlen = (*self->s_ops->so_getpeername)(self,
                                               addrbuf,
                                              *paddrlen,
                                               mode);
   } else {
    /* Copy the saved peer address. */
    memcpy(addrbuf,&self->s_peeraddr,
           MIN(self->s_peeraddr_len,*paddrlen));
    *paddrlen = self->s_peeraddr_len;
   }
   /* Actually receive data. */
   result = (*self->s_ops->so_recv)(self,iov,pbufsize,mode,packet_mode);
  } else {
   error_throw(E_NOT_IMPLEMENTED);
  }
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
 return result;
}


/* Extended variants of `socket_send' and `socket_sendto' that
 * include information about outgoing ancillary data.
 * @param: anc:     Ancillary data.
 * @param: ancsize: The size of the ancillary data buffer. */
PUBLIC size_t KCALL
socket_senda(struct socket *__restrict self_,
             USER CHECKED struct iovec const *iov_, size_t num_bytes_,
             USER CHECKED struct cmsghdr const *anc_, size_t ancsize_,
             iomode_t mode_, packet_iomode_t packet_mode_) {
 struct socket *EXCEPT_VAR self = self_;
 USER CHECKED struct iovec const *EXCEPT_VAR iov = iov_;
 size_t EXCEPT_VAR num_bytes = num_bytes_;
 USER CHECKED struct cmsghdr const *EXCEPT_VAR anc = anc_;
 size_t EXCEPT_VAR ancsize = ancsize_;
 iomode_t EXCEPT_VAR mode = mode_;
 packet_iomode_t EXCEPT_VAR packet_mode = packet_mode_;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->s_lock,mode);
 TRY {
  if (self->s_state & SOCKET_STATE_FSHUTWR)
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (!(self->s_state & SOCKET_STATE_FCONNECTED))
      throw_net_error(ERROR_NET_NOT_CONNECTED);
  if (self->s_ops->so_senda) {
   result = (*self->s_ops->so_senda)(self,iov,num_bytes,anc,ancsize,mode,packet_mode);
  } else if (self->s_ops->so_send) {
   result = (*self->s_ops->so_send)(self,iov,num_bytes,mode,packet_mode);
  } else if (self->s_ops->so_sendato) {
   /* Send data to the ~connected~ peer. */
   result = (*self->s_ops->so_sendato)(self,iov,num_bytes,NULL,0,
                                      (struct sockaddr *)&self->s_peeraddr,
                                       self->s_peeraddr_len,
                                       mode,packet_mode);
  } else if (self->s_ops->so_sendto) {
   /* Send data to the ~connected~ peer. */
   result = (*self->s_ops->so_sendto)(self,iov,num_bytes,
                                     (struct sockaddr *)&self->s_peeraddr,
                                      self->s_peeraddr_len,
                                      mode,packet_mode);
  } else {
   error_throw(E_NOT_IMPLEMENTED);
  }
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
 return result;
}
PUBLIC size_t KCALL
socket_sendato(struct socket *__restrict self_,
               USER CHECKED struct iovec const *iov_, size_t num_bytes_,
               USER CHECKED struct cmsghdr const *anc_, size_t ancsize_,
               USER CHECKED struct sockaddr const *addrbuf_,
               socklen_t addrlen_, iomode_t mode_,
               packet_iomode_t packet_mode_) {
 struct socket *EXCEPT_VAR self = self_;
 USER CHECKED struct iovec const *EXCEPT_VAR iov = iov_;
 size_t EXCEPT_VAR num_bytes = num_bytes_;
 USER CHECKED struct cmsghdr const *EXCEPT_VAR anc = anc_;
 size_t EXCEPT_VAR ancsize = ancsize_;
 iomode_t EXCEPT_VAR mode = mode_;
 packet_iomode_t EXCEPT_VAR packet_mode = packet_mode_;
 USER CHECKED struct sockaddr const *EXCEPT_VAR addrbuf = addrbuf_;
 socklen_t EXCEPT_VAR addrlen = addrlen_;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
again:
 rwlock_readf(&self->s_lock,mode);
 TRY {
  if (self->s_state & SOCKET_STATE_FSHUTWR)
      throw_net_error(ERROR_NET_SHUTDOWN);
  if (self->s_ops->so_sendato) {
   result = (*self->s_ops->so_sendato)(self,iov,num_bytes,anc,ancsize,addrbuf,addrlen,mode,packet_mode);
  } else if (self->s_ops->so_sendto) {
   result = (*self->s_ops->so_sendto)(self,iov,num_bytes,addrbuf,addrlen,mode,packet_mode);
  } else if (self->s_ops->so_send || self->s_ops->so_senda) {
   if (!(self->s_state & SOCKET_STATE_FCONNECTED)) {
    rwlock_writef(&self->s_lock,mode);
    TRY {
     if unlikely(!self->s_ops->so_connect) {
      /* Without a connect callback, we can arbitrarily re-connect during each send. */
      if unlikely(addrlen > sizeof(struct sockaddr_storage))
         error_throw(E_INVALID_ARGUMENT);
      memcpy(&self->s_sockaddr,addrbuf,addrlen);
      self->s_sockaddr_len = addrlen;
      /* An empty address length is unsed to indicate a disconnect() */
      if unlikely(!addrlen)
         throw_net_error(ERROR_NET_NOT_CONNECTED);
     } else {
      (*self->s_ops->so_connect)(self,addrbuf,addrlen,mode);
     }
     self->s_state |= SOCKET_STATE_FCONNECTED;
    } FINALLY {
     rwlock_endwrite(&self->s_lock);
    }
   } else if (!self->s_ops->so_connect) {
    /* Already connected. - Check if it's connected to the same address. */
    if (self->s_peeraddr_len != addrlen ||
        memcmp(addrbuf,&self->s_peeraddr,addrlen) != 0) {
     /* Reconnect a connection-less socket if it changed targets. */
     rwlock_writef(&self->s_lock,mode);
     TRY {
      /* Without a connect callback, we can arbitrarily re-connect during each send. */
      if unlikely(addrlen > sizeof(struct sockaddr_storage))
         error_throw(E_INVALID_ARGUMENT);
      memcpy(&self->s_sockaddr,addrbuf,addrlen);
      self->s_sockaddr_len = addrlen;
      /* An empty address length is unsed to indicate a disconnect() */
      if unlikely(!addrlen)
         throw_net_error(ERROR_NET_NOT_CONNECTED);
      self->s_state |= SOCKET_STATE_FCONNECTED;
     } FINALLY {
      rwlock_endwrite(&self->s_lock);
     }
    }
   } else if (self->s_ops->so_getpeername) {
    /* Already connected. - Check if it's connected to the same address. */
    struct sockaddr *EXCEPT_VAR temp_addrbuf;
    socklen_t temp_addrbuf_len = MIN(addrlen,sizeof(struct sockaddr_storage));
    socklen_t req_addrbuf_len;
    temp_addrbuf = (struct sockaddr *)malloca(temp_addrbuf_len);
    TRY {
     req_addrbuf_len = (*self->s_ops->so_getpeername)(self,temp_addrbuf,temp_addrbuf_len,mode);
     if (req_addrbuf_len != addrlen)
         throw_net_error(ERROR_NET_CANNOT_RECONNECT); /* Different targets */
     if unlikely(req_addrbuf_len > temp_addrbuf_len) {
      struct sockaddr *EXCEPT_VAR dyn_buffer;
      COMPILER_WRITE_BARRIER();
      /* Need a larger socket address buffer. */
      dyn_buffer = (struct sockaddr *)kmalloc(req_addrbuf_len,GFP_SHARED);
      TRY {
       req_addrbuf_len = (*self->s_ops->so_getpeername)(self,temp_addrbuf,req_addrbuf_len,mode);
       if (req_addrbuf_len != addrlen ||
          (memcmp(temp_addrbuf,addrbuf,addrlen) != 0))
           throw_net_error(ERROR_NET_CANNOT_RECONNECT); /* Different targets */
      } FINALLY {
       kfree(dyn_buffer);
      }
     } else {
      if (memcmp(temp_addrbuf,addrbuf,addrlen) != 0)
          throw_net_error(ERROR_NET_CANNOT_RECONNECT); /* Different targets */
     }
    } FINALLY {
     freea(temp_addrbuf);
    }
   } else {
    /* Already connected. - Check if it's connected to the same address. */
    if (self->s_peeraddr_len != addrlen ||
        memcmp(addrbuf,&self->s_peeraddr,addrlen) != 0)
        throw_net_error(ERROR_NET_CANNOT_RECONNECT); /* Different targets */
   }
   /* The socket is now connected to the sendto() target. */
   if (self->s_ops->so_senda) {
    result = (*self->s_ops->so_senda)(self,iov,num_bytes,anc,ancsize,mode,packet_mode);
   } else {
    result = (*self->s_ops->so_send)(self,iov,num_bytes,mode,packet_mode);
   }
  } else {
   error_throw(E_NOT_IMPLEMENTED);
  }
 } FINALLY {
  if (rwlock_endread(&self->s_lock))
      goto again;
 }
 return result;
}
















/* Socket handle bindings. */
INTERN size_t KCALL
handle_socket_read(struct socket *__restrict self,
                   USER CHECKED void *buf, size_t num_bytes,
                   iomode_t mode) {
 size_t result,required_size;
 struct iovec iov[1];
 iov[0].iov_base = buf;
 iov[0].iov_len  = num_bytes;
 assert((mode & ~IO_NOIOVCHECK) == (PACKET_IO_FRDNEVER) ||
        (mode & ~IO_NOIOVCHECK) == (PACKET_IO_FRDIFFIT|PACKET_IO_FRDTRUNC));
again:
 required_size = num_bytes;
 if (!socket_recv(self,iov,&required_size,mode,
                  PACKET_IO_FRDIFFIT|PACKET_IO_FRDTRUNC))
      result = 0; /* Emulate EOF */
 else {
  if (!required_size)
       goto again; /* Ignore empty packets. */
  result = num_bytes;
 }
 return result;
}
INTERN size_t KCALL
handle_socket_write(struct socket *__restrict self,
                    USER CHECKED void const *buf, size_t num_bytes,
                    iomode_t mode) {
 struct iovec iov[1];
 iov[0].iov_base = (byte_t *)buf;
 iov[0].iov_len  = num_bytes;
 return socket_send(self,iov,num_bytes,mode,PACKET_IO_FWRNORMAL);
}
DEFINE_INTERN_ALIAS(handle_socket_poll,socket_poll);



DECL_END

#endif /* !GUARD_KERNEL_SRC_NET_SOCKET_C */
