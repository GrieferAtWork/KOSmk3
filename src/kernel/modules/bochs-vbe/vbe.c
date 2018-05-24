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
#ifndef GUARD_KERNEL_MODULES_BOCHS_VBE_VBE_C
#define GUARD_KERNEL_MODULES_BOCHS_VBE_VBE_C 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kos/types.h>
#include <dev/pci.h>
#include <fs/device.h>
#include <sys/io.h>
#include <kernel/bind.h>
#include <kernel/debug.h>
#include <kernel/vm.h>
#include <kos/kdev_t.h>
#include <kos/bochs-vbe.h>
#include <sys/stat.h>
#include <except.h>
#include <stdio.h>

#include "vbe.h"

DECL_BEGIN



INTERN vm_ppage_t vbe_lfb = VM_ADDR2PAGE(VBE_DISPI_LFB_PHYSICAL_ADDRESS);
INTERN DEFINE_ATOMIC_RWLOCK(vbe_lock); /* VBE lock. */
INTERN u16 vbe_id          = 0; /* [const] The initial value of `VBE_DISPI_INDEX_ID' */
INTERN u16 vbe_curr_xres   = 0; /* [lock(vbe_lock)] The current value of `VBE_DISPI_INDEX_XRES' */
INTERN u16 vbe_curr_yres   = 0; /* [lock(vbe_lock)] The current value of `VBE_DISPI_INDEX_YRES' */
INTERN u16 vbe_curr_bpp    = 0; /* [lock(vbe_lock)] The current value of `VBE_DISPI_INDEX_BPP' */
INTERN u16 vbe_curr_enable = 0; /* [lock(vbe_lock)] The current value of `VBE_DISPI_INDEX_ENABLE' */


PRIVATE ssize_t KCALL
VBE_Ioctl(struct character_device *__restrict UNUSED(self),
          unsigned long cmd, USER UNCHECKED void *arg,
          iomode_t flags) {
 ssize_t result = 0;
 switch (cmd) {

 case BOCHS_VBE_GETENABLE:
  validate_writable(arg,sizeof(unsigned int));
#if VBE_DISPI_ENABLED == BOCHS_VBE_FENABLED
  *(unsigned int *)arg = ATOMIC_READ(vbe_curr_enable) & VBE_DISPI_ENABLED;
#else
  *(unsigned int *)arg = ATOMIC_READ(vbe_curr_enable) & VBE_DISPI_ENABLED ? BOCHS_VBE_FENABLED : 0;
#endif
  break;

 {
  unsigned int new_mode;
 case BOCHS_VBE_SETENABLE:
  new_mode = (unsigned int)(uintptr_t)arg;
  COMPILER_READ_BARRIER();
  if (new_mode & ~(BOCHS_VBE_FENABLED))
      error_throw(E_INVALID_ARGUMENT);
  atomic_rwlock_write(&vbe_lock);
#if VBE_DISPI_ENABLED == BOCHS_VBE_FENABLED
  vbe_curr_enable = (u16)new_mode;
#else
  vbe_curr_enable = (u16)new_mode & BOCHS_VBE_FENABLED ? VBE_DISPI_ENABLED : 0;
#endif
  if (vbe_curr_enable & BOCHS_VBE_FENABLED)
      vbe_curr_enable |= VBE_DISPI_VBE_ENABLED;
  vbe_write(VBE_DISPI_INDEX_ENABLE,vbe_curr_enable);
  atomic_rwlock_endwrite(&vbe_lock);
 } break;

 {
  struct bochs_vbe_bppsupp supp;
 case BOCHS_VBE_BPPSUPP:
  validate_writable(arg,sizeof(struct bochs_vbe_bppsupp));
  supp = *(struct bochs_vbe_bppsupp *)arg;
  COMPILER_READ_BARRIER();
  validate_writable(supp.bs_supp,supp.bs_bufsiz);
  /* TODO: Based on `vbe_id' */
  if (supp.bs_bufsiz >= 2)
      supp.bs_supp[0] = 8;
  supp.bs_bufsiz = 2;
 } break;

 {
  struct bochs_vbe_format info;
 case BOCHS_VBE_GETFORMAT:
  validate_writable(arg,sizeof(struct bochs_vbe_format));
  atomic_rwlock_read(&vbe_lock);
  if (!(vbe_curr_enable & VBE_DISPI_ENABLED)) {
   atomic_rwlock_endread(&vbe_lock);
   /* Error: VBE hasn't been enabled. */
   error_throw(E_IOERROR);
  }
  info.vf_resx = vbe_curr_xres;
  info.vf_resy = vbe_curr_yres;
  info.vf_bpp  = vbe_curr_bpp;
  atomic_rwlock_endread(&vbe_lock);
  COMPILER_WRITE_BARRIER();
  *(struct bochs_vbe_format *)arg = info;
 } break;

 {
  struct bochs_vbe_format info;
 case BOCHS_VBE_SETFORMAT:
  info = *(struct bochs_vbe_format *)arg;
  COMPILER_READ_BARRIER();
  /* Validate the resolution */
  if (info.vf_resx < 1 || info.vf_resx > 1024 ||
      info.vf_resy < 1 || info.vf_resy > 768)
      error_throw(E_INVALID_ARGUMENT);
  /* TODO: Based on `vbe_id', allow more than only 8 */
  if (info.vf_bpp != 8)
      error_throw(E_INVALID_ARGUMENT);
  atomic_rwlock_write(&vbe_lock);
  if (vbe_curr_enable & VBE_DISPI_ENABLED) {
   /* Must not change resolution while enabled. */
   vbe_write(VBE_DISPI_INDEX_ENABLE,
             VBE_DISPI_DISABLED);
  }
  /* Set new values. */
  vbe_write(VBE_DISPI_INDEX_XRES,info.vf_resx);
  vbe_write(VBE_DISPI_INDEX_YRES,info.vf_resy);
  vbe_write(VBE_DISPI_INDEX_BPP,info.vf_bpp);

  /* Read out what the values that were actually set. */
  vbe_curr_xres = vbe_read(VBE_DISPI_INDEX_XRES);
  vbe_curr_yres = vbe_read(VBE_DISPI_INDEX_YRES);
  vbe_curr_bpp  = vbe_read(VBE_DISPI_INDEX_BPP);
  if (vbe_curr_enable & VBE_DISPI_ENABLED) {
   /* Re-enable the display. */
   vbe_write(VBE_DISPI_INDEX_ENABLE,
             vbe_curr_enable);
  }
  atomic_rwlock_endwrite(&vbe_lock);
 } break;

 default:
  error_throw(E_NOT_IMPLEMENTED);
 }
 return result;
}


