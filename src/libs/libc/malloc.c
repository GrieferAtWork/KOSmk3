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
#ifndef GUARD_LIBS_LIBC_MALLOC_C
#define GUARD_LIBS_LIBC_MALLOC_C 1

#include "libc.h"
#include "malloc.h"
#include "errno.h"

#include <errno.h>

DECL_BEGIN


CRT_DOS ATTR_MALLOC void *LIBCCALL
libd_aligned_malloc(size_t num_bytes, size_t min_alignment) {
 return libc_memalign(min_alignment,num_bytes);
}
CRT_DOS ATTR_MALLOC void *LIBCCALL
libd_aligned_offset_malloc(size_t num_bytes, size_t min_alignment, ptrdiff_t offset) {
 return libc_memalign_offset(min_alignment,num_bytes,offset);
}
CRT_DOS void *LIBCCALL
libd_aligned_realloc(void *ptr, size_t num_bytes, size_t min_alignment) {
 return libc_realign(ptr,min_alignment,num_bytes);
}
CRT_DOS void *LIBCCALL
libd_aligned_recalloc(void *ptr, size_t count,
                      size_t num_bytes, size_t min_alignment) {
 if (__builtin_mul_overflow(count,num_bytes,&num_bytes)) {
  libc_seterrno(ENOMEM);
  return NULL;
 }
 return libc_recalign(ptr,min_alignment,num_bytes);
}
CRT_DOS void *LIBCCALL
libd_aligned_offset_realloc(void *ptr, size_t num_bytes,
                            size_t min_alignment, ptrdiff_t offset) {
 return libc_realign_offset(ptr,min_alignment,num_bytes,offset);
}

CRT_DOS void *LIBCCALL
libd_aligned_offset_recalloc(void *ptr, size_t count, size_t num_bytes,
                             size_t min_alignment, ptrdiff_t offset) {
 if (__builtin_mul_overflow(count,num_bytes,&num_bytes)) {
  libc_seterrno(ENOMEM);
  return NULL;
 }
 return libc_recalign_offset(ptr,min_alignment,num_bytes,offset);
}
CRT_DOS size_t LIBCCALL
libd_aligned_msize(void *ptr, size_t UNUSED(min_alignment),
                   ptrdiff_t UNUSED(offset)) {
 return libc_malloc_usable_size(ptr);
}


/* Define dos-specific malloc functions. */
EXPORT(__DSYM(_aligned_malloc),libd_aligned_malloc);
EXPORT(__DSYM(_aligned_msize),libd_aligned_msize);
EXPORT(__DSYM(_aligned_offset_malloc),libd_aligned_offset_malloc);
EXPORT(__DSYM(_aligned_offset_realloc),libd_aligned_offset_realloc);
EXPORT(__DSYM(_aligned_offset_recalloc),libd_aligned_offset_recalloc);
EXPORT(__DSYM(_aligned_realloc),libd_aligned_realloc);
EXPORT(__DSYM(_aligned_recalloc),libd_aligned_recalloc);

#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libd_aligned_malloc_dbg,libd_aligned_malloc);
DEFINE_INTERN_ALIAS(libd_aligned_realloc_dbg,libd_aligned_realloc);
DEFINE_INTERN_ALIAS(libd_aligned_recalloc_dbg,libd_aligned_recalloc);
#else /* CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP */
CRT_DOS void *LIBCCALL
libd_aligned_malloc_dbg(size_t num_bytes, size_t min_alignment,
                        char const *UNUSED(file),
                        int UNUSED(lno)) {
 return libc_memalign(min_alignment,num_bytes);
}
CRT_DOS void *LIBCCALL
libd_aligned_realloc_dbg(void *memory, size_t num_bytes, size_t min_alignment,
                         char const *UNUSED(file),
                         int UNUSED(lno)) {
 return libc_realign(memory,min_alignment,num_bytes);
}
CRT_DOS void *LIBCCALL
libd_aligned_recalloc_dbg(void *memory, size_t count,
                          size_t num_bytes, size_t min_alignment,
                          char const *UNUSED(file),
                          int UNUSED(lno)) {
 if (__builtin_mul_overflow(count,num_bytes,&num_bytes)) {
  libc_seterrno(ENOMEM);
  return NULL;
 }
 return libc_recalign(memory,min_alignment,num_bytes);
}
#endif /* !CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP */


#if defined(CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP) && \
    __SIZEOF_SIZE_T__ == __SIZEOF_PTRDIFF_T__
DEFINE_INTERN_ALIAS(libd_aligned_offset_malloc_dbg,libd_aligned_offset_malloc);
DEFINE_INTERN_ALIAS(libd_aligned_offset_realloc_dbg,libd_aligned_offset_realloc);
DEFINE_INTERN_ALIAS(libd_aligned_offset_recalloc_dbg,libd_aligned_offset_recalloc);
#else
CRT_DOS void *LIBCCALL
libd_aligned_offset_malloc_dbg(size_t num_bytes, size_t min_alignment, size_t offset,
                               char const *UNUSED(file),
                               int UNUSED(lno)) {
 return libc_memalign_offset(min_alignment,num_bytes,(ptrdiff_t)offset);
}
CRT_DOS void *LIBCCALL
libd_aligned_offset_realloc_dbg(void *memory, size_t num_bytes,
                                size_t min_alignment, size_t offset,
                                char const *UNUSED(file),
                                int UNUSED(lno)) {
 return libc_realign_offset(memory,min_alignment,num_bytes,(ptrdiff_t)offset);
}
CRT_DOS void *LIBCCALL
libd_aligned_offset_recalloc_dbg(void *memory, size_t count,
                                 size_t num_bytes, size_t min_alignment,
                                 size_t offset,
                                 char const *UNUSED(file),
                                 int UNUSED(lno)) {
 if (__builtin_mul_overflow(count,num_bytes,&num_bytes)) {
  libc_seterrno(ENOMEM);
  return NULL;
 }
 return libc_recalign_offset(memory,min_alignment,num_bytes,(ptrdiff_t)offset);
}
#endif

