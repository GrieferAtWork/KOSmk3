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
#ifndef GUARD_HYBRID_STRING_C
#define GUARD_HYBRID_STRING_C 1
#define _KOS_SOURCE 2

#include "hybrid.h"
#include <hybrid/compiler.h>
#include <hybrid/asm.h>
#include <kos/types.h>
#include <hybrid/byteswap.h>
#include <assert.h>

#define CRT_STRING \
  INTERN ATTR_WEAK ATTR_SECTION(".text.crt.string")

#ifndef __INTELLISENSE__
#ifndef CONFIG_NO_64BIT_STRING
#define SIZE 8
#include "memory-template.c.inl"
#endif /* !CONFIG_NO_64BIT_STRING */
#define SIZE 4
#include "memory-template.c.inl"
#define SIZE 2
#include "memory-template.c.inl"
#define SIZE 1
#include "memory-template.c.inl"


#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR
#include "string-template.c.inl"
#ifndef CONFIG_LIBC_LIMITED_API
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#include "string-template.c.inl"
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR32
#include "string-template.c.inl"
#endif /* !CONFIG_LIBC_LIMITED_API */

#endif


DECL_BEGIN

EXPORT(memcasecmp,libc_memcasecmp);
CRT_STRING int LIBCCALL
libc_memcasecmp(u8 const *a, u8 const *b, size_t num_bytes) {
 while (num_bytes--) {
  u8 ca = *a++,cb = *b++;
  /* Quick check: do they match before we transform their casing? */
  if (ca == cb) continue;
  ca = libc_tolower(ca);
  cb = libc_tolower(cb);
  /* Check again now that casing has been transformed. */
  if (ca == cb) continue;
  /* Return the difference. */
  return (int)ca - (int)cb;
 }
 return 0;
}

#ifndef CONFIG_LIBC_LIMITED_API
EXPORT(swab,libc_swab);
CRT_STRING void LIBCCALL
libc_swab(void const *__restrict from,
          void *__restrict to,
          int n_bytes) {
 u16 *dst,*src;
 if (n_bytes <= 0) return;
 n_bytes >>= 1;
 dst = (u16 *)to;
 src = (u16 *)from;
 while (n_bytes--) {
  *dst = BSWAP16(*src);
  ++dst;
  ++src;
 }
}

EXPORT(memcasecmp_l,libc_memcasecmp_l);
CRT_STRING int LIBCCALL
libc_memcasecmp_l(u8 const *a, u8 const *b, size_t num_bytes, locale_t locale) {
 while (num_bytes--) {
  u8 ca = *a++,cb = *b++;
  /* Quick check: do they match before we transform their casing? */
  if (ca == cb) continue;
  ca = libc_tolower_l(ca,locale);
  cb = libc_tolower_l(cb,locale);
  /* Check again now that casing has been transformed. */
  if (ca == cb) continue;
  /* Return the difference. */
  return (int)ca - (int)cb;
 }
 return 0;
}


EXPORT(memmem,libc_memmem);
CRT_STRING void *LIBCCALL
libc_memmem(void *haystack, size_t haystacklen,
            void *needle, size_t needlelen) {
 byte_t *iter,*end;
 if unlikely(needlelen > haystacklen) return NULL;
 end = (iter = (byte_t *)haystack)+(haystacklen-needlelen);
 for (;;) {
  if (libc_memcmp(iter,needle,needlelen) == 0)
      return iter;
  if (iter == end) break;
  ++iter;
 }
 return NULL;
}

EXPORT(memcasemem,libc_memcasemem);
CRT_STRING void *LIBCCALL
libc_memcasemem(void *haystack, size_t haystacklen,
                void *needle, size_t needlelen) {
 byte_t *iter,*end;
 if unlikely(needlelen > haystacklen) return NULL;
 end = (iter = (byte_t *)haystack)+(haystacklen-needlelen);
 for (;;) {
  if (libc_memcasecmp(iter,needle,needlelen) == 0)
      return iter;
  if (iter == end) break;
  ++iter;
 }
 return NULL;
}

EXPORT(memcasemem_l,libc_memcasemem_l);
CRT_STRING void *LIBCCALL
libc_memcasemem_l(void *haystack, size_t haystacklen,
                  void *needle, size_t needlelen,
                  locale_t locale) {
 byte_t *iter,*end;
 if unlikely(needlelen > haystacklen) return NULL;
 end = (iter = (byte_t *)haystack)+(haystacklen-needlelen);
 for (;;) {
  if (libc_memcasecmp_l(iter,needle,needlelen,locale) == 0)
      return iter;
  if (iter == end) break;
  ++iter;
 }
 return NULL;
}


EXPORT(bcopy,libc_bcopy);
CRT_STRING void LIBCCALL
libc_bcopy(void const *src, void *dst, size_t n) {
 libc_memmove((u8 *)dst,(u8 *)src,n);
}

EXPORT(bzero,libc_bzero);
CRT_STRING void LIBCCALL
libc_bzero(void *__restrict s, size_t n) {
 libc_memset((u8 *)s,0,n);
}

EXPORT(dirname,libc_dirname);
CRT_STRING char *LIBCCALL
libc_dirname(char *path) {
 char *iter;
 if (!path || !*path) ret_cwd: return ".";
 iter = libc_strend(path)-1;
 while (*iter == '/') {
  if (iter == path) { iter[1] = '\0'; return path; }
  --iter;
 }
 while (iter >= path && *iter != '/') --iter;
 if (iter < path) goto ret_cwd;
 if (iter == path) ++iter;
 *iter = '\0';
 return path;
}

EXPORT(__xpg_basename,libc_xpg_basename);
CRT_STRING char *LIBCCALL
libc_xpg_basename(char *path) {
 assertf(0,"TODO");
 return NULL;
}

EXPORT(basename,libc_basename);
CRT_STRING char *LIBCCALL
libc_basename(char const *__restrict path) {
 char ch,*iter = (char *)path,*result = NULL;
 if (!path || !*path) return ".";
 do if ((ch = *iter++) == '/') result = iter;
 while (ch);
 if unlikely(!result) return (char *)path; /* Path doesn't contain '/'. */
 if (*result) return result; /* Last character isn't a '/'. */
 iter = result;
 while (iter != path && iter[-1] == '/') --iter;
 if (iter == path) return result-1; /* Only `'/'"-characters. */
 *iter = '\0'; /* Trim all ending `'/'"-characters. */
 while (iter != path && iter[-1] != '/') --iter; /* Scan until the previous '/'. */
 return iter; /* Returns string after previous '/'. */
}

