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
#ifndef GUARD_KERNEL_SRC_CORE_ELF_C
#define GUARD_KERNEL_SRC_CORE_ELF_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/safecall.h>
#include <hybrid/minmax.h>
#include <hybrid/host.h>
#include <hybrid/align.h>
#include <hybrid/section.h>
#include <kernel/debug.h>
#include <kernel/vm.h>
#include <kernel/malloc.h>
#include <fs/driver.h>
#include <fs/linker.h>
#include <fs/path.h>
#include <fs/node.h>
#include <string.h>
#include <except.h>
#include <assert.h>
#include <endian.h>

#include <elf.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/elf.h>
#include <i386-kos/elf-reloc.h>
#else
#error "Unsupported architecture"
#endif

DECL_BEGIN

#ifdef CONFIG_ELF_SUPPORT_CLASS3264
#if (ELFCLASS != ELFCLASS64) || (__SIZEOF_POINTER__ <= 4)
#error "CONFIG_ELF_SUPPORT_CLASS3264 can only be used if the preferred host is a 64-bit host"
#endif
#endif

#ifdef CONFIG_ELF_USING_RELA
#define RELINFO_COUNT 3
#else
#define RELINFO_COUNT 2
#endif

#ifndef ELF_HEADER_SUPPORTED
#ifndef EM_HOST
#error "Arch must either define `EM_HOST' or `ELF_HEADER_SUPPORTED(x)'"
#endif
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ELF_HEADER_SUPPORTED(x) \
 ((x).e_ident[EI_DATA] == ELFDATA2LSB && \
  (x).e_ident[EI_CLASS] == ELFCLASS && \
  (x).e_machine == EM_HOST)
#else
#define ELF_HEADER_SUPPORTED(x) \
 ((x).e_ident[EI_DATA] == ELFDATA2MSB && \
  (x).e_ident[EI_CLASS] == ELFCLASS && \
  (x).e_machine == EM_HOST)
#endif
#endif


typedef Elf_Word Elf_String; /* Offset into the string table `di_strtab' (when present) */

typedef struct {
    image_rva_t        ri_rel;               /* [valid_if(ri_relsiz != 0)][const] The address of the relocations table. */
    Elf_Word           ri_relsiz;            /* [const] The size (in bytes) of the relocations table. */
    Elf_Word           ri_relent;            /* [valid_if(ri_relsiz != 0)][OR(>= sizeof(Elf_Rel),>= sizeof(Elf_Rela))]
                                              * [const] The size of a single relocation entry. */
} RelInfo;

typedef struct {
    image_rva_t        di_init;              /* [0..1][const] Address of an init-function. */
    image_rva_t        di_fini;              /* [0..1][const] Address of a fini-function. */
    image_rva_t        di_preinit_array;     /* [valid_if(di_preinit_array_siz != 0)][const] Address of an array of init-functions. */
    image_rva_t        di_init_array;        /* [valid_if(di_init_array_siz != 0)][const] Address of an array of init-functions. */
    image_rva_t        di_fini_array;        /* [valid_if(di_fini_array_siz != 0)][const] Address of an array of fini-functions. */
    image_rva_t        di_strtab;            /* [valid_if(di_strsiz != 0)][const] The address of the string table. */
    image_rva_t        di_hashtab;           /* [valid_if(di_hashsiz != 0)][const] The address of the hash table. */
    image_rva_t        di_symtab;            /* [valid_if(di_symsiz != 0)][const] The address of the symbol table. */
    image_rva_t        di_pltgot;            /* [0..1][const] Procedure linkage table address. */
    image_rva_t        di_needmin;           /* [<= di_needend][const] Pointer to the first `DT_NEEDED' Elf_Dyn-entry in the module's dynamic segment. */
    image_rva_t        di_needend;           /* [>= di_needmin][const] Pointer after the last `DT_NEEDED' Elf_Dyn-entry in the module's dynamic segment. */
    Elf_String         di_name;              /* [valid_if(< di_strsiz)][const] Module name override. */
    Elf_String         di_runpath;           /* [valid_if(< di_strsiz)][const] Search path used when scanning for dependencies. */
    Elf_Word           di_preinit_array_siz; /* [const] Size of init-functions found at `di_preinit_array' (in bytes). */
    Elf_Word           di_init_array_siz;    /* [const] Size of init-functions found at `di_init_array' (in bytes). */
    Elf_Word           di_fini_array_siz;    /* [const] Size of fini-functions found at `di_fini_array' (in bytes). */
    Elf_Word           di_strsiz;            /* [const] The size (in bytes) of the string table. */
    Elf_Word           di_hashsiz;           /* [const] The max length of the hash table. */
    Elf_Word           di_symsiz;            /* [const] The max length of the symbol table. */
    Elf_Word           di_syment;            /* [const] The size of a single symbol table entry. */
    Elf_Word           di_symcnt;            /* [const][== di_symsiz / di_syment] The max amount of valid symbols. */
#define DYN_FNORMAL    0x0000                /* Normal dynamic module flags. */
#define DYN_FSYMBOLIC  0x0001                /* The module uses symbolic symbol resolution (its symbols resolve to itself) */
#define DYN_FBINDNOW   0x0400                /* Bind jump relocations immediately. */
#define DYN_FSTATICTLS 0x0800                /* The module uses the static TLS model. */
#define DYN_FDEBUG     0x1000                /* The module requests debug functionality to be enabled. */
#define DYN_FTEXTREL   0x2000                /* The module contains text relocations. */
#ifdef CONFIG_ELF_USING_RELA
#define DYN_FRELAJMP   0x4000                /* JMP relocations use Elf_Rela */
#endif
#define DYN_FNORUNPATH 0x8000                /* Set when `DT_RUNPATH' was encountered to prevent parsing of `DT_RPATH' */
    Elf_Word           di_flags;             /* [const] Set of `DYN_F*' */
    Elf_Word         __di_pad;               /* ... */
    union {
#define RELINFO_REL    0                     /* [.ri_relent >= sizeof(Elf_Rel)] Normal relocations. */
#define RELINFO_JMP    1                     /* [.ri_relent >= sizeof(Elf_Rel)] Jump-relocation. */
#ifdef CONFIG_ELF_USING_RELA
#define RELINFO_RELA   2                     /* [.ri_relent >= sizeof(Elf_Rela)] Relocations with addend. */
#endif
        RelInfo        di_reloc[RELINFO_COUNT]; /* Relocation tables. */
        struct {
            RelInfo    r_rel;                /* [.ri_relent >= sizeof(Elf_Rel)] Normal relocations. */
            RelInfo    r_jmp;                /* [.ri_relent >= sizeof(Elf_Rel)] Jump-relocation. */
#ifdef CONFIG_ELF_USING_RELA
            RelInfo    r_rela;               /* [.ri_relent >= sizeof(Elf_Rela)] Relocations with addend. */
#endif
        }              di_rel;
    };
} DynInfo;

typedef struct module_data {
    Elf_Off                   e_shoff;    /* [const] Section header table file offset */
    Elf_Half                  e_phnum;    /* [const][!0] Program header table entry count */
    Elf_Half                  e_shnum;    /* [const] Section header table entry count */
    Elf_Half                  e_shstrndx; /* [const] Section header string table index */
#define ELFMODULE_FNORMAL     0x0000      /* Normal ELF Module flags. */
#define ELFMODULE_FDYNLOADING 0x0001      /* [atomic] Dynamic linkage information is currently being loaded. */
#define ELFMODULE_FDYNLOADED  0x0010      /* [lock(WRITE_ONCE)] Dynamic linkage has been loaded. */
    Elf_Half                  e_flags;    /* Set of `ELFMODULE_F*' */
    DynInfo                   e_dyn;      /* [valid_if(ELFMODULE_FDYNLOADED)] Dynamic linkage information. */
#define SHSTRTAB_MAXSIZE      0x800       /* The max length (in bytes) of `e_shstrtab' */
    char                     *e_shstrtab; /* [lock(WRITE_ONCE)][0..1][owned] Section name string table. */
    Elf_Shdr                 *e_shdr;     /* [lock(WRITE_ONCE)][0..e_shnum][owned] Vector of section headers. */
    REF struct vm_region    **e_sections; /* [0..1][0..e_phnum][lock(WRITE_ONCE)][owned]
                                           *  Vector of VM regions that can be used to map program headers.
                                           *  NOTE: NULL entries mark entries for program headers that aren't PT_LOAD. */
    Elf_Phdr                  e_phdr[1];  /* [e_phnum] Vector of program headers. */
} ElfModule;

#define ELF_FOREACH_NEEDED(mod,loadaddr,entry) \
 for ((entry) = (Elf_Dyn *)((loadaddr)+(mod)->e_dyn.di_needmin); \
      (entry) < (Elf_Dyn *)((loadaddr)+(mod)->e_dyn.di_needend); ++(entry)) \
 if ((entry)->d_tag != DT_NEEDED); else


INTDEF struct module_type elf_module_type;

PRIVATE ATTR_NOTHROW void KCALL
Elf_FiniModule(struct module *__restrict self) {
 struct vm_region **vector;
 ElfModule *mod = self->m_data;
 if (!mod) return;
 vector = mod->e_sections;
 if (vector) {
  Elf_Half i = mod->e_phnum;
  while (i--) {
   if (vector[i])
       vm_region_decref(vector[i]);
  }
  kfree(vector);
 }
 kfree(mod->e_shstrtab);
 kfree(mod->e_shdr);
 kfree(mod);
}

