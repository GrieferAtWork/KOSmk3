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
#ifndef GUARD_KERNEL_MODULES_PROCFS_ROOT_C
#define GUARD_KERNEL_MODULES_PROCFS_ROOT_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <fs/node.h>
#include <fs/path.h>
#include <string.h>

#include "inode.h"

DECL_BEGIN

PRIVATE void KCALL
Root_Enum(struct directory_node *__restrict UNUSED(node),
          directory_enum_callback_t callback, void *arg) {
 (*callback)("cmdline",    DT_REG,PROCFS_INODE_CMDLINE,    arg);
 (*callback)("self",       DT_LNK,PROCFS_INODE_SELF,       arg);
 (*callback)("thread-self",DT_LNK,PROCFS_INODE_THREAD_SELF,arg);
}


INTERN struct inode_operations Iprocfs_root_dir = {
    /* /proc */
    .io_directory = {
        .d_oneshot = {
            .o_enum = &Root_Enum,
        }
    }
};

DECL_END

#endif /* !GUARD_KERNEL_MODULES_PROCFS_ROOT_C */
