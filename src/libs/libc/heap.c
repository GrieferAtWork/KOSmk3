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
#ifndef GUARD_LIBS_LIBC_HEAP_C
#define GUARD_LIBS_LIBC_HEAP_C 1
#define __EXPOSE_HEAP_INTERNALS 1
#define __OMIT_HEAP_CONSTANT_P_WRAPPERS 1
#define CONFIG_DEBUG_HEAP 1

#include "libc.h"
#include "heap.h"
#include "system.h"
#include "rtl.h"

#include <kos/heap.h>
#include <syslog.h>
#include <fcntl.h>
#include <format-printer.h>

#ifndef __INTELLISENSE__
#define OPTION_DEBUG_HEAP 1
#include "heap-impl.c.inl" /* Debug heaps. */
#include "heap-impl.c.inl" /* Regular heaps. */
#endif

#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>

DECL_BEGIN

STATIC_ASSERT_MSG(GFP_ATOMIC == O_NONBLOCK,
                  "Code is allowed to assume that these flags are identical");

INTERN ATTR_NORETURN void KCALL
libc_heap_allocation_failed(size_t n_bytes) {
 struct exception_info *info;
 /* Throw a bad-allocation error */
 info = libc_error_info();
 info->e_error.e_code = E_BADALLOC;
 info->e_error.e_flag = ERR_FNORMAL;
 libc_memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_badalloc.ba_amount   = n_bytes;
 info->e_error.e_badalloc.ba_resource = ERROR_BADALLOC_VIRTMEMORY;
 libc_error_throw_current();
 __builtin_unreachable();
}

INTERN u8 KCALL
libc_mfree_get_checksum(struct mfree *__restrict self) {
 u8 sum = 0,*iter,*end;
 end = (iter = (u8 *)&self->mf_size)+sizeof(self->mf_size);
 for (; iter != end; ++iter) sum += *iter;
 return 0 - sum;
}



INTERN struct heap *libc_debug_heaps = NULL;
INTERN DEFINE_ATOMIC_RWLOCK(libc_debug_heaps_lock);

INTERN void LIBCCALL
libc_heap_register_d(struct heap *__restrict self) {
 atomic_rwlock_write(&libc_debug_heaps_lock);
 COMPILER_READ_BARRIER();
 if (!self->h_chain.le_pself)
      LIST_INSERT(libc_debug_heaps,self,h_chain);
 atomic_rwlock_endwrite(&libc_debug_heaps_lock);
}
INTERN void LIBCCALL
libc_heap_unregister_d(struct heap *__restrict self) {
 atomic_rwlock_write(&libc_debug_heaps_lock);
 if (self->h_chain.le_pself) {
  LIST_REMOVE(self,h_chain);
  self->h_chain.le_pself = NULL;
 }
 atomic_rwlock_endwrite(&libc_debug_heaps_lock);
}



PRIVATE bool KCALL
quick_verify_mfree(struct mfree *__restrict self) {
 TRY {
  if (!IS_ALIGNED((uintptr_t)self,HEAP_ALIGNMENT)) return false;
  if (!IS_ALIGNED(self->mf_size,HEAP_ALIGNMENT)) return false;
  if (*self->mf_lsize.le_pself != self) return false;
  return true;
 } CATCH (E_SEGFAULT) {
 }
 return false;
}


