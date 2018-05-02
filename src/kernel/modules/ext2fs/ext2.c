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
#ifndef GUARD_KERNEL_MODULES_EXT2FS_EXT2_C
#define GUARD_KERNEL_MODULES_EXT2FS_EXT2_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/byteswap.h>
#include <hybrid/byteorder.h>
#include <hybrid/align.h>
#include <fs/node.h>
#include <fs/path.h>
#include <fs/driver.h>
#include <fs/iomode.h>
#include <fs/device.h>
#include <kernel/debug.h>
#include <string.h>
#include <except.h>

#include "ext2.h"

DECL_BEGIN

STATIC_ASSERT(sizeof(Ext2INode) == 128);


INTERN ATTR_RETNONNULL struct block_group *KCALL
Ext2_Group(struct superblock *__restrict fs, ext2_bgroup_t index, iomode_t mode) {
 Ext2 *ext = fs->s_fsdata;
 struct block_group *result;
 assert(index < ext->sd_bgroups_cnt);
 result = &ext->sd_groups[index];
 if (!(result->bg_flags & BLOCK_GROUP_FLOADED)) {
  Ext2Blockgroup group;
  if (block_device_read(fs->s_device,&group,sizeof(group),
                        ext->sd_bgroups_pos+
                        index*EXT2_BLOCKGROUP_SIZE,
                        mode) != sizeof(group))
      error_throw(E_WOULDBLOCK);
  rwlock_write(&ext->sd_lock);
  COMPILER_READ_BARRIER();
  if (!(ATOMIC_READ(result->bg_flags) & BLOCK_GROUP_FLOADED)) {
   assert(!result->bg_busage);
   assert(!result->bg_iusage);
   /* Fill in the block groups data fields. */
   result->bg_busage_addr = BSWAP_LE2H32(group.bg_busage);
   result->bg_iusage_addr = BSWAP_LE2H32(group.bg_iusage);
   result->bg_inodes      = BSWAP_LE2H32(group.bg_inodes);
   result->bg_free_blocks = BSWAP_LE2H16(group.bg_free_blocks);
   result->bg_free_inodes = BSWAP_LE2H16(group.bg_free_inodes);
   result->bg_num_dirs    = BSWAP_LE2H16(group.bg_num_dirs);
   result->bg_flags |= BLOCK_GROUP_FLOADED;
  }
  rwlock_endwrite(&ext->sd_lock);
 }
 return result;
}

INTERN pos_t KCALL
Ext2_InoAddr(struct superblock *__restrict fs,
             ext2_ino_t ino, iomode_t mode) {
 struct block_group *block; pos_t result;
 ext2_bgroup_t group; ext2_ino_t offset;
 Ext2 *ext = fs->s_fsdata;
 ASSERT_BOUNDS(ext->sd_bound_inodes,ino);
 group   = EXT2_INO_BGRP_INDEX(ext,ino);
 offset  = EXT2_INO_BGRP_OFFSET(ext,ino);
 block   = Ext2_Group(fs,group,mode);
 result  = EXT2_BLOCK2ADDR(ext,block->bg_inodes);
 result += offset * ext->sd_inode_size;
 return result;
}



INTERN void KCALL
Ext2_ReadINode(struct superblock *__restrict fs,
               ext2_ino_t ino,
               Ext2INode *__restrict node,
               iomode_t mode) {
 pos_t addr = Ext2_InoAddr(fs,ino,mode);
#if 0
 debug_printf("READ_INODE: %I32u at %I64u\n",ino,addr);
#endif
 if (block_device_read(fs->s_device,node,128,addr,mode) != 128)
     error_throw(E_WOULDBLOCK);
}
INTERN void KCALL
Ext2_WriteINode(struct superblock *__restrict fs,
                ext2_ino_t ino,
                Ext2INode const *__restrict node,
                iomode_t mode) {
 pos_t addr = Ext2_InoAddr(fs,ino,mode);
 if (block_device_write(fs->s_device,node,128,addr,mode) != 128)
     error_throw(E_WOULDBLOCK);
}


PRIVATE void KCALL
free_x2_table(struct block_table_x2 *__restrict self, size_t size) {
 size_t i;
 assert(self->b2_tables);
 for (i = 0; i < size; ++i)
    kfree(self->b2_tables[i]);
 kfree(self->b2_tables);
 kfree(self);
}

PRIVATE void KCALL
free_x3_table(struct block_table_x3 *__restrict self, size_t size) {
 size_t i;
 assert(self->b3_tables);
 for (i = 0; i < size; ++i) {
  if (self->b3_tables[i])
      free_x2_table(self->b3_tables[i],size);
 }
 kfree(self->b3_tables);
 kfree(self);
}

