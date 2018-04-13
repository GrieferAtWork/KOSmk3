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
#ifndef GUARD_LIBS_LIBC_ERRNO_C
#define GUARD_LIBS_LIBC_ERRNO_C 1

#include "libc.h"
#include "errno.h"

#include <errno.h>
#include <bits/dos-errno.h>
#include <winapi/winerror.h>

DECL_BEGIN

INTERN errno_t LIBCCALL libc_errno_dos2kos(derrno_t value) {
 errno_t result;
 switch (value) {
 case 0                     : result = 0; break;
 case __DOS_EPERM           : result = EPERM; break;
 case __DOS_ENOENT          : result = ENOENT; break;
 case __DOS_ESRCH           : result = ESRCH; break;
 case __DOS_EINTR           : result = EINTR; break;
 case __DOS_EIO             : result = EIO; break;
 case __DOS_ENXIO           : result = ENXIO; break;
 case __DOS_E2BIG           : result = E2BIG; break;
 case __DOS_ENOEXEC         : result = ENOEXEC; break;
 case __DOS_EBADF           : result = EBADF; break;
 case __DOS_ECHILD          : result = ECHILD; break;
 case __DOS_EAGAIN          : result = EAGAIN; break;
 case __DOS_ENOMEM          : result = ENOMEM; break;
 case __DOS_EACCES          : result = EACCES; break;
 case __DOS_EFAULT          : result = EFAULT; break;
 case __DOS_EBUSY           : result = EBUSY; break;
 case __DOS_EEXIST          : result = EEXIST; break;
 case __DOS_EXDEV           : result = EXDEV; break;
 case __DOS_ENODEV          : result = ENODEV; break;
 case __DOS_ENOTDIR         : result = ENOTDIR; break;
 case __DOS_EISDIR          : result = EISDIR; break;
 case __DOS_ENFILE          : result = ENFILE; break;
 case __DOS_EMFILE          : result = EMFILE; break;
 case __DOS_ENOTTY          : result = ENOTTY; break;
 case __DOS_EFBIG           : result = EFBIG; break;
 case __DOS_ENOSPC          : result = ENOSPC; break;
 case __DOS_ESPIPE          : result = ESPIPE; break;
 case __DOS_EROFS           : result = EROFS; break;
 case __DOS_EMLINK          : result = EMLINK; break;
 case __DOS_EPIPE           : result = EPIPE; break;
 case __DOS_EDOM            : result = EDOM; break;
 case __DOS_EDEADLK         : result = EDEADLK; break;
 case __DOS_ENAMETOOLONG    : result = ENAMETOOLONG; break;
 case __DOS_ENOLCK          : result = ENOLCK; break;
 case __DOS_ENOSYS          : result = ENOSYS; break;
 case __DOS_ENOTEMPTY       : result = ENOTEMPTY; break;
 case __DOS_ERANGE          : result = ERANGE; break;
 case __DOS_EILSEQ          : result = EILSEQ; break;
 case __DOS_EADDRINUSE      : result = EADDRINUSE; break;
 case __DOS_EADDRNOTAVAIL   : result = EADDRNOTAVAIL; break;
 case __DOS_EAFNOSUPPORT    : result = EAFNOSUPPORT; break;
 case __DOS_EALREADY        : result = EALREADY; break;
 case __DOS_EBADMSG         : result = EBADMSG; break;
 case __DOS_ECANCELED       : result = ECANCELED; break;
 case __DOS_ECONNABORTED    : result = ECONNABORTED; break;
 case __DOS_ECONNREFUSED    : result = ECONNREFUSED; break;
 case __DOS_ECONNRESET      : result = ECONNRESET; break;
 case __DOS_EDESTADDRREQ    : result = EDESTADDRREQ; break;
 case __DOS_EHOSTUNREACH    : result = EHOSTUNREACH; break;
 case __DOS_EIDRM           : result = EIDRM; break;
 case __DOS_EINPROGRESS     : result = EINPROGRESS; break;
 case __DOS_EISCONN         : result = EISCONN; break;
 case __DOS_ELOOP           : result = ELOOP; break;
 case __DOS_EMSGSIZE        : result = EMSGSIZE; break;
 case __DOS_ENETDOWN        : result = ENETDOWN; break;
 case __DOS_ENETRESET       : result = ENETRESET; break;
 case __DOS_ENETUNREACH     : result = ENETUNREACH; break;
 case __DOS_ENOBUFS         : result = ENOBUFS; break;
 case __DOS_ENODATA         : result = ENODATA; break;
 case __DOS_ENOLINK         : result = ENOLINK; break;
 case __DOS_ENOMSG          : result = ENOMSG; break;
 case __DOS_ENOPROTOOPT     : result = ENOPROTOOPT; break;
 case __DOS_ENOSR           : result = ENOSR; break;
 case __DOS_ENOSTR          : result = ENOSTR; break;
 case __DOS_ENOTCONN        : result = ENOTCONN; break;
 case __DOS_ENOTRECOVERABLE : result = ENOTRECOVERABLE; break;
 case __DOS_ENOTSOCK        : result = ENOTSOCK; break;
 case __DOS_ENOTSUP         : result = EOPNOTSUPP; break;
 case __DOS_EOPNOTSUPP      : result = EOPNOTSUPP; break;
 case __DOS_EOVERFLOW       : result = EOVERFLOW; break;
 case __DOS_EOWNERDEAD      : result = EOWNERDEAD; break;
 case __DOS_EPROTO          : result = EPROTO; break;
 case __DOS_EPROTONOSUPPORT : result = EPROTONOSUPPORT; break;
 case __DOS_EPROTOTYPE      : result = EPROTOTYPE; break;
 case __DOS_ETIME           : result = ETIME; break;
 case __DOS_ETIMEDOUT       : result = ETIMEDOUT; break;
 case __DOS_ETXTBSY         : result = ETXTBSY; break;
 case __DOS_EWOULDBLOCK     : result = EWOULDBLOCK; break;
 case __DOS_STRUNCATE       : result = KOS_STRUNCATE; break;
 default                    : result = EINVAL; break;
 }
 return result;
}
INTERN derrno_t LIBCCALL libc_errno_kos2dos(errno_t value) {
 errno_t result;
 switch (value) {
 case 0               : result = 0; break;
 case EPERM           : result = __DOS_EPERM; break;
 case ENOENT          : result = __DOS_ENOENT; break;
 case ESRCH           : result = __DOS_ESRCH; break;
 case EINTR           : result = __DOS_EINTR; break;
 case EIO             : result = __DOS_EIO; break;
 case ENXIO           : result = __DOS_ENXIO; break;
 case E2BIG           : result = __DOS_E2BIG; break;
 case ENOEXEC         : result = __DOS_ENOEXEC; break;
 case EBADF           : result = __DOS_EBADF; break;
 case ECHILD          : result = __DOS_ECHILD; break;
 case EAGAIN          : result = __DOS_EAGAIN; break;
 case ENOMEM          : result = __DOS_ENOMEM; break;
 case EACCES          : result = __DOS_EACCES; break;
 case EFAULT          : result = __DOS_EFAULT; break;
 case EBUSY           : result = __DOS_EBUSY; break;
 case EEXIST          : result = __DOS_EEXIST; break;
 case EXDEV           : result = __DOS_EXDEV; break;
 case ENODEV          : result = __DOS_ENODEV; break;
 case ENOTDIR         : result = __DOS_ENOTDIR; break;
 case EISDIR          : result = __DOS_EISDIR; break;
 case EINVAL          : result = __DOS_EINVAL; break;
 case ENFILE          : result = __DOS_ENFILE; break;
 case EMFILE          : result = __DOS_EMFILE; break;
 case ENOTTY          : result = __DOS_ENOTTY; break;
 case ETXTBSY         : result = __DOS_ETXTBSY; break;
 case EFBIG           : result = __DOS_EFBIG; break;
 case ENOSPC          : result = __DOS_ENOSPC; break;
 case ESPIPE          : result = __DOS_ESPIPE; break;
 case EROFS           : result = __DOS_EROFS; break;
 case EMLINK          : result = __DOS_EMLINK; break;
 case EPIPE           : result = __DOS_EPIPE; break;
 case EDOM            : result = __DOS_EDOM; break;
 case ERANGE          : result = __DOS_ERANGE; break;
 case EDEADLK         : result = __DOS_EDEADLK; break;
 case ENAMETOOLONG    : result = __DOS_ENAMETOOLONG; break;
 case ENOLCK          : result = __DOS_ENOLCK; break;
 case ENOSYS          : result = __DOS_ENOSYS; break;
 case ENOTEMPTY       : result = __DOS_ENOTEMPTY; break;
 case ELOOP           : result = __DOS_ELOOP; break;
 case ENOMSG          : result = __DOS_ENOMSG; break;
 case EIDRM           : result = __DOS_EIDRM; break;
 case ENOSTR          : result = __DOS_ENOSTR; break;
 case ENODATA         : result = __DOS_ENODATA; break;
 case ETIME           : result = __DOS_ETIME; break;
 case ENOSR           : result = __DOS_ENOSR; break;
 case ENOLINK         : result = __DOS_ENOLINK; break;
 case EPROTO          : result = __DOS_EPROTO; break;
 case EBADMSG         : result = __DOS_EBADMSG; break;
 case EOVERFLOW       : result = __DOS_EOVERFLOW; break;
 case EBADFD          : result = __DOS_EBADF; break;
 case EILSEQ          : result = __DOS_EILSEQ; break;
 case ENOTSOCK        : result = __DOS_ENOTSOCK; break;
 case EDESTADDRREQ    : result = __DOS_EDESTADDRREQ; break;
 case EMSGSIZE        : result = __DOS_EMSGSIZE; break;
 case EPROTOTYPE      : result = __DOS_EPROTOTYPE; break;
 case ENOPROTOOPT     : result = __DOS_ENOPROTOOPT; break;
 case EPROTONOSUPPORT : result = __DOS_EPROTONOSUPPORT; break;
 case EOPNOTSUPP      : result = __DOS_EOPNOTSUPP; break;
 case EAFNOSUPPORT    : result = __DOS_EAFNOSUPPORT; break;
 case EADDRINUSE      : result = __DOS_EADDRINUSE; break;
 case EADDRNOTAVAIL   : result = __DOS_EADDRNOTAVAIL; break;
 case ENETDOWN        : result = __DOS_ENETDOWN; break;
 case ENETUNREACH     : result = __DOS_ENETUNREACH; break;
 case ENETRESET       : result = __DOS_ENETRESET; break;
 case ECONNABORTED    : result = __DOS_ECONNABORTED; break;
 case ECONNRESET      : result = __DOS_ECONNRESET; break;
 case ENOBUFS         : result = __DOS_ENOBUFS; break;
 case EISCONN         : result = __DOS_EISCONN; break;
 case ENOTCONN        : result = __DOS_ENOTCONN; break;
 case ETIMEDOUT       : result = __DOS_ETIMEDOUT; break;
 case ECONNREFUSED    : result = __DOS_ECONNREFUSED; break;
 case EHOSTUNREACH    : result = __DOS_EHOSTUNREACH; break;
 case EALREADY        : result = __DOS_EALREADY; break;
 case EINPROGRESS     : result = __DOS_EINPROGRESS; break;
 case ECANCELED       : result = __DOS_ECANCELED; break;
 case EOWNERDEAD      : result = __DOS_EOWNERDEAD; break;
 case ENOTRECOVERABLE : result = __DOS_ENOTRECOVERABLE; break;
 case KOS_STRUNCATE   : result = __DOS_STRUNCATE; break;
 default              : result = __DOS_EOTHER; break;
 }
 return result;
}
INTERN derrno_t LIBCCALL libc_errno_nt2dos(u32 value) {
 derrno_t result;
 switch (value) {
 case 0                            : result = 0; break;
 case ERROR_INVALID_FUNCTION       : result = __DOS_EINVAL; break;
 case ERROR_FILE_NOT_FOUND         : result = __DOS_ENOENT; break;
 case ERROR_PATH_NOT_FOUND         : result = __DOS_ENOENT; break;
 case ERROR_TOO_MANY_OPEN_FILES    : result = __DOS_EMFILE; break;
 case ERROR_ACCESS_DENIED          : result = __DOS_EACCES; break;
 case ERROR_INVALID_HANDLE         : result = __DOS_EBADF; break;
 case ERROR_ARENA_TRASHED          : result = __DOS_ENOMEM; break;
 case ERROR_NOT_ENOUGH_MEMORY      : result = __DOS_ENOMEM; break;
 case ERROR_INVALID_BLOCK          : result = __DOS_ENOMEM; break;
 case ERROR_BAD_ENVIRONMENT        : result = __DOS_E2BIG; break;
 case ERROR_BAD_FORMAT             : result = __DOS_ENOEXEC; break;
 case ERROR_INVALID_ACCESS         : result = __DOS_EINVAL; break;
 case ERROR_INVALID_DATA           : result = __DOS_EINVAL; break;
 case ERROR_INVALID_DRIVE          : result = __DOS_ENOENT; break;
 case ERROR_CURRENT_DIRECTORY      : result = __DOS_EACCES; break;
 case ERROR_NOT_SAME_DEVICE        : result = __DOS_EXDEV; break;
 case ERROR_NO_MORE_FILES          : result = __DOS_ENOENT; break;
 case ERROR_BAD_NETPATH            : result = __DOS_ENOENT; break;
 case ERROR_NETWORK_ACCESS_DENIED  : result = __DOS_EACCES; break;
 case ERROR_BAD_NET_NAME           : result = __DOS_ENOENT; break;
 case ERROR_FILE_EXISTS            : result = __DOS_EEXIST; break;
 case ERROR_CANNOT_MAKE            : result = __DOS_EACCES; break;
 case ERROR_FAIL_I24               : result = __DOS_EACCES; break;
 case ERROR_INVALID_PARAMETER      : result = __DOS_EINVAL; break;
 case ERROR_NO_PROC_SLOTS          : result = __DOS_EAGAIN; break;
 case ERROR_DRIVE_LOCKED           : result = __DOS_EACCES; break;
 case ERROR_BROKEN_PIPE            : result = __DOS_EPIPE; break;
 case ERROR_DISK_FULL              : result = __DOS_ENOSPC; break;
 case ERROR_INVALID_TARGET_HANDLE  : result = __DOS_EBADF; break;
 case ERROR_WAIT_NO_CHILDREN       : result = __DOS_ECHILD; break;
 case ERROR_CHILD_NOT_COMPLETE     : result = __DOS_ECHILD; break;
 case ERROR_DIRECT_ACCESS_HANDLE   : result = __DOS_EBADF; break;
#if 1 /* DOS Returns EINVAL for this one... (Don't ask me why) */
 case ERROR_NEGATIVE_SEEK          : result = __DOS_EINVAL; break;
#else
 case ERROR_NEGATIVE_SEEK          : result = __DOS_ESPIPE; break;
#endif
 case ERROR_SEEK_ON_DEVICE         : result = __DOS_EACCES; break;
 case ERROR_DIR_NOT_EMPTY          : result = __DOS_ENOTEMPTY; break;
 case ERROR_NOT_LOCKED             : result = __DOS_EACCES; break;
 case ERROR_BAD_PATHNAME           : result = __DOS_ENOENT; break;
 case ERROR_MAX_THRDS_REACHED      : result = __DOS_EAGAIN; break;
 case ERROR_LOCK_FAILED            : result = __DOS_EACCES; break;
 case ERROR_ALREADY_EXISTS         : result = __DOS_EEXIST; break;
#if 1 /* DOS Returns ENOENT for this one... (Don't ask me why) */
 case ERROR_FILENAME_EXCED_RANGE   : result = __DOS_ENOENT; break;
#else
 case ERROR_FILENAME_EXCED_RANGE   : result = __DOS_ENAMETOOLONG; break;
#endif
 case ERROR_NESTING_NOT_ALLOWED    : result = __DOS_EAGAIN; break;
 case ERROR_NOT_ENOUGH_QUOTA       : result = __DOS_ENOMEM; break;
 case ERROR_WRITE_PROTECT ... ERROR_SHARING_BUFFER_EXCEEDED: result = __DOS_EACCES; break;
 case ERROR_INVALID_STARTING_CODESEG ... ERROR_INFLOOP_IN_RELOC_CHAIN: result = __DOS_ENOEXEC; break;
  /* Error codes that weren't actually used by DOS, but had matching errno values. */
 case ERROR_POSSIBLE_DEADLOCK      : result = __DOS_EDEADLOCK; break;
  /* I would have thought `__DOS_EOTHER' here, but the real deal returns `__DOS_EINVAL', too... */
 default: result = __DOS_EINVAL; break;
 }
 return result;
}
INTERN u32 LIBCCALL libc_errno_dos2nt(derrno_t value) {
 u32 result;
 switch (value) {
 case 0                            : result = 0; break;
 case __DOS_EINVAL                 : result = ERROR_INVALID_PARAMETER; break;
 case __DOS_ENOENT                 : result = ERROR_FILE_NOT_FOUND; break;
 case __DOS_EMFILE                 : result = ERROR_TOO_MANY_OPEN_FILES; break;
 case __DOS_EACCES                 : result = ERROR_ACCESS_DENIED; break;
 case __DOS_EBADF                  : result = ERROR_INVALID_HANDLE; break;
 case __DOS_ENOMEM                 : result = ERROR_NOT_ENOUGH_MEMORY; break;
 case __DOS_E2BIG                  : result = ERROR_BAD_ENVIRONMENT; break;
 case __DOS_ENOEXEC                : result = ERROR_BAD_FORMAT; break;
 case __DOS_EXDEV                  : result = ERROR_NOT_SAME_DEVICE; break;
 case __DOS_EEXIST                 : result = ERROR_ALREADY_EXISTS; break;
  /* None of these options ~really~ fit... */
 case __DOS_EAGAIN                 : result = ERROR_NO_PROC_SLOTS; break;
 //case __DOS_EAGAIN               : result = ERROR_MAX_THRDS_REACHED; break;
 //case __DOS_EAGAIN               : result = ERROR_NESTING_NOT_ALLOWED; break;
 case __DOS_EPIPE                  : result = ERROR_BROKEN_PIPE; break;
 case __DOS_ENOSPC                 : result = ERROR_DISK_FULL; break;
 case __DOS_ECHILD                 : result = ERROR_WAIT_NO_CHILDREN; break;
 case __DOS_ESPIPE                 : result = ERROR_NEGATIVE_SEEK; break;
 case __DOS_ENOTEMPTY              : result = ERROR_DIR_NOT_EMPTY; break;
  /* Error codes that weren't actually used by DOS. */
 case __DOS_ENAMETOOLONG           : result = ERROR_FILENAME_EXCED_RANGE; break;
 case __DOS_EDEADLOCK              : result = ERROR_POSSIBLE_DEADLOCK; break;
 case __DOS_EPERM                  : result = ERROR_INVALID_FUNCTION; break;
 case __DOS_ESRCH                  : result = ERROR_PROC_NOT_FOUND; break;
 case __DOS_EIO                    : result = ERROR_IO_DEVICE; break;
 case __DOS_ENXIO                  : result = ERROR_DEVICE_REMOVED; break;
 case __DOS_EFAULT                 : result = ERROR_INVALID_ADDRESS; break;
 case __DOS_ENODEV                 : result = ERROR_DEVICE_REMOVED; break;
 default                           : result = ERROR_INVALID_FUNCTION; break;
 }
 return result;
}
INTERN errno_t LIBCCALL libc_errno_nt2kos(u32 value) {
 return libc_errno_dos2kos(libc_errno_nt2dos(value));
}
INTERN u32 LIBCCALL libc_errno_kos2nt(errno_t value) {
 return libc_errno_dos2nt(libc_errno_kos2dos(value));
}



