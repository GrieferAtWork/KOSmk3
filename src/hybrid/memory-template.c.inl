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
#define SIZE  2
#endif

#ifndef CONFIG_LIBC_LIMITED_API
#include "../libs/libc/libc.h"
#include "../libs/libc/malloc.h"
#endif

#include <assert.h>
#include <alloca.h>
#include <stdbool.h>

DECL_BEGIN

#if SIZE == 1
#define T     u8
#define Ts    s8
#define Ti    int
#define Tsi   int
#define F2(x) x
#elif SIZE == 2
#define T     u16
#define Ts    s16
#define F2(x) x##w
#elif SIZE == 4
#define T     u32
#define Ts    s32
#define F2(x) x##l
#elif SIZE == 8
#define T     u64
#define Ts    s64
#define F2(x) x##q
#else
#error Invalid SIZE
#endif
#ifndef Ti
#define Ti    T
#endif
#ifndef Tsi
#define Tsi   Ts
#endif


#define STRING_LARGE_OP_LIMIT 0x400
#ifndef CONFIG_NO_64BIT_STRING
#undef CONFIG_NATIVE_64BIT_STRINGS
#if __SIZEOF_POINTER__ >= 8
#define CONFIG_NATIVE_64BIT_STRINGS 1
#endif
#define STRING_ALIGNMENT_MASK 7
#else
#define STRING_ALIGNMENT_MASK 3
#endif

#define F(x) F2(x)

CRT_STRING T *LIBCCALL
F(libc_memcpy)(T *__restrict dst,
               T *__restrict src,
               size_t n) {
#if SIZE == 1
 if (n >= STRING_LARGE_OP_LIMIT) {
  switch (n & STRING_ALIGNMENT_MASK) {
  case 0:
#ifdef CONFIG_NATIVE_64BIT_STRINGS
   return (T *)libc_memcpyq((u64 *)dst,(u64 *)src,n >> 3);
#endif /* CONFIG_NATIVE_64BIT_STRINGS */
  case 4: return (T *)libc_memcpyl((u32 *)dst,(u32 *)src,n >> 2);
  case 6:
  case 2: return (T *)libc_memcpyw((u16 *)dst,(u16 *)src,n >> 1);
  default: break;
  }
 }
#endif
 {
  T *result = dst;
  while (n--)
      *dst++ = *src++;
  return result;
 }
}

CRT_STRING T *LIBCCALL
F(libc_memset)(T *__restrict dst,
               Ti byte, size_t n) {
#if SIZE == 1 && !defined(__OPTIMIZE_SIZE__)
 if (n >= STRING_LARGE_OP_LIMIT) {
  switch (n & STRING_ALIGNMENT_MASK) {
#ifdef CONFIG_NATIVE_64BIT_STRINGS
  {
   u32 temp;
  case 0:
   temp = ((u16)byte << 8 |
           (u16)byte);
   temp = ((u32)temp << 16 |
           (u32)temp);
   return (T *)libc_memsetq((u64 *)dst,
                           ((u64)temp << 32 |
                            (u64)temp),
                             n >> 3);
  }
#endif /* CONFIG_NATIVE_64BIT_STRINGS */
  {
   u16 temp;
#ifndef CONFIG_NATIVE_64BIT_STRINGS
  case 0:
#endif /* !CONFIG_NATIVE_64BIT_STRINGS */
  case 4:
   temp = ((u16)byte << 8 |
           (u16)byte);
   return (T *)libc_memsetl((u32 *)dst,
                           ((u32)temp << 16 |
                            (u32)temp),
                             n >> 2);
  }
  {
  case 6:
  case 2:
   return (T *)libc_memsetw((u16 *)dst,
                           ((u16)byte << 8 |
                            (u16)byte),
                             n >> 1);
  }
  default: break;
  }
 }
#endif
 {
  T *result = dst;
  COMPILER_BARRIER();
  while (n--)
      *dst++ = (T)byte;
  return result;
 }
}


CRT_STRING T *LIBCCALL
F(libc_mempcpy)(T *__restrict dst,
                T *__restrict src,
                size_t n) {
 return F(libc_memcpy)(dst,src,n) + n;
}

