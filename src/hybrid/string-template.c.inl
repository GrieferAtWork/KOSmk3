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
#ifdef __INTELLISENSE__
#include "string.c"
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#endif

#include "hybrid.h"
#include <assert.h>

#ifndef FORMAT_OPTION_CHARTYPE
#error "Must #define FORMAT_OPTION_CHARTYPE"
#endif

#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#define T_sizeof                   __SIZEOF_CHAR__
#define T_int                      int
#define T_char                     char
#define T_schar                    signed char
#define T_uchar                    unsigned char
#define T_tolower(x)              (char)libc_tolower(x)
#define T_toupper(x)              (char)libc_toupper(x)
#define T_tolower_l(x,locale)     (char)libc_tolower_l(x,locale)
#define T_toupper_l(x,locale)     (char)libc_toupper_l(x,locale)
#define T_isspace(x)                    libc_isspace(x)
#define T_isspace_l(x,locale)           libc_isspace_l(x,locale)
#define glob_aX(x)                 a##x
#define libc_aX(x)                 libc_a##x
#define glob_strX(x)               str##x
#define libc_strX(x)               libc_str##x
#define glob_stpX(x)               stp##x
#define libc_stpX(x)               libc_stp##x
#define glob_fuzzy_strX(x)         fuzzy_str##x
#define libc_fuzzy_strX(x)         libc_fuzzy_str##x
#define glob_wildstrX(x)           wildstr##x
#define libc_wildstrX(x)           libc_wildstr##x
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#define T_sizeof                   2
#define T_int                      wint_t
#define T_char                     char16_t
#define T_schar                    s16
#define T_uchar                    u16
#define glob_aX(x)                 __SYMw16(w##x)
#define libc_aX(x)                 libc_aw16##x
#define glob_strX(x)               __SYMw16(wcs##x)
#define libc_strX(x)               libc_w16##x
#define glob_stpX(x)               __SYMw16(wcp##x)
#define libc_stpX(x)               libc_w16p##x
#define glob_fuzzy_strX(x)         __SYMw16(fuzzy_wcs##x)
#define libc_fuzzy_strX(x)         libc_fuzzy_w16##x
#define glob_wildstrX(x)           __SYMw16(wildwcs##x)
#define libc_wildstrX(x)           libc_wildw16##x
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR32
#define T_sizeof                   4
#define T_int                      wint_t
#define T_char                     char32_t
#define T_schar                    s32
#define T_uchar                    u32
#define glob_aX(x)                 __SYMw32(w##x)
#define libc_aX(x)                 libc_aw32##x
#define glob_strX(x)               __SYMw32(wcs##x)
#define libc_strX(x)               libc_w32##x
#define glob_stpX(x)               __SYMw32(wcp##x)
#define libc_stpX(x)               libc_w32p##x
#define glob_fuzzy_strX(x)         __SYMw32(fuzzy_wcs##x)
#define libc_fuzzy_strX(x)         libc_fuzzy_w32##x
#define glob_wildstrX(x)           __SYMw32(wildwcs##x)
#define libc_wildstrX(x)           libc_wildw32##x
#else
#error "Invalid character type"
#endif

#ifndef T_tolower
#define T_tolower(x)              (T_char)libc_towlower(x)
#define T_toupper(x)              (T_char)libc_towupper(x)
#define T_tolower_l(x,locale)     (T_char)libc_towlower_l(x,locale)
#define T_toupper_l(x,locale)     (T_char)libc_towupper_l(x,locale)
#define T_isspace(x)                      libc_iswspace(x)
#define T_isspace_l(x,locale)             libc_iswspace_l(x,locale)
#endif

#ifndef T_NUL
#define T_NUL '\0'
#endif

#ifndef CRT_STRING
#define CRT_STRING \
  INTERN ATTR_WEAK ATTR_SECTION(".text.crt.string")
#endif /* !CRT_STRING */

#ifndef EXPORT_SYMBOL
#define EXPORT_SYMBOL(x)      DEFINE_PUBLIC_WEAK_ALIAS(glob_strX(x),libc_strX(x))
#define EXPORT_SYMBOLa(x)     DEFINE_PUBLIC_WEAK_ALIAS(glob_aX(x),libc_aX(x))
#define EXPORT_SYMBOLp(x)     DEFINE_PUBLIC_WEAK_ALIAS(glob_stpX(x),libc_stpX(x))
#define EXPORT_SYMBOLfuzzy(x) DEFINE_PUBLIC_WEAK_ALIAS(glob_fuzzy_strX(x),libc_fuzzy_strX(x))
#define EXPORT_SYMBOLwild(x)  DEFINE_PUBLIC_WEAK_ALIAS(glob_wildstrX(x),libc_wildstrX(x))
#endif

