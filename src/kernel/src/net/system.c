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
#ifndef GUARD_KERNEL_SRC_NET_SYSTEM_C
#define GUARD_KERNEL_SRC_NET_SYSTEM_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <net/socket.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <kernel/debug.h>
#include <fs/handle.h>
#include <except.h>
#include <string.h>
#include <sys/socket.h>

DECL_BEGIN

#define throw_net_error(code) error_throwf(E_NET_ERROR,code)

DEFINE_SYSCALL3(socket,int,domain,int,type,int,protocol) {
 unsigned int COMPILER_IGNORE_UNINITIALIZED(result);
 struct handle EXCEPT_VAR hsocket;
 hsocket.h_mode = HANDLE_MODE(HANDLE_TYPE_FSOCKET,IO_RDWR);
 if (type & SOCK_NONBLOCK) hsocket.h_flag |= IO_NONBLOCK;
 if (type & SOCK_CLOEXEC)  hsocket.h_flag |= IO_HANDLE_FCLOEXEC;
 if (type & SOCK_CLOFORK)  hsocket.h_flag |= IO_HANDLE_FCLOFORK;
 type &= ~(SOCK_NONBLOCK|SOCK_CLOEXEC|SOCK_CLOFORK);
 if (protocol > PF_MAX)
     error_throw(E_INVALID_ARGUMENT);
 if (type > SOCK_DCCP && type != SOCK_PACKET)
     error_throw(E_INVALID_ARGUMENT);
 /* Construct the socket. */
 hsocket.h_object.o_socket = socket_create((u16)domain,
                                           (u16)type,
                                           (u16)protocol);
 TRY {
  result = handle_put(hsocket);
 } FINALLY {
  socket_decref(hsocket.h_object.o_socket);
 }
 return result;
}

#define CHECK_SOCKET_HANDLE(hsocket,sockfd) \
do{ \
  if ((hsocket).h_type != HANDLE_TYPE_FSOCKET) \
      error_throwf(E_INVALID_HANDLE,(sockfd),ERROR_INVALID_HANDLE_FWRONGTYPE, \
                  (hsocket).h_type,HANDLE_TYPE_FSOCKET); \
}__WHILE0

DEFINE_SYSCALL3(bind,fd_t,sockfd,struct sockaddr *,addr,socklen_t,len) {
 REF struct handle EXCEPT_VAR hsocket;
 validate_readable(addr,len);
 hsocket = handle_get(sockfd);
 TRY {
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  socket_bind(hsocket.h_object.o_socket,addr,len,hsocket.h_mode);
 } FINALLY {
  handle_decref(hsocket);
 }
 return 0;
}

DEFINE_SYSCALL2(listen,fd_t,sockfd,int,max_backlog) {
 REF struct handle EXCEPT_VAR hsocket;
 /* XXX: This `5' should be configurable via a /proc setting. */
 if unlikely(max_backlog <= 0 || max_backlog > 5)
    error_throw(E_INVALID_ARGUMENT);
 hsocket = handle_get(sockfd);
 TRY {
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  socket_listen(hsocket.h_object.o_socket,max_backlog,hsocket.h_mode);
 } FINALLY {
  handle_decref(hsocket);
 }
 return 0;
}