INTERN void KCALL
ExtINode_Fini(struct inode *__restrict self) {
 struct inode_data *node = self->i_fsdata;
 if (!node) return;
 kfree(node->i_siblock);
 if (node->i_diblock)
     free_x2_table(node->i_diblock,self->i_super->s_fsdata->sd_ind_blocksize);
 if (node->i_tiblock)
     free_x3_table(node->i_tiblock,self->i_super->s_fsdata->sd_ind_blocksize);
 kfree(node);
}


INTERN void KCALL
ExtINode_LoadAttr(struct inode *__restrict self) {
 Ext2INode data; Ext2 *ext;
 struct inode_data *node;
 unsigned int i; mode_t real_mode;
 Ext2_ReadINode(self->i_super,
                self->i_attr.a_ino,
               &data,IO_RDONLY);
 if ((node = self->i_fsdata) == NULL) {
  /* Lazily allocate INode-specific data. */
  node = (struct inode_data *)kmalloc(sizeof(struct inode_data),
                                      GFP_SHARED|GFP_CALLOC);
  self->i_fsdata = node;
 }
 assert(!node->i_siblock);
 assert(!node->i_diblock);
 assert(!node->i_tiblock);
 /* Copy unused, but cached fields. */
 node->i_dtime      = data.i_dtime;
 node->i_flags      = data.i_flags;
 node->i_os1        = data.i_os1;
 node->i_generation = data.i_generation;
 node->i_acl        = data.i_acl;
 node->i_fragment   = data.i_fragment;
 node->i_os2[0]     = data.i_os2[0];
 node->i_os2[1]     = data.i_os2[1];
 node->i_os2[2]     = data.i_os2[2];
 /* Copy block pointers. */
 for (i = 0; i < EXT2_DIRECT_BLOCK_COUNT; ++i)
     node->i_dblock[i] = BSWAP_LE2H32(data.i_dblock[i]);
 node->i_siblock_addr = BSWAP_LE2H32(data.i_siblock);
 node->i_diblock_addr = BSWAP_LE2H32(data.i_diblock);
 node->i_tiblock_addr = BSWAP_LE2H32(data.i_tiblock);
 /* Read general-purpose INode attributes. */
 self->i_nlink = BSWAP_LE2H16(data.i_nlink);
 real_mode = BSWAP_LE2H16(data.i_mode);
 /* Race condition: The file got deleted and another node with the
  *                 same ID, but of a different type may have been
  *                 created in the mean time.
  *                 Handle this case by acting as though the file
  *                 had been deleted.
  * (XXX: This really shouldn't have happened though, unless user-space
  *       has manually been writing to the underlying partitions, because
  *       otherwise the file should still be of the same type, as an
  *       API call going through the kernel would have marked our INode
  *       as deleted, meaning that there'd be no situation where its
  *       attributes could still be loadable... At least I think there
  *       isn't a way, although this behavior might be allowed?)
  *       Anyways: Just handle miss-matching INode types as missing files. */
 if ((self->i_attr.a_mode & S_IFMT) != (real_mode & S_IFMT))
      error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_FILE_NOT_FOUND);
 self->i_attr.a_mode          = real_mode;
 self->i_attr.a_uid           = BSWAP_LE2H16(data.i_uid);
 self->i_attr.a_gid           = BSWAP_LE2H16(data.i_gid);
 self->i_attr.a_size          = BSWAP_LE2H32(data.i_size_low);
 self->i_attr.a_atime.tv_sec  = BSWAP_LE2H32(data.i_atime);
 self->i_attr.a_atime.tv_nsec = 0;
 self->i_attr.a_mtime.tv_sec  = BSWAP_LE2H32(data.i_mtime);
 self->i_attr.a_mtime.tv_nsec = 0;
 self->i_attr.a_ctime.tv_sec  = BSWAP_LE2H32(data.i_ctime);
 self->i_attr.a_ctime.tv_nsec = 0;
 self->i_attr.a_blocks        = BSWAP_LE2H32(data.i_blocks);

 ext = self->i_super->s_fsdata;

 /* Parse Version-specific fields. */
 if (ext->sd_feat_mountro & EXT2_FEAT_MRO_FSIZE64)
     self->i_attr.a_size |= (u64)BSWAP_LE2H32(data.i_size_high) << 32;

 /* Parse OS-specific fields. */
 switch (ext->sd_os) {

 case EXT2_OS_FGNU_HURD:
  self->i_attr.a_mode |= BSWAP_LE2H16(data.i_os_hurd.h_mode_high) << 16;
  /* Linux-specific fields are also implemented by HURD, so just fallthrough */
  ATTR_FALLTHROUGH
 case EXT2_OS_FLINUX:
  self->i_attr.a_uid |= BSWAP_LE2H16(data.i_os_linux.l_uid_high) << 16;
  self->i_attr.a_gid |= BSWAP_LE2H16(data.i_os_linux.l_gid_high) << 16;
  break;

 default: break;
 }
}

