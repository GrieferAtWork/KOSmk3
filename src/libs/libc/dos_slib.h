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
#ifndef GUARD_LIBS_LIBC_DOS_SLIB_H
#define GUARD_LIBS_LIBC_DOS_SLIB_H 1

#include "libc.h"
#include "stdio.h"

#ifdef __CC__
DECL_BEGIN

/* strcat_s() */
INTDEF errno_t LIBCCALL libc_strcat_s(char *__restrict dst, size_t dstsize, char const *__restrict src);
INTDEF derrno_t LIBCCALL libc_dos_strcat_s(char *__restrict dst, size_t dstsize, char const *__restrict src);

/* strncat_s() */
INTDEF errno_t LIBCCALL libc_strncat_s(char *__restrict dst, size_t dstsize, char const *__restrict src, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_strncat_s(char *__restrict dst, size_t dstsize, char const *__restrict src, size_t maxlen);

/* strcpy_s() */
INTDEF errno_t LIBCCALL libc_strcpy_s(char *__restrict dst, size_t dstsize, char const *__restrict src);
INTDEF derrno_t LIBCCALL libc_dos_strcpy_s(char *__restrict dst, size_t dstsize, char const *__restrict src);

/* strncpy_s() */
INTDEF errno_t LIBCCALL libc_strncpy_s(char *__restrict dst, size_t dstsize, char const *__restrict src, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_strncpy_s(char *__restrict dst, size_t dstsize, char const *__restrict src, size_t maxlen);

/* wcscat_s() */
INTDEF errno_t LIBCCALL libc_w16cat_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src);
INTDEF errno_t LIBCCALL libc_w32cat_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src);
INTDEF derrno_t LIBCCALL libc_dos_w16cat_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src);
INTDEF derrno_t LIBCCALL libc_dos_w32cat_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src);

/* wcsncat_s() */
INTDEF errno_t LIBCCALL libc_w16ncat_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src, size_t maxlen);
INTDEF errno_t LIBCCALL libc_w32ncat_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_w16ncat_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_w32ncat_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src, size_t maxlen);

/* wcscpy_s() */
INTDEF errno_t LIBCCALL libc_w16cpy_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src);
INTDEF errno_t LIBCCALL libc_w32cpy_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src);
INTDEF derrno_t LIBCCALL libc_dos_w16cpy_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src);
INTDEF derrno_t LIBCCALL libc_dos_w32cpy_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src);

/* wcsncpy_s() */
INTDEF errno_t LIBCCALL libc_w16ncpy_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src, size_t maxlen);
INTDEF errno_t LIBCCALL libc_w32ncpy_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_w16ncpy_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_w32ncpy_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src, size_t maxlen);

/* memcpy_s() */
INTDEF errno_t LIBCCALL libc_memcpy_s(void *__restrict dst, size_t dstsize, void const *__restrict src, size_t srcsize);
INTDEF derrno_t LIBCCALL libc_dos_memcpy_s(void *__restrict dst, size_t dstsize, void const *__restrict src, size_t srcsize);

/* memmove_s() */
INTDEF errno_t LIBCCALL libc_memmove_s(void *dst, size_t dstsize, void const *src, size_t srcsize);
INTDEF derrno_t LIBCCALL libc_dos_memmove_s(void *dst, size_t dstsize, void const *src, size_t srcsize);

/* wmemcpy_s() */
INTDEF errno_t LIBCCALL libc_w16memcpy_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src, size_t srcsize);
INTDEF errno_t LIBCCALL libc_w32memcpy_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src, size_t srcsize);
INTDEF derrno_t LIBCCALL libc_dos_w16memcpy_s(char16_t *__restrict dst, size_t dstsize, char16_t const *__restrict src, size_t srcsize);
INTDEF derrno_t LIBCCALL libc_dos_w32memcpy_s(char32_t *__restrict dst, size_t dstsize, char32_t const *__restrict src, size_t srcsize);

/* wmemmove_s() */
INTDEF errno_t LIBCCALL libc_w16memmove_s(char16_t *dst, size_t dstsize, char16_t const *src, size_t srcsize);
INTDEF errno_t LIBCCALL libc_w32memmove_s(char32_t *dst, size_t dstsize, char32_t const *src, size_t srcsize);
INTDEF derrno_t LIBCCALL libc_dos_w16memmove_s(char16_t *dst, size_t dstsize, char16_t const *src, size_t srcsize);
INTDEF derrno_t LIBCCALL libc_dos_w32memmove_s(char32_t *dst, size_t dstsize, char32_t const *src, size_t srcsize);

/* strset_s() */
INTDEF errno_t LIBCCALL libc_strset_s(char *__restrict str, size_t maxlen, int val);
INTDEF derrno_t LIBCCALL libc_dos_strset_s(char *__restrict str, size_t maxlen, int val);

