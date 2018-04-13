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
#ifndef GUARD_KERNEL_SRC_DEV_PCI_C
#define GUARD_KERNEL_SRC_DEV_PCI_C 1

#include <hybrid/compiler.h>
#include <hybrid/align.h>
#include <hybrid/list/list.h>
#include <hybrid/section.h>
#include <kos/types.h>
#include <sys/io.h>

#include <dev/devconfig.h>
#include <dev/pci.h>
#include <fs/device.h>
#include <fs/driver.h>
#include <except.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/heap.h>
#include <kernel/malloc.h>
#include <kernel/memory.h>

#ifdef CONFIG_HAVE_DEV_PCI
DECL_BEGIN


/* TODO: Compatibility with Memory mapped PCI (PCI Express; Much faster than I/O ports) */
/* TODO: Compatibility with Configuration Space Access Mechanism #2 */

#define PCI_ADDR_PORT 0xcf8
#define PCI_DATA_PORT 0xcfc

PUBLIC u32 FCALL pci_readaddr(pci_addr_t addr) {
 assert(IS_ALIGNED(addr,PCI_ADDR_ALIGN));
 outl(PCI_ADDR_PORT,addr);
 return inl(PCI_DATA_PORT);
}
PUBLIC void FCALL pci_writeaddr(pci_addr_t addr, u32 value) {
 assert(IS_ALIGNED(addr,PCI_ADDR_ALIGN));
 outl(PCI_ADDR_PORT,addr);
 outl(PCI_DATA_PORT,value);
}

/* Return a PCI device located at the given
 * `address', or NULL if no such device exists. */
PUBLIC struct pci_device *KCALL
lookup_pci_device(pci_addr_t address) {
 struct pci_device *result;
 PCI_FOREACH(result) {
  if (result->pd_base == address)
      break;
 }
 return result;
}




/* [0..1][const] Linked list of discovered PCI devices. */
PUBLIC SLIST_HEAD(struct pci_device) pci_list = NULL;

PRIVATE void KCALL CheckPCIFun(pci_addr_t addr);
PRIVATE void KCALL CheckPCIDev(pci_bus_t bus, pci_dev_t dev);
PRIVATE void KCALL CheckPCIBus(pci_bus_t bus);


PRIVATE void KCALL
AllocatePCIDeviceRessource(struct pci_device *__restrict dev,
                           struct pci_resource *__restrict res) {
#if 0
#if 0
 /* Allocate PCI resources (memory and I/O ranges). */
 if (res->pr_flags&PCI_RESOURCE_IO) {
  ioaddr_t addr;
  /* Try to allocate I/O space at the pre-configured address. */
  if (res->pr_begin+res->pr_size   >= res->pr_begin &&
      res->pr_begin+res->pr_size-1 <= (ioport_t)-1 &&
      E_ISOK(io_malloc_at((ioport_t)res->pr_begin,(iosize_t)res->pr_size,THIS_INSTANCE)))
      return false; /* No need to update */
  /* If that failed, allocate a new range. */
  addr = io_malloc((iosize_t)res->pr_align,
                   (iosize_t)res->pr_size,
                   (ioport_t)-1,THIS_INSTANCE);
  if (E_ISERR(addr)) {
   syslog(LOG_DEBUG,FREESTR("[PCI] Failed to allocate %Iu bytes of I/O address space for BAR #%d of PCI device at %p\n"),
          res->pr_size,(int)(res-dev->pd_resources),dev->pd_addr);
   /* Delete the BAR to hide our failure. */
   res->pr_flags = 0;
   return false;
  } else {
   res->pr_begin = (PHYS uintptr_t)addr;
  }
 } else
#endif
 if (res->pr_flags&PCI_RESOURCE_FMEM) {
  /* Allocate physical memory for the device. */
  vm_ppage_t addr;
  /* TODO: page_align() -- res->pr_align */
  addr = page_malloc(res->pr_size,
                    (res->pr_flags&PCI_RESOURCE_FMEM16) ? X86_MZONE_1MB : MZONE_ANY);
  /* Assign memory. */
  res->pr_begin = (PHYS uintptr_t)addr;
 } else
#endif
 {
  error_throw(E_BADALLOC);
 }
}

