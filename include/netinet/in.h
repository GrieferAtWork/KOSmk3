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
#ifndef _NETINET_IN_H
#define _NETINET_IN_H 1

/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <__stdinc.h>
#include <bits/types.h>
#include <features.h>
#include <stdint.h>
#include <sys/socket.h>
#include <bits/in.h>

__SYSDECL_BEGIN

/* Standard well-defined IP protocols. */
enum {
    IPPROTO_IP      = 0,   /* Dummy protocol for TCP. */
    IPPROTO_ICMP    = 1,   /* Internet Control Message Protocol. */
    IPPROTO_IGMP    = 2,   /* Internet Group Management Protocol. */
    IPPROTO_IPIP    = 4,   /* IPIP tunnels (older KA9Q tunnels use 94). */
    IPPROTO_TCP     = 6,   /* Transmission Control Protocol. */
    IPPROTO_EGP     = 8,   /* Exterior Gateway Protocol. */
    IPPROTO_PUP     = 12,  /* PUP protocol. */
    IPPROTO_UDP     = 17,  /* User Datagram Protocol. */
    IPPROTO_IDP     = 22,  /* XNS IDP protocol. */
    IPPROTO_TP      = 29,  /* SO Transport Protocol Class 4. */
    IPPROTO_DCCP    = 33,  /* Datagram Congestion Control Protocol. */
    IPPROTO_IPV6    = 41,  /* IPv6 header. */
    IPPROTO_RSVP    = 46,  /* Reservation Protocol. */
    IPPROTO_GRE     = 47,  /* General Routing Encapsulation. */
    IPPROTO_ESP     = 50,  /* encapsulating security payload. */
    IPPROTO_AH      = 51,  /* authentication header. */
    IPPROTO_MTP     = 92,  /* Multicast Transport Protocol. */
    IPPROTO_BEETPH  = 94,  /* IP option pseudo header for BEET. */
    IPPROTO_ENCAP   = 98,  /* Encapsulation Header. */
    IPPROTO_PIM     = 103, /* Protocol Independent Multicast. */
    IPPROTO_COMP    = 108, /* Compression Header Protocol. */
    IPPROTO_SCTP    = 132, /* Stream Control Transmission Protocol. */
    IPPROTO_UDPLITE = 136, /* UDP-Lite protocol. */
    IPPROTO_MPLS    = 137, /* MPLS in IP. */
    IPPROTO_RAW     = 255, /* Raw IP packets. */
    IPPROTO_MAX
};
#define IPPROTO_IP      IPPROTO_IP
#define IPPROTO_ICMP    IPPROTO_ICMP
#define IPPROTO_IGMP    IPPROTO_IGMP
#define IPPROTO_IPIP    IPPROTO_IPIP
#define IPPROTO_TCP     IPPROTO_TCP
#define IPPROTO_EGP     IPPROTO_EGP
#define IPPROTO_PUP     IPPROTO_PUP
#define IPPROTO_UDP     IPPROTO_UDP
#define IPPROTO_IDP     IPPROTO_IDP
#define IPPROTO_TP      IPPROTO_TP
#define IPPROTO_DCCP    IPPROTO_DCCP
#define IPPROTO_IPV6    IPPROTO_IPV6
#define IPPROTO_RSVP    IPPROTO_RSVP
#define IPPROTO_GRE     IPPROTO_GRE
#define IPPROTO_ESP     IPPROTO_ESP
#define IPPROTO_AH      IPPROTO_AH
#define IPPROTO_MTP     IPPROTO_MTP
#define IPPROTO_BEETPH  IPPROTO_BEETPH
#define IPPROTO_ENCAP   IPPROTO_ENCAP
#define IPPROTO_PIM     IPPROTO_PIM
#define IPPROTO_COMP    IPPROTO_COMP
#define IPPROTO_SCTP    IPPROTO_SCTP
#define IPPROTO_UDPLITE IPPROTO_UDPLITE
#define IPPROTO_MPLS    IPPROTO_MPLS
#define IPPROTO_RAW     IPPROTO_RAW

