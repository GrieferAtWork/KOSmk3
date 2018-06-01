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
#ifndef _I386_KOS_BITS_SIGACTION_H
#define _I386_KOS_BITS_SIGACTION_H 1

#include <bits-generic/sigaction.h>

#ifdef __CRT_KOS
#include <kos/i386-kos/bits/compat.h>
#if defined(__EXPOSE_CPU_COMPAT) && defined(__CC__)
#include <bits/siginfo.h>
#include <bits/sigset.h>
#include <hybrid/string.h>

__DECL_BEGIN

struct ucontext_compat;
struct sigaction_compat {
    /* Signal handler. */
    union {
        __X86_PTRCC(__sighandler_t) sa_handler;
        __X86_PTRCC(void (__ATTR_CDECL *)(int __signo, siginfo_compat_t *__info, struct ucontext_compat *__ctx)) sa_sigaction;
    };
    __sigset_t sa_mask;
    int        sa_flags;
    __X86_PTRCC(void (__ATTR_CDECL *)(void)) sa_restorer;
};

__LOCAL void (__LIBCCALL sigaction_to_sigaction_compat)(struct sigaction_compat *__restrict __dst,
                                                        struct sigaction const *__restrict __src) {
 __dst->sa_restorer = (__X86_PTRCC(void (__ATTR_CDECL *)(void)))(__X86_INTPTRCC)(__UINTPTR_TYPE__)__src->sa_restorer;
 __dst->sa_handler  = (__X86_PTRCC(__sighandler_t))(__X86_INTPTRCC)(__UINTPTR_TYPE__)__src->sa_handler;
 __dst->sa_flags    = __src->sa_flags;
 __hybrid_memcpy(&__dst->sa_mask,&__src->sa_mask,sizeof(__sigset_t));
}
__LOCAL void (__LIBCCALL sigaction_compat_to_sigaction)(struct sigaction *__restrict __dst,
                                                        struct sigaction_compat const *__restrict __src) {
 __dst->sa_restorer = (void (__ATTR_CDECL *)(void))(__UINTPTR_TYPE__)__src->sa_restorer;
 __dst->sa_handler  = (__sighandler_t)(__UINTPTR_TYPE__)__src->sa_handler;
 __dst->sa_flags    = __src->sa_flags;
 __hybrid_memcpy(&__dst->sa_mask,&__src->sa_mask,sizeof(__sigset_t));
}


__DECL_END
#endif
#endif /* __CRT_KOS */

#endif /* !_I386_KOS_BITS_SIGACTION_H */
