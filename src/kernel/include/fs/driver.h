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
#include <hybrid/section.h>
#include <kos/types.h>
#include <kernel/sections.h>
#include <fs/linker.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/driver.h>
#else
#error "Unsupported architecture"
#endif


DECL_BEGIN


/* DRIVER INITIALIZATION:
 *     TRY {
 *         #1: Execute functions enumeratable by `application_enuminit()'
 *         TRY {
 *             #2: Step through elements of `DRIVER_TAG_PARM' and execute handlers
 *                 matching arguments provided by the module commandline.
 *             #3: Execute all elements of `DRIVER_TAG_INIT' (in ascending order)
 *                 NOTE: At link-time, this vector is ordered to start with
 *                      `DEFINE_DRIVER_PREINIT()', before functions registered
 *                       by `DEFINE_DRIVER_INIT()' show up.
 *                 Arguments that were handled by driver parameter handlers
 *                 are removed from the driver commandline.
 *             #4: If defined, execute `DRIVER_TAG_MAIN' with the remaining driver
 *                 arguments. When not defined, warn about any driver
 *                 arguments that were left unused.
 *             #5: Unmap the `DRIVER_TAG_FREE' segment. (if defined and non-empty)
 *         } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
 *             #2.1: Execute all functions from `DRIVER_TAG_FINI' in reverse order
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
 *
 * DRIVER FINALIZATION:
 *     #1: Execute all functions from `DRIVER_TAG_FINI' tags in reverse order
 *     #2: Execute functions enumeratable by `application_enumfini()'
 */


#define DRIVER_PARAM_TYPE_OPTION  0x00 /* `[-[-]]dp_name=dp_hand:arg' (invoked as `driver_param_handler_t') */
#define DRIVER_PARAM_TYPE_FLAG    0x01 /* `[-[-]]dp_name' (invoked as `driver_flag_handler_t') */
#define DRIVER_PARAM_TYPE_MASK    0x07 /* Mask for the parameter type (other bits are served for flags) */

#if __SIZEOF_POINTER__ == 4
#define DRIVER_PARAM_SHORTNAME_MAXLEN 11
#else
#define DRIVER_PARAM_SHORTNAME_MAXLEN 23
#endif