INTERN void KCALL
ExtINode_SaveAttr(struct inode *__restrict self) {
 Ext2INode data; Ext2 *ext; unsigned int i;
 struct inode_data *node = self->i_fsdata;
 assert(node != NULL);
 /* Copy unused, but cached fields. */
 data.i_dtime      = node->i_dtime;
 data.i_flags      = node->i_flags;
 data.i_os1        = node->i_os1;
 data.i_generation = node->i_generation;
 data.i_acl        = node->i_acl;
 data.i_fragment   = node->i_fragment;
 data.i_os2[0]     = node->i_os2[0];
 data.i_os2[1]     = node->i_os2[1];
 data.i_os2[2]     = node->i_os2[2];
 /* Copy block pointers. */
 for (i = 0; i < EXT2_DIRECT_BLOCK_COUNT; ++i)
     data.i_dblock[i] = BSWAP_H2LE32(node->i_dblock[i]);
 data.i_siblock = BSWAP_H2LE32(node->i_siblock_addr);
 data.i_diblock = BSWAP_H2LE32(node->i_diblock_addr);
 data.i_tiblock = BSWAP_H2LE32(node->i_tiblock_addr);
 /* Write general-purpose INode attributes. */
 data.i_nlink    = BSWAP_H2LE16(self->i_nlink);
 data.i_mode     = BSWAP_H2LE16(self->i_attr.a_mode);
 data.i_uid      = BSWAP_H2LE16((u16)self->i_attr.a_uid);
 data.i_gid      = BSWAP_H2LE16((u16)self->i_attr.a_gid);
 data.i_size_low = BSWAP_H2LE32(self->i_attr.a_size);
 data.i_atime    = BSWAP_H2LE32((u32)self->i_attr.a_atime.tv_sec);
 data.i_mtime    = BSWAP_H2LE32((u32)self->i_attr.a_mtime.tv_sec);
 data.i_ctime    = BSWAP_H2LE32((u32)self->i_attr.a_ctime.tv_sec);
 data.i_blocks   = BSWAP_H2LE32((u32)self->i_attr.a_blocks);

 ext = self->i_super->s_fsdata;

 /* Parse Version-specific fields. */
 if (ext->sd_feat_mountro & EXT2_FEAT_MRO_FSIZE64)
      data.i_size_high = BSWAP_H2LE32((u32)(self->i_attr.a_size >> 32));
 else data.i_size_high = (le32)0;

 /* Encode OS-specific fields. */
 switch (ext->sd_os) {

 case EXT2_OS_FGNU_HURD:
  data.i_os_hurd.h_mode_high = BSWAP_H2LE16((u16)(self->i_attr.a_mode >> 16));
  /* Linux-specific fields are also implemented by HURD, so just fallthrough */
  ATTR_FALLTHROUGH
 case EXT2_OS_FLINUX:
  data.i_os_linux.l_uid_high = BSWAP_H2LE16((u16)(self->i_attr.a_uid >> 16));
  data.i_os_linux.l_gid_high = BSWAP_H2LE16((u16)(self->i_attr.a_gid >> 16));
  break;

 default: break;
 }

 /* Write the new INode descriptor to disk. */
 Ext2_WriteINode(self->i_super,
                 self->i_attr.a_ino,
                &data,IO_WRONLY);
}



/* Returns the single-indirection table for the given INode.
 * NOTE: The caller is responsible to ensure that attributes of the
 *       INode have been loaded (`inode_loadattr()' has been called) */
