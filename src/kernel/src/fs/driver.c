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
#ifndef GUARD_KERNEL_SRC_FS_DRIVER_C
#define GUARD_KERNEL_SRC_FS_DRIVER_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/align.h>
#include <kernel/malloc.h>
#include <kos/types.h>
#include <fs/linker.h>
#include <fs/driver.h>
#include <fs/node.h>
#include <fs/path.h>
#include <kernel/debug.h>
#include <kernel/sections.h>
#include <unwind/debug_line.h>
#include <elf.h>
#include <ctype.h>
#include <string.h>
#include <except.h>
#include <alloca.h>

DECL_BEGIN

STATIC_ASSERT(sizeof(struct driver_param) == 4*sizeof(void *));


INTDEF void ASMCALL _start(void);
INTDEF struct except_handler kernel_except_start[];
INTDEF struct except_handler kernel_except_end[];
INTDEF byte_t kernel_except_size[];
INTDEF byte_t kernel_ehframe_start[];
INTDEF byte_t kernel_ehframe_end[];
INTDEF byte_t kernel_ehframe_size[];
INTDEF byte_t kernel_debug_line_start[];
INTDEF byte_t kernel_debug_line_end[];
INTDEF byte_t kernel_debug_line_size[];

/* Clutch to make sure that the .debug_line section is allocated. */
INTERN ATTR_USED ATTR_SECTION(".data.debug_line.clutch")
void *__debug_line_clutch = NULL;


struct kernel_symbol_entry {
    char const *kse_name; /* [0..1] Symbol name (NULL is used as for hash-chain sentinel) */
    void       *kse_base; /* [1..1][valid_if(kse_name)] Symbol address */
    size_t      kse_size; /* [valid_if(kse_name)] Symbol size */
    uintptr_t   kse_hash; /* [valid_if(kse_name)] ELF-compatible symbol name hash */
};

struct kernel_symbol_table_struct {
    uintptr_t                  kst_mask;   /* Symbol hash mask. */
    struct kernel_symbol_entry kst_tab[1]; /* [kst_mask+1] Symbol hash vector. */
};

INTDEF struct kernel_symbol_table_struct kernel_symbol_table;

PRIVATE struct module_symbol KCALL
kernel_symbol(struct application *__restrict UNUSED(app),
              USER CHECKED char const *__restrict name,
              u32 hash) {
 struct module_symbol result;
 uintptr_t perturb,j;
 /* Search the kernel symbol hash-vector. */
 perturb = j = hash & kernel_symbol_table.kst_mask;
 for (;; j = ((j << 2) + j + perturb + 1),perturb >>= 5) {
  struct kernel_symbol_entry *entry;
  entry = &kernel_symbol_table.kst_tab[j & kernel_symbol_table.kst_mask];
  if unlikely(!entry->kse_name) break;
  if (entry->kse_hash != hash) continue;
  if (strcmp(entry->kse_name,name) != 0) continue;
  result.ms_type = MODULE_SYMBOL_NORMAL;
  result.ms_base = entry->kse_base;
  result.ms_size = entry->kse_size;
  debug_printf("KERNEL_SYMBOL(%q) -> %p\n",name,result.ms_base);
  return result;
 }
 result.ms_type = MODULE_SYMBOL_INVALID;
 return result;
}


PRIVATE struct module_section KCALL
kernel_section(struct application *__restrict UNUSED(app),
               USER CHECKED char const *__restrict name) {
 struct module_section result;
 if (!strcmp(name,".except"))
      return kernel_module.m_sect.m_except;
 if (!strcmp(name,".eh_frame"))
      return kernel_module.m_sect.m_eh_frame;
 /* TODO: .debug_line */

 result.ms_size = 0;
 return result;
}


PRIVATE struct module_type kernel_module_type = {
    .m_flags   = MODULE_TYPE_FNORMAL,
    .m_symbol  = &kernel_symbol,
    .m_section = &kernel_section,
};

