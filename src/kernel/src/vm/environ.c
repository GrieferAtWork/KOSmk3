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
#ifndef GUARD_KERNEL_SRC_VM_ENVIRON_C
#define GUARD_KERNEL_SRC_VM_ENVIRON_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/align.h>
#include <hybrid/limits.h>
#include <kernel/vm.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <kernel/user.h>
#include <kernel/bind.h>
#include <sys/mman.h>
#include <linux/limits.h>
#include <except.h>
#include <kos/environ.h>
#include <string.h>

DECL_BEGIN

PUBLIC ATTR_PERVM USER UNCHECKED
struct process_environ *vm_environ = NULL;

DEFINE_PERVM_CLONE(vm_clone_environ);
INTERN void KCALL vm_clone_environ(struct vm *__restrict new_vm) {
 /* Copy the process environment pointer. */
 FORVM(new_vm,vm_environ) = PERVM(vm_environ);
}



/* Construct a memory region describing application environment
 * data, matching the given `argv' and `envp' data vectors.
 * NOTE: This function is assuming that the calling thread is
 *       the only thread running in the associated VM!
 * NOTE: The `struct application_environ' structure contained
 *       in the resulting region hasn't been relocated, yet.
 *       Once the caller has mapped it, they should invoke
 *      `environ_relocate()' in order to adjust region-relative
 *       pointers. */
