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
#ifndef GUARD_KERNEL_SRC_CORE_ATA_C
#define GUARD_KERNEL_SRC_CORE_ATA_C 1
#define _KOS_SOURCE 1

/* Required because we can't be interrupted
 * once a disk operation is started. */
#define _NOSERVE_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/section.h>
#include <hybrid/host.h>
#include <hybrid/align.h>
#include <kernel/user.h>
#include <kernel/heap.h>
#include <kernel/malloc.h>
#include <kernel/debug.h>
#include <i386-kos/pic.h>
#include <fs/driver.h>
#include <fs/device.h>
#include <sched/mutex.h>
#include <sched/async_signal.h>
#include <sys/mount.h>
#include <string.h>
#include <fcntl.h>
#include <except.h>
#include <stdio.h>
#include <assert.h>
#include <sys/io.h>

#include "ata.h"

DECL_BEGIN

#define Ata_ResetBusInterruptCounter() \
 (Ata_BusInterruptCounter[0] = Ata_BusInterruptCounter[1] = 0)
INTERN volatile unsigned int Ata_BusInterruptCounter[2] = { 0, 0 };
INTERN struct async_sig Ata_BusInterruptSignal[2] = { ASYNC_SIG_INIT, ASYNC_SIG_INIT };
INTERN DEFINE_MUTEX(Ata_SubSystemLock);

/* The max amount of time to wait when expecting an interrupt. */
INTERN WEAK jtime_t Ata_InterruptTimeout = JIFFIES_FROM_SECONDS(2);
INTERN WEAK jtime_t Ata_BusyTimeout = JIFFIES_FROM_SECONDS(2);
INTERN WEAK jtime_t Ata_DrqTimeout = JIFFIES_FROM_SECONDS(2);

/* The max number of times to reset the ATA before giving up.
 * HINT: The number of attempts made as a result of this is `Ata_MaxResetCount+1' */
INTERN WEAK unsigned int Ata_MaxResetCount = 2;



/* ====================================================================== */
/*   ATA Bus wait control functions.                                      */
/* ====================================================================== */
PRIVATE bool KCALL
Ata_WaitForBusInterrupt(u16 bus, jtime_t timeout) {
 jtime_t abs_timeout;
 volatile unsigned int *counter;
 assert(bus == ATA_BUS_PRIMARY ||
        bus == ATA_BUS_SECONDARY);
 counter = &Ata_BusInterruptCounter[!(bus&ATA_BUS_FPRIMARY)];
 abs_timeout = jiffies + timeout;
 while (!ATOMIC_READ(*counter)) {
  struct async_task_connection con;
  task_connect_async(&con,&Ata_BusInterruptSignal[!(bus&ATA_BUS_FPRIMARY)]);
  /* Check the counter once again. (The counter may have been incremented while we were connecting) */
  if (ATOMIC_READ(*counter)) { task_disconnect_async(); break; }
  if (!task_waitfor_async_noserve(abs_timeout)) {
   debug_printf("*counter = %u\n",ATOMIC_READ(*counter));
   return false;
  }
 }
 ATOMIC_FETCHDEC(*counter);
 return true;
}
PRIVATE void KCALL Ata_WaitForBusy(u16 bus) {
 u8 status; jtime_t abs_timeout;
 if (!(inb(ATA_DCR(bus)) & ATA_DCR_BSY))
       return;
 abs_timeout = jiffies + Ata_BusyTimeout;
 while ((status = inb(ATA_DCR(bus))) & ATA_DCR_BSY) {
  if unlikely((jiffies >= abs_timeout) ||
              (status & ATA_DCR_ERR))
     error_throw(E_IOERROR);
  task_yield();
 }
}
PRIVATE void KCALL Ata_ResetBus(u16 bus) {
 outb(ATA_DCR(bus),ATA_CTRL_SRST);
 ATA_SELECT_DELAY(bus);
 outb(ATA_DCR(bus),0);
 ATA_SELECT_DELAY(bus);
}

