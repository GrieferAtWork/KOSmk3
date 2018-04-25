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
#ifndef GUARD_LIBS_LIBC_DOS_CRT_C
#define GUARD_LIBS_LIBC_DOS_CRT_C 1

#include "libc.h"
#include "dos_crt.h"
#include "malloc.h"
#include "rtl.h"
#include "environ.h"
#include "heap.h"
#include "nop.h"
#include "sched.h"
#include <hybrid/xch.h>
#include <kos/environ.h>

DECL_BEGIN

CRT_DOS_BSS int32_t libd_crtAssertBusy = 0;
CRT_DOS_BSS int32_t libd_crtBreakAlloc = 0;
CRT_DOS_BSS int32_t libd_crtDbgFlag = 0;
EXPORT(__DSYM(_crtAssertBusy),libd_crtAssertBusy);
EXPORT(__DSYM(_crtBreakAlloc),libd_crtBreakAlloc);
EXPORT(__DSYM(_crtDbgFlag),libd_crtDbgFlag);

EXPORT(__DSYM(__p__crtAssertBusy),libd_p_crtAssertBusy);
CRT_DOS int32_t *LIBCCALL libd_p_crtAssertBusy(void) {
 return &libd_crtAssertBusy;
}

EXPORT(__DSYM(__p__crtBreakAlloc),libd_p_crtBreakAlloc);
CRT_DOS int32_t *LIBCCALL libd_p_crtBreakAlloc(void) {
 return &libd_crtBreakAlloc;
}

EXPORT(__DSYM(__p__crtDbgFlag),libd_p_crtDbgFlag);
CRT_DOS int32_t *LIBCCALL libd_p_crtDbgFlag(void) {
 return &libd_crtDbgFlag;
}

EXPORT(__DSYM(_CrtSetBreakAlloc),libd_CrtSetBreakAlloc);
CRT_DOS int32_t LIBCCALL libd_CrtSetBreakAlloc(int32_t breakalloc) {
 return XCH(libd_crtBreakAlloc,breakalloc);
}


EXPORT(__DSYM(_CrtCheckMemory),libd_CrtCheckMemory);
CRT_DOS int LIBCCALL libd_CrtCheckMemory(void) {
 libc_heap_validate_all();
 return 0;
}

EXPORT(__DSYM(_CrtSetDbgFlag),libd_CrtSetDbgFlag);
CRT_DOS int32_t LIBCCALL libd_CrtSetDbgFlag(int32_t newflag) {
 return XCH(libd_crtDbgFlag,newflag);
}

EXPORT(__DSYM(_CrtDoForAllClientObjects),libd_CrtDoForAllClientObjects);
CRT_DOS void LIBCCALL
libd_CrtDoForAllClientObjects(void (LIBCCALL *pfn)(void *, void *), void *context) {
 /* XXX: Re-implement? */
}

EXPORT(__DSYM(_CrtIsValidPointer),libd_CrtIsValidPointer);
CRT_DOS int LIBCCALL
libd_CrtIsValidPointer(const void *ptr, uint32_t bytes, int readwrite) {
 /* Not ~really~ implemented (But DOS doesn't do anything else either...) */
 return ptr != NULL;
}

EXPORT(__DSYM(_CrtIsValidHeapPointer),libd_CrtIsValidHeapPointer);
CRT_DOS int LIBCCALL
libd_CrtIsValidHeapPointer(const void *heapptr) {
 /* XXX: Check if `heapptr' is a valid mall-pointer when debug-malloc was enabled? */
 return heapptr != NULL;
}

EXPORT(__DSYM(_CrtIsMemoryBlock),libd_CrtIsMemoryBlock);
CRT_DOS int LIBCCALL
libd_CrtIsMemoryBlock(const void *memory, uint32_t bytes,
                      int32_t *requestnumber, char **file,
                      int32_t *lno) {
 if (requestnumber) *requestnumber = 0;
 if (file) *file = NULL;
 if (lno) *lno = 0;
 return 0;
}

EXPORT(__DSYM(_CrtReportBlockType),libd_CrtReportBlockType);
CRT_DOS int LIBCCALL libd_CrtReportBlockType(const void *memory) {
 return libd_CrtIsValidHeapPointer(memory) ? 1 : -1;
}

CRT_DOS_BSS CRT_DUMP_CLIENT libd_crtDumpClient = NULL;
EXPORT(__DSYM(_CrtGetDumpClient),libd_CrtGetDumpClient);
CRT_DOS CRT_DUMP_CLIENT LIBCCALL libd_CrtGetDumpClient(void) {
 return libd_crtDumpClient;
}
EXPORT(__DSYM(_CrtSetDumpClient),libd_CrtSetDumpClient);
CRT_DOS CRT_DUMP_CLIENT LIBCCALL libd_CrtSetDumpClient(CRT_DUMP_CLIENT pfnnewdump) {
 return XCH(libd_crtDumpClient,pfnnewdump);
}

