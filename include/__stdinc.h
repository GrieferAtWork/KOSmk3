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
#ifndef ___STDINC_H
#define ___STDINC_H 1

#ifndef __NO_KOS_SYSTEM_HEADERS__
/* Indicator for user-applications that KOS's system headers are available.
 * >> #include <string.h> // Will define `__KOS_SYSTEM_HEADERS__'
 * >> #ifdef __KOS_SYSTEM_HEADERS__
 * >> #include <hybrid/compiler.h> // Pull in KOS-specific headers without relying on `__has_include'.
 * >> #endif
 */
#define __KOS_SYSTEM_HEADERS__ 1

/* Starting with the 3rd rendition, a version number has been
 * introduced to keep track of changes and allow API compatibility
 * between KOS Mk3 and its future relatives with versions starting
 * at v200.
 * Because v200 didn't have this marker and because v100 exists, too,
 * code that wishes to substitute this version number of older releases
 * should do the following:
 * >> #ifdef __KOS__
 * >> // Will pull in `__stdinc' for v200+, which
 * >> // defines `__KOS_VERSION__' for v300+
 * >> #include <stddef.h>
 * >> #ifndef __KOS_VERSION__
 * >> // Test for an old #include-guard found only in KOS Mk1 (<kos/config.h>)
 * >> // (that header is implicitly included by <stddef.h>, just as <__stdinc.h> is)
 * >> // NOTE: The naming convention for guards has changed since, and a
 * >> //       header of the same name would not be guarded as `_KOS_CONFIG_H',
 * >> //       `__GUARD_KOS_CONFIG_H' or `GUARD_KOS_CONFIG_H', meaning that
 * >> //       `__KOS_CONFIG_H__' will never be used again.
 * >> #ifdef __KOS_CONFIG_H__
 * >> #   define __KOS_VERSION__ 100
 * >> #else
 * >> #   define __KOS_VERSION__ 200
 * >> #endif
 * >> #endif
 * >> #endif
 */
#ifndef __KOS_VERSION__
#define __KOS_VERSION__        300
#endif /* !__KOS_VERSION__ */
#endif /* !__NO_KOS_SYSTEM_HEADERS__ */

/* ... */

#if defined(__cplusplus) || defined(__INTELLISENSE__) || \
  (!defined(__LINKER__) && !defined(__ASSEMBLY__) && \
   !defined(__ASSEMBLER__) && !defined(__assembler) && \
   !defined(__DEEMON__))
#define __CC__ 1 /* C Compiler. */
#define __CCAST(T) (T)
#else
#define __CCAST(T) /* Nothing */
#endif

#include "compiler/pp-generic.h"

#define __COMPILER_LENOF(arr)          (sizeof(arr)/sizeof(*(arr)))
#define __COMPILER_ENDOF(arr)   ((arr)+(sizeof(arr)/sizeof(*(arr))))
#define __COMPILER_STRLEN(str)         (sizeof(str)/sizeof(char)-1)
#define __COMPILER_STREND(str)  ((str)+(sizeof(str)/sizeof(char)-1))

#if !defined(__CC__)
#   include "compiler/other.h"
#elif defined(__GNUC__)
#   include "compiler/gcc.h"
#elif defined(_MSC_VER)
#   include "compiler/msvc.h"
#else
#   include "compiler/generic.h"
#endif

#ifdef __cplusplus
#   include "compiler/c++.h"
#else
#   include "compiler/c.h"
#endif

#ifndef __SYSDECL_BEGIN
#define __SYSDECL_BEGIN __DECL_BEGIN
#define __SYSDECL_END   __DECL_END
#endif /* !__SYSDECL_BEGIN */

#ifdef __INTELLISENSE__
#   define __NOTHROW       /* Nothing */
#elif defined(__cplusplus)
#   define __NOTHROW(prot) prot __CXX_NOEXCEPT
#elif defined(__NO_ATTR_NOTHROW)
#   define __NOTHROW(prot) prot
//#elif defined(__NO_ATTR_NOTHROW_SUFFIX)
//# define __NOTHROW(prot) __ATTR_NOTHROW prot
#else
#   define __NOTHROW(prot) __ATTR_NOTHROW prot
#endif