PRIVATE void KCALL
Elf_LoadSections(struct module *__restrict self) {
 Elf_Shdr *EXCEPT_VAR vector;
 ElfModule *mod = self->m_data;
 /* Quick check if the sections have already been loaded. */
 if (mod->e_shdr) return;
 vector = (Elf_Shdr *)kmalloc(mod->e_shnum*sizeof(Elf_Shdr),
                              GFP_SHARED|GFP_CALLOC);
 TRY {
  /* Read section header data from disk. */
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
  if (ELF_ISMACHINE32(self->m_machine)) {
   unsigned int i;
   inode_kreadall(&self->m_fsloc->re_node,vector,
                   mod->e_shnum*sizeof(Elf32_Shdr),
                   mod->e_shoff,IO_RDONLY);
   i = mod->e_shnum;
   while (i--) {
    Elf32_Shdr hdr;
    memcpy(&hdr,&((Elf32_Shdr *)vector)[i],sizeof(Elf32_Phdr));
    ELF_SHDR32TO64(vector[i],hdr);
   }
  } else
#endif
  {
   inode_kreadall(&self->m_fsloc->re_node,vector,
                   mod->e_shnum*sizeof(Elf_Shdr),
                   mod->e_shoff,IO_RDONLY);
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(vector);
  error_rethrow();
 }
 /* Save the section vector, WRITE_ONCE-style. */
 if (!ATOMIC_CMPXCH(mod->e_shdr,NULL,vector))
      kfree(vector);
}

PRIVATE char *KCALL
Elf_LoadShStrtab(struct module *__restrict self) {
 char *EXCEPT_VAR result; Elf_Shdr *strtab;
 ElfModule *mod = self->m_data;
 /* Quick check if the sections have already been loaded. */
 if (mod->e_shstrtab) return mod->e_shstrtab;
 if (mod->e_shstrndx >= mod->e_shnum) return NULL;
 /* Load section headers. */
 if (!mod->e_shdr) Elf_LoadSections(self);
 COMPILER_READ_BARRIER();
 strtab = &mod->e_shdr[mod->e_shstrndx];
 if unlikely(strtab->sh_size > SHSTRTAB_MAXSIZE)
    return NULL;
 result = (char *)kmalloc((strtab->sh_size+1)*sizeof(char),
                           GFP_SHARED|GFP_CALLOC);
 TRY {
  /* Read section header data from disk. */
  inode_kreadall(&self->m_fsloc->re_node,result,
                  strtab->sh_size,strtab->sh_offset,IO_RDONLY);
  result[strtab->sh_size] = '\0';
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(result);
  error_rethrow();
 }

 /* Save the section vector, WRITE_ONCE-style. */
 if (!ATOMIC_CMPXCH(mod->e_shstrtab,NULL,result))
      kfree(result);
 return mod->e_shstrtab;
}


PRIVATE void KCALL
Elf_LoadRegions(struct module *__restrict self) {
 REF struct vm_region **EXCEPT_VAR vector;
 Elf_Half i,count; ElfModule *mod = self->m_data;
 /* Quick check if the sections have already been loaded. */
 if (mod->e_sections) return;
 count  = mod->e_phnum;
 vector = (REF struct vm_region **)kmalloc(count*
                                           sizeof(REF struct vm_region *),
                                           GFP_SHARED|GFP_CALLOC);
 i = 0;
 TRY {
  for (; i < count; ++i) {
   struct vm_region *region; size_t page_offset;
   if (mod->e_phdr[i].p_type != PT_LOAD) continue;
   page_offset = mod->e_phdr[i].p_vaddr & (PAGESIZE-1);
   region = vm_region_alloc(CEILDIV(mod->e_phdr[i].p_memsz+page_offset,PAGESIZE));
   /* Prevent user-space from modifying cached module data. */
   region->vr_flags |= VM_REGION_FCANTSHARE;
   region->vr_part0.vp_refcnt = 1; /* Keep one reference for the `ElfModule' */
   assert(!region->vr_part0.vp_chain.le_next);
   if (mod->e_phdr[i].p_filesz) {
    region->vr_init                 = VM_REGION_INIT_FFILE_RO;
    region->vr_setup.s_file.f_node  = &self->m_fsloc->re_node;
    inode_incref(region->vr_setup.s_file.f_node);
    region->vr_setup.s_file.f_begin = page_offset;
    region->vr_setup.s_file.f_start = mod->e_phdr[i].p_offset;
    region->vr_setup.s_file.f_size  = mod->e_phdr[i].p_filesz;
   } else {
    /* .bss style section. */
    region->vr_init = VM_REGION_INIT_FFILLER;
   }  
   vector[i] = region;
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Delete all regions allocated thus far. */
  while (i--) vm_region_decref(vector[i]);
  kfree(vector);
  error_rethrow();
 }

 /* Save the section vector, WRITE_ONCE-style. */
 if (!ATOMIC_CMPXCH(mod->e_sections,NULL,vector)) {
  while (count--) {
   if (vector[count])
       vm_region_destroy(vector[count]);
  }
  kfree(vector);
 }
}


typedef struct {
    Elf_Word ht_nbuckts;
    Elf_Word ht_nchains;
} Elf_HashTable;

#ifdef CONFIG_ELF_SUPPORT_CLASS3264
typedef struct {
    Elf32_Word ht_nbuckts;
    Elf32_Word ht_nchains;
} Elf32_HashTable;
#endif

LOCAL void KCALL
Elf_LoadDynamic(ElfModule *__restrict self,
                struct module *__restrict mod,
                USER CHECKED byte_t *__restrict begin,
                USER CHECKED byte_t *__restrict end,
                USER CHECKED uintptr_t loadaddr,
                image_rva_t image_end) {
 ElfModule *EXCEPT_VAR xself = self;
 for (;;) {
  Elf_Word flags = ATOMIC_FETCHOR(self->e_flags,ELFMODULE_FDYNLOADING);
  if (flags & ELFMODULE_FDYNLOADED) return; /* The other thread finished. */
  if (!(flags & ELFMODULE_FDYNLOADING)) break; /* Now's our time to shine. */
  task_yield(); /* Wait a bit... */
 }
 TRY {
  Elf_Dyn *dyn;
  /* Load dynamic information. */
  memset(&self->e_dyn,0,sizeof(DynInfo));
  dyn = (Elf_Dyn *)begin;
  self->e_dyn.di_needmin = (image_rva_t)-1;
  /* Set default values for entity sizes. */
  self->e_dyn.di_strsiz             = (Elf_Word)-1;
  self->e_dyn.di_symsiz             = (Elf_Word)-1;
  self->e_dyn.di_name               = (Elf_String)-1;
  self->e_dyn.di_runpath            = (Elf_String)-1;
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
  if (ELF_ISMACHINE32(mod->m_machine)) {
   self->e_dyn.di_syment             = sizeof(Elf32_Sym);
   self->e_dyn.di_reloc[0].ri_relent = sizeof(Elf32_Rel);
   self->e_dyn.di_reloc[1].ri_relent = sizeof(Elf32_Rel);
#ifdef CONFIG_ELF_USING_RELA
   self->e_dyn.di_reloc[2].ri_relent = sizeof(Elf32_Rela);
#endif
  } else
#endif
  {
   self->e_dyn.di_syment             = sizeof(Elf_Sym);
   self->e_dyn.di_reloc[0].ri_relent = sizeof(Elf_Rel);
   self->e_dyn.di_reloc[1].ri_relent = sizeof(Elf_Rel);
#ifdef CONFIG_ELF_USING_RELA
   self->e_dyn.di_reloc[2].ri_relent = sizeof(Elf_Rela);
#endif
  }

#ifdef CONFIG_ELF_SUPPORT_CLASS3264
  for (; dyn < (Elf_Dyn *)end; *(byte_t **)&dyn +=
       ELF_ISMACHINE32(mod->m_machine) ? sizeof(Elf32_Dyn)
                                       : sizeof(Elf64_Dyn))
#else
  for (; dyn < (Elf_Dyn *)end; ++dyn)
#endif
  {
   Elf_Dyn tag;
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
   if (ELF_ISMACHINE32(mod->m_machine)) {
    ELF_DYN32TO64(tag,*(Elf32_Dyn *)dyn);
   } else
#endif
   {
    memcpy(&tag,dyn,sizeof(Elf_Dyn));
   }

   switch (tag.d_tag) {

   case DT_NULL:
    /* Sentinel-style terminator. */
    goto done;

   {
    image_rva_t tagaddr;
   case DT_NEEDED:
    /* Keep track of all needed-entries. */
    tagaddr = (image_rva_t)((uintptr_t)dyn - loadaddr);
    if (self->e_dyn.di_needmin > tagaddr)
        self->e_dyn.di_needmin = tagaddr;
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
    if (ELF_ISMACHINE32(mod->m_machine)) {
     self->e_dyn.di_needend = tagaddr+sizeof(Elf32_Dyn);
    } else
#endif
    {
     self->e_dyn.di_needend = tagaddr+sizeof(Elf_Dyn);
    }
   } break;

   case DT_HASH:
    self->e_dyn.di_hashtab = tag.d_un.d_ptr;
    /* Load basic information from the hash-table (if it exists). */
    if (self->e_dyn.di_hashtab >= image_end)
     self->e_dyn.di_hashsiz = 0;
    else {
     Elf_HashTable *hashtab; Elf_Word sym_count,hash_size;
     hashtab   = (Elf_HashTable *)(loadaddr + self->e_dyn.di_hashtab);
     sym_count = hashtab->ht_nchains;
     hash_size = sizeof(Elf_HashTable)+((hashtab->ht_nchains + sym_count)*sizeof(Elf_Word));
     hash_size = MIN(hash_size,image_end-self->e_dyn.di_hashtab);
     self->e_dyn.di_hashsiz = hash_size;
     self->e_dyn.di_symcnt  = sym_count;
    }
    break;

   case DT_STRTAB:   self->e_dyn.di_strtab  = tag.d_un.d_ptr; break;
   case DT_SYMTAB:   self->e_dyn.di_symtab  = tag.d_un.d_ptr; break;
   case DT_STRSZ:    self->e_dyn.di_strsiz  = tag.d_un.d_val; break;
   case DT_SYMENT:   self->e_dyn.di_syment  = tag.d_un.d_val; break;
   case DT_INIT:     self->e_dyn.di_init    = tag.d_un.d_ptr; break;
   case DT_FINI:     self->e_dyn.di_fini    = tag.d_un.d_ptr; break;
   case DT_SONAME:   self->e_dyn.di_name    = tag.d_un.d_val; break;
   case DT_SYMBOLIC: self->e_dyn.di_flags  |= DYN_FSYMBOLIC; break;

#ifdef CONFIG_ELF_USING_RELA
   case DT_RELA:     self->e_dyn.di_rel.r_rela.ri_rel    = tag.d_un.d_ptr; break;
   case DT_RELASZ:   self->e_dyn.di_rel.r_rela.ri_relsiz = tag.d_un.d_val; break;
   case DT_RELAENT:  self->e_dyn.di_rel.r_rela.ri_relent = tag.d_un.d_val;
                     if (self->e_dyn.di_flags&DYN_FRELAJMP)
                         self->e_dyn.di_rel.r_jmp.ri_relent = tag.d_un.d_val;
                     break;
#endif
   case DT_REL:      self->e_dyn.di_rel.r_rel.ri_rel    = tag.d_un.d_ptr; break;
   case DT_RELSZ:    self->e_dyn.di_rel.r_rel.ri_relsiz = tag.d_un.d_val; break;
   case DT_RELENT:   self->e_dyn.di_rel.r_rel.ri_relent = tag.d_un.d_val; break;
   case DT_PLTGOT:   self->e_dyn.di_pltgot              = tag.d_un.d_ptr; break;
   case DT_JMPREL:   self->e_dyn.di_rel.r_jmp.ri_rel    = tag.d_un.d_ptr; break;
   case DT_PLTRELSZ: self->e_dyn.di_rel.r_jmp.ri_relsiz = tag.d_un.d_val; break;
#ifdef CONFIG_ELF_USING_RELA
   case DT_PLTREL:
    if (tag.d_un.d_val == DT_RELA) {
     self->e_dyn.di_flags              |= DYN_FRELAJMP;
     self->e_dyn.di_rel.r_jmp.ri_relent = self->e_dyn.di_rel.r_rela.ri_relent;
    }
    break;
#else
   case DT_PLTREL:
    break;
#endif
   case DT_TEXTREL:         self->e_dyn.di_flags |= DYN_FTEXTREL; break;
   case DT_DEBUG:           self->e_dyn.di_flags |= DYN_FDEBUG; break;
   case DT_INIT_ARRAY:      self->e_dyn.di_init_array = tag.d_un.d_ptr; break;
   case DT_FINI_ARRAY:      self->e_dyn.di_fini_array = tag.d_un.d_ptr; break;
   case DT_INIT_ARRAYSZ:    self->e_dyn.di_init_array_siz = tag.d_un.d_val; break;
   case DT_FINI_ARRAYSZ:    self->e_dyn.di_fini_array_siz = tag.d_un.d_val; break;
   case DT_PREINIT_ARRAY:   self->e_dyn.di_preinit_array = tag.d_un.d_ptr; break;
   case DT_PREINIT_ARRAYSZ: self->e_dyn.di_preinit_array_siz = tag.d_un.d_val; break;

   case DT_BIND_NOW: self->e_dyn.di_flags |= DYN_FBINDNOW; break;
   case DT_FLAGS:
    if (tag.d_un.d_val & DF_SYMBOLIC)   self->e_dyn.di_flags |= DYN_FSYMBOLIC;
    if (tag.d_un.d_val & DF_TEXTREL)    self->e_dyn.di_flags |= DYN_FTEXTREL;
    if (tag.d_un.d_val & DF_BIND_NOW)   self->e_dyn.di_flags |= DYN_FBINDNOW;
    if (tag.d_un.d_val & DF_STATIC_TLS) self->e_dyn.di_flags |= DYN_FSTATICTLS;
    break;

   case DT_RPATH:
    if (!(self->e_dyn.di_flags&DYN_FNORUNPATH))
           self->e_dyn.di_runpath = tag.d_un.d_val;
    break;
   case DT_RUNPATH:
    /* Library search path when scanning for dependencies. */
    self->e_dyn.di_runpath = tag.d_un.d_val;
    self->e_dyn.di_flags |= (DYN_FNORUNPATH);
    break;

   default:
    break;
   }
  }
done:;
  /* Do some validations. */
  if (self->e_dyn.di_needmin > self->e_dyn.di_needend)
      self->e_dyn.di_needmin = self->e_dyn.di_needend = 0;
  if (self->e_dyn.di_strtab >= image_end) self->e_dyn.di_strsiz = 0;
  else self->e_dyn.di_strsiz = MIN(self->e_dyn.di_strsiz,image_end-self->e_dyn.di_strtab);
  if (self->e_dyn.di_hashtab >= image_end) self->e_dyn.di_hashsiz = 0;
  else {
   image_rva_t hash_max = (image_rva_t)(image_end-self->e_dyn.di_hashtab);
   if unlikely(self->e_dyn.di_hashsiz > hash_max) {
    /* Hash-table is incorrectly sized (and therefor corrupt) */
    self->e_dyn.di_hashsiz = 0;
   }
  }
  if (self->e_dyn.di_symtab >= image_end ||
      self->e_dyn.di_syment < offsetof(Elf_Sym,st_shndx)) {
no_symbols:
   self->e_dyn.di_symsiz = 0;
   self->e_dyn.di_syment = 0;
   self->e_dyn.di_symcnt = 0;
  } else {
   image_rva_t sym_max = (image_rva_t)(image_end-self->e_dyn.di_symtab);
   self->e_dyn.di_symsiz = self->e_dyn.di_symcnt * self->e_dyn.di_syment;
   if (self->e_dyn.di_symsiz > sym_max) {
    self->e_dyn.di_symsiz = (Elf_Word)sym_max;
    self->e_dyn.di_symcnt = (Elf_Word)sym_max / self->e_dyn.di_syment;
    if (!self->e_dyn.di_symcnt) {
     self->e_dyn.di_symsiz = 0;
     self->e_dyn.di_syment = 0;
     goto no_symbols;
    }
   }
  }
  if (!self->e_dyn.di_strsiz) {
   self->e_dyn.di_hashsiz = 0; /* Without a string table, there can be no hash-table. */
   self->e_dyn.di_needmin = self->e_dyn.di_needend = 0; /* Without a string table, there can be no dependencies. */
  }
  if (!self->e_dyn.di_symsiz)
       self->e_dyn.di_hashsiz = 0; /* Without a symbol table, there can be no hash-table. */
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
  if (ELF_ISMACHINE32(mod->m_machine)) {
   if ((self->e_dyn.di_rel.r_rel.ri_relent < sizeof(Elf32_Rel)) ||
       (self->e_dyn.di_rel.r_rel.ri_relent > self->e_dyn.di_rel.r_rel.ri_relsiz))
        self->e_dyn.di_rel.r_rel.ri_relsiz = 0;
   if ((self->e_dyn.di_rel.r_jmp.ri_relent < sizeof(Elf32_Rel)) ||
       (self->e_dyn.di_rel.r_jmp.ri_relent > self->e_dyn.di_rel.r_jmp.ri_relsiz)
#ifdef CONFIG_ELF_USING_RELA
       || ((self->e_dyn.di_flags & DYN_FRELAJMP) &&
            self->e_dyn.di_rel.r_jmp.ri_relent < sizeof(Elf32_Rela))
#endif
       )
        self->e_dyn.di_rel.r_jmp.ri_relsiz = 0;
#ifdef CONFIG_ELF_USING_RELA
   if ((self->e_dyn.di_rel.r_rela.ri_relent < sizeof(Elf32_Rela)) ||
       (self->e_dyn.di_rel.r_rela.ri_relent > self->e_dyn.di_rel.r_rela.ri_relsiz))
        self->e_dyn.di_rel.r_rela.ri_relsiz = 0;
#endif
  } else
#endif
  {
   if ((self->e_dyn.di_rel.r_rel.ri_relent < sizeof(Elf_Rel)) ||
       (self->e_dyn.di_rel.r_rel.ri_relent > self->e_dyn.di_rel.r_rel.ri_relsiz))
        self->e_dyn.di_rel.r_rel.ri_relsiz = 0;
   if ((self->e_dyn.di_rel.r_jmp.ri_relent < sizeof(Elf_Rel)) ||
       (self->e_dyn.di_rel.r_jmp.ri_relent > self->e_dyn.di_rel.r_jmp.ri_relsiz)
#ifdef CONFIG_ELF_USING_RELA
       || ((self->e_dyn.di_flags & DYN_FRELAJMP) &&
            self->e_dyn.di_rel.r_jmp.ri_relent < sizeof(Elf_Rela))
#endif
       )
        self->e_dyn.di_rel.r_jmp.ri_relsiz = 0;
#ifdef CONFIG_ELF_USING_RELA
   if ((self->e_dyn.di_rel.r_rela.ri_relent < sizeof(Elf_Rela)) ||
       (self->e_dyn.di_rel.r_rela.ri_relent > self->e_dyn.di_rel.r_rela.ri_relsiz))
        self->e_dyn.di_rel.r_rela.ri_relsiz = 0;
#endif
  }
  if (self->e_dyn.di_name >= self->e_dyn.di_strsiz)
      self->e_dyn.di_name = self->e_dyn.di_strsiz;
  if (self->e_dyn.di_runpath >= self->e_dyn.di_strsiz)
      self->e_dyn.di_runpath = self->e_dyn.di_strsiz;

#if 0
  debug_printf("ELF_DYNAMIC:\n");
  debug_printf("\tdi_init          = %p\n",self->e_dyn.di_init);
  debug_printf("\tdi_fini          = %p\n",self->e_dyn.di_fini);
  debug_printf("\tdi_preinit_array = %p (%u entries)\n",self->e_dyn.di_preinit_array,self->e_dyn.di_preinit_array_siz);
  debug_printf("\tdi_init_array    = %p (%u entries)\n",self->e_dyn.di_init_array,self->e_dyn.di_init_array_siz);
  debug_printf("\tdi_fini_array    = %p (%u entries)\n",self->e_dyn.di_fini_array,self->e_dyn.di_fini_array_siz);
  debug_printf("\tdi_strtab        = %p (%u bytes)\n",self->e_dyn.di_strtab,self->e_dyn.di_strsiz);
  debug_printf("\tdi_hashtab       = %p (%u bytes)\n",self->e_dyn.di_hashtab,self->e_dyn.di_hashsiz);
  debug_printf("\tdi_symtab        = %p (%u bytes, %u entries, %u per entry)\n",self->e_dyn.di_symtab,self->e_dyn.di_symsiz,self->e_dyn.di_symcnt,self->e_dyn.di_syment);
  debug_printf("\tdi_needmin       = %p\n",self->e_dyn.di_needmin);
  debug_printf("\tdi_needend       = %p\n",self->e_dyn.di_needend);
  debug_printf("\tdi_gotaddr       = %p\n",self->e_dyn.di_gotaddr);
  debug_printf("\tdi_name          = %p%s\n",self->e_dyn.di_name,self->e_dyn.di_name >= self->e_dyn.di_strsiz ? " (UNSET)" : "");
  debug_printf("\tdi_runpath       = %p%s\n",self->e_dyn.di_runpath,self->e_dyn.di_runpath >= self->e_dyn.di_strsiz ? " (UNSET)" : "");
  debug_printf("\tdi_flags         = %.4I16X\n",self->e_dyn.di_flags);
  debug_printf("RELOCATIONS:\n");
  debug_printf("\tr_rel  = %p (%u bytes, %u per entry)\n",self->e_dyn.di_rel.r_rel.ri_rel,self->e_dyn.di_rel.r_rel.ri_relsiz,self->e_dyn.di_rel.r_rel.ri_relent);
  debug_printf("\tr_jmp  = %p (%u bytes, %u per entry)\n",self->e_dyn.di_rel.r_jmp.ri_rel,self->e_dyn.di_rel.r_jmp.ri_relsiz,self->e_dyn.di_rel.r_jmp.ri_relent);
#ifdef CONFIG_ELF_USING_RELA
  debug_printf("\tr_rela = %p (%u bytes, %u per entry)\n",self->e_dyn.di_rel.r_rela.ri_rel,self->e_dyn.di_rela.r_rel.ri_relsiz,self->e_dyn.di_rela.r_rel.ri_relent);
#endif
#endif

 } FINALLY {
  if (FINALLY_WILL_RETHROW)
       ATOMIC_FETCHAND(xself->e_flags,~ELFMODULE_FDYNLOADING);
  else ATOMIC_FETCHOR(xself->e_flags,ELFMODULE_FDYNLOADED);
 }
}


PRIVATE ATTR_NOINLINE void KCALL
Application_AllocateStaticTLSSegment(struct application *__restrict app) {
 if (app->a_flags & APPLICATION_FHASTLS) return;
 if (MODULE_HASTLS(app->a_module)) {
  struct module *mod = app->a_module;
  TRY {
   /* Allocate a static TLS segment for the application. */
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
   /* XXX: Special handling? */
#endif
   app->a_tlsoff = tls_alloc((void *)(app->a_loadaddr +
                                      mod->m_tlsmin),
                              mod->m_tlstplsz,
                              MODULE_TLSSIZE(mod),
                              mod->m_tlsalign);
  } CATCH (E_BADALLOC) {
   /* Ignore failure to allocate the application's TLS segment.
    * In this case, TLS allocations are performed at runtime. */
   if (error_info()->e_error.e_badalloc.ba_resource != ERROR_BADALLOC_VIRTMEMORY)
       error_rethrow();
   error_handled();
   return;
  }
 }
 ATOMIC_FETCHOR(app->a_flags,APPLICATION_FHASTLS);
}




PRIVATE void KCALL
Elf_NewApplication(struct module_patcher *__restrict self) {
 struct application *EXCEPT_VAR app = self->mp_app;
 ElfModule *EXCEPT_VAR mod = app->a_module->m_data;
 USER CHECKED uintptr_t loadaddr = app->a_loadaddr;
 USER CHECKED uintptr_t EXCEPT_VAR loadpage = loadaddr;
 USER CHECKED uintptr_t image_min,image_end;
 unsigned int EXCEPT_VAR i;
 struct vm_region **EXCEPT_VAR vector;
 vm_prot_t force_prot = 0;
 if (self->mp_apptype & APPLICATION_TYPE_FDRIVER)
     force_prot |= PROT_NOUSER;
 image_min = APPLICATION_MAPMIN(app);
 image_end = APPLICATION_MAPEND(app);
 assert(IS_ALIGNED(loadpage,PAGESIZE));
 loadpage /= PAGEALIGN; /* Turn into a page number. */
 /* Make sure that regions have been loaded. */
 Elf_LoadRegions(app->a_module);

 /* Map all the regions. */
 vector = mod->e_sections;
 assert(vector);
 i = 0;
 TRY {
  for (; i < mod->e_phnum; ++i) {
   switch (mod->e_phdr[i].p_type) {

   case PT_LOAD:
    assert(vector[i]);
    /* Map application segments. */
    vm_mapat(loadpage + VM_ADDR2PAGE(mod->e_phdr[i].p_vaddr),
             vector[i]->vr_size,0,vector[i],force_prot|(
#if PF_X == PROT_EXEC && PF_W == PROT_WRITE && PF_R == PROT_READ
             mod->e_phdr[i].p_flags & (PF_X|PF_W|PF_R)),
#else
           ((mod->e_phdr[i].p_flags&PF_X) ? PROT_EXEC : 0)|
           ((mod->e_phdr[i].p_flags&PF_W) ? PROT_WRITE : 0)|
           ((mod->e_phdr[i].p_flags&PF_R) ? PROT_READ : 0)),
#endif
            &application_notify,app);
    break;

   {
    USER CHECKED byte_t *dynbegin,*dynend;
    unsigned int dyn_index;
   case PT_DYNAMIC:
    dyn_index = i;
    /* Map all segments that haven't been mapped yet. */
    for (++i; i < mod->e_phnum; ++i) {
     switch (mod->e_phdr[i].p_type) {

     case PT_LOAD:
      /* Map application segments. */
      vm_mapat(loadpage + VM_ADDR2PAGE(mod->e_phdr[i].p_vaddr),
               vector[i]->vr_size,0,vector[i],force_prot|(
#if PF_X == PROT_EXEC && PF_W == PROT_WRITE && PF_R == PROT_READ
               mod->e_phdr[i].p_flags & (PF_X|PF_W|PF_R)),
#else
             ((mod->e_phdr[i].p_flags&PF_X) ? PROT_EXEC : 0)|
             ((mod->e_phdr[i].p_flags&PF_W) ? PROT_WRITE : 0)|
             ((mod->e_phdr[i].p_flags&PF_R) ? PROT_READ : 0)),
#endif
              &application_notify,app);
      break;

     case PT_TLS:
      Application_AllocateStaticTLSSegment(app);
      break;

     default: break;
     }
    }

    /* Relocations + dynamic linking, 'n stuff. */
    dynbegin = (byte_t *)((loadpage * PAGESIZE) + mod->e_phdr[dyn_index].p_vaddr);
    dynend   = dynbegin + mod->e_phdr[dyn_index].p_memsz;
    /* Validate the range of the dynamic section. */
    if unlikely((uintptr_t)dynend < (uintptr_t)dynbegin ||
                (uintptr_t)dynbegin < image_min ||
                (uintptr_t)dynend > image_end)
       error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADDYNAMIC);
    /* Load dynamic linkage information the first time around. */
    if (!(mod->e_flags & ELFMODULE_FDYNLOADED)) {
     Elf_LoadDynamic(mod,app->a_module,dynbegin,dynend,
                     loadaddr,app->a_module->m_size);
    }

    /* Load required dependencies. */
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
    if (ELF_ISMACHINE32(app->a_module->m_machine)) {
     Elf32_Dyn *iter,*end;
     char *strtab = (char *)(loadaddr + mod->e_dyn.di_strtab);
     iter = (Elf32_Dyn *)(loadaddr + mod->e_dyn.di_needmin);
     end  = (Elf32_Dyn *)(loadaddr + mod->e_dyn.di_needend);
     /* Set the alternative module run path according to what the ELF binary wants. */
     if (mod->e_dyn.di_runpath < mod->e_dyn.di_strsiz)
         patcher_setaltpath(self,strtab + mod->e_dyn.di_runpath);
     for (; iter < end; ++iter) {
      Elf32_Addr offset;
      char *name;
      if (iter->d_tag != DT_NEEDED)
          continue;
      offset = iter->d_un.d_ptr;
      COMPILER_READ_BARRIER();
      name = strtab + offset;
      if (offset >= mod->e_dyn.di_strsiz)
          continue;
      /* Load the required dependency. */
      patcher_require_string(self,name,strlen(name));
     }
    } else
#endif
    {
     Elf_Dyn *iter,*end;
     char *strtab = (char *)(loadaddr + mod->e_dyn.di_strtab);
     iter = (Elf_Dyn *)(loadaddr + mod->e_dyn.di_needmin);
     end  = (Elf_Dyn *)(loadaddr + mod->e_dyn.di_needend);
     /* Set the alternative module run path according to what the ELF binary wants. */
     if (mod->e_dyn.di_runpath < mod->e_dyn.di_strsiz)
         patcher_setaltpath(self,strtab + mod->e_dyn.di_runpath);
     for (; iter < end; ++iter) {
      Elf_Addr offset;
      char *name;
      if (iter->d_tag != DT_NEEDED)
          continue;
      offset = iter->d_un.d_ptr;
      COMPILER_READ_BARRIER();
      name = strtab + offset;
      if (offset >= mod->e_dyn.di_strsiz)
          continue;
      /* Load the required dependency. */
      patcher_require_string(self,name,strlen(name));
     }
    }
   } break;

   case PT_TLS:
    Application_AllocateStaticTLSSegment(app);
    break;

   default: break;
   }
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
#if 1
  /* Unmap everything that had already been mapped. */
  while (i--) {
   if (!vector[i]) continue;
   vm_unmap(loadpage + VM_ADDR2PAGE(mod->e_phdr[i].p_vaddr),
            vector[i]->vr_size,VM_UNMAP_TAG|VM_UNMAP_NOEXCEPT,
            app);
  }
  /* Make sure that unmapped memory is synced immediately.
   * NOTE: An unmap() race condition is prevented because the
   *       caller is holding a lock on the effective VM. */
  vm_sync(loadpage+app->a_module->m_imagemin,
          app->a_module->m_imageend-
          app->a_module->m_imagemin);
#endif
  error_rethrow();
 }
}

