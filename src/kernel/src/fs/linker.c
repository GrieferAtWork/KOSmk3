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
#ifndef GUARD_KERNEL_SRC_FS_LINKER_C
#define GUARD_KERNEL_SRC_FS_LINKER_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <hybrid/debug.h>
#include <hybrid/align.h>
#include <hybrid/list/list.h>
#include <kernel/heap.h>
#include <kernel/syscall.h>
#include <kernel/vm.h>
#include <kernel/malloc.h>
#include <kernel/sections.h>
#include <kernel/user.h>
#include <kernel/bind.h>
#include <fs/node.h>
#include <fs/path.h>
#include <fs/linker.h>
#include <fs/driver.h>
#include <unwind/debug_line.h>
#include <unwind/eh_frame.h>
#include <unwind/linker.h>
#include <string.h>
#include <except.h>
#include <stdlib.h>
#include <elf.h>

DECL_BEGIN

INTERN void KCALL
vmapps_destroy(struct vmapps *__restrict self) {
 size_t i;
 for (i = 0; i < self->va_count; ++i)
     application_weak_decref(self->va_apps[i]);
 kfree(self);
}


/* [0..1][lock(THIS_VM->vm_lock)] Per-vm chain application vector. */
PUBLIC ATTR_PERVM struct vmapps *vm_apps = NULL;

DEFINE_PERVM_FINI(vm_apps_fini);
INTERN void KCALL vm_apps_fini(struct vm *__restrict self) {
 struct vmapps *apps = FORVM(self,vm_apps);
 assert(!apps || apps->va_share != 0);
 if (apps && ATOMIC_DECFETCH(apps->va_share) == 0)
     vmapps_destroy(apps);
}

DEFINE_PERVM_CLONE(vm_apps_clone);
INTERN void KCALL vm_apps_clone(struct vm *__restrict new_vm)     {
 struct vmapps *apps = PERVM(vm_apps);
 if (apps) {
  /* Share the applications vector. */
  assert(apps->va_share != 0);
  ATOMIC_FETCHINC(apps->va_share);
  FORVM(new_vm,vm_apps) = apps;
 }
}



/* Return the primary application (the first application
 * in the `vm_apps' vector with a valid reference counter).
 * This is the application that should be listed as `/proc/self/exe',
 * as well as the one used during implicit handle casts. */
PUBLIC REF struct application *KCALL
vm_apps_primary(struct vm *__restrict effective_vm) {
 struct vm *EXCEPT_VAR xeffective_vm = effective_vm;
 REF struct application *COMPILER_IGNORE_UNINITIALIZED(result);
 struct vmapps *apps;
again:
 vm_acquire_read(effective_vm);
 TRY {
  apps = FORVM(effective_vm,vm_apps);
  if (apps) {
   size_t i;
recheck_apps:
   atomic_rwlock_read(&apps->va_lock);
   for (i = 0; i < apps->va_count; ++i) {
    result = apps->va_apps[i];
    if (ATOMIC_INCIFNONZERO(result->a_refcnt))
        break; /* Found it! */
    /* Stale application (get rid of it...) */
    if (!atomic_rwlock_tryupgrade(&apps->va_lock)) {
     /* Let someone else deal with this one. */
     result = NULL;
     continue;
    }
    /* Get rid of this one ourself. */
    --apps->va_count;
    memmove(&apps->va_apps[i],&apps->va_apps[i+1],
            (apps->va_count-i)*
             sizeof(WEAK REF struct application *));
    atomic_rwlock_endwrite(&apps->va_lock);
    /* Drop the weak reference that was stored in the vector. */
    application_weak_decref(result);
    result = NULL;
    /* If this was the last application, delete the VM_APPS vector. */
    if (!apps->va_count) {
     if (ATOMIC_DECFETCH(apps->va_share) == 0)
         vmapps_destroy(apps);
#ifdef CONFIG_VM_USE_RWLOCK
     vm_acquire(effective_vm);
     FORVM(effective_vm,vm_apps) = NULL;
     vm_release(effective_vm);
#else
     FORVM(effective_vm,vm_apps) = NULL;
#endif
     break; /* No applications... */
    }
    /* Start over the search with the stale application now gone. */
    goto recheck_apps;
   }
   atomic_rwlock_endread(&apps->va_lock);
  }
 } FINALLY {
  if (vm_release_read(xeffective_vm))
      goto again;
 }
 return result;
}


PUBLIC struct dl_symbol KCALL
vm_apps_dlsym(USER CHECKED char const *__restrict name) {
 return vm_apps_dlsym2(name,patcher_symhash(name));
}
PUBLIC struct dl_symbol KCALL
vm_apps_dlsym2(USER CHECKED char const *__restrict name, u32 hash) {
 struct dl_symbol result,new_result;
 struct application *EXCEPT_VAR app; struct vmapps *apps;
 struct vm *EXCEPT_VAR myvm = THIS_VM;
again:
 vm_acquire_read(myvm);
 TRY {
recheck_apps:
  result.ds_type = MODULE_SYMBOL_INVALID;
  apps = FORVM(myvm,vm_apps);
  if (apps) {
   size_t i;
   atomic_rwlock_read(&apps->va_lock);
   for (i = 0; i < apps->va_count; ++i) {
    app = apps->va_apps[i];
    if (ATOMIC_INCIFNONZERO(app->a_refcnt)) {
     atomic_rwlock_endread(&apps->va_lock);
     TRY {
      /* Lookup a symbol in this application. */
      new_result = (*app->a_module->m_type->m_symbol)(app,name,hash);
     } FINALLY {
      application_decref(app);
     }
     if (new_result.ds_type < result.ds_type) {
      result = new_result;
      if (result.ds_type == MODULE_SYMBOL_NORMAL)
          goto got_result;
     }
     atomic_rwlock_read(&apps->va_lock);
     continue;
    }
    /* Stale application (get rid of it...) */
    if (!atomic_rwlock_tryupgrade(&apps->va_lock))
         continue; /* Let someone else deal with this one. */
    /* Get rid of this one ourself. */
    --apps->va_count;
    memmove(&apps->va_apps[i],&apps->va_apps[i+1],
            (apps->va_count-i)*
             sizeof(WEAK REF struct application *));
    atomic_rwlock_endwrite(&apps->va_lock);
    /* Drop the weak reference that was stored in the vector. */
    application_weak_decref(app);
    /* If this was the last application, delete the VM_APPS vector. */
    if (!apps->va_count) {
     if (ATOMIC_DECFETCH(apps->va_share) == 0)
         vmapps_destroy(apps);
#ifdef CONFIG_VM_USE_RWLOCK
     vm_acquire(myvm);
     FORVM(myvm,vm_apps) = NULL;
     vm_release(myvm);
#else
     FORVM(myvm,vm_apps) = NULL;
#endif
     goto got_result; /* No applications... */
    }
    /* Start over the search with the stale application now gone. */
    goto recheck_apps;
   }
   atomic_rwlock_endread(&apps->va_lock);
got_result:;
  }
 } FINALLY {
  if (vm_release_read(myvm))
      goto again;
 }
 return result;
}