/* strnset_s() */
INTDEF errno_t LIBCCALL libc_strnset_s(char *__restrict str, size_t buflen, int val, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_strnset_s(char *__restrict str, size_t buflen, int val, size_t maxlen);

/* strlwr_s() */
INTDEF errno_t LIBCCALL libc_strlwr_s(char *__restrict str, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_strlwr_s(char *__restrict str, size_t maxlen);

/* strlwr_s_l() */
INTDEF errno_t LIBCCALL libc_strlwr_s_l(char *__restrict str, size_t maxlen, locale_t locale);
INTDEF derrno_t LIBCCALL libc_dos_strlwr_s_l(char *__restrict str, size_t maxlen, locale_t locale);

/* strupr_s() */
INTDEF errno_t LIBCCALL libc_strupr_s(char *__restrict str, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_strupr_s(char *__restrict str, size_t maxlen);

/* strupr_s_l() */
INTDEF errno_t LIBCCALL libc_strupr_s_l(char *__restrict str, size_t maxlen, locale_t locale);
INTDEF derrno_t LIBCCALL libc_dos_strupr_s_l(char *__restrict str, size_t maxlen, locale_t locale);

/* wcsset_s() */
INTDEF errno_t LIBCCALL libc_w16set_s(char16_t *__restrict str, size_t maxlen, char16_t val);
INTDEF errno_t LIBCCALL libc_w32set_s(char32_t *__restrict str, size_t maxlen, char32_t val);
INTDEF derrno_t LIBCCALL libc_dos_w16set_s(char16_t *__restrict str, size_t maxlen, char16_t val);
INTDEF derrno_t LIBCCALL libc_dos_w32set_s(char32_t *__restrict str, size_t maxlen, char32_t val);

/* wcsnset_s() */
INTDEF errno_t LIBCCALL libc_w16nset_s(char16_t *__restrict str, size_t buflen, char16_t val, size_t maxlen);
INTDEF errno_t LIBCCALL libc_w32nset_s(char32_t *__restrict str, size_t buflen, char32_t val, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_w16nset_s(char16_t *__restrict str, size_t buflen, char16_t val, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_w32nset_s(char32_t *__restrict str, size_t buflen, char32_t val, size_t maxlen);

/* wcslwr_s() */
INTDEF errno_t LIBCCALL libc_w16lwr_s(char16_t *__restrict str, size_t maxlen);
INTDEF errno_t LIBCCALL libc_w32lwr_s(char32_t *__restrict str, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_w16lwr_s(char16_t *__restrict str, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_w32lwr_s(char32_t *__restrict str, size_t maxlen);

/* wcsupr_s() */
INTDEF errno_t LIBCCALL libc_w16upr_s(char16_t *__restrict str, size_t maxlen);
INTDEF errno_t LIBCCALL libc_w32upr_s(char32_t *__restrict str, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_w16upr_s(char16_t *__restrict str, size_t maxlen);
INTDEF derrno_t LIBCCALL libc_dos_w32upr_s(char32_t *__restrict str, size_t maxlen);

/* wcslwr_s_l() */
INTDEF errno_t LIBCCALL libc_w16lwr_s_l(char16_t *__restrict str, size_t maxlen, locale_t locale);
INTDEF errno_t LIBCCALL libc_w32lwr_s_l(char32_t *__restrict str, size_t maxlen, locale_t locale);
INTDEF derrno_t LIBCCALL libc_dos_w16lwr_s_l(char16_t *__restrict str, size_t maxlen, locale_t locale);
INTDEF derrno_t LIBCCALL libc_dos_w32lwr_s_l(char32_t *__restrict str, size_t maxlen, locale_t locale);

/* wcsupr_s_l() */
INTDEF errno_t LIBCCALL libc_w16upr_s_l(char16_t *__restrict str, size_t maxlen, locale_t locale);
INTDEF errno_t LIBCCALL libc_w32upr_s_l(char32_t *__restrict str, size_t maxlen, locale_t locale);
INTDEF derrno_t LIBCCALL libc_dos_w16upr_s_l(char16_t *__restrict str, size_t maxlen, locale_t locale);
INTDEF derrno_t LIBCCALL libc_dos_w32upr_s_l(char32_t *__restrict str, size_t maxlen, locale_t locale);


INTDEF size_t LIBCCALL libc_dos_fread_s(void *__restrict buf, size_t bufsize, size_t elemsize, size_t elemcount, FILE *__restrict stream);
INTDEF size_t LIBCCALL libc_dos_fread_unlocked_s(void *__restrict buf, size_t bufsize, size_t elemsize, size_t elemcount, FILE *__restrict stream);


DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_DOS_SLIB_H */
