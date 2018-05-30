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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_ELF_RELOC_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_ELF_RELOC_H 1
#ifdef __INTELLISENSE__
#define _KOS_SOURCE 1
#endif

#include <hybrid/compiler.h>
#include <hybrid/align.h>
#include <hybrid/section.h>
#include <kos/types.h>
#include <kernel/paging.h>
#include <kernel/vm.h>
#include <kernel/debug.h>
#include <elf.h>
#include <except.h>
#include <string.h>
#include <fs/linker.h>
#include <fs/path.h>
#include <fs/node.h>
#include "elf.h"

DECL_BEGIN

#ifdef __x86_64__
#define Elf64_PerformRelocation  Elf_PerformRelocation
#else
#define Elf32_PerformRelocation  Elf_PerformRelocation
#endif



FORCELOCAL bool KCALL
Elf32_PerformRelocation(byte_t *reladdr,
                        Elf32_Word r_info,
#ifdef __x86_64__
                        Elf32_Sword r_addend,
#endif
                        struct application *__restrict app,
                        u32 loadaddr,
                        u32 image_end,
                        byte_t *symbol_table,
                        Elf32_Word symbol_count,
                        Elf32_Word symbol_size,
                        char *string_table,
#ifdef CONFIG_HIGH_KERNEL
                        char *string_table_end,
#endif
                        struct module_patcher *__restrict patcher,
                        bool load_as_symbolic
                
                        ) {
 char const *sym_name; u32 sym_hash;
 bool extern_sym = false;
 Elf32_Sym *sym; u32 value;
 unsigned int symid = ELF32_R_SYM(r_info);
 unsigned int type  = ELF32_R_TYPE(r_info);
 if (type == R_386_TLS_DTPMOD32 || type == R_386_TLS_DTPOFF32) {
  struct kernel_symbol_info symbol;
  if unlikely(symid >= symbol_count)
     error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADSYMBOL);
  sym   = (Elf32_Sym *)(symbol_table + (symid * symbol_size));
  symbol.si_symbol.ds_base = (void *)(uintptr_t)(loadaddr + sym->st_value);
  if (sym->st_shndx != SHN_UNDEF) {
   /* Link against local symbols by default. */
   if (ELF32_ST_TYPE(sym->st_info) == STT_TLS)
       symbol.si_symbol.ds_base = (void *)(uintptr_t)(sym->st_value + (app->a_tlsoff - MODULE_TLSSIZE(app->a_module)));
   else if (sym->st_shndx == SHN_ABS)
       symbol.si_symbol.ds_base = (void *)(uintptr_t)sym->st_value;
got_symbol_ex:
   symbol.si_symbol.ds_size = sym->st_size;
   symbol.si_defmod         = app;
  } else {
   sym_name = string_table+sym->st_name;
#ifdef CONFIG_HIGH_KERNEL
   if (sym_name >= string_table_end)
       error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADSTRING);
#else
   if (sym_name < strtab)
       error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADSTRING);
