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
#ifndef GUARD_KERNEL_SRC_CORE_FAT_C
#define GUARD_KERNEL_SRC_CORE_FAT_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1 /* mempcpy() */

#include <hybrid/align.h>
#include <hybrid/byteswap.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/host.h>
#include <hybrid/section.h>
#include <hybrid/timeutil.h>
#include <kos/types.h>
#include <kernel/debug.h>
#include <kernel/user.h>
#include <kernel/malloc.h>
#include <kernel/vm.h>
#include <fs/device.h>
#include <fs/driver.h>
#include <fs/linker.h>
#include <fs/node.h>
#include <fs/iomode.h>
#include <linux/msdos_fs.h>

#include <fcntl.h>
#include <alloca.h>
#include <assert.h>
#include <byteswap.h>
#include <ctype.h>
#include <dirent.h>
#include <endian.h>
#include <except.h>
#include <string.h>

#include "fat.h"

DECL_BEGIN
#define FAT_ISSPACE(c)  isctype(c,(_ISspace|_IScntrl|_ISblank))

/* Fat INode operators. */
PRIVATE ATTR_NOTHROW void KCALL Fat_FinalizeNode(struct inode *__restrict self);
PRIVATE ATTR_NOTHROW void KCALL Fat16_FinalizeRootDirectoryNode(struct inode *__restrict self);
PRIVATE size_t KCALL Fat32_PRead(struct inode *__restrict self, CHECKED USER void *buf, size_t bufsize, pos_t pos, iomode_t flags);
PRIVATE size_t KCALL Fat32_PWrite(struct inode *__restrict self, CHECKED USER void const *buf, size_t bufsize, pos_t pos, iomode_t flags);
PRIVATE void KCALL Fat_LoadINodeFromFatFile(struct inode *__restrict self, FatFile const *__restrict file);
PRIVATE void KCALL Fat_SaveINodeToFatFile(struct inode const *__restrict self, FatFile *__restrict file);
PRIVATE void KCALL Fat32_LoadAttr(struct inode *__restrict self);
PRIVATE void KCALL Fat32_SaveAttr(struct inode *__restrict self);
PRIVATE void KCALL Fat32_MaskAttr(struct inode *__restrict self);
PRIVATE void KCALL Fat16_RootDirectoryEntryLoadAttr(struct inode *__restrict self);
PRIVATE void KCALL Fat16_RootDirectoryEntrySaveAttr(struct inode *__restrict self);
PRIVATE ssize_t KCALL Fat_Ioctl(struct inode *__restrict self, unsigned long cmd, USER UNCHECKED void *arg, iomode_t flags);
PRIVATE void KCALL Fat_TruncateINode(struct inode *__restrict self, pos_t new_smaller_size);
PRIVATE REF struct directory_entry *KCALL Fat_ReadDirectory(struct directory_node *__restrict self, pos_t *__restrict pentry_pos, iomode_t flags);
PRIVATE void KCALL Fat_CreateFile(struct directory_node *__restrict target_directory,
                                  struct directory_entry *__restrict target_dirent,
                                  struct regular_node *__restrict new_node);
PRIVATE void KCALL Fat_CreateDirectory(struct directory_node *__restrict target_directory,
                                       struct directory_entry *__restrict target_dirent,
                                       struct directory_node *__restrict new_node);



/* File creation helper functions. */

/* Allocate and write new entries for `new_node' under
 * the name `target_dirent' within `target_directory'. */
PRIVATE void KCALL
Fat_AddFileToDirectory(struct directory_node *__restrict target_directory,
                       struct directory_entry *__restrict target_dirent,
                       struct inode *__restrict new_node);
/* Construct the 8.3 FatFile for `self', using `entry' as filename.
 * This function automatically determines if an LFN entry must be used
 * and saves the 8.3 version of the filename in `file' before returning
 * the number of consecutive `FatFile' structures required to save the entry.
 * In the event that the filename can be represented in 8.3 format, ONE(1) is returned.
 * @return: * : The number of consecutive directory entries
 *              required to safe the given filename.
 *              To actually perform that write, use `Fat_Write8Dot3Filename()'
 * @throw: E_FS_ERROR.ERROR_FS_ILLEGAL_PATH: `entry' contains illegal FAT filename characters. */
PRIVATE u32 KCALL Fat_Create8Dot3Filename(struct directory_node *__restrict target_directory,
                                          struct directory_entry *__restrict entry,
                                          struct inode *__restrict self, FatFile *__restrict file);
/* NOTE: This function initializes the following fields of `self':
 *    - i_attr.a_ino */
PRIVATE void KCALL Fat_Write8Dot3Filename(struct directory_node *__restrict target_directory,
                                          struct directory_entry *__restrict entry,
                                          FatFile *__restrict file, pos_t pos, u32 num_entries,
                                          struct inode *__restrict self);
/* Allocate/free unused ranges in FAT directories.
 * Free range indices are directory-stream positions divided by `sizeof(FatFile)'
 * Multiple free entries refer to consecutive entries that all have a `MARKER_UNUSED' marker.
 * When `FatDirectory_GetFreeRange()' is called, the function will check if there is a known
 * free range of at least `num_entries' consecutive files, and if there is, delete its internal
 * representation and return the index of its first part.
 * If there isn't, continue enumeration of the directory until `INODE_FDIRLOADED' is set,
 * checking for free ranges of sufficient sizes all-the-while.
 * If the directory has been fully enumerated without locating a free range of sufficient
 * size, return `self->d_dirend / sizeof(FatFile)', indicating that new entries should be
 * appended at the end.
 * NOTE: The caller must be holding a write-lock to `self' */
PRIVATE u32 KCALL FatDirectory_GetFreeRange(struct directory_node *__restrict self, u32 num_entries);
PRIVATE void KCALL FatDirectory_AddFreeRange(struct directory_node *__restrict self, u32 starting_entry, u32 num_entries);
/* Similar to `FatDirectory_AddFreeRange(entry_number,1)', but don't cause PANIC() if
 * the given `entry_number' had already been registered as a free directory entry. */
PRIVATE void KCALL FatDirectory_AddFreeFile(struct directory_node *__restrict self, u32 entry_number);


/* The fat get/set implementation for different table sizes. */
PRIVATE FatClusterIndex KCALL Fat12_GetFatIndirection(Fat *__restrict self, FatClusterIndex index);
PRIVATE FatClusterIndex KCALL Fat16_GetFatIndirection(Fat *__restrict self, FatClusterIndex index);
PRIVATE FatClusterIndex KCALL Fat32_GetFatIndirection(Fat *__restrict self, FatClusterIndex index);
PRIVATE void KCALL Fat16_SetFatIndirection(Fat *__restrict self, FatClusterIndex index, FatClusterIndex indirection_target);
PRIVATE void KCALL Fat12_SetFatIndirection(Fat *__restrict self, FatClusterIndex index, FatClusterIndex indirection_target);
PRIVATE void KCALL Fat32_SetFatIndirection(Fat *__restrict self, FatClusterIndex index, FatClusterIndex indirection_target);
PRIVATE FatSectorIndex KCALL Fat12_GetTableSector(Fat *__restrict self, FatClusterIndex id);
PRIVATE FatSectorIndex KCALL Fat16_GetTableSector(Fat *__restrict self, FatClusterIndex id);
PRIVATE FatSectorIndex KCALL Fat32_GetTableSector(Fat *__restrict self, FatClusterIndex id);

INTDEF struct inode_operations const Fat_FileNodeOperators;
INTDEF struct inode_operations const Fat_DirectoryNodeOperators;
INTDEF struct inode_operations const Fat16_RootDirectoryFileEntryNodeOperators;
INTDEF struct inode_operations const Fat16_RootDirectoryDirectoryEntryNodeOperators;
INTDEF struct inode_operations const Fat16_RootDirectoryNodeOperators;
INTDEF struct inode_operations const Fat32_RootDirectoryNodeOperators;


PRIVATE ATTR_NORETURN void KCALL
throw_fs_error(u16 fs_error_code) {
 struct exception_info *info;
 info = error_info();
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_code                        = E_FILESYSTEM_ERROR;
 info->e_error.e_flag                        = ERR_FNORMAL;
 info->e_error.e_filesystem_error.fs_errcode = fs_error_code;
 error_throw_current();
 __builtin_unreachable();
}

INTERN size_t KCALL
Fat_ReadFromCluster(struct superblock *__restrict self,
                    CHECKED USER byte_t *buf,
                    size_t bufsize, pos_t abs_diskpos,
                    FatClusterIndex cluster, iomode_t flags) {
 size_t temp,result = bufsize;
 Fat *fat = self->s_fsdata;
 TRY {
  while (bufsize) {
   size_t max_io;
   size_t cluster_offset = (size_t)(abs_diskpos % fat->f_clustersize);
   if (cluster >= fat->f_cluster_eof) {
    /* Read all ZEROes after EOF. */
    memset(buf,0,bufsize);
    return bufsize;
   }
   max_io = fat->f_clustersize - cluster_offset;
   /* Optimization: When reading large amounts of data, check if the
    *               underlying disk chunks were allocated consecutively.
    *               If they were, then we can simply do one continuous
    *               read, processing more than one cluster at a time. */
   COMPILER_WRITE_BARRIER();
   while (unlikely(max_io < bufsize) &&
          Fat_GetFatIndirection(self,cluster,flags) == cluster+1) {
    max_io += fat->f_clustersize;
    ++cluster;
   }
   if (max_io > bufsize)
       max_io = bufsize;
   temp = block_device_read(self->s_device,
                            buf,
                            max_io,
                            abs_diskpos,
                            flags);
   bufsize -= max_io;
   if (!bufsize) break;
   if unlikely(temp < max_io) {
    result -= bufsize;
    break;
   }
   abs_diskpos += max_io,buf += max_io;
   COMPILER_WRITE_BARRIER();
   cluster = Fat_GetFatIndirection(self,cluster,flags);
  }
 } CATCH (E_WOULDBLOCK) {
  result -= bufsize;
 }
 return result;
}
INTERN size_t KCALL
Fat_WriteToCluster(struct superblock *__restrict self,
                   CHECKED USER byte_t const *buf,
                   size_t bufsize, pos_t abs_diskpos,
                   FatClusterIndex cluster, iomode_t flags) {
 size_t temp,result = bufsize;
 Fat *fat = self->s_fsdata;
 TRY {
  while (bufsize) {
   size_t max_io;
   size_t cluster_offset = (size_t)(abs_diskpos % fat->f_clustersize);
   assert(cluster < fat->f_cluster_eof);
   max_io = fat->f_clustersize - cluster_offset;
   /* Optimization: When reading large amounts of data, check if the
    *               underlying disk chunks were allocated consecutively.
    *               If they were, then we can simply do one continuous
    *               read, processing more than one cluster at a time. */
   COMPILER_WRITE_BARRIER();
   while (unlikely(max_io < bufsize) &&
          Fat_GetFatIndirection(self,cluster,flags) == cluster+1) {
    max_io += fat->f_clustersize;
    ++cluster;
   }
   if (max_io > bufsize)
       max_io = bufsize;
   temp = block_device_write(self->s_device,
                             buf,
                             max_io,
                             abs_diskpos,
                             flags);
   bufsize -= max_io;
   if (!bufsize) break;
   if unlikely(temp < max_io) {
    result -= bufsize;
    break;
   }
   abs_diskpos += max_io,buf += max_io;
   COMPILER_WRITE_BARRIER();
   cluster = Fat_GetFatIndirection(self,cluster,flags);
   if (cluster >= fat->f_cluster_eof_marker) {
    FatClusterIndex next_index;
    /* Allocate another cluster. */
    mutex_getf(&fat->f_fat_lock,flags);
    TRY {
     /* Allocate a new, free cluster. */
     next_index = Fat_FindFreeCluster(self,flags);
     Fat_SetFatIndirection(self,next_index,fat->f_cluster_eof_marker,flags); /* next -> void */
     Fat_SetFatIndirection(self,cluster,next_index,flags);                   /* prev -> next */
    } FINALLY {
     mutex_put(&fat->f_fat_lock);
    }
    cluster = next_index;
   }
  }
 } CATCH (E_WOULDBLOCK) {
  result -= bufsize;
 }
 return result;
}


