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
#ifndef GUARD_LIBS_LIBC_EXEC_H
#define GUARD_LIBS_LIBC_EXEC_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     EXEC                                                                              */
/* ===================================================================================== */
INTDEF int ATTR_CDECL libc_execl(char const *path, char const *args, ...);
INTDEF int ATTR_CDECL libc_execle(char const *path, char const *args, ... /*, char *const envp[] */);
INTDEF int ATTR_CDECL libc_execlp(char const *file, char const *args, ...);
INTDEF int ATTR_CDECL libc_execlpe(char const *file, char const *args, ... /*, char *const envp[] */);
INTDEF int ATTR_CDECL libc_dos_execl(char const *path, char const *args, ...);
INTDEF int ATTR_CDECL libc_dos_execle(char const *path, char const *args, ... /*, char *const envp[] */);
INTDEF int ATTR_CDECL libc_dos_execlp(char const *file, char const *args, ...);
INTDEF int ATTR_CDECL libc_dos_execlpe(char const *file, char const *args, ... /*, char *const envp[] */);
INTDEF int LIBCCALL libc_execv(char const *path, char *const argv[]);
INTDEF int LIBCCALL libc_execve(char const *path, char *const argv[], char *const envp[]);
INTDEF int LIBCCALL libc_execvp(char const *file, char *const argv[]);
INTDEF int LIBCCALL libc_execvpe(char const *file, char *const argv[], char *const envp[]);
INTDEF int LIBCCALL libc_dos_execv(char const *path, char *const argv[]);
INTDEF int LIBCCALL libc_dos_execve(char const *path, char *const argv[], char *const envp[]);
INTDEF int LIBCCALL libc_dos_execvp(char const *file, char *const argv[]);
INTDEF int LIBCCALL libc_dos_execvpe(char const *file, char *const argv[], char *const envp[]);
INTDEF int ATTR_CDECL libc_fexecl(int exec_fd, char const *args, ...);
INTDEF int ATTR_CDECL libc_fexecle(int exec_fd, char const *args, ... /*, char *const envp[] */);
INTDEF int LIBCCALL libc_fexecv(int exec_fd, char *const argv[]);
INTDEF int LIBCCALL libc_fexecve(int exec_fd, char *const argv[], char *const envp[]);
INTDEF int ATTR_CDECL libc_fexeclat(fd_t dfd, char const *path, char const *args, ... /*, int flags*/);
INTDEF int ATTR_CDECL libc_fexecleat(fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/);
INTDEF int ATTR_CDECL libc_fexeclpat(char const *file, char const *args, ... /*, int flags*/);
INTDEF int ATTR_CDECL libc_fexeclpeat(char const *file, char const *args, ... /*, char *const envp[], int flags*/);
INTDEF int ATTR_CDECL libc_dos_fexeclat(fd_t dfd, char const *path, char const *args, ... /*, int flags*/);
INTDEF int ATTR_CDECL libc_dos_fexecleat(fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/);
INTDEF int ATTR_CDECL libc_dos_fexeclpat(char const *file, char const *args, ... /*, int flags*/);
INTDEF int ATTR_CDECL libc_dos_fexeclpeat(char const *file, char const *args, ... /*, char *const envp[], int flags*/);
INTDEF int LIBCCALL libc_fexecvat(fd_t dfd, char const *path, char *const argv[], int flags);
INTDEF int LIBCCALL libc_fexecveat(fd_t dfd, char const *path, char *const argv[], char *const envp[], int flags);
INTDEF int LIBCCALL libc_fexecvpat(char const *file, char *const argv[], int flags);
INTDEF int LIBCCALL libc_fexecvpeat(char const *file, char *const argv[], char *const envp[], int flags);
INTDEF int LIBCCALL libc_dos_fexecvat(fd_t dfd, char const *path, char *const argv[], int flags);
INTDEF int LIBCCALL libc_dos_fexecveat(fd_t dfd, char const *path, char *const argv[], char *const envp[], int flags);
INTDEF int LIBCCALL libc_dos_fexecvpat(char const *file, char *const argv[], int flags);
INTDEF int LIBCCALL libc_dos_fexecvpeat(char const *file, char *const argv[], char *const envp[], int flags);
/* DOS's SPAWN functions */
INTDEF pid_t LIBCCALL libc_cwait(int *tstat, pid_t pid, int action);
INTDEF pid_t ATTR_CDECL libc_spawnl(int mode, char const *path, char const *args, ...);
INTDEF pid_t ATTR_CDECL libc_spawnle(int mode, char const *path, char const *args, ... /*, char *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_spawnlp(int mode, char const *file, char const *args, ...);
INTDEF pid_t ATTR_CDECL libc_spawnlpe(int mode, char const *file, char const *args, ... /*, char *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_dos_spawnl(int mode, char const *path, char const *args, ...);
INTDEF pid_t ATTR_CDECL libc_dos_spawnle(int mode, char const *path, char const *args, ... /*, char *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_dos_spawnlp(int mode, char const *file, char const *args, ...);
INTDEF pid_t ATTR_CDECL libc_dos_spawnlpe(int mode, char const *file, char const *args, ... /*, char *const envp[] */);
INTDEF pid_t LIBCCALL libc_spawnv(int mode, char const *path, char *const argv[]);
INTDEF pid_t LIBCCALL libc_spawnve(int mode, char const *path, char *const argv[], char *const envp[]);
INTDEF pid_t LIBCCALL libc_spawnvp(int mode, char const *file, char *const argv[]);
INTDEF pid_t LIBCCALL libc_spawnvpe(int mode, char const *file, char *const argv[], char *const envp[]);
INTDEF pid_t LIBCCALL libc_dos_spawnv(int mode, char const *path, char *const argv[]);
INTDEF pid_t LIBCCALL libc_dos_spawnve(int mode, char const *path, char *const argv[], char *const envp[]);
INTDEF pid_t LIBCCALL libc_dos_spawnvp(int mode, char const *file, char *const argv[]);
INTDEF pid_t LIBCCALL libc_dos_spawnvpe(int mode, char const *file, char *const argv[], char *const envp[]);
INTDEF pid_t ATTR_CDECL libc_fspawnl(int mode, int exec_fd, char const *args, ...);
INTDEF pid_t ATTR_CDECL libc_fspawnle(int mode, int exec_fd, char const *args, ... /*, char *const envp[] */);
INTDEF pid_t LIBCCALL libc_fspawnv(int mode, int exec_fd, char *const argv[]);
INTDEF pid_t LIBCCALL libc_fspawnve(int mode, int exec_fd, char *const argv[], char *const envp[]);
INTDEF pid_t ATTR_CDECL libc_fspawnlat(int mode, fd_t dfd, char const *path, char const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_fspawnleat(int mode, fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_fspawnlpat(int mode, char const *file, char const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_fspawnlpeat(int mode, char const *file, char const *args, ... /*, char *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_dos_fspawnlat(int mode, fd_t dfd, char const *path, char const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_dos_fspawnleat(int mode, fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_dos_fspawnlpat(int mode, char const *file, char const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_dos_fspawnlpeat(int mode, char const *file, char const *args, ... /*, char *const envp[], int flags*/);
INTDEF pid_t LIBCCALL libc_fspawnvat(int mode, fd_t dfd, char const *path, char *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_fspawnveat(int mode, fd_t dfd, char const *path, char *const argv[], char *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_fspawnvpat(int mode, char const *file, char *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_fspawnvpeat(int mode, char const *file, char *const argv[], char *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_dos_fspawnvat(int mode, fd_t dfd, char const *path, char *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_dos_fspawnveat(int mode, fd_t dfd, char const *path, char *const argv[], char *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_dos_fspawnvpat(int mode, char const *file, char *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_dos_fspawnvpeat(int mode, char const *file, char *const argv[], char *const envp[], int flags);
/* EXEC W/ exception support */
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xexecl(char const *path, char const *args, ...);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xexecle(char const *path, char const *args, ... /*, char *const envp[] */);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xexeclp(char const *file, char const *args, ...);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xexeclpe(char const *file, char const *args, ... /*, char *const envp[] */);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xexecv(char const *path, char *const argv[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xexecve(char const *path, char *const argv[], char *const envp[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xexecvp(char const *file, char *const argv[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xexecvpe(char const *file, char *const argv[], char *const envp[]);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xfexecl(int exec_fd, char const *args, ...);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xfexecle(int exec_fd, char const *args, ... /*, char *const envp[] */);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xfexecv(int exec_fd, char *const argv[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xfexecve(int exec_fd, char *const argv[], char *const envp[]);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xfexeclat(fd_t dfd, char const *path, char const *args, ... /*, int flags*/);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xfexecleat(fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xfexeclpat(char const *file, char const *args, ... /*, int flags*/);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xfexeclpeat(char const *file, char const *args, ... /*, char *const envp[], int flags*/);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xfexecvat(fd_t dfd, char const *path, char *const argv[], int flags);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xfexecveat(fd_t dfd, char const *path, char *const argv[], char *const envp[], int flags);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xfexecvpat(char const *file, char *const argv[], int flags);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xfexecvpeat(char const *file, char *const argv[], char *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_Xcwait(int *tstat, pid_t pid, int action);
INTDEF pid_t LIBCCALL libc_Xspawnv(int mode, char const *path, char *const argv[]);
INTDEF pid_t LIBCCALL libc_Xspawnve(int mode, char const *path, char *const argv[], char *const envp[]);
INTDEF pid_t LIBCCALL libc_Xspawnvp(int mode, char const *file, char *const argv[]);
INTDEF pid_t LIBCCALL libc_Xspawnvpe(int mode, char const *file, char *const argv[], char *const envp[]);
INTDEF pid_t LIBCCALL libc_Xfspawnv(int mode, int exec_fd, char *const argv[]);
INTDEF pid_t LIBCCALL libc_Xfspawnve(int mode, int exec_fd, char *const argv[], char *const envp[]);
INTDEF pid_t LIBCCALL libc_Xfspawnvat(int mode, fd_t dfd, char const *path, char *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_Xfspawnvpat(int mode, char const *file, char *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_Xfspawnveat(int mode, fd_t dfd, char const *path, char *const argv[], char *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_Xfspawnvpeat(int mode, char const *file, char *const argv[], char *const envp[], int flags);
INTDEF pid_t ATTR_CDECL libc_Xspawnl(int mode, char const *path, char const *args, ...);
INTDEF pid_t ATTR_CDECL libc_Xspawnle(int mode, char const *path, char const *args, ... /*, char *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_Xspawnlp(int mode, char const *file, char const *args, ...);
INTDEF pid_t ATTR_CDECL libc_Xspawnlpe(int mode, char const *file, char const *args, ... /*, char *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_Xfspawnl(int mode, int exec_fd, char const *args, ...);
INTDEF pid_t ATTR_CDECL libc_Xfspawnle(int mode, int exec_fd, char const *args, ... /*, char *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_Xfspawnlat(int mode, fd_t dfd, char const *path, char const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_Xfspawnleat(int mode, fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_Xfspawnlpat(int mode, char const *file, char const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_Xfspawnlpeat(int mode, char const *file, char const *args, ... /*, char *const envp[], int flags*/);

/* Wide-character enabled exec() functions */
INTDEF int ATTR_CDECL libc_w16execl(char16_t const *path, char16_t const *args, ...);
INTDEF int ATTR_CDECL libc_w32execl(char32_t const *path, char32_t const *args, ...);
INTDEF int ATTR_CDECL libc_w16execle(char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF int ATTR_CDECL libc_w32execle(char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF int ATTR_CDECL libc_w16execlp(char16_t const *file, char16_t const *args, ...);
INTDEF int ATTR_CDECL libc_w32execlp(char32_t const *file, char32_t const *args, ...);
INTDEF int ATTR_CDECL libc_w16execlpe(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF int ATTR_CDECL libc_w32execlpe(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF int ATTR_CDECL libc_dos_w16execl(char16_t const *path, char16_t const *args, ...);
INTDEF int ATTR_CDECL libc_dos_w32execl(char32_t const *path, char32_t const *args, ...);
INTDEF int ATTR_CDECL libc_dos_w16execle(char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF int ATTR_CDECL libc_dos_w32execle(char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF int ATTR_CDECL libc_dos_w16execlp(char16_t const *file, char16_t const *args, ...);
INTDEF int ATTR_CDECL libc_dos_w32execlp(char32_t const *file, char32_t const *args, ...);
INTDEF int ATTR_CDECL libc_dos_w16execlpe(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF int ATTR_CDECL libc_dos_w32execlpe(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF int LIBCCALL libc_w16execv(char16_t const *path, char16_t *const argv[]);
INTDEF int LIBCCALL libc_w32execv(char32_t const *path, char32_t *const argv[]);
INTDEF int LIBCCALL libc_w16execve(char16_t const *path, char16_t *const argv[], char16_t *const envp[]);
INTDEF int LIBCCALL libc_w32execve(char32_t const *path, char32_t *const argv[], char32_t *const envp[]);
INTDEF int LIBCCALL libc_w16execvp(char16_t const *file, char16_t *const argv[]);
INTDEF int LIBCCALL libc_w32execvp(char32_t const *file, char32_t *const argv[]);
INTDEF int LIBCCALL libc_w16execvpe(char16_t const *file, char16_t *const argv[], char16_t *const envp[]);
INTDEF int LIBCCALL libc_w32execvpe(char32_t const *file, char32_t *const argv[], char32_t *const envp[]);
INTDEF int LIBCCALL libc_dos_w16execv(char16_t const *path, char16_t *const argv[]);
INTDEF int LIBCCALL libc_dos_w32execv(char32_t const *path, char32_t *const argv[]);
INTDEF int LIBCCALL libc_dos_w16execve(char16_t const *path, char16_t *const argv[], char16_t *const envp[]);
INTDEF int LIBCCALL libc_dos_w32execve(char32_t const *path, char32_t *const argv[], char32_t *const envp[]);
INTDEF int LIBCCALL libc_dos_w16execvp(char16_t const *file, char16_t *const argv[]);
INTDEF int LIBCCALL libc_dos_w32execvp(char32_t const *file, char32_t *const argv[]);
INTDEF int LIBCCALL libc_dos_w16execvpe(char16_t const *file, char16_t *const argv[], char16_t *const envp[]);
INTDEF int LIBCCALL libc_dos_w32execvpe(char32_t const *file, char32_t *const argv[], char32_t *const envp[]);
INTDEF int ATTR_CDECL libc_w16fexecl(int exec_fd, char16_t const *args, ...);
INTDEF int ATTR_CDECL libc_w32fexecl(int exec_fd, char32_t const *args, ...);
INTDEF int ATTR_CDECL libc_w16fexecle(int exec_fd, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF int ATTR_CDECL libc_w32fexecle(int exec_fd, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF int LIBCCALL libc_w16fexecv(int exec_fd, char16_t *const argv[]);
INTDEF int LIBCCALL libc_w32fexecv(int exec_fd, char32_t *const argv[]);
INTDEF int LIBCCALL libc_w16fexecve(int exec_fd, char16_t *const argv[], char16_t *const envp[]);
INTDEF int LIBCCALL libc_w32fexecve(int exec_fd, char32_t *const argv[], char32_t *const envp[]);
INTDEF int ATTR_CDECL libc_w16fexeclat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/);
INTDEF int ATTR_CDECL libc_w32fexeclat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/);
INTDEF int ATTR_CDECL libc_w16fexecleat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/);
INTDEF int ATTR_CDECL libc_w32fexecleat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/);
INTDEF int ATTR_CDECL libc_w16fexeclpat(char16_t const *file, char16_t const *args, ... /*, int flags*/);
INTDEF int ATTR_CDECL libc_w32fexeclpat(char32_t const *file, char32_t const *args, ... /*, int flags*/);
INTDEF int ATTR_CDECL libc_w16fexeclpeat(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/);
INTDEF int ATTR_CDECL libc_w32fexeclpeat(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/);
INTDEF int ATTR_CDECL libc_dos_w16fexeclat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/);
INTDEF int ATTR_CDECL libc_dos_w32fexeclat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/);
INTDEF int ATTR_CDECL libc_dos_w16fexecleat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/);
INTDEF int ATTR_CDECL libc_dos_w32fexecleat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/);
INTDEF int ATTR_CDECL libc_dos_w16fexeclpat(char16_t const *file, char16_t const *args, ... /*, int flags*/);
INTDEF int ATTR_CDECL libc_dos_w32fexeclpat(char32_t const *file, char32_t const *args, ... /*, int flags*/);
INTDEF int ATTR_CDECL libc_dos_w16fexeclpeat(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/);
INTDEF int ATTR_CDECL libc_dos_w32fexeclpeat(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/);
INTDEF int LIBCCALL libc_w16fexecvat(fd_t dfd, char16_t const *path, char16_t *const argv[], int flags);
INTDEF int LIBCCALL libc_w32fexecvat(fd_t dfd, char32_t const *path, char32_t *const argv[], int flags);
INTDEF int LIBCCALL libc_w16fexecveat(fd_t dfd, char16_t const *path, char16_t *const argv[], char16_t *const envp[], int flags);
INTDEF int LIBCCALL libc_w32fexecveat(fd_t dfd, char32_t const *path, char32_t *const argv[], char32_t *const envp[], int flags);
INTDEF int LIBCCALL libc_w16fexecvpat(char16_t const *file, char16_t *const argv[], int flags);
INTDEF int LIBCCALL libc_w32fexecvpat(char32_t const *file, char32_t *const argv[], int flags);
INTDEF int LIBCCALL libc_w16fexecvpeat(char16_t const *file, char16_t *const argv[], char16_t *const envp[], int flags);
INTDEF int LIBCCALL libc_w32fexecvpeat(char32_t const *file, char32_t *const argv[], char32_t *const envp[], int flags);
INTDEF int LIBCCALL libc_dos_w16fexecvat(fd_t dfd, char16_t const *path, char16_t *const argv[], int flags);
INTDEF int LIBCCALL libc_dos_w32fexecvat(fd_t dfd, char32_t const *path, char32_t *const argv[], int flags);
INTDEF int LIBCCALL libc_dos_w16fexecveat(fd_t dfd, char16_t const *path, char16_t *const argv[], char16_t *const envp[], int flags);
INTDEF int LIBCCALL libc_dos_w32fexecveat(fd_t dfd, char32_t const *path, char32_t *const argv[], char32_t *const envp[], int flags);
INTDEF int LIBCCALL libc_dos_w16fexecvpat(char16_t const *file, char16_t *const argv[], int flags);
INTDEF int LIBCCALL libc_dos_w32fexecvpat(char32_t const *file, char32_t *const argv[], int flags);
INTDEF int LIBCCALL libc_dos_w16fexecvpeat(char16_t const *file, char16_t *const argv[], char16_t *const envp[], int flags);
INTDEF int LIBCCALL libc_dos_w32fexecvpeat(char32_t const *file, char32_t *const argv[], char32_t *const envp[], int flags);

INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw16execl(char16_t const *path, char16_t const *args, ...);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw32execl(char32_t const *path, char32_t const *args, ...);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw16execle(char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw32execle(char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw16execlp(char16_t const *file, char16_t const *args, ...);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw32execlp(char32_t const *file, char32_t const *args, ...);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw16execlpe(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw32execlpe(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw16execv(char16_t const *path, char16_t *const argv[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw32execv(char32_t const *path, char32_t *const argv[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw16execve(char16_t const *path, char16_t *const argv[], char16_t *const envp[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw32execve(char32_t const *path, char32_t *const argv[], char32_t *const envp[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw16execvp(char16_t const *file, char16_t *const argv[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw32execvp(char32_t const *file, char32_t *const argv[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw16execvpe(char16_t const *file, char16_t *const argv[], char16_t *const envp[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw32execvpe(char32_t const *file, char32_t *const argv[], char32_t *const envp[]);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw16fexecl(int exec_fd, char16_t const *args, ...);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw32fexecl(int exec_fd, char32_t const *args, ...);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw16fexecle(int exec_fd, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw32fexecle(int exec_fd, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw16fexecv(int exec_fd, char16_t *const argv[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw32fexecv(int exec_fd, char32_t *const argv[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw16fexecve(int exec_fd, char16_t *const argv[], char16_t *const envp[]);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw32fexecve(int exec_fd, char32_t *const argv[], char32_t *const envp[]);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw16fexeclat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw32fexeclat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw16fexecleat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw32fexecleat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw16fexeclpat(char16_t const *file, char16_t const *args, ... /*, int flags*/);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw32fexeclpat(char32_t const *file, char32_t const *args, ... /*, int flags*/);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw16fexeclpeat(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_Xw32fexeclpeat(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw16fexecvat(fd_t dfd, char16_t const *path, char16_t *const argv[], int flags);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw32fexecvat(fd_t dfd, char32_t const *path, char32_t *const argv[], int flags);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw16fexecveat(fd_t dfd, char16_t const *path, char16_t *const argv[], char16_t *const envp[], int flags);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw32fexecveat(fd_t dfd, char32_t const *path, char32_t *const argv[], char32_t *const envp[], int flags);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw16fexecvpat(char16_t const *file, char16_t *const argv[], int flags);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw32fexecvpat(char32_t const *file, char32_t *const argv[], int flags);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw16fexecvpeat(char16_t const *file, char16_t *const argv[], char16_t *const envp[], int flags);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xw32fexecvpeat(char32_t const *file, char32_t *const argv[], char32_t *const envp[], int flags);

/* Wide-character enabled spawn() functions */
INTDEF pid_t ATTR_CDECL libc_w16spawnl(int mode, char16_t const *path, char16_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_w32spawnl(int mode, char32_t const *path, char32_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_w16spawnle(int mode, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_w32spawnle(int mode, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_w16spawnlp(int mode, char16_t const *file, char16_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_w32spawnlp(int mode, char32_t const *file, char32_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_w16spawnlpe(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_w32spawnlpe(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_dos_w16spawnl(int mode, char16_t const *path, char16_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_dos_w32spawnl(int mode, char32_t const *path, char32_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_dos_w16spawnle(int mode, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_dos_w32spawnle(int mode, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_dos_w16spawnlp(int mode, char16_t const *file, char16_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_dos_w32spawnlp(int mode, char32_t const *file, char32_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_dos_w16spawnlpe(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_dos_w32spawnlpe(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF pid_t LIBCCALL libc_w16spawnv(int mode, char16_t const *path, char16_t *const argv[]);
INTDEF pid_t LIBCCALL libc_w32spawnv(int mode, char32_t const *path, char32_t *const argv[]);
INTDEF pid_t LIBCCALL libc_w16spawnve(int mode, char16_t const *path, char16_t *const argv[], char16_t *const envp[]);
INTDEF pid_t LIBCCALL libc_w32spawnve(int mode, char32_t const *path, char32_t *const argv[], char32_t *const envp[]);
INTDEF pid_t LIBCCALL libc_w16spawnvp(int mode, char16_t const *file, char16_t *const argv[]);
INTDEF pid_t LIBCCALL libc_w32spawnvp(int mode, char32_t const *file, char32_t *const argv[]);
INTDEF pid_t LIBCCALL libc_w16spawnvpe(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[]);
INTDEF pid_t LIBCCALL libc_w32spawnvpe(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[]);
INTDEF pid_t LIBCCALL libc_dos_w16spawnv(int mode, char16_t const *path, char16_t *const argv[]);
INTDEF pid_t LIBCCALL libc_dos_w32spawnv(int mode, char32_t const *path, char32_t *const argv[]);
INTDEF pid_t LIBCCALL libc_dos_w16spawnve(int mode, char16_t const *path, char16_t *const argv[], char16_t *const envp[]);
INTDEF pid_t LIBCCALL libc_dos_w32spawnve(int mode, char32_t const *path, char32_t *const argv[], char32_t *const envp[]);
INTDEF pid_t LIBCCALL libc_dos_w16spawnvp(int mode, char16_t const *file, char16_t *const argv[]);
INTDEF pid_t LIBCCALL libc_dos_w32spawnvp(int mode, char32_t const *file, char32_t *const argv[]);
INTDEF pid_t LIBCCALL libc_dos_w16spawnvpe(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[]);
INTDEF pid_t LIBCCALL libc_dos_w32spawnvpe(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[]);
INTDEF pid_t ATTR_CDECL libc_w16fspawnl(int mode, int exec_fd, char16_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_w32fspawnl(int mode, int exec_fd, char32_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_w16fspawnle(int mode, int exec_fd, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_w32fspawnle(int mode, int exec_fd, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF pid_t LIBCCALL libc_w16fspawnv(int mode, int exec_fd, char16_t *const argv[]);
INTDEF pid_t LIBCCALL libc_w32fspawnv(int mode, int exec_fd, char32_t *const argv[]);
INTDEF pid_t LIBCCALL libc_w16fspawnve(int mode, int exec_fd, char16_t *const argv[], char16_t *const envp[]);
INTDEF pid_t LIBCCALL libc_w32fspawnve(int mode, int exec_fd, char32_t *const argv[], char32_t *const envp[]);
INTDEF pid_t ATTR_CDECL libc_w16fspawnlat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_w32fspawnlat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_w16fspawnleat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_w32fspawnleat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_w16fspawnlpat(int mode, char16_t const *file, char16_t const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_w32fspawnlpat(int mode, char32_t const *file, char32_t const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_w16fspawnlpeat(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_w32fspawnlpeat(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_dos_w16fspawnlat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_dos_w32fspawnlat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_dos_w16fspawnleat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_dos_w32fspawnleat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_dos_w16fspawnlpat(int mode, char16_t const *file, char16_t const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_dos_w32fspawnlpat(int mode, char32_t const *file, char32_t const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_dos_w16fspawnlpeat(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_dos_w32fspawnlpeat(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/);
INTDEF pid_t LIBCCALL libc_w16fspawnvat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_w32fspawnvat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_w16fspawnveat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], char16_t *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_w32fspawnveat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], char32_t *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_w16fspawnvpat(int mode, char16_t const *file, char16_t *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_w32fspawnvpat(int mode, char32_t const *file, char32_t *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_w16fspawnvpeat(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_w32fspawnvpeat(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_dos_w16fspawnvat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_dos_w32fspawnvat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_dos_w16fspawnveat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], char16_t *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_dos_w32fspawnveat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], char32_t *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_dos_w16fspawnvpat(int mode, char16_t const *file, char16_t *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_dos_w32fspawnvpat(int mode, char32_t const *file, char32_t *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_dos_w16fspawnvpeat(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_dos_w32fspawnvpeat(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[], int flags);

INTDEF pid_t ATTR_CDECL libc_Xw16spawnl(int mode, char16_t const *path, char16_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_Xw32spawnl(int mode, char32_t const *path, char32_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_Xw16spawnle(int mode, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_Xw32spawnle(int mode, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_Xw16spawnlp(int mode, char16_t const *file, char16_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_Xw32spawnlp(int mode, char32_t const *file, char32_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_Xw16spawnlpe(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_Xw32spawnlpe(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF pid_t LIBCCALL libc_Xw16spawnv(int mode, char16_t const *path, char16_t *const argv[]);
INTDEF pid_t LIBCCALL libc_Xw32spawnv(int mode, char32_t const *path, char32_t *const argv[]);
INTDEF pid_t LIBCCALL libc_Xw16spawnve(int mode, char16_t const *path, char16_t *const argv[], char16_t *const envp[]);
INTDEF pid_t LIBCCALL libc_Xw32spawnve(int mode, char32_t const *path, char32_t *const argv[], char32_t *const envp[]);
INTDEF pid_t LIBCCALL libc_Xw16spawnvp(int mode, char16_t const *file, char16_t *const argv[]);
INTDEF pid_t LIBCCALL libc_Xw32spawnvp(int mode, char32_t const *file, char32_t *const argv[]);
INTDEF pid_t LIBCCALL libc_Xw16spawnvpe(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[]);
INTDEF pid_t LIBCCALL libc_Xw32spawnvpe(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[]);
INTDEF pid_t ATTR_CDECL libc_Xw16fspawnl(int mode, int exec_fd, char16_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_Xw32fspawnl(int mode, int exec_fd, char32_t const *args, ...);
INTDEF pid_t ATTR_CDECL libc_Xw16fspawnle(int mode, int exec_fd, char16_t const *args, ... /*, char16_t *const envp[] */);
INTDEF pid_t ATTR_CDECL libc_Xw32fspawnle(int mode, int exec_fd, char32_t const *args, ... /*, char32_t *const envp[] */);
INTDEF pid_t LIBCCALL libc_Xw16fspawnv(int mode, int exec_fd, char16_t *const argv[]);
INTDEF pid_t LIBCCALL libc_Xw32fspawnv(int mode, int exec_fd, char32_t *const argv[]);
INTDEF pid_t LIBCCALL libc_Xw16fspawnve(int mode, int exec_fd, char16_t *const argv[], char16_t *const envp[]);
INTDEF pid_t LIBCCALL libc_Xw32fspawnve(int mode, int exec_fd, char32_t *const argv[], char32_t *const envp[]);
INTDEF pid_t ATTR_CDECL libc_Xw16fspawnlat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_Xw32fspawnlat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_Xw16fspawnleat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_Xw32fspawnleat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_Xw16fspawnlpat(int mode, char16_t const *file, char16_t const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_Xw32fspawnlpat(int mode, char32_t const *file, char32_t const *args, ... /*, int flags*/);
INTDEF pid_t ATTR_CDECL libc_Xw16fspawnlpeat(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/);
INTDEF pid_t ATTR_CDECL libc_Xw32fspawnlpeat(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/);
INTDEF pid_t LIBCCALL libc_Xw16fspawnvat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_Xw32fspawnvat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_Xw16fspawnveat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], char16_t *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_Xw32fspawnveat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], char32_t *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_Xw16fspawnvpat(int mode, char16_t const *file, char16_t *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_Xw32fspawnvpat(int mode, char32_t const *file, char32_t *const argv[], int flags);
INTDEF pid_t LIBCCALL libc_Xw16fspawnvpeat(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[], int flags);
INTDEF pid_t LIBCCALL libc_Xw32fspawnvpeat(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[], int flags);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_UNISTD_H */