#define __FCALL                  __ATTR_FASTCALL
#define __KCALL                  __ATTR_STDCALL

#if defined(__COMPILER_HAVE_AUTOTYPE) && !defined(__NO_XBLOCK)
#   define __COMPILER_UNUSED(expr)  __XBLOCK({ __auto_type __expr = (expr); __expr; })
#elif defined(__COMPILER_HAVE_TYPEOF) && !defined(__NO_XBLOCK)
#   define __COMPILER_UNUSED(expr)  __XBLOCK({ __typeof__(expr) __expr = (expr); __expr; })
#else
#   define __COMPILER_UNUSED(expr) (expr)
#endif
#ifndef __COMPILER_IGNORE_UNINITIALIZED
#define __COMPILER_IGNORE_UNINITIALIZED(var) var
#endif

#define __COMPILER_OFFSETAFTER(s,m) ((__SIZE_TYPE__)(&((s *)0)->m+1))
#ifdef __INTELLISENSE__
#define __COMPILER_CONTAINER_OF(ptr,type,member) \
  (__XBLOCK({ __STATIC_ASSERT_MSG(__builtin_types_compatible_p( \
                                 __typeof__(ptr), \
                                 __typeof__(&((type *)0)->member)), \
                                 "\n\tCOMPILER_CONTAINER_OF(...)\n" \
                                 "\t\t'" #ptr "' must be a pointer \n\t\t'" #type "::" #member " *'\n"); \
             (void)0; }),((type *)((__UINTPTR_TYPE__)(ptr)-__builtin_offsetof(type,member))))
#else
#define __COMPILER_CONTAINER_OF(ptr,type,member) \
  ((type *)((__UINTPTR_TYPE__)(ptr)-__builtin_offsetof(type,member)))
#endif
#ifndef __DEFINE_PUBLIC_ALIAS
#ifdef __ASSEMBLER__
#   define __DEFINE_PRIVATE_ALIAS(new,old)      .local new; .set new, old;
#   define __DEFINE_PUBLIC_ALIAS(new,old)       .global new; .set new, old;
#   define __DEFINE_INTERN_ALIAS(new,old)       .global new; .hidden new; .set new, old;
#   define __DEFINE_PRIVATE_WEAK_ALIAS(new,old) .weak new; .local new; .set new, old;
#   define __DEFINE_PUBLIC_WEAK_ALIAS(new,old)  .weak new; .global new; .set new, old;
#   define __DEFINE_INTERN_WEAK_ALIAS(new,old)  .weak new; .global new; .hidden new; .set new, old;
#elif defined(__COMPILER_HAVE_GCC_ASM)
#   define __DEFINE_ALIAS_STR(x) #x
#   define __DEFINE_PRIVATE_ALIAS(new,old)      __asm__(".local " __DEFINE_ALIAS_STR(new) "\n.set " __DEFINE_ALIAS_STR(new) "," __DEFINE_ALIAS_STR(old) "\n")
#   define __DEFINE_PUBLIC_ALIAS(new,old)       __asm__(".global " __DEFINE_ALIAS_STR(new) "\n.set " __DEFINE_ALIAS_STR(new) "," __DEFINE_ALIAS_STR(old) "\n")
#   define __DEFINE_INTERN_ALIAS(new,old)       __asm__(".global " __DEFINE_ALIAS_STR(new) "\n.hidden " __DEFINE_ALIAS_STR(new) "\n.set " __DEFINE_ALIAS_STR(new) "," __DEFINE_ALIAS_STR(old) "\n")
#   define __DEFINE_PRIVATE_WEAK_ALIAS(new,old) __asm__(".weak " __DEFINE_ALIAS_STR(new) "\n.local " __DEFINE_ALIAS_STR(new) "\n.set " __DEFINE_ALIAS_STR(new) "," __DEFINE_ALIAS_STR(old) "\n")
#   define __DEFINE_PUBLIC_WEAK_ALIAS(new,old)  __asm__(".weak " __DEFINE_ALIAS_STR(new) "\n.global " __DEFINE_ALIAS_STR(new) "\n.set " __DEFINE_ALIAS_STR(new) "," __DEFINE_ALIAS_STR(old) "\n")
#   define __DEFINE_INTERN_WEAK_ALIAS(new,old)  __asm__(".weak " __DEFINE_ALIAS_STR(new) "\n.global " __DEFINE_ALIAS_STR(new) "\n.hidden " __DEFINE_ALIAS_STR(new) "\n.set " __DEFINE_ALIAS_STR(new) "," __DEFINE_ALIAS_STR(old) "\n")
#else
#   define __DEFINE_PRIVATE_ALIAS(new,old)      /* Nothing */
#   define __DEFINE_PUBLIC_ALIAS(new,old)       /* Nothing */
#   define __DEFINE_INTERN_ALIAS(new,old)       /* Nothing */
#   define __DEFINE_PRIVATE_WEAK_ALIAS(new,old) /* Nothing */
#   define __DEFINE_PUBLIC_WEAK_ALIAS(new,old)  /* Nothing */
#   define __DEFINE_INTERN_WEAK_ALIAS(new,old)  /* Nothing */
#   define __NO_DEFINE_ALIAS 1
#endif
#endif /* !__DEFINE_PUBLIC_ALIAS */

#if !defined(__PE__) && !defined(__ELF__)
/* Try to determine current binary format using other platform
 * identifiers. (When KOS headers are used on other systems) */
#if defined(__linux__) || defined(__linux) || defined(linux) || \
    defined(__unix__) || defined(__unix) || defined(unix)
#   define __ELF__ 1
#elif defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__MINGW32__) || defined(__WINDOWS__) || \
      defined(_WIN16) || defined(WIN16) || defined(_WIN32) || defined(WIN32) || \
      defined(_WIN64) || defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
      defined(_WIN32_WCE) || defined(WIN32_WCE)
#   define __PE__  1
#else
#   warning "Target binary format not defined. - Assuming `__ELF__'"
#   define __ELF__ 1
#endif
#endif

#ifdef __PE__
#   define __IMPDEF  extern __ATTR_DLLIMPORT
#   define __EXPDEF  extern __ATTR_DLLEXPORT
#   define __PUBDEF  extern
#   define __PRIVATE static
#   define __INTDEF  extern
#   define __PUBLIC  __ATTR_DLLEXPORT
#   define __INTERN  /* Nothing */
#else
#   define __IMPDEF  extern __ATTR_VISIBILITY("default")
#   define __EXPDEF  extern __ATTR_VISIBILITY("default")
#   define __PUBDEF  extern __ATTR_VISIBILITY("default")
#   define __PUBLIC         __ATTR_VISIBILITY("default")
#   define __PRIVATE static
#   define __INTDEF  extern __ATTR_VISIBILITY("hidden")
#   define __INTERN         __ATTR_VISIBILITY("hidden")
#endif


#ifdef __INTELLISENSE__
#   define __UNUSED         /* Nothing */
#elif defined(__cplusplus) || defined(__DEEMON__)
#   define __UNUSED(name)   /* Nothing */
#elif !defined(__NO_ATTR_UNUSED)
#   define __UNUSED(name)   name __ATTR_UNUSED
#elif defined(__LCLINT__)
#   define __UNUSED(name)   /*@unused@*/ name
#elif defined(_MSC_VER)
#   define __UNUSED(name)   name
#   pragma warning(disable: 4100)
#else
#   define __UNUSED(name)   name
#endif

#define __IGNORE_REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)                                /* nothing */
#define __IGNORE_REDIRECT_VOID(decl,attr,cc,name,param,asmname,args)                                   /* nothing */
#ifdef __CC__
#define __NOREDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)                                     decl attr Treturn (cc name) __P(param);
#define __NOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args)                             decl attr Treturn __NOTHROW((cc name) __P(param));
#define __NOREDIRECT_VOID(decl,attr,cc,name,param,asmname,args)                                        decl attr void (cc name) __P(param);
#define __NOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args)                                decl attr void __NOTHROW((cc name) __P(param));
#define __VNOREDIRECT(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)         decl attr Treturn (cc name) __P(param);
#define __VNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) decl attr Treturn __NOTHROW((cc name) __P(param));
#define __VNOREDIRECT_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)            decl attr void (cc name) __P(param);
#define __VNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)    decl attr void __NOTHROW((cc name) __P(param));
#define __XNOREDIRECT(decl,attr,Treturn,cc,name,param,asmname,code)                                    decl attr Treturn (cc name) __P(param);
#define __XNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code)                            decl attr Treturn __NOTHROW((cc name) __P(param));
#define __XNOREDIRECT_VOID(decl,attr,cc,name,param,asmname,code)                                       decl attr void (cc name) __P(param);
#define __XNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code)                               decl attr void __NOTHROW((cc name) __P(param));
#else
#define __NOREDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)                                     /* nothing */
#define __NOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args)                             /* nothing */
#define __NOREDIRECT_VOID(decl,attr,cc,name,param,asmname,args)                                        /* nothing */
#define __NOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args)                                /* nothing */
#define __VNOREDIRECT(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)         /* nothing */
#define __VNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) /* nothing */
#define __VNOREDIRECT_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)            /* nothing */
#define __VNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)    /* nothing */
#define __XNOREDIRECT(decl,attr,Treturn,cc,name,param,asmname,code)                                    /* nothing */
#define __XNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code)                            /* nothing */
#define __XNOREDIRECT_VOID(decl,attr,cc,name,param,asmname,code)                                       /* nothing */
#define __XNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code)                               /* nothing */
#endif

/* General purpose redirection implementation. */
#ifndef __REDIRECT
#ifdef __INTELLISENSE__
/* Only declare the functions for intellisense to minimize IDE lag. */
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn (cc name) param;
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn __NOTHROW((cc name) param);
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
    decl attr void (cc name) param;
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
    decl attr void __NOTHROW((cc name) param);
#define __VREDIRECT(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) \
    decl attr Treturn (cc name) __P(param);
#define __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) \
    decl attr Treturn __NOTHROW((cc name) __P(param));
#define __VREDIRECT_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start) \
    decl attr void (cc name) __P(param);
#define __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start) \
    decl attr void __NOTHROW((cc name) __P(param));
#define __XREDIRECT(decl,attr,Treturn,cc,name,param,asmname,code) \
    decl attr Treturn (cc name) __P(param);
#define __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code) \
    decl attr Treturn __NOTHROW((cc name) __P(param));
#define __XREDIRECT_VOID(decl,attr,cc,name,param,asmname,code) \
    decl attr void (cc name) __P(param);
#define __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code) \
    decl attr void __NOTHROW((cc name) __P(param));
#elif !defined(__CC__)
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)                                     /* Nothing */
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args)                             /* Nothing */
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args)                                        /* Nothing */
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args)                                /* Nothing */
#define __VREDIRECT(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)         /* Nothing */
#define __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) /* Nothing */
#define __VREDIRECT_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)            /* Nothing */
#define __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)    /* Nothing */
#define __XREDIRECT(decl,attr,Treturn,cc,name,param,asmname,code)                                    /* Nothing */
#define __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code)                            /* Nothing */
#define __XREDIRECT_VOID(decl,attr,cc,name,param,asmname,code)                                       /* Nothing */
#define __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code)                               /* Nothing */
#elif !defined(__NO_ASMNAME)
/* Use GCC family's assembly name extension. */
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn (cc name) __P(param) __ASMNAME(__PP_PRIVATE_STR(asmname));
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
    decl attr Treturn __NOTHROW((cc name) __P(param)) __ASMNAME(__PP_PRIVATE_STR(asmname));
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
    decl attr void (cc name) __P(param) __ASMNAME(__PP_PRIVATE_STR(asmname));
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
    decl attr void __NOTHROW((cc name) __P(param)) __ASMNAME(__PP_PRIVATE_STR(asmname));
