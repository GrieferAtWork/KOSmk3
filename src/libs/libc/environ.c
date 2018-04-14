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
#ifndef GUARD_LIBS_LIBC_ENVIRON_C
#define GUARD_LIBS_LIBC_ENVIRON_C 1

#include "libc.h"
#include "widechar.h"
#include "errno.h"
#include "unistd.h"
#include "sync.h"
#include "rtl.h"
#include <kos/sched/mutex.h>

#include <errno.h>
#include <syslog.h>


DECL_BEGIN

PRIVATE DEFINE_MUTEX(environ_lock);
INTDEF int LIBCCALL libc_env_acquire(void) { return libc_mutex_get_timed64(&environ_lock,NULL); }
INTDEF void LIBCCALL libc_env_release(void) { libc_mutex_put(&environ_lock); }

PUBLIC char **environ = NULL;
INTERN char16_t **w16environ = NULL;
INTERN char16_t **w32environ = NULL;


INTERN char ***LIBCCALL libc_p_environ(void) { return &environ; }
CRT_WIDECHAR char16_t **LIBCCALL libc_get_w16environ(void) { return NULL; /* TODO */ }
CRT_WIDECHAR char32_t **LIBCCALL libc_get_w32environ(void) { return NULL; /* TODO */ }
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL char16_t **LIBCCALL libc_Xget_w16environ(void) { libc_error_throw(E_NOT_IMPLEMENTED); }
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL char32_t **LIBCCALL libc_Xget_w32environ(void) { libc_error_throw(E_NOT_IMPLEMENTED); }


INTERN char *LIBCCALL kimpl_libc_getenv(char const *name) { return NULL; } /* TODO */
INTERN char *LIBCCALL dimpl_libc_getenv(char const *name) { return NULL; } /* TODO */
INTERN int LIBCCALL kimpl_libc_setenv(char const *name, char const *value, int replace) { libc_seterrno(ENOSYS); return -1; } /* TODO */
INTERN int LIBCCALL dimpl_libc_setenv(char const *name, char const *value, int replace) { libc_seterrno(ENOSYS); return -1; } /* TODO */
INTERN int LIBCCALL kimpl_libc_putenv(char *string) { libc_seterrno(ENOSYS); return -1; } /* TODO */
INTERN int LIBCCALL dimpl_libc_putenv(char *string) { libc_seterrno(ENOSYS); return -1; } /* TODO */
INTERN int LIBCCALL libc_clearenv(void) { libc_seterrno(ENOSYS); return -1; } /* TODO */
INTERN int LIBCCALL libc_unsetenv(char const *name) { libc_seterrno(ENOSYS); return -1; } /* TODO */


INTERN char *LIBCCALL libc_getenv(char const *name) { return LIBC_DOSMODE_ENABLED() ? dimpl_libc_getenv(name) : kimpl_libc_getenv(name); }
INTERN int LIBCCALL libc_setenv(char const *name, char const *value, int replace) { return LIBC_DOSMODE_ENABLED() ? dimpl_libc_setenv(name,value,replace) : kimpl_libc_setenv(name,value,replace); }
INTERN int LIBCCALL libc_putenv(char *string) { return LIBC_DOSMODE_ENABLED() ? dimpl_libc_putenv(string) : kimpl_libc_putenv(string); }
INTERN char *LIBCCALL libc_dos_getenv(char const *name) { return LIBC_DOSMODE_DISABLED() ? kimpl_libc_getenv(name) : dimpl_libc_getenv(name); }
INTERN int LIBCCALL libc_dos_setenv(char const *name, char const *value, int replace) { return LIBC_DOSMODE_DISABLED() ? kimpl_libc_setenv(name,value,replace) : dimpl_libc_setenv(name,value,replace); }
INTERN int LIBCCALL libc_dos_putenv(char *string) { return LIBC_DOSMODE_DISABLED() ? kimpl_libc_putenv(string) : dimpl_libc_putenv(string); }

INTERN derrno_t LIBCCALL libd_putenv_s(char const *name, char const *value) { return libc_setenv(name,value,1) ? libc_dos_geterrno() : 0; }
INTERN derrno_t LIBCCALL libd_dos_putenv_s(char const *name, char const *value) { return libc_dos_setenv(name,value,1) ? libc_dos_geterrno() : 0; }
EXPORT(__p__environ,               libc_p_environ);
EXPORT(clearenv,                   libc_clearenv);
EXPORT(unsetenv,                   libc_unsetenv);
EXPORT(__KSYM(getenv),             libc_getenv);
EXPORT(__DSYM(getenv),             libc_dos_getenv);
EXPORT(__KSYM(putenv),             libc_putenv);
EXPORT(__DSYM(_putenv),            libc_dos_putenv);
EXPORT(__KSYM(putenv_s),           libd_putenv_s);
EXPORT(__DSYM(_putenv_s),          libd_dos_putenv_s);
EXPORT(__KSYM(setenv),             libc_setenv);
EXPORT(__DSYM(setenv),             libc_dos_setenv);

EXPORT_STRONG(_environ,environ);
EXPORT_STRONG(__environ,environ);

DECL_END

#endif /* !GUARD_LIBS_LIBC_ENVIRON_C */
