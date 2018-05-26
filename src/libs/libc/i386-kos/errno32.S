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
#define _KOS_SOURCE 2

#include <hybrid/compiler.h>
#include <hybrid/asm.h>
#include <asm/cfi.h>
#include <kos/thread.h>
#include "../libc.h"


/* struct task_segment { */
#define ts_self        TASK_SEGMENT_OFFSETOF_SELF        /* struct task_segment  *ts_self; */
#define ts_xcurrent    TASK_SEGMENT_OFFSETOF_XCURRENT    /* struct exception_info ts_xcurrent; */
#define ts_eformat     TASK_SEGMENT_OFFSETOF_EFORMAT     /* u8                    ts_eformat; */
#define ts_errno       TASK_SEGMENT_OFFSETOF_ERRNO       /* __UINT32_TYPE__       ts_errno; */
#define ts_dos_errno   TASK_SEGMENT_OFFSETOF_DOS_ERRNO   /* __UINT32_TYPE__       ts_dos_errno; */
#define ts_nt_errno    TASK_SEGMENT_OFFSETOF_NT_ERRNO    /* __UINT32_TYPE__       ts_nt_errno; */
/* }; */


DEFINE_INTERN(libc_errno_dos2kos)
DEFINE_INTERN(libc_errno_nt2kos)
DEFINE_INTERN(libc_errno_nt2dos)
DEFINE_INTERN(libc_errno_kos2dos)
DEFINE_INTERN(libc_errno_dos2nt)
DEFINE_INTERN(libc_errno_kos2nt)

.section .text.crt.errno
.cfi_startproc
PRIVATE_ENTRY(setformat_kos)
	cmpb  $(__X86_TASK_ERRNO_FKOS), %taskseg:ts_eformat
	je    1f
	cmpb  $(__X86_TASK_ERRNO_FDOS), %taskseg:ts_eformat
	jne   2f
	pushl_cfi %taskseg:ts_dos_errno
	call  libc_errno_dos2kos
	jmp   3f
	.cfi_adjust_cfa_offset -4
2:	pushl_cfi %taskseg:ts_nt_errno
	call  libc_errno_nt2kos
3:	addl  $4,                 %esp
	.cfi_adjust_cfa_offset -4
	movl  %eax,               %taskseg:ts_errno
	movb  $(__X86_TASK_ERRNO_FKOS), %taskseg:ts_eformat
1:	ret
SYMEND(setformat_kos)
PRIVATE_ENTRY(setformat_dos)
	cmpb  $(__X86_TASK_ERRNO_FDOS), %taskseg:ts_eformat
	je    1f
	cmpb  $(__X86_TASK_ERRNO_FKOS), %taskseg:ts_eformat
	jne   2f
	pushl_cfi %taskseg:ts_errno
	call  libc_errno_kos2dos
	jmp   3f
	.cfi_adjust_cfa_offset -4
2:	pushl_cfi %taskseg:ts_nt_errno
	call  libc_errno_nt2dos
3:	addl  $4,                 %esp
	.cfi_adjust_cfa_offset -4
	movl  %eax,               %taskseg:ts_dos_errno
	movb  $(__X86_TASK_ERRNO_FDOS), %taskseg:ts_eformat
1:	ret
SYMEND(setformat_dos)
PRIVATE_ENTRY(setformat_nt)
	cmpb  $(__X86_TASK_ERRNO_FNT),  %taskseg:ts_eformat
	je    1f
	cmpb  $(__X86_TASK_ERRNO_FKOS), %taskseg:ts_eformat
	jne   2f
	pushl_cfi %taskseg:ts_errno
	call  libc_errno_kos2nt
	jmp   3f
	.cfi_adjust_cfa_offset -4
2:	pushl_cfi %taskseg:ts_nt_errno
	call  libc_errno_dos2nt
3:	addl  $4,                 %esp
	.cfi_adjust_cfa_offset -4
	movl  %eax,               %taskseg:ts_nt_errno
	movb  $(__X86_TASK_ERRNO_FNT),  %taskseg:ts_eformat
