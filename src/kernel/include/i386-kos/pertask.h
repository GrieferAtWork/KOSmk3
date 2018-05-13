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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_PERTASK_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_PERTASK_H 1

#include <hybrid/compiler.h>
#include <hybrid/typecore.h>
#include <kos/i386-kos/tls.h>

#ifdef __CC__
DECL_BEGIN
struct task;

#ifdef __ASM_TASK_SEGMENT_ISFS
#define __X86_PERTASK_SEGMENT "%%fs:"
#else
#define __X86_PERTASK_SEGMENT "%%gs:"
#endif


#define THIS_TASK  __get_this_task()
FORCELOCAL ATTR_CONST WUNUSED struct task *(KCALL __get_this_task)(void) {
    register struct task *__result;
#ifdef __x86_64__
    __asm__("movq " __X86_PERTASK_SEGMENT "0, %0\n" : "=r" (__result));
#else
    __asm__("movl " __X86_PERTASK_SEGMENT "0, %0\n" : "=r" (__result));
#endif
    return __result;
}

#define PERTASK(x)    (*(__typeof__(&(x)))__get_per_task(&(x)))
FORCELOCAL ATTR_CONST WUNUSED void *(KCALL __get_per_task)(void *__ptr) {
    register void *__result;
#ifdef __x86_64__
    __asm__("addq " __X86_PERTASK_SEGMENT "0, %0\n" : "=r" (__result) : "0" (__ptr));
#else
    __asm__("addl " __X86_PERTASK_SEGMENT "0, %0\n" : "=r" (__result) : "0" (__ptr));
#endif
    return __result;
}