#if T_sizeof == 1
#define libc_memcpystr(dst,src,num_chars)   ((T_char *)libc_memcpy((u8 *)(dst),(u8 *)(src),num_chars))
#define libc_mempcpystr(dst,src,num_chars)  ((T_char *)libc_mempcpy((u8 *)(dst),(u8 *)(src),num_chars))
#define libc_memmovestr(dst,src,num_chars)  ((T_char *)libc_memmove((u8 *)(dst),(u8 *)(src),num_chars))
#define libc_mempmovestr(dst,src,num_chars) ((T_char *)libc_mempmove((u8 *)(dst),(u8 *)(src),num_chars))
#define libc_memsetstr(dst,chr,num_chars)   ((T_char *)libc_memset((u8 *)(dst),(u8)(chr),num_chars))
#define libc_mempsetstr(dst,chr,num_chars)  ((T_char *)libc_mempset((u8 *)(dst),(u8)(chr),num_chars))
#elif T_sizeof == 2
#define libc_memcpystr(dst,src,num_chars)   ((T_char *)libc_memcpyw((u16 *)(dst),(u16 *)(src),num_chars))
#define libc_mempcpystr(dst,src,num_chars)  ((T_char *)libc_mempcpyw((u16 *)(dst),(u16 *)(src),num_chars))
#define libc_memmovestr(dst,src,num_chars)  ((T_char *)libc_memmovew((u16 *)(dst),(u16 *)(src),num_chars))
#define libc_mempmovestr(dst,src,num_chars) ((T_char *)libc_mempmovew((u16 *)(dst),(u16 *)(src),num_chars))
#define libc_memsetstr(dst,chr,num_chars)   ((T_char *)libc_memsetw((u16 *)(dst),(u16)(chr),num_chars))
#define libc_mempsetstr(dst,chr,num_chars)  ((T_char *)libc_mempsetw((u16 *)(dst),(u16)(chr),num_chars))
#elif T_sizeof == 4
#define libc_memcpystr(dst,src,num_chars)   ((T_char *)libc_memcpyl((u32 *)(dst),(u32 *)(src),num_chars))
#define libc_mempcpystr(dst,src,num_chars)  ((T_char *)libc_mempcpyl((u32 *)(dst),(u32 *)(src),num_chars))
#define libc_memmovestr(dst,src,num_chars)  ((T_char *)libc_memmovel((u32 *)(dst),(u32 *)(src),num_chars))
#define libc_mempmovestr(dst,src,num_chars) ((T_char *)libc_mempmovel((u32 *)(dst),(u32 *)(src),num_chars))
#define libc_memsetstr(dst,chr,num_chars)   ((T_char *)libc_memsetl((u32 *)(dst),(u32)(chr),num_chars))
#define libc_mempsetstr(dst,chr,num_chars)  ((T_char *)libc_mempsetl((u32 *)(dst),(u32)(chr),num_chars))
#elif T_sizeof == 8
#define libc_memcpystr(dst,src,num_chars)   ((T_char *)libc_memcpyq((u64 *)(dst),(u64 *)(src),num_chars))
#define libc_mempcpystr(dst,src,num_chars)  ((T_char *)libc_mempcpyq((u64 *)(dst),(u64 *)(src),num_chars))
#define libc_memmovestr(dst,src,num_chars)  ((T_char *)libc_memmoveq((u64 *)(dst),(u64 *)(src),num_chars))
#define libc_mempmovestr(dst,src,num_chars) ((T_char *)libc_mempmoveq((u64 *)(dst),(u64 *)(src),num_chars))
#define libc_memsetstr(dst,chr,num_chars)   ((T_char *)libc_memsetq((u64 *)(dst),(u64)(chr),num_chars))
#define libc_mempsetstr(dst,chr,num_chars)  ((T_char *)libc_mempsetq((u64 *)(dst),(u64)(chr),num_chars))
#else
#error "Unsupported character size"
#endif


DECL_BEGIN

CRT_STRING T_char *LIBCCALL
libc_strX(end)(T_char *__restrict string) {
 while (*string) ++string;
 return string;
}
EXPORT_SYMBOL(end);

CRT_STRING size_t LIBCCALL
libc_strX(len)(T_char *__restrict string) {
 return (size_t)(libc_strX(end)(string) - string);
}
EXPORT_SYMBOL(len);

CRT_STRING T_char *LIBCCALL
libc_strX(nend)(T_char *__restrict string, size_t maxlen) {
 T_char *end = string+maxlen;
 while (string != end && *string) ++string;
 return string;
}
EXPORT_SYMBOL(nend);

CRT_STRING size_t LIBCCALL
libc_strX(nlen)(T_char *__restrict string, size_t maxlen) {
 return (size_t)(libc_strX(nend)(string,maxlen) - string);
}
EXPORT_SYMBOL(nlen);

CRT_STRING T_char *LIBCCALL
libc_strX(chr)(T_char *__restrict string, T_int ch) {
 for (; *string; ++string)
     if (*string == (T_char)ch)
         return string;
 return NULL;
}
EXPORT_SYMBOL(chr);

CRT_STRING T_char *LIBCCALL
libc_strX(chrnul)(T_char *__restrict string, T_int ch) {
 for (; *string; ++string)
     if (*string == ch)
         break;
 return string;
}
EXPORT_SYMBOL(chrnul);

CRT_STRING T_char *LIBCCALL
libc_strX(rchr)(T_char *__restrict string, T_int ch) {
 T_char *result = NULL;
 for (; *string; ++string)
     if (*string == ch)
         result = string;
 return result;
}
EXPORT_SYMBOL(rchr);

CRT_STRING T_char *LIBCCALL
libc_strX(rchrnul)(T_char *__restrict string, T_int ch) {
 T_char *result = (T_char *)string-1,*iter = (T_char *)string;
 do if (*iter == (T_char)ch) result = iter;
 while (*iter++);
 return result;
}
EXPORT_SYMBOL(rchrnul);

CRT_STRING int LIBCCALL
libc_strX(cmp)(T_char *a, T_char *b) {
 T_char ca,cb;
 do if ((ca = *a++) != (cb = *b++))
         return (int)((T_schar)ca - (T_schar)cb);
 while (ca);
 return 0;
}
EXPORT_SYMBOL(cmp);

CRT_STRING int LIBCCALL
libc_strX(ncmp)(T_char *a, T_char *b, size_t max_chars) {
 T_char ca,cb;
 do {
  if (!max_chars--) break;
  if ((ca = *a++) != (cb = *b++))
       return (int)((T_schar)ca - (T_schar)cb);
 } while (ca);
 return 0;
}
EXPORT_SYMBOL(ncmp);

CRT_STRING int LIBCCALL
libc_strX(casecmp)(T_char *a, T_char *b) {
 T_char ca,cb;
 do {
  if ((ca = *a++) != (cb = *b++)) {
   /* Unify casing and try again. */
   ca = T_toupper(ca);
   cb = T_toupper(cb);
   if (ca != cb)
       return (int)((T_schar)ca - (T_schar)cb);
  }
 } while (ca);
 return 0;
}
EXPORT_SYMBOL(casecmp);

CRT_STRING int LIBCCALL
libc_strX(ncasecmp)(T_char *a, T_char *b, size_t max_chars) {
 T_char ca,cb;
 do {
  if (!max_chars--) break;
  if ((ca = *a++) != (cb = *b++)) {
   /* Unify casing and try again. */
   ca = T_toupper(ca);
   cb = T_toupper(cb);
   if (ca != cb)
       return (int)((T_schar)ca - (T_schar)cb);
  }
 } while (ca);
 return 0;
}
EXPORT_SYMBOL(ncasecmp);


#ifndef CONFIG_LIBC_LIMITED_API

CRT_STRING size_t LIBCCALL
libc_strX(lcat)(T_char *__restrict dst,
                T_char *__restrict src,
                size_t dst_size) {
 size_t result = libc_strX(len)(src);
 T_char *new_dst = dst + libc_strX(nlen)(dst,dst_size);
 size_t copy_size = (dst_size -= (new_dst-dst),
                     result < dst_size ? result : dst_size-1);
 libc_memcpystr(new_dst,src,copy_size);
 new_dst[copy_size] = T_NUL;
 return result + (new_dst-dst);
}
EXPORT_SYMBOL(lcat);

