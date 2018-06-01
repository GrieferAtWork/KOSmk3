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
#ifndef _SYS_GENERIC_MMAN_H
#define _SYS_GENERIC_MMAN_H 1
#define _SYS_MMAN_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>
#include <bits/mman.h>

#ifndef __CRT_GLC
#error "<sys/mman.h> is not supported by the linked libc"
#endif /* !__CRT_GLC */

__SYSDECL_BEGIN

#ifdef __CC__
#ifndef __off_t_defined
#define __off_t_defined 1
typedef __typedef_off_t off_t;
#endif /* !__off_t_defined */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __size_t size_t;
#endif /* !__size_t_defined */

#ifndef __mode_t_defined
#define __mode_t_defined 1
typedef __mode_t mode_t;
#endif /* !__mode_t_defined */
#endif /* __CC__ */

#define PROT_NONE     0x00 /* Data cannot be accessed. */
#define PROT_EXEC     0x01 /* Data can be executed. */
#define PROT_WRITE    0x02 /* Data can be written. */
#define PROT_READ     0x04 /* Data can be read. */
#define PROT_SEM      0x08
#ifdef __USE_KOS
#if __KOS_VERSION__ >= 300
#define PROT_LOOSE    0x10 /* Unmap the region within the when cloning a VM (`CLONE_VM'). */
#else
#define PROT_LOOSE    0x10 /* Unmap the region within the when cloning a VM (`CLONE_VM').
                            * NOTE: Implicitly set for all system-allocated user stacks,
                            *       except for that of the calling thread. */
#endif
#define PROT_SHARED   0x20 /* Changes are shared, even after the VM was cloned (`CLONE_VM').
                            * NOTE: Same as the `MAP_SHARED' map flag, but
                            *       can be set like any other protection. */
#ifdef __KERNEL__
#define PROT_NOUSER   0x40 /* Map memory as inaccessible to user-space.
                            * WARNING: Not fully enforced for addresses within user-memory. */
#if __KOS_VERSION__ < 300
#define PROT_CLEAN    0x80 /* Unset whenever user-space re-maps a page as writable. - Cannot be removed.
                            * NOTE: This flag is used to prevent rootfork() from working
                            *       when called from an otherwise read-only module after
                            *       some portion of the section containing the root-fork
                            *       system call has been mapped as writable.
                            *    >> rootfork() fails when any page in the calling section (.text)
                            *       isn't part of a root-module, isn't fully mapped, was
                            *       re-mapped somewhere else, or been made writable at any point. */
#endif
#endif /* __KERNEL__ */
#endif /* __USE_KOS */

#ifdef __KERNEL__
#define PROT_MASK     0x3f /* Mask of flags accessible from user-space. */
#endif


#ifdef __USE_KOS
#define MAP_AUTOMATIC     0x00000000 /* Use sharing behavior specified by `PROT_SHARED'. */
#endif
#define MAP_SHARED        0x00000001 /* Share changes. */
#define MAP_PRIVATE	      0x00000002 /* Changes are private. */
#define MAP_FIXED         0x00000010 /* Interpret addr exactly. */
#ifdef __USE_MISC
#define MAP_TYPE          0x0000000f /* Mask for type of mapping. */
#define MAP_FILE          0x00000000 /* Do use a file. */
#define MAP_ANONYMOUS     0x00000020 /* Don't use a file. */
#endif
#define MAP_ANON          0x00000020 /* Don't use a file. */
                            
/* Other flags. */
#ifdef __USE_MISC
#define MAP_32BIT         0x00000040 /* Only give out 32-bit addresses. */
#endif

/* These are Linux-specific (But also supported by KOS). */
#ifdef __USE_MISC
#define MAP_GROWSDOWN     0x00000100 /* Stack-like segment. */
#define MAP_GROWSUP       0x00000200 /* Stack-like segment growing upwards. */
#define MAP_DENYWRITE     0x00000800 /* Ignored. */
#define MAP_EXECUTABLE    0x00001000 /* Ignored. */
#define MAP_LOCKED        0x00002000 /* Lock the mapping. */
#define MAP_NORESERVE     0x00004000 /* Don't check for reservations. */
#define MAP_POPULATE      0x00008000 /* Populate (prefault) pagetables. */
#define MAP_NONBLOCK      0x00010000 /* Do not block on IO. */
#define MAP_STACK         0x00020000 /* Allocation is for a stack.
                                      * NOTE: KOS uses this flag to determine where
                                      *       automatic memory mappings are allocated at. */
