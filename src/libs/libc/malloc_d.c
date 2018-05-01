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
#ifndef GUARD_LIBS_LIBC_MALLOC_D_C
#define GUARD_LIBS_LIBC_MALLOC_D_C 1
#define __EXPOSE_HEAP_INTERNALS 1
#define CONFIG_DEBUG_HEAP 1
#define local_heap_d   heap

#include "libc.h"
#include "malloc.h"
#include "heap.h"
#include "errno.h"

#include <hybrid/list/atree.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kos/heap.h>
#include <errno.h>
#include <except.h>
#include <malloc.h>

DECL_BEGIN

/* TODO: Use a node-based system, similar to what `mall' does in the kernel. */

PRIVATE struct heap mheap = HEAP_INIT(PAGESIZE*16,PAGESIZE*16);

struct mptr {
    union {
        size_t m_size; /* Allocated pointer size (including this header). */
        byte_t m_pad[HEAP_ALIGNMENT]; /* ... */
    };
};

/* Using our awesome heap API, it's child's
 * play to implement a basic malloc() function. */
PRIVATE void *LIBCCALL malloc_failed(void) {
 libc_seterrno(ENOMEM);
 return NULL;
}
PRIVATE ATTR_NORETURN void LIBCCALL Xmalloc_failed(size_t num_bytes) {
 struct exception_info *info = libc_error_info();
 libc_memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_code                 = E_BADALLOC;
 info->e_error.e_flag                 = ERR_FNORMAL;
 info->e_error.e_badalloc.ba_resource = ERROR_BADALLOC_VIRTMEMORY;
 info->e_error.e_badalloc.ba_amount   = num_bytes;
 libc_error_throw_current();
 __builtin_unreachable();
}

INTERN ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL
libc_Xmalloc_d(size_t num_bytes) {
 struct mptr *result;
 struct heapptr ptr;
 size_t total;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     Xmalloc_failed(num_bytes);
 ptr = libc_heap_alloc_d(&mheap,total,0);
 result = (struct mptr *)ptr.hp_ptr;
 result->m_size = ptr.hp_siz;
 return result+1;
}

INTERN ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL
libc_Xcalloc_d(size_t count, size_t num_bytes) {
 struct mptr *result;
 struct heapptr ptr;
 size_t total;
 if (__builtin_mul_overflow(count,num_bytes,&total))
     Xmalloc_failed(num_bytes);
 if (__builtin_add_overflow(total,sizeof(struct mptr),&total))
     Xmalloc_failed(total);
 ptr = libc_heap_alloc_d(&mheap,total,GFP_CALLOC);
 result = (struct mptr *)ptr.hp_ptr;
 result->m_size = ptr.hp_siz;
 return result+1;
}

INTERN ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL
libc_Xmemalign_d(size_t min_alignment, size_t num_bytes) {
 struct mptr *result;
 struct heapptr ptr;
 size_t total;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     Xmalloc_failed(num_bytes);
 ptr = libc_heap_align_d(&mheap,min_alignment,sizeof(struct mptr),total,0);
 result = (struct mptr *)ptr.hp_ptr;
 result->m_size = ptr.hp_siz;
 return result+1;
}

INTERN ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL
libc_Xmemalign_offset_d(size_t min_alignment, size_t num_bytes, ptrdiff_t offset) {
 struct mptr *result;
 struct heapptr ptr;
 size_t total;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     Xmalloc_failed(num_bytes);
 ptr = libc_heap_align_d(&mheap,min_alignment,offset+sizeof(struct mptr),total,0);
 result = (struct mptr *)ptr.hp_ptr;
 result->m_size = ptr.hp_siz;
 return result+1;
}

INTERN ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL
libc_Xmemcalign_d(size_t min_alignment, size_t num_bytes) {
 struct mptr *result;
 struct heapptr ptr;
 size_t total;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     Xmalloc_failed(num_bytes);
 ptr = libc_heap_align_d(&mheap,min_alignment,sizeof(struct mptr),total,GFP_CALLOC);
 result = (struct mptr *)ptr.hp_ptr;
 result->m_size = ptr.hp_siz;
 return result+1;
}

