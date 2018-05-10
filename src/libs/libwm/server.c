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
#include <string.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include "libwm.h"

DECL_BEGIN

/* These are populated by `libwm_init()'
 * Pre-initialize them to -1 to cause E_INVALID_HANDLE
 * exceptions if library users attempt to use API functions
 * before the library has actually been initialized. */
INTERN fd_t libwms_socket = -1;


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


INTERN unsigned int WMCALL
libwms_sendrequest(struct wms_request *__restrict req) {
 unsigned int result = libwms_gentoken();
 req->r_echo = result;
 /* Send the request */
 if (Xsend(libwms_socket,(byte_t *)req,
           sizeof(struct wms_request),0) !=
           sizeof(struct wms_request))
     return 0;
 return result;
}

INTERN void WMCALL
libwms_recvresponse(unsigned int token,
                    struct wms_response *__restrict resp) {
 for (;;) {
  struct pollfd p;
  p.fd     = libwms_socket;
  p.events = POLLIN;
  if (!Xpoll(&p,1,2000))
       error_throw(E_NOT_IMPLEMENTED);
  if (Xrecv(libwms_socket,(byte_t *)resp,sizeof(struct wms_response),0) !=
                                         sizeof(struct wms_response))
       error_throw(E_NOT_IMPLEMENTED);
  libwms_handle(resp);
  if (resp->r_echo == token)
      break; /* This is what we were waiting for. */
 }
 /* Interpret special sever responses. */
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

INTERN fd_t WMCALL
libwms_recvresponse_fd(unsigned int token,
                       struct wms_response *__restrict resp) {
 fd_t EXCEPT_VAR result = -1;
 TRY {
  for (;;) {
   size_t total;
   struct pollfd p;
   struct msghdr msg;
   struct iovec iov[1];
   union {
    struct cmsghdr hdr;
    byte_t buf[CMSG_SPACE(sizeof(fd_t))];
   } result_buffer;
   p.fd     = libwms_socket;
   p.events = POLLIN;
   if (!Xpoll(&p,1,2000))
        error_throw(E_NOT_IMPLEMENTED);
   iov[0].iov_base              = resp;
   iov[0].iov_len               = sizeof(struct wms_response);
   msg.msg_name                 = NULL;
   msg.msg_namelen              = 0;
   msg.msg_iov                  = iov;
   msg.msg_iovlen               = 1;
   msg.msg_control              = result_buffer.buf;
   msg.msg_controllen           = sizeof(result_buffer);
   msg.msg_flags                = 0;
#if 0 /* This is filled in by the kernel! */
   result_buffer.hdr.cmsg_len   = CMSG_SPACE(sizeof(fd_t));
   result_buffer.hdr.cmsg_level = SOL_SOCKET;
   result_buffer.hdr.cmsg_type  = SCM_RIGHTS;
   *(fd_t *)CMSG_DATA(&result_buffer.hdr) = -1;
#endif
   total = Xrecvmsg(libwms_socket,&msg,MSG_CMSG_CLOEXEC);
   if (CMSG_FIRSTHDR(&msg) == &result_buffer.hdr &&
       result_buffer.hdr.cmsg_level == SOL_SOCKET &&
       result_buffer.hdr.cmsg_type  == SCM_RIGHTS) {
    if (result >= 0) close(result);
    result = *(fd_t *)CMSG_DATA(&result_buffer.hdr);
   }
   if (total != sizeof(struct wms_response)) {
    /* EOF (the server may have crashed?) */
    error_throwf(E_NET_ERROR,ERROR_NET_SHUTDOWN);
   }
   libwms_handle(resp);
   if (resp->r_echo == token)
       break; /* This is what we were waiting for. */
   /* Process unrelated response packets by themself. */
   if (result >= 0) close(result),result = -1;
   total = 0;
  }
  /* Make sure that we actually received a file descriptor. */
  if (result < 0)
      error_throw(E_NOT_IMPLEMENTED);

  /* Interpret special sever responses. */
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
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  if (result >= 0)
      close(result);
  error_rethrow();
 }
 return result;
}



DECL_END

#endif /* !GUARD_LIBS_LIBWM_SERVER_C */