EXPORT(__DSYM(_CrtMemCheckpoint),libd_CrtMemCheckpoint);
CRT_DOS void LIBCCALL libd_CrtMemCheckpoint(CrtMemState *UNUSED(state)) {
}

EXPORT(__DSYM(_CrtMemDifference),libd_CrtMemDifference);
DEFINE_NOP_FUNCTION_ZERO(CRT_DOS,int,libd_CrtMemDifference,
                        (CrtMemState *UNUSED(state),
                         CrtMemState const *UNUSED(oldstate),
                         CrtMemState const *UNUSED(newstate)))

EXPORT(__DSYM(_CrtMemDumpAllObjectsSince),libd_CrtMemDumpAllObjectsSince);
DEFINE_NOP_FUNCTION_VOID(CRT_DOS,libd_CrtMemDumpAllObjectsSince,(CrtMemState const *UNUSED(state)))

EXPORT(__DSYM(_CrtMemDumpStatistics),libd_CrtMemDumpStatistics);
DEFINE_NOP_FUNCTION_VOID(CRT_DOS,libd_CrtMemDumpStatistics,(CrtMemState const *UNUSED(state)))

EXPORT(__DSYM(_CrtDumpMemoryLeaks),libd_CrtDumpMemoryLeaks);
CRT_DOS int LIBCCALL libd_CrtDumpMemoryLeaks(void) {
 return (int)libc_heap_dump_leaks();
}

CRT_DOS_BSS int32_t libd_crtCheckCount = 0;
EXPORT(__DSYM(_CrtGetCheckCount),libd_CrtGetCheckCount);
CRT_DOS int32_t LIBCCALL libd_CrtGetCheckCount(void) {
 return libd_crtCheckCount;
}
EXPORT(__DSYM(_CrtSetCheckCount),libd_CrtSetCheckCount);
CRT_DOS int32_t LIBCCALL libd_CrtSetCheckCount(int32_t checkcount) {
 return XCH(libd_crtCheckCount,checkcount);
}

CRT_DOS_BSS CRT_REPORT_HOOK libd_crtReportHook = NULL;
EXPORT(__DSYM(_CrtGetReportHook),libd_CrtGetReportHook);
CRT_DOS CRT_REPORT_HOOK LIBCCALL libd_CrtGetReportHook(void) {
 return libd_crtReportHook;
}
EXPORT(__DSYM(_CrtSetReportHook),libd_CrtSetReportHook);
CRT_DOS CRT_REPORT_HOOK LIBCCALL libd_CrtSetReportHook(CRT_REPORT_HOOK pfnnewhook) {
 return XCH(libd_crtReportHook,pfnnewhook);
}
DEFINE_INTERN_ALIAS(libd_CrtSetReportHook2,libd_CrtSetReportHook);
DEFINE_INTERN_ALIAS(libd_CrtSetReportHookW2,libd_CrtSetReportHook);
EXPORT(__DSYM(_CrtSetReportHook2),libd_CrtSetReportHook2);
EXPORT(__DSYM(_CrtSetReportHookW2),libd_CrtSetReportHookW2);

EXPORT(__DSYM(_CrtSetReportMode),libd_CrtSetReportMode);
DEFINE_NOP_FUNCTION_ZERO(CRT_DOS,int,libd_CrtSetReportMode,
                        (int UNUSED(reporttype), int UNUSED(reportmode)))
EXPORT(__DSYM(_CrtSetReportFile),libd_CrtSetReportFile);
DEFINE_NOP_FUNCTION_ZERO(CRT_DOS,_HFILE,libd_CrtSetReportFile,
                        (int UNUSED(reporttype), _HFILE UNUSED(reportfile)))

EXPORT(__DSYM(_CrtDbgReport),libd_CrtDbgReport);
CRT_DOS int LIBCCALL
libd_CrtDbgReport(int reporttype, char const *file, int lno,
                  char const *modulename, char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_core_assertion_failure("_CrtDbgReport",file,lno,modulename,format,args);
#if 0 /* Noreturn... */
 va_end(args);
 return 0;
#endif
}
EXPORT(__DSYM(_CrtDbgReportW),libd_CrtDbgReportW);
EXPORT("DSYM$?_CrtDbgReportW%%YAHHPEBGH00ZZ",libd_CrtDbgReportW);
CRT_DOS int LIBCCALL
libd_CrtDbgReportW(int reporttype, char16_t const *file, int lno,
                   char16_t const *modulename, char16_t const *format, ...) {
 /* TODO */
 return 0;
}