struct kernel_directory_entry {
    /* XXX: This structure must be kept in sync with `/src/kernel/include/fs/node.h' */
    ATOMIC_DATA ref_t                 de_refcnt;
    struct directory_entry           *de_next;
    LIST_NODE(struct directory_entry) de_bypos;
    struct PACKED {
        pos_t                         de_start;
        unsigned char                 de_data[16];
    }                                 de_fsdata;
    pos_t                             de_pos;
    union PACKED {
        REF struct inode             *de_virtual;
        ino_t                         de_ino;
    };
    uintptr_t                         de_hash;
    u16                               de_namelen;
    unsigned char                     de_type;
    char                              de_name[COMPILER_STRLEN(KERNEL_BIN_FILENAME)+1];
};

PRIVATE struct kernel_directory_entry kernel_bin_dirent = {
    .de_refcnt  = 1,
    .de_hash    = KERNEL_BIN_FILENAME_HASH,
    .de_namelen = COMPILER_STRLEN(KERNEL_BIN_FILENAME),
    .de_type    = DT_REG,
    .de_name    = KERNEL_BIN_FILENAME
};
PRIVATE struct path kernel_bin_path = {
    .p_refcnt   = 1,
    .p_vfs      = &vfs_kernel,
    .p_parent   = &vfs_kernel.v_root,
    .p_dirent   = (struct directory_entry *)&kernel_bin_dirent,
    .p_lock     = ATOMIC_RWLOCK_INIT,
    .p_flags    = PATH_FCLOSED
};

PRIVATE struct module_debug kernel_debug = {
    .md_debug_line = {
        .ms_base    = (uintptr_t)kernel_debug_line_start,
        .ms_size    = (uintptr_t)kernel_debug_line_size,
        .ms_offset  = 0,
        .ms_type    = SHT_PROGBITS,
        .ms_flags   = SHF_WRITE|SHF_ALLOC,
        .ms_entsize = 0
    },
    .md_data = kernel_debug_line_start
};

PUBLIC struct module kernel_module = {
    .m_refcnt    = 1,
    .m_type      = &kernel_module_type,
    .m_driver    = &kernel_driver,
    .m_path      = &kernel_bin_path,
    .m_fsloc     = NULL,
    .m_fixedbase = 0,
    .m_imagemin  = (uintptr_t)kernel_start,
    .m_imageend  = (uintptr_t)kernel_end_raw,
    .m_entry     = (image_rva_t)(uintptr_t)&_start,
    .m_flags     = MODULE_FFIXED|MODULE_FENTRY|MODULE_FSECTLOADED|MODULE_FSECTLOADING,
    .m_sect      = {
        .m_except = {
            .ms_base    = (image_rva_t)(uintptr_t)kernel_except_start,
            .ms_size    = (size_t)kernel_except_size,
            .ms_type    = SHT_PROGBITS,
            .ms_flags   = SHF_ALLOC,
            .ms_entsize = sizeof(struct except_handler), /* Sure... Why not? (although this isn't a rule...) */
        },
        .m_eh_frame = {
            .ms_base    = (image_rva_t)(uintptr_t)kernel_ehframe_start,
            .ms_size    = (size_t)kernel_ehframe_size,
            .ms_type    = SHT_PROGBITS,
            .ms_flags   = SHF_ALLOC,
        },
    },
    .m_debug     = &kernel_debug,
};


PRIVATE struct driver_specs const kernel_driver_specs = {
    .ds_version = DRIVER_SPECS_VERSION,
    .ds_init    = 0, /* XXX: Pointer to core driver initializers here? */
    .ds_init_sz = 0,
    .ds_fini    = 0,
    .ds_fini_sz = 0,
    .ds_parm    = 0, /* XXX: Implement param support within the kernel itself? */
    .ds_parm_sz = 0,
    .ds_main    = 0
};

DEFINE_INTERN_ALIAS(this_driver,kernel_driver);
PUBLIC struct driver kernel_driver = {
    .d_app = {
        .a_refcnt   = 0x3fffffff,
        .a_weakcnt  = 1,
        .a_mapcnt   = 1,
        .a_loadaddr = 0,
        .a_bounds = {
           .b_min   = (uintptr_t)kernel_start,
           .b_max   = (uintptr_t)kernel_end_raw-1,
        },
        .a_module   = &kernel_module,
        .a_requirec = 0,
        .a_requirev = NULL,
        .a_type     = APPLICATION_TYPE_FDRIVER,
        .a_flags    = APPLICATION_FDIDINIT|APPLICATION_FTRUSTED,
    },
    .d_spec = &kernel_driver_specs
};

