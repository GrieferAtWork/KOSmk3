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
#ifndef _KOS_MASK_H
#define _KOS_MASK_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

/* Increment a counter `X' by one, only increasing
 * and carrying bits that are also 1 in `MASK'.
 * The incremented value is then returned.
 * NOTE: 1-bits in `X' that are not set in `MASK' are
 *       ignored and will appear as 0 in the return value.
 * @assume(POST(RETURN > (X & MASK) || RETURN == 0))
 * @assume(POST((RETURN & ~MASK) == 0))
 */
__LOCAL __UINT8_TYPE__ (__LIBCCALL mask_incb)(__UINT8_TYPE__ __x, __UINT8_TYPE__ __mask);
__LOCAL __UINT16_TYPE__ (__LIBCCALL mask_incw)(__UINT16_TYPE__ __x, __UINT16_TYPE__ __mask);
__LOCAL __UINT32_TYPE__ (__LIBCCALL mask_incl)(__UINT32_TYPE__ __x, __UINT32_TYPE__ __mask);
__LOCAL __UINT64_TYPE__ (__LIBCCALL mask_incq)(__UINT64_TYPE__ __x, __UINT64_TYPE__ __mask);

/* Same as the functions above, but return `false' if no
 * overflow happened, or `true' if an overflow did happen.
 * In other words, return the final carry. */
__LOCAL __BOOL (__LIBCCALL mask_inc_overflowb)(__UINT8_TYPE__ __x, __UINT8_TYPE__ __mask, __UINT8_TYPE__ *__restrict __presult);
__LOCAL __BOOL (__LIBCCALL mask_inc_overfloww)(__UINT16_TYPE__ __x, __UINT16_TYPE__ __mask, __UINT16_TYPE__ *__restrict __presult);
__LOCAL __BOOL (__LIBCCALL mask_inc_overflowl)(__UINT32_TYPE__ __x, __UINT32_TYPE__ __mask, __UINT32_TYPE__ *__restrict __presult);
__LOCAL __BOOL (__LIBCCALL mask_inc_overflowq)(__UINT64_TYPE__ __x, __UINT64_TYPE__ __mask, __UINT64_TYPE__ *__restrict __presult);


/* Return the smallest value greater than, or equal to `X' that is fully masked by `MASK'.
 * @assume(PRE(X >= MASK))
 * @assume(POST((RETURN & ~MASK) == 0)) */
__LOCAL __UINT8_TYPE__ (__LIBCCALL mask_minb)(__UINT8_TYPE__ __x, __UINT8_TYPE__ __mask);
__LOCAL __UINT16_TYPE__ (__LIBCCALL mask_minw)(__UINT16_TYPE__ __x, __UINT16_TYPE__ __mask);
__LOCAL __UINT32_TYPE__ (__LIBCCALL mask_minl)(__UINT32_TYPE__ __x, __UINT32_TYPE__ __mask);
__LOCAL __UINT64_TYPE__ (__LIBCCALL mask_minq)(__UINT64_TYPE__ __x, __UINT64_TYPE__ __mask);


#ifndef __INTELLISENSE__
#define __DEFINE_MASK_INC(name,T) \
__LOCAL T \
(__LIBCCALL name)(T __x, T __mask) { \
 T __result = 0; \
 T __bitmask = 1; \
 T __valuemask; \
 T __carry = 1; /* `1', because we're doing an inc() */ \
 do if (__mask & __bitmask) { \
  __valuemask = __x & __bitmask; \
  if (__carry) { \
   /* 0 + 1 -> 01 */ \
   /* 1 + 1 -> 10 */ \
   if (!__valuemask) \
        __result |= __bitmask; \
   __carry = __valuemask; \
  } else { \
   /* 0 + 0 -> 00 */ \
   /* 1 + 0 -> 01 */ \
   __result |= __valuemask; \
  } \
 } while ((__bitmask <<= 1) != 0); \
 return __result; \
}

