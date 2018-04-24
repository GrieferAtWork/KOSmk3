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
#ifndef GUARD_KERNEL_MODULES_PE_LOADER_C
#define GUARD_KERNEL_MODULES_PE_LOADER_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/limits.h>
#include <hybrid/align.h>
#include <hybrid/host.h>
#include <fs/driver.h>
#include <fs/linker.h>
#include <kernel/debug.h>
#include <fs/node.h>
#include <sys/mman.h>
#include <except.h>
#include <string.h>
#include <assert.h>
#include <elf.h>

#include "loader.h"

DECL_BEGIN

#if defined(__x86_64__)
#define IMAGE_FILE_MACHINE_HOST  IMAGE_FILE_MACHINE_AMD64
#elif defined(__i386__)
#define IMAGE_FILE_MACHINE_HOST  IMAGE_FILE_MACHINE_I386
#elif defined(__arm__)
#define IMAGE_FILE_MACHINE_HOST  IMAGE_FILE_MACHINE_ARM
#else
#error "Unknown host"
#endif


PRIVATE bool KCALL
Pe_LoadModule(struct module *__restrict mod) {
 IMAGE_DOS_HEADER dosHeader;
 IMAGE_NT_HEADERS ntHeader;
 pos_t poSectionHeaderStart;
 PE_MODULE *pPeModule;
 inode_kreadall((struct inode *)mod->m_fsloc,
                &dosHeader,sizeof(dosHeader),0,
                 IO_RDONLY);
 if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
     goto nope;
 inode_kreadall((struct inode *)mod->m_fsloc,&ntHeader,
                 FIELD_OFFSET(IMAGE_NT_HEADERS,OptionalHeader),
                 dosHeader.e_lfanew,
                 IO_RDONLY);
 /* Check the NT header. */
 if (ntHeader.Signature != IMAGE_NT_SIGNATURE)
     goto nope;
 if (ntHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_HOST)
     goto nope;
 poSectionHeaderStart = (dosHeader.e_lfanew+
                         FIELD_OFFSET(IMAGE_NT_HEADERS,OptionalHeader)+
                         ntHeader.FileHeader.SizeOfOptionalHeader);
 /* Read data for the optional headers. */
 if (ntHeader.FileHeader.SizeOfOptionalHeader) {
  /* Protect against future headers. */
  if unlikely(ntHeader.FileHeader.SizeOfOptionalHeader > sizeof(ntHeader.OptionalHeader))
              ntHeader.FileHeader.SizeOfOptionalHeader = sizeof(ntHeader.OptionalHeader);
  /* Read optional NT headers. */
  inode_kreadall((struct inode *)mod->m_fsloc,
                 &ntHeader.OptionalHeader,
                  ntHeader.FileHeader.SizeOfOptionalHeader,
                  dosHeader.e_lfanew+FIELD_OFFSET(IMAGE_NT_HEADERS,OptionalHeader),
                  IO_RDONLY);
 }

 /* Make sure there are sections. */
 if unlikely(ntHeader.FileHeader.NumberOfSections == 0)
    goto nope;
 /* Limit the max number of sections. */
 if unlikely(ntHeader.FileHeader.NumberOfSections > 0xff)
    goto nope;

 /* Allocate a PE module data descriptor. */
 pPeModule = (PE_MODULE *)kmalloc(FIELD_OFFSET(PE_MODULE,pm_Sections)+
                                 (ntHeader.FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER)),
                                  GFP_SHARED);
 TRY {
  /* Clear out leading memory. */
  memset(pPeModule,0,FIELD_OFFSET(PE_MODULE,pm_Sections));

  pPeModule->pm_NumSections = ntHeader.FileHeader.NumberOfSections;
  /* Read the section headers. */
  inode_kreadall((struct inode *)mod->m_fsloc,
                  pPeModule->pm_Sections,
                 (ntHeader.FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER)),
                  poSectionHeaderStart,IO_RDONLY);
