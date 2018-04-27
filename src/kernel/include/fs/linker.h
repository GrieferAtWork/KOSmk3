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
#ifndef GUARD_KERNEL_INCLUDE_FS_LINKER_H
#define GUARD_KERNEL_INCLUDE_FS_LINKER_H 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <hybrid/host.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/sections.h>
#include <kernel/vm.h>
#include <kos/bound.h>
#include <kos/dl.h>
#include <kos/types.h>

DECL_BEGIN

#ifdef __CC__
struct dl_section;
struct module_type;
struct module_data;
struct module_debug;
struct module;
struct application;
struct regular_node;
struct driver;
#endif /* __CC__ */

/* An image-relative, virtual address.
 * This address can be converted into an absolute address by adding
 * the load address `a_loadaddr' of an associated application instance. */
#ifdef __CC__
typedef VIRT uintptr_t image_rva_t;
#endif /* __CC__ */

#define MODULE_MAX_MAGIC   12 /* Max number of magic bytes for determining module types. */


#define MODULE_TYPE_FNORMAL      0x0000 /* Normal module. */
#define MODULE_TYPE_FPROXY       0x0001 /* Modules of this type act as a command-line proxy. */
#define MODULE_TYPE_FPAGEALIGNED 0x0002 /* The `m_newapp' operator requires a page-aligned `load_address'. */

#ifdef __CC__
struct module_patcher {
    struct module_patcher        *mp_root;     /* [1..1] Root module patcher. */
    struct module_patcher        *mp_prev;     /* [0..1] Underlying patch controller. */
    struct application           *mp_app;      /* [1..1] The application that is being patched. */
    size_t                        mp_requirec; /* Amount of required dependencies. */
    size_t                        mp_requirea; /* Allocated amount of required dependencies. */
    WEAK REF struct application **mp_requirev; /* [1..1][0..a_requirec][owned]
                                                * Vector dependencies loaded for this module.
                                                * Each is a weak reference in order to resolve
                                                * reference loops. */
    USER CHECKED char const      *mp_runpath;  /* [0..1][valid_if(mp_root == self)] Primary `:' / `;'-separated list of module library paths. */
    USER CHECKED char const      *mp_altpath;  /* [0..1] Secondary `:' / `;'-separated list of module library paths. */
    /* TODO: Cache module search paths. */
    u16                           mp_flags;    /* Path operation flags. */
    u16                           mp_apptype;  /* The type of application with which to load dependencies (One of `APPLICATION_TYPE_F*') */
    u16                           mp_appflags; /* Flags used for constructing dependencies (Set of `APPLICATION_F*') */
};

FUNDEF void KCALL patcher_fini(struct module_patcher *__restrict self);

/* Add `mod' as a required dependency of `self', loading it as a new dependency.
 * NOTE: The caller must be holding a lock on the effective VM. */
FUNDEF ATTR_RETNONNULL struct application *KCALL
patcher_require(struct module_patcher *__restrict self,
                struct module *__restrict mod);

/* Search `mp_runpath' and `mp_altpath' for a module `name',
 * then passing that module to `patcher_require()' in order
 * to load it as a dependency.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_NOT_FOUND: The named module could not be found. */
FUNDEF ATTR_RETNONNULL struct application *KCALL
patcher_require_string(struct module_patcher *__restrict self,
                       USER CHECKED char const *__restrict name,
                       size_t name_length);

/* Return the absolute address of symbol, given its name.
 * If the symbol doesn't exist, return NULL instead.
 * @param: search_current: When false, don't search the current
 *                         module when deep-bind is enabled,
 *                         but return NULL instead. */
FUNDEF void *KCALL
patcher_symaddr(struct module_patcher *__restrict self,
                USER CHECKED char const *__restrict name,
                u32 hash, bool search_current);

/* Return the hash for a given symbol name. */
FUNDEF u32 KCALL patcher_symhash(USER CHECKED char const *__restrict name);

#define patcher_setaltpath(x,path) \
   ((x)->mp_altpath = (path))
#endif /* __CC__ */





#ifdef __CC__
typedef void (KCALL *module_enumerator_t)(module_callback_t func, void *arg);

struct module_type {
    LIST_NODE(struct module_type)   m_types;  /* [lock(module_types.mt_lock)] Chain of registered module types. */
    u16                             m_flags;  /* Module type flags (Set of `MODULE_TYPE_F*') */
    u16                             m_magsz;  /* Amount of magic bits that should be detected before the module is loaded.
                                               * When ZERO(0), there is no magic and any file is considered applicable for
                                               * loading using this module type. */
    byte_t                          m_magic[MODULE_MAX_MAGIC]; /* [m_magsz] Magic module indicators. */
    struct driver                  *m_driver;

