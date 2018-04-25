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
#ifndef _KOS_DL_H
#define _KOS_DL_H 1

#include <__stdinc.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#define DL_OPEN_FNORMAL      0x0000 /* Normal patching flags. */
#define DL_OPEN_FDEEPBIND    0x0008 /* Perform deep binding, preferring local symbols over global ones. */
#define DL_OPEN_FGLOBAL      0x0100 /* Once relocated, add the application to the chain of active VM-globals. */
#define DL_OPEN_FNOASLR      0x0200 /* Disable address space layout randomization (don't add some random shift mapping relocatable modules). */
#define DL_OPEN_FNOCASE      0x1000 /* Ignore casing while loading dependencies by name. */
#ifdef __KERNEL__
#define DL_OPEN_FDOSRUN      0x2000 /* Interpret paths from `mp_runpath' as dos paths (using `;' as seperator) */
#define DL_OPEN_FDOSALT      0x4000 /* Interpret paths from `mp_altpath' as dos paths (using `;' as seperator)
                                     * NOTE: This flag can be set by user-space, but may interfere with the
                                     *       operation of linker drivers, leading to them failing to find
                                     *       the proper module. */
#else
#define DL_OPEN_FDOSRUN      0x2000 /* Interpret paths from `RUNPATH' as dos paths (using `;' as seperator) */
#endif
#define DL_OPEN_FMASK        0x7308 /* Mask of known DL_OPEN flags. */



#define MODULE_INFO_CLASS_BASIC     0x0000 /* Basic module information. */
#ifdef __CC__
struct module_basic_info {
    /* [MODULE_INFO_CLASS_BASIC] */
    __uintptr_t     mi_loadaddr; /* Module load address (added to image-relative pointers to make
                                  * them absolute; also used for `EXCEPTION_HANDLER_FRELATIVE')
                                  * Additionally, this value is used as mmap-tag when it comes to
                                  * describing which VM nodes are associated with the module.
                                  * In other words, you can specify this value as tag in an
                                  * `xmunmap()' can to ensure that only mappings of this module
                                  * are deleted.
                                  * NOTE: This value may be page-aligned! */
    __uintptr_t     mi_segstart; /* Lowest mapped address associated with the module.
                                  * To force a module to be unmapped (including modules that
                                  * were mapped at the time `exec()' was called), simply do an
                                  * munmap() starting at this address, and randing to `mi_segend'
                                  * HINT: In the current implementation, this is the value that is
                                  *       used to identify module handles, however that may change! */
    __uintptr_t     mi_segend;   /* One past the address of the greatest byte mapped by this module.
                                  * Note that not everything in this range must actually be mapped, as
                                  * well as the fact that this and `mi_segstart' may not be page-aligned. */
};
#endif /* __CC__ */

#define APPLICATION_FNORMAL  0x0000 /* Normal application flags. */
#define APPLICATION_FDIDINIT 0x0001 /* [lock(WRITE_ONCE)] Custom module initializers have been executed. */
#define APPLICATION_FDIDFINI 0x0002 /* [lock(WRITE_ONCE)] Custom module finalizers have been executed. */
#ifdef __KERNEL__
#define APPLICATION_FCLOSING 0x0200 /* [lock(WRITE_ONCE)] A driver is being closed and must not be used to create new global hooks. */
#endif /* __KERNEL__ */
#define APPLICATION_FDYNAMIC 0x4000 /* [const] The module has been opened dynamically and can therefor be closed using `dlclose()'. */
#define APPLICATION_FTRUSTED 0x8000 /* [const] Module's code can be trusted and various security checks can be skipped during parsing of FDE/exception data. */

#define MODULE_INFO_CLASS_STATE  0x0001 /* Module state / reference counting information. */
#ifdef __CC__
struct module_state_info {
    /* [MODULE_INFO_CLASS_STATE] */
    __ref_t         si_mapcnt;   /* Amount of individual segments over which this application is split. */
    __ref_t         si_loadcnt;  /* Amount of times the application has been loaded using `xdlopen()' and friends (recursion counter for `xdlclose()'). */
    __uintptr_t     si_appflags; /* Set of `APPLICATION_F*' */
};
#endif /* __CC__ */


#define MODULE_INFO_CLASS_PATH   0x0002 /* Module path. */
#ifdef __CC__
struct module_path_info {
    /* [MODULE_INFO_CLASS_PATH] */
    unsigned int    pi_format;   /* [IN] The format in which to generate the path (Set of `REALPATH_F*'). */
    __uintptr_t     pi_loadaddr; /* [OUT] The load address of the module (Same as `MODULE_INFO_CLASS_BASIC::mi_loadaddr') */
    char           *pi_path;     /* [OUT] Filled with a NUL-terminated representation of the module's path. */
    size_t          pi_pathlen;  /* [IN|OUT] Updated with the required buffer size. */
};
#endif /* __CC__ */



#ifndef __KERNEL__
#if __KOS_VERSION__ >= 300
/* @param: DFD:        A directory file descriptor used as base path, or the file
 *                     to open when `PATH' is empty and `AT_EMPTY_PATH' is given.
 * @param: PATH:       A path relative to `DFD' that should be opened.
 * @param: AT_FLAGS:   Set of `AT_*'
 * @param: OPEN_FLAGS: Set of `DL_OPEN_F*'
 * @param: RUNPATH:    A `;' or `:' seperated list of paths to search for dependencies
 *                     You may pass `NULL' to use a default search path, or one based
 *                     on the value of the `LD_LIBRARY_PATH' environment variable set
 *                     at the time `exec()' was invoked on the program.
 * @return: * :        A pointer into some mmap()-ed segment associated with the application.
 *                     If the application was already opened, the previous instance will be
 *                     re-returned. This pointer can later be passed to `xdlclose()'
 * @return: NULL:      Failed to open / link the module (see `errno') */
