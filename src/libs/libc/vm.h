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
#ifndef GUARD_LIBS_LIBC_VM_H
#define GUARD_LIBS_LIBC_VM_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     VM                                                                                */
/* ===================================================================================== */
struct mmap_info_v1;
INTDEF void *LIBCCALL libc_mmap(void *addr, size_t len, int prot, int flags, fd_t fd, pos32_t offset);
INTDEF void *LIBCCALL libc_mmap64(void *addr, size_t len, int prot, int flags, fd_t fd, pos64_t offset);
INTDEF void *ATTR_CDECL libc_mremap(void *addr, size_t old_len, size_t new_len, int flags, ...);
INTDEF int LIBCCALL libc_munmap(void *addr, size_t len);
INTDEF int LIBCCALL libc_mprotect(void *addr, size_t len, int prot);
INTDEF int LIBCCALL libc_msync(void *addr, size_t len, int flags);
INTDEF int LIBCCALL libc_mlock(void const *addr, size_t len);
INTDEF int LIBCCALL libc_munlock(void const *addr, size_t len);
INTDEF int LIBCCALL libc_mlockall(int flags);
INTDEF int LIBCCALL libc_munlockall(void);
INTDEF int LIBCCALL libc_shm_open(char const *name, oflag_t oflag, mode_t mode);
INTDEF int LIBCCALL libc_dos_shm_open(char const *name, oflag_t oflag, mode_t mode);
INTDEF int LIBCCALL libc_shm_unlink(char const *name);
INTDEF int LIBCCALL libc_dos_shm_unlink(char const *name);
INTDEF int LIBCCALL libc_madvise(void *addr, size_t len, int advice);
INTDEF int LIBCCALL libc_mincore(void *start, size_t len, unsigned char *vec);
INTDEF int LIBCCALL libc_posix_madvise(void *addr, size_t len, int advice);
INTDEF int LIBCCALL libc_remap_file_pages(void *start, size_t size, int prot, size_t pgoff, int flags);
INTDEF void *LIBCCALL libc_xmmap1(struct mmap_info_v1 const *data);
INTDEF ssize_t LIBCCALL libc_xmunmap(void *addr, size_t len, int flags, void *tag);
INTDEF ssize_t LIBCCALL libc_xmprotect(void *start, size_t len, int protmask, int protflag, int flags, void *tag);
INTDEF int LIBCCALL libc_brk(void *addr);
INTDEF void *LIBCCALL libc_sbrk(intptr_t delta);
INTDEF int LIBCCALL libc_swapon(char const *specialfile, int flags);
INTDEF int LIBCCALL libc_swapoff(char const *specialfile);
INTDEF void *LIBCCALL libc_Xmmap(void *addr, size_t len, int prot, int flags, fd_t fd, pos32_t offset);
INTDEF void *LIBCCALL libc_Xmmap64(void *addr, size_t len, int prot, int flags, fd_t fd, pos64_t offset);
INTDEF void *ATTR_CDECL libc_Xmremap(void *addr, size_t old_len, size_t new_len, int flags, ...);
INTDEF void LIBCCALL libc_Xmunmap(void *addr, size_t len);
INTDEF void LIBCCALL libc_Xmprotect(void *addr, size_t len, int prot);
INTDEF void LIBCCALL libc_Xmsync(void *addr, size_t len, int flags);
INTDEF void LIBCCALL libc_Xmlock(void const *addr, size_t len);
INTDEF void LIBCCALL libc_Xmunlock(void const *addr, size_t len);
INTDEF void LIBCCALL libc_Xmlockall(int flags);
INTDEF void LIBCCALL libc_Xmunlockall(void);
INTDEF int LIBCCALL libc_Xshm_open(char const *name, oflag_t oflag, mode_t mode);
INTDEF void LIBCCALL libc_Xshm_unlink(char const *name);
INTDEF void LIBCCALL libc_Xmadvise(void *addr, size_t len, int advice);
INTDEF void LIBCCALL libc_Xmincore(void *start, size_t len, unsigned char *vec);
INTDEF void LIBCCALL libc_Xposix_madvise(void *addr, size_t len, int advice);
INTDEF void LIBCCALL libc_Xremap_file_pages(void *start, size_t size, int prot, size_t pgoff, int flags);
INTDEF void *LIBCCALL libc_Xxmmap1(struct mmap_info_v1 const *data);
INTDEF size_t LIBCCALL libc_Xxmunmap(void *addr, size_t len, int flags, void *tag);
INTDEF size_t LIBCCALL libc_Xxmprotect(void *start, size_t len, int protmask, int protflag, int flags, void *tag);
INTDEF void LIBCCALL libc_Xbrk(void *addr);
INTDEF void *LIBCCALL libc_Xsbrk(intptr_t delta);
INTDEF void LIBCCALL libc_Xswapon(char const *specialfile, int flags);
INTDEF void LIBCCALL libc_Xswapoff(char const *specialfile);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_VM_H */