CRT_STRING size_t LIBCCALL
libc_strX(lcpy)(T_char *__restrict dst,
                T_char *__restrict src,
                size_t dst_size) {
 size_t result = libc_strX(len)(src);
 size_t copy_size = result < dst_size ? result : dst_size-1;
 libc_memcpystr(dst,src,copy_size);
 dst[copy_size] = T_NUL;
 return result;
}
EXPORT_SYMBOL(lcpy);


CRT_STRING int LIBCCALL
libc_strX(casecmp_l)(T_char *a, T_char *b, locale_t locale) {
 T_char ca,cb;
 do {
  if ((ca = *a++) != (cb = *b++)) {
   /* Unify casing and try again. */
   ca = T_toupper_l(ca,locale);
   cb = T_toupper_l(cb,locale);
   if (ca != cb)
       return (int)((T_schar)ca - (T_schar)cb);
  }
 } while (ca);
 return 0;
}
EXPORT_SYMBOL(casecmp_l);

CRT_STRING int LIBCCALL
libc_strX(ncasecmp_l)(T_char *a, T_char *b,
                      size_t max_chars, locale_t locale) {
 T_char ca,cb;
 do {
  if (!max_chars--) break;
  if ((ca = *a++) != (cb = *b++)) {
   /* Unify casing and try again. */
   ca = T_toupper_l(ca,locale);
   cb = T_toupper_l(cb,locale);
   if (ca != cb)
       return (int)((T_schar)ca - (T_schar)cb);
  }
 } while (ca);
 return 0;
}
EXPORT_SYMBOL(ncasecmp_l);

CRT_STRING T_char *LIBCCALL
libc_strX(nchr)(T_char *__restrict haystack,
                T_int needle, size_t maxlen) {
 T_char *end = haystack+maxlen;
 for (; haystack != end && *haystack; ++haystack)
     if (*haystack == needle)
         return haystack;
 return NULL;
}
EXPORT_SYMBOL(nchr);

CRT_STRING T_char *LIBCCALL
libc_strX(nrchr)(T_char *__restrict haystack,
                 T_int needle, size_t maxlen) {
 T_char *end = haystack+maxlen,*result = NULL;
 for (; haystack != end && *haystack; ++haystack)
     if (*haystack == needle)
         result = haystack;
 return result;
}
EXPORT_SYMBOL(nrchr);

CRT_STRING T_char *LIBCCALL
libc_strX(nchrnul)(T_char *__restrict haystack,
                   T_int needle, size_t maxlen) {
 T_char *end = haystack+maxlen;
 for (; haystack != end &&
       *haystack &&
       *haystack != needle;
      ++haystack);
 return haystack;
}
EXPORT_SYMBOL(nchrnul);

CRT_STRING T_char *LIBCCALL
libc_strX(nrchrnul)(T_char *__restrict haystack,
                    T_int needle, size_t maxlen) {
 T_char *end = haystack+maxlen,*result = haystack-1;
 for (; haystack != end && *haystack; ++haystack)
     if (*haystack == needle)
         result = haystack;
 return result;
}
EXPORT_SYMBOL(nrchrnul);

CRT_STRING size_t LIBCCALL
libc_strX(off)(T_char *__restrict haystack, int needle) {
 return (size_t)(libc_strX(chrnul)(haystack,needle)-haystack);
}
EXPORT_SYMBOL(off);

CRT_STRING size_t LIBCCALL
libc_strX(roff)(T_char *__restrict haystack, int needle) {
 return (size_t)(libc_strX(rchrnul)(haystack,needle)-haystack);
}
EXPORT_SYMBOL(roff);

CRT_STRING size_t LIBCCALL
libc_strX(noff)(T_char *__restrict haystack, int needle, size_t maxlen) {
 return (size_t)(libc_strX(nchrnul)(haystack,needle,maxlen)-haystack);
}
EXPORT_SYMBOL(noff);

CRT_STRING size_t LIBCCALL
libc_strX(nroff)(T_char *__restrict haystack, int needle, size_t maxlen) {
 return (size_t)(libc_strX(nrchrnul)(haystack,needle,maxlen)-haystack);
}
EXPORT_SYMBOL(nroff);



CRT_STRING T_char *LIBCCALL
libc_stpX(cpy)(T_char *__restrict dst, T_char *__restrict src) {
 return libc_mempcpystr(dst,src,libc_strX(len)(src)+1) - 1;
}
EXPORT_SYMBOLp(cpy);

CRT_STRING T_char *LIBCCALL
libc_stpX(ncpy)(T_char *__restrict dst, T_char *__restrict src, size_t n) {
 size_t src_len = libc_strX(nlen)(src,n);
 libc_memcpystr(dst,src,src_len);
 libc_memsetstr(dst+src_len,T_NUL,n-src_len);
 return dst+src_len;
}
EXPORT_SYMBOLp(ncpy);

CRT_STRING T_char *LIBCCALL
libc_strX(cpy)(T_char *__restrict dst, T_char *__restrict src) {
 return libc_memcpystr(dst,src,libc_strX(len)(src)+1);
}
EXPORT_SYMBOL(cpy);

CRT_STRING T_char *LIBCCALL
libc_strX(ncpy)(T_char *__restrict dst, T_char *__restrict src, size_t n) {
 size_t src_len = libc_strX(nlen)(src,n);
 libc_memcpystr(dst,src,src_len);
 libc_memsetstr(dst+src_len,T_NUL,n-src_len);
 return dst;
}
EXPORT_SYMBOL(ncpy);

CRT_STRING T_char *LIBCCALL
libc_strX(cat)(T_char *__restrict dst, T_char *__restrict src) {
 libc_memcpystr(libc_strX(end)(dst),src,libc_strX(len)(src)+1);
 return dst;
}
EXPORT_SYMBOL(cat);

CRT_STRING T_char *LIBCCALL
libc_strX(ncat)(T_char *__restrict dst, T_char *__restrict src, size_t n) {
 size_t src_len = libc_strX(nlen)(src,n);
 T_char *target = libc_strX(end)(dst);
 libc_memcpystr(target,src,src_len);
 target[src_len] = T_NUL;
 return dst;
}
EXPORT_SYMBOL(ncat);

CRT_STRING T_char *LIBCCALL
libc_strX(str)(T_char *haystack, T_char *needle) {
 T_char *hay2,*ned_iter,ch;
 T_char needle_start = *needle++;
 while ((ch = *haystack++) != T_NUL) {
  if (ch == needle_start) {
   hay2 = haystack,ned_iter = needle;
   while ((ch = *ned_iter++) != T_NUL) {
    if (*hay2++ != ch)
        goto miss;
   }
   return haystack-1;
  }
miss:;
 }
 return NULL;
}
EXPORT_SYMBOL(str);

