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
#ifndef GUARD_KERNEL_SRC_KERNEL_IOPORT_C
#define GUARD_KERNEL_SRC_KERNEL_IOPORT_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <dev/devconfig.h>

#ifdef CONFIG_HAVE_IOPORTS
#include <kernel/ioport.h>
#include <hybrid/section.h>
#include <hybrid/align.h>
#include <hybrid/minmax.h>
#include <hybrid/byteswap.h>
#include <hybrid/atomic.h>
#include <assert.h>
#include <except.h>
#include <kos/mask.h>

DECL_BEGIN

#if __SIZEOF_POINTER__ == 4
#define DATAWORD_BITS  32
#define SWAPPTR(x) BSWAP_BE2H32((be32)(x))
#else
#define DATAWORD_BITS  64
#define SWAPPTR(x) BSWAP_BE2H64((be64)(x))
#endif

/* We need an SHL implementation that returns `0' for
 * input values that are larger than 32/64 bits.
 * On X86, the regular << operator will truncate the
 * number of bits by which to shift with mask `31' (or `63'),
 * meaning that `1 << 32' evaluates to 1 (because 1 << (32&1) == 1 << 0 == 1),
 * instead of `0', which we need for the bit masking logic below. */
#if 1
#define FIXED_SHL(x,y) ((y) >= DATAWORD_BITS ? 0 : ((x) << (y)))
#else
#define FIXED_SHL(x,y) ((x) << (y))
#endif


INTERN ATTR_BSS uintptr_t io_usemask[(IO_MAXPORT+1)/sizeof(uintptr_t)];

/* Try to allocate the given I/O address range.
 * NOTE: Can be used to allocate I/O memory that is normally reserved. */
PUBLIC bool KCALL
io_alloc_at(ioport_t base, ioport_t num_ports) {
 ioport_t alloc_base = base;
 assert((base + num_ports) >= base);
 assert(!num_ports || (base + num_ports-1) <= IO_MAXPORT);
 while (num_ports) {
  uintptr_t word_offset,alloc_ports,origword,alloc_mask;
  uintptr_t *iter = &io_usemask[base / DATAWORD_BITS];
  word_offset = (uintptr_t)base & (DATAWORD_BITS-1);
  /* Generate the allocation mask. */
  alloc_ports = MIN(DATAWORD_BITS - word_offset,num_ports);
  alloc_mask  = (FIXED_SHL((uintptr_t)1,alloc_ports)-1) << word_offset;
  alloc_mask  = SWAPPTR(alloc_mask);
  do {
   origword = ATOMIC_READ(*iter);
   if ((origword & alloc_mask) != 0)
        goto nope_release; /* Memory in question has already been allocated. */
  } while (!ATOMIC_CMPXCH(*iter,origword,origword | alloc_mask));
  num_ports -= alloc_ports;
  base     += alloc_ports;
 }
 return true;
nope_release:
 /* Free everything that was already allocated. */
 io_free(alloc_base,base-alloc_base);
 return false;
}

/* Free a previously allocated I/O port range. */
PUBLIC void KCALL
io_free(ioport_t base, ioport_t num_ports) {
 assert((base + num_ports) >= base);
 assert(!num_ports || (base + num_ports-1) <= IO_MAXPORT);
 while (num_ports) {
  uintptr_t word_offset,free_ports,free_mask;
  uintptr_t *iter = &io_usemask[base / DATAWORD_BITS];
  word_offset = (uintptr_t)base & (DATAWORD_BITS-1);
  /* Generate the allocation mask. */
  free_ports = MIN(DATAWORD_BITS - word_offset,num_ports);
  free_mask  = (FIXED_SHL((uintptr_t)1,free_ports)-1) << word_offset;
  free_mask  = SWAPPTR(free_mask);
#ifdef NDEBUG
  ATOMIC_FETCHAND(*iter,~free_mask);
#else
  {
   uintptr_t origword;
   origword = ATOMIC_FETCHAND(*iter,~free_mask);
   assertf((origword & free_mask) == free_mask,
           "I/O Address range was never allocated\n"
           "origword  = %p\n"
           "free_mask = %p\n",
           origword,free_mask);
  }
#endif
  num_ports -= free_ports;
  base      += free_ports;
 }
}



/* Dynamically allocate an I/O address range of `num_ports' consecutive I/O ports.
 * All 1-bits of the returned port can be masked by `mask'.
 * @throw: E_BADALLOC: Insufficient available dynamic I/O ports.
 * @return: * : The first allocated I/O port. */
PUBLIC ioport_t KCALL io_alloc(ioport_t mask, ioport_t num_ports) {
 ioport_t result;
again:
 assert(mask      != 0);
 assert(num_ports != 0);
 if unlikely(mask < IO_ALLOCBASE)
    goto fail;
 /* Figure out where to start allocating ports. */
#if __SIZEOF_IOPORT_T__ == 1
 result = mask_minb(IO_ALLOCBASE,mask);
#elif __SIZEOF_IOPORT_T__ == 2
 result = mask_minw(IO_ALLOCBASE,mask);
#elif __SIZEOF_IOPORT_T__ == 4
 result = mask_minl(IO_ALLOCBASE,mask);
#else
 result = mask_minq(IO_ALLOCBASE,mask);
#endif
 do {
  /* Try to allocate a port range at the given address. */
  if (io_alloc_at(result,num_ports))
      return result;
 }
#if __SIZEOF_IOPORT_T__ == 1
 while (!mask_inc_overflowb(result,mask,&result));
#elif __SIZEOF_IOPORT_T__ == 2
 while (!mask_inc_overfloww(result,mask,&result));
#elif __SIZEOF_IOPORT_T__ == 4
 while (!mask_inc_overflowl(result,mask,&result));
#else
 while (!mask_inc_overflowq(result,mask,&result));
#endif
fail:
 error_throw_resumablef(E_BADALLOC,
                        ERROR_BADALLOC_IOPORT,
                       (size_t)num_ports);
 goto again;
}

DECL_END
#endif /* CONFIG_HAVE_IOPORTS */

#endif /* !GUARD_KERNEL_SRC_KERNEL_IOPORT_C */