PRIVATE ATTR_FREETEXT
struct pci_device *KCALL RegisterPCIDevice(pci_addr_t addr) {
 struct pci_device *result; u32 maxsize,state;
 struct pci_resource *iter; unsigned int i;
 result = NULL;
 TRY {
  result = (struct pci_device *)kmalloc(sizeof(struct pci_device),
                                        GFP_SHARED|GFP_CALLOC);
  result->pd_base = addr;
  /* Read fields. */
  result->pd_dev0 = pci_read(addr,PCI_DEV0);
  result->pd_dev8 = pci_read(addr,PCI_DEV8);
  result->pd_devc = pci_read(addr,PCI_DEVC);

  iter = result->pd_res;
  switch (result->pd_header) {

  case PCI_DEVC_HEADER_GENERIC:
   for (i = 0; i < 6; ++i,++iter) {
    pci_reg_t reg = PCI_GDEV_BAR0+i*(PCI_GDEV_BAR1-PCI_GDEV_BAR0);
    state = pci_read(addr,reg);
    if (!(state&~0xf)) continue; /* Unused. */
    pci_write(addr,reg,(u32)-1);
    maxsize = pci_read(addr,reg);
    //syslog(LOG_DEBUG,FREESTR("state = %p/%p\n"),state,maxsize);
    if (state&1) {
     /* I/O range. */
     iter->pr_begin  = state&~0x3;
     iter->pr_size   = (~(maxsize&~0x3))+1;
     iter->pr_flags |= PCI_RESOURCE_FIO;
    } else {
     /* Memory range. */
     iter->pr_begin  = state&~0xf;
     iter->pr_size   = (~(maxsize&~0xf))+1;
     iter->pr_flags |= PCI_RESOURCE_FMEM;
     iter->pr_flags |= (state & 0x6); /* Memory type. */
     /* A 64-bit memory range takes up 2 BARs. */
     if (iter->pr_flags&PCI_RESOURCE_FMEM64 && !(i&1)) ++i,++iter;
    }
    if (iter->pr_size)
        iter->pr_align = 1 << (__builtin_ffsl(iter->pr_size)-1);
    else iter->pr_flags = 0; /* Ignore this case */
    /* Allocate PCI resources */
    if (reg <= PCI_GDEV_BAR5) {
     AllocatePCIDeviceRessource(result,iter);
     state = (u32)((state&1)|iter->pr_begin);
 #if __SIZEOF_POINTER__ > 4
     if ((iter->pr_flags&PCI_RESOURCE_FMEM64) && !(i&1)) {
      pci_write(addr,reg+(PCI_GDEV_BAR1-PCI_GDEV_BAR0),
               (u32)((state&1)|(iter->pr_begin >> 32)));
     }
 #endif
    }
    pci_write(addr,reg,state);
   }
   break;

  case PCI_DEVC_HEADER_BRIDGE:
   for (i = 0; i < 2; ++i,++iter) {
    pci_reg_t reg = PCI_BDEV_BAR0+i*(PCI_BDEV_BAR1-PCI_BDEV_BAR0);
    state = pci_read(addr,reg);
    if (!(state&~0xf)) continue; /* Unused. */
    pci_write(addr,reg,(u32)-1);
    maxsize = pci_read(addr,reg);
    if (state&1) {
     /* I/O range. */
     iter->pr_begin  = state&~0x3;
     iter->pr_size   = (~(maxsize&0x3))+1;
     iter->pr_flags |= PCI_RESOURCE_FIO;
    } else {
     /* Memory range. */
     iter->pr_begin  = state&~0xf;
     iter->pr_size   = (~(maxsize&0xf))+1;
     iter->pr_flags |= PCI_RESOURCE_FMEM;
     iter->pr_flags |= (state&0x6); /* Memory type. */
     /* A 64-bit memory range takes up 2 BARs. */
     if (iter->pr_flags&PCI_RESOURCE_FMEM64 && !(i&1)) ++i,++iter;
    }
    if (iter->pr_size)
        iter->pr_align = 1 << (__builtin_ffsl(iter->pr_size)-1);
    /* Allocate PCI resources */
    AllocatePCIDeviceRessource(result,iter);
    state = (u32)((state&1)|iter->pr_begin);
 #if __SIZEOF_POINTER__ > 4
    if (iter->pr_flags&PCI_RESOURCE_MEM64 && !(i&1) &&
        reg <= PCI_GDEV_BAR5) {
     pci_write(addr,reg+(PCI_BDEV_BAR1-PCI_BDEV_BAR0),
              (u32)((state&1)|(iter->pr_begin >> 32)));
    }
 #endif
    pci_write(addr,reg,state);
   }
   break;

  case PCI_DEVC_HEADER_CARDBUS:
   /* XXX: `PCI_CDEV_MEMBASE0' and friends? */
   break;

  default: break;
  }

  debug_printf(FREESTR("[PCI] Device at %I32p - %.4I16x:%.4I16x "
                       "(class 0x%x, subclass 0x%x, progif 0x%x, rev 0x%x)\n"),
               addr,
               result->pd_vendorid,
               result->pd_deviceid,
               result->pd_classid,
               result->pd_subclassid,
               result->pd_progifid,
               result->pd_revid);
  SLIST_INSERT(pci_list,result,pd_chain);
  return result;
 } CATCH (E_BADALLOC) {
  if (result) {
   kfree(result);
   result = NULL;
  }
 }
 return NULL;
}