INTERN ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL
libc_Xmemcalign_offset_d(size_t min_alignment, size_t num_bytes, ptrdiff_t offset) {
 struct mptr *result;
 struct heapptr ptr;
 size_t total;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     Xmalloc_failed(num_bytes);
 ptr = libc_heap_align_d(&mheap,min_alignment,offset+sizeof(struct mptr),total,GFP_CALLOC);
 result = (struct mptr *)ptr.hp_ptr;
 result->m_size = ptr.hp_siz;
 return result+1;
}

INTERN ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL
libc_Xvalloc_d(size_t num_bytes) {
 return libc_Xmemalign_d(PAGESIZE,num_bytes);
}

INTERN ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL
libc_Xpvalloc_d(size_t num_bytes) {
 return libc_Xmemalign_d(PAGESIZE,CEIL_ALIGN(num_bytes,PAGESIZE));
}

INTERN size_t LIBCCALL
libc_malloc_usable_size_d(void *__restrict ptr) {
 return ptr ? ((struct mptr *)ptr)[-1].m_size : 0;
}

INTERN ATTR_RETNONNULL void *LIBCCALL
libc_Xrealloc_d(void *ptr, size_t num_bytes) {
 struct heapptr newptr;
 struct mptr *result;
 size_t total;
 if (!ptr) return libc_Xmalloc_d(num_bytes);
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     Xmalloc_failed(num_bytes);
 result = ((struct mptr *)ptr)-1;
 newptr = libc_heap_realloc_d(&mheap,result,result->m_size,total,0,0);
 result = (struct mptr *)newptr.hp_ptr;
 result->m_size = newptr.hp_siz;
 return result+1;
}

INTERN ATTR_RETNONNULL void *LIBCCALL
libc_Xrecalloc_d(void *ptr, size_t num_bytes) {
 struct heapptr newptr;
 struct mptr *result;
 size_t total;
 if (!ptr) return libc_Xcalloc_d(1,num_bytes);
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     Xmalloc_failed(num_bytes);
 result = ((struct mptr *)ptr)-1;
 newptr = libc_heap_realloc_d(&mheap,result,result->m_size,total,GFP_CALLOC,0);
 result = (struct mptr *)newptr.hp_ptr;
 result->m_size = newptr.hp_siz;
 return result+1;
}

INTERN ATTR_RETNONNULL void *LIBCCALL
libc_Xrealign_d(void *ptr, size_t min_alignment, size_t num_bytes) {
 struct heapptr newptr;
 struct mptr *result;
 size_t total;
 if (!ptr) return libc_Xmemalign_d(min_alignment,num_bytes);
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     Xmalloc_failed(num_bytes);
 result = ((struct mptr *)ptr)-1;
 newptr = libc_heap_realign_d(&mheap,result,result->m_size,min_alignment,
                             sizeof(struct mptr),total,0,0);
 result = (struct mptr *)newptr.hp_ptr;
 result->m_size = newptr.hp_siz;
 return result+1;
}

INTERN ATTR_RETNONNULL void *LIBCCALL
libc_Xrecalign_d(void *ptr, size_t min_alignment, size_t num_bytes) {
 struct heapptr newptr;
 struct mptr *result;
 size_t total;
 if (!ptr) return libc_Xmemcalign_d(min_alignment,num_bytes);
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     Xmalloc_failed(num_bytes);
 result = ((struct mptr *)ptr)-1;
 newptr = libc_heap_realign_d(&mheap,result,result->m_size,min_alignment,
                             sizeof(struct mptr),total,GFP_CALLOC,0);
 result = (struct mptr *)newptr.hp_ptr;
 result->m_size = newptr.hp_siz;
 return result+1;
}

INTERN ATTR_RETNONNULL void *LIBCCALL
libc_Xrealign_offset_d(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset) {
 struct heapptr newptr;
 struct mptr *result;
 size_t total;
 if (!ptr) return libc_Xmemalign_offset_d(min_alignment,num_bytes,offset);
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     Xmalloc_failed(num_bytes);
 result = ((struct mptr *)ptr)-1;
 newptr = libc_heap_realign_d(&mheap,result,result->m_size,min_alignment,
                             offset+sizeof(struct mptr),total,0,0);
 result = (struct mptr *)newptr.hp_ptr;
 result->m_size = newptr.hp_siz;
 return result+1;
}

