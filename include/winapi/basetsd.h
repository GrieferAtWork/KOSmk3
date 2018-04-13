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
#ifndef _BASETSD_H_
#define _BASETSD_H_ 1

#include "__winstd.h"

#if (defined(__x86_64) || defined(__ia64__)) && !defined(RC_INVOKED)
typedef __UINT64_TYPE__ POINTER_64_INT;
#else
typedef __UINT32_TYPE__ POINTER_64_INT;
#endif
#if defined(_MSC_VER) || defined(__INTELLISENSE__)
#define POINTER_32    __ptr32
#define POINTER_64    __ptr64
#else
#define POINTER_32    /* Nothing */
#define POINTER_64    /* Nothing */
#endif
#define FIRMWARE_PTR  /* Nothing */

__DECL_BEGIN

typedef __INT8_TYPE__ INT8,*PINT8;
typedef __INT16_TYPE__ INT16,*PINT16;
typedef __INT32_TYPE__ INT32,*PINT32;
typedef __INT64_TYPE__ INT64,*PINT64;
typedef __UINT8_TYPE__ UINT8,*PUINT8;
typedef __UINT16_TYPE__ UINT16,*PUINT16;
typedef __UINT32_TYPE__ UINT32,*PUINT32;
typedef __UINT64_TYPE__ UINT64,*PUINT64;
typedef __INT32_TYPE__ LONG32,*PLONG32;
typedef __UINT32_TYPE__ ULONG32,*PULONG32;
typedef __UINT32_TYPE__ DWORD32,*PDWORD32;

#ifndef _W64
#define _W64
#endif

#if __SIZEOF_POINTER__ >= 8
#define __int3264 __int64
#else
#define __int3264 __int32
#endif
typedef __INTPTR_TYPE__ INT_PTR,*PINT_PTR;
typedef __UINTPTR_TYPE__ UINT_PTR,*PUINT_PTR;
typedef __LONGPTR_TYPE__ LONG_PTR,*PLONG_PTR;
typedef __ULONGPTR_TYPE__ ULONG_PTR,*PULONG_PTR;

#if __SIZEOF_POINTER__ >= 8
#define ADDRESS_TAG_BIT 0x40000000000ULL
typedef __INT64_TYPE__ SHANDLE_PTR;
typedef __UINT64_TYPE__ HANDLE_PTR;
typedef __UINT32_TYPE__ UHALF_PTR,*PUHALF_PTR;
typedef __INT32_TYPE__ HALF_PTR,*PHALF_PTR;

__LOCAL __ULONG32_TYPE__ (HandleToULong)(void const *h) { return((__ULONG32_TYPE__) (ULONG_PTR) h); }
__LOCAL __LONG32_TYPE__ (HandleToLong)(void const *h) { return((__LONG32_TYPE__) (LONG_PTR) h); }
__LOCAL void *(ULongToHandle)(const __ULONG32_TYPE__ h) { return((void *) (UINT_PTR) h); }
__LOCAL void *(LongToHandle)(const __LONG32_TYPE__ h) { return((void *) (INT_PTR) h); }
__LOCAL __ULONG32_TYPE__ (PtrToUlong)(void const *p) { return((__ULONG32_TYPE__) (ULONG_PTR) p); }
__LOCAL __UINT32_TYPE__ (PtrToUint)(void const *p) { return((__UINT32_TYPE__) (UINT_PTR) p); }
__LOCAL __UINT16_TYPE__ (PtrToUshort)(void const *p) { return((__UINT16_TYPE__) (__ULONG32_TYPE__) (ULONG_PTR) p); }
__LOCAL __LONG32_TYPE__ (PtrToLong)(void const *p) { return((__LONG32_TYPE__) (LONG_PTR) p); }
__LOCAL __INT32_TYPE__ (PtrToInt)(void const *p) { return((__INT32_TYPE__) (INT_PTR) p); }
__LOCAL __INT16_TYPE__ (PtrToShort)(void const *p) { return((__INT16_TYPE__) (__LONG32_TYPE__) (LONG_PTR) p); }
__LOCAL void *(IntToPtr)(const __INT32_TYPE__ i) { return((void *)(INT_PTR)i); }
__LOCAL void *(UIntToPtr)(const __UINT32_TYPE__ ui) { return((void *)(UINT_PTR)ui); }
__LOCAL void *(LongToPtr)(const __LONG32_TYPE__ l) { return((void *)(LONG_PTR)l); }
__LOCAL void *(ULongToPtr)(const __ULONG32_TYPE__ ul) { return((void *)(ULONG_PTR)ul); }

