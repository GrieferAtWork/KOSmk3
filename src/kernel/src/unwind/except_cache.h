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
#ifndef GUARD_KERNEL_SRC_KERNEL_UNWIND_EXCEPT_CACHE_H
#define GUARD_KERNEL_SRC_KERNEL_UNWIND_EXCEPT_CACHE_H 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <unwind/linker.h>
#include <unwind/eh_frame.h>
#include <kernel/debug.h>
#include <kernel/heap.h>
#include <kernel/malloc.h>
#include <fs/linker.h>
#include <fs/driver.h>
#include <string.h>
#include <assert.h>
#include <except.h>

DECL_BEGIN

#undef CONFIG_DISABLE_EXCEPTION_HANDLER_CACHE
#if 0
#define CONFIG_DISABLE_EXCEPTION_HANDLER_CACHE 1
#endif


#if 1
/* Reduce the memory impact of these caches and speed up
 * a valid, but _really_ extensive scenario where there is
 * little pre-cached memory available, meaning that trying
 * to allocate anything results in 4 exceptions being stacked
 * ontop of each other.
 * While that scenario is completely valid (and does resolve itself),
 * it takes quite a while until it does.
 * By disabling tracing of caches, that scenario doesn't happen:
 * ..\src\kernel\src\unwind\except_cache.c(588,0) : except_cache_lookup : C013E9D9 : ESP C02462C0, EBP C0246344
 * ..\src\kernel\src\unwind\except_cache.c(728,0) : kernel_findexcept : C013EE13 : ESP C024634C, EBP C0246368
 * ..\src\kernel\src\unwind\linker.c(172,0) : linker_findexcept : C0140F99 : ESP C0246370, EBP C02463B8
 * ..\src\kernel\src\unwind\linker.c(46,0) : linker_findexcept_consafe : C0140AE8 : ESP C02463C0, EBP C0246428
 * ..\src\kernel\i386-kos\except.c(105,0) : libc_error_rethrow_at : C010768D : ESP C0246430, EBP C02464D8
 * ..\src\hybrid\i386-kos\except32.S(68,0) : .error_rethrow_eax_is_ip : C0103940 : ESP C02464E0, EBP C0246520
 * ..\src\kernel\i386-kos\scheduler32.S(299,0) : task_yield : C0124D2A : ESP C024651C, EBP C0246520
 * ..\include\hybrid\sync\atomic-rwlock.h(178,0) : atomic_rwlock_write : C0130B46 : ESP C0246520, EBP C0246520
 * ..\src\kernel\src\kernel\mall.c(1104,0) : define_user_traceing_point : C01338A1 : ESP C0246528, EBP C0246530
 * ..\src\kernel\src\kernel\mall.c(1120,0) : mall_trace : C0133932 : ESP C0246538, EBP C0246548
 * ..\src\kernel\src\kernel\mall.c(1212,0) : heap_alloc : C0133BFC : ESP C0246550, EBP C0246588
 * ..\src\kernel\include\kernel\heap.h(219,0) : heap_alloc : C013DCAD
 * ..\src\kernel\src\unwind\except_cache.c(214,0) : alloc_info : C013DCAD : ESP C0246590, EBP C02466C4
 * ..\src\kernel\src\unwind\except_cache.c(472,0) : allocate_exception_info : C013E5A7 : ESP C02466CC, EBP C0246738
 * ..\src\kernel\src\unwind\except_cache.c(592,0) : except_cache_lookup : C013E9F8 : ESP C0246740, EBP C02467DC
 * ..\src\kernel\src\unwind\except_cache.c(728,0) : kernel_findexcept : C013EE13 : ESP C02467E4, EBP C0246800
 * ..\src\kernel\src\unwind\linker.c(172,0) : linker_findexcept : C0140F99 : ESP C0246808, EBP C0246850
 * ..\src\kernel\src\unwind\linker.c(46,0) : linker_findexcept_consafe : C0140AE8 : ESP C0246858, EBP C02468C0
 * ..\src\kernel\i386-kos\except.c(105,0) : libc_error_rethrow_at : C010768D : ESP C02468C8, EBP C0246970
 * ..\src\hybrid\i386-kos\except32.S(68,0) : .error_rethrow_eax_is_ip : C0103940 : ESP C0246978, EBP C02469B8
 * ..\src\kernel\i386-kos\scheduler32.S(299,0) : task_yield : C0124D2A : ESP C02469B4, EBP C02469B8
 * ..\include\hybrid\sync\atomic-rwlock.h(178,0) : atomic_rwlock_write : C0130B46 : ESP C02469B8, EBP C02469B8
 * ..\src\kernel\src\kernel\mall.c(1104,0) : define_user_traceing_point : C01338A1 : ESP C02469C0, EBP C02469C8
 * ..\src\kernel\src\kernel\mall.c(1120,0) : mall_trace : C0133932 : ESP C02469D0, EBP C02469E0
 * ..\src\kernel\src\kernel\mall.c(1212,0) : heap_alloc : C0133BFC : ESP C02469E8, EBP C0246A20
 * ..\src\kernel\include\kernel\heap.h(219,0) : heap_alloc : C0140268
 * ..\src\kernel\src\unwind\fde_cache.c(152,0) : alloc_info : C0140268 : ESP C0246A28, EBP C0246B5C
 * ..\src\kernel\src\unwind\fde_cache.c(234,0) : fde_cache_insert : C0140684 : ESP C0246B64, EBP C0246BB0
 * ..\src\kernel\src\unwind\fde_cache.c(342,0) : kernel_eh_findfde : C0140A30 : ESP C0246BB8, EBP C0246BC0
 * ..\src\kernel\src\unwind\linker.c(79,0) : linker_findfde : C0140C25 : ESP C0246BC8, EBP C0246C10
 * ..\src\kernel\src\unwind\linker.c(40,0) : linker_findfde_consafe : C0140A78 : ESP C0246C18, EBP C0246C78
 * ..\src\kernel\i386-kos\except.c(99,0) : libc_error_rethrow_at : C010763A : ESP C0246C80, EBP C0246D24
 * ..\src\kernel\i386-kos\hw_except.c(108,0) : error_rethrow_atuser : C0108E6D : ESP C0246D2C, EBP C0246D34
 * ..\src\kernel\i386-kos\hw_except.c(369,0) : x86_handle_pagefault : C010979F : ESP C0246D3C, EBP C0246E4C
 * ..\src\kernel\i386-kos\hw_except32.S(138,0) : irq_0e : C0101362 : ESP C0246E54, EBP C0246EB4
 * -> All the way down here is where the original exception happened.
 *    All of the stuff above is able to resolve itself, but it really
 *    takes a while. - A more permanent solution for this would be to
 *    introduce a heap_alloc() function that never throws an exception,
 *    but rather indicates an allocation failure in some other way.
 *    It wouldn't even have to be able to interface with core_alloc(),
 *    as it would only ever be used with GFP_NOMAP.
 * ..\src\kernel\src\kernel\mall.c(235,0) : mall_reachable_data : C0131991 : ESP C0246E90, EBP C0246EB4
 * ..\src\kernel\src\kernel\mall.c(359,0) : mall_search_leaks_impl : C0131DFD : ESP C0246EBC, EBP C0246F00
 * ..\src\kernel\src\kernel\mall.c(564,0) : mall_dump_leaks : C0132607 : ESP C0246F08, EBP C0246F48
 * ..\src\kernel\src\kernel\kernctl.c(53,0) : kernel_control : C0130668 : ESP C0246F50, EBP C0246F78
 * ..\src\kernel\src\kernel\kernctl.c(145,0) : SYSC_xkernctl : C01308D4 : ESP C0246F80, EBP C0246F98
 * ..\src\kernel\src\kernel\kernctl.c(140,0) : sys_xkernctl : C01308B5 : ESP C0246FA0, EBP C0246FB8
 * ..\src\kernel\i386-kos\syscall32.S(326,0) : .sysenter_after_tracing : C0102C92 : ESP C0246FC0, EBP AFFFFE94
 * ..\??(0,0) : ?? : BEEFCF24 : ESP AFFFFE94, EBP CCCCCCCC
 */
