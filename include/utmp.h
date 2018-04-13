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
#ifndef _UTMP_H
#define _UTMP_H 1

#include <__stdinc.h>
#include <features.h>
#include <sys/types.h>
#include <bits/utmp.h>

#ifndef __CRT_GLC
#error "<utmp.h> is not supported by the linked libc"
#endif

__SYSDECL_BEGIN

/* Compatibility names for the strings of the canonical file names.  */
#define UTMP_FILE      _PATH_UTMP
#define UTMP_FILENAME  _PATH_UTMP
#define WTMP_FILE      _PATH_WTMP
#define WTMP_FILENAME  _PATH_WTMP

#ifdef __CC__
__LIBC int (__LIBCCALL login_tty)(__fd_t __fd);
__LIBC void (__LIBCCALL login)(struct utmp const *__entry);
__LIBC int (__LIBCCALL logout)(char const *__ut_line);
__LIBC void (__LIBCCALL logwtmp)(char const *__ut_line, char const *__ut_name, char const *__ut_host);
__LIBC void (__LIBCCALL updwtmp)(char const *__wtmp_file, struct utmp const *__utmp);
__LIBC int (__LIBCCALL utmpname)(char const *__file);
__LIBC struct utmp *(__LIBCCALL getutent)(void);
__LIBC void (__LIBCCALL setutent)(void);
__LIBC void (__LIBCCALL endutent)(void);
__LIBC struct utmp *(__LIBCCALL getutid)(struct utmp const *__id);
__LIBC struct utmp *(__LIBCCALL getutline)(struct utmp const *__line);
__LIBC struct utmp *(__LIBCCALL pututline)(struct utmp const *__utmp_ptr);
#ifdef __USE_MISC
__LIBC int (__LIBCCALL getutent_r)(struct utmp *__buffer, struct utmp **__result);
__LIBC int (__LIBCCALL getutid_r)(struct utmp const *__id, struct utmp *__buffer, struct utmp **__result);
__LIBC int (__LIBCCALL getutline_r)(struct utmp const *__line, struct utmp *__buffer, struct utmp **__result);
#endif /* __USE_MISC */
#endif /* __CC__ */

__SYSDECL_END

#endif /* !_UTMP_H */
