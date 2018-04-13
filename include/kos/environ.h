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
#ifndef _KOS_ENVIRON_H
#define _KOS_ENVIRON_H 1

#include <hybrid/compiler.h>
#include <asm/types.h>

DECL_BEGIN

#define __PROCESS_ENVIRON_OFFSETOF_SELF      0
#define __PROCESS_ENVIRON_OFFSETOF_SIZE      __SIZEOF_POINTER__
#define __PROCESS_ENVIRON_OFFSETOF_ARGC   (2*__SIZEOF_POINTER__)
#define __PROCESS_ENVIRON_OFFSETOF_ENVC   (3*__SIZEOF_POINTER__)
#define __PROCESS_ENVIRON_OFFSETOF_ARGV   (4*__SIZEOF_POINTER__)
#define __PROCESS_ENVIRON_OFFSETOF_ENVP   (5*__SIZEOF_POINTER__)
#if defined(__KERNEL__) || defined(__USE_KOS)
#define PROCESS_ENVIRON_OFFSETOF_SELF      __PROCESS_ENVIRON_OFFSETOF_SELF
#define PROCESS_ENVIRON_OFFSETOF_SIZE      __PROCESS_ENVIRON_OFFSETOF_SIZE
#define PROCESS_ENVIRON_OFFSETOF_ARGC      __PROCESS_ENVIRON_OFFSETOF_ARGC
#define PROCESS_ENVIRON_OFFSETOF_ENVC      __PROCESS_ENVIRON_OFFSETOF_ENVC
#define PROCESS_ENVIRON_OFFSETOF_ARGV      __PROCESS_ENVIRON_OFFSETOF_ARGV
#define PROCESS_ENVIRON_OFFSETOF_ENVP      __PROCESS_ENVIRON_OFFSETOF_ENVP
#endif
#ifdef __CC__
struct process_environ {
    struct process_environ *pe_self; /* Self-pointer. */
    __size_t                pe_size; /* Size of the environment region (in bytes) */
    __size_t                pe_argc; /* Amount of entries in the `pe_argv' vector below. */
    __size_t                pe_envc; /* Amount of entries in the `pe_envp' vector below. */
    char                  **pe_argv; /* Vector of application arguments. */
    char                  **pe_envp; /* Vector of environment strings. */
    /* ... Unspecified / pending data + string tables + string vectors.
     * NOTE: New fields must always be added here. */
};
#endif /* __CC__ */

DECL_END

#endif /* !_KOS_ENVIRON_H */
