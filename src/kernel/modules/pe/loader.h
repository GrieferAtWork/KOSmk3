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
#ifndef GUARD_KERNEL_MODULES_PE_LOADER_H
#define GUARD_KERNEL_MODULES_PE_LOADER_H 1

#include <hybrid/compiler.h>
#include <fs/driver.h>
#include <fs/linker.h>

#include <winapi/winnt.h>

DECL_BEGIN


typedef struct PACKED module_data {
    union PACKED {
        IMAGE_DATA_DIRECTORY     pm_Dir[3]; /* Data directories we keep track of */
        struct PACKED {
            IMAGE_DATA_DIRECTORY pm_Export; /* IMAGE_DIRECTORY_ENTRY_IMPORT */
            IMAGE_DATA_DIRECTORY pm_Import; /* IMAGE_DIRECTORY_ENTRY_EXPORT */
            IMAGE_DATA_DIRECTORY pm_Reloc;  /* IMAGE_DIRECTORY_ENTRY_BASERELOC */
        };
    };
    WORD                         pm_NumSections; /* The Number of sections. */
    WORD                       __pm_Pad[(sizeof(void *)-sizeof(WORD))/sizeof(WORD)]; /* ... */
    REF struct vm_region       **pm_Regions;     /* [0..1][const][0..pm_NumSections][lock(WRITE_ONCE)][owned]
                                                  * Vector of lazily allocated VM regions mapping application data. */
    IMAGE_SECTION_HEADER         pm_Sections[1]; /* [pm_NumSections] Vector of program sections. */
} PE_MODULE;


#define SHOULD_USE_IMAGE_SECTION_HEADER(x) \
      (((x)->VirtualAddress+(x)->Misc.VirtualSize) > (x)->VirtualAddress && \
      !((x)->Characteristics&(IMAGE_SCN_LNK_INFO|IMAGE_SCN_LNK_REMOVE)))


DECL_END

#endif /* !GUARD_KERNEL_MODULES_PE_LOADER_H */
