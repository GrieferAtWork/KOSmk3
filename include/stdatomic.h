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
#ifndef _STDATOMIC_H
#define _STDATOMIC_H 1
#define _STDATOMIC_H_ 1

#include <__stdinc.h>

#ifdef __cplusplus

/* C++ defines all of this stuff in `<atomic>' inside the `std' namespace. */
#include <atomic>

#ifndef __memory_order_defined
#define __memory_order_defined 1
__NAMESPACE_STD_USING(memory_order_relaxed)
__NAMESPACE_STD_USING(memory_order_consume)
__NAMESPACE_STD_USING(memory_order_acquire)
__NAMESPACE_STD_USING(memory_order_release)
__NAMESPACE_STD_USING(memory_order_acq_rel)
__NAMESPACE_STD_USING(memory_order_seq_cst)
__NAMESPACE_STD_USING(memory_order)
#endif /* !__memory_order_defined */

__NAMESPACE_STD_USING(atomic_thread_fence)
__NAMESPACE_STD_USING(atomic_signal_fence)
__NAMESPACE_STD_USING(atomic_is_lock_free)
__NAMESPACE_STD_USING(atomic_init)

#ifndef __atomic_types_defined
#define __atomic_types_defined 1
__NAMESPACE_STD_USING(atomic_bool)
__NAMESPACE_STD_USING(atomic_char)
__NAMESPACE_STD_USING(atomic_schar)
__NAMESPACE_STD_USING(atomic_uchar)
__NAMESPACE_STD_USING(atomic_short)
__NAMESPACE_STD_USING(atomic_ushort)
__NAMESPACE_STD_USING(atomic_int)
__NAMESPACE_STD_USING(atomic_uint)
__NAMESPACE_STD_USING(atomic_long)
__NAMESPACE_STD_USING(atomic_ulong)
#ifdef __COMPILER_HAVE_LONGLONG
__NAMESPACE_STD_USING(atomic_llong)
__NAMESPACE_STD_USING(atomic_ullong)
#endif /* __COMPILER_HAVE_LONGLONG */
__NAMESPACE_STD_USING(atomic_char16_t)
__NAMESPACE_STD_USING(atomic_char32_t)
__NAMESPACE_STD_USING(atomic_wchar_t)
__NAMESPACE_STD_USING(atomic_int_least8_t)
__NAMESPACE_STD_USING(atomic_uint_least8_t)
__NAMESPACE_STD_USING(atomic_int_least16_t)
__NAMESPACE_STD_USING(atomic_uint_least16_t)
__NAMESPACE_STD_USING(atomic_int_least32_t)
__NAMESPACE_STD_USING(atomic_uint_least32_t)
__NAMESPACE_STD_USING(atomic_int_least64_t)
__NAMESPACE_STD_USING(atomic_uint_least64_t)
__NAMESPACE_STD_USING(atomic_int_fast8_t)
__NAMESPACE_STD_USING(atomic_uint_fast8_t)
__NAMESPACE_STD_USING(atomic_int_fast16_t)
__NAMESPACE_STD_USING(atomic_uint_fast16_t)
__NAMESPACE_STD_USING(atomic_int_fast32_t)
__NAMESPACE_STD_USING(atomic_uint_fast32_t)
__NAMESPACE_STD_USING(atomic_int_fast64_t)
__NAMESPACE_STD_USING(atomic_uint_fast64_t)
__NAMESPACE_STD_USING(atomic_intptr_t)
__NAMESPACE_STD_USING(atomic_uintptr_t)
__NAMESPACE_STD_USING(atomic_size_t)
__NAMESPACE_STD_USING(atomic_ptrdiff_t)
__NAMESPACE_STD_USING(atomic_intmax_t)
__NAMESPACE_STD_USING(atomic_uintmax_t)
#endif /* !__atomic_types_defined */

