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
#ifndef GUARD_KERNEL_SRC_KERNEL_MEMINFO_C
#define GUARD_KERNEL_SRC_KERNEL_MEMINFO_C 1
#define _KOS_SOURCE 2 /* 'assertf' */

#include <hybrid/align.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <kernel/memory.h>
#include <string.h>

DECL_BEGIN

PUBLIC char const memtype_names[MEMTYPE_COUNT][8] = {
    [MEMTYPE_NDEF]      = "-",
    [MEMTYPE_PRESERVE]  = "presrv",
    [MEMTYPE_RAM]       = "ram",
    [MEMTYPE_ALLOCATED] = "alloc",
    [MEMTYPE_KFREE]     = "kfree",
    [MEMTYPE_KERNEL]    = "kernel",
    [MEMTYPE_NVS]       = "nvs",
    [MEMTYPE_DEVICE]    = "device",
    [MEMTYPE_BADRAM]    = "badram",
};

#define MEMINFO_MAXCOUNT    (MEMINSTALL_EARLY_BUFSIZE/sizeof(struct meminfo))
INTDEF struct meminfo MEMINSTALL_EARLY_BUFFER[MEMINFO_MAXCOUNT];
PUBLIC size_t _mem_info_c ASMNAME("mem_info_c") = MEMINSTALL_EARLY_PREDEFINED;
PUBLIC struct meminfo *_mem_info_v ASMNAME("mem_info_v") = MEMINSTALL_EARLY_BUFFER;
PUBLIC struct meminfo *_mem_info_last ASMNAME("mem_info_last") = MEMINSTALL_EARLY_BUFFER+(MEMINSTALL_EARLY_PREDEFINED-1);

/* During early memory initialization, we can directly address the 's_kerninfo' symbol
 * to get rid of an additional indirection that would occur if '_mem_info_v' was used. */
#define MEMINFO_V           _mem_info_v
#define MEMINFO_C           _mem_info_c
#define SET_MEMINFO_C(x)   (_mem_info_last = MEMINFO_V+(_mem_info_c = (x))-1)
#define INC_MEMINFO_C()    (++_mem_info_last,++_mem_info_c)
#define DEC_MEMINFO_C()    (--_mem_info_last,--_mem_info_c)
#define MEMINFO_HASPREV(x) ((x) != MEMINFO_V)
#define MEMINFO_HASNEXT(x) ((x) != _mem_info_last)
#define MEMINFO_PREV(x)    ((x)-1)
#define MEMINFO_NEXT(x)    ((x)+1)

#undef MEMINFO_FOREACH
#define MEMINFO_FOREACH(iter) \
 for ((iter) = MEMINFO_V; (iter) <= _mem_info_last; ++(iter))

#define MEMINFO_DELETE(info) \
 memmove((info),(info)+1,\
        ((MEMINFO_V+DEC_MEMINFO_C())-(info))*sizeof(struct meminfo))
#define MEMINFO_DUP(info) \
 (memmove((info)+1,(info),\
         ((MEMINFO_V+MEMINFO_C)-(info))*sizeof(struct meminfo)),\
  INC_MEMINFO_C())

/* Split meminfo at 'addr' and return a pointer to
 * the upper block (the one starting at 'addr') */
PRIVATE ATTR_FREETEXT
struct meminfo *KCALL meminfo_split_at(PHYS u64 addr) {
 struct meminfo *iter = MEMINFO_V;
 while (iter->mi_addr < addr) {
  if (++iter == MEMINFO_V+MEMINFO_C) {
   /* Append at the end. */
   if (MEMINFO_C == MEMINFO_MAXCOUNT)
       assertf(0,FREESTR("No more available memory information slots\n"));
   INC_MEMINFO_C();
   iter->mi_type = MEMINFO_PREV(iter)->mi_type;
   goto setup_iter;
  }
 }
 if (iter->mi_addr != addr) {
  /* Must insert a split here. */
  if (MEMINFO_C == MEMINFO_MAXCOUNT)
      assertf(0,FREESTR("No more available memory information slots\n"));
  MEMINFO_DUP(iter);
  iter->mi_type = MEMINFO_HASPREV(iter) ? MEMINFO_PREV(iter)->mi_type : MEMTYPE_NDEF;
setup_iter:
  iter->mi_addr = addr;
 }
 return iter;
}
#define meminfo_merge_with_next(info) \
        meminfo_merge_with_prev(MEMINFO_NEXT(info))