CRT_STRING T_char *LIBCCALL
libc_strX(casestr)(T_char *haystack, T_char *needle) {
 T_char *hay2,*ned_iter,ch;
 T_char needle_start = T_tolower(*needle++);
 while ((ch = *haystack++) != T_NUL) {
  if (needle_start == ch ||
      needle_start == T_tolower(ch)) {
   hay2 = haystack,ned_iter = needle;
   while ((ch = *ned_iter++) != T_NUL) {
    if (*hay2 != ch && T_tolower(*hay2) != T_tolower(ch))
        goto miss;
    ++hay2;
   }
   return haystack-1;
  }
miss:;
 }
 return NULL;
}
EXPORT_SYMBOL(casestr);

CRT_STRING T_char *LIBCCALL
libc_strX(casestr_l)(T_char *haystack, T_char *needle, locale_t locale) {
 T_char *hay2,*ned_iter,ch;
 T_char needle_start = T_tolower_l(*needle++,locale);
 while ((ch = *haystack++) != T_NUL) {
  if (needle_start == ch ||
      needle_start == T_tolower_l(ch,locale)) {
   hay2 = haystack,ned_iter = needle;
   while ((ch = *ned_iter++) != T_NUL) {
    if (*hay2 != ch && T_tolower_l(*hay2,locale) != T_tolower_l(ch,locale))
        goto miss;
    ++hay2;
   }
   return haystack-1;
  }
miss:;
 }
 return NULL;
}
EXPORT_SYMBOL(casestr_l);

CRT_STRING size_t LIBCCALL
libc_fuzzy_strX(cmp)(T_char *a, T_char *b) {
#if T_sizeof == 1
 return libc_fuzzy_memcmp((u8 *)a,libc_strX(len)(a),
                          (u8 *)b,libc_strX(len)(b));
#elif T_sizeof == 2
 return libc_fuzzy_memcmpw((u16 *)a,libc_strX(len)(a),
                           (u16 *)b,libc_strX(len)(b));
#elif T_sizeof == 4
 return libc_fuzzy_memcmpl((u32 *)a,libc_strX(len)(a),
                           (u32 *)b,libc_strX(len)(b));
#else
 return libc_fuzzy_memcmp((u8 *)a,libc_strX(len)(a)*sizeof(T_char),
                          (u8 *)b,libc_strX(len)(b)*sizeof(T_char));
#endif
}
EXPORT_SYMBOLfuzzy(cmp);

CRT_STRING size_t LIBCCALL
libc_fuzzy_strX(ncmp)(T_char *a, size_t max_a_chars,
                      T_char *b, size_t max_b_chars) {
#if T_sizeof == 1
 return libc_fuzzy_memcmp((u8 *)a,libc_strX(nlen)(a,max_a_chars),
                          (u8 *)b,libc_strX(nlen)(b,max_b_chars));
#elif T_sizeof == 2
 return libc_fuzzy_memcmpw((u16 *)a,libc_strX(nlen)(a,max_a_chars),
                           (u16 *)b,libc_strX(nlen)(b,max_b_chars));
#elif T_sizeof == 4
 return libc_fuzzy_memcmpl((u32 *)a,libc_strX(nlen)(a,max_a_chars),
                           (u32 *)b,libc_strX(nlen)(b,max_b_chars));
#else
 return libc_fuzzy_memcmp((u8 *)a,libc_strX(nlen)(a,max_a_chars)*sizeof(T_char),
                          (u8 *)b,libc_strX(nlen)(b,max_b_chars)*sizeof(T_char));
#endif
}
EXPORT_SYMBOLfuzzy(ncmp);

CRT_STRING size_t LIBCCALL
libc_fuzzy_strX(casecmp)(T_char *a, T_char *b) {
#if T_sizeof == 1
 return libc_fuzzy_memcasecmp((u8 *)a,libc_strX(len)(a),
                              (u8 *)b,libc_strX(len)(b));
#elif T_sizeof == 2
 return libc_fuzzy_memcasecmpw((u16 *)a,libc_strX(len)(a),
                               (u16 *)b,libc_strX(len)(b));
#elif T_sizeof == 4
 return libc_fuzzy_memcasecmpl((u32 *)a,libc_strX(len)(a),
                               (u32 *)b,libc_strX(len)(b));
#else
 return libc_fuzzy_memcasecmp((u8 *)a,libc_strX(len)(a)*sizeof(T_char),
                              (u8 *)b,libc_strX(len)(b)*sizeof(T_char));
#endif
}
EXPORT_SYMBOLfuzzy(casecmp);

CRT_STRING size_t LIBCCALL
libc_fuzzy_strX(ncasecmp)(T_char *a, size_t max_a_chars,
                          T_char *b, size_t max_b_chars) {
#if T_sizeof == 1
 return libc_fuzzy_memcasecmp((u8 *)a,libc_strX(nlen)(a,max_a_chars),
                              (u8 *)b,libc_strX(nlen)(b,max_b_chars));
#elif T_sizeof == 2
 return libc_fuzzy_memcasecmpw((u16 *)a,libc_strX(nlen)(a,max_a_chars),
                               (u16 *)b,libc_strX(nlen)(b,max_b_chars));
#elif T_sizeof == 4
 return libc_fuzzy_memcasecmpl((u32 *)a,libc_strX(nlen)(a,max_a_chars),
                               (u32 *)b,libc_strX(nlen)(b,max_b_chars));
#else
 return libc_fuzzy_memcasecmp((u8 *)a,libc_strX(nlen)(a,max_a_chars)*sizeof(T_char),
                              (u8 *)b,libc_strX(nlen)(b,max_b_chars)*sizeof(T_char));
#endif
}
EXPORT_SYMBOLfuzzy(ncasecmp);

CRT_STRING size_t LIBCCALL
libc_fuzzy_strX(casecmp_l)(T_char *a, T_char *b, locale_t locale) {
#if T_sizeof == 1
 return libc_fuzzy_memcasecmp_l((u8 *)a,libc_strX(len)(a),
                                (u8 *)b,libc_strX(len)(b),
                                locale);
#elif T_sizeof == 2
 return libc_fuzzy_memcasecmpw_l((u16 *)a,libc_strX(len)(a),
                                 (u16 *)b,libc_strX(len)(b),
                                 locale);
#elif T_sizeof == 4
 return libc_fuzzy_memcasecmpl_l((u32 *)a,libc_strX(len)(a),
                                 (u32 *)b,libc_strX(len)(b),
                                 locale);
#else
 return libc_fuzzy_memcasecmp_l((u8 *)a,libc_strX(len)(a)*sizeof(T_char),
                                (u8 *)b,libc_strX(len)(b)*sizeof(T_char),
                                locale);
#endif
}
EXPORT_SYMBOLfuzzy(casecmp_l);