CRT_STRING T *LIBCCALL
PP_CAT2(F(libc_memcpy),_d)(T *__restrict dst,
                           T *__restrict src,
                           size_t n, DEBUGINFO) {
 assertf_d(!((uintptr_t)dst > (uintptr_t)src && (uintptr_t)dst < ((uintptr_t)src+n*SIZE)),
              DEBUGINFO_FWD,PP_STR(F(memcpy)) "(%p,%p,%Iu) : %Iu bytes from dst are overlapping with src (use `memmove' instead)",
              dst,src,n,((uintptr_t)src+n*SIZE)-(uintptr_t)dst);
 assertf_d(!((uintptr_t)src >(uintptr_t)dst && (uintptr_t)src < ((uintptr_t)dst+n*SIZE)),
              DEBUGINFO_FWD,PP_STR(F(memcpy)) "(%p,%p,%Iu) : %Iu bytes from src are overlapping with dst (use `memmove' instead)",
              dst,src,n,((uintptr_t)dst+n*SIZE)-(uintptr_t)src);
 return F(libc_memcpy)(dst,src,n);
}

CRT_STRING T *LIBCCALL
PP_CAT2(F(libc_mempcpy),_d)(T *__restrict dst,
                            T *__restrict src,
                            size_t n, DEBUGINFO) {
 assertf_d(!((uintptr_t)dst > (uintptr_t)src && (uintptr_t)dst < ((uintptr_t)src+n*SIZE)),
              DEBUGINFO_FWD,PP_STR(F(memcpy)) "(%p,%p,%Iu) : %Iu bytes from dst are overlapping with src (use `memmove' instead)",
              dst,src,n,((uintptr_t)src+n*SIZE)-(uintptr_t)dst);
 assertf_d(!((uintptr_t)src >(uintptr_t)dst && (uintptr_t)src < ((uintptr_t)dst+n*SIZE)),
              DEBUGINFO_FWD,PP_STR(F(memcpy)) "(%p,%p,%Iu) : %Iu bytes from src are overlapping with dst (use `memmove' instead)",
              dst,src,n,((uintptr_t)dst+n*SIZE)-(uintptr_t)src);
 return F(libc_mempcpy)(dst,src,n);
}

CRT_STRING T *LIBCCALL
F(libc_mempset)(T *__restrict p,
                Ts byte, size_t n) {
 return F(libc_memset)(p,byte,n) + n;
}

CRT_STRING Tsi LIBCCALL
F(libc_memcmp)(T *a, T *b, size_t n) {
 T av,bv; av = bv = 0;
 while (n-- && ((av = *a++) == (bv = *b++)));
 return (Tsi)av - (Tsi)bv;
}

CRT_STRING T *LIBCCALL
F(libc_memchr)(T *__restrict haystack, Ti needle, size_t n) {
 for (; n--; ++haystack) {
  if (*haystack == (T)needle)
       return haystack;
 }
 return NULL;
}
CRT_STRING T *LIBCCALL
F(libc_memxchr)(T *__restrict haystack, Ti needle, size_t n) {
 for (; n--; ++haystack) {
  if (*haystack != (T)needle)
       return haystack;
 }
 return NULL;
}
CRT_STRING T *LIBCCALL
F(libc_memrchr)(T *__restrict haystack, Ti needle, size_t n) {
 haystack += n;
 while (n--) {
  if (*--haystack == (T)needle)
      return haystack;
 }
 return NULL;
}
CRT_STRING T *LIBCCALL
F(libc_memrxchr)(T *__restrict haystack, Ti needle, size_t n) {
 haystack += n;
 while (n--) {
  if (*--haystack != (T)needle)
      return haystack;
 }
 return NULL;
}

CRT_STRING T *LIBCCALL
F(libc_memend)(T *__restrict haystack, Ti needle, size_t n) {
 for (; n--; ++haystack) {
  if (*haystack == (T)needle)
       break;
 }
 return haystack;
}
CRT_STRING T *LIBCCALL
F(libc_memxend)(T *__restrict haystack, Ti needle, size_t n) {
 for (; n--; ++haystack) {
  if (*haystack != (T)needle)
       break;
 }
 return haystack;
}
CRT_STRING T *LIBCCALL
F(libc_memrend)(T *__restrict haystack, Ti needle, size_t n) {
 haystack += n;
 while (n--) {
  if (*--haystack == (T)needle)
      break;
 }
 return haystack;
}
CRT_STRING T *LIBCCALL
F(libc_memrxend)(T *__restrict haystack, Ti needle, size_t n) {
 haystack += n;
 while (n--) {
  if (*--haystack != (T)needle)
      break;
 }
 return haystack;
}