enum {
    IPPROTO_HOPOPTS  = 0,  /* IPv6 Hop-by-Hop options. */
    IPPROTO_ROUTING  = 43, /* IPv6 routing header. */
    IPPROTO_FRAGMENT = 44, /* IPv6 fragmentation header. */
    IPPROTO_ICMPV6   = 58, /* ICMPv6. */
    IPPROTO_NONE     = 59, /* IPv6 no next header. */
    IPPROTO_DSTOPTS  = 60, /* IPv6 destination options. */
    IPPROTO_MH       = 135 /* IPv6 mobility header. */
};
#define IPPROTO_HOPOPTS  IPPROTO_HOPOPTS
#define IPPROTO_ROUTING  IPPROTO_ROUTING
#define IPPROTO_FRAGMENT IPPROTO_FRAGMENT
#define IPPROTO_ICMPV6   IPPROTO_ICMPV6
#define IPPROTO_NONE     IPPROTO_NONE
#define IPPROTO_DSTOPTS  IPPROTO_DSTOPTS
#define IPPROTO_MH       IPPROTO_MH

typedef uint16_t in_port_t; /* Type to represent a port. */

/* Standard well-known ports. */
enum {
    IPPORT_ECHO         = 7,    /* Echo service. */
    IPPORT_DISCARD      = 9,    /* Discard transmissions service. */
    IPPORT_SYSTAT       = 11,   /* System status service. */
    IPPORT_DAYTIME      = 13,   /* Time of day service. */
    IPPORT_NETSTAT      = 15,   /* Network status service. */
    IPPORT_FTP          = 21,   /* File Transfer Protocol. */
    IPPORT_TELNET       = 23,   /* Telnet protocol. */
    IPPORT_SMTP         = 25,   /* Simple Mail Transfer Protocol. */
    IPPORT_TIMESERVER   = 37,   /* Timeserver service. */
    IPPORT_NAMESERVER   = 42,   /* Domain Name Service. */
    IPPORT_WHOIS        = 43,   /* Internet Whois service. */
    IPPORT_MTP          = 57,
    IPPORT_TFTP         = 69,   /* Trivial File Transfer Protocol. */
    IPPORT_RJE          = 77,
    IPPORT_FINGER       = 79,   /* Finger service. */
    IPPORT_TTYLINK      = 87,
    IPPORT_SUPDUP       = 95,   /* SUPDUP protocol. */
    IPPORT_EXECSERVER   = 512,  /* execd service. */
    IPPORT_LOGINSERVER  = 513,  /* rlogind service. */
    IPPORT_CMDSERVER    = 514,
    IPPORT_EFSSERVER    = 520,
    IPPORT_BIFFUDP      = 512,  /* UDP ports. */
    IPPORT_WHOSERVER    = 513,  /* ... */
    IPPORT_ROUTESERVER  = 520,  /* ... */
    IPPORT_RESERVED     = 1024, /* Ports less than this value are reserved for privileged processes. */
    IPPORT_USERRESERVED = 5000  /* Ports greater this value are reserved for (non-privileged) servers. */
};

/* Definitions of the bits in an Internet address integer.
 * On subnets, host and network parts are found according to
 * the subnet mask, not these masks. */
#define IN_CLASSA(a)       ((((in_addr_t)(a)) & 0x80000000) == 0)
#define IN_CLASSA_NET          0xff000000
#define IN_CLASSA_NSHIFT       24
#define IN_CLASSA_HOST        (0xffffffff & ~IN_CLASSA_NET)
#define IN_CLASSA_MAX          128
#define IN_CLASSB(a)       ((((in_addr_t)(a)) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET          0xffff0000
#define IN_CLASSB_NSHIFT       16
#define IN_CLASSB_HOST        (0xffffffff & ~IN_CLASSB_NET)
#define IN_CLASSB_MAX          65536
#define IN_CLASSC(a)       ((((in_addr_t)(a)) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET          0xffffff00
#define IN_CLASSC_NSHIFT       8
#define IN_CLASSC_HOST        (0xffffffff & ~IN_CLASSC_NET)
#define IN_CLASSD(a)       ((((in_addr_t)(a)) & 0xf0000000) == 0xe0000000)
#define IN_MULTICAST(a)        IN_CLASSD(a)
#define IN_EXPERIMENTAL(a) ((((in_addr_t)(a)) & 0xe0000000) == 0xe0000000)
#define IN_BADCLASS(a)     ((((in_addr_t)(a)) & 0xf0000000) == 0xf0000000)