INTERN size_t KCALL
Fat32_ReadFromINode(struct inode *__restrict self,
                    CHECKED USER byte_t *buf,
                    size_t bufsize, pos_t pos, iomode_t flags) {
 size_t temp,result = bufsize;
 Fat *fat = self->i_super->s_fsdata;
 assert(self->i_super->s_root != (struct directory_node *)self ||
        self->i_super->s_fsdata->f_type == FAT32);
 if (bufsize) for (;;) {
  FatClusterIndex cluster; pos_t diskpos;
  size_t cluster_number = (size_t)(pos / fat->f_clustersize);
  size_t cluster_offset = (size_t)(pos % fat->f_clustersize);
  size_t max_io;
  cluster = Fat_GetFileCluster(self,cluster_number,
                               FAT_GETCLUSTER_MODE_FNORMAL,
                               flags);
  if (cluster >= fat->f_cluster_eof) {
   /* Read all ZEROes after EOF. */
   memset(buf,0,bufsize);
   return bufsize;
  }
  diskpos  = FAT_CLUSTERADDR(fat,cluster);
  diskpos += cluster_offset;
  max_io   = fat->f_clustersize - cluster_offset;
  /* Optimization: When reading large amounts of data, check if the
   *               underlying disk chunks were allocated consecutively.
   *               If they were, then we can simply do one continuous
   *               read, processing more than one cluster at a time. */
  while (max_io < bufsize &&
         Fat_GetFileCluster(self,cluster_number+1,
                            FAT_GETCLUSTER_MODE_FNORMAL,
                            flags) ==
         cluster+1) {
   max_io += fat->f_clustersize;
   ++cluster_number,++cluster;
  }
  if (max_io > bufsize)
      max_io = bufsize;
  temp = block_device_read(self->i_super->s_device,
                           buf,
                           max_io,
                           diskpos,
                           flags);
  bufsize -= max_io;
  if (!bufsize) break;
  if unlikely(temp < max_io) {
   result -= bufsize;
   break;
  }
  pos += max_io;
  buf += max_io;
 }
 return result;
}
INTERN size_t KCALL
Fat32_WriteToINode(struct inode *__restrict self,
                   CHECKED USER byte_t const *buf,
                   size_t bufsize, pos_t pos, iomode_t flags) {
 size_t temp,result = bufsize;
 Fat *fat = self->i_super->s_fsdata;
 assert(self->i_super->s_root != (struct directory_node *)self ||
        self->i_super->s_fsdata->f_type == FAT32);
 if (bufsize) for (;;) {
  FatClusterIndex cluster; pos_t diskpos;
  size_t cluster_number = (size_t)(pos / fat->f_clustersize);
  size_t cluster_offset = (size_t)(pos % fat->f_clustersize);
  size_t max_io;
  cluster = Fat_GetFileCluster(self,cluster_number,
                               FAT_GETCLUSTER_MODE_FCREATE|
                               FAT_GETCLUSTER_MODE_FNOZERO,
                               flags);
  diskpos  = FAT_CLUSTERADDR(fat,cluster);
  diskpos += cluster_offset;
  max_io   = fat->f_clustersize - cluster_offset;
  /* Optimization: When reading large amounts of data, check if the
   *               underlying disk chunks were allocated consecutively.
   *               If they were, then we can simply do one continuous
   *               read, processing more than one cluster at a time. */
  while (max_io < bufsize &&
         Fat_GetFileCluster(self,cluster_number+1,
                            FAT_GETCLUSTER_MODE_FCREATE|
                            FAT_GETCLUSTER_MODE_FNOZERO,
                            flags) ==
         cluster+1) {
   max_io += fat->f_clustersize;
   ++cluster_number,++cluster;
  }
  if (max_io > bufsize)
      max_io = bufsize;
  temp = block_device_write(self->i_super->s_device,
                            buf,
                            max_io,
                            diskpos,
                            flags);
  bufsize -= max_io;
  if (!bufsize) break;
  if unlikely(temp < max_io) {
   result -= bufsize;
   break;
  }
  pos += max_io;
  buf += max_io;
 }
 return result;
}

INTERN size_t KCALL
Fat16_ReadFromRootDirectory(struct inode *__restrict self,
                            CHECKED USER byte_t *buf,
                            size_t bufsize, pos_t pos, iomode_t flags) {
 size_t max_read,temp;
 assert(self->i_super->s_root == (struct directory_node *)self);
 assert(self->i_super->s_fsdata->f_type != FAT32);
 max_read = self->i_fsdata->i16_root.f16_rootsiz;
 if (pos >= max_read) { memset(buf,0,bufsize); return bufsize; }
 max_read -= (size_t)pos;
 /* Clear out trailing memory. */
 if (max_read < bufsize)
     memset(buf+max_read,0,bufsize-max_read);
 else max_read = bufsize;
 /* Read actual data. */
 temp = block_device_read(self->i_super->s_device,
                          buf,
                          max_read,
                          self->i_fsdata->i16_root.f16_rootpos + pos,
                          flags);
 if (temp < max_read)
     return temp;
 return bufsize;
}
INTERN size_t KCALL
Fat16_WriteToRootDirectory(struct inode *__restrict self,
                           CHECKED USER byte_t const *buf,
                           size_t bufsize, pos_t pos, iomode_t flags) {
 size_t max_write;
 assert(self->i_super->s_root == (struct directory_node *)self);
 assert(self->i_super->s_fsdata->f_type != FAT32);
 max_write = self->i_fsdata->i16_root.f16_rootsiz;
 if (pos >= max_write) goto too_large;
 max_write -= (size_t)pos;
 if (max_write < bufsize) goto too_large;
 /* Write actual data. */
 return block_device_write(self->i_super->s_device,
                           buf,
                           bufsize,
                           self->i_fsdata->i16_root.f16_rootpos + pos,
                           flags);
too_large:
 throw_fs_error(ERROR_FS_DISK_FULL);
}

INTERN size_t KCALL
Fat_ReadFromINode(struct inode *__restrict self,
                  CHECKED USER byte_t *buf,
                  size_t bufsize, pos_t pos, iomode_t flags) {
 if (self != (struct inode *)self->i_super->s_root ||
     self->i_super->s_fsdata->f_type == FAT32) {
  return Fat32_ReadFromINode(self,buf,bufsize,pos,flags);
 } else {
  return Fat16_ReadFromRootDirectory(self,buf,bufsize,pos,flags);
 }
}
INTERN size_t KCALL
Fat_WriteToINode(struct inode *__restrict self,
                 CHECKED USER byte_t const *buf,
                 size_t bufsize, pos_t pos, iomode_t flags) {
 if (self != (struct inode *)self->i_super->s_root ||
     self->i_super->s_fsdata->f_type == FAT32) {
  return Fat32_WriteToINode(self,buf,bufsize,pos,flags);
 } else {
  return Fat16_WriteToRootDirectory(self,buf,bufsize,pos,flags);
 }
}
INTERN pos_t KCALL
Fat_GetAbsDiskPos(struct inode *__restrict self, pos_t pos, iomode_t flags) {
 if (self != (struct inode *)self->i_super->s_root ||
     self->i_super->s_fsdata->f_type == FAT32) {
  Fat *fat = self->i_super->s_fsdata;
  FatClusterIndex cluster; pos_t diskpos;
  size_t cluster_number = (size_t)(pos / fat->f_clustersize);
  size_t cluster_offset = (size_t)(pos % fat->f_clustersize);
  cluster = Fat_GetFileCluster(self,cluster_number,
                               FAT_GETCLUSTER_MODE_FCREATE|
                               FAT_GETCLUSTER_MODE_FNOZERO,
                               flags);
  diskpos  = FAT_CLUSTERADDR(fat,cluster);
  diskpos += cluster_offset;
  return diskpos;
 }
 pos += self->i_fsdata->i16_root.f16_rootpos;
 return pos;
}





INTERN FatClusterIndex KCALL
Fat_GetFileCluster(struct inode *__restrict node,
                   size_t nth_cluster,
                   unsigned int mode, iomode_t flags) {
 FatClusterIndex result;
 FatNode *data = node->i_fsdata;
 Fat *fat = node->i_super->s_fsdata;
 bool has_write_lock = false;
 assert(rwlock_reading(&node->i_lock));
 if unlikely(!data->i_clusterc) {
  /* Load the initial file cluster. */
  inode_loadattr(node);
  assert(data->i_clusterc != 0);
 }
 if (nth_cluster < data->i_clusterc) {
  result = data->i_clusterv[nth_cluster];
  if unlikely(result >= fat->f_cluster_eof) {
   /* Deal with EOF clusters (check if we're supposed to create new ones) */
   result = fat->f_cluster_eof_marker;
   goto create_cluster;
  }
  return result;
 }
 result = fat->f_cluster_eof_marker;
 TRY {
  for (;;) {
   FatClusterIndex next_index;
   assertf(nth_cluster >= data->i_clusterc,
           "nth_cluster      = %Iu\n"
           "data->i_clusterc = %Iu\n",
           nth_cluster,data->i_clusterc);
   if (data->i_clusterv[data->i_clusterc-1] >= fat->f_cluster_eof) {
create_cluster:
    /* Last cluster points into the void.
     * Check if we're supposed to create more. */
    if (!(mode & FAT_GETCLUSTER_MODE_FCREATE)) break;
    if (!has_write_lock) {
     rwlock_write(&node->i_lock);
     has_write_lock = true;
    }
    mutex_getf(&fat->f_fat_lock,flags);
    TRY {
     /* Allocate a new, free cluster. */
     next_index = Fat_FindFreeCluster(node->i_super,flags);
     if (mode & FAT_GETCLUSTER_MODE_FNOZERO) {
      /* XXX: ZERO-initialize the new memory? */
     }
     /* Mark the cluster as an EOF cluster. */
     Fat_SetFatIndirection(node->i_super,next_index,
                           fat->f_cluster_eof_marker,flags);
     data->i_clusterv[data->i_clusterc-1] = next_index;
     if (data->i_clusterc == 1) {
      /* The pointer to the first cluster is stored in the INode.
       * Since we've just written that pointer, mark the node as changed. */
      inode_changed(node);
     } else {
      /* Link the previous cluster onto the new one */
      Fat_SetFatIndirection(node->i_super,
                            data->i_clusterv[data->i_clusterc-2],
                            next_index,flags);
     }
    } FINALLY {
     mutex_put(&fat->f_fat_lock);
    }
    if (nth_cluster == data->i_clusterc-1) {
     result = next_index;
     break;
    }
    assert(data->i_clusterv[data->i_clusterc-1] < fat->f_cluster_eof);
   }
   /* Dereference the FAT table at the previous index. */
   next_index = data->i_clusterv[data->i_clusterc-1];
   next_index = Fat_GetFatIndirection(node->i_super,next_index,flags);
   assert(data->i_clusterc <= data->i_clustera);
   if (!has_write_lock) {
    rwlock_write(&node->i_lock);
    has_write_lock = true;
   }
   if (data->i_clusterc == data->i_clustera) {
    /* Allocate what should be a sufficient number of clusters. */
    size_t new_alloc = CEILDIV(node->i_attr.a_size,
                               fat->f_clustersize)+1;
    if unlikely(new_alloc < data->i_clustera)
                new_alloc = data->i_clustera*2;
    if unlikely(!new_alloc) new_alloc = 2;
    TRY {
     data->i_clusterv = (FatClusterIndex *)krealloc(data->i_clusterv,new_alloc*
                                                    sizeof(FatClusterIndex),
                                                    GFP_SHARED);
    } CATCH (E_BADALLOC) {
     /* Try to allocate the minimum increment. */
     new_alloc = data->i_clusterc+1;
     data->i_clusterv = (FatClusterIndex *)krealloc(data->i_clusterv,new_alloc*
                                                    sizeof(FatClusterIndex),
                                                    GFP_SHARED);
    }
    data->i_clustera = new_alloc;
   }
   assert(data->i_clusterc < data->i_clustera);
   /* Add the new cluster information to the node. */
   data->i_clusterv[data->i_clusterc++] = next_index;
   if (nth_cluster == data->i_clusterc-1) {
    result = next_index;
    break;
   }
  }
 } FINALLY {
  if (has_write_lock)
      rwlock_endwrite(&node->i_lock);
 }
 return result;
}















