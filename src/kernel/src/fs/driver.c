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
#ifndef GUARD_KERNEL_SRC_FS_DRIVER_C
#define GUARD_KERNEL_SRC_FS_DRIVER_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <fs/linker.h>
#include <fs/driver.h>
#include <fs/node.h>
#include <fs/path.h>
#include <kernel/sections.h>
#include <unwind/debug_line.h>
#include <elf.h>
#include <string.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/driver.h>
#else
#error "Unsupported architecture"
#endif


DECL_BEGIN

INTDEF void ASMCALL _start(void);
INTDEF struct except_handler kernel_except_start[];
INTDEF struct except_handler kernel_except_end[];
INTDEF byte_t kernel_except_size[];
INTDEF byte_t kernel_ehframe_start[];
INTDEF byte_t kernel_ehframe_end[];
INTDEF byte_t kernel_ehframe_size[];
INTDEF byte_t kernel_debug_line_start[];
INTDEF byte_t kernel_debug_line_end[];
INTDEF byte_t kernel_debug_line_size[];

/* Clutch to make sure that the .debug_line section is allocated. */
INTERN ATTR_USED ATTR_SECTION(".data.debug_line.clutch")
void *__debug_line_clutch = NULL;


PRIVATE struct module_symbol KCALL
kernel_symbol(struct application *__restrict UNUSED(app),
              USER CHECKED char const *__restrict name, u32 hash) {
 struct module_symbol result;
 /* TODO: symbols exported from the kernel core! */
 result.ms_type = MODULE_SYMBOL_INVALID;
 return result;
}
PRIVATE struct module_section KCALL
kernel_section(struct application *__restrict UNUSED(app),
               USER CHECKED char const *__restrict name) {
 struct module_section result;
 if (!strcmp(name,".except"))
      return kernel_module.m_sect.m_except;
 if (!strcmp(name,".eh_frame"))
      return kernel_module.m_sect.m_eh_frame;
 /* TODO: .debug_line */

 result.ms_size = 0;
 return result;
}


PRIVATE struct module_type kernel_module_type = {
    .m_flags   = MODULE_TYPE_FNORMAL,
    .m_symbol  = &kernel_symbol,
    .m_section = &kernel_section,
};

struct kernel_directory_entry {
    /* XXX: This structure must be kept in sync with `/src/kernel/include/fs/node.h' */
    ATOMIC_DATA ref_t                 de_refcnt;
    struct directory_entry           *de_next;
    LIST_NODE(struct directory_entry) de_bypos;
    struct PACKED {
        pos_t                         de_start;
        unsigned char                 de_data[16];
    }                                 de_fsdata;
    pos_t                             de_pos;
    union PACKED {
        REF struct inode             *de_virtual;
        ino_t                         de_ino;
    };
    uintptr_t                         de_hash;
    u16                               de_namelen;
    unsigned char                     de_type;
    char                              de_name[COMPILER_STRLEN(KERNEL_BIN_FILENAME)+1];
};

PRIVATE struct kernel_directory_entry kernel_bin_dirent = {
    .de_refcnt  = 1,
    .de_hash    = KERNEL_BIN_FILENAME_HASH,
    .de_namelen = COMPILER_STRLEN(KERNEL_BIN_FILENAME),
    .de_type    = DT_REG,
    .de_name    = KERNEL_BIN_FILENAME
};
PRIVATE struct path kernel_bin_path = {
    .p_refcnt   = 1,
    .p_vfs      = &vfs_kernel,
    .p_parent   = &vfs_kernel.v_root,
    .p_dirent   = (struct directory_entry *)&kernel_bin_dirent,
    .p_lock     = ATOMIC_RWLOCK_INIT,
    .p_flags    = PATH_FCLOSED
};

PRIVATE struct module_debug kernel_debug = {
    .md_debug_line = {
        .ms_base    = (uintptr_t)kernel_debug_line_start,
        .ms_size    = (uintptr_t)kernel_debug_line_size,
        .ms_offset  = 0,
        .ms_type    = SHT_PROGBITS,
        .ms_flags   = SHF_WRITE|SHF_ALLOC,
        .ms_entsize = 0
    },
    .md_data = kernel_debug_line_start
};

PUBLIC struct module kernel_module = {
    .m_refcnt    = 1,
    .m_type      = &kernel_module_type,
    .m_driver    = &kernel_driver,
    .m_path      = &kernel_bin_path,
    .m_fsloc     = NULL,
    .m_fixedbase = 0,
    .m_imagemin  = (uintptr_t)kernel_start,
    .m_imageend  = (uintptr_t)kernel_end_raw,
    .m_entry     = (image_rva_t)(uintptr_t)&_start,
    .m_flags     = MODULE_FFIXED|MODULE_FENTRY|MODULE_FSECTLOADED|MODULE_FSECTLOADING,
    .m_sect      = {
        .m_except = {
            .ms_base    = (image_rva_t)(uintptr_t)kernel_except_start,
            .ms_size    = (size_t)kernel_except_size,
            .ms_type    = SHT_PROGBITS,
            .ms_flags   = SHF_ALLOC,
            .ms_entsize = sizeof(struct except_handler), /* Sure... Why not? (although this isn't a rule...) */
        },
        .m_eh_frame = {
            .ms_base    = (image_rva_t)(uintptr_t)kernel_ehframe_start,
            .ms_size    = (size_t)kernel_ehframe_size,
            .ms_type    = SHT_PROGBITS,
            .ms_flags   = SHF_ALLOC,
        },
    },
    .m_debug     = &kernel_debug,
};


DEFINE_INTERN_ALIAS(this_driver,kernel_driver);
PUBLIC struct driver kernel_driver = {
    .d_app = {
        .a_refcnt   = 0x3fffffff,
        .a_weakcnt  = 1,
        .a_mapcnt   = 1,
        .a_loadaddr = 0,
        .a_bounds = {
           .b_min   = (uintptr_t)kernel_start,
           .b_max   = (uintptr_t)kernel_end_raw-1,
        },
        .a_module   = &kernel_module,
        .a_requirec = 0,
        .a_requirev = NULL,
        .a_type     = APPLICATION_TYPE_FDRIVER,
        .a_flags    = APPLICATION_FDIDINIT|APPLICATION_FTRUSTED,
    },
};

DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_DRIVER_C */
