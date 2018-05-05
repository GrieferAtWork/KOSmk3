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
#ifndef GUARD_LIBS_LIBC_INET_SOCKET_C
#define GUARD_LIBS_LIBC_INET_SOCKET_C 1
#define _GNU_SOURCE 1

#include "../libc.h"
#include "../system.h"
#include "../errno.h"
#include "socket.h"
#include <hybrid/compiler.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <hybrid/timespec.h>
#include <net/if.h>
#include <assert.h>

DECL_BEGIN


DEFINE_PUBLIC_ALIAS(ntohl,libc_ntohl);
DEFINE_PUBLIC_ALIAS(ntohs,libc_ntohs);
DEFINE_PUBLIC_ALIAS(htonl,libc_htonl);
DEFINE_PUBLIC_ALIAS(htons,libc_htons);

#if __BYTE_ORDER == __BIG_ENDIAN
CRT_NET uint32_t LIBCCALL libc_ntohl(uint32_t netlong) { return netlong; }
CRT_NET uint16_t LIBCCALL libc_ntohs(uint16_t netshort) { return netshort; }
DEFINE_INTERN_ALIAS(libc_htonl,libc_ntohl);
DEFINE_INTERN_ALIAS(libc_htons,libc_ntohs);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
CRT_NET uint32_t LIBCCALL libc_ntohl(uint32_t netlong) { return __bswap_32(netlong); }
CRT_NET uint16_t LIBCCALL libc_ntohs(uint16_t netshort) { return __bswap_16(netshort); }
DEFINE_INTERN_ALIAS(libc_htonl,libc_ntohl);
DEFINE_INTERN_ALIAS(libc_htons,libc_ntohs);
#else
#error FIXME
#endif

DEFINE_PUBLIC_ALIAS(socketpair,libc_socketpair);
CRT_NET int LIBCCALL
libc_socketpair(int domain, int type, int protocol, int fds[2]) {
 assertf(0,"TODO");
 return -1;
}

DEFINE_PUBLIC_ALIAS(sockatmark,libc_sockatmark);
CRT_NET int LIBCCALL libc_sockatmark(fd_t fd) {
 assertf(0,"TODO");
 return -1;
}

DEFINE_PUBLIC_ALIAS(isfdtype,libc_isfdtype);
CRT_NET int LIBCCALL
libc_isfdtype(fd_t fd, int fdtype) {
 assertf(0,"TODO");
 return -1;
}


DEFINE_PUBLIC_ALIAS(sendmsg,libc_sendmsg);
CRT_NET ssize_t LIBCCALL
libc_sendmsg(fd_t fd, struct msghdr const *message, int flags) {
 assertf(0,"TODO");
 return -1;
}

DEFINE_PUBLIC_ALIAS(recvmsg,libc_recvmsg);
CRT_NET ssize_t LIBCCALL
libc_recvmsg(fd_t fd, struct msghdr *message, int flags) {
 assertf(0,"TODO");
 return -1;
}

DEFINE_PUBLIC_ALIAS(sendmmsg,libc_sendmmsg);
CRT_NET int LIBCCALL
libc_sendmmsg(fd_t fd, struct mmsghdr *vmessages,
              unsigned int vlen, int flags) {
 assertf(0,"TODO");
 return -1;
}

DEFINE_PUBLIC_ALIAS(recvmmsg,libc_recvmmsg);
CRT_NET int LIBCCALL
libc_recvmmsg(fd_t fd, struct mmsghdr *vmessages,
              unsigned int vlen, int flags,
              struct timespec *tmo) {
 assertf(0,"TODO");
 return -1;
}


DEFINE_PUBLIC_ALIAS(Xsocketpair,libc_Xsocketpair);
CRT_NET void LIBCCALL
libc_Xsocketpair(int domain, int type, int protocol, int fds[2]) {
 assertf(0,"TODO");
}

DEFINE_PUBLIC_ALIAS(Xsockatmark,libc_Xsockatmark);
CRT_NET int LIBCCALL libc_Xsockatmark(fd_t fd) {
 assertf(0,"TODO");
 return 0;
}

DEFINE_PUBLIC_ALIAS(Xsendmsg,libc_Xsendmsg);
CRT_NET size_t LIBCCALL
libc_Xsendmsg(fd_t fd, struct msghdr const *message, int flags) {
 assertf(0,"TODO");
 return 0;
}

DEFINE_PUBLIC_ALIAS(Xrecvmsg,libc_Xrecvmsg);
CRT_NET ssize_t LIBCCALL
libc_Xrecvmsg(fd_t fd, struct msghdr *message, int flags) {
 assertf(0,"TODO");
 return 0;
}

DEFINE_PUBLIC_ALIAS(Xsendmmsg,libc_Xsendmmsg);
CRT_NET unsigned int LIBCCALL
libc_Xsendmmsg(fd_t fd, struct mmsghdr *vmessages,
               unsigned int vlen, int flags) {
 assertf(0,"TODO");
 return 0;
}

DEFINE_PUBLIC_ALIAS(Xrecvmmsg,libc_Xrecvmmsg);
CRT_NET unsigned int LIBCCALL
libc_Xrecvmmsg(fd_t fd, struct mmsghdr *vmessages,
               unsigned int vlen, int flags,
               struct timespec *tmo) {
 assertf(0,"TODO");
 return 0;
}

DECL_END

#endif /* !GUARD_LIBS_LIBC_INET_SOCKET_C */
