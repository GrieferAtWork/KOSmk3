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
#ifndef GUARD_LIBS_LIBWM_SERVER_C
#define GUARD_LIBS_LIBWM_SERVER_C 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <wm/api.h>
#include <wm/server.h>
#include <wm/window.h>
#include <unistd.h>
#include <except.h>
#include <sys/poll.h>

#include "libwm.h"

DECL_BEGIN

/* These are populated by `libwm_init()'
 * Pre-initialize them to -1 to cause E_INVALID_HANDLE
 * exceptions if library users attempt to use API functions
 * before the library has actually been initialized. */
INTERN fd_t libwms_requestfd = -1;
INTERN fd_t libwms_resposefd = -1;


PRIVATE unsigned int libwms_token = 1;
PRIVATE unsigned int WMCALL libwms_gentoken(void) {
 unsigned int result;
 /* Don't use ZERO(0) as a token, as that one's
  * used as echo value for event-related responses. */
 while ((result = ATOMIC_FETCHINC(libwms_token)) == 0);
 return result;
}

INTERN void WMCALL
libwms_handle(struct wms_response *__restrict resp) {
 if (resp->r_answer != WMS_RESPONSE_EVENT)
     return; /* Ignore unrelated responses */
 /* Deal with the response that we received. */
 resp->r_event.e_common.c_window = libwm_window_fromid(resp->r_event.e_common.c_winid);
 if unlikely(!resp->r_event.e_common.c_window)
    return; /* Unused window ID (might happen due to race
             * conditions related to the server-client model) */
 TRY {
  libwm_event_dealwith(&resp->r_event);
 } FINALLY {
  libwm_window_decref(resp->r_event.e_common.c_window);
 }
}


DEFINE_PUBLIC_ALIAS(wms_sendrequest,libwms_sendrequest);
INTERN unsigned int WMCALL
libwms_sendrequest(struct wms_request *__restrict req) {
 size_t part = 0;
 unsigned int result = libwms_gentoken();
 req->r_echo = result;
 /* Send the request */
 do part += Xwrite(libwms_requestfd,(byte_t *)req + part,
                   sizeof(struct wms_request) - part);
 while (part < sizeof(struct wms_request));
 return result;
}

DEFINE_PUBLIC_ALIAS(wms_recvresponse,libwms_recvresponse);
INTERN void WMCALL
libwms_recvresponse(unsigned int token,
                    struct wms_response *__restrict resp) {
 for (;;) {
  size_t part = 0;
  struct pollfd p;
  p.fd     = libwms_resposefd;
  p.events = POLLIN;
  if (!Xpoll(&p,1,2000))
       error_throw(E_INVALID_ARGUMENT);
  do part += Xread(libwms_resposefd,(byte_t *)resp + part,
                   sizeof(struct wms_response) - part);
  while (part < sizeof(struct wms_response));
  if (resp->r_echo == token)
      break; /* This is what we were waiting for. */
  /* Process unrelated response packets by themself. */
  libwms_handle(resp);
 }
 /* Interpret special sever resposenses. */
 switch (resp->r_answer) {

 case WMS_RESPONSE_FAILED:
  /* Rethrow the error that caused the failure. */
  error_throw_ex(resp->r_failed.f_except.e_code,
                &resp->r_failed.f_except);
  break;

 case WMS_RESPONSE_BADCMD:
  /* The command isn't implemented. */
  error_throw(E_NOT_IMPLEMENTED);
  break;

 default: break;
 }
}



DECL_END

#endif /* !GUARD_LIBS_LIBWM_SERVER_C */
