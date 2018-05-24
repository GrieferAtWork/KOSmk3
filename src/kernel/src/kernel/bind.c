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
#ifndef GUARD_KERNEL_SRC_KERNEL_BIND_C
#define GUARD_KERNEL_SRC_KERNEL_BIND_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/malloc.h>
#include <kernel/bind.h>
#include <kernel/cache.h>
#include <kos/safecall.h>
#include <fs/driver.h>
#include <except.h>
#include <string.h>

DECL_BEGIN

/*  Keep track of driver bindings and invoke them
 *  before / after kernel bindings have executed.
 * (aka. execute them from a kernel binding registered in a `*.post'
 *  section for initializers, and `*.pre' section for finalizers). */

struct driver_function_binding {
    REF struct driver *fb_driver; /* [1..1][const] The driver responsible for this binding. */
    image_rva_t const *fb_start;  /* [1..fb_count][const] Pointer to a vector of image-relative pointers corresponding to callbacks. */
    size_t             fb_count;  /* [!0][const] The number of callbacks registered by the driver. */
};

struct binding_controller {
    atomic_rwlock_t                 bc_lock;   /* Lock for this binding controller. */
    size_t                          bc_count;  /* [lock(bc_lock)] The number of driver bindings. */
    struct driver_function_binding *bc_vector; /* [lock(bc_lock)][0..bc_count][owned] Vector of driver bindings. */
};

#define BINDING_CONTROLLER_INIT   { ATOMIC_RWLOCK_INIT, 0, NULL }