#define INADDR_ANY           ((in_addr_t)0x00000000) /* Address to accept any incoming messages. */
#define INADDR_BROADCAST     ((in_addr_t)0xffffffff) /* Address to send to all hosts. */
#define INADDR_NONE          ((in_addr_t)0xffffffff) /* Address indicating an error return. */

#define IN_LOOPBACKNET         127 /* Network number for local host loopback. */
#ifndef INADDR_LOOPBACK            /* Address to loopback in software to local host. */
#define INADDR_LOOPBACK      ((in_addr_t)0x7f000001) /* Inet 127.0.0.1. */
#endif

/* Defines for Multicast INADDR. */
#define INADDR_UNSPEC_GROUP    ((in_addr_t)0xe0000000) /* 224.0.0.0 */
#define INADDR_ALLHOSTS_GROUP  ((in_addr_t)0xe0000001) /* 224.0.0.1 */
#define INADDR_ALLRTRS_GROUP   ((in_addr_t)0xe0000002) /* 224.0.0.2 */
#define INADDR_MAX_LOCAL_GROUP ((in_addr_t)0xe00000ff) /* 224.0.0.255 */

/* IPv6 address */
struct in6_addr {
#ifdef __COMPILER_HAVE_TRANSPARENT_UNION
    union {
#undef s6_addr
        uint8_t  s6_addr[16];
#ifdef __USE_MISC
#undef s6_addr16
#undef s6_addr32
        uint16_t s6_addr16[8];
        uint32_t s6_addr32[4];
#endif /* __USE_MISC */
    };
#else /* __COMPILER_HAVE_TRANSPARENT_UNION */
    union {
        uint8_t  __u6_addr8[16];
#undef s6_addr
#define s6_addr      __in6_u.__u6_addr8
#ifdef __USE_MISC
        uint16_t __u6_addr16[8];
        uint32_t __u6_addr32[4];
#undef s6_addr16
#undef s6_addr32
#define s6_addr16 __in6_u.__u6_addr16
#define s6_addr32 __in6_u.__u6_addr32
#endif /* __USE_MISC */
    } __in6_u;
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
};


#ifndef __KERNEL__
#undef in6addr_any
#undef in6addr_loopback
__LIBC struct in6_addr const (in6addr_any);        /* :: */
__LIBC struct in6_addr const (in6addr_loopback);   /* ::1 */
#endif /* !__KERNEL__ */
#define IN6ADDR_ANY_INIT      {{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}}
#define IN6ADDR_LOOPBACK_INIT {{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}}}
#define INET_ADDRSTRLEN  16
#define INET6_ADDRSTRLEN 46

/* Structure describing an Internet socket address. */
struct sockaddr_in {
 __SOCKADDR_COMMON(sin_);
 in_port_t      sin_port; /* Port number. */
 struct in_addr sin_addr; /* Internet address. */
 /* Pad to size of `struct sockaddr'. */
 unsigned char  sin_zero[sizeof(struct sockaddr)-__SOCKADDR_COMMON_SIZE-
                         sizeof(in_port_t)-sizeof(struct in_addr)];
};

struct sockaddr_in6 {
    __SOCKADDR_COMMON(sin6_);
    in_port_t       sin6_port;     /* Transport layer port # */
    uint32_t        sin6_flowinfo; /* IPv6 flow information */
    struct in6_addr sin6_addr;     /* IPv6 address */
    uint32_t        sin6_scope_id; /* IPv6 scope-id */
};

#ifdef __USE_MISC
/* IPv4 multicast request. */
struct ip_mreq {
    struct in_addr imr_multiaddr; /* IP multicast address of group. */
    struct in_addr imr_interface; /* Local IP address of interface. */
};
struct ip_mreq_source {
    struct in_addr imr_multiaddr;  /* IP multicast address of group. */
    struct in_addr imr_interface;  /* IP address of source. */
    struct in_addr imr_sourceaddr; /* IP address of interface. */
};
#endif /* __USE_MISC */

/* IPv6 multicast request. */
struct ipv6_mreq {
    struct in6_addr ipv6mr_multiaddr; /* IPv6 multicast address of group */
    unsigned int    ipv6mr_interface; /* local interface */
};

#ifdef __USE_MISC
/* Multicast group request. */
struct group_req {
    uint32_t                gr_interface; /* Interface index. */
    struct sockaddr_storage gr_group;     /* Group address. */
};

