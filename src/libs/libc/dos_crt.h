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
#ifndef GUARD_LIBS_LIBC_DOS_CRT_H
#define GUARD_LIBS_LIBC_DOS_CRT_H 1

#include "libc.h"

#ifdef __CC__
DECL_BEGIN

typedef void (LIBCCALL *CRT_DUMP_CLIENT)(void *, size_t);
typedef int (LIBCCALL *CRT_REPORT_HOOK)(int32_t, char *, int32_t *);
typedef int (LIBCCALL *CRT_REPORT_HOOKW)(int32_t, wchar_t *, int32_t *);
typedef int (LIBCCALL *CRT_ALLOC_HOOK)(int32_t, void *, size_t, int32_t, int32_t, unsigned char const *, int32_t);

#define _FREE_BLOCK      0
#define _NORMAL_BLOCK    1
#define _CRT_BLOCK       2
#define _IGNORE_BLOCK    3
#define _CLIENT_BLOCK    4
#define _MAX_BLOCKS      5

struct CrtMemBlockHeader;
typedef struct CrtMemState {
    struct CrtMemBlockHeader * pBlockHeader;
    size_t lCounts[_MAX_BLOCKS];
    size_t lSizes[_MAX_BLOCKS];
    size_t lHighWaterCount;
    size_t lTotalCount;
} CrtMemState;
typedef void *_HFILE;


INTDEF int LIBCCALL libd_CrtCheckMemory(void);
INTDEF int32_t LIBCCALL libd_CrtSetDbgFlag(int32_t newflag);
INTDEF void LIBCCALL libd_CrtDoForAllClientObjects(void (LIBCCALL *pfn)(void *, void *), void *context);
INTDEF int LIBCCALL libd_CrtIsValidPointer(const void *ptr, uint32_t bytes, int readwrite);
INTDEF int LIBCCALL libd_CrtIsValidHeapPointer(const void *heapptr);
INTDEF int LIBCCALL libd_CrtIsMemoryBlock(const void *memory, uint32_t bytes, int32_t *requestnumber, char **file, int32_t *lno);
INTDEF int LIBCCALL libd_CrtReportBlockType(const void *memory);
INTDEF CRT_DUMP_CLIENT LIBCCALL libd_CrtGetDumpClient(void);
INTDEF CRT_DUMP_CLIENT LIBCCALL libd_CrtSetDumpClient(CRT_DUMP_CLIENT pfnnewdump);
INTDEF void LIBCCALL libd_CrtMemCheckpoint(CrtMemState *state);
INTDEF int LIBCCALL libd_CrtMemDifference(CrtMemState *state, CrtMemState const *oldstate, CrtMemState const *newstate);
INTDEF void LIBCCALL libd_CrtMemDumpAllObjectsSince(CrtMemState const *state);
INTDEF void LIBCCALL libd_CrtMemDumpStatistics(CrtMemState const *state);
INTDEF int LIBCCALL libd_CrtDumpMemoryLeaks(void);
INTDEF int32_t LIBCCALL libd_CrtSetCheckCount(int32_t checkcount);
INTDEF int32_t LIBCCALL libd_CrtGetCheckCount(void);
INTDEF int32_t *LIBCCALL libd_p_crtAssertBusy(void);
INTDEF int32_t libd_crtAssertBusy;
INTDEF CRT_REPORT_HOOK LIBCCALL libd_CrtGetReportHook(void);
INTDEF CRT_REPORT_HOOK LIBCCALL libd_CrtSetReportHook(CRT_REPORT_HOOK pfnnewhook);
INTDEF int LIBCCALL libd_CrtSetReportHook2(int mode, CRT_REPORT_HOOK pfnnewhook);
INTDEF int LIBCCALL libd_CrtSetReportHookW2(int mode, CRT_REPORT_HOOKW pfnnewhook);
INTDEF int LIBCCALL libd_CrtSetReportMode(int reporttype, int reportmode);
INTDEF _HFILE LIBCCALL libd_CrtSetReportFile(int reporttype, _HFILE reportfile);
INTDEF int LIBCCALL libd_CrtDbgReport(int reporttype, char const *file, int lno, char const *modulename, char const *format, ...);
INTDEF size_t LIBCCALL libd_CrtSetDebugFillThreshold(size_t newdebugfillthreshold);
INTDEF int LIBCCALL libd_CrtDbgReportW(int reporttype, char16_t const *file, int lno, char16_t const *modulename, char16_t const *format, ...);
INTDEF void LIBCCALL libd_CrtDbgBreak(void);
INTDEF int32_t *LIBCCALL libd_p_crtBreakAlloc(void);
INTDEF int32_t libd_crtBreakAlloc;
INTDEF int32_t LIBCCALL libd_CrtSetBreakAlloc(int32_t breakalloc);
INTDEF CRT_ALLOC_HOOK LIBCCALL libd_CrtGetAllocHook(void);
INTDEF CRT_ALLOC_HOOK LIBCCALL libd_CrtSetAllocHook(CRT_ALLOC_HOOK pfnnewhook);
INTDEF int32_t *LIBCCALL libd_p_crtDbgFlag(void);
INTDEF int32_t libd_crtDbgFlag;
INTDEF void LIBCCALL libd_vacopy(char **pdst, char *src);
INTDEF void LIBCCALL libd_freea(void *ptr);


/*
    _CRT_RTC_INIT
    _CRT_RTC_INITW
    _Cbuild
    _CreateFrameInfo
    _CrtSetDbgBlockType
    _CxxThrowException
    _FCbuild
    _FindAndUnlinkFrame
    _GetImageBase
    _GetThrowImageBase
    _Getdays
    _Getmonths
    _Gettnames
    _HUGE
    _IsExceptionObjectToBeDestroyed
    _LCbuild
    _SetImageBase
    _SetThrowImageBase
    _SetWinRTOutOfMemoryExceptionCallback
    _VCrtDbgReportA
    _VCrtDbgReportW
    _W_Getdays
    _W_Getmonths
    _W_Gettnames
    _Wcsftime
    _XcptFilter
*/

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_DOS_CRT_H */