INTERN ATTR_RETNONNULL void *LIBCCALL
libc_Xrecalign_offset_d(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset) {
 struct heapptr newptr;
 struct mptr *result;
 size_t total;
 if (!ptr) return libc_Xmemcalign_offset_d(min_alignment,num_bytes,offset);
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     Xmalloc_failed(num_bytes);
 result = ((struct mptr *)ptr)-1;
 newptr = libc_heap_realign_d(&mheap,result,result->m_size,min_alignment,
                             offset+sizeof(struct mptr),total,GFP_CALLOC,0);
 result = (struct mptr *)newptr.hp_ptr;
 result->m_size = newptr.hp_siz;
 return result+1;
}

INTERN void *LIBCCALL
libc_Xrealloc_in_place_d(void *ptr, size_t num_bytes) {
 struct mptr *result;
 size_t total;
 if (!ptr) return NULL;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     Xmalloc_failed(num_bytes);
 result = ((struct mptr *)ptr)-1;
 total = CEIL_ALIGN(total,HEAP_ALIGNMENT);
compare:
 if (total > result->m_size) {
  size_t more_bytes;
  /* Try to allocate more memory. */
  more_bytes = libc_heap_allat_d(&mheap,(void *)((uintptr_t)result+result->m_size),
                                total-result->m_size,0);
  if (!more_bytes)
       return NULL; /* Memory was already in use. */
  /* Extend the data block. */
  result->m_size += more_bytes;
  return result+1;
 } else if (total < result->m_size) {
  if unlikely(!total) { total = HEAP_ALIGNMENT; goto compare; }
  /* Free unused memory. */
  libc_heap_free_d(&mheap,(void *)((uintptr_t)result+total),
                  result->m_size-total,0);
  /* Update the data block size. */
  result->m_size = total;
 }
 return ptr;
}

INTERN ATTR_RETNONNULL void *LIBCCALL
libc_Xrecalloc_in_place_d(void *ptr, size_t num_bytes) {
 struct mptr *result;
 size_t total;
 if (!ptr) return NULL;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     Xmalloc_failed(num_bytes);
 result = ((struct mptr *)ptr)-1;
 total = CEIL_ALIGN(total,HEAP_ALIGNMENT);
compare:
 if (total > result->m_size) {
  size_t more_bytes;
  /* Try to allocate more memory. */
  more_bytes = libc_heap_allat_d(&mheap,(void *)((uintptr_t)result+result->m_size),
                                total-result->m_size,GFP_CALLOC);
  if (!more_bytes)
       return NULL; /* Memory was already in use. */
  /* Extend the data block. */
  result->m_size += more_bytes;
  return result+1;
 } else if (total < result->m_size) {
  if unlikely(!total) { total = HEAP_ALIGNMENT; goto compare; }
  /* Free unused memory. */
  libc_heap_free_d(&mheap,(void *)((uintptr_t)result+total),
                  result->m_size-total,0);
  /* Update the data block size. */
  result->m_size = total;
 }
 return ptr;
}

INTERN void LIBCCALL libc_free_d(void *ptr) {
 struct mptr *mp;
 if (!ptr) return;
 mp = (struct mptr *)ptr-1;
 libc_heap_free_d(&mheap,mp,mp->m_size,0);
}


INTERN ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL
libc_Xmemdup_d(void const *__restrict ptr, size_t n_bytes) {
 void *result = libc_Xmalloc_d(n_bytes);
 if (result) libc_memcpy(result,ptr,n_bytes);
 return result;
}
INTERN ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL
libc_Xmemcdup_d(void const *__restrict ptr, int needle, size_t n_bytes) {
 if (n_bytes) {
  void const *endaddr = libc_memchr(ptr,needle,n_bytes-1);
  if (endaddr) n_bytes = ((uintptr_t)endaddr-(uintptr_t)ptr)+1;
 }
 return libc_Xmemdup_d(ptr,n_bytes);
}

INTERN ATTR_RETNONNULL ATTR_MALLOC char *LIBCCALL
libc_Xstrdup_d(char const *__restrict str) {
 size_t len = (libc_strlen(str)+1)*sizeof(char);
 char *result = (char *)libc_Xmalloc_d(len);
 if (result) libc_memcpy(result,str,len);
 return result;
}
INTERN ATTR_RETNONNULL ATTR_MALLOC char *LIBCCALL
libc_Xstrndup_d(char const *__restrict str, size_t max_chars) {
 char *result;
 max_chars = libc_strnlen(str,max_chars);
 result = (char *)libc_Xmalloc_d((max_chars+1)*sizeof(char));
 if (result) {
  libc_memcpy(result,str,max_chars*sizeof(char));
  result[max_chars] = '\0';
 }
 return result;
}