/* ====================================================================== */
/*   FAT Directory file creation functions.                               */
/* ====================================================================== */

PRIVATE u8 const dos8dot3_valid[256/8] = {
/*[[[deemon
#include <util>
function is_valid(ch) {
    if (ch <= 31 || ch == 127) return false;
    return string.chr(ch) !in "\"*+,/:;<=>?\\[]|.";
}
local valid_chars = [false] * 256;
for (local x: util.range(256))
    valid_chars[x] = is_valid(x);
local valid_bits = [0] * (256/8);
for (local x: util.range(256/8)) {
    local mask = 0;
    for (local y: util.range(8)) {
        if (valid_chars[x*8+y])
            mask = mask | (1 << y);
    }
    valid_bits[x] = mask;
}
for (local x: util.range(256/8)) {
    if ((x % 8) == 0) {
        print "\n    ",;
    }
    print "0x%.2I8x," % valid_bits[x],;
}
]]]*/
    0x00,0x00,0x00,0x00,0xfb,0x23,0xff,0x03,
    0xff,0xff,0xff,0xc7,0xff,0xff,0xff,0x6f,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
//[[[end]]]
};
//LOCAL ATTR_CONST int KCALL dos8dot3_isvalid(char ch) {
// if (ch <= 31 || ch == 127) return 0;
// return !strchr("\"*+,/:;<=>?\\[]|.",ch);
//}
#define dos8dot3_isvalid(ch) \
       (dos8dot3_valid[((u8)(ch))/8] & (1 << (((u8)(ch))%8)))

PRIVATE u8 KCALL
Fat_CalculateLFNCheckSum(char const *__restrict dos83) {
 unsigned int i; u8 sum = 0;
 for (i = 11; i; i--)
     sum = ((sum & 1) << 7) + (sum >> 1) + *dos83++;
 return sum;
}

PRIVATE u32 KCALL
Fat_Create8Dot3Filename(struct directory_node *__restrict target_directory,
                        struct directory_entry *__restrict entry,
                        struct inode *__restrict self, FatFile *__restrict file) {
 char *name,*extstart; bool has_mixed_case = false;
 size_t extsize,basesize,namesize; u32 result;
 /* Save generic INode attributes (does not include the 8.3 filename). */
 Fat_SaveINodeToFatFile(self,file);
 /* Make sure the entire directory has been read so we can disambiguate file names. */
 while (!(target_directory->d_node.i_flags&INODE_FDIRLOADED) &&
          directory_readnext(target_directory,IO_RDONLY) != NULL);

 memset(file->f_nameext,' ',sizeof(file->f_nameext));
 file->f_ntflags = NTFLAG_NONE;
 /* Strip leading+terminating dots & space. */
 name     = entry->de_name;
 namesize = entry->de_namelen;
 while (namesize && (FAT_ISSPACE(*name) || *name == '.')) ++name,--namesize;
 while (namesize && (FAT_ISSPACE(name[namesize-1]))) --namesize;
 extstart = name+namesize;
 while (extstart != name && (extstart[-1] != '.')) --extstart;
 if (extstart == name) {
  extstart = name+namesize; /* No extension */
  extsize  = 0;
  basesize = namesize;
 } else {
  basesize = (size_t)(extstart-name)-1;
  extsize  = (namesize-basesize)-1;
 }

 /* Strip space characters before the extension and after the name. */
 while (namesize && FAT_ISSPACE(name[namesize-1])) ++name,--namesize;
 while (extsize && FAT_ISSPACE(*extstart)) ++extstart,--extsize;

 /* Generate the extension */
 if (extsize) {
  char *dst,*iter,*end,ch;
  dst = file->f_nameext+8;
  end = (iter = extstart)+MIN(extsize,3);
  file->f_ntflags |= NTFLAG_LOWEXT;
  while (iter != end) {
   ch = *iter++;
   if (islower(ch)) {
    if (!(file->f_ntflags&NTFLAG_LOWEXT))
        has_mixed_case = true;
    ch = toupper(ch);
   } else {
    if (file->f_ntflags&NTFLAG_LOWEXT && iter != extstart)
        has_mixed_case = true;
    file->f_ntflags &= ~(NTFLAG_LOWEXT);
   }
   if unlikely(!dos8dot3_isvalid(ch))
      ch = '~';
   *dst++ = ch;
  }
 }

 /* Confirm that the name and extension fit DOS8.3 */
 if (basesize <= 8 && extsize <= 3 &&
     name == entry->de_name && name+basesize == extstart-!!extsize &&
     extstart+extsize == entry->de_name+entry->de_namelen) {
  char *iter,*end,*dst,ch;
  /* We can generate a (possibly mixed-case) 8.3-compatible filename */
  end = (iter = name)+basesize,dst = file->f_name;
  file->f_ntflags |= NTFLAG_LOWBASE;
  while (iter != end) {
   ch = *iter++;
   if (islower(ch)) {
    if (!(file->f_ntflags&NTFLAG_LOWBASE))
        has_mixed_case = true;
    ch = toupper(ch);
   } else {
    if (file->f_ntflags&NTFLAG_LOWBASE && iter != extstart)
        has_mixed_case = true;
    file->f_ntflags &= ~(NTFLAG_LOWBASE);
   }
   if unlikely(!dos8dot3_isvalid(ch))
      ch = '~';
   *dst++ = ch;
  }
  /* Fix 0xE5 --> 0x05 (srsly, dos?) */
  if (((u8 *)file->f_name)[0] == 0xE5)
      ((u8 *)file->f_name)[0] = 0x05;
  if (has_mixed_case) goto need_lfn;
  result = 1;
 } else {
#define FAT_8DOT3_MAX_DISAMBIGUATION  (0xffff*9)
  unsigned int disambiguation,matchsize;
  unsigned int retry_hex,retry_dig;
  char *dst,*iter,*end,ch;
need_lfn:
  file->f_ntflags = NTFLAG_NONE;
  disambiguation  = 0;
  for (;; ++disambiguation) {
   /* Must __MUST__ generate a long filename, also
    * taking the value of 'retry' into consideration.
    * Now for the hard part: The filename itself... */
   retry_hex = (disambiguation / 9);
   retry_dig = (disambiguation % 9);

   /* The first 2 short characters always match the
    * first 2 characters of the original base (in uppercase).
    * If no hex-range retries are needed, the first 6 match. */
   matchsize = retry_hex ? 2 : 6;
   if (matchsize > basesize) matchsize = basesize;
   end = (iter = name)+matchsize,dst = file->f_nameext;
   while (iter != end) {
    ch = toupper(*iter++);
    *dst++ = dos8dot3_isvalid(ch) ? ch : '~';
   }
   if (retry_hex) {
    PRIVATE char const xch[16] = {'0','1','2','3','4','5','6','7',
                                  '8','9','A','B','C','D','E','F'};
    /* Following the matching characters are 4 hex-chars
     * whenever more than 9 retry attempts have failed
     * >> This multiplies the amount of available names by 0xffff */
    *dst++ = xch[(retry_hex & 0xf000) >> 12];
    *dst++ = xch[(retry_hex & 0x0f00) >> 8];
    *dst++ = xch[(retry_hex & 0x00f0) >> 4];
    *dst++ = xch[(retry_hex & 0x000f)];
   }
   assert(dst <= file->f_nameext+6);
   /* Following the shared name and the hex part is always a tilde '~' */
   *dst++ = '~';
   /* The last character then, is the non-hex digit (1..9) */
   *dst = '1'+retry_dig;
   /* Fix 0xE5 --> 0x05 (srsly, dos?) */
   if (((u8 *)file->f_nameext)[0] == 0xE5)
       ((u8 *)file->f_nameext)[0] =  0x05;
   /* Search if a directory entry with a DOS 8.3 name
    * matching this short filename already exists. */
   {
    struct directory_entry *iter;
    iter = target_directory->d_bypos;
    for (; iter; iter = iter->de_next) {
     if (memcmp(FAT_DIRECTORY_ENTRY83(iter),
                file->f_nameext,(8+3)*sizeof(char)) == 0)
         goto next_disambig;
    }
   }
   break;
next_disambig:
   if unlikely(disambiguation == FAT_8DOT3_MAX_DISAMBIGUATION)
      throw_fs_error(ERROR_FS_DISK_FULL);
  }
  result = 1+CEILDIV(entry->de_namelen,LFN_NAME);
 }

 /* Copy the 8.3 filename into the directory entry's FS-specific data field. */
 memcpy(FAT_DIRECTORY_ENTRY83(entry),
        file->f_nameext,(8+3)*sizeof(char));
 return result;
}

PRIVATE void KCALL
Fat_Write8Dot3Filename(struct directory_node *__restrict target_directory,
                       struct directory_entry *__restrict entry,
                       FatFile *__restrict file, pos_t pos, u32 num_entries,
                       struct inode *__restrict self) {
 FatFile *buffer;
 assert(num_entries);
 /* Construct the buffer. */
 buffer = (FatFile *)alloca(num_entries*sizeof(FatFile));
 memcpy(&buffer[num_entries-1],file,sizeof(FatFile));
 /* Generate LFN entries. */
 if (num_entries > 1) {
  char *src; unsigned int length; FatFile *iter,*end;
  u8 chksum = Fat_CalculateLFNCheckSum(file->f_nameext);
  length = entry->de_namelen+1; /* +1 because we're supposed to include the trailing \0 */
  assert(num_entries == 1+CEILDIV(length,LFN_NAME));
  src     = entry->de_name+length;
  length %= LFN_NAME;
  if (!length) length = LFN_NAME;
  src -= length;
  end = (iter = buffer)+(num_entries-1);
  iter->lfn_seqnum = 0x40; /* Weird marker... */
  for (; iter != end; ++iter) {
   le16 part[LFN_NAME]; unsigned int i;
   iter->lfn_seqnum |= (u8)((num_entries-1)-(iter-buffer));
   iter->lfn_attr    = ATTR_LONGFILENAME;
   iter->lfn_csum    = chksum;
   iter->lfn_type    = 0;
   iter->lfn_clus    = (le16)0;
   /* Translate the filename portion. */
   for (i = 0; i < length; ++i)
       part[i] = BSWAP_H2LE16((u8)src[i]);
   if (LFN_NAME != length)
       memset(part+length,0xff,2*(LFN_NAME-length));
   /* Copy the filename part into the buffer. */
   memcpyw(iter->lfn_name_1,part,LFN_NAME1);
   memcpyw(iter->lfn_name_2,part+LFN_NAME1,LFN_NAME2);
   memcpyw(iter->lfn_name_3,part+LFN_NAME1+LFN_NAME2,LFN_NAME3);
   src   -= LFN_NAME;
   length = LFN_NAME;
  }
 }

 /* Save the data to the directory. */
 Fat_WriteToINode(&target_directory->d_node,
                 (byte_t *)buffer,
                  num_entries*sizeof(FatFile),
                  pos,IO_WRONLY);
 /* Save directory and disk positions to generate the INO number. */
 entry->de_fsdata.de_start = pos;
 entry->de_pos             = pos + (num_entries-1)*sizeof(FatFile);
 self->i_attr.a_ino        = Fat_GetAbsDiskPos(&target_directory->d_node,entry->de_pos,IO_RDONLY);
 entry->de_ino             = self->i_attr.a_ino;
}
PRIVATE u32 KCALL
FatDirectory_GetFreeRange(struct directory_node *__restrict self,
                          u32 num_entries) {
 FatNode *node = self->d_node.i_fsdata; u32 result;
 FatDirectoryFreeRange *vector; size_t i,count;
 FatDirectoryFreeRange *candy = NULL;
again:
 vector = node->i_directory.i_freev;
 count  = node->i_directory.i_freec;
 for (i = 0; i < count; ++i) {
  if (vector[i].dfr_size < num_entries) continue; /* Too small. */
  /* Use the candidate (candy) that best matches the required size. */
  if (!candy || candy->dfr_size > vector[i].dfr_size)
       candy = &vector[i];
 }
 if (candy) {
  /* Allocate from this entry. */
  result            = candy->dfr_start;
  candy->dfr_start += num_entries;
  candy->dfr_size  -= num_entries;
  if (!candy->dfr_size) {
   /* Candy is now empty (delete it). */
   i = candy-vector;
   --node->i_directory.i_freec;
   memmove(candy,candy+1,
           node->i_directory.i_freec*
           sizeof(FatDirectoryFreeRange));
  }
  return result;
 }
 /* Make sure that the entirety of the directory has been loaded. */
 if (!(self->d_node.i_flags & INODE_FDIRLOADED)) {
  if (directory_readnext(self,IO_RDONLY)) {
#if 1
   /* Read all remaining entries... */
   while (directory_readnext(self,IO_RDONLY));
#endif
   goto again;
  }
 }
 /* Return a pointer to the end of the directory.
  * NOTE: Special handling here for FAT12/16 root directories. */
 if (self->d_node.i_super->s_root == self &&
     self->d_node.i_super->s_fsdata->f_type != FAT32 &&
     self->d_dirend+(num_entries*sizeof(FatFile)) >=
     self->d_node.i_fsdata->i16_root.f16_rootsiz)
     throw_fs_error(ERROR_FS_DISK_FULL);
 return (u32)(self->d_dirend / sizeof(FatFile));
}
PRIVATE void KCALL
FatDirectory_AllocFreeRange(FatNode *__restrict node) {
 size_t new_alloc = node->i_directory.i_freea*2;
 FatDirectoryFreeRange *vector;
 if (!new_alloc) new_alloc = 2;
 vector = node->i_directory.i_freev;
 TRY {
  vector = (FatDirectoryFreeRange *)krealloc(vector,new_alloc*
                                             sizeof(FatDirectoryFreeRange),
                                             GFP_SHARED|GFP_NOFS);
 } CATCH (E_BADALLOC) {
  /* Try with minimal increment. */
  new_alloc = node->i_directory.i_freec+1;
  vector = (FatDirectoryFreeRange *)krealloc(vector,new_alloc*
                                             sizeof(FatDirectoryFreeRange),
                                             GFP_SHARED|GFP_NOFS);
 }
 node->i_directory.i_freev = vector;
 node->i_directory.i_freea = new_alloc;
}