DEFINE_SYSCALL4(accept4,
                fd_t,sockfd,
                USER UNCHECKED struct sockaddr *,addr,
                USER UNCHECKED socklen_t *,len,int,flags) {
 unsigned int COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct handle EXCEPT_VAR hsocket;
 REF struct handle hresult;
 if (flags & ~(SOCK_NONBLOCK|SOCK_CLOEXEC|SOCK_CLOFORK))
     error_throw(E_INVALID_ARGUMENT);
 if (addr != NULL) {
  socklen_t addr_buflen;
  validate_writable(len,sizeof(socklen_t));
  addr_buflen = *len;
  COMPILER_READ_BARRIER();
  validate_writable(addr,addr_buflen);
  hresult.h_mode = HANDLE_MODE(HANDLE_TYPE_FSOCKET,IO_RDWR);
  if (flags & SOCK_NONBLOCK) hresult.h_flag |= IO_NONBLOCK;
  if (flags & SOCK_CLOEXEC)  hresult.h_flag |= IO_HANDLE_FCLOEXEC;
  if (flags & SOCK_CLOFORK)  hresult.h_flag |= IO_HANDLE_FCLOFORK;
  hsocket = handle_get(sockfd);
  TRY {
   CHECK_SOCKET_HANDLE(hsocket,sockfd);
   hresult.h_object.o_socket = socket_accept(hsocket.h_object.o_socket,hsocket.h_mode);
  } FINALLY {
   handle_decref(hsocket);
  }
  if (!hresult.h_object.o_socket)
       return -EWOULDBLOCK;
  TRY {
   /* Lookup the peer socket address of the new connection. */
   *len = socket_getpeername(hresult.h_object.o_socket,
                             addr,addr_buflen,
                             hresult.h_mode);
   /* Finally, insert the new connection into the handle manager. */
   result = handle_put(hresult);
  } FINALLY {
   socket_decref(hresult.h_object.o_socket);
  }
 } else {
  hresult.h_mode = HANDLE_MODE(HANDLE_TYPE_FSOCKET,IO_RDWR);
  if (flags & SOCK_NONBLOCK) hresult.h_flag |= IO_NONBLOCK;
  if (flags & SOCK_CLOEXEC)  hresult.h_flag |= IO_HANDLE_FCLOEXEC;
  if (flags & SOCK_CLOFORK)  hresult.h_flag |= IO_HANDLE_FCLOFORK;
  hsocket = handle_get(sockfd);
  TRY {
   CHECK_SOCKET_HANDLE(hsocket,sockfd);
   hresult.h_object.o_socket = socket_accept(hsocket.h_object.o_socket,
                                             hsocket.h_mode);
  } FINALLY {
   handle_decref(hsocket);
  }
  if (!hresult.h_object.o_socket)
       error_throw(E_WOULDBLOCK);
  TRY {
   /* Finally, insert the new connection into the handle manager. */
   result = handle_put(hresult);
  } FINALLY {
   socket_decref(hresult.h_object.o_socket);
  }
 }
 return result;
}

DEFINE_SYSCALL3(accept,
                fd_t,sockfd,
                USER UNCHECKED struct sockaddr *,addr,
                USER UNCHECKED socklen_t *,len) {
 return SYSC_accept4(sockfd,addr,len,0);
}


DEFINE_SYSCALL3(connect,
                fd_t,sockfd,
                USER UNCHECKED struct sockaddr *,addr,
                socklen_t,len) {
 REF struct handle EXCEPT_VAR hsocket;
 validate_readable(addr,len);
 hsocket = handle_get(sockfd);
 TRY {
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  socket_connect(hsocket.h_object.o_socket,addr,len,hsocket.h_mode);
 } FINALLY {
  handle_decref(hsocket);
 }
 return 0;
}

DEFINE_SYSCALL3(xgetsockname,
                fd_t,sockfd,
                USER UNCHECKED struct sockaddr *,addr,
                socklen_t,len) {
 socklen_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct handle EXCEPT_VAR hsocket;
 validate_writable(addr,len);
 hsocket = handle_get(sockfd);
 TRY {
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  result = socket_getsockname(hsocket.h_object.o_socket,addr,len,hsocket.h_mode);
 } FINALLY {
  handle_decref(hsocket);
 }
 return result;
}

DEFINE_SYSCALL3(xgetpeername,
                fd_t,sockfd,
                USER UNCHECKED struct sockaddr *,addr,
                socklen_t,len) {
 socklen_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct handle EXCEPT_VAR hsocket;
 validate_writable(addr,len);
 hsocket = handle_get(sockfd);
 TRY {
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  result = socket_getpeername(hsocket.h_object.o_socket,addr,len,hsocket.h_mode);
 } FINALLY {
  handle_decref(hsocket);
 }
 return result;
}