EXPORT(memccpy,libc_memccpy);
CRT_STRING void *LIBCCALL
libc_memccpy(void *__restrict dst,
             void const *__restrict src,
             int c, size_t n) {
 byte_t *dst_iter,*end;
 byte_t const *src_iter;
 end = (dst_iter = (byte_t *)dst)+n;
 src_iter = (byte_t const *)src;
 while (dst_iter != end) {
  if ((*dst_iter++ = *src_iter++) == c)
        return dst_iter;
 }
 return NULL;
}

EXPORT(memfrob,libc_memfrob);
INTERN u8 *LIBCCALL
libc_memfrob(u8 *__restrict s, size_t n) {
 u8 *iter = s;
 while (n--) *iter++ ^= 42; /* -_-   yeah... */
 return s;
}


#endif /* !CONFIG_LIBC_LIMITED_API */



#ifndef CONFIG_LIBC_LIMITED_API

/* Export wide-character string functions. */
EXPORT(__SYMw16(wmemchr), libc_memchrw);
EXPORT(__SYMw32(wmemchr), libc_memchrl);
EXPORT(__SYMw16(wmemcmp), libc_memcmpw);
EXPORT(__SYMw32(wmemcmp), libc_memcmpl);
EXPORT(__SYMw16(wmemcpy), libc_memcpyw);
EXPORT(__SYMw32(wmemcpy), libc_memcpyl);
EXPORT(__SYMw16(wmemmove),libc_memmovew);
EXPORT(__SYMw32(wmemmove),libc_memmovel);
EXPORT(__SYMw16(wmempcpy),libc_mempcpyw);
EXPORT(__SYMw32(wmempcpy),libc_mempcpyl);
EXPORT(__SYMw16(wmemset), libc_memsetw);
EXPORT(__SYMw32(wmemset), libc_memsetl);


/* Special handling for exporting `strtok()' and `strtok_r()' */
EXPORT(strtok,libc_strtok);
EXPORT(strtok_r,libc_strtok_r);
EXPORT(strtok_s,libc_strtok_r); /* DOS's name for `strtok_r()' */
EXPORT(__SYMw32(wcstok),libc_w32tok_r);
EXPORT(__SYMw32(__wcstok_f),libc_w32tok_r);
EXPORT(__DSYMw16(wcstok),libc_w16tok); /* This one's a workaround for a bug in DOS. */
EXPORT(__DSYMw32(wcstok),libc_w32tok); /* This one's a workaround for a bug in DOS. */
EXPORT(__SYMw16(wcstok_s),libc_w16tok_r);
EXPORT(__SYMw32(wcstok_s),libc_w32tok_r);


/* DOS Symbols */
EXPORT(_stricmp,libc_strcasecmp);
EXPORT(_stricmp_l,libc_strcasecmp_l);
EXPORT(_strnicmp,libc_strncasecmp);
EXPORT(_strnicmp_l,libc_strncasecmp_l);
EXPORT(_swab,libc_swab);
EXPORT(_memicmp,libc_memcasecmp);
EXPORT(_memicmp_l,libc_memcasecmp_l);
EXPORT(_strlwr,libc_strlwr);
EXPORT(_strupr,libc_strupr);
EXPORT(_strlwr_l,libc_strlwr_l);
EXPORT(_strupr_l,libc_strupr_l);
EXPORT(_strrev,libc_strrev);
EXPORT(_strset,libc_strset);
EXPORT(_strnset,libc_strnset);
EXPORT(_strxfrm_l,libc_strxfrm_l);
EXPORT(_strcoll_l,libc_strcoll_l);
EXPORT(_stricoll,libc_strcasecoll);
EXPORT(_stricoll_l,libc_strcasecoll_l);
EXPORT(_strncoll,libc_strncoll);
EXPORT(_strncoll_l,libc_strncoll_l);
EXPORT(_strnicoll,libc_strncasecoll);
EXPORT(_strnicoll_l,libc_strncasecoll_l);
EXPORT(__SYMw16(_wcsicmp),libc_w16casecmp);
EXPORT(__SYMw32(_wcsicmp),libc_w32casecmp);
EXPORT(__SYMw16(_wcsicmp_l),libc_w16casecmp_l);
EXPORT(__SYMw32(_wcsicmp_l),libc_w32casecmp_l);
EXPORT(__SYMw16(_wcscoll_l),libc_w16coll_l);
EXPORT(__SYMw32(_wcscoll_l),libc_w32coll_l);
EXPORT(__SYMw16(_wcsicoll),libc_w16casecoll);
EXPORT(__SYMw32(_wcsicoll),libc_w32casecoll);
EXPORT(__SYMw16(_wcsicoll_l),libc_w16casecoll_l);
EXPORT(__SYMw32(_wcsicoll_l),libc_w32casecoll_l);
EXPORT(__SYMw16(_wcslwr),libc_w16lwr);
EXPORT(__SYMw32(_wcslwr),libc_w32lwr);
EXPORT(__SYMw16(_wcslwr_l),libc_w16lwr_l);
EXPORT(__SYMw32(_wcslwr_l),libc_w32lwr_l);
EXPORT(__SYMw16(_wcsupr),libc_w16upr);
EXPORT(__SYMw32(_wcsupr),libc_w32upr);
EXPORT(__SYMw16(_wcsupr_l),libc_w16upr_l);
EXPORT(__SYMw32(_wcsupr_l),libc_w32upr_l);
EXPORT(__SYMw16(_wcsnset),libc_w16nset);
EXPORT(__SYMw32(_wcsnset),libc_w32nset);
EXPORT(__SYMw16(_wcsrev),libc_w16rev);
EXPORT(__SYMw32(_wcsrev),libc_w32rev);
EXPORT(__SYMw16(_wcsset),libc_w16set);
EXPORT(__SYMw32(_wcsset),libc_w32set);
EXPORT(__SYMw16(_wcsnicmp),libc_w16ncasecmp);
EXPORT(__SYMw32(_wcsnicmp),libc_w32ncasecmp);
EXPORT(__SYMw16(_wcsnicmp_l),libc_w16ncasecmp_l);
EXPORT(__SYMw32(_wcsnicmp_l),libc_w32ncasecmp_l);
EXPORT(__SYMw16(_wcsncoll),libc_w16ncoll);
EXPORT(__SYMw32(_wcsncoll),libc_w32ncoll);
EXPORT(__SYMw16(_wcsncoll_l),libc_w16ncoll_l);
EXPORT(__SYMw32(_wcsncoll_l),libc_w32ncoll_l);
EXPORT(__SYMw16(_wcsnicoll),libc_w16ncasecoll);
EXPORT(__SYMw32(_wcsnicoll),libc_w32ncasecoll);
EXPORT(__SYMw16(_wcsnicoll_l),libc_w16ncasecoll_l);
EXPORT(__SYMw32(_wcsnicoll_l),libc_w32ncasecoll_l);
EXPORT(__SYMw16(_wcsxfrm_l),libc_w16xfrm_l);
EXPORT(__SYMw32(_wcsxfrm_l),libc_w32xfrm_l);