/* Sanitize the given commandline:
 *    - Split at spaces (by replacing with \0)
 *    - Correctly interpret '...' and "..." pairs
 *    - Correctly interpret \ being used to escape the next character
 *    - Remove empty arguments (merge consecutive \0 characters at the end)
 * NOTE: The caller must ensure that `cmdline ...+= strlen(cmdline)+2' are writable.
 * Following this, arguments can be enumerated using:
 * >> for (; *cmdline; cmdline = strend(cmdline)+1) {
 * >>     debug_printf("arg = %q\n",cmdline);
 * >> }
 */
PRIVATE void KCALL format_commandline(char *cmdline) {
 char in_quote = 0;
 char *end = strend(cmdline);
continue_parsing:
 for (; cmdline < end; ++cmdline) {
  char ch = *cmdline;
  if (ch == '\\') {
   /* Escape the next character. */
   /* Delete the backslash that was used for escaping. */
   memmove(cmdline,cmdline+1,
          (--end-cmdline)*sizeof(char));
   ++cmdline;
   goto continue_parsing;
  }
  if (ch == in_quote) {
   /* End quotation. */
   in_quote = 0;
delete_char:
   memmove(cmdline,cmdline+1,
          (--end-cmdline)*sizeof(char));
   goto continue_parsing;
  }
  if (ch == '\"' || ch == '\'') {
   /* Start quotation */
   in_quote = ch;
   goto delete_char;
  }
  if (!in_quote && isspace(ch)) {
   /* End of argument. */
   *cmdline++ = '\0';
   /* Delete consecutive spaces. */
   while (cmdline != end && isspace(*cmdline)) {
    memmove(cmdline,cmdline+1,
           (--end-cmdline)*sizeof(char));
   }
   goto continue_parsing;
  }
 }
 /* Add trailing NUL-characters.
  * NOTE: We add to as a marker to terminate the strend()-iterator */
 cmdline[0] = '\0';
 cmdline[1] = '\0';
}



INTERN ATTR_FREETEXT void KCALL
kernel_relocate_commandline(void) {
 size_t cmdline_length;
 if (!kernel_driver.d_cmdline)
      return; /* Nothing to do here. */
 cmdline_length = strlen(kernel_driver.d_cmdline);
 /* Duplicate the kernel commandline into GFP_SHARED memory. */
 kernel_driver.d_cmdline = (char *)memcpy(kmalloc((cmdline_length+2)*sizeof(char),
                                                   GFP_SHARED),
                                          kernel_driver.d_cmdline,
                                         (cmdline_length+1)*sizeof(char));
 kernel_driver.d_cmdline[cmdline_length+2] = 0;
}

PRIVATE unsigned int KCALL
serve_driver_params(struct driver_param *__restrict params,
                    size_t num_params, uintptr_t load_addr,
                    unsigned int argc, char **argv);


INTDEF struct driver_param kernel_coredriver_param_start[];
INTDEF uintptr_t kernel_coredriver_param_count[];

INTERN ATTR_FREETEXT void KCALL
kernel_eval_commandline(void) {
 unsigned int argc; char **argv,*cmdline;
 if (!kernel_driver.d_cmdline)
      return; /* Nothing to do here. */
 debug_printf("[BOOT] Kernel command line: %q\n",kernel_driver.d_cmdline);
 /* Format the kernel commandline. */
 format_commandline(kernel_driver.d_cmdline);
 cmdline = kernel_driver.d_cmdline;
 for (argc = 0; *cmdline; cmdline = strend(cmdline)+1) ++argc;
 argv = (char **)malloca(argc*sizeof(char *));
 cmdline = kernel_driver.d_cmdline;
 for (argc = 0; *cmdline; cmdline = strend(cmdline)+1)
      argv[argc++] = cmdline;
 /* Serve kernel commandline parameters. */
 serve_driver_params(kernel_coredriver_param_start,
                    (size_t)kernel_coredriver_param_count,0,
                     argc,argv);
 freea(argv);
}







