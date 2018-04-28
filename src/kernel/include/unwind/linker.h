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
#ifndef GUARD_KERNEL_INCLUDE_UNWIND_LINKER_H
#define GUARD_KERNEL_INCLUDE_UNWIND_LINKER_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/context.h>
#include <stdbool.h>

DECL_BEGIN

struct exception_handler_info {
    CHECKED void        *ehi_entry;  /* Handler entry point (either `eh_entry' or `ed_handler'). */
#if __SIZEOF_POINTER__ == 8
    u32                  ehi_flag;   /* Handler flags (Set of `EXCEPTION_HANDLER_F*') */
    u32                  ehi_mask;   /* Handler mask */
#else
    u16                  ehi_flag;   /* Handler flags (Set of `EXCEPTION_HANDLER_F*') */
    u16                  ehi_mask;   /* Handler mask */
#endif
    struct { /* Exception descriptor-specific information. */
        __UINT16_TYPE__  ed_type;    /* The type of descriptor (One of `EXCEPT_DESC_TYPE_*'). */
        __UINT16_TYPE__  ed_flags;   /* Descriptor flags (Set of `EXCEPT_DESC_F*'). */
        __UINT16_TYPE__  ed_safe;    /* The amount of bytes of stack-memory that should be
                                      * reserved before the descriptor is invoked.
                                      * This usually equals the total size of arguments passed
                                      * to the function in which code is being protected, plus
                                      * an additional 4/8 bytes for the return address.
                                      * Unless the `EXCEPT_DESC_FDEALLOC_CONTINUE' flag is set,
                                      * this is the amount of stack-memory that is copied before
                                      * jumping to `ed_handler', and when `EXCEPT_DESC_FDEALLOC_CONTINUE'
                                      * isn't set, this is the offset subtracted from SP to re-reserve
                                      * this amount of memory and prevent it from being clobbered.
                                      * In either case, ESP/RSP will point to `CFA(FUNCTION_OF(:eh_begin)) - ed_safe'
                                      * before more additional information is optionally pushed onto the stack,
                                      * based on flags defined above (XXX: No such flags exist, yet).
                                      * This value should never be smaller than 4/8 because it must
                                      * include the exception handler return address. */
        __UINT16_TYPE__  ed_pad;     /* ... */
    }                    ehi_desc;

};

struct fde_info;

/* TODO: These functions must return a `struct handle' to whatever object
 *       must be kept alive in order to ensure that FDE information remains
 *       available (struct application / struct vm_region)
 */

/* Lookup the application at `ip' and load the FDE entry associated with `ip'.
 * NOTE: The caller is responsible to ensure, or deal with problems
 *       caused by the associated application suddenly being unmapped.
 * @return: true:  Successfully located the FDE entry.
 * @return: false: Could not find an FDE entry for `ip' */
FUNDEF bool KCALL linker_findfde(uintptr_t ip, struct fde_info *__restrict result);
FUNDEF bool KCALL linker_findfde_consafe(uintptr_t ip, struct fde_info *__restrict result);

/* Lookup the effective exception handler for the given `ip' and fill in `result'
 * NOTE: The caller is responsible to ensure, or deal with problems
 *       caused by the associated application suddenly being unmapped.
 * @return: true:  Successfully located an exception handler.
 * @return: false: Could not find an exception handler for `ip' */
FUNDEF bool KCALL linker_findexcept(uintptr_t ip, u16 exception_code,
                                    struct exception_handler_info *__restrict result);
FUNDEF bool KCALL linker_findexcept_consafe(uintptr_t ip, u16 exception_code,
                                            struct exception_handler_info *__restrict result);

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_UNWIND_LINKER_H */
