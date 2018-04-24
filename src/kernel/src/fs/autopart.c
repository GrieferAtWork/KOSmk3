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
#ifndef GUARD_KERNEL_SRC_FS_AUTOPART_C
#define GUARD_KERNEL_SRC_FS_AUTOPART_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/debug.h>
#include <fs/device.h>
#include <assert.h>
#include <except.h>

#include "autopart.h"
#include <hybrid/byteswap.h>

DECL_BEGIN

PRIVATE minor_t KCALL
efi_autopart(struct block_device *__restrict parent_partition,
             pos_t efi_start, minor_t partition_id) {
 debug_printf("TODO: EFI partition\n");

 return partition_id;
}


PRIVATE void KCALL
block_device_delparts(struct block_device *__restrict self) {
 struct block_device *part;
 assert(self->b_master == self);
 atomic_rwlock_write(&self->b_partlock);
 for (;;) {
  part = ATOMIC_READ(self->b_partitions);
  if (!part) break; /* All existing partitions have been removed. */
  LIST_REMOVE(part,b_partition.p_partitions);
  part->b_partition.p_partitions.le_pself = NULL;
  part->b_partition.p_partitions.le_next  = NULL;
  /* Keep a reference to the device doesn't die on us while we unregister it. */
  block_device_incref(part);
  atomic_rwlock_endwrite(&self->b_partlock);
  TRY {
   unregister_device(&part->b_device);
  } FINALLY {
   block_device_decref(part);
  }
  atomic_rwlock_write(&self->b_partlock);
 }
 atomic_rwlock_endwrite(&self->b_partlock);
}

PRIVATE minor_t KCALL
block_device_do_autopart(struct block_device *__restrict self,
                         minor_t partition_id) {
 minor_t EXCEPT_VAR xpartition_id = partition_id;
 mbr_data_t mbr;
 unsigned int i;
 /* Read the MBR disk header. */
 block_device_read(self,&mbr,sizeof(mbr),MBR_DATA_OFFSET,IO_RDONLY);
 /* Now create the new set of partitions. */

 if (mbr.mbr_sig[0] != 0x55 ||
     mbr.mbr_sig[1] != 0xaa)
     return partition_id;
 /* Enumerate the MBR disk array. */
 for (i = 0; i < COMPILER_LENOF(mbr.mbr_part); ++i) {
  u64 lba_start,lba_size;
  if (mbr.mbr_part[i].pt.pt_sysid == 0)
      continue; /* Empty partition */
  if ((mbr.mbr_part[i].pt.pt_bootable & PART_BOOTABLE_LBA48) &&
      (mbr.mbr_part[i].pt_48.pt_sig1 == PART48_SIG1 &&
       mbr.mbr_part[i].pt_48.pt_sig2 == PART48_SIG2)) {
   lba_start  = (u64)BSWAP_LE2H32(mbr.mbr_part[i].pt_48.pt_lbastart);
   lba_start |= (u64)BSWAP_LE2H16(mbr.mbr_part[i].pt_48.pt_lbastarthi) << 32;
   lba_size   = (u64)BSWAP_LE2H32(mbr.mbr_part[i].pt_48.pt_lbasize);
   lba_size  |= (u64)BSWAP_LE2H16(mbr.mbr_part[i].pt_48.pt_lbasizehi) << 32;
  } else {
   lba_start = (u64)BSWAP_LE2H32(mbr.mbr_part[i].pt_32.pt_lbastart);
   lba_size  = (u64)BSWAP_LE2H32(mbr.mbr_part[i].pt_32.pt_lbasize);
  }
  if unlikely(!lba_size)
     continue;
  TRY {
   REF struct block_device *part;
   if (mbr.mbr_part[i].pt.pt_sysid == 0xee) {
    /* Special case: EFI partition */
    xpartition_id = efi_autopart(self,lba_start,xpartition_id);
   } else {
    /* Construct a new partition. */
    do part = block_device_partition(self,lba_start,lba_size,xpartition_id,
                                    (u8 *)&mbr.mbr_part[i]),
       ++xpartition_id;
    while (!part);
    debug_printf("[MBR] Created partition #%d (%[dev]) for %I64u...%I64u of %[dev] (%I64ux%Iu bytes)\n",
                (int)xpartition_id-1,part->b_device.d_devno,
                (u64)(part->b_partstart),
                (u64)(part->b_partstart+part->b_blockcount),
                      part->b_master->b_device.d_devno,
                (u64)(part->b_blockcount),part->b_blocksize);
    TRY {
     if (mbr.mbr_part[i].pt.pt_sysid == 0x05 &&
         /* NOTE: Check for `lba_start' to prevent infinite recursion! */
         lba_start != 0) {
      /* Recursively scan and load extended partitions. */
      xpartition_id = block_device_do_autopart(part,xpartition_id);
     }
    } FINALLY {
     block_device_decref(part);
    }
   }
  } CATCH (E_INVALID_ARGUMENT) {
   /* Invalid partition. */
  }
 }
 return xpartition_id;
}

PUBLIC void KCALL
block_device_autopart(struct block_device *__restrict self) {
 /* Delete existing partitions. */
 block_device_delparts(self);
 /* Recursively create partitions for this device. */
 block_device_do_autopart(self,0);
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_AUTOPART_C */