    /* [1..1] Initialize the given module.
     * This function must initialize the following members:
     *   - m_fixedbase (Optionally; pre-initialized to ZERO(0))
     *   - m_imagemin
     *   - m_imageend
     *   - m_entry
     *   - m_flags (Optionally; pre-initialized to `MODULE_FNORMAL')
     *   - m_data (Optionally; pre-initialized to `NULL')
     *   - m_got (Optionally; Allowed to be initialized during the first use of `m_loadapp'; pre-initialized to `0')
     * @assume(mod->m_type   == self);
     * @assume(mod->m_driver == self->m_driver);
     * @assume(mod->m_path   != NULL);
     * @assume(mod->m_fsloc  != NULL);
     * @assume(POST(mod->m_imageend > mod->m_imagemin));
     * @return: true:    The module was loaded.
     * @return: false:   The module cannot be loaded using this module type.
     * @throw: E_NO_DATA:        Same as returning `false' (To handle corrupt images with out-of-bound offsets)
     * @throw: E_DIVIDE_BY_ZERO: Same as returning `false' (To handle corrupt images with broken arithmetic)
     * @throw: E_OVERFLOW:       Same as returning `false' (To handle corrupt images with overflowing pointers)
     * @throw: E_INDEX_ERROR:    Same as returning `false' (To handle corrupt images with invalid pointers) */
    bool (KCALL *m_loadmodule)(struct module *__restrict mod);

    /* [0..1] Finalize the given given module (called during `module_destroy'). */
    /*ATTR_NOTHROW*/ void (KCALL *m_fini)(struct module *__restrict self);

    union PACKED {
        /* [1..1] Load the given application (map segments and load dependencies)
         * NOTE: If not already done by `m_loadmodule', this function
         *       must also initialize the following members.
         *   - self->mp_app->a_module->m_got  (originally initialized to ZERO(0))
         * @throw: E_NOT_EXECUTABLE: The module is corrupted or malicious.
         * @throw: E_SEGFAULT:       Same as `E_NOT_EXECUTABLE'
         * @throw: E_NO_DATA:        Same as `E_NOT_EXECUTABLE'
         * @throw: E_DIVIDE_BY_ZERO: Same as `E_NOT_EXECUTABLE'
         * @throw: E_OVERFLOW:       Same as `E_NOT_EXECUTABLE'
         * @throw: E_INDEX_ERROR:    Same as `E_NOT_EXECUTABLE' */
        void (KCALL *m_loadapp)(struct module_patcher *__restrict self);
        /* [1..1][valid_if(MODULE_TYPE_FPROXY)]
         *  Invoke a proxy-application module.
         *  WARNING: This operator must not be called from kernel-jobs,
         *           at it is allowed to (and actually required to) assume
         *           that the calling thread originates from user-space. */
        void (KCALL *m_newapp_exec)(int placeholder); /* TODO */
    };
    /* [1..1] Execute relocations for the given application.
     * @throw: E_NOT_EXECUTABLE: The module is corrupted or malicious.
     * @throw: E_SEGFAULT:       Same as `E_NOT_EXECUTABLE'
     * @throw: E_NO_DATA:        Same as `E_NOT_EXECUTABLE'
     * @throw: E_DIVIDE_BY_ZERO: Same as `E_NOT_EXECUTABLE'
     * @throw: E_OVERFLOW:       Same as `E_NOT_EXECUTABLE'
     * @throw: E_INDEX_ERROR:    Same as `E_NOT_EXECUTABLE' */
    void (KCALL *m_patchapp)(struct module_patcher *__restrict self);

    /* [0..1] Enumerate module initializer functions (in order of execution, first --> last). */
    void (KCALL *m_enuminit)(struct application *__restrict app,
                             module_enumerator_t func, void *arg);

    /* [0..1] Same as `m_enuminit', but used to enumerate module finalizers. */
    void (KCALL *m_enumfini)(struct application *__restrict app,
                             module_enumerator_t func, void *arg);
    /* [1..1] Return the symbol mapping to `name'.
     * @return: MODULE_SYMBOL_INVALID: The given symbol `name' could not be found. */
    struct dl_symbol (KCALL *m_symbol)(struct application *__restrict app,
                                       USER CHECKED char const *__restrict name, u32 hash);