#endif
   /* Find the symbol within shared libraries. */
   sym_hash = patcher_symhash(sym_name);
   symbol = patcher_syminfo(patcher,sym_name,sym_hash,false);
   /* NOTE: Weak symbols are linked as NULL when not found. */
   if (symbol.si_symbol.ds_type == MODULE_SYMBOL_INVALID) {
    if (ELF_ST_BIND(sym->st_info) != STB_WEAK) goto patch_failed;
    symbol.si_symbol.ds_base = (void *)0;
    goto got_symbol_ex;
   }
  }
  switch (type) {

  case R_386_TLS_DTPMOD32:
   vm_cow_ptr(reladdr);
   if (symbol.si_defmod->a_flags & APPLICATION_FHASTLS) {
    value   = (symbol.si_defmod->a_tlsoff - MODULE_TLSSIZE(symbol.si_defmod->a_module));
    value <<= 1;
    /* Libc's version of `__tls_get_addr()' uses
     * this bit to detect static TLS allocation. */
    value  |= 1;
    *(uintptr_t *)reladdr = value;
   } else {
    value = symbol.si_defmod->a_loadaddr;
    *(uintptr_t *)reladdr = CEIL_ALIGN(value,2);
   }
   return true;

  case R_386_TLS_DTPOFF32:
   value  = (uintptr_t)symbol.si_symbol.ds_base;
   value -= (symbol.si_defmod->a_tlsoff - MODULE_TLSSIZE(symbol.si_defmod->a_module));
   vm_cow_ptr(reladdr);
   *(uintptr_t *)reladdr = value;
   return true;

  default: __builtin_unreachable();
  }
 }
 if (type == R_386_RELATIVE) {
  vm_cow_ptr(reladdr);
  *(uintptr_t *)reladdr += (uintptr_t)loadaddr;
  return true;
 }
 if unlikely(symid >= symbol_count)
    error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADSYMBOL);
 sym   = (Elf32_Sym *)(symbol_table + (symid * symbol_size));
 value = (u32)loadaddr + sym->st_value;
 if (sym->st_shndx != SHN_UNDEF) {
  /* Link against local symbols by default. */
  if (ELF32_ST_TYPE(sym->st_info) == STT_TLS)
      value = sym->st_value + (app->a_tlsoff - MODULE_TLSSIZE(app->a_module));
  else if (sym->st_shndx == SHN_ABS)
      value = (uintptr_t)sym->st_value;
 } else {
find_extern:
  if (sym->st_shndx != SHN_UNDEF && load_as_symbolic) {
   /* Use symbolic symbol resolution (Keep using the private symbol version). */
   goto got_symbol;
  }

  sym_name = string_table+sym->st_name;
#ifdef CONFIG_HIGH_KERNEL
  if (sym_name >= string_table_end)
      error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADSTRING);
#else
  if (sym_name < strtab)
      error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADSTRING);
#endif

  /* NOTE: Don't search the module itself. - We already know its
   *       symbol address and can link them below without a lookup. */
  if ((patcher->mp_flags & DL_OPEN_FDEEPBIND) &&
      (sym->st_shndx != SHN_UNDEF)) {
   value = (u32)loadaddr+sym->st_value;
   if (ELF32_ST_TYPE(sym->st_info) == STT_TLS)
       value = sym->st_value + (app->a_tlsoff - MODULE_TLSSIZE(app->a_module));
   else if (sym->st_shndx == SHN_ABS)
       value = (u32)sym->st_value;
  } else {
   /* Find the symbol within shared libraries. */
   sym_hash = patcher_symhash(sym_name);
   value = (u32)(uintptr_t)patcher_symaddr(patcher,sym_name,sym_hash,
                                           sym->st_shndx != SHN_UNDEF);
   /* NOTE: Weak symbols are linked as NULL when not found. */
   if (!value) {
    if (ELF_ST_BIND(sym->st_info) == STB_WEAK) goto got_symbol;
patch_failed:
    debug_printf(COLDSTR("[ELF] Failed to patch symbol %q in %q\n"),
                 sym_name,app->a_module->m_path->p_dirent->de_name);
    error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_NOSYMBOL);
   }
  }
got_symbol:
  extern_sym = true;
 }

#ifdef CONFIG_ELF_USING_RELA
 value += r_addend;