PRIVATE char const *const empty_errlist[] = { NULL };
PRIVATE int const empty_errlist_sz = 0;
INTERN char **LIBCCALL libc_sys_errlist(void) { return (char **)empty_errlist; }
INTERN int *LIBCCALL libc_sys_nerr(void) { return (int *)&empty_errlist_sz; }
INTERN derrno_t LIBCCALL libc_dos_geterrno2(derrno_t *value) { if (!value) return __DOS_EINVAL; *value = libc_dos_geterrno(); return 0; }
INTERN derrno_t LIBCCALL libc_dos_seterrno2(derrno_t value) { libc_dos_seterrno(value); return 0; }
INTERN derrno_t LIBCCALL libc_nt_geterrno2(u32 *value) { if (!value) return __DOS_EINVAL; *value = libc_nt_geterrno(); return 0; }
INTERN derrno_t LIBCCALL libc_nt_seterrno2(u32 value) { libc_nt_seterrno(value); return 0; }

/* Export errno symbols */
EXPORT(errno_dos2kos,libc_errno_dos2kos);
EXPORT(errno_nt2kos,libc_errno_nt2kos);
EXPORT(errno_nt2dos,libc_errno_nt2dos);
EXPORT(errno_kos2dos,libc_errno_kos2dos);
EXPORT(errno_dos2nt,libc_errno_dos2nt);
EXPORT(errno_kos2nt,libc_errno_kos2nt);
EXPORT(_dosmaperr,libc_errno_nt2dos);
EXPORT(__sys_errlist,libc_sys_errlist);
EXPORT(__sys_nerr,libc_sys_nerr);
EXPORT(_get_errno,libc_dos_geterrno2);    /* DOS */
EXPORT(_set_errno,libc_dos_seterrno2);    /* DOS */
EXPORT(_get_doserrno,libc_nt_geterrno2);  /* NT */
EXPORT(_set_doserrno,libc_nt_seterrno2);  /* NT */


DECL_END

#endif /* !GUARD_LIBS_LIBC_ERRNO_C */