1:	ret
SYMEND(setformat_nt)



INTERN_ENTRY(libc_errno)
	call  setformat_kos
	movl  $(ts_errno), %eax
	addl  %taskseg:ts_self, %eax
	ret
SYMEND(libc_errno)

INTERN_ENTRY(libc_dos_errno)
	call  setformat_dos
	movl  $(ts_dos_errno), %eax
	addl  %taskseg:ts_self, %eax
	ret
SYMEND(libc_dos_errno)

INTERN_ENTRY(libc_nt_errno)
	call  setformat_nt
	movl  $(ts_nt_errno), %eax
	addl  %taskseg:ts_self, %eax
	ret
SYMEND(libc_nt_errno)

INTERN_ENTRY(libc_geterrno)
	call  setformat_kos
	movl  %taskseg:ts_errno, %eax
	ret
SYMEND(libc_geterrno)

INTERN_ENTRY(libc_dos_geterrno)
	call  setformat_dos
	movl  %taskseg:ts_dos_errno, %eax
	ret
SYMEND(libc_dos_geterrno)

INTERN_ENTRY(libc_nt_geterrno)
	call  setformat_nt
	movl  %taskseg:ts_nt_errno, %eax
	ret
SYMEND(libc_nt_geterrno)

INTERN_ENTRY(libc_seterrno)
	movl  4(%esp), %eax
	movl  %eax, %taskseg:ts_errno
	movb  $(__X86_TASK_ERRNO_FKOS), %taskseg:ts_eformat
	ret
SYMEND(libc_seterrno)

INTERN_ENTRY(libc_dos_seterrno)
	movl  4(%esp), %eax
	movl  %eax, %taskseg:ts_dos_errno
	movb  $(__X86_TASK_ERRNO_FDOS), %taskseg:ts_eformat
	ret
SYMEND(libc_dos_seterrno)

INTERN_ENTRY(libc_nt_seterrno)
	movl  4(%esp), %eax
	movl  %eax, %taskseg:ts_nt_errno
	movb  $(__X86_TASK_ERRNO_FNT), %taskseg:ts_eformat
	ret
SYMEND(libc_nt_seterrno)

INTERN_ENTRY(libc_gettid)
	movl  %taskseg:TASK_SEGMENT_OFFSETOF_TID, %eax
	ret
SYMEND(libc_gettid)

INTERN_ENTRY(libc_current)
	movl  %taskseg:ts_self, %eax
	ret
SYMEND(libc_current)
INTERN_ENTRY(libc_procenv)
	movl  %taskseg:TASK_SEGMENT_OFFSETOF_PROCESS, %eax
	ret
SYMEND(libc_procenv)
.cfi_endproc

EXPORT(__DSYM(_errno),             libc_dos_errno);    /* DOS */
EXPORT(__get_doserrno,             libc_dos_geterrno); /* DOS */
EXPORT(__set_doserrno,             libc_dos_seterrno); /* DOS */
EXPORT(__doserrno,                 libc_nt_errno);     /* NT */
EXPORT(__get_nterrno,              libc_nt_geterrno);  /* NT */
EXPORT(__set_nterrno,              libc_nt_seterrno);  /* NT */
EXPORT(__errno,                    libc_errno);        /* Cygwin */
EXPORT(__errno_location,           libc_errno);        /* GLibC */
EXPORT(_errno,                     libc_errno);        /* KOS (Mk2 defined this one?) */
EXPORT(__get_errno,                libc_geterrno);     /* KOS */
EXPORT(__set_errno,                libc_seterrno);     /* KOS */

/* TLS control functions. */
EXPORT(__current,                  libc_current);
EXPORT(__procenv,                  libc_procenv);
EXPORT(gettid,                     libc_gettid);
EXPORT(__threadid,                 libc_gettid);
EXPORT(__threadhandle,             libc_gettid); /* XXX: Maybe someday return a handle? */