__REDIRECT_EXCEPT_UFS(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY_ALT(dlopen),void *,__LIBCCALL,xfdlopenat,
                     (__fd_t __dfd, char const *__path, __atflag_t __at_flags, int __open_flags, char const *__runpath),
                     (__dfd,__path,__at_flags,__open_flags,__runpath))
#ifdef __USE_EXCEPT
__LIBC __ATTR_RETNONNULL __PORT_KOSONLY
void *(__LIBCCALL Xxfdlopenat)(__fd_t __dfd, char const *__path, __atflag_t __at_flags,
                               int __open_flags, char const *__runpath);
#endif /* __USE_EXCEPT */

/* Query information about a module, given its handle.
 * @param: INFO_CLASS: One of `MODULE_INFO_CLASS_*'
 * @param: BUF:        A pointer to a user-space buffer to-be filled with data.
 * @param: BUFSIZE:    The size of the given buffer.
 * @return: * :        The required buffer size.
 *                     When greater than `BUFSIZE', `BUF' is in an undefined state, and
 *                     the caller should reallocate and repeat the call with a larger buffer
 *                    (or more precisely, with a buffer of at least `RETURN' bytes).
 * @throw: E_SEGFAULT:         The given `HANDLE' is faulty, or the associated application is corrupt.
 * @throw: E_INVALID_ARGUMENT: The given `INFO_CLASS' wasn't recognized.
 */
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xdlmodule_info,
                 (void *__handle, unsigned int __info_class, void *__buf, __size_t __bufsize),
                 (__handle,__info_class,__buf,__bufsize))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __size_t (__LIBCCALL Xxdlmodule_info)(void *__handle, unsigned int __info_class, void *__buf, __size_t __bufsize);
#endif /* __USE_EXCEPT */

/* Operating in the same way as `xfrealpath()', lookup the path of a given module.
 * REMINDER: A module handle is simply any mapped pointer within that module, meaning
 *           you can even pass a function pointer to figure out what module is implementing
 *           that function.
 * @param: BUF:         A buffer to be filled with the name of the path.
 *                      [xdlpath] When NULL, malloc() a new buffer of `BUFSIZE' bytes, or if
 *                               `BUFSIZE' is ZERO(0), allocate a buffer of appropriate size.
 * @param: BUFSIZE:     The size of the provided `BUF' (in characters)
 * @param: TYPE:        The type of path that should be returned. (Set of `REALPATH_F*')
 * @return: * :         [xdlpath] Either `BUF', or the newly allocated buffer when `BUF' was NULL
 * @return: NULL:       [xdlpath] An error occurred (see `errno')
 * @return: * :         [xdlpath2] The required buffer size (in characters), excluding a terminated NUL-character
 * @return: >= BUFSIZE: [xdlpath2] Only a portion of the path was printed. Pass a buffer capable of holding at least `return+1' characters.
 * @return: -1 :        [xdlpath2] An error occurred (see `errno') */
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY __XATTR_RETNONNULL,char *,__LIBCCALL,xdlpath,(void *__handle, char *__buf, __size_t __bufsize, unsigned int __type),(__handle,__buf,__bufsize,__type))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xdlpath2,(void *__handle, char *__buf, __size_t __bufsize, unsigned int __type),(__handle,__buf,__bufsize,__type))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL char *(__LIBCCALL Xxdlpath)(void *__handle, char *__buf, __size_t __bufsize, unsigned int __type);
__LIBC __PORT_KOSONLY __EXCEPT_SELECT(__size_t,__ssize_t) (__LIBCCALL Xxdlpath2)(void *__handle, char *__buf, __size_t __bufsize, unsigned int __type);
#endif /* __USE_EXCEPT */

#endif /* __KOS_VERSION__ >= 300 */

#ifndef __xdlopen_defined
#define __xdlopen_defined 1
#if __KOS_VERSION__ >= 300
__REDIRECT_EXCEPT_UFS(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY_ALT(dlopen),void *,__LIBCCALL,xdlopen,(char const *__filename, int __open_flags),(__filename,__open_flags))
#else
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY_ALT(dlopen),void *,__LIBCCALL,xdlopen,(char const *__filename, int __open_flags),(__filename,__open_flags))
#endif
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY_ALT(dlopen),void *,__LIBCCALL,xfdlopen,(__fd_t __fd, int __open_flags),(__fd,__open_flags))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY_ALT(dlsym),void *,__LIBCCALL,xdlsym,(void *__handle, char const *__symbol),(__handle,__symbol))
__LIBC __PORT_KOSONLY_ALT(dlclose) int (__LIBCCALL xdlclose)(void *__handle);
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL void * (__LIBCCALL Xxdlopen)(char const *__filename, int __flags);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL void * (__LIBCCALL Xxfdlopen)(__fd_t __fd, int __open_flags);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL void *(__LIBCCALL Xxdlsym)(void *__handle, char const *__symbol);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xxdlclose)(void *__handle);
#endif /* __USE_EXCEPT */
#endif /* !__xdlopen_defined */

#endif


__SYSDECL_END

#endif /* !_KOS_DL_H */