INTERN ATTR_RETNONNULL struct block_table *KCALL
Ext_GetINodeSTable(struct inode *__restrict self, iomode_t flags) {
 struct block_table *result; Ext2 *ext;
 if ((result = self->i_fsdata->i_siblock) != NULL)
      return result;
 ext = self->i_super->s_fsdata;
 /* Allocate a new block table. */
 assert(ext->sd_blocksize == ext->sd_ind_blocksize*sizeof(ext2_block_t));
 result = (struct block_table *)kmalloc(offsetof(struct block_table,bt_blocks)+
                                        ext->sd_blocksize,GFP_SHARED);
 TRY {
  result->bt_flags = BLOCK_TABLE_FNORMAL;
  /* Read the table from disk. */
  if (block_device_read(self->i_super->s_device,
                        result->bt_blocks,
                        ext->sd_blocksize,
                        EXT2_BLOCK2ADDR(ext,self->i_fsdata->i_siblock_addr),
                        flags) != ext->sd_blocksize)
      error_throw(E_WOULDBLOCK);

  /* Save the buffer within the INode. */
  rwlock_write(&self->i_lock);
  /* Check if the block has been allocated in the mean time. */
  if unlikely(self->i_fsdata->i_siblock) {
   struct block_table *new_result;
   new_result = self->i_fsdata->i_siblock;
   rwlock_endwrite(&self->i_lock);
   kfree(result);
   return new_result;
  }
  /* Save the block. */
  self->i_fsdata->i_siblock = result;
  rwlock_endwrite(&self->i_lock);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(result);
  error_rethrow();
 }
 return result;
}

/* Return the indirection block table for doubly and triply indirect block pointers:
 * Ext_GetINodeDTable() -> i_diblock->b2_tables[x2_index]
 * Ext_GetINodeTTable() -> i_tiblock->b3_tables[x3_index]->b2_tables[x2_index] */
INTERN ATTR_RETNONNULL struct block_table *KCALL
Ext_GetINodeDTable(struct inode *__restrict self,
                   size_t x2_index, iomode_t flags) {
 /* TODO (It's really just the same as `Ext_GetINodeSTable()') */
 error_throw(E_NOT_IMPLEMENTED);
}
INTERN ATTR_RETNONNULL struct block_table *KCALL
Ext_GetINodeTTable(struct inode *__restrict self,
                   size_t x3_index, size_t x2_index, iomode_t flags) {
 /* TODO (It's really just the same as `Ext_GetINodeSTable()') */
 error_throw(E_NOT_IMPLEMENTED);
}



/* Return the data block referenced by the given `index' within `self'.
 * If the data block hasn't been allocated, return ZERO(0), which
 * the caller should then interpret as the corresponding data being
 * all ZEROes. */
INTERN ext2_block_t KCALL
Ext_GetINodeBlockAtIndex(struct inode *__restrict self,
                         ext2_blockid_t index, iomode_t flags) {
 Ext2 *ext; size_t ind_tabsize;
 /* Make sure that INode attribute have been loaded! */
 inode_loadattr(self);
 /* Check for direct data blocks. */
 if (index < EXT2_DIRECT_BLOCK_COUNT)
     return self->i_fsdata->i_dblock[index];
 index -= EXT2_DIRECT_BLOCK_COUNT;
 ext         = self->i_super->s_fsdata;
 ind_tabsize = ext->sd_ind_blocksize;
 /* Check for singly indirect data blocks. */
 if (index < ind_tabsize)
     return Ext_GetINodeSTable(self,flags)->bt_blocks[index];
 index -= ind_tabsize;
 /* Check for doubly indirect data blocks. */
 if (index < ind_tabsize*ind_tabsize) {
  /* Doubly indirect block pointers. */
  return Ext_GetINodeDTable(self,index >> (ext->sd_block_shift-2),
                                 flags)
                     ->bt_blocks[index & (ind_tabsize-1)];
 }
 index -= ind_tabsize*ind_tabsize;
 if (index < ind_tabsize*ind_tabsize*ind_tabsize) {
  /* Triply indirect block pointers. */
  return Ext_GetINodeTTable(self,index >> ((ext->sd_block_shift-2)*2),
                                (index >>  (ext->sd_block_shift-2)) & (ind_tabsize-1),
                                 flags)
                     ->bt_blocks[index & (ind_tabsize-1)];
 }
 /* Fallback: anything still in excess of this
  *           point is just considered unallocated. */
 return 0;
}