INTERN ATTR_RETNONNULL ATTR_MALLOC char16_t *LIBCCALL
libc_Xw16dup_d(char16_t const *__restrict str) {
 size_t len = (libc_w16len(str)+1)*sizeof(char16_t);
 char16_t *result = (char16_t *)libc_Xmalloc_d(len);
 if (result) libc_memcpy(result,str,len);
 return result;
}
INTERN ATTR_RETNONNULL ATTR_MALLOC char32_t *LIBCCALL
libc_Xw32dup_d(char32_t const *__restrict str) {
 size_t len = (libc_w32len(str)+1)*sizeof(char32_t);
 char32_t *result = (char32_t *)libc_Xmalloc_d(len);
 if (result) libc_memcpy(result,str,len);
 return result;
}
INTERN ATTR_RETNONNULL ATTR_MALLOC char16_t *LIBCCALL
libc_Xw16ndup_d(char16_t const *__restrict str, size_t max_chars) {
 char16_t *result;
 max_chars = libc_w16nlen(str,max_chars);
 result = (char16_t *)libc_Xmalloc_d((max_chars+1)*sizeof(char16_t));
 if (result) {
  libc_memcpy(result,str,max_chars*sizeof(char16_t));
  result[max_chars] = '\0';
 }
 return result;
}
INTERN ATTR_RETNONNULL ATTR_MALLOC char32_t *LIBCCALL
libc_Xw32ndup_d(char32_t const *__restrict str, size_t max_chars) {
 char32_t *result;
 max_chars = libc_w32nlen(str,max_chars);
 result = (char32_t *)libc_Xmalloc_d((max_chars+1)*sizeof(char32_t));
 if (result) {
  libc_memcpy(result,str,max_chars*sizeof(char32_t));
  result[max_chars] = '\0';
 }
 return result;
}


INTERN ATTR_MALLOC void *LIBCCALL
libc_malloc_d(size_t num_bytes) {
 struct mptr *result;
 struct heapptr ptr;
 size_t total;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     return malloc_failed();
 TRY {
  ptr = libc_heap_alloc_d(&mheap,total,0);
 } CATCH_HANDLED (E_BADALLOC) {
  return malloc_failed();
 }
 result = (struct mptr *)ptr.hp_ptr;
 result->m_size = ptr.hp_siz;
 return result+1;
}

INTERN ATTR_MALLOC void *LIBCCALL
libc_calloc_d(size_t count, size_t num_bytes) {
 struct mptr *result;
 struct heapptr ptr;
 size_t total;
 if (__builtin_mul_overflow(count,num_bytes,&total))
     return malloc_failed();
 if (__builtin_add_overflow(total,sizeof(struct mptr),&total))
     return malloc_failed();
 TRY {
  ptr = libc_heap_alloc_d(&mheap,total,GFP_CALLOC);
 } CATCH_HANDLED (E_BADALLOC) {
  return malloc_failed();
 }
 result = (struct mptr *)ptr.hp_ptr;
 result->m_size = ptr.hp_siz;
 return result+1;
}

INTERN ATTR_MALLOC void *LIBCCALL
libc_memalign_d(size_t min_alignment, size_t num_bytes) {
 struct mptr *result;
 struct heapptr ptr;
 size_t total;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     return malloc_failed();
 TRY {
  ptr = libc_heap_align_d(&mheap,min_alignment,sizeof(struct mptr),total,0);
 } CATCH_HANDLED (E_BADALLOC) {
  return malloc_failed();
 }
 result = (struct mptr *)ptr.hp_ptr;
 result->m_size = ptr.hp_siz;
 return result+1;
}

INTERN ATTR_MALLOC void *LIBCCALL
libc_memalign_offset_d(size_t min_alignment, size_t num_bytes, ptrdiff_t offset) {
 struct mptr *result;
 struct heapptr ptr;
 size_t total;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     return malloc_failed();
 TRY {
  ptr = libc_heap_align_d(&mheap,min_alignment,offset+sizeof(struct mptr),total,0);
 } CATCH_HANDLED (E_BADALLOC) {
  return malloc_failed();
 }
 result = (struct mptr *)ptr.hp_ptr;
 result->m_size = ptr.hp_siz;
 return result+1;
}