CRT_STRING size_t LIBCCALL
libc_fuzzy_strX(ncasecmp_l)(T_char *a, size_t max_a_chars,
                            T_char *b, size_t max_b_chars, locale_t locale) {
#if T_sizeof == 1
 return libc_fuzzy_memcasecmp_l((u8 *)a,libc_strX(nlen)(a,max_a_chars),
                                (u8 *)b,libc_strX(nlen)(b,max_b_chars),
                                 locale);
#elif T_sizeof == 2
 return libc_fuzzy_memcasecmpw_l((u16 *)a,libc_strX(nlen)(a,max_a_chars),
                                 (u16 *)b,libc_strX(nlen)(b,max_b_chars),
                                  locale);
#elif T_sizeof == 4
 return libc_fuzzy_memcasecmpl_l((u32 *)a,libc_strX(nlen)(a,max_a_chars),
                                 (u32 *)b,libc_strX(nlen)(b,max_b_chars),
                                  locale);
#else
 return libc_fuzzy_memcasecmp_l((u8 *)a,libc_strX(nlen)(a,max_a_chars)*sizeof(T_char),
                                (u8 *)b,libc_strX(nlen)(b,max_b_chars)*sizeof(T_char),
                                 locale);
#endif
}
EXPORT_SYMBOLfuzzy(ncasecmp_l);


CRT_STRING int LIBCCALL
libc_wildstrX(cmp)(T_char *pattern, T_char *string) {
 T_char card_post;
 for (;;) {
  if (!*string) {
   /* End of string (if the patter is empty, or only contains '*', we have a match) */
   while (*pattern == '*') ++pattern;
   return -(int)*pattern;
  }
  if (!*pattern)
      return (int)*string; /* Pattern end doesn't match */
  if (*pattern == '*') {
   /* Skip starts */
   do ++pattern; while (*pattern == '*');
   if ((card_post = *pattern++) == '\0')
        return 0; /* Pattern ends with '*' (matches everything) */
   if (card_post == '?') goto next; /* Match any --> already found */
   for (;;) {
    T_char ch = *string++;
    if (ch == card_post) {
     /* Recursively check if the rest of the string and pattern match */
     if (!libc_wildstrX(cmp)(string,pattern))
          return 0;
    } else if (!ch) {
     return -(int)card_post; /* Wildcard suffix not found */
    }
   }
  }
  if (*pattern == *string || *pattern == '?') {
next: ++string,++pattern;
   continue; /* single character match */
  }
  break; /* mismatch */
 }
 return *string-*pattern;
}
EXPORT_SYMBOLwild(cmp);

CRT_STRING int LIBCCALL
libc_wildstrX(casecmp_l)(T_char *pattern, T_char *string, locale_t locale) {
 T_char card_post;
 for (;;) {
  if (!*string) {
   /* End of string (if the patter is empty, or only contains '*', we have a match) */
   while (*pattern == '*') ++pattern;
   return -(int)*pattern;
  }
  if (!*pattern)
      return (int)*string; /* Pattern end doesn't match */
  if (*pattern == '*') {
   /* Skip starts */
   do ++pattern; while (*pattern == '*');
   if ((card_post = *pattern++) == '\0')
        return 0; /* Pattern ends with '*' (matches everything) */
   if (card_post == '?') goto next; /* Match any --> already found */
   card_post = T_tolower_l(card_post,locale);
   for (;;) {
    T_char ch = *string++;
    if (card_post == ch ||
        card_post == T_tolower_l(ch,locale)) {
     /* Recursively check if the rest of the string and pattern match */
     if (!libc_wildstrX(casecmp_l)(string,pattern,locale))
          return 0;
    } else if (!ch) {
     return -(int)card_post; /* Wildcard suffix not found */
    }
   }
  }
  if (*pattern == *string || *pattern == '?' ||
      T_tolower_l(*pattern,locale) ==
      T_tolower_l(*string,locale)) {
next: ++string,++pattern;
   continue; /* single character match */
  }
  break; /* mismatch */
 }
 return *string-*pattern;
}
EXPORT_SYMBOLwild(casecmp_l);

CRT_STRING int LIBCCALL
libc_wildstrX(casecmp)(T_char *pattern, T_char *string) {
 return libc_wildstrX(casecmp_l)(pattern,string,NULL);
}
EXPORT_SYMBOLwild(casecmp);

CRT_STRING int LIBCCALL
libc_strX(verscmp)(T_char *a, T_char *b) {
 T_char ca,cb,*a_start = a;
 do {
  if ((ca = *a) != (cb = *b)) {
   unsigned int vala,valb;
   /* Unwind common digits. */
   while (a != a_start) {
    if (a[-1] < '0' || a[-1] > '9') break;
    cb = ca = *--a,--b;
   }
   /* Check if both strings have digit sequences in the same places. */
   if ((ca < '0' || ca > '9') &&
       (cb < '0' || cb > '9'))
       return (int)((T_schar)ca - (T_schar)cb);
   /* Deal with leading zeros. */
   if (ca == '0') return -1;
   if (cb == '0') return 1;
   /* Compare digits. */
   vala = ca - '0';
   valb = cb - '0';
   for (;;) { ca = *a++; if (ca < '0' || ca > '9') break; vala *= 10; vala += ca-'0'; }
   for (;;) { cb = *b++; if (cb < '0' || cb > '9') break; valb *= 10; valb += cb-'0'; }
   return (int)vala - (int)valb;
  }
  ++a,++b;
 } while (ca);
 return 0;
}
EXPORT_SYMBOL(verscmp);

CRT_STRING T_char *LIBCCALL
libc_strX(sep)(T_char **__restrict stringp, T_char *__restrict delim) {
 T_char *result,*iter;
 if (!stringp || (result = *stringp) == NULL || !*result)
      return NULL;
 for (iter = result;
     *iter && !libc_strX(chr)(delim,*iter);
    ++iter);
 if (*iter) *iter++ = T_NUL;
 *stringp = iter;
 return result;
}
EXPORT_SYMBOL(sep);