EXPORT(__DSYM(_aligned_malloc_dbg),libd_aligned_malloc_dbg);
EXPORT(__DSYM(_aligned_realloc_dbg),libd_aligned_realloc_dbg);
EXPORT(__DSYM(_aligned_recalloc_dbg),libd_aligned_recalloc_dbg);
EXPORT(__DSYM(_aligned_offset_malloc_dbg),libd_aligned_offset_malloc_dbg);
EXPORT(__DSYM(_aligned_offset_realloc_dbg),libd_aligned_offset_realloc_dbg);
EXPORT(__DSYM(_aligned_offset_recalloc_dbg),libd_aligned_offset_recalloc_dbg);


/*TODO:
_Check_return_ _Ret_maybenull_z_ _CRTIMP wchar_t * __cdecl _wcsdup_dbg(
        _In_opt_z_ const wchar_t * _Str,
        _In_ int _BlockType,
        _In_opt_z_ const char * _Filename,
        _In_ int _LineNumber
        );

_Check_return_ _Ret_maybenull_z_ _CRTIMP char * __cdecl _tempnam_dbg(
        _In_opt_z_ const char * _DirName,
        _In_opt_z_ const char * _FilePrefix,
        _In_ int _BlockType,
        _In_opt_z_ const char * _Filename,
        _In_ int _LineNumber
        );

_Check_return_ _Ret_maybenull_z_ _CRTIMP wchar_t * __cdecl _wtempnam_dbg(
        _In_opt_z_ const wchar_t * _DirName,
        _In_opt_z_ const wchar_t * _FilePrefix,
        _In_ int _BlockType,
        _In_opt_z_ const char * _Filename,
        _In_ int _LineNumber
        );

_Check_return_ _Ret_maybenull_z_ _CRTIMP char * __cdecl _fullpath_dbg(
        _Out_writes_opt_z_(_SizeInBytes) char * _FullPath,
        _In_z_ const char * _Path,
        _In_ size_t _SizeInBytes,
        _In_ int _BlockType,
        _In_opt_z_ const char * _Filename,
        _In_ int _LineNumber
        );

_Check_return_ _Ret_maybenull_z_ _CRTIMP wchar_t * __cdecl _wfullpath_dbg(
        _Out_writes_opt_z_(_SizeInWords) wchar_t * _FullPath,
        _In_z_ const wchar_t * _Path,
        _In_ size_t _SizeInWords,
        _In_ int _BlockType,
        _In_opt_z_ const char * _Filename,
        _In_ int _LineNumber
        );

_Check_return_ _Ret_maybenull_z_ _CRTIMP char * __cdecl _getcwd_dbg(
        _Out_writes_opt_z_(_SizeInBytes) char * _DstBuf,
        _In_ int _SizeInBytes,
        _In_ int _BlockType,
        _In_opt_z_ const char * _Filename,
        _In_ int _LineNumber
        );

_Check_return_ _Ret_maybenull_z_ _CRTIMP char * __cdecl _getdcwd_dbg(
        _In_ int _Drive,
        _Out_writes_opt_z_(_SizeInBytes) char * _DstBuf,
        _In_ int _SizeInBytes,
        _In_ int _BlockType,
        _In_opt_z_ const char * _Filename,
        _In_ int _LineNumber
        );

_Check_return_ _Ret_maybenull_z_ char * __cdecl _getdcwd_lk_dbg(
        _In_ int _Drive,
        _Out_writes_opt_z_(_SizeInBytes) char * _DstBuf,
        _In_ int _SizeInBytes,
        _In_ int _BlockType,
        _In_opt_z_ const char * _Filename,
        _In_ int _LineNumber
        );

_Check_return_ _Ret_maybenull_z_ wchar_t * __cdecl _wgetdcwd_lk_dbg(
        _In_ int _Drive,
        _Out_writes_opt_z_(_SizeInWords) wchar_t * _DstBuf,
        _In_ int _SizeInWords,
        _In_ int _BlockType,
        _In_opt_z_ const char * _Filename,
        _In_ int _LineNumber
        );

_Check_return_wat_ _CRTIMP errno_t __cdecl _dupenv_s_dbg(
        _Outptr_result_buffer_maybenull_(*_PBufferSizeInBytes) _Outptr_result_z_ char ** _PBuffer,
        _Out_opt_ size_t * _PBufferSizeInBytes,
        _In_z_ const char * _VarName,
        _In_ int _BlockType,
        _In_opt_z_ const char * _Filename,
        _In_ int _LineNumber
        );

_Check_return_wat_ _CRTIMP errno_t __cdecl _wdupenv_s_dbg(
        _Outptr_result_buffer_maybenull_(*_PBufferSizeInWords) _Outptr_result_z_ wchar_t ** _PBuffer,
        _Out_opt_ size_t * _PBufferSizeInWords,
        _In_z_ const wchar_t * _VarName,
        _In_ int _BlockType,
        _In_opt_z_ const char * _Filename,
        _In_ int _LineNumber
        );
*/

DECL_END

#endif /* !GUARD_LIBS_LIBC_MALLOC_C */