INTERN void LIBCCALL
libc_heap_validate(struct heap *__restrict self) {
 unsigned int i;
 if (!(self->h_type & HEAP_TYPE_FDEBUG))
      return; /* Heap is not debug-enabled. */
 if (!atomic_rwlock_tryread(&self->h_lock))
      return;
 for (i = 0; i < COMPILER_LENOF(self->h_size); ++i) {
  struct mfree **piter,*iter;
  piter = &self->h_size[i];
  for (; (iter = *piter) != NULL; piter = &iter->mf_lsize.le_next) {
   void *faulting_address; u32 expected_data;
   assertf(IS_ALIGNED((uintptr_t)iter,HEAP_ALIGNMENT),
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Free node at %p isn't aligned by `HEAP_ALIGNMENT' (pointed to by %p)",
           piter,(uintptr_t)piter+sizeof(void *)-1,iter,piter);
   assertf(iter->mf_size >= HEAP_MINSIZE,
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Free node at %p is too small (%Iu bytes) (size bucket %Iu/%Iu)\n",
          &iter->mf_size,(uintptr_t)&iter->mf_size+sizeof(size_t)-1,
           iter,iter->mf_size,i,COMPILER_LENOF(self->h_size));
   assertf((uintptr_t)iter+iter->mf_size > (uintptr_t)iter,
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Free node at %p is too large (%Iu bytes) (size bucket %Iu/%Iu)\n",
          &iter->mf_size,(uintptr_t)&iter->mf_size+sizeof(size_t)-1,
           iter,iter->mf_size,i,COMPILER_LENOF(self->h_size));
   assertf(IS_ALIGNED(iter->mf_size,HEAP_ALIGNMENT),
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Size of free node at %p...%p (%Iu;%#Ix bytes) isn't aligned by `HEAP_ALIGNMENT'",
          &iter->mf_size,(uintptr_t)&iter->mf_size+sizeof(size_t)-1,
           MFREE_MIN(iter),MFREE_MAX(iter),iter->mf_size,iter->mf_size);
   assertf(!(iter->mf_flags&~MFREE_FMASK),
           "\tPotential USE-AFTER-FREE of <%p>\n"
           "Free node at %p...%p contains invalid flags %x",
           &iter->mf_flags,MFREE_MIN(iter),
           MFREE_MAX(iter),iter->mf_flags);
   assertf(!iter->mf_laddr.a_min || quick_verify_mfree(iter->mf_laddr.a_min),
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Free node at %p...%p has broken min-pointer to invalid node at %p",
          (uintptr_t)&iter->mf_laddr.a_min,
          (uintptr_t)&iter->mf_laddr.a_min+sizeof(void *)-1,
           MFREE_MIN(iter),MFREE_MAX(iter),iter->mf_laddr.a_min);
   assertf(!iter->mf_laddr.a_max || quick_verify_mfree(iter->mf_laddr.a_max),
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Free node at %p...%p has broken min-pointer to invalid node at %p",
          (uintptr_t)&iter->mf_laddr.a_max,
          (uintptr_t)&iter->mf_laddr.a_max+sizeof(void *)-1,
           MFREE_MIN(iter),MFREE_MAX(iter),iter->mf_laddr.a_max);
   assertf(iter->mf_lsize.le_pself == piter,
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Expected self pointer of free node %p...%p at %p, but is actually at %p",
          (uintptr_t)&iter->mf_lsize.le_pself,
          (uintptr_t)&iter->mf_lsize.le_pself+sizeof(void *)-1,
           MFREE_MIN(iter),MFREE_MAX(iter),piter,iter->mf_lsize.le_pself);
   assertf(iter->mf_szchk == libc_mfree_get_checksum(iter),
           "\tPotential USE-AFTER-FREE of <%p...%p> of %p\n"
           "Invalid checksum in free node %p...%p (expected %#.2I8x, but got %#.2I8x)",
          (uintptr_t)&iter->mf_size,(uintptr_t)(&iter->mf_size+1)-1,
          (uintptr_t)&iter->mf_szchk,
           MFREE_MIN(iter),MFREE_MAX(iter),
           libc_mfree_get_checksum(iter),iter->mf_szchk);
   /* Verify memory of this node. */
   expected_data    = iter->mf_flags&MFREE_FZERO ? 0 : DEBUGHEAP_NO_MANS_LAND;
   faulting_address = sys_xfind_modified_address(iter->mf_data,expected_data,
                                                (iter->mf_size-SIZEOF_MFREE)/4);
   if unlikely(faulting_address) {
    u8 *fault_start = (u8 *)faulting_address - 32;
    u8 *page_base = (u8 *)FLOOR_ALIGN((uintptr_t)faulting_address,PAGESIZE);
    if (fault_start < page_base) fault_start = page_base;
    if (fault_start < (u8 *)iter->mf_data)
        fault_start = (u8 *)iter->mf_data;
    libc_syslog(LOG_ERROR,"\n\n\n");
    libc_format_hexdump(&libc_syslog_printer,SYSLOG_PRINTER_CLOSURE(LOG_ERROR),
                        fault_start,16+2*((u8 *)faulting_address-fault_start),
                        16,FORMAT_HEXDUMP_FLAG_ADDRESS);
    libc_syslog(LOG_ERROR,"\n");
    assertf(0,
            "\tIllegal USE-AFTER-FREE of <%p>\n"
            "Free node:     %p...%p\n"
            "Node offset:   %Iu (%#Ix)\n"
            "Expected byte: %.2I8x\n"
            "Found byte:    %.2I8x",
            faulting_address,
            MFREE_MIN(iter),MFREE_MAX(iter),
           (uintptr_t)faulting_address-MFREE_MIN(iter),
           (uintptr_t)faulting_address-MFREE_MIN(iter),
           ((u8 *)&expected_data)[(uintptr_t)faulting_address & 3],
           *(u8 *)faulting_address);
   }
  }
 }
 atomic_rwlock_endread(&self->h_lock);
}


INTERN void LIBCCALL
libc_heap_validate_all(void) {
 struct heap *iter;
 atomic_rwlock_read(&libc_debug_heaps_lock);
 LIST_FOREACH(iter,libc_debug_heaps,h_chain) {
  libc_heap_validate(iter);
 }
 atomic_rwlock_endread(&libc_debug_heaps_lock);
}

DECL_END

#endif /* !GUARD_LIBS_LIBC_HEAP_C */