#endif
 switch (type) {

 case R_386_NONE: break;
 case R_386_8:
  vm_cowb(reladdr);
  *(u8 *)reladdr += (u8)value;
  break;
 case R_386_PC8:
  vm_cowb(reladdr);
  *(u8 *)reladdr += (u8)((uintptr_t)value - (uintptr_t)reladdr);
  break;
 case R_386_16:
  vm_coww(reladdr);
  *(u16 *)reladdr += (u16)value;
  break;
 case R_386_PC16:
  vm_coww(reladdr);
  *(u16 *)reladdr += (u16)((uintptr_t)value - (uintptr_t)reladdr);
  break;
 case R_386_32:
  vm_cowl(reladdr);
  *(u32 *)reladdr += (u32)value;
  break;
 case R_386_PC32:
  vm_cowl(reladdr);
  *(u32 *)reladdr += (u32)((uintptr_t)value - (uintptr_t)reladdr);
  break;
 case R_386_COPY:
  if (!extern_sym) goto find_extern;
  if (patcher->mp_apptype != APPLICATION_TYPE_FDRIVER) {
   /* Make sure not to copy kernel data.
    * NOTE: We can't just do 'DATA_CHECK(rel_value,sym->st_size)',
    *       because the symbol may be apart of a different module.
    *       But if it is, it would be too expensive to search
    *       the potentially _very_ large chain of loaded modules
    *       for the one containing `rel_value'.
    * >> So instead we rely on the fact that the caller is capturing
    *    page faults, and simply go ahead and copy the data.
    *    If it fails, the caller will correctly determine `-EFAULT'
    *    and everything can go on as normal without us having to
    *    waste a whole bunch of time validating a pointer. */
   uintptr_t sym_end;
   if (__builtin_add_overflow(value,sym->st_size,&sym_end))
       goto symend_overflow;
   if (sym_end > KERNEL_BASE) {
    char *sym_name;
symend_overflow:
    sym_name = string_table + sym->st_name;
    if (sym_name < string_table || sym_name >= string_table_end)
        sym_name = "??" "?";
    debug_printf(COLDSTR("[ELF] Faulty copy-relocation against %q targeting %p...%p in kernel space from `%q'\n"),
                 sym_name,value,sym_end-1,app->a_module->m_path->p_dirent->de_name);
    error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADSYMADDR);
   }
  }
  vm_cow(reladdr,sym->st_size);
  memcpy((void *)reladdr,
         (void *)(uintptr_t)value,
          sym->st_size);
  break;

 case R_386_GLOB_DAT:
  if (!extern_sym) goto find_extern;
  ATTR_FALLTHROUGH
 case R_386_JMP_SLOT:
  vm_cow_ptr(reladdr);
  *(uintptr_t *)reladdr = (uintptr_t)value;
  break;

 case R_386_TLS_TPOFF:
  vm_cowl(reladdr);
  *(u32 *)reladdr += (u32)value;
  break;
 case R_386_TLS_TPOFF32:
  vm_cowl(reladdr);
  *(u32 *)reladdr -= (u32)value;
  break;

#define R_386_GOT32        3            /* 32 bit GOT entry */
#define R_386_PLT32        4            /* 32 bit PLT address */
#define R_386_GOTOFF       9            /* 32 bit offset to GOT */
#define R_386_GOTPC        10           /* 32 bit PC relative offset to GOT */
#define R_386_32PLT        11
#define R_386_TLS_IE       15           /* Address of GOT entry for static TLS block offset */
#define R_386_TLS_GOTIE    16           /* GOT entry for static TLS block offset */
#define R_386_TLS_LE       17           /* Offset relative to static TLS block */

#define R_386_TLS_GD       18           /* Direct 32 bit for GNU version of general dynamic thread local data */
#define R_386_TLS_LDM      19           /* Direct 32 bit for GNU version of local dynamic thread local data in LE code */
#define R_386_TLS_GD_32    24           /* Direct 32 bit for general dynamic thread local data */
#define R_386_TLS_GD_PUSH  25           /* Tag for pushl in GD TLS code */
#define R_386_TLS_GD_CALL  26           /* Relocation for call to __tls_get_addr() */
#define R_386_TLS_GD_POP   27           /* Tag for popl in GD TLS code */
#define R_386_TLS_LDM_32   28           /* Direct 32 bit for local dynamic thread local data in LE code */
#define R_386_TLS_LDM_PUSH 29           /* Tag for pushl in LDM TLS code */
#define R_386_TLS_LDM_CALL 30           /* Relocation for call to __tls_get_addr() in LDM code */
#define R_386_TLS_LDM_POP  31           /* Tag for popl in LDM TLS code */
#define R_386_TLS_LDO_32   32           /* Offset relative to TLS block */
#define R_386_TLS_IE_32    33           /* GOT entry for negated static TLS block offset */
#define R_386_TLS_LE_32    34           /* Negated offset relative to static TLS block */

 default:
  return false;
 }
 return true;
}