DEFINE_SYSCALL3(getsockname,
                fd_t,sockfd,
                USER UNCHECKED struct sockaddr *,addr,
                USER UNCHECKED socklen_t *,len) {
 validate_writable(len,sizeof(socklen_t));
 *len = SYSC_xgetsockname(sockfd,addr,*len);
 return 0;
}

DEFINE_SYSCALL3(getpeername,
                fd_t,sockfd,
                USER UNCHECKED struct sockaddr *,addr,
                USER UNCHECKED socklen_t *,len) {
 validate_writable(len,sizeof(socklen_t));
 *len = SYSC_xgetpeername(sockfd,addr,*len);
 return 0;
}

DEFINE_SYSCALL6(sendto,fd_t,sockfd,
                USER UNCHECKED void *,buf,size_t,buflen,int,flags,
                USER UNCHECKED struct sockaddr *,addr,socklen_t,addrlen) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct handle EXCEPT_VAR hsocket; struct iovec iov[1];
 if (flags & ~(MSG_CONFIRM|MSG_DONTROUTE|MSG_DONTWAIT|
               MSG_EOR|MSG_MORE|MSG_NOSIGNAL|MSG_OOB))
     error_throw(E_INVALID_ARGUMENT);
 validate_readable(buf,buflen);
 iov[0].iov_base = buf;
 iov[0].iov_len  = buflen;
 if (addrlen) validate_readable(addr,addrlen);
 hsocket = handle_get(sockfd);
 TRY {
  iomode_t iomode;
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  iomode = hsocket.h_mode;
  if (flags & MSG_DONTWAIT)
      flags &= ~MSG_DONTWAIT,
      iomode |= IO_NONBLOCK;
  if (addrlen) {
   result = socket_sendto(hsocket.h_object.o_socket,iov,
                          buflen,addr,addrlen,iomode,
                          flags);
  } else {
   result = socket_send(hsocket.h_object.o_socket,iov,
                        buflen,iomode,
                        flags);
  }
 } FINALLY {
  handle_decref(hsocket);
 }
 return result;
}

DEFINE_SYSCALL6(recvfrom,fd_t,sockfd,
                USER UNCHECKED void *,buf,size_t,buflen,int,flags,
                USER UNCHECKED struct sockaddr *,addr,socklen_t *,paddrlen) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct handle EXCEPT_VAR hsocket; struct iovec iov[1];
 if (flags & ~(MSG_DONTWAIT|MSG_ERRQUEUE|MSG_OOB|
               MSG_PEEK|MSG_TRUNC|MSG_WAITALL))
     error_throw(E_INVALID_ARGUMENT);
 validate_writable(buf,buflen);
 iov[0].iov_base = buf;
 iov[0].iov_len  = buflen;
 if (paddrlen) {
  socklen_t addrlen;
  validate_writable(paddrlen,sizeof(socklen_t));
  addrlen = *paddrlen;
  COMPILER_READ_BARRIER();
  validate_writable(addr,addrlen);
  hsocket = handle_get(sockfd);
  TRY {
   iomode_t iomode;
   CHECK_SOCKET_HANDLE(hsocket,sockfd);
   iomode = hsocket.h_mode | IO_NOIOVCHECK;
   if (flags & MSG_DONTWAIT)
       flags &= ~MSG_DONTWAIT,
       iomode |= IO_NONBLOCK;
again_from:
   result = buflen;
   if (!socket_recvfrom(hsocket.h_object.o_socket,iov,
                       &result,addr,&addrlen,iomode,flags))
        result = 0;
   else if (!result) goto again_from; /* Ignore zero-length packets here. */
   /* Write back the updated address length. */
   *paddrlen = addrlen;
  } FINALLY {
   handle_decref(hsocket);
  }
 } else {
  hsocket = handle_get(sockfd);
  TRY {
   iomode_t iomode;
   CHECK_SOCKET_HANDLE(hsocket,sockfd);
   iomode = hsocket.h_mode | IO_NOIOVCHECK;
   if (flags & MSG_DONTWAIT)
       flags &= ~MSG_DONTWAIT,
       iomode |= IO_NONBLOCK;
again:
   result = buflen;
   if (!socket_recv(hsocket.h_object.o_socket,iov,
                   &result,iomode,flags))
        result = 0;
   else if (!result) goto again; /* Ignore zero-length packets here. */
  } FINALLY {
   handle_decref(hsocket);
  }
 }
 if (!(flags & MSG_TRUNC) && result > buflen)
       result = buflen;
 return result;
}



