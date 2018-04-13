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
#ifndef _PARTS_DOS_WIO_H
#define _PARTS_DOS_WIO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef __intptr_t_defined
#define __intptr_t_defined 1
typedef __intptr_t intptr_t;
#endif /* !__intptr_t_defined */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#ifndef _FSIZE_T_DEFINED
#define _FSIZE_T_DEFINED
typedef __UINT32_TYPE__ _fsize_t;
#endif /* _FSIZE_T_DEFINED */

#ifndef _WFINDDATA_T_DEFINED
#define _WFINDDATA_T_DEFINED 1
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("attrib")
#pragma push_macro("time_create")
#pragma push_macro("time_access")
#pragma push_macro("time_write")
#pragma push_macro("size")
#pragma push_macro("name")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

#undef attrib
#undef time_create
#undef time_access
#undef time_write
#undef size
#undef name

struct _wfinddata32_t {
    __UINT32_TYPE__ attrib;
    __time32_t      time_create;
    __time32_t      time_access;
    __time32_t      time_write;
    _fsize_t        size;
    wchar_t         name[260];
};

struct _wfinddata32i64_t {
    __UINT32_TYPE__ attrib;
    __time32_t      time_create;
    __time32_t      time_access;
    __time32_t      time_write;
    __INT64_TYPE__  size;
    wchar_t         name[260];
};

struct _wfinddata64i32_t {
    __UINT32_TYPE__ attrib;
    __time64_t      time_create;
    __time64_t      time_access;
    __time64_t      time_write;
    _fsize_t        size;
    wchar_t         name[260];
};

struct _wfinddata64_t {
    __UINT32_TYPE__ attrib;
    __time64_t      time_create;
    __time64_t      time_access;
    __time64_t      time_write;
    __INT64_TYPE__  size;
    wchar_t         name[260];
};
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("name")
#pragma pop_macro("size")
#pragma pop_macro("time_write")
#pragma pop_macro("time_access")
#pragma pop_macro("time_create")
#pragma pop_macro("attrib")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#ifdef __USE_TIME_BITS64
#define _wfinddata_t    _wfinddata64i32_t
#define _wfindfirst     _wfindfirst64i32
#define _wfindnext      _wfindnext64i32
#define _wfinddatai64_t _wfinddata64_t
#define _wfindfirsti64  _wfindfirst64
#define _wfindnexti64   _wfindnext64
#else /* __USE_TIME_BITS64 */
#define _wfinddata_t    _wfinddata32_t
#define _wfinddatai64_t _wfinddata32i64_t
#define _wfindfirst     _wfindfirst32
#define _wfindnext      _wfindnext32
#define _wfindfirsti64  _wfindfirst32i64
#define _wfindnexti64   _wfindnext32i64
#endif /* !__USE_TIME_BITS64 */
#endif /* !_WFINDDATA_T_DEFINED */


#ifndef _WIO_DEFINED
#define _WIO_DEFINED 1
__VREDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__ATTR_CDECL,wopen,(wchar_t const *__restrict __file, int __oflag, ...),TODO,(__file,__oflag),__oflag)
__VREDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__ATTR_CDECL,wsopen,(wchar_t const *__restrict __file, int __oflag, int __sflag, ...),TODO,(__file,__oflag,__sflag),__sflag)
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,wcreat,(wchar_t const *__restrict __file, int __pmode),(__file,__pmode))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1,2)),errno_t,__LIBCCALL,wsopen_s,(int *__restrict __fd, wchar_t const *__restrict __file, int __oflag, int __sflag, int __pflags),(__fd,__file,__oflag,__sflag,__pflags))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__IF_NUSE_EXCEPT(__WUNUSED) __PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,waccess,(wchar_t const *__restrict __file, int __type),(__file,__type))
__REDIRECT_UFSDPB(__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),errno_t,__LIBCCALL,waccess_s,(wchar_t const *__restrict __file, int __type),(__file,__type))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,wchmod,(wchar_t const *__restrict __file, int __mode),(__file,__mode))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,wunlink,(wchar_t const *__restrict __file),(__file))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,wrename,(wchar_t const *__oldname, wchar_t const *__newname),(__oldname,__newname))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),errno_t,__LIBCCALL,wmktemp_s,(wchar_t *__restrict __templatename, size_t __sizeinwords),(__templatename,__sizeinwords))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,wfindfirst32,(wchar_t const *__restrict __file, struct _wfinddata32_t *__restrict __finddata),(__file,__finddata))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,wfindfirst64,(wchar_t const *__restrict __file, struct _wfinddata64_t *__restrict __finddata),(__file,__finddata))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,wfindfirst32i64,(wchar_t const *__restrict __file, struct _wfinddata32i64_t *__restrict __finddata),(__file,__finddata))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,wfindfirst64i32,(wchar_t const *__restrict __file, struct _wfinddata64i32_t *__restrict __finddata),(__file,__finddata))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((2)),int,__LIBCCALL,wfindnext32,(intptr_t __findfd, struct _wfinddata32_t *__restrict __finddata),(__findfd,__finddata))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((2)),int,__LIBCCALL,wfindnext64,(intptr_t __findfd, struct _wfinddata64_t *__restrict __finddata),(__findfd,__finddata))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((2)),int,__LIBCCALL,wfindnext32i64,(intptr_t __findfd, struct _wfinddata32i64_t *__restrict __finddata),(__findfd,__finddata))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((2)),int,__LIBCCALL,wfindnext64i32,(intptr_t __findfd, struct _wfinddata64i32_t *__restrict __finddata),(__findfd,__finddata))
#endif /* !_WIO_DEFINED */

__SYSDECL_END

#endif /* !_PARTS_DOS_WIO_H */