/* String --> integer aliases. */
EXPORT(strtoq,libc_strto64);
EXPORT(strtoq_l,libc_strto64_l);
EXPORT(strtouq,libc_strtou64);
EXPORT(strtouq_l,libc_strtou64_l);
EXPORT(__SYMw16(wcstoq),libc_w16to64);
EXPORT(__SYMw32(wcstoq),libc_w32to64);
EXPORT(__SYMw16(wcstoq_l),libc_w16to64_l);
EXPORT(__SYMw32(wcstoq_l),libc_w32to64_l);
EXPORT(__SYMw16(wcstouq),libc_w16tou64);
EXPORT(__SYMw32(wcstouq),libc_w32tou64);
EXPORT(__SYMw16(wcstouq_l),libc_w16tou64_l);
EXPORT(__SYMw32(wcstouq_l),libc_w32tou64_l);

#if __SIZEOF_LONG_LONG__ > 4
EXPORT(atoll,libc_ato64);
EXPORT(atoll_l,libc_ato64_l);
EXPORT(atoull,libc_atou64);
EXPORT(atoull_l,libc_atou64_l);
EXPORT(__SYMw16(wtoll),libc_aw16to64);
EXPORT(__SYMw32(wtoll),libc_aw32to64);
EXPORT(__SYMw16(wtoll_l),libc_aw16to64_l);
EXPORT(__SYMw32(wtoll_l),libc_aw32to64_l);
EXPORT(__SYMw16(wtoull),libc_aw16tou64);
EXPORT(__SYMw32(wtoull),libc_aw32tou64);
EXPORT(__SYMw16(wtoull_l),libc_aw16tou64_l);
EXPORT(__SYMw32(wtoull_l),libc_aw32tou64_l);
EXPORT(strtoll,libc_strto64);
EXPORT(strtoll_l,libc_strto64_l);
EXPORT(strtoull,libc_strtou64);
EXPORT(strtoull_l,libc_strtou64_l);
EXPORT(__SYMw16(wcstoll),libc_w16to64);
EXPORT(__SYMw32(wcstoll),libc_w32to64);
EXPORT(__SYMw16(wcstoll_l),libc_w16to64_l);
EXPORT(__SYMw32(wcstoll_l),libc_w32to64_l);
EXPORT(__SYMw16(wcstoull),libc_w16tou64);
EXPORT(__SYMw32(wcstoull),libc_w32tou64);
EXPORT(__SYMw16(wcstoull_l),libc_w16tou64_l);
EXPORT(__SYMw32(wcstoull_l),libc_w32tou64_l);
#else
EXPORT(__KSYM(atoll),libc_ato32);
EXPORT(__DSYM(atoll),libc_ato64);
EXPORT(__KSYM(atoll_l),libc_ato32_l);
EXPORT(__DSYM(atoll_l),libc_ato64_l);
EXPORT(__KSYM(atoull),libc_atou32);
EXPORT(__DSYM(atoull),libc_atou64);
EXPORT(__KSYM(atoull_l),libc_atou32_l);
EXPORT(__DSYM(atoull_l),libc_atou64_l);
EXPORT(__KSYMw16(wtoll),libc_aw16to32);
EXPORT(__DSYMw16(wtoll),libc_aw16to64);
EXPORT(__KSYMw32(wtoll),libc_aw32to32);
EXPORT(__DSYMw32(wtoll),libc_aw32to64);
EXPORT(__KSYMw16(wtoll_l),libc_aw16to32_l);
EXPORT(__DSYMw16(wtoll_l),libc_aw16to64_l);
EXPORT(__KSYMw32(wtoll_l),libc_aw32to32_l);
EXPORT(__DSYMw32(wtoll_l),libc_aw32to64_l);
EXPORT(__KSYMw16(wtoull),libc_aw16tou32);
EXPORT(__DSYMw16(wtoull),libc_aw16tou64);
EXPORT(__KSYMw32(wtoull),libc_aw32tou32);
EXPORT(__DSYMw32(wtoull),libc_aw32tou64);
EXPORT(__KSYMw16(wtoull_l),libc_aw16tou32_l);
EXPORT(__DSYMw16(wtoull_l),libc_aw16tou64_l);
EXPORT(__KSYMw32(wtoull_l),libc_aw32tou32_l);
EXPORT(__DSYMw32(wtoull_l),libc_aw32tou64_l);
EXPORT(__KSYM(strtoll),libc_strto32);
EXPORT(__DSYM(strtoll),libc_strto64);
EXPORT(__KSYM(strtoll_l),libc_strto32_l);
EXPORT(__DSYM(strtoll_l),libc_strto64_l);
EXPORT(__KSYM(strtoull),libc_strtou32);
EXPORT(__DSYM(strtoull),libc_strtou64);
EXPORT(__KSYM(strtoull_l),libc_strtou32_l);
EXPORT(__DSYM(strtoull_l),libc_strtou64_l);
EXPORT(__KSYMw16(wcstoll),libc_w16to32);
EXPORT(__DSYMw16(wcstoll),libc_w16to64);
EXPORT(__KSYMw32(wcstoll),libc_w32to32);
EXPORT(__DSYMw32(wcstoll),libc_w32to64);
EXPORT(__KSYMw16(wcstoll_l),libc_w16to32_l);
EXPORT(__DSYMw16(wcstoll_l),libc_w16to64_l);
EXPORT(__KSYMw32(wcstoll_l),libc_w32to32_l);
EXPORT(__DSYMw32(wcstoll_l),libc_w32to64_l);
EXPORT(__KSYMw16(wcstoull),libc_w16tou32);
EXPORT(__DSYMw16(wcstoull),libc_w16tou64);
EXPORT(__KSYMw32(wcstoull),libc_w32tou32);
EXPORT(__DSYMw32(wcstoull),libc_w32tou64);
EXPORT(__KSYMw16(wcstoull_l),libc_w16tou32_l);
EXPORT(__DSYMw16(wcstoull_l),libc_w16tou64_l);
EXPORT(__KSYMw32(wcstoull_l),libc_w32tou32_l);
EXPORT(__DSYMw32(wcstoull_l),libc_w32tou64_l);
#endif