DEFINE_SYSCALL4(send,fd_t,sockfd,
                USER UNCHECKED void *,buf,size_t,buflen,int,flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct handle EXCEPT_VAR hsocket; struct iovec iov[1];
 if (flags & ~(MSG_CONFIRM|MSG_DONTROUTE|MSG_DONTWAIT|
               MSG_EOR|MSG_MORE|MSG_NOSIGNAL|MSG_OOB))
     error_throw(E_INVALID_ARGUMENT);
 validate_readable(buf,buflen);
 iov[0].iov_base = buf;
 iov[0].iov_len  = buflen;
 hsocket = handle_get(sockfd);
 TRY {
  iomode_t iomode;
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  iomode = hsocket.h_mode | IO_NOIOVCHECK;
  if (flags & MSG_DONTWAIT)
      flags &= ~MSG_DONTWAIT,
      iomode |= IO_NONBLOCK;
  result = socket_send(hsocket.h_object.o_socket,iov,
                       buflen,iomode,flags);
 } FINALLY {
  handle_decref(hsocket);
 }
 return result;
}

DEFINE_SYSCALL4(recv,fd_t,sockfd,
                USER UNCHECKED void *,buf,size_t,buflen,int,flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct handle EXCEPT_VAR hsocket;
 struct iovec iov[1];
 if (flags & ~(MSG_DONTWAIT|MSG_ERRQUEUE|MSG_OOB|
               MSG_PEEK|MSG_TRUNC|MSG_WAITALL))
     error_throw(E_INVALID_ARGUMENT);
 validate_writable(buf,buflen);
 iov[0].iov_base = buf;
 iov[0].iov_len  = buflen;
 hsocket = handle_get(sockfd);
 TRY {
  iomode_t iomode;
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  iomode = hsocket.h_mode | IO_NOIOVCHECK;
  if (flags & MSG_DONTWAIT)
      flags &= ~MSG_DONTWAIT,
      iomode |= IO_NONBLOCK;
again:
  result = buflen;
  if (!socket_recv(hsocket.h_object.o_socket,iov,
                  &result,iomode,flags))
       result = 0;
  else if (!result) goto again; /* Ignore zero-length packets here. */
 } FINALLY {
  handle_decref(hsocket);
 }
 if (!(flags & MSG_TRUNC) && result > buflen)
       result = buflen;
 return result;
}



DEFINE_SYSCALL5(xgetsockopt,fd_t,sockfd,int,level,int,optname,
                USER UNCHECKED void *,optval,socklen_t,optlen) {
 socklen_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct handle EXCEPT_VAR hsocket;
 validate_readable(optval,optlen);
 hsocket = handle_get(sockfd);
 TRY {
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  result = socket_getsockopt(hsocket.h_object.o_socket,
                             level,optname,optval,optlen,
                             hsocket.h_mode);
 } FINALLY {
  handle_decref(hsocket);
 }
 return result;
}
DEFINE_SYSCALL5(getsockopt,fd_t,sockfd,int,level,int,optname,
                USER UNCHECKED void *,optval,
                USER UNCHECKED socklen_t *,poptlen) {
 validate_writable(poptlen,sizeof(socklen_t));
 *poptlen = (socklen_t)SYSC_xgetsockopt(sockfd,level,optname,optval,*poptlen);
 return 0;
}

DEFINE_SYSCALL5(setsockopt,fd_t,sockfd,int,level,int,optname,
                USER UNCHECKED void *,optval,socklen_t,optlen) {
 REF struct handle EXCEPT_VAR hsocket;
 validate_readable(optval,optlen);
 hsocket = handle_get(sockfd);
 TRY {
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  socket_setsockopt(hsocket.h_object.o_socket,
                    level,optname,optval,optlen,
                    hsocket.h_mode);
 } FINALLY {
  handle_decref(hsocket);
 }
 return 0;
}


