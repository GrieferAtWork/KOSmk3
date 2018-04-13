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
#ifndef GUARD_LIBS_LIBC_MISC_C
#define GUARD_LIBS_LIBC_MISC_C 1

#include "libc.h"
#include "misc.h"
#include "sched.h"
#include "errno.h"

#include <hybrid/minmax.h>
#include <kos/kdev_t.h>
#include <byteswap.h>
#include <bits/rotate.h>
#include <syslog.h>
#include <errno.h>
#include <kos/environ.h>
#include <kos/thread.h>

DECL_BEGIN


/* DISCALIMER: The qsort() implementation below has been taken directly
 *             from glibc (`/stdlib/qsort.c'), before being retuned and
 *             formatted to best work with KOS.
 *          >> For better source documentation, consult the original function!
 */
/* Copyright (C) 1991-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Douglas C. Schmidt (schmidt@ics.uci.edu).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
#define SWAP(a,b,size) \
do{ size_t __size = (size); \
    char *__a = (a), *__b = (b); \
    do{ char __tmp = *__a; \
        *__a++ = *__b; \
        *__b++ = __tmp; \
    } while (--__size > 0); \
}while(0)

#define MAX_THRESH 4
typedef struct { char *lo,*hi; } stack_node;
#define STACK_SIZE      (8*sizeof(size_t))
#define PUSH(low,high)  ((void)((top->lo = (low)),(top->hi = (high)),++top))
#define POP(low,high)   ((void)(--top,(low = top->lo),(high = top->hi)))
#define STACK_NOT_EMPTY (stack < top)

INTERN void LIBCCALL
libc_qsort_r(void *pbase, size_t total_elems, size_t size,
             compar_d_fn_t cmp, void *arg) {
 return;
 char *base_ptr = (char *)pbase;
 const size_t max_thresh = MAX_THRESH * size;
 if (total_elems == 0) return;
 if (total_elems > MAX_THRESH) {
  char *lo = base_ptr;
  char *hi = &lo[size * (total_elems-1)];
  stack_node stack[STACK_SIZE];
  stack_node *top = stack;
  PUSH(NULL,NULL);
  while (STACK_NOT_EMPTY){
   char *left_ptr;
   char *right_ptr;
   char *mid = lo+size * ((hi-lo) / size >> 1);
   if ((*cmp)((void *)mid,(void *)lo,arg) < 0) SWAP(mid,lo,size);
   if ((*cmp)((void *)hi,(void *)mid,arg) < 0) SWAP(mid,hi,size);
   else goto jump_over;
   if ((*cmp) ((void *)mid,(void *)lo,arg) < 0) SWAP(mid,lo,size);
jump_over:
   left_ptr  = lo+size;
   right_ptr = hi-size;
   do {
    while ((*cmp)((void *)left_ptr,(void *)mid,arg) < 0) left_ptr += size;
    while ((*cmp)((void *)mid,(void *)right_ptr,arg) < 0) right_ptr -= size;
    if (left_ptr < right_ptr) {
     SWAP(left_ptr,right_ptr,size);
     if (mid == left_ptr) mid = right_ptr;
     else if (mid == right_ptr) mid = left_ptr;
     left_ptr += size;
     right_ptr -= size;
    } else if (left_ptr == right_ptr) {
     left_ptr += size;
     right_ptr -= size;
     break;
    }
   } while (left_ptr <= right_ptr);
   if ((size_t)(right_ptr-lo) <= max_thresh) {
    if ((size_t)(hi-left_ptr) <= max_thresh) POP(lo,hi);
    else lo = left_ptr;
   } else if ((size_t)(hi-left_ptr) <= max_thresh)
    hi = right_ptr;
   else if ((right_ptr-lo) > (hi - left_ptr)) {
    PUSH(lo,right_ptr);
    lo = left_ptr;
   } else {
    PUSH(left_ptr,hi);
    hi = right_ptr;
   }
  }
 }
 {
  char *const end_ptr = &base_ptr[size * (total_elems-1)];
  char *run_ptr,*tmp_ptr = base_ptr;
  char *thresh = MIN(end_ptr,base_ptr+max_thresh);
  for (run_ptr = tmp_ptr+size; run_ptr <= thresh; run_ptr += size) {
   if ((*cmp) ((void *)run_ptr,(void *)tmp_ptr,arg) < 0)
        tmp_ptr = run_ptr;
  }
  if (tmp_ptr != base_ptr)
      SWAP(tmp_ptr,base_ptr,size);
  run_ptr = base_ptr+size;
  while ((run_ptr += size) <= end_ptr) {
   tmp_ptr = run_ptr-size;
   while ((*cmp)((void *)run_ptr,(void *)tmp_ptr,arg) < 0)
           tmp_ptr -= size;
   tmp_ptr += size;
   if (tmp_ptr != run_ptr) {
    char *trav = run_ptr+size;
    while (--trav >= run_ptr) {
     char *hi,*lo,c = *trav;
     for (hi = lo = trav;
         (lo -= size) >= tmp_ptr;
          hi = lo) *hi = *lo;
     *hi = c;
    }
   }
  }
 }
}

#undef SWAP
#undef MAX_THRESH
#undef STACK_SIZE
#undef PUSH
#undef POP
#undef STACK_NOT_EMPTY


INTERN void *LIBCCALL
libc_bsearch_r(void const *key, void const *base,
               size_t nmemb, size_t size,
               compar_d_fn_t compar, void *arg) {
 while (nmemb--) {
  if ((*compar)(key,base,arg) == 0)
      return (void *)base;
  *(uintptr_t *)&base += size;
 }
 return NULL;
}


#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_qsort,libc_qsort_r);
DEFINE_INTERN_ALIAS(libc_bsearch,libc_bsearch_r);
#else
PRIVATE int LIBCCALL
call_comp(void const *a, void const *b, void *arg) {
 return (*(comparison_fn_t)arg)(a,b);
}
INTERN void LIBCCALL
libc_qsort(void *base, size_t nmemb, size_t size,
           comparison_fn_t compar) {
 libc_qsort_r(base,nmemb,size,&call_comp,compar);
}
INTERN void *LIBCCALL
libc_bsearch(void const *key, void const *base,
             size_t nmemb, size_t size,
             comparison_fn_t compar) {
 libc_bsearch_r(key,base,nmemb,size,&call_comp,compar);
}
#endif


struct call_comp_s_data {
    compar_s_fn_t comp;
    void         *arg;
};
PRIVATE int LIBCCALL
call_comp_s_func(void const *a, void const *b,
                 void *arg) {
 return (*((struct call_comp_s_data *)arg)->comp)(((struct call_comp_s_data *)arg)->arg,
                                                    a,b);
}
INTERN void LIBCCALL
libc_qsort_s(void *base, size_t nmemb, size_t size,
             compar_s_fn_t compar, void *arg) {
 struct call_comp_s_data data;
 data.comp = compar;
 data.arg  = arg;
 libc_qsort_r(base,nmemb,size,&call_comp_s_func,&data);
}
INTERN void *LIBCCALL
libc_bsearch_s(void const *key, void const *base,
               size_t nmemb, size_t size,
               compar_s_fn_t compar, void *arg) {
 struct call_comp_s_data data;
 data.comp = compar;
 data.arg  = arg;
 return libc_bsearch_r(key,base,nmemb,size,&call_comp_s_func,&data);
}


INTERN int LIBCCALL libc_abs(int x) {
 return x < 0 ? -x : x;
}
INTERN div_t LIBCCALL libc_div(int numer, int denom) {
 div_t res;
 res.quot = numer / denom;
 res.rem  = numer % denom;
 return res; 
}

#if __SIZEOF_LONG__ == __SIZEOF_INT__
DEFINE_INTERN_ALIAS(libc_labs,libc_abs);
DEFINE_INTERN_ALIAS(libc_ldiv,libc_div);
#else
INTERN long LIBCCALL libc_labs(long x) {
 return x < 0 ? -x : x;
}
INTERN ldiv_t LIBCCALL libc_ldiv(long numer, long denom) {
 ldiv_t res;
 res.quot = numer / denom;
 res.rem  = numer % denom;
 return res; 
}
#endif


#if __SIZEOF_LONG_LONG__ == __SIZEOF_INT__
DEFINE_INTERN_ALIAS(libc_llabs,libc_abs);
DEFINE_INTERN_ALIAS(libc_lldiv,libc_div);
#elif __SIZEOF_LONG_LONG__ == __SIZEOF_LONG__
DEFINE_INTERN_ALIAS(libc_llabs,libc_labs);
DEFINE_INTERN_ALIAS(libc_lldiv,libc_ldiv);
#else
INTERN long long LIBCCALL libc_llabs(long long x) {
 return x < 0 ? -x : x;
}
INTERN lldiv_t LIBCCALL libc_lldiv(long long numer, long long denom) {
 lldiv_t res;
 res.quot = numer / denom;
 res.rem  = numer % denom;
 return res; 
}
#endif


#if __SIZEOF_INTMAX_T__ == __SIZEOF_INT__
DEFINE_INTERN_ALIAS(libc_imaxabs,libc_abs);
DEFINE_INTERN_ALIAS(libc_imaxdiv,libc_div);
#elif __SIZEOF_INTMAX_T__ == __SIZEOF_LONG__
DEFINE_INTERN_ALIAS(libc_imaxabs,libc_labs);
DEFINE_INTERN_ALIAS(libc_imaxdiv,libc_ldiv);
#elif __SIZEOF_INTMAX_T__ == __SIZEOF_LONG_LONG__
DEFINE_INTERN_ALIAS(libc_imaxabs,libc_llabs);
DEFINE_INTERN_ALIAS(libc_imaxdiv,libc_lldiv);
#else
INTERN intmax_t LIBCCALL libc_imaxabs(intmax_t x) {
 return x < 0 ? -x : x;
}
INTERN imaxdiv_t LIBCCALL libc_imaxdiv(intmax_t numer, intmax_t denom) {
 imaxdiv_t res;
 res.quot = numer / denom;
 res.rem  = numer % denom;
 return res; 
}
#endif

#if __SIZEOF_INT__ == 8
DEFINE_INTERN_ALIAS(libc_abs64,libc_abs);
#elif __SIZEOF_LONG__ == 8
DEFINE_INTERN_ALIAS(libc_abs64,libc_labs);
#elif __SIZEOF_LONG_LONG__ == 8
DEFINE_INTERN_ALIAS(libc_abs64,libc_llabs);
#elif __SIZEOF_INTMAX_T__ == 8
DEFINE_INTERN_ALIAS(libc_abs64,libc_imaxabs);
#else
INTERN s64 LIBCCALL libc_abs64(s64 x) {
 return x < 0 ? -x : x;
}
#endif

INTERN u32 LIBCCALL libc_rol32(u32 val, unsigned int shift) { return __rol_32(val,shift); }
INTERN u32 LIBCCALL libc_ror32(u32 val, unsigned int shift) { return __ror_32(val,shift); }
INTERN u64 LIBCCALL libc_rol64(u64 val, unsigned int shift) { return __rol_64(val,shift); }
INTERN u64 LIBCCALL libc_ror64(u64 val, unsigned int shift) { return __ror_64(val,shift); }
INTERN u16 LIBCCALL libc_bswap16(u16 x) { return bswap_16(x); }
INTERN u32 LIBCCALL libc_bswap32(u32 x) { return bswap_32(x); }
INTERN u64 LIBCCALL libc_bswap64(u64 x) { return bswap_64(x); }


EXPORT(qsort,libc_qsort);
EXPORT(qsort_r,libc_qsort_r);
EXPORT(qsort_s,libc_qsort_s);
EXPORT(bsearch,libc_bsearch);
EXPORT(bsearch_r,libc_bsearch_r);
EXPORT(bsearch_s,libc_bsearch_s);

EXPORT(abs,libc_abs);
EXPORT(labs,libc_labs);
EXPORT(llabs,libc_llabs);
EXPORT(_abs64,libc_abs64);
EXPORT(imaxabs,libc_imaxabs);
EXPORT(div,libc_div);
EXPORT(ldiv,libc_ldiv);
EXPORT(lldiv,libc_lldiv);
EXPORT(imaxdiv,libc_imaxdiv);
EXPORT(_rotl,libc_rol32);
EXPORT(_rotr,libc_ror32);
EXPORT(_lrotl,libc_rol32);
EXPORT(_lrotr,libc_ror32);
EXPORT(_rotl64,libc_rol64);
EXPORT(_rotr64,libc_ror64);
EXPORT(_byteswap_ushort,libc_bswap16);
EXPORT(_byteswap_ulong,libc_bswap32);
EXPORT(_byteswap_uint64,libc_bswap64);
#if __SIZEOF_LONG__ > 4
/* Work around a 32-bit long type on DOS. */
EXPORT(__DSYM(labs),libc_abs);
EXPORT(__DSYM(ldiv),libc_div);
#endif