#if __SIZEOF_LONG__ > 4
EXPORT(__KSYM(strtol),libc_strto64);
EXPORT(__KSYM(strtol_l),libc_strto64_l);
EXPORT(__KSYM(strtoul),libc_strtou64);
EXPORT(__KSYM(strtoul_l),libc_strtou64_l);
EXPORT(__KSYMw16(wcstol),libc_w16to64);
EXPORT(__KSYMw32(wcstol),libc_w32to64);
EXPORT(__KSYMw16(wcstol_l),libc_w16to64_l);
EXPORT(__KSYMw32(wcstol_l),libc_w32to64_l);
EXPORT(__KSYMw16(wcstoul),libc_w16tou64);
EXPORT(__KSYMw32(wcstoul),libc_w32tou64);
EXPORT(__KSYMw16(wcstoul_l),libc_w16tou64_l);
EXPORT(__KSYMw32(wcstoul_l),libc_w32tou64_l);
/* On DOS, `long' is always 32 bits wide. */
EXPORT(__DSYM(strtol),libc_strto32);
EXPORT(__DSYM(strtol_l),libc_strto32_l);
EXPORT(__DSYM(strtoul),libc_strtou32);
EXPORT(__DSYM(strtoul_l),libc_strtou32_l);
EXPORT(__DSYMw16(wcstol),libc_w16to32);
EXPORT(__DSYMw32(wcstol),libc_w32to32);
EXPORT(__DSYMw16(wcstol_l),libc_w16to32_l);
EXPORT(__DSYMw32(wcstol_l),libc_w32to32_l);
EXPORT(__DSYMw16(wcstoul),libc_w16tou32);
EXPORT(__DSYMw32(wcstoul),libc_w32tou32);
EXPORT(__DSYMw16(wcstoul_l),libc_w16tou32_l);
EXPORT(__DSYMw32(wcstoul_l),libc_w32tou32_l);
EXPORT(__KSYM(atol),libc_ato64);
EXPORT(__KSYM(atol_l),libc_ato64_l);
EXPORT(__KSYM(atoul),libc_atou64);
EXPORT(__KSYM(atoul_l),libc_atou64_l);
EXPORT(__KSYMw16(wtol),libc_aw16to64);
EXPORT(__KSYMw32(wtol),libc_aw32to64);
EXPORT(__KSYMw16(wtol_l),libc_aw16to64_l);
EXPORT(__KSYMw32(wtol_l),libc_aw32to64_l);
EXPORT(__KSYMw16(wtoul),libc_aw16tou64);
EXPORT(__KSYMw32(wtoul),libc_aw32tou64);
EXPORT(__KSYMw16(wtoul_l),libc_aw16tou64_l);
EXPORT(__KSYMw32(wtoul_l),libc_aw32tou64_l);
/* On DOS, `long' is always 32 bits wide. */
EXPORT(__DSYM(atol),libc_ato32);
EXPORT(__DSYM(atol_l),libc_ato32_l);
EXPORT(__DSYM(atoul),libc_atou32);
EXPORT(__DSYM(atoul_l),libc_atou32_l);
EXPORT(__DSYMw16(wtol),libc_aw16to32);
EXPORT(__DSYMw32(wtol),libc_aw32to32);
EXPORT(__DSYMw16(wtol_l),libc_aw16to32_l);
EXPORT(__DSYMw32(wtol_l),libc_aw32to32_l);
EXPORT(__DSYMw16(wtoul),libc_aw16tou32);
EXPORT(__DSYMw32(wtoul),libc_aw32tou32);
EXPORT(__DSYMw16(wtoul_l),libc_aw16tou32_l);
EXPORT(__DSYMw32(wtoul_l),libc_aw32tou32_l);
#else
EXPORT(strtol,libc_strto32);
EXPORT(strtol_l,libc_strto32_l);
EXPORT(strtoul,libc_strtou32);
EXPORT(strtoul_l,libc_strtou32_l);
EXPORT(__SYMw16(wcstol),libc_w16to32);
EXPORT(__SYMw32(wcstol),libc_w32to32);
EXPORT(__SYMw16(wcstol_l),libc_w16to32_l);
EXPORT(__SYMw32(wcstol_l),libc_w32to32_l);
EXPORT(__SYMw16(wcstoul),libc_w16tou32);
EXPORT(__SYMw32(wcstoul),libc_w32tou32);
EXPORT(__SYMw16(wcstoul_l),libc_w16tou32_l);
EXPORT(__SYMw32(wcstoul_l),libc_w32tou32_l);
EXPORT(atol,libc_ato32);
EXPORT(atol_l,libc_ato32_l);
EXPORT(atoul,libc_atou32);
EXPORT(atoul_l,libc_atou32_l);
EXPORT(__SYMw16(wtol),libc_aw16to32);
EXPORT(__SYMw32(wtol),libc_aw32to32);
EXPORT(__SYMw16(wtol_l),libc_aw16to32_l);
EXPORT(__SYMw32(wtol_l),libc_aw32to32_l);
EXPORT(__SYMw16(wtoul),libc_aw16tou32);
EXPORT(__SYMw32(wtoul),libc_aw32tou32);
EXPORT(__SYMw16(wtoul_l),libc_aw16tou32_l);
EXPORT(__SYMw32(wtoul_l),libc_aw32tou32_l);
#endif