PRIVATE unsigned int KCALL
serve_driver_params(struct driver_param *__restrict params,
                    size_t num_params, uintptr_t load_addr,
                    unsigned int argc, char **argv) {
 unsigned int i = 0;
next_arg:
 for (; i < argc; ++i) {
  char *arg = argv[i];
  struct driver_param *iter = params;
  struct driver_param *end = params+num_params;
  if (arg[0] == '-') ++arg;
  if (arg[0] == '-') ++arg;
  for (; iter < end; ++iter) {
   char *name = DRIVER_PARAM_NAME(load_addr,iter);
   size_t namelen = strlen(name);
   if (memcmp(arg,name,namelen*sizeof(char)) != 0) continue;
   switch (iter->dp_type) {

   case DRIVER_PARAM_TYPE_OPTION:
    /* Invoke a parameter option callback. */
    if (arg[namelen] != '=') continue;
    SAFECALL_KCALL_VOID_1(*DRIVER_PARAM_HAND(load_addr,iter),
                           arg+namelen+1);
    break;

   case DRIVER_PARAM_TYPE_FLAG:
    /* Invoke a parameter flag callback. */
    SAFECALL_KCALL_VOID_0(*DRIVER_PARAM_FLAGHAND(load_addr,iter));
    break;

   default: continue;
   }
   /* Delete this argument. */
   memmove(&argv[i],&argv[i+1],
          (--argc-i)*sizeof(char *));
   goto next_arg;
  }
 }
 return argc;
}


PRIVATE void KCALL
driver_unbind_globals(struct driver *__restrict self) {
 /* TODO */
}

PUBLIC REF struct driver *KCALL
kernel_getmod(struct module *__restrict mod) {
 REF struct driver *COMPILER_IGNORE_UNINITIALIZED(result);
again:
 vm_acquire_read(&vm_kernel);
 TRY {
  struct vmapps *apps; size_t i;
  apps = FORVM(&vm_kernel,vm_apps);
  assert(apps);
  for (i = 0; i < apps->va_count; ++i) {
   result = (REF struct driver *)apps->va_apps[i];
   if (result->d_app.a_module != mod) continue;
   if unlikely(result->d_app.a_flags & APPLICATION_FCLOSING) continue;
   if unlikely(!driver_tryincref(result)) continue;
   goto done;
  }
  result = NULL;
done:;
 } FINALLY {
  if (vm_release_read(&vm_kernel))
      goto again;
 }
 return result;
}

PRIVATE void KCALL
exec_callback(module_callback_t func,
              void *UNUSED(arg)) {
 SAFECALL_KCALL_VOID_0(*func);
}


PRIVATE char const *module_runpath = "/mod";
DEFINE_DRIVER_STRING(module_runpath,"module-runpath");


