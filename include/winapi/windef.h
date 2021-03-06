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
#ifndef _WINDEF_
#define _WINDEF_ 1

#include "__winstd.h"

#ifndef STRICT
#define STRICT 1
#endif

__DECL_BEGIN

#ifndef WINVER
#define WINVER 0x0502
#endif

#ifndef BASETYPES
#define BASETYPES 1
typedef unsigned long ULONG,*PULONG;
typedef unsigned short USHORT,*PUSHORT;
typedef unsigned char UCHAR,*PUCHAR;
typedef char *PSZ;
#endif

#define MAX_PATH 260

#ifndef NULL
#define NULL __NULLPTR
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifndef _NO_W32_PSEUDO_MODIFIERS
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef OPTIONAL
#define OPTIONAL
#endif
#endif

#undef far
#undef near
#undef pascal
#undef FAR
#undef NEAR

#define far           /* Nothing */
#define near          /* Nothing */
#define pascal        __stdcall
#define cdecl         /* Nothing */
#ifndef CDECL
#define CDECL         /* Nothing */
#endif
#ifndef CALLBACK
#define CALLBACK      __stdcall
#endif
#ifndef WINAPI
#define WINAPI        __stdcall
#endif
#define WINAPIV       __cdecl
#define APIENTRY      WINAPI
#define APIPRIVATE    WINAPI
#define PASCAL        WINAPI
#define WINAPI_INLINE WINAPI
#define FAR           /* Nothing */
#define NEAR          /* Nothing */
#ifndef CONST
#define CONST         const
#endif



#ifndef FASTCALL
#if defined(_M_IX86)
#define FASTCALL             __fastcall
#else
#define FASTCALL
#endif
#endif /* !FASTCALL */
#ifndef NTAPI
#define NTAPI                __stdcall
#endif
#ifndef NTSYSAPI
#define NTSYSAPI             DECLSPEC_IMPORT
#define NTSYSCALLAPI         DECLSPEC_IMPORT
#endif
#define STDMETHODCALLTYPE    WINAPI
#define STDMETHODVCALLTYPE   __cdecl
#define STDAPICALLTYPE       WINAPI
#define STDAPIVCALLTYPE      __cdecl
#define STDAPI EXTERN_C      HRESULT WINAPI
#define STDAPI_(type)        EXTERN_C type WINAPI
#define STDMETHODIMP         HRESULT WINAPI
#define STDMETHODIMP_(type)  type WINAPI
#define STDAPIV              EXTERN_C HRESULT STDAPIVCALLTYPE
#define STDAPIV_(type)       EXTERN_C type STDAPIVCALLTYPE
#define STDMETHODIMPV        HRESULT STDMETHODVCALLTYPE
#define STDMETHODIMPV_(type) type STDMETHODVCALLTYPE



#ifndef _DEF_WINBOOL_
#define _DEF_WINBOOL_ 1
typedef int WINBOOL;
#pragma push_macro("BOOL")
#undef BOOL
#if !defined(__OBJC__) && !defined(__OBJC_BOOL) && !defined(__objc_INCLUDE_GNU)
typedef WINBOOL BOOL;
#endif
#pragma pop_macro("BOOL")
typedef WINBOOL *PBOOL;
typedef WINBOOL *LPBOOL;
#endif /* !_DEF_WINBOOL_ */

#ifndef _INT_DEFINED
#define _INT_DEFINED 1
typedef int INT;
#endif /* !_INT_DEFINED */

typedef INT *PINT,*LPINT;
typedef unsigned char BYTE,*PBYTE,*LPBYTE;
typedef unsigned short WORD,*PWORD,*LPWORD;
typedef unsigned long DWORD,*PDWORD,*LPDWORD;
typedef float FLOAT,*PFLOAT;
typedef unsigned int UINT,*PUINT;
typedef long *LPLONG;
typedef void *LPVOID;
#ifndef _LPCVOID_DEFINED
#define _LPCVOID_DEFINED
typedef CONST void *LPCVOID;
#endif

#ifndef _PVOID_DEFINED
#define _PVOID_DEFINED 1
typedef void *PVOID;
typedef __POINTER64(void) PVOID64;
#endif /* !_PVOID_DEFINED */

