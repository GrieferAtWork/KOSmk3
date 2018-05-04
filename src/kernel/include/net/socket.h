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
#include <kos/types.h>

DECL_BEGIN

struct socket;

struct socket_ops {
    void (KCALL *so_fini)(struct socket *__restrict self);
};

struct socket {
    ATOMIC_DATA ref_t  s_refcnt; /* Socket reference counter. */
    struct socket_ops *s_ops;    /* [1..1][const] Socket operators. */
};





DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_NET_SOCKET_H */