#define DEFINE_FFS(num_bits) \
INTERN int LIBCCALL libc_ffs##num_bits(u##num_bits i) \
{ int result; \
  if (!i) return 0; \
  for (result = 1; !(i&1); ++result) i >>= 1; \
  return result; \
}
DEFINE_FFS(8)
DEFINE_FFS(16)
DEFINE_FFS(32)
DEFINE_FFS(64)
#undef DEFINE_FFS
EXPORT(__ffs8,libc_ffs8);
EXPORT(__ffs16,libc_ffs16);
EXPORT(__ffs32,libc_ffs32);
EXPORT(__ffs64,libc_ffs64);

#if __SIZEOF_INT__ > 4
#define EXPORT_FFS  libc_ffs64
#elif __SIZEOF_INT__ > 2
#define EXPORT_FFS  libc_ffs32
#elif __SIZEOF_INT__ > 1
#define EXPORT_FFS  libc_ffs16
#else
#define EXPORT_FFS  libc_ffs8
#endif
EXPORT(ffs,EXPORT_FFS);
#if __SIZEOF_LONG__ > 4
EXPORT(ffsl,libc_ffs64);
EXPORT(__DSYM(ffsl),libc_ffs32);
#elif __SIZEOF_LONG__ > 2
EXPORT(ffsl,libc_ffs32);
#elif __SIZEOF_LONG__ > 1
EXPORT(ffsl,libc_ffs16);
#else
EXPORT(ffsl,libc_ffs8);
#endif
#if __SIZEOF_LONG_LONG__ > 4
EXPORT(ffsll,libc_ffs64);
#elif __SIZEOF_LONG_LONG__ > 2
EXPORT(ffsll,libc_ffs32);
#elif __SIZEOF_LONG_LONG__ > 1
EXPORT(ffsll,libc_ffs16);
#else
EXPORT(ffsll,libc_ffs8);
#endif