#ifndef VOID
#define VOID void
typedef char  CHAR;
typedef short SHORT;
typedef long  LONG;
#endif
#ifndef __WCHAR_DEFINED
#define __WCHAR_DEFINED
typedef __CHAR16_TYPE__ WCHAR;
#endif
#ifndef _PWCHAR_DEFINED
#define _PWCHAR_DEFINED 1
typedef WCHAR *PWCHAR,*LPWCH,*PWCH;
#endif
#ifndef _LPCWCH_DEFINED
#define _LPCWCH_DEFINED 1
typedef CONST WCHAR *LPCWCH,*PCWCH;
#endif
#ifndef _NWPSTR_DEFINED
#define _NWPSTR_DEFINED 1
typedef WCHAR *NWPSTR,*LPWSTR,*PWSTR;
#endif
#ifndef _PZPWSTR_DEFINED
#define _PZPWSTR_DEFINED 1
typedef PWSTR *PZPWSTR;
#endif
#ifndef _PCZPWSTR_DEFINED
#define _PCZPWSTR_DEFINED 1
typedef CONST PWSTR *PCZPWSTR;
#endif
#ifndef _LPUWSTR_DEFINED
#define _LPUWSTR_DEFINED 1
typedef WCHAR UNALIGNED *LPUWSTR,*PUWSTR;
#endif
#ifndef _LPCWSTR_DEFINED
#define _LPCWSTR_DEFINED 1
typedef CONST WCHAR *LPCWSTR,*PCWSTR;
#endif
#ifndef _PZPCWSTR_DEFINED
#define _PZPCWSTR_DEFINED 1
typedef PCWSTR *PZPCWSTR;
#endif
#ifndef _LPCUWSTR_DEFINED
#define _LPCUWSTR_DEFINED 1
typedef CONST WCHAR UNALIGNED *LPCUWSTR,*PCUWSTR;
#endif

#ifndef _PCHAR_DEFINED
#define _PCHAR_DEFINED 1
typedef CHAR *PCHAR,*LPCH,*PCH;
#endif
#ifndef _LPCCH_DEFINED
#define _LPCCH_DEFINED 1
typedef CONST CHAR *LPCCH,*PCCH;
#endif
#ifndef _NPSTR_DEFINED
#define _NPSTR_DEFINED 1
typedef CHAR *NPSTR,*LPSTR,*PSTR;
#endif
#ifndef _PZPSTR_DEFINED
#define _PZPSTR_DEFINED 1
typedef PSTR *PZPSTR;
#endif
#ifndef _PCZPSTR_DEFINED
#define _PCZPSTR_DEFINED 1
typedef CONST PSTR *PCZPSTR;
#endif
#ifndef _LPCSTR_DEFINED
#define _LPCSTR_DEFINED 1
typedef CONST CHAR *LPCSTR,*PCSTR;
#endif
#ifndef _PZPCSTR_DEFINED
#define _PZPCSTR_DEFINED 1
typedef PCSTR *PZPCSTR;
#endif

#if defined(UNICODE)
#ifndef _TCHAR_DEFINED
#define _TCHAR_DEFINED
typedef WCHAR TCHAR,*PTCHAR;
typedef WCHAR TBYTE,*PTBYTE;
#endif
typedef LPWSTR LPTCH,PTCH,PTSTR,LPTSTR,LP;
typedef LPCWSTR PCTSTR,LPCTSTR;
typedef LPUWSTR PUTSTR,LPUTSTR;
typedef LPCUWSTR PCUTSTR,LPCUTSTR;
#define __TEXT(quote) L##quote
#else
#ifndef _TCHAR_DEFINED
#define _TCHAR_DEFINED
typedef char TCHAR,*PTCHAR;
typedef unsigned char TBYTE,*PTBYTE;
#endif
typedef LPSTR LPTCH,PTCH,PTSTR,LPTSTR,PUTSTR,LPUTSTR;
typedef LPCSTR PCTSTR,LPCTSTR,PCUTSTR,LPCUTSTR;
#define __TEXT(quote) quote
#endif
#define TEXT(quote) __TEXT(quote)