CRT_DOS_DATA size_t libd_crtDebugFillThreshold = SIZE_MAX;
EXPORT(__DSYM(_CrtSetDebugFillThreshold),libd_CrtSetDebugFillThreshold);
CRT_DOS size_t LIBCCALL libd_CrtSetDebugFillThreshold(size_t newdebugfillthreshold) {
 return XCH(libd_crtDebugFillThreshold,newdebugfillthreshold);
}

EXPORT(__DSYM(_CrtDbgBreak),libd_CrtDbgBreak);
CRT_DOS void LIBCCALL libd_CrtDbgBreak(void) {
#if defined(__i386__) || defined(__x86_64__)
 __asm__("int3");
#else
#warning "No breakpoint instruction defined"
#endif
}

CRT_DOS_BSS CRT_ALLOC_HOOK libd_crtAllocHook = NULL;
EXPORT(__DSYM(_CrtGetAllocHook),libd_CrtGetAllocHook);
CRT_DOS CRT_ALLOC_HOOK LIBCCALL libd_CrtGetAllocHook(void) {
 return libd_crtAllocHook;
}
EXPORT(__DSYM(_CrtSetAllocHook),libd_CrtSetAllocHook);
CRT_DOS CRT_ALLOC_HOOK LIBCCALL
libd_CrtSetAllocHook(CRT_ALLOC_HOOK pfnnewhook) {
 return XCH(libd_crtAllocHook,pfnnewhook);
}

EXPORT(__DSYM(_vacopy),libd_vacopy);
CRT_DOS void LIBCCALL
libd_vacopy(char **pdst, char *src) {
 *pdst = src;
}


#if defined(__i386__) || defined(__x86_64__)
#   define FREEA_MARKER_SIZE   8
#elif defined(__ia64__)
#   define FREEA_MARKER_SIZE   16
#elif defined(__arm__)
#   define FREEA_MARKER_SIZE   8
#endif
#define FREEA_HEAP_MARKER      0xDDDD


EXPORT(__DSYM(_freea),libd_freea);
EXPORT(__DSYM(_freea_s),libd_freea);
CRT_DOS void LIBCCALL libd_freea(void *ptr) {
 if (ptr) {
  *(uintptr_t *)&ptr -= FREEA_MARKER_SIZE;
  if (*(u32 *)ptr == FREEA_HEAP_MARKER)
      libc_free(ptr);
 }
}

DEFINE_NOP_FUNCTION_ZERO(CRT_DOS,int,libd_set_error_mode,(int UNUSED(mode)))
DEFINE_NOP_FUNCTION_VOID(CRT_DOS,libd_set_app_type,(int UNUSED(type)))
EXPORT(__DSYM(_seterrormode),libd_set_error_mode);
EXPORT(__DSYM(_set_error_mode),libd_set_error_mode);
EXPORT(__DSYM(__set_app_type),libd_set_app_type);


EXPORT(__DSYM(__getmainargs),libd_getmainargs);
CRT_DOS int LIBCCALL
libd_getmainargs(int *pargc, char ***pargv, char ***penvp,
                 int do_wildcard, dos_startupinfo_t *info) {
 struct process_environ *proc;
 proc = libc_procenv();
 if likely(proc) {
  if (pargc) *pargc = proc->pe_argc;
  if (pargv) *pargv = proc->pe_argv;
  if (penvp) *penvp = proc->pe_envp;
 } else {
  if (pargc) *pargc = 1;
  if (pargv) *pargv = environ;
  if (penvp) *penvp = environ;
 }
 return 0;
}

EXPORT(__DSYM(_XcptFilter),libd_XcptFilter);
CRT_DOS int LIBCCALL
libd_XcptFilter(u32 xno, struct _EXCEPTION_POINTERS *infp_ptrs) {
 /* XXX: TODO? */
 return 0;
}

CRT_DOS EXCEPTION_DISPOSITION LIBCCALL
libd_except_handler4(struct _EXCEPTION_RECORD *ExceptionRecord,
                     void *EstablisherFrame,
                     struct _CONTEXT *ContextRecord,
                     void *DispatcherContext) {
 /* ??? What's this supposed to do? */
 return EXCEPTION_CONTINUE_SEARCH;
}

EXPORT(__DSYM(_except_handler2),libd_except_handler4);
EXPORT(__DSYM(_except_handler3),libd_except_handler4);
EXPORT(__DSYM(_except_handler_3),libd_except_handler4);
EXPORT(__DSYM(_except_handler4),libd_except_handler4); /* XXX: Are all the others OK? */
EXPORT(__DSYM(_except_handler4_common),libd_except_handler4);




DECL_END

#endif /* !GUARD_LIBS_LIBC_DOS_CRT_C */