PRIVATE void KCALL
FatDirectory_AddFreeRange(struct directory_node *__restrict self,
                          u32 starting_entry, u32 num_entries) {
 FatNode *node = self->d_node.i_fsdata;
 FatDirectoryFreeRange *vector = node->i_directory.i_freev;
 size_t i,count = node->i_directory.i_freec;
 for (i = 0; i < count; ++i) {
  u32 entry_end = vector[i].dfr_start+vector[i].dfr_size;
  if (starting_entry > entry_end) continue;
  if (starting_entry != entry_end) break;
  /* Extend this entry. */
  vector[i].dfr_size += num_entries;
  if (i != count-1) {
   entry_end += num_entries;
   assert(entry_end <= vector[i+1].dfr_start);
   if (entry_end == vector[i+1].dfr_start) {
    /* Merge with successor. */
    vector[i].dfr_size += vector[i+1].dfr_size;
    --node->i_directory.i_freec;
    memmove(&vector[i+1],&vector[i+2],
           (count-2)*sizeof(FatDirectoryFreeRange));
   }
  }
  return;
 }
 /* Check to extend the following entry. */
 if (i != count-1) {
  assert(starting_entry+num_entries <= vector[i+1].dfr_start);
  if (starting_entry+num_entries == vector[i+1].dfr_start) {
   /* Extend the following range. */
   vector[i+1].dfr_start -= num_entries;
   vector[i+1].dfr_size  += num_entries;
   return;
  }
 }

 /* Insert at `i' */
 if (count == node->i_directory.i_freea) {
  FatDirectory_AllocFreeRange(node);
  vector = node->i_directory.i_freev;
 }
 if (i != node->i_directory.i_freec)
     memmove(&vector[i+1],&vector[i],
            (node->i_directory.i_freec-i)*
             sizeof(FatDirectoryFreeRange));
 /* Construct the entry. */
 vector[i].dfr_start = starting_entry;
 vector[i].dfr_size  = num_entries;
 ++node->i_directory.i_freec;
}
PRIVATE void KCALL
FatDirectory_AddFreeFile(struct directory_node *__restrict self,
                         u32 entry_number) {
 FatNode *node = self->d_node.i_fsdata;
 FatDirectoryFreeRange *vector = node->i_directory.i_freev;
 size_t index = node->i_directory.i_freec;
 /* Optimize for the case of adding new entries near the end. */
 assert(index <= node->i_directory.i_freea);
 for (; index; --index) {
  if (entry_number >= vector[index-1].dfr_start) {
   u32 entry_end;
   entry_end  = vector[index-1].dfr_start;
   entry_end += vector[index-1].dfr_size;
   if (entry_number < entry_end)
       return; /* Already marked as free */
   if (entry_number == entry_end) {
    /* Extend entry above. */
    ++vector[index-1].dfr_size;
    assertf(index == node->i_directory.i_freec ||
            entry_end+1 < vector[index-1].dfr_start,
            "We handle merging as a downwards operation");
    return;
   }
   /* Check if we can extend the next entry downwards. */
   if (index != node->i_directory.i_freec &&
       entry_number == vector[index].dfr_start-1) {
    --vector[index].dfr_start;
    assert(vector[index].dfr_start >= entry_end);
    assert(node->i_directory.i_freec >= 2);
    if (vector[index].dfr_start == entry_end) {
     /* Merge the 2 entries. */
     vector[index-1].dfr_size += vector[index].dfr_size;
     --node->i_directory.i_freec;
     memmove(&vector[index],&vector[index+1],
            (size_t)(node->i_directory.i_freec-index)*
             sizeof(FatDirectoryFreeRange));
    }
    return;
   }
   break;
  }
 }
 /* Insert a new entry at `index' */
 if (node->i_directory.i_freec == node->i_directory.i_freea) {
  FatDirectory_AllocFreeRange(node);
  vector = node->i_directory.i_freev;
 }
 /* Make space for the new entry. */
 if (index != node->i_directory.i_freec)
     memmove(&vector[index+1],&vector[index],
            (node->i_directory.i_freec-index)*
             sizeof(FatDirectoryFreeRange));
 /* Construct the entry. */
 vector[index].dfr_start = entry_number;
 vector[index].dfr_size  = 1;
 ++node->i_directory.i_freec;
}















/* ====================================================================== */
/*   FAT Inode operator functions.                                        */
/* ====================================================================== */
PRIVATE ATTR_NOTHROW void KCALL
Fat_FinalizeNode(struct inode *__restrict self) {
 if (self->i_fsdata) {
  kfree(self->i_fsdata->i_directory.i_freev);
  kfree(self->i_fsdata->i_clusterv);
  kfree(self->i_fsdata);
 }
}
PRIVATE ATTR_NOTHROW void KCALL
Fat16_FinalizeRootDirectoryNode(struct inode *__restrict self) {
 kfree(self->i_fsdata);
}


PRIVATE size_t KCALL
Fat32_PRead(struct inode *__restrict self,
            CHECKED USER void *buf, size_t bufsize,
            pos_t pos, iomode_t flags) {
 size_t result = bufsize;
 inode_loadattr(self);
 /* Truncate the effective read-size. */
 if (pos + result > self->i_attr.a_size)
     result = self->i_attr.a_size - pos;
 Fat32_ReadFromINode(self,(byte_t *)buf,result,pos,flags);
 return result;
}

PRIVATE size_t KCALL
Fat32_PWrite(struct inode *__restrict self,
             CHECKED USER void const *buf, size_t bufsize,
             pos_t pos, iomode_t flags) {
 pos_t new_size = pos+bufsize;
 inode_loadattr(self);
 if (new_size > self->i_attr.a_size) {
  pos_t old_size;
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
   Fat32_WriteToINode(self,(byte_t *)buf,bufsize,pos,flags);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   self->i_attr.a_size = old_size;
   error_rethrow();
  }
  inode_changed(self);
  return bufsize;
 }
do_write:
 Fat32_WriteToINode(self,(byte_t *)buf,bufsize,pos,flags);
 return bufsize;
}

DEFINE_MONTH_STARTING_DAY_OF_YEAR;

LOCAL time_t KCALL
Fat_DecodeFileDate(FileDate self) {
 time_t result; unsigned int year;
 year    = self.fd_year+1980;
 result  = YEARS2DAYS(year-LINUX_TIME_START_YEAR);
 result += MONTH_STARTING_DAY_OF_YEAR(ISLEAPYEAR(year),(self.fd_month-1) % 12);
 result += self.fd_day-1;
 return result*SECONDS_PER_DAY;
}
LOCAL FileDate KCALL
Fat_EncodeFileDate(time_t tmt) {
 FileDate result; unsigned int year;
 u8 i; time_t const *monthvec;
 tmt /= SECONDS_PER_DAY;
 tmt += YEARS2DAYS(LINUX_TIME_START_YEAR);
 year = DAYS2YEARS(tmt);
 monthvec = __time_monthstart_yday[ISLEAPYEAR(year)];
 tmt -= YEARS2DAYS(year);
 result.fd_year = year > 1980 ? year-1980 : 0;
 /* Find the appropriate month. */
 for (i = 1; i < 12; ++i) if (monthvec[i] >= tmt) break;
 result.fd_month = i;
 result.fd_day = (tmt-monthvec[i-1])+1;
 return result;
}
LOCAL time_t KCALL Fat_DecodeFileTime(FileTime self) {
 return ((time_t)self.ft_hour*60*60)+
        ((time_t)self.ft_min*60)+
        ((time_t)self.ft_sec*2);
}
LOCAL FileTime KCALL Fat_EncodeFileTime(time_t tmt) {
 FileTime result;
 result.ft_sec  = (tmt % 60)/2;
 result.ft_min  = ((tmt/60) % 60);
 result.ft_hour = ((tmt/(60*60)) % 24);
 return result;
}


PRIVATE void KCALL
Fat_LoadINodeFromFatFile(struct inode *__restrict self,
                         FatFile const *__restrict file) {
 FatNode *data = self->i_fsdata;
 /* file --> self */
 assert((data->i_clusterv != NULL) ==
        (data->i_clustera != 0));
 if (!data->i_clustera) {
  data->i_clusterv = (FatClusterIndex *)kmalloc(2*sizeof(FatClusterIndex),
                                                GFP_SHARED|GFP_NOFS);
  data->i_clustera = 2;
 }
 /* Setup the initial cluster. */
 data->i_clusterc    = 1;
 data->i_clusterv[0] = (BSWAP_LE2H16(file->f_clusterlo) |
                       (BSWAP_LE2H16(file->f_clusterhi) << 16));
 memcpy(&data->i_file,&file->f_attr,sizeof(data->i_file));
 self->i_attr.a_size  = BSWAP_LE2H32(file->f_size);
 assert(!(self->i_super->s_fsdata->f_mode & ~0777));
 self->i_attr.a_uid   = self->i_super->s_fsdata->f_uid;
 self->i_attr.a_gid   = self->i_super->s_fsdata->f_gid;
 self->i_attr.a_mode &= S_IFMT;
 self->i_attr.a_mode |= 0222 | (self->i_super->s_fsdata->f_mode & 0555);
 /* Implement the read-only attribute. */
 if (file->f_attr & ATTR_READONLY)
     self->i_attr.a_mode &= ~0222;

 /* Convert timestamps. */
 self->i_attr.a_atime.tv_sec  = Fat_DecodeFileDate(file->f_atime);
 self->i_attr.a_atime.tv_nsec = 0;
 self->i_attr.a_mtime.tv_sec  = Fat_DecodeFileDate(file->f_mtime.fc_date);
 self->i_attr.a_mtime.tv_sec += Fat_DecodeFileTime(file->f_mtime.fc_time);
 self->i_attr.a_mtime.tv_nsec = 0;
 self->i_attr.a_ctime.tv_sec  = Fat_DecodeFileDate(file->f_ctime.fc_date);
 self->i_attr.a_ctime.tv_sec += Fat_DecodeFileTime(file->f_ctime.fc_time);
 self->i_attr.a_ctime.tv_nsec = file->f_ctime.fc_sectenth * (1000000000l/200l);
}

