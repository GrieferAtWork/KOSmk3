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
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <syslog.h>
#include <except.h>

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
    { "insmod", KERNEL_CONTROL_INSMOD },
    { "delmod", KERNEL_CONTROL_DELMOD },
    { "cc",     KERNEL_CONTROL_CLEARCACHES },
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
  u32 id;
  if (strcmp(ctls[i].name,cmd) != 0) continue;
  id = ctls[i].cmd;
  switch (id) {

  case KERNEL_CONTROL_INSMOD:
  case KERNEL_CONTROL_DELMOD:
   if (argc < 3) goto more_args;
   /* NOTE: The 3rd argument to `KERNEL_CONTROL_INSMOD' is optional. */
   return Xkernctl(id,argv[2],argv[3]);

  default:
   return Xkernctl(id);
  }
 }
 if (!strcmp(cmd,"fault")) {
  int *ptr = (int *)42;
  while (++(*ptr++));
  return 0;
 }
 if (!strcmp(cmd,"alloc")) {
  TRY {
   for (;;) {
    Xmmap(NULL,16*PAGESIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);
    /*printf("%p\n",p);*/
   }
  } CATCH (E_BADALLOC) {
   error_printf("Catch E_BADALLOC caused by mmap()\n"
                "Thrown from here:\n");
   return 0;
  }
 }
 if (!strcmp(cmd,"ill")) {
  asm("hlt");
  return 0;
 }

 fprintf(stderr,"Unknown command %q\n",cmd);
 fprintf(stderr,"Type `%s --help' for a list of known commands\n",
         program_invocation_short_name);
 return 1;
more_args:
 fprintf(stderr,"Command %q requires additional arguments\n",cmd);
 return 1;
}


DECL_END

#endif /* !GUARD_APPS_KERNCTL_MAIN_C */
