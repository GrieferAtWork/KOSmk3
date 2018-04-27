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
#ifndef GUARD_LIBS_LIBC_EXEC_C
#define GUARD_LIBS_LIBC_EXEC_C 1
#define _GNU_SOURCE 1

#include "libc.h"
#include "exec.h"
#include "system.h"
#include "errno.h"
#include "environ.h"
#include "widechar.h"
#include "sched.h"
#include <errno.h>

DECL_BEGIN

INTERN int LIBCCALL libc_execv(char const *path, char *const argv[]) { return libc_execve(path,argv,environ); }
INTERN int LIBCCALL libc_execve(char const *path, char *const argv[], char *const envp[]) { return SET_SYSTEM_ERROR(sys_execve(path,argv,envp)); }
INTERN int LIBCCALL libc_execvp(char const *file, char *const argv[]) { return libc_fexecvpeat(file,argv,environ,0); }
INTERN int LIBCCALL libc_execvpe(char const *file, char *const argv[], char *const envp[]) { return libc_fexecvpeat(file,argv,envp,0); }
CRT_DOS int LIBCCALL libc_dos_execv(char const *path, char *const argv[]) { return libc_fexecveat(AT_FDCWD,path,argv,environ,AT_DOSPATH); }
CRT_DOS int LIBCCALL libc_dos_execve(char const *path, char *const argv[], char *const envp[]) { return libc_fexecveat(AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_DOS int LIBCCALL libc_dos_execvp(char const *file, char *const argv[]) { return libc_fexecvpeat(file,argv,environ,AT_DOSPATH); }
CRT_DOS int LIBCCALL libc_dos_execvpe(char const *file, char *const argv[], char *const envp[]) { return libc_fexecvpeat(file,argv,envp,AT_DOSPATH); }
INTERN int LIBCCALL libc_fexecv(int exec_fd, char *const argv[]) { return libc_fexecveat(exec_fd,libc_empty_string,argv,environ,AT_EMPTY_PATH); }
INTERN int LIBCCALL libc_fexecve(int exec_fd, char *const argv[], char *const envp[]) { return libc_fexecveat(exec_fd,libc_empty_string,argv,envp,AT_EMPTY_PATH); }
INTERN int LIBCCALL libc_fexecvat(fd_t dfd, char const *path, char *const argv[], int flags) { return libc_fexecveat(dfd,path,argv,environ,flags); }
INTERN int LIBCCALL libc_fexecveat(fd_t dfd, char const *path, char *const argv[], char *const envp[], int flags) { return SET_SYSTEM_ERROR(sys_execveat(dfd,path,argv,envp,flags)); }
INTERN int LIBCCALL libc_fexecvpat(char const *file, char *const argv[], int flags) { return libc_fexecvpeat(file,argv,environ,flags); }
CRT_DOS_EXT int LIBCCALL libc_dos_fexecvat(fd_t dfd, char const *path, char *const argv[], int flags) { return libc_fexecveat(dfd,path,argv,environ,flags|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fexecveat(fd_t dfd, char const *path, char *const argv[], char *const envp[], int flags) { return libc_fexecveat(dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fexecvpat(char const *file, char *const argv[], int flags) { return libc_fexecvpeat(file,argv,environ,flags|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fexecvpeat(char const *file, char *const argv[], char *const envp[], int flags) { return libc_fexecvpeat(file,argv,envp,flags|AT_DOSPATH); }
INTERN int LIBCCALL
libc_fexecvpeat(char const *file,
                char *const argv[],
                char *const envp[],
                int flags) {
 /* TODO: Search $PATH */
 libc_seterrno(ENOSYS);
 return -1;
}


CRT_DOS pid_t LIBCCALL libc_spawnv(int mode, char const *path, char *const argv[]) { return libc_fspawnveat(mode,AT_FDCWD,path,argv,environ,0); }
CRT_DOS pid_t LIBCCALL libc_spawnve(int mode, char const *path, char *const argv[], char *const envp[]) { return libc_fspawnveat(mode,AT_FDCWD,path,argv,envp,0); }
CRT_DOS pid_t LIBCCALL libc_spawnvp(int mode, char const *file, char *const argv[]) { return libc_fspawnvpeat(mode,file,argv,environ,0); }
CRT_DOS pid_t LIBCCALL libc_spawnvpe(int mode, char const *file, char *const argv[], char *const envp[]) { return libc_fspawnvpeat(mode,file,argv,envp,0); }
CRT_DOS pid_t LIBCCALL libc_dos_spawnv(int mode, char const *path, char *const argv[]) { return libc_fspawnveat(mode,AT_FDCWD,path,argv,environ,AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_dos_spawnve(int mode, char const *path, char *const argv[], char *const envp[]) { return libc_fspawnveat(mode,AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_dos_spawnvp(int mode, char const *file, char *const argv[]) { return libc_fspawnvpeat(mode,file,argv,environ,AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_dos_spawnvpe(int mode, char const *file, char *const argv[], char *const envp[]) { return libc_fspawnvpeat(mode,file,argv,envp,AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_fspawnv(int mode, int exec_fd, char *const argv[]) { return libc_fspawnveat(mode,exec_fd,libc_empty_string,argv,environ,AT_EMPTY_PATH); }
CRT_DOS pid_t LIBCCALL libc_fspawnve(int mode, int exec_fd, char *const argv[], char *const envp[]) { return libc_fspawnveat(mode,exec_fd,libc_empty_string,argv,envp,AT_EMPTY_PATH); }
CRT_DOS pid_t LIBCCALL libc_fspawnvat(int mode, fd_t dfd, char const *path, char *const argv[], int flags) { return libc_fspawnveat(mode,dfd,path,argv,environ,flags); }
CRT_DOS pid_t LIBCCALL libc_fspawnvpat(int mode, char const *file, char *const argv[], int flags) { return libc_fspawnvpeat(mode,file,argv,environ,flags); }
CRT_DOS pid_t LIBCCALL libc_dos_fspawnvat(int mode, fd_t dfd, char const *path, char *const argv[], int flags) { return libc_fspawnveat(mode,dfd,path,argv,environ,flags|AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_dos_fspawnveat(int mode, fd_t dfd, char const *path, char *const argv[], char *const envp[], int flags) { return libc_fspawnveat(mode,dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_dos_fspawnvpat(int mode, char const *file, char *const argv[], int flags) { return libc_fspawnvpeat(mode,file,argv,environ,flags|AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_dos_fspawnvpeat(int mode, char const *file, char *const argv[], char *const envp[], int flags) { return libc_fspawnvpeat(mode,file,argv,envp,flags|AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL
libc_fspawnveat(int mode, fd_t dfd,
                char const *path,
                char *const argv[],
                char *const envp[],
                int flags) {
 /* TODO */
 libc_seterrno(ENOSYS);
 return -1;
}
CRT_DOS pid_t LIBCCALL
libc_fspawnvpeat(int mode,
                 char const *file,
                 char *const argv[],
                 char *const envp[],
                 int flags) {
 /* TODO: Search $PATH */
 libc_seterrno(ENOSYS);
 return -1;
}

CRT_WIDECHAR int LIBCCALL libc_w16execv(char16_t const *path, char16_t *const argv[]) { return libc_w16fexecveat(AT_FDCWD,path,argv,libc_get_w16environ(),0); }
CRT_WIDECHAR int LIBCCALL libc_w32execv(char32_t const *path, char32_t *const argv[]) { return libc_w32fexecveat(AT_FDCWD,path,argv,libc_get_w32environ(),0); }
CRT_WIDECHAR int LIBCCALL libc_w16execve(char16_t const *path, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fexecveat(AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR int LIBCCALL libc_w32execve(char32_t const *path, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fexecveat(AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR int LIBCCALL libc_w16execvp(char16_t const *file, char16_t *const argv[]) { return libc_w16fexecvpeat(file,argv,libc_get_w16environ(),0); }
CRT_WIDECHAR int LIBCCALL libc_w32execvp(char32_t const *file, char32_t *const argv[]) { return libc_w32fexecvpeat(file,argv,libc_get_w32environ(),0); }
CRT_WIDECHAR int LIBCCALL libc_w16execvpe(char16_t const *file, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fexecvpeat(file,argv,envp,0); }
CRT_WIDECHAR int LIBCCALL libc_w32execvpe(char32_t const *file, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fexecvpeat(file,argv,envp,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16execv(char16_t const *path, char16_t *const argv[]) { return libc_w16fexecveat(AT_FDCWD,path,argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32execv(char32_t const *path, char32_t *const argv[]) { return libc_w32fexecveat(AT_FDCWD,path,argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16execve(char16_t const *path, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fexecveat(AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32execve(char32_t const *path, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fexecveat(AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16execvp(char16_t const *file, char16_t *const argv[]) { return libc_w16fexecvpeat(file,argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32execvp(char32_t const *file, char32_t *const argv[]) { return libc_w32fexecvpeat(file,argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16execvpe(char16_t const *file, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fexecvpeat(file,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32execvpe(char32_t const *file, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fexecvpeat(file,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_w16fexecv(int exec_fd, char16_t *const argv[]) { return libc_w16fexecveat(exec_fd,libc_empty_string16,argv,libc_get_w16environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR int LIBCCALL libc_w32fexecv(int exec_fd, char32_t *const argv[]) { return libc_w32fexecveat(exec_fd,libc_empty_string32,argv,libc_get_w32environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR int LIBCCALL libc_w16fexecve(int exec_fd, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fexecveat(exec_fd,libc_empty_string16,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR int LIBCCALL libc_w32fexecve(int exec_fd, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fexecveat(exec_fd,libc_empty_string32,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR int LIBCCALL libc_w16fexecvat(fd_t dfd, char16_t const *path, char16_t *const argv[], int flags) { return libc_w16fexecveat(dfd,path,argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR int LIBCCALL libc_w32fexecvat(fd_t dfd, char32_t const *path, char32_t *const argv[], int flags) { return libc_w32fexecveat(dfd,path,argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR int LIBCCALL libc_w16fexecvpat(char16_t const *file, char16_t *const argv[], int flags) { return libc_w16fexecvpeat(file,argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR int LIBCCALL libc_w32fexecvpat(char32_t const *file, char32_t *const argv[], int flags) { return libc_w32fexecvpeat(file,argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16fexecvat(fd_t dfd, char16_t const *path, char16_t *const argv[], int flags) { return libc_w16fexecveat(dfd,path,argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32fexecvat(fd_t dfd, char32_t const *path, char32_t *const argv[], int flags) { return libc_w32fexecveat(dfd,path,argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16fexecveat(fd_t dfd, char16_t const *path, char16_t *const argv[], char16_t *const envp[], int flags) { return libc_w16fexecveat(dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32fexecveat(fd_t dfd, char32_t const *path, char32_t *const argv[], char32_t *const envp[], int flags) { return libc_w32fexecveat(dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16fexecvpat(char16_t const *file, char16_t *const argv[], int flags) { return libc_w16fexecvpeat(file,argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32fexecvpat(char32_t const *file, char32_t *const argv[], int flags) { return libc_w32fexecvpeat(file,argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16fexecvpeat(char16_t const *file, char16_t *const argv[], char16_t *const envp[], int flags) { return libc_w16fexecvpeat(file,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32fexecvpeat(char32_t const *file, char32_t *const argv[], char32_t *const envp[], int flags) { return libc_w32fexecvpeat(file,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16spawnv(int mode, char16_t const *path, char16_t *const argv[]) { return libc_w16fspawnveat(mode,AT_FDCWD,path,argv,libc_get_w16environ(),0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32spawnv(int mode, char32_t const *path, char32_t *const argv[]) { return libc_w32fspawnveat(mode,AT_FDCWD,path,argv,libc_get_w32environ(),0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16spawnve(int mode, char16_t const *path, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fspawnveat(mode,AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32spawnve(int mode, char32_t const *path, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fspawnveat(mode,AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16spawnvp(int mode, char16_t const *file, char16_t *const argv[]) { return libc_w16fspawnvpeat(mode,file,argv,libc_get_w16environ(),0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32spawnvp(int mode, char32_t const *file, char32_t *const argv[]) { return libc_w32fspawnvpeat(mode,file,argv,libc_get_w32environ(),0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16spawnvpe(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fspawnvpeat(mode,file,argv,envp,0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32spawnvpe(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fspawnvpeat(mode,file,argv,envp,0); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16spawnv(int mode, char16_t const *path, char16_t *const argv[]) { return libc_w16fspawnveat(mode,AT_FDCWD,path,argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32spawnv(int mode, char32_t const *path, char32_t *const argv[]) { return libc_w32fspawnveat(mode,AT_FDCWD,path,argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16spawnve(int mode, char16_t const *path, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fspawnveat(mode,AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32spawnve(int mode, char32_t const *path, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fspawnveat(mode,AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16spawnvp(int mode, char16_t const *file, char16_t *const argv[]) { return libc_w16fspawnvpeat(mode,file,argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32spawnvp(int mode, char32_t const *file, char32_t *const argv[]) { return libc_w32fspawnvpeat(mode,file,argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16spawnvpe(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fspawnvpeat(mode,file,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32spawnvpe(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fspawnvpeat(mode,file,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16fspawnv(int mode, int exec_fd, char16_t *const argv[]) { return libc_w16fspawnveat(mode,exec_fd,libc_empty_string16,argv,libc_get_w16environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32fspawnv(int mode, int exec_fd, char32_t *const argv[]) { return libc_w32fspawnveat(mode,exec_fd,libc_empty_string32,argv,libc_get_w32environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16fspawnve(int mode, int exec_fd, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fspawnveat(mode,exec_fd,libc_empty_string16,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32fspawnve(int mode, int exec_fd, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fspawnveat(mode,exec_fd,libc_empty_string32,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16fspawnvat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], int flags) { return libc_w16fspawnveat(mode,dfd,path,argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32fspawnvat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], int flags) { return libc_w32fspawnveat(mode,dfd,path,argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16fspawnvpat(int mode, char16_t const *file, char16_t *const argv[], int flags) { return libc_w16fspawnvpeat(mode,file,argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32fspawnvpat(int mode, char32_t const *file, char32_t *const argv[], int flags) { return libc_w32fspawnvpeat(mode,file,argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16fspawnvat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], int flags) { return libc_w16fspawnveat(mode,dfd,path,argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32fspawnvat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], int flags) { return libc_w32fspawnveat(mode,dfd,path,argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16fspawnveat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], char16_t *const envp[], int flags) { return libc_w16fspawnveat(mode,dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32fspawnveat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], char32_t *const envp[], int flags) { return libc_w32fspawnveat(mode,dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16fspawnvpat(int mode, char16_t const *file, char16_t *const argv[], int flags) { return libc_w16fspawnvpeat(mode,file,argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32fspawnvpat(int mode, char32_t const *file, char32_t *const argv[], int flags) { return libc_w32fspawnvpeat(mode,file,argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16fspawnvpeat(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[], int flags) { return libc_w16fspawnvpeat(mode,file,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32fspawnvpeat(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[], int flags) { return libc_w32fspawnvpeat(mode,file,argv,envp,flags|AT_DOSPATH); }



EXPORT(cwait,libc_cwait);
EXPORT(_cwait,libc_cwait);
CRT_DOS pid_t LIBCCALL
libc_cwait(int *tstat, pid_t pid, int UNUSED(action)) {
 /* This one's pretty simple, because it's literally just a waitpid() system call...
  * (It even returns the same thing, that being the PID of the joined process...) */
 return libc_waitpid(pid,tstat,WEXITED);
 /* NOTE: Apparently, the `action' argument is completely ignored... */
}



#if defined(__i386__) && !defined(__x86_64__)
#define CAPTURE_ARGV() \
 char **argv = (char **)&args
#define CAPTURE_ARGV_FLAGS() \
 char **argv = (char **)&args; \
 char **_temp = argv; int flags; \
 while (*_temp++); \
 flags = *(int *)_temp
#define CAPTURE_ARGV_ENVP() \
 char **argv = (char **)&args; \
 char **envp = argv; \
 while (*envp++); \
 envp = *(char ***)envp
#define CAPTURE_ARGV_ENVP_FLAGS() \
 char **argv = (char **)&args; \
 char **envp = argv; int flags; \
 while (*envp++); \
 envp = *(char ***)envp; \
 flags = *(int *)((char ***)envp+1)
#else
#define CAPTURE_ARGV() \
 char **argv; \
 va_list va; size_t argc = 1; \
 va_start(va,args); \
 while (va_arg(va,char *)) ++argc; \
 va_end(va); \
 argv = (char **)alloca(argc*sizeof(char *)); \
 va_start(va,args); argc = 0; \
 while ((argv[argc++] = va_arg(va,char *)) != NULL); \
 va_end(va)
#define CAPTURE_ARGV_FLAGS() \
 char **argv; int flags; \
 va_list va; size_t argc = 1; \
 va_start(va,args); \
 while (va_arg(va,char *)) ++argc; \
 va_end(va); \
 argv = (char **)alloca(argc*sizeof(char *)); \
 va_start(va,args); argc = 0; \
 while ((argv[argc++] = va_arg(va,char *)) != NULL); \
 flags = va_arg(va,int); \
 va_end(va)
#define CAPTURE_ARGV_ENVP() \
 char **argv,**envp; \
 va_list va; size_t argc = 1; \
 va_start(va,args); \
 while (va_arg(va,char *)) ++argc; \
 va_end(va); \
 argv = (char **)alloca(argc*sizeof(char *)); \
 va_start(va,args); argc = 0; \
 while ((argv[argc++] = va_arg(va,char *)) != NULL); \
 envp = va_arg(va,char **); \
 va_end(va)
#define CAPTURE_ARGV_ENVP_FLAGS() \
 char **argv,**envp; int flags; \
 va_list va; size_t argc = 1; \
 va_start(va,args); \
 while (va_arg(va,char *)) ++argc; \
 va_end(va); \
 argv = (char **)alloca(argc*sizeof(char *)); \
 va_start(va,args); argc = 0; \
 while ((argv[argc++] = va_arg(va,char *)) != NULL); \
 envp = va_arg(va,char **); \
 flags = va_arg(va,int); \
 va_end(va)
#endif

INTERN int ATTR_CDECL libc_execl(char const *path, char const *args, ...) { CAPTURE_ARGV(); return libc_execve(path,argv,environ); }
INTERN int ATTR_CDECL libc_execle(char const *path, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_execve(path,argv,envp); }
INTERN int ATTR_CDECL libc_execlp(char const *file, char const *args, ...) { CAPTURE_ARGV(); return libc_execvpe(file,argv,environ); }
INTERN int ATTR_CDECL libc_execlpe(char const *file, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_execvpe(file,argv,envp); }
CRT_DOS int ATTR_CDECL libc_dos_execl(char const *path, char const *args, ...) { CAPTURE_ARGV(); return libc_fexecveat(AT_FDCWD,path,argv,environ,AT_DOSPATH); }
CRT_DOS int ATTR_CDECL libc_dos_execle(char const *path, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_fexecveat(AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_DOS int ATTR_CDECL libc_dos_execlp(char const *file, char const *args, ...) { CAPTURE_ARGV(); return libc_fexecvpeat(file,argv,environ,AT_DOSPATH); }
CRT_DOS int ATTR_CDECL libc_dos_execlpe(char const *file, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_fexecvpeat(file,argv,envp,AT_DOSPATH); }
INTERN int ATTR_CDECL libc_fexecl(int exec_fd, char const *args, ...) { CAPTURE_ARGV(); return libc_fexecve(exec_fd,argv,environ); }
INTERN int ATTR_CDECL libc_fexecle(int exec_fd, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_fexecve(exec_fd,argv,envp); }
INTERN int ATTR_CDECL libc_fexeclat(fd_t dfd, char const *path, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fexecveat(dfd,path,argv,environ,flags); }
INTERN int ATTR_CDECL libc_fexecleat(fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fexecveat(dfd,path,argv,envp,flags); }
INTERN int ATTR_CDECL libc_fexeclpat(char const *file, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fexecvpeat(file,argv,environ,flags); }
INTERN int ATTR_CDECL libc_fexeclpeat(char const *file, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fexecvpeat(file,argv,envp,flags); }
CRT_DOS int ATTR_CDECL libc_dos_fexeclat(fd_t dfd, char const *path, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fexecveat(dfd,path,argv,environ,flags|AT_DOSPATH); }
CRT_DOS int ATTR_CDECL libc_dos_fexecleat(fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fexecveat(dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_DOS int ATTR_CDECL libc_dos_fexeclpat(char const *file, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fexecvpeat(file,argv,environ,flags|AT_DOSPATH); }
CRT_DOS int ATTR_CDECL libc_dos_fexeclpeat(char const *file, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fexecvpeat(file,argv,envp,flags|AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_spawnl(int mode, char const *path, char const *args, ...) { CAPTURE_ARGV(); return libc_spawnve(mode,path,argv,environ); }
CRT_DOS pid_t ATTR_CDECL libc_spawnle(int mode, char const *path, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_spawnve(mode,path,argv,envp); }
CRT_DOS pid_t ATTR_CDECL libc_spawnlp(int mode, char const *file, char const *args, ...) { CAPTURE_ARGV(); return libc_spawnvpe(mode,file,argv,environ); }
CRT_DOS pid_t ATTR_CDECL libc_spawnlpe(int mode, char const *file, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_spawnvpe(mode,file,argv,envp); }
CRT_DOS pid_t ATTR_CDECL libc_dos_spawnl(int mode, char const *path, char const *args, ...) { CAPTURE_ARGV(); return libc_fspawnveat(mode,AT_FDCWD,path,argv,environ,AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_dos_spawnle(int mode, char const *path, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_fspawnveat(mode,AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_dos_spawnlp(int mode, char const *file, char const *args, ...) { CAPTURE_ARGV(); return libc_fspawnvpeat(mode,file,argv,environ,AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_dos_spawnlpe(int mode, char const *file, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_fspawnvpeat(mode,file,argv,envp,AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_fspawnl(int mode, int exec_fd, char const *args, ...) { CAPTURE_ARGV(); return libc_fspawnve(mode,exec_fd,argv,environ); }
CRT_DOS pid_t ATTR_CDECL libc_fspawnle(int mode, int exec_fd, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_fspawnve(mode,exec_fd,argv,envp); }
CRT_DOS pid_t ATTR_CDECL libc_fspawnlat(int mode, fd_t dfd, char const *path, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fspawnveat(mode,dfd,path,argv,environ,flags); }
CRT_DOS pid_t ATTR_CDECL libc_fspawnleat(int mode, fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fspawnveat(mode,dfd,path,argv,envp,flags); }
CRT_DOS pid_t ATTR_CDECL libc_fspawnlpat(int mode, char const *file, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fspawnvpeat(mode,file,argv,environ,flags); }
CRT_DOS pid_t ATTR_CDECL libc_fspawnlpeat(int mode, char const *file, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fspawnvpeat(mode,file,argv,envp,flags); }
CRT_DOS pid_t ATTR_CDECL libc_dos_fspawnlat(int mode, fd_t dfd, char const *path, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fspawnveat(mode,dfd,path,argv,environ,flags|AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_dos_fspawnleat(int mode, fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fspawnveat(mode,dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_dos_fspawnlpat(int mode, char const *file, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fspawnvpeat(mode,file,argv,environ,flags|AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_dos_fspawnlpeat(int mode, char const *file, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fspawnvpeat(mode,file,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_w16execl(char16_t const *path, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16execve(path,(char16_t **)argv,libc_get_w16environ()); }
CRT_WIDECHAR int ATTR_CDECL libc_w32execl(char32_t const *path, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32execve(path,(char32_t **)argv,libc_get_w32environ()); }
CRT_WIDECHAR int ATTR_CDECL libc_w16execle(char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16execve(path,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR int ATTR_CDECL libc_w32execle(char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32execve(path,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR int ATTR_CDECL libc_w16execlp(char16_t const *file, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16execvpe(file,(char16_t **)argv,libc_get_w16environ()); }
CRT_WIDECHAR int ATTR_CDECL libc_w32execlp(char32_t const *file, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32execvpe(file,(char32_t **)argv,libc_get_w32environ()); }
CRT_WIDECHAR int ATTR_CDECL libc_w16execlpe(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16execvpe(file,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR int ATTR_CDECL libc_w32execlpe(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32execvpe(file,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16execl(char16_t const *path, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16fexecveat(AT_FDCWD,path,(char16_t **)argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32execl(char32_t const *path, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32fexecveat(AT_FDCWD,path,(char32_t **)argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16execle(char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16fexecveat(AT_FDCWD,path,(char16_t **)argv,(char16_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32execle(char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32fexecveat(AT_FDCWD,path,(char32_t **)argv,(char32_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16execlp(char16_t const *file, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16fexecvpeat(file,(char16_t **)argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32execlp(char32_t const *file, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32fexecvpeat(file,(char32_t **)argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16execlpe(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16fexecvpeat(file,(char16_t **)argv,(char16_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32execlpe(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32fexecvpeat(file,(char32_t **)argv,(char32_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_w16fexecl(int exec_fd, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16fexecve(exec_fd,(char16_t **)argv,libc_get_w16environ()); }
CRT_WIDECHAR int ATTR_CDECL libc_w32fexecl(int exec_fd, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32fexecve(exec_fd,(char32_t **)argv,libc_get_w32environ()); }
CRT_WIDECHAR int ATTR_CDECL libc_w16fexecle(int exec_fd, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16fexecve(exec_fd,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR int ATTR_CDECL libc_w32fexecle(int exec_fd, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32fexecve(exec_fd,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR int ATTR_CDECL libc_w16fexeclat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fexecveat(dfd,path,(char16_t **)argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w32fexeclat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fexecveat(dfd,path,(char32_t **)argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w16fexecleat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fexecveat(dfd,path,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w32fexecleat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fexecveat(dfd,path,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w16fexeclpat(char16_t const *file, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fexecvpeat(file,(char16_t **)argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w32fexeclpat(char32_t const *file, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fexecvpeat(file,(char32_t **)argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w16fexeclpeat(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fexecvpeat(file,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w32fexeclpeat(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fexecvpeat(file,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16fexeclat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fexecveat(dfd,path,(char16_t **)argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32fexeclat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fexecveat(dfd,path,(char32_t **)argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16fexecleat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fexecveat(dfd,path,(char16_t **)argv,(char16_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32fexecleat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fexecveat(dfd,path,(char32_t **)argv,(char32_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16fexeclpat(char16_t const *file, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fexecvpeat(file,(char16_t **)argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32fexeclpat(char32_t const *file, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fexecvpeat(file,(char32_t **)argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16fexeclpeat(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fexecvpeat(file,(char16_t **)argv,(char16_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32fexeclpeat(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fexecvpeat(file,(char32_t **)argv,(char32_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16spawnl(int mode, char16_t const *path, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16spawnve(mode,path,(char16_t **)argv,libc_get_w16environ()); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32spawnl(int mode, char32_t const *path, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32spawnve(mode,path,(char32_t **)argv,libc_get_w32environ()); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16spawnle(int mode, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16spawnve(mode,path,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32spawnle(int mode, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32spawnve(mode,path,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16spawnlp(int mode, char16_t const *file, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16spawnvpe(mode,file,(char16_t **)argv,libc_get_w16environ()); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32spawnlp(int mode, char32_t const *file, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32spawnvpe(mode,file,(char32_t **)argv,libc_get_w32environ()); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16spawnlpe(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16spawnvpe(mode,file,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32spawnlpe(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32spawnvpe(mode,file,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16spawnl(int mode, char16_t const *path, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16fspawnveat(mode,AT_FDCWD,path,(char16_t **)argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32spawnl(int mode, char32_t const *path, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32fspawnveat(mode,AT_FDCWD,path,(char32_t **)argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16spawnle(int mode, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16fspawnveat(mode,AT_FDCWD,path,(char16_t **)argv,(char16_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32spawnle(int mode, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32fspawnveat(mode,AT_FDCWD,path,(char32_t **)argv,(char32_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16spawnlp(int mode, char16_t const *file, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16fspawnvpeat(mode,file,(char16_t **)argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32spawnlp(int mode, char32_t const *file, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32fspawnvpeat(mode,file,(char32_t **)argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16spawnlpe(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16fspawnvpeat(mode,file,(char16_t **)argv,(char16_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32spawnlpe(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32fspawnvpeat(mode,file,(char32_t **)argv,(char32_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16fspawnl(int mode, int exec_fd, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16fspawnve(mode,exec_fd,(char16_t **)argv,libc_get_w16environ()); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32fspawnl(int mode, int exec_fd, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32fspawnve(mode,exec_fd,(char32_t **)argv,libc_get_w32environ()); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16fspawnle(int mode, int exec_fd, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16fspawnve(mode,exec_fd,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32fspawnle(int mode, int exec_fd, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32fspawnve(mode,exec_fd,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16fspawnlat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fspawnveat(mode,dfd,path,(char16_t **)argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32fspawnlat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fspawnveat(mode,dfd,path,(char32_t **)argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16fspawnleat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fspawnveat(mode,dfd,path,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32fspawnleat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fspawnveat(mode,dfd,path,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16fspawnlpat(int mode, char16_t const *file, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fspawnvpeat(mode,file,(char16_t **)argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32fspawnlpat(int mode, char32_t const *file, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fspawnvpeat(mode,file,(char32_t **)argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16fspawnlpeat(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fspawnvpeat(mode,file,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32fspawnlpeat(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fspawnvpeat(mode,file,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16fspawnlat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fspawnveat(mode,dfd,path,(char16_t **)argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32fspawnlat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fspawnveat(mode,dfd,path,(char32_t **)argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16fspawnleat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fspawnveat(mode,dfd,path,(char16_t **)argv,(char16_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32fspawnleat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fspawnveat(mode,dfd,path,(char32_t **)argv,(char32_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16fspawnlpat(int mode, char16_t const *file, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fspawnvpeat(mode,file,(char16_t **)argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32fspawnlpat(int mode, char32_t const *file, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fspawnvpeat(mode,file,(char32_t **)argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16fspawnlpeat(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fspawnvpeat(mode,file,(char16_t **)argv,(char16_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32fspawnlpeat(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fspawnvpeat(mode,file,(char32_t **)argv,(char32_t **)envp,flags|AT_DOSPATH); }




/* Exec functions */
EXPORT(fexecv,                     libc_fexecv);
EXPORT(fexecve,                    libc_fexecve);
EXPORT(__KSYM(execv),              libc_execv);
EXPORT(__DSYM(_execv),             libc_dos_execv);
EXPORT(__KSYM(execve),             libc_execve);
EXPORT(__DSYM(_execve),            libc_dos_execve);
EXPORT(__KSYM(execvp),             libc_execvp);
EXPORT(__DSYM(_execvp),            libc_dos_execvp);
EXPORT(__KSYM(execvpe),            libc_execvpe);
EXPORT(__DSYM(_execvpe),           libc_dos_execvpe);
EXPORT(__KSYM(fexecvat),           libc_fexecvat);
EXPORT(__DSYM(fexecvat),           libc_dos_fexecvat);
EXPORT(__KSYM(fexecveat),          libc_fexecveat);
EXPORT(__DSYM(fexecveat),          libc_dos_fexecveat);
EXPORT(__KSYM(fexecvpat),          libc_fexecvpat);
EXPORT(__DSYM(fexecvpat),          libc_dos_fexecvpat);
EXPORT(__KSYM(fexecvpeat),         libc_fexecvpeat);
EXPORT(__DSYM(fexecvpeat),         libc_dos_fexecvpeat);
EXPORT(__KSYM(execl),              libc_execl);
EXPORT(__DSYM(_execl),             libc_dos_execl);
EXPORT(__KSYM(execle),             libc_execle);
EXPORT(__DSYM(_execle),            libc_dos_execle);
EXPORT(__KSYM(execlp),             libc_execlp);
EXPORT(__DSYM(_execlp),            libc_dos_execlp);
EXPORT(__KSYM(execlpe),            libc_execlpe);
EXPORT(__DSYM(_execlpe),           libc_dos_execlpe);
EXPORT(fexecl,                     libc_fexecl);
EXPORT(fexecle,                    libc_fexecle);
EXPORT(__KSYM(fexeclat),           libc_fexeclat);
EXPORT(__DSYM(fexeclat),           libc_dos_fexeclat);
EXPORT(__KSYM(fexecleat),          libc_fexecleat);
EXPORT(__DSYM(fexecleat),          libc_dos_fexecleat);
EXPORT(__KSYM(fexeclpat),          libc_fexeclpat);
EXPORT(__DSYM(fexeclpat),          libc_dos_fexeclpat);
EXPORT(__KSYM(fexeclpeat),         libc_fexeclpeat);
EXPORT(__DSYM(fexeclpeat),         libc_dos_fexeclpeat);

/* Spawn functions */
EXPORT(fspawnv,                    libc_fspawnv);
EXPORT(fspawnve,                   libc_fspawnve);
EXPORT(__KSYM(spawnv),             libc_spawnv);
EXPORT(__DSYM(_spawnv),            libc_dos_spawnv);
EXPORT(__KSYM(spawnve),            libc_spawnve);
EXPORT(__DSYM(_spawnve),           libc_dos_spawnve);
EXPORT(__KSYM(spawnvp),            libc_spawnvp);
EXPORT(__DSYM(_spawnvp),           libc_dos_spawnvp);
EXPORT(__KSYM(spawnvpe),           libc_spawnvpe);
EXPORT(__DSYM(_spawnvpe),          libc_dos_spawnvpe);
EXPORT(__KSYM(fspawnvat),          libc_fspawnvat);
EXPORT(__DSYM(fspawnvat),          libc_dos_fspawnvat);
EXPORT(__KSYM(fspawnveat),         libc_fspawnveat);
EXPORT(__DSYM(fspawnveat),         libc_dos_fspawnveat);
EXPORT(__KSYM(fspawnvpat),         libc_fspawnvpat);
EXPORT(__DSYM(fspawnvpat),         libc_dos_fspawnvpat);
EXPORT(__KSYM(fspawnvpeat),        libc_fspawnvpeat);
EXPORT(__DSYM(fspawnvpeat),        libc_dos_fspawnvpeat);
EXPORT(__KSYM(spawnl),             libc_spawnl);
EXPORT(__DSYM(_spawnl),            libc_dos_spawnl);
EXPORT(__KSYM(spawnle),            libc_spawnle);
EXPORT(__DSYM(_spawnle),           libc_dos_spawnle);
EXPORT(__KSYM(spawnlp),            libc_spawnlp);
EXPORT(__DSYM(_spawnlp),           libc_dos_spawnlp);
EXPORT(__KSYM(spawnlpe),           libc_spawnlpe);
EXPORT(__DSYM(_spawnlpe),          libc_dos_spawnlpe);
EXPORT(fspawnl,                    libc_fspawnl);
EXPORT(fspawnle,                   libc_fspawnle);
EXPORT(__KSYM(fspawnlat),          libc_fspawnlat);
EXPORT(__DSYM(fspawnlat),          libc_dos_fspawnlat);
EXPORT(__KSYM(fspawnleat),         libc_fspawnleat);
EXPORT(__DSYM(fspawnleat),         libc_dos_fspawnleat);
EXPORT(__KSYM(fspawnlpat),         libc_fspawnlpat);
EXPORT(__DSYM(fspawnlpat),         libc_dos_fspawnlpat);
EXPORT(__KSYM(fspawnlpeat),        libc_fspawnlpeat);
EXPORT(__DSYM(fspawnlpeat),        libc_dos_fspawnlpeat);


/* Export wide-character exec() and spawn() functions. */
EXPORT(__SYMw16(wfexecl),          libc_w16fexecl);
EXPORT(__SYMw32(wfexecl),          libc_w32fexecl);
EXPORT(__SYMw16(wfexecle),         libc_w16fexecle);
EXPORT(__SYMw32(wfexecle),         libc_w32fexecle);
EXPORT(__SYMw16(wfexecv),          libc_w16fexecv);
EXPORT(__SYMw32(wfexecv),          libc_w32fexecv);
EXPORT(__SYMw16(wfexecve),         libc_w16fexecve);
EXPORT(__SYMw32(wfexecve),         libc_w32fexecve);
EXPORT(__KSYMw16(wexecl),          libc_w16execl);
EXPORT(__KSYMw32(wexecl),          libc_w32execl);
EXPORT(__DSYMw16(_wexecl),         libc_dos_w16execl);
EXPORT(__DSYMw32(wexecl),          libc_dos_w32execl);
EXPORT(__KSYMw16(wexecle),         libc_w16execle);
EXPORT(__KSYMw32(wexecle),         libc_w32execle);
EXPORT(__DSYMw16(_wexecle),        libc_dos_w16execle);
EXPORT(__DSYMw32(wexecle),         libc_dos_w32execle);
EXPORT(__KSYMw16(wexeclp),         libc_w16execlp);
EXPORT(__KSYMw32(wexeclp),         libc_w32execlp);
EXPORT(__DSYMw16(_wexeclp),        libc_dos_w16execlp);
EXPORT(__DSYMw32(wexeclp),         libc_dos_w32execlp);
EXPORT(__KSYMw16(wexeclpe),        libc_w16execlpe);
EXPORT(__KSYMw32(wexeclpe),        libc_w32execlpe);
EXPORT(__DSYMw16(_wexeclpe),       libc_dos_w16execlpe);
EXPORT(__DSYMw32(wexeclpe),        libc_dos_w32execlpe);
EXPORT(__KSYMw16(wexecv),          libc_w16execv);
EXPORT(__KSYMw32(wexecv),          libc_w32execv);
EXPORT(__DSYMw16(_wexecv),         libc_dos_w16execv);
EXPORT(__DSYMw32(wexecv),          libc_dos_w32execv);
EXPORT(__KSYMw16(wexecve),         libc_w16execve);
EXPORT(__KSYMw32(wexecve),         libc_w32execve);
EXPORT(__DSYMw16(_wexecve),        libc_dos_w16execve);
EXPORT(__DSYMw32(wexecve),         libc_dos_w32execve);
EXPORT(__KSYMw16(wexecvp),         libc_w16execvp);
EXPORT(__KSYMw32(wexecvp),         libc_w32execvp);
EXPORT(__DSYMw16(_wexecvp),        libc_dos_w16execvp);
EXPORT(__DSYMw32(wexecvp),         libc_dos_w32execvp);
EXPORT(__KSYMw16(wexecvpe),        libc_w16execvpe);
EXPORT(__KSYMw32(wexecvpe),        libc_w32execvpe);
EXPORT(__DSYMw16(_wexecvpe),       libc_dos_w16execvpe);
EXPORT(__DSYMw32(wexecvpe),        libc_dos_w32execvpe);
EXPORT(__KSYMw16(wfexeclat),       libc_w16fexeclat);
EXPORT(__KSYMw32(wfexeclat),       libc_w32fexeclat);
EXPORT(__DSYMw16(wfexeclat),       libc_dos_w16fexeclat);
EXPORT(__DSYMw32(wfexeclat),       libc_dos_w32fexeclat);
EXPORT(__KSYMw16(wfexecleat),      libc_w16fexecleat);
EXPORT(__KSYMw32(wfexecleat),      libc_w32fexecleat);
EXPORT(__DSYMw16(wfexecleat),      libc_dos_w16fexecleat);
EXPORT(__DSYMw32(wfexecleat),      libc_dos_w32fexecleat);
EXPORT(__KSYMw16(wfexeclpat),      libc_w16fexeclpat);
EXPORT(__KSYMw32(wfexeclpat),      libc_w32fexeclpat);
EXPORT(__DSYMw16(wfexeclpat),      libc_dos_w16fexeclpat);
EXPORT(__DSYMw32(wfexeclpat),      libc_dos_w32fexeclpat);
EXPORT(__KSYMw16(wfexeclpeat),     libc_w16fexeclpeat);
EXPORT(__KSYMw32(wfexeclpeat),     libc_w32fexeclpeat);
EXPORT(__DSYMw16(wfexeclpeat),     libc_dos_w16fexeclpeat);
EXPORT(__DSYMw32(wfexeclpeat),     libc_dos_w32fexeclpeat);
EXPORT(__KSYMw16(wfexecvat),       libc_w16fexecvat);
EXPORT(__KSYMw32(wfexecvat),       libc_w32fexecvat);
EXPORT(__DSYMw16(wfexecvat),       libc_dos_w16fexecvat);
EXPORT(__DSYMw32(wfexecvat),       libc_dos_w32fexecvat);
EXPORT(__KSYMw16(wfexecvpat),      libc_w16fexecvpat);
EXPORT(__KSYMw32(wfexecvpat),      libc_w32fexecvpat);
EXPORT(__DSYMw16(wfexecvpat),      libc_dos_w16fexecvpat);
EXPORT(__DSYMw32(wfexecvpat),      libc_dos_w32fexecvpat);
//EXPORT(__KSYMw16(wfexecveat),    libc_w16fexecveat); /* Exported by `widechar.c' */
//EXPORT(__KSYMw32(wfexecveat),    libc_w32fexecveat); /* Exported by `widechar.c' */
EXPORT(__DSYMw16(wfexecveat),      libc_dos_w16fexecveat);
EXPORT(__DSYMw32(wfexecveat),      libc_dos_w32fexecveat);
//EXPORT(__KSYMw16(wfexecvpeat),   libc_w16fexecvpeat); /* Exported by `widechar.c' */
//EXPORT(__KSYMw32(wfexecvpeat),   libc_w32fexecvpeat); /* Exported by `widechar.c' */
EXPORT(__DSYMw16(wfexecvpeat),     libc_dos_w16fexecvpeat);
EXPORT(__DSYMw32(wfexecvpeat),     libc_dos_w32fexecvpeat);
EXPORT(__SYMw16(wfspawnl),         libc_w16fspawnl);
EXPORT(__SYMw32(wfspawnl),         libc_w32fspawnl);
EXPORT(__SYMw16(wfspawnle),        libc_w16fspawnle);
EXPORT(__SYMw32(wfspawnle),        libc_w32fspawnle);
EXPORT(__SYMw16(wfspawnv),         libc_w16fspawnv);
EXPORT(__SYMw32(wfspawnv),         libc_w32fspawnv);
EXPORT(__SYMw16(wfspawnve),        libc_w16fspawnve);
EXPORT(__SYMw32(wfspawnve),        libc_w32fspawnve);
EXPORT(__KSYMw16(wspawnl),         libc_w16spawnl);
EXPORT(__KSYMw32(wspawnl),         libc_w32spawnl);
EXPORT(__DSYMw16(_wspawnl),        libc_dos_w16spawnl);
EXPORT(__DSYMw32(wspawnl),         libc_dos_w32spawnl);
EXPORT(__KSYMw16(wspawnle),        libc_w16spawnle);
EXPORT(__KSYMw32(wspawnle),        libc_w32spawnle);
EXPORT(__DSYMw16(_wspawnle),       libc_dos_w16spawnle);
EXPORT(__DSYMw32(wspawnle),        libc_dos_w32spawnle);
EXPORT(__KSYMw16(wspawnlp),        libc_w16spawnlp);
EXPORT(__KSYMw32(wspawnlp),        libc_w32spawnlp);
EXPORT(__DSYMw16(_wspawnlp),       libc_dos_w16spawnlp);
EXPORT(__DSYMw32(wspawnlp),        libc_dos_w32spawnlp);
EXPORT(__KSYMw16(wspawnlpe),       libc_w16spawnlpe);
EXPORT(__KSYMw32(wspawnlpe),       libc_w32spawnlpe);
EXPORT(__DSYMw16(_wspawnlpe),      libc_dos_w16spawnlpe);
EXPORT(__DSYMw32(wspawnlpe),       libc_dos_w32spawnlpe);
EXPORT(__KSYMw16(wspawnv),         libc_w16spawnv);
EXPORT(__KSYMw32(wspawnv),         libc_w32spawnv);
EXPORT(__DSYMw16(_wspawnv),        libc_dos_w16spawnv);
EXPORT(__DSYMw32(wspawnv),         libc_dos_w32spawnv);
EXPORT(__KSYMw16(wspawnve),        libc_w16spawnve);
EXPORT(__KSYMw32(wspawnve),        libc_w32spawnve);
EXPORT(__DSYMw16(_wspawnve),       libc_dos_w16spawnve);
EXPORT(__DSYMw32(wspawnve),        libc_dos_w32spawnve);
EXPORT(__KSYMw16(wspawnvp),        libc_w16spawnvp);
EXPORT(__KSYMw32(wspawnvp),        libc_w32spawnvp);
EXPORT(__DSYMw16(_wspawnvp),       libc_dos_w16spawnvp);
EXPORT(__DSYMw32(wspawnvp),        libc_dos_w32spawnvp);
EXPORT(__KSYMw16(wspawnvpe),       libc_w16spawnvpe);
EXPORT(__KSYMw32(wspawnvpe),       libc_w32spawnvpe);
EXPORT(__DSYMw16(_wspawnvpe),      libc_dos_w16spawnvpe);
EXPORT(__DSYMw32(wspawnvpe),       libc_dos_w32spawnvpe);
EXPORT(__KSYMw16(wfspawnlat),      libc_w16fspawnlat);
EXPORT(__KSYMw32(wfspawnlat),      libc_w32fspawnlat);
EXPORT(__DSYMw16(wfspawnlat),      libc_dos_w16fspawnlat);
EXPORT(__DSYMw32(wfspawnlat),      libc_dos_w32fspawnlat);
EXPORT(__KSYMw16(wfspawnleat),     libc_w16fspawnleat);
EXPORT(__KSYMw32(wfspawnleat),     libc_w32fspawnleat);
EXPORT(__DSYMw16(wfspawnleat),     libc_dos_w16fspawnleat);
EXPORT(__DSYMw32(wfspawnleat),     libc_dos_w32fspawnleat);
EXPORT(__KSYMw16(wfspawnlpat),     libc_w16fspawnlpat);
EXPORT(__KSYMw32(wfspawnlpat),     libc_w32fspawnlpat);
EXPORT(__DSYMw16(wfspawnlpat),     libc_dos_w16fspawnlpat);
EXPORT(__DSYMw32(wfspawnlpat),     libc_dos_w32fspawnlpat);
EXPORT(__KSYMw16(wfspawnlpeat),    libc_w16fspawnlpeat);
EXPORT(__KSYMw32(wfspawnlpeat),    libc_w32fspawnlpeat);
EXPORT(__DSYMw16(wfspawnlpeat),    libc_dos_w16fspawnlpeat);
EXPORT(__DSYMw32(wfspawnlpeat),    libc_dos_w32fspawnlpeat);
EXPORT(__KSYMw16(wfspawnvat),      libc_w16fspawnvat);
EXPORT(__KSYMw32(wfspawnvat),      libc_w32fspawnvat);
EXPORT(__DSYMw16(wfspawnvat),      libc_dos_w16fspawnvat);
EXPORT(__DSYMw32(wfspawnvat),      libc_dos_w32fspawnvat);
EXPORT(__KSYMw16(wfspawnvpat),     libc_w16fspawnvpat);
EXPORT(__KSYMw32(wfspawnvpat),     libc_w32fspawnvpat);
EXPORT(__DSYMw16(wfspawnvpat),     libc_dos_w16fspawnvpat);
EXPORT(__DSYMw32(wfspawnvpat),     libc_dos_w32fspawnvpat);
//EXPORT(__KSYMw16(wfspawnveat),   libc_w16fspawnveat); /* Exported by `widechar.c' */
//EXPORT(__KSYMw32(wfspawnveat),   libc_w32fspawnveat); /* Exported by `widechar.c' */
EXPORT(__DSYMw16(wfspawnveat),     libc_dos_w16fspawnveat);
EXPORT(__DSYMw32(wfspawnveat),     libc_dos_w32fspawnveat);
//EXPORT(__KSYMw16(wfspawnvpeat),  libc_w16fspawnvpeat); /* Exported by `widechar.c' */
//EXPORT(__KSYMw32(wfspawnvpeat),  libc_w32fspawnvpeat); /* Exported by `widechar.c' */
EXPORT(__DSYMw16(wfspawnvpeat),    libc_dos_w16fspawnvpeat);
EXPORT(__DSYMw32(wfspawnvpeat),    libc_dos_w32fspawnvpeat);










EXPORT(Xexecv,libc_Xexecv);
EXPORT(Xexecvp,libc_Xexecvp);
EXPORT(Xexecvpe,libc_Xexecvpe);
EXPORT(Xfexecv,libc_Xfexecv);
EXPORT(Xfexecve,libc_Xfexecve);
EXPORT(Xfexecvat,libc_Xfexecvat);
EXPORT(Xfexecvpat,libc_Xfexecvpat);
EXPORT(Xfexecvpeat,libc_Xfexecvpeat);
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xexecv(char const *path, char *const argv[]) { Xsys_execve(path,argv,environ); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xexecvp(char const *file, char *const argv[]) { libc_Xfexecvpeat(file,argv,environ,0); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xexecvpe(char const *file, char *const argv[], char *const envp[]) { libc_Xfexecvpeat(file,argv,envp,0); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xfexecv(int exec_fd, char *const argv[]) { Xsys_execveat(exec_fd,libc_empty_string,argv,environ,AT_EMPTY_PATH); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xfexecve(int exec_fd, char *const argv[], char *const envp[]) { Xsys_execveat(exec_fd,libc_empty_string,argv,envp,AT_EMPTY_PATH); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xfexecvat(fd_t dfd, char const *path, char *const argv[], int flags) { Xsys_execveat(dfd,path,argv,environ,flags); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xfexecvpat(char const *file, char *const argv[], int flags) { libc_Xfexecvpeat(file,argv,environ,flags); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL
libc_Xfexecvpeat(char const *file,
                 char *const argv[],
                 char *const envp[],
                 int flags) {
 /* TODO: Search $PATH */
 error_throw(E_NOT_IMPLEMENTED);
}


EXPORT(Xexecl,libc_Xexecl);
EXPORT(Xexecle,libc_Xexecle);
EXPORT(Xexeclp,libc_Xexeclp);
EXPORT(Xexeclpe,libc_Xexeclpe);
EXPORT(Xfexecl,libc_Xfexecl);
EXPORT(Xfexecle,libc_Xfexecle);
EXPORT(Xfexeclat,libc_Xfexeclat);
EXPORT(Xfexecleat,libc_Xfexecleat);
EXPORT(Xfexeclpat,libc_Xfexeclpat);
EXPORT(Xfexeclpeat,libc_Xfexeclpeat);
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xexecl(char const *path, char const *args, ...) { CAPTURE_ARGV(); libc_Xexecve(path,argv,environ); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xexecle(char const *path, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xexecve(path,argv,envp); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xexeclp(char const *file, char const *args, ...) { CAPTURE_ARGV(); libc_Xexecvpe(file,argv,environ); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xexeclpe(char const *file, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xexecvpe(file,argv,envp); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xfexecl(int exec_fd, char const *args, ...) { CAPTURE_ARGV(); libc_Xfexecve(exec_fd,argv,environ); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xfexecle(int exec_fd, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xfexecve(exec_fd,argv,envp); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xfexeclat(fd_t dfd, char const *path, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); Xsys_execveat(dfd,path,argv,environ,flags); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xfexecleat(fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); Xsys_execveat(dfd,path,argv,envp,flags); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xfexeclpat(char const *file, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); libc_Xfexecvpeat(file,argv,environ,flags); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xfexeclpeat(char const *file, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); libc_Xfexecvpeat(file,argv,envp,flags); }

EXPORT(Xcwait,libc_Xcwait);
INTERN pid_t LIBCCALL
libc_Xcwait(int *tstat, pid_t pid, int UNUSED(action)) {
 /* This one's pretty simple, because it's literally just a waitpid() system call...
  * (It even returns the same thing, that being the PID of the joined process...) */
 return libc_Xwaitpid(pid,tstat,WEXITED);
 /* NOTE: Apparently, the `action' argument is completely ignored... */
}


EXPORT(Xspawnv,libc_Xspawnv);
EXPORT(Xspawnve,libc_Xspawnve);
EXPORT(Xspawnvp,libc_Xspawnvp);
EXPORT(Xspawnvpe,libc_Xspawnvpe);
EXPORT(Xfspawnv,libc_Xfspawnv);
EXPORT(Xfspawnve,libc_Xfspawnve);
EXPORT(Xfspawnvat,libc_Xfspawnvat);
EXPORT(Xfspawnveat,libc_Xfspawnveat);
EXPORT(Xfspawnvpat,libc_Xfspawnvpat);
EXPORT(Xfspawnvpeat,libc_Xfspawnvpeat);
CRT_EXCEPT pid_t LIBCCALL libc_Xspawnv(int mode, char const *path, char *const argv[]) { return libc_Xfspawnveat(mode,AT_FDCWD,path,argv,environ,0); }
CRT_EXCEPT pid_t LIBCCALL libc_Xspawnve(int mode, char const *path, char *const argv[], char *const envp[]) { return libc_Xfspawnveat(mode,AT_FDCWD,path,argv,envp,0); }
CRT_EXCEPT pid_t LIBCCALL libc_Xspawnvp(int mode, char const *file, char *const argv[]) { return libc_Xfspawnvpeat(mode,file,argv,environ,0); }
CRT_EXCEPT pid_t LIBCCALL libc_Xspawnvpe(int mode, char const *file, char *const argv[], char *const envp[]) { return libc_Xfspawnvpeat(mode,file,argv,envp,0); }
CRT_EXCEPT pid_t LIBCCALL libc_Xfspawnv(int mode, int exec_fd, char *const argv[]) { return libc_Xfspawnveat(mode,exec_fd,libc_empty_string,argv,environ,AT_EMPTY_PATH); }
CRT_EXCEPT pid_t LIBCCALL libc_Xfspawnve(int mode, int exec_fd, char *const argv[], char *const envp[]) { return libc_Xfspawnveat(mode,exec_fd,libc_empty_string,argv,envp,AT_EMPTY_PATH); }
CRT_EXCEPT pid_t LIBCCALL libc_Xfspawnvat(int mode, fd_t dfd, char const *path, char *const argv[], int flags) { return libc_Xfspawnveat(mode,dfd,path,argv,environ,flags); }
CRT_EXCEPT pid_t LIBCCALL libc_Xfspawnvpat(int mode, char const *file, char *const argv[], int flags) { return libc_Xfspawnvpeat(mode,file,argv,environ,flags); }
CRT_EXCEPT pid_t LIBCCALL
libc_Xfspawnveat(int mode, fd_t dfd,
                 char const *path,
                 char *const argv[],
                 char *const envp[],
                 int flags) {
 /* TODO */
 error_throw(E_NOT_IMPLEMENTED);
}
CRT_EXCEPT pid_t LIBCCALL
libc_Xfspawnvpeat(int mode,
                  char const *file,
                  char *const argv[],
                  char *const envp[],
                  int flags) {
 /* TODO: Search $PATH */
 error_throw(E_NOT_IMPLEMENTED);
}


EXPORT(Xspawnl,libc_Xspawnl);
EXPORT(Xspawnle,libc_Xspawnle);
EXPORT(Xspawnlp,libc_Xspawnlp);
EXPORT(Xspawnlpe,libc_Xspawnlpe);
EXPORT(Xfspawnl,libc_Xfspawnl);
EXPORT(Xfspawnle,libc_Xfspawnle);
EXPORT(Xfspawnlat,libc_Xfspawnlat);
EXPORT(Xfspawnleat,libc_Xfspawnleat);
EXPORT(Xfspawnlpat,libc_Xfspawnlpat);
EXPORT(Xfspawnlpeat,libc_Xfspawnlpeat);
CRT_EXCEPT pid_t ATTR_CDECL libc_Xspawnl(int mode, char const *path, char const *args, ...) { CAPTURE_ARGV(); return libc_Xspawnve(mode,path,argv,environ); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xspawnle(int mode, char const *path, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xspawnve(mode,path,argv,envp); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xspawnlp(int mode, char const *file, char const *args, ...) { CAPTURE_ARGV(); return libc_Xspawnvpe(mode,file,argv,environ); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xspawnlpe(int mode, char const *file, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xspawnvpe(mode,file,argv,envp); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xfspawnl(int mode, int exec_fd, char const *args, ...) { CAPTURE_ARGV(); return libc_Xfspawnve(mode,exec_fd,argv,environ); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xfspawnle(int mode, int exec_fd, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xfspawnve(mode,exec_fd,argv,envp); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xfspawnlat(int mode, fd_t dfd, char const *path, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_Xfspawnveat(mode,dfd,path,argv,environ,flags); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xfspawnleat(int mode, fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_Xfspawnveat(mode,dfd,path,argv,envp,flags); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xfspawnlpat(int mode, char const *file, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_Xfspawnvpeat(mode,file,argv,environ,flags); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xfspawnlpeat(int mode, char const *file, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_Xfspawnvpeat(mode,file,argv,envp,flags); }





EXPORT(__SYMw16(Xwexecv),libc_Xw16execv);
EXPORT(__SYMw32(Xwexecv),libc_Xw32execv);
EXPORT(__SYMw16(Xwexecve),libc_Xw16execve);
EXPORT(__SYMw32(Xwexecve),libc_Xw32execve);
EXPORT(__SYMw16(Xwexecvp),libc_Xw16execvp);
EXPORT(__SYMw32(Xwexecvp),libc_Xw32execvp);
EXPORT(__SYMw16(Xwexecvpe),libc_Xw16execvpe);
EXPORT(__SYMw32(Xwexecvpe),libc_Xw32execvpe);
EXPORT(__SYMw16(Xwfexecv),libc_Xw16fexecv);
EXPORT(__SYMw32(Xwfexecv),libc_Xw32fexecv);
EXPORT(__SYMw16(Xwfexecve),libc_Xw16fexecve);
EXPORT(__SYMw32(Xwfexecve),libc_Xw32fexecve);
EXPORT(__SYMw16(Xwfexecvat),libc_Xw16fexecvat);
EXPORT(__SYMw32(Xwfexecvat),libc_Xw32fexecvat);
EXPORT(__SYMw16(Xwfexecvpat),libc_Xw16fexecvpat);
EXPORT(__SYMw32(Xwfexecvpat),libc_Xw32fexecvpat);
EXPORT(__SYMw16(Xwspawnv),libc_Xw16spawnv);
EXPORT(__SYMw32(Xwspawnv),libc_Xw32spawnv);
EXPORT(__SYMw16(Xwspawnve),libc_Xw16spawnve);
EXPORT(__SYMw32(Xwspawnve),libc_Xw32spawnve);
EXPORT(__SYMw16(Xwspawnvp),libc_Xw16spawnvp);
EXPORT(__SYMw32(Xwspawnvp),libc_Xw32spawnvp);
EXPORT(__SYMw16(Xwspawnvpe),libc_Xw16spawnvpe);
EXPORT(__SYMw32(Xwspawnvpe),libc_Xw32spawnvpe);
EXPORT(__SYMw16(Xwfspawnv),libc_Xw16fspawnv);
EXPORT(__SYMw32(Xwfspawnv),libc_Xw32fspawnv);
EXPORT(__SYMw16(Xwfspawnve),libc_Xw16fspawnve);
EXPORT(__SYMw32(Xwfspawnve),libc_Xw32fspawnve);
EXPORT(__SYMw16(Xwfspawnvat),libc_Xw16fspawnvat);
EXPORT(__SYMw32(Xwfspawnvat),libc_Xw32fspawnvat);
EXPORT(__SYMw16(Xwfspawnvpat),libc_Xw16fspawnvpat);
EXPORT(__SYMw32(Xwfspawnvpat),libc_Xw32fspawnvpat);
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16execv(char16_t const *path, char16_t *const argv[]) { libc_Xw16fexecveat(AT_FDCWD,path,argv,libc_Xget_w16environ(),0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32execv(char32_t const *path, char32_t *const argv[]) { libc_Xw32fexecveat(AT_FDCWD,path,argv,libc_Xget_w32environ(),0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16execve(char16_t const *path, char16_t *const argv[], char16_t *const envp[]) { libc_Xw16fexecveat(AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32execve(char32_t const *path, char32_t *const argv[], char32_t *const envp[]) { libc_Xw32fexecveat(AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16execvp(char16_t const *file, char16_t *const argv[]) { libc_Xw16fexecvpeat(file,argv,libc_Xget_w16environ(),0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32execvp(char32_t const *file, char32_t *const argv[]) { libc_Xw32fexecvpeat(file,argv,libc_Xget_w32environ(),0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16execvpe(char16_t const *file, char16_t *const argv[], char16_t *const envp[]) { libc_Xw16fexecvpeat(file,argv,envp,0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32execvpe(char32_t const *file, char32_t *const argv[], char32_t *const envp[]) { libc_Xw32fexecvpeat(file,argv,envp,0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16fexecv(int exec_fd, char16_t *const argv[]) { libc_Xw16fexecveat(exec_fd,libc_empty_string16,argv,libc_Xget_w16environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32fexecv(int exec_fd, char32_t *const argv[]) { libc_Xw32fexecveat(exec_fd,libc_empty_string32,argv,libc_Xget_w32environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16fexecve(int exec_fd, char16_t *const argv[], char16_t *const envp[]) { libc_Xw16fexecveat(exec_fd,libc_empty_string16,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32fexecve(int exec_fd, char32_t *const argv[], char32_t *const envp[]) { libc_Xw32fexecveat(exec_fd,libc_empty_string32,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16fexecvat(fd_t dfd, char16_t const *path, char16_t *const argv[], int flags) { libc_Xw16fexecveat(dfd,path,argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32fexecvat(fd_t dfd, char32_t const *path, char32_t *const argv[], int flags) { libc_Xw32fexecveat(dfd,path,argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16fexecvpat(char16_t const *file, char16_t *const argv[], int flags) { libc_Xw16fexecvpeat(file,argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32fexecvpat(char32_t const *file, char32_t *const argv[], int flags) { libc_Xw32fexecvpeat(file,argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16spawnv(int mode, char16_t const *path, char16_t *const argv[]) { return libc_Xw16fspawnveat(mode,AT_FDCWD,path,argv,libc_Xget_w16environ(),0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32spawnv(int mode, char32_t const *path, char32_t *const argv[]) { return libc_Xw32fspawnveat(mode,AT_FDCWD,path,argv,libc_Xget_w32environ(),0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16spawnve(int mode, char16_t const *path, char16_t *const argv[], char16_t *const envp[]) { return libc_Xw16fspawnveat(mode,AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32spawnve(int mode, char32_t const *path, char32_t *const argv[], char32_t *const envp[]) { return libc_Xw32fspawnveat(mode,AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16spawnvp(int mode, char16_t const *file, char16_t *const argv[]) { return libc_Xw16fspawnvpeat(mode,file,argv,libc_Xget_w16environ(),0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32spawnvp(int mode, char32_t const *file, char32_t *const argv[]) { return libc_Xw32fspawnvpeat(mode,file,argv,libc_Xget_w32environ(),0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16spawnvpe(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[]) { return libc_Xw16fspawnvpeat(mode,file,argv,envp,0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32spawnvpe(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[]) { return libc_Xw32fspawnvpeat(mode,file,argv,envp,0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16fspawnv(int mode, int exec_fd, char16_t *const argv[]) { return libc_Xw16fspawnveat(mode,exec_fd,libc_empty_string16,argv,libc_Xget_w16environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32fspawnv(int mode, int exec_fd, char32_t *const argv[]) { return libc_Xw32fspawnveat(mode,exec_fd,libc_empty_string32,argv,libc_Xget_w32environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16fspawnve(int mode, int exec_fd, char16_t *const argv[], char16_t *const envp[]) { return libc_Xw16fspawnveat(mode,exec_fd,libc_empty_string16,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32fspawnve(int mode, int exec_fd, char32_t *const argv[], char32_t *const envp[]) { return libc_Xw32fspawnveat(mode,exec_fd,libc_empty_string32,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16fspawnvat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], int flags) { return libc_Xw16fspawnveat(mode,dfd,path,argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32fspawnvat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], int flags) { return libc_Xw32fspawnveat(mode,dfd,path,argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16fspawnvpat(int mode, char16_t const *file, char16_t *const argv[], int flags) { return libc_Xw16fspawnvpeat(mode,file,argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32fspawnvpat(int mode, char32_t const *file, char32_t *const argv[], int flags) { return libc_Xw32fspawnvpeat(mode,file,argv,libc_Xget_w32environ(),flags); }


EXPORT(__SYMw16(Xwexecl),libc_Xw16execl);
EXPORT(__SYMw32(Xwexecl),libc_Xw32execl);
EXPORT(__SYMw16(Xwexecle),libc_Xw16execle);
EXPORT(__SYMw32(Xwexecle),libc_Xw32execle);
EXPORT(__SYMw16(Xwexeclp),libc_Xw16execlp);
EXPORT(__SYMw32(Xwexeclp),libc_Xw32execlp);
EXPORT(__SYMw16(Xwexeclpe),libc_Xw16execlpe);
EXPORT(__SYMw32(Xwexeclpe),libc_Xw32execlpe);
EXPORT(__SYMw16(Xwfexecl),libc_Xw16fexecl);
EXPORT(__SYMw32(Xwfexecl),libc_Xw32fexecl);
EXPORT(__SYMw16(Xwfexecle),libc_Xw16fexecle);
EXPORT(__SYMw32(Xwfexecle),libc_Xw32fexecle);
EXPORT(__SYMw16(Xwfexeclat),libc_Xw16fexeclat);
EXPORT(__SYMw32(Xwfexeclat),libc_Xw32fexeclat);
EXPORT(__SYMw16(Xwfexecleat),libc_Xw16fexecleat);
EXPORT(__SYMw32(Xwfexecleat),libc_Xw32fexecleat);
EXPORT(__SYMw16(Xwfexeclpat),libc_Xw16fexeclpat);
EXPORT(__SYMw32(Xwfexeclpat),libc_Xw32fexeclpat);
EXPORT(__SYMw16(Xwfexeclpeat),libc_Xw16fexeclpeat);
EXPORT(__SYMw32(Xwfexeclpeat),libc_Xw32fexeclpeat);
EXPORT(__SYMw16(Xwspawnl),libc_Xw16spawnl);
EXPORT(__SYMw32(Xwspawnl),libc_Xw32spawnl);
EXPORT(__SYMw16(Xwspawnle),libc_Xw16spawnle);
EXPORT(__SYMw32(Xwspawnle),libc_Xw32spawnle);
EXPORT(__SYMw16(Xwspawnlp),libc_Xw16spawnlp);
EXPORT(__SYMw32(Xwspawnlp),libc_Xw32spawnlp);
EXPORT(__SYMw16(Xwspawnlpe),libc_Xw16spawnlpe);
EXPORT(__SYMw32(Xwspawnlpe),libc_Xw32spawnlpe);
EXPORT(__SYMw16(Xwfspawnl),libc_Xw16fspawnl);
EXPORT(__SYMw32(Xwfspawnl),libc_Xw32fspawnl);
EXPORT(__SYMw16(Xwfspawnle),libc_Xw16fspawnle);
EXPORT(__SYMw32(Xwfspawnle),libc_Xw32fspawnle);
EXPORT(__SYMw16(Xwfspawnlat),libc_Xw16fspawnlat);
EXPORT(__SYMw32(Xwfspawnlat),libc_Xw32fspawnlat);
EXPORT(__SYMw16(Xwfspawnleat),libc_Xw16fspawnleat);
EXPORT(__SYMw32(Xwfspawnleat),libc_Xw32fspawnleat);
EXPORT(__SYMw16(Xwfspawnlpat),libc_Xw16fspawnlpat);
EXPORT(__SYMw32(Xwfspawnlpat),libc_Xw32fspawnlpat);
EXPORT(__SYMw16(Xwfspawnlpeat),libc_Xw16fspawnlpeat);
EXPORT(__SYMw32(Xwfspawnlpeat),libc_Xw32fspawnlpeat);
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16execl(char16_t const *path, char16_t const *args, ...) { CAPTURE_ARGV(); libc_Xw16execve(path,(char16_t **)argv,libc_Xget_w16environ()); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32execl(char32_t const *path, char32_t const *args, ...) { CAPTURE_ARGV(); libc_Xw32execve(path,(char32_t **)argv,libc_Xget_w32environ()); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16execle(char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xw16execve(path,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32execle(char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xw32execve(path,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16execlp(char16_t const *file, char16_t const *args, ...) { CAPTURE_ARGV(); libc_Xw16execvpe(file,(char16_t **)argv,libc_Xget_w16environ()); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32execlp(char32_t const *file, char32_t const *args, ...) { CAPTURE_ARGV(); libc_Xw32execvpe(file,(char32_t **)argv,libc_Xget_w32environ()); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16execlpe(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xw16execvpe(file,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32execlpe(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xw32execvpe(file,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16fexecl(int exec_fd, char16_t const *args, ...) { CAPTURE_ARGV(); libc_Xw16fexecve(exec_fd,(char16_t **)argv,libc_Xget_w16environ()); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32fexecl(int exec_fd, char32_t const *args, ...) { CAPTURE_ARGV(); libc_Xw32fexecve(exec_fd,(char32_t **)argv,libc_Xget_w32environ()); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16fexecle(int exec_fd, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xw16fexecve(exec_fd,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32fexecle(int exec_fd, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xw32fexecve(exec_fd,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16fexeclat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); libc_Xw16fexecveat(dfd,path,(char16_t **)argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32fexeclat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); libc_Xw32fexecveat(dfd,path,(char32_t **)argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16fexecleat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); libc_Xw16fexecveat(dfd,path,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32fexecleat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); libc_Xw32fexecveat(dfd,path,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16fexeclpat(char16_t const *file, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); libc_Xw16fexecvpeat(file,(char16_t **)argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32fexeclpat(char32_t const *file, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); libc_Xw32fexecvpeat(file,(char32_t **)argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16fexeclpeat(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); libc_Xw16fexecvpeat(file,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32fexeclpeat(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); libc_Xw32fexecvpeat(file,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16spawnl(int mode, char16_t const *path, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_Xw16spawnve(mode,path,(char16_t **)argv,libc_Xget_w16environ()); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32spawnl(int mode, char32_t const *path, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_Xw32spawnve(mode,path,(char32_t **)argv,libc_Xget_w32environ()); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16spawnle(int mode, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xw16spawnve(mode,path,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32spawnle(int mode, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xw32spawnve(mode,path,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16spawnlp(int mode, char16_t const *file, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_Xw16spawnvpe(mode,file,(char16_t **)argv,libc_Xget_w16environ()); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32spawnlp(int mode, char32_t const *file, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_Xw32spawnvpe(mode,file,(char32_t **)argv,libc_Xget_w32environ()); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16spawnlpe(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xw16spawnvpe(mode,file,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32spawnlpe(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xw32spawnvpe(mode,file,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16fspawnl(int mode, int exec_fd, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_Xw16fspawnve(mode,exec_fd,(char16_t **)argv,libc_Xget_w16environ()); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32fspawnl(int mode, int exec_fd, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_Xw32fspawnve(mode,exec_fd,(char32_t **)argv,libc_Xget_w32environ()); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16fspawnle(int mode, int exec_fd, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xw16fspawnve(mode,exec_fd,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32fspawnle(int mode, int exec_fd, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xw32fspawnve(mode,exec_fd,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16fspawnlat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_Xw16fspawnveat(mode,dfd,path,(char16_t **)argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32fspawnlat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_Xw32fspawnveat(mode,dfd,path,(char32_t **)argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16fspawnleat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_Xw16fspawnveat(mode,dfd,path,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32fspawnleat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_Xw32fspawnveat(mode,dfd,path,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16fspawnlpat(int mode, char16_t const *file, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_Xw16fspawnvpeat(mode,file,(char16_t **)argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32fspawnlpat(int mode, char32_t const *file, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_Xw32fspawnvpeat(mode,file,(char32_t **)argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16fspawnlpeat(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_Xw16fspawnvpeat(mode,file,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32fspawnlpeat(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_Xw32fspawnvpeat(mode,file,(char32_t **)argv,(char32_t **)envp,flags); }




EXPORT(__KSYMw16(wfexecveat),libc_w16fexecveat);
CRT_WIDECHAR int LIBCCALL
libc_w16fexecveat(fd_t dfd,
                  char16_t const *path,
                  char16_t *const argv[],
                  char16_t *const envp[],
                  int flags) {
 LIBC_TRY {
  libc_Xw16fexecveat(dfd,path,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw16(wfexecvpeat),libc_w16fexecvpeat);
CRT_WIDECHAR int LIBCCALL
libc_w16fexecvpeat(char16_t const *file,
                   char16_t *const argv[],
                   char16_t *const envp[],
                   int flags) {
 LIBC_TRY {
  libc_Xw16fexecvpeat(file,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw16(wfspawnveat),libc_w16fspawnveat);
CRT_WIDECHAR pid_t LIBCCALL
libc_w16fspawnveat(int mode, fd_t dfd,
                   char16_t const *path,
                   char16_t *const argv[],
                   char16_t *const envp[],
                   int flags) {
 LIBC_TRY {
  return libc_Xw16fspawnveat(mode,dfd,path,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw16(wfspawnvpeat),libc_w16fspawnvpeat);
CRT_WIDECHAR pid_t LIBCCALL
libc_w16fspawnvpeat(int mode,
                    char16_t const *file,
                    char16_t *const argv[],
                    char16_t *const envp[],
                    int flags) {
 LIBC_TRY {
  return libc_Xw16fspawnvpeat(mode,file,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw32(wfexecveat),libc_w32fexecveat);
CRT_WIDECHAR int LIBCCALL
libc_w32fexecveat(fd_t dfd,
                  char32_t const *path,
                  char32_t *const argv[],
                  char32_t *const envp[],
                  int flags) {
 LIBC_TRY {
  libc_Xw32fexecveat(dfd,path,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw32(wfexecvpeat),libc_w32fexecvpeat);
CRT_WIDECHAR int LIBCCALL
libc_w32fexecvpeat(char32_t const *file,
                   char32_t *const argv[],
                   char32_t *const envp[],
                   int flags) {
 LIBC_TRY {
  libc_Xw32fexecvpeat(file,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw32(wfspawnveat),libc_w32fspawnveat);
CRT_WIDECHAR pid_t LIBCCALL
libc_w32fspawnveat(int mode, fd_t dfd,
                   char32_t const *path,
                   char32_t *const argv[],
                   char32_t *const envp[],
                   int flags) {
 LIBC_TRY {
  return libc_Xw32fspawnveat(mode,dfd,path,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw32(wfspawnvpeat),libc_w32fspawnvpeat);
CRT_WIDECHAR pid_t LIBCCALL
libc_w32fspawnvpeat(int mode,
                    char32_t const *file,
                    char32_t *const argv[],
                    char32_t *const envp[],
                    int flags) {
 LIBC_TRY {
  return libc_Xw32fspawnvpeat(mode,file,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}


EXPORT(__SYMw16(Xwfexecveat),libc_Xw16fexecveat);
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL
libc_Xw16fexecveat(fd_t dfd,
                   char16_t const *path,
                   char16_t *const argv[],
                   char16_t *const envp[],
                   int flags) {
 /* TODO */
 libc_error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(__SYMw16(Xwfexecvpeat),libc_Xw16fexecvpeat);
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL
libc_Xw16fexecvpeat(char16_t const *file,
                    char16_t *const argv[],
                    char16_t *const envp[],
                    int flags) {
 /* TODO */
 libc_error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(__SYMw16(Xwfspawnveat),libc_Xw16fspawnveat);
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL
libc_Xw16fspawnveat(int mode, fd_t dfd,
                    char16_t const *path,
                    char16_t *const argv[],
                    char16_t *const envp[],
                    int flags) {
 /* TODO */
 libc_error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(__SYMw16(Xwfspawnvpeat),libc_Xw16fspawnvpeat);
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL
libc_Xw16fspawnvpeat(int mode,
                     char16_t const *file,
                     char16_t *const argv[],
                     char16_t *const envp[],
                     int flags) {
 /* TODO: Search $PATH */
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(__SYMw32(Xwfexecveat),libc_Xw32fexecveat);
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL
libc_Xw32fexecveat(fd_t dfd,
                  char32_t const *path,
                  char32_t *const argv[],
                  char32_t *const envp[],
                  int flags) {
 /* TODO */
 libc_error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(__SYMw32(Xwfexecvpeat),libc_Xw32fexecvpeat);
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL
libc_Xw32fexecvpeat(char32_t const *file,
                    char32_t *const argv[],
                    char32_t *const envp[],
                    int flags) {
 /* TODO: Search $PATH */
 libc_error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(__SYMw32(Xwfspawnveat),libc_Xw32fspawnveat);
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL
libc_Xw32fspawnveat(int mode, fd_t dfd,
                    char32_t const *path,
                    char32_t *const argv[],
                    char32_t *const envp[],
                    int flags) {
 /* TODO */
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(__SYMw32(Xwfspawnvpeat),libc_Xw32fspawnvpeat);
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL
libc_Xw32fspawnvpeat(int mode,
                     char32_t const *file,
                     char32_t *const argv[],
                     char32_t *const envp[],
                     int flags) {
 /* TODO: Search $PATH */
 libc_error_throw(E_NOT_IMPLEMENTED);
}



DECL_END

#endif /* !GUARD_LIBS_LIBC_EXEC_C */