#define __VREDIRECT(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) \
    decl attr Treturn (cc name) __P(param) __ASMNAME(__PP_PRIVATE_STR(asmnamef));
#define __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) \
    decl attr Treturn __NOTHROW((cc name) __P(param)) __ASMNAME(__PP_PRIVATE_STR(asmnamef));
#define __VREDIRECT_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start) \
    decl attr void (cc name) __P(param) __ASMNAME(__PP_PRIVATE_STR(asmnamef));
#define __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start) \
    decl attr void __NOTHROW((cc name) __P(param)) __ASMNAME(__PP_PRIVATE_STR(asmnamef));
#define __XREDIRECT(decl,attr,Treturn,cc,name,param,asmname,code) \
    decl attr Treturn (cc name) __P(param) __ASMNAME(__PP_PRIVATE_STR(asmname));
#define __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code) \
    decl attr Treturn __NOTHROW((cc name) __P(param)) __ASMNAME(__PP_PRIVATE_STR(asmname));
#define __XREDIRECT_VOID(decl,attr,cc,name,param,asmname,code) \
    decl attr void (cc name) __P(param) __ASMNAME(__PP_PRIVATE_STR(asmname));
#define __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code) \
    decl attr void __NOTHROW((cc name) __P(param)) __ASMNAME(__PP_PRIVATE_STR(asmname));