#ifdef __CC__
typedef void (KCALL *driver_init_t)(void);
typedef void (KCALL *driver_fini_t)(void);
typedef void (KCALL *driver_flag_handler_t)(void);
typedef void (KCALL *driver_param_handler_t)(char *arg);
typedef void (KCALL *driver_main_t)(int argc, char **argv);
struct PACKED driver_param {
    /* sizeof(THIS) == 4*POINTER */
    u8                  dp_type;     /* Parameter type (One of `DRIVER_PARAM_TYPE_F*') */
    union PACKED {
        struct PACKED {
            char        dp_zero;     /* When ZERO, a name pointer is used. */
            char      __dp_pad[sizeof(void *)-(sizeof(u8)+sizeof(char))];
            image_rva_t dp_name_ptr; /* [valid_if(dp_zero == 0)]
                                      * Image-relative pointer to the parameter name. */
            image_rva_t dp_unused;   /* Currently unused. */
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
#define DRIVER_PARAM_FLAGHAND(loadaddr,param) \
       ((driver_flag_handler_t)((loadaddr)+(param)->dp_hand))
#endif /* __CC__ */


#define DRIVER_TAG_STOP 0x0000 /* END tag (sentinel) (Must _ALWAYS_ be ZERO(0)!) */
#define DRIVER_TAG_MAIN 0x0001 /* Pointer to the module main() function.
                                * Called after `DRIVER_TAG_INIT' (the last step during initialization).
                                * NOTE: Driver parameters already processed by `ds_parm' are not
                                *       passed to this function.
                                * NOTE: This symbol should be a function called `module_main'
                                * >> void KCALL module_main(int argc, char **argv) {
                                * >> }
                                * NOTE: `dt_count' is undefined. */
#define DRIVER_TAG_INIT 0x0002 /* Pointer to an array of `image_rva_t' with `dt_count' elements.
                                * Called after ELF module constructors; may be located in `.free' memory. */
#define DRIVER_TAG_FINI 0x0004 /* Pointer to an array of `image_rva_t' with `dt_count' elements.
                                * Called before ELF module destructors. (executed in reverse order) */
#define DRIVER_TAG_PARM 0x0005 /* Pointer to an array of `struct driver_param' with `dt_count' elements.
                                * Called after ELF module constructors; before `DRIVER_TAG_INIT'. */
#define DRIVER_TAG_FREE 0x0006 /* Starting page number of the driver's .free section. (vm_vpage_t)
                                * NOTE: `dt_count' is the size of the driver's .free section (in pages) */
#define DRIVER_TAG_FNORMAL   0x0000 /* Normal driver tag flags. */
#define DRIVER_TAG_FOPTIONAL 0x0001 /* Normal driver tag is optional. */

#ifdef __CC__
struct PACKED driver_tag {
    u16          dt_name;  /* Name of the tag (One of `DRIVER_TAG_*')
                            * NOTE: Unknown tags are ignored if the `DRIVER_TAG_FOPTIONAL' flag is set,
                            *       otherwise unknown tags cause driver initialization to fail
                            *       with `E_NOT_IMPLEMENTED'. */
    u16          dt_flag;  /* Driver tag flags (Set of `DRIVER_TAG_F*') */
#if __SIZEOF_POINTER__ > 4
    u16        __dt_pad[(sizeof(void *)-2)/2]; /* ... */
#endif
    image_rva_t  dt_start; /* Starting address of the tag's data block. */
    size_t       dt_count; /* Number of elements within the tag's data block. */
};

struct driver {
    struct application       d_app;     /* The underlying application. */
    struct driver_tag const *d_spec;    /* [1..1][const] Driver specifications (`dlsym("__$$OS$driver_specs")') */
    char                    *d_cmdline; /* [0..1][lock(WRITE_ONCE && creator)][owned] Driver commandline.
                                         * Set once during driver initialization and never changed.
                                         * NOTE: This string is split into arguments by NUL-characters,
                                         *       followed by 2 trailing NUL characters. */
    size_t                   d_cmdsize; /* [lock(WRITE_ONCE && creator)] The total length of the
                                         * cmdline (including one of the 2 trailing NUL characters). */
};

/* Increment/decrement the reference counter of the given driver `x' */
#define driver_tryincref(x) application_tryincref(&(x)->d_app)
#define driver_incref(x)    application_incref(&(x)->d_app)
#define driver_decref(x)    application_decref(&(x)->d_app)

/* The kernel module & driver. */
DATDEF struct module kernel_module;
DATDEF struct driver kernel_driver;

/* Private to every driver module: The driver's own driver descriptor. */
#ifdef CONFIG_BUILDING_KERNEL_CORE
INTDEF struct driver this_driver;
#else
/* Public visibility, because provided by the kernel. */
DATDEF struct driver this_driver;
#endif




/* Load `mod' as a driver into the kernel.
 * NOTE: The caller is responsible to ensure that `module_commandline_length'
 *       isn't too large, although the only real limit is available memory.
 *       However, since this function is usually called using `kernctl()'
 *      (a user-space facility), the caller should still validate that
 *       the commandline isn't too long (because the kernel has to copy it) */
FUNDEF ATTR_RETNONNULL REF struct driver *
KCALL kernel_insmod(struct module *__restrict mod, bool *pwas_newly_loaded,
                    USER CHECKED char const *module_commandline,
                    size_t module_commandline_length);


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
#define DEFINE_DRIVER_PREINIT(func)       DEFINE_ABS_CALLBACK(".rodata.core_driver.preinit",func)
#define DEFINE_DRIVER_INIT(func)          DEFINE_ABS_CALLBACK(".rodata.core_driver.init",func)
#define DEFINE_DRIVER_POSTINIT(func)      DEFINE_ABS_CALLBACK(".rodata.core_driver.postinit",func)
#else
#define DEFINE_DRIVER_PREINIT(func)       DEFINE_REL_CALLBACK(".rodata.driver.preinit",func)
#define DEFINE_DRIVER_INIT(func)          DEFINE_REL_CALLBACK(".rodata.driver.init",func)
#define DEFINE_DRIVER_POSTINIT(func)      DEFINE_REL_CALLBACK(".rodata.driver.postinit",func)
#define DEFINE_DRIVER_PREFINI(func)       DEFINE_REL_CALLBACK(".rodata.driver.prefini",func)
#define DEFINE_DRIVER_FINI(func)          DEFINE_REL_CALLBACK(".rodata.driver.fini",func)
#define DEFINE_DRIVER_POSTFINI(func)      DEFINE_REL_CALLBACK(".rodata.driver.postfini",func)
#endif


/* Symbol name generator. */
#define __DRIVER_BOOL_SYMNAME   __PP_CAT2(__driver_bool_,__LINE__)
#define __DRIVER_STRING_SYMNAME __PP_CAT2(__driver_string_,__LINE__)
#define __DRIVER_PARAM_SYMNAME  __PP_CAT2(__driver_param_,__LINE__)
#define __DRIVER_FLAG_SYMNAME   __PP_CAT2(__driver_flag_,__LINE__)


/* Define a driver parameter handler:
 * >> DEFINE_DRIVER_PARAM("foo",foo_handler); // "foo=xxx"
 * >> PRIVATE ATTR_FREETEXT ATTR_USED void KCALL foo_handler(char *arg) {
 * >>     debug_printf("driver initializer with foo=%s\n",arg);
 * >> }
 * >> DEFINE_DRIVER_FLAG("bar",bar_handler); // "bar"
 * >> PRIVATE ATTR_FREETEXT ATTR_USED void KCALL bar_handler(void) {
 * >>     debug_printf("driver initializer with bar\n");
 * >> }
 */
#define DEFINE_DRIVER_PARAM_FUNC(name,handler) \
        DEFINE_DRIVER_PARAM_EX(name,DRIVER_PARAM_TYPE_OPTION,handler)
#define DEFINE_DRIVER_FLAG_FUNC(name,handler) \
        DEFINE_DRIVER_PARAM_EX(name,DRIVER_PARAM_TYPE_FLAG,handler)


/* Define a boolean driver flag that is set to `true'
 * when the appropriate commandline option is passed.
 * >> INTERN bool operate_in_alternate_mode = false;
 * >> DEFINE_DRIVER_BOOL(operate_in_alternate_mode,"altmode");
 * >> void foobar() {
 * >>     if (!operate_in_alternate_mode) {
 * >>        // A
 * >>     } else {
 * >>        // B
 * >>     }
 * >> } */
#define DEFINE_DRIVER_BOOL(varname,paramname) \
        DEFINE_DRIVER_PARAM_EX(paramname,DRIVER_PARAM_TYPE_FLAG,__DRIVER_BOOL_SYMNAME); \
        PRIVATE ATTR_FREETEXT ATTR_USED void KCALL __DRIVER_BOOL_SYMNAME(void) { varname = true; }

/* Enable a set of flags `flagmask' in `flagset'
 * when `paramname' is passed on the commandline.
 * >> INTERN u32 my_driver_flags = 0;
 * >> DEFINE_DRIVER_FLAG(my_driver_flags,"fa",0x0001);
 * >> DEFINE_DRIVER_FLAG(my_driver_flags,"fb",0x0002);
 * >> DEFINE_DRIVER_FLAG(my_driver_flags,"fc",0x0004);
 * >> DEFINE_DRIVER_FLAG(my_driver_flags,"fd",0x0008); */
#define DEFINE_DRIVER_FLAG(flagset,paramname,flagmask) \
        DEFINE_DRIVER_PARAM_EX(paramname,DRIVER_PARAM_TYPE_FLAG,__DRIVER_FLAG_SYMNAME); \
        PRIVATE ATTR_FREETEXT ATTR_USED void KCALL __DRIVER_FLAG_SYMNAME(void) { flagset |= flagmask; }


/* Enable a set of flags `flagmask' in `flagset'
 * when `paramname' is passed on the commandline.
 * >> INTERN char *my_driver_string = "foo";
 * >> DEFINE_DRIVER_STRING(my_driver_string,"stringoption"); // stringoption=bar */
#define DEFINE_DRIVER_STRING(varname,paramname) \
        DEFINE_DRIVER_PARAM_FUNC(paramname,__DRIVER_STRING_SYMNAME); \
        PRIVATE ATTR_FREETEXT ATTR_USED void KCALL \
        __DRIVER_STRING_SYMNAME(char const *__restrict arg) { varname = arg; }


/* Define a driver parameter:
 * >> DRIVER_PARAM_FUNC("foo",arg) {
 * >>     debug_printf("driver initializer with foo=%s\n",arg);
 * >> }
 * >> DRIVER_FLAG_FUNC("bar") {
 * >>     debug_printf("driver initializer with bar\n");
 * >> } */
#define DRIVER_PARAM_FUNC(name,arg_name) \
        DEFINE_DRIVER_PARAM_FUNC(name,__DRIVER_PARAM_SYMNAME); \
        PRIVATE ATTR_FREETEXT ATTR_USED void KCALL \
        __DRIVER_PARAM_SYMNAME(char const *__restrict arg_name)
#define DRIVER_FLAG_FUNC(name) \
        DEFINE_DRIVER_FLAG_FUNC(name,__DRIVER_FLAG_SYMNAME); \
        PRIVATE ATTR_FREETEXT ATTR_USED void KCALL __DRIVER_FLAG_SYMNAME(void)



DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_DRIVER_H */
