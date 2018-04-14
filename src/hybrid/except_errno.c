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
#ifndef GUARD_HYBRID_EXCEPT_ERRNO_C
#define GUARD_HYBRID_EXCEPT_ERRNO_C 1

#include "hybrid.h"
#include <hybrid/compiler.h>
#include <except.h>
#include <errno.h>
#ifndef __KERNEL__
#include "../libs/libc/rtl.h"
#include <syslog.h>
#define PRINTF(...) libc_syslog(LOG_DEBUG,__VA_ARGS__)
#else
#include <kernel/debug.h>
#define PRINTF(...) debug_printf(__VA_ARGS__)
#endif


DECL_BEGIN

INTERN errno_t FCALL
libc_exception_errno(struct exception_info *__restrict info) {
 errno_t result = EPERM;
 /*PRINTF("libc_exception_errno()\n");*/
 switch (info->e_error.e_code) {
 case E_BADALLOC:               result = ENOMEM; break;
 case E_INTERRUPT:              result = EINTR; break;
 case E_INVALID_HANDLE:         result = EBADF; break;
 case E_IOERROR:                result = EIO; break;
 case E_NO_DATA:                result = ENODATA; break;
 case E_NOT_EXECUTABLE:         result = ENOEXEC; break;
 case E_INVALID_ARGUMENT:       result = EINVAL; break;
 case E_TOO_MANY_HANDLES:       result = EMFILE; break;
 case E_PROCESS_EXITED:         result = ESRCH; break;
 case E_STACK_OVERFLOW:         result = EFAULT; break; /* Barely questionable... */
 case E_UNHANDLED_INTERRUPT:    result = EFAULT; break; /* Definitely questionable... */
 case E_UNKNOWN_SYSTEMCALL:     result = ENOSYS; break;
 case E_NOT_IMPLEMENTED:        result = ENOSYS; break;
 case E_INVALID_ALIGNMENT:      result = EFAULT; break;
#ifdef E_INVALID_SEGMENT
 case E_INVALID_SEGMENT:        result = EINVAL; break;
#endif
 case E_WOULDBLOCK:             result = EWOULDBLOCK; break;
 case E_SEGFAULT:               result = EFAULT; break;
 case E_OVERFLOW:               result = EOVERFLOW; break;
 case E_INDEX_ERROR:            result = ERANGE; break;
 case E_BREAKPOINT:             result = EINVAL; break; /* Definitely questionable... */
 case E_DIVIDE_BY_ZERO:         result = EINVAL; break; /* Mostly questionable... */
 case E_ILLEGAL_INSTRUCTION:    result = EPERM; break;  /* A bit questionable... */
 case E_PRIVILEGED_INSTRUCTION: result = EACCES; break; /* Somewhat questionable... */
 case E_FILESYSTEM_ERROR:
  switch (info->e_error.e_filesystem_error.fs_errcode) {
  case ERROR_FS_FILE_NOT_FOUND:
  case ERROR_FS_PATH_NOT_FOUND:        result = ENOENT; break;
  case ERROR_FS_NOT_A_DIRECTORY:       result = ENOTDIR; break;
  case ERROR_FS_TOO_MANY_LINKS:        result = ELOOP; break;
  case ERROR_FS_DIRECTORY_NOT_EMPTY:   result = ENOTEMPTY; break;
  case ERROR_FS_ILLEGAL_PATH:          result = EINVAL; break;
  case ERROR_FS_CROSSDEVICE_LINK:      result = EXDEV; break;
  case ERROR_FS_FILENAME_TOO_LONG:     result = ENAMETOOLONG; break;
  case ERROR_FS_FILE_ALREADY_EXISTS:   result = EEXIST; break;
  case ERROR_FS_UNSUPPORTED_FUNCTION:  result = EPERM; break;
  case ERROR_FS_READONLY_FILESYSTEM:   result = EROFS; break;
  case ERROR_FS_TRUNCATE_GREATER_SIZE: result = EINVAL; break;
  case ERROR_FS_ACCESS_ERROR:          result = EACCES; break;
  case ERROR_FS_DISK_FULL:             result = ENOSPC; break;
  case ERROR_FS_RMDIR_REGULAR:         result = ENOTDIR; break;
  case ERROR_FS_UNLINK_DIRECTORY:      result = EISDIR; break;
  case ERROR_FS_REMOVE_MOUNTPOINT:     result = EBUSY; break; /* Linux uses EINVAL for this. */
  case ERROR_FS_UNMOUNT_NOTAMOUNT:     result = EINVAL; break; /* Linux uses EINVAL for this (in umount). */
  case ERROR_FS_RENAME_NOTAMOUNT:      result = EBUSY; break; /* Linux uses EBUSY for this. */
  case ERROR_FS_NEGATIVE_SEEK:         result = ESPIPE; break;
  case ERROR_FS_CORRUPTED_FILESYSTEM:  result = EIO; break; /* ??? Questionable? */
  default: break;
  }
  break;
#ifdef E_BUFFER_TOO_SMALL
 case E_BUFFER_TOO_SMALL: result = ERANGE; break;
#endif
#ifdef E_UNICODE_ERROR
 case E_UNICODE_ERROR:    result = EILSEQ; break;
#endif
 default: break; /* What 'you gonna do? */
 }
 return result;
}

EXPORT(exception_errno,libc_exception_errno);


DECL_END

#endif /* !GUARD_HYBRID_EXCEPT_ERRNO_C */
