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
#ifndef GUARD_KERNEL_MODULES_A_OUT_A_OUT_C
#define GUARD_KERNEL_MODULES_A_OUT_A_OUT_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/align.h>
#include <hybrid/limits.h>
#include <kos/types.h>
#include <fs/linker.h>
#include <fs/driver.h>
#include <fs/node.h>
#include <kernel/vm.h>
#include <hybrid/wordbits.h>
#include <string.h>
#include <elf.h>
#include <a.out.h>

DECL_BEGIN


/* Indices for the `ao_regions' vector. */
#define AO_REGION_TEXT  0
#define AO_REGION_DATA  1
#define AO_REGION_BSS   2
#define AO_REGION_COUNT 3

PRIVATE vm_prot_t const ao_region_prot[AO_REGION_COUNT] = {
    [AO_REGION_TEXT] = PROT_READ|PROT_EXEC,
    [AO_REGION_DATA] = PROT_READ|PROT_WRITE,
    [AO_REGION_BSS]  = PROT_READ|PROT_WRITE
};

struct ao_region {
    image_rva_t ar_begin;  /* [valid_if(ar_length)] Starting address in memory. */
    image_rva_t ar_length; /* Number of bytes in memory. */
    pos_t       ar_start;  /* [valid_if(ar_length && INDEXOF(self) != AO_REGION_BSS)]
                            * Starting address of file data. */
};


typedef struct module_data {
    struct exec           ao_header;                   /* The file header of the a.out binary. */
    REF struct vm_region *ao_regions[AO_REGION_COUNT]; /* [0..1][lock(WRITE_ONCE)][*] Lazily allocated regions. */
    struct ao_region      ao_sect[AO_REGION_COUNT];    /* Sections. */
} AOut;


PRIVATE bool KCALL
AOut_LoadModule(struct module *__restrict mod) {
 struct exec header; AOut *ao;
 image_rva_t module_min,module_end,temp;
 inode_kreadall(&mod->m_fsloc->re_node,
                &header,sizeof(header),0,
                 IO_RDONLY);
 if (N_BADMAG(header))
     return false;
#if defined(__i386__) || defined(__x86_64__)
 /* NOTE: As an extension, KOS allows loading of a.out binaries on x86-64 */
 if (N_MACHTYPE(header) != M_386)
     return false;
#else
#error "Unsuported a.out architecture"
#endif
 if (!header.a_text && !header.a_data)
      return false; /* You need to give me ~something~ */
 mod->m_flags |= MODULE_FENTRY;
 mod->m_entry  = header.a_entry;
 ao = (AOut *)kmalloc(sizeof(AOut),GFP_SHARED);
 memset(ao->ao_regions,0,sizeof(ao->ao_regions));
 memcpy(&ao->ao_header,&header,sizeof(struct exec));
 ao->ao_sect[AO_REGION_TEXT].ar_length = header.a_text;
 ao->ao_sect[AO_REGION_DATA].ar_length = header.a_data;
 ao->ao_sect[AO_REGION_BSS].ar_length  = header.a_bss;
 module_min = (image_rva_t)-1;
 module_end = 0;
 if (header.a_text) {
  temp = N_TXTADDR(header);
  if unlikely(!IS_ALIGNED(temp,PAGESIZE))
     goto nope_ao;
  ao->ao_sect[AO_REGION_TEXT].ar_begin = temp;
  ao->ao_sect[AO_REGION_TEXT].ar_start = N_TXTOFF(header);
  if (module_min > temp)
      module_min = temp;
  temp += header.a_text;
  if (module_end < temp)
      module_end = temp;
 }
 if (header.a_data) {
  temp = N_DATADDR(header);
  if unlikely(!IS_ALIGNED(temp,PAGESIZE))
     goto nope_ao;
  ao->ao_sect[AO_REGION_DATA].ar_begin = temp;
  ao->ao_sect[AO_REGION_DATA].ar_start = N_DATOFF(header);
  if (module_min > temp)
      module_min = temp;
  temp += header.a_data;
  if (module_end < temp)
      module_end = temp;
 }
 if (header.a_bss) {
  temp = N_BSSADDR(header);
  if unlikely(!IS_ALIGNED(temp,PAGESIZE))
     goto nope_ao;
  ao->ao_sect[AO_REGION_BSS].ar_begin = temp;
  if (module_min > temp)
      module_min = temp;
  temp += header.a_bss;
  if (module_end < temp)
      module_end = temp;
 }
 mod->m_imagemin = module_min;
 mod->m_imageend = module_end;
 /* Without relocations, the module is fixed. */
 if (!header.a_drsize && !header.a_trsize)
      mod->m_flags |= MODULE_FFIXED;
 return true;
nope_ao:
 kfree(ao);
 return false;
}