PRIVATE void KCALL
Fat_SaveINodeToFatFile(struct inode const *__restrict self,
                       FatFile *__restrict file) {
 FatNode *data = self->i_fsdata;
 u32 cluster;
 /* self --> file */
 assert((data->i_clusterv != NULL) ==
        (data->i_clustera != 0));
 if (!data->i_clusterc) {
  cluster = self->i_super->s_fsdata->f_cluster_eof_marker;
 } else {
  cluster = data->i_clusterv[0];
 }
 /* Copy basic file data. */
 memcpy(&file->f_attr,&data->i_file,sizeof(data->i_file));
 file->f_clusterlo = BSWAP_H2LE16(cluster & 0xffff);
 file->f_clusterhi = BSWAP_H2LE16((u16)(cluster >> 16));
 file->f_size      = BSWAP_H2LE32(self->i_attr.a_size);
 /* Implement the read-only attribute. */
 if (!(self->i_attr.a_mode & 0222))
       file->f_attr |= ATTR_READONLY;
 /* Convert timestamps. */
 file->f_atime             = Fat_EncodeFileDate(self->i_attr.a_atime.tv_sec);
 file->f_mtime.fc_date     = Fat_EncodeFileDate(self->i_attr.a_mtime.tv_sec);
 file->f_mtime.fc_time     = Fat_EncodeFileTime(self->i_attr.a_mtime.tv_sec);
 file->f_ctime.fc_date     = Fat_EncodeFileDate(self->i_attr.a_ctime.tv_sec);
 file->f_ctime.fc_time     = Fat_EncodeFileTime(self->i_attr.a_ctime.tv_sec);
 file->f_ctime.fc_sectenth = (u8)(self->i_attr.a_ctime.tv_nsec / (1000000000l/200l));
 /* Set the ARCHIVE flag to indicate a file that has been modified. */
 file->f_attr |= ATTR_ARCHIVE;
}

PRIVATE void KCALL
Fat32_LoadAttr(struct inode *__restrict self) {
 byte_t buf[sizeof(FatFile)-offsetof(FatFile,f_attr)];
 /* Read the FAT-FILE from disk. */
 block_device_read(self->i_super->s_device,buf,sizeof(buf),
                  (pos_t)self->i_attr.a_ino+offsetof(FatFile,f_attr),
                   IO_RDONLY);
 /* Load Node data from the file. */
 Fat_LoadINodeFromFatFile(self,COMPILER_CONTAINER_OF((u8 *)buf,FatFile,f_attr));
}
PRIVATE void KCALL
Fat32_SaveAttr(struct inode *__restrict self) {
 byte_t buf[sizeof(FatFile)-offsetof(FatFile,f_attr)];
 /* Save Node data to a fat-file. */
 Fat_SaveINodeToFatFile(self,COMPILER_CONTAINER_OF((u8 *)buf,FatFile,f_attr));
 /* Write the FAT-FILE to disk. */
 block_device_write(self->i_super->s_device,buf,sizeof(buf),
                   (pos_t)self->i_attr.a_ino+offsetof(FatFile,f_attr),
                    IO_WRONLY);
}
PRIVATE void KCALL
Fat32_MaskAttr(struct inode *__restrict self) {
 Fat *super = self->i_super->s_fsdata;
 /* Reset owner and group. */
 self->i_attr.a_uid = super->f_uid;
 self->i_attr.a_gid = super->f_gid;
 /* Mask permissions other than write */
 self->i_attr.a_mode &= ~0555;
 self->i_attr.a_mode |= super->f_mode & 0555;
 /* Widen write permissions to all channels. */
 if (self->i_attr.a_mode & 0222)
     self->i_attr.a_mode |= 0222;
}

PRIVATE void KCALL
Fat16_RootDirectoryEntryLoadAttr(struct inode *__restrict self) {
#if 0
 FatFile buf;
 /* Read the FAT-FILE from disk. */
 block_device_read(self->i_super->s_device,&buf,sizeof(buf),
                  (pos_t)self->i_attr.a_ino/*+offsetof(FatFile,f_attr)*/);
 debug_printf("FILE: %q\n",buf.f_nameext);
 /* Load Node data from the file. */
 Fat_LoadINodeFromFatFile(self,&buf);
#else
 byte_t buf[sizeof(FatFile)-offsetof(FatFile,f_attr)];
 /* Read the FAT-FILE from disk. */
 block_device_read(self->i_super->s_device,buf,sizeof(buf),
                  (pos_t)self->i_attr.a_ino+offsetof(FatFile,f_attr),
                   IO_RDONLY);
 /* Load Node data from the file. */
 Fat_LoadINodeFromFatFile(self,COMPILER_CONTAINER_OF((u8 *)buf,FatFile,f_attr));
#endif
}
PRIVATE void KCALL
Fat16_RootDirectoryEntrySaveAttr(struct inode *__restrict self) {
 byte_t buf[sizeof(FatFile)-offsetof(FatFile,f_attr)];
 /* Save Node data to a fat-file. */
 Fat_SaveINodeToFatFile(self,COMPILER_CONTAINER_OF((u8 *)buf,FatFile,f_attr));
 /* Write the FAT-FILE to disk. */
 block_device_write(self->i_super->s_device,buf,sizeof(buf),
                   (pos_t)self->i_attr.a_ino+offsetof(FatFile,f_attr),
                    IO_WRONLY);
}

PRIVATE void KCALL
Fat_TruncateINode(struct inode *__restrict self,
                  pos_t new_smaller_size) {
 FatNode *node = self->i_fsdata;
 Fat *fat = self->i_super->s_fsdata;
 size_t new_cluster_count; FatClusterIndex delete_start;
 new_cluster_count = (size_t)CEILDIV(new_smaller_size,fat->f_clustersize);
 delete_start = Fat_GetFileCluster(self,new_cluster_count,
                                   FAT_GETCLUSTER_MODE_FNORMAL,
                                   IO_RDONLY);
 if (delete_start >= fat->f_cluster_eof)
     return; /* Already truncated. */
 node->i_clusterc = new_cluster_count;
 if (new_cluster_count != 0) {
  /* Cut off the previous cluster. */
  Fat_SetFatIndirection(self->i_super,
                        Fat_GetFileCluster(self,new_cluster_count-1,
                                           FAT_GETCLUSTER_MODE_FNORMAL,
                                           IO_RDONLY),
                        fat->f_cluster_eof_marker,
                        IO_WRONLY);
 }
 /* Now delete the chain of clusters that just got truncated. */
 Fat_DeleteClusterChain(self->i_super,delete_start,IO_RDWR);
}


PRIVATE REF struct directory_entry *KCALL
Fat_ReadDirectory(struct directory_node *__restrict self,
                  pos_t *__restrict pentry_pos,
                  iomode_t flags) {
 REF struct directory_entry *result;
 pos_t pos = *pentry_pos; FatFile file;
continue_reading:
 if (Fat_ReadFromINode((struct inode *)self,(byte_t *)&file,sizeof(FatFile),pos,flags) < sizeof(FatFile))
     error_throw(E_WOULDBLOCK);
 if (file.f_marker == MARKER_DIREND)
     return NULL; /* End of directory */
 if (file.f_marker == MARKER_UNUSED) {
  /* Keep track of free directory entries. */
  if ((pos % sizeof(FatFile)) == 0)
       FatDirectory_AddFreeFile(self,pos/sizeof(FatFile));
  goto continue_reading; /* Unused entry */
 }
 pos += sizeof(FatFile);
#if 1
#if 1
#define FILE_IS_LFN(x) \
 ((x).f_attr == ATTR_LONGFILENAME)
#else
#define FILE_IS_LFN(x) \
 ((x).f_attr == ATTR_LONGFILENAME && \
  (x).lfn_clus == (le16)0 && \
  (x).lfn_name_3[0] != (le16)0)
#endif
 if (FILE_IS_LFN(file)) {
  char lfn_name[LFN_SEQNUM_MAXCOUNT*LFN_NAME];
  u32  lfn_valid = 0;
  pos_t lfn_start = pos-sizeof(FatFile);
  do {
   char *part;
   unsigned int index = (file.lfn_seqnum & 0x1f)-1;
   lfn_valid |= 1 << index;
   part = &lfn_name[index * LFN_NAME];
   /* Copy characters from this name. */
   part[0]  = (char)((u8 *)file.lfn_name_1)[0];
   part[1]  = (char)((u8 *)file.lfn_name_1)[2];
   part[2]  = (char)((u8 *)file.lfn_name_1)[4];
   part[3]  = (char)((u8 *)file.lfn_name_1)[6];
   part[4]  = (char)((u8 *)file.lfn_name_1)[8];
   part[5]  = (char)((u8 *)file.lfn_name_2)[0];
   part[6]  = (char)((u8 *)file.lfn_name_2)[2];
   part[7]  = (char)((u8 *)file.lfn_name_2)[4];
   part[8]  = (char)((u8 *)file.lfn_name_2)[6];
   part[9]  = (char)((u8 *)file.lfn_name_2)[8];
   part[10] = (char)((u8 *)file.lfn_name_2)[10];
   part[11] = (char)((u8 *)file.lfn_name_3)[0];
   part[12] = (char)((u8 *)file.lfn_name_3)[2];
   do {
    if (Fat_ReadFromINode((struct inode *)self,(byte_t *)&file,sizeof(FatFile),pos,flags) < sizeof(FatFile))
        error_throw(E_WOULDBLOCK);
    pos += sizeof(FatFile);
   } while (file.f_marker == MARKER_UNUSED);
  } while (FILE_IS_LFN(file));
  if unlikely(lfn_valid & (lfn_valid+1)) {
   /* `lfn_valid' isn't a complete mask. (some part of the name might have been corrupted?) */
   unsigned int index = 0;
   u32 mask;
   while ((mask = 1 << index,lfn_valid >= mask)) {
    if (lfn_valid & mask) { ++index; continue; }
    memmove(lfn_name,lfn_name+LFN_NAME,
          ((LFN_SEQNUM_MAXCOUNT-1)-index)*LFN_NAME);
    lfn_valid  |= mask;
    lfn_valid >>= 1;
   }
  }

  /* Calculate the max length of the LFN filename. */
  lfn_valid = ((lfn_valid+1) >> 1) * LFN_NAME;
  for (;;) {
   if unlikely(!lfn_valid) goto dos_8dot3;
   /* Strip trailing \0 and \xff characters. */
   if ((u8)(lfn_name[lfn_valid-1]+1) > 1) break;
   --lfn_valid;
  }
  
  /* Allocate the directory entry. */
  result = (REF struct directory_entry *)kmalloc(offsetof(struct directory_entry,de_name)+
                                                (lfn_valid+1)*sizeof(char),
                                                 GFP_SHARED|GFP_NOFS);
  result->de_refcnt          = 1;
  result->de_pos             = lfn_start;
  result->de_fsdata.de_start = pos - sizeof(FatFile);
  /* Use the absolute-ondisk position of the file's FatFile as INode number. */
  result->de_ino = Fat_GetAbsDiskPos(&self->d_node,
                                      result->de_fsdata.de_start,
                                      flags);

  /* Copy the filename. */
  result->de_namelen = lfn_valid;
  memcpy(result->de_name,lfn_name,lfn_valid);
  result->de_name[lfn_valid] = '\0';
  /* Copy the dos 8.3 filename. */
  memcpy(FAT_DIRECTORY_ENTRY83(result),
         file.f_nameext,(8+3)*sizeof(char));
 } else
#endif
 {
  char *iter,*dst,*end; unsigned int i;
  char orig_name[8+3];
  char entry_name[8+1+3+1]; u16 name_length;
dos_8dot3:
  memcpy(orig_name,file.f_nameext,(8+3)*sizeof(char));
  /* Fix lowercase filenames. */
  if (file.f_ntflags & NTFLAG_LOWBASE) {
   for (i = 0; i < 8; ++i)
       file.f_name[i] = tolower(file.f_name[i]);
  }
  if (file.f_ntflags & NTFLAG_LOWEXT) {
   for (i = 0; i < 3; ++i)
       file.f_ext[i] = tolower(file.f_ext[i]);
  }
  /* Deal with this one... */
  if unlikely(file.f_marker == MARKER_IS0XE5)
     file.f_name[0] = 0xe5;
  /* Convert the filename into a regular string. */
  dst  = entry_name;
  iter = file.f_name,end = COMPILER_ENDOF(file.f_name);
  while (iter != end && isspace(*iter)) ++iter;
  dst = (char *)mempcpy(dst,iter,(size_t)(end-iter));
  while (dst != entry_name && isspace(dst[-1])) --dst;
  *dst++ = '.';
  iter = file.f_ext,end = COMPILER_ENDOF(file.f_ext);
  while (iter != end && isspace(*iter)) ++iter;
  dst = (char *)mempcpy(dst,iter,(size_t)(end-iter));
  while (dst != entry_name && isspace(dst[-1])) --dst;
  if (dst != entry_name && dst[-1] == '.') --dst;
  *dst = 0;
  name_length = (u16)(dst-entry_name);
  /* Check for entries that we're supposed to skip over. */
  if (name_length <= 2 && entry_name[0] == '.') {
   /* The kernel implements these itself, so
    * we don't actually want to emit them! */
   if (name_length == 1)
       goto continue_reading; /* Directory-self-reference. */
   if (entry_name[1] == '.')
       goto continue_reading; /* Directory-parent-reference. */
  }


  /* Create a short-directory entry. */
  result = (REF struct directory_entry *)kmalloc(offsetof(struct directory_entry,de_name)+
                                                (name_length+1)*sizeof(char),
                                                 GFP_SHARED|GFP_NOFS);
  result->de_refcnt          = 1;
  result->de_pos             = pos - sizeof(FatFile);
  result->de_fsdata.de_start = result->de_pos;
  /* Use the absolute on-disk position of the file's FatFile as INode number. */
  result->de_ino = Fat_GetAbsDiskPos(&self->d_node,
                                      result->de_pos,
                                      flags);
  /* Copy the entry name. */
  result->de_namelen = name_length;
  memcpy(result->de_name,entry_name,
        (name_length+1)*sizeof(char));
  /* Copy the unmodified, original DOS 8.3 filename. */
  memcpy(FAT_DIRECTORY_ENTRY83(result),
         orig_name,(8+3)*sizeof(char));
 }
 /* Fill in the hash field. */
 result->de_hash = directory_entry_hash(result->de_name,result->de_namelen);
 result->de_type = (file.f_attr & ATTR_DIRECTORY) ? DT_DIR : DT_REG;
 *pentry_pos = pos;
 return result;
}