/* Table of kernel driver binding tables. */
PRIVATE struct binding_controller kernel_binding_table[] = {
    [DRIVER_TAG_BPERTASK_INIT          - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT,
    [DRIVER_TAG_BPERTASK_FINI          - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT,
    [DRIVER_TAG_BPERTASK_STARTUP       - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT,
    [DRIVER_TAG_BPERTASK_CLEANUP       - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT,
    [DRIVER_TAG_BPERTASK_CLONE         - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT,
    [DRIVER_TAG_BPERVM_INIT            - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT,
    [DRIVER_TAG_BPERVM_FINI            - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT,
    [DRIVER_TAG_BPERVM_CLONE           - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT,
    [DRIVER_TAG_BGLOBAL_CLEAR_CACHES   - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT,
    [DRIVER_TAG_BPERTASK_CLEAR_CACHES  - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT,
    [DRIVER_TAG_BPERVM_CLEAR_CACHES    - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT,
    [DRIVER_TAG_BGLOBAL_UNBIND_DRIVER  - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT,
    [DRIVER_TAG_BPERTASK_UNBIND_DRIVER - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT,
    [DRIVER_TAG_BPERVM_UNBIND_DRIVER   - DRIVER_TAG_BIND_START] = BINDING_CONTROLLER_INIT
};


#define INVOKE_END_LABEL  PP_CAT2(end_label_,__LINE__)

#define INVOKE_BIND_CONTROLLER(self,test,argc,argv,...) \
do{ size_t i,callback_i; \
    struct driver_function_binding bind; \
    atomic_rwlock_read(&(self)->bc_lock); \
    TRY { \
     for (i = 0; i < (self)->bc_count; ++i) { \
      uintptr_t load_addr; \
      bind = (self)->bc_vector[i]; \
      if (bind.fb_driver->d_app.a_flags & APPLICATION_FCLOSING) \
          continue; /* Skip unbound drivers. */ \
      load_addr = bind.fb_driver->d_app.a_loadaddr; \
      for (callback_i = 0; callback_i < bind.fb_count; ++callback_i) { \
       SAFECALL_KCALL_VOID_##argc(*(void(KCALL *)argv)(load_addr + bind.fb_start[callback_i]),##__VA_ARGS__); \
       if (test) \
           goto INVOKE_END_LABEL; \
      } \
     } \
INVOKE_END_LABEL: \
     ; \
    } FINALLY { \
     atomic_rwlock_endread(&(self)->bc_lock); \
    } \
}__WHILE0



/* Register driver invocation bindings. */
DEFINE_PERTASK_INIT(bind_pertask_init);
PRIVATE ATTR_USED void KCALL
bind_pertask_init(struct task *__restrict thread) {
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BPERTASK_INIT - DRIVER_TAG_BIND_START],0,1,(struct task *__restrict),thread);
}
DEFINE_PERTASK_FINI(bind_pertask_fini);
PRIVATE ATTR_USED void KCALL
bind_pertask_fini(struct task *__restrict thread) {
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BPERTASK_FINI - DRIVER_TAG_BIND_START],0,1,(struct task *__restrict),thread);
}
DEFINE_PERTASK_STARTUP(bind_pertask_startup);
PRIVATE ATTR_USED void KCALL bind_pertask_startup(u32 flags) {
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BPERTASK_STARTUP - DRIVER_TAG_BIND_START],0,1,(u32),flags);
}
DEFINE_PERTASK_CLEANUP(bind_pertask_cleanup);
PRIVATE ATTR_USED void KCALL bind_pertask_cleanup(void) {
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BPERTASK_CLEANUP - DRIVER_TAG_BIND_START],0,0,(void));
}
DEFINE_PERTASK_CLONE(bind_pertask_clone);
PRIVATE ATTR_USED void KCALL bind_pertask_clone(struct task *__restrict thread, u32 flags) {
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BPERTASK_CLONE - DRIVER_TAG_BIND_START],
                         0,2,(struct task *__restrict,u32),thread,flags);
}
DEFINE_PERVM_INIT(bind_pervm_init);
PRIVATE ATTR_USED void KCALL bind_pervm_init(struct vm *__restrict self) {
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BPERVM_INIT - DRIVER_TAG_BIND_START],
                         0,1,(struct vm *__restrict),self);
}
DEFINE_PERVM_FINI(bind_pervm_fini);
PRIVATE ATTR_USED void KCALL bind_pervm_fini(struct vm *__restrict self) {
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BPERVM_FINI - DRIVER_TAG_BIND_START],
                         0,1,(struct vm *__restrict),self);
}
DEFINE_PERVM_CLONE(bind_pervm_clone);
PRIVATE ATTR_USED void KCALL bind_pervm_clone(struct vm *__restrict self) {
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BPERVM_CLONE - DRIVER_TAG_BIND_START],
                         0,1,(struct vm *__restrict),self);
}
DEFINE_PERTASK_CACHE_CLEAR(bind_pertask_clear_caches);
PRIVATE ATTR_USED void KCALL bind_pertask_clear_caches(struct task *__restrict thread) {
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BPERTASK_CLEAR_CACHES - DRIVER_TAG_BIND_START],
                         kernel_cc_done(),1,(struct task *__restrict),thread);
}
DEFINE_PERVM_CACHE_CLEAR(bind_pervm_clear_caches);
PRIVATE ATTR_USED void KCALL bind_pervm_clear_caches(struct vm *__restrict self) {
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BPERVM_CLEAR_CACHES - DRIVER_TAG_BIND_START],
                         kernel_cc_done(),1,(struct vm *__restrict),self);
}
DEFINE_PERTASK_UNBIND_DRIVER(bind_pertask_unbind_driver);
PRIVATE ATTR_USED void KCALL
bind_pertask_unbind_driver(struct task *__restrict thread,
                           struct driver *__restrict d) {
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BPERTASK_UNBIND_DRIVER - DRIVER_TAG_BIND_START],
                         0,2,(struct task *__restrict,struct driver *__restrict),thread,d);
}
DEFINE_PERVM_UNBIND_DRIVER(bind_pervm_unbind_driver);
PRIVATE ATTR_USED void KCALL
bind_pervm_unbind_driver(struct vm *__restrict self,
                         struct driver *__restrict d) {
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BPERVM_UNBIND_DRIVER - DRIVER_TAG_BIND_START],
                         0,2,(struct vm *__restrict,struct driver *__restrict),self,d);
}



DEFINE_GLOBAL_UNBIND_DRIVER(bind_global_unbind_driver);
PRIVATE ATTR_USED void KCALL
bind_global_unbind_driver(struct driver *__restrict d) {
 struct binding_controller *iter;
 size_t binding_count = 0;
 /* Invoke driver bindings for the unbind-driver binding.
  * (Yes, that sounds weird, but it allows drivers to
  *  provide their own bindings for other drivers, alongside
  *  automatic unbinding when bound drivers are closed) */
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BPERVM_CLEAR_CACHES - DRIVER_TAG_BIND_START],
                         0,1,(struct driver *__restrict),d);
 /* Delete binding controller entries for `d' */
 for (iter = kernel_binding_table;
      iter < COMPILER_ENDOF(kernel_binding_table); ++iter) {
  size_t i; bool has_write_lock = false;
  atomic_rwlock_read(&iter->bc_lock);
table_again:
  i = 0;
table_continue:
  for (; i < iter->bc_count; ++i) {
   if (iter->bc_vector[i].fb_driver != d) continue;
   /* Delete this binding. */
   if (!has_write_lock) {
    has_write_lock = true;
    if (!atomic_rwlock_upgrade(&iter->bc_lock))
         goto table_again;
   }
   /* Adjust to pop this driver's binding entry. */
   --iter->bc_count;
   memmove(&iter->bc_vector[i],
           &iter->bc_vector[i+1],
           (iter->bc_count-i)*
            sizeof(struct driver_function_binding));
   goto table_continue;
  }
  if (has_write_lock)
       atomic_rwlock_endwrite(&iter->bc_lock);
  else atomic_rwlock_endread(&iter->bc_lock);
 }
 assertf(d->d_app.a_refcnt > binding_count,
         "The caller must also be holding a reference, so...");
 ATOMIC_FETCHSUB(d->d_app.a_refcnt,binding_count);
}


