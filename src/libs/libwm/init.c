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
#ifndef GUARD_LIBS_LIBWM_INIT_C
#define GUARD_LIBS_LIBWM_INIT_C 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <wm/api.h>
#include <sys/socket.h>

#include <string.h>
#include <unistd.h>
#include <sys/un.h>

#include "libwm.h"

DECL_BEGIN

__asm__(
".pushsection .data\n\t"
".global __stack_chk_fail_local\n\t"
".hidden __stack_chk_fail_local\n\t"
"__stack_chk_fail_local:\n\t"
"	jmp  __stack_chk_guard\n\t"
".size __stack_chk_fail_local, . - __stack_chk_fail_local\n\t"
".popsection"
);


DEFINE_PUBLIC_ALIAS(wm_init,libwm_init);
INTERN void WMCALL libwm_init(void) {
 struct sockaddr_un addr;
 /* Create a new unix domain socket. */
 libwms_socket = Xsocket(AF_UNIX,SOCK_STREAM,AF_UNIX);
 TRY {
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path,"/dev/wms");

  /* Connect to the window server running under `/dev/wms' */
  Xconnect(libwms_socket,(struct sockaddr *)&addr,sizeof(addr));
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  close(libwms_socket);
  error_rethrow();
 }
}

DEFINE_PUBLIC_ALIAS(wm_fini,libwm_fini);
INTERN ATTR_NOTHROW void WMCALL libwm_fini(void) {
 /* TODO: Close all windows that are still open. */
 close(libwms_socket);
 libwms_socket = -1;
}



DECL_END

#endif /* !GUARD_LIBS_LIBWM_INIT_C */