CRT_STRING size_t LIBCCALL
F(libc_memlen)(T *__restrict haystack, Ti needle, size_t n) {
 return (size_t)(F(libc_memend)(haystack,needle,n) - haystack);
}
CRT_STRING size_t LIBCCALL
F(libc_memxlen)(T *__restrict haystack, Ti needle, size_t n) {
 return (size_t)(F(libc_memxend)(haystack,needle,n) - haystack);
}
CRT_STRING size_t LIBCCALL
F(libc_memrlen)(T *__restrict haystack, Ti needle, size_t n) {
 return (size_t)(F(libc_memrend)(haystack,needle,n) - haystack);
}
CRT_STRING size_t LIBCCALL
F(libc_memrxlen)(T *__restrict haystack, Ti needle, size_t n) {
 return (size_t)(F(libc_memrxend)(haystack,needle,n) - haystack);
}

CRT_STRING T *LIBCCALL
F(libc_rawmemchr)(T *__restrict haystack, Ti needle) {
 for (;; ++haystack) {
  if (*haystack == (T)needle)
       break;
 }
 return haystack;
}
CRT_STRING T *LIBCCALL
F(libc_rawmemxchr)(T *__restrict haystack, Ti needle) {
 for (;; ++haystack) {
  if (*haystack != (T)needle)
       break;
 }
 return haystack;
}

CRT_STRING T *LIBCCALL
F(libc_rawmemrchr)(T *__restrict haystack, Ti needle) {
 for (;;) {
  if (*--haystack == (T)needle)
       break;
 }
 return haystack;
}

CRT_STRING T *LIBCCALL
F(libc_rawmemrxchr)(T *__restrict haystack, Ti needle) {
 for (;;) {
  if (*--haystack != (T)needle)
       break;
 }
 return haystack;
}

CRT_STRING size_t LIBCCALL
F(libc_rawmemlen)(T *__restrict haystack, Ti needle) {
 return (size_t)(F(libc_rawmemchr)(haystack,needle) - haystack);
}

CRT_STRING size_t LIBCCALL
F(libc_rawmemxlen)(T *__restrict haystack, Ti needle) {
 return (size_t)(F(libc_rawmemxchr)(haystack,needle) - haystack);
}

CRT_STRING size_t LIBCCALL
F(libc_rawmemrlen)(T *__restrict haystack, Ti needle) {
 return (size_t)(F(libc_rawmemrchr)(haystack,needle) - haystack);
}

CRT_STRING size_t LIBCCALL
F(libc_rawmemrxlen)(T *__restrict haystack, Ti needle) {
 return (size_t)(F(libc_rawmemrxchr)(haystack,needle) - haystack);
}

#if SIZE != 1
CRT_STRING T *LIBCCALL
F(libc_mempat)(T *__restrict dst, Ti pattern, size_t n_bytes) {
 byte_t *iter = (byte_t *)dst;
#if SIZE == 2
 if (n_bytes && (uintptr_t)iter & (SIZE-1)) {
  *iter = ((u8 *)&pattern)[1];
  ++iter,--n_bytes;
 }
#else
 while (n_bytes && (uintptr_t)iter & (SIZE-1)) {
  *iter = ((u8 *)&pattern)[(uintptr_t)iter & (SIZE-1)];
  ++iter,--n_bytes;
 }
#endif
 F(libc_memset)((T *)iter,(Ti)pattern,(size_t)(n_bytes/SIZE));
 iter += n_bytes & ~(SIZE-1);
 n_bytes &= (SIZE-1);
#if SIZE == 2
 if (n_bytes) *iter = ((u8 *)&pattern)[0];
#else
 while (n_bytes) {
  *iter = ((u8 *)&pattern)[(uintptr_t)iter & (SIZE-1)];
  ++iter,--n_bytes;
 }
#endif
 return dst;
}
#endif