PRIVATE ATTR_NOTHROW void KCALL
AOut_FiniModule(struct module *__restrict mod) {
 AOut *ao = mod->m_data;
 if (ao) {
  unsigned int i;
  for (i = 0; i < AO_REGION_COUNT; ++i)
     if (ao->ao_regions[i])
         vm_region_decref(ao->ao_regions[i]);
  kfree(ao);
 }
}


PRIVATE REF struct vm_region *KCALL
AOut_ConstructRegion(struct module *__restrict mod,
                     unsigned int index) {
 REF struct vm_region *result;
 AOut *ao = mod->m_data;
 result = vm_region_alloc(CEILDIV(ao->ao_sect[index].ar_length,PAGESIZE));
 result->vr_flags |= (VM_REGION_FCANTSHARE|VM_REGION_FDONTMERGE);
 if (index == AO_REGION_BSS) {
  /* Initialize the segment using ZERO-memory. */
  result->vr_init = VM_REGION_INIT_FFILLER;
  result->vr_setup.s_filler = 0;
 } else {
  /* Initialize as a file ->to-> memory mapping. */
  result->vr_init                  = VM_REGION_INIT_FFILE_RO;
  result->vr_setup.s_file.f_node   = &mod->m_fsloc->re_node;
  result->vr_setup.s_file.f_begin  = 0;
  result->vr_setup.s_file.f_start  = ao->ao_sect[index].ar_start;
  result->vr_setup.s_file.f_size   = ao->ao_sect[index].ar_length;
  result->vr_setup.s_file.f_filler = 0;
  inode_incref(result->vr_setup.s_file.f_node);
 }
 return result;
}


PRIVATE void KCALL
AOut_LoadApp(struct module_patcher *__restrict self) {
 struct application *app = self->mp_app;
 AOut *ao = app->a_module->m_data;
 uintptr_t loadpage = VM_ADDR2PAGE(app->a_loadaddr);
 unsigned int i; vm_prot_t force_prot = PROT_NONE;
 if (app->a_type & APPLICATION_TYPE_FDRIVER)
     force_prot |= PROT_NOUSER;
 for (i = 0; i < AO_REGION_COUNT; ++i) {
  struct vm_region *region;
  /* Skip empty segments. */
  if (!ao->ao_sect[i].ar_length) continue;
  /* Lookup, or lazily construct the VM region for this segment. */
  region = ATOMIC_READ(ao->ao_regions[i]);
  if (!region) {
   struct vm_region *old_region;
   region = AOut_ConstructRegion(app->a_module,i);
   /* Safe the newly constructed region. */
   old_region = ATOMIC_CMPXCH_VAL(ao->ao_regions[i],NULL,region);
   if unlikely(old_region) {
    /* Use the region that was already constructed before. */
    vm_region_decref(region);
    region = old_region;
   }
  }
  /* Map this segment to memory. */
  vm_mapat(loadpage + VM_ADDR2PAGE(ao->ao_sect[i].ar_begin),
           CEILDIV(ao->ao_sect[i].ar_length,PAGESIZE),
           0,
           region,
           ao_region_prot[i],
          &application_notify,
           app);
 }
}

PRIVATE void KCALL
AOut_PatchApp(struct module_patcher *__restrict self) {
 /* TODO */
}


PRIVATE struct dl_symbol KCALL
AOut_GetSymbol(struct application *__restrict app,
               USER CHECKED char const *__restrict name,
               u32 UNUSED(hash)) {
 struct dl_symbol result;
 /* TODO */
 result.ds_type = MODULE_SYMBOL_INVALID;
 return result;
}