#define HEAP_ALLOC_INFO  heap_alloc_untraced
#define HEAP_FREE_INFO   heap_free_untraced
#else
#define HEAP_ALLOC_INFO  heap_alloc
#define HEAP_FREE_INFO   heap_free
#endif

#define FREE_INFO(x) \
   HEAP_FREE_INFO(&kernel_heaps[GFP_SHARED],x,(x)->ic_size,GFP_SHARED)


PRIVATE ATTR_NOTHROW void KCALL except_free_info_node(struct except_info_cache *__restrict node);

#ifndef CONFIG_DISABLE_EXCEPTION_HANDLER_CACHE
PRIVATE ATTR_NOINLINE struct except_info_cache *KCALL alloc_info(size_t info_count);
#endif /* !CONFIG_DISABLE_EXCEPTION_HANDLER_CACHE */


#ifdef CONFIG_ELF_SUPPORT_CLASS3264
PRIVATE ATTR_NOTHROW bool KCALL
lookup_exception_without_cache_compat(struct exception_handler_compat *__restrict iter,
                                      struct exception_handler_compat *__restrict end,
                                      uintptr_t rel_ip, u16 exception_code,
                                      struct application *__restrict app,
                                      struct exception_handler_info *__restrict result);
#ifndef CONFIG_DISABLE_EXCEPTION_HANDLER_CACHE
PRIVATE ATTR_NOTHROW bool KCALL
load_exception_descriptor_compat(struct module *__restrict mod,
                                 struct application *__restrict app, uintptr_t loadaddr,
                                 struct exception_descriptor_compat const *__restrict pdesc,
                                 struct exception_handler_info *__restrict result);
PRIVATE ATTR_NOTHROW struct except_info_cache *KCALL
allocate_exception_info_compat(struct module *__restrict mod,
                               struct application *__restrict app,
                               struct exception_handler_compat *__restrict begin,
                               struct exception_handler_compat *__restrict end,
                               uintptr_t loadaddr, uintptr_t rel_ip);
#endif /* !CONFIG_DISABLE_EXCEPTION_HANDLER_CACHE */
#endif

PRIVATE ATTR_NOTHROW bool KCALL
lookup_exception_without_cache(struct exception_handler *__restrict iter,
                               struct exception_handler *__restrict end,
                               uintptr_t rel_ip, u16 exception_code,
                               struct application *__restrict app,
                               struct exception_handler_info *__restrict result);
#ifndef CONFIG_DISABLE_EXCEPTION_HANDLER_CACHE
PRIVATE ATTR_NOTHROW bool KCALL
load_exception_descriptor(struct module *__restrict mod,
                          struct application *__restrict app, uintptr_t loadaddr,
                          struct exception_descriptor const *__restrict pdesc,
                          struct exception_handler_info *__restrict result);
PRIVATE ATTR_NOTHROW struct except_info_cache *KCALL
allocate_exception_info(struct module *__restrict mod,
                        struct application *__restrict app,
                        struct exception_handler *__restrict begin,
                        struct exception_handler *__restrict end,
                        uintptr_t loadaddr, uintptr_t rel_ip);
#endif /* !CONFIG_DISABLE_EXCEPTION_HANDLER_CACHE */

DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_UNWIND_EXCEPT_CACHE_H */