#if __SIZEOF_INTMAX_T__ > 4
EXPORT(strtoimax,libc_strto64);
EXPORT(strtoumax,libc_strtou64);
EXPORT(strtoimax_l,libc_strto64_l);
EXPORT(strtoumax_l,libc_strtou64_l);
EXPORT(__SYMw16(wcstoimax),libc_w16to64);
EXPORT(__SYMw32(wcstoimax),libc_w32to64);
EXPORT(__SYMw16(wcstoumax),libc_w16tou64);
EXPORT(__SYMw32(wcstoumax),libc_w32tou64);
EXPORT(__SYMw16(wcstoimax_l),libc_w16to64_l);
EXPORT(__SYMw32(wcstoimax_l),libc_w32to64_l);
EXPORT(__SYMw16(wcstoumax_l),libc_w16tou64_l);
EXPORT(__SYMw32(wcstoumax_l),libc_w32tou64_l);
#else
EXPORT(strtoimax,libc_strto32);
EXPORT(strtoumax,libc_strtou32);
EXPORT(strtoimax_l,libc_strto32_l);
EXPORT(strtoumax_l,libc_strtou32_l);
EXPORT(__SYMw16(wcstoimax),libc_w16to32);
EXPORT(__SYMw32(wcstoimax),libc_w32to32);
EXPORT(__SYMw16(wcstoumax),libc_w16tou32);
EXPORT(__SYMw32(wcstoumax),libc_w32tou32);
EXPORT(__SYMw16(wcstoimax_l),libc_w16to32_l);
EXPORT(__SYMw32(wcstoimax_l),libc_w32to32_l);
EXPORT(__SYMw16(wcstoumax_l),libc_w16tou32_l);
EXPORT(__SYMw32(wcstoumax_l),libc_w32tou32_l);
#endif

#if __SIZEOF_INT__ > 4
EXPORT(__KSYM(_atoi_l),libc_ato64_l);
EXPORT(__DSYM(_atoi_l),libc_ato32_l);
EXPORT(__KSYM(atoi),libc_ato64);
EXPORT(__KSYM(atoi_l),libc_ato64_l);
EXPORT(__DSYM(atoi),libc_ato32);
EXPORT(__DSYM(atoi_l),libc_ato32_l);
EXPORT(__KSYMw16(wtoi),libc_aw16to64);
EXPORT(__DSYMw16(wtoi),libc_aw16to32);
EXPORT(__KSYMw16(wtoi_l),libc_aw16to64_l);
EXPORT(__DSYMw16(wtoi_l),libc_aw16to32_l);
EXPORT(__KSYMw32(wtoi),libc_aw32to64);
EXPORT(__DSYMw32(wtoi),libc_aw32to32);
EXPORT(__KSYMw32(wtoi_l),libc_aw32to64_l);
EXPORT(__DSYMw32(wtoi_l),libc_aw32to32_l);
EXPORT(__KSYMw16(_wtoi),libc_aw16to64);
EXPORT(__DSYMw16(_wtoi),libc_aw16to32);
EXPORT(__KSYMw16(_wtoi_l),libc_aw16to64_l);
EXPORT(__DSYMw16(_wtoi_l),libc_aw16to32_l);
EXPORT(__KSYMw32(_wtoi),libc_aw32to64);
EXPORT(__DSYMw32(_wtoi),libc_aw32to32);
EXPORT(__KSYMw32(_wtoi_l),libc_aw32to64_l);
EXPORT(__DSYMw32(_wtoi_l),libc_aw32to32_l);
#else
EXPORT(_atoi_l,libc_ato32_l);
EXPORT(atoi,libc_ato32);
EXPORT(atoi_l,libc_ato32_l);
EXPORT(__SYMw16(wtoi),libc_aw16to32);
EXPORT(__SYMw32(wtoi),libc_aw32to32);
EXPORT(__SYMw16(wtoi_l),libc_aw16to32_l);
EXPORT(__SYMw32(wtoi_l),libc_aw32to32_l);
EXPORT(__SYMw16(_wtoi),libc_aw16to32);
EXPORT(__SYMw32(_wtoi),libc_aw32to32);
EXPORT(__SYMw16(_wtoi_l),libc_aw16to32_l);
EXPORT(__SYMw32(_wtoi_l),libc_aw32to32_l);
#endif

/* Work around the long-double clutch present in DOS. */
EXPORT(__KSYM(_strtold_l),libc_strtold_l);
EXPORT(__DSYM(_strtold_l),libc_strtod_l);
EXPORT(__KSYM(strtold),libc_strtold);
EXPORT(__DSYM(strtold),libc_strtod);
EXPORT(__KSYM(strtold_l),libc_strtold_l);
EXPORT(__DSYM(strtold_l),libc_strtod_l);
EXPORT(__KSYMw16(wcstold),libc_w16told);
EXPORT(__KSYMw32(wcstold),libc_w32told);
EXPORT(__DSYMw16(wcstold),libc_w16tod);
EXPORT(__DSYMw32(wcstold),libc_w32tod);
EXPORT(__KSYMw16(wcstold_l),libc_w16told_l);
EXPORT(__KSYMw32(wcstold_l),libc_w32told_l);
EXPORT(__DSYMw16(wcstold_l),libc_w16tod_l);
EXPORT(__DSYMw32(wcstold_l),libc_w32tod_l);
EXPORT(__KSYM(atold),libc_atold);
EXPORT(__DSYM(atold),libc_atod);
EXPORT(__KSYM(atold_l),libc_atold_l);
EXPORT(__DSYM(atold_l),libc_atod_l);
EXPORT(__KSYMw16(wtold),libc_aw16told);
EXPORT(__KSYMw32(wtold),libc_aw32told);
EXPORT(__DSYMw16(wtold),libc_aw16tod);
EXPORT(__DSYMw32(wtold),libc_aw32tod);
EXPORT(__KSYMw16(wtold_l),libc_aw16told_l);
EXPORT(__KSYMw32(wtold_l),libc_aw32told_l);
EXPORT(__DSYMw16(wtold_l),libc_aw16tod_l);
EXPORT(__DSYMw32(wtold_l),libc_aw32tod_l);