#elif defined(__cplusplus)
/* In C++, we can use use namespaces to prevent collisions with incompatible prototypes. */
#define __REDIRECT_UNIQUE  __PP_CAT2(__u,__LINE__)
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
namespace __intern { namespace __REDIRECT_UNIQUE { extern "C" { decl Treturn (cc asmname) param; } } } \
__LOCAL attr Treturn (cc name) param { \
    return (__intern::__REDIRECT_UNIQUE:: asmname) args; \
}
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
namespace __intern { namespace __REDIRECT_UNIQUE { extern "C" { decl Treturn __NOTHROW((cc asmname) param); } } } \
__LOCAL attr Treturn __NOTHROW((cc name) param) { \
    return (__intern::__REDIRECT_UNIQUE:: asmname) args; \
}
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
namespace __intern { namespace __REDIRECT_UNIQUE { extern "C" { decl void (cc asmname) param; } } } \
__LOCAL attr void (cc name) param { \
    (__intern::__REDIRECT_UNIQUE:: asmname) args; \
}
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
namespace __intern { namespace __REDIRECT_UNIQUE { extern "C" { decl void __NOTHROW((cc asmname) param); } } } \
__LOCAL attr void __NOTHROW((cc name) param) { \
    (__intern::__REDIRECT_UNIQUE:: asmname) args; \
}
#define ____PRIVATE_VREDIRECT_UNPACK(...) __VA_ARGS__
#define __VREDIRECT(decl,attr,Treturn,cc,namef,param,asmnamef,vasmnamef,args,before_va_start) \
namespace __intern { namespace __REDIRECT_UNIQUE { extern "C" { decl Treturn (cc vasmnamef)(____PRIVATE_VREDIRECT_UNPACK param, __builtin_va_list ____va_args); } } } \
__LOCAL attr Treturn (cc name) param { \
    Treturn ____va_result; \
    __builtin_va_list ____va_args; \
    __builtin_va_start(____va_args,before_va_start); \
    ____va_result = (__intern::__REDIRECT_UNIQUE:: vasmnamef)(____PRIVATE_VREDIRECT_UNPACK args,____va_args); \
    __builtin_va_end(____va_args); \
    return ____va_result; \
}
#define __VREDIRECT_VOID(decl,attr,cc,namef,param,asmnamef,vasmnamef,args,before_va_start) \
namespace __intern { namespace __REDIRECT_UNIQUE { extern "C" { decl void (cc vasmnamef)(____PRIVATE_VREDIRECT_UNPACK param, __builtin_va_list ____va_args); } } } \
__LOCAL attr void (cc name) param { \
    __builtin_va_list ____va_args; \
    __builtin_va_start(____va_args,before_va_start); \
    (__intern::__REDIRECT_UNIQUE:: vasmnamef)(____PRIVATE_VREDIRECT_UNPACK args,____va_args); \
    __builtin_va_end(____va_args); \
}
#define __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,namef,param,asmnamef,vasmnamef,args,before_va_start) \
namespace __intern { namespace __REDIRECT_UNIQUE { extern "C" { decl Treturn __NOTHROW((cc vasmnamef)(____PRIVATE_VREDIRECT_UNPACK param, __builtin_va_list ____va_args)); } } } \
__LOCAL attr Treturn __NOTHROW((cc name) param) { \
    Treturn ____va_result; \
    __builtin_va_list ____va_args; \
    __builtin_va_start(____va_args,before_va_start); \
    ____va_result = (__intern::__REDIRECT_UNIQUE:: vasmnamef)(____PRIVATE_VREDIRECT_UNPACK args,____va_args); \
    __builtin_va_end(____va_args); \
    return ____va_result; \
}
#define __VREDIRECT_VOID_NOTHROW(decl,attr,cc,namef,param,asmnamef,vasmnamef,args,before_va_start) \
namespace __intern { namespace __REDIRECT_UNIQUE { extern "C" { decl void __NOTHROW((cc vasmnamef)(____PRIVATE_VREDIRECT_UNPACK param, __builtin_va_list ____va_args)); } } } \
__LOCAL attr void __NOTHROW((cc name) param) { \
    __builtin_va_list ____va_args; \
    __builtin_va_start(____va_args,before_va_start); \
    (__intern::__REDIRECT_UNIQUE:: vasmnamef)(____PRIVATE_VREDIRECT_UNPACK args,____va_args); \
    __builtin_va_end(____va_args); \
}
#define __XREDIRECT(decl,attr,Treturn,cc,namef,param,asmname,code) \
__LOCAL attr Treturn (cc name) param code
#define __VREDIRECT_VOID(decl,attr,cc,namef,param,asmname,code) \
__LOCAL attr void (cc name) param code
#define __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,namef,param,asmname,code) \
__LOCAL attr Treturn __NOTHROW((cc name) param) code
#define __VREDIRECT_VOID_NOTHROW(decl,attr,cc,namef,param,asmname,code) \
__LOCAL attr void __NOTHROW((cc name) param) code
#else
/* Fallback: Assume that the compiler supports scoped declarations,
 *           as well as deleting them once the scope ends.
 * NOTE: GCC actually doesn't support this one, somehow keeping
 *       track of the C declaration types even after the scope ends,
 *       causing it to fail fatal()-style with incompatible-prototype errors.
 * HINT: This implementation does how ever work for MSVC when compiling for C.
 */