__NAMESPACE_STD_USING(atomic_compare_exchange_strong_explicit)
__NAMESPACE_STD_USING(atomic_compare_exchange_weak_explicit)
__NAMESPACE_STD_USING(atomic_exchange_explicit)
__NAMESPACE_STD_USING(atomic_fetch_add_explicit)
__NAMESPACE_STD_USING(atomic_fetch_and_explicit)
__NAMESPACE_STD_USING(atomic_fetch_or_explicit)
__NAMESPACE_STD_USING(atomic_fetch_sub_explicit)
__NAMESPACE_STD_USING(atomic_fetch_xor_explicit)
__NAMESPACE_STD_USING(atomic_load_explicit)
__NAMESPACE_STD_USING(atomic_store_explicit)
__NAMESPACE_STD_USING(atomic_compare_exchange_strong)
__NAMESPACE_STD_USING(atomic_compare_exchange_weak)
__NAMESPACE_STD_USING(atomic_exchange)
__NAMESPACE_STD_USING(atomic_fetch_add)
__NAMESPACE_STD_USING(atomic_fetch_and)
__NAMESPACE_STD_USING(atomic_fetch_or)
__NAMESPACE_STD_USING(atomic_fetch_sub)
__NAMESPACE_STD_USING(atomic_fetch_xor)
__NAMESPACE_STD_USING(atomic_load)
__NAMESPACE_STD_USING(atomic_store)
#ifndef __atomic_flag_defined
#define __atomic_flag_defined 1
__NAMESPACE_STD_USING(atomic_flag)
#endif /* !__atomic_flag_defined */
__NAMESPACE_STD_USING(atomic_flag_test_and_set_explicit)
__NAMESPACE_STD_USING(atomic_flag_clear_explicit)
__NAMESPACE_STD_USING(atomic_flag_test_and_set)
__NAMESPACE_STD_USING(atomic_flag_clear)

#else /* __cplusplus */
#include <hybrid/__atomic.h>
#include <hybrid/typecore.h>

#define ATOMIC_BOOL_LOCK_FREE     __GCC_ATOMIC_BOOL_LOCK_FREE
#define ATOMIC_CHAR_LOCK_FREE     __GCC_ATOMIC_CHAR_LOCK_FREE
#define ATOMIC_CHAR16_T_LOCK_FREE __GCC_ATOMIC_CHAR16_T_LOCK_FREE
#define ATOMIC_CHAR32_T_LOCK_FREE __GCC_ATOMIC_CHAR32_T_LOCK_FREE
#define ATOMIC_WCHAR_T_LOCK_FREE  __GCC_ATOMIC_WCHAR_T_LOCK_FREE
#define ATOMIC_SHORT_LOCK_FREE    __GCC_ATOMIC_SHORT_LOCK_FREE
#define ATOMIC_INT_LOCK_FREE      __GCC_ATOMIC_INT_LOCK_FREE
#define ATOMIC_LONG_LOCK_FREE     __GCC_ATOMIC_LONG_LOCK_FREE
#define ATOMIC_LLONG_LOCK_FREE    __GCC_ATOMIC_LLONG_LOCK_FREE
#define ATOMIC_POINTER_LOCK_FREE  __GCC_ATOMIC_POINTER_LOCK_FREE

#ifndef __std_memory_order_defined
#define __std_memory_order_defined 1
__NAMESPACE_STD_BEGIN
typedef enum {
    memory_order_relaxed = __ATOMIC_RELAXED,
    memory_order_consume = __ATOMIC_CONSUME,
    memory_order_acquire = __ATOMIC_ACQUIRE,
    memory_order_release = __ATOMIC_RELEASE,
    memory_order_acq_rel = __ATOMIC_ACQ_REL,
    memory_order_seq_cst = __ATOMIC_SEQ_CST
} memory_order;
__NAMESPACE_STD_END
#endif
#ifndef __memory_order_defined
#define __memory_order_defined 1
__NAMESPACE_STD_USING(memory_order_relaxed)
__NAMESPACE_STD_USING(memory_order_consume)
__NAMESPACE_STD_USING(memory_order_acquire)
__NAMESPACE_STD_USING(memory_order_release)
__NAMESPACE_STD_USING(memory_order_acq_rel)
__NAMESPACE_STD_USING(memory_order_seq_cst)
__NAMESPACE_STD_USING(memory_order)
#endif /* !__memory_order_defined */