PRIVATE void KCALL Ata_WaitForDrq(u16 bus) {
 u8 status; jtime_t abs_timeout;
 if (((status = inb(ATA_DCR(bus))) & ATA_DCR_BSY) == 0) {
  abs_timeout = jiffies + Ata_BusyTimeout;
  while ((status = inb(ATA_DCR(bus))) & ATA_DCR_BSY) {
   if unlikely((jiffies >= abs_timeout) ||
               (status & ATA_DCR_ERR))
      error_throw(E_IOERROR);
   task_yield();
  }
 }
 if (status & ATA_DCR_ERR)
     error_throw(E_IOERROR);
 if (status & ATA_DCR_DRQ)
     return;
 abs_timeout = jiffies + Ata_DrqTimeout;
 for (;;) {
  task_yield();
  status = inb(ATA_DCR(bus));
  if unlikely((jiffies >= abs_timeout) ||
              (status & ATA_DCR_ERR))
     error_throw(E_IOERROR);
  if (status & ATA_DCR_DRQ) break;
 }
}


/* Reset the specified ATA device.
 * @return: true:  The reset succeeded.
 * @return: false: The reset failed. */
PRIVATE bool KCALL Ata_Reset(AtaDevice *__restrict self) {
 Ata_ResetBus(self->a_bus);
 return true;
}