    /* [0..1] Return information about a section, given its name.
     * @return. ms_size == 0: The given section `name' could be not found. */
    struct dl_section (KCALL *m_section)(struct application *__restrict app,
                                         USER CHECKED char const *__restrict name);
};
#endif /* __CC__ */


#ifdef __CC__
struct module {
    ATOMIC_DATA ref_t             m_refcnt;    /* Module reference counter. */
    struct module_type           *m_type;      /* [1..1][const] Module type and associated operations. */
    REF struct driver            *m_driver;    /* [1..1][const] The driver implementing the API for this module. */
    REF struct path              *m_path;      /* [0..1][const] The filesystem path of this module (if known). */
    REF struct regular_node      *m_fsloc;     /* [0..1][const] File-system location of this module (if known).
                                                *  Drivers loaded via the bios commandline, and the kernel itself do
                                                *  not have any explicit filesystem location associated with themself. */
    VIRT uintptr_t                m_fixedbase; /* [const] Fixed, virtual base address of this module; load address hint.. */
    image_rva_t                   m_imagemin;  /* [const] Offset from `m_fixedbase', the address of the lowest segment.
                                                *  Using this value, the actual required memory range of the image
                                                *  can be determined. */
    union PACKED {
        image_rva_t               m_imageend;  /* [const] The end address of the greatest section of the module. */
        image_rva_t               m_size;      /* [const] The module length (in bytes), offset from `m_fixedbase' */
    };
    image_rva_t                   m_entry;     /* [valid_if(MODULE_FENTRY)] Image entry point. */
    u16                           m_flags;     /* Module flags (Set of `MODULE_F*') */
    u16                           m_recent;    /* [INTERNAL] How often has this module been used recently. */
#if __SIZEOF_POINTER__ > 4
    u16                           m_pad[(sizeof(void *)-4)/2];
#endif

    struct module_data           *m_data;      /* [0..?] Module type-specific data (pre-initialized to NULL) */
    struct {
        struct dl_section         m_except;    /* The `.except' section (NOTE: You can assume that this section has `SHF_ALLOC' set if it exists).
                                                * NOTE: `ds_base' is actually an `image_rva_t' */
        struct dl_section         m_eh_frame;  /* The `.eh_frame' section (NOTE: You can assume that this section has `SHF_ALLOC' set if it exists).
                                                * NOTE: `ds_base' is actually an `image_rva_t' */
    }                             m_sect;      /* [valid_if(MODULE_FSECTLOADED)] Special section data. */
    struct module_debug          *m_debug;     /* [0..1][lock(WRITE_ONCE)] Module debug information (for addr2line) */
};

/* Destroy a previously allocated module. */
FUNDEF ATTR_NOTHROW void KCALL module_destroy(struct module *__restrict self);

/* Increment/decrement the reference counter of the given module `x' */
#define module_incref(x)  ATOMIC_FETCHINC((x)->m_refcnt)
#define module_decref(x) (ATOMIC_DECFETCH((x)->m_refcnt) || (module_destroy(x),0))

/* Allocate a new, NULL-initialized module with 1 reference. */
FUNDEF ATTR_MALLOC ATTR_RETNONNULL REF struct module *KCALL module_alloc(void);

/* Try to load the given module.
 * The caller must first initialize:
 *   - mod->m_type
 *   - mod->m_path
 *   - mod->m_fsloc
 *   - mod->m_driver (to `mod->m_type->m_driver')
 *   - mod->m_fixedbase (to `0')
 *   - mod->m_data (to `NULL')
 *   - mod->m_flags (to `MODULE_FNORMAL')
 * @return: true:  Successfully loaded the module.
 *                (The caller should cache it in `mod->m_fsloc->re_module.m_module')
 * @return: false: The module's type cannot be used to load the associated filesystem location.
 *           NOTE: This is also returned when the `m_loadmodule' operator throws
 *                 one of the errors listed in the operator documentation. */
FUNDEF bool KCALL module_load(struct module *__restrict mod);

/* Open the module stored under the given filesystem location.
 * If a module has been cached for that INode, return the cached
 * module. Otherwise, try to automatically determine the module's
 * file format and load it using the appropriate type.
 * NOTE: The caller is responsible to check for required permissions if
 *       they choose to do so by calling `inode_access(node,R_OK|X_OK)'.
 * @throw: E_NOT_EXECUTABLE: The given path doesn't describe an executable module. */