#define __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args) \
__LOCAL attr Treturn (cc name) param { \
    decl Treturn (cc asmname) param; \
    return (asmname) args; \
}
#define __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
__LOCAL attr Treturn __NOTHROW((cc name) param) { \
    decl Treturn __NOTHROW((cc asmname) param); \
    return (asmname) args; \
}
#define __REDIRECT_VOID(decl,attr,cc,name,param,asmname,args) \
__LOCAL attr void (cc name) param { \
    decl void (cc asmname) param; \
    (asmname) args; \
}
#define __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
__LOCAL attr void __NOTHROW((cc name) param) { \
    decl void __NOTHROW((cc asmname) param); \
    (asmname) args; \
}
#define ____PRIVATE_VREDIRECT_UNPACK(...) __VA_ARGS__
#define __VREDIRECT(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) \
__LOCAL attr Treturn (cc name) param { \
    decl Treturn (cc vasmnamef)(____PRIVATE_VREDIRECT_UNPACK param, __builtin_va_list ____va_args); \
    __builtin_va_list ____va_args; \
    Treturn ____va_result; \
    __builtin_va_start(____va_args,before_va_start); \
    ____va_result = (vasmnamef)(____PRIVATE_VREDIRECT_UNPACK args,____va_args); \
    __builtin_va_end(____va_args); \
    return ____va_result; \
}
#define __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) \
__LOCAL attr Treturn __NOTHROW((cc name) param) { \
    decl Treturn __NOTHROW((cc vasmnamef)(____PRIVATE_VREDIRECT_UNPACK param, __builtin_va_list ____va_args)); \
    __builtin_va_list ____va_args; \
    Treturn ____va_result; \
    __builtin_va_start(____va_args,before_va_start); \
    ____va_result = (vasmnamef)(____PRIVATE_VREDIRECT_UNPACK args,____va_args); \
    __builtin_va_end(____va_args); \
    return ____va_result; \
}
#define __VREDIRECT_VOID(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start) \
__LOCAL attr void (cc name) param { \
    decl void (cc vasmnamef)(____PRIVATE_VREDIRECT_UNPACK param, __builtin_va_list ____va_args); \
    __builtin_va_list ____va_args; \
    __builtin_va_start(____va_args,before_va_start); \
    (vasmnamef)(____PRIVATE_VREDIRECT_UNPACK args,____va_args); \
    __builtin_va_end(____va_args); \
}
#define __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start) \
__LOCAL attr void __NOTHROW((cc name) param) { \
    decl void __NOTHROW((cc vasmnamef)(____PRIVATE_VREDIRECT_UNPACK param, __builtin_va_list ____va_args)); \
    __builtin_va_list ____va_args; \
    __builtin_va_start(____va_args,before_va_start); \
    (vasmnamef)(____PRIVATE_VREDIRECT_UNPACK args,____va_args); \
    __builtin_va_end(____va_args); \
}
#define __XREDIRECT(decl,attr,Treturn,cc,name,param,asmname,code) \
__LOCAL attr Treturn (cc name) param code
#define __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,code) \
__LOCAL attr Treturn __NOTHROW((cc name) param) code
#define __XREDIRECT_VOID(decl,attr,cc,name,param,asmname,code) \
__LOCAL attr void (cc name) param code
#define __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,asmname,code) \
__LOCAL attr void __NOTHROW((cc name) param) code
#endif
#endif /* !__REDIRECT */