PRIVATE void KCALL
Fat_AddFileToDirectory(struct directory_node *__restrict target_directory,
                       struct directory_entry *__restrict target_dirent,
                       struct inode *__restrict new_node) {
 FatFile dos83; u32 num_files; u32 file_index;
 /* Construct the 8.3 filename. */
 num_files = Fat_Create8Dot3Filename(target_directory,target_dirent,
                                     new_node,&dos83);
 /* Search for a free range of suitable size. */
 file_index = FatDirectory_GetFreeRange(target_directory,num_files);
 TRY {
  /* Write the new file entries to the directory. */
  Fat_Write8Dot3Filename(target_directory,target_dirent,
                         &dos83,file_index*sizeof(FatFile),
                         num_files,new_node);
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  /* TODO: `FatDirectory_AddFreeRange()' might override the active exception */
  FatDirectory_AddFreeRange(target_directory,file_index,num_files);
  error_rethrow();
 }
}

PRIVATE ATTR_RETNONNULL FatNode *KCALL
Fat_AllocateEmptyNode(Fat *__restrict fat) {
 FatNode *result = (FatNode *)kmalloc(sizeof(FatNode),
                                      GFP_SHARED|GFP_CALLOC|GFP_NOFS);
 result->i_clusterc = 1;
 result->i_clustera = 2;
 TRY {
  result->i_clusterv = (FatClusterIndex *)kmalloc(2*sizeof(FatClusterIndex),
                                                  GFP_SHARED|GFP_CALLOC|GFP_NOFS);
  result->i_clusterv[0] = fat->f_cluster_eof_marker;
  return result;
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  kfree(result);
  error_rethrow();
 }
 __builtin_unreachable();
}


PRIVATE void KCALL
Fat_CreateFile(struct directory_node *__restrict target_directory,
               struct directory_entry *__restrict target_dirent,
               struct regular_node *__restrict new_node) {
 new_node->re_node.i_nlink  = 1;
 new_node->re_node.i_fsdata = Fat_AllocateEmptyNode(target_directory->d_node.i_super->s_fsdata);
 new_node->re_node.i_ops    = &Fat_FileNodeOperators;
 /* Special case: FAT12/16 root directory nodes have their own operators. */
 if (target_directory->d_node.i_super->s_root == target_directory &&
     target_directory->d_node.i_super->s_fsdata->f_type != FAT32)
     new_node->re_node.i_ops = &Fat16_RootDirectoryFileEntryNodeOperators;
 Fat_AddFileToDirectory(target_directory,target_dirent,&new_node->re_node);
}

PRIVATE FatFile directory_pattern[3] = {
    [0] = { /* '.' */
        .f_name    = {'.', [1 ... 7] = ' ' },
        .f_ext     = {[0 ... 2] = ' ' },
        .f_attr    = ATTR_DIRECTORY,
        .f_ntflags = NTFLAG_NONE,
        .f_size    = 0,
    },
    [1] = { /* '..' */
        .f_name    = {'.','.', [2 ... 7] = ' ' },
        .f_ext     = {[0 ... 2] = ' ' },
        .f_attr    = ATTR_DIRECTORY,
        .f_ntflags = NTFLAG_NONE,
        .f_size    = 0,
    },
    [2] = {
        .f_marker  = MARKER_DIREND,
    },
};

PRIVATE void KCALL
Fat_CreateDirectory(struct directory_node *__restrict target_directory,
                    struct directory_entry *__restrict target_dirent,
                    struct directory_node *__restrict new_node) {
 FatFile dirinit[3]; FatClusterIndex clusno;
 new_node->d_node.i_nlink  = 1;
 new_node->d_node.i_fsdata = Fat_AllocateEmptyNode(target_directory->d_node.i_super->s_fsdata);
 new_node->d_node.i_ops    = &Fat_DirectoryNodeOperators;
 /* Special case: FAT12/16 root directory nodes have their own operators. */
 if (target_directory->d_node.i_super->s_root == target_directory &&
     target_directory->d_node.i_super->s_fsdata->f_type != FAT32)
     new_node->d_node.i_ops = &Fat16_RootDirectoryDirectoryEntryNodeOperators;
 /* Allocate the initial data block for the directory. */
 memcpy(dirinit,directory_pattern,sizeof(directory_pattern));
 /* Set the parent directory cluster number. */
 clusno = target_directory->d_node.i_fsdata->i_clusterv[0];
 dirinit[1].f_clusterlo = BSWAP_H2LE16((u16)clusno);
 dirinit[1].f_clusterhi = BSWAP_H2LE16((u16)(clusno >> 16));
 assert(new_node->d_node.i_flags & INODE_FATTRLOADED);
 /* Allocate the initial cluster for the new directory. */
 clusno = Fat_GetFileCluster(&new_node->d_node,0,
                             FAT_GETCLUSTER_MODE_FCREATE|
                             FAT_GETCLUSTER_MODE_FNOZERO,
                             IO_RDONLY);
 dirinit[0].f_clusterlo = BSWAP_H2LE16((u16)clusno);
 dirinit[0].f_clusterhi = BSWAP_H2LE16((u16)(clusno >> 16));
 TRY {
  /* Write the directory initialization template. */
  block_device_write(target_directory->d_node.i_super->s_device,dirinit,sizeof(dirinit),
                     FAT_CLUSTERADDR(target_directory->d_node.i_super->s_fsdata,clusno),
                     IO_WRONLY);
  /* Add the new directory to its parent directory. */
  Fat_AddFileToDirectory(target_directory,target_dirent,&new_node->d_node);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Delete data of the new directory. */
  Fat_SetFatIndirection(target_directory->d_node.i_super,
                        clusno,FAT_CLUSTER_UNUSED,
                        IO_WRONLY);
  error_rethrow();
 }
}



PRIVATE ssize_t KCALL
Fat_Ioctl(struct inode *__restrict self,
          unsigned long cmd,
          USER UNCHECKED void *arg,
          iomode_t UNUSED(mode)) {
 switch (cmd) {

 case FAT_IOCTL_GET_ATTRIBUTES:
  validate_writable(arg,4);
  *(u32 *)arg = self->i_fsdata->i_file.f_attr;
  break;

 {
  u32 new_flags;
 case FAT_IOCTL_SET_ATTRIBUTES:
  validate_readable(arg,4);
  new_flags = *(u32 *)arg;
  COMPILER_WRITE_BARRIER();
  if (new_flags & ATTR_DIRECTORY)
      error_throw(E_INVALID_ARGUMENT);
  self->i_fsdata->i_file.f_attr &= ~ATTR_DIRECTORY;
  self->i_fsdata->i_file.f_attr |= (u8)new_flags;
  inode_changed(self);
 } break;

 case FAT_IOCTL_GET_VOLUME_ID:
  validate_writable(arg,4);
  *(u32 *)arg = self->i_super->s_fsdata->f_volid;
  break;

#if 0 /* Would be too much work for nothing... */
 case VFAT_IOCTL_READDIR_BOTH:
  break;

 case VFAT_IOCTL_READDIR_SHORT:
  break;
#endif

 default:
  error_throw(E_NOT_IMPLEMENTED);
 }
 return 0;
}



















/* ====================================================================== */
/*   FAT INode operations / type classes.                                 */
/* ====================================================================== */
INTERN struct inode_operations const Fat_FileNodeOperators = {
    .io_fini     = &Fat_FinalizeNode,
    .io_loadattr = &Fat32_LoadAttr,
    .io_saveattr = &Fat32_SaveAttr,
    .io_maskattr = &Fat32_MaskAttr,
    .io_file = {
        .f_pread    = &Fat32_PRead,
        .f_pwrite   = &Fat32_PWrite,
        .f_truncate = &Fat_TruncateINode,
        .f_ioctl    = &Fat_Ioctl,
    },
};
INTERN struct inode_operations const Fat_DirectoryNodeOperators = {
    .io_fini     = &Fat_FinalizeNode,
    .io_loadattr = &Fat32_LoadAttr,
    .io_saveattr = &Fat32_SaveAttr,
    .io_maskattr = &Fat32_MaskAttr,
    .io_file = {
        .f_ioctl    = &Fat_Ioctl,
    },
    .io_directory = {
        .d_readdir  = &Fat_ReadDirectory,
        .d_creat    = &Fat_CreateFile,
        .d_mkdir    = &Fat_CreateDirectory,
    }
};
INTERN struct inode_operations const Fat16_RootDirectoryFileEntryNodeOperators = {
    .io_fini     = &Fat_FinalizeNode,
    .io_loadattr = &Fat16_RootDirectoryEntryLoadAttr,
    .io_saveattr = &Fat16_RootDirectoryEntrySaveAttr,
    .io_maskattr = &Fat32_MaskAttr,
    .io_file = {
        .f_pread    = &Fat32_PRead,
        .f_pwrite   = &Fat32_PWrite,
        .f_truncate = &Fat_TruncateINode,
        .f_ioctl    = &Fat_Ioctl,
    },
};
INTERN struct inode_operations const Fat16_RootDirectoryDirectoryEntryNodeOperators = {
    .io_fini     = &Fat_FinalizeNode,
    .io_loadattr = &Fat16_RootDirectoryEntryLoadAttr,
    .io_saveattr = &Fat16_RootDirectoryEntrySaveAttr,
    .io_maskattr = &Fat32_MaskAttr,
    .io_file = {
        .f_ioctl    = &Fat_Ioctl,
    },
    .io_directory = {
        .d_readdir  = &Fat_ReadDirectory,
        .d_creat    = &Fat_CreateFile,
        .d_mkdir    = &Fat_CreateDirectory,
    }
};
INTERN struct inode_operations const Fat16_RootDirectoryNodeOperators = {
    .io_fini = &Fat16_FinalizeRootDirectoryNode,
    .io_file = {
        .f_ioctl    = &Fat_Ioctl,
    },
    .io_directory = {
        .d_readdir  = &Fat_ReadDirectory,
        .d_creat    = &Fat_CreateFile,
        .d_mkdir    = &Fat_CreateDirectory,
    }
};
INTERN struct inode_operations const Fat32_RootDirectoryNodeOperators = {
    .io_fini = &Fat_FinalizeNode,
    .io_file = {
        .f_ioctl    = &Fat_Ioctl,
    },
    .io_directory = {
        .d_readdir  = &Fat_ReadDirectory,
        .d_creat    = &Fat_CreateFile,
        .d_mkdir    = &Fat_CreateDirectory,
    }
};