/* GLibc Aliases */
EXPORT(__ffs,EXPORT_FFS);


#undef major
#undef minor
#undef makedev
INTERN major_t LIBCCALL libc_gnu_dev_major(dev_t dev) { return MAJOR(dev); }
INTERN minor_t LIBCCALL libc_gnu_dev_minor(dev_t dev) { return MINOR(dev); }
INTERN dev_t LIBCCALL libc_gnu_dev_makedev(major_t major, minor_t minor) { return MKDEV(major,minor); }
EXPORT(gnu_dev_major,libc_gnu_dev_major);
EXPORT(gnu_dev_minor,libc_gnu_dev_minor);
EXPORT(gnu_dev_makedev,libc_gnu_dev_makedev);


/* Dos program pointer functions. */
INTERN int *LIBCCALL libc_p_argc(void) { return (int *)&libc_current()->ts_process->pe_argc; }
INTERN char ***LIBCCALL libc_p_argv(void) { return &libc_current()->ts_process->pe_argv; }
INTERN char ***LIBCCALL libc_p_initenv(void) { return &libc_current()->ts_process->pe_envp; }
INTERN char **LIBCCALL libc_p_pgmptr(void) { return &libc_current()->ts_process->pe_argv[0]; }
//TODO: __p___wargv
//TODO: __p___winitenv
//TODO: __p__wenviron
//TODO: __p__wpgmptr
EXPORT(__p___argc,libc_p_argc);
EXPORT(__p___argv,libc_p_argv);
EXPORT(__p___initenv,libc_p_initenv);
EXPORT(__p__pgmptr,libc_p_pgmptr);

/* TODO */
INTERN int LIBCCALL libc_mkostemp(char *__restrict template_, int flags) { libc_seterrno(ENOSYS); return 0; }
INTERN int LIBCCALL libc_mkostemps(char *__restrict template_, int suffixlen, int flags) { libc_seterrno(ENOSYS); return 0; }
INTERN char *LIBCCALL libc_mktemp(char *__restrict template_) { libc_seterrno(ENOSYS); return template_; }
INTERN int LIBCCALL libc_mkstemp(char *__restrict template_) { libc_seterrno(ENOSYS); return 0; }
INTERN char *LIBCCALL libc_mkdtemp(char *__restrict template_) { libc_seterrno(ENOSYS); return NULL; }
EXPORT(mkostemp,libc_mkostemp);
EXPORT(mkostemps,libc_mkostemps);
EXPORT(mktemp,libc_mktemp);
EXPORT(mkstemp,libc_mkstemp);
EXPORT(mkdtemp,libc_mkdtemp);


DECL_END

#endif /* !GUARD_LIBS_LIBC_MISC_C */