PRIVATE ATTR_FREETEXT void KCALL
meminfo_merge_with_prev(struct meminfo *__restrict info) {
 if (MEMINFO_HASPREV(info) &&
     MEMINFO_PREV(info)->mi_type == info->mi_type)
     MEMINFO_DELETE(info);
}

INTERN ATTR_FREETEXT void KCALL
mem_install(PHYS u64 base, u64 num_bytes, memtype_t type) {
 struct meminfo *start,*stop,*iter;
 u64 addr_end;
 if unlikely(!num_bytes) return;
 start = meminfo_split_at(base);
 assert(start->mi_addr == base);
 if unlikely((addr_end = base+num_bytes) <= base) {
  /* Region extends until the end of the address space. */
  stop = MEMINFO_V+MEMINFO_C;
 } else {
  stop = meminfo_split_at(base+num_bytes);
  assert(start != stop);
  assert(start->mi_addr == base);
  assert(stop->mi_addr == addr_end);
 }
 assertf(start < stop,"%p...%p\n",base,base+num_bytes-1);
 /* Go over all information slots within
  * the given range and update them. */
 for (iter = start; iter < stop; ++iter) {
  if (type < iter->mi_type)
      continue; /* `type' is less significant. */
  iter->mi_type = type;
  if (MEMINFO_HASPREV(iter) && MEMINFO_PREV(iter)->mi_type == type) { delete_iter: MEMINFO_DELETE(iter); --iter,--stop; }
  if (MEMINFO_HASNEXT(iter) && MEMINFO_NEXT(iter)->mi_type == type) { ++iter; goto delete_iter; }
 }

 /* Re-merge start and stop information slots. */
 if (stop != MEMINFO_V+MEMINFO_C)
     meminfo_merge_with_prev(stop);
 meminfo_merge_with_prev(start);
}

INTERN ATTR_FREETEXT void KCALL mem_unpreserve(void) {
 struct meminfo *iter;
 MEMINFO_FOREACH(iter) {
  u64 info_begin,info_end;
  if (iter->mi_type != MEMTYPE_ALLOCATED &&
      iter->mi_type != MEMTYPE_PRESERVE)
      continue;
  if (iter->mi_type == MEMTYPE_PRESERVE) {
   /* Free up previously preserved memory as general purpose RAM. */
   info_begin = CEIL_ALIGN(MEMINFO_BEGIN(iter),PAGESIZE);
   info_end   = FLOOR_ALIGN(MEMINFO_END(iter),PAGESIZE);
   if (info_end > info_begin) {
    info_end -= info_begin;
    page_free((pageptr_t)(info_begin/PAGESIZE),
              (size_t)((info_end-info_begin)/PAGESIZE));
   }
  }
  iter->mi_type = MEMTYPE_RAM;
  /* Check for merge with adjacent slots. */
  if (MEMINFO_HASPREV(iter) && MEMINFO_PREV(iter)->mi_type == MEMTYPE_RAM) { delete_iter: MEMINFO_DELETE(iter); --iter; }
  if (MEMINFO_HASNEXT(iter) && MEMINFO_NEXT(iter)->mi_type == MEMTYPE_RAM) { ++iter; goto delete_iter; }
 }
}

#if 0 /* TODO */
INTERN ATTR_FREETEXT
void KCALL mem_relocate_info(void) {
 struct meminfo *new_info;
 new_info = (struct meminfo *)kmalloc(MEMINFO_C*
                                      sizeof(struct meminfo),
                                      GFP_SHARED);
 if unlikely(!new_info) PANIC(FREESTR("Failed to allocate %Iu bytes for relocated meminfo"),
                              MEMINFO_C*sizeof(struct meminfo));
 memcpy(new_info,MEMINFO_V,MEMINFO_C*sizeof(struct meminfo));
 _mem_info_v    = new_info;
 _mem_info_last = new_info+MEMINFO_C-1;
}
#endif

DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_MEMINFO_C */