PRIVATE void KCALL
make_writable(ElfModule *__restrict mod, uintptr_t loadaddr,
              struct application *__restrict app) {
 unsigned int i; REF struct vm_region **vector;
 vector = mod->e_sections;
 for (i = 0; i < mod->e_phnum; ++i) {
  if (mod->e_phdr[i].p_type != PT_LOAD) continue;
  if (mod->e_phdr[i].p_flags & PF_W) continue;
  /* Add write permissions to this segment. */
  vm_protect(VM_ADDR2PAGE(loadaddr+mod->e_phdr[i].p_vaddr),
             vector[i]->vr_size,~0,PROT_WRITE,
             VM_PROTECT_TAG,app);
 }
}
PRIVATE void KCALL
make_readonly(ElfModule *__restrict mod, uintptr_t loadaddr,
              struct application *__restrict app) {
 unsigned int i; REF struct vm_region **vector;
 vector = mod->e_sections;
 for (i = 0; i < mod->e_phnum; ++i) {
  if (mod->e_phdr[i].p_type != PT_LOAD) continue;
  if (mod->e_phdr[i].p_flags & PF_W) continue;
  /* Remove write permissions from this segment. */
  vm_protect(VM_ADDR2PAGE(loadaddr+mod->e_phdr[i].p_vaddr),
             vector[i]->vr_size,~PROT_WRITE,0,
             VM_PROTECT_TAG,app);
 }
}