INTERN size_t KCALL
Ext_ReadFromINode(struct inode *__restrict self,
                  CHECKED USER void *buf, size_t bufsize,
                  pos_t pos, iomode_t flags) {
 Ext2 *ext = self->i_super->s_fsdata;
 ext2_blockid_t block_id; size_t offset;
 ext2_block_t block;
 size_t result = 0;
 if (bufsize) for (;;) {
  size_t max_read;
  block_id = (ext2_blockid_t)(pos >> ext->sd_block_shift);
  offset   = (size_t)pos & ext->sd_blockmask;
  max_read = ext->sd_blocksize - offset;
  block    = Ext_GetINodeBlockAtIndex(self,block_id,flags);
  if (block >= ext->sd_total_blocks) {
   unsigned int i;
   debug_printf("BAD_BLOCK\n");
   for (i = 0; i < 12; ++i)
       debug_printf("\tDBLOCK[%d] = %I32u (%I32x)\n",i,
                     self->i_fsdata->i_dblock[i],self->i_fsdata->i_dblock[i]);
  }
  if (!block) {
   /* Optimize for contingency */
   while (max_read < bufsize &&
          Ext_GetINodeBlockAtIndex(self,block_id+1,flags) == 0)
          ++block_id,max_read += ext->sd_blocksize;
   if (max_read > bufsize)
       max_read = bufsize;
   /* Unallocated blocks always yield all zeros. */
   memset(buf,0,max_read);
   COMPILER_WRITE_BARRIER();
   result  += max_read;
   bufsize -= max_read;
   if (!bufsize) break;
  } else {
   ext2_block_t next_block = block+1;
   size_t disk_read;
   /* Optimize for contingency */
   while (max_read < bufsize &&
          Ext_GetINodeBlockAtIndex(self,block_id+1,flags) == next_block)
          ++block_id,++next_block,
          max_read += ext->sd_blocksize;
   if (max_read > bufsize)
       max_read = bufsize;
   /* Read data from disk. */
   disk_read = block_device_read(self->i_super->s_device,
                                 buf,
                                 max_read,
                                 EXT2_BLOCK2ADDR(ext,block)+offset,
                                 flags);
   result  += disk_read;
   bufsize -= disk_read;
   if (!bufsize) break;
   if (disk_read != max_read)
       break; /* Not everything could be read (the operation would have blocked...) */
  }
  /* Continue reading the next block. */
  pos                += max_read;
  *(uintptr_t *)&buf += max_read;
 }
 return result;
}

INTERN size_t KCALL
Ext_WriteToINode(struct inode *__restrict self,
                 CHECKED USER void const *buf, size_t bufsize,
                 pos_t pos, iomode_t flags) {
 /* TODO */
 error_throw(E_NOT_IMPLEMENTED);
}


INTERN size_t KCALL
ExtINode_PRead(struct inode *__restrict self,
               CHECKED USER void *buf, size_t bufsize,
               pos_t pos, iomode_t flags) {
 size_t result = bufsize;
 inode_loadattr(self);
 /* Truncate the effective read-size. */
 if (pos + result > self->i_attr.a_size)
     result = self->i_attr.a_size - pos;
 result = Ext_ReadFromINode(self,(byte_t *)buf,result,pos,flags);
 return result;
}

INTERN size_t KCALL
ExtINode_PWrite(struct inode *__restrict self,
                CHECKED USER void const *buf, size_t bufsize,
                pos_t pos, iomode_t flags) {
 struct inode *EXCEPT_VAR xself = self;
 pos_t new_size = pos+bufsize;
 inode_loadattr(self);
 if (new_size > self->i_attr.a_size) {
  pos_t EXCEPT_VAR old_size;
  /* Limit the max write area to 32 bits */
  if unlikely(new_size > (u32)-1) {
   if unlikely(pos >= (u32)-1)
      return 0;
   new_size = (u32)-1;
   bufsize = new_size - pos;
   if (self->i_attr.a_size == new_size)
       goto do_write;
  }
  old_size = self->i_attr.a_size;
  self->i_attr.a_size = new_size;
  /* Update the file's size. */
  TRY {
   bufsize = Ext_WriteToINode(self,(byte_t *)buf,bufsize,pos,flags);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   xself->i_attr.a_size = old_size;
   error_rethrow();
  }
  inode_changed(self);
  return bufsize;
 }
do_write:
 return Ext_WriteToINode(self,(byte_t *)buf,bufsize,pos,flags);
}


