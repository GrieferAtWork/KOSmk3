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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_BIND_H
#define GUARD_KERNEL_INCLUDE_KERNEL_BIND_H 1

#include <hybrid/compiler.h>
#include <fs/driver.h>

/* Misc. global bindings. */
DECL_BEGIN

/* To deal with initialization order of driver bindings,
 * the kernel core needs much more precise binding ordering. */
#ifdef CONFIG_BUILDING_KERNEL_CORE
#define __EMIT_ENUMERATE_KERNEL_BINDINGS_SECTION3(SECTION,name) \
          SECTION(name.pre) SECTION(name) SECTION(name.post)
#else
#define __EMIT_ENUMERATE_KERNEL_BINDINGS_SECTION3(SECTION,name) SECTION(name)
#endif

/* @param: BINDING(name,sections...): Callback invoked for every kernel binding
 * @param: SECTION(name):             Wrapper for section names.
 * This macro is invoked by linker scripts to generate callback vectors
 * for various kernel bindings, both within the kernel core, as well as
 * within driver modules. */
#define ENUMERATE_KERNEL_BINDINGS(BINDING,SECTION) \
    BINDING(pertask_init,         SECTION(.rodata.callback.pertask.init)) \
    BINDING(pertask_fini,         SECTION(.rodata.callback.pertask.fini)) \
    BINDING(pertask_startup,      SECTION(.rodata.callback.pertask.startup)) \
    BINDING(pertask_cleanup,      SECTION(.rodata.callback.pertask.cleanup.onexit) \
                                  SECTION(.rodata.callback.pertask.cleanup)) \
    BINDING(pertask_clone,        SECTION(.rodata.callback.pertask.clone)) \
    BINDING(pervm_init,           SECTION(.rodata.callback.pervm.init)) \
    BINDING(pervm_fini,           SECTION(.rodata.callback.pervm.fini)) \
    BINDING(pervm_clone,          SECTION(.rodata.callback.pervm.clone)) \
    BINDING(pervm_unmapuser,      SECTION(.rodata.callback.pervm.unmapuser)) \
    BINDING(global_clear_caches,  SECTION(.rodata.callback.global.clear_caches)) \
    BINDING(pertask_clear_caches, SECTION(.rodata.callback.pertask.clear_caches)) \
    BINDING(pervm_clear_caches,   SECTION(.rodata.callback.pervm.clear_caches)) \
    BINDING(global_unbind_driver, SECTION(.rodata.callback.global.unbind_driver)) \
    BINDING(pertask_unbind_driver,SECTION(.rodata.callback.pertask.unbind_driver)) \
    BINDING(pervm_unbind_driver,  SECTION(.rodata.callback.pervm.unbind_driver)) \
/**/

/* Driver tags for callback bindings.
 * NOTES:
 *  - The ordering of driver bindings between different drivers is undefined.
 *  - Driver finalizer bindings are executed before kernel finalizer binders are.
 *    e.g.: DRIVER:`DEFINE_PERTASK_FINI()' is executed before
 *          KERNEL:`DEFINE_PERTASK_FINI()'
 *  - Other driver bindings are executed after kernel binders were.
 *    e.g.: DRIVER:`DEFINE_PERTASK_INIT()' is executed after
 *          KERNEL:`DEFINE_PERTASK_INIT()' */
