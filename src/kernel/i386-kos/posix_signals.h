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
#ifndef GUARD_KERNEL_I386_KOS_POSIX_SIGNALS_H
#define GUARD_KERNEL_I386_KOS_POSIX_SIGNALS_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <sys/ucontext.h>
#include <i386-kos/syscall.h>
#include <signal.h>

DECL_BEGIN

struct PACKED signal_frame {
    /* Regular signal frame (without `SA_SIGINFO') */
    void                          *sf_sigreturn; /* #PF-syscall address for `sys_sigreturn'. */
    intptr_t                       sf_signo;     /* Signal number argument. */
    void                          *sf_pad[2];    /* Padding... (for sf_infop and sf_contextp, and required for
                                                  * binary compatibility when unwinding `signal_frame_ex') */
    uintptr_t                      sf_mode;      /* The signal handler return mode (One of `TASK_USERCTX_F*',
                                                  * or one of `X86_SYSCALL_TYPE_F*'). */
    mcontext_t                     sf_return;    /* Return context. */
    sigset_t                       sf_sigmask;   /* Return signal mask. */
};
struct PACKED signal_frame_ex {
    /* Extended signal frame (with `SA_SIGINFO') */
    union PACKED {
        struct signal_frame        sf_frame;     /* Basic frame (has binary compatibility) */
        struct PACKED {
            void                  *sf_sigreturn; /* #PF-syscall address for `sys_sigreturn'. */
            intptr_t               sf_signo;     /* Signal number argument. */
            siginfo_t             *sf_infop;     /* [== &sf_info] Signal info argument. */
            ucontext_t            *sf_contextp;  /* [== &sf_return] Signal context argument. */
            uintptr_t              sf_mode;      /* The signal handler return mode (One of `TASK_USERCTX_F*',
                                                  * or one of `X86_SYSCALL_TYPE_F*'). */
            ucontext_t             sf_return;    /* Signal return context. */
            siginfo_t              sf_info;      /* Signal information. */
        };
    };
};

DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_POSIX_SIGNALS_H */
