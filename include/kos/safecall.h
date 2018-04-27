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
#ifndef _KOS_SAFECALL_H
#define _KOS_SAFECALL_H 1

#include <__stdinc.h>
#include <hybrid/host.h>

__SYSDECL_BEGIN

/* SAFECALL:
 *    - Add additional debug-checks surrounding a call to
 *      a dynamic function pointer to a KCALL-function.
 *    - Since KCALL uses STDCALL on i386, the problem arises
 *      that due to improper usage, more or less than the expected
 *      number of arguments are being freed.
 *    - Such inconsistencies can lead to hard-to-track and strange
 *      kernel faults, when in actuality the problem results from
 *      a missing argument in some dynamic function pointer.
 *    - However, using inline assembly, additional checks can be
 *      inserted which allow code to detect these inconsistencies
 *      and cause a kernel panic at the proper time, including
 *      information about the exact source location of the broken
 *      function and the expected argument total.
 * USAGE:
 *  >> typedef void (KCALL *my_pfunction)(int x, int y, int z);
 *  >> DATDEF my_pfunction callback;
 *  >> PRIVATE void KCALL invoke(int x, int y, int z) {
 *  >> #if 0 // Normal way of invoking
 *  >>     (*callback)(x,y,z);
 *  >> #else // Safe (checked) way of invoking
 *  >>     SAFECALL_KCALL_VOID_3(callback,x,y,z);
 *  >> #endif
 *  >> }
 */

#ifdef __KERNEL__
#ifdef NDEBUG
#ifndef CONFIG_SAFECALL
#undef  CONFIG_NO_SAFECALL
#define CONFIG_NO_SAFECALL 1
#endif
#endif

#ifndef __i386__
#undef CONFIG_SAFECALL
#undef CONFIG_NO_SAFECALL
#define CONFIG_NO_SAFECALL 1
#endif

#ifndef CONFIG_NO_SAFECALL
#ifdef __ASSEMBLER__
#define __SAFECALL_SAFED_ESP     esi
#define __SAFECALL_TARGET_EIP    edi
#if defined(CONFIG_BUILDING_KERNEL_CORE) || \
  (!defined(__PIC__) && !defined(__PIE__))
#define __SAFECALL_MISSMATCH     x86_stdcall_missmatch
#else /* CONFIG_BUILDING_KERNEL_CORE */
#define __SAFECALL_MISSMATCH     x86_stdcall_missmatch@PLT
#endif /* !CONFIG_BUILDING_KERNEL_CORE */
#else /* __ASSEMBLER__ */
#define __SAFECALL_SAFED_ESP    "esi"
#define __SAFECALL_TARGET_EIP   "edi"
#define __SAFECALL_TARGET_EIP_C "D"
#if defined(CONFIG_BUILDING_KERNEL_CORE) || \
  (!defined(__PIC__) && !defined(__PIE__))
#define __SAFECALL_MISSMATCH    "x86_stdcall_missmatch"
#else /* CONFIG_BUILDING_KERNEL_CORE */
#define __SAFECALL_MISSMATCH    "x86_stdcall_missmatch@PLT"
#endif /* !CONFIG_BUILDING_KERNEL_CORE */
#endif /* !__ASSEMBLER__ */

