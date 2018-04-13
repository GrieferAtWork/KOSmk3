/* Copyright (c) 2018 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following ____restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_LIBS_LIBC_DOS_SLIB_C
#define GUARD_LIBS_LIBC_DOS_SLIB_C 1

#include "libc.h"
#include "dos_slib.h"
#include "errno.h"
#include <errno.h>
#include <hybrid/minmax.h>

#ifdef __CC__
DECL_BEGIN

/* strcat_s() */
EXPORT(__KSYM(strcat_s),libc_strcat_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_strcat_s(char *__restrict dst, size_t dstsize,
              char const *__restrict src) {
 while (dstsize && *dst) ++dst,--dstsize;
 if (!dstsize) return EINVAL;
 while ((*dst++ = *src++) != 0 && --dstsize);
 if (!dstsize) return ERANGE;
 return 0;
}
EXPORT(__DSYM(strcat_s),libc_dos_strcat_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_strcat_s(char *__restrict dst, size_t dstsize,
                  char const *__restrict src) {
 return libc_errno_kos2dos(libc_strcat_s(dst,dstsize,src));
}

/* strncat_s() */
EXPORT(__KSYM(strncat_s),libc_strncat_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_strncat_s(char *__restrict dst, size_t dstsize,
               char const *__restrict src, size_t maxlen) {
 while (dstsize && *dst) ++dst,--dstsize;
 if (!dstsize) return EINVAL;
 if (maxlen != (size_t)-1 && dstsize < maxlen) return ERANGE;
 while (maxlen && (*dst++ = *src++) != 0 && --dstsize) --maxlen;
 if (maxlen) *dst = 0;
 if (!dstsize) {
  if (maxlen == (size_t)-1) {
   dst[dstsize-1] = 0;
   return KOS_STRUNCATE;
  }
  return ERANGE;
 }
 return 0;
}
EXPORT(__DSYM(strncat_s),libc_dos_strncat_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_strncat_s(char *__restrict dst, size_t dstsize,
                   char const *__restrict src, size_t maxlen) {
 return libc_errno_kos2dos(libc_strncat_s(dst,dstsize,src,maxlen));
}


/* strcpy_s() */
EXPORT(__KSYM(strcpy_s),libc_strcpy_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_strcpy_s(char *__restrict dst, size_t dstsize,
              char const *__restrict src) {
 if (!dstsize) return EINVAL;
 while ((*dst++ = *src++) != 0 && --dstsize);
 if (!dstsize) return ERANGE;
 return 0;
}
EXPORT(__DSYM(strcpy_s),libc_dos_strcpy_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_strcpy_s(char *__restrict dst, size_t dstsize,
                  char const *__restrict src) {
 return libc_errno_kos2dos(libc_strcpy_s(dst,dstsize,src));
}



/* strncpy_s() */
EXPORT(__KSYM(strncpy_s),libc_strncpy_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_strncpy_s(char *__restrict dst, size_t dstsize,
               char const *__restrict src, size_t maxlen) {
 if (!maxlen) return 0;
 if (maxlen != (size_t)-1 && dstsize < maxlen) return ERANGE;
 while ((*dst++ = *src++) != 0 && --dstsize && --maxlen);
 if (maxlen) *dst = 0;
 if (!dstsize) {
  if (maxlen == (size_t)-1) {
   dst[dstsize-1] = 0;
   return KOS_STRUNCATE;
  }
  return ERANGE;
 }
 return 0;
}
EXPORT(__DSYM(strncpy_s),libc_dos_strncpy_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_strncpy_s(char *__restrict dst, size_t dstsize,
                   char const *__restrict src, size_t maxlen) {
 return libc_errno_kos2dos(libc_strncpy_s(dst,dstsize,src,maxlen));
}







/* wcscat_s() */
EXPORT(__KSYMw16(wcscat_s),libc_w16cat_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w16cat_s(char16_t *__restrict dst, size_t dstsize,
              char16_t const *__restrict src) {
 while (dstsize && *dst) ++dst,--dstsize;
 if (!dstsize) return EINVAL;
 while ((*dst++ = *src++) != 0 && --dstsize);
 if (!dstsize) return ERANGE;
 return 0;
}
EXPORT(__KSYMw32(wcscat_s),libc_w32cat_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w32cat_s(char32_t *__restrict dst, size_t dstsize,
              char32_t const *__restrict src) {
 while (dstsize && *dst) ++dst,--dstsize;
 if (!dstsize) return EINVAL;
 while ((*dst++ = *src++) != 0 && --dstsize);
 if (!dstsize) return ERANGE;
 return 0;
}
EXPORT(__DSYMw16(wcscat_s),libc_dos_w16cat_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16cat_s(char16_t *__restrict dst, size_t dstsize,
                  char16_t const *__restrict src) {
 return libc_errno_kos2dos(libc_w16cat_s(dst,dstsize,src));
}
EXPORT(__DSYMw32(wcscat_s),libc_dos_w32cat_s);
CRT_DOS_EXT derrno_t LIBCCALL
libc_dos_w32cat_s(char32_t *__restrict dst, size_t dstsize,
                  char32_t const *__restrict src) {
 return libc_errno_kos2dos(libc_w32cat_s(dst,dstsize,src));
}

/* wcsncat_s() */
EXPORT(__KSYMw16(wcsncat_s),libc_w16ncat_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w16ncat_s(char16_t *__restrict dst, size_t dstsize,
               char16_t const *__restrict src, size_t maxlen) {
 while (dstsize && *dst) ++dst,--dstsize;
 if (!dstsize) return EINVAL;
 if (maxlen != (size_t)-1 && dstsize < maxlen) return ERANGE;
 while (maxlen && (*dst++ = *src++) != 0 && --dstsize) --maxlen;
 if (maxlen) *dst = 0;
 if (!dstsize) {
  if (maxlen == (size_t)-1) {
   dst[dstsize-1] = 0;
   return KOS_STRUNCATE;
  }
  return ERANGE;
 }
 return 0;
}
EXPORT(__KSYMw32(wcsncat_s),libc_w32ncat_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w32ncat_s(char32_t *__restrict dst, size_t dstsize,
               char32_t const *__restrict src, size_t maxlen) {
 while (dstsize && *dst) ++dst,--dstsize;
 if (!dstsize) return EINVAL;
 if (maxlen != (size_t)-1 && dstsize < maxlen) return ERANGE;
 while (maxlen && (*dst++ = *src++) != 0 && --dstsize) --maxlen;
 if (maxlen) *dst = 0;
 if (!dstsize) {
  if (maxlen == (size_t)-1) {
   dst[dstsize-1] = 0;
   return KOS_STRUNCATE;
  }
  return ERANGE;
 }
 return 0;
}
EXPORT(__DSYMw16(wcsncat_s),libc_dos_w16ncat_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16ncat_s(char16_t *__restrict dst, size_t dstsize,
                   char16_t const *__restrict src, size_t maxlen) {
 return libc_errno_kos2dos(libc_w16ncat_s(dst,dstsize,src,maxlen));
}
EXPORT(__DSYMw32(wcsncat_s),libc_dos_w32ncat_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w32ncat_s(char32_t *__restrict dst, size_t dstsize,
                   char32_t const *__restrict src, size_t maxlen) {
 return libc_errno_kos2dos(libc_w32ncat_s(dst,dstsize,src,maxlen));
}



/* wcscpy_s() */
EXPORT(__KSYMw16(wcscpy_s),libc_w16cpy_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w16cpy_s(char16_t *__restrict dst, size_t dstsize,
              char16_t const *__restrict src) {
 if (!dstsize) return EINVAL;
 while ((*dst++ = *src++) != 0 && --dstsize);
 if (!dstsize) return ERANGE;
 return 0;
}
EXPORT(__KSYMw32(wcscpy_s),libc_w32cpy_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w32cpy_s(char32_t *__restrict dst, size_t dstsize,
              char32_t const *__restrict src) {
 if (!dstsize) return EINVAL;
 while ((*dst++ = *src++) != 0 && --dstsize);
 if (!dstsize) return ERANGE;
 return 0;
}
EXPORT(__DSYMw16(wcscpy_s),libc_dos_w16cpy_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16cpy_s(char16_t *__restrict dst, size_t dstsize,
                  char16_t const *__restrict src) {
 return libc_errno_kos2dos(libc_w16cpy_s(dst,dstsize,src));
}
EXPORT(__DSYMw32(wcscpy_s),libc_dos_w32cpy_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w32cpy_s(char32_t *__restrict dst, size_t dstsize,
                  char32_t const *__restrict src) {
 return libc_errno_kos2dos(libc_w32cpy_s(dst,dstsize,src));
}



/* wcsncpy_s() */
EXPORT(__KSYMw16(wcsncpy_s),libc_w16ncpy_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w16ncpy_s(char16_t *__restrict dst, size_t dstsize,
               char16_t const *__restrict src, size_t maxlen) {
 if (!maxlen) return 0;
 if (maxlen != (size_t)-1 && dstsize < maxlen) return ERANGE;
 while ((*dst++ = *src++) != 0 && --dstsize && --maxlen);
 if (maxlen) *dst = 0;
 if (!dstsize) {
  if (maxlen == (size_t)-1) {
   dst[dstsize-1] = 0;
   return KOS_STRUNCATE;
  }
  return ERANGE;
 }
 return 0;
}
EXPORT(__KSYMw32(wcsncpy_s),libc_w32ncpy_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w32ncpy_s(char32_t *__restrict dst, size_t dstsize,
               char32_t const *__restrict src, size_t maxlen) {
 if (!maxlen) return 0;
 if (maxlen != (size_t)-1 && dstsize < maxlen) return ERANGE;
 while ((*dst++ = *src++) != 0 && --dstsize && --maxlen);
 if (maxlen) *dst = 0;
 if (!dstsize) {
  if (maxlen == (size_t)-1) {
   dst[dstsize-1] = 0;
   return KOS_STRUNCATE;
  }
  return ERANGE;
 }
 return 0;
}
EXPORT(__DSYMw16(wcsncpy_s),libc_dos_w16ncpy_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16ncpy_s(char16_t *__restrict dst, size_t dstsize,
                   char16_t const *__restrict src, size_t maxlen) {
 return libc_errno_kos2dos(libc_w16ncpy_s(dst,dstsize,src,maxlen));
}
EXPORT(__DSYMw32(wcsncpy_s),libc_dos_w32ncpy_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w32ncpy_s(char32_t *__restrict dst, size_t dstsize,
                   char32_t const *__restrict src, size_t maxlen) {
 return libc_errno_kos2dos(libc_w32ncpy_s(dst,dstsize,src,maxlen));
}






/* memcpy_s() */
EXPORT(__KSYM(memcpy_s),libc_memcpy_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_memcpy_s(void *__restrict dst, size_t dstsize,
              void const *__restrict src, size_t srcsize) {
 if (!srcsize) return 0;
 if (!src || dstsize < srcsize) {
  libc_memset(dst,0,dstsize);
  if (!src) return EINVAL;
  return ERANGE;
 }
 libc_memcpy(dst,src,srcsize);
 return 0;
}
EXPORT(__DSYM(memcpy_s),libc_dos_memcpy_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_memcpy_s(void *__restrict dst, size_t dstsize,
                  void const *__restrict src, size_t srcsize) {
 return libc_errno_kos2dos(libc_memcpy_s(dst,dstsize,src,srcsize));
}

/* memmove_s() */
EXPORT(__KSYM(memmove_s),libc_memmove_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_memmove_s(void *dst, size_t dstsize,
               void const *src, size_t srcsize) {
 if (!srcsize) return 0;
 if (!dst || !src) return EINVAL;
 if (dstsize < srcsize) return ERANGE;
 libc_memmove(dst,src,srcsize);
 return 0;
}
EXPORT(__DSYM(memmove_s),libc_dos_memmove_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_memmove_s(void *dst, size_t dstsize,
                   void const *src, size_t srcsize) {
 return libc_errno_kos2dos(libc_memmove_s(dst,dstsize,src,srcsize));
}

/* wmemcpy_s() */
EXPORT(__KSYMw16(wmemcpy_s),libc_w16memcpy_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w16memcpy_s(char16_t *__restrict dst, size_t dstsize,
                 char16_t const *__restrict src, size_t srcsize) {
 if (!srcsize) return 0;
 if (!src || dstsize < srcsize) {
  libc_memsetw(dst,0,dstsize);
  if (!src) return EINVAL;
  return ERANGE;
 }
 libc_memcpyw(dst,src,srcsize);
 return 0;
}
EXPORT(__KSYMw32(wmemcpy_s),libc_w32memcpy_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w32memcpy_s(char32_t *__restrict dst, size_t dstsize,
                 char32_t const *__restrict src, size_t srcsize) {
 if (!srcsize) return 0;
 if (!src || dstsize < srcsize) {
  libc_memsetl(dst,0,dstsize);
  if (!src) return EINVAL;
  return ERANGE;
 }
 libc_memcpyl(dst,src,srcsize);
 return 0;
}
EXPORT(__DSYMw16(wmemcpy_s),libc_dos_w16memcpy_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16memcpy_s(char16_t *__restrict dst, size_t dstsize,
                     char16_t const *__restrict src, size_t srcsize) {
 return libc_errno_kos2dos(libc_w16memcpy_s(dst,dstsize,src,srcsize));
}
EXPORT(__DSYMw32(wmemcpy_s),libc_dos_w32memcpy_s);
CRT_DOS_EXT derrno_t LIBCCALL
libc_dos_w32memcpy_s(char32_t *__restrict dst, size_t dstsize,
                     char32_t const *__restrict src, size_t srcsize) {
 return libc_errno_kos2dos(libc_w32memcpy_s(dst,dstsize,src,srcsize));
}

/* wmemmove_s() */
EXPORT(__KSYMw16(wmemmove_s),libc_w16memmove_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w16memmove_s(char16_t *dst, size_t dstsize,
                  char16_t const *src, size_t srcsize) {
 if (!srcsize) return 0;
 if (!dst || !src) return EINVAL;
 if (dstsize < srcsize) return ERANGE;
 libc_memmovew(dst,src,srcsize);
 return 0;
}
EXPORT(__KSYMw32(wmemmove_s),libc_w32memmove_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w32memmove_s(char32_t *dst, size_t dstsize,
                  char32_t const *src, size_t srcsize) {
 if (!srcsize) return 0;
 if (!dst || !src) return EINVAL;
 if (dstsize < srcsize) return ERANGE;
 libc_memmovel(dst,src,srcsize);
 return 0;
}
EXPORT(__DSYMw16(wmemmove_s),libc_dos_w16memmove_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16memmove_s(char16_t *dst, size_t dstsize,
                      char16_t const *src, size_t srcsize) {
 return libc_errno_kos2dos(libc_w16memmove_s(dst,dstsize,src,srcsize));
}
EXPORT(__DSYMw32(wmemmove_s),libc_dos_w32memmove_s);
CRT_DOS_EXT derrno_t LIBCCALL
libc_dos_w32memmove_s(char32_t *dst, size_t dstsize,
                      char32_t const *src, size_t srcsize) {
 return libc_errno_kos2dos(libc_w32memmove_s(dst,dstsize,src,srcsize));
}




/* strset_s() */
EXPORT(__KSYM(strset_s),libc_strset_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_strset_s(char *__restrict str, size_t maxlen, int val) {
 if (!str) return maxlen ? EINVAL : 0;
 while (*str && --maxlen) *str++ = (char)val;
 if (!maxlen) return EINVAL;
 return 0;
}
EXPORT(__DSYM(_strset_s),libc_dos_strset_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_strset_s(char *__restrict str, size_t maxlen, int val) {
 return libc_errno_kos2dos(libc_strset_s(str,maxlen,val));
}

/* strnset_s() */
EXPORT(__KSYM(strnset_s),libc_strnset_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_strnset_s(char *__restrict str, size_t buflen,
               int val, size_t maxlen) {
 if (!str) return buflen ? EINVAL : 0;
 while (*str && maxlen && --buflen) { *str++ = (char)val; --maxlen; }
 if (!maxlen) while (*str && --buflen) ++str;
 if (!buflen) return EINVAL;
 return 0;
}
EXPORT(__DSYM(_strnset_s),libc_dos_strnset_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_strnset_s(char *__restrict str, size_t buflen,
                   int val, size_t maxlen) {
 return libc_errno_kos2dos(libc_strnset_s(str,buflen,val,maxlen));
}






/* strlwr_s() */
EXPORT(__KSYM(strlwr_s),libc_strlwr_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_strlwr_s(char *__restrict str, size_t maxlen) {
 return libc_strlwr_s_l(str,maxlen,NULL);
}
EXPORT(__DSYM(_strlwr_s),libc_dos_strlwr_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_strlwr_s(char *__restrict str, size_t maxlen) {
 return libc_dos_strlwr_s_l(str,maxlen,NULL);
}

/* strlwr_s_l() */
EXPORT(__KSYM(strlwr_s_l),libc_strlwr_s_l);
CRT_DOS_EXT errno_t LIBCCALL
libc_strlwr_s_l(char *__restrict str, size_t maxlen, locale_t locale) {
 size_t count;
 if (!str) return EINVAL;
 count = libc_strnlen(str,maxlen);
 if (count >= maxlen) return EINVAL;
 for (; count; --count,++str) *str = libc_toupper_l(*str,locale);
 return 0;
}
EXPORT(__DSYM(_strlwr_s_l),libc_dos_strlwr_s_l);
CRT_DOS derrno_t LIBCCALL
libc_dos_strlwr_s_l(char *__restrict str, size_t maxlen, locale_t locale) {
 return libc_errno_kos2dos(libc_strlwr_s_l(str,maxlen,locale));
}



/* strupr_s() */
EXPORT(__KSYM(strupr_s),libc_strupr_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_strupr_s(char *__restrict str, size_t maxlen) {
 return libc_strupr_s_l(str,maxlen,NULL);
}
EXPORT(__DSYM(_strupr_s),libc_dos_strupr_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_strupr_s(char *__restrict str, size_t maxlen) {
 return libc_dos_strupr_s_l(str,maxlen,NULL);
}

/* strupr_s_l() */
EXPORT(__KSYM(strupr_s_l),libc_strupr_s_l);
CRT_DOS_EXT errno_t LIBCCALL
libc_strupr_s_l(char *__restrict str, size_t maxlen, locale_t locale) {
 size_t count;
 if (!str) return EINVAL;
 count = libc_strnlen(str,maxlen);
 if (count >= maxlen) return EINVAL;
 for (; count; --count,++str) *str = libc_toupper_l(*str,locale);
 return 0;
}
EXPORT(__DSYM(_strupr_s_l),libc_dos_strupr_s_l);
CRT_DOS derrno_t LIBCCALL
libc_dos_strupr_s_l(char *__restrict str, size_t maxlen, locale_t locale) {
 return libc_errno_kos2dos(libc_strupr_s_l(str,maxlen,locale));
}









/* wcsset_s() */
CRT_DOS_EXT errno_t LIBCCALL
libc_w16set_s(char16_t *__restrict str, size_t maxlen, char16_t val) {
 if (!str) return maxlen ? EINVAL : 0;
 while (*str && --maxlen) *str++ = val;
 if (!maxlen) return EINVAL;
 return 0;
}
EXPORT(__DSYMw16(_wcsset_s),libc_dos_w16set_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16set_s(char16_t *__restrict str, size_t maxlen, char16_t val) {
 return libc_errno_kos2dos(libc_w16set_s(str,maxlen,val));
}
CRT_DOS_EXT errno_t LIBCCALL
libc_w32set_s(char32_t *__restrict str, size_t maxlen, char32_t val) {
 if (!str) return maxlen ? EINVAL : 0;
 while (*str && --maxlen) *str++ = val;
 if (!maxlen) return EINVAL;
 return 0;
}
EXPORT(__DSYMw32(wcsset_s),libc_dos_w32set_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w32set_s(char32_t *__restrict str, size_t maxlen, char32_t val) {
 return libc_errno_kos2dos(libc_w32set_s(str,maxlen,val));
}





/* wcsnset_s() */
CRT_DOS_EXT errno_t LIBCCALL
libc_w16nset_s(char16_t *__restrict str, size_t buflen, char16_t val, size_t maxlen) {
 if (!str) return buflen ? EINVAL : 0;
 while (*str && maxlen && --buflen) { *str++ = val; --maxlen; }
 if (!maxlen) while (*str && --buflen) ++str;
 if (!buflen) return EINVAL;
 return 0;
}
EXPORT(__DSYMw16(_wcsnset_s),libc_dos_w16nset_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16nset_s(char16_t *__restrict str, size_t buflen, char16_t val, size_t maxlen) {
 return libc_errno_kos2dos(libc_w16nset_s(str,buflen,val,maxlen));
}
CRT_DOS_EXT errno_t LIBCCALL
libc_w32nset_s(char32_t *__restrict str, size_t buflen, char32_t val, size_t maxlen) {
 if (!str) return buflen ? EINVAL : 0;
 while (*str && maxlen && --buflen) { *str++ = val; --maxlen; }
 if (!maxlen) while (*str && --buflen) ++str;
 if (!buflen) return EINVAL;
 return 0;
}
EXPORT(__DSYMw32(wcsnset_s),libc_dos_w32nset_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w32nset_s(char32_t *__restrict str, size_t buflen, char32_t val, size_t maxlen) {
 return libc_errno_kos2dos(libc_w32nset_s(str,buflen,val,maxlen));
}







/* wcslwr_s() */
EXPORT(__KSYMw16(wcslwr_s),libc_w16upr_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w16lwr_s(char16_t *__restrict str, size_t maxlen) {
 return libc_w16lwr_s_l(str,maxlen,NULL);
}
EXPORT(__KSYMw32(wcslwr_s),libc_w32upr_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w32lwr_s(char32_t *__restrict str, size_t maxlen) {
 return libc_w32lwr_s_l(str,maxlen,NULL);
}
EXPORT(__DSYMw16(_wcslwr_s),libc_dos_w16lwr_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16lwr_s(char16_t *__restrict str, size_t maxlen) {
 return libc_dos_w16lwr_s_l(str,maxlen,NULL);
}
EXPORT(__DSYMw32(wcslwr_s),libc_dos_w32lwr_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w32lwr_s(char32_t *__restrict str, size_t maxlen) {
 return libc_dos_w32lwr_s_l(str,maxlen,NULL);
}






/* wcsupr_s() */
EXPORT(__KSYMw16(wcsupr_s),libc_w16upr_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w16upr_s(char16_t *__restrict str, size_t maxlen) {
 return libc_w16upr_s_l(str,maxlen,NULL);
}
EXPORT(__KSYMw32(wcsupr_s),libc_w32upr_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w32upr_s(char32_t *__restrict str, size_t maxlen) {
 return libc_w32upr_s_l(str,maxlen,NULL);
}
EXPORT(__DSYMw16(_wcsupr_s),libc_dos_w16upr_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16upr_s(char16_t *__restrict str, size_t maxlen) {
 return libc_dos_w16upr_s_l(str,maxlen,NULL);
}
EXPORT(__DSYMw32(wcsupr_s),libc_dos_w32upr_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w32upr_s(char32_t *__restrict str, size_t maxlen) {
 return libc_dos_w32upr_s_l(str,maxlen,NULL);
}






/* wcslwr_s_l() */
CRT_DOS_EXT errno_t LIBCCALL
libc_w16lwr_s_l(char16_t *__restrict str, size_t maxlen, locale_t locale) {
 size_t count;
 if (!str) return EINVAL;
 count = libc_w16nlen(str,maxlen);
 if (count >= maxlen) return EINVAL;
 for (; count; --count,++str) *str = libc_towlower_l(*str,locale);
 return 0;
}
CRT_DOS_EXT errno_t LIBCCALL
libc_w32lwr_s_l(char32_t *__restrict str, size_t maxlen, locale_t locale) {
 size_t count;
 if (!str) return EINVAL;
 count = libc_w32nlen(str,maxlen);
 if (count >= maxlen) return EINVAL;
 for (; count; --count,++str) *str = libc_towlower_l(*str,locale);
 return 0;
}
EXPORT(__DSYMw16(_wcslwr_s_l),libc_dos_w16lwr_s_l);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16lwr_s_l(char16_t *__restrict str, size_t maxlen, locale_t locale) {
 return libc_errno_kos2dos(libc_w16lwr_s_l(str,maxlen,locale));
}
EXPORT(__DSYMw32(wcslwr_s_l),libc_dos_w32lwr_s_l);
CRT_DOS derrno_t LIBCCALL
libc_dos_w32lwr_s_l(char32_t *__restrict str, size_t maxlen, locale_t locale) {
 return libc_errno_kos2dos(libc_w32lwr_s_l(str,maxlen,locale));
}









CRT_DOS_EXT errno_t LIBCCALL
libc_w16upr_s_l(char16_t *__restrict str, size_t maxlen, locale_t locale) {
 size_t count;
 if (!str) return EINVAL;
 count = libc_w16nlen(str,maxlen);
 if (count >= maxlen) return EINVAL;
 for (; count; --count,++str) *str = libc_towupper_l(*str,locale);
 return 0;
}
CRT_DOS_EXT errno_t LIBCCALL
libc_w32upr_s_l(char32_t *__restrict str, size_t maxlen, locale_t locale) {
 size_t count;
 if (!str) return EINVAL;
 count = libc_w32nlen(str,maxlen);
 if (count >= maxlen) return EINVAL;
 for (; count; --count,++str) *str = libc_towupper_l(*str,locale);
 return 0;
}
EXPORT(__DSYMw16(_wcsupr_s_l),libc_dos_w16upr_s_l);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16upr_s_l(char16_t *__restrict str, size_t maxlen, locale_t locale) {
 return libc_errno_kos2dos(libc_w16upr_s_l(str,maxlen,locale));
}
EXPORT(__DSYMw32(wcsupr_s_l),libc_dos_w32upr_s_l);
CRT_DOS derrno_t LIBCCALL
libc_dos_w32upr_s_l(char32_t *__restrict str, size_t maxlen, locale_t locale) {
 return libc_errno_kos2dos(libc_w32upr_s_l(str,maxlen,locale));
}






DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_DOS_SLIB_C */