CRT_STRING T *LIBCCALL
F(libc_memmove)(T *dst, T *src, size_t n) {
 T *iter,*end;
 if (dst < src) {
  end = (iter = (T *)dst)+n;
  while (iter != end) *iter++ = *src++;
 } else {
  iter = (end = (T *)dst)+n,src += n;
  while (iter != end) *--iter = *--src;
 }
 return dst;
}

CRT_STRING T *LIBCCALL
F(libc_mempmove)(T *dst, T *src, size_t n) {
 T *iter,*end;
 if (dst < src) {
  end = (iter = (T *)dst)+n;
  while (iter != end) *iter++ = *src++;
 } else {
  iter = (end = (T *)dst)+n,src += n;
  while (iter != end) *--iter = *--src;
  iter += n;
 }
 return iter;
}


DEFINE_PUBLIC_WEAK_ALIAS(F(memcpy),F(libc_memcpy));
DEFINE_PUBLIC_WEAK_ALIAS(F(mempcpy),F(libc_mempcpy));
DEFINE_PUBLIC_WEAK_ALIAS(F(memset),F(libc_memset));
DEFINE_PUBLIC_WEAK_ALIAS(F(mempset),F(libc_mempset));
DEFINE_PUBLIC_WEAK_ALIAS(F(memmove),F(libc_memmove));
DEFINE_PUBLIC_WEAK_ALIAS(F(mempmove),F(libc_mempmove));
DEFINE_PUBLIC_WEAK_ALIAS(F(memcmp),F(libc_memcmp));
DEFINE_PUBLIC_WEAK_ALIAS(F(memchr),F(libc_memchr));
DEFINE_PUBLIC_WEAK_ALIAS(F(memxchr),F(libc_memxchr));
DEFINE_PUBLIC_WEAK_ALIAS(F(memrchr),F(libc_memrchr));
DEFINE_PUBLIC_WEAK_ALIAS(F(memrxchr),F(libc_memrxchr));
DEFINE_PUBLIC_WEAK_ALIAS(F(memend),F(libc_memend));
DEFINE_PUBLIC_WEAK_ALIAS(F(memxend),F(libc_memxend));
DEFINE_PUBLIC_WEAK_ALIAS(F(memrend),F(libc_memrend));
DEFINE_PUBLIC_WEAK_ALIAS(F(memrxend),F(libc_memrxend));
DEFINE_PUBLIC_WEAK_ALIAS(F(memlen),F(libc_memlen));
DEFINE_PUBLIC_WEAK_ALIAS(F(memxlen),F(libc_memxlen));
DEFINE_PUBLIC_WEAK_ALIAS(F(memrlen),F(libc_memrlen));
DEFINE_PUBLIC_WEAK_ALIAS(F(memrxlen),F(libc_memrxlen));
DEFINE_PUBLIC_WEAK_ALIAS(F(rawmemchr),F(libc_rawmemchr));
DEFINE_PUBLIC_WEAK_ALIAS(F(rawmemxchr),F(libc_rawmemxchr));
DEFINE_PUBLIC_WEAK_ALIAS(F(rawmemrchr),F(libc_rawmemrchr));
DEFINE_PUBLIC_WEAK_ALIAS(F(rawmemrxchr),F(libc_rawmemrxchr));
DEFINE_PUBLIC_WEAK_ALIAS(F(rawmemlen),F(libc_rawmemlen));
DEFINE_PUBLIC_WEAK_ALIAS(F(rawmemxlen),F(libc_rawmemxlen));
DEFINE_PUBLIC_WEAK_ALIAS(F(rawmemrlen),F(libc_rawmemrlen));
DEFINE_PUBLIC_WEAK_ALIAS(F(rawmemrxlen),F(libc_rawmemrxlen));
DEFINE_PUBLIC_WEAK_ALIAS(PP_CAT2(F(_memcpy),_d),PP_CAT2(F(libc_memcpy),_d));
DEFINE_PUBLIC_WEAK_ALIAS(PP_CAT2(F(_mempcpy),_d),PP_CAT2(F(libc_mempcpy),_d));
#if SIZE > 1
DEFINE_PUBLIC_WEAK_ALIAS(F(mempat),F(libc_mempat));
#endif