INTERN ATTR_MALLOC void *LIBCCALL
libc_memcalign_d(size_t min_alignment, size_t num_bytes) {
 struct mptr *result;
 struct heapptr ptr;
 size_t total;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     return malloc_failed();
 TRY {
  ptr = libc_heap_align_d(&mheap,min_alignment,sizeof(struct mptr),total,GFP_CALLOC);
 } CATCH_HANDLED (E_BADALLOC) {
  return malloc_failed();
 }
 result = (struct mptr *)ptr.hp_ptr;
 result->m_size = ptr.hp_siz;
 return result+1;
}

INTERN ATTR_MALLOC void *LIBCCALL
libc_memcalign_offset_d(size_t min_alignment, size_t num_bytes, ptrdiff_t offset) {
 struct mptr *result;
 struct heapptr ptr;
 size_t total;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     return malloc_failed();
 TRY {
  ptr = libc_heap_align_d(&mheap,min_alignment,offset+sizeof(struct mptr),total,GFP_CALLOC);
 } CATCH_HANDLED (E_BADALLOC) {
  return malloc_failed();
 }
 result = (struct mptr *)ptr.hp_ptr;
 result->m_size = ptr.hp_siz;
 return result+1;
}

INTERN ATTR_MALLOC void *LIBCCALL
libc_valloc_d(size_t num_bytes) {
 return libc_memalign_d(PAGESIZE,num_bytes);
}

INTERN ATTR_MALLOC void *LIBCCALL
libc_pvalloc_d(size_t num_bytes) {
 return libc_memalign_d(PAGESIZE,CEIL_ALIGN(num_bytes,PAGESIZE));
}


INTERN void *LIBCCALL
libc_realloc_d(void *ptr, size_t num_bytes) {
 struct heapptr newptr;
 struct mptr *result;
 size_t total;
 if (!ptr) return libc_malloc_d(num_bytes);
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     return malloc_failed();
 result = ((struct mptr *)ptr)-1;
 TRY {
  newptr = libc_heap_realloc_d(&mheap,result,result->m_size,total,0,0);
 } CATCH_HANDLED (E_BADALLOC) {
  return malloc_failed();
 }
 result = (struct mptr *)newptr.hp_ptr;
 result->m_size = newptr.hp_siz;
 return result+1;
}

INTERN void *LIBCCALL
libc_recalloc_d(void *ptr, size_t num_bytes) {
 struct heapptr newptr;
 struct mptr *result;
 size_t total;
 if (!ptr) return libc_calloc_d(1,num_bytes);
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     return malloc_failed();
 result = ((struct mptr *)ptr)-1;
 TRY {
  newptr = libc_heap_realloc_d(&mheap,result,result->m_size,total,GFP_CALLOC,0);
 } CATCH_HANDLED (E_BADALLOC) {
  return malloc_failed();
 }
 result = (struct mptr *)newptr.hp_ptr;
 result->m_size = newptr.hp_siz;
 return result+1;
}

INTERN void *LIBCCALL
libc_realign_d(void *ptr, size_t min_alignment, size_t num_bytes) {
 struct heapptr newptr;
 struct mptr *result;
 size_t total;
 if (!ptr) return libc_memalign_d(min_alignment,num_bytes);
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     return malloc_failed();
 result = ((struct mptr *)ptr)-1;
 TRY {
  newptr = libc_heap_realign_d(&mheap,result,result->m_size,min_alignment,
                              sizeof(struct mptr),total,0,0);
 } CATCH_HANDLED (E_BADALLOC) {
  return malloc_failed();
 }
 result = (struct mptr *)newptr.hp_ptr;
 result->m_size = newptr.hp_siz;
 return result+1;
}

INTERN void *LIBCCALL
libc_recalign_d(void *ptr, size_t min_alignment, size_t num_bytes) {
 struct heapptr newptr;
 struct mptr *result;
 size_t total;
 if (!ptr) return libc_memcalign_d(min_alignment,num_bytes);
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     return malloc_failed();
 result = ((struct mptr *)ptr)-1;
 TRY {
  newptr = libc_heap_realign_d(&mheap,result,result->m_size,min_alignment,
                              sizeof(struct mptr),total,GFP_CALLOC,0);
 } CATCH_HANDLED (E_BADALLOC) {
  return malloc_failed();
 }
 result = (struct mptr *)newptr.hp_ptr;
 result->m_size = newptr.hp_siz;
 return result+1;
}