PUBLIC ATTR_RETNONNULL struct vm_node *KCALL
environ_alloc(USER UNCHECKED char *USER UNCHECKED *argv,
              USER UNCHECKED char *USER UNCHECKED *envp) {
 size_t argc,envc,i;
 struct vm_node *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct vm_region *EXCEPT_VAR result_region;
 struct vm *EXCEPT_VAR myvm = THIS_VM;
 vm_acquire(myvm);
 TRY {
  validate_readable(argv,sizeof(char *));
  validate_readable_opt(envp,sizeof(char *));
  argc = envc = 0;
  for (; argv[argc]; ++argc);
  if (envp) for (; envp[envc]; ++envc);
#if 0
  else if (FORVM(myvm,vm_environ)) {
   /* Try try to re-use the original environment block of the calling process. */
   TRY {
    envp = FORVM(myvm,vm_environ)->pe_envp;
   } CATCH_HANDLED (E_SEGFAULT) {
    envp = NULL;
   }
   if (envp) for (; envp[envc]; ++envc);
  }
#endif
  /* Allocate a new memory region for the worst-case scenario. */
  result_region = vm_region_alloc(CEILDIV(ARG_MAX,PAGESIZE));
  result_region->vr_init = VM_REGION_INIT_FFILLER;
  TRY {
   result = (struct vm_node *)kmalloc(sizeof(struct vm_node),
                                      GFP_SHARED|GFP_LOCKED);
   TRY {
    size_t COMPILER_IGNORE_UNINITIALIZED(total_size);
    /* Find a temporary location where we can map our working buffer in user-space.
     * NOTE: Since this function is only ever used when all others threads that
     *       may have used the VM at some point have already died, we can safely
     *       use user-space addresses for this. */
    result->vn_node.a_vmin = vm_getfree(0,
                                        CEILDIV(ARG_MAX,PAGESIZE)+1, /* +1 so we can map a temporary range with an unmapped trailing
                                                                      * page, thus being able to safely deal with requests exceeding
                                                                      * the environment block limit. */
                                        1,
                                        0,
                                        VM_GETFREE_FABOVE);
    result->vn_node.a_vmax = result->vn_node.a_vmin;
    result->vn_node.a_vmax += CEILDIV(ARG_MAX,PAGESIZE)-1;
    result->vn_start                  = 0;
    result->vn_region                 = result_region;
    result->vn_notify                 = NULL;
    result->vn_flag                   = VM_NODE_FNORMAL;
    result->vn_prot                   = PROT_READ|PROT_WRITE|PROT_NOUSER; /* NOUSER... just in case. */
    result_region->vr_part0.vp_refcnt = 1;
    /* Set the DONTMERGE flag so we can safely re-extract the region below. */
    result_region->vr_flags |= VM_REGION_FDONTMERGE;
    /* Temporarily map the node. */
    vm_insert_and_activate_node(myvm,result);
    TRY {
     /* Generate the environment region. */
     struct process_environ *env;
     byte_t *iter;
     char **pargv_vector;
     char **penvp_vector;

     env = (struct process_environ *)VM_PAGE2ADDR(result->vn_node.a_vmin);
     env->pe_argc = argc;
     env->pe_envc = envc;

     /* Start filling in the buffer. */
     iter = (byte_t *)(env+1);
     pargv_vector = (char **)iter,iter += (argc+1)*sizeof(char *);
     penvp_vector = (char **)iter,iter += (envc+1)*sizeof(char *);
     env->pe_argv = (char **)((uintptr_t)pargv_vector - (uintptr_t)env);
     env->pe_envp = (char **)((uintptr_t)penvp_vector - (uintptr_t)env);

     /* Make sure that both vectors will be NULL-terminated. */
     pargv_vector[argc] = NULL;
     penvp_vector[envc] = NULL;

     /* NOTE: If we exceed the limit of what's the allowed size of
      *       the process environment block, a PAGEFAULT will be
      *       thrown. */

     /* Copy argument strings. */
     for (i = 0; i < argc; ++i) {
      size_t len; char *str;
      pargv_vector[i] = (char *)((uintptr_t)iter - (uintptr_t)env);
      str = argv[i],len = strlen(str)+1;
      memcpy(iter,str,len*sizeof(char));
      iter += len*sizeof(char);
     }

     /* Copy environment strings. */
     for (i = 0; i < envc; ++i) {
      size_t len; char *str;
      penvp_vector[i] = (char *)((uintptr_t)iter - (uintptr_t)env);
      str = envp[i],len = strlen(str)+1;
      memcpy(iter,str,len*sizeof(char));
      iter += len*sizeof(char);
     }

     /* Set the total, used size of the region. */
     total_size = (uintptr_t)iter-(uintptr_t)env;
     env->pe_size = total_size;

    } FINALLY {
     /* Remove our temporary mapping from the VM once again. */
     vm_pop_nodes(myvm,
                  result->vn_node.a_vmin,
                  result->vn_node.a_vmax,
                  VM_UNMAP_NOEXCEPT,NULL);
     pagedir_map(result->vn_node.a_vmin,
                 VM_NODE_SIZE(result),0,
                 PAGEDIR_MAP_FUNMAP);
     pagedir_sync(0,VM_NODE_SIZE(result));
     /* Delete the DONTMERGE flag, now that we've extracted the region. */
     result_region->vr_flags &= ~VM_REGION_FDONTMERGE;
    }
    {
     vm_raddr_t used_size;
     struct vm_part *first_unused_part;
     /* Truncate the saved length of the region. */
     assert(result_region->vr_refcnt == 1);
     used_size = CEILDIV(total_size,PAGESIZE);
     result_region->vr_size = used_size;
     first_unused_part = result_region->vr_parts;
     while (first_unused_part &&
            first_unused_part->vp_start < used_size)
            first_unused_part = first_unused_part->vp_chain.le_next;
     /* Delete a potentially unused trailing part. */
     if (first_unused_part) {
      assert(first_unused_part != result_region->vr_parts);
      assert(!first_unused_part->vp_chain.le_next);
      assert(first_unused_part->vp_state == VM_PART_MISSING);
      *first_unused_part->vp_chain.le_pself = NULL;
      /* Free the unused part. */
      if (first_unused_part != &result_region->vr_part0)
          kfree(first_unused_part);
     }
     /* Update the size of the node. */
     result->vn_node.a_vmax  = result->vn_node.a_vmin;
     result->vn_node.a_vmax += used_size-1;
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    kfree(result);
    error_rethrow();
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   vm_region_decref(result_region);
   error_rethrow();
  }
 } FINALLY {
  vm_release(myvm);
 }
 return result;
}


/* Relocate relative pointers in `self' to become absolute. */
FUNDEF void KCALL
environ_relocate(USER CHECKED struct process_environ *__restrict self) {
 size_t i,count; char **vector;
 *(uintptr_t *)&self->pe_argv += (uintptr_t)self;
 *(uintptr_t *)&self->pe_envp += (uintptr_t)self;
 vector = self->pe_argv,count = self->pe_argc;
 assert((uintptr_t)vector < KERNEL_BASE);
 for (i = 0; i < count; ++i) *(uintptr_t *)&vector[i] += (uintptr_t)self;
 vector = self->pe_envp,count = self->pe_envc;
 assert((uintptr_t)vector < KERNEL_BASE);
 for (i = 0; i < count; ++i) *(uintptr_t *)&vector[i] += (uintptr_t)self;
}


PUBLIC void KCALL
environ_create(char const *__restrict init_name,
               size_t init_name_length) {
 REF struct vm_region *EXCEPT_VAR region; size_t req_bytes;
 struct process_environ *COMPILER_IGNORE_UNINITIALIZED(env);
 byte_t *iter;
 req_bytes = (sizeof(struct process_environ)+
              2*sizeof(char *)+ /* argv: { init_name, NULL } */
              1*sizeof(char *)+ /* envp: { NULL } */
             (init_name_length+1)*sizeof(char));
 /* Allocate a new region for the process environment block. */
 region = vm_region_alloc(CEILDIV(req_bytes,PAGESIZE));
 region->vr_init = VM_REGION_INIT_FFILLER;
 TRY {
  /* Map the process environment region into memory. */
  env = (struct process_environ *)vm_map(VM_USERENV_HINT,
                                         region->vr_size,
                                         1,
                                         0,
                                         VM_USERENV_MODE,
                                         0,
                                         region,
                                         PROT_READ|PROT_WRITE,
                                         NULL,
                                         NULL);
 } FINALLY {
  vm_region_decref(region);
 }
 /* Fill in the region. */
 env->pe_self = env;
 env->pe_size = req_bytes;
 env->pe_argc = 1;
 env->pe_envc = 0;
 iter = (byte_t *)(env+1);
 env->pe_argv = (char **)iter;
 iter += sizeof(char *);
 *(char **)iter = NULL;
 iter += sizeof(char *);
 env->pe_envp = (char **)iter;
 *(char **)iter = NULL;
 iter += sizeof(char *);
 env->pe_argv[0] = (char *)iter;
 memcpy(iter,init_name,init_name_length*sizeof(char));
 ((char *)iter)[init_name_length] = '\0';

 /* Save the address of the region. */
 PERVM(vm_environ) = env;
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_VM_ENVIRON_C */