/* ====================================================================== */
/*   ATA low-level I/O implementation.                                    */
/* ====================================================================== */
LOCAL void KCALL
Ata_ReceiveDataSectors(u16 bus, void *buffer, u16 num_blocks) {
 do {
  if (!Ata_WaitForBusInterrupt(bus,Ata_InterruptTimeout))
       error_throw(E_IOERROR);
  Ata_WaitForDrq(bus);
  insw(ATA_DATA(bus),buffer,ATA_SECTOR_SIZE/2);
  *(uintptr_t *)&buffer += ATA_SECTOR_SIZE;
 } while (--num_blocks != 0);
}
LOCAL void KCALL
Ata_TransmitDataSectors(u16 bus, void const *buffer, u16 num_blocks) {
 do {
  unsigned int i;
  //if (!Ata_WaitForBusInterrupt(bus,Ata_InterruptTimeout))
  //     error_throw(E_IOERROR);
  Ata_WaitForDrq(bus);
  for (i = 0; i < ATA_SECTOR_SIZE; ++i)
     outw(ATA_DATA(bus),((u16 *)buffer)[i]);
  *(uintptr_t *)&buffer += ATA_SECTOR_SIZE;
 } while (--num_blocks != 0);
}
LOCAL void KCALL
Ata_FlushBuffers(u16 bus, u8 command) {
 outb(ATA_COMMAND(bus),command);
 if (!Ata_WaitForBusInterrupt(bus,Ata_InterruptTimeout))
      error_throw(E_IOERROR);
}
LOCAL void KCALL
Ata_ReadDataUsing48BitLBA(u16 bus, u8 drive, u64 lba, void *buffer, u16 num_blocks) {
 task_serve();
 mutex_get(&Ata_SubSystemLock);
 TRY {
  Ata_WaitForBusy(bus);
  Ata_ResetBusInterruptCounter();
  outb(ATA_DRIVE_SELECT(bus),0x40|(drive & ATA_DRIVE_FSLAVE));
  ATA_SELECT_DELAY(bus);
  //outb(ATA_FEATURES(bus),0x0);       /* PIO mode */
  outb(ATA_SECTOR_COUNT(bus),(u8)(num_blocks >> 8));
  outb(ATA_ADDRESS1(bus),(u8)(lba >> 24));
  outb(ATA_ADDRESS2(bus),(u8)(lba >> 32));
  outb(ATA_ADDRESS3(bus),(u8)(lba >> 40));
  outb(ATA_SECTOR_COUNT(bus),(u8)num_blocks);
  outb(ATA_ADDRESS1(bus),(u8)lba);
  outb(ATA_ADDRESS2(bus),(u8)(lba >> 8));
  outb(ATA_ADDRESS3(bus),(u8)(lba >> 16));
  outb(ATA_COMMAND(bus),ATA_COMMAND_READ_PIO_EXT);
  Ata_ReceiveDataSectors(bus,buffer,num_blocks);
 } FINALLY {
  mutex_put(&Ata_SubSystemLock);
 }
}
LOCAL void KCALL
Ata_WriteDataUsing48BitLBA(u16 bus, u8 drive, u64 lba, void const *buffer, u16 num_blocks) {
 task_serve();
 mutex_get(&Ata_SubSystemLock);
 TRY {
  Ata_WaitForBusy(bus);
  Ata_ResetBusInterruptCounter();
  outb(ATA_DRIVE_SELECT(bus),0x40|(drive & ATA_DRIVE_FSLAVE));
  ATA_SELECT_DELAY(bus);
  //outb(ATA_FEATURES(bus),0x0);       /* PIO mode */
  outb(ATA_SECTOR_COUNT(bus),(u8)(num_blocks >> 8));
  outb(ATA_ADDRESS1(bus),(u8)(lba >> 24));
  outb(ATA_ADDRESS2(bus),(u8)(lba >> 32));
  outb(ATA_ADDRESS3(bus),(u8)(lba >> 40));
  outb(ATA_SECTOR_COUNT(bus),(u8)num_blocks);
  outb(ATA_ADDRESS1(bus),(u8)lba);
  outb(ATA_ADDRESS2(bus),(u8)(lba >> 8));
  outb(ATA_ADDRESS3(bus),(u8)(lba >> 16));
  outb(ATA_COMMAND(bus),ATA_COMMAND_WRITE_PIO_EXT);
  Ata_TransmitDataSectors(bus,buffer,num_blocks);
  Ata_FlushBuffers(bus,ATA_COMMAND_CACHE_FLUSH_EXT);
 } FINALLY {
  mutex_put(&Ata_SubSystemLock);
 }
}
LOCAL void KCALL
Ata_ReadDataUsing28BitLBA(u16 bus, u8 drive, u32 lba, void *buffer, u8 num_blocks) {
 task_serve();
 mutex_get(&Ata_SubSystemLock);
 TRY {
  Ata_WaitForBusy(bus);
  Ata_ResetBusInterruptCounter();
  outb(ATA_DRIVE_SELECT(bus),
      (drive+(0xe0-ATA_DRIVE_MASTER))|
      ((lba >> 24) & 0xf));
  ATA_SELECT_DELAY(bus);
  outb(ATA_FEATURES(bus),0x0);       /* PIO mode */
  outb(ATA_SECTOR_COUNT(bus),num_blocks);
  outb(ATA_ADDRESS1(bus),(u8)lba);
  outb(ATA_ADDRESS2(bus),(u8)(lba >> 8));
  outb(ATA_ADDRESS3(bus),(u8)(lba >> 16));
  outb(ATA_COMMAND(bus),ATA_COMMAND_READ_PIO);
  Ata_ReceiveDataSectors(bus,buffer,num_blocks);
 } FINALLY {
  mutex_put(&Ata_SubSystemLock);
 }
}
LOCAL void KCALL
Ata_WriteDataUsing28BitLBA(u16 bus, u8 drive, u32 lba, void const *buffer, u8 num_blocks) {
 task_serve();
 mutex_get(&Ata_SubSystemLock);
 TRY {
  Ata_WaitForBusy(bus);
  Ata_ResetBusInterruptCounter();
  outb(ATA_DRIVE_SELECT(bus),
      (drive+(0xe0-ATA_DRIVE_MASTER))|
      ((lba >> 24) & 0xf));
  ATA_SELECT_DELAY(bus);
  //outb(ATA_FEATURES(bus),0x0);       /* PIO mode */
  outb(ATA_SECTOR_COUNT(bus),num_blocks);
  outb(ATA_ADDRESS1(bus),(u8)lba);
  outb(ATA_ADDRESS2(bus),(u8)(lba >> 8));
  outb(ATA_ADDRESS3(bus),(u8)(lba >> 16));
  outb(ATA_COMMAND(bus),ATA_COMMAND_READ_PIO);
  Ata_TransmitDataSectors(bus,buffer,num_blocks);
  Ata_FlushBuffers(bus,ATA_COMMAND_CACHE_FLUSH);
 } FINALLY {
  mutex_put(&Ata_SubSystemLock);
 }
}
LOCAL void KCALL
Ata_ReadDataUsingCHSAddressing(u16 bus, u8 drive, u8 sector, u8 head,
                               u16 cylinder, void *buffer, u8 num_blocks) {
 task_serve();
 mutex_get(&Ata_SubSystemLock);
 TRY {
  Ata_WaitForBusy(bus);
  Ata_ResetBusInterruptCounter();
  outb(ATA_DRIVE_SELECT(bus),
      (drive+(0xe0-ATA_DRIVE_MASTER))|
      (head & 0xf));
  ATA_SELECT_DELAY(bus);
  //outb(ATA_FEATURES(bus),0x0);       /* PIO mode */
  outb(ATA_SECTOR_COUNT(bus),num_blocks);
  outb(ATA_ADDRESS1(bus),(u8)sector);
  outb(ATA_ADDRESS2(bus),(u8)cylinder);
  outb(ATA_ADDRESS3(bus),(u8)(cylinder >> 8));
  outb(ATA_COMMAND(bus),ATA_COMMAND_READ_SECTORS);
  Ata_ReceiveDataSectors(bus,buffer,num_blocks);
 } FINALLY {
  mutex_put(&Ata_SubSystemLock);
 }
}
LOCAL void KCALL
Ata_WriteDataUsingCHSAddressing(u16 bus, u8 drive, u8 sector, u8 head,
                                u16 cylinder, void const *buffer, u8 num_blocks) {
 task_serve();
 mutex_get(&Ata_SubSystemLock);
 TRY {
  Ata_WaitForBusy(bus);
  Ata_ResetBusInterruptCounter();
  outb(ATA_DRIVE_SELECT(bus),
      (drive+(0xe0-ATA_DRIVE_MASTER))|
      (head & 0xf));
  ATA_SELECT_DELAY(bus);
  //outb(ATA_FEATURES(bus),0x0);       /* PIO mode */
  outb(ATA_SECTOR_COUNT(bus),num_blocks);
  outb(ATA_ADDRESS1(bus),(u8)sector);
  outb(ATA_ADDRESS2(bus),(u8)cylinder);
  outb(ATA_ADDRESS3(bus),(u8)(cylinder >> 8));
  outb(ATA_COMMAND(bus),ATA_COMMAND_READ_SECTORS);
  Ata_TransmitDataSectors(bus,buffer,num_blocks);
  Ata_FlushBuffers(bus,ATA_COMMAND_CACHE_FLUSH);
 } FINALLY {
  mutex_put(&Ata_SubSystemLock);
 }
}