#define PtrToPtr64(p)      ((void *) p)
#define Ptr64ToPtr(p)      ((void *) p)
#define HandleToHandle64(h) (PtrToPtr64(h))
#define Handle64ToHandle(h) (Ptr64ToPtr(h))
__LOCAL void *(Ptr32ToPtr)(void const *p) { return (void *)p; }
__LOCAL void *(Handle32ToHandle)(void const *h) { return((void *) h); }
__LOCAL void *(PtrToPtr32)(void const *p) { return((void *) (ULONG_PTR) p); }

#define HandleToHandle32(h) (PtrToPtr32(h))
#else

#define ADDRESS_TAG_BIT 0x80000000UL

typedef __UINT16_TYPE__ UHALF_PTR,*PUHALF_PTR;
typedef __INT16_TYPE__ HALF_PTR,*PHALF_PTR;
typedef __LONG32_TYPE__ SHANDLE_PTR;
typedef __ULONG32_TYPE__ HANDLE_PTR;

#define HandleToULong(h) ((ULONG)(ULONG_PTR)(h))
#define HandleToLong(h) ((LONG)(LONG_PTR) (h))
#define ULongToHandle(ul) ((HANDLE)(ULONG_PTR) (ul))
#define LongToHandle(h) ((HANDLE)(LONG_PTR) (h))
#define PtrToUlong(p) ((ULONG)(ULONG_PTR) (p))
#define PtrToLong(p) ((LONG)(LONG_PTR) (p))
#define PtrToUint(p) ((UINT)(UINT_PTR) (p))
#define PtrToInt(p) ((INT)(INT_PTR) (p))
#define PtrToUshort(p) ((__UINT16_TYPE__)(ULONG_PTR)(p))
#define PtrToShort(p) ((__INT16_TYPE__)(LONG_PTR)(p))
#define IntToPtr(i) ((VOID *)(INT_PTR)((__INT32_TYPE__)i))
#define UIntToPtr(ui) ((VOID *)(UINT_PTR)((__UINT32_TYPE__)ui))
#define LongToPtr(l) ((VOID *)(LONG_PTR)((__LONG32_TYPE__)l))
#define ULongToPtr(ul) ((VOID *)(ULONG_PTR)((__ULONG32_TYPE__)ul))

__LOCAL void *(PtrToPtr64)(void const *p) { return((void *) (ULONG_PTR)p); }
__LOCAL void *(Ptr64ToPtr)(void const *p) { return((void *) (ULONG_PTR) p); }
__LOCAL void *(HandleToHandle64)(void const *h) { return((void *) h); }
__LOCAL void *(Handle64ToHandle)(void const *h) { return((void *) (ULONG_PTR) h); }

#define Ptr32ToPtr(p) ((void *) p)
#define Handle32ToHandle(h) (Ptr32ToPtr(h))
#define PtrToPtr32(p) ((void *) p)
#define HandleToHandle32(h) (PtrToPtr32(h))
#endif

#define HandleToUlong(h) HandleToULong(h)
#define UlongToHandle(ul) ULongToHandle(ul)
#define UlongToPtr(ul) ULongToPtr(ul)
#define UintToPtr(ui) UIntToPtr(ui)

#define MAXUINT_PTR (~((UINT_PTR)0))
#define MAXINT_PTR ((INT_PTR)(MAXUINT_PTR >> 1))
#define MININT_PTR (~MAXINT_PTR)

#define MAXULONG_PTR (~((ULONG_PTR)0))
#define MAXLONG_PTR ((LONG_PTR)(MAXULONG_PTR >> 1))
#define MINLONG_PTR (~MAXLONG_PTR)

#define MAXUHALF_PTR ((UHALF_PTR)~0)
#define MAXHALF_PTR ((HALF_PTR)(MAXUHALF_PTR >> 1))
#define MINHALF_PTR (~MAXHALF_PTR)

typedef ULONG_PTR        SIZE_T,*PSIZE_T;
typedef LONG_PTR         SSIZE_T,*PSSIZE_T;
typedef ULONG_PTR        DWORD_PTR,*PDWORD_PTR;
typedef __LONG64_TYPE__  LONG64,*PLONG64;
typedef __ULONG64_TYPE__ ULONG64,*PULONG64;
typedef __UINT64_TYPE__  DWORD64,*PDWORD64;
typedef ULONG_PTR        KAFFINITY;
typedef KAFFINITY       *PKAFFINITY;

__DECL_END

#endif /* !_BASETSD_H_ */