CRT_STRING T_char *LIBCCALL
libc_strX(fry)(T_char *__restrict str) {
 T_char temp;
 size_t i,count = libc_strX(len)(str);
 for (i = 0; i < count; ++i) {
  size_t j = libc_rand();
  j = i+(j % (count-i));
  /* Swap these 2 characters. */
  temp = str[i];
  str[i] = str[j];
  str[j] = temp;
 }
 return str;
}
EXPORT_SYMBOL(fry);


CRT_STRING T_char *LIBCCALL
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
libc_index(T_char *__restrict haystack, T_int needle)
#else
libc_strX(index)(T_char *__restrict haystack, T_int needle)
#endif
{
 for (; *haystack; ++haystack)
    if (*haystack == needle)
        return haystack;
 if (!needle) return haystack;
 return NULL;
}

CRT_STRING T_char *LIBCCALL
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
libc_rindex(T_char *__restrict haystack, T_int needle)
#else
libc_strX(rindex)(T_char *__restrict haystack, T_int needle)
#endif
{
 T_char *result = NULL;
 for (; *haystack; ++haystack)
    if (*haystack == needle)
        result = haystack;
 if (!needle) return haystack;
 return result;
}

#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
EXPORT(index,libc_index);
EXPORT(rindex,libc_rindex);
#else
EXPORT_SYMBOL(index);
EXPORT_SYMBOL(rindex);
#endif


CRT_STRING size_t LIBCCALL
libc_strX(spn)(T_char *s, T_char *accept) {
 T_char *iter = s;
 while (libc_strX(chr)(accept,*iter)) ++iter;
 return (size_t)(iter-s);
}
EXPORT_SYMBOL(spn);

CRT_STRING size_t LIBCCALL
libc_strX(cspn)(T_char *s, T_char *reject) {
 T_char *iter = s;
 while (*iter && !libc_strX(chr)(reject,*iter)) ++iter;
 return (size_t)(iter-s);
}
EXPORT_SYMBOL(cspn);

CRT_STRING T_char *LIBCCALL
libc_strX(pbrk)(T_char *s, T_char *accept) {
 T_char *ned_iter,haych,ch;
 while ((haych = *s++) != T_NUL) {
  ned_iter = accept;
  while ((ch = *ned_iter++) != T_NUL) {
   if (haych == ch)
       return s-1;
  }
 }
 return NULL;
}
EXPORT_SYMBOL(pbrk);

CRT_STRING T_char *LIBCCALL
libc_strX(tok_r)(T_char *__restrict s,
                 T_char *__restrict delim,
                 T_char **__restrict save_ptr) {
 T_char *end;
 if (!s) s = *save_ptr;
 if (!*s) { *save_ptr = s; return NULL; }
 s += libc_strX(spn)(s,delim);
 if (!*s) { *save_ptr = s; return NULL; }
 end = s+libc_strX(cspn)(s,delim);
 if (!*end) { *save_ptr = end; return s; }
 *end = T_NUL;
 *save_ptr = end+1;
 return s;
}
//EXPORT_SYMBOL(tok_r); /* Exported manually due to wide-character clutch */

CRT_STRING T_char *LIBCCALL
libc_strX(tok)(T_char *__restrict s,
               T_char *__restrict delim) {
 PRIVATE T_char *strtok_safe;
 return libc_strX(tok_r)(s,delim,&strtok_safe);
}
//EXPORT_SYMBOL(tok); /* Exported manually due to wide-character clutch */


CRT_STRING int LIBCCALL
libc_strX(coll_l)(T_char *s1, T_char *s2, locale_t l) {
 return libc_strX(cmp)(s1,s2); /* XXX: todo? */
}
EXPORT_SYMBOL(coll_l);

CRT_STRING int LIBCCALL
libc_strX(ncoll_l)(T_char *s1, T_char *s2, size_t max_chars, locale_t l) {
 return libc_strX(ncmp)(s1,s2,max_chars); /* XXX: todo? */
}
EXPORT_SYMBOL(ncoll_l);

CRT_STRING int LIBCCALL
libc_strX(casecoll_l)(T_char *s1, T_char *s2, locale_t l) {
 return libc_strX(casecmp)(s1,s2); /* XXX: todo? */
}
EXPORT_SYMBOL(casecoll_l);

CRT_STRING int LIBCCALL
libc_strX(ncasecoll_l)(T_char *s1, T_char *s2, size_t max_chars, locale_t l) {
 return libc_strX(ncasecmp)(s1,s2,max_chars); /* XXX: todo? */
}
EXPORT_SYMBOL(ncasecoll_l);

CRT_STRING size_t LIBCCALL
libc_strX(xfrm_l)(T_char *__restrict dst,
                  T_char *__restrict src,
                  size_t n, locale_t l) {
 assertf(0,"TODO");
}
EXPORT_SYMBOL(xfrm_l);

CRT_STRING int LIBCCALL
libc_strX(coll)(T_char *s1, T_char *s2) {
 return libc_strX(coll_l)(s1,s2,NULL);
}
EXPORT_SYMBOL(coll);

CRT_STRING int LIBCCALL
libc_strX(ncoll)(T_char *s1, T_char *s2, size_t max_chars) {
 return libc_strX(ncoll_l)(s1,s2,max_chars,NULL);
}
EXPORT_SYMBOL(ncoll);

CRT_STRING int LIBCCALL
libc_strX(casecoll)(T_char *s1, T_char *s2) {
 return libc_strX(casecoll_l)(s1,s2,NULL);
}
EXPORT_SYMBOL(casecoll);

CRT_STRING int LIBCCALL
libc_strX(ncasecoll)(T_char *s1, T_char *s2, size_t max_chars) {
 return libc_strX(ncasecoll_l)(s1,s2,max_chars,NULL);
}
EXPORT_SYMBOL(ncasecoll);

CRT_STRING size_t LIBCCALL
libc_strX(xfrm)(T_char *__restrict dst,
                T_char *__restrict src, size_t n) {
 return libc_strX(xfrm_l)(dst,src,n,NULL);
}
EXPORT_SYMBOL(xfrm);


CRT_STRING T_char *LIBCCALL
libc_strX(rev)(T_char *__restrict str) {
#if T_sizeof == 1
 return (T_char *)libc_memrev((u8 *)str,libc_strX(len)(str));
#elif T_sizeof == 2
 return (T_char *)libc_memrevw((u16 *)str,libc_strX(len)(str));
#elif T_sizeof == 4
 return (T_char *)libc_memrevl((u32 *)str,libc_strX(len)(str));
#elif T_sizeof == 8
 return (T_char *)libc_memrevq((u64 *)str,libc_strX(len)(str));
#else
 return (T_char *)libc_memrev((u8 *)str,libc_strX(len)(str)*sizeof(T_char));
#endif
}
EXPORT_SYMBOL(rev);