PRIVATE void KCALL
Ata_ReadLBA28(AtaDevice *__restrict self,
              CHECKED USER void *buf, size_t num_blocks,
              blkaddr_t first_block) {
 while (num_blocks) {
  unsigned int EXCEPT_VAR reset_count = 0;
  u8 part = num_blocks > 0xff ? 0xff : (u8)num_blocks;
  TRY {
retry_io:
   Ata_ReadDataUsing28BitLBA(self->a_bus,
                             self->a_drive,
                            (u32)first_block,
                             buf,part);
  } CATCH (E_IOERROR) {
   if (reset_count++ < Ata_MaxResetCount &&
       Ata_Reset(self)) {
    error_handled();
    goto retry_io;
   }
   error_rethrow();
  }
  num_blocks -= part;
 }
}
PRIVATE void KCALL
Ata_ReadLBA48(AtaDevice *__restrict self,
              CHECKED USER void *buf, size_t num_blocks,
              blkaddr_t first_block) {
 while (num_blocks) {
  unsigned int EXCEPT_VAR reset_count = 0;
  u16 part = num_blocks > 0xffff ? 0xffff : (u16)num_blocks;
  TRY {
retry_io:
   Ata_ReadDataUsing48BitLBA(self->a_bus,
                             self->a_drive,
                            (u32)first_block,
                             buf,part);
  } CATCH (E_IOERROR) {
   if (reset_count++ < Ata_MaxResetCount &&
       Ata_Reset(self)) {
    error_handled();
    goto retry_io;
   }
   error_rethrow();
  }
  num_blocks -= part;
 }
}
PRIVATE void KCALL
Ata_ReadCHS(AtaDevice *__restrict self,
            CHECKED USER void *buf, size_t num_blocks,
            blkaddr_t first_block) {
 while (num_blocks) {
  unsigned int EXCEPT_VAR reset_count = 0;
  u32 temp      =  first_block / (u8)self->a_chs_info.a_sectors_per_track;
  u8  sector    = (first_block % (u8)self->a_chs_info.a_sectors_per_track)+1;
  u8  head      =  temp % (u8)self->a_chs_info.a_number_of_heads;
  u16 cylinder  =  temp / (u8)self->a_chs_info.a_number_of_heads;
  u8  max_count = self->a_chs_info.a_sectors_per_track - sector;
  if (max_count > num_blocks)
      max_count = (u8)num_blocks;
  assert(max_count);
  TRY {
retry_io:
   Ata_ReadDataUsingCHSAddressing(self->a_bus,self->a_drive,
                                  sector,head,cylinder,buf,
                                  max_count);
  } CATCH (E_IOERROR) {
   if (reset_count++ < Ata_MaxResetCount &&
       Ata_Reset(self)) {
    error_handled();
    goto retry_io;
   }
   error_rethrow();
  }
  num_blocks -= max_count;
 }
}