struct group_source_req {
    uint32_t                gsr_interface; /* Interface index. */
    struct sockaddr_storage gsr_group;     /* Group address. */
    struct sockaddr_storage gsr_source;    /* Source address. */
};

/* Full-state filter operations. */
struct ip_msfilter {
    struct in_addr imsf_multiaddr; /* IP multicast address of group. */
    struct in_addr imsf_interface; /* Local IP address of interface. */
    uint32_t       imsf_fmode;     /* Filter mode. */
    uint32_t       imsf_numsrc;    /* Number of source addresses. */
    struct in_addr imsf_slist[1];  /* Source addresses. */
};
#define IP_MSFILTER_SIZE(numsrc) (sizeof(struct ip_msfilter) \
                                 -sizeof(struct in_addr) \
                                 +(numsrc)*sizeof(struct in_addr))

struct group_filter {
    uint32_t                gf_interface; /* Interface index. */
    struct sockaddr_storage gf_group;     /* Group address. */
    uint32_t                gf_fmode;     /* Filter mode. */
    uint32_t                gf_numsrc;    /* Number of source addresses. */
    struct sockaddr_storage gf_slist[1];  /* Source addresses. */
};
#define GROUP_FILTER_SIZE(numsrc) (sizeof(struct group_filter) \
                                  -sizeof(struct sockaddr_storage) \
                                  +((numsrc)*sizeof(struct sockaddr_storage)))
#endif

#ifndef __KERNEL__
__LIBC __ATTR_CONST uint32_t (__LIBCCALL ntohl)(uint32_t __netlong);
__LIBC __ATTR_CONST uint16_t (__LIBCCALL ntohs)(uint16_t __netshort);
__LIBC __ATTR_CONST uint32_t (__LIBCCALL htonl)(uint32_t __hostlong);
__LIBC __ATTR_CONST uint16_t (__LIBCCALL htons)(uint16_t __hostshort);
#endif /* !__KERNEL__ */

#include <hybrid/byteorder.h>
#include <bits/byteswap.h>

#if defined(__OPTIMIZE__) || defined(__KERNEL__)
#if __BYTE_ORDER == __BIG_ENDIAN
#   define ntohl(x)   (x)
#   define ntohs(x)   (x)
#   define htonl(x)   (x)
#   define htons(x)   (x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#   define ntohl(x)    __bswap_32(x)
#   define ntohs(x)    __bswap_16(x)
#   define htonl(x)    __bswap_32(x)
#   define htons(x)    __bswap_16(x)
#endif
#endif

#ifndef __NO_XBLOCK
#define IN6_IS_ADDR_UNSPECIFIED(a) __XBLOCK({ struct in6_addr const *__a = (struct in6_addr const *)(a); __XRETURN __a->s6_addr32[0] == 0 && __a->s6_addr32[1] == 0 && __a->s6_addr32[2] == 0 && __a->s6_addr32[3] == 0; })
#define IN6_IS_ADDR_LOOPBACK(a)    __XBLOCK({ struct in6_addr const *__a = (struct in6_addr const *)(a); __XRETURN __a->s6_addr32[0] == 0 && __a->s6_addr32[1] == 0 && __a->s6_addr32[2] == 0 && __a->s6_addr32[3] == htonl(1); }))
#define IN6_IS_ADDR_LINKLOCAL(a)   __XBLOCK({ struct in6_addr const *__a = (struct in6_addr const *)(a); __XRETURN (__a->s6_addr32[0] & htonl(0xffc00000)) == htonl(0xfe800000); })
#define IN6_IS_ADDR_SITELOCAL(a)   __XBLOCK({ struct in6_addr const *__a = (struct in6_addr const *)(a); __XRETURN (__a->s6_addr32[0] & htonl(0xffc00000)) == htonl(0xfec00000); })
#define IN6_IS_ADDR_V4MAPPED(a)    __XBLOCK({ struct in6_addr const *__a = (struct in6_addr const *)(a); __XRETURN __a->s6_addr32[0] == 0 && __a->s6_addr32[1] == 0 && __a->s6_addr32[2] == htonl(0xffff); })
#define IN6_IS_ADDR_V4COMPAT(a)    __XBLOCK({ struct in6_addr const *__a = (struct in6_addr const *)(a); __XRETURN __a->s6_addr32[0] == 0 && __a->s6_addr32[1] == 0 && __a->s6_addr32[2] == 0 && ntohl (__a->s6_addr32[3]) > 1; })
#define IN6_ARE_ADDR_EQUAL(a,b)    __XBLOCK({ struct in6_addr const *__a = (struct in6_addr const *)(a); \
                                              struct in6_addr const *__b = (struct in6_addr const *)(b); \
                                              __XRETURN __a->s6_addr32[0] == __b->s6_addr32[0] && \
                                                        __a->s6_addr32[1] == __b->s6_addr32[1] && \
                                                        __a->s6_addr32[2] == __b->s6_addr32[2] && \
                                                        __a->s6_addr32[3] == __b->s6_addr32[3]; }))