/* NOTE: Disabled because of a problem related to
 *       inline assembly that I do no know how to fix:
 *       By specifying "esp" as a clobber register, we can
 *       prevent GCC from passing argument operands as memory
 *       locations that are relative to ESP.
 *       Makes sense: After all ESP changes when we start pushing values.
 *       However, GCC also interprets an ESP clobber operand as a sign
 *       that the assembly will change ESP. Yet that is exactly what we
 *       are trying to prevent: Any change being made to ESP, as we try
 *       to assert that the stack pointer is restored properly after a
 *       call to an STDCALL function was made.
 *       Yet because GCC things we are changing ESP, it will generate code
 *       that omits argument cleanup code as the result of calling a function
 *       immediately prior to the safecall, if that call was a CDECL function:
 *       >> debug_printf("foo = %d, bar = %d\n",10,20);
 *       >> SAFECALL_KCALL_VOID_0(*function_pointer);
 *       ASSEMBLY:
 *       >> .section .rodata.str
 *       >> .Lstring:
 *       >>     .string "foo = %d, bar = %d\n"
 *       >> .section .text
 *       >>     pushl $20
 *       >>     pushl $10
 *       >>     pushl $.Lstring
 *       >>     call  debug_printf
 *       >> #   addl  $12, %esp              // This instruction right here is missing because GCC things it doesn't matter
 *       >>     movl  function_pointer, %edi // GCC, loading registers for our assembly
 *       >>     movl  %esp, %esi             // This is where our SAFECALL macro starts
 *       >>     call  *%edi
 *       >>     cmpl  %esi, %esp
 *       >>     je    1f
 *       >>     call  x86_stdcall_missmatch
 *       >> 1:
 *       The problem here arises from the fact that GCC will even do this
 *       in a tightly packed loop. So you can imaging the consequences, and
 *       if you can't: the result is the same as calling `alloca()' in a loop.
 *       In other words: Because GCC does this, we'd run into a stack overflow
 *       sooner or later, or even worse: because GCC things that ESP has changed,
 *       it will make a bunch of other (false) assumptions and generate completely
 *       broken assembly...
 */
#if !defined(__INTELLISENSE__) && 0
#define __SAFECALL_KCALL_VOID(push_text,...) \
 __XBLOCK({ __asm__ __volatile__("movl %%esp, %%" __SAFECALL_SAFED_ESP "\n\t" \
                                  push_text \
                                 "call *%%" __SAFECALL_TARGET_EIP "\n\t" \
                                 "cmpl %%" __SAFECALL_SAFED_ESP ", %%esp\n\t" \
                                 "je   1f\n\t" \
                                 "call " __SAFECALL_MISSMATCH "\n\t" \
                                 "1:\n\t" \
                                 : \
                                 : __VA_ARGS__ \
                                 : "cc", "memory", __SAFECALL_SAFED_ESP, "esp"); \
            __asm__ __volatile__("" : : : "memory", "eax", "ecx", "edx"); \
           (void)0; })
#define __SAFECALL_KCALL(Treturn,push_text,...) \
 __XBLOCK({ register Treturn __safecall_result; \
            __asm__ __volatile__("movl %%esp, %%" __SAFECALL_SAFED_ESP "\n\t" \
                                  push_text \
                                 "call *%%" __SAFECALL_TARGET_EIP "\n\t" \
                                 "cmpl %%" __SAFECALL_SAFED_ESP ", %%esp\n\t" \
                                 "je   1f\n\t" \
                                 "call " __SAFECALL_MISSMATCH "\n\t" \
                                 "1:\n\t" \
                                 : "=a" (__safecall_result) \
                                 : __VA_ARGS__ \
                                 : "cc", "memory", __SAFECALL_SAFED_ESP, "esp"); \
            __asm__ __volatile__("" : : : "memory", "eax", "ecx", "edx"); \
            __XRETURN __safecall_result; })
#define SAFECALL_KCALL_VOID_0(func) \
      __SAFECALL_KCALL_VOID("",__SAFECALL_TARGET_EIP_C (func))
#define SAFECALL_KCALL_VOID_1(func,arg0) \
      __SAFECALL_KCALL_VOID("pushl %1\n\t",__SAFECALL_TARGET_EIP_C (func),"g" ((__UINTPTR_TYPE__)(arg0)))
#define SAFECALL_KCALL_VOID_2(func,arg0,arg1) \
      __SAFECALL_KCALL_VOID("pushl %2\n\tpushl %1\n\t",__SAFECALL_TARGET_EIP_C (func),"g" ((__UINTPTR_TYPE__)(arg0)),"g" ((__UINTPTR_TYPE__)(arg1)))