#if 1
#define __x86_pertask_getb(lvalue)        XBLOCK({ register __typeof__(lvalue) __pt_res; __asm__("movb " __X86_PERTASK_SEGMENT "%1, %0" : "=q" (__pt_res) : "m" (lvalue)); XRETURN __pt_res; })
#define __x86_pertask_getw(lvalue)        XBLOCK({ register __typeof__(lvalue) __pt_res; __asm__("movw " __X86_PERTASK_SEGMENT "%1, %0" : "=r" (__pt_res) : "m" (lvalue)); XRETURN __pt_res; })
#define __x86_pertask_getl(lvalue)        XBLOCK({ register __typeof__(lvalue) __pt_res; __asm__("movl " __X86_PERTASK_SEGMENT "%1, %0" : "=r" (__pt_res) : "m" (lvalue)); XRETURN __pt_res; })
#define __x86_pertask_setb(lvalue,val)    XBLOCK({ __asm__("movb %b1, " __X86_PERTASK_SEGMENT "%0" : : "m" (lvalue), "iq" (val)); (void)0; })
#define __x86_pertask_setw(lvalue,val)    XBLOCK({ __asm__("movw %w1, " __X86_PERTASK_SEGMENT "%0" : : "m" (lvalue), "ir" (val)); (void)0; })
#define __x86_pertask_setl(lvalue,val)    XBLOCK({ __asm__("movl %1, " __X86_PERTASK_SEGMENT "%0" : : "m" (lvalue), "ir" (val)); (void)0; })
#define __x86_pertask_xchb(lvalue,val)    XBLOCK({ register __typeof__(lvalue) __pt_res; __asm__("xchgb " __X86_PERTASK_SEGMENT "%1, %0" : "=q" (__pt_res) : "m" (lvalue), "0" (val)); XRETURN __pt_res; })
#define __x86_pertask_xchw(lvalue,val)    XBLOCK({ register __typeof__(lvalue) __pt_res; __asm__("xchgw " __X86_PERTASK_SEGMENT "%1, %0" : "=r" (__pt_res) : "m" (lvalue), "0" (val)); XRETURN __pt_res; })
#define __x86_pertask_xchl(lvalue,val)    XBLOCK({ register __typeof__(lvalue) __pt_res; __asm__("xchgl " __X86_PERTASK_SEGMENT "%1, %0" : "=r" (__pt_res) : "m" (lvalue), "0" (val)); XRETURN __pt_res; })
#define __x86_pertask_testb(lvalue)       XBLOCK({ register __BOOL __pt_res; __asm__("testb $0xff, " __X86_PERTASK_SEGMENT "%1" : "=@ccnz" (__pt_res) : "m" (lvalue)); XRETURN __pt_res; })
#define __x86_pertask_testw(lvalue)       XBLOCK({ register __BOOL __pt_res; __asm__("testw $0xffff, " __X86_PERTASK_SEGMENT "%1" : "=@ccnz" (__pt_res) : "m" (lvalue)); XRETURN __pt_res; })
#define __x86_pertask_testl(lvalue)       XBLOCK({ register __BOOL __pt_res; __asm__("testl $0xffffffff, " __X86_PERTASK_SEGMENT "%1" : "=@ccnz" (__pt_res) : "m" (lvalue)); XRETURN __pt_res; })
#define __x86_pertask_testfb(lvalue,mask) XBLOCK({ register __BOOL __pt_res; __asm__("testb %2, " __X86_PERTASK_SEGMENT "%1" : "=@ccnz" (__pt_res) : "m" (lvalue), "iq" (mask)); XRETURN __pt_res; })
#define __x86_pertask_testfw(lvalue,mask) XBLOCK({ register __BOOL __pt_res; __asm__("testw %2, " __X86_PERTASK_SEGMENT "%1" : "=@ccnz" (__pt_res) : "m" (lvalue), "ir" (mask)); XRETURN __pt_res; })
#define __x86_pertask_testfl(lvalue,mask) XBLOCK({ register __BOOL __pt_res; __asm__("testl %2, " __X86_PERTASK_SEGMENT "%1" : "=@ccnz" (__pt_res) : "m" (lvalue), "ir" (mask)); XRETURN __pt_res; })
#ifdef __x86_64__
#define __x86_pertask_getq(lvalue)        XBLOCK({ register __typeof__(lvalue) __pt_res; __asm__("movq " __X86_PERTASK_SEGMENT "%1, %0" : "=r" (__pt_res) : "m" (lvalue)); XRETURN __pt_res; })
#define __x86_pertask_setq(lvalue,val)    XBLOCK({ __asm__("movq %1, " __X86_PERTASK_SEGMENT "%0" : : "m" (lvalue), "ir" (val)); (void)0; })
#define __x86_pertask_xchq(lvalue,val)    XBLOCK({ register __typeof__(lvalue) __pt_res; __asm__("xchgq " __X86_PERTASK_SEGMENT "%1, %0" : "=r" (__pt_res) : "m" (lvalue), "0" (val)); XRETURN __pt_res; })
#define __x86_pertask_testq(lvalue)       XBLOCK({ register __BOOL __pt_res; __asm__("testq $0xffffffffffffffff, " __X86_PERTASK_SEGMENT "%1" : "=@ccnz" (__pt_res) : "m" (lvalue)); XRETURN __pt_res; })
#define __x86_pertask_testfq(lvalue,mask) XBLOCK({ register __BOOL __pt_res; __asm__("testq %2, " __X86_PERTASK_SEGMENT "%1" : "=@ccnz" (__pt_res) : "m" (lvalue), "ir" (mask)); XRETURN __pt_res; })
#define PERTASK_GET(x) \
  __builtin_choose_expr(sizeof(x) == 1,__x86_pertask_getb(x), \
  __builtin_choose_expr(sizeof(x) == 2,__x86_pertask_getw(x), \
  __builtin_choose_expr(sizeof(x) == 4,__x86_pertask_getl(x), \
  __builtin_choose_expr(sizeof(x) == 8,__x86_pertask_getq(x), \
                       (__typeof__(x))PERTASK(x)))))
#define PERTASK_SET(x,v) \
  __builtin_choose_expr(sizeof(x) == 1,__x86_pertask_setb(x,v), \
  __builtin_choose_expr(sizeof(x) == 2,__x86_pertask_setw(x,v), \
  __builtin_choose_expr(sizeof(x) == 4,__x86_pertask_setl(x,v), \
  __builtin_choose_expr(sizeof(x) == 8,__x86_pertask_setq(x,v), \
                       (void)(PERTASK(x) = (v))))))