#define HAS_OPTION(opt) \
   (ntHeader.FileHeader.SizeOfOptionalHeader >= \
    COMPILER_OFFSETAFTER(IMAGE_OPTIONAL_HEADER,opt))

  if (HAS_OPTION(ImageBase)) {
   mod->m_fixedbase = ntHeader.OptionalHeader.ImageBase;
   mod->m_flags    |= MODULE_FBASEHINT;
  }
  if (HAS_OPTION(AddressOfEntryPoint)) {
   mod->m_entry  = ntHeader.OptionalHeader.AddressOfEntryPoint;
   mod->m_flags |= MODULE_FENTRY;
  }

  {
   image_rva_t image_min = (image_rva_t)-1;
   image_rva_t image_end = 0;
   WORD i;
   for (i = 0; i < pPeModule->pm_NumSections; ++i) {
    image_rva_t section_min,section_end;
    if (!SHOULD_USE_IMAGE_SECTION_HEADER(&pPeModule->pm_Sections[i]))
         continue;
    section_min  = pPeModule->pm_Sections[i].VirtualAddress;
    section_end  = section_min;
    section_end += pPeModule->pm_Sections[i].Misc.VirtualSize;
    if (image_min > section_min)
        image_min = section_min;
    if (image_end < section_end)
        image_end = section_end;
   }
   mod->m_imagemin = image_min;
   mod->m_imageend = image_end;
  }

  if (mod->m_entry <  mod->m_imagemin ||
      mod->m_entry >= mod->m_imageend) {
   /* Doesn't actually have an entry point... */
   mod->m_entry  = 0;
   mod->m_flags &= ~MODULE_FENTRY;
  }
  if (HAS_OPTION(DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]))
      pPeModule->pm_Export = ntHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
  if (HAS_OPTION(DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]))
      pPeModule->pm_Import = ntHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  if (HAS_OPTION(DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]))
      pPeModule->pm_Reloc = ntHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

  {
   unsigned int i;
   /* Validate data directories. */
   for (i = 0; i < COMPILER_LENOF(pPeModule->pm_Dir); ++i) {
    if (pPeModule->pm_Dir[i].VirtualAddress < mod->m_imagemin ||
       (pPeModule->pm_Dir[i].VirtualAddress+pPeModule->pm_Dir[i].Size) < pPeModule->pm_Dir[i].VirtualAddress ||
       (pPeModule->pm_Dir[i].VirtualAddress+pPeModule->pm_Dir[i].Size) > mod->m_imageend)
        pPeModule->pm_Dir[i].Size = 0;
   }
  }
  /* Check if the module is has a fixed load address. */
  if (!pPeModule->pm_Reloc.Size &&
      (ntHeader.FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED))
       mod->m_flags |= MODULE_FFIXED;


 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(pPeModule);
  error_rethrow();
 }
 mod->m_data = pPeModule;
 return true;
nope:
 return false;
}

PRIVATE ATTR_NOTHROW void KCALL
Pe_FiniModule(struct module *__restrict mod) {
 kfree(mod->m_data);
}

