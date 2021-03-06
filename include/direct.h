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
#ifndef _DIRECT_H
#define _DIRECT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifndef _DISKFREE_T_DEFINED
#define _DISKFREE_T_DEFINED 1
struct _diskfree_t {
    __UINT32_TYPE__ total_clusters;
    __UINT32_TYPE__ avail_clusters;
    __UINT32_TYPE__ sectors_per_cluster;
    __UINT32_TYPE__ bytes_per_sector;
};
#endif /* !_DISKFREE_T_DEFINED */

#ifndef __KERNEL__

__REDIRECT_EXCEPT_UFSDPB(__LIBC,__XATTR_RETNONNULL __WUNUSED_SUGGESTED,char *,__LIBCCALL,getcwd,(char *__buf, size_t __size),(__buf,__size))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,chdir,(char const *__path),(__path));
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,rmdir,(char const *__path),(__path));

#ifdef __CRT_DOS
#define _getdcwd_nolock _getdcwd
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__XATTR_RETNONNULL __PORT_NODOS_ALT(getcwd) __WUNUSED_SUGGESTED,char *,__LIBCCALL,getdcwd,(int __drive, char *__buf, size_t __size),(__drive,__buf,__size))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_NODOS,int,__LIBCCALL,chdrive,(int __drive),(__drive))
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__WUNUSED __PORT_NODOS,int,__LIBCCALL,getdrive,(void),())
__REDIRECT_UFSDPB(__LIBC,__WUNUSED __PORT_NODOS,__ULONG32_TYPE__,__LIBCCALL,getdrives,(void),())
#ifndef _GETDISKFREE_DEFINED
#define _GETDISKFREE_DEFINED 1
__LIBC __PORT_NODOS unsigned int (__LIBCCALL _getdiskfree)(unsigned int __drive, struct _diskfree_t *__diskfree);
#endif /* !_GETDISKFREE_DEFINED */
#endif /* __CRT_DOS */

/* A small hand full of functions defined in '<direct.h>' */
#ifndef __getcwd_defined
#define __getcwd_defined 1
__REDIRECT_EXCEPT_UFSDPA(__LIBC,__XATTR_RETNONNULL __WUNUSED_SUGGESTED,char *,__LIBCCALL,getcwd,(char *__buf, size_t __size),(__buf,__size))
#endif /* !__getcwd_defined */
#ifndef __chdir_defined
#define __chdir_defined 1
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,chdir,(char const *__path),(__path))
#endif /* !__chdir_defined */
#ifndef __rmdir_defined
#define __rmdir_defined 1
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,rmdir,(char const *__path),(__path))
#endif /* !__rmdir_defined */

#ifdef __DOS_COMPAT__
__REDIRECT_UFS(__LIBC,__NONNULL((1)),int,__LIBCCALL,_mkdir,(char const *__path),(__path))
#else /* __CRT_DOS && __USE_DOSFS */
__REDIRECT_EXCEPT_UFS_XVOID_(__LIBC,__NONNULL((1)),int,__LIBCCALL,__kos_mkdir,(char const *__path, int __mode),mkdir,(__path,__mode))
#ifdef __USE_EXCEPT_API
__LOCAL __NONNULL((1)) void (__LIBCCALL _mkdir)(char const *__path) { __kos_mkdir(__path,0755); }
#else
__LOCAL __NONNULL((1)) int (__LIBCCALL _mkdir)(char const *__path) { return __kos_mkdir(__path,0755); }
#endif
#endif /* !__CRT_DOS || !__USE_DOSFS */

#ifndef __mkdir_defined
#define __mkdir_defined 1
#ifdef __DOS_COMPAT__
__REDIRECT_UFS_(__LIBC,__NONNULL((1)),int,__LIBCCALL,mkdir,(char const *__path),_mkdir,(__path))
#else /* __USE_DOSFS */
#ifdef __USE_EXCEPT_API
__LOCAL __NONNULL((1)) void (__LIBCCALL mkdir)(char const *__path) { __kos_mkdir(__path,0755); }
#else
__LOCAL __NONNULL((1)) int (__LIBCCALL mkdir)(char const *__path) { return __kos_mkdir(__path,0755); }
#endif
#endif /* !__USE_DOSFS */
#endif /* !__mkdir_defined */
#endif /* !__KERNEL__ */

__SYSDECL_END

#ifdef __USE_DOS
#include "parts/dos/wdirect.h"
#endif /* __USE_DOS */
#ifdef __USE_EXCEPT
#include "parts/kos3/except/direct.h"
#ifdef __USE_DOS
#include "parts/kos3/except/wdirect.h"
#endif /* __USE_DOS */
#endif

#endif /* !_DIRECT_H */