#define MAP_HUGETLB       0x00040000 /* Create huge page mapping. */
#define MAP_HUGE_SHIFT    26
#define MAP_HUGE_MASK     0x3f
#define MAP_UNINITIALIZED 0x04000000 /* For anonymous mmap, memory could be uninitialized.
                                      * NOTE: Implied for physical mappings.
                                      * NOTE: The kernel may initialize memory randomly in sandboxed threads. */
#endif /* __USE_MISC */


/* Flags for 'mremap'. */
#ifdef __USE_GNU
#   define MREMAP_MAYMOVE 1
#   define MREMAP_FIXED   2
#endif

#ifndef MAP_FAILED
#define MAP_FAILED   ((void *)-1)
#endif /* !MAP_FAILED */

#if !defined(__KERNEL__) && defined(__CC__)
#ifdef __CRT_GLC
__REDIRECT_EXCEPT_FS64(__LIBC,__WUNUSED,void *,__LIBCCALL,mmap,
                      (void *__addr, size_t __len, int __prot, int __flags, __fd_t __fd, __FS_TYPE(off) __offset),
                      (__addr,__len,__prot,__flags,__fd,__offset))
__LIBC int (__LIBCCALL munmap)(void *__addr, size_t __len);
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,mprotect,(void *__addr, size_t __len, int __prot),(__addr,__len,__prot))
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,msync,(void *__addr, size_t __len, int __flags),(__addr,__len,__flags))
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,mlock,(void const *__addr, size_t __len),(__addr,__len))
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,munlock,(void const *__addr, size_t __len),(__addr,__len))
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,mlockall,(int __flags),(__flags))
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,munlockall,(void),())
__REDIRECT_EXCEPT_UFS(__LIBC,,int,__LIBCCALL,shm_open,(char const *__name, __oflag_t __oflag, mode_t __mode),(__name,__oflag,__mode))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,,int,__LIBCCALL,shm_unlink,(char const *__name),(__name))
#ifdef __USE_MISC
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,madvise,(void *__addr, size_t __len, int __advice),(__addr,__len,__advice))
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,mincore,(void *__start, size_t __len, unsigned char *__vec),(__start,__len,__vec))
#endif /* __USE_MISC */
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT(__LIBC,,void *,__LIBCCALL,mmap64,(void *__addr, size_t __len, int __prot, int __flags, __fd_t __fd, __off64_t __offset),(__addr,__len,__prot,__flags,__fd,__offset))
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_XOPEN2K
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,posix_madvise,(void *__addr, size_t __len, int __advice),(__addr,__len,__advice))
#endif /* __USE_XOPEN2K */
#ifdef __USE_GNU
__VREDIRECT_EXCEPT(__LIBC,__WUNUSED_SUGGESTED,void *,__ATTR_CDECL,mremap,(void *__addr, size_t __old_len, size_t __new_len, int __flags, ...),TODO,(__addr,__old_len,__new_len,__flags),__flags)
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,remap_file_pages,(void *__start, size_t __size, int __prot, size_t __pgoff, int __flags),(__start,__size,__prot,__pgoff,__flags))
#endif /* __USE_GNU */
#endif /* __CRT_GLC */
#endif /* !__KERNEL__ && __CC__ */

__SYSDECL_END

#ifdef __USE_EXCEPT
#include <parts/kos3/except/sys/mman.h>
#endif

#ifdef __USE_KOS3
#ifdef _WCHAR_H
#ifndef _PARTS_KOS3_SYS_WMMAN_H
#include <parts/kos3/sys/wmman.h>
#endif
#endif
#ifdef __USE_UTF
#ifdef _UCHAR_H
#ifndef _PARTS_KOS3_SYS_UMMAN_H
#include <parts/kos3/sys/umman.h>
#endif
#endif
#endif /* __USE_UTF */
#endif /* __USE_KOS3 */

#endif /* !_SYS_GENERIC_MMAN_H */
