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
#ifndef GUARD_HYBRID_RANDOM_C
#define GUARD_HYBRID_RANDOM_C 1
#define _KOS_SOURCE 2

#include <hybrid/compiler.h>
#include <hybrid/asm.h>
#include <kos/types.h>

#include "hybrid.h"

DECL_BEGIN

PRIVATE u32 rand_seed = 0;
PRIVATE u32 const rand_map[] = {
    0x11e0ebcc,0xa914eba6,0xe400e438,0xa6c4a4df,
    0x0da46171,0x4b9a27d1,0x201910ae,0x95e213cb,
    0xd5ce0943,0x00005fdc,0x0319257d,0x09280b06,
    0x1148c0a6,0x07a24139,0x021214a6,0x03221af8
};


INTERN void LIBCCALL libc_srand(rand_seed_t seed) {
 rand_seed = (u32)seed;
}
INTERN rand_t LIBCCALL libc_rand(void) {
 rand_seed  = (((rand_seed+7) << 1)/3);
 rand_seed ^= rand_map[(rand_seed >> (rand_seed&7)) % COMPILER_LENOF(rand_map)];
#ifdef __KERNEL__
 return (rand_t)rand_seed;
#else
 return (rand_t)(rand_seed & 0x7fffffff);
#endif
}

EXPORT(rand,libc_rand);
EXPORT(srand,libc_srand);

#ifndef CONFIG_LIBC_LIMITED_API
INTERN rand_t LIBCCALL libc_rand_r(rand_seed_r_t *__restrict pseed) {
 u32 seed = (u32)*pseed;
 seed  = (((seed+7) << 1)/3);
 seed ^= rand_map[(seed >> (seed&7)) % COMPILER_LENOF(rand_map)];
 *pseed = (rand_seed_r_t)seed;
#ifdef __KERNEL__
 return (rand_t)seed;
#else
 return (rand_t)(seed & 0x7fffffff);
#endif
}


#if (defined(__KERNEL__) ? __SIZEOF_LONG__ == 4 : __SIZEOF_LONG__ == __SIZEOF_INT__)
DEFINE_INTERN_ALIAS(libc_srandom,libc_srand);
DEFINE_INTERN_ALIAS(libc_random,libc_rand);
#else
INTERN void LIBCCALL libc_srandom(unsigned int seed) { rand_seed = (u32)seed; }
INTERN long LIBCCALL libc_random(void) { return (long)libc_rand(); }
#endif
EXPORT(rand_r,libc_rand_r);
EXPORT(random,libc_random);
EXPORT(srandom,libc_srandom);
#endif /* !CONFIG_LIBC_LIMITED_API */


DECL_END

#endif /* !GUARD_HYBRID_RANDOM_C */