PRIVATE void KCALL
Ata_WriteLBA28(AtaDevice *__restrict self,
               CHECKED USER void *buf, size_t num_blocks,
               blkaddr_t first_block) {
 while (num_blocks) {
  unsigned int EXCEPT_VAR reset_count = 0;
  u8 part = num_blocks > 0xff ? 0xff : (u8)num_blocks;
  TRY {
retry_io:
   Ata_WriteDataUsing28BitLBA(self->a_bus,
                              self->a_drive,
                             (u32)first_block,
                              buf,part);
  } CATCH (E_IOERROR) {
   if (reset_count++ < Ata_MaxResetCount &&
       Ata_Reset(self)) {
    error_handled();
    goto retry_io;
   }
   error_rethrow();
  }
  num_blocks -= part;
 }
}
PRIVATE void KCALL
Ata_WriteLBA48(AtaDevice *__restrict self,
               CHECKED USER void *buf, size_t num_blocks,
               blkaddr_t first_block) {
 while (num_blocks) {
  unsigned int EXCEPT_VAR reset_count = 0;
  u16 part = num_blocks > 0xffff ? 0xffff : (u16)num_blocks;
  TRY {
retry_io:
   Ata_WriteDataUsing48BitLBA(self->a_bus,
                              self->a_drive,
                             (u32)first_block,
                              buf,part);
  } CATCH (E_IOERROR) {
   if (reset_count++ < Ata_MaxResetCount &&
       Ata_Reset(self)) {
    error_handled();
    goto retry_io;
   }
   error_rethrow();
  }
  num_blocks -= part;
 }
}
PRIVATE void KCALL
Ata_WriteCHS(AtaDevice *__restrict self,
             CHECKED USER void *buf, size_t num_blocks,
             blkaddr_t first_block) {
 while (num_blocks) {
  unsigned int EXCEPT_VAR reset_count = 0;
  u32 temp      =  first_block / (u8)self->a_chs_info.a_sectors_per_track;
  u8  sector    = (first_block % (u8)self->a_chs_info.a_sectors_per_track)+1;
  u8  head      =  temp % (u8)self->a_chs_info.a_number_of_heads;
  u16 cylinder  =  temp / (u8)self->a_chs_info.a_number_of_heads;
  u8  max_count = self->a_chs_info.a_sectors_per_track - sector;
  if (max_count > num_blocks)
      max_count = (u8)num_blocks;
  assert(max_count);
  TRY {
retry_io:
   Ata_WriteDataUsingCHSAddressing(self->a_bus,self->a_drive,
                                   sector,head,cylinder,buf,
                                   max_count);
  } CATCH (E_IOERROR) {
   if (reset_count++ < Ata_MaxResetCount &&
       Ata_Reset(self)) {
    error_handled();
    goto retry_io;
   }
   error_rethrow();
  }
  num_blocks -= max_count;
 }
}

