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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_PERTASK_ARITH_H
#define GUARD_KERNEL_INCLUDE_SCHED_PERTASK_ARITH_H 1

#include <hybrid/compiler.h>
#include <sched/pertask.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/pertask-arith.h>
#endif

DECL_BEGIN

#ifdef __CC__

#ifdef __INTELLISENSE__
#undef PERTASK_ADD
#undef PERTASK_SUB
#undef PERTASK_INC
#undef PERTASK_DEC
#undef PERTASK_INV
#undef PERTASK_NEG
#undef PERTASK_SHL
#undef PERTASK_SHR

#define PERTASK_ADD PERTASK_ADD
#define PERTASK_SUB PERTASK_SUB
#define PERTASK_MUL PERTASK_MUL
#define PERTASK_AND PERTASK_AND
#define PERTASK_OR  PERTASK_OR 
#define PERTASK_XOR PERTASK_XOR
#define PERTASK_DIV PERTASK_DIV
#define PERTASK_INC PERTASK_INC
#define PERTASK_DEC PERTASK_DEC
#define PERTASK_INV PERTASK_INV
#define PERTASK_NEG PERTASK_NEG
#define PERTASK_SHL PERTASK_SHL
#define PERTASK_SHR PERTASK_SHR

extern "C++" {
template<class T, class S> FUNDEF void (KCALL PERTASK_ADD)(T &object, S value);
template<class T, class S> FUNDEF void (KCALL PERTASK_SUB)(T &object, S value);
template<class T, class S> FUNDEF void (KCALL PERTASK_MUL)(T &object, S value);
template<class T, class S> FUNDEF void (KCALL PERTASK_DIV)(T &object, S value);
template<class T, class S> FUNDEF void (KCALL PERTASK_AND)(T &object, S value);
template<class T, class S> FUNDEF void (KCALL PERTASK_OR)(T &object, S value);
template<class T, class S> FUNDEF void (KCALL PERTASK_XOR)(T &object, S value);
template<class T> FUNDEF void (KCALL PERTASK_INC)(T &object);
template<class T> FUNDEF void (KCALL PERTASK_DEC)(T &object);
template<class T> FUNDEF void (KCALL PERTASK_INV)(T &object);
template<class T> FUNDEF void (KCALL PERTASK_NEG)(T &object);
template<class T> FUNDEF void (KCALL PERTASK_SHL)(T &object, unsigned int shift);
template<class T> FUNDEF void (KCALL PERTASK_SHR)(T &object, unsigned int shift);
}
#endif

#ifndef PERTASK_ADD
#define PERTASK_ADD(x,v) (void)(PERTASK(x) += (v))
#endif
#ifndef PERTASK_SUB
#define PERTASK_SUB(x,v) (void)(PERTASK(x) -= (v))
#endif
#ifndef PERTASK_MUL
#define PERTASK_MUL(x,v) (void)(PERTASK(x) *= (v))
#endif
#ifndef PERTASK_DIV
#define PERTASK_DIV(x,v) (void)(PERTASK(x) /= (v))
#endif
#ifndef PERTASK_AND
#define PERTASK_AND(x,v) (void)(PERTASK(x) &= (v))
#endif
#ifndef PERTASK_OR
#define PERTASK_OR(x,v)  (void)(PERTASK(x) |= (v))
#endif
#ifndef PERTASK_XOR
#define PERTASK_XOR(x,v) (void)(PERTASK(x) ^= (v))
#endif
#ifndef PERTASK_INC
#define PERTASK_INC(x)   (void)(++PERTASK(x))
#endif
#ifndef PERTASK_DEC
#define PERTASK_DEC(x)   (void)(--PERTASK(x))
#endif
#ifndef PERTASK_INV
#define PERTASK_INV(x)   (void)(PERTASK(x) ^= -1)
#endif
#ifndef PERTASK_NEG
#define PERTASK_NEG(x)   (void)(PERTASK(x) = -PERTASK(x))
#endif
#ifndef PERTASK_SHL
#define PERTASK_SHL(x,y) (void)(PERTASK(x) <<= y)
#endif
#ifndef PERTASK_SHR
#define PERTASK_SHR(x,y) (void)(PERTASK(x) >>= y)
#endif

#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_PERTASK_ARITH_H */
