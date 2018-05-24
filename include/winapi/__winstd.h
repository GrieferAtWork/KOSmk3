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
#ifndef _WINAPI___WINSTD_H
#define _WINAPI___WINSTD_H 1

#include <__stdinc.h>
#include <stddef.h>
#include <stdarg.h>
#include <hybrid/typecore.h>
#include <hybrid/host.h>

#undef __cdecl
#undef _X86_
#undef WIN32
#define DUMMYUNIONNAME   /* Nothing */
#define DUMMYUNIONNAME2  /* Nothing */
#define DUMMYSTRUCTNAME  /* Nothing */
#define DUMMYSTRUCTNAME2 /* Nothing */

#define WIN32       1
#define _WIN32      1
#ifdef __x86_64__
#   define _AMD64_  1
#   define WIN64    1
#   define _WIN64   1
#elif defined(__i386__)
#   define _X86_    1
#elif defined(__arm__)
#   define _ARM_    1
#endif
#if defined(__x86_64) && \
  !(defined(_X86_) || defined(__i386__) || defined(_IA64_))
#if !defined(_AMD64_)
#define _AMD64_
#endif
#endif /* _AMD64_ */
#if defined(__ia64__) && \
  !(defined(_X86_) || defined(__x86_64) || defined(_AMD64_))
#if !defined(_IA64_)
#define _IA64_
#endif
#endif /* _IA64_ */


#if !defined(_MSC_VER) && !defined(__INTELLISENSE__)
#undef __int8
#undef __int16
#undef __int32
#undef __int64
#undef __stdcall
#undef __cdecl
#undef __declspec
#undef __unaligned
#undef __fastcall
#define __int8        char
#define __int16       short
#define __int32       int
#define __int64       long long
#define __stdcall     __ATTR_STDCALL
#define __cdecl       __ATTR_CDECL
#define __declspec(x) __attribute__((x))
#define __unaligned   __ATTR_PACKED
#define __fastcall    __ATTR_FASTCALL
#define __MSVCRT__ 1
#undef _MSVCRT_
#endif

#if defined(_MSC_VER) || defined(__INTELLISENSE__)
#define __POINTER32(T) T *__ptr32
#define __POINTER64(T) T *__ptr64
#elif __SIZEOF_POINTER__ == 4
#define __POINTER32(T) T *
#define __POINTER64(T) __UINT64_TYPE__
#elif __SIZEOF_POINTER__ == 8
#define __POINTER32(T) __UINT32_TYPE__
#define __POINTER64(T) T *
#else
#define __POINTER32(T) __UINT32_TYPE__
#define __POINTER64(T) __UINT64_TYPE__
#endif



#undef  UNALIGNED    /* avoid redefinition warnings vs __winstd.h */
#undef  UNALIGNED64
#if defined(_M_MRX000) || defined(_M_ALPHA) || \
    defined(_M_PPC) || defined(_M_IA64) || \
    defined(_M_AMD64)
#define ALIGNMENT_MACHINE
#define UNALIGNED   __unaligned
#if defined(_WIN64)
#define UNALIGNED64 __unaligned
#else
#define UNALIGNED64
#endif
#else
#undef ALIGNMENT_MACHINE
#define UNALIGNED
#define UNALIGNED64
#endif

#if defined(_M_MRX000) && \
  !(defined(MIDL_PASS) || \
    defined(RC_INVOKED)) && \
    defined(ENABLE_RESTRICTED)
#define RESTRICTED_POINTER __restrict
#else
#define RESTRICTED_POINTER
#endif


#ifndef __WCHAR_DEFINED
#define __WCHAR_DEFINED 1
typedef __CHAR16_TYPE__ WCHAR;
#endif /* !__WCHAR_DEFINED */

#define _SIZE_T_DEFINED 1
#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif

#define _SSIZE_T_DEFINED 1
#ifndef __ssize_t_defined
#define __ssize_t_defined 1
typedef __SSIZE_TYPE__ ssize_t;
#endif

#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED 1
#endif

#define _PTRDIFF_T_DEFINED 1
#ifndef __ptrdiff_t_defined
#define __ptrdiff_t_defined 1
typedef __PTRDIFF_TYPE__ ptrdiff_t;
#endif

#define _UINTPTR_T_DEFINED 1
#ifndef __uintptr_t_defined
#define __uintptr_t_defined 1
typedef __UINTPTR_TYPE__ uintptr_t;
#endif

#define _INTPTR_T_DEFINED 1
#ifndef __intptr_t_defined
#define __intptr_t_defined 1
typedef __INTPTR_TYPE__ intptr_t;
#endif

#define _INTEGRAL_MAX_BITS 64

