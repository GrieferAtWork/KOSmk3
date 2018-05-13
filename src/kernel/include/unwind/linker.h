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
#include <hybrid/list/atree.h>
#include <kos/types.h>
#include <kos/context.h>
#include <stdbool.h>

DECL_BEGIN

struct exception_handler_info {
    CHECKED void         *ehi_entry;  /* Handler entry point (either `eh_entry' or `ed_handler'). */
    __UINTPTR_HALF_TYPE__ ehi_flag;   /* Handler flags (Set of `EXCEPTION_HANDLER_F*') */
    __UINTPTR_HALF_TYPE__ ehi_mask;   /* Handler mask */
    struct { /* Exception descriptor-specific information. */
        __UINT16_TYPE__   ed_type;    /* The type of descriptor (One of `EXCEPT_DESC_TYPE_*'). */
        __UINT16_TYPE__   ed_flags;   /* Descriptor flags (Set of `EXCEPT_DESC_F*'). */
        __UINT16_TYPE__   ed_safe;    /* The amount of bytes of stack-memory that should be
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
        __UINT16_TYPE__   ed_pad;     /* ... */
    }                     ehi_desc;

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


#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Utility functions for working with EXCEPT caches. */
struct except_cache;
struct module;
struct application;
struct except_handler;

/* Exception handler cache design:
 *
 *    EXC1          EXC2
 *     |             |
 *     AAAAAAAAAAAAAAAAA          ]\
 *    BB                          ] \ Various overlapping exception handlers found in
 *   CCCCC DD          I          ] / .except (including A, which is not expressible in C)
 * EEEEEEEEEE FF G HHHHHHHHHH     ]/
 * 
 *    EXH1          EXH2
 *     A           AAAA           ]\  The resulting (ordered and impossible to overlap)
 *     B           HHHH           ] \ exception handler cache entries, including all active
 *     C                          ] / handlers. The order here mirrors their order within
 *     E                          ]/  the .except section.
 *
 * In order to reduce cache impact, in the example above, A,B,C and E would be compared against
 * each other, and exception handlers obstructed by other handlers will not be included in the
 * cache entry:
 * Assuming that `A' is a `CATCH(E_SEGFAULT)', if one of B,C and E also is an E_SEGFAULT handler,
 * it will not be included in the cache entry EXH1. However the most likely case would be something
 * like `B' being an `EXCEPT(EXCEPT_EXECUTE_HANDLER)' guard, which will resulting in C and E being
 * removed from the EXH1 entry unconditionally.
 *  -> However this doesn't mean that a handler like E can never be called, because as you can see
 *     above, the EXH1 handler is only 1 character wide (representative of 1 byte), meaning that
 *     a later search for exception handler at `EXC1 + 1' will not make use of the cache entry
 *     previously created for `EXC1', but use its own sequence (which would then be A,C,E and have
 *     a width of 2 bytes).
 * If it wasn't clear already, the width of an exception handler cache entry is determined by
 * the width of the intersection of all exception handlers overlapping at the address of the
 * instruction for which exception handlers are queried, which is then truncated by the exclusion
 * of all exception handlers that are not overlapping at a specific address.
 *  -> An example of this can be seen by the fact that the width of EXH2 is truncated by
 *     the exception handler I, which isn't overlapping with EXH2's lookup address, but
 *     is overlapping with the guard ranges of A and H.
 * This truncation is required to ensure that a later lookup at I will not re-use EXH2's
 * exception handler cache entry, but create a new one which would then include I. */
struct except_info_cache {
    ATREE_NODE(struct except_info_cache,uintptr_t) ic_node;    /* [0..1][owned] Address tree node of this cache entry (using image-relative addresses). */
    size_t                                         ic_size;    /* [const] Heap-size of this FDE info cache data block. */
    size_t                                         ic_count;   /* [const] The number of handlers effective within this range.
                                                                * NOTE: This is allowed to be ZERO(0) if no handlers exist for the range. */
    struct exception_handler_info                  ic_info[1]; /* [const][ic_count] Exception handler info
                                                                *                  (ordered by priority from most significant to least).
                                                                * NOTE: Unlike normally, the `ehi_entry' field in
                                                                *       this structure behaves as an image-relative
                                                                *       offset, rather than an absolute address. */
};


/* Finalize the given EXCEPT cache. */
INTDEF ATTR_NOTHROW void KCALL except_cache_fini(struct except_cache *__restrict self);

/* Clear the EXCEPT cache (should be called during `kernel_cc_invoke()') */
INTDEF ATTR_NOTHROW void KCALL except_cache_clear(struct except_cache *__restrict self);

INTDEF ATTR_NOTHROW bool KCALL
except_cache_lookup(struct except_handler *__restrict iter,
                    struct except_handler *__restrict end,
                    uintptr_t rel_ip, u16 exception_code,
                    struct application *__restrict app,
                    struct exception_handler_info *__restrict result);

/* Find an EXCEPT entry belonging to the kernel core. */
INTDEF ATTR_NOTHROW bool KCALL
kernel_findexcept(uintptr_t ip, u16 exception_code,
                  struct exception_handler_info *__restrict result);

#endif

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_UNWIND_LINKER_H */
