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

#define DRIVER_PARAM_TYPE_OPTION  0x00 /* `[-[-]]dp_name=dp_hand:arg' */
#define DRIVER_PARAM_TYPE_FLAG    0x01 /* `[-[-]]dp_name' (dp_hand:arg=NULL) */
#define DRIVER_PARAM_TYPE_MASK    0x07 /* Mask for the parameter type (other bits are served for flags) */

#define DRIVER_SPECS_VERSION  0x0000 /* The current driver specs version. */

#ifdef __CC__
typedef void (KCALL *driver_init_t)(void);
typedef void (KCALL *driver_fini_t)(void);
typedef void (KCALL *driver_param_handler_t)(char *arg);
typedef void (KCALL *driver_main_t)(int argc, char **argv);
struct PACKED driver_param {
    /* sizeof(THIS) == 4*POINTER */
    u8                  dp_type;     /* Parameter type (One of `DRIVER_PARAM_TYPE_F*') */
    union PACKED {
        struct PACKED {
            char        dp_zero;
            char      __dp_pad[sizeof(void *)-(sizeof(u8)+sizeof(char))];
            image_rva_t dp_name_ptr; /* [valid_if(dp_zero == 0)]
                                      * Image-relative pointer to the parameter name. */
        };
        char            dp_name_inline[((sizeof(void *)*3)-sizeof(u8))/sizeof(char)];
                                     /* [valid_if(dp_zero != 0)]
                                      * Inline name of this parameter. */
    };
    image_rva_t         dp_hand;     /* Parameter handler function (`driver_param_handler_t'). */
};
#define DRIVER_PARAM_NAME(loadaddr,param) \
       ((param)->dp_zero ? (param)->dp_name_inline : (char *)((loadaddr)+(param)->dp_name_ptr))
#define DRIVER_PARAM_HAND(loadaddr,param) \
       ((driver_param_handler_t)((loadaddr)+(param)->dp_hand))


struct PACKED driver_specs {
    /* Driver specifications.
     * INITIALIZATION:
     *     TRY {
     *         #1: Execute functions enumeratable by `application_enuminit()'
     *         TRY {
     *             #2: Step through elements of `ds_parm' and execute handlers
     *                 matching arguments provided by the module commandline.
     *             #3: Execute all elements of `ds_init' (in ascending order)
     *                 NOTE: At link-time, this vector is ordered to start with
     *                      `DEFINE_DRIVER_PREINIT()', before functions registered
     *                       by `DEFINE_DRIVER_INIT()' show up.
     *                 Arguments that were handled by driver parameter handlers
     *                 are removed from the driver commandline.
     *             #4: If defined, execute `ds_main' with the remaining driver
     *                 arguments. When not defined, warn about any driver
     *                 arguments that were left unused.
     *         } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     *             #2.1: Execute all functions from `ds_fini' in reverse order
     *                   NOTE: Yes, all fini functions are executed when a single
     *                         init fails, meaning that fini should work assume
     *                         static pre-initialization.
     *             error_rethrow();
     *         } 
     *     } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     *         #1.1: Execute functions enumeratable by `application_enumfini()'
     *               NOTE: Yes, all fini functions are executed when a single
     *                     init fails, meaning that fini should work assume
     *                     static pre-initialization.
     *         error_rethrow();
     *     } 
     * FINALIZATION:
     *     #1: Execute all functions from `ds_fini' in reverse order
     *     #2: Execute functions enumeratable by `application_enumfini()' */
    uintptr_t      ds_version; /* [== DRIVER_SPECS_VERSION] Driver specifications version. */
    image_rva_t    ds_init;    /* Pointer to an array of `image_rva_t' with `ds_init_sz' elements.
                                * Called after ELF module constructors; may be located in `.free' memory. */
    size_t         ds_init_sz; /* Number of driver initializers. */
    image_rva_t    ds_fini;    /* Pointer to an array of `image_rva_t' with `ds_fini_sz' elements.
                                * Called before ELF module destructors. (executed in reverse order) */
    size_t         ds_fini_sz; /* Number of driver finalizers. */
    image_rva_t    ds_parm;    /* Pointer to an array of `struct driver_param' with `ds_parm_sz' elements.
                                * Called after ELF module constructors; before `ds_init'. */
    size_t         ds_parm_sz; /* Number of driver finalizers. */
    image_rva_t    ds_main;    /* Pointer to the module main() function, or ZERO(0) if no such function exists.
                                * Called after `ds_init' (the last step during initialization).
                                * NOTE: Driver parameters already processed by `ds_parm' are not
                                *       passed to this function.
                                * NOTE: This symbol should be a function called `module_main'
                                * >> void KCALL module_main(int argc, char **argv) {
                                * >> }
                                */
};

struct driver {
    struct application   d_app;  /* The underlying application. */
    struct driver_specs *d_spec; /* [1..1][const] Driver specifications (`dlsym("__$$OS$driver_specs")') */
    /* TODO ... */
};

/* Increment/decrement the reference counter of the given driver `x' */
#define driver_tryincref(x) application_tryincref(&(x)->d_app)
#define driver_incref(x)    application_incref(&(x)->d_app)
#define driver_decref(x)    application_decref(&(x)->d_app)

/* The kernel module & driver. */
DATDEF struct module kernel_module;
DATDEF struct driver kernel_driver;

/* Private to every driver module: The driver's own driver descriptor. */
INTDEF struct driver this_driver;




/* Load `mod' as a driver into the kernel. */
FUNDEF ATTR_RETNONNULL REF struct driver *
KCALL kernel_insmod(struct module *__restrict mod, bool *pwas_newly_loaded,
                    USER CHECKED char const *module_commandline);


/* Delete the given module:
 *   #1: Set the `APPLICATION_FCLOSING' flag if it wasn't set already.
 *   #2: Get rid of all global hooks of the driver (devices, filesystem types, etc.).
 *   #3: Send `E_INTERRUPT' exceptions to all threads who's active stack portion
 *       contains pointers directed into the segment of the application.
 *  XXX: Terminate threads started by `app'?
 *  XXX: Unmap memory of `app' */
FUNDEF void KCALL kernel_delmod(struct driver *__restrict app);

/* Lookup the driver associated with a given module, or
 * return NULL if that module hasn't been loaded as a driver. */
FUNDEF REF struct driver *KCALL kernel_getmod(struct module *__restrict mod);


#endif /* __CC__ */


#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Define an initializer function to-be called during core driver initialization.
 * >> INTERN ATTR_FREETEXT void KCALL my_core_driver_init(void); */
#define DEFINE_CORE_DRIVER_PREINIT(func) DEFINE_CALLBACK(".rodata.core_driver.preinit.free",func)
#define DEFINE_CORE_DRIVER_INIT(func)    DEFINE_CALLBACK(".rodata.core_driver.init.free",func)
#define DEFINE_DRIVER_PREINIT(func)      DEFINE_CORE_DRIVER_PREINIT(func)
#define DEFINE_DRIVER_INIT(func)         DEFINE_CORE_DRIVER_INIT(func)
#else
#define DEFINE_DRIVER_PREINIT(func) /* TODO */
#define DEFINE_DRIVER_INIT(func)    /* TODO */
#define DEFINE_DRIVER_FINI(func)    /* TODO */
#endif



DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_DRIVER_H */