PRIVATE bool ignore_elf_type = false;
PRIVATE bool ignore_elf_reloc = false;
DEFINE_DRIVER_BOOL(ignore_elf_type,"elf-ignore-type");
DEFINE_DRIVER_BOOL(ignore_elf_reloc,"elf-ignore-bad-reloc");




PRIVATE void KCALL
Elf_PatchApplication(struct module_patcher *__restrict self) {
 struct application *app = self->mp_app;
 ElfModule *mod = app->a_module->m_data;
 unsigned int relocation_group;
 USER CHECKED uintptr_t loadaddr = app->a_loadaddr;
 char *strtab = (char *)(loadaddr + mod->e_dyn.di_strtab);
#ifdef CONFIG_HIGH_KERNEL
 char *strtab_end = strtab + mod->e_dyn.di_strsiz;
#endif
 byte_t *symtab = (byte_t *)(loadaddr + mod->e_dyn.di_symtab);
#ifdef CONFIG_HIGH_KERNEL
 byte_t *image_end = (byte_t *)APPLICATION_MAPEND(app);
#else
 byte_t *image_min = (byte_t *)APPLICATION_MAPMIN(app);
#endif
 if (mod->e_dyn.di_flags & DYN_FTEXTREL)
     make_writable(mod,loadaddr,app);
 /* Initialize this application's static TLS segment. */
 if (app->a_flags & APPLICATION_FHASTLS)
     tls_init(app->a_tlsoff,(void *)(loadaddr + app->a_module->m_tlsmin),
              app->a_module->m_tlstplsz,MODULE_TLSSIZE(app->a_module));
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
 if (ELF_ISMACHINE32(app->a_module->m_machine)) {
  for (relocation_group = 0;
       relocation_group < RELINFO_COUNT; ++relocation_group) {
   Elf32_Word ent = mod->e_dyn.di_reloc[relocation_group].ri_relent;
   Elf32_Rel *iter = (Elf32_Rel *)(loadaddr + mod->e_dyn.di_reloc[relocation_group].ri_rel);
   Elf32_Rel *end = (Elf32_Rel *)((uintptr_t)iter + mod->e_dyn.di_reloc[relocation_group].ri_relsiz);
   assert(iter >= end || ent);
   for (; iter < end; *(uintptr_t *)&iter += ent) {
    byte_t *reladdr = (byte_t *)(loadaddr + iter->r_offset);
#ifdef CONFIG_ELF_USING_RELA
    Elf32_Sword addend = 0;
    if (relocation_group == RELINFO_RELA ||
       (relocation_group == RELINFO_JMP &&
        mod->e_dyn.di_flags & DYN_FRELAJMP))
        addend = ((Elf_Rela *)iter)->r_addend;
#endif
#ifdef CONFIG_HIGH_KERNEL
    if (reladdr >= image_end)
        error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADRELADDR);
#else
    if (reladdr < image_min)
        error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADRELADDR);
#endif
    if (!Elf32_PerformRelocation(reladdr,
                                (Elf32_Word)iter->r_info,
#ifdef CONFIG_ELF_USING_RELA
                                (Elf32_Sword)addend,
#endif
                                 app,
                                (u32)(uintptr_t)loadaddr,
                                (u32)(uintptr_t)image_end,
                                 symtab,
                                (Elf32_Word)mod->e_dyn.di_symcnt,
                                (Elf32_Word)mod->e_dyn.di_syment,
                                 strtab,
#ifdef CONFIG_HIGH_KERNEL
                                 strtab_end,
#endif
                                 self,
                                (mod->e_dyn.di_flags & DYN_FSYMBOLIC) != 0)) {
     char *sym_name; Elf32_Sym *sym;
     unsigned int symid = ELF32_R_SYM(iter->r_info);
     COMPILER_READ_BARRIER();
     if unlikely(symid >= mod->e_dyn.di_symcnt)
        error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADSYMBOL);
     sym = (Elf32_Sym *)(symtab + (symid * (Elf32_Word)mod->e_dyn.di_syment));
     sym_name = strtab+sym->st_name;
     if (sym_name < strtab || sym_name >= strtab_end)
         sym_name = "??" "?";
     debug_printf(COLDSTR("[ELF] Unknown 32-bit relocation #%u at %p(%p) (%#Ix with symbol %#Ix; %q) in `%[path]'\n"),
                ((uintptr_t)iter-(loadaddr + mod->e_dyn.di_reloc[relocation_group].ri_rel))/
                                             mod->e_dyn.di_reloc[relocation_group].ri_relent,
                  iter,iter->r_offset,
                 (uintptr_t)ELF32_R_TYPE(iter->r_info),
                 (uintptr_t)symid,
                  sym_name,app->a_module->m_path);
     if (!ignore_elf_reloc)
          error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADRELOC);
    }
   }
  }
 } else
