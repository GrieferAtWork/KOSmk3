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
#ifndef GUARD_APPS_KERNCTL_MAIN_C
#define GUARD_APPS_KERNCTL_MAIN_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/kernctl.h>
#include <kos/types.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

DECL_BEGIN

#define CTL_NAME_MAXLEN  8
struct ctl {
    char name[CTL_NAME_MAXLEN];
    u32  cmd;
};

PRIVATE struct ctl const ctls[] = {
    { "leak",   KERNEL_CONTROL_DBG_DUMP_LEAKS },
    { "hcheck", KERNEL_CONTROL_DBG_CHECK_PADDING },
    { "hval",   KERNEL_CONTROL_DBG_HEAP_VALIDATE },
    { "ton",    KERNEL_CONTROL_TRACE_SYSCALLS_ON },
    { "toff",   KERNEL_CONTROL_TRACE_SYSCALLS_OFF },
};


int main(int argc, char *argv[]) {
 unsigned int i;
 char *cmd;
 if (argc < 2) {
  fprintf(stderr,"Usage: %s (--help | COMMAND)\n",
          program_invocation_short_name);
  return 1;
 }
 cmd = argv[1];
 if (!strcmp(cmd,"--help")) {
  fprintf(stderr,"Usage: %s (--help | COMMAND)\n",
          program_invocation_short_name);
  fprintf(stderr,"Execute kernel control commands.\n");
  fprintf(stderr,"COMMAND should be one of:\n");
  for (i = 0; i < COMPILER_LENOF(ctls); ++i) {
   fprintf(stderr,"\t%s\n",ctls[i].name);
  }
  return 0;
 }
 for (i = 0; i < COMPILER_LENOF(ctls); ++i) {
  unsigned int count = 10000;
  if (strcmp(ctls[i].name,cmd) != 0) continue;
  while (count--) Xkernctl(ctls[i].cmd);
  return 0;
 }
 fprintf(stderr,"Unknown command %q\n",cmd);
 fprintf(stderr,"Type `%s --help' for a list of known commands\n",
         program_invocation_short_name);
 return 1;
}


DECL_END

#endif /* !GUARD_APPS_KERNCTL_MAIN_C */