PRIVATE ssize_t KCALL
Ata_Ioctl(AtaDevice *__restrict self,
          unsigned long cmd, USER UNCHECKED void *arg,
          iomode_t flags) {
 switch (cmd) {

 case BLKSECTGET:
  validate_writable(arg,sizeof(unsigned short));
  if (self->a_device.b_io.io_read ==
     (void(KCALL *)(struct block_device *__restrict,
                    CHECKED USER void *,size_t,blkaddr_t))
                   &Ata_ReadLBA48) {
   *(unsigned short *)arg = 0xffff;
  } else if (self->a_device.b_io.io_read ==
            (void(KCALL *)(struct block_device *__restrict,
                           CHECKED USER void *,size_t,blkaddr_t))
                          &Ata_ReadLBA28) {
   *(unsigned short *)arg = 0xff;
  } else if (self->a_device.b_io.io_read ==
            (void(KCALL *)(struct block_device *__restrict,
                           CHECKED USER void *,size_t,blkaddr_t))
                          &Ata_ReadCHS) {
   *(unsigned short *)arg = self->a_chs_info.a_sectors_per_track;
  } else {
   /* Shouldn't get here... */
   *(unsigned short *)arg = 1;
  }
  break;

 default:
  error_throw(E_NOT_IMPLEMENTED);
 }
 return 0;
}



STATIC_ASSERT(sizeof(AtaDeviceSpecifications) == 512);