#endif
 {
  for (relocation_group = 0;
       relocation_group < RELINFO_COUNT; ++relocation_group) {
   Elf_Word ent = mod->e_dyn.di_reloc[relocation_group].ri_relent;
   Elf_Rel *iter = (Elf_Rel *)(loadaddr + mod->e_dyn.di_reloc[relocation_group].ri_rel);
   Elf_Rel *end = (Elf_Rel *)((uintptr_t)iter + mod->e_dyn.di_reloc[relocation_group].ri_relsiz);
   assert(iter >= end || ent);
   for (; iter < end; *(uintptr_t *)&iter += ent) {
    byte_t *reladdr = (byte_t *)(loadaddr + iter->r_offset);
#ifdef CONFIG_ELF_USING_RELA
    Elf_Sword addend = 0;
    if (relocation_group == RELINFO_RELA ||
       (relocation_group == RELINFO_JMP &&
        mod->e_dyn.di_flags & DYN_FRELAJMP))
        addend = ((Elf_Rela *)iter)->r_addend;
#endif
#ifdef CONFIG_HIGH_KERNEL
    if (reladdr >= image_end)
        error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADRELADDR);
#else
    if (reladdr < image_min)
        error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADRELADDR);
#endif
    if (!Elf_PerformRelocation(reladdr,
                               iter->r_info,
#ifdef CONFIG_ELF_USING_RELA
                               addend,
#endif
                               app,
                               loadaddr,
                              (uintptr_t)image_end,
                               symtab,
                               mod->e_dyn.di_symcnt,
                               mod->e_dyn.di_syment,
                               strtab,
#ifdef CONFIG_HIGH_KERNEL
                               strtab_end,
#endif
                               self,
                              (mod->e_dyn.di_flags & DYN_FSYMBOLIC) != 0)) {
     char *sym_name; Elf_Sym *sym;
     unsigned int symid = ELF_R_SYM(iter->r_info);
     COMPILER_READ_BARRIER();
     if unlikely(symid >= mod->e_dyn.di_symcnt)
        error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADSYMBOL);
     sym = (Elf_Sym *)(symtab + (symid * (Elf_Word)mod->e_dyn.di_syment));
     sym_name = strtab+sym->st_name;
     if (sym_name < strtab || sym_name >= strtab_end)
         sym_name = "??" "?";
     debug_printf(COLDSTR("[ELF] Unknown relocation #%u at %p(%p) (%#Ix with symbol %#Ix; %q) in `%[path]'\n"),
                ((uintptr_t)iter-(loadaddr + mod->e_dyn.di_reloc[relocation_group].ri_rel))/
                                             mod->e_dyn.di_reloc[relocation_group].ri_relent,
                  iter,iter->r_offset,
                 (uintptr_t)ELF_R_TYPE(iter->r_info),
                 (uintptr_t)symid,
                  sym_name,app->a_module->m_path);
     if (!ignore_elf_reloc)
          error_throwf(E_NOT_EXECUTABLE,ERROR_NOT_EXECUTABLE_BADRELOC);
    }
   }
  }
 }
 if (mod->e_dyn.di_flags & DYN_FTEXTREL)
     make_readonly(mod,loadaddr,app);
}