PRIVATE ATTR_FREETEXT void KCALL CheckPCIFun(pci_addr_t addr) {
 struct pci_device *dev;
 if (lookup_pci_device(addr))
     return; /* Already registered. */
 dev = RegisterPCIDevice(addr);
 if (!dev) return;
 if (dev->pd_classid == PCI_DEV8_CLASS_BRIDGE/* &&
     dev->pd_subclassid == PCI_DEV8_CLASS_MUMEDIA*/) {
  /* Recursively enumerate a bridge device. */
  CheckPCIBus(PCI_BDEV18_SECONDARY_BUS(pci_read(addr,PCI_BDEV18)));
 }
}
PRIVATE ATTR_FREETEXT void KCALL CheckPCIDev(pci_bus_t bus, pci_dev_t dev) {
 pci_addr_t addr = PCI_ADDR(bus,dev,0);
 if (PCI_DEV0_VENDOR(pci_read(addr,PCI_DEV0)) == PCI_DEV0_VENDOR_NODEV)
     return;
 CheckPCIFun(addr);
 if (PCI_DEVC_HEADER(pci_read(addr,PCI_DEVC)) &
     PCI_DEVC_HEADER_MULTIDEV) {
  u8 i; /* Recursively check secondary functions of a multi-device. */
  for (i = 0; i < PCI_ADDR_FUNCOUNT; ++i,addr += 1 << PCI_ADDR_FUNSHIFT) {
   if (PCI_DEV0_VENDOR(pci_read(addr,PCI_DEV0)) == PCI_DEV0_VENDOR_NODEV)
       continue;
   CheckPCIFun(addr);
  }
 }
}
PRIVATE ATTR_FREETEXT void KCALL CheckPCIBus(pci_bus_t bus) {
 u8 device;
 for (device = 0; device < 32; device++)
      CheckPCIDev(bus,device);
}
DEFINE_DRIVER_PREINIT(pci_discover);
PRIVATE ATTR_USED ATTR_FREETEXT void KCALL pci_discover(void) {
 /* TODO: Check if PCI is even available? */
 /* TODO: Check which Configuration Space Access Mechanism can/must be used. */

 /* Perform a simple PCI device discovery. */
#if 1
 /* Recursive scanner. */
 u32 resp = pci_read(PCI_ADDR(0,0,0),PCI_DEVC);
 if (PCI_DEVC_HEADER(resp)&PCI_DEVC_HEADER_MULTIDEV) {
  /* Single controller. */
  CheckPCIBus(0);
 } else {
  /* Multiple controllers. */
  pci_fun_t fun = 0;
  for (; fun < PCI_ADDR_FUNCOUNT; ++fun) {
   pci_addr_t addr = PCI_ADDR(0,0,fun);
   if (PCI_DEV0_VENDOR(pci_read(addr,PCI_DEV0)) == PCI_DEV0_VENDOR_NODEV)
       continue;
   CheckPCIBus(addr);
  }
 }
#else
 /* Brute-force scanner (TODO: Add a commandline option for this). */
 u8 bus = 0;
 for (;;) {
  check_bus(bus);
  if (bus == 0xff) break;
  ++bus;
 }
#endif
}




DECL_END
#endif /* CONFIG_HAVE_DEV_PCI */

#endif /* !GUARD_KERNEL_SRC_DEV_PCI_C */
