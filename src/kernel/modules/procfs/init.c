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
#ifndef GUARD_KERNEL_MODULES_PROCFS_INIT_C
#define GUARD_KERNEL_MODULES_PROCFS_INIT_C 1

#include <hybrid/compiler.h>
#include <fs/node.h>
#include <fs/driver.h>
#include <kernel/debug.h>

DECL_BEGIN

DEFINE_DRIVER_INIT(my_driver_foo);
void KCALL my_driver_foo(void) {
 debug_printf("my_driver_foo()\n");
}

INTERN void KCALL module_main(int argc, char *argv[]) {
 int i;
 debug_printf("module_main(%d,%p)\n",argc,argv);
 for (i = 0; i < argc; ++i)
     debug_printf("\targv[%d] = %q\n",i,argv[i]);
 
}


DECL_END

#endif /* !GUARD_KERNEL_MODULES_PROCFS_INIT_C */
