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
#ifndef GUARD_KERNEL_INCLUDE_FS_DRIVER_H
#define GUARD_KERNEL_INCLUDE_FS_DRIVER_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/sections.h>
#include <fs/linker.h>

DECL_BEGIN

#ifdef __CC__
struct driver {
    struct application d_app; /* The underlying application. */
    /* TODO ... */
};
#endif /* __CC__ */

/* Increment/decrement the reference counter of the given driver `x' */
#define driver_tryincref(x) application_tryincref(&(x)->d_app)
#define driver_incref(x)    application_incref(&(x)->d_app)
#define driver_decref(x)    application_decref(&(x)->d_app)

#ifdef __CC__
/* The kernel module & driver. */
DATDEF struct module kernel_module;
DATDEF struct driver kernel_driver;

/* Private to every driver module: The driver's own driver descriptor. */
INTDEF struct driver this_driver;
#endif /* __CC__ */


#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Define an initializer function to-be called during core driver initialization.
 * >> INTERN ATTR_FREETEXT void KCALL my_core_driver_init(void); */
#define DEFINE_CORE_DRIVER_PREINIT(func) DEFINE_CALLBACK(".rodata.core_driver.preinit.free",func)
#define DEFINE_CORE_DRIVER_INIT(func)    DEFINE_CALLBACK(".rodata.core_driver.init.free",func)
#define DEFINE_DRIVER_PREINIT(func) DEFINE_CORE_DRIVER_PREINIT(func)
#define DEFINE_DRIVER_INIT(func)    DEFINE_CORE_DRIVER_INIT(func)
#else
#define DEFINE_DRIVER_PREINIT(func) /* TODO */
#define DEFINE_DRIVER_INIT(func)    /* TODO */
#endif




DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_DRIVER_H */