/* DOS Aliases for some functions. */
EXPORT(_strtof_l,libc_strtof_l);
EXPORT(_strtoimax_l,libc_strto64_l);
EXPORT(_strtoumax_l,libc_strtou64_l);
EXPORT(_strtod_l,libc_strtod_l);
EXPORT(_strtoi64,libc_strto64);
EXPORT(_strtoi64_l,libc_strto64_l);
EXPORT(_strtoui64,libc_strtou64);
EXPORT(_strtoui64_l,libc_strtou64_l);
EXPORT(_atodbl,libc_atod);
EXPORT(_atodbl_l,libc_atod_l);
EXPORT(_atof,libc_atof);
EXPORT(_atof_l,libc_atof_l);
EXPORT(_atoflt,libc_atof);
EXPORT(_atoflt_l,libc_atof_l);
EXPORT(__KSYM(_atoldbl),libc_atold);
EXPORT(__DSYM(_atoldbl),libc_atod);
EXPORT(__KSYM(_atoldbl_l),libc_atold_l);
EXPORT(__DSYM(_atoldbl_l),libc_atod_l);
EXPORT(_atoi64,libc_ato64);
EXPORT(_atoi64_l,libc_ato64_l);
EXPORT(__SYMw16(_wtof),libc_w16tof);
EXPORT(__SYMw32(_wtof),libc_w32tof);
EXPORT(__SYMw16(_wtof_l),libc_w16tof_l);
EXPORT(__SYMw32(_wtof_l),libc_w32tof_l);
EXPORT(__SYMw16(_wtoi64),libc_w16to64);
EXPORT(__SYMw32(_wtoi64),libc_w32to64);
EXPORT(__SYMw16(_wtoi64_l),libc_w16to64_l);
EXPORT(__SYMw32(_wtoi64_l),libc_w32to64_l);


/* DOS names for some wide-character functions. */
EXPORT(__SYMw16(_wcstoimax_l),libc_w16to64_l);
EXPORT(__SYMw32(_wcstoimax_l),libc_w32to64_l);
EXPORT(__SYMw16(_wcstoumax_l),libc_w16to64_l);
EXPORT(__SYMw32(_wcstoumax_l),libc_w32to64_l);
EXPORT(__SYMw16(_wcstod_l),libc_w32tod_l);
EXPORT(__SYMw32(_wcstod_l),libc_w32tod_l);
EXPORT(__SYMw16(_wcstof_l),libc_w32tof_l);
EXPORT(__SYMw32(_wcstof_l),libc_w32tof_l);
EXPORT(__KSYMw16(_wcstold_l),libc_w32told_l);
EXPORT(__KSYMw32(_wcstold_l),libc_w32told_l);
EXPORT(__DSYMw16(_wcstold_l),libc_w32tod_l);
EXPORT(__DSYMw32(_wcstold_l),libc_w32tod_l);
EXPORT(__SYMw16(_wcstoi64),libc_w32to64);
EXPORT(__SYMw32(_wcstoi64),libc_w32to64);
EXPORT(__SYMw16(_wcstoi64_l),libc_w32to64_l);
EXPORT(__SYMw32(_wcstoi64_l),libc_w32to64_l);
EXPORT(__SYMw16(_wcstoui64),libc_w32tou64);
EXPORT(__SYMw32(_wcstoui64),libc_w32tou64);
EXPORT(__SYMw16(_wcstoui64_l),libc_w32tou64_l);
EXPORT(__SYMw32(_wcstoui64_l),libc_w32tou64_l);

#if __SIZEOF_LONG_LONG__ > 4
EXPORT(_atoll_l,libc_ato64_l);
EXPORT(_strtoll_l,libc_strto64_l);
EXPORT(_strtoull_l,libc_strtou64_l);
EXPORT(__SYMw16(_wcstoll_l),libc_w32to64_l);
EXPORT(__SYMw32(_wcstoll_l),libc_w32to64_l);
EXPORT(__SYMw16(_wcstoull_l),libc_w32tou64_l);
EXPORT(__SYMw32(_wcstoull_l),libc_w32tou64_l);
EXPORT(__SYMw16(_wtoll),libc_w16to64);
EXPORT(__SYMw32(_wtoll),libc_w32to64);
EXPORT(__SYMw16(_wtoull),libc_w16tou64);
EXPORT(__SYMw32(_wtoull),libc_w32tou64);
EXPORT(__SYMw16(_wtoll_l),libc_w16to64_l);
EXPORT(__SYMw32(_wtoll_l),libc_w32to64_l);
EXPORT(__SYMw16(_wtoull_l),libc_w16tou64_l);
EXPORT(__SYMw32(_wtoull_l),libc_w32tou64_l);
#else
EXPORT(__KSYM(_atoll_l),libc_ato32_l);
EXPORT(__DSYM(_atoll_l),libc_ato64_l);
EXPORT(__KSYM(_strtoll_l),libc_strto32_l);
EXPORT(__DSYM(_strtoll_l),libc_strto64_l);
EXPORT(__KSYM(_strtoull_l),libc_strtou32_l);
EXPORT(__DSYM(_strtoull_l),libc_strtou64_l);
EXPORT(__KSYMw16(_wcstoll_l),libc_w32to32_l);
EXPORT(__DSYMw16(_wcstoll_l),libc_w32to64_l);
EXPORT(__KSYMw32(_wcstoll_l),libc_w32to32_l);
EXPORT(__DSYMw32(_wcstoll_l),libc_w32to64_l);
EXPORT(__KSYMw16(_wcstoull_l),libc_w32tou32_l);
EXPORT(__DSYMw16(_wcstoull_l),libc_w32tou64_l);
EXPORT(__KSYMw32(_wcstoull_l),libc_w32tou32_l);
EXPORT(__DSYMw32(_wcstoull_l),libc_w32tou64_l);
EXPORT(__KSYMw16(_wtoll),libc_w16to32);
EXPORT(__DSYMw16(_wtoll),libc_w16to64);
EXPORT(__KSYMw32(_wtoll),libc_w32to32);
EXPORT(__DSYMw32(_wtoll),libc_w32to64);
EXPORT(__KSYMw16(_wtoull),libc_w16tou32);
EXPORT(__DSYMw16(_wtoull),libc_w16tou64);
EXPORT(__KSYMw32(_wtoull),libc_w32tou32);
EXPORT(__DSYMw32(_wtoull),libc_w32tou64);
EXPORT(__KSYMw16(_wtoll_l),libc_w16to32_l);
EXPORT(__DSYMw16(_wtoll_l),libc_w16to64_l);
EXPORT(__KSYMw32(_wtoll_l),libc_w32to32_l);
EXPORT(__DSYMw32(_wtoll_l),libc_w32to64_l);
EXPORT(__KSYMw16(_wtoull_l),libc_w16tou32_l);
EXPORT(__DSYMw16(_wtoull_l),libc_w16tou64_l);
EXPORT(__KSYMw32(_wtoull_l),libc_w32tou32_l);
EXPORT(__DSYMw32(_wtoull_l),libc_w32tou64_l);
#endif