INTERN REF struct directory_entry *KCALL
ExtDirectory_ReadDir(struct directory_node *__restrict self,
                     pos_t *__restrict pentry_pos,
                     iomode_t flags) {
 Ext2Dirent entry; u16 entsize,namlen;
 REF struct directory_entry *result;
 unsigned char entry_type; pos_t entry_pos;
 inode_loadattr(&self->d_node);
again:
 entry_pos = *pentry_pos;
 if (Ext_ReadFromINode(&self->d_node,&entry,sizeof(Ext2Dirent),
                        entry_pos,flags) != sizeof(Ext2Dirent))
     error_throw(E_WOULDBLOCK);
 entsize = BSWAP_LE2H16(entry.d_entsize);
 if (entsize <= sizeof(Ext2Dirent)) return NULL; /* End of directory */
 *pentry_pos += entsize;
 if (!entry.d_ino) goto again; /* Unused entry. */
 if (self->d_node.i_super->s_fsdata->sd_feat_required & EXT2_FEAT_REQ_FDIRENT_TYPE) {
  entry_type = entry.d_type;
  namlen     = entry.d_namlen_low;
  if ((entsize-sizeof(Ext2Dirent)) > 0xff) {
   /* The filename is longer than 255 characters, and
    * we only have the least significant 8 bits of its
    * actual length.
    * To deal with this, we must scan ahead and read a byte at every
    * `entry_pos + sizeof(Ext2Dirent) + entry.d_namlen_low + N * 256'
    * that is still located below `entry_pos + entsize' until we find
    * a NUL-character. */
   pos_t name_endpos,name_endpos_max;
   name_endpos      = entry_pos;
   name_endpos     += sizeof(Ext2Dirent);
   name_endpos     += entry.d_namlen_low;
   name_endpos     += 256*sizeof(char);
   name_endpos_max  = entry_pos;
   name_endpos_max += entsize;
   while (name_endpos < name_endpos_max) {
    char ch;
    if (Ext_ReadFromINode(&self->d_node,&ch,sizeof(char),
                           name_endpos,flags) != sizeof(char))
        error_throw(E_WOULDBLOCK);
    if (!ch) break;
    name_endpos += 256*sizeof(char);
   }
   namlen = (u16)((name_endpos-sizeof(Ext2Dirent)) - entry_pos)/sizeof(char);
  }
 } else {
  pos_t ino_addr; le16 ino_type;
  ino_addr = Ext2_InoAddr(self->d_node.i_super,
                          BSWAP_LE2H32(entry.d_ino),
                          flags);
  /* Read the directory entry type from its INode. */
  if (block_device_read(self->d_node.i_super->s_device,&ino_type,2,
                        ino_addr+offsetof(Ext2INode,i_mode),flags) != 2)
      error_throw(E_WOULDBLOCK);
  entry_type = IFTODT(BSWAP_LE2H16(ino_type));
  namlen = BSWAP_LE2H16(entry.d_namlen);
 }
 /* Construct the resulting directory entry. */
 result = (REF struct directory_entry *)kmalloc(offsetof(struct directory_entry,de_name)+
                                               (namlen+1)*sizeof(char),GFP_SHARED);
 TRY {
  result->de_refcnt  = 1;
  result->de_pos     = entry_pos;
  result->de_ino     = BSWAP_LE2H32(entry.d_ino);
  result->de_namelen = namlen;
  result->de_type    = entry_type;
  /* Read in the directory entry's name. */
  if (Ext_ReadFromINode(&self->d_node,result->de_name,namlen*sizeof(char),
                         entry_pos+sizeof(Ext2Dirent),flags) !=
                         namlen*sizeof(char))
      error_throw(E_WOULDBLOCK);

  switch (namlen) {
  case 2:
   if (result->de_name[1] != '.') break;
   ATTR_FALLTHROUGH
  case 1:
   if (result->de_name[0] != '.') break;
   /* Skip `.' and `..' -- Those are emulated by the VFS layer. */
   kfree(result);
   goto again;
  default: break;
  }

  /* Ensure NUL-termination, and generate the hash. */
  result->de_name[namlen] = '\0';
  result->de_hash = directory_entry_hash(result->de_name,namlen);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(result);
  error_rethrow();
 }
 return result;
}



PRIVATE void KCALL
ExtINode_ReadLink(struct symlink_node *__restrict self) {
 char *text; size_t textlen;
 /* Make sure that INode attributes have been loaded.
  * -> Required for the `a_size' field. */
 inode_loadattr(&self->sl_node);
 textlen = (size_t)self->sl_node.i_attr.a_size;
 text = (char *)kmalloc(textlen*sizeof(char),GFP_SHARED);
 TRY {
  struct inode_data *node;
  node = self->sl_node.i_fsdata;
  if (textlen*sizeof(char) <= (EXT2_DIRECT_BLOCK_COUNT+3)*4
#if 0 /* XXX: ASCII data is usually written in a way that causes this check to succeed,
       *      but what about 1 or 2-character links? This is little endian after all,
       *      so that would end up with a really small number that might be lower
       *      that the actual number block blocks...
       * LATER: From what little I can gather, at some point Ext2 just started placing
       *        symlink data that was small enough within the INode itself.
       *        Although sources state that prior to this data was written in actual
       *        blocks, what isn't stated is anything about how to differenciate
       *        these two cases other than the link size. */
      &&
      node->i_dblock[0] >= self->sl_node.i_super->s_fsdata->sd_total_blocks
#endif
      ) {
   /* XXX: Is this really how we discern between the 2 methods?
    *      Shouldn't there be some kind of flag somewhere? */
   memcpy(text,&node->i_dblock,textlen*sizeof(char));
  } else {
   /* Read the symlink text. */
   Ext_ReadFromINode(&self->sl_node,
                      text,
                      textlen,
                      0,
                      IO_RDONLY);
  }
  /* Save the symlink text in its designated location. */
  self->sl_text = text;
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(text);
  error_rethrow();
 }
}




