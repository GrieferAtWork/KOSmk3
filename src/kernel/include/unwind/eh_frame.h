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
#ifndef GUARD_KERNEL_INCLUDE_UNWIND_EH_FRAME_H
#define GUARD_KERNEL_INCLUDE_UNWIND_EH_FRAME_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <stdbool.h>

DECL_BEGIN

typedef uintptr_t uleb128_t;
typedef intptr_t  sleb128_t;
typedef uintptr_t ehptr_t;
typedef u8        eh_instr_t; /* Exception handler instruction pointer (part of its miniature interpreter) */

/* Absolute pointer encoding formats */
#define DW_EH_PE_absptr  0x00
#define DW_EH_PE_uleb128 0x01
#define DW_EH_PE_udata2  0x02
#define DW_EH_PE_udata4  0x03
#define DW_EH_PE_udata8  0x04
#define DW_EH_PE_sleb128 0x09
#define DW_EH_PE_sdata2  0x0a
#define DW_EH_PE_sdata4  0x0b
#define DW_EH_PE_sdata8  0x0c
/* Relative pointer encoding formats */
#define DW_EH_PE_pcrel   0x10
#define DW_EH_PE_textrel 0x20
#define DW_EH_PE_datarel 0x30
#define DW_EH_PE_funcrel 0x40
#define DW_EH_PE_aligned 0x50
/* Special */
#define DW_EH_PE_omit    0xff /* Omit the pointer. */


/* EH_FRAME specifications taken from here:
 *    https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic.pdf
 * Note that some comments have been taken directly from this PDF.
 */


/* The `.eh_frame' section shall contain 1 or more Call Frame Information (CFI) records.
 * The number of records present shall be determined by size of the section as contained in
 * the section header. Each CFI record contains a Common Information Entry (CIE) record
 * followed by 1 or more Frame Description Entry (FDE) records. Both CIEs and FDEs
 * shall be aligned to an addressing unit sized boundary. */

struct CIE {
    /* CIE -- Common Information Entry */
    u32       c_length;        /* The length of the CIE structure, excluding this field.
                                * When ZERO(0), the CIE acts as a terminator.
                                * When (u32)-1, the actual length is stored in `c_length64',
                                * a field that doesn't exist under other circumstances. */
    u64       c_length64;      /* [exists_if(c_length == (u32)-1)] 64-bit length. */
    u32       c_cieid;         /* CIE Record ID. Must be ZERO(0) to indicate a CIE record. */
    u8        c_version;       /* CIE Version (Must always be ONE(1)) */
    char      c_augstr[1];     /* Inline, NUL-terminated augmentation string (case-sensitive).
                                *  - `z' A 'z' may be present as the first character of the string. If present, the
                                *        Augmentation Data field shall be present. The contents of the Augmentation Data
                                *        shall be interpreted according to other characters in the Augmentation String.
                                *  - `L' A 'L' may be present at any position after the first character of the string. This
                                *        character may only be present if 'z' is the first character of the string. If present,
                                *        it indicates the presence of one argument in the Augmentation Data of the CIE, and
                                *        a corresponding argument in the Augmentation Data of the FDE. The argument in the
                                *        Augmentation Data of the CIE is 1-byte and represents the pointer encoding used
                                *        for the argument in the Augmentation Data of the FDE, which is the address of a
                                *        language-specific data area (LSDA). The size of the LSDA pointer is specified
                                *        by the pointer encoding used.
                                *  - `P' A 'P' may be present at any position after the first character of the string. This
                                *        character may only be present if 'z' is the first character of the string. If present,
                                *        it indicates the presence of two arguments in the Augmentation Data of the CIE. The
                                *        first argument is 1-byte and represents the pointer encoding used for the second
                                *        argument, which is the address of a personality routine handler. The personality
                                *        routine is used to handle language and vendor-specific tasks. The system unwind
                                *        library interface accesses the language-specific exception handling semantics via
                                *        the pointer to the personality routine. The personality routine does not have an
                                *        ABI-specific name. The size of the personality routine pointer is specified by the
                                *        pointer encoding used.
                                *  - `R' A 'R' may be present at any position after the first character of the string. This
                                *        character may only be present if 'z' is the first character of the string. If present,
                                *        The Augmentation Data shall include a 1 byte argument that represents the pointer
                                *        encoding for the address pointers used in the FDE.
                                *        NOTE: The argument is one of `DW_EH_PE_*'
                                */
    uleb128_t c_codealignfac;  /* An unsigned LEB128 encoded value that is factored out of all advance location
                                * instructions that are associated with this CIE or its FDEs. This value shall be
                                * multiplied by the delta argument of an advance location instruction to obtain the
                                * new location value. */
    sleb128_t c_dataalignfac;  /* A signed LEB128 encoded value that is factored out of all offset instructions that
                                * are associated with this CIE or its FDEs. This value shall be multiplied by the
                                * register offset argument of an offset instruction to obtain the new offset value. */
    uleb128_t c_returnreg;     /* An unsigned LEB128 constant that indicates which column in the
                                * rule table represents the return address of the function. 
                                * Note that this column might not correspond to an actual machine register. */
    uleb128_t c_auglength;     /* [exists_if("z" in c_augstr)]
                                * An unsigned LEB128 encoded value indicating the length in bytes of the
                                * Augmentation Data. This field is only present if the Augmentation String
                                * contains the character 'z' */
    byte_t    c_augdata[1];    /* [c_auglength]
                                * A block of data whose contents are defined by the contents of the Augmentation
                                * String as described below. This field is only present if the Augmentation String
                                * contains the character 'z'. The size of this data is given by the Augmentation Length. */
    byte_t    c_initinstr[1];  /* Initial set of Call Frame Instructions. The number of instructions
                                * is determined by the remaining space in the CIE record. */
    byte_t    c_pad[1];        /* Padding to align by pointers. */
};