INTERN void KCALL
register_driver_binding(struct driver *__restrict d,
                        struct driver_tag const *__restrict tag) {
 struct binding_controller *self;
 size_t avail_entries,old_count;
 struct driver_function_binding *old_vector;
 struct driver_function_binding *new_vector;
 assert(tag->dt_name >= DRIVER_TAG_BMIN &&
        tag->dt_name <= DRIVER_TAG_BMAX);
 self = &kernel_binding_table[tag->dt_name-DRIVER_TAG_BIND_START];
again:
 atomic_rwlock_write(&self->bc_lock);
 avail_entries = (kmalloc_usable_size(self->bc_vector) /
                  sizeof(struct driver_function_binding));
 if (self->bc_count < avail_entries) {
  /* Make use of pre-allocated memory. */
  self->bc_vector[self->bc_count].fb_driver = d;
  self->bc_vector[self->bc_count].fb_start  = (image_rva_t *)(d->d_app.a_loadaddr + tag->dt_start);
  self->bc_vector[self->bc_count].fb_count  = tag->dt_count;
  driver_incref(d); /* The reference stored in `fb_driver' */
  ++self->bc_count;
  atomic_rwlock_endwrite(&self->bc_lock);
  /* Already done. */
  return;
 }
 /* Must allocate a new bindings vector to account for the driver. */
 old_count = self->bc_count;
 atomic_rwlock_endwrite(&self->bc_lock);
 /* NOTE: No need for buffering here. - Drivers aren't loaded all the
  *       time, so don't allocate more memory than we actually need to! */
 new_vector = (struct driver_function_binding *)kmalloc((old_count+1)*
                                                         sizeof(struct driver_function_binding),
                                                         GFP_SHARED);
 atomic_rwlock_write(&self->bc_lock);
 /* Race condition: another thread increased the vector size in the mean time. */
 if unlikely(self->bc_count > old_count) {
  atomic_rwlock_endwrite(&self->bc_lock);
  kfree(new_vector);
  goto again;
 }
 /* Copy all old callbacks into our new vector. */
 old_vector = self->bc_vector;
 memcpy(new_vector,old_vector,self->bc_count*
        sizeof(struct driver_function_binding));
 self->bc_vector = new_vector;
 /* Append an entry for the new binding. */
 new_vector[self->bc_count].fb_driver = d;
 new_vector[self->bc_count].fb_start  = (image_rva_t *)(d->d_app.a_loadaddr + tag->dt_start);
 new_vector[self->bc_count].fb_count  = tag->dt_count;
 driver_incref(d); /* The reference stored in `fb_driver' */
 ++self->bc_count;
 atomic_rwlock_endwrite(&self->bc_lock);
 /* Free the old vector. */
 kfree(old_vector);
}


DEFINE_GLOBAL_CACHE_CLEAR(clear_bind_caches);
PRIVATE ATTR_USED void KCALL clear_bind_caches(void) {
 size_t i;
 struct binding_controller *self;
 INVOKE_BIND_CONTROLLER(&kernel_binding_table[DRIVER_TAG_BGLOBAL_CLEAR_CACHES - DRIVER_TAG_BIND_START],
                         kernel_cc_done(),0,(void));
 for (i = 0; i < COMPILER_LENOF(kernel_binding_table); ++i) {
  self = &kernel_binding_table[i];
  atomic_rwlock_write(&self->bc_lock);
  if (!self->bc_count) {
   krealloc(self->bc_vector,self->bc_count*
            sizeof(struct driver_function_binding),
            GFP_SHARED|GFP_NOMOVE|GFP_NOTRIM);
  }
  atomic_rwlock_endwrite(&self->bc_lock);
 }
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_BIND_C */