#define PERTASK_XCH(x,v) \
  __builtin_choose_expr(sizeof(x) == 1,__x86_pertask_xchb(x,v), \
  __builtin_choose_expr(sizeof(x) == 2,__x86_pertask_xchw(x,v), \
  __builtin_choose_expr(sizeof(x) == 4,__x86_pertask_xchl(x,v), \
  __builtin_choose_expr(sizeof(x) == 8,__x86_pertask_xchq(x,v), \
                        XCH(PERTASK(x),v)))))
#define PERTASK_TEST(x) \
  __builtin_choose_expr(sizeof(x) == 1,__x86_pertask_testb(x), \
  __builtin_choose_expr(sizeof(x) == 2,__x86_pertask_testw(x), \
  __builtin_choose_expr(sizeof(x) == 4,__x86_pertask_testl(x), \
  __builtin_choose_expr(sizeof(x) == 8,__x86_pertask_testq(x), \
                        !!PERTASK(x)))))
#define PERTASK_TESTF(x,flags) \
  __builtin_choose_expr(sizeof(x) == 1,__x86_pertask_testfb(x,flags), \
  __builtin_choose_expr(sizeof(x) == 2,__x86_pertask_testfw(x,flags), \
  __builtin_choose_expr(sizeof(x) == 4,__x86_pertask_testfl(x,flags), \
  __builtin_choose_expr(sizeof(x) == 8,__x86_pertask_testfq(x,flags), \
                        !!(PERTASK(x) & (flags))))))
#else
#define PERTASK_GET(x) \
  __builtin_choose_expr(sizeof(x) == 1,__x86_pertask_getb(x), \
  __builtin_choose_expr(sizeof(x) == 2,__x86_pertask_getw(x), \
  __builtin_choose_expr(sizeof(x) == 4,__x86_pertask_getl(x), \
                       (__typeof__(x))PERTASK(x))))
#define PERTASK_SET(x,v) \
  __builtin_choose_expr(sizeof(x) == 1,__x86_pertask_setb(x,v), \
  __builtin_choose_expr(sizeof(x) == 2,__x86_pertask_setw(x,v), \
  __builtin_choose_expr(sizeof(x) == 4,__x86_pertask_setl(x,v), \
                       (void)(PERTASK(x) = (v)))))
#define PERTASK_XCH(x,v) \
  __builtin_choose_expr(sizeof(x) == 1,__x86_pertask_xchb(x,v), \
  __builtin_choose_expr(sizeof(x) == 2,__x86_pertask_xchw(x,v), \
  __builtin_choose_expr(sizeof(x) == 4,__x86_pertask_xchl(x,v), \
                        XCH(PERTASK(x),v))))
#define PERTASK_TEST(x) \
  __builtin_choose_expr(sizeof(x) == 1,__x86_pertask_testb(x), \
  __builtin_choose_expr(sizeof(x) == 2,__x86_pertask_testw(x), \
  __builtin_choose_expr(sizeof(x) == 4,__x86_pertask_testl(x), \
                        !!PERTASK(x))))
#define PERTASK_TESTF(x,flags) \
  __builtin_choose_expr(sizeof(x) == 1,__x86_pertask_testfb(x,flags), \
  __builtin_choose_expr(sizeof(x) == 2,__x86_pertask_testfw(x,flags), \
  __builtin_choose_expr(sizeof(x) == 4,__x86_pertask_testfl(x,flags), \
                        !!(PERTASK(x) & (flags)))))
#endif
#endif


/* Set the current value of the user TLS register (the base address of %gs/%fs)
 * NOTE: During a context switch, this is done automatically. */
FUNDEF void KCALL set_user_tls_register(void *value);
#ifndef CONFIG_NO_DOS_COMPAT
FUNDEF void KCALL set_user_tib_register(void *value);
#endif /* !CONFIG_NO_DOS_COMPAT */

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_PERTASK_H */