#ifndef _PSHORT_DEFINED
#define _PSHORT_DEFINED 1
typedef SHORT *PSHORT;
#endif
#ifndef _PLONG_DEFINED
#define _PLONG_DEFINED 1
typedef LONG *PLONG;
#endif

#ifndef DECLARE_HANDLE
#ifdef STRICT
#define DECLARE_HANDLE(name) \
    struct name##__ { int unused; }; typedef struct name##__ *name
#else
#define DECLARE_HANDLE(name) \
    typedef HANDLE name
#endif
typedef void *HANDLE;
typedef HANDLE *PHANDLE;
#endif /* !DECLARE_HANDLE */

#ifndef _FCHAR_DEFINED
#define _FCHAR_DEFINED 1
typedef UCHAR FCHAR;
#endif
#ifndef _FSHORT_DEFINED
#define _FSHORT_DEFINED 1
typedef USHORT FSHORT;
#endif
#ifndef _FLONG_DEFINED
#define _FLONG_DEFINED 1
typedef ULONG FLONG;
#endif

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED 1
typedef LONG HRESULT;
#endif

#ifndef _CCHAR_DEFINED
#define _CCHAR_DEFINED 1
typedef char CCHAR;
#endif

#ifndef _LCID_DEFINED
#define _LCID_DEFINED 1
typedef DWORD LCID;
#endif
#ifndef _PLCID_DEFINED
#define _PLCID_DEFINED 1
typedef PDWORD PLCID;
#endif
#ifndef _LANGID_DEFINED
#define _LANGID_DEFINED 1
typedef WORD LANGID;
#endif


#ifdef __ia64__
__ATTR_ALIGNED(16)
#endif
typedef struct _FLOAT128 {
    __INT64_TYPE__ LowPart;
    __INT64_TYPE__ HighPart;
} FLOAT128,*PFLOAT128;

#ifndef _ULONGLONG_
#define _ULONGLONG_
typedef __INT64_TYPE__  LONGLONG,*PLONGLONG;
typedef __UINT64_TYPE__ ULONGLONG,*PULONGLONG;
#define MAXLONGLONG     __INT64_MAX__
typedef LONGLONG USN; /* Update Sequence Number */
#endif /* !_ULONGLONG_ */

#ifndef __LARGE_INTEGER_DEFINED
#define __LARGE_INTEGER_DEFINED 1
#ifdef MIDL_PASS
typedef struct _LARGE_INTEGER
#else
typedef union _LARGE_INTEGER
#endif /* MIDL_PASS */
{
#ifndef MIDL_PASS
    struct {
        ULONG LowPart;
        LONG HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        ULONG LowPart;
        LONG HighPart;
    } u;
#endif /* !MIDL_PASS */
    LONGLONG QuadPart;
} LARGE_INTEGER,*PLARGE_INTEGER;
#ifdef MIDL_PASS
typedef struct _ULARGE_INTEGER
#else
typedef union _ULARGE_INTEGER
#endif
{
#ifndef MIDL_PASS
    struct {
        ULONG LowPart;
        ULONG HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        ULONG LowPart;
        ULONG HighPart;
    } u;
#endif /* !MIDL_PASS */
    ULONGLONG QuadPart;
} ULARGE_INTEGER,*PULARGE_INTEGER;
#endif /* !__LARGE_INTEGER_DEFINED */

#ifndef __LUID_DEFINED
#define __LUID_DEFINED 1
typedef struct _LUID {
    DWORD LowPart;
    LONG  HighPart;
} LUID,*PLUID;
#endif /* !__LUID_DEFINED */

#ifndef _DWORDLONG_
#define _DWORDLONG_ 1
typedef ULONGLONG DWORDLONG,*PDWORDLONG;
#endif /* !_DWORDLONG_ */

#ifndef _BOOLEAN_
#define _BOOLEAN_ 1
typedef BYTE BOOLEAN,*PBOOLEAN;
#endif /* !_BOOLEAN_ */

#define MINCHAR   0x80
#define MAXCHAR   0x7f
#define MINSHORT  0x8000
#define MAXSHORT  0x7fff
#define MINLONG   0x80000000
#define MAXLONG   0x7fffffff
#define MAXBYTE   0xff
#define MAXWORD   0xffff
#define MAXDWORD  0xffffffff