INTERN void *LIBCCALL
libc_realign_offset_d(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset) {
 struct heapptr newptr;
 struct mptr *result;
 size_t total;
 if (!ptr) return libc_memalign_offset_d(min_alignment,num_bytes,offset);
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     return malloc_failed();
 result = ((struct mptr *)ptr)-1;
 TRY {
  newptr = libc_heap_realign_d(&mheap,result,result->m_size,min_alignment,
                              offset+sizeof(struct mptr),total,0,0);
 } CATCH_HANDLED (E_BADALLOC) {
  return malloc_failed();
 }
 result = (struct mptr *)newptr.hp_ptr;
 result->m_size = newptr.hp_siz;
 return result+1;
}

INTERN void *LIBCCALL
libc_recalign_offset_d(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset) {
 struct heapptr newptr;
 struct mptr *result;
 size_t total;
 if (!ptr) return libc_memcalign_offset_d(min_alignment,num_bytes,offset);
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     return malloc_failed();
 result = ((struct mptr *)ptr)-1;
 TRY {
  newptr = libc_heap_realign_d(&mheap,result,result->m_size,min_alignment,
                              offset+sizeof(struct mptr),total,GFP_CALLOC,0);
 } CATCH_HANDLED (E_BADALLOC) {
  return malloc_failed();
 }
 result = (struct mptr *)newptr.hp_ptr;
 result->m_size = newptr.hp_siz;
 return result+1;
}

INTERN void *LIBCCALL
libc_realloc_in_place_d(void *ptr, size_t num_bytes) {
 struct mptr *result;
 size_t total;
 if (!ptr) return NULL;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     return malloc_failed();
 result = ((struct mptr *)ptr)-1;
 total = CEIL_ALIGN(total,HEAP_ALIGNMENT);
compare:
 if (total > result->m_size) {
  size_t COMPILER_IGNORE_UNINITIALIZED(more_bytes);
  /* Try to allocate more memory. */
  TRY {
   more_bytes = libc_heap_allat_d(&mheap,(void *)((uintptr_t)result+result->m_size),
                                 total-result->m_size,0);
  } CATCH_HANDLED (E_BADALLOC) {
   return malloc_failed();
  }
  if (!more_bytes)
       return NULL; /* Memory was already in use. */
  /* Extend the data block. */
  result->m_size += more_bytes;
  return result+1;
 } else if (total < result->m_size) {
  if unlikely(!total) { total = HEAP_ALIGNMENT; goto compare; }
  /* Free unused memory. */
  libc_heap_free_d(&mheap,(void *)((uintptr_t)result+total),
                  result->m_size-total,0);
  /* Update the data block size. */
  result->m_size = total;
 }
 return ptr;
}

INTERN void *LIBCCALL
libc_recalloc_in_place_d(void *ptr, size_t num_bytes) {
 struct mptr *result;
 size_t total;
 if (!ptr) return NULL;
 if (__builtin_add_overflow(num_bytes,sizeof(struct mptr),&total))
     return malloc_failed();
 result = ((struct mptr *)ptr)-1;
 total = CEIL_ALIGN(total,HEAP_ALIGNMENT);
compare:
 if (total > result->m_size) {
  size_t COMPILER_IGNORE_UNINITIALIZED(more_bytes);
  /* Try to allocate more memory. */
  TRY {
   more_bytes = libc_heap_allat_d(&mheap,(void *)((uintptr_t)result+result->m_size),
                                 total-result->m_size,GFP_CALLOC);
  } CATCH_HANDLED (E_BADALLOC) {
   return malloc_failed();
  }
  if (!more_bytes)
       return NULL; /* Memory was already in use. */
  /* Extend the data block. */
  result->m_size += more_bytes;
  return result+1;
 } else if (total < result->m_size) {
  if unlikely(!total) { total = HEAP_ALIGNMENT; goto compare; }
  /* Free unused memory. */
  libc_heap_free_d(&mheap,(void *)((uintptr_t)result+total),
                  result->m_size-total,0);
  /* Update the data block size. */
  result->m_size = total;
 }
 return ptr;
}

INTERN int LIBCCALL
libc_posix_memalign_d(void **__restrict pp,
                      size_t alignment,
                      size_t n_bytes) {
 void *result = NULL;
 size_t d = alignment / sizeof(void *);
 size_t r = alignment % sizeof(void *);
 if (r != 0 || !d || (d&(d-1)) != 0)
     return EINVAL;
 result = libc_memalign_d(alignment,n_bytes);
 if (!result) return ENOMEM;
 *pp = result;
 return 0;
}

