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
#ifndef _PARTS_KOS2_UIO_H
#define _PARTS_KOS2_UIO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

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

struct _w16finddata32_t {
    __UINT32_TYPE__ attrib;
    __time32_t      time_create;
    __time32_t      time_access;
    __time32_t      time_write;
    _fsize_t        size;
    char16_t        name[260];
};
struct _w32finddata32_t {
    __UINT32_TYPE__ attrib;
    __time32_t      time_create;
    __time32_t      time_access;
    __time32_t      time_write;
    _fsize_t        size;
    char32_t        name[260];
};
struct _w16finddata32i64_t {
    __UINT32_TYPE__ attrib;
    __time32_t      time_create;
    __time32_t      time_access;
    __time32_t      time_write;
    __INT64_TYPE__  size;
    char16_t        name[260];
};
struct _w32finddata32i64_t {
    __UINT32_TYPE__ attrib;
    __time32_t      time_create;
    __time32_t      time_access;
    __time32_t      time_write;
    __INT64_TYPE__  size;
    char32_t        name[260];
};

struct _w16finddata64i32_t {
    __UINT32_TYPE__ attrib;
    __time64_t      time_create;
    __time64_t      time_access;
    __time64_t      time_write;
    _fsize_t        size;
    char16_t        name[260];
};
struct _w32finddata64i32_t {
    __UINT32_TYPE__ attrib;
    __time64_t      time_create;
    __time64_t      time_access;
    __time64_t      time_write;
    _fsize_t        size;
    char32_t        name[260];
};
struct _w16finddata64_t {
    __UINT32_TYPE__ attrib;
    __time64_t      time_create;
    __time64_t      time_access;
    __time64_t      time_write;
    __INT64_TYPE__  size;
    char16_t        name[260];
};
struct _w32finddata64_t {
    __UINT32_TYPE__ attrib;
    __time64_t      time_create;
    __time64_t      time_access;
    __time64_t      time_write;
    __INT64_TYPE__  size;
    char32_t        name[260];
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
#define _w16finddata_t    _w16finddata64i32_t
#define _w32finddata_t    _w32finddata64i32_t
#define _w16findfirst     _w16findfirst64i32
#define _w32findfirst     _w32findfirst64i32
#define _w16findnext      _w16findnext64i32
#define _w32findnext      _w32findnext64i32
#define _w16finddatai64_t _w16finddata64_t
#define _w32finddatai64_t _w32finddata64_t
#define _w16findfirsti64  _w16findfirst64
#define _w32findfirsti64  _w32findfirst64
#define _w16findnexti64   _w16findnext64
#define _w32findnexti64   _w32findnext64
#else /* __USE_TIME_BITS64 */
#define _w16finddata_t    _w16finddata32_t
#define _w32finddata_t    _w32finddata32_t
#define _w16finddatai64_t _w16finddata32i64_t
#define _w32finddatai64_t _w32finddata32i64_t
#define _w16findfirst     _w16findfirst32
#define _w32findfirst     _w32findfirst32
#define _w16findnext      _w16findnext32
#define _w32findnext      _w32findnext32
#define _w16findfirsti64  _w16findfirst32i64
#define _w32findfirsti64  _w32findfirst32i64
#define _w16findnexti64   _w16findnext32i64
#define _w32findnexti64   _w32findnext32i64
#endif /* !__USE_TIME_BITS64 */

__VREDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),__fd_t,__ATTR_CDECL,_w16open,(char16_t const *__restrict __file, __oflag_t __oflag, ...),wsopen,TODO,(__file,__oflag),__oflag)
__VREDIRECT_EXCEPT_UFSW32  (__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),__fd_t,__ATTR_CDECL,_w32open,(char32_t const *__restrict __file, __oflag_t __oflag, ...),wsopen,TODO,(__file,__oflag),__oflag)
__VREDIRECT_UFSDPW16(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),__fd_t,__ATTR_CDECL,_w16sopen,(char16_t const *__restrict __file, __oflag_t __oflag, int __sflag, ...),wsopen,TODO,(__file,__oflag,__sflag),__sflag)
__VREDIRECT_UFSW32  (__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),__fd_t,__ATTR_CDECL,_w32sopen,(char32_t const *__restrict __file, __oflag_t __oflag, int __sflag, ...),wsopen,TODO,(__file,__oflag,__sflag),__sflag)
__REDIRECT_EXCEPT_UFSDPW16(__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),__fd_t,__LIBCCALL,_w16creat,(char16_t const *__restrict __file, __mode_t __mode),wcreat,(__file,__mode))
__REDIRECT_EXCEPT_UFSW32  (__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),__fd_t,__LIBCCALL,_w32creat,(char32_t const *__restrict __file, __mode_t __mode),wcreat,(__file,__mode))
__REDIRECT_UFSDPW16(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1,2)),errno_t,__LIBCCALL,_w16sopen_s,(__fd_t *__restrict __fd, char16_t const *__restrict __file, __oflag_t __oflag, int __sflag, __mode_t __mode),wsopen_s,(__fd,__file,__oflag,__sflag,__mode))
__REDIRECT_UFSW32  (__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1,2)),errno_t,__LIBCCALL,_w32sopen_s,(__fd_t *__restrict __fd, char32_t const *__restrict __file, __oflag_t __oflag, int __sflag, __mode_t __mode),wsopen_s,(__fd,__file,__oflag,__sflag,__mode))
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__IF_NUSE_EXCEPT(__WUNUSED) __PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,_w16access,(char16_t const *__restrict __file, int __type),waccess,(__file,__type))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__IF_NUSE_EXCEPT(__WUNUSED) __PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,_w32access,(char32_t const *__restrict __file, int __type),waccess,(__file,__type))
__REDIRECT_UFSDPW16(__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),errno_t,__LIBCCALL,_w16access_s,(char16_t const *__restrict __file, int __type),waccess_s,(__file,__type))
__REDIRECT_UFSW32  (__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),errno_t,__LIBCCALL,_w32access_s,(char32_t const *__restrict __file, int __type),waccess_s,(__file,__type))
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,_w16chmod,(char16_t const *__restrict __file, __mode_t __mode),wchmod,(__file,__mode))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,_w32chmod,(char32_t const *__restrict __file, __mode_t __mode),wchmod,(__file,__mode))
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,_w16unlink,(char16_t const *__restrict __file),wunlink,(__file))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,_w32unlink,(char32_t const *__restrict __file),wunlink,(__file))
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,_w16rename,(char16_t const *__oldname, char16_t const *__newname),wrename,(__oldname,__newname))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,_w32rename,(char32_t const *__oldname, char32_t const *__newname),wrename,(__oldname,__newname))
__REDIRECT_UFSDPW16(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),errno_t,__LIBCCALL,_w16mktemp_s,(char16_t *__restrict __templatename, size_t __sizeinwords),wmktemp_s,(__templatename,__sizeinwords))
__REDIRECT_UFSW32  (__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),errno_t,__LIBCCALL,_w32mktemp_s,(char32_t *__restrict __templatename, size_t __sizeinwords),wmktemp_s,(__templatename,__sizeinwords))
__REDIRECT_UFSDPW16(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,_w16findfirst32,(char16_t const *__restrict __file, struct _w16finddata32_t *__restrict __finddata),wfindfirst32,(__file,__finddata))
__REDIRECT_UFSW32  (__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,_w32findfirst32,(char32_t const *__restrict __file, struct _w32finddata32_t *__restrict __finddata),wfindfirst32,(__file,__finddata))
__REDIRECT_UFSDPW16(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,_w16findfirst64,(char16_t const *__restrict __file, struct _w16finddata64_t *__restrict __finddata),wfindfirst64,(__file,__finddata))
__REDIRECT_UFSW32  (__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,_w32findfirst64,(char32_t const *__restrict __file, struct _w32finddata64_t *__restrict __finddata),wfindfirst64,(__file,__finddata))
__REDIRECT_UFSDPW16(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,_w16findfirst32i64,(char16_t const *__restrict __file, struct _w16finddata32i64_t *__restrict __finddata),wfindfirst32i64,(__file,__finddata))
__REDIRECT_UFSW32  (__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,_w32findfirst32i64,(char32_t const *__restrict __file, struct _w32finddata32i64_t *__restrict __finddata),wfindfirst32i64,(__file,__finddata))
__REDIRECT_UFSDPW16(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,_w16findfirst64i32,(char16_t const *__restrict __file, struct _w16finddata64i32_t *__restrict __finddata),wfindfirst64i32,(__file,__finddata))
__REDIRECT_UFSW32  (__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,_w32findfirst64i32,(char32_t const *__restrict __file, struct _w32finddata64i32_t *__restrict __finddata),wfindfirst64i32,(__file,__finddata))
__REDIRECT_DPW16(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((2)),int,__LIBCCALL,_w16findnext32,(intptr_t __findfd, struct _w16finddata32_t *__restrict __finddata),wfindnext32,(__findfd,__finddata))
__REDIRECT_W32  (__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((2)),int,__LIBCCALL,_w32findnext32,(intptr_t __findfd, struct _w32finddata32_t *__restrict __finddata),wfindnext32,(__findfd,__finddata))
__REDIRECT_DPW16(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((2)),int,__LIBCCALL,_w16findnext64,(intptr_t __findfd, struct _w16finddata64_t *__restrict __finddata),wfindnext64,(__findfd,__finddata))
__REDIRECT_W32  (__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((2)),int,__LIBCCALL,_w32findnext64,(intptr_t __findfd, struct _w32finddata64_t *__restrict __finddata),wfindnext64,(__findfd,__finddata))
__REDIRECT_DPW16(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((2)),int,__LIBCCALL,_w16findnext32i64,(intptr_t __findfd, struct _w16finddata32i64_t *__restrict __finddata),wfindnext32i64,(__findfd,__finddata))
__REDIRECT_W32  (__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((2)),int,__LIBCCALL,_w32findnext32i64,(intptr_t __findfd, struct _w32finddata32i64_t *__restrict __finddata),wfindnext32i64,(__findfd,__finddata))
__REDIRECT_DPW16(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((2)),int,__LIBCCALL,_w16findnext64i32,(intptr_t __findfd, struct _w16finddata64i32_t *__restrict __finddata),wfindnext64i32,(__findfd,__finddata))
__REDIRECT_W32  (__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((2)),int,__LIBCCALL,_w32findnext64i32,(intptr_t __findfd, struct _w32finddata64i32_t *__restrict __finddata),wfindnext64i32,(__findfd,__finddata))

__SYSDECL_END

#ifdef __USE_EXCEPT
#include <parts/kos3/except/uio.h>
#endif /* __USE_EXCEPT */


#endif /* !_PARTS_KOS2_UIO_H */