PRIVATE void KCALL
VBE_Stat(struct character_device *__restrict UNUSED(self),
         USER CHECKED struct stat64 *result) {
 result->st_size    = VBE_DISPI_TOTAL_VIDEO_MEMORY_MB * 1024 * 1024;
 result->st_blksize = VBE_DISPI_BANK_SIZE_KB * 1024;
 result->st_blocks  = (VBE_DISPI_TOTAL_VIDEO_MEMORY_MB * 1024) / VBE_DISPI_BANK_SIZE_KB;
}


PRIVATE REF struct vm_region *vbe_region = NULL;
DEFINE_DRIVER_FINI(fini_vbe_region);
PRIVATE ATTR_USED void KCALL fini_vbe_region(void) {
 REF struct vm_region *region = vbe_region;
 if (region) vm_region_decref(region);
}

PRIVATE REF struct vm_region *KCALL
VBE_Mmap(struct character_device *__restrict UNUSED(self),
         /*vm_raddr_t*/uintptr_t page_index,
         /*vm_raddr_t*/uintptr_t *__restrict pregion_start) {
 REF struct vm_region *result,*new_result;
 *pregion_start = page_index;
 result = ATOMIC_READ(vbe_region);
 if (!result) {
  /* Construct a memory region for the LFB (Linear Frame Buffer). */
  result = vm_region_alloc((VBE_DISPI_TOTAL_VIDEO_MEMORY_MB * 1024 * 1024) / PAGESIZE);
  result->vr_type           = VM_REGION_PHYSICAL;
  result->vr_part0.vp_state = VM_PART_INCORE;
  result->vr_part0.vp_flags = VM_PART_FKEEP|VM_PART_FWEAKREF|VM_PART_FNOSWAP;
  result->vr_part0.vp_phys.py_num_scatter = 1;
  result->vr_part0.vp_phys.py_iscatter[0].ps_addr = vbe_lfb;
  result->vr_part0.vp_phys.py_iscatter[0].ps_size = (VBE_DISPI_TOTAL_VIDEO_MEMORY_MB * 1024 * 1024) / PAGESIZE;
  new_result = ATOMIC_CMPXCH_VAL(vbe_region,NULL,result);
  if unlikely(new_result != NULL) {
   vm_region_decref(result);
   result = new_result;
  }
 }
 vm_region_incref(result);
 return result;
}


PRIVATE struct character_device_ops VBE_Ops = {
    .c_file = {
        .f_ioctl = &VBE_Ioctl,
        .f_stat  = &VBE_Stat,
        .f_mmap  = &VBE_Mmap,
    }
};


DEFINE_DRIVER_INIT(vbe_init);
PRIVATE ATTR_USED ATTR_FREETEXT void KCALL vbe_init(void) {
 struct character_device *dev;
 /* Search for the PCI device of the VBE adapter. */
 struct pci_device *pci_dev;
 PCI_FOREACH(pci_dev) {
  if ((pci_dev->pd_vendorid == 0x1234 && pci_dev->pd_deviceid == 0x1111) ||
      (pci_dev->pd_vendorid == 0x80ee && pci_dev->pd_deviceid == 0xbeef)) {
   /* Found it! */
   if (!PCI_RESOURCE_ISMEM(pci_dev->pd_res[PD_RESOURCE_BAR0].pr_flags))
        goto not_available;
   vbe_lfb = VM_ADDR2PAGE(pci_dev->pd_res[PD_RESOURCE_BAR0].pr_begin);
   goto got_pci;
  }
 }
 goto not_available;
got_pci:
 /* Check if the VBE feature register contains a valid value. */
 vbe_id = vbe_read(VBE_DISPI_INDEX_ID);
 if (vbe_id < VBE_DISPI_ID0 || vbe_id > VBE_DISPI_ID_FUTURE_MAX)
     goto not_available;
 vbe_write(VBE_DISPI_INDEX_ID,VBE_DISPI_ID4);
 vbe_id = vbe_read(VBE_DISPI_INDEX_ID);
 dev = CHARACTER_DEVICE_ALLOC(struct character_device);
 TRY {
  dev->c_ops            = &VBE_Ops;
  dev->c_device.d_devno = DV_BOCHS_VBE;
  sprintf(dev->c_device.d_namebuf,FREESTR("bochs-vbe"));
  register_device(&dev->c_device);
 } FINALLY {
  character_device_decref(dev);
 }
 return;
not_available:
 error_throwf(E_NO_DEVICE,ERROR_NO_DEVICE_FCHARDEV,(dev_t)DV_BOCHS_VBE);
}


DECL_END

#endif /* !GUARD_KERNEL_MODULES_BOCHS_VBE_VBE_C */