PRIVATE struct inode_operations Ext2_DirectoryOps = {
    .io_fini     = &ExtINode_Fini,
    .io_loadattr = &ExtINode_LoadAttr,
    .io_saveattr = &ExtINode_SaveAttr,
    .io_directory = {
        .d_readdir = &ExtDirectory_ReadDir,
    }
};

PRIVATE struct inode_operations Ext2_RegularOps = {
    .io_fini     = &ExtINode_Fini,
    .io_loadattr = &ExtINode_LoadAttr,
    .io_saveattr = &ExtINode_SaveAttr,
    .io_file = {
        .f_pread  = &ExtINode_PRead,
        .f_pwrite = &ExtINode_PWrite,
    }
};

PRIVATE struct inode_operations Ext2_DeviceOps = {
    .io_fini     = &ExtINode_Fini,
    .io_loadattr = &ExtINode_LoadAttr,
    .io_saveattr = &ExtINode_SaveAttr,
};

PRIVATE struct inode_operations Ext2_SymlinkOps = {
    .io_fini     = &ExtINode_Fini,
    .io_loadattr = &ExtINode_LoadAttr,
    .io_saveattr = &ExtINode_SaveAttr,
    .io_symlink  = {
        .sl_readlink = &ExtINode_ReadLink,
    }
};



PRIVATE void KCALL
Ext2FS_Open(struct superblock *__restrict self,
            UNCHECKED USER char *args) {
 Ext2Superblock super;
 struct block_device *dev = self->s_device;
 struct superblock_data *ext;
 u32 num_block_groups,temp;
 bool must_mount_ro = false;

 /* Read the superblock. */
 block_device_read(dev,&super,sizeof(super),
                   EXT2_SUPERBLOCK_OFFSET,
                   IO_RDONLY);

 /* Do some basic verification to check if this is really an EXT2 filesystem. */
 if (BSWAP_LE2H16(super.e_signature) != EXT2_SIGNATURE)
     error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_CORRUPTED_FILESYSTEM);

 /* Do some validation on the superblock state, and see if what there is even possible. */
 if (BSWAP_LE2H32(super.e_free_inodes) > BSWAP_LE2H32(super.e_total_inodes) ||
     BSWAP_LE2H32(super.e_free_blocks) > BSWAP_LE2H32(super.e_total_blocks))
     error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_CORRUPTED_FILESYSTEM);
 if (!super.e_blocks_per_group ||
     !super.e_inodes_per_group)
     error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_CORRUPTED_FILESYSTEM);

 num_block_groups = CEILDIV(BSWAP_LE2H32(super.e_total_blocks),
                            BSWAP_LE2H32(super.e_blocks_per_group));
 temp             = CEILDIV(BSWAP_LE2H32(super.e_total_inodes),
                            BSWAP_LE2H32(super.e_inodes_per_group));
 if (num_block_groups != temp)
     error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_CORRUPTED_FILESYSTEM);
 if (!num_block_groups)
     error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_CORRUPTED_FILESYSTEM);

 /* Allocate Ext2-specific superblock data. */
 ext = (struct superblock_data *)kmalloc(COMPILER_OFFSETOF(struct superblock_data,sd_groups)+
                                         num_block_groups*sizeof(struct block_group),
                                         GFP_SHARED|GFP_CALLOC);
 self->s_fsdata = ext;

 ext->sd_block_shift = 10+BSWAP_LE2H32(super.e_log2_blocksz);
 if unlikely(ext->sd_block_shift >= 32)
    error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_CORRUPTED_FILESYSTEM);

 ext->sd_blocksize     = 1 << ext->sd_block_shift;
 ext->sd_blockmask     = ext->sd_blocksize-1;
 ext->sd_ind_blocksize = ext->sd_blocksize/4;