CRT_STRING T_char *LIBCCALL
libc_strX(nrev)(T_char *__restrict str, size_t max_chars) {
#if T_sizeof == 1
 return (T_char *)libc_memrev((u8 *)str,libc_strX(nlen)(str,max_chars));
#elif T_sizeof == 2
 return (T_char *)libc_memrevw((u16 *)str,libc_strX(nlen)(str,max_chars));
#elif T_sizeof == 4
 return (T_char *)libc_memrevl((u32 *)str,libc_strX(nlen)(str,max_chars));
#elif T_sizeof == 8
 return (T_char *)libc_memrevq((u64 *)str,libc_strX(nlen)(str,max_chars));
#else
 return (T_char *)libc_memrev((u8 *)str,libc_strX(nlen)(str,max_chars)*sizeof(T_char));
#endif
}
EXPORT_SYMBOL(nrev);

CRT_STRING T_char *LIBCCALL
libc_strX(lwr_l)(T_char *__restrict str, locale_t locale) {
 T_char *iter = str;
 for (; *iter; ++iter) *iter = T_tolower_l(*iter,locale);
 return str;
}
EXPORT_SYMBOL(lwr_l);

CRT_STRING T_char *LIBCCALL
libc_strX(upr_l)(T_char *__restrict str, locale_t locale) {
 T_char *iter = str;
 for (; *iter; ++iter) *iter = T_toupper_l(*iter,locale);
 return str;
}
EXPORT_SYMBOL(upr_l);

CRT_STRING T_char *LIBCCALL
libc_strX(lwr)(T_char *__restrict str) {
 return libc_strX(lwr_l)(str,NULL);
}
EXPORT_SYMBOL(lwr);

CRT_STRING T_char *LIBCCALL
libc_strX(upr)(T_char *__restrict str) {
 return libc_strX(upr_l)(str,NULL);
}
EXPORT_SYMBOL(upr);

CRT_STRING T_char *LIBCCALL
libc_strX(set)(T_char *__restrict str, T_int chr) {
 T_char *iter = str;
 for (; *iter; ++iter) *iter = (T_char)chr;
 return str;
}
EXPORT_SYMBOL(set);

CRT_STRING T_char *LIBCCALL
libc_strX(nset)(T_char *__restrict str, T_int chr, size_t maxlen) {
 T_char *iter = str,*end = str+maxlen;
 for (; iter != end && *iter; ++iter) *iter = (T_char)chr;
 return str;
}
EXPORT_SYMBOL(nset);

CRT_STRING u32 LIBCCALL
libc_strX(tou32_l)(T_char *__restrict nptr,
                   T_char **endptr,
                   int base, locale_t locale) {
 u32 result,temp;
 if (!base) {
  if (*nptr == '0') {
   ++nptr;
   if (*nptr == 'x' || *nptr == 'X') ++nptr,base = 16;
   else if (*nptr == 'b' || *nptr == 'B') ++nptr,base = 2;
   else base = 8;
  } else base = 10;
 }
 result = 0;
 for (;;) {
  T_char ch = *nptr;
  if (ch >= '0' && ch <= '9') temp = (u32)(ch-'0');
  else if (ch >= 'a' && ch <= 'z') temp = (u32)(10+(ch-'a'));
  else if (ch >= 'A' && ch <= 'Z') temp = (u32)(10+(ch-'A'));
  else break;
  if (temp >= (unsigned int)base) break;
  ++nptr;
  result *= base;
  result += temp;
 }
 if (endptr) *endptr = nptr;
 return result;
}
EXPORT_SYMBOL(tou32_l);

CRT_STRING u64 LIBCCALL
libc_strX(tou64_l)(T_char *__restrict nptr,
                   T_char **endptr,
                   int base, locale_t locale) {
 u64 result,temp;
 if (!base) {
  if (*nptr == '0') {
   ++nptr;
   if (*nptr == 'x' || *nptr == 'X') ++nptr,base = 16;
   else if (*nptr == 'b' || *nptr == 'B') ++nptr,base = 2;
   else base = 8;
  } else base = 10;
 }
 result = 0;
 for (;;) {
  T_char ch = *nptr;
  if (ch >= '0' && ch <= '9') temp = (u64)(ch-'0');
  else if (ch >= 'a' && ch <= 'z') temp = (u64)(10+(ch-'a'));
  else if (ch >= 'A' && ch <= 'Z') temp = (u64)(10+(ch-'A'));
  else break;
  if (temp >= (unsigned int)base) break;
  ++nptr;
  result *= base;
  result += temp;
 }
 if (endptr) *endptr = nptr;
 return result;
}
EXPORT_SYMBOL(tou64_l);

CRT_STRING s32 LIBCCALL
libc_strX(to32_l)(T_char *__restrict nptr,
                  T_char **endptr,
                  int base, locale_t locale) {
 u32 result; bool neg = false;
 while (*nptr == '-') neg = !neg,++nptr;
 result = libc_strX(tou32_l)(nptr,endptr,base,locale);
 return neg ? -(s32)result : (s32)result;
}
EXPORT_SYMBOL(to32_l);

CRT_STRING s64 LIBCCALL
libc_strX(to64_l)(T_char *__restrict nptr,
                  T_char **endptr,
                  int base, locale_t locale) {
 u64 result; bool neg = false;
 while (*nptr == '-') neg = !neg,++nptr;
 result = libc_strX(tou64_l)(nptr,endptr,base,locale);
 return neg ? -(s64)result : (s64)result;
}
EXPORT_SYMBOL(to64_l);

CRT_STRING long double LIBCCALL
libc_strX(told_l)(T_char *__restrict nptr,
                  T_char **endptr, locale_t locale) {
 assertf(0,"TODO");
 return 0;
}
//EXPORT_SYMBOL(told_l);

CRT_STRING double LIBCCALL
libc_strX(tod_l)(T_char *__restrict nptr,
                 T_char **endptr, locale_t locale) {
 return (double)libc_strX(told_l)(nptr,endptr,locale);
}
EXPORT_SYMBOL(tod_l);

CRT_STRING float LIBCCALL
libc_strX(tof_l)(T_char *__restrict nptr,
                 T_char **endptr, locale_t locale) {
 return (float)libc_strX(told_l)(nptr,endptr,locale);
}
EXPORT_SYMBOL(tof_l);


