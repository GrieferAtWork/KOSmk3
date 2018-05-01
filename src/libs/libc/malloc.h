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
#ifndef GUARD_LIBS_LIBC_MALLOC_H
#define GUARD_LIBS_LIBC_MALLOC_H 1

#include "libc.h"
#include <kos/types.h>
#include <malloc.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     MALLOC                                                                            */
/* ===================================================================================== */
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *(LIBCCALL libc_Xmalloc)(size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *(LIBCCALL libc_Xcalloc)(size_t count, size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *(LIBCCALL libc_Xmemalign)(size_t min_alignment, size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *(LIBCCALL libc_Xmemalign_offset)(size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *(LIBCCALL libc_Xmemcalign)(size_t min_alignment, size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *(LIBCCALL libc_Xmemcalign_offset)(size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *(LIBCCALL libc_Xvalloc)(size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *(LIBCCALL libc_Xpvalloc)(size_t num_bytes);
INTDEF ATTR_RETNONNULL void *(LIBCCALL libc_Xrealloc)(void *ptr, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *(LIBCCALL libc_Xrecalloc)(void *ptr, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *(LIBCCALL libc_Xrealign)(void *ptr, size_t min_alignment, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *(LIBCCALL libc_Xrealign_offset)(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_RETNONNULL void *(LIBCCALL libc_Xrecalign)(void *ptr, size_t min_alignment, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *(LIBCCALL libc_Xrecalign_offset)(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_RETNONNULL void *(LIBCCALL libc_Xrealloc_in_place)(void *ptr, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *(LIBCCALL libc_Xrecalloc_in_place)(void *ptr, size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *(LIBCCALL libc_Xmemdup)(void const *__restrict ptr, size_t n_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *(LIBCCALL libc_Xmemcdup)(void const *__restrict ptr, int needle, size_t n_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *(LIBCCALL libc_Xstrdup)(char const *__restrict str);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *(LIBCCALL libc_Xstrndup)(char const *__restrict str, size_t max_chars);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char16_t *(LIBCCALL libc_Xw16dup)(char16_t const *__restrict str);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char32_t *(LIBCCALL libc_Xw32dup)(char32_t const *__restrict str);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char16_t *(LIBCCALL libc_Xw16ndup)(char16_t const *__restrict str, size_t max_chars);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char32_t *(LIBCCALL libc_Xw32ndup)(char32_t const *__restrict str, size_t max_chars);
INTDEF ATTR_MALLOC void *(LIBCCALL libc_malloc)(size_t num_bytes);
INTDEF ATTR_MALLOC void *(LIBCCALL libc_calloc)(size_t count, size_t num_bytes);
INTDEF ATTR_MALLOC void *(LIBCCALL libc_memalign)(size_t min_alignment, size_t num_bytes);
INTDEF ATTR_MALLOC void *(LIBCCALL libc_memalign_offset)(size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_MALLOC void *(LIBCCALL libc_memcalign)(size_t min_alignment, size_t num_bytes);
INTDEF ATTR_MALLOC void *(LIBCCALL libc_memcalign_offset)(size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_MALLOC void *(LIBCCALL libc_valloc)(size_t num_bytes);
INTDEF ATTR_MALLOC void *(LIBCCALL libc_pvalloc)(size_t num_bytes);
INTDEF size_t (LIBCCALL libc_malloc_usable_size)(void *__restrict ptr);
INTDEF void *(LIBCCALL libc_realloc)(void *ptr, size_t num_bytes);
INTDEF void *(LIBCCALL libc_recalloc)(void *ptr, size_t num_bytes);
INTDEF void *(LIBCCALL libc_realign)(void *ptr, size_t min_alignment, size_t num_bytes);
INTDEF void *(LIBCCALL libc_realign_offset)(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF void *(LIBCCALL libc_recalign)(void *ptr, size_t min_alignment, size_t num_bytes);
INTDEF void *(LIBCCALL libc_recalign_offset)(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF void *(LIBCCALL libc_realloc_in_place)(void *ptr, size_t num_bytes);
INTDEF void *(LIBCCALL libc_recalloc_in_place)(void *ptr, size_t num_bytes);
INTDEF void NOTHROW((LIBCCALL libc_free)(void *ptr));
INTDEF int (LIBCCALL libc_posix_memalign)(void **__restrict pp, size_t min_alignment, size_t n_bytes);
INTDEF int (LIBCCALL libc_mallopt)(int parameter_number, int parameter_value);
INTDEF int (LIBCCALL libc_malloc_trim)(size_t pad);
INTDEF ATTR_MALLOC void *(LIBCCALL libc_memdup)(void const *__restrict ptr, size_t n_bytes);
INTDEF ATTR_MALLOC void *(LIBCCALL libc_memcdup)(void const *__restrict ptr, int needle, size_t n_bytes);
INTDEF ATTR_MALLOC char *(LIBCCALL libc_strdup)(char const *__restrict str);
INTDEF ATTR_MALLOC char *(LIBCCALL libc_strndup)(char const *__restrict str, size_t max_chars);
INTDEF ATTR_MALLOC char16_t *(LIBCCALL libc_w16dup)(char16_t const *__restrict str);
INTDEF ATTR_MALLOC char32_t *(LIBCCALL libc_w32dup)(char32_t const *__restrict str);
INTDEF ATTR_MALLOC char16_t *(LIBCCALL libc_w16ndup)(char16_t const *__restrict str, size_t max_chars);
INTDEF ATTR_MALLOC char32_t *(LIBCCALL libc_w32ndup)(char32_t const *__restrict str, size_t max_chars);
INTDEF WUNUSED struct mallinfo (LIBCCALL libc_mallinfo)(void);
INTDEF void (LIBCCALL libc_malloc_stats)(void);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmalloc_f(size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmalloc_d(size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xcalloc_f(size_t count, size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xcalloc_d(size_t count, size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmemalign_f(size_t min_alignment, size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmemalign_d(size_t min_alignment, size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmemalign_offset_f(size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmemalign_offset_d(size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmemcalign_f(size_t min_alignment, size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmemcalign_d(size_t min_alignment, size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmemcalign_offset_f(size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmemcalign_offset_d(size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xvalloc_f(size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xvalloc_d(size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xpvalloc_f(size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xpvalloc_d(size_t num_bytes);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrealloc_f(void *ptr, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrealloc_d(void *ptr, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrecalloc_f(void *ptr, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrecalloc_d(void *ptr, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrealign_f(void *ptr, size_t min_alignment, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrealign_d(void *ptr, size_t min_alignment, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrealign_offset_f(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrealign_offset_d(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrecalign_f(void *ptr, size_t min_alignment, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrecalign_d(void *ptr, size_t min_alignment, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrecalign_offset_f(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrecalign_offset_d(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrealloc_in_place_f(void *ptr, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrealloc_in_place_d(void *ptr, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrecalloc_in_place_f(void *ptr, size_t num_bytes);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xrecalloc_in_place_d(void *ptr, size_t num_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmemdup_f(void const *__restrict ptr, size_t n_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmemdup_d(void const *__restrict ptr, size_t n_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmemcdup_f(void const *__restrict ptr, int needle, size_t n_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC void *LIBCCALL libc_Xmemcdup_d(void const *__restrict ptr, int needle, size_t n_bytes);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *LIBCCALL libc_Xstrdup_f(char const *__restrict str);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *LIBCCALL libc_Xstrdup_d(char const *__restrict str);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *LIBCCALL libc_Xstrndup_f(char const *__restrict str, size_t max_chars);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *LIBCCALL libc_Xstrndup_d(char const *__restrict str, size_t max_chars);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char16_t *LIBCCALL libc_Xw16dup_f(char16_t const *__restrict str);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char32_t *LIBCCALL libc_Xw32dup_f(char32_t const *__restrict str);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char16_t *LIBCCALL libc_Xw16dup_d(char16_t const *__restrict str);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char32_t *LIBCCALL libc_Xw32dup_d(char32_t const *__restrict str);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char16_t *LIBCCALL libc_Xw16ndup_f(char16_t const *__restrict str, size_t max_chars);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char32_t *LIBCCALL libc_Xw32ndup_f(char32_t const *__restrict str, size_t max_chars);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char16_t *LIBCCALL libc_Xw16ndup_d(char16_t const *__restrict str, size_t max_chars);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char32_t *LIBCCALL libc_Xw32ndup_d(char32_t const *__restrict str, size_t max_chars);
INTDEF ATTR_MALLOC void *LIBCCALL libc_malloc_f(size_t num_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_malloc_d(size_t num_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_calloc_f(size_t count, size_t num_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_calloc_d(size_t count, size_t num_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_memalign_f(size_t min_alignment, size_t num_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_memalign_d(size_t min_alignment, size_t num_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_memalign_offset_f(size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_MALLOC void *LIBCCALL libc_memalign_offset_d(size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_MALLOC void *LIBCCALL libc_memcalign_f(size_t min_alignment, size_t num_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_memcalign_d(size_t min_alignment, size_t num_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_memcalign_offset_f(size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_MALLOC void *LIBCCALL libc_memcalign_offset_d(size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF ATTR_MALLOC void *LIBCCALL libc_valloc_f(size_t num_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_valloc_d(size_t num_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_pvalloc_f(size_t num_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_pvalloc_d(size_t num_bytes);
INTDEF size_t LIBCCALL libc_malloc_usable_size_f(void *__restrict ptr);
INTDEF size_t LIBCCALL libc_malloc_usable_size_d(void *__restrict ptr);
INTDEF void *LIBCCALL libc_realloc_f(void *ptr, size_t num_bytes);
INTDEF void *LIBCCALL libc_realloc_d(void *ptr, size_t num_bytes);
INTDEF void *LIBCCALL libc_recalloc_f(void *ptr, size_t num_bytes);
INTDEF void *LIBCCALL libc_recalloc_d(void *ptr, size_t num_bytes);
INTDEF void *LIBCCALL libc_realign_f(void *ptr, size_t min_alignment, size_t num_bytes);
INTDEF void *LIBCCALL libc_realign_d(void *ptr, size_t min_alignment, size_t num_bytes);
INTDEF void *LIBCCALL libc_realign_offset_f(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF void *LIBCCALL libc_realign_offset_d(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF void *LIBCCALL libc_recalign_f(void *ptr, size_t min_alignment, size_t num_bytes);
INTDEF void *LIBCCALL libc_recalign_d(void *ptr, size_t min_alignment, size_t num_bytes);
INTDEF void *LIBCCALL libc_recalign_offset_f(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF void *LIBCCALL libc_recalign_offset_d(void *ptr, size_t min_alignment, size_t num_bytes, ptrdiff_t offset);
INTDEF void *LIBCCALL libc_realloc_in_place_f(void *ptr, size_t num_bytes);
INTDEF void *LIBCCALL libc_realloc_in_place_d(void *ptr, size_t num_bytes);
INTDEF void *LIBCCALL libc_recalloc_in_place_f(void *ptr, size_t num_bytes);
INTDEF void *LIBCCALL libc_recalloc_in_place_d(void *ptr, size_t num_bytes);
INTDEF void LIBCCALL libc_free_f(void *ptr);
INTDEF void LIBCCALL libc_free_d(void *ptr);
INTDEF int LIBCCALL libc_posix_memalign_f(void **__restrict pp, size_t min_alignment, size_t n_bytes);
INTDEF int LIBCCALL libc_posix_memalign_d(void **__restrict pp, size_t min_alignment, size_t n_bytes);
INTDEF int LIBCCALL libc_mallopt_f(int parameter_number, int parameter_value);
INTDEF int LIBCCALL libc_mallopt_d(int parameter_number, int parameter_value);
INTDEF int LIBCCALL libc_malloc_trim_f(size_t pad);
INTDEF int LIBCCALL libc_malloc_trim_d(size_t pad);
INTDEF ATTR_MALLOC void *LIBCCALL libc_memdup_f(void const *__restrict ptr, size_t n_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_memdup_d(void const *__restrict ptr, size_t n_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_memcdup_f(void const *__restrict ptr, int needle, size_t n_bytes);
INTDEF ATTR_MALLOC void *LIBCCALL libc_memcdup_d(void const *__restrict ptr, int needle, size_t n_bytes);
INTDEF ATTR_MALLOC char *LIBCCALL libc_strdup_f(char const *__restrict str);
INTDEF ATTR_MALLOC char *LIBCCALL libc_strdup_d(char const *__restrict str);
INTDEF ATTR_MALLOC char *LIBCCALL libc_strndup_f(char const *__restrict str, size_t max_chars);
INTDEF ATTR_MALLOC char *LIBCCALL libc_strndup_d(char const *__restrict str, size_t max_chars);
INTDEF ATTR_MALLOC char16_t *LIBCCALL libc_w16dup_f(char16_t const *__restrict str);
INTDEF ATTR_MALLOC char32_t *LIBCCALL libc_w32dup_f(char32_t const *__restrict str);
INTDEF ATTR_MALLOC char16_t *LIBCCALL libc_w16dup_d(char16_t const *__restrict str);
INTDEF ATTR_MALLOC char32_t *LIBCCALL libc_w32dup_d(char32_t const *__restrict str);
INTDEF ATTR_MALLOC char16_t *LIBCCALL libc_w16ndup_f(char16_t const *__restrict str, size_t max_chars);
INTDEF ATTR_MALLOC char32_t *LIBCCALL libc_w32ndup_f(char32_t const *__restrict str, size_t max_chars);
INTDEF ATTR_MALLOC char16_t *LIBCCALL libc_w16ndup_d(char16_t const *__restrict str, size_t max_chars);
INTDEF ATTR_MALLOC char32_t *LIBCCALL libc_w32ndup_d(char32_t const *__restrict str, size_t max_chars);
INTDEF WUNUSED struct mallinfo LIBCCALL libc_mallinfo_f(void);
INTDEF WUNUSED struct mallinfo LIBCCALL libc_mallinfo_d(void);
INTDEF void LIBCCALL libc_malloc_stats_f(void);
INTDEF void LIBCCALL libc_malloc_stats_d(void);

struct heapinfo;
INTDEF WUNUSED struct mallinfo LIBCCALL libc_mallinfo_impl(struct heapinfo info);
INTDEF void LIBCCALL libc_malloc_stats_impl(struct heapinfo info);

/* DOS-specific functions. */
INTDEF ATTR_MALLOC void *(LIBCCALL libd_aligned_malloc)(size_t num_bytes, size_t min_alignment);
INTDEF ATTR_MALLOC void *(LIBCCALL libd_aligned_offset_malloc)(size_t num_bytes, size_t min_alignment, ptrdiff_t offset);
INTDEF void *(LIBCCALL libd_aligned_realloc)(void *ptr, size_t num_bytes, size_t min_alignment);
INTDEF void *(LIBCCALL libd_aligned_recalloc)(void *ptr, size_t count, size_t num_bytes, size_t min_alignment);
INTDEF void *(LIBCCALL libd_aligned_offset_realloc)(void *ptr, size_t num_bytes, size_t min_alignment, ptrdiff_t offset);
INTDEF void *(LIBCCALL libd_aligned_offset_recalloc)(void *ptr, size_t count, size_t num_bytes, size_t min_alignment, ptrdiff_t offset);
INTDEF size_t (LIBCCALL libd_aligned_msize)(void *ptr, size_t min_alignment, ptrdiff_t offset); /* Why does this need `min_alignment' and `offset'? - Whatever... We just ignore those arguments. */
INTDEF void *LIBCCALL libd_aligned_malloc_dbg(size_t num_bytes, size_t min_alignment, char const *file, int lno);
INTDEF void *LIBCCALL libd_aligned_realloc_dbg(void *memory, size_t num_bytes, size_t min_alignment, char const *file, int lno);
INTDEF void *LIBCCALL libd_aligned_recalloc_dbg(void *memory, size_t count, size_t num_bytes, size_t min_alignment, char const *file, int lno);
INTDEF void *LIBCCALL libd_aligned_offset_malloc_dbg(size_t num_bytes, size_t min_alignment, size_t offset, char const *file, int lno);
INTDEF void *LIBCCALL libd_aligned_offset_realloc_dbg(void *memory, size_t num_bytes, size_t min_alignment, size_t offset, char const *file, int lno);
INTDEF void *LIBCCALL libd_aligned_offset_recalloc_dbg(void *memory, size_t count, size_t num_bytes, size_t min_alignment, size_t offset, char const *file, int lno);


/* Must be called before any allocations are made in order
 * to redirect libc heap functions to their debug-counterparts. */
INTDEF void LIBCCALL libc_set_debug_malloc(void);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_MALLOC_H */