#define SAFECALL_KCALL_VOID_3(func,arg0,arg1,arg2) \
      __SAFECALL_KCALL_VOID("pushl %3\n\tpushl %2\n\tpushl %1\n\t", \
                            __SAFECALL_TARGET_EIP_C (func),"g" ((__UINTPTR_TYPE__)arg0),"g" ((__UINTPTR_TYPE__)arg1),"g" ((__UINTPTR_TYPE__)(arg2)))
#define SAFECALL_KCALL_VOID_4(func,arg0,arg1,arg2,arg3) \
      __SAFECALL_KCALL_VOID("pushl %4\n\tpushl %3\n\tpushl %2\n\t" \
                            "pushl %1\n\t", \
                            __SAFECALL_TARGET_EIP_C (func),"g" ((__UINTPTR_TYPE__)arg0),"g" ((__UINTPTR_TYPE__)arg1),"g" ((__UINTPTR_TYPE__)arg2), \
                            "g" ((__UINTPTR_TYPE__)arg3))
#define SAFECALL_KCALL_VOID_5(func,arg0,arg1,arg2,arg3,arg4) \
      __SAFECALL_KCALL_VOID("pushl %5\n\tpushl %4\n\tpushl %3\n\t" \
                            "pushl %2\n\tpushl %1\n\t", \
                            __SAFECALL_TARGET_EIP_C (func), \
                            "g" ((__UINTPTR_TYPE__)arg0),"g" ((__UINTPTR_TYPE__)arg1),"g" ((__UINTPTR_TYPE__)arg2), \
                            "g" ((__UINTPTR_TYPE__)arg3),"g" ((__UINTPTR_TYPE__)arg4))
#define SAFECALL_KCALL_VOID_6(func,arg0,arg1,arg2,arg3,arg4,arg5) \
      __SAFECALL_KCALL_VOID("pushl %6\n\tpushl %5\n\tpushl %4\n\t" \
                            "pushl %3\n\tpushl %2\n\tpushl %1\n\t", \
                            __SAFECALL_TARGET_EIP_C (func),"g" ((__UINTPTR_TYPE__)arg0),"g" ((__UINTPTR_TYPE__)arg1),"g" ((__UINTPTR_TYPE__)arg2), \
                            "g" ((__UINTPTR_TYPE__)arg3),"g" ((__UINTPTR_TYPE__)arg4),"g" ((__UINTPTR_TYPE__)arg5))
#define SAFECALL_KCALL_0(func) \
      __SAFECALL_KCALL(__typeof__((*func)()),"",__SAFECALL_TARGET_EIP_C (func))
#define SAFECALL_KCALL_1(func,arg0) \
      __SAFECALL_KCALL(__typeof__((*func)(arg0)), \
                       "pushl %2\n\t", \
                       __SAFECALL_TARGET_EIP_C (func),"g" ((__UINTPTR_TYPE__)arg0))
#define SAFECALL_KCALL_2(func,arg0,arg1) \
      __SAFECALL_KCALL(__typeof__((*func)(arg0,arg1)), \
                       "pushl %3\n\tpushl %2\n\t", \
                       __SAFECALL_TARGET_EIP_C (func),"g" ((__UINTPTR_TYPE__)arg0),"g" ((__UINTPTR_TYPE__)arg1))
#define SAFECALL_KCALL_3(func,arg0,arg1,arg2) \
      __SAFECALL_KCALL(__typeof__((*func)(arg0,arg1,arg2)), \
                       "pushl %4\n\tpushl %3\n\tpushl %2\n\t", \
                       __SAFECALL_TARGET_EIP_C (func),"g" ((__UINTPTR_TYPE__)arg0),"g" ((__UINTPTR_TYPE__)arg1),"g" ((__UINTPTR_TYPE__)arg2))