#define DRIVER_TAG_BPERTASK_INIT          (DRIVER_TAG_BIND_START+0)  /* `DEFINE_PERTASK_INIT()' */
#define DRIVER_TAG_BPERTASK_FINI          (DRIVER_TAG_BIND_START+1)  /* `DEFINE_PERTASK_FINI()' */
#define DRIVER_TAG_BPERTASK_STARTUP       (DRIVER_TAG_BIND_START+2)  /* `DEFINE_PERTASK_STARTUP()' */
#define DRIVER_TAG_BPERTASK_CLEANUP       (DRIVER_TAG_BIND_START+3)  /* `DEFINE_PERTASK_CLEANUP()' */
#define DRIVER_TAG_BPERTASK_CLONE         (DRIVER_TAG_BIND_START+4)  /* `DEFINE_PERTASK_CLONE()' */
#define DRIVER_TAG_BPERVM_INIT            (DRIVER_TAG_BIND_START+5)  /* `DEFINE_PERVM_INIT()' */
#define DRIVER_TAG_BPERVM_FINI            (DRIVER_TAG_BIND_START+6)  /* `DEFINE_PERVM_FINI()' */
#define DRIVER_TAG_BPERVM_CLONE           (DRIVER_TAG_BIND_START+7)  /* `DEFINE_PERVM_CLONE()' */
#define DRIVER_TAG_BPERVM_UNMAPUSER       (DRIVER_TAG_BIND_START+8)  /* `DEFINE_PERVM_UNMAPUSER()' */
#define DRIVER_TAG_BGLOBAL_CLEAR_CACHES   (DRIVER_TAG_BIND_START+9)  /* `DEFINE_GLOBAL_CACHE_CLEAR()' */
#define DRIVER_TAG_BPERTASK_CLEAR_CACHES  (DRIVER_TAG_BIND_START+10) /* `DEFINE_PERTASK_CACHE_CLEAR()' */
#define DRIVER_TAG_BPERVM_CLEAR_CACHES    (DRIVER_TAG_BIND_START+11) /* `DEFINE_PERVM_CACHE_CLEAR()' */
#define DRIVER_TAG_BGLOBAL_UNBIND_DRIVER  (DRIVER_TAG_BIND_START+12) /* `DEFINE_GLOBAL_UNBIND_DRIVER()' */
#define DRIVER_TAG_BPERTASK_UNBIND_DRIVER (DRIVER_TAG_BIND_START+13) /* `DEFINE_PERTASK_UNBIND_DRIVER()' */
#define DRIVER_TAG_BPERVM_UNBIND_DRIVER   (DRIVER_TAG_BIND_START+14) /* `DEFINE_PERVM_UNBIND_DRIVER()' */