#define atomic_thread_fence(order) __hybrid_atomic_thread_fence(order)
#define atomic_signal_fence(order) __hybrid_atomic_signal_fence(order)
#define atomic_is_lock_free(obj)   __hybrid_atomic_lockfree(sizeof(*(obj)), obj)

#define ATOMIC_VAR_INIT(value)       (value)
#if __has_extension(c_atomic) || __has_extension(cxx_atomic) /* clang */
#define atomic_init(obj,value)        __c11_atomic_init(obj,value)
#else
#define atomic_init(obj,value)       (void)(*(obj) = (value))
#endif

#if __has_extension(c_atomic) || __has_extension(cxx_atomic)
#define __DEFINE_ATOMIC(T) struct { _Atomic T __val; }
#else
#define __DEFINE_ATOMIC(T) struct { volatile T __val; }
#endif

#ifndef __atomic_types_defined
#define __atomic_types_defined 1
typedef __DEFINE_ATOMIC(__BOOL)                atomic_bool;
typedef __DEFINE_ATOMIC(char)                  atomic_char;
typedef __DEFINE_ATOMIC(signed char)           atomic_schar;
typedef __DEFINE_ATOMIC(unsigned char)         atomic_uchar;
typedef __DEFINE_ATOMIC(short)                 atomic_short;
typedef __DEFINE_ATOMIC(unsigned short)        atomic_ushort;
typedef __DEFINE_ATOMIC(int)                   atomic_int;
typedef __DEFINE_ATOMIC(unsigned int)          atomic_uint;
typedef __DEFINE_ATOMIC(long)                  atomic_long;
typedef __DEFINE_ATOMIC(unsigned long)         atomic_ulong;
#ifdef __COMPILER_HAVE_LONGLONG
typedef __DEFINE_ATOMIC(__LONGLONG)            atomic_llong;
typedef __DEFINE_ATOMIC(__ULONGLONG)           atomic_ullong;
#endif /* __COMPILER_HAVE_LONGLONG */
typedef __DEFINE_ATOMIC(__CHAR16_TYPE__)       atomic_char16_t;
typedef __DEFINE_ATOMIC(__CHAR32_TYPE__)       atomic_char32_t;
typedef __DEFINE_ATOMIC(__WCHAR_TYPE__)        atomic_wchar_t;
typedef __DEFINE_ATOMIC(__INT_LEAST8_TYPE__)   atomic_int_least8_t;
typedef __DEFINE_ATOMIC(__UINT_LEAST8_TYPE__)  atomic_uint_least8_t;
typedef __DEFINE_ATOMIC(__INT_LEAST16_TYPE__)  atomic_int_least16_t;
typedef __DEFINE_ATOMIC(__UINT_LEAST16_TYPE__) atomic_uint_least16_t;
typedef __DEFINE_ATOMIC(__INT_LEAST32_TYPE__)  atomic_int_least32_t;
typedef __DEFINE_ATOMIC(__UINT_LEAST32_TYPE__) atomic_uint_least32_t;
typedef __DEFINE_ATOMIC(__INT_LEAST64_TYPE__)  atomic_int_least64_t;
typedef __DEFINE_ATOMIC(__UINT_LEAST64_TYPE__) atomic_uint_least64_t;
typedef __DEFINE_ATOMIC(__INT_FAST8_TYPE__)    atomic_int_fast8_t;
typedef __DEFINE_ATOMIC(__UINT_FAST8_TYPE__)   atomic_uint_fast8_t;
typedef __DEFINE_ATOMIC(__INT_FAST16_TYPE__)   atomic_int_fast16_t;
typedef __DEFINE_ATOMIC(__UINT_FAST16_TYPE__)  atomic_uint_fast16_t;
typedef __DEFINE_ATOMIC(__INT_FAST32_TYPE__)   atomic_int_fast32_t;
typedef __DEFINE_ATOMIC(__UINT_FAST32_TYPE__)  atomic_uint_fast32_t;
typedef __DEFINE_ATOMIC(__INT_FAST64_TYPE__)   atomic_int_fast64_t;
typedef __DEFINE_ATOMIC(__UINT_FAST64_TYPE__)  atomic_uint_fast64_t;
typedef __DEFINE_ATOMIC(__INTPTR_TYPE__)       atomic_intptr_t;
typedef __DEFINE_ATOMIC(__UINTPTR_TYPE__)      atomic_uintptr_t;
typedef __DEFINE_ATOMIC(__SIZE_TYPE__)         atomic_size_t;
typedef __DEFINE_ATOMIC(__PTRDIFF_TYPE__)      atomic_ptrdiff_t;
typedef __DEFINE_ATOMIC(__INTMAX_TYPE__)       atomic_intmax_t;
typedef __DEFINE_ATOMIC(__UINTMAX_TYPE__)      atomic_uintmax_t;
#endif /* !__atomic_types_defined */