#define _ANONYMOUS_UNION
#define _ANONYMOUS_STRUCT
#define __MINGW_IMPORT               __IMPDEF
#define __MINGW_ATTRIB_NORETURN      __ATTR_NORETURN
#define __MINGW_ATTRIB_CONST         __ATTR_CONST
#define __MINGW_ATTRIB_DEPRECATED    __ATTR_DEPRECATED_
#define __MINGW_ATTRIB_MALLOC        __ATTR_MALLOC
#define __MINGW_ATTRIB_PURE          __ATTR_PURE
#define __MINGW_ATTRIB_NONNULL(arg)  __NONNULL(arg)
#define __MINGW_NOTHROW              __ATTR_NOTHROW
#define __GNUC_VA_LIST               __builtin_va_list
#define _CRTIMP                      extern
#define __CRT_INLINE                 __FORCELOCAL
#define __CRT__NO_INLINE             1
#define _CRT_ALIGN(x)                __ATTR_ALIGNED(x)
#define DECLSPEC_ALIGN(x)            __ATTR_ALIGNED(x)
#define _CRT_PACKING                 8
#define __CRT_UNALIGNED
#define _CONST_RETURN
#define __CRT_STRINGIZE(_Value)      #_Value
#define _CRT_STRINGIZE(_Value)       __CRT_STRINGIZE(_Value)
#define __CRT_WIDE(_String)          L##_String
#define _CRT_WIDE(_String)           __CRT_WIDE(_String)
#define DECLSPEC_NORETURN            __ATTR_NORETURN
#define DECLARE_STDCALL_P(type)      __stdcall type
#define NOCRYPT                      1
#define NOSERVICE                    1
#define NOMCX                        1
#define NOIME                        1
#define TYPE_ALIGNMENT(t)            __COMPILER_ALIGNOF(t)

#ifdef _WIN64
#ifdef _AMD64_
#define PROBE_ALIGNMENT(_s)          TYPE_ALIGNMENT(DWORD)
#elif defined(_IA64_)
#define PROBE_ALIGNMENT(_s)         (TYPE_ALIGNMENT(_s) > TYPE_ALIGNMENT(DWORD) ? TYPE_ALIGNMENT(_s) : TYPE_ALIGNMENT(DWORD))
#else
#error No Target Architecture
#endif
#define PROBE_ALIGNMENT32(_s)        TYPE_ALIGNMENT(DWORD)
#else
#define PROBE_ALIGNMENT(_s)          TYPE_ALIGNMENT(DWORD)
#endif


#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#define C_ASSERT(e) STATIC_ASSERT(e)
#if defined(_X86_) || defined(__ia64__) || defined(__x86_64)
#define DECLSPEC_IMPORT __IMPDEF
#else
#define DECLSPEC_IMPORT
#endif
#ifndef SYSTEM_CACHE_ALIGNMENT_SIZE
#if defined(_AMD64_) || defined(_X86_)
#define SYSTEM_CACHE_ALIGNMENT_SIZE 64
#else
#define SYSTEM_CACHE_ALIGNMENT_SIZE 128
#endif
#endif
#ifndef DECLSPEC_CACHEALIGN
#define DECLSPEC_CACHEALIGN DECLSPEC_ALIGN(SYSTEM_CACHE_ALIGNMENT_SIZE)
#endif

#ifndef DECLSPEC_UUID
#define DECLSPEC_UUID(x)
#endif

#ifndef DECLSPEC_NOVTABLE
#define DECLSPEC_NOVTABLE
#endif

#ifndef DECLSPEC_SELECTANY
#define DECLSPEC_SELECTANY __declspec(selectany)
#endif

#ifndef NOP_FUNCTION
#define NOP_FUNCTION (void)0
#endif

#ifndef DECLSPEC_NOINLINE
#if (_MSC_VER >= 1300)
#define DECLSPEC_NOINLINE  __declspec(noinline)
#elif defined(__GNUC__)
#define DECLSPEC_NOINLINE __attribute__((noinline))
#else
#define DECLSPEC_NOINLINE
#endif
#endif /* DECLSPEC_NOINLINE */

#ifndef FORCEINLINE
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#define FORCEINLINE __forceinline
#elif defined(_MSC_VER)
#define FORCEINLINE __inline
#else /* __GNUC__ */
#define FORCEINLINE extern __inline__ __attribute__((always_inline))
#endif
#endif /* FORCEINLINE */

#ifndef DECLSPEC_DEPRECATED
#define DECLSPEC_DEPRECATED __ATTR_DEPRECATED_
#define DEPRECATE_SUPPORTED
#endif

#define DECLSPEC_DEPRECATED_DDK
#define PRAGMA_DEPRECATED_DDK 0


#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1

#ifndef WINVER
#define WINVER 0x0502
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x502
#endif


#endif /* !_WINAPI___WINSTD_H */