#ifdef __x86_64__
FORCELOCAL bool KCALL
Elf64_PerformRelocation(byte_t *reladdr,
                        Elf64_Word r_info,
                        Elf64_Sword r_addend,
                        struct application *__restrict app,
                        uintptr_t loadaddr,
                        uintptr_t image_end,
                        byte_t *symbol_table,
                        Elf64_Word symbol_count,
                        Elf64_Word symbol_size,
                        char *string_table,
#ifdef CONFIG_HIGH_KERNEL
                        char *string_table_end,
#endif
                        struct module_patcher *__restrict patcher,
                        bool load_as_symbolic
                
                        ) {
 char const *sym_name; u32 sym_hash;
 bool extern_sym = false;
 Elf64_Sym *sym; u64 value;
 unsigned int symid = ELF64_R_SYM(r_info);
 unsigned int type  = ELF64_R_TYPE(r_info);
 if (type == R_X86_64_RELATIVE ||
     type == R_X86_64_RELATIVE64) {
  vm_cow_ptr(reladdr);
  *(uintptr_t *)reladdr += (uintptr_t)loadaddr;
  return true;
 }
 if unlikely(symid >= symbol_count)
    error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADSYMBOL);
 sym   = (Elf64_Sym *)(symbol_table + (symid * symbol_size));
 value = (u64)loadaddr + sym->st_value;
 if (sym->st_shndx != SHN_UNDEF) {
  /* Link against local symbols by default. */
  if (ELF64_ST_TYPE(sym->st_info) == STT_TLS)
      value = sym->st_value + (app->a_tlsoff - MODULE_TLSSIZE(app->a_module));
  else if (sym->st_shndx == SHN_ABS)
      value = (uintptr_t)sym->st_value;
 } else {
find_extern:
  if (sym->st_shndx != SHN_UNDEF && load_as_symbolic) {
   /* Use symbolic symbol resolution (Keep using the private symbol version). */
   goto got_symbol;
  }

  sym_name = string_table+sym->st_name;
#ifdef CONFIG_HIGH_KERNEL
  if (sym_name >= string_table_end)
      error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADSTRING);
#else
  if (sym_name < strtab)
      error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADSTRING);
#endif

  /* NOTE: Don't search the module itself. - We already know its
   *       symbol address and can link them below without a lookup. */
  if ((patcher->mp_flags & DL_OPEN_FDEEPBIND) &&
      (sym->st_shndx != SHN_UNDEF)) {
   value = (u64)loadaddr+sym->st_value;
   if (ELF64_ST_TYPE(sym->st_info) == STT_TLS)
       value = sym->st_value + (app->a_tlsoff - MODULE_TLSSIZE(app->a_module));
   else if (sym->st_shndx == SHN_ABS)
       value = (u64)sym->st_value;
  } else {
   /* Find the symbol within shared libraries. */
   sym_hash = patcher_symhash(sym_name);
   value = (u64)patcher_symaddr(patcher,sym_name,sym_hash,
                                sym->st_shndx != SHN_UNDEF);
   /* NOTE: Weak symbols are linked as NULL when not found. */
   if (!value) {
    if (ELF_ST_BIND(sym->st_info) == STB_WEAK) goto got_symbol;
    debug_printf(COLDSTR("[ELF] Failed to patch symbol %q in %q\n"),
                 sym_name,app->a_module->m_path->p_dirent->de_name);
    error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_NOSYMBOL);
   }
  }