struct FDE {
    /* FDE -- Frame Description Entry */
    u32       f_length;        /* The length of the FDE structure, excluding this field.
                                * When ZERO(0), the FDE acts as a terminator.
                                * When (u32)-1, the actual length is stored in `f_length64',
                                * a field that doesn't exist under other circumstances. */
    u64       f_length64;      /* [exists_if(f_length == (u32)-1)] 64-bit length. */
    u32       f_cieptr;        /* A 4 byte unsigned value that when subtracted from the offset of the CIE Pointer
                                * in the current FDE yields the offset of the start of the associated CIE. This
                                * value shall never be 0 (ZERO indicates a CIE structure). */
    ehptr_t   f_pcbegin;       /* Starting address of this FDE (Encoded as one of `DW_EH_PE_*', using `R' from `CIE::c_augstr') */
    ehptr_t   f_pcsize;        /* The absolute size of this FDE (Encoded as one of `DW_EH_PE_*', using `R' from `CIE::c_augstr & 0xf') */
    uleb128_t f_auglength;     /* An unsigned LEB128 encoded value indicating the length in bytes of the
                                * Augmentation Data. This field is only present if the Augmentation String
                                * in the associated CIE contains the character 'z' */
    byte_t    f_augdata[1];    /* [f_auglength]
                                * A block of data whose contents are defined by the contents of the Augmentation
                                * String in the associated CIE as described above. This field is only present if the
                                * Augmentation String in the associated CIE contains the character 'z'. The size of
                                * this data is given by the Augmentation Length */
    byte_t    c_callinstr[1];  /* A set of Call Frame Instructions. */
};