PRIVATE ATTR_FREETEXT void KCALL
Ata_RegisterAtaDevice(u16 bus, u8 drive) {
 AtaDeviceSpecifications device_specs;
 AtaDevice *EXCEPT_VAR self;
 debug_printf("[ATA] Found ATA device on %.4I16x:%.2I8x\n",bus,drive);
 self = BLOCK_DEVICE_ALLOC(AtaDevice,63);
 TRY {

#if 0 /* The caller already did this. */
  outb(ATA_DRIVE_SELECT(bus),drive);
  ATA_SELECT_DELAY(bus);
#endif

  outb(ATA_FEATURES(bus),1); /* ??? */
  outb(ATA_COMMAND(bus),ATA_COMMAND_IDENTIFY);
  ATA_SELECT_DELAY(bus);
  if (!Ata_WaitForBusInterrupt(bus,20))
       error_throw(E_IOERROR);
  Ata_WaitForBusy(bus);
  insw(ATA_DATA(bus),&device_specs,256);
#if 1
  if (device_specs.CommandSetActive.BigLba) {
#if 1 /* Prefer using 28-bit LBA if possible. (It's faster) */
   if (device_specs.Capabilities.LbaSupported &&
      (u64)device_specs.UserAddressableSectors48 ==
      (u64)device_specs.UserAddressableSectors)
       goto use_lba28;
#endif
   self->a_device.b_blockcount  = (blkcnt_t)device_specs.UserAddressableSectors48;
   self->a_device.b_io.io_read  = (void(KCALL *)(struct block_device *__restrict,CHECKED USER void *,size_t,blkaddr_t))&Ata_ReadLBA48;
   self->a_device.b_io.io_write = (void(KCALL *)(struct block_device *__restrict,CHECKED USER void const *,size_t,blkaddr_t))&Ata_WriteLBA48;
  } else if (device_specs.Capabilities.LbaSupported) {
use_lba28:
   self->a_device.b_blockcount  = (blkcnt_t)device_specs.UserAddressableSectors;
   self->a_device.b_io.io_read  = (void(KCALL *)(struct block_device *__restrict,CHECKED USER void *,size_t,blkaddr_t))&Ata_ReadLBA28;
   self->a_device.b_io.io_write = (void(KCALL *)(struct block_device *__restrict,CHECKED USER void const *,size_t,blkaddr_t))&Ata_WriteLBA28;
  } else
#endif
  {
   /* Use CHS addressing. */
   self->a_chs_info.a_number_of_heads   = (u8)device_specs.NumHeads;
   self->a_chs_info.a_cylinders         = (u16)device_specs.NumCylinders;
   self->a_chs_info.a_sectors_per_track = (u8)device_specs.NumSectorsPerTrack;
   self->a_device.b_blockcount = (blkcnt_t)self->a_chs_info.a_number_of_heads *
                                 (blkcnt_t)self->a_chs_info.a_cylinders *
                                 (blkcnt_t)self->a_chs_info.a_sectors_per_track;
   self->a_device.b_io.io_read  = (void(KCALL *)(struct block_device *__restrict,CHECKED USER void *,size_t,blkaddr_t))&Ata_ReadCHS;
   self->a_device.b_io.io_write = (void(KCALL *)(struct block_device *__restrict,CHECKED USER void const *,size_t,blkaddr_t))&Ata_WriteCHS;
  }
  if (!self->a_device.b_blockcount)
       error_throw(E_IOERROR);
  self->a_device.b_io.io_ioctl = (ssize_t(KCALL *)(struct block_device *__restrict,unsigned long,USER UNCHECKED void *,iomode_t))&Ata_Ioctl;

  /* Finalize the new block-device and register it. */
  self->a_bus                     = bus;
  self->a_drive                   = drive;
  self->a_device.b_blocksize      = 512;
  self->a_device.b_device.d_devno = MKDEV(bus   == ATA_BUS_PRIMARY  ? 3 : 22,
                                          drive == ATA_DRIVE_MASTER ? 0 : 64);
  sprintf(self->a_device.b_device.d_namebuf,"hd%c",
        ((bus == ATA_BUS_PRIMARY) ? 'a' : 'c')+
         (drive == ATA_DRIVE_MASTER ? 0 : 1));

  /* Register the device. */
  register_device(&self->a_device.b_device);
  /* Automatically construct partitions. */
  block_device_autopart(&self->a_device);

 } FINALLY {
  device_decref(&self->a_device.b_device);
 }
}