#if 1 /* I'm guessing this is the right answer, but the wiki really isn't clear...
       * [...] The table is located in the block immediately following the Superblock
       *    -> That would mean a fixed partition offset of `2048'
       * [...] the Block Group Descriptor Table will begin at block 2 [for 1024 bytes per block].
       *       For any other block size, it will begin at block 1
       * I'm going with the later variant here, since that seems to make the most sense. */
 if (ext->sd_block_shift <= 10)
      ext->sd_bgroups_pos = EXT2_BLOCK2ADDR(ext,2);
 else ext->sd_bgroups_pos = EXT2_BLOCK2ADDR(ext,1);
#else
 ext->sd_bgroups_pos = EXT2_SUPERBLOCK_OFFSET+1024;
#endif
 ext->sd_bgroups_cnt = num_block_groups;
 ext->sd_ino_per_bgrp = BSWAP_LE2H32(super.e_inodes_per_group);
 ext->sd_blk_per_bgrp = BSWAP_LE2H32(super.e_blocks_per_group);
 ext->sd_total_inodes = BSWAP_LE2H32(super.e_total_inodes);
 ext->sd_total_blocks = BSWAP_LE2H32(super.e_total_blocks);
 ext->sd_version = ((u32)BSWAP_LE2H16(super.e_version_major) << 16 |
                    (u32)BSWAP_LE2H16(super.e_version_minor));
 ext->sd_bound_inodes.b_min = 1;
 rwlock_cinit(&ext->sd_lock);

 if (ext->sd_version >= EXT2_VERSION_1) {
  ext->sd_feat_optional = BSWAP_LE2H32(super.e_feat_optional);
  ext->sd_feat_required = BSWAP_LE2H32(super.e_feat_required);
  ext->sd_feat_mountro  = BSWAP_LE2H32(super.e_feat_mountro);
  ext->sd_inode_size    = BSWAP_LE2H16(super.e_inode_size);
  /* Make sure that the specified INode size isn't too small. */
  if unlikely(ext->sd_inode_size < 128)
     error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_CORRUPTED_FILESYSTEM);

  /* Check if we're implementing all the required features.
   * NOTE: Just to improve compatibility, we treat journaling
   *       as a mount-ro feature. */
  if (ext->sd_feat_required & ~(EXT2_FEAT_REQ_FDIRENT_TYPE|
                                EXT2_FEAT_REQ_FREPLAY_JOURNAL|
                                EXT2_FEAT_REQ_FJOURNAL))
      error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_CORRUPTED_FILESYSTEM);
  if (ext->sd_feat_required & (EXT2_FEAT_REQ_FREPLAY_JOURNAL|
                               EXT2_FEAT_REQ_FJOURNAL))
      must_mount_ro = true;
  if (ext->sd_feat_mountro & ~(EXT2_FEAT_MRO_FSIZE64))
      must_mount_ro = true;
 } else {
  ext->sd_inode_size = 128;
 }

 /* On EXT2 filesystem, the root directory is always Inode number #2 */
 self->s_root->d_node.i_attr.a_ino = EXT2_ROOT_DIRECTORY_INO;
 self->s_root->d_node.i_ops        = &Ext2_DirectoryOps;

 (void)must_mount_ro; /* TODO */

}

PRIVATE void KCALL
Ext2FS_Fini(struct superblock *__restrict self) {
 Ext2 *data = self->s_fsdata;
 if (data) {
  ext2_bgroup_t i;
  /* Free lazily allocated block group buffers. */
  for (i = 0; i < data->sd_bgroups_cnt; ++i) {
   kfree(data->sd_groups[i].bg_busage);
   kfree(data->sd_groups[i].bg_iusage);
  }
  kfree(data);
 }
}



PRIVATE void KCALL
Ext2FS_OpenNode(struct inode *__restrict node,
                struct directory_node *__restrict UNUSED(parent_directory),
                struct directory_entry *__restrict UNUSED(parent_directory_entry)) {
 switch (node->i_attr.a_mode & S_IFMT) {

 case S_IFREG:
  node->i_ops = &Ext2_RegularOps;
  break;

 case S_IFCHR:
 case S_IFBLK:
  node->i_ops = &Ext2_DeviceOps;
  break;

 case S_IFDIR:
  node->i_ops = &Ext2_DirectoryOps;
  break;

 case S_IFLNK:
  node->i_ops = &Ext2_SymlinkOps;
  break;

 default:
  /* Throw an unsupported-function error for any other type of node. */
  error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_UNSUPPORTED_FUNCTION);
 }
}

INTERN struct superblock_type ext2fs_type = {
    .st_name      = "ext2",
    .st_flags     = SUPERBLOCK_TYPE_FNORMAL,
    .st_driver    = &this_driver,
    .st_open      = &Ext2FS_Open,
    .st_functions = {
        .f_fini     = &Ext2FS_Fini,
        .f_opennode = &Ext2FS_OpenNode
    }
};

DEFINE_FILESYSTEM_TYPE(ext2fs_type);

DECL_END

#endif /* !GUARD_KERNEL_MODULES_EXT2FS_EXT2_C */