/* ====================================================================== */
/*   FAT class-specific table index functions                             */
/* ====================================================================== */

PRIVATE FatSectorIndex KCALL
Fat12_GetTableSector(Fat *__restrict self,
                     FatClusterIndex id) {
 return (id + (id/2)) / self->f_sectorsize;
}
PRIVATE FatSectorIndex KCALL
Fat16_GetTableSector(Fat *__restrict self,
                     FatClusterIndex id) {
 return (id * 2) / self->f_sectorsize;
}
PRIVATE FatSectorIndex KCALL
Fat32_GetTableSector(Fat *__restrict self,
                     FatClusterIndex id) {
 return (id * 4) / self->f_sectorsize;
}

PRIVATE FatClusterIndex KCALL
Fat12_GetFatIndirection(Fat *__restrict self,
                        FatClusterIndex index) {
 u16 val;
 assertf(index < self->f_fat_length,
         "Out-of-bounds FAT index: %I32u >= %I32u",
         index,self->f_fat_length);
 val = *(u16 *)((uintptr_t)self->f_fat_table+(index+(index >> 1)));
 if (index & 1)
      val >>= 4;
 else val &= 0x0fff;
 return val;
}
PRIVATE void KCALL
Fat12_SetFatIndirection(Fat *__restrict self,
                        FatClusterIndex index,
                        FatClusterIndex indirection_target) {
 u16 *pval;
 assertf(index < self->f_fat_length,
         "Out-of-bounds FAT index: %I32u >= %I32u",
         index,self->f_fat_length);
 pval = ((u16 *)((uintptr_t)self->f_fat_table+(index+(index/2))));
 if (index&1)
      *pval  = (*pval&0xf)|(indirection_target << 4);
 else *pval |= indirection_target & 0xfff;
}

PRIVATE FatClusterIndex KCALL
Fat16_GetFatIndirection(Fat *__restrict self,
                        FatClusterIndex index) {
 assertf(index < self->f_fat_length,
         "Out-of-bounds FAT index: %I32u >= %I32u",
         index,self->f_fat_length);
 return ((u16 *)self->f_fat_table)[index];
}
PRIVATE void KCALL
Fat16_SetFatIndirection(Fat *__restrict self,
                        FatClusterIndex index,
                        FatClusterIndex indirection_target) {
 assertf(index < self->f_fat_length,
         "Out-of-bounds FAT index: %I32u >= %I32u",
         index,self->f_fat_length);
 ((u16 *)self->f_fat_table)[index] = (u16)indirection_target;
}

PRIVATE FatClusterIndex KCALL
Fat32_GetFatIndirection(Fat *__restrict self,
                        FatClusterIndex index) {
 assertf(index < self->f_fat_length,
         "Out-of-bounds FAT index: %I32u >= %I32u",
         index,self->f_fat_length);
 return ((u32 *)self->f_fat_table)[index];
}
PRIVATE void KCALL
Fat32_SetFatIndirection(Fat *__restrict self,
                        FatClusterIndex index,
                        FatClusterIndex indirection_target) {
 assertf(index < self->f_fat_length,
         "Out-of-bounds FAT index: %I32u >= %I32u",
         index,self->f_fat_length);
 ((u32 *)self->f_fat_table)[index] = indirection_target;
}


INTERN FatClusterIndex KCALL
Fat_GetFatIndirection(struct superblock *__restrict self,
                      FatClusterIndex index, iomode_t flags) {
 Fat *fat = self->s_fsdata;
 FatSectorIndex table_sector;
 table_sector = (*fat->f_fat_sector)(fat,index);
 assertf(table_sector < fat->f_sec4fat,
         "Out-of-bounds FAT sector index: %I32u >= %I32u",
         table_sector,fat->f_sec4fat);
 if (!FAT_META_GTLOAD(fat,table_sector)) {
  /* Must load missing table sectors. */
  mutex_getf(&fat->f_fat_lock,flags);
  TRY {
   /* Lazily load the fat table sector that hasn't been loaded yet. */
   block_device_read(self->s_device,
                    (void *)((uintptr_t)fat->f_fat_table+(table_sector*fat->f_sectorsize)),
                     fat->f_sectorsize,
                     FAT_SECTORADDR(fat,fat->f_fat_start+table_sector),
                     flags);
  } FINALLY {
   mutex_put(&fat->f_fat_lock);
  }
  FAT_META_STLOAD(fat,table_sector);
 }
 /* Now just read the FAT entry. */
 return (*fat->f_fat_get)(fat,index);
}


INTERN void KCALL
Fat_SetFatIndirection(struct superblock *__restrict self,
                      FatClusterIndex index,
                      FatClusterIndex indirection_target,
                      iomode_t flags) {
 Fat *fat = self->s_fsdata;
 FatSectorIndex table_sector;
 assert(mutex_holding(&fat->f_fat_lock));
 table_sector = (*fat->f_fat_sector)(fat,index);
 assertf(table_sector < fat->f_sec4fat,
         "Out-of-bounds FAT sector index: %I32u >= %I32u",
         table_sector,fat->f_sec4fat);
 if (!FAT_META_GTLOAD(fat,table_sector)) {
  /* Must load missing table sectors. */
  block_device_read(self->s_device,
                   (void *)((uintptr_t)fat->f_fat_table+(table_sector*fat->f_sectorsize)),
                    fat->f_sectorsize,
                    FAT_SECTORADDR(fat,fat->f_fat_start+table_sector),
                     flags);
  FAT_META_STLOAD(fat,table_sector);
 }
 /* Now just override the FAT entry. */
 (*fat->f_fat_set)(fat,index,indirection_target);
 /* Mark the metadata associated with the sector as changed. */
 FAT_META_STCHNG(fat,table_sector);
 fat->f_flags |= FAT_FCHANGED;
}


INTERN FatClusterIndex KCALL
Fat_FindFreeCluster(struct superblock *__restrict self,
                    iomode_t flags) {
 FatClusterIndex result;
 Fat *fat = self->s_fsdata;
 assert(mutex_holding(&fat->f_fat_lock));
 result = fat->f_free_pos;
 for (; result < fat->f_cluster_eof; ++result) {
  if (Fat_GetFatIndirection(self,result,flags) == FAT_CLUSTER_UNUSED)
      goto found_one;
 }
 /* Scan everything before our previous location. */
 result = 0;
 for (; result < fat->f_free_pos; ++result) {
  if (Fat_GetFatIndirection(self,result,flags) == FAT_CLUSTER_UNUSED)
      goto found_one;
 }
 /* Disk is full... */
 throw_fs_error(ERROR_FS_DISK_FULL);

found_one:
 fat->f_free_pos = result;
 return result;
}

INTERN void KCALL
Fat_DeleteClusterChain(struct superblock *__restrict self,
                       FatClusterIndex first_delete_index,
                       iomode_t flags) {
 Fat *fat = self->s_fsdata;
 assert(mutex_holding(&fat->f_fat_lock));
 while (first_delete_index < fat->f_cluster_eof) {
  FatClusterIndex next;
  /* Read the next link. */
  next = Fat_GetFatIndirection(self,first_delete_index,flags);
  /* Mark the link as being unused now. */
  Fat_SetFatIndirection(self,first_delete_index,FAT_CLUSTER_UNUSED,flags);
  /* Continue deleting all entries from the chain. */
  first_delete_index = next;
 }
}




LOCAL void KCALL
Fat_WriteFatIndirectionTableSegment(struct superblock *__restrict self,
                                    FatSectorIndex fat_sector_index,
                                    size_t num_sectors) {
 Fat *fat = self->s_fsdata; u8 n = fat->f_fat_count;
 FatSectorIndex sector_start; void *sector_buffer; size_t sector_bytes;
 assert(mutex_holding(&fat->f_fat_lock));
 assertf(fat_sector_index+num_sectors <= fat->f_sec4fat,
         "Out-of-bounds FAT sector index: %I32u+%I32u(%I32u) > %I32u",
         fat_sector_index,num_sectors,fat_sector_index+num_sectors,fat->f_sec4fat);
 sector_buffer = (void *)((uintptr_t)fat->f_fat_table+(fat_sector_index*fat->f_sectorsize));
 sector_bytes  = (size_t)(num_sectors*fat->f_sectorsize);
 sector_start  = fat->f_fat_start+fat_sector_index;
 debug_printf("[FAT] Saving changed meta-sectors %I32u..%I32u of 0..%I32u (%I32u..%I32u)\n",
              fat_sector_index,fat_sector_index+num_sectors-1,fat->f_sec4fat-1,
              sector_start,sector_start+num_sectors-1);
 /* Write to all redundant FAT copies. */
 while (n--) {
  block_device_write(self->s_device,sector_buffer,sector_bytes,
                     FAT_SECTORADDR(fat,sector_start+n*fat->f_sec4fat),
                     IO_WRONLY);
 }
}
INTERN void KCALL
Fat_WriteFatIndirectionTable(struct superblock *__restrict self) {
 FatSectorIndex changed_begin,changed_end;
 Fat *fat = self->s_fsdata;
 if (!(fat->f_flags & FAT_FCHANGED))
       return;
 mutex_get(&fat->f_fat_lock);
 TRY {
  if (fat->f_flags & FAT_FCHANGED) {
   /* Let's do this! */
   changed_begin = 0;
   for (;;) {
    /* Search for chains for changed FAT entries and save them. */
    while (changed_begin != fat->f_sec4fat &&
          !FAT_META_GTCHNG(fat,changed_begin))
           ++changed_begin;
    changed_end = changed_begin;
    while (changed_end != fat->f_sec4fat &&
           FAT_META_GTCHNG(fat,changed_end))
           ++changed_end;
    if (changed_end == changed_begin) {
     assert(changed_begin == fat->f_sec4fat);
     assert(changed_end   == fat->f_sec4fat);
     break;
    }
    Fat_WriteFatIndirectionTableSegment(self,changed_begin,
                                       (size_t)(changed_end-changed_begin));
    /* If changes have been saved successfully, delete all the change bits. */
    while (changed_begin != changed_end) {
     FAT_META_UTCHNG(fat,changed_begin);
     ++changed_begin;
    }
   }
   fat->f_flags &= ~FAT_FCHANGED;
  }
 } FINALLY {
  mutex_put(&fat->f_fat_lock);
 }
}




/* ====================================================================== */
/*   FAT filesystem loader                                                */
/* ====================================================================== */
PRIVATE void KCALL trimspecstring(char *__restrict buf, size_t size) {
 while (size && FAT_ISSPACE(*buf)) { memmove(buf,buf+1,--size); buf[size] = '\0'; }
 while (size && FAT_ISSPACE(buf[size-1])) buf[--size] = '\0';
}