#if 0
PRIVATE void KCALL
AtaPI_ReadDataFromSector(u16 bus, u8 drive, u32 lba, void *buffer) {
 u8 status; size_t size;
 u8 read_cmd[12] = { 0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
 task_serve();
 mutex_get(&Ata_SubSystemLock);
 TRY {
  Ata_ResetBusInterruptCounter();
  outb(ATA_DRIVE_SELECT(bus),drive & (1 << 4)); /* XXX: What's this about the slave bit? */
  ATA_SELECT_DELAY(bus);             /* 400ns delay */
  outb(ATA_FEATURES(bus),0x0);       /* PIO mode */
  outb(ATA_ADDRESS2(bus),ATAPI_SECTOR_SIZE & 0xFF);
  outb(ATA_ADDRESS3(bus),ATAPI_SECTOR_SIZE >> 8);
  outb(ATA_COMMAND(bus),ATA_COMMAND_PACKET); /* ATA PACKET command */
  while ((status = inb(ATA_COMMAND(bus))) & ATA_DCR_BSY)
      task_yield();
  while (!((status = inb(ATA_COMMAND(bus))) & ATA_DCR_DRQ)) {
   if (status & ATA_DCR_ERR)
       error_throw(E_IOERROR);
   task_yield();
  }
  /* DRQ */
  read_cmd[9] = 1;              /* 1 sector */
  read_cmd[2] = (lba >> 0x18) & 0xFF;   /* most sig. byte of LBA */
  read_cmd[3] = (lba >> 0x10) & 0xFF;
  read_cmd[4] = (lba >> 0x08) & 0xFF;
  read_cmd[5] = (lba >> 0x00) & 0xFF;   /* least sig. byte of LBA */
  /* Send ATAPI/SCSI command */
  outsw(ATA_DATA(bus),read_cmd,6);
  /* Wait for IRQ that says the data is ready. */
  if (!Ata_WaitForBusInterrupt(bus,Ata_InterruptTimeout))
      error_throw(E_IOERROR);
  /* Read actual size */
  size = ((size_t)inb(ATA_ADDRESS3(bus)) << 8) |
          (size_t)inb(ATA_ADDRESS2(bus));
  /* This example code only supports the case where the data transfer
    * of one sector is done in one step. */
  assert(size == ATAPI_SECTOR_SIZE);
  /* Read data. */
  insw(ATA_DATA(bus),buffer,size / 2);
  /* The controller will send another IRQ even though we've read all
   * the data we want.  Wait for it -- so it doesn't interfere with
   * subsequent operations: */
  Ata_WaitForBusInterrupt(bus,10);
  /* Wait for BSY and DRQ to clear, indicating Command Finished */
  while ((status = inb(ATA_COMMAND(bus))) & 0x88)
      task_yield();
 } FINALLY {
  mutex_put(&Ata_SubSystemLock);
 }
}
#endif

PRIVATE ATTR_FREETEXT void KCALL
Ata_RegisterAtaPiDevice(u16 bus, u8 drive) {
 debug_printf("[ATA] Found ATAPI device on %.4I16x:%.2I8x\n",bus,drive);
 /* TODO */
}



PRIVATE ATTR_FREETEXT void KCALL
Ata_ProbeBusLocations(u16 bus, u8 drive) {
 u8 cl,ch;

 /* Do a soft reset on both ATA device control ports. */
 Ata_ResetBus(bus);

 /* Select the requested drive. */
 outb(ATA_DRIVE_SELECT(bus),drive);
 ATA_SELECT_DELAY(bus);

 /* Wait for the command to be acknowledged. */
 Ata_WaitForBusy(bus);

 /* Figure out what kind of drive this is. */
 cl = inb(ATA_ADDRESS2(bus));
 ch = inb(ATA_ADDRESS3(bus));
 /*debug_printf("cl = %x, ch = %x\n",cl,ch);*/

 /* This combination indicates no-device. */
 if (cl == 0xff && ch == 0xff)
     return;

 if ((cl == 0x00 && ch == 0x00) || /* PATA */
     (cl == 0x3c && ch == 0xc3))   /* SATA */
      Ata_RegisterAtaDevice(bus,drive);
  
 if ((cl == 0x14 && ch == 0xeb) || /* PATAPI */
     (cl == 0x69 && ch == 0x96))   /* SATAPI */
      Ata_RegisterAtaPiDevice(bus,drive);
}

DEFINE_DRIVER_INIT(Ata_InitializeSubSystem);
INTERN ATTR_FREETEXT void KCALL Ata_InitializeSubSystem(void) {
 TRY Ata_ProbeBusLocations(ATA_BUS_PRIMARY,ATA_DRIVE_MASTER); CATCH_HANDLED(E_IOERROR) {}
 TRY Ata_ProbeBusLocations(ATA_BUS_PRIMARY,ATA_DRIVE_SLAVE); CATCH_HANDLED(E_IOERROR) {}
 TRY Ata_ProbeBusLocations(ATA_BUS_SECONDARY,ATA_DRIVE_MASTER); CATCH_HANDLED(E_IOERROR) {}
 TRY Ata_ProbeBusLocations(ATA_BUS_SECONDARY,ATA_DRIVE_SLAVE); CATCH_HANDLED(E_IOERROR) {}
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_CORE_ATA_C */