INTERN int LIBCCALL
libc_mallopt_d(int parameter_number,
               int parameter_value) {
 switch (parameter_number) {

 case M_TRIM_THRESHOLD:
  if (parameter_value < PAGESIZE)
      return 0;
  mheap.h_freethresh = (size_t)parameter_value;
  return 1;

 case M_GRANULARITY:
  if (parameter_value < PAGESIZE ||
    ((parameter_value & (parameter_value-1)) != 0))
      return 0;
  mheap.h_overalloc = (size_t)parameter_value;
  return 1;

 default: break;
 }
 return 0;
}

INTERN int LIBCCALL
libc_malloc_trim_d(size_t pad) {
 return libc_heap_trim_d(&mheap,pad) != 0;
}


INTERN ATTR_MALLOC void *LIBCCALL
libc_memdup_d(void const *__restrict ptr, size_t n_bytes) {
 void *result = libc_malloc_d(n_bytes);
 if (result) libc_memcpy(result,ptr,n_bytes);
 return result;
}
INTERN ATTR_MALLOC void *LIBCCALL
libc_memcdup_d(void const *__restrict ptr, int needle, size_t n_bytes) {
 if (n_bytes) {
  void const *endaddr = libc_memchr(ptr,needle,n_bytes-1);
  if (endaddr) n_bytes = ((uintptr_t)endaddr-(uintptr_t)ptr)+1;
 }
 return libc_memdup_d(ptr,n_bytes);
}
INTERN ATTR_MALLOC char *LIBCCALL
libc_strdup_d(char const *__restrict str) {
 size_t len = (libc_strlen(str)+1)*sizeof(char);
 char *result = (char *)libc_malloc_d(len);
 if (result) libc_memcpy(result,str,len);
 return result;
}
INTERN ATTR_MALLOC char *LIBCCALL
libc_strndup_d(char const *__restrict str, size_t max_chars) {
 char *result;
 max_chars = libc_strnlen(str,max_chars);
 result = (char *)libc_malloc_d((max_chars+1)*sizeof(char));
 if (result) {
  libc_memcpy(result,str,max_chars*sizeof(char));
  result[max_chars] = '\0';
 }
 return result;
}
INTERN ATTR_MALLOC char16_t *LIBCCALL
libc_w16dup_d(char16_t const *__restrict str) {
 size_t len = (libc_w16len(str)+1)*sizeof(char16_t);
 char16_t *result = (char16_t *)libc_malloc_d(len);
 if (result) libc_memcpy(result,str,len);
 return result;
}
INTERN ATTR_MALLOC char32_t *LIBCCALL
libc_w32dup_d(char32_t const *__restrict str) {
 size_t len = (libc_w32len(str)+1)*sizeof(char32_t);
 char32_t *result = (char32_t *)libc_malloc_d(len);
 if (result) libc_memcpy(result,str,len);
 return result;
}
INTERN ATTR_MALLOC char16_t *LIBCCALL
libc_w16ndup_d(char16_t const *__restrict str, size_t max_chars) {
 char16_t *result;
 max_chars = libc_w16nlen(str,max_chars);
 result = (char16_t *)libc_malloc_d((max_chars+1)*sizeof(char16_t));
 if (result) {
  libc_memcpy(result,str,max_chars*sizeof(char16_t));
  result[max_chars] = '\0';
 }
 return result;
}
INTERN ATTR_MALLOC char32_t *LIBCCALL
libc_w32ndup_d(char32_t const *__restrict str, size_t max_chars) {
 char32_t *result;
 max_chars = libc_w32nlen(str,max_chars);
 result = (char32_t *)libc_malloc_d((max_chars+1)*sizeof(char32_t));
 if (result) {
  libc_memcpy(result,str,max_chars*sizeof(char32_t));
  result[max_chars] = '\0';
 }
 return result;
}

INTERN WUNUSED struct mallinfo LIBCCALL libc_mallinfo_d(void) {
 return libc_mallinfo_impl(libc_heap_info_d(&mheap));
}
INTERN void LIBCCALL libc_malloc_stats_d(void) {
 libc_malloc_stats_impl(libc_heap_info_d(&mheap));
}




DECL_END

#endif /* !GUARD_LIBS_LIBC_MALLOC_D_C */