FUNDEF ATTR_RETNONNULL REF struct module *KCALL
module_open(struct regular_node *__restrict node,
            struct path *__restrict modpath);
/* Small wrapper around `module_open()'.
 * If the node pointed to by `modpath' isn't a
 * regular node, an `E_NOT_EXECUTABLE' is thrown. */
FUNDEF ATTR_RETNONNULL REF struct module *KCALL
module_openpath(struct path *__restrict modpath);

#ifndef CONFIG_NO_RECENT_MODULES
/* Cache the given module as recently-used.
 * NOTE: This function is automatically called by `application_load()'
 * The recently-used module cache is designed to keep metadata required
 * to load some module that is being used quite often loaded in memory.
 * That way, applications using the module can quickly be loaded again,
 * without the need of re-loading all associated meta-data.
 * This cache operates similar to the block-page cache used by block_device,
 * meaning that modules used more often than others will stay in cache longer
 * before being discarded new new modules that have been used even more.
 * XXX: This could be used to generate a sort-of heatmap of binaries
 *      executed more often than others in a running system... */
FUNDEF void KCALL module_recent(struct module *__restrict mod);

/* Clear the cache of recently-used modules. */
FUNDEF void KCALL module_clear_recent(void);
#endif /* !CONFIG_NO_RECENT_MODULES */

/* Load special exception data, ensuring that `MODULE_FSECTLOADED'
 * has been set for `self->a_module->m_flags'.
 * This function is used to lazily initialize exception handler data.
 * @return: true:  All sections required for exception handling have been, or already were loaded.
 * @return: false: At least one of the special sections required for exception handling is missing. */
FUNDEF bool KCALL module_loadexcept(struct application *__restrict self);


struct module_types_struct {
    atomic_rwlock_t               mt_lock;  /* Lock for `mt_types' */
    LIST_HEAD(struct module_type) mt_types; /* [0..1] Chain of known module types. */
};

/* Known module types. */
DATDEF struct module_types_struct module_types;

/* Register the given module type as known.
 * The type will automatically be removed once
 * the implementing driver gets unloaded. */
FUNDEF void KCALL register_module_type(struct module_type *__restrict self);