#ifdef __KOS_SYSTEM_HEADERS__
#ifndef __KERNEL__
#ifdef __CC__
__NAMESPACE_STD_BEGIN struct __IO_FILE;
__NAMESPACE_STD_END
#endif /* __CC__ */
#ifndef __FILE
#define __FILE     struct __NAMESPACE_STD_SYM __IO_FILE
#endif /* !__FILE */
#endif
#endif /* __KOS_SYSTEM_HEADERS__ */

#ifndef __LIBCCALL
#ifdef __KERNEL__
#   define __LIBCCALL __KCALL
#else
#   define __LIBCCALL /* Nothing */
#   define __LIBCCALL_CALLER_CLEANUP 1
#endif
#endif

#ifndef __LIBC
#ifdef __BUILDING_LIBC
#define __LIBC    __EXPDEF
#else
#define __LIBC    __IMPDEF
#endif
#endif

/* Annotations */
#define __CLEARED      /* Annotation for allocators returning zero-initialized memory. */
#define __WEAK         /* Annotation for weakly referenced data/data updated randomly with both the old/new state remaining valid forever. */
#define __REF          /* Annotation for reference holders. */
#define __ATOMIC_DATA  /* Annotation for atomic data. */
#define __PAGE_ALIGNED /* Annotation for page-aligned pointers. */
#define __USER         /* Annotation for user-space memory (default outside kernel). */
#define __HOST         /* Annotation for kernel-space memory (default within kernel). */
#define __VIRT         /* Annotation for virtual memory (default). */
#define __PHYS         /* Annotation for physical memory. */
#define __MMIO         /* Annotation for memory-mapped I/O-port pointers. */
#define __NOIRQ        /* Annotation for functions that require interrupts to be disabled. */
#define __NOMP         /* Annotation for functions that are not thread-safe and require caller-synchronization. */
#define __ASMCALL      /* Annotation for functions that are implemented in assembly and require a custom calling convention. */
#define __INITDATA     /* Annotation for data that apart of .free sections, meaning that accessing it is illegal after some specific point in time. */
#define __INITCALL     /* Annotation for functions that apart of .free sections, meaning that calling it is illegal after some specific point in time. */
#define __CLEANUP      /* Annotation for so-called cleanup functions (as already defined by the documentation of `_EXCEPT_API' in `<features.h>'). */
#if __KOS_VERSION__ < 300
#define __CRIT         /* Annotation for functions that require `TASK_ISCRIT()' (When called from within the kernel). */
#define         /* Annotation for functions that require `TASK_ISSAFE()' (When called from within the kernel). */
#define __PERCPU       /* Annotation for variables that must be accessed using the per-cpu API. */
#endif

#endif /* !___STDINC_H */