/* Append the given application at the end of `effective_vm's `vm_apps' vector.
 * NOTE: The caller must be holding a lock on `effective_vm->vm_lock'
 * NOTE: This function will automatically perform copy-on-write! */
PUBLIC void KCALL
vm_apps_append(struct vm *__restrict effective_vm,
               struct application *__restrict app) {
 struct vmapps *EXCEPT_VAR apps;
 assert(vm_holding(effective_vm));
again:
 apps = FORVM(effective_vm,vm_apps);
 if (!apps) {
  /* Allocate an initial buffer for 2 apps (`some_app' + `libc.so') */
  apps = (struct vmapps *)kmalloc(offsetof(struct vmapps,va_apps)+
                                  2*sizeof(WEAK REF struct application *),
                                  GFP_SHARED);
  COMPILER_READ_BARRIER();
  /* Who knowns what kmalloc() did before we got here.
   * Better re-check that no applications have been allocated, yet. */
  if unlikely(FORVM(effective_vm,vm_apps) != NULL) {
   kfree(apps);
   goto again;
  }
  /* NOTE: Since the caller is holding a lock on the VM, we know that
   *       its `vm_apps' field won't arbitrarily be changed by another
   *       thread. The reason for this check right now is to make sure
   *       that `kmalloc()' didn't modify it... */
  apps->va_share = 1;
  atomic_rwlock_init(&apps->va_lock);
  apps->va_count = 1;
  apps->va_alloc  = kmalloc_usable_size(apps);
  apps->va_alloc -= offsetof(struct vmapps,va_apps);
  apps->va_alloc /= sizeof(WEAK REF struct application *);
  assert(apps->va_alloc >= 2);
  apps->va_apps[0] = app;
  application_weak_incref(app);
  assert(!FORVM(effective_vm,vm_apps));
  FORVM(effective_vm,vm_apps) = apps;
  return;
 }
 assert(apps->va_count <= apps->va_alloc);
 atomic_rwlock_write(&apps->va_lock);
 if (apps->va_share != 1 ||
     apps->va_count == apps->va_alloc) {
  /* Unshare the applications vector. */
  struct vmapps *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(new_apps);
  size_t EXCEPT_VAR count; size_t i,new_alloc;
  count = apps->va_count;
  new_alloc = count+1;
  if (apps->va_share == 1)
      new_alloc = count*2;
  atomic_rwlock_endwrite(&apps->va_lock);
  TRY {
   new_apps = (struct vmapps *)kmalloc(offsetof(struct vmapps,va_apps)+
                                      (new_alloc*sizeof(WEAK REF struct application *)),
                                       GFP_SHARED);
  } CATCH_HANDLED (E_BADALLOC) {
   /* Try again with a minimal buffer increment. */
   new_alloc = count+1;
   new_apps = (struct vmapps *)kmalloc(offsetof(struct vmapps,va_apps)+
                                      (new_alloc*sizeof(WEAK REF struct application *)),
                                       GFP_SHARED);
  }
  /* Re-acquire a read lock. */
  atomic_rwlock_read(&apps->va_lock);
  if unlikely(apps->va_count > count) {
   atomic_rwlock_endread(&apps->va_lock);
   kfree(new_apps);
   goto again;
  }
  count = apps->va_count;
  new_apps->va_share  = 1;
  atomic_rwlock_init_write(&new_apps->va_lock);
  new_apps->va_alloc  = kmalloc_usable_size(new_apps);
  new_apps->va_alloc -= offsetof(struct vmapps,va_apps);
  new_apps->va_alloc /= sizeof(WEAK REF struct application *);
  assert(new_apps->va_alloc >= new_alloc);
  new_apps->va_count  = 0;
  /* Copy references to all real applications. */
  for (i = 0; i < count; ++i) {
   struct application *temp = apps->va_apps[i];
   if (temp->a_refcnt == 0) continue;
   application_weak_incref(temp);
   new_apps->va_apps[new_apps->va_count] = temp;
   ++new_apps->va_count;
  }
  atomic_rwlock_endread(&apps->va_lock);
  /* Set the new apps as active. */
  FORVM(effective_vm,vm_apps) = new_apps;
  /* Drop a reference from the old applications vector. */
  if (ATOMIC_DECFETCH(apps->va_share) == 0)
      vmapps_destroy(apps);
  apps = new_apps;
 }
 /* Append the given application. */
 apps->va_apps[apps->va_count++] = app;
 application_weak_incref(app);
 atomic_rwlock_endwrite(&apps->va_lock);
}



/* Destroy a previously allocated module. */
PUBLIC ATTR_NOTHROW void KCALL
module_destroy(struct module *__restrict self) {
 if (self->m_type && self->m_type->m_fini)
     SAFECALL_KCALL_VOID_1(*self->m_type->m_fini,self);
 if (self->m_fsloc) {
  /* Clear the filesystem cache entry for this module. */
  atomic_rwlock_write(&self->m_fsloc->re_module.m_lock);
  if (self->m_fsloc->re_module.m_module == self)
      self->m_fsloc->re_module.m_module = NULL;
  atomic_rwlock_endwrite(&self->m_fsloc->re_module.m_lock);
  inode_decref((struct inode *)self->m_fsloc);
 }
 if (self->m_debug)
     module_debug_delete(self->m_debug);
 if (self->m_driver)
     driver_decref(self->m_driver);
 if (self->m_path)
     path_decref(self->m_path);
 except_cache_fini(&self->m_exc_cache);
 fde_cache_fini(&self->m_fde_cache);
 kfree(self);
}

PUBLIC ATTR_MALLOC ATTR_RETNONNULL
REF struct module *KCALL module_alloc(void) {
 REF struct module *result;
 result = (REF struct module *)kmalloc(sizeof(struct module),
                                       GFP_SHARED|GFP_CALLOC);
 result->m_refcnt = 1;
 atomic_rwlock_cinit(&result->m_fde_cache.fc_lock);
 return result;
}