#include "basetsd.h"
#include "specstrings.h"

typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;

#ifndef __cplusplus
#ifndef NOMINMAX
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#endif
#endif

#define MAKEWORD(a,b) ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))
#define MAKELONG(a,b) ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#define LOWORD(l)     ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)     ((WORD)((DWORD_PTR)(l) >> 16))
#define LOBYTE(w)     ((BYTE)((DWORD_PTR)(w) & 0xff))
#define HIBYTE(w)     ((BYTE)((DWORD_PTR)(w) >> 8))

#ifndef WIN_INTERNAL
DECLARE_HANDLE (HWND);
DECLARE_HANDLE (HHOOK);
#ifdef WINABLE
DECLARE_HANDLE (HEVENT);
#endif
#endif

typedef WORD    ATOM;
typedef HANDLE *SPHANDLE,*LPHANDLE;
typedef HANDLE HGLOBAL,HLOCAL;
typedef HANDLE GLOBALHANDLE,LOCALHANDLE;
#ifdef _WIN64
typedef INT_PTR (WINAPI *FARPROC)();
typedef INT_PTR (WINAPI *NEARPROC)();
typedef INT_PTR (WINAPI *PROC)();
#else
typedef int (WINAPI *FARPROC)();
typedef int (WINAPI *NEARPROC)();
typedef int (WINAPI *PROC)();
#endif

typedef void *HGDIOBJ;

DECLARE_HANDLE(HKEY);
typedef HKEY *PHKEY;

DECLARE_HANDLE(HACCEL);
DECLARE_HANDLE(HBITMAP);
DECLARE_HANDLE(HBRUSH);
DECLARE_HANDLE(HCOLORSPACE);
DECLARE_HANDLE(HDC);
DECLARE_HANDLE(HGLRC);
DECLARE_HANDLE(HDESK);
DECLARE_HANDLE(HENHMETAFILE);
DECLARE_HANDLE(HFONT);
DECLARE_HANDLE(HICON);
DECLARE_HANDLE(HMENU);
DECLARE_HANDLE(HMETAFILE);
DECLARE_HANDLE(HINSTANCE);
typedef HINSTANCE HMODULE;
DECLARE_HANDLE(HPALETTE);
DECLARE_HANDLE(HPEN);
DECLARE_HANDLE(HRGN);
DECLARE_HANDLE(HRSRC);
DECLARE_HANDLE(HSTR);
DECLARE_HANDLE(HTASK);
DECLARE_HANDLE(HWINSTA);
DECLARE_HANDLE(HKL);
DECLARE_HANDLE(HMONITOR);
DECLARE_HANDLE(HWINEVENTHOOK);
DECLARE_HANDLE(HUMPD);

typedef int HFILE;
typedef HICON HCURSOR;
typedef DWORD COLORREF;
typedef DWORD *LPCOLORREF;

#define HFILE_ERROR ((HFILE)-1)

typedef struct tagRECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT,*PRECT,*NPRECT,*LPRECT;

typedef const RECT *LPCRECT;

typedef struct _RECTL {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECTL,*PRECTL,*LPRECTL;

typedef const RECTL *LPCRECTL;

typedef struct tagPOINT {
  LONG x;
  LONG y;
} POINT,*PPOINT,*NPPOINT,*LPPOINT;

typedef struct _POINTL {
  LONG x;
  LONG y;
} POINTL,*PPOINTL;

typedef struct tagSIZE {
  LONG cx;
  LONG cy;
} SIZE,*PSIZE,*LPSIZE;

typedef SIZE SIZEL;
typedef SIZE *PSIZEL,*LPSIZEL;

typedef struct tagPOINTS {
  SHORT x;
  SHORT y;
} POINTS,*PPOINTS,*LPPOINTS;

#ifndef _FILETIME_
#define _FILETIME_
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME,*PFILETIME,*LPFILETIME;
#endif /* !_FILETIME_ */

#ifndef DECLSPEC_SELECTANY
#define DECLSPEC_SELECTANY __declspec(selectany)
#endif

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#endif


__DECL_END

#ifndef NT_INCLUDED
#ifndef _WINNT_
#include "winnt.h"
#endif /* !_WINNT_ */
#endif

#endif /* _WINDEF_ */