PRIVATE u16 const shutdown_how_flags[] = {
    [SHUT_RD]   = SOCKET_STATE_FSHUTRD,
    [SHUT_WR]   = SOCKET_STATE_FSHUTWR,
    [SHUT_RDWR] = SOCKET_STATE_FSHUTRD|SOCKET_STATE_FSHUTWR,
};

DEFINE_SYSCALL2(shutdown,fd_t,sockfd,unsigned int,how) {
 REF struct handle EXCEPT_VAR hsocket;
 if (how >= COMPILER_LENOF(shutdown_how_flags))
     error_throw(E_INVALID_ARGUMENT);
 hsocket = handle_get(sockfd);
 TRY {
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  socket_shutdown(hsocket.h_object.o_socket,
                  shutdown_how_flags[how]);
 } FINALLY {
  handle_decref(hsocket);
 }
 return 0;
}


PRIVATE size_t KCALL
socket_recvmsg(struct socket *__restrict self,
               struct msghdr *__restrict msg,
               iomode_t mode, packet_iomode_t packet_mode) {
 bool recv_ok;
 size_t total_length = 0,i,result;
 struct iovec *vec = msg->msg_iov;
 validate_readable(vec,msg->msg_iovlen*sizeof(struct iovec));
 for (i = 0; i < msg->msg_iovlen; ++i)
      total_length += vec[i].iov_len;
again:
 result = total_length;
 if (msg->msg_controllen) {
  if (msg->msg_namelen) {
   recv_ok = socket_recvafrom(self,
                              msg->msg_iov,
                             &result,
                             (struct cmsghdr *)msg->msg_control,
                             &msg->msg_controllen,
                             (struct sockaddr *)msg->msg_name,
                             &msg->msg_namelen,
                              mode,
                              packet_mode);
  } else {
   recv_ok = socket_recva(self,
                          msg->msg_iov,
                         &result,
                         (struct cmsghdr *)msg->msg_control,
                         &msg->msg_controllen,
                          mode,
                          packet_mode);
  }
 } else {
  if (msg->msg_namelen) {
   recv_ok = socket_recvfrom(self,
                             msg->msg_iov,
                            &result,
                            (struct sockaddr *)msg->msg_name,
                            &msg->msg_namelen,
                             mode,
                             packet_mode);
  } else {
   recv_ok = socket_recv(self,
                         msg->msg_iov,
                        &result,
                         mode,
                         packet_mode);
  }
 }
 if (!recv_ok)
      return 0; /* EOF */
 if (!result)
      goto again; /* Ignore empty packets here. */
 return result;
}

PRIVATE size_t KCALL
socket_sendmsg(struct socket *__restrict self,
               struct msghdr const *__restrict msg,
               iomode_t mode, packet_iomode_t packet_mode) {
 size_t result;
 size_t total_length = 0,i;
 struct iovec *vec = msg->msg_iov;
 validate_readable(vec,msg->msg_iovlen*sizeof(struct iovec));
 for (i = 0; i < msg->msg_iovlen; ++i)
      total_length += vec[i].iov_len;
 if (msg->msg_controllen) {
  if (msg->msg_namelen) {
   result = socket_sendato(self,
                           msg->msg_iov,
                           total_length,
                          (struct cmsghdr *)msg->msg_control,
                           msg->msg_controllen,
                          (struct sockaddr *)msg->msg_name,
                           msg->msg_namelen,
                           mode,
                           packet_mode);
  } else {
   result = socket_senda(self,
                         msg->msg_iov,
                         total_length,
                        (struct cmsghdr *)msg->msg_control,
                         msg->msg_controllen,
                         mode,
                         packet_mode);
  }
 } else {
  if (msg->msg_namelen) {
   result = socket_sendto(self,
                          msg->msg_iov,
                          total_length,
                         (struct sockaddr *)msg->msg_name,
                          msg->msg_namelen,
                          mode,
                          packet_mode);
  } else {
   result = socket_send(self,
                        msg->msg_iov,
                        total_length,
                        mode,
                        packet_mode);
  }
 }
 return result;
}