#define DEFINE_DRIVER_CALLBACK_TAG(tag,name) \
        DEFINE_DRIVER_TAG(tag,DRIVER_TAG_FOPTIONAL,name##_start,name##_count)


/* >> void KCALL my_func(struct task *__restrict thread);
 * Register a finalizer that should be invoked during the initializatino of a task. */
#define DEFINE_PERTASK_INIT(x) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BPERTASK_INIT,pertask_init) \
        DEFINE_ABS_CALLBACK(".rodata.callback.pertask.init",x)

/* >> ATTR_NOTHROW void KCALL my_func(struct task *__restrict thread);
 * Register a finalizer that should be invoked during the destruction of a task. */
#define DEFINE_PERTASK_FINI(x) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BPERTASK_FINI,pertask_fini) \
        DEFINE_ABS_CALLBACK(".rodata.callback.pertask.fini",x)

/* >> void KCALL my_func(u32 flags);
 * Register a callback that should be invoked in the context of a new
 * thread before that thread actually starts executing its payload:
 * @param: flags: Set of `CLONE_*' from `<bits/sched.h>' */
#define DEFINE_PERTASK_STARTUP(x) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BPERTASK_STARTUP,pertask_startup) \
        DEFINE_ABS_CALLBACK(".rodata.callback.pertask.startup",x)

/* >> ATTR_NOTHROW void KCALL my_func(void);
 * Register a callback that should be invoked in the context of the affected
 * thread just prior to it exiting (after `TASK_STATE_FTERMINATING' is set,
 * but before `TASK_STATE_FTERMINATED' is). */
#define DEFINE_PERTASK_CLEANUP(x) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BPERTASK_CLEANUP,pertask_cleanup) \
        DEFINE_ABS_CALLBACK(".rodata.callback.pertask.cleanup",x)

/* >> void KCALL my_func(struct task *__restrict new_thread, u32 flags);
 * @param: flags: Set of `CLONE_*' from `<bits/sched.h>'
 * Register a callback that should be invoked
 * when the calling thread should be cloned: */
#define DEFINE_PERTASK_CLONE(x) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BPERTASK_CLONE,pertask_clone) \
        DEFINE_ABS_CALLBACK(".rodata.callback.pertask.clone",x)

/* >> void KCALL my_func(struct vm *__restrict vm);
 * Register a callback as a finalizer that should be invoked during the destruction of a VM. */
#define DEFINE_PERVM_INIT(x) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BPERVM_INIT,pervm_init) \
        DEFINE_ABS_CALLBACK(".rodata.callback.pervm.init",x)

/* >> ATTR_NOTHROW void KCALL my_func(struct vm *__restrict vm);
 * Register a callback as a finalizer that should be invoked during the destruction of a VM. */
#define DEFINE_PERVM_FINI(x) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BPERVM_FINI,pervm_fini) \
        DEFINE_ABS_CALLBACK(".rodata.callback.pervm.fini",x)

/* >> void KCALL my_func(struct vm *__restrict new_vm);
 * Register a callback that should be invoked when the calling VM should be cloned. */
#define DEFINE_PERVM_CLONE(x) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BPERVM_CLONE,pervm_clone) \
        DEFINE_ABS_CALLBACK(".rodata.callback.pervm.clone",x)


/* >> void KCALL clear_my_cache(void);
 * Called to clear global caches.
 * Clear callbacks may occasionally invoke `kernel_cc_continue()'
 * to check if they should stop clearing caches.
 * NOTE: The clearing of caches is synchronized using an exclusive global
 *       lock, meaning that cache clear callbacks only need synchronize
 *       themself with other callbacks related to said cache. */
#define DEFINE_GLOBAL_CACHE_CLEAR(func) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BGLOBAL_CLEAR_CACHES,global_clear_caches) \
        DEFINE_CALLBACK(".rodata.callback.global.clear_caches",func)

/* >> void KCALL clear_my_cache(struct task *__restrict thread);
 * Same as `DEFINE_GLOBAL_CACHE_CLEAR()', but called once for every running thread. */
#define DEFINE_PERTASK_CACHE_CLEAR(func) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BPERTASK_CLEAR_CACHES,pertask_clear_caches) \
        DEFINE_CALLBACK(".rodata.callback.pertask.clear_caches",func)

/* >> void KCALL clear_my_cache(struct vm *__restrict self);
 * Same as `DEFINE_GLOBAL_CACHE_CLEAR()', but called once for every existing VM. */
#define DEFINE_PERVM_CACHE_CLEAR(func) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BPERVM_CLEAR_CACHES,pervm_clear_caches) \
        DEFINE_CALLBACK(".rodata.callback.pervm.clear_caches",func)


/* >> void KCALL unbind_driver(struct driver *__restrict d);
 * Called to delete global bindings that may have been registered by the given driver `d'.
 * This includes filesystem types, devices, module types, and much more. */
#define DEFINE_GLOBAL_UNBIND_DRIVER(func) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BGLOBAL_UNBIND_DRIVER,global_unbind_driver) \
        DEFINE_CALLBACK(".rodata.callback.global.unbind_driver",func)

/* >> void KCALL unbind_driver(struct task *__restrict thread, struct driver *__restrict d);
 * Same as `DEFINE_GLOBAL_UNBIND_DRIVER()', but called once for every running thread. */
#define DEFINE_PERTASK_UNBIND_DRIVER(func) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BPERTASK_UNBIND_DRIVER,pertask_unbind_driver) \
        DEFINE_CALLBACK(".rodata.callback.pertask.unbind_driver",func)

/* >> void KCALL unbind_driver(struct vm *__restrict self, struct driver *__restrict d);
 * Same as `DEFINE_GLOBAL_UNBIND_DRIVER()', but called once for every existing VM. */
#define DEFINE_PERVM_UNBIND_DRIVER(func) \
        DEFINE_DRIVER_CALLBACK_TAG(DRIVER_TAG_BPERVM_UNBIND_DRIVER,pervm_unbind_driver) \
        DEFINE_CALLBACK(".rodata.callback.pervm.unbind_driver",func)



DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_BIND_H */
