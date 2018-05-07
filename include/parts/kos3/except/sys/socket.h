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
#ifndef _PARTS_KOS3_EXCEPT_SYS_SOCKET_H
#define _PARTS_KOS3_EXCEPT_SYS_SOCKET_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#ifndef _SYS_GENERIC_SOCKET_H
#include <sys/socket.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

__LIBC __PORT_KOSONLY __fd_t (__LIBCCALL Xsocket)(int __domain, int __type, int __protocol);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsocketpair)(int __domain, int __type, int __protocol, __fd_t __fds[2]);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xbind)(__fd_t __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xconnect)(__fd_t __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
__LIBC __PORT_KOSONLY socklen_t (__LIBCCALL Xxgetsockname)(__fd_t __fd, __SOCKADDR_ARG __addr, socklen_t __len);
__LIBC __PORT_KOSONLY socklen_t (__LIBCCALL Xxgetpeername)(__fd_t __fd, __SOCKADDR_ARG __addr, socklen_t __len);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xgetsockname)(__fd_t __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __len);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xgetpeername)(__fd_t __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __len);
__LIBC __PORT_KOSONLY __WUNUSED_SUGGESTED size_t (__LIBCCALL Xsend)(__fd_t __fd, void const *__buf, size_t __bufsize, int __flags);
__LIBC __PORT_KOSONLY __WUNUSED size_t (__LIBCCALL Xrecv)(__fd_t __fd, void *__buf, size_t __bufsize, int __flags);
__LIBC __PORT_KOSONLY __WUNUSED_SUGGESTED size_t (__LIBCCALL Xsendto)(__fd_t __fd, void const *__buf, size_t __bufsize, int __flags, __CONST_SOCKADDR_ARG __addr, socklen_t __addr_len);
__LIBC __PORT_KOSONLY __WUNUSED size_t (__LIBCCALL Xrecvfrom)(__fd_t __fd, void *__restrict __buf, size_t __bufsize, int __flags, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len);
__LIBC __PORT_KOSONLY __WUNUSED_SUGGESTED size_t (__LIBCCALL Xsendmsg)(__fd_t __fd, struct msghdr const *__message, int __flags);
__LIBC __PORT_KOSONLY __WUNUSED size_t (__LIBCCALL Xrecvmsg)(__fd_t __fd, struct msghdr *__message, int __flags);
__LIBC __PORT_KOSONLY socklen_t (__LIBCCALL Xxgetsockopt)(__fd_t __fd, int __level, int __optname, void *__restrict __optval, socklen_t __optlen);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xgetsockopt)(__fd_t __fd, int __level, int __optname, void *__restrict __optval, socklen_t *__restrict __optlen);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsetsockopt)(__fd_t __fd, int __level, int __optname, void const *__optval, socklen_t __optlen);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xlisten)(__fd_t __fd, int __max_backlog);
__LIBC __PORT_KOSONLY __fd_t (__LIBCCALL Xaccept)(__fd_t __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xshutdown)(__fd_t __fd, int __how);
#ifdef __USE_GNU
/* @param: FLAGS: Set of `SOCK_NONBLOCK|SOCK_CLOEXEC|SOCK_CLOFORK' */
__LIBC __PORT_KOSONLY __fd_t (__LIBCCALL Xaccept4)(__fd_t __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len, int __flags);
__LIBC __PORT_KOSONLY unsigned int (__LIBCCALL Xsendmmsg)(__fd_t __fd, struct mmsghdr *__vmessages, unsigned int __vlen, int __flags);
__REDIRECT_TM64(__LIBC,__PORT_KOSONLY __WUNUSED,unsigned int,__LIBCCALL,Xrecvmmsg,(__fd_t __fd, struct mmsghdr *__vmessages, unsigned int __vlen, int __flags, struct timespec *__tmo),(__fd,__vmessages,__vlen,__flags,__tmo))
#ifdef __USE_TIME64
__LIBC __PORT_KOSONLY __WUNUSED unsigned int (__LIBCCALL Xrecvmmsg64)(__fd_t __fd, struct mmsghdr *__vmessages, unsigned int __vlen, int __flags, struct __timespec64 *__tmo);
#endif /* __USE_TIME64 */
#endif /* __USE_GNU */
#ifdef __USE_XOPEN2K
__LIBC __PORT_KOSONLY __WUNUSED int (__LIBCCALL Xsockatmark)(__fd_t __fd);
#endif /* __USE_XOPEN2K */
#ifdef __USE_MISC
__LIBC __PORT_KOSONLY __WUNUSED int (__LIBCCALL Xisfdtype)(__fd_t __fd, int __fdtype);
#endif /* __USE_MISC */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SYS_SOCKET_H */
