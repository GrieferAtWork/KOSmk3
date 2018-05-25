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
#ifndef GUARD_LIBS_LIBC_I386_KOS_ENTRY_C
#define GUARD_LIBS_LIBC_I386_KOS_ENTRY_C 1
#define _GNU_SOURCE 1

#include "../libc.h"
#include "../entry.h"
#include "../sched.h"
#include "../exit.h"
#include <hybrid/compiler.h>
#include <kos/environ.h>
#include <kos/thread.h>
#include <hybrid/asm.h>
#include <syslog.h>
#include <unistd.h>
#include <hybrid/section.h>

DECL_BEGIN

struct envdata;

STATIC_ASSERT(offsetof(struct task_segment,ts_process) ==
              TASK_SEGMENT_OFFSETOF_PROCESS);

#ifndef CONFIG_LIBC_HAVE_ARCH_ENTRY
PRIVATE char *empty_vector[] = { NULL };
EXPORT(__entry1,libc_entry);
INTERN ATTR_HOTTEXT ATTR_NORETURN
void (FCALL libc_entry)(pmain main) {
 struct process_environ *proc;
 int exit_status;
 /* Load the process environment map. */
 proc = libc_procenv();
 if likely(proc) {
  environ = proc->pe_envp;
  exit_status = (*main)(proc->pe_argc,
                        proc->pe_argv,
                        proc->pe_envp);
 } else {
  /* Shouldn't ~really~ happen... */
  environ = empty_vector;
  exit_status = (*main)(0,
                        empty_vector,
                        empty_vector);
 }
 libc_exit(exit_status);
}
#endif /* !CONFIG_LIBC_HAVE_ARCH_ENTRY1 */

EXPORT(__entry,libc_old_entry);
INTERN ATTR_HOTTEXT ATTR_NORETURN
void (FCALL libc_old_entry)(void *__restrict env, pmain main) {
 libc_entry(main);
}

DECL_END

#endif /* !GUARD_LIBS_LIBC_I386_KOS_ENTRY_C */