got_symbol:
  extern_sym = true;
 }
 switch (type) {

 case R_X86_64_NONE: break;

 case R_X86_64_8:
  vm_cowb(reladdr);
  *(u8 *)reladdr += (u8)value;
  break;
 case R_X86_64_PC8:
  vm_cowb(reladdr);
  *(u8 *)reladdr += (u8)((uintptr_t)value - (uintptr_t)reladdr);
  break;
 case R_X86_64_16:
  vm_coww(reladdr);
  *(u16 *)reladdr += (u16)value;
  break;
 case R_X86_64_PC16:
  vm_coww(reladdr);
  *(u16 *)reladdr += (u16)((uintptr_t)value - (uintptr_t)reladdr);
  break;
 case R_X86_64_32:
 case R_X86_64_32S:
  vm_cowl(reladdr);
  *(u32 *)reladdr += (u32)value;
  break;
 case R_X86_64_PC32:
  vm_cowl(reladdr);
  *(u32 *)reladdr += (u32)((uintptr_t)value - (uintptr_t)reladdr);
  break;
 case R_X86_64_64:
  vm_cowl(reladdr);
  *(u64 *)reladdr += (u64)value;
  break;
 case R_X86_64_PC64:
  vm_cowl(reladdr);
  *(u64 *)reladdr += (u64)((uintptr_t)value - (uintptr_t)reladdr);
  break;


 case R_X86_64_COPY:
  if (!extern_sym) goto find_extern;
  if (patcher->mp_apptype != APPLICATION_TYPE_FDRIVER) {
   /* Make sure not to copy kernel data.
    * NOTE: We can't just do 'DATA_CHECK(rel_value,sym->st_size)',
    *       because the symbol may be apart of a different module.
    *       But if it is, it would be too expensive to search
    *       the potentially _very_ large chain of loaded modules
    *       for the one containing `rel_value'.
    * >> So instead we rely on the fact that the caller is capturing
    *    page faults, and simply go ahead and copy the data.
    *    If it fails, the caller will correctly determine `-EFAULT'
    *    and everything can go on as normal without us having to
    *    waste a whole bunch of time validating a pointer. */
   uintptr_t sym_end;
   if (__builtin_add_overflow(value,sym->st_size,&sym_end))
       goto symend_overflow;
   if (sym_end > KERNEL_BASE) {
    char *sym_name;
symend_overflow:
    sym_name = string_table + sym->st_name;
    if (sym_name < string_table || sym_name >= string_table_end)
        sym_name = "??" "?";
    debug_printf(COLDSTR("[ELF] Faulty copy-relocation against %q targeting %p...%p in kernel space from `%q'\n"),
                 sym_name,value,sym_end-1,app->a_module->m_path->p_dirent->de_name);
    error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_NOSYMBOL);
   }
  }
  vm_cow(reladdr,sym->st_size);
  memcpy((void *)reladdr,
         (void *)(uintptr_t)value,
          sym->st_size);
  break;

 case R_X86_64_GLOB_DAT:
  if (!extern_sym) goto find_extern;
  ATTR_FALLTHROUGH
 case R_X86_64_JUMP_SLOT:
  vm_cow_ptr(reladdr);
  *(uintptr_t *)reladdr = (uintptr_t)value;
  break;

#define R_X86_64_GOT32          3       /* 32 bit GOT entry */
#define R_X86_64_PLT32          4       /* 32 bit PLT address */
#define R_X86_64_GOTPCREL       9       /* 32 bit signed PC relative offset to GOT */
#define R_X86_64_DTPMOD64       16      /* ID of module containing symbol */
#define R_X86_64_DTPOFF64       17      /* Offset in module's TLS block */
#define R_X86_64_TPOFF64        18      /* Offset in initial TLS block */
#define R_X86_64_TLSGD          19      /* 32 bit signed PC relative offset to two GOT entries for GD symbol */
#define R_X86_64_TLSLD          20      /* 32 bit signed PC relative offset to two GOT entries for LD symbol */
#define R_X86_64_DTPOFF32       21      /* Offset in TLS block */
#define R_X86_64_GOTTPOFF       22      /* 32 bit signed PC relative offset to GOT entry for IE symbol */
#define R_X86_64_TPOFF32        23      /* Offset in initial TLS block */
#define R_X86_64_GOTOFF64       25      /* word64 S + A - GOT */
#define R_X86_64_GOTPC32        26      /* word32 GOT + A - P */
#define R_X86_64_GOT64          27      /* word64 G + A */
#define R_X86_64_GOTPCREL64     28      /* word64 G + GOT - P + A */
#define R_X86_64_GOTPC64        29      /* word64 GOT - P + A */
#define R_X86_64_GOTPLT64       30      /* word64 G + A */
#define R_X86_64_PLTOFF64       31      /* word64 L - GOT + A */
#define R_X86_64_SIZE32         32      /* word32 Z + A */
#define R_X86_64_SIZE64         33      /* word64 Z + A */
#define R_X86_64_GOTPC32_TLSDESC 34     /* word32 */
#define R_X86_64_GOTPC32_TLSDEC R_X86_64_GOTPC32_TLSDESC
#define R_X86_64_TLSDESC_CALL   35      /* none */
#define R_X86_64_TLSDESC        36      /* word64,word64 */
#define R_X86_64_IRELATIVE      37
#define R_X86_64_PC32_BND       39
#define R_X86_64_PLT32_BND      40
#define R_X86_64_GOTPCRELX      41
#define R_X86_64_REX_GOTPCRELX	42

 default:
  return false;
 }
 return true;
}
#endif



DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_ELF_RELOC_H */