PRIVATE struct dl_symbol KCALL
Elf_GetSymbolAddress(struct application *__restrict app,
                     USER CHECKED char const *__restrict name,
                     u32 hash) {
 ElfModule *self = app->a_module->m_data;
 uintptr_t load_addr = (uintptr_t)app->a_loadaddr;
 struct dl_symbol result;
 result.ds_type = MODULE_SYMBOL_INVALID;
 if (self->e_dyn.di_symcnt != 0) {
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
  if (ELF_ISMACHINE32(app->a_module->m_machine)) {
   Elf32_Sym *symtab_begin,*symtab_end,*symtab_iter;
   char *string_table = (char *)(load_addr + self->e_dyn.di_strtab);
   char *string_end = (char *)((uintptr_t)string_table+self->e_dyn.di_strsiz);
   while (string_end != string_table && string_end[-1] != '\0') --string_end;
   if unlikely(string_end == string_table) goto end;
   symtab_begin = (Elf32_Sym *)(load_addr + self->e_dyn.di_symtab);
   symtab_end   = (Elf32_Sym *)((uintptr_t)symtab_begin+self->e_dyn.di_symsiz);
   if (self->e_dyn.di_hashsiz != 0) {
    /* Make use of '.hash' information! */
    Elf32_HashTable *phashtab;
    Elf32_HashTable hashtab; Elf32_Word *ptable,chain;
    phashtab = (Elf32_HashTable *)(load_addr + self->e_dyn.di_hashtab);
    hashtab = *phashtab;
    COMPILER_READ_BARRIER();
    if unlikely(!hashtab.ht_nbuckts || !hashtab.ht_nbuckts) {
     /* Nope. - The hash-table is broken. */
broken_hash32:
     debug_printf(COLDSTR("[ELF] Module `%q' contains invalid hash table\n"),
                  app->a_module->m_path->p_dirent->de_name);
     ATOMIC_WRITE(self->e_dyn.di_hashsiz,0);
    } else {
     Elf32_Word max_attempts = hashtab.ht_nchains;
     /* Make sure the hash-table isn't too large. */
     if unlikely((sizeof(Elf32_HashTable)+(hashtab.ht_nbuckts+
                                           hashtab.ht_nchains)*
                  sizeof(Elf32_Word)) > self->e_dyn.di_hashsiz) goto broken_hash32;
     /* Make sure the hash-table isn't trying to go out-of-bounds. */
     if unlikely(hashtab.ht_nchains > self->e_dyn.di_symcnt) goto broken_hash32;
     ptable  = (Elf32_Word *)(phashtab+1);
     chain   = ptable[hash % hashtab.ht_nbuckts];
     ptable += hashtab.ht_nbuckts;
     while likely(max_attempts--) {
      char *sym_name;
      if unlikely(chain == STN_UNDEF ||
                  chain >= self->e_dyn.di_symcnt)
         break;
      /* Check this candidate. */
      symtab_iter = (Elf32_Sym *)((uintptr_t)symtab_begin+chain*self->e_dyn.di_syment);
      assert(symtab_iter >= symtab_begin);
      assert(symtab_iter <  symtab_end);
      sym_name = string_table+symtab_iter->st_name;
      if unlikely((uintptr_t)sym_name <  (uintptr_t)string_table || 
                  (uintptr_t)sym_name >= (uintptr_t)string_end) break;
#if 0
      debug_printf(COLDSTR("Checking hashed symbol name %q == %q (chain = %X; value = %p)\n"),
                   name,sym_name,chain,symtab_iter->st_value);
#endif
      if (strcmp(sym_name,name) != 0) goto next_candidate32;
      if (symtab_iter->st_shndx == SHN_UNDEF) goto end; /* Symbol not defined by this library. */
      result.ds_base = (void *)(uintptr_t)(u32)symtab_iter->st_value;
      result.ds_size = symtab_iter->st_size;
      if (ELF32_ST_TYPE(symtab_iter->st_info) == STT_TLS)
        *(uintptr_t *)&result.ds_base += (app->a_tlsoff - MODULE_TLSSIZE(app->a_module));
      else if (symtab_iter->st_shndx != SHN_ABS)
        *(uintptr_t *)&result.ds_base += load_addr;
      result.ds_type = MODULE_SYMBOL_NORMAL;
      if (ELF32_ST_BIND(symtab_iter->st_info) == STB_WEAK)
          result.ds_type = MODULE_SYMBOL_WEAK;
      goto end;
next_candidate32:
      if unlikely(chain >= hashtab.ht_nchains) /* Shouldn't happen. */
           chain = ptable[chain % hashtab.ht_nchains];
      else chain = ptable[chain];
     }
#if 0
     debug_printf(COLDSTR("[ELF] Failed to find symbol %q in hash table of `%q' (hash = %x)\n"),
                  name,app->a_module->m_path->p_dirent->de_name,hash);
#endif
    }
   }
   for (symtab_iter = symtab_begin;
        symtab_iter < symtab_end;
      *(uintptr_t *)&symtab_iter += self->e_dyn.di_syment) {
    char *sym_name = string_table+symtab_iter->st_name;
    if unlikely((uintptr_t)sym_name <  (uintptr_t)string_table || 
                (uintptr_t)sym_name >= (uintptr_t)string_end) break;
    if (strcmp(sym_name,name) != 0) continue;
    if (symtab_iter->st_shndx == SHN_UNDEF) goto end; /* Symbol not defined by this library. */
    result.ds_base = (void *)(uintptr_t)(u32)symtab_iter->st_value;
    result.ds_size = symtab_iter->st_size;
    if (symtab_iter->st_shndx != SHN_ABS)
      *(uintptr_t *)&result.ds_base += load_addr;
    result.ds_type = MODULE_SYMBOL_NORMAL;
    if (ELF32_ST_BIND(symtab_iter->st_info) == STB_WEAK)
        result.ds_type = MODULE_SYMBOL_WEAK;
    goto end;
   }
  } else
#endif
  {
   Elf_Sym *symtab_begin,*symtab_end,*symtab_iter;
   char *string_table = (char *)(load_addr + self->e_dyn.di_strtab);
   char *string_end = (char *)((uintptr_t)string_table+self->e_dyn.di_strsiz);
   while (string_end != string_table && string_end[-1] != '\0') --string_end;
   if unlikely(string_end == string_table) goto end;
   symtab_begin = (Elf_Sym *)(load_addr + self->e_dyn.di_symtab);
   symtab_end   = (Elf_Sym *)((uintptr_t)symtab_begin+self->e_dyn.di_symsiz);
   if (self->e_dyn.di_hashsiz != 0) {
    /* Make use of '.hash' information! */
    Elf_HashTable *phashtab;
    Elf_HashTable hashtab; Elf_Word *ptable,chain;
    phashtab = (Elf_HashTable *)(load_addr + self->e_dyn.di_hashtab);
    hashtab = *phashtab;
    COMPILER_READ_BARRIER();
    if unlikely(!hashtab.ht_nbuckts || !hashtab.ht_nbuckts) {
     /* Nope. - The hash-table is broken. */
broken_hash:
     debug_printf(COLDSTR("[ELF] Module `%q' contains invalid hash table\n"),
                  app->a_module->m_path->p_dirent->de_name);
     ATOMIC_WRITE(self->e_dyn.di_hashsiz,0);
    } else {
     Elf_Word max_attempts = hashtab.ht_nchains;
     /* Make sure the hash-table isn't too large. */
     if unlikely((sizeof(Elf_HashTable)+(hashtab.ht_nbuckts+
                                         hashtab.ht_nchains)*
                  sizeof(Elf_Word)) > self->e_dyn.di_hashsiz) goto broken_hash;
     /* Make sure the hash-table isn't trying to go out-of-bounds. */
     if unlikely(hashtab.ht_nchains > self->e_dyn.di_symcnt) goto broken_hash;
     ptable  = (Elf_Word *)(phashtab+1);
     chain   = ptable[hash % hashtab.ht_nbuckts];
     ptable += hashtab.ht_nbuckts;
     while likely(max_attempts--) {
      char *sym_name;
      if unlikely(chain == STN_UNDEF ||
                  chain >= self->e_dyn.di_symcnt)
         break;
      /* Check this candidate. */
      symtab_iter = (Elf_Sym *)((uintptr_t)symtab_begin+chain*self->e_dyn.di_syment);
      assert(symtab_iter >= symtab_begin);
      assert(symtab_iter <  symtab_end);
      sym_name = string_table+symtab_iter->st_name;
      if unlikely((uintptr_t)sym_name <  (uintptr_t)string_table || 
                  (uintptr_t)sym_name >= (uintptr_t)string_end) break;
#if 0
      debug_printf(COLDSTR("Checking hashed symbol name %q == %q (chain = %X; value = %p)\n"),
                   name,sym_name,chain,symtab_iter->st_value);
#endif
      if (strcmp(sym_name,name) != 0) goto next_candidate;
      if (symtab_iter->st_shndx == SHN_UNDEF) goto end; /* Symbol not defined by this library. */
      result.ds_base = (void *)symtab_iter->st_value;
      result.ds_size = symtab_iter->st_size;
      if (ELF_ST_TYPE(symtab_iter->st_info) == STT_TLS)
        *(uintptr_t *)&result.ds_base += (app->a_tlsoff - MODULE_TLSSIZE(app->a_module));
      else if (symtab_iter->st_shndx != SHN_ABS)
        *(uintptr_t *)&result.ds_base += load_addr;
      result.ds_type = MODULE_SYMBOL_NORMAL;
      if (ELF_ST_BIND(symtab_iter->st_info) == STB_WEAK)
          result.ds_type = MODULE_SYMBOL_WEAK;
      goto end;
next_candidate:
      if unlikely(chain >= hashtab.ht_nchains) /* Shouldn't happen. */
           chain = ptable[chain % hashtab.ht_nchains];
      else chain = ptable[chain];
     }
#if 0
     debug_printf(COLDSTR("[ELF] Failed to find symbol %q in hash table of `%q' (hash = %x)\n"),
                  name,app->a_module->m_path->p_dirent->de_name,hash);
#endif
    }
   }
   for (symtab_iter = symtab_begin;
        symtab_iter < symtab_end;
      *(uintptr_t *)&symtab_iter += self->e_dyn.di_syment) {
    char *sym_name = string_table+symtab_iter->st_name;
    if unlikely((uintptr_t)sym_name <  (uintptr_t)string_table || 
                (uintptr_t)sym_name >= (uintptr_t)string_end) break;
    if (strcmp(sym_name,name) != 0) continue;
    if (symtab_iter->st_shndx == SHN_UNDEF) goto end; /* Symbol not defined by this library. */
    result.ds_base = (void *)symtab_iter->st_value;
    result.ds_size = symtab_iter->st_size;
    if (symtab_iter->st_shndx != SHN_ABS)
      *(uintptr_t *)&result.ds_base += load_addr;
    result.ds_type = MODULE_SYMBOL_NORMAL;
    if (ELF_ST_BIND(symtab_iter->st_info) == STB_WEAK)
        result.ds_type = MODULE_SYMBOL_WEAK;
    goto end;
   }
  }
 }
end:
 return result;
}

