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
#ifndef GUARD_KERNEL_SRC_VM_PHYS_C
#define GUARD_KERNEL_SRC_VM_PHYS_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kernel/vm.h>
#include <kernel/debug.h>
#include <sched/task.h>
#include <string.h>
#include <except.h>
#include <assert.h>

DECL_BEGIN

PUBLIC void KCALL
vm_copyfromphys(USER CHECKED void *dst, PHYS vm_phys_t src, size_t num_bytes) {
 vm_vpage_t temp = task_temppage();
 byte_t const *base = (byte_t *)VM_PAGE2ADDR(temp);
 uintptr_t offset;
 while (num_bytes) {
  size_t page_copy;
  vm_acquire(&vm_kernel);
  TRY {
   pagedir_mapone(temp,
                 (vm_ppage_t)VM_ADDR2PAGE(src),
                  PAGEDIR_MAP_FREAD);
  } FINALLY {
   vm_release(&vm_kernel);
  }
  pagedir_syncone(temp);
  offset    = src & (PAGESIZE-1);
  page_copy = PAGESIZE - offset;
  if (page_copy > num_bytes)
      page_copy = num_bytes;
  /* Copy data into user-space. */
  memcpy(dst,base + offset,page_copy);
  src                += page_copy;
  *(uintptr_t *)&dst += page_copy;
  num_bytes          -= page_copy;
 }
}
PUBLIC void KCALL
vm_copytophys(PHYS vm_phys_t dst, USER CHECKED void const *src, size_t num_bytes) {
 vm_vpage_t temp = task_temppage();
 byte_t *base = (byte_t *)VM_PAGE2ADDR(temp);
 uintptr_t offset;
 while (num_bytes) {
  size_t page_copy;
  vm_acquire(&vm_kernel);
  TRY {
   pagedir_mapone(temp,
                 (vm_ppage_t)VM_ADDR2PAGE(dst),
                  PAGEDIR_MAP_FREAD|
                  PAGEDIR_MAP_FWRITE);
  } FINALLY {
   vm_release(&vm_kernel);
  }
  pagedir_syncone(temp);
  offset    = dst & (PAGESIZE-1);
  page_copy = PAGESIZE - offset;
  if (page_copy > num_bytes)
      page_copy = num_bytes;
  /* Copy data into user-space. */
  memcpy(base + offset,src,page_copy);
  dst                += page_copy;
  *(uintptr_t *)&src += page_copy;
  num_bytes          -= page_copy;
 }
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_VM_PHYS_C */