PUBLIC bool KCALL
module_load(struct module *__restrict mod) {
 bool COMPILER_IGNORE_UNINITIALIZED(result);
 assert(mod->m_refcnt == 1);
 assert(mod->m_type);
 assert(mod->m_driver);
 assert(mod->m_type->m_driver == mod->m_driver);
 assert(mod->m_path);
 assert(mod->m_fsloc);
 assert(mod->m_fixedbase == 0);
 assert(mod->m_flags == MODULE_FNORMAL);
 assert(mod->m_data == NULL);
#ifndef NDEBUG
 /* Debug-initialize members that must be initialized by `m_loadmodule()' */
 memset(&mod->m_imagemin,0xcc,sizeof(image_rva_t));
 memset(&mod->m_imageend,0xcc,sizeof(image_rva_t));
 memset(&mod->m_entry,0xcc,sizeof(image_rva_t));
#endif
 TRY {
  result = SAFECALL_KCALL_1(*mod->m_type->m_loadmodule,mod);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  except_t code = error_code();
  /* Check for errors that indicate a corrupt module. */
  if (code == E_NO_DATA || code == E_DIVIDE_BY_ZERO ||
      code == E_OVERFLOW || code == E_INDEX_ERROR)
      return false;
  error_rethrow();
 }
 return result;
}


PUBLIC struct module_types_struct module_types = {
    .mt_lock  = ATOMIC_RWLOCK_INIT,
    .mt_types = NULL
};

PUBLIC void KCALL
register_module_type(struct module_type *__restrict self) {
 /* Assert required fields. */
 assert(self->m_driver);
 assert(self->m_loadmodule);
 assert(self->m_loadapp);
 assert(self->m_patchapp);
 assert(self->m_symbol);
 atomic_rwlock_write(&module_types.mt_lock);
 assert(!self->m_types.le_pself);
 /* Insert the module type into the chain of known types. */
 LIST_INSERT(module_types.mt_types,self,m_types);
 atomic_rwlock_endwrite(&module_types.mt_lock);
}


DEFINE_GLOBAL_UNBIND_DRIVER(unbind_driver_module_types);
PRIVATE ATTR_USED void KCALL
unbind_driver_module_types(struct driver *__restrict d) {
 struct module_type *iter,*next;
 atomic_rwlock_write(&module_types.mt_lock);
 iter = module_types.mt_types;
 while (iter) {
  next = iter->m_types.le_next;
  if (iter->m_driver == d) {
   LIST_REMOVE(iter,m_types);
   /* Remove this driver binding. */
   iter->m_types.le_pself = NULL;
   assert(d->d_app.a_refcnt >= 2);
   ATOMIC_FETCHDEC(d->d_app.a_refcnt);
  }
  iter = next;
 }
 atomic_rwlock_endwrite(&module_types.mt_lock);
}



