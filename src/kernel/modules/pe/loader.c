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
#include <fs/path.h>
#include <fs/driver.h>
#include <fs/linker.h>
#include <kernel/malloc.h>
#include <kernel/debug.h>
#include <kernel/user.h>
#include <fs/node.h>
#include <kos/bound.h>
#include <sys/mman.h>
#include <alloca.h>
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
  /* Validate the size of the relocation section. */
  if (pPeModule->pm_Reloc.Size < sizeof(IMAGE_BASE_RELOCATION))
      pPeModule->pm_Reloc.Size = 0;
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
 WORD i; REF struct vm_region **pRegions;
 struct application *pApp = self->mp_app;
 struct module *pModule = pApp->a_module;
 PE_MODULE *pPeModule = pModule->m_data;
 uintptr_t pLoadPage = pApp->a_loadaddr;
 vm_prot_t forceProt = 0;
 if (self->mp_apptype & APPLICATION_TYPE_FDRIVER)
     forceProt |= PROT_NOUSER;
 assert(IS_ALIGNED(pLoadPage,PAGESIZE));
 pLoadPage /= PAGESIZE;

 pRegions = PeModule_AllocRegions(pPeModule,pModule);
 i = 0;
 TRY {
  for (; i < pPeModule->pm_NumSections; ++i) {
   PIMAGE_SECTION_HEADER pSection;
   vm_prot_t sectionProt = forceProt;
   if unlikely(!pRegions[i]) continue; /* Unused region */
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
            pRegions[i]->vr_size,0,pRegions[i],sectionProt,
           &application_notify,pApp);
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Unmap everything that had already been mapped. */
  while (i--) {
   if (!pRegions[i]) continue;
   vm_unmap(pLoadPage + VM_ADDR2PAGE(pPeModule->pm_Sections[i].VirtualAddress),
            pRegions[i]->vr_size,VM_UNMAP_TAG|VM_UNMAP_NOEXCEPT,pApp);
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
make_writable(PE_MODULE *__restrict mod, uintptr_t loadaddr,
              struct application *__restrict app) {
 unsigned int i; REF struct vm_region **vector;
 vector = mod->pm_Regions;
 for (i = 0; i < mod->pm_NumSections; ++i) {
  if (!vector[i]) continue;
  if (mod->pm_Sections[i].Characteristics & IMAGE_SCN_MEM_WRITE) continue;
  /* Add write permissions to this segment. */
  vm_protect(VM_ADDR2PAGE(loadaddr+mod->pm_Sections[i].VirtualAddress),
             vector[i]->vr_size,~0,PROT_WRITE,VM_PROTECT_TAG,app);
 }
}
PRIVATE void KCALL
make_readonly(PE_MODULE *__restrict mod, uintptr_t loadaddr,
              struct application *__restrict app) {
 unsigned int i; REF struct vm_region **vector;
 vector = mod->pm_Regions;
 for (i = 0; i < mod->pm_NumSections; ++i) {
  if (!vector[i]) continue;
  if (mod->pm_Sections[i].Characteristics & IMAGE_SCN_MEM_WRITE) continue;
  /* Remove write permissions from this segment. */
  vm_protect(VM_ADDR2PAGE(loadaddr+mod->pm_Sections[i].VirtualAddress),
             vector[i]->vr_size,~PROT_WRITE,0,VM_PROTECT_TAG,app);
 }
}


PRIVATE ATTR_NOINLINE LPVOID KCALL
Pe_ImportSymbol(struct application *__restrict pApp,
                struct application *__restrict pDependency,
                PIMAGE_IMPORT_BY_NAME pEntry) {
 struct dl_symbol sym;
 if (pDependency->a_module->m_type == &Pe_ModuleType) {
  /* TODO: Use `pEntry->Hint' */
 } else if (memcmp(pEntry->Name,"KOS$",4) != 0) {
  /* NOTE: Only do KOS/DOS indirect linking when
   *       combining PE and non-PE (ELF) modules. */
  size_t szSymbolLength;
  LPSTR pDosSymbolName;
  /* Prefer linking against DOS$ symbols. */
  szSymbolLength = strnlen((char *)pEntry->Name,512);
  pDosSymbolName = (LPSTR)malloca((5+szSymbolLength)*sizeof(CHAR));
  pDosSymbolName[0] = 'D';
  pDosSymbolName[1] = 'O';
  pDosSymbolName[2] = 'S';
  pDosSymbolName[3] = '$';
  memcpy(pDosSymbolName+4,pEntry->Name,szSymbolLength*sizeof(CHAR));
  pDosSymbolName[szSymbolLength+4] = '\0';
  sym = application_dlsym(pDependency,pDosSymbolName);
  if (sym.ds_type != MODULE_SYMBOL_INVALID)
      return sym.ds_base;
 }
 /* Symbol Do a regular symbol lookup. */
 sym = application_dlsym(pDependency,(char *)pEntry->Name);
 if (sym.ds_type != MODULE_SYMBOL_INVALID)
     return sym.ds_base;
 /* Symbol wasn't found... */
 debug_printf(COLDSTR("[PE] Failed to patch symbol %q in %q, imported from %q\n"),
              pEntry->Name,
              pApp->a_module->m_path->p_dirent->de_name,
              pDependency->a_module->m_path->p_dirent->de_name);
 error_throw(E_NOT_EXECUTABLE);
}


PRIVATE void KCALL
Pe_PatchApp(struct module_patcher *__restrict self) {
 struct module_patcher *EXCEPT_VAR xself = self;
 struct application *EXCEPT_VAR pApp = self->mp_app;
 struct module *EXCEPT_VAR pModule = pApp->a_module;
 PE_MODULE *EXCEPT_VAR pPeModule = pModule->m_data;
 uintptr_t EXCEPT_VAR pLoadAddr = pApp->a_loadaddr;
 /* Deal with import tables. */
 if (pPeModule->pm_Import.Size >= sizeof(IMAGE_IMPORT_DESCRIPTOR)) {
  PIMAGE_IMPORT_DESCRIPTOR EXCEPT_VAR pImportIter;
  PIMAGE_IMPORT_DESCRIPTOR EXCEPT_VAR pImportEnd;
  pImportIter = (PIMAGE_IMPORT_DESCRIPTOR)(pLoadAddr +
                                           pPeModule->pm_Import.VirtualAddress);
  pImportEnd  = (PIMAGE_IMPORT_DESCRIPTOR)((uintptr_t)pImportIter+
                                           pPeModule->pm_Import.Size);
  /* Since this is a PE binary, make sure to
   * ignore casing when loading dependencies. */
  self->mp_flags |= DL_OPEN_FNOCASE;
  for (; pImportIter < pImportEnd; ++pImportIter) {
   LPSTR EXCEPT_VAR pImportFilename;
   struct application *EXCEPT_VAR pDependency;
   /* The import is terminated by a ZERO-entry.
    * https://msdn.microsoft.com/en-us/library/ms809762.aspx */
   if (!pImportIter->Characteristics &&
       !pImportIter->TimeDateStamp &&
       !pImportIter->ForwarderChain &&
       !pImportIter->Name &&
       !pImportIter->FirstThunk)
        break;
   pImportFilename = (LPSTR)(pLoadAddr+
                             pImportIter->Name);
   ASSERT_BOUNDS(pApp->a_bounds,(uintptr_t)pImportFilename);
   TRY {
    pDependency = patcher_require_string(xself,pImportFilename,
                                         user_strlen(pImportFilename));
   } CATCH (E_FILESYSTEM_ERROR) {
    u16 fs_code = error_info()->e_error.e_filesystem_error.fs_errcode;
    if (ERROR_FS_IS_NOT_FOUND(fs_code) &&
        memcasecmp(pImportFilename,"msvcr",5*sizeof(char)) == 0) {
     error_handled();
     /* Substitute msvcr* libraries with `libc.so'. */
     pDependency = patcher_require_string(xself,"libc.so",7);
    } else {
     debug_printf("[PE] Cannot find dependency %p:%q\n",pImportFilename,pImportFilename);
     error_rethrow();
    }
   }
   {
    /* Import module symbols. */
    PIMAGE_THUNK_DATA pThunkIter;
    PIMAGE_THUNK_DATA pThunk2Iter;
    pThunkIter  = (IMAGE_THUNK_DATA *)(pLoadAddr+
                                      (pImportIter->OriginalFirstThunk
                                     ? pImportIter->OriginalFirstThunk
                                     : pImportIter->FirstThunk));
    pThunk2Iter = (IMAGE_THUNK_DATA *)(pLoadAddr+
                                      (pImportIter->FirstThunk
                                     ? pImportIter->FirstThunk
                                     : pImportIter->OriginalFirstThunk));
    for (;; ++pThunkIter,++pThunk2Iter) {
     PIMAGE_IMPORT_BY_NAME pImportEntry;
     LPVOID pImportAddress;
     ASSERT_BOUNDS(pApp->a_bounds,(uintptr_t)pThunkIter);
     if (!pThunkIter->u1.AddressOfData)
          break; /* ZERO-Terminated. */
     ASSERT_BOUNDS(pApp->a_bounds,(uintptr_t)pThunk2Iter);
     pImportEntry = (PIMAGE_IMPORT_BY_NAME)(pLoadAddr+pThunkIter->u1.AddressOfData);
     ASSERT_BOUNDS(pApp->a_bounds,(uintptr_t)pImportEntry);
     /* Import the symbol. */
     pImportAddress = Pe_ImportSymbol(pApp,pDependency,pImportEntry);
     pThunkIter->u1.AddressOfData  = (uintptr_t)pImportAddress;
     pThunk2Iter->u1.AddressOfData = (uintptr_t)pImportAddress;
    }
   }
  }
 }

 /* Deal with relocations. */
 if (pPeModule->pm_Reloc.Size != 0 &&
     pLoadAddr != pModule->m_fixedbase) {
  bool EXCEPT_VAR bChangedToWritable = false;
  if (pPeModule->pm_Flags & PE_MODULE_FTEXTREL)
      make_writable(pPeModule,pLoadAddr,pApp),
      bChangedToWritable = true;
  TRY {
   IMAGE_BASE_RELOCATION *EXCEPT_VAR pBlockIter;
   IMAGE_BASE_RELOCATION *EXCEPT_VAR pBlockEnd;
   uintptr_t EXCEPT_VAR pRelocDelta;
   pRelocDelta = pLoadAddr - pModule->m_fixedbase;
   pBlockIter = (IMAGE_BASE_RELOCATION *)pPeModule->pm_Reloc.VirtualAddress;
   pBlockEnd  = (IMAGE_BASE_RELOCATION *)((uintptr_t)pBlockIter+pPeModule->pm_Reloc.Size/
                                           sizeof(IMAGE_BASE_RELOCATION));
   while (pBlockIter < pBlockEnd) {
    DWORD dwMaxBlockSize;
    DWORD dwBlockSize = ATOMIC_READ(pBlockIter->SizeOfBlock);
    WORD *pRelIter,*pRelEnd; uintptr_t pBlockBase;
    if unlikely(!dwBlockSize) dwBlockSize = 1;
    if unlikely(dwBlockSize < sizeof(IMAGE_BASE_RELOCATION)) {
     *(uintptr_t *)&pBlockIter += dwBlockSize;
     continue;
    }
    dwMaxBlockSize = (DWORD)((uintptr_t)pBlockEnd-(uintptr_t)pBlockIter);
    if unlikely(dwBlockSize > dwMaxBlockSize)
                dwBlockSize = dwMaxBlockSize;
    pBlockBase = pLoadAddr + ATOMIC_READ(pBlockIter->VirtualAddress);
#if PAGESIZE >= 0x1000
    ASSERT_BOUNDS(pApp->a_bounds,pBlockBase);
#endif
    pRelIter = (WORD *)((uintptr_t)pBlockIter + sizeof(IMAGE_BASE_RELOCATION));
    pRelEnd  = (WORD *)((uintptr_t)pRelIter + dwBlockSize);
    for (; pRelIter < pRelEnd; ++pRelIter) {
restart_rel_iter:
     TRY {
      /* Execute the relocations. */
      WORD wKey = ATOMIC_READ(*pRelIter);
      uintptr_t pRelAddr = pBlockBase + (wKey & 0xfff);
      /* PE uses a 4-bit ID to differentiate between relocation types
       * (meaning there can only ever be up to 16 of them) */
      wKey >>= 12;
#if PAGESIZE < 0x1000
      ASSERT_BOUNDS(pApp->a_bounds,pRelAddr);
#endif

      switch (wKey) {
      case IMAGE_REL_BASED_ABSOLUTE:
       /* QUOTE:"The base relocation is skipped" */
       break;

#if __SIZEOF_POINTER__ > 4
      case IMAGE_REL_BASED_HIGHLOW:
       *(u32 *)pRelAddr += pRelocDelta & 0xffffffffull;
       break;
      case IMAGE_REL_BASED_HIGHADJ:
       *(u32 *)pRelAddr += (pRelocDelta & 0xffffffff00000000ull) >> 32;
       break;
#else
      case IMAGE_REL_BASED_HIGHLOW:
      case IMAGE_REL_BASED_HIGHADJ:
       *(u32 *)pRelAddr += pRelocDelta;
       break;
#endif

      case IMAGE_REL_BASED_LOW:
       *(u16 *)pRelAddr += pRelocDelta & 0xffff;
       break;
      case IMAGE_REL_BASED_HIGH:
       *(u16 *)pRelAddr += (pRelocDelta & 0xffff0000) >> 16;
       break;

      case IMAGE_REL_BASED_DIR64:
       *(u64 *)pRelAddr += (u64)pRelocDelta;
       break;

      case IMAGE_REL_BASED_IA64_IMM64: /* TODO? */
      default:
       debug_printf("[PE] Unsupported relocation %I8u against %p\n",
                     wKey,pRelAddr);
       break;
      }
     } CATCH (E_SEGFAULT) {
      /* Check if the segfault was caused because of a write,
       * and if it was, try to re-load read-only segments as
       * writable. */
      if (bChangedToWritable)
          error_rethrow();
#if defined(__i386__) || defined(__x86_64__)
      if (!(error_info()->e_error.e_segfault.sf_reason & X86_SEGFAULT_FWRITE))
            error_rethrow();
#endif
      error_handled();
      COMPILER_BARRIER();
      make_writable(pPeModule,pLoadAddr,pApp);
      COMPILER_BARRIER();
      bChangedToWritable = true;
      goto restart_rel_iter;
     }
    }
    *(uintptr_t *)&pBlockIter += dwBlockSize;
   }
  } FINALLY {
   if (bChangedToWritable) {
    make_readonly(pPeModule,pLoadAddr,pApp);
    ATOMIC_FETCHOR(pPeModule->pm_Flags,
                   PE_MODULE_FTEXTREL);
   }
  }
 }
}


PRIVATE struct dl_symbol KCALL
Pe_GetSymbol(struct application *__restrict app,
             USER CHECKED char const *__restrict name,
             u32 hash) {
 struct dl_symbol result;
 /* XXX: Lazily fill in a symbol table cache. */
 /* TODO */
 result.ds_type = MODULE_SYMBOL_INVALID;
 return result;
}


PRIVATE struct dl_section KCALL
Pe_GetSection(struct application *__restrict app,
              USER CHECKED char const *__restrict name) {
 struct dl_section result; WORD i;
 PE_MODULE *pPeModule = app->a_module->m_data;
 for (i = 0; i < pPeModule->pm_NumSections; ++i) {
  DWORD dwSectionFlags;
  if (strcmp((char *)pPeModule->pm_Sections[i].Name,name) != 0)
      continue;
  /* Found it! */
  result.ds_base = (void *)(pPeModule->pm_Sections[i].VirtualAddress+
                            app->a_loadaddr);
  result.ds_size    = pPeModule->pm_Sections[i].SizeOfRawData;
  result.ds_offset  = pPeModule->pm_Sections[i].PointerToRawData;
  result.ds_type    = SHT_PROGBITS;
  result.ds_flags   = 0;
  result.ds_entsize = 0;
  dwSectionFlags    = pPeModule->pm_Sections[i].Characteristics;
  if (dwSectionFlags & IMAGE_SCN_LNK_INFO)
   result.ds_type = SHT_NOTE;
  else if (!(dwSectionFlags & IMAGE_SCN_LNK_REMOVE)) {
   result.ds_flags |= SHF_ALLOC;
  }
  if (dwSectionFlags & IMAGE_SCN_MEM_SHARED)
      result.ds_flags |= SHF_SHARED;
  if (dwSectionFlags & IMAGE_SCN_MEM_EXECUTE)
      result.ds_flags |= SHF_EXECINSTR;
  if (dwSectionFlags & IMAGE_SCN_MEM_READ)
      result.ds_flags |= SHF_ALLOC;
  if (dwSectionFlags & IMAGE_SCN_MEM_WRITE)
      result.ds_flags |= SHF_WRITE;
  if ((dwSectionFlags & IMAGE_SCN_CNT_UNINITIALIZED_DATA) &&
       result.ds_size == 0) {
   result.ds_type = SHT_NOBITS;
   result.ds_size = pPeModule->pm_Sections[i].Misc.VirtualSize;
  }
  return result;
 }
 result.ds_size  = 0;
 result.ds_flags = 0;
 return result;
}


DEFINE_MODULE_TYPE(Pe_ModuleType);
INTERN struct module_type Pe_ModuleType = {
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

DECL_END

#endif /* !GUARD_KERNEL_MODULES_PE_LOADER_C */