PRIVATE bool KCALL
Elf_LoadModule(REF struct module *__restrict self) {
 REF ElfModule *EXCEPT_VAR result;
 Elf_Ehdr hdr; unsigned int i;
 inode_kreadall(&self->m_fsloc->re_node,&hdr,sizeof(hdr),0,IO_RDONLY);
 if (hdr.e_ident[EI_MAG0] != ELFMAG0) goto fail;
 if (hdr.e_ident[EI_MAG1] != ELFMAG1) goto fail;
 if (hdr.e_ident[EI_MAG2] != ELFMAG2) goto fail;
 if (hdr.e_ident[EI_MAG3] != ELFMAG3) goto fail;
 if (hdr.e_ident[EI_VERSION] != EV_CURRENT) goto fail;
 if (hdr.e_version != EV_CURRENT) goto fail;
 /*if (hdr.e_ident[EI_OSABI] != ELFOSABI_SYSV) goto fail;*/
 if ((hdr.e_type != ET_EXEC && hdr.e_type != ET_DYN) &&
     !ignore_elf_type) goto fail;
 if (!ELF_HEADER_SUPPORTED(hdr)) goto fail;
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
 if (hdr.e_ident[EI_CLASS] == ELFCLASS32) {
  /* Convert header into 64-bit. */
  Elf32_Ehdr hdr32;
  memcpy(&hdr32,&hdr,sizeof(Elf32_Ehdr));
  hdr.e_entry     = hdr32.e_entry;
  hdr.e_phoff     = hdr32.e_phoff;
  hdr.e_shoff     = hdr32.e_shoff;
  hdr.e_flags     = hdr32.e_flags;
  hdr.e_ehsize    = hdr32.e_ehsize;
  hdr.e_phentsize = hdr32.e_phentsize;
  hdr.e_phnum     = hdr32.e_phnum;
  hdr.e_shentsize = hdr32.e_shentsize;
  hdr.e_shnum     = hdr32.e_shnum;
  hdr.e_shstrndx  = hdr32.e_shstrndx;
  if (hdr.e_ehsize < sizeof(Elf32_Ehdr)) goto fail;
  if (!hdr.e_phnum) goto fail;
  if (hdr.e_phnum > 0xff) goto fail; /* Limit the max number of program headers
                                      * (there should really only be around 5-10) */
  if (hdr.e_phentsize != sizeof(Elf32_Phdr)) goto fail; /* XXX: Allow padding? */
  if (hdr.e_shentsize != sizeof(Elf32_Shdr) && hdr.e_shnum) goto fail;
 } else
#endif
 {
  if (hdr.e_ehsize < sizeof(Elf_Ehdr)) goto fail;
  if (!hdr.e_phnum) goto fail;
  if (hdr.e_phnum > 0xff) goto fail; /* Limit the max number of program headers
                                      * (there should really only be around 5-10) */
  if (hdr.e_phentsize != sizeof(Elf_Phdr)) goto fail; /* XXX: Allow padding? */
  if (hdr.e_shentsize != sizeof(Elf_Shdr) && hdr.e_shnum) goto fail;
 }

 result = (REF ElfModule *)kmalloc(offsetof(ElfModule,e_phdr)+
                                  (hdr.e_phnum*sizeof(Elf_Phdr)),
                                   GFP_SHARED|GFP_CALLOC);
 self->m_entry   = hdr.e_entry;
 self->m_machine = hdr.e_machine;
 if (hdr.e_type == ET_EXEC)
     self->m_flags = MODULE_FFIXED|MODULE_FENTRY;
 result->e_shoff    = hdr.e_shoff;
 result->e_phnum    = hdr.e_phnum;
 result->e_shnum    = hdr.e_shnum;
 result->e_shstrndx = hdr.e_shstrndx;
 TRY {
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
  if (hdr.e_ident[EI_CLASS] == ELFCLASS32) {
   unsigned int i;
   inode_kreadall(&self->m_fsloc->re_node,result->e_phdr,
                  hdr.e_phnum*sizeof(Elf32_Phdr),
                  hdr.e_phoff,IO_RDONLY);
   i = hdr.e_phnum;
   while (i--) {
    Elf32_Phdr hdr;
    memcpy(&hdr,&((Elf32_Phdr *)result->e_phdr)[i],sizeof(Elf32_Phdr));
    ELF_PHDR32TO64(result->e_phdr[i],hdr);
   }
  } else
#endif
  {
   inode_kreadall(&self->m_fsloc->re_node,result->e_phdr,
                  hdr.e_phnum*sizeof(Elf_Phdr),hdr.e_phoff,IO_RDONLY);
  }
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  kfree(result);
  error_rethrow();
 }

 /* Determine the min/max mapping of the module. */
 {
  uintptr_t image_min = (uintptr_t)-1,image_end = 0;
  for (i = 0; i < hdr.e_phnum; ++i) {
   uintptr_t phdr_begin,phdr_end;
   phdr_begin = result->e_phdr[i].p_vaddr;
   phdr_end   = phdr_begin + result->e_phdr[i].p_memsz;
   if unlikely(phdr_end <= phdr_begin) {
    /* Broken entry... */
    result->e_phdr[i].p_type = PT_NULL;
    continue;
   }
   switch (result->e_phdr[i].p_type) {
   case PT_LOAD:
    if (image_end < phdr_end)
        image_end = phdr_end;
    if (image_min > phdr_begin)
        image_min = phdr_begin;
    break;

   case PT_TLS:
    if unlikely(MODULE_HASTLS(self)) {
     /* We already had a TLS segment... */
     result->e_phdr[i].p_type = PT_NULL;
     break;
    }
    self->m_tlsmin   = phdr_begin;
    self->m_tlsend   = phdr_end;
    self->m_tlstplsz = result->e_phdr[i].p_filesz;
    self->m_tlsalign = result->e_phdr[i].p_align;
    if (self->m_tlstplsz > result->e_phdr[i].p_memsz)
        self->m_tlstplsz = result->e_phdr[i].p_memsz;
    assert(MODULE_TLSSIZE(self) == result->e_phdr[i].p_memsz);

    /* Ensure that `self->m_tlsalign' is a valid power-of-2 alignment. */
    if unlikely(!self->m_tlsalign) self->m_tlsalign = 1;
    else if unlikely((self->m_tlsalign & (self->m_tlsalign-1)) != 0) {
     size_t temp = 1;
     while (temp < self->m_tlsalign && !(temp & ((size_t)1 << ((sizeof(temp)*8)-1))))
            temp <<= 1;
     self->m_tlsalign = temp;
    }
    break;

   default: break;
   }
  }
  if unlikely(image_min >= image_end)
     goto fail_r;
  self->m_imagemin = image_min;
  self->m_imageend = image_end;
 }
 self->m_data = result;
 return true;
fail_r:
 kfree(result);
fail:
 return false;
}