PRIVATE void KCALL
Fat_OpenSuperblock(struct superblock *__restrict self,
                   UNCHECKED USER char *args) {
 FatDiskHeader disk_header;
 struct block_device *dev = self->s_device;
 Fat *fat; FatNode *root_node;

 (void)args; /* TODO: User-arguments. */

 /* Read the FAT disk header. */
 block_device_read(dev,&disk_header,sizeof(disk_header),0,IO_RDONLY);

 /* Validate the boot signature. */
 if unlikely(disk_header.fat32.f32_bootsig[0] != 0x55 ||
             disk_header.fat32.f32_bootsig[1] != 0xAA)
    throw_fs_error(ERROR_FS_CORRUPTED_FILESYSTEM);

 /* Allocate FAT data structures. */
 fat = (Fat *)kmalloc(sizeof(Fat),GFP_SHARED);
 TRY {

  /* Check some other clear identifiers for an invalid FAT header. */
  fat->f_sectorsize  = BSWAP_LE2H16(disk_header.bpb.bpb_bytes_per_sector);
  if unlikely((fat->f_sectorsize != 512  && fat->f_sectorsize != 1024 &&
               fat->f_sectorsize != 2048 && fat->f_sectorsize != 4096) ||
              (!disk_header.bpb.bpb_sectors_per_cluster) ||
              (!disk_header.bpb.bpb_reserved_sectors) || /* What's the first sector, then? */
              (!fat->f_sectorsize % sizeof(FatFile)) != 0)
     throw_fs_error(ERROR_FS_CORRUPTED_FILESYSTEM);

  root_node = (FatNode *)kmalloc(sizeof(FatNode),GFP_SHARED|GFP_CALLOC);
  TRY {

   /* Extract some common information. */
   fat->f_fat_start   = (FatSectorIndex)BSWAP_LE2H16(disk_header.bpb.bpb_reserved_sectors);
   fat->f_sec4clus    = (size_t)disk_header.bpb.bpb_sectors_per_cluster;
   fat->f_fat_count   = (u32)disk_header.bpb.bpb_fatc;
   fat->f_clustersize = (size_t)(fat->f_sec4clus*fat->f_sectorsize);

   /* Figure out what kind of FAT filesystem this is. */
   if (!disk_header.bpb.bpb_sectors_per_fat ||
       !disk_header.bpb.bpb_maxrootsize) {
    fat->f_dat_start = fat->f_fat_start+(BSWAP_LE2H32(disk_header.fat32.f32_sectors_per_fat)*
                                         disk_header.bpb.bpb_fatc);
    fat->f_type = FAT32;
   } else {
    u32 fat_size,root_sectors;
    u32 data_sectors,total_clusters;
    root_sectors = CEILDIV(BSWAP_LE2H16(disk_header.bpb.bpb_maxrootsize)*
                           sizeof(FatFile),fat->f_sectorsize);
    fat_size = (disk_header.bpb.bpb_fatc*
                BSWAP_LE2H16(disk_header.bpb.bpb_sectors_per_fat));
    fat->f_dat_start = BSWAP_LE2H16(disk_header.bpb.bpb_reserved_sectors);
    fat->f_dat_start += fat_size;
    fat->f_dat_start += root_sectors;
    /* Calculate the total number of data sectors. */
    if (disk_header.bpb.bpb_shortsectorc) {
     data_sectors = (u32)BSWAP_LE2H16(disk_header.bpb.bpb_shortsectorc);
    } else {
     data_sectors = BSWAP_LE2H32(disk_header.bpb.bpb_longsectorc);
    }
    data_sectors -= BSWAP_LE2H16(disk_header.bpb.bpb_reserved_sectors);
    data_sectors -= fat_size;
    data_sectors -= root_sectors;
    /* Calculate the total number of data clusters. */
    total_clusters = data_sectors/disk_header.bpb.bpb_sectors_per_cluster;
         if (total_clusters > FAT16_MAXCLUSTERS) fat->f_type = FAT32;
    else if (total_clusters > FAT12_MAXCLUSTERS) fat->f_type = FAT16;
    else                                         fat->f_type = FAT12;
   }

   if (fat->f_type == FAT32) {
    if (disk_header.fat32.f32_signature != 0x28 &&
        disk_header.fat32.f32_signature != 0x29)
        throw_fs_error(ERROR_FS_CORRUPTED_FILESYSTEM);
    fat->f_volid = BSWAP_LE2H32(disk_header.fat32.f32_volid);
    memcpy(fat->f_label,disk_header.fat32.f32_label,sizeof(disk_header.fat32.f32_label));
    memcpy(fat->f_sysname,disk_header.fat32.f32_sysname,sizeof(disk_header.fat32.f32_sysname));
    fat->f_sec4fat            = BSWAP_LE2H32(disk_header.fat32.f32_sectors_per_fat);
    fat->f_cluster_eof        = (fat->f_sec4fat*fat->f_sectorsize)/4;
    fat->f_cluster_eof_marker = 0xffffffff;
    /* Must lookup the cluster of the root directory. */
    root_node->i_clusterc     = 1;
    root_node->i_clustera     = 2;
    root_node->i_clusterv     = (FatClusterIndex *)kmalloc(2*sizeof(FatClusterIndex),GFP_SHARED);
    root_node->i_clusterv[0]  = BSWAP_LE2H32(disk_header.fat32.f32_root_cluster);
    fat->f_fat_get            = &Fat32_GetFatIndirection;
    fat->f_fat_set            = &Fat32_SetFatIndirection;
    fat->f_fat_sector         = &Fat32_GetTableSector;
   } else {
    if (fat->f_type == FAT12) {
     fat->f_cluster_eof_marker = 0xfff;
     fat->f_fat_get            = &Fat12_GetFatIndirection;
     fat->f_fat_set            = &Fat12_SetFatIndirection;
     fat->f_fat_sector         = &Fat12_GetTableSector;
    } else {
     fat->f_cluster_eof_marker = 0xffff;
     fat->f_fat_get            = &Fat16_GetFatIndirection;
     fat->f_fat_set            = &Fat16_SetFatIndirection;
     fat->f_fat_sector         = &Fat16_GetTableSector;
    }
    if (disk_header.fat16.f16_signature != 0x28 &&
        disk_header.fat16.f16_signature != 0x29)
        throw_fs_error(ERROR_FS_CORRUPTED_FILESYSTEM);
    fat->f_volid = BSWAP_LE2H32(disk_header.fat16.f16_volid);
    memcpy(fat->f_label,disk_header.fat16.f16_label,sizeof(disk_header.fat16.f16_label));
    memcpy(fat->f_sysname,disk_header.fat16.f16_sysname,sizeof(disk_header.fat16.f16_sysname));
    root_node->i16_root.f16_rootpos  = BSWAP_LE2H16(disk_header.bpb.bpb_reserved_sectors);
    root_node->i16_root.f16_rootpos += (disk_header.bpb.bpb_fatc*
                                        BSWAP_LE2H16(disk_header.bpb.bpb_sectors_per_fat));
    root_node->i16_root.f16_rootpos *= fat->f_sectorsize;
    fat->f_sec4fat                   = BSWAP_LE2H16(disk_header.bpb.bpb_sectors_per_fat);
    root_node->i16_root.f16_rootsiz  = (u32)BSWAP_LE2H16(disk_header.bpb.bpb_maxrootsize);
    root_node->i16_root.f16_rootsiz *= sizeof(FatFile);
    fat->f_cluster_eof               = (fat->f_sec4fat*fat->f_sectorsize)/2;
   }
   if (fat->f_cluster_eof_marker < fat->f_cluster_eof)
       fat->f_cluster_eof_marker = fat->f_cluster_eof;

   fat->f_fat_table = NULL;
   fat->f_fat_meta  = NULL;
   COMPILER_WRITE_BARRIER();
   self->s_root->d_node.i_fsdata = root_node;
   self->s_fsdata                = fat;
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   if (fat->f_type == FAT32)
       kfree(root_node->i_clusterv);
   kfree(root_node);
   error_rethrow();
  }
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  kfree(fat);
  error_rethrow();
 }

 memcpy(&fat->f_oem,disk_header.bpb.bpb_oem,8*sizeof(char));
 fat->f_fat_size = fat->f_sec4fat*fat->f_sectorsize;
 trimspecstring(fat->f_oem,8);
 trimspecstring(fat->f_label,11);
 trimspecstring(fat->f_sysname,8);

 fat->f_uid                          = 0;
 fat->f_gid                          = 0;
 fat->f_mode                         = 0555; /* Read/Execute permissions for everyone. */
 self->s_root->d_node.i_attr.a_mode |= 0555;

 /* Setup operators */
 if (fat->f_type == FAT32) {
  self->s_root->d_node.i_ops = &Fat32_RootDirectoryNodeOperators;
 } else {
  self->s_root->d_node.i_ops = &Fat16_RootDirectoryNodeOperators;
 }
 self->s_root->d_node.i_nlink = 1;

 /* Set the no-blocks flag because otherwise we'd have to load
  * the entire CLUSTER vector of a file very time we're loading
  * its attributes, just to we could correctly fill in the
  * `a_blocks' attribute.
  * Instead, don't do that and have the core just fake that
  * attribute in stat() system calls. */
 self->s_flags |= SUPERBLOCK_FNOBLOCKS;

 /* Set the attribute-loaded flag of the root directory node. */
 self->s_root->d_node.i_flags |= INODE_FATTRLOADED;

 /* Allocate the metadata tables. */
 fat->f_free_pos  = 0;
 fat->f_fat_table = kmalloc(fat->f_fat_size,GFP_SHARED);
 fat->f_fat_meta  = (byte_t *)kmalloc(CEILDIV(fat->f_sec4fat,8/FAT_METABITS),
                                      GFP_SHARED|GFP_CALLOC);
 mutex_init(&fat->f_fat_lock);
}

PRIVATE void KCALL
Fat_FinalizeSuperblock(struct superblock *__restrict self) {
 if (!self->s_fsdata) return;
 kfree(self->s_fsdata->f_fat_table);
 kfree(self->s_fsdata->f_fat_meta);
 kfree(self->s_fsdata);
}
PRIVATE void KCALL
Fat_OpenINode(struct inode *__restrict node,
              struct directory_node *__restrict parent_directory,
              struct directory_entry *__restrict parent_directory_entry) {
 struct superblock *super;
 switch (node->i_attr.a_mode & S_IFMT) {

 case S_IFREG:
  node->i_ops = &Fat_FileNodeOperators;
  break;

 case S_IFDIR:
  node->i_ops = &Fat_DirectoryNodeOperators;
  break;

 default:
  /* Throw an unsupported-function error for any other type of node. */
  throw_fs_error(ERROR_FS_UNSUPPORTED_FUNCTION);
 }
 node->i_nlink  = 1;
 node->i_fsdata = (FatNode *)kmalloc(sizeof(FatNode),
                                     GFP_SHARED|GFP_CALLOC|GFP_NOFS);
 super = parent_directory->d_node.i_super;
 if (super->s_root == parent_directory &&
     super->s_fsdata->f_type != FAT32) {
  /* FAT-16 root directory entries have special operators. */
  if ((node->i_attr.a_mode & S_IFMT) == S_IFREG) {
   node->i_ops = &Fat16_RootDirectoryFileEntryNodeOperators;
  } else {
   node->i_ops = &Fat16_RootDirectoryDirectoryEntryNodeOperators;
  }
 }
}



PRIVATE struct superblock_type fat = {
    .st_name   = "fat",
    .st_flags  = SUPERBLOCK_TYPE_FNORMAL,
    .st_driver = &this_driver,
    .st_open   = &Fat_OpenSuperblock,
    .st_functions = {
        .f_fini     = &Fat_FinalizeSuperblock,
        .f_opennode = &Fat_OpenINode,
        .f_sync     = &Fat_WriteFatIndirectionTable
    }
};

DEFINE_FILESYSTEM_TYPE(fat);


DECL_END

#endif /* !GUARD_KERNEL_SRC_CORE_FAT_C */