#ifndef CONFIG_LIBC_LIMITED_API
#if SIZE != 8
DEFINE_PUBLIC_WEAK_ALIAS(F(fuzzy_memcmp),F(libc_fuzzy_memcmp));
CRT_STRING size_t LIBCCALL
F(libc_fuzzy_memcmp)(T *a, size_t a_words, T *b, size_t b_words) {
 size_t *v0,*v1,i,j,cost,temp;
 bool is_malloc = false;
 if unlikely(!a_words) return b_words;
 if unlikely(!b_words) return a_words;
 if (b_words > a_words) {
  { T *temp = a; a = b; b = temp; }
  { size_t temp = a_words; a_words = b_words; b_words = temp; }
 }
 if (b_words >= 128) {
  v0 = (size_t *)libc_malloc((b_words+1)*sizeof(size_t));
  if unlikely(!v0) goto use_alloca;
  v1 = (size_t *)libc_malloc((b_words+1)*sizeof(size_t));
  if unlikely(!v1) { libc_free(v1); goto use_alloca; }
  is_malloc = true;
 } else use_alloca: {
  v0 = (size_t *)alloca((b_words+1)*sizeof(size_t));
  v1 = (size_t *)alloca((b_words+1)*sizeof(size_t));
 }
 for (i = 0; i < b_words; ++i) v0[i] = i;
 for (i = 0; i < a_words; ++i) {
  v1[0] = i+1;
  for (j = 0; j < b_words; j++) {
   cost  = a[i] != b[j];
   cost += v0[j];
   temp  = v1[j]+1;
   if (cost > temp) cost = temp;
   temp  = v0[j+1]+1;
   if (cost > temp) cost = temp;
   v1[j+1] = cost;
  }
#if __SIZEOF_SIZE_T__ == 8
  libc_memcpyq((u64 *)v0,(u64 *)v1,b_words);
#elif __SIZEOF_SIZE_T__ == 4
  libc_memcpyl((u32 *)v0,(u32 *)v1,b_words);
#elif __SIZEOF_SIZE_T__ == 2
  libc_memcpyw((u16 *)v0,(u16 *)v1,b_words);
#else
  libc_memcpy((u8 *)v0,(u8 *)v1,b_words*sizeof(size_t));
#endif
 }
 temp = v1[b_words];
 if (is_malloc)
     libc_free(v1),
     libc_free(v0);
 return temp;
}
DEFINE_PUBLIC_WEAK_ALIAS(F(fuzzy_memcasecmp),F(libc_fuzzy_memcasecmp));
CRT_STRING size_t LIBCCALL
F(libc_fuzzy_memcasecmp)(T *a, size_t a_words, T *b, size_t b_words) {
 size_t *v0,*v1,i,j,cost,temp;
 bool is_malloc = false;
 if unlikely(!a_words) return b_words;
 if unlikely(!b_words) return a_words;
 if (b_words > a_words) {
  { T *temp = a; a = b; b = temp; }
  { size_t temp = a_words; a_words = b_words; b_words = temp; }
 }
 if (b_words >= 128) {
  v0 = (size_t *)libc_malloc((b_words+1)*sizeof(size_t));
  if unlikely(!v0) goto use_alloca;
  v1 = (size_t *)libc_malloc((b_words+1)*sizeof(size_t));
  if unlikely(!v1) { libc_free(v1); goto use_alloca; }
  is_malloc = true;
 } else use_alloca: {
  v0 = (size_t *)alloca((b_words+1)*sizeof(size_t));
  v1 = (size_t *)alloca((b_words+1)*sizeof(size_t));
 }
 for (i = 0; i < b_words; ++i) v0[i] = i;
 for (i = 0; i < a_words; ++i) {
  v1[0] = i+1;
  for (j = 0; j < b_words; j++) {
   cost  = a[i] != b[j];
#if SIZE == 1
   if (cost)
       cost = libc_tolower(a[i]) != libc_tolower(b[j]);
#else
   if (cost)
       cost = libc_towlower(a[i]) != libc_towlower(b[j]);
#endif
   cost += v0[j];
   temp  = v1[j]+1;
   if (cost > temp) cost = temp;
   temp  = v0[j+1]+1;
   if (cost > temp) cost = temp;
   v1[j+1] = cost;
  }
#if __SIZEOF_SIZE_T__ == 8
  libc_memcpyq((u64 *)v0,(u64 *)v1,b_words);
#elif __SIZEOF_SIZE_T__ == 4
  libc_memcpyl((u32 *)v0,(u32 *)v1,b_words);
#elif __SIZEOF_SIZE_T__ == 2
  libc_memcpyw((u16 *)v0,(u16 *)v1,b_words);
#else
  libc_memcpy((u8 *)v0,(u8 *)v1,b_words*sizeof(size_t));
#endif
 }
 if (is_malloc) libc_free(v1),libc_free(v0);
 temp = v1[b_words];
 if (is_malloc)
     libc_free(v1),
     libc_free(v0);
 return temp;
}
DEFINE_PUBLIC_WEAK_ALIAS(PP_CAT2(F(fuzzy_memcasecmp),_l),PP_CAT2(F(libc_fuzzy_memcasecmp),_l));
CRT_STRING size_t LIBCCALL
PP_CAT2(F(libc_fuzzy_memcasecmp),_l)(T *a, size_t a_words,
                                     T *b, size_t b_words,
                                     locale_t locale) {
 size_t *v0,*v1,i,j,cost,temp;
 bool is_malloc = false;
 if unlikely(!a_words) return b_words;
 if unlikely(!b_words) return a_words;
 if (b_words > a_words) {
  { T *temp = a; a = b; b = temp; }
  { size_t temp = a_words; a_words = b_words; b_words = temp; }
 }
 if (b_words >= 128) {
  v0 = (size_t *)libc_malloc((b_words+1)*sizeof(size_t));
  if unlikely(!v0) goto use_alloca;
  v1 = (size_t *)libc_malloc((b_words+1)*sizeof(size_t));
  if unlikely(!v1) { libc_free(v1); goto use_alloca; }
  is_malloc = true;
 } else use_alloca: {
  v0 = (size_t *)alloca((b_words+1)*sizeof(size_t));
  v1 = (size_t *)alloca((b_words+1)*sizeof(size_t));
 }
 for (i = 0; i < b_words; ++i) v0[i] = i;
 for (i = 0; i < a_words; ++i) {
  v1[0] = i+1;
  for (j = 0; j < b_words; j++) {
   cost  = a[i] != b[j];
#if SIZE == 1
   if (cost)
       cost = libc_tolower_l(a[i],locale) != libc_tolower_l(b[j],locale);
#else
   if (cost)
       cost = libc_towlower_l(a[i],locale) != libc_towlower_l(b[j],locale);
#endif
   cost += v0[j];
   temp  = v1[j]+1;
   if (cost > temp) cost = temp;
   temp  = v0[j+1]+1;
   if (cost > temp) cost = temp;
   v1[j+1] = cost;
  }
#if __SIZEOF_SIZE_T__ == 8
  libc_memcpyq((u64 *)v0,(u64 *)v1,b_words);
#elif __SIZEOF_SIZE_T__ == 4
  libc_memcpyl((u32 *)v0,(u32 *)v1,b_words);
#elif __SIZEOF_SIZE_T__ == 2
  libc_memcpyw((u16 *)v0,(u16 *)v1,b_words);
#else
  libc_memcpy((u8 *)v0,(u8 *)v1,b_words*sizeof(size_t));
#endif
 }
 if (is_malloc) libc_free(v1),libc_free(v0);
 temp = v1[b_words];
 if (is_malloc)
     libc_free(v1),
     libc_free(v0);
 return temp;
}

#endif

CRT_STRING T *LIBCCALL
F(libc_memrev)(T *__restrict buf, size_t num_words) {
 T *iter,*end;
 end = (iter = buf)+num_words;
 while (iter < end) {
  T temp = *iter;
  *iter++ = *--end;
  *end = temp;
 }
 return buf;
}
DEFINE_PUBLIC_WEAK_ALIAS(F(memrev),F(libc_memrev));

#endif /* !CONFIG_LIBC_LIMITED_API */


DECL_END

#undef T
#undef Ts
#undef Ti
#undef Tsi
#undef F2
#undef F

#undef SIZE