PUBLIC ATTR_RETNONNULL REF struct driver *
KCALL kernel_insmod(struct module *__restrict mod,
                    bool *pwas_newly_loaded,
                    USER CHECKED char const *module_commandline,
                    size_t module_commandline_length) {
 REF struct driver *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(result);
again:
 vm_acquire_read(&vm_kernel);
 TRY {
  struct module_patcher EXCEPT_VAR patcher;
  /* Load drivers as though they were dependencies of the kernel itself. */
  patcher.mp_root     = (struct module_patcher *)&patcher;
  patcher.mp_prev     = NULL;
  patcher.mp_app      = &kernel_driver.d_app;
  patcher.mp_requirec = 0;
  patcher.mp_requirea = 0;
  patcher.mp_requirev = NULL;
  patcher.mp_runpath  = module_runpath;
  patcher.mp_altpath  = NULL;
  patcher.mp_flags    = DL_OPEN_FNORMAL|DL_OPEN_FGLOBAL;
  patcher.mp_apptype  = APPLICATION_TYPE_FDRIVER;
  patcher.mp_appflags = APPLICATION_FTRUSTED;
  TRY {
   /* Open the module user the new patcher. */
   result = (REF struct driver *)patcher_require((struct module_patcher *)&patcher,mod);
   driver_incref(result);
  } FINALLY {
   patcher_fini((struct module_patcher *)&patcher);
  }
 } FINALLY {
  if (vm_release_read(&vm_kernel))
      goto again;
 }
 if (ATOMIC_FETCHOR(result->d_app.a_flags,APPLICATION_FDIDINIT) &
                                          APPLICATION_FDIDINIT) {
  *pwas_newly_loaded = false;
 } else {
  TRY {
   struct module_symbol sym;
   struct driver_specs *EXCEPT_VAR spec;
   uintptr_t load_addr; size_t i;
   image_rva_t *vec;
   /* Copy the driver commandline. */
   if (module_commandline_length) {
    result->d_cmdline = (char *)kmalloc((module_commandline_length+2)*
                                         sizeof(char),GFP_SHARED);
    COMPILER_READ_BARRIER();
    /* Copy the commandline from user-space (CAUTION: SEGFAULT) */
    memcpy(result->d_cmdline,module_commandline,
           module_commandline_length*sizeof(char));
    COMPILER_READ_BARRIER();
    /* Ensure NUL-termination (don't rely on user-space
     * actually being NUL-terminated, which may no longer
     * be the case since user_strlen(), since user-space
     * may have modified the commandline from another thread). */
    result->d_cmdline[module_commandline_length] = '\0';
   }
   debug_printf("[MOD] Driver `%[path]' loaded at %p...%p (%q)\n",
                mod->m_path,
                APPLICATION_MAPMIN(&result->d_app),
                APPLICATION_MAPMAX(&result->d_app),
                result->d_cmdline ? result->d_cmdline : "");
   /* Format the commandline to split it into individual arguments. */
   *pwas_newly_loaded = true;
   sym = application_dlsym(&result->d_app,
                           "__$$OS$driver_specs");
   if unlikely(sym.ms_type == MODULE_SYMBOL_INVALID)
      error_throw(E_INVALID_ARGUMENT);
   result->d_spec = spec = (struct driver_specs *)sym.ms_base;
   if unlikely(result->d_spec->ds_version != DRIVER_SPECS_VERSION)
      error_throw(E_INVALID_ARGUMENT);
   load_addr = result->d_app.a_loadaddr;
   TRY {
    char **EXCEPT_VAR argv = NULL;
    unsigned int argc;
    /* Verify driver spec pointers (prevent system crashes due to corrupt drivers).
     * NOTE: These are split into individual calls to `error_throw()', so that a
     *       traceback can quickly reveil which of these checks got triggered. */
    if ((spec->ds_free_sz &&
        (spec->ds_free < FLOORDIV(mod->m_imagemin,PAGESIZE) ||
         spec->ds_free+spec->ds_free_sz < spec->ds_free ||
         spec->ds_free+spec->ds_free_sz > CEILDIV(mod->m_imageend,PAGESIZE))))
         error_throw(E_INVALID_ARGUMENT);
    if ((spec->ds_init_sz &&
        (spec->ds_init < mod->m_imagemin ||
         spec->ds_init+spec->ds_init_sz*sizeof(image_rva_t) < spec->ds_init ||
         spec->ds_init+spec->ds_init_sz*sizeof(image_rva_t) > mod->m_imageend)))
         error_throw(E_INVALID_ARGUMENT);
    if ((spec->ds_fini_sz &&
        (spec->ds_fini < mod->m_imagemin ||
         spec->ds_fini+spec->ds_fini_sz*sizeof(image_rva_t) < spec->ds_fini ||
         spec->ds_fini+spec->ds_fini_sz*sizeof(image_rva_t) > mod->m_imageend)))
         error_throw(E_INVALID_ARGUMENT);
    if ((spec->ds_parm_sz &&
        (spec->ds_parm < mod->m_imagemin ||
         spec->ds_parm+spec->ds_parm_sz*sizeof(image_rva_t) < spec->ds_parm ||
         spec->ds_parm+spec->ds_parm_sz*sizeof(image_rva_t) > mod->m_imageend)))
         error_throw(E_INVALID_ARGUMENT);
    if ((spec->ds_main &&
        (spec->ds_main < mod->m_imagemin ||
         spec->ds_main > mod->m_imageend)))
         error_throw(E_INVALID_ARGUMENT);

    /* Call ELF constructors. */
    if (mod->m_type->m_enuminit)
        SAFECALL_KCALL_VOID_3(mod->m_type->m_enuminit,&result->d_app,&exec_callback,NULL);
    TRY {
     /* Evaluate the module commandline. */
     char *cmdline = result->d_cmdline;
     format_commandline(result->d_cmdline);
     for (argc = 1; *cmdline; cmdline = strend(cmdline)+1) ++argc;
     argv = (char **)malloca(argc*sizeof(char *));
     cmdline = result->d_cmdline;
     for (argc = 0; *cmdline; cmdline = strend(cmdline)+1)
          argv[argc++] = cmdline;
     /* Serve driver parameters. */
     argc = serve_driver_params((struct driver_param *)(load_addr + spec->ds_parm),
                                 spec->ds_parm_sz,load_addr,argc,argv);
     argv[argc] = NULL; /* Add a trailing NULL-argument */

#if 0
     debug_printf("spec->ds_free    = %p\n",spec->ds_free);
     debug_printf("spec->ds_free_sz = %p\n",spec->ds_free_sz);
     debug_printf("spec->ds_init    = %p\n",spec->ds_init);
     debug_printf("spec->ds_init_sz = %p\n",spec->ds_init_sz);
     debug_printf("spec->ds_fini    = %p\n",spec->ds_fini);
     debug_printf("spec->ds_fini_sz = %p\n",spec->ds_fini_sz);
     debug_printf("spec->ds_parm    = %p\n",spec->ds_parm);
     debug_printf("spec->ds_parm_sz = %p\n",spec->ds_parm_sz);
     debug_printf("spec->ds_main    = %p\n",spec->ds_main);
#endif

     vec = (image_rva_t *)(load_addr + spec->ds_init);
     for (i = 0; i < spec->ds_init_sz; ++i)
         SAFECALL_KCALL_VOID_0(*(driver_init_t)(load_addr + vec[i]));
     if (spec->ds_main != 0) {
      /* Pass the remaining arguments to the driver main() function. */
      SAFECALL_KCALL_VOID_2(*(driver_main_t)(load_addr + spec->ds_main),
                            argc,argv);
     }
     /* Unmap the driver's .free if it exists. */
     if (spec->ds_free_sz != 0) {
      /* Make sure that the .free segment is properly aligned. */
      vm_unmap(VM_ADDR2PAGE(load_addr)+spec->ds_free,spec->ds_free_sz,
               VM_UNMAP_TAG|VM_UNMAP_NOEXCEPT|VM_UNMAP_SYNC,&result->d_app);
     }
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     if (argv) freea(argv);
     size_t i;
     driver_unbind_globals(result);
     vec = (image_rva_t *)(load_addr + spec->ds_fini);
     i = spec->ds_fini_sz;
     while (i--) SAFECALL_KCALL_VOID_0(*(driver_fini_t)(load_addr + vec[i]));
     error_rethrow();
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    driver_unbind_globals(result);
    if (!(ATOMIC_FETCHOR(result->d_app.a_flags,APPLICATION_FDIDFINI) & APPLICATION_FDIDFINI)) {
     /* Call ELF destructors. */
     if (mod->m_type->m_enumfini)
         SAFECALL_KCALL_VOID_3(*mod->m_type->m_enumfini,&result->d_app,&exec_callback,NULL);
    }
    error_rethrow();
   }

  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   driver_unbind_globals(result);
   vm_unmap(VM_ADDR2PAGE(APPLICATION_MAPBEGIN(&result->d_app)),
            VM_SIZE2PAGES(APPLICATION_MAPSIZE(&result->d_app)),
            VM_UNMAP_NOEXCEPT|VM_UNMAP_SYNC|VM_UNMAP_TAG,
           &result->d_app);
   driver_decref(result);
   error_rethrow();
  }
 }
 return result;
}

PUBLIC void KCALL
kernel_delmod(struct driver *__restrict app) {
 driver_unbind_globals(app);
 /* TODO */
 error_throw(E_NOT_IMPLEMENTED);
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_DRIVER_C */