/* ELF Initialization / Finalization enumerators. */
PRIVATE void KCALL
Elf_EnumInitializers(struct application *__restrict app,
                     module_enumerator_t func, void *arg) {
 uintptr_t loadaddr = app->a_loadaddr;
 ElfModule *mod = app->a_module->m_data;
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
 if (ELF_ISMACHINE32(app->a_module->m_machine)) {
  if (mod->e_dyn.di_init_array_siz) {
   u32 *begin = (u32 *)(loadaddr + mod->e_dyn.di_init_array);
   u32 *iter  = (u32 *)((uintptr_t)begin + mod->e_dyn.di_init_array_siz);
   while (iter-- > begin) SAFECALL_KCALL_VOID_2(*func,(module_callback_t)(uintptr_t)*iter,arg);
  }
  if (mod->e_dyn.di_preinit_array_siz) {
   u32 *begin = (u32 *)(loadaddr + mod->e_dyn.di_preinit_array);
   u32 *iter  = (u32 *)((uintptr_t)begin + mod->e_dyn.di_preinit_array_siz);
   while (iter-- > begin) SAFECALL_KCALL_VOID_2(*func,(module_callback_t)(uintptr_t)*iter,arg);
  }
 } else
#endif
 {
  if (mod->e_dyn.di_init_array_siz) {
   module_callback_t *begin = (module_callback_t *)(loadaddr + mod->e_dyn.di_init_array);
   module_callback_t *iter  = (module_callback_t *)((uintptr_t)begin + mod->e_dyn.di_init_array_siz);
   while (iter-- > begin) SAFECALL_KCALL_VOID_2(*func,*iter,arg);
  }
  if (mod->e_dyn.di_preinit_array_siz) {
   module_callback_t *begin = (module_callback_t *)(loadaddr + mod->e_dyn.di_preinit_array);
   module_callback_t *iter  = (module_callback_t *)((uintptr_t)begin + mod->e_dyn.di_preinit_array_siz);
   while (iter-- > begin) SAFECALL_KCALL_VOID_2(*func,*iter,arg);
  }
 }
 if (mod->e_dyn.di_init != 0)
     SAFECALL_KCALL_VOID_2(*func,(module_callback_t)(loadaddr + mod->e_dyn.di_init),arg);
}
PRIVATE void KCALL
Elf_EnumFinalizers(struct application *__restrict app,
                   module_enumerator_t func, void *arg) {
 uintptr_t loadaddr = app->a_loadaddr;
 ElfModule *mod = app->a_module->m_data;
 if (mod->e_dyn.di_fini != 0)
     SAFECALL_KCALL_VOID_2(*func,(module_callback_t)(loadaddr + mod->e_dyn.di_fini),arg);
 if (mod->e_dyn.di_fini_array_siz) {
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
  if (ELF_ISMACHINE32(app->a_module->m_machine)) {
   u32 *iter = (u32 *)(loadaddr + mod->e_dyn.di_fini_array);
   u32 *end  = (u32 *)((uintptr_t)iter + mod->e_dyn.di_fini_array_siz);
   for (; iter < end; ++iter) SAFECALL_KCALL_VOID_2(*func,(module_callback_t)(uintptr_t)*iter,arg);
  } else
#endif
  {
   module_callback_t *iter = (module_callback_t *)(loadaddr + mod->e_dyn.di_fini_array);
   module_callback_t *end  = (module_callback_t *)((uintptr_t)iter + mod->e_dyn.di_fini_array_siz);
   for (; iter < end; ++iter) SAFECALL_KCALL_VOID_2(*func,*iter,arg);
  }
 }
}

PRIVATE struct dl_section KCALL
Elf_GetSectionAddress(struct application *__restrict app,
                      USER CHECKED char const *__restrict name) {
 struct dl_section result;
 Elf_Shdr *vector; unsigned int i;
 struct module *appmod = app->a_module;
 ElfModule *mod = appmod->m_data;
 char *shstrtab,*shstrend;
 /* Load the section header string table. */
 shstrtab = Elf_LoadShStrtab(appmod);
 if unlikely(!shstrtab) goto not_found;
 vector   = mod->e_shdr;
 shstrend = shstrtab + vector[mod->e_shstrndx].sh_size;
 /* Search for a section matching the given name. */
 for (i = 0; i < mod->e_shnum; ++i) {
  char *secnam = shstrtab + vector[i].sh_name;
  if unlikely(secnam < shstrtab) continue;
  if unlikely(secnam >= shstrend) continue;
  if (strcmp(secnam,name) != 0) continue;
  /* Found it! */
  result.ds_base    = (void *)vector[i].sh_addr;
  result.ds_size    = vector[i].sh_size;
  result.ds_offset  = vector[i].sh_offset;
  result.ds_type    = vector[i].sh_type;
  result.ds_flags   = vector[i].sh_flags;
  result.ds_entsize = vector[i].sh_entsize;
  /* Verify the section's address range. */
  if unlikely((uintptr_t)result.ds_base < appmod->m_imagemin &&
             (result.ds_flags & SHF_ALLOC))
     continue;
  if unlikely((uintptr_t)result.ds_base+result.ds_size > appmod->m_imageend)
     result.ds_size = appmod->m_imageend - (uintptr_t)result.ds_base;
  *(uintptr_t *)&result.ds_base += app->a_loadaddr;
  return result;
 }
not_found:
 result.ds_size = 0;
 debug_printf("[ELF] Section %q could not be found in `%[path]'\n",
              name,app->a_module->m_path);
 return result;
}



INTERN struct module_type elf_module_type = {
    .m_magsz      = SELFMAG,
    .m_flags      = MODULE_TYPE_FPAGEALIGNED,
    .m_magic      = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3 },
    .m_driver     = &this_driver,
    .m_fini       = &Elf_FiniModule,
    .m_loadmodule = &Elf_LoadModule,
    .m_loadapp    = &Elf_NewApplication,
    .m_patchapp   = &Elf_PatchApplication,
    .m_symbol     = &Elf_GetSymbolAddress,
    .m_enuminit   = &Elf_EnumInitializers,
    .m_enumfini   = &Elf_EnumFinalizers,
    .m_section    = &Elf_GetSectionAddress
};
DEFINE_MODULE_TYPE(elf_module_type);


DECL_END

#endif /* !GUARD_KERNEL_SRC_CORE_ELF_C */