#define DEFINE_MODULE_TYPE(x) \
   DEFINE_DRIVER_INIT(_module_##x##_init); \
   INTERN ATTR_FREETEXT void KCALL _module_##x##_init(void) \
   { \
     register_module_type(&x); \
   }
#endif /* __CC__ */







#define APPLICATION_TYPE_FUSERAPP 0x0000 /* A regular, old user-space application. */
#define APPLICATION_TYPE_FDRIVER  0x0001 /* A driver module (`struct application' is actually `struct driver' from <fs/driver.h>) */
#define APPLICATION_TYPE_FKERNEL  0x7fff /* The kernel application itself. */

#ifdef __CC__
struct application {
    ATOMIC_DATA ref_t                 a_refcnt;   /* Application reference counter.
                                                   * NOTE: When non-ZERO, holds a reference to `a_weakcnt' */
    ATOMIC_DATA ref_t                 a_weakcnt;  /* Weak reference counter. */
    ATOMIC_DATA ref_t                 a_mapcnt;   /* The amount of times that this application
                                                   * is being mapped in the associated VM (when
                                                   * ZERO(0), the application has been unmapped).
                                                   * NOTE: When non-ZERO, holds a reference to `a_refcnt' */
    ATOMIC_DATA ref_t                 a_loadcnt;  /* Amount of times the application was opened by user-space. */
    VIRT uintptr_t                    a_loadaddr; /* [const] Application load address (in the associated VM)
                                                   * NOTE: Applications are private to VMs, so you
                                                   *       should only ever be able to get your hands
                                                   *       on one if its mapped within your own VM.
                                                   *       With that in mind, that means this is a
                                                   *       pointer into your own VM.
                                                   * NOTE: This value should be added to all pointers
                                                   *       stored in the module image. */
    struct bound_ptr                  a_bounds;   /* [const] Bounds of this application:
                                                   *  { a_loadaddr + a_module->m_imagemin,
                                                   *    a_loadaddr + a_module->m_imageend-1 } */
    REF struct module                *a_module;   /* [const][1..1] The loaded module. */
    size_t                            a_requirec; /* [lock(WRITE_ONCE)] Amount of required dependencies. */
    WEAK REF struct application     **a_requirev; /* [1..1][0..a_requirec][owned][lock(WRITE_ONCE)]
                                                   * Vector dependencies loaded for this module.
                                                   * Each is a weak reference in order to resolve
                                                   * reference loops. */
    u16                               a_type;     /* [const] Application type (One of `APPLICATION_TYPE_F*') */
    u16                               a_flags;    /* Application flags (Set of `APPLICATION_F*') */
    u16                               a_pad[(sizeof(void *)-2)/2];
};
#define APPLICATION_MAPMIN(x)    ((x)->a_bounds.b_min)
#define APPLICATION_MAPMAX(x)    ((x)->a_bounds.b_max)
#define APPLICATION_MAPBEGIN(x)  ((x)->a_bounds.b_min)
#define APPLICATION_MAPEND(x)    ((x)->a_bounds.b_max+1)
#define APPLICATION_MAPSIZE(x)  (((x)->a_bounds.b_max-(x)->a_bounds.b_min)+1)


/* Lookup a symbol / section within the given application. */
FUNDEF struct dl_symbol KCALL
application_dlsym(struct application *__restrict self,
                  USER CHECKED char const *__restrict name);
FUNDEF struct dl_symbol KCALL
application_dlsym3(struct application *__restrict self,
                   USER CHECKED char const *__restrict name, u32 hash);
FUNDEF struct dl_section KCALL
application_dlsect(struct application *__restrict self,
                   USER CHECKED char const *__restrict name);

/* Enumerate initializers/finalizers and set the associated `APPLICATION_FDID*' flag.
 * If the flag has already been set, or if the application doesn't
 * implement initializers/finalizers, return immediately. */
FUNDEF void KCALL application_enuminit(struct application *__restrict app, module_enumerator_t func, void *arg);
FUNDEF void KCALL application_enumfini(struct application *__restrict app, module_enumerator_t func, void *arg);

/* Load application initializers/finalizers for
 * execution by the given user-space CPU context. */
FUNDEF void KCALL
application_loaduserinit(struct application *__restrict app,
                         struct cpu_hostcontext_user *__restrict context);
FUNDEF void KCALL
application_loaduserfini(struct application *__restrict app,
                         struct cpu_hostcontext_user *__restrict context);


/* VM Node Notification callback for Application mappings. */
FUNDEF void *KCALL
application_notify(void *closure, unsigned int code,
                   vm_vpage_t addr, size_t num_pages,
                   void *arg);

/* Allocate a new, NULL-initialized application with 1 regular and 1 weak reference.
 * The caller must then initialize the following members:
 *   - a_module
 *   - a_loadaddr
 *   - a_type (Pre-initialized to `type')
 *   - a_flags (Pre-initialized to `APPLICATION_FNORMAL')
 * @throw: E_BADALLOC: Not enough available memory. */
FUNDEF ATTR_MALLOC ATTR_RETNONNULL
REF struct application *KCALL application_alloc(u16 type);

/* Wrapper around `application_load' and `application_patch'.
 * @param: flags: Set of `DL_OPEN_F*' */
FUNDEF void KCALL
application_loadroot(struct application *__restrict self, u16 flags,
                     USER CHECKED char const *runpath);

/* Automatically find a free location for `self' or ensure that its
 * fixed address isn't already in use while keeping a lock on the
 * effective VM.
 * Additionally, check that `APPLICATION_TYPE_FDRIVER' applications
 * end up in kernel-space, and `APPLICATION_TYPE_FUSERAPP' end up in
 * user-space before mapping the application at its proper location.
 * NOTE: This function will automatically initialize `a_loadaddr'
 * NOTE: The caller must be holding a lock to the effective VM.
 * NOTE: Unless `DL_OPEN_FNOASLR' is set, or the application's
 *       module isn't relocatable, a random shift is added to where
 *       the application is actually loaded at.
 * @throw: E_NOT_EXECUTABLE: The fixed address range is located outside of
 *                           the application's purpose, as described by its
 *                           type. (e.g. `APPLICATION_TYPE_FDRIVER' in user-space,
 *                           or `APPLICATION_TYPE_FUSERAPP' in kernel-space)
 * @throw: E_BADALLOC: No suitable free range found for a dynamically relocatable application.
 * @throw: E_BADALLOC: The address range required by a fixed-address application is already in use. */
FUNDEF void KCALL application_load(struct module_patcher *__restrict self);
/* Execute relocations (Same exceptions and restrictions as `application_load()').
 * @param: context: When non-NULL, update this context to invoke initializers before returning. */
FUNDEF void KCALL application_patch(struct module_patcher *__restrict self);

/* Destroy a previously allocated application. */
FUNDEF ATTR_NOTHROW void KCALL
application_destroy(struct application *__restrict self);
FUNDEF ATTR_NOTHROW void KCALL
application_weak_destroy(struct application *__restrict self);

/* Increment/decrement the reference counter of the given application `x' */
#define application_tryincref(x)    ATOMIC_INCIFNONZERO((x)->a_refcnt)
#define application_incref(x)       ATOMIC_FETCHINC((x)->a_refcnt)
#define application_decref(x)      (ATOMIC_DECFETCH((x)->a_refcnt) || (application_destroy(x),0))
#define application_weak_incref(x)  ATOMIC_FETCHINC((x)->a_weakcnt)
#define application_weak_decref(x) (ATOMIC_DECFETCH((x)->a_weakcnt) || (application_weak_destroy(x),0))



struct vmapps {
    ATOMIC_DATA ref_t            va_share;   /* [lock(THIS_VM->vm_lock)] Amount of VMs sharing this vmapps descriptor. */
    /* NOTE: Using `va_lock', dead applications may be removed from
     *       the vector below, however, new application may not be
     *       added, and ones still alive may not be deleted when
     *       `va_share' is greater than ONE(1). */
    atomic_rwlock_t              va_lock;    /* Lock for this VM APPS descriptor. */
    size_t                       va_count;   /* [lock(va_lock)] Amount of apps found in the vector below. */
    size_t                       va_alloc;   /* [lock(va_lock)] Allocated number of apps. */
    WEAK REF struct application *va_apps[1]; /* [1..1][va_count][lock(va_lock)]
                                              * Vector of VM apps, sorted in order or visibility, which is equal to the following:
                                              * >> function enum_global_apps(app) {
                                              * >>     if (app is none) return;
                                              * >>     yield app;
                                              * >>     for (local x: app.a_requirev) {
                                              * >>         yield enum_global_apps(x)...;
                                              * >>     }
                                              * >> }
                                              * >> enum_global_apps(app_vmglobals);
                                              * Additional applications loaded after `exec()' are inserted at
                                              * the end of this chain, meaning they have a lower visibility
                                              * than the core set of applications and libraries.
                                              * After a fork() (or CLONE_VM), the old and new VMs will be
                                              * sharing this vector copy-on-write-style (`va_share > 1') */
};

/* [0..1][lock(THIS_VM->vm_lock)] Per-vm chain application vector. */
DATDEF ATTR_PERVM struct vmapps *vm_apps;

/* Append the given application at the end of `effective_vm's `vm_apps' vector.
 * NOTE: The caller must be holding a lock on `effective_vm->vm_lock'
 * NOTE: This function will automatically perform copy-on-write! */
FUNDEF void KCALL vm_apps_append(struct vm *__restrict effective_vm,
                                 struct application *__restrict app);

/* Return the primary application (the first application
 * in the `vm_apps' vector with a valid reference counter).
 * This is the application that should be listed as `/proc/self/exe',
 * as well as the one used during implicit handle casts.
 * @return: NULL: The VM has no primary application. */
FUNDEF REF struct application *KCALL
vm_apps_primary(struct vm *__restrict effective_vm);


/* Lookup a symbol in the current set of VM applications.
 * NOTE: These functions will automatically deal with weak symbol visibility,
 *       where the first encountered non-weak match is returned in favor of
 *       the first encountered match. */
FUNDEF struct dl_symbol KCALL vm_apps_dlsym(USER CHECKED char const *__restrict name);
FUNDEF struct dl_symbol KCALL vm_apps_dlsym2(USER CHECKED char const *__restrict name, u32 hash);

/* Queue initializers/finalizers for all loaded modules
 * that haven't been initialized/finalized, yet.
 * NOTE: `vm_apps_finiall()' will not queue finalizers for
 *        modules that were never initialized to begin with. */
FUNDEF void KCALL vm_apps_initall(struct cpu_hostcontext_user *__restrict context);
FUNDEF void KCALL vm_apps_finiall(struct cpu_hostcontext_user *__restrict context);

/* Lookup the application associated with the given user-space application handle.
 * NOTE: When `user_handle' is NULL, return the primary application. */
FUNDEF ATTR_RETNONNULL REF struct application *KCALL vm_getapp(USER UNCHECKED void *user_handle);

#endif /* __CC__ */


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_LINKER_H */