PRIVATE struct dl_section KCALL
AOut_GetSection(struct application *__restrict app,
                USER CHECKED char const *__restrict name) {
 struct dl_section result;
 AOut *ao = app->a_module->m_data;
 if (*name++ != '.') goto unknown;
 /* Manually recognize common names for sections. */
 if (!strcmp(name,"text") &&
      ao->ao_sect[AO_REGION_TEXT].ar_length) {
  result.ds_base   = (void *)(app->a_loadaddr + ao->ao_sect[AO_REGION_TEXT].ar_begin);
  result.ds_size   = ao->ao_sect[AO_REGION_TEXT].ar_length;
  result.ds_offset = ao->ao_sect[AO_REGION_TEXT].ar_start;
  result.ds_type   = SHT_PROGBITS;
  result.ds_flags  = SHF_ALLOC|SHF_EXECINSTR;
  return result;
 }
 if (!strcmp(name,"data") &&
      ao->ao_sect[AO_REGION_DATA].ar_length) {
  result.ds_base   = (void *)(app->a_loadaddr + ao->ao_sect[AO_REGION_DATA].ar_begin);
  result.ds_size   = ao->ao_sect[AO_REGION_DATA].ar_length;
  result.ds_offset = ao->ao_sect[AO_REGION_DATA].ar_start;
  result.ds_type   = SHT_PROGBITS;
  result.ds_flags  = SHF_ALLOC|SHF_WRITE;
  return result;
 }
 if (!strcmp(name,"bss") &&
      ao->ao_sect[AO_REGION_BSS].ar_length) {
  result.ds_base  = (void *)(app->a_loadaddr + ao->ao_sect[AO_REGION_BSS].ar_begin);
  result.ds_size  = ao->ao_sect[AO_REGION_BSS].ar_length;
  result.ds_type  = SHT_NOBITS;
  result.ds_flags = SHF_ALLOC|SHF_WRITE;
  return result;
 }
 if (!strcmp(name,"rel.text") && N_TRSIZE(ao->ao_header)) {
  /* Don't use `SHT_REL', because a.out uses its own format! */
  result.ds_size    = N_TRSIZE(ao->ao_header);
  result.ds_offset  = N_TRELOFF(ao->ao_header);
  result.ds_type    = SHT_PROGBITS;
  result.ds_flags   = 0;
  result.ds_entsize = sizeof(struct relocation_info);
  return result;
 }
 if (!strcmp(name,"rel.data") && N_DRSIZE(ao->ao_header)) {
  /* Don't use `SHT_REL', because a.out uses its own format! */
  result.ds_size    = N_DRSIZE(ao->ao_header);
  result.ds_offset  = N_DRELOFF(ao->ao_header);
  result.ds_type    = SHT_PROGBITS;
  result.ds_flags   = 0;
  result.ds_entsize = sizeof(struct relocation_info);
  return result;
 }
 if (!strcmp(name,"symtab") && N_SYMSIZE(ao->ao_header)) {
  /* Don't use `SHT_SYMTAB', because a.out uses its own format! */
  result.ds_size    = N_SYMSIZE(ao->ao_header);
  result.ds_offset  = N_DRELOFF(ao->ao_header);
  result.ds_type    = SHT_PROGBITS;
  result.ds_flags   = 0;
  result.ds_entsize = sizeof(struct nlist);
  return result;
 }
unknown:
 result.ds_size = 0;
 return result;
}

INTERN struct module_type AOut_ModuleTypes[] = {
    {   .m_flags      = MODULE_TYPE_FPAGEALIGNED,
        .m_magsz      = 2,
        .m_magic      = { INT16_BYTE(OMAGIC,0), INT16_BYTE(OMAGIC,1) },
        .m_driver     = &this_driver,
        .m_loadmodule = &AOut_LoadModule,
        .m_fini       = &AOut_FiniModule,
        .m_loadapp    = &AOut_LoadApp,
        .m_patchapp   = &AOut_PatchApp,
        .m_symbol     = &AOut_GetSymbol,
        .m_section    = &AOut_GetSection,
    },
    {   .m_flags      = MODULE_TYPE_FPAGEALIGNED,
        .m_magsz      = 2,
        .m_magic      = { INT16_BYTE(NMAGIC,0), INT16_BYTE(NMAGIC,1) },
        .m_driver     = &this_driver,
        .m_loadmodule = &AOut_LoadModule,
        .m_fini       = &AOut_FiniModule,
        .m_loadapp    = &AOut_LoadApp,
        .m_patchapp   = &AOut_PatchApp,
        .m_symbol     = &AOut_GetSymbol,
        .m_section    = &AOut_GetSection,
    },
    {   .m_flags      = MODULE_TYPE_FPAGEALIGNED,
        .m_magsz      = 2,
        .m_magic      = { INT16_BYTE(ZMAGIC,0), INT16_BYTE(ZMAGIC,1) },
        .m_driver     = &this_driver,
        .m_loadmodule = &AOut_LoadModule,
        .m_fini       = &AOut_FiniModule,
        .m_loadapp    = &AOut_LoadApp,
        .m_patchapp   = &AOut_PatchApp,
        .m_symbol     = &AOut_GetSymbol,
        .m_section    = &AOut_GetSection,
    },
    {   .m_flags      = MODULE_TYPE_FPAGEALIGNED,
        .m_magsz      = 2,
        .m_magic      = { INT16_BYTE(QMAGIC,0), INT16_BYTE(QMAGIC,1) },
        .m_driver     = &this_driver,
        .m_loadmodule = &AOut_LoadModule,
        .m_fini       = &AOut_FiniModule,
        .m_loadapp    = &AOut_LoadApp,
        .m_patchapp   = &AOut_PatchApp,
        .m_symbol     = &AOut_GetSymbol,
        .m_section    = &AOut_GetSection,
    }
};

DEFINE_DRIVER_INIT(aout_init);
PRIVATE ATTR_USED ATTR_FREETEXT void KCALL aout_init(void) {
 unsigned int i;
 /* Register all a.out module type variants. */
 for (i = 0; i < COMPILER_LENOF(AOut_ModuleTypes); ++i)
     register_module_type(&AOut_ModuleTypes[i]);
}

DECL_END

#endif /* !GUARD_KERNEL_MODULES_A_OUT_A_OUT_C */