#define atomic_compare_exchange_strong_explicit(object,expected,desired,success,failure) \
    ((*(expected) = __hybrid_atomic_cmpxch_val((object)->__val,*(expected),desired,success,failure)) == (desired))
#define atomic_compare_exchange_weak_explicit(object,expected,desired,success,failure) \
    ((*(expected) = __hybrid_atomic_cmpxch_val_weak((object)->__val,*(expected),desired,success,failure)) == (desired))
#define atomic_exchange_explicit(object,desired,order)  __hybrid_atomic_xch((object)->__val,desired,order)
#define atomic_fetch_add_explicit(object,operand,order) __hybrid_atomic_fetchadd((object)->__val,operand,order)
#define atomic_fetch_and_explicit(object,operand,order) __hybrid_atomic_fetchand((object)->__val,operand,order)
#define atomic_fetch_or_explicit(object,operand,order)  __hybrid_atomic_fetchor((object)->__val,operand,order)
#define atomic_fetch_sub_explicit(object,operand,order) __hybrid_atomic_fetchsub((object)->__val,operand,order)
#define atomic_fetch_xor_explicit(object,operand,order) __hybrid_atomic_fetchxor((object)->__val,operand,order)
#define atomic_load_explicit(object,order)              __hybrid_atomic_load((object)->__val,order)
#define atomic_store_explicit(object,desired,order)     __hybrid_atomic_store((object)->__val,desired,order)

#define atomic_compare_exchange_strong(object,expected,desired) atomic_compare_exchange_strong_explicit(object,expected,desired,memory_order_seq_cst,memory_order_seq_cst)
#define atomic_compare_exchange_weak(object,expected,desired)   atomic_compare_exchange_weak_explicit(object,expected,desired,memory_order_seq_cst,memory_order_seq_cst)
#define atomic_exchange(object,desired)       atomic_exchange_explicit(object,desired,memory_order_seq_cst)
#define atomic_fetch_add(object,operand)      atomic_fetch_add_explicit(object,operand,memory_order_seq_cst)
#define atomic_fetch_and(object,operand)      atomic_fetch_and_explicit(object,operand,memory_order_seq_cst)
#define atomic_fetch_or(object,operand)       atomic_fetch_or_explicit(object,operand,memory_order_seq_cst)
#define atomic_fetch_sub(object,operand)      atomic_fetch_sub_explicit(object,operand,memory_order_seq_cst)
#define atomic_fetch_xor(object,operand)      atomic_fetch_xor_explicit(object,operand,memory_order_seq_cst)
#define atomic_load(object)                   atomic_load_explicit(object,memory_order_seq_cst)
#define atomic_store(object,desired)          atomic_store_explicit(object,desired,memory_order_seq_cst)

#ifndef __atomic_flag_defined
#define __atomic_flag_defined 1
typedef struct { atomic_bool __val; } atomic_flag;
#endif /* !__atomic_flag_defined */
#define ATOMIC_FLAG_INIT                           { ATOMIC_VAR_INIT(0) }
#define atomic_flag_test_and_set_explicit(flg,order) atomic_exchange_explicit(&(flg)->__val,1,order)
#define atomic_flag_clear_explicit(flg,order)        atomic_store_explicit(&(flg)->__val,0,order)
#define atomic_flag_test_and_set(flg)                atomic_flag_test_and_set_explicit(flg,memory_order_seq_cst)
#define atomic_flag_clear(flg)                       atomic_flag_clear_explicit(flg,memory_order_seq_cst)
#endif /* !__cplusplus */

#endif /* !_STDATOMIC_H */