/* Work around a 32-bit long type on DOS. */
#if __SIZEOF_LONG__ <= 4
EXPORT(_atol_l,libc_ato32_l);
EXPORT(_strtol_l,libc_strto32);
EXPORT(_strtoul_l,libc_strtou32);
EXPORT(__SYMw16(_wcstol_l),libc_w16to32);
EXPORT(__SYMw32(_wcstol_l),libc_w32to32);
EXPORT(__SYMw16(_wcstoul_l),libc_w16tou32);
EXPORT(__SYMw32(_wcstoul_l),libc_w32tou32);
EXPORT(__SYMw16(_wtol),libc_w16to32);
EXPORT(__SYMw32(_wtol),libc_w32to32);
EXPORT(__SYMw16(_wtoul),libc_w16tou32);
EXPORT(__SYMw32(_wtoul),libc_w32tou32);
EXPORT(__SYMw16(_wtol_l),libc_w16to32_l);
EXPORT(__SYMw32(_wtol_l),libc_w32to32_l);
EXPORT(__SYMw16(_wtoul_l),libc_w16tou32_l);
EXPORT(__SYMw32(_wtoul_l),libc_w32tou32_l);
#else
EXPORT(__KSYM(_atol_l),libc_ato64_l);
EXPORT(__DSYM(_atol_l),libc_ato32_l);
EXPORT(__KSYM(_strtol_l),libc_strto64);
EXPORT(__DSYM(_strtol_l),libc_strto32);
EXPORT(__KSYM(_strtoul_l),libc_strtou64);
EXPORT(__DSYM(_strtoul_l),libc_strtou32);
EXPORT(__KSYMw16(_wcstol_l),libc_w16to64);
EXPORT(__DSYMw16(_wcstol_l),libc_w16to32);
EXPORT(__KSYMw32(_wcstol_l),libc_w32to64);
EXPORT(__DSYMw32(_wcstol_l),libc_w32to32);
EXPORT(__KSYMw16(_wcstoul_l),libc_w16tou64);
EXPORT(__DSYMw16(_wcstoul_l),libc_w16tou32);
EXPORT(__KSYMw32(_wcstoul_l),libc_w32tou64);
EXPORT(__DSYMw32(_wcstoul_l),libc_w32tou32);
EXPORT(__KSYMw16(_wtol),libc_w16to64);
EXPORT(__DSYMw16(_wtol),libc_w16to32);
EXPORT(__KSYMw32(_wtol),libc_w32to64);
EXPORT(__DSYMw32(_wtol),libc_w32to32);
EXPORT(__KSYMw16(_wtoul),libc_w16tou64);
EXPORT(__DSYMw16(_wtoul),libc_w16tou32);
EXPORT(__KSYMw32(_wtoul),libc_w32tou64);
EXPORT(__DSYMw32(_wtoul),libc_w32tou32);
EXPORT(__KSYMw16(_wtol_l),libc_w16to64_l);
EXPORT(__DSYMw16(_wtol_l),libc_w16to32_l);
EXPORT(__KSYMw32(_wtol_l),libc_w32to64_l);
EXPORT(__DSYMw32(_wtol_l),libc_w32to32_l);
EXPORT(__KSYMw16(_wtoul_l),libc_w16tou64_l);
EXPORT(__DSYMw16(_wtoul_l),libc_w16tou32_l);
EXPORT(__KSYMw32(_wtoul_l),libc_w32tou64_l);
EXPORT(__DSYMw32(_wtoul_l),libc_w32tou32_l);
#endif


/* GLibc aliases. */
EXPORT_STRONG(__wcscasecmp_l,libc_w32casecmp_l);
EXPORT_STRONG(__wcsncasecmp_l,libc_w32ncasecmp_l);
EXPORT_STRONG(__wcscoll_l,libc_w32coll_l);
EXPORT_STRONG(__strverscmp,libc_strverscmp);
EXPORT_STRONG(__strxfrm_l,libc_strxfrm_l);
EXPORT_STRONG(__wcsxfrm_l,libc_w32xfrm_l);
#if __SIZEOF_LONG__ <= 4
EXPORT_STRONG(__strtol_l,libc_strto32_l);
EXPORT_STRONG(__wcstol_l,libc_w32to32_l);
EXPORT_STRONG(__strtoul_l,libc_strtou32_l);
EXPORT_STRONG(__wcstoul_l,libc_w32tou32_l);
#else
EXPORT_STRONG(__strtol_l,libc_strto64_l);
EXPORT_STRONG(__wcstol_l,libc_w32to64_l);
EXPORT_STRONG(__strtoul_l,libc_strtou64_l);
EXPORT_STRONG(__wcstoul_l,libc_w32tou64_l);
#endif
EXPORT_STRONG(__strtoll_l,libc_strto64_l);
EXPORT_STRONG(__wcstoll_l,libc_w32to64_l);
EXPORT_STRONG(__strtoull_l,libc_strtou64_l);
EXPORT_STRONG(__wcstoull_l,libc_w32tou64_l);
EXPORT_STRONG(__strtof_l,libc_strtof_l);
EXPORT_STRONG(__wcstof_l,libc_w32tof_l);
EXPORT_STRONG(__strtod_l,libc_strtod_l);
EXPORT_STRONG(__wcstod_l,libc_w32tod_l);
EXPORT_STRONG(__strtold_l,libc_strtold_l);
EXPORT_STRONG(__wcstold_l,libc_w32told_l);