LOCAL ATTR_RETNONNULL REF struct vm_region **KCALL
PeModule_AllocRegions(PE_MODULE *__restrict pPeModule,
                      struct module *__restrict pModule) {
 REF struct vm_region **old_result; WORD EXCEPT_VAR i;
 REF struct vm_region **EXCEPT_VAR result;
 result = ATOMIC_READ(pPeModule->pm_Regions);
 if (result != NULL) return result;
 result = (REF struct vm_region **)kmalloc(pPeModule->pm_NumSections*
                                           sizeof(REF struct vm_region *),
                                           GFP_SHARED);
 i = 0;
 TRY {
  /* Construct PE regions. */
  for (; i < pPeModule->pm_NumSections; ++i) {
   PIMAGE_SECTION_HEADER pSection;
   REF struct vm_region *pRegion;
   size_t page_offset;
   pSection = &pPeModule->pm_Sections[i];
   if unlikely(!SHOULD_USE_IMAGE_SECTION_HEADER(pSection)) {
    result[i] = NULL;
    continue;
   }
   /* Allocate a new VM region. */
   page_offset = pSection->VirtualAddress & (PAGESIZE-1);
   pRegion = vm_region_alloc(CEILDIV(pSection->Misc.VirtualSize+
                                     page_offset,
                                     PAGESIZE));
   pRegion->vr_flags |= VM_REGION_FCANTSHARE;
   pRegion->vr_part0.vp_refcnt = 1; /* Keep one reference for the `ElfModule' */
   if (pSection->SizeOfRawData != 0) {
    pRegion->vr_init = VM_REGION_INIT_FFILE_RO;
    pRegion->vr_setup.s_file.f_node  = &pModule->m_fsloc->re_node;
    inode_incref(pRegion->vr_setup.s_file.f_node);
    pRegion->vr_setup.s_file.f_begin = page_offset;
    pRegion->vr_setup.s_file.f_start = pSection->PointerToRawData;
    pRegion->vr_setup.s_file.f_size  = pSection->SizeOfRawData;
   } else {
    /* .bss style section. */
    pRegion->vr_init = VM_REGION_INIT_FFILLER;
   }  
   result[i] = pRegion;
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  while (i--) {
   if (result[i])
       vm_region_decref(result[i]);
  }
  kfree(result);
  error_rethrow();
 }
 old_result = ATOMIC_CMPXCH_VAL(pPeModule->pm_Regions,
                                NULL,
                                result);
 if likely(old_result == NULL) return result;
 for (i = 0; i < pPeModule->pm_NumSections; ++i) {
  if (result[i])
      vm_region_decref(result[i]);
 }
 kfree(result);
 return old_result;
}

PRIVATE void KCALL
Pe_LoadApp(struct module_patcher *__restrict self) {
 WORD i; REF struct vm_region **regions;
 struct application *pApp = self->mp_app;
 struct module *pModule = pApp->a_module;
 PE_MODULE *pPeModule = pModule->m_data;
 uintptr_t pLoadPage = pApp->a_loadaddr;
 vm_prot_t forceProt = 0;
 if (self->mp_apptype & APPLICATION_TYPE_FDRIVER)
     forceProt |= PROT_NOUSER;
 assert(IS_ALIGNED(pLoadPage,PAGESIZE));
 pLoadPage /= PAGESIZE;

 regions = PeModule_AllocRegions(pPeModule,pModule);
 i = 0;
 TRY {
  for (; i < pPeModule->pm_NumSections; ++i) {
   PIMAGE_SECTION_HEADER pSection;
   vm_prot_t sectionProt = forceProt;
   if unlikely(!regions[i]) continue; /* Unused region */
   pSection = &pPeModule->pm_Sections[i];
   if (pSection->Characteristics & IMAGE_SCN_MEM_SHARED)
       sectionProt |= PROT_SHARED;
   if (pSection->Characteristics & IMAGE_SCN_MEM_EXECUTE)
       sectionProt |= PROT_EXEC;
   if ((pSection->Characteristics & IMAGE_SCN_MEM_READ) ||
      !(pSection->Characteristics & (IMAGE_SCN_LNK_REMOVE|IMAGE_SCN_LNK_INFO)))
        sectionProt |= PROT_READ;
   if (pSection->Characteristics & IMAGE_SCN_MEM_WRITE)
       sectionProt |= PROT_WRITE;

   /* Map application segments. */
   vm_mapat(pLoadPage + VM_ADDR2PAGE(pSection->VirtualAddress),
            regions[i]->vr_size,0,regions[i],sectionProt,
           &application_notify,pApp);
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Unmap everything that had already been mapped. */
  while (i--) {
   if (!regions[i]) continue;
   vm_unmap(pLoadPage + VM_ADDR2PAGE(pPeModule->pm_Sections[i].VirtualAddress),
            regions[i]->vr_size,VM_UNMAP_TAG|VM_UNMAP_NOEXCEPT,pApp);
  }
  /* Make sure that unmapped memory is synced immediately.
   * NOTE: An unmap() race condition is prevented because the
   *       caller is holding a lock on the effective VM. */
  vm_sync(pLoadPage+pModule->m_imagemin,
          pModule->m_imageend-
          pModule->m_imagemin);
  error_rethrow();
 }
}



PRIVATE void KCALL
Pe_PatchApp(struct module_patcher *__restrict self) {
 /* TODO */
 debug_printf("TODO: Pe_PatchApp()\n");
}


PRIVATE struct module_symbol KCALL
Pe_GetSymbol(struct application *__restrict app,
             USER CHECKED char const *__restrict name,
             u32 hash) {
 struct module_symbol result;
 /* XXX: Lazily fill in a symbol table cache. */
 /* TODO */
 result.ms_type = MODULE_SYMBOL_INVALID;
 return result;
}

PRIVATE struct module_section KCALL
Pe_GetSection(struct application *__restrict app,
              USER CHECKED char const *__restrict name) {
 struct module_section result; WORD i;
 PE_MODULE *pPeModule = app->a_module->m_data;
 for (i = 0; i < pPeModule->pm_NumSections; ++i) {
  DWORD dwSectionFlags;
  if (strcmp((char *)pPeModule->pm_Sections[i].Name,name) != 0)
      continue;
  /* Found it! */
  result.ms_base    = pPeModule->pm_Sections[i].VirtualAddress;
  result.ms_size    = pPeModule->pm_Sections[i].SizeOfRawData;
  result.ms_offset  = pPeModule->pm_Sections[i].PointerToRawData;
  result.ms_type    = SHT_PROGBITS;
  result.ms_flags   = 0;
  result.ms_entsize = 0;
  dwSectionFlags    = pPeModule->pm_Sections[i].Characteristics;
  if (dwSectionFlags & IMAGE_SCN_LNK_INFO)
   result.ms_type = SHT_NOTE;
  else if (!(dwSectionFlags & IMAGE_SCN_LNK_REMOVE)) {
   result.ms_flags |= SHF_ALLOC;
  }
  if (dwSectionFlags & IMAGE_SCN_MEM_EXECUTE)
      result.ms_flags |= SHF_EXECINSTR;
  if (dwSectionFlags & IMAGE_SCN_MEM_READ)
      result.ms_flags |= SHF_ALLOC;
  if (dwSectionFlags & IMAGE_SCN_MEM_WRITE)
      result.ms_flags |= SHF_WRITE;
  if ((dwSectionFlags & IMAGE_SCN_CNT_UNINITIALIZED_DATA) &&
       result.ms_size == 0) {
   result.ms_type = SHT_NOBITS;
   result.ms_size = pPeModule->pm_Sections[i].Misc.VirtualSize;
  }
  return result;
 }
 result.ms_size  = 0;
 result.ms_flags = 0;
 return result;
}


PRIVATE struct module_type pe_type = {
    .m_flags      = MODULE_TYPE_FPAGEALIGNED,
    .m_magsz      = 2,
    .m_magic      = { 'M', 'Z' },
    .m_driver     = &this_driver,
    .m_loadmodule = &Pe_LoadModule,
    .m_fini       = &Pe_FiniModule,
    .m_loadapp    = &Pe_LoadApp,
    .m_patchapp   = &Pe_PatchApp,
    .m_symbol     = &Pe_GetSymbol,
    .m_section    = &Pe_GetSection,
};
DEFINE_MODULE_TYPE(pe_type);

DECL_END

#endif /* !GUARD_KERNEL_MODULES_PE_LOADER_C */