PRIVATE ATTR_RETNONNULL REF struct module *KCALL
module_open_new(struct regular_node *__restrict node,
                struct path *__restrict modpath) {
 /* Read some magic. */
 byte_t magic[MODULE_MAX_MAGIC];
 size_t magsz = inode_read(&node->re_node,magic,sizeof(magic),0,IO_RDONLY);
 struct module_type *EXCEPT_VAR type;
 REF struct module *EXCEPT_VAR result;
 if unlikely(!magsz) error_throw(E_NOT_EXECUTABLE);
 /* Search for a fitting type. */
again:
 atomic_rwlock_read(&module_types.mt_lock);
 type = module_types.mt_types;
 for (; type; type = type->m_types.le_next) {
  /* Check for matching magic. */
  if (magsz < type->m_magsz) continue;
  if (memcmp(type->m_magic,magic,type->m_magsz) != 0) continue;
  /* Got the proper type! (At least I think we do...) */
  if (type->m_driver->d_app.a_flags & APPLICATION_FCLOSING) continue; /* Stale driver. */
  if (!driver_tryincref(type->m_driver)) continue; /* Stale driver. */
  atomic_rwlock_endread(&module_types.mt_lock);
  TRY {
   result = module_alloc();
   /* Fill in the module. */
   path_incref(modpath);
   inode_incref(&node->re_node);
   result->m_type   = type;
   result->m_driver = type->m_driver; /* Inherit reference. */
   result->m_path   = modpath;        /* Inherit reference. */
   result->m_fsloc  = node;           /* Inherit reference. */
   TRY {
    /* Try to load the module. */
    if (!module_load(result)) {
     assert(result->m_driver == type->m_driver);
     result->m_driver = NULL; /* Steal back reference. */
     module_destroy(result);
     goto continue_searching;
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    module_destroy(result);
    error_rethrow();
   }
   /* Got it! */
   return result;
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   driver_decref(type->m_driver);
   error_rethrow();
  }
continue_searching:
  atomic_rwlock_read(&module_types.mt_lock);
  /* Continue searching... */
  if (!ATOMIC_DECIFNOTONE(type->m_driver->d_app.a_refcnt)) {
   atomic_rwlock_endread(&module_types.mt_lock);
   driver_decref(type->m_driver);
   goto again;
  }
 }
 atomic_rwlock_endread(&module_types.mt_lock);
 error_throw(E_NOT_EXECUTABLE);
}


PUBLIC ATTR_RETNONNULL REF struct module *KCALL
module_open(struct regular_node *__restrict node,
            struct path *__restrict modpath) {
 REF struct module *result,*new_result;
 atomic_rwlock_read(&node->re_module.m_lock);
 /* Quick check: is the module already in cache. */
 result = node->re_module.m_module;
 if (result && ATOMIC_INCIFNONZERO(result->m_refcnt)) {
  atomic_rwlock_endread(&node->re_module.m_lock);
  return result;
 }
 atomic_rwlock_endread(&node->re_module.m_lock);

 /* Open a new module. */
 result = module_open_new(node,modpath);

 /* Save the module in-cache and check if another thread was faster. */
 atomic_rwlock_write(&node->re_module.m_lock);
 /* Quick check: is the module already in cache. */
 new_result = node->re_module.m_module;
 if (unlikely(new_result) &&
     ATOMIC_INCIFNONZERO(new_result->m_refcnt)) {
  atomic_rwlock_endwrite(&node->re_module.m_lock);
  module_decref(result);
  return new_result;
 }
 /* Save the module in-cache (weak reference). */
 node->re_module.m_module = result;
 atomic_rwlock_endwrite(&node->re_module.m_lock);

 return result;
}

PUBLIC ATTR_RETNONNULL REF struct module *KCALL
module_openpath(struct path *__restrict modpath) {
 REF struct module *COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct regular_node *EXCEPT_VAR module_node;
 atomic_rwlock_read(&modpath->p_lock);
 /* Extract the node pointed to by the given path. */
 module_node = (REF struct regular_node *)modpath->p_node;
 if unlikely(!INODE_ISREG(&module_node->re_node)) {
  atomic_rwlock_endread(&modpath->p_lock);
  error_throw(E_NOT_EXECUTABLE);
 }
 inode_incref(&module_node->re_node);
 atomic_rwlock_endread(&modpath->p_lock);
 /* Open the node and path as a module. */
 TRY {
  result = module_open(module_node,modpath);
 } FINALLY {
  inode_decref(&module_node->re_node);
 }
 return result;
}

PUBLIC bool KCALL
module_loadexcept(struct application *__restrict self) {
 struct module *EXCEPT_VAR mod = self->a_module;
 for (;;) {
  u16 flags = ATOMIC_FETCHOR(mod->m_flags,MODULE_FSECTLOADING);
  if (mod->m_flags & MODULE_FSECTLOADED) goto done;
  if (!(flags & MODULE_FSECTLOADING)) break; /* Now's our time to shine. */
  task_yield(); /* Wait a bit... */
 }
 TRY {
  /* Load special sections. */
  if (!mod->m_type->m_section) {
   mod->m_sect.m_eh_frame.ds_size = 0;
   mod->m_sect.m_except.ds_size   = 0;
  } else {
   /* Load special sections. */
   mod->m_sect.m_eh_frame = (*mod->m_type->m_section)(self,".eh_frame");
   mod->m_sect.m_except   = (*mod->m_type->m_section)(self,".except");
   *(uintptr_t *)&mod->m_sect.m_eh_frame.ds_base -= self->a_loadaddr;
   *(uintptr_t *)&mod->m_sect.m_except.ds_base   -= self->a_loadaddr;
   /* Make sure that the sections have been allocated in memory. */
   if (!(mod->m_sect.m_eh_frame.ds_flags & SHF_ALLOC))
         mod->m_sect.m_eh_frame.ds_size = 0;
   if (!(mod->m_sect.m_except.ds_flags & SHF_ALLOC))
         mod->m_sect.m_except.ds_size = 0;
  }
 } FINALLY {
  if (FINALLY_WILL_RETHROW)
       ATOMIC_FETCHAND(mod->m_flags,~MODULE_FSECTLOADING);
  else ATOMIC_FETCHOR(mod->m_flags,MODULE_FSECTLOADED);
 }
done:
 return (mod->m_sect.m_eh_frame.ds_size != 0 &&
         mod->m_sect.m_except.ds_size != 0);
}








PUBLIC void *KCALL
application_notify(void *closure, unsigned int code,
                   vm_vpage_t addr, size_t num_pages,
                   void *arg) {
 struct application *self;
 self = (struct application *)closure;
 switch (code) {

 case VM_NOTIFY_INCREF:
  assert(self->a_refcnt != 0);
  if (ATOMIC_FETCHINC(self->a_mapcnt) == 0)
      application_incref(self);
  break;

#if 0 /* XXX: Free TLS in the old vm? */
 case VM_NOTIFY_RESTORE_VM:
  break;
#endif

 case VM_NOTIFY_UNMAP:
  /* Free the TLS segment, if the application had one. */
  if (self->a_flags & APPLICATION_FHASTLS) {
   uintptr_t tls_min = self->a_loadaddr + self->a_module->m_tlsmin;
   uintptr_t tls_end = self->a_loadaddr + self->a_module->m_tlsend;
   tls_min = FLOORDIV(tls_min,PAGESIZE);
   tls_end = CEILDIV(tls_end,PAGESIZE);
   if (tls_min < addr+num_pages && tls_end > addr) {
    /* Free the TLS allocation of this application. */
    if (ATOMIC_FETCHAND(self->a_flags,~APPLICATION_FHASTLS) & APPLICATION_FHASTLS)
        tls_free(self->a_tlsoff);
   }
  }
  break;

 case VM_NOTIFY_DECREF:
  assert(self->a_refcnt != 0);
  assert(self->a_mapcnt != 0);
  if (ATOMIC_DECFETCH(self->a_mapcnt) == 0)
      application_decref(self);
  break;

 default:
  return NULL;
 }
 return closure;
}

PUBLIC ATTR_MALLOC ATTR_RETNONNULL REF
struct application *KCALL application_alloc(u16 type) {
 REF struct application *result;
 assert(type == APPLICATION_TYPE_FUSERAPP ||
        type == APPLICATION_TYPE_FDRIVER);
 result = (REF struct application *)kmalloc(type & APPLICATION_TYPE_FDRIVER
                                            ? sizeof(struct driver)
                                            : sizeof(struct application),
                                            GFP_SHARED|GFP_CALLOC);
 /* Setup reference counters. */
 result->a_refcnt  = 1;
 result->a_weakcnt = 1;
 result->a_type    = type;
 return result;
}


/* Destroy a previously allocated application. */
PUBLIC ATTR_NOTHROW void KCALL
application_weak_destroy(struct application *__restrict self) {
 kfree(self);
}

PUBLIC ATTR_NOTHROW void KCALL
application_destroy(struct application *__restrict self) {
 size_t i;
 assert(!self->a_mapcnt);
 if (self->a_type & APPLICATION_TYPE_FDRIVER) {
  struct driver *me = (struct driver *)self;
  /* Free the dynamically allocated driver commandline (if defined) */
  kfree(me->d_cmdline);
 }
 if (self->a_module)
     module_decref(self->a_module);
 for (i = 0; i < self->a_requirec; ++i)
     application_weak_decref(self->a_requirev[i]);
 kfree(self->a_requirev);
 
 /* Drop the weak reference held by the regular reference counter. */
 application_weak_decref(self);
}


PRIVATE void KCALL app_doload(struct module_patcher *__restrict self) {
 TRY {
  SAFECALL_KCALL_VOID_1(*self->mp_app->a_module->m_type->m_loadapp,self);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  except_t code = error_code();
  /* TODO: Re-enable `E_SEGFAULT' */
  if (/*code == E_SEGFAULT || */code == E_NO_DATA ||
      code == E_DIVIDE_BY_ZERO || code == E_OVERFLOW ||
      code == E_INDEX_ERROR) {
   struct exception_info *info = error_info();
   memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
   info->e_error.e_code = E_NOT_EXECUTABLE;
  }
  error_rethrow();
 }
}
PRIVATE void KCALL app_dopatch(struct module_patcher *__restrict self) {
 TRY {
  SAFECALL_KCALL_VOID_1(*self->mp_app->a_module->m_type->m_patchapp,self);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  except_t code = error_code();
  /* TODO: Re-enable `E_SEGFAULT' */
  if (/*code == E_SEGFAULT || */code == E_NO_DATA ||
      code == E_DIVIDE_BY_ZERO || code == E_OVERFLOW ||
      code == E_INDEX_ERROR) {
   struct exception_info *info = error_info();
   memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
   info->e_error.e_code = E_NOT_EXECUTABLE;
  }
  error_rethrow();
 }
}


/* Automatically find a free location for `self' or ensure that its
 * fixed address isn't already in use while keeping a lock on the
 * effective VM.
 * Additionally, check that `APPLICATION_TYPE_FDRIVER' applications
 * end up in kernel-space, and `APPLICATION_TYPE_FUSERAPP' end up in
 * user-space before mapping the application at its proper location.
 * NOTE: This function calls `application_load()', meaning that the
 *       application will be registered as VM-global upon success.
 * NOTE: This function will automatically initialize `a_loadaddr'
 * @throw: E_BADALLOC: No suitable free range found for a dynamically relocatable application.
 * @throw: E_BADALLOC: The address range required by a fixed-address application is already in use. */
PUBLIC void KCALL
application_load(struct module_patcher *__restrict self) {
 struct application *app = self->mp_app;
 struct module *mod = app->a_module;
 assert((self->mp_root == self) ==
        (self->mp_prev == NULL));
 if (mod->m_flags & MODULE_FFIXED) {
  vm_vpage_t app_min,app_end;
  /* Fixed base address. */
  app->a_loadaddr = mod->m_fixedbase;
  if (__builtin_add_overflow(app->a_loadaddr,mod->m_imagemin,&app_min)) goto noexec;
  if (__builtin_add_overflow(app->a_loadaddr,mod->m_imageend,&app_end)) goto noexec;
  app->a_bounds.b_min = app_min;
  app->a_bounds.b_max = app_end-1;
  app_min = FLOORDIV(app_min,PAGESIZE);
  app_end = CEILDIV(app_end,PAGESIZE);
  if (app_min > app_end || app_end > VM_VPAGE_MAX+1)
      goto noexec;
  /* Validate associated address ranges. */
  if (app->a_type & APPLICATION_TYPE_FDRIVER) {
   assert(vm_holding(&vm_kernel));
   if (app_min < X86_KERNEL_BASE_PAGE)
       goto noexec;
  } else {
   assert(vm_holding(THIS_VM));
   if (app_end > X86_KERNEL_BASE_PAGE)
       goto noexec;
  }
  /* Load the application. */
  app_doload(self);
 } else {
  /* Dynamic base address. */
  size_t req_pages;
  vm_vpage_t app_base,app_hint;
  req_pages = CEILDIV(mod->m_imageend-
                      mod->m_imagemin,
                      PAGESIZE);
  /* Calculate the page number for a base address hint. */
  app_hint = FLOORDIV(mod->m_fixedbase+mod->m_imagemin,PAGESIZE);

  /* Find a suitable location for the load address. */
  if (app->a_type & APPLICATION_TYPE_FDRIVER) {
   if (!(mod->m_flags & MODULE_FBASEHINT) ||
        (app_hint < X86_KERNEL_BASE_PAGE) ||
        (app_hint+req_pages < app_hint) ||
        (app_hint+req_pages > VM_VPAGE_MAX+1))
         app_hint = VM_KERNELDRIVER_HINT;
   assert(vm_holding(&vm_kernel));
  } else {
   if (!(mod->m_flags & MODULE_FBASEHINT) ||
        (app_hint+req_pages > X86_KERNEL_BASE_PAGE) ||
        (app_hint+req_pages < app_hint))
         app_hint = VM_USERLIB_HINT;
   assert(vm_holding(THIS_VM));
  }
#if 0
  /* XXX: Maybe integrate ASLR directly into `vm_getfree()'? (as a flag?) */
  if (!(self->mp_flags & DL_OPEN_FNOASLR))
        app_hint += rand() & 0xff;
#endif
  app_base = vm_getfree(app_hint,req_pages,1,0,
#if VM_USERLIB_MODE == VM_KERNELDRIVER_MODE
                        VM_KERNELDRIVER_MODE
#else
                       (app->a_type & APPLICATION_TYPE_FDRIVER)
                        ? VM_KERNELDRIVER_MODE
                        : VM_USERLIB_MODE
#endif
                        );
  /* Convert the map page to the load address. */
  app_base *= PAGESIZE;
  app_base -= mod->m_imagemin;
  app->a_loadaddr = app_base;
  app->a_bounds.b_min = app_base+mod->m_imagemin;
  app->a_bounds.b_max = app_base+mod->m_imageend-1;

  /* Load the application. */
  app_doload(self);

  /* Shouldn't happen: the application wasn't mapped? */
  if unlikely(!app->a_mapcnt) {
#if 0
   vm_unmap(VM_ADDR2PAGE(app_base+mod->m_imagemin),
            req_pages,VM_UNMAP_NOEXCEPT|VM_UNMAP_SYNC,NULL);
#endif
   error_throw(E_NOT_EXECUTABLE);
  }
 }
#ifndef CONFIG_NO_RECENT_MODULES
 /* Cache the module of this application as having been used recently. */
 module_recent(app->a_module);
#endif /* !CONFIG_NO_RECENT_MODULES */
 return;
noexec:
 error_throw(E_NOT_EXECUTABLE);
}

PUBLIC void KCALL
application_patch(struct module_patcher *__restrict self) {
 struct application *app = self->mp_app;
 struct vm *effective_vm;
 assert((self->mp_root == self) ==
        (self->mp_prev == NULL));
 effective_vm = (app->a_type & APPLICATION_TYPE_FDRIVER) ? &vm_kernel : THIS_VM;
 assert(vm_holding(effective_vm));

 /* Patch the application. */
 app_dopatch(self);

 if (self->mp_requirec != self->mp_requirea) {
  assert(self->mp_requirec < self->mp_requirea);
  assert(self->mp_requirec != 0);
  self->mp_requirev = (WEAK REF struct application **)krealloc(self->mp_requirev,self->mp_requirec*
                                                               sizeof(WEAK REF struct application *),
                                                               GFP_SHARED);
 }

 /* Copy dependencies. */
 app->a_requirev = self->mp_requirev; /* Inherit. */
 COMPILER_WRITE_BARRIER();
 app->a_requirec = self->mp_requirec;
 self->mp_requirev = NULL;
 self->mp_requirec = 0;
 COMPILER_WRITE_BARRIER();

 /* Make the application available globally. */
 if (self->mp_flags & DL_OPEN_FGLOBAL)
     vm_apps_append(effective_vm,app);
}


PUBLIC void KCALL
patcher_fini(struct module_patcher *__restrict self) {
 size_t i;
 if (!self->mp_requirev) return;
 for (i = 0; i < self->mp_requirec; ++i)
     application_weak_decref(self->mp_requirev[i]);
 kfree(self->mp_requirev);
}


PRIVATE struct application *KCALL
patcher_getrequire(struct module_patcher *__restrict self,
                   struct module *__restrict dependency) {
 size_t i;
 /* Check if this patcher _is_ for that same module. */
 if (self->mp_app->a_module == dependency)
     return self->mp_app;
 /* Search the dependency vector in reverse order. */
 i = self->mp_requirec;
 while (i--) {
  if (self->mp_requirev[i]->a_module == dependency)
      return self->mp_requirev[i];
 }
 return NULL;
}

/* Add `mod' as a required dependency of `self', loading it as a new dependency.
 * NOTE: The caller must be holding a lock on the effective VM. */
PUBLIC ATTR_RETNONNULL struct application *KCALL
patcher_require(struct module_patcher *EXCEPT_VAR self,
                struct module *__restrict mod) {
 struct module_patcher *EXCEPT_VAR xself = self;
 struct module_patcher *iter;
 struct application *EXCEPT_VAR result;
 struct vm *effective_vm;
 /* Search for an existing use of this dependency in the patcher tree. */
 assert(self);
 for (iter = self; iter; iter = iter->mp_prev) {
  result = patcher_getrequire(iter,mod);
  if (result) return result;
 }
 /* Search the effective VM-globals table for the module. */
 if (self->mp_apptype & APPLICATION_TYPE_FDRIVER) {
  effective_vm = &vm_kernel;
 } else {
  effective_vm = THIS_VM;
 }
 assert(vm_holding(effective_vm));
 {
  struct vmapps *apps;
  apps = FORVM(effective_vm,vm_apps);
  if (apps) {
   size_t i;
recheck_apps:
   atomic_rwlock_read(&apps->va_lock);
   for (i = 0; i < apps->va_count; ++i) {
    result = apps->va_apps[i];
    if (result->a_refcnt == 0) {
     /* Remove dangling applications. */
     if (!atomic_rwlock_tryupgrade(&apps->va_lock))
          continue;
     --apps->va_count;
     memmove(&apps->va_apps[i],&apps->va_apps[i+1],
             (apps->va_count-i)*
              sizeof(WEAK REF struct application *));
     atomic_rwlock_endwrite(&apps->va_lock);
     /* Drop the weak reference that was stored in the vector. */
     application_weak_decref(result);
     /* If this was the last application, delete the VM_APPS vector. */
     if (!apps->va_count) {
      if (ATOMIC_DECFETCH(apps->va_share) == 0)
          vmapps_destroy(apps);
      FORVM(effective_vm,vm_apps) = NULL;
      goto not_cached;
     }
     goto recheck_apps;
    }
    if (result->a_module == mod) {
     atomic_rwlock_endread(&apps->va_lock);
     goto got_application;
    }
   }
   atomic_rwlock_endread(&apps->va_lock);
  }
 }
not_cached:
 /* Create a new dependency for `mod'. */
 {
  struct module_patcher patcher;
  struct module_patcher *EXCEPT_VAR ppatcher = &patcher;
  result              = application_alloc(self->mp_apptype);
  result->a_module    = mod;
  module_incref(mod);
  patcher.mp_root     = self->mp_root;
  patcher.mp_prev     = self;
  patcher.mp_app      = result;
  patcher.mp_requirec = 0;
  patcher.mp_requirea = 0;
  patcher.mp_requirev = NULL;
  patcher.mp_altpath  = NULL;
  patcher.mp_flags    = self->mp_flags;
  patcher.mp_apptype  = self->mp_apptype;
  patcher.mp_appflags = self->mp_appflags;
#ifndef NDEBUG /* Mustn't be used by non-root patchers. */
  memset((void *)&patcher.mp_runpath,0xcc,sizeof(char *));
#endif
  TRY {
   /* Load the application. */
   application_load(&patcher);
   /* Patch the application. */
   application_patch(&patcher);

   assertf(result->a_mapcnt != 0,"The application didn't get mapped");
   assertf(result->a_refcnt >= 2,
           "We're holding one reference, and the other must be "
           "held by the application being mapped (a_mapcnt != 0).");
  } FINALLY {
   application_decref(result);
   patcher_fini(ppatcher);
  }

got_application:
  /* Add the application to the list of requirements of the current patcher. */
  assert(self->mp_requirec <= self->mp_requirea);
  if (self->mp_requirec == self->mp_requirea) {
   size_t EXCEPT_VAR new_alloc = self->mp_requirea * 2;
   if (!new_alloc) new_alloc = 1;
   TRY {
    self->mp_requirev = (WEAK REF struct application **)krealloc(self->mp_requirev,new_alloc*
                                                                 sizeof(WEAK REF struct application *),
                                                                 GFP_SHARED);
   } CATCH (E_BADALLOC) {
    if (new_alloc == xself->mp_requirec+1) error_rethrow();
    error_handled();
    new_alloc = xself->mp_requirec+1;
    xself->mp_requirev = (WEAK REF struct application **)krealloc(xself->mp_requirev,new_alloc*
                                                                  sizeof(WEAK REF struct application *),
                                                                  GFP_SHARED);
   }
   xself->mp_requirea = new_alloc;
  }
  application_weak_incref(result);
  xself->mp_requirev[xself->mp_requirec++] = result; /* Inherit reference. */
 }
 return result;
}


#define throw_fs_error(fs_error_code) \
        __EXCEPT_INVOKE_THROW_NORETURN(throw_fs_error(fs_error_code))
PRIVATE __EXCEPT_NORETURN void
(KCALL throw_fs_error)(u16 fs_error_code) {
 struct exception_info *info;
 info = error_info();
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_code                        = E_FILESYSTEM_ERROR;
 info->e_error.e_flag                        = ERR_FNORMAL;
 info->e_error.e_filesystem_error.fs_errcode = fs_error_code;
 error_throw_current();
 __builtin_unreachable();
}



PRIVATE ATTR_RETNONNULL struct application *KCALL
patcher_open_path(struct module_patcher *__restrict self,
                  struct path *__restrict p,
                  USER CHECKED char const *__restrict name,
                  u16 name_length) {
 struct application *COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct module *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(appmodule);
 REF struct path *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(module_path);
 TRY {
  module_path = (self->mp_flags & DL_OPEN_FNOCASE) ? path_casechild(p,name,name_length)
                                                   : path_child(p,name,name_length);
 } CATCH (E_FILESYSTEM_ERROR) {
  if (error_info()->e_error.e_filesystem_error.fs_errcode == ERROR_FS_PATH_NOT_FOUND)
      error_info()->e_error.e_filesystem_error.fs_errcode = ERROR_FS_FILE_NOT_FOUND;
  error_rethrow();
 }
 TRY {
  /* Try to open a module from the generated path. */
  appmodule = module_openpath(module_path);
 } FINALLY {
  path_decref(module_path);
 }
 TRY {
  /* Add the module as a required dependency. */
  result = patcher_require(self,appmodule);
 } FINALLY {
  module_decref(appmodule);
 }
 return result;
}

PUBLIC ATTR_RETNONNULL struct application *KCALL
patcher_require_string(struct module_patcher *__restrict self_,
                       USER CHECKED char const *__restrict name_,
                       size_t name_length_) {
 struct module_patcher *EXCEPT_VAR self = self_;
 USER CHECKED char const *EXCEPT_VAR name = name_;
 size_t EXCEPT_VAR name_length = name_length_;
 struct application *COMPILER_IGNORE_UNINITIALIZED(result);
 struct module_patcher *EXCEPT_VAR iter;
 REF struct path *EXCEPT_VAR search_path;
 USER CHECKED char *EXCEPT_VAR p;
 USER CHECKED char *EXCEPT_VAR end;
 char EXCEPT_VAR ch;
 if unlikely(name_length > (u16)-1)
    goto not_found;
 iter = self;
 do {
  end = p = (char *)iter->mp_altpath;
  if (!p) goto next_iter;
next_part:
  for (;; ++end) {
   ch = *end;
   if (!ch) break;
   if (ch == ':' && !(iter->mp_flags&DL_OPEN_FDOSALT)) break;
   if (ch == ';' && (iter->mp_flags&DL_OPEN_FDOSALT)) break;
  }
  COMPILER_BARRIER();
  TRY {
   search_path = fs_path(NULL,p,(size_t)(end-p),NULL,
                        (iter->mp_flags&DL_OPEN_FDOSALT)
                       ? FS_MODE_FDIRECTORY|FS_MODE_FIGNORE_TRAILING_SLASHES|FS_MODE_FDOSPATH
                       : FS_MODE_FDIRECTORY|FS_MODE_FIGNORE_TRAILING_SLASHES);
   /* Search for the module in this path. */
   TRY {
    result = patcher_open_path(self,search_path,name,(u16)name_length);
   } FINALLY {
    path_decref(search_path);
   }
  } CATCH (E_FILESYSTEM_ERROR) {
   struct exception_info *info = (error_info)();
   if (info->e_error.e_filesystem_error.fs_errcode != ERROR_FS_NOT_A_DIRECTORY &&
       info->e_error.e_filesystem_error.fs_errcode != ERROR_FS_PATH_NOT_FOUND)
       error_rethrow();
   COMPILER_BARRIER();
   error_handled();
   if (ch) {
    p = ++end;
    goto next_part;
   }
  }
  return result;
next_iter:;
 } while ((iter = iter->mp_prev) != NULL);
 iter = self->mp_root;
 end = p = (char *)iter->mp_runpath;
 if (!p) goto not_found;
next_rootpart:
 for (;; ++end) {
  ch = *end;
  if (!ch) break;
  if (ch == ':' && !(iter->mp_flags&DL_OPEN_FDOSRUN)) break;
  if (ch == ';' && (iter->mp_flags&DL_OPEN_FDOSRUN)) break;
 }
 COMPILER_BARRIER();
 TRY {
  search_path = fs_path(NULL,p,(size_t)(end-p),NULL,
                       (iter->mp_flags&DL_OPEN_FDOSRUN)
                      ? FS_MODE_FDIRECTORY|FS_MODE_FIGNORE_TRAILING_SLASHES|FS_MODE_FDOSPATH
                      : FS_MODE_FDIRECTORY|FS_MODE_FIGNORE_TRAILING_SLASHES);
  /* Search for the module in this path. */
  TRY {
   result = patcher_open_path(self,search_path,name,(u16)name_length);
  } FINALLY {
   path_decref(search_path);
  }
  return result;
 } CATCH (E_FILESYSTEM_ERROR) {
  struct exception_info *info = (error_info)();
  if (info->e_error.e_filesystem_error.fs_errcode != ERROR_FS_NOT_A_DIRECTORY &&
      info->e_error.e_filesystem_error.fs_errcode != ERROR_FS_PATH_NOT_FOUND)
      error_rethrow();
  error_handled();
 }
 COMPILER_BARRIER();
 if (ch) {
  p = ++end;
  goto next_rootpart;
 }
not_found:
 throw_fs_error(ERROR_FS_FILE_NOT_FOUND);
}


PRIVATE struct dl_symbol KCALL
impl_patcher_symaddr(struct module_patcher *__restrict self,
                     USER CHECKED char const *__restrict name,
                     u32 hash, bool search_current) {
 struct dl_symbol result,weak_result; size_t i;
 weak_result.ds_type = MODULE_SYMBOL_INVALID;
 /* Deep binding prefers the module itself above others. */
 if (self->mp_flags & DL_OPEN_FDEEPBIND) {
  if (!search_current) goto done;
  result = (*self->mp_app->a_module->m_type->m_symbol)(self->mp_app,name,hash);
  if (result.ds_type != MODULE_SYMBOL_INVALID) {
   if (result.ds_type == MODULE_SYMBOL_NORMAL)
       goto done;
   weak_result = result;
  }
 }

 /* Search upper modules first. */
 if (self->mp_prev) {
  result = impl_patcher_symaddr(self->mp_prev,name,hash,true);
  if (result.ds_type != MODULE_SYMBOL_INVALID) {
   if (result.ds_type == MODULE_SYMBOL_NORMAL)
       goto done;
   weak_result = result;
  }
 }


 /* Search dependencies in ascending order. */
 for (i = 0; i < self->mp_requirec; ++i) {
  struct application *dep = self->mp_requirev[i];
  result = (*dep->a_module->m_type->m_symbol)(dep,name,hash);
  if (result.ds_type != MODULE_SYMBOL_INVALID) {
   if (result.ds_type == MODULE_SYMBOL_NORMAL) goto done;
   weak_result = result;
  }
 }

 /* Search the module being patched itself. */
 if (!(self->mp_flags & DL_OPEN_FDEEPBIND)) {
  result = (*self->mp_app->a_module->m_type->m_symbol)(self->mp_app,name,hash);
  if (result.ds_type != MODULE_SYMBOL_INVALID) {
   if (result.ds_type == MODULE_SYMBOL_NORMAL) goto done;
   weak_result = result;
  }
 }
 return weak_result;
done:
 return result;
}


#define ELFNAME_THIS_DRIVER  "this_driver"
#define ELFHASH_THIS_DRIVER  0x5c3fc52

PUBLIC void *KCALL
patcher_symaddr(struct module_patcher *__restrict self,
                USER CHECKED char const *__restrict name,
                u32 hash, bool search_current) {
 struct dl_symbol result;
 result = impl_patcher_symaddr(self,name,hash,search_current);
 if (result.ds_type != MODULE_SYMBOL_INVALID)
     return result.ds_base;
 if (self->mp_apptype & APPLICATION_TYPE_FDRIVER) {
  /* Special symbols provided for drivers. */
  assertf(patcher_symhash(ELFNAME_THIS_DRIVER) == ELFHASH_THIS_DRIVER,
          "`ELFHASH_THIS_DRIVER' should be 0x%x",
          patcher_symhash(ELFNAME_THIS_DRIVER));
  if (hash == ELFHASH_THIS_DRIVER &&
      strcmp(name,ELFNAME_THIS_DRIVER) == 0)
      return (struct driver *)self->mp_app;
 }
 return NULL;
}

PUBLIC u32 KCALL
patcher_symhash(USER CHECKED char const *__restrict name) {
 /* HINT: This is the same hashing algorithm used by ELF. */
 u32 h = 0,g;
 while (*name) {
  h = (h << 4) + *name++;
  g = h & 0xf0000000;
  if (g) h ^= g >> 24;
  h &= ~g;
 }
 return h;
}






PUBLIC void KCALL
application_loadroot(struct application *__restrict self, u16 flags,
                     USER CHECKED char const *runpath) {
 struct module_patcher patcher;
 struct module_patcher *EXCEPT_VAR ppatcher = &patcher;
 struct vm *EXCEPT_VAR effective_vm;
 patcher.mp_root     = &patcher;
 patcher.mp_prev     = NULL;
 patcher.mp_app      = self;
 patcher.mp_requirec = 0;
 patcher.mp_requirea = 0;
 patcher.mp_requirev = NULL;
 patcher.mp_runpath  = runpath;
 patcher.mp_altpath  = NULL;
 patcher.mp_flags    = flags;
 patcher.mp_apptype  = self->a_type;
 patcher.mp_appflags = self->a_flags;
 effective_vm = (patcher.mp_apptype & APPLICATION_TYPE_FDRIVER) ? &vm_kernel : THIS_VM;
 vm_acquire(effective_vm);
 TRY {
  /* Load the application. */
  application_load(&patcher);
  /* Patch the application. */
  application_patch(&patcher);
 } FINALLY {
  vm_release(effective_vm);
  patcher_fini(ppatcher);
 }
}


PUBLIC struct dl_symbol KCALL
application_dlsym(struct application *__restrict self,
                  USER CHECKED char const *__restrict name) {
 return application_dlsym3(self,name,patcher_symhash(name));
}
PUBLIC struct dl_symbol KCALL
application_dlsym3(struct application *__restrict self,
                   USER CHECKED char const *__restrict name,
                   u32 hash) {
 /* Lookup a symbol within the associated module. */
 return (*self->a_module->m_type->m_symbol)(self,name,hash);
}

PUBLIC struct dl_section KCALL
application_dlsect(struct application *__restrict self,
                   USER CHECKED char const *__restrict name) {
 struct dl_section (KCALL *pfun)(struct application *__restrict app,
                                     USER CHECKED char const *__restrict name);
 pfun = self->a_module->m_type->m_section;
 if (!pfun) {
  struct dl_section result;
  result.ds_size = 0; /* return ZERO(0) to indicate an empty section. */
  return result;
 }
 /* Lookup a section within the associated module. */
 return (*pfun)(self,name);
}

PUBLIC void KCALL
application_enuminit(struct application *__restrict app,
                     module_enumerator_t func, void *arg) {
 void (KCALL *callback)(struct application *__restrict app,
                        module_enumerator_t func, void *arg);
 callback = app->a_module->m_type->m_enuminit;
 if (!(ATOMIC_FETCHOR(app->a_flags,APPLICATION_FDIDINIT)&APPLICATION_FDIDINIT) && callback)
     SAFECALL_KCALL_VOID_3(*callback,app,func,arg);
}
PUBLIC void KCALL
application_enumfini(struct application *__restrict app,
                     module_enumerator_t func, void *arg) {
 void (KCALL *callback)(struct application *__restrict app,
                        module_enumerator_t func, void *arg);
 u16 old_flags;
 callback = app->a_module->m_type->m_enumfini;
 do {
  old_flags = ATOMIC_READ(app->a_flags);
  if (!(old_flags & APPLICATION_FDIDINIT))
      return; /* Never initialized. */
  if (old_flags & APPLICATION_FDIDFINI)
      return; /* Already finalized. */
 } while (!ATOMIC_CMPXCH_WEAK(app->a_flags,old_flags,old_flags|APPLICATION_FDIDFINI));
 if (callback)
     SAFECALL_KCALL_VOID_3(*callback,app,func,arg);
}

PUBLIC void KCALL
vm_apps_initall(struct cpu_hostcontext_user *__restrict context) {
 struct vmapps *apps;
 struct vm *EXCEPT_VAR myvm = THIS_VM;
again:
 vm_acquire_read(myvm);
 TRY {
  apps = FORVM(myvm,vm_apps);
  if (apps) {
   size_t i;
again_inner:
   atomic_rwlock_read(&apps->va_lock);
   for (i = 0; i < apps->va_count; ++i) {
    WEAK REF struct application *EXCEPT_VAR app;
    app = apps->va_apps[i];
    if (app->a_flags & APPLICATION_FDIDINIT) continue;
    if (!application_tryincref(app)) continue;
    atomic_rwlock_endread(&apps->va_lock);
    TRY {
     application_loaduserinit(app,context);
    } FINALLY {
     application_decref(app);
    }
    goto again_inner;
   }
   atomic_rwlock_endread(&apps->va_lock);
  }
 } FINALLY {
  if (vm_release_read(myvm))
      goto again;
 }
}
PUBLIC void KCALL
vm_apps_finiall(struct cpu_hostcontext_user *__restrict context) {
 struct vmapps *apps;
 struct vm *EXCEPT_VAR myvm = THIS_VM;
again:
 vm_acquire_read(myvm);
 TRY {
  apps = FORVM(myvm,vm_apps);
  if (apps) {
   size_t i;
again_inner:
   atomic_rwlock_read(&apps->va_lock);
   for (i = 0; i < apps->va_count; ++i) {
    WEAK REF struct application *EXCEPT_VAR app;
    app = apps->va_apps[i];
    if (app->a_flags & APPLICATION_FDIDFINI) continue;
    if (!(app->a_flags & APPLICATION_FDIDINIT)) continue;
    if (!application_tryincref(app)) continue;
    atomic_rwlock_endread(&apps->va_lock);
    TRY {
     application_loaduserfini(app,context);
    } FINALLY {
     application_decref(app);
    }
    goto again_inner;
   }
   atomic_rwlock_endread(&apps->va_lock);
  }
 } FINALLY {
  if (vm_release_read(myvm))
      goto again;
 }
}

PUBLIC ATTR_RETNONNULL REF struct application *
KCALL vm_getapp(USER UNCHECKED void *user_handle) {
 REF struct application *COMPILER_IGNORE_UNINITIALIZED(result);
 struct vm *EXCEPT_VAR myvm;
 if (!user_handle) {
  result = vm_apps_primary(THIS_VM);
  if unlikely(!result)
     error_throwf(E_SEGFAULT,SEGFAULT_BADREAD,user_handle);
  return result;
 }
 validate_readable(user_handle,1);
 myvm = THIS_VM;
again:
 vm_acquire_read(myvm);
 TRY {
  struct vm_node *node;
  node = vm_getnode(VM_ADDR2PAGE((uintptr_t)user_handle));
  if (!node || node->vn_notify != &application_notify)
       error_throwf(E_SEGFAULT,SEGFAULT_BADREAD,user_handle);
  result = (REF struct application *)node->vn_closure;
  assert(result->a_refcnt != 0);
  application_incref(result);
 } FINALLY {
  if (vm_release_read(myvm))
      goto again;
 }
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_LINKER_C */