#else
#define IN6_IS_ADDR_UNSPECIFIED(a) (((const uint32_t *)(a))[0] == 0 && ((const uint32_t *)(a))[1] == 0 && ((const uint32_t *)(a))[2] == 0 && ((const uint32_t *)(a))[3] == 0)
#define IN6_IS_ADDR_LOOPBACK(a)    (((const uint32_t *)(a))[0] == 0 && ((const uint32_t *)(a))[1] == 0 && ((const uint32_t *)(a))[2] == 0 && ((const uint32_t *)(a))[3] == htonl(1))
#define IN6_IS_ADDR_LINKLOCAL(a)  ((((const uint32_t *)(a))[0] & htonl(0xffc00000)) == htonl(0xfe800000))
#define IN6_IS_ADDR_SITELOCAL(a)  ((((const uint32_t *)(a))[0] & htonl(0xffc00000)) == htonl(0xfec00000))
#define IN6_IS_ADDR_V4MAPPED(a)   ((((const uint32_t *)(a))[0] == 0) && (((const uint32_t *)(a))[1] == 0) && (((const uint32_t *)(a))[2] == htonl(0xffff)))
#define IN6_IS_ADDR_V4COMPAT(a)   ((((const uint32_t *)(a))[0] == 0) && (((const uint32_t *)(a))[1] == 0) && (((const uint32_t *)(a))[2] == 0) && (ntohl (((const uint32_t *)(a))[3]) > 1))
#define IN6_ARE_ADDR_EQUAL(a,b)   ((((const uint32_t *)(a))[0] == ((const uint32_t *)(b))[0]) && \
                                   (((const uint32_t *)(a))[1] == ((const uint32_t *)(b))[1]) && \
                                   (((const uint32_t *)(a))[2] == ((const uint32_t *)(b))[2]) && \
                                   (((const uint32_t *)(a))[3] == ((const uint32_t *)(b))[3]))
#endif
#define IN6_IS_ADDR_MULTICAST(a)    (((const uint8_t *)(a))[0] == 0xff)

#ifdef __USE_MISC
#ifndef __KERNEL__
__LIBC int (__LIBCCALL bindresvport)(int __sockfd, struct sockaddr_in *__sock_in);
__LIBC int (__LIBCCALL bindresvport6)(int __sockfd, struct sockaddr_in6 *__sock_in);
#endif /* !__KERNEL__ */
#endif
#define IN6_IS_ADDR_MC_NODELOCAL(a) (IN6_IS_ADDR_MULTICAST(a) && ((((const uint8_t *)(a))[1] & 0xf) == 0x1))
#define IN6_IS_ADDR_MC_LINKLOCAL(a) (IN6_IS_ADDR_MULTICAST(a) && ((((const uint8_t *)(a))[1] & 0xf) == 0x2))
#define IN6_IS_ADDR_MC_SITELOCAL(a) (IN6_IS_ADDR_MULTICAST(a) && ((((const uint8_t *)(a))[1] & 0xf) == 0x5))
#define IN6_IS_ADDR_MC_ORGLOCAL(a)  (IN6_IS_ADDR_MULTICAST(a) && ((((const uint8_t *)(a))[1] & 0xf) == 0x8))
#define IN6_IS_ADDR_MC_GLOBAL(a)    (IN6_IS_ADDR_MULTICAST(a) && ((((const uint8_t *)(a))[1] & 0xf) == 0xe))

#ifdef __USE_GNU
struct cmsghdr;