#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_strtof_internal,libc_strtof);
DEFINE_INTERN_ALIAS(libc_wcstof_internal,libc_w32tof);
DEFINE_INTERN_ALIAS(libc_strtod_internal,libc_strtod);
DEFINE_INTERN_ALIAS(libc_wcstod_internal,libc_w32tod);
DEFINE_INTERN_ALIAS(libc_strtold_internal,libc_strtold);
DEFINE_INTERN_ALIAS(libc_wcstold_internal,libc_w32told);
DEFINE_INTERN_ALIAS(libc_strtoll_internal,libc_strto64);
DEFINE_INTERN_ALIAS(libc_wcstoll_internal,libc_w32to64);
DEFINE_INTERN_ALIAS(libc_strtoull_internal,libc_strtou64);
DEFINE_INTERN_ALIAS(libc_wcstoull_internal,libc_w32tou64);
#if __SIZEOF_LONG__ <= 4
DEFINE_INTERN_ALIAS(libc_wcstol_internal,libc_w32to32);
DEFINE_INTERN_ALIAS(libc_strtol_internal,libc_strto32);
DEFINE_INTERN_ALIAS(libc_wcstoul_internal,libc_w32tou32);
DEFINE_INTERN_ALIAS(libc_strtoul_internal,libc_strtou32);
#else
DEFINE_INTERN_ALIAS(libc_wcstol_internal,libc_w32to64);
DEFINE_INTERN_ALIAS(libc_strtol_internal,libc_strto64);
DEFINE_INTERN_ALIAS(libc_wcstoul_internal,libc_w32tou64);
DEFINE_INTERN_ALIAS(libc_strtoul_internal,libc_strtou64);
#endif
#else
CRT_GLC float LIBCCALL libc_strtof_internal(char const *nptr, char **endptr, int UNUSED(group)) { return libc_strtof(nptr,endptr); }
CRT_GLC float LIBCCALL libc_wcstof_internal(char32_t const *nptr, char32_t **endptr, int UNUSED(group)) { return libc_w32tof(nptr,endptr); }
CRT_GLC double LIBCCALL libc_strtod_internal(char const *nptr, char **endptr, int UNUSED(group)) { return libc_strtod(nptr,endptr); }
CRT_GLC double LIBCCALL libc_wcstod_internal(char32_t const *nptr, char32_t **endptr, int UNUSED(group)) { return libc_w32tod(nptr,endptr); }
CRT_GLC long double LIBCCALL libc_strtold_internal(char const *nptr, char **endptr, int UNUSED(group)) { return libc_strtold(nptr,endptr); }
CRT_GLC long double LIBCCALL libc_wcstold_internal(char32_t const *nptr, char32_t **endptr, int UNUSED(group)) { return libc_w32told(nptr,endptr); }
CRT_GLC long long LIBCCALL libc_strtoll_internal(char const *nptr, char **endptr, int UNUSED(group)) { return libc_strto64(nptr,endptr); }
CRT_GLC long long LIBCCALL libc_wcstoll_internal(char32_t const *nptr, char32_t **endptr, int UNUSED(group)) { return libc_w32to64(nptr,endptr); }
CRT_GLC unsigned long long LIBCCALL libc_strtoull_internal(char const *nptr, char **endptr, int UNUSED(group)) { return libc_strtou64(nptr,endptr); }
CRT_GLC unsigned long long LIBCCALL libc_wcstoull_internal(char32_t const *nptr, char32_t **endptr, int UNUSED(group)) { return libc_w32tou64(nptr,endptr); }
#if __SIZEOF_LONG__ <= 4
CRT_GLC long LIBCCALL libc_strtol_internal(char const *nptr, char **endptr, int UNUSED(group)) { return libc_strto32(nptr,endptr); }
CRT_GLC long LIBCCALL libc_wcstol_internal(char32_t const *nptr, char32_t **endptr, int UNUSED(group)) { return libc_w32to32(nptr,endptr); }
CRT_GLC unsigned long LIBCCALL libc_strtoul_internal(char const *nptr, char **endptr, int UNUSED(group)) { return libc_strtou32(nptr,endptr); }
CRT_GLC unsigned long LIBCCALL libc_wcstoul_internal(char32_t const *nptr, char32_t **endptr, int UNUSED(group)) { return libc_w32tou32(nptr,endptr); }
#else
DEFINE_INTERN_ALIAS(libc_strtol_internal,libc_strtoll_internal);
DEFINE_INTERN_ALIAS(libc_wcstol_internal,libc_wcstoll_internal);
DEFINE_INTERN_ALIAS(libc_strtoul_internal,libc_strtoull_internal);
DEFINE_INTERN_ALIAS(libc_wcstoul_internal,libc_wcstoull_internal);
#endif
#endif

EXPORT_STRONG(__strtof_internal,libc_strtof_internal);
EXPORT_STRONG(__wcstof_internal,libc_wcstof_internal);
EXPORT_STRONG(__strtod_internal,libc_strtod_internal);
EXPORT_STRONG(__wcstod_internal,libc_wcstod_internal);
EXPORT_STRONG(__strtold_internal,libc_strtold_internal);
EXPORT_STRONG(__wcstold_internal,libc_wcstold_internal);
EXPORT_STRONG(__strtol_internal,libc_strtol_internal);
EXPORT_STRONG(__wcstol_internal,libc_wcstol_internal);
EXPORT_STRONG(__strtoul_internal,libc_strtoul_internal);
EXPORT_STRONG(__wcstoul_internal,libc_wcstoul_internal);
EXPORT_STRONG(__strtoq_internal,libc_strtoll_internal);
EXPORT_STRONG(__strtoll_internal,libc_strtoll_internal);
EXPORT_STRONG(__wcstoll_internal,libc_wcstoll_internal);
EXPORT_STRONG(__strtouq_internal,libc_strtoull_internal);
EXPORT_STRONG(__strtoull_internal,libc_strtoull_internal);
EXPORT_STRONG(__wcstoull_internal,libc_wcstoull_internal);

EXPORT_STRONG(__stpcpy,libc_stpcpy);
EXPORT_STRONG(__stpncpy,libc_stpncpy);
EXPORT_STRONG(__strcasecmp,libc_strcasecmp);
EXPORT_STRONG(__strcasecmp_l,libc_strcasecmp_l);
EXPORT_STRONG(__strcasestr,libc_strcasestr);
EXPORT_STRONG(__strcoll_l,libc_strcoll_l);
EXPORT_STRONG(__strncasecmp_l,libc_strncasecmp_l);
EXPORT_STRONG(__strtok_r,libc_strtok_r);
EXPORT_STRONG(__rawmemchr,libc_rawmemchr);
EXPORT_STRONG(__mempcpy,libc_mempcpy);
EXPORT_STRONG(__bzero,libc_bzero);


#endif /* !CONFIG_LIBC_LIMITED_API */



DECL_END


#endif /* !GUARD_HYBRID_STRING_C */