__DEFINE_MASK_INC(mask_incb,__UINT8_TYPE__)
__DEFINE_MASK_INC(mask_incw,__UINT16_TYPE__)
__DEFINE_MASK_INC(mask_incl,__UINT32_TYPE__)
__DEFINE_MASK_INC(mask_incq,__UINT64_TYPE__)
#undef __DEFINE_MASK_INC

#define __DEFINE_MASK_INC_OVERFLOW(name,T) \
__LOCAL __BOOL \
(__LIBCCALL name)(T __x, T __mask, T *__restrict __presult) { \
 T __result = 0; \
 T __bitmask = 1; \
 T __valuemask; \
 T __carry = 1; /* `1', because we're doing an inc() */ \
 do if (__mask & __bitmask) { \
  __valuemask = __x & __bitmask; \
  if (__carry) { \
   /* 0 + 1 -> 01 */ \
   /* 1 + 1 -> 10 */ \
   if (!__valuemask) \
        __result |= __bitmask; \
   __carry = __valuemask; \
  } else { \
   /* 0 + 0 -> 00 */ \
   /* 1 + 0 -> 01 */ \
   __result |= __valuemask; \
  } \
 } while ((__bitmask <<= 1) != 0); \
 *__presult = __result; \
 return !!__carry; \
}

__DEFINE_MASK_INC_OVERFLOW(mask_inc_overflowb,__UINT8_TYPE__)
__DEFINE_MASK_INC_OVERFLOW(mask_inc_overfloww,__UINT16_TYPE__)
__DEFINE_MASK_INC_OVERFLOW(mask_inc_overflowl,__UINT32_TYPE__)
__DEFINE_MASK_INC_OVERFLOW(mask_inc_overflowq,__UINT64_TYPE__)
#undef __DEFINE_MASK_INC_OVERFLOW

__LOCAL __BOOL (__LIBCCALL mask_inc_overflowb)(__UINT8_TYPE__ __x, __UINT8_TYPE__ __mask, __UINT8_TYPE__ *__restrict __presult);
__LOCAL __BOOL (__LIBCCALL mask_inc_overfloww)(__UINT16_TYPE__ __x, __UINT16_TYPE__ __mask, __UINT16_TYPE__ *__restrict __presult);
__LOCAL __BOOL (__LIBCCALL mask_inc_overflowl)(__UINT32_TYPE__ __x, __UINT32_TYPE__ __mask, __UINT32_TYPE__ *__restrict __presult);
__LOCAL __BOOL (__LIBCCALL mask_inc_overflowq)(__UINT64_TYPE__ __x, __UINT64_TYPE__ __mask, __UINT64_TYPE__ *__restrict __presult);


#define __DEFINE_MASK_MIN(name,T) \
__LOCAL T \
(__LIBCCALL name)(T __x, T __mask) { \
 /* Merge all 1-bits with low significance with more significant bits \
  * until the first 1-bit in `MASK' is encountered. Then repeat the \
  * process until all 1-bits in `X' are masked by `MASK' */ \
 T __bitmask = 1; \
 T __carry = 0; \
 do { \
  /* Advance carry. */ \
  __carry <<= 1; \
  if (!(__mask & __bitmask)) { \
   /* Non-Masked bit -> merge carry & delete source bit. */ \
   __carry  |= __x & __bitmask; \
   __x      &= ~__bitmask; \
  } else { \
   /* Masked bit -> Copy carry into the source word. */ \
   __x      |= __carry; \
   __carry   = 0; \
  } \
 } while ((__bitmask <<= 1) != 0); \
 return __x; \
}
__DEFINE_MASK_MIN(mask_minb,__UINT8_TYPE__)
__DEFINE_MASK_MIN(mask_minw,__UINT16_TYPE__)
__DEFINE_MASK_MIN(mask_minl,__UINT32_TYPE__)
__DEFINE_MASK_MIN(mask_minq,__UINT64_TYPE__)
#undef __DEFINE_MASK_MIN
#endif




__SYSDECL_END

#endif /* !_KOS_MASK_H */
