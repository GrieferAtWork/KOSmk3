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

#ifdef __CC__
DECL_BEGIN
struct task;

#if 1
#define THIS_TASK                         __get_this_task()
FORCELOCAL ATTR_CONST WUNUSED struct task *(KCALL __get_this_task)(void) {
    register struct task *__result;
#ifdef __ASM_TASK_SEGMENT_ISFS
#ifdef __x86_64__
    __asm__("movq %%fs:0, %0\n" : "=r" (__result));
#else
    __asm__("movl %%fs:0, %0\n" : "=r" (__result));
#endif
#else
#ifdef __x86_64__
    __asm__("movq %%gs:0, %0\n" : "=r" (__result));
#else
    __asm__("movl %%gs:0, %0\n" : "=r" (__result));
#endif
#endif
    return __result;
}

#define PERTASK(x)    (*(__typeof__(&(x)))__get_per_task(&(x)))
FORCELOCAL ATTR_CONST WUNUSED void *(KCALL __get_per_task)(void *__ptr) {
    register void *__result;
#ifdef __ASM_TASK_SEGMENT_ISFS
#ifdef __x86_64__
    __asm__("addq %%fs:0, %0\n" : "=r" (__result) : "0" (__ptr));
#else
    __asm__("addl %%fs:0, %0\n" : "=r" (__result) : "0" (__ptr));
#endif
#else
#ifdef __x86_64__
    __asm__("addq %%gs:0, %0\n" : "=r" (__result) : "0" (__ptr));
#else
    __asm__("addl %%gs:0, %0\n" : "=r" (__result) : "0" (__ptr));
#endif
#endif
    return __result;
}
#elif 1
#ifdef __ASM_TASK_SEGMENT_ISFS
#ifdef __x86_64__
#define THIS_TASK \
 XBLOCK({ register struct task *_this_task; \
          __asm__("movq %%fs:0, %0\n" : "=r" (_this_task)); \
          XRETURN _this_task; })
#define PERTASK(x) \
 (*XBLOCK({ register void *_result; \
            __asm__("addq %%fs:0, %0\n" \
                    : "=r" (_result) : "0" ((void *)(uintptr_t)&(x))); \
            XRETURN (__typeof__(&(x)))_result; }))
#else
#define THIS_TASK \
 XBLOCK({ register struct task *_this_task; \
          __asm__("movl %%fs:0, %0\n" : "=r" (_this_task)); \
          XRETURN _this_task; })
#define PERTASK(x) \
 (*XBLOCK({ register void *_result; \
            __asm__("addl %%fs:0, %0\n" \
                    : "=r" (_result) : "0" ((void *)(uintptr_t)&(x))); \
            XRETURN (__typeof__(&(x)))_result; }))
#endif
#else /* __ASM_TASK_SEGMENT_ISFS */
#ifdef __x86_64__
#define THIS_TASK \
 XBLOCK({ register struct task *_this_task; \
          __asm__("movq %%gs:0, %0\n" : "=r" (_this_task)); \
          XRETURN _this_task; })
#define PERTASK(x) \
 (*XBLOCK({ register void *_result; \
            __asm__("addq %%gs:0, %0\n" \
                    : "=r" (_result) : "0" ((void *)(uintptr_t)&(x))); \
            XRETURN (__typeof__(&(x)))_result; }))
#else
#define THIS_TASK \
 XBLOCK({ register struct task *_this_task; \
          __asm__("movl %%gs:0, %0\n" : "=r" (_this_task)); \
          XRETURN _this_task; })
#define PERTASK(x) \
 (*XBLOCK({ register void *_result; \
            __asm__("addl %%gs:0, %0\n" \
                    : "=r" (_result) : "0" ((void *)(uintptr_t)&(x))); \
            XRETURN (__typeof__(&(x)))_result; }))
#endif
#endif /* !__ASM_TASK_SEGMENT_ISFS */
#else
#ifdef __ASM_TASK_SEGMENT_ISFS
#ifdef __x86_64__
#define THIS_TASK \
 XBLOCK({ register struct task *_this_task; \
          __asm__("movq %%fs:0, %0\n" : "=r" (_this_task)); \
          XRETURN _this_task; })
#define PERTASK(x) \
 (*XBLOCK({ register void *_result = (void *)(uintptr_t)&(x); \
            __asm__("addq %%fs:0, %0\n" \
                    : "+r" (_result)); \
            XRETURN (__typeof__(&(x)))_result; }))
#else
#define THIS_TASK \
 XBLOCK({ register struct task *_this_task; \
          __asm__("movl %%fs:0, %0\n" : "=r" (_this_task)); \
          XRETURN _this_task; })
#define PERTASK(x) \
 (*XBLOCK({ register void *_result = (void *)(uintptr_t)&(x); \
            __asm__("addl %%fs:0, %0\n" \
                    : "+r" (_result)); \
            XRETURN (__typeof__(&(x)))_result; }))
#endif
#else /* __ASM_TASK_SEGMENT_ISFS */
#ifdef __x86_64__
#define THIS_TASK \
 XBLOCK({ register struct task *_this_task; \
          __asm__("movq %%gs:0, %0\n" : "=r" (_this_task)); \
          XRETURN _this_task; })
#define PERTASK(x) \
 (*XBLOCK({ register void *_result = (void *)(uintptr_t)&(x); \
            __asm__("addq %%gs:0, %0\n" \
                    : "+r" (_result)); \
            XRETURN (__typeof__(&(x)))_result; }))
#else
#define THIS_TASK \
 XBLOCK({ register struct task *_this_task; \
          __asm__("movl %%gs:0, %0\n" : "=r" (_this_task)); \
          XRETURN _this_task; })
#define PERTASK(x) \
 (*XBLOCK({ register void *_result = (void *)(uintptr_t)&(x); \
            __asm__("addl %%gs:0, %0\n" \
                    : "+r" (_result)); \
            XRETURN (__typeof__(&(x)))_result; }))
#endif
#endif /* !__ASM_TASK_SEGMENT_ISFS */
#endif

/* Set the current value of the user TLS register (the base address of %gs/%fs)
 * NOTE: During a context switch, this is done automatically. */
FUNDEF void KCALL set_user_tls_register(void *value);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_PERTASK_H */