/* IPv6 packet information. */
struct in6_pktinfo {
 struct in6_addr ipi6_addr; /* src/dst IPv6 address */
 unsigned int ipi6_ifindex; /* send/recv interface index */
};

/* IPv6 MTU information. */
struct ip6_mtuinfo {
 struct sockaddr_in6 ip6m_addr; /* dst address including zone ID */
 uint32_t ip6m_mtu;             /* path MTU in host byte order */
};

#ifndef __KERNEL__
__LIBC __ATTR_DEPRECATED_ int (__LIBCCALL inet6_option_space)(int __nbytes);
__LIBC __ATTR_DEPRECATED_ int (__LIBCCALL inet6_option_init)(void *__bp, struct cmsghdr **__cmsgp, int __type);
__LIBC __ATTR_DEPRECATED_ int (__LIBCCALL inet6_option_append)(struct cmsghdr *__cmsg, const uint8_t *__typep, int __multx, int __plusy);
__LIBC __ATTR_DEPRECATED_ uint8_t *(__LIBCCALL inet6_option_alloc)(struct cmsghdr *__cmsg, int __datalen, int __multx, int __plusy);
__LIBC __ATTR_DEPRECATED_ int (__LIBCCALL inet6_option_next)(const struct cmsghdr *__cmsg, uint8_t **__tptrp);
__LIBC __ATTR_DEPRECATED_ int (__LIBCCALL inet6_option_find)(const struct cmsghdr *__cmsg, uint8_t **__tptrp, int __type);
__LIBC int (__LIBCCALL inet6_opt_init)(void *__extbuf, socklen_t __extlen);
__LIBC int (__LIBCCALL inet6_opt_append)(void *__extbuf, socklen_t __extlen, int __offset, uint8_t __type, socklen_t __len, uint8_t __align, void **__databufp);
__LIBC int (__LIBCCALL inet6_opt_finish)(void *__extbuf, socklen_t __extlen, int __offset);
__LIBC int (__LIBCCALL inet6_opt_set_val)(void *__databuf, int __offset, void *__val, socklen_t __vallen);
__LIBC int (__LIBCCALL inet6_opt_next)(void *__extbuf, socklen_t __extlen, int __offset, uint8_t *__typep, socklen_t *__lenp, void **__databufp);
__LIBC int (__LIBCCALL inet6_opt_find)(void *__extbuf, socklen_t __extlen, int __offset, uint8_t __type, socklen_t *__lenp, void **__databufp);
__LIBC int (__LIBCCALL inet6_opt_get_val)(void *__databuf, int __offset, void *__val, socklen_t __vallen);
__LIBC socklen_t (__LIBCCALL inet6_rth_space)(int __type, int __segments);
__LIBC void *(__LIBCCALL inet6_rth_init)(void *__bp, socklen_t __bp_len, int __type, int __segments);
__LIBC int (__LIBCCALL inet6_rth_add)(void *__bp, struct in6_addr const *__addr);
__LIBC int (__LIBCCALL inet6_rth_reverse)(void const *__in, void *__out);
__LIBC int (__LIBCCALL inet6_rth_segments)(void const *__bp);
__LIBC struct in6_addr *(__LIBCCALL inet6_rth_getaddr)(void const *__bp, int __index);
__LIBC int (__LIBCCALL getipv4sourcefilter)(int __s, struct in_addr __interface_addr, struct in_addr __group, uint32_t *__fmode, uint32_t *__numsrc, struct in_addr *__slist);
__LIBC int (__LIBCCALL setipv4sourcefilter)(int __s, struct in_addr __interface_addr, struct in_addr __group, uint32_t __fmode, uint32_t __numsrc, const struct in_addr *__slist);
__LIBC int (__LIBCCALL getsourcefilter)(int __s, uint32_t __interface_addr, struct sockaddr const *__group, socklen_t __grouplen, uint32_t *__fmode, uint32_t *__numsrc, struct sockaddr_storage *__slist);
__LIBC int (__LIBCCALL setsourcefilter)(int __s, uint32_t __interface_addr, struct sockaddr const *__group, socklen_t __grouplen, uint32_t __fmode, uint32_t __numsrc, const struct sockaddr_storage *__slist);
#endif /* !__KERNEL__ */
#endif /* __USE_GNU */

__SYSDECL_END

#endif    /* netinet/in.h */