struct fde_info {
    struct CIE        *fi_cie;       /* [1..1] Starting address of the associated CIE. */
    struct FDE        *fi_fde;       /* [1..1] Starting address of the associated FDE. */
    struct eh_relinfo *fi_relinfo;   /* [0..1] Relocation information. */
    uintptr_t          fi_pcbegin;   /* [< fi_pcend] PC (Program counter) starting address. */
    uintptr_t          fi_pcend;     /* [> fi_pcbegin] PC (Program counter) ending address. */
    char              *fi_augstr;    /* [1..1] Augmentation string pointed to by `fi_cie' */
    uintptr_t          fi_codealign; /* Code alignment. (Multiplied with the delta argument of an advance location instruction) */
    intptr_t           fi_dataalign; /* Data alignment. (Multiplied with the register offset argument of an offset instruction) */
    uintptr_t          fi_persofun;  /* [0..1] Address of a personality handler function. */
    uintptr_t          fi_lsdaaddr;  /* [0..1] Address of a language-specific data block. */
    size_t             fi_initsize;  /* Max size of the `fi_inittext' block in bytes. */
    eh_instr_t        *fi_inittext;  /* [0..fi_initsize] Pointer to initial EH instructions (usually describing compiler-generated frames) */
    size_t             fi_evalsize;  /* Max size of the `fi_evaltext' block in bytes. */
    eh_instr_t        *fi_evaltext;  /* [0..fi_evalsize] Pointer to FDE-specific EH instructions (This is the meat of it all) */
    uintptr_t          fi_retreg;    /* Return register number. */
    u8                 fi_encptr;    /* Encoding used for pointers (One of `DW_EH_PE_*') */
    u8                 fi_enclsda;   /* Encoding used for LSDA (One of `DW_EH_PE_*') */
    u8                 fi_encperso;  /* Encoding used for personality function pointer (One of `DW_EH_PE_*') */
    u8                 fi_sigframe;  /* Non-zero if this is a signal frame. */
};
struct eh_relinfo {
    /* Relocations information required by `eh_findfde()'. */
    uintptr_t   er_textaddr; /* Starting address of the `.text' section. */
};


/* Find the FDE associated with a given `ip' by
 * searching the given `eh_frame' section.
 * If not found, `false' is returned.
 * @param: relinfo: When non-NULL, additional relocation information.
 * WARNING: Do not call this function if the origin
 *          of `eh_frame_start' cannot be trusted. */
FUNDEF bool KCALL
eh_findfde(byte_t *__restrict eh_frame_start,
           size_t eh_frame_size, uintptr_t ip,
           struct fde_info *__restrict result,
           struct eh_relinfo *relinfo);

struct cpu_context;

/* Emulate the behavior of an return-statement, unwinding
 * the stack described by the given CPU-context, and the
 * associated `info' that should be retrieved through
 * `eh_findfde()' with the IP stored in `ctx'.
 * If this is not possible, `ctx' is left undefined, and `false' is returned.
 * @param: flags: Set of `EH_F*' */
FUNDEF bool KCALL eh_return(struct fde_info *__restrict info,
                            struct cpu_context *__restrict ctx,
                            unsigned int flags);

/* Jump to another location within the active CPU context,
 * given the associated `info' that should be retrieved through
 * `eh_findfde()' with the IP stored in `ctx'.
 * The given `abs_ip' should also be located within the FDE described by `info',
 * however if it is, or isn't has no influence on whether or not this function succeeds.
 * If this is not possible, `ctx' is left undefined, and `false' is returned.
 * In the end, this function is used to do the final `jmp' to an exception handler
 * after `eh_return()' has been used to unwind the stack until one was found,
 * meaning that this is an integral part of KOSmk3's new exception handling system.
 * @param: flags: Set of `EH_F*' */
FUNDEF bool KCALL eh_jmp(struct fde_info *__restrict info,
                         struct cpu_context *__restrict ctx,
                         uintptr_t abs_ip,
                         unsigned int flags);

#define EH_FNORMAL               0x0000 /* Normal flags. */
#define EH_FRESTRICT_USERSPACE   0x0001 /* Validate user-space pointers before dereferencing them. */
#define EH_FDONT_UNWIND_SIGFRAME 0x0002 /* Don't unwind signal frames. (for use with `eh_return') */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_UNWIND_EH_FRAME_H */
