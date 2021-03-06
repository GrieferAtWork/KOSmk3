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
#ifndef _BITS_GENERIC_UTMP_H
#define _BITS_GENERIC_UTMP_H 1
#define _BITS_UTMP_H 1

#include <__stdinc.h>
#include <paths.h>
#include <sys/time.h>
#include <sys/types.h>
#include <bits/wordsize.h>

__SYSDECL_BEGIN

/* The `struct utmp' type, describing entries in the utmp file.  GNU version.
   Copyright (C) 1993-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#define UT_LINESIZE    32
#define UT_NAMESIZE    32
#define UT_HOSTSIZE    256

#ifdef __CC__
/* The structure describing an entry in the database of previous logins. */
struct lastlog {
#ifdef __WORDSIZE_TIME64_COMPAT32
    int32_t  ll_time;
#else
    __time_t ll_time;
#endif
    char     ll_line[UT_LINESIZE];
    char     ll_host[UT_HOSTSIZE];
};


/* The structure describing the status of a terminated process.
 * This type is used in `struct utmp' below. */
struct exit_status {
    short int e_termination; /* Process termination status. */
    short int e_exit;        /* Process exit status. */
};

/* The structure describing an entry in the user accounting database. */
struct utmp {
    short int ut_type;            /* Type of login. */
    pid_t ut_pid;                 /* Process ID of login process. */
    char ut_line[UT_LINESIZE];    /* Devicename. */
    char ut_id[4];                /* Inittab ID. */
    union {
        char ut_name[UT_NAMESIZE];/* Username. */
        char ut_user[UT_NAMESIZE];/* Username. */
    };
    char ut_host[UT_HOSTSIZE];    /* Hostname for remote login. */
    struct exit_status ut_exit;   /* Exit status of a process marked as DEAD_PROCESS. */
    /* The ut_session and ut_tv fields must be the same size when compiled
     * 32- and 64-bit.  This allows data files and shared memory to be
     * shared between 32- and 64-bit applications. */
#ifdef __WORDSIZE_TIME64_COMPAT32
    int32_t         ut_session;   /* Session ID, used for windowing. */
    union {
        int32_t     ut_time;      /* Time entry was made. */
        int32_t     ut_xtime;     /* Time entry was made. */
        struct {
            int32_t tv_sec;       /* Seconds. */
            int32_t tv_usec;      /* Microseconds. */
        } ut_tv;                  /* Time entry was made. */
    };
#else
    long int        ut_session;    /* Session ID, used for windowing. */
    union {
        int32_t     ut_time;       /* Time entry was made. */
        int32_t     ut_xtime;      /* Time entry was made. */
        struct __timeval32 ut_tv; /* Time entry was made. */
    };
#endif
    union {
        int32_t     ut_addr;       /* Internet address of remote host. */
        int32_t     ut_addr_v6[4]; /* Internet address of remote host. */
    };
    char __glibc_reserved[20];    /* Reserved for future use. */
};
#endif /* __CC__ */

/* Values for the `ut_type' field of a `struct utmp'. */
#define EMPTY           0 /* No valid user accounting information. */
#define RUN_LVL         1 /* The system's runlevel. */
#define BOOT_TIME       2 /* Time of system boot. */
#define NEW_TIME        3 /* Time after system clock changed. */
#define OLD_TIME        4 /* Time when system clock changed. */
#define INIT_PROCESS    5 /* Process spawned by the init process. */
#define LOGIN_PROCESS   6 /* Session leader of a logged in user. */
#define USER_PROCESS    7 /* Normal process. */
#define DEAD_PROCESS    8 /* Terminated process. */
#define ACCOUNTING      9

/* Old Linux name for the EMPTY type. */
#define UT_UNKNOWN      EMPTY

/* Tell the user that we have a modern system with
 * UT_HOST, UT_PID, UT_TYPE, UT_ID and UT_TV fields. */
#define _HAVE_UT_TYPE   1
#define _HAVE_UT_PID    1
#define _HAVE_UT_ID     1
#define _HAVE_UT_TV     1
#define _HAVE_UT_HOST   1

__SYSDECL_END

#endif /* !_BITS_GENERIC_UTMP_H */
