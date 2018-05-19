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
#ifndef GUARD_LIBS_LIBC_TLS_C
#define GUARD_LIBS_LIBC_TLS_C 1

#include "libc.h"
#include "rtl.h"
#include "tls.h"
#include "system.h"
#include "malloc.h"
#include "dl.h"
#include <except.h>
#include <kos/thread.h>
#include <kos/dl.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/align.h>
#include <syslog.h>

DECL_BEGIN

INTERN void *FCALL libc_dynamic_tls_addr(TLS_index *__restrict index) {
 uintptr_t handle;
 struct dynamic_tls *dyntls;
 struct module_tls_info info;
 handle = index->ti_moduleid;
 dyntls = GET_DYNAMIC_TLS();
 while (dyntls) {
  if (dyntls->dt_module == handle)
      return dyntls->dt_data + index->ti_tlsoffset;
  dyntls = dyntls->dt_next;
 }
 /* Query TLS information for the module in question. */
 if (libc_Xxdlmodule_info((void *)index->ti_moduleid,
                           MODULE_INFO_CLASS_TLS,
                          &info,sizeof(info)) != sizeof(info))
     error_throw(E_NOT_IMPLEMENTED);
 /* Allocate a new dynamic TLS segment. */
 dyntls = (struct dynamic_tls *)libc_memcalign_offset(MAX(info.ti_tls_align,
                                                          COMPILER_ALIGNOF(struct dynamic_tls)),
                                                      offsetof(struct dynamic_tls,dt_data)+
                                                      info.ti_tls_size,
                                                      offsetof(struct dynamic_tls,dt_data));
 /* Copy the TLS template into the dynamic TLS segment. */
 libc_memcpy(dyntls->dt_data,info.ti_template_base,info.ti_template_size);
 dyntls->dt_module = index->ti_moduleid;
 dyntls->dt_next   = GET_DYNAMIC_TLS();
 SET_DYNAMIC_TLS(dyntls);
 return dyntls->dt_data + index->ti_tlsoffset;
}

INTERN void LIBCCALL libc_free_dynamic_tls(void) {
 struct dynamic_tls *dyntls,*next;
 struct task_segment *me = libc_current();
 dyntls = me->ts_tls;
 while (dyntls) {
  next = dyntls->dt_next;
  libc_free(dyntls);
  dyntls = next;
 }
 /* Free the thread's read locks */
 libc_free(me->ts_locks);
}

EXPORT(exit_thread,libc_exit_thread);
INTERN ATTR_NORETURN void LIBCCALL
libc_exit_thread(int exit_code) {
 libc_free_dynamic_tls();
 /* NOTE: This is not the exit() function (that one calls SYS_exit_group)
  *       Due to historic reasons, the `SYS_exit' system call only terminates
  *       the calling thread, and the `SYS_exit_group' system call terminates
  *       the calling process! */
 sys_exit(exit_code);
}

EXPORT(_endthreadex,libc_endthreadex);
#if __SIZEOF_INT__ == 4
DEFINE_INTERN_ALIAS(libc_endthreadex,libc_exit_thread);
#else
CRT_DOS void LIBCCALL libc_endthreadex(u32 exitcode) {
 libc_exit_thread((int)exitcode);
}
#endif


DECL_END

#endif /* !GUARD_LIBS_LIBC_TLS_C */