#define SAFECALL_KCALL_4(func,arg0,arg1,arg2,arg3) \
      __SAFECALL_KCALL(__typeof__((*func)(arg0,arg1,arg2,arg3)), \
                       "pushl %5\n\tpushl %4\n\tpushl %3\n\t" \
                       "pushl %2\n\t", \
                       __SAFECALL_TARGET_EIP_C (func),"g" ((__UINTPTR_TYPE__)arg0),"g" ((__UINTPTR_TYPE__)arg1),"g" ((__UINTPTR_TYPE__)arg2), \
                       "g" ((__UINTPTR_TYPE__)arg3))
#define SAFECALL_KCALL_5(func,arg0,arg1,arg2,arg3,arg4) \
      __SAFECALL_KCALL(__typeof__((*func)(arg0,arg1,arg2,arg3,arg4)), \
                       "pushl %6\n\tpushl %5\n\tpushl %4\n\t" \
                       "pushl %3\n\tpushl %2\n\t", \
                       __SAFECALL_TARGET_EIP_C (func), \
                       "g" ((__UINTPTR_TYPE__)arg0),"g" ((__UINTPTR_TYPE__)arg1),"g" ((__UINTPTR_TYPE__)arg2), \
                       "g" ((__UINTPTR_TYPE__)arg3),"g" ((__UINTPTR_TYPE__)arg4))
#define SAFECALL_KCALL_6(func,arg0,arg1,arg2,arg3,arg4,arg5) \
      __SAFECALL_KCALL(__typeof__((*func)(arg0,arg1,arg2,arg3,arg4,arg5)), \
                       "pushl %7\n\tpushl %6\n\tpushl %5\n\t" \
                       "pushl %4\n\tpushl %3\n\tpushl %2\n\t", \
                       __SAFECALL_TARGET_EIP_C (func),"g" ((__UINTPTR_TYPE__)arg0),"g" ((__UINTPTR_TYPE__)arg1),"g" ((__UINTPTR_TYPE__)arg2), \
                       "g" ((__UINTPTR_TYPE__)arg3),"g" ((__UINTPTR_TYPE__)arg4),"g" ((__UINTPTR_TYPE__)arg5))
#endif /* !__INTELLISENSE__ */
#endif
#endif /* __KERNEL__ */


#ifndef SAFECALL_KCALL_VOID_0
#define SAFECALL_KCALL_VOID_0(func)                               (*func)()
#define SAFECALL_KCALL_VOID_1(func,arg0)                          (*func)(arg0)
#define SAFECALL_KCALL_VOID_2(func,arg0,arg1)                     (*func)(arg0,arg1)
#define SAFECALL_KCALL_VOID_3(func,arg0,arg1,arg2)                (*func)(arg0,arg1,arg2)
#define SAFECALL_KCALL_VOID_4(func,arg0,arg1,arg2,arg3)           (*func)(arg0,arg1,arg2,arg3)
#define SAFECALL_KCALL_VOID_5(func,arg0,arg1,arg2,arg3,arg4)      (*func)(arg0,arg1,arg2,arg3,arg4)
#define SAFECALL_KCALL_VOID_6(func,arg0,arg1,arg2,arg3,arg4,arg5) (*func)(arg0,arg1,arg2,arg3,arg4,arg5)
#endif

#ifndef SAFECALL_KCALL_0
#define SAFECALL_KCALL_0(func)                                    (*func)()
#define SAFECALL_KCALL_1(func,arg0)                               (*func)(arg0)
#define SAFECALL_KCALL_2(func,arg0,arg1)                          (*func)(arg0,arg1)
#define SAFECALL_KCALL_3(func,arg0,arg1,arg2)                     (*func)(arg0,arg1,arg2)
#define SAFECALL_KCALL_4(func,arg0,arg1,arg2,arg3)                (*func)(arg0,arg1,arg2,arg3)
#define SAFECALL_KCALL_5(func,arg0,arg1,arg2,arg3,arg4)           (*func)(arg0,arg1,arg2,arg3,arg4)
#define SAFECALL_KCALL_6(func,arg0,arg1,arg2,arg3,arg4,arg5)      (*func)(arg0,arg1,arg2,arg3,arg4,arg5)
#endif


__SYSDECL_END

#endif /* !_KOS_SAFECALL_H */