DEFINE_SYSCALL3(recvmsg,fd_t,sockfd,
                USER UNCHECKED struct msghdr *,msg,int,flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct handle EXCEPT_VAR hsocket; struct msghdr kernel_msg;
 if (flags & ~(MSG_CONFIRM|MSG_DONTROUTE|MSG_DONTWAIT|
               MSG_EOR|MSG_MORE|MSG_NOSIGNAL|MSG_OOB|
               MSG_CMSG_CLOEXEC))
     error_throw(E_INVALID_ARGUMENT);
 validate_readable(msg,sizeof(*msg));
 memcpy(&kernel_msg,msg,sizeof(*msg));
 COMPILER_BARRIER();
 hsocket = handle_get(sockfd);
 TRY {
  iomode_t iomode;
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  iomode = hsocket.h_mode;
  if (flags & MSG_DONTWAIT)
      flags &= ~MSG_DONTWAIT,
      iomode |= IO_NONBLOCK;
  result = socket_recvmsg(hsocket.h_object.o_socket,
                         &kernel_msg,iomode,flags);
  memcpy(msg,&kernel_msg,sizeof(*msg));
 } FINALLY {
  handle_decref(hsocket);
 }
 return result;
}

DEFINE_SYSCALL3(sendmsg,fd_t,sockfd,
                USER UNCHECKED struct msghdr const *,msg,int,flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct handle EXCEPT_VAR hsocket; struct msghdr kernel_msg;
 if (flags & ~(MSG_CONFIRM|MSG_DONTROUTE|MSG_DONTWAIT|
               MSG_EOR|MSG_MORE|MSG_NOSIGNAL|MSG_OOB))
     error_throw(E_INVALID_ARGUMENT);
 validate_readable(msg,sizeof(*msg));
 memcpy(&kernel_msg,msg,sizeof(*msg));
 COMPILER_BARRIER();
 hsocket = handle_get(sockfd);
 TRY {
  iomode_t iomode;
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  iomode = hsocket.h_mode;
  if (flags & MSG_DONTWAIT)
      flags &= ~MSG_DONTWAIT,
      iomode |= IO_NONBLOCK;
  result = socket_sendmsg(hsocket.h_object.o_socket,
                         &kernel_msg,iomode,flags);
 } FINALLY {
  handle_decref(hsocket);
 }
 return result;
}

DEFINE_SYSCALL4(sendmmsg,fd_t,sockfd,
                USER UNCHECKED struct mmsghdr *,msg,
                unsigned int,vlen,int,flags) {
 unsigned int COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct handle EXCEPT_VAR hsocket;
 if (flags & ~(MSG_CONFIRM|MSG_DONTROUTE|MSG_DONTWAIT|
               MSG_EOR|MSG_MORE|MSG_NOSIGNAL|MSG_OOB))
     error_throw(E_INVALID_ARGUMENT);
 validate_readablem(msg,vlen,sizeof(*msg));
 COMPILER_BARRIER();
 hsocket = handle_get(sockfd);
 TRY {
  iomode_t iomode;
  struct mmsghdr kernel_msg;
  CHECK_SOCKET_HANDLE(hsocket,sockfd);
  iomode = hsocket.h_mode;
  if (flags & MSG_DONTWAIT)
      flags &= ~MSG_DONTWAIT,
      iomode |= IO_NONBLOCK;
  for (result = 0; result < vlen; ++result) {
   memcpy(&kernel_msg,&msg[result],sizeof(struct mmsghdr));
   kernel_msg.msg_len = (unsigned int)socket_sendmsg(hsocket.h_object.o_socket,
                                                    &kernel_msg.msg_hdr,
                                                     iomode,flags);
   if (!kernel_msg.msg_len) break;
   COMPILER_BARRIER();
   msg[result].msg_len = kernel_msg.msg_len;
   COMPILER_WRITE_BARRIER();
  }
 } FINALLY {
  handle_decref(hsocket);
 }
 return result;
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_NET_SYSTEM_C */