CRT_STRING s32 LIBCCALL
libc_strX(to32)(T_char *__restrict nptr,
                T_char **endptr, int base) {
 return libc_strX(to32_l)(nptr,endptr,base,NULL);
}
EXPORT_SYMBOL(to32);

CRT_STRING s64 LIBCCALL
libc_strX(to64)(T_char *__restrict nptr,
                T_char **endptr, int base) {
 return libc_strX(to64_l)(nptr,endptr,base,NULL);
}
EXPORT_SYMBOL(to64);

CRT_STRING u32 LIBCCALL
libc_strX(tou32)(T_char *__restrict nptr,
                 T_char **endptr, int base) {
 return libc_strX(tou32_l)(nptr,endptr,base,NULL);
}
EXPORT_SYMBOL(tou32);

CRT_STRING u64 LIBCCALL
libc_strX(tou64)(T_char *__restrict nptr,
                 T_char **endptr, int base) {
 return libc_strX(tou64_l)(nptr,endptr,base,NULL);
}
EXPORT_SYMBOL(tou64);

CRT_STRING float LIBCCALL
libc_strX(tof)(T_char *__restrict nptr,
               T_char **endptr) {
 return libc_strX(tof_l)(nptr,endptr,NULL);
}
EXPORT_SYMBOL(tof);

CRT_STRING double LIBCCALL
libc_strX(tod)(T_char *__restrict nptr,
               T_char **endptr) {
 return libc_strX(tod_l)(nptr,endptr,NULL);
}
EXPORT_SYMBOL(tod);

CRT_STRING long double LIBCCALL
libc_strX(told)(T_char *__restrict nptr,
                T_char **endptr) {
 return libc_strX(told_l)(nptr,endptr,NULL);
}
// EXPORT_SYMBOL(told); /* Exported manually due to DOS using the same type for `double' and `long double' */


CRT_STRING u32 LIBCCALL
libc_aX(tou32_l)(T_char *__restrict nptr, locale_t locale) {
 while (T_isspace_l(*nptr,locale)) ++nptr;
 return libc_strX(tou32_l)(nptr,NULL,10,locale);
}
EXPORT_SYMBOLa(tou32_l);

CRT_STRING u64 LIBCCALL
libc_aX(tou64_l)(T_char *__restrict nptr, locale_t locale) {
 while (T_isspace_l(*nptr,locale)) ++nptr;
 return libc_strX(tou64_l)(nptr,NULL,10,locale);
}
EXPORT_SYMBOLa(tou64_l);

CRT_STRING s32 LIBCCALL
libc_aX(to32_l)(T_char *__restrict nptr, locale_t locale) {
 while (T_isspace_l(*nptr,locale)) ++nptr;
 return libc_strX(to32_l)(nptr,NULL,10,locale);
}
EXPORT_SYMBOLa(to32_l);

CRT_STRING s64 LIBCCALL
libc_aX(to64_l)(T_char *__restrict nptr, locale_t locale) {
 while (T_isspace_l(*nptr,locale)) ++nptr;
 return libc_strX(to64_l)(nptr,NULL,10,locale);
}
EXPORT_SYMBOLa(to64_l);

CRT_STRING float LIBCCALL
libc_aX(tof_l)(T_char *__restrict nptr, locale_t locale) {
 while (T_isspace_l(*nptr,locale)) ++nptr;
 return libc_strX(tof_l)(nptr,NULL,locale);
}
EXPORT_SYMBOLa(tof_l);

CRT_STRING double LIBCCALL
libc_aX(tod_l)(T_char *__restrict nptr, locale_t locale) {
 while (T_isspace_l(*nptr,locale)) ++nptr;
 return libc_strX(tod_l)(nptr,NULL,locale);
}
EXPORT_SYMBOLa(tod_l);

CRT_STRING long double LIBCCALL
libc_aX(told_l)(T_char *__restrict nptr, locale_t locale) {
 while (T_isspace_l(*nptr,locale)) ++nptr;
 return libc_strX(told_l)(nptr,NULL,locale);
}
// EXPORT_SYMBOL(told); /* Exported manually due to DOS using the same type for `double' and `long double' */


CRT_STRING s32 LIBCCALL
libc_aX(to32)(T_char *__restrict nptr) {
 return libc_aX(to32_l)(nptr,NULL);
}
EXPORT_SYMBOLa(to32);

CRT_STRING s64 LIBCCALL
libc_aX(to64)(T_char *__restrict nptr) {
 return libc_aX(to64_l)(nptr,NULL);
}
EXPORT_SYMBOLa(to64);

CRT_STRING u32 LIBCCALL
libc_aX(tou32)(T_char *__restrict nptr) {
 return libc_aX(tou32_l)(nptr,NULL);
}
EXPORT_SYMBOLa(tou32);

CRT_STRING u64 LIBCCALL
libc_aX(tou64)(T_char *__restrict nptr) {
 return libc_aX(tou64_l)(nptr,NULL);
}
EXPORT_SYMBOLa(tou64);

CRT_STRING float LIBCCALL
libc_aX(tof)(T_char *__restrict nptr) {
 return libc_aX(tof_l)(nptr,NULL);
}
EXPORT_SYMBOLa(tof);

CRT_STRING double LIBCCALL
libc_aX(tod)(T_char *__restrict nptr) {
 return libc_aX(tod_l)(nptr,NULL);
}
EXPORT_SYMBOLa(tod);

CRT_STRING long double LIBCCALL
libc_aX(told)(T_char *__restrict nptr) {
 return libc_aX(told_l)(nptr,NULL);
}
//EXPORT_SYMBOLa(told);



#endif /* !CONFIG_LIBC_LIMITED_API */

DECL_END

#undef libc_memcpystr
#undef libc_mempcpystr
#undef libc_memmovestr
#undef libc_mempmovestr
#undef libc_memsetstr
#undef libc_mempsetstr

#undef T_NUL
#undef T_sizeof
#undef T_int
#undef T_char
#undef T_schar
#undef T_uchar
#undef T_tolower
#undef T_toupper
#undef T_tolower_l
#undef T_toupper_l
#undef T_isspace
#undef T_isspace_l
#undef glob_aX
#undef libc_aX
#undef glob_strX
#undef libc_strX
#undef glob_stpX
#undef libc_stpX
#undef glob_fuzzy_strX
#undef libc_fuzzy_strX
#undef glob_wildstrX
#undef libc_wildstrX
#undef FORMAT_OPTION_CHARTYPE
