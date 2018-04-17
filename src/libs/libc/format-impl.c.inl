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
#ifdef __INTELLISENSE__
#include "format.c"
#define CHARACTER_TYPE  1
#endif

#include "stdio/file.h"
#include "malloc.h"
#include "unicode.h"
#include "unistd.h"

#include <limits.h>
#include <hybrid/align.h>
#include <hybrid/minmax.h>
#include <unicode.h>
#include <stdio.h>
#include <wchar.h>

#if CHARACTER_TYPE == CHARACTER_TYPE_CHAR
#define T_char              char
#define T_stringprinter     stringprinter
#define T_pformatprinter    pformatprinter
#define T_pformatgetc       pformatgetc
#define T_pformatungetc     pformatungetc
#define T_buffer            buffer
#define libc_stringX(x)     libc_string##x
#define libc_XstringX(x)    libc_Xstring##x
#define libc_XwX(x)         libc_X##x
#define libc_wX(x)          libc_##x
#define libc_vwX(x)         libc_v##x
#define libc_swX(x)         libc_s##x
#define libc_vswX(x)        libc_vs##x
#define libc_snwX(x)        libc_sn##x
#define libc_vsnwX(x)       libc_vsn##x
#define libc_scwX(x)        libc_sc##x
#define libc_vscwX(x)       libc_vsc##x
#define libc_aswX(x)        libc_as##x
#define libc_vaswX(x)       libc_vas##x
#define libc_fwX(x)         libc_f##x
#define libc_vfwX(x)        libc_vf##x
#define libc_strX(x)        libc_str##x
#define libc_vstrX(x)       libc_vstr##x
#define libc_XstrX(x)       libc_Xstr##x
#define libc_XvstrX(x)      libc_Xvstr##x
#define libc_format_cX(x)   libc_format_##x
#define libc_format_vcX(x)  libc_format_v##x
#define libc_format_strX(x) libc_format_str##x
#define libd_stringX(x)     libd_string##x
#define libd_XstringX(x)    libd_Xstring##x
#define libd_XwX(x)         libd_X##x
#define libd_wX(x)          libd_##x
#define libd_vwX(x)         libd_v##x
#define libd_swX(x)         libd_s##x
#define libd_vswX(x)        libd_vs##x
#define libd_snwX(x)        libd_sn##x
#define libd_vsnwX(x)       libd_vsn##x
#define libd_scwX(x)        libd_sc##x
#define libd_vscwX(x)       libd_vsc##x
#define libd_aswX(x)        libd_as##x
#define libd_vaswX(x)       libd_vas##x
#define libd_fwX(x)         libd_f##x
#define libd_vfwX(x)        libd_vf##x
#define libd_strX(x)        libd_str##x
#define libd_vstrX(x)       libd_vstr##x
#define libd_XstrX(x)       libd_Xstr##x
#define libd_XvstrX(x)      libd_Xvstr##x
#define libd_format_cX(x)   libd_format_##x
#define libd_format_vcX(x)  libd_format_v##x
#elif CHARACTER_TYPE == CHARACTER_TYPE_CHAR16
#define T_char              char16_t
#define T_stringprinter     string16printer
#define T_pformatprinter    pw16formatprinter
#define T_pformatgetc       pw16formatgetc
#define T_pformatungetc     pw16formatungetc
#define T_buffer            w16buffer
#define T_printer           w16printer
#define libc_stringX(x)     libc_string16##x
#define libc_XstringX(x)    libc_Xstring16##x
#define libc_XwX(x)         libc_Xw16##x
#define libc_wX(x)          libc_w16##x
#define libc_vwX(x)         libc_vw16##x
#define libc_swX(x)         libc_sw16##x
#define libc_vswX(x)        libc_vsw16##x
#define libc_snwX(x)        libc_snw16##x
#define libc_vsnwX(x)       libc_vsnw16##x
#define libc_scwX(x)        libc_scw16##x
#define libc_vscwX(x)       libc_vscw16##x
#define libc_aswX(x)        libc_asw16##x
#define libc_vaswX(x)       libc_vasw16##x
#define libc_fwX(x)         libc_fw16##x
#define libc_vfwX(x)        libc_vfw16##x
#define libc_strX(x)        libc_w16##x
#define libc_vstrX(x)       libc_vw16##x
#define libc_XstrX(x)       libc_Xw16##x
#define libc_XvstrX(x)      libc_Xvw16##x
#define libc_format_cX(x)   libc_format_w16##x
#define libc_format_vcX(x)  libc_format_vw16##x
#define libc_format_strX(x) libc_format_w16##x
#define libd_stringX(x)     libd_string16##x
#define libd_XstringX(x)    libd_Xstring16##x
#define libd_XwX(x)         libd_Xw16##x
#define libd_wX(x)          libd_w16##x
#define libd_vwX(x)         libd_vw16##x
#define libd_swX(x)         libd_sw16##x
#define libd_vswX(x)        libd_vsw16##x
#define libd_snwX(x)        libd_snw16##x
#define libd_vsnwX(x)       libd_vsnw16##x
#define libd_scwX(x)        libd_scw16##x
#define libd_vscwX(x)       libd_vscw16##x
#define libd_aswX(x)        libd_asw16##x
#define libd_vaswX(x)       libd_vasw16##x
#define libd_fwX(x)         libd_fw16##x
#define libd_vfwX(x)        libd_vfw16##x
#define libd_strX(x)        libd_w16##x
#define libd_vstrX(x)       libd_vw16##x
#define libd_XstrX(x)       libd_Xw16##x
#define libd_XvstrX(x)      libd_Xvw16##x
#define libd_format_cX(x)   libd_format_w16##x
#define libd_format_vcX(x)  libd_format_vw16##x
#elif CHARACTER_TYPE == CHARACTER_TYPE_CHAR32
#define T_char              char32_t
#define T_stringprinter     string32printer
#define T_pformatprinter    pw32formatprinter
#define T_pformatgetc       pw32formatgetc
#define T_pformatungetc     pw32formatungetc
#define T_buffer            w32buffer
#define T_printer           w32printer
#define libc_stringX(x)     libc_string32##x
#define libc_XstringX(x)    libc_Xstring32##x
#define libc_XwX(x)         libc_Xw32##x
#define libc_wX(x)          libc_w32##x
#define libc_vwX(x)         libc_vw32##x
#define libc_swX(x)         libc_sw32##x
#define libc_vswX(x)        libc_vsw32##x
#define libc_snwX(x)        libc_snw32##x
#define libc_vsnwX(x)       libc_vsnw32##x
#define libc_scwX(x)        libc_scw32##x
#define libc_vscwX(x)       libc_vscw32##x
#define libc_aswX(x)        libc_asw32##x
#define libc_vaswX(x)       libc_vasw32##x
#define libc_fwX(x)         libc_fw32##x
#define libc_vfwX(x)        libc_vfw32##x
#define libc_strX(x)        libc_w32##x
#define libc_vstrX(x)       libc_vw32##x
#define libc_XstrX(x)       libc_Xw32##x
#define libc_XvstrX(x)      libc_Xvw32##x
#define libc_format_cX(x)   libc_format_w32##x
#define libc_format_vcX(x)  libc_format_vw32##x
#define libc_format_strX(x) libc_format_w32##x
#define libd_stringX(x)     libd_string32##x
#define libd_XstringX(x)    libd_Xstring32##x
#define libd_XwX(x)         libd_Xw32##x
#define libd_wX(x)          libd_w32##x
#define libd_swX(x)         libd_sw32##x
#define libd_vswX(x)        libd_vsw32##x
#define libd_snwX(x)        libd_snw32##x
#define libd_vsnwX(x)       libd_vsnw32##x
#define libd_scwX(x)        libd_scw32##x
#define libd_vscwX(x)       libd_vscw32##x
#define libd_aswX(x)        libd_asw32##x
#define libd_vaswX(x)       libd_vasw32##x
#define libd_fwX(x)         libd_fw32##x
#define libd_vfwX(x)        libd_vfw32##x
#define libd_vwX(x)         libd_vw32##x
#define libd_strX(x)        libd_w32##x
#define libd_vstrX(x)       libd_vw32##x
#define libd_XstrX(x)       libd_Xw32##x
#define libd_XvstrX(x)      libd_Xvw32##x
#define libd_format_cX(x)   libd_format_w32##x
#define libd_format_vcX(x)  libd_format_vw32##x
#else
#error "Invalid type"
#endif

#define libc_format_X(x)   libc_format_cX(x)
#define libc_format_vX(x)  libc_format_vcX(x)
#define libd_format_X(x)   libd_format_cX(x)
#define libd_format_vX(x)  libd_format_vcX(x)

#ifndef T_NUL
#define T_NUL  '\0'
#endif


DECL_BEGIN

/* String printer functions for construction of C-style NUL-terminated strings. */
INTERN int LIBCCALL
libc_stringX(printer_init)(struct T_stringprinter *__restrict self,
                           size_t hint) {
 if (!hint) hint = 16*sizeof(void *);
 self->sp_buffer = (T_char *)libc_malloc((hint+1)*sizeof(T_char));
 if unlikely(!self->sp_buffer) {
  self->sp_bufpos = NULL;
  self->sp_buffer = NULL;
  return -1;
 }
 self->sp_bufpos = self->sp_buffer;
 self->sp_bufend = self->sp_buffer+hint;
 self->sp_bufend[0] = T_NUL;
 return 0;
}
INTERN void LIBCCALL
libc_XstringX(printer_init)(struct T_stringprinter *__restrict self,
                            size_t hint) {
 if (!hint) hint = 16*sizeof(void *);
 self->sp_buffer = (T_char *)libc_Xmalloc((hint+1)*sizeof(T_char));
 self->sp_bufpos = self->sp_buffer;
 self->sp_bufend = self->sp_buffer+hint;
 self->sp_bufend[0] = T_NUL;
}
INTERN T_char *LIBCCALL
libc_stringX(printer_pack)(struct T_stringprinter *__restrict self,
                           size_t *length) {
 T_char *result; size_t result_size;
 assert(self->sp_bufpos >= self->sp_buffer);
 assert(self->sp_bufpos <= self->sp_bufend);
 result_size = (size_t)(self->sp_bufpos-self->sp_buffer);
 if (self->sp_bufpos != self->sp_bufend) {
  result = (T_char *)libc_realloc(self->sp_buffer,
                                 (result_size+1)*
                                  sizeof(T_char));
  if unlikely(!result) result = self->sp_buffer;
 } else {
  result = self->sp_buffer;
 }
 result[result_size] = T_NUL;
 self->sp_buffer = NULL;
 if (length) *length = result_size;
 return result;
}
#ifndef LIBC_GENERIC_STRINGPRINTER_FINI_DEFINED
#define LIBC_GENERIC_STRINGPRINTER_FINI_DEFINED 1
INTERN void LIBCCALL
libc_generic_stringprinter_fini(struct stringprinter *__restrict self) {
 libc_free(self->sp_buffer);
}
#endif
DEFINE_INTERN_ALIAS(libc_stringX(printer_fini),libc_generic_stringprinter_fini);
INTERN ssize_t LIBCCALL
libc_stringX(printer_print)(T_char const *__restrict data,
                            size_t datalen, void *closure) {
 struct T_stringprinter *self;
 size_t size_avail,newsize,reqsize;
 T_char *new_buffer;
 self = (struct T_stringprinter *)closure;
 assert(self->sp_bufpos >= self->sp_buffer);
 assert(self->sp_bufpos <= self->sp_bufend);
 size_avail = (size_t)(self->sp_bufend-self->sp_bufpos);
 if unlikely(size_avail < datalen) {
  /* Must allocate more memory. */
  newsize = (size_t)(self->sp_bufend-self->sp_buffer);
  assert(newsize);
  reqsize = newsize+(datalen-size_avail);
  /* Double the buffer size until it is of sufficient length. */
  do newsize *= 2; while (newsize < reqsize);
  /* Reallocate the buffer (But include 1 character for the terminating '\0') */
  new_buffer = (T_char *)libc_realloc(self->sp_buffer,(newsize+1)*sizeof(T_char));
  if unlikely(!new_buffer) {
   newsize = reqsize;
   new_buffer = (T_char *)libc_realloc(self->sp_buffer,(newsize+1)*sizeof(T_char));
   if (!new_buffer) return -1;
  }
  self->sp_bufpos = new_buffer+(self->sp_bufpos-self->sp_buffer);
  self->sp_bufend = new_buffer+newsize;
  self->sp_buffer = new_buffer;
 }
 libc_memcpy(self->sp_bufpos,data,datalen*sizeof(T_char));
 self->sp_bufpos += datalen;
 assert(self->sp_bufpos <= self->sp_bufend);
 return (ssize_t)datalen;
}
INTERN ssize_t LIBCCALL
libc_XstringX(printer_print)(T_char const *__restrict data,
                             size_t datalen, void *closure) {
 struct T_stringprinter *self;
 size_t size_avail,newsize,reqsize;
 T_char *COMPILER_IGNORE_UNINITIALIZED(new_buffer);
 self = (struct T_stringprinter *)closure;
 assert(self->sp_bufpos >= self->sp_buffer);
 assert(self->sp_bufpos <= self->sp_bufend);
 size_avail = (size_t)(self->sp_bufend-self->sp_bufpos);
 if unlikely(size_avail < datalen) {
  /* Must allocate more memory. */
  newsize = (size_t)(self->sp_bufend-self->sp_buffer);
  assert(newsize);
  reqsize = newsize+(datalen-size_avail);
  /* Double the buffer size until it is of sufficient length. */
  do newsize *= 2; while (newsize < reqsize);
  /* Reallocate the buffer (But include 1 character for the terminating '\0') */
  LIBC_TRY {
   new_buffer = (T_char *)libc_Xrealloc(self->sp_buffer,(newsize+1)*sizeof(T_char));
  } LIBC_CATCH (E_BADALLOC) {
   newsize    = reqsize;
   new_buffer = (T_char *)libc_Xrealloc(self->sp_buffer,(newsize+1)*sizeof(T_char));
  }
  self->sp_bufpos = new_buffer+(self->sp_bufpos-self->sp_buffer);
  self->sp_bufend = new_buffer+newsize;
  self->sp_buffer = new_buffer;
 }
 libc_memcpy(self->sp_bufpos,data,datalen*sizeof(T_char));
 self->sp_bufpos += datalen;
 assert(self->sp_bufpos <= self->sp_bufend);
 return (ssize_t)datalen;
}




/* String buffers. */
#ifndef BUFFER_FLUSHTRUNC
#define BUFFER_FLUSHTRUNC   256
#define BUFFER_PRINTLIMIT   4096
#define BUFFER_SIZEALIGN    64
#endif
#ifndef BUFFER_USED
#define BUFFER_USED(x)     (size_t)((x)->b_bufpos-(x)->b_buffer)
#define BUFFER_SIZE(x)     (size_t)((x)->b_bufend-(x)->b_buffer)
#define BUFFER_UNUSED(x)   (size_t)((x)->b_bufend-(x)->b_bufpos)
#define BUFFER_REMAIN(x)   (size_t)(BUFFER_PRINTLIMIT-BUFFER_SIZE(x))
#endif

/* Process a given printer return value into the state of the given buffer. */
#ifndef BUFFER_ADDSTATE
#define BUFFER_ADDSTATE(x,y) \
  (unlikely((y) < 0) ? (x)->b_state = (y) : \
   unlikely(__builtin_add_overflow((x)->b_state,(y),&(x)->b_state)) \
   ? (x)->b_state = SSIZE_MAX : 0)
#endif

#ifndef LIBC_GENERIC_BUFFER_INIT_DEFINED
#define LIBC_GENERIC_BUFFER_INIT_DEFINED 1
INTERN void LIBCCALL
libc_generic_buffer_init(struct buffer *__restrict self,
                         pformatprinter printer, void *closure) {
 self->b_printer = printer;
 self->b_closure = closure;
 self->b_state   = 0;
 self->b_buffer  = NULL;
 self->b_bufpos  = NULL;
 self->b_bufend  = NULL;
}
#endif
DEFINE_INTERN_ALIAS(libc_wX(buffer_init),libc_generic_buffer_init);
INTERN ssize_t LIBCCALL
libc_wX(buffer_fini)(struct T_buffer *__restrict buf) {
 ssize_t result = BUFFER_USED(buf);
 /* print remaining data if there is some and no error occurred. */
 LIBC_TRY {
  if (result && buf->b_state >= 0) {
   result = (*buf->b_printer)(buf->b_buffer,(size_t)result,
                              buf->b_closure);
   if likely(result >= 0) result += buf->b_state;
  }
 } LIBC_FINALLY {
  /* Free the allocate data buffer. */
  libc_free(buf->b_buffer);
 }
 return result;
}
INTERN ssize_t LIBCCALL
libc_wX(buffer_flush)(struct T_buffer *__restrict buf) {
 ssize_t result;
 /* Check for an error. */
 if (buf->b_state < 0)
     return buf->b_state;
 /* Check if unprinted data is available. */
 result = BUFFER_USED(buf);
 if (result) {
  result = (*buf->b_printer)(buf->b_buffer,(size_t)result,
                             buf->b_closure);
  BUFFER_ADDSTATE(buf,result);
 }
 /* Reset the buffer position. */
 buf->b_bufpos = buf->b_buffer;
 /* If the buffer was quite large, delete it.
  * Otherwise, keep it around so it can be re-used the next time print is called.
  * >> This way, we keep a small memory footprint, and optimize ourself
  *    for the intended use-case of the user printing large amounts of
  *    data through small chunks. */
 if (BUFFER_SIZE(buf) > BUFFER_FLUSHTRUNC) {
  libc_free(buf->b_buffer);
  buf->b_buffer = NULL;
  buf->b_bufpos = NULL;
  buf->b_bufend = NULL;
 }
 return result;
}
INTERN ssize_t LIBCCALL
libc_wX(buffer_print)(T_char const *__restrict data,
                      size_t datalen, void *closure) {
 /* The heart of the buffer: Its printer callback. */
 struct T_buffer *buf = (struct T_buffer *)closure;
 ssize_t result,temp; size_t copy_size;
 assert(buf);
 assert(buf->b_bufpos >= buf->b_buffer);
 assert(buf->b_bufpos <= buf->b_bufend);
 assert(buf->b_buffer <= buf->b_bufend);
 /* Return the number of written bytes by default. */
 result = (ssize_t)datalen;
 /* Unlikely: Handle large amounts of data. */
 if unlikely(datalen >= BUFFER_PRINTLIMIT) {
  /* Make sure to flush existing data to preserve print order. */
  temp = libc_wX(buffer_flush)(buf);
  if unlikely(temp < 0) return temp;
  /* Now print this huge block of data. */
  temp = (*buf->b_printer)(data,datalen,buf->b_closure);
  /* Add the state to what has already been printed. */
  BUFFER_ADDSTATE(buf,temp);
  if (temp < 0) return temp;
  goto end;
 }

again:
 /* Fill unused data. */
 copy_size = MIN(BUFFER_UNUSED(buf),datalen);
 /*if (copy_size)*/
 {
  libc_memcpy(buf->b_bufpos,data,copy_size*sizeof(T_char));
  data          += copy_size;
  datalen       -= copy_size;
  buf->b_bufpos += copy_size;
 }

 /* Allocate more memory. */
 copy_size = MIN(BUFFER_REMAIN(buf),datalen);
 if (copy_size) {
  size_t new_size; T_char *new_buffer;
  new_size = BUFFER_SIZE(buf)+copy_size;
  assert(new_size <= BUFFER_PRINTLIMIT);
  new_size = CEIL_ALIGN(new_size,BUFFER_SIZEALIGN);
  assert(new_size <= BUFFER_PRINTLIMIT);
  new_buffer = (T_char *)libc_realloc(buf->b_buffer,new_size*sizeof(T_char));
  if unlikely(!new_buffer) {
   /* Special case: Flush the existing buffer to get more unused data. */
   temp = libc_wX(buffer_flush)(buf);
   if (temp < 0) return temp;
   assert(buf->b_bufpos == buf->b_buffer);
   if (datalen < BUFFER_UNUSED(buf)) {
    /* Simple case: We can copy the remainder into the currently empty buffer. */
    libc_memcpy(buf->b_bufpos,data,datalen*sizeof(T_char));
    buf->b_bufpos += datalen;
    goto end;
   }
   goto again;
  }
  buf->b_bufpos = new_buffer+(buf->b_bufpos-buf->b_buffer);
  buf->b_bufend = new_buffer+new_size;
  buf->b_buffer = new_buffer;
  /* Copy the data we've just allocated memory for. */
  libc_memcpy(buf->b_bufpos,data,copy_size*sizeof(T_char));
  data          += copy_size;
  datalen       -= copy_size;
  buf->b_bufpos += copy_size;
 }

 /* At this point, we know that we're either done, or that the buffer is full. */
 assert(datalen < BUFFER_PRINTLIMIT); /* Already assured at the start. */
 if (datalen) {
  /* NOTE: Before calling the printer, make sure we're not in an error state. */
  if (buf->b_state < 0) return buf->b_state;
  assert(BUFFER_USED(buf) == BUFFER_PRINTLIMIT);
  assert(BUFFER_SIZE(buf) == BUFFER_PRINTLIMIT);
  /* If the buffer is full, print it. */
  temp = (*buf->b_printer)(buf->b_buffer,BUFFER_PRINTLIMIT,
                           buf->b_closure);
  BUFFER_ADDSTATE(buf,temp);
  if (temp < 0) return temp;
  /* Copy all the unprinted data into the (now) empty buffer. */
  libc_memcpy(buf->b_buffer,data,datalen*sizeof(T_char));
  buf->b_bufpos = buf->b_bufpos+datalen;
 }
end:
 return result;
}

/* Conversion printers. */
#if CHARACTER_TYPE != CHARACTER_TYPE_CHAR
#ifndef LIBC_GENERIC_PRINTER_INIT_DEFINED
#define LIBC_GENERIC_PRINTER_INIT_DEFINED 1
INTERN void LIBCCALL
libc_generic_printer_init(struct w16printer *__restrict wp,
                          pw16formatprinter printer, void *closure) {
 wp->p_printer = printer;
 wp->p_closure = closure;
 wp->p_buffer  = NULL;
 wp->p_buflen  = 0;
 mbstate_reset(&wp->p_mbstate);
}
#endif
#ifndef LIBC_GENERIC_PRINTER_FINI_DEFINED
#define LIBC_GENERIC_PRINTER_FINI_DEFINED 1
INTERN void LIBCCALL
libc_generic_printer_fini(struct w16printer *__restrict wp) {
 libc_free(wp->p_buffer);
}
#endif
DEFINE_INTERN_ALIAS(libc_wX(printer_init),libc_generic_printer_init);
DEFINE_INTERN_ALIAS(libc_wX(printer_fini),libc_generic_printer_fini);

#ifndef PRINTER_MAXBUF
#define PRINTER_MAXBUF  (4096/sizeof(T_char))
#endif

INTERN ssize_t LIBCCALL
libc_wX(printer_print)(char const *__restrict data,
                       size_t datalen, void *closure) {
 T_char minbuf[1]; ssize_t result = 0,temp;
 struct T_printer *wp = (struct T_printer *)closure;
 T_char *buf = wp->p_buffer;
 for (;;) {
  temp = MIN(datalen,PRINTER_MAXBUF);
  if (!temp) break;
  /* Allocate a buffer that should always be of sufficient size. */
  if ((size_t)temp > wp->p_buflen) {
   T_char *new_buffer; size_t newsize = CEIL_ALIGN((size_t)temp,16);
   new_buffer = (T_char *)libc_realloc(wp->p_buffer,newsize*sizeof(T_char));
   if unlikely(!new_buffer) {
    buf  = minbuf;
    temp = COMPILER_LENOF(minbuf);
   } else {
    wp->p_buffer = buf = new_buffer;
    wp->p_buflen = newsize;
   }
  }
  assert(datalen);
  /* Convert text to utf16 / utf32. */
#if CHARACTER_TYPE == CHARACTER_TYPE_CHAR16
  temp = (ssize_t)libc_utf8to16((char const *)&data,(size_t)&datalen,
                                 buf,wp->p_buflen,&wp->p_mbstate,
                                 UNICODE_F_NOZEROTERM|UNICODE_F_UPDATESRC|
                                 UNICODE_F_SETERRNO);
#else
  temp = (ssize_t)libc_utf8to32((char const *)&data,(size_t)&datalen,
                                 buf,wp->p_buflen,&wp->p_mbstate,
                                 UNICODE_F_NOZEROTERM|UNICODE_F_UPDATESRC|
                                 UNICODE_F_SETERRNO);
#endif
  if unlikely((size_t)temp == UNICODE_ERROR) return temp;
  /* Call the underlying pointer. */
  temp = (*wp->p_printer)(buf,(size_t)temp,wp->p_closure);
  /* Forward print errors. */
  if unlikely(temp < 0) return temp;
  result += temp;
 }
 return result;
}
INTERN ssize_t LIBCCALL
libc_XwX(printer_print)(char const *__restrict data,
                        size_t datalen, void *closure) {
 T_char minbuf[1]; ssize_t result = 0,temp;
 struct T_printer *wp = (struct T_printer *)closure;
 T_char *buf = wp->p_buffer;
 for (;;) {
  temp = MIN(datalen,PRINTER_MAXBUF);
  if (!temp) break;
  /* Allocate a buffer that should always be of sufficient size. */
  if ((size_t)temp > wp->p_buflen) {
   T_char *new_buffer; size_t newsize = CEIL_ALIGN((size_t)temp,16);
   new_buffer = (T_char *)libc_realloc(wp->p_buffer,newsize*sizeof(T_char));
   if unlikely(!new_buffer) {
    buf  = minbuf;
    temp = COMPILER_LENOF(minbuf);
   } else {
    wp->p_buffer = buf = new_buffer;
    wp->p_buflen = newsize;
   }
  }
  assert(datalen);
  /* Convert text to utf16 / utf32. */
#if CHARACTER_TYPE == CHARACTER_TYPE_CHAR16
  temp = (ssize_t)libc_Xutf8to16((char const *)&data,(size_t)&datalen,
                                  buf,wp->p_buflen,&wp->p_mbstate,
                                  UNICODE_F_NOZEROTERM|UNICODE_F_UPDATESRC|
                                  UNICODE_F_SETERRNO);
#else
  temp = (ssize_t)libc_Xutf8to32((char const *)&data,(size_t)&datalen,
                                  buf,wp->p_buflen,&wp->p_mbstate,
                                  UNICODE_F_NOZEROTERM|UNICODE_F_UPDATESRC|
                                  UNICODE_F_SETERRNO);
#endif
  if unlikely((size_t)temp == UNICODE_ERROR) return temp;
  /* Call the underlying pointer. */
  temp = (*wp->p_printer)(buf,(size_t)temp,wp->p_closure);
  /* Forward print errors. */
  if unlikely(temp < 0) return temp;
  result += temp;
 }
 return result;
}
#endif /* CHARACTER_TYPE != CHARACTER_TYPE_CHAR */

#if CHARACTER_TYPE != CHARACTER_TYPE_CHAR
/* Wide-character format helpers. */
INTDEF ssize_t
(LIBCCALL libc_format_cX(quote))(T_pformatprinter printer, void *closure,
                                 T_char const *__restrict text,
                                 size_t textlen, u32 flags) {
 return libc_format_cX(quote_l)(printer,closure,text,textlen,flags,NULL);
}
INTDEF ssize_t
(LIBCCALL libc_format_cX(hexdump))(T_pformatprinter printer, void *closure,
                                   void const *__restrict data, size_t size,
                                   size_t linesize, u32 flags) {
 return libc_format_cX(hexdump_l)(printer,closure,data,size,linesize,flags,NULL);
}

/* Wide-character format printers. */
INTERN ssize_t
(LIBCCALL libc_format_vcX(printf))(T_pformatprinter printer, void *closure,
                                  T_char const *__restrict format, va_list args) {
 return libc_format_vcX(printf_l)(printer,closure,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_format_vcX(printf))(T_pformatprinter printer, void *closure,
                                  T_char const *__restrict format, va_list args) {
 return libd_format_vcX(printf_l)(printer,closure,format,NULL,args);
}

/* Wide-character format scanners. */
INTERN ssize_t
(LIBCCALL libc_format_vcX(scanf))(T_pformatgetc scanner,
                                  T_pformatungetc returnch, void *closure,
                                  T_char const *__restrict format, va_list args) {
 return libc_format_vcX(scanf_l)(scanner,returnch,closure,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_format_vcX(scanf))(T_pformatgetc scanner,
                                  T_pformatungetc returnch, void *closure,
                                  T_char const *__restrict format, va_list args) {
 return libd_format_vcX(scanf_l)(scanner,returnch,closure,format,NULL,args);
}
#endif /* CHARACTER_TYPE != CHARACTER_TYPE_CHAR */

/* Format printers with support for positional arguments. */
INTERN ssize_t
(LIBCCALL libc_format_vX(printf_p))(T_pformatprinter printer, void *closure,
                                    T_char const *__restrict format, va_list args) {
 return libc_format_vX(printf_p_l)(printer,closure,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_format_vX(printf_p))(T_pformatprinter printer, void *closure,
                                    T_char const *__restrict format, va_list args) {
 return libd_format_vX(printf_p_l)(printer,closure,format,NULL,args);
}

/* Format printers with an automatic, intermediate buffer. */
INTERN ssize_t LIBCCALL
libc_format_vcX(bprintf_l)(T_pformatprinter printer, void *closure,
                           T_char const *__restrict format,
                           locale_t locale, va_list args) {
 ssize_t result;
 struct T_buffer buf = WBUFFER_INIT(printer,closure);
 LIBC_TRY {
  libc_format_vX(printf_l)(&libc_wX(buffer_print),
                           &buf,format,locale,args);
 } LIBC_FINALLY {
  result = libc_wX(buffer_fini)(&buf);
 }
 return result;
}
INTERN ssize_t LIBCCALL
libd_format_vcX(bprintf_l)(T_pformatprinter printer, void *closure,
                           T_char const *__restrict format,
                           locale_t locale, va_list args) {
 ssize_t result;
 struct T_buffer buf = WBUFFER_INIT(printer,closure);
 LIBC_TRY {
  libd_format_vX(printf_l)(&libc_wX(buffer_print),
                           &buf,format,locale,args);
 } LIBC_FINALLY {
  result = libc_wX(buffer_fini)(&buf);
 }
 return result;
}
INTERN ssize_t LIBCCALL
libc_format_vcX(bprintf_p_l)(T_pformatprinter printer, void *closure,
                             T_char const *__restrict format,
                             locale_t locale, va_list args) {
 ssize_t result;
 struct T_buffer buf = WBUFFER_INIT(printer,closure);
 LIBC_TRY {
  libc_format_vX(printf_p_l)(&libc_wX(buffer_print),
                             &buf,format,locale,args);
 } LIBC_FINALLY {
  result = libc_wX(buffer_fini)(&buf);
 }
 return result;
}
INTERN ssize_t LIBCCALL
libd_format_vcX(bprintf_p_l)(T_pformatprinter printer, void *closure,
                             T_char const *__restrict format,
                             locale_t locale, va_list args) {
 ssize_t result;
 struct T_buffer buf = WBUFFER_INIT(printer,closure);
 LIBC_TRY {
  libd_format_vX(printf_p_l)(&libc_wX(buffer_print),
                             &buf,format,locale,args);
 } LIBC_FINALLY {
  result = libc_wX(buffer_fini)(&buf);
 }
 return result;
}

INTERN ssize_t
(LIBCCALL libc_format_vcX(bprintf))(T_pformatprinter printer, void *closure,
                                    T_char const *__restrict format, va_list args) {
 return libc_format_vcX(bprintf_l)(printer,closure,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_format_vcX(bprintf))(T_pformatprinter printer, void *closure,
                                    T_char const *__restrict format, va_list args) {
 return libd_format_vcX(bprintf_l)(printer,closure,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libc_format_vcX(bprintf_p))(T_pformatprinter printer, void *closure,
                                      T_char const *__restrict format, va_list args) {
 return libc_format_vcX(bprintf_p_l)(printer,closure,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_format_vcX(bprintf_p))(T_pformatprinter printer, void *closure,
                                      T_char const *__restrict format, va_list args) {
 return libd_format_vcX(bprintf_p_l)(printer,closure,format,NULL,args);
}


/* Unlimited string printers */
PRIVATE ssize_t LIBCCALL
libc_swX(printf_callback)(T_char const *__restrict data, size_t datalen,
                          T_char **pbuffer) {
 libc_memcpy(*pbuffer,data,datalen*sizeof(T_char));
 *pbuffer += datalen;
 return datalen;
}

INTERN size_t LIBCCALL
libc_vswX(printf_l)(T_char *__restrict buf,
                    T_char const *__restrict format,
                    locale_t locale, va_list args) {
 size_t result;
 result = (size_t)libc_format_vX(printf_l)((T_pformatprinter)&libc_swX(printf_callback),
                                           (void *)&buf,format,locale,args);
 /* Random fact: Forgetting to terminate the string here
  *              breaks tab-completion in busybox's ash... */
 *buf = T_NUL;
 return result;
}
INTERN size_t LIBCCALL
libd_vswX(printf_l)(T_char *__restrict buf,
                    T_char const *__restrict format,
                    locale_t locale, va_list args) {
 size_t result;
 result = (size_t)libd_format_vX(printf_l)((T_pformatprinter)&libc_swX(printf_callback),
                                           (void *)&buf,format,locale,args);
 *buf = T_NUL;
 return result;
}
INTERN size_t LIBCCALL
libc_vswX(printf_p_l)(T_char *__restrict buf,
                      T_char const *__restrict format,
                      locale_t locale, va_list args) {
 size_t result;
 result = (size_t)libc_format_vX(printf_p_l)((T_pformatprinter)&libc_swX(printf_callback),
                                             (void *)&buf,format,locale,args);
 *buf = T_NUL;
 return result;
}
INTERN size_t LIBCCALL
libd_vswX(printf_p_l)(T_char *__restrict buf,
                      T_char const *__restrict format,
                      locale_t locale, va_list args) {
 size_t result;
 result = (size_t)libd_format_vX(printf_p_l)((T_pformatprinter)&libc_swX(printf_callback),
                                             (void *)&buf,format,locale,args);
 *buf = T_NUL;
 return result;
}

INTERN size_t
(LIBCCALL libc_vswX(printf))(T_char *__restrict buf,
                             T_char const *__restrict format,
                             va_list args) {
 return libc_vswX(printf_l)(buf,format,NULL,args);
}
INTERN size_t
(LIBCCALL libd_vswX(printf))(T_char *__restrict buf,
                             T_char const *__restrict format,
                             va_list args) {
 return libd_vswX(printf_l)(buf,format,NULL,args);
}
INTERN size_t
(LIBCCALL libc_vswX(printf_p))(T_char *__restrict buf,
                               T_char const *__restrict format,
                               va_list args) {
 return libc_vswX(printf_p_l)(buf,format,NULL,args);
}
INTERN size_t
(LIBCCALL libd_vswX(printf_p))(T_char *__restrict buf,
                               T_char const *__restrict format,
                               va_list args) {
 return libd_vswX(printf_p_l)(buf,format,NULL,args);
}


struct libc_swX(nprintf_data) { T_char *bufpos,*bufend; };
PRIVATE ssize_t LIBCCALL
libc_swX(nprintf_callback)(T_char const *__restrict data, size_t datalen,
                           struct libc_swX(nprintf_data) *__restrict buf) {
 /* Don't exceed the buffer end */
 if (buf->bufpos < buf->bufend) {
  size_t maxwrite = (size_t)(buf->bufend-buf->bufpos);
  libc_memcpy(buf->bufpos,data,
              MIN(maxwrite,datalen)*
              sizeof(T_char));
 }
 /* Still seek past the end, as to
  * calculate the required buffersize. */
 buf->bufpos += datalen;
 return datalen;
}

INTERN ssize_t
(LIBCCALL libc_format_strX(ftime))(T_pformatprinter printer, void *closure,
                                   T_char const *__restrict format,
                                   struct tm const *__restrict tm) {
 return libc_format_strX(ftime_l)(printer,closure,format,tm,NULL);
}
INTERN size_t LIBCCALL
libc_strX(ftime_l)(T_char *__restrict buf, size_t buflen,
                   T_char const *__restrict format,
                   struct tm const *__restrict tp, locale_t locale) {
 struct libc_swX(nprintf_data) data; data.bufpos = buf;
 if (__builtin_add_overflow((uintptr_t)buf,buflen,(uintptr_t *)&data.bufend))
     data.bufend = (T_char *)(uintptr_t)-1;
 libc_format_strX(ftime_l)((T_pformatprinter)&libc_swX(nprintf_callback),
                           &data,format,tp,locale);
 if likely(data.bufpos < data.bufend)
    *data.bufpos = T_NUL;
 return (size_t)(data.bufpos-buf);
}
INTERN size_t LIBCCALL
libc_strX(ftime)(T_char *__restrict buf, size_t buflen,
                 T_char const *__restrict format,
                 struct tm const *__restrict tp) {
 return libc_strX(ftime_l)(buf,buflen,format,tp,NULL);
}


INTERN size_t LIBCCALL
libc_vsnwX(printf_l)(T_char *__restrict buf, size_t buflen,
                     T_char const *__restrict format,
                     locale_t locale, va_list args) {
 struct libc_swX(nprintf_data) data; data.bufpos = buf;
 if (__builtin_add_overflow((uintptr_t)buf,buflen,(uintptr_t *)&data.bufend))
     data.bufend = (T_char *)(uintptr_t)-1;
 libc_format_vX(printf_l)((T_pformatprinter)&libc_swX(nprintf_callback),
                          &data,format,locale,args);
 if likely(data.bufpos < data.bufend) *data.bufpos = T_NUL;
 return (size_t)(data.bufpos-buf);
}
INTERN size_t LIBCCALL
libd_vsnwX(printf_l)(T_char *__restrict buf, size_t buflen,
                     T_char const *__restrict format,
                     locale_t locale, va_list args) {
 struct libc_swX(nprintf_data) data; data.bufpos = buf;
 if (__builtin_add_overflow((uintptr_t)buf,buflen,(uintptr_t *)&data.bufend))
     data.bufend = (T_char *)(uintptr_t)-1;
 libd_format_vX(printf_l)((T_pformatprinter)&libc_swX(nprintf_callback),
                          &data,format,locale,args);
 if likely(data.bufpos < data.bufend) *data.bufpos = T_NUL;
 return (size_t)(data.bufpos-buf);
}
INTERN size_t LIBCCALL
libc_vsnwX(printf_p_l)(T_char *__restrict buf, size_t buflen,
                       T_char const *__restrict format,
                       locale_t locale, va_list args) {
 struct libc_swX(nprintf_data) data; data.bufpos = buf;
 if (__builtin_add_overflow((uintptr_t)buf,buflen,(uintptr_t *)&data.bufend))
     data.bufend = (T_char *)(uintptr_t)-1;
 libc_format_vX(printf_p_l)((T_pformatprinter)&libc_swX(nprintf_callback),
                            &data,format,locale,args);
 if likely(data.bufpos < data.bufend) *data.bufpos = T_NUL;
 return (size_t)(data.bufpos-buf);
}
INTERN size_t LIBCCALL
libd_vsnwX(printf_p_l)(T_char *__restrict buf, size_t buflen,
                       T_char const *__restrict format,
                       locale_t locale, va_list args) {
 struct libc_swX(nprintf_data) data; data.bufpos = buf;
 if (__builtin_add_overflow((uintptr_t)buf,buflen,(uintptr_t *)&data.bufend))
     data.bufend = (T_char *)(uintptr_t)-1;
 libd_format_vX(printf_p_l)((T_pformatprinter)&libc_swX(nprintf_callback),
                            &data,format,locale,args);
 if likely(data.bufpos < data.bufend) *data.bufpos = T_NUL;
 return (size_t)(data.bufpos-buf);
}

INTERN size_t
(LIBCCALL libc_vsnwX(printf))(T_char *__restrict buf, size_t buflen,
                              T_char const *__restrict format,
                              va_list args) {
 return libc_vsnwX(printf_l)(buf,buflen,format,NULL,args);
}
INTERN size_t
(LIBCCALL libd_vsnwX(printf))(T_char *__restrict buf, size_t buflen,
                              T_char const *__restrict format,
                              va_list args) {
 return libd_vsnwX(printf_l)(buf,buflen,format,NULL,args);
}
INTERN size_t
(LIBCCALL libc_vsnwX(printf_p))(T_char *__restrict buf, size_t buflen,
                                T_char const *__restrict format,
                                va_list args) {
 return libc_vsnwX(printf_p_l)(buf,buflen,format,NULL,args);
}
INTERN size_t
(LIBCCALL libd_vsnwX(printf_p))(T_char *__restrict buf, size_t buflen,
                                T_char const *__restrict format,
                                va_list args) {
 return libd_vsnwX(printf_p_l)(buf,buflen,format,NULL,args);
}



#if CHARACTER_TYPE == CHARACTER_TYPE_CHAR
#ifndef FORWARD_LIBC_VDPRINTF_CALLBACK_DEFINED
#define FORWARD_LIBC_VDPRINTF_CALLBACK_DEFINED 1
#endif
INTERN ssize_t LIBCCALL
libc_vdprintf_callback(char const *__restrict data, size_t datalen,
                       void *fd) {
 return libc_write((int)(uintptr_t)fd,data,datalen);
}
INTERN ssize_t LIBCCALL
libc_vwX(dprintf_l)(fd_t fd, T_char const *__restrict format,
                    locale_t locale, va_list args) {
 return LIBC_DPRINTF_IS_BUFFERED
      ? libc_format_vcX(bprintf_l)(&libc_vdprintf_callback,(void *)(uintptr_t)(unsigned int)fd,format,locale,args)
      : libc_format_vcX(printf_l)(&libc_vdprintf_callback,(void *)(uintptr_t)(unsigned int)fd,format,locale,args);
}
INTERN ssize_t LIBCCALL
libd_vwX(dprintf_l)(fd_t fd, T_char const *__restrict format,
                    locale_t locale, va_list args) {
 return LIBC_DPRINTF_IS_BUFFERED
      ? libd_format_vcX(bprintf_l)(&libc_vdprintf_callback,(void *)(uintptr_t)(unsigned int)fd,format,locale,args)
      : libd_format_vcX(printf_l)(&libc_vdprintf_callback,(void *)(uintptr_t)(unsigned int)fd,format,locale,args);
}
INTERN ssize_t LIBCCALL
libc_vwX(dprintf_p_l)(fd_t fd, T_char const *__restrict format,
                      locale_t locale, va_list args) {
 return LIBC_DPRINTF_IS_BUFFERED
      ? libc_format_vcX(bprintf_p_l)(&libc_vdprintf_callback,(void *)(uintptr_t)(unsigned int)fd,format,locale,args)
      : libc_format_vcX(printf_p_l)(&libc_vdprintf_callback,(void *)(uintptr_t)(unsigned int)fd,format,locale,args);
}
INTERN ssize_t LIBCCALL
libd_vwX(dprintf_p_l)(fd_t fd, T_char const *__restrict format,
                      locale_t locale, va_list args) {
 return LIBC_DPRINTF_IS_BUFFERED
      ? libd_format_vcX(bprintf_p_l)(&libc_vdprintf_callback,(void *)(uintptr_t)(unsigned int)fd,format,locale,args)
      : libd_format_vcX(printf_p_l)(&libc_vdprintf_callback,(void *)(uintptr_t)(unsigned int)fd,format,locale,args);
}
#else
#ifndef FORWARD_LIBC_VDPRINTF_CALLBACK_DEFINED
#define FORWARD_LIBC_VDPRINTF_CALLBACK_DEFINED 1
INTDEF ssize_t LIBCCALL
libc_vdprintf_callback(char const *__restrict data, size_t datalen,
                       void *fd);
#endif /* !FORWARD_LIBC_VDPRINTF_CALLBACK_DEFINED */
struct libc_wX(vdprintf_data) {
    int       fd;
    mbstate_t state;
};
PRIVATE ssize_t LIBCCALL
libc_wX(vdprintf_callback)(T_char const *__restrict data, size_t datalen,
                           struct libc_wX(vdprintf_data) *__restrict ctx) {
#if CHARACTER_TYPE == CHARACTER_TYPE_CHAR16
 return libc_format_w16sztomb(&libc_vdprintf_callback,
                             (void *)(uintptr_t)(unsigned int)ctx->fd,
                              data,datalen,&ctx->state,
                              UNICODE_F_NOFAIL|UNICODE_F_NOZEROTERM);
#else
 return libc_format_w32sztomb(&libc_vdprintf_callback,
                             (void *)(uintptr_t)(unsigned int)ctx->fd,
                              data,datalen,&ctx->state,
                              UNICODE_F_NOFAIL|UNICODE_F_NOZEROTERM);
#endif
}
INTERN ssize_t LIBCCALL
libc_vwX(dprintf_l)(fd_t fd, T_char const *__restrict format,
                    locale_t locale, va_list args) {
 struct libc_wX(vdprintf_data) data;
 data.fd = fd;
 mbstate_reset(&data.state);
 return LIBC_DPRINTF_IS_BUFFERED
      ? libc_format_vcX(bprintf_l)((T_pformatprinter)&libc_wX(vdprintf_callback),&data,format,locale,args)
      : libc_format_vcX(printf_l)((T_pformatprinter)&libc_wX(vdprintf_callback),&data,format,locale,args);
}
INTERN ssize_t LIBCCALL
libd_vwX(dprintf_l)(fd_t fd, T_char const *__restrict format,
                    locale_t locale, va_list args) {
 struct libc_wX(vdprintf_data) data;
 data.fd = fd;
 mbstate_reset(&data.state);
 return LIBC_DPRINTF_IS_BUFFERED
      ? libd_format_vcX(bprintf_l)((T_pformatprinter)&libc_wX(vdprintf_callback),&data,format,locale,args)
      : libd_format_vcX(printf_l)((T_pformatprinter)&libc_wX(vdprintf_callback),&data,format,locale,args);
}
INTERN ssize_t LIBCCALL
libc_vwX(dprintf_p_l)(fd_t fd, T_char const *__restrict format,
                      locale_t locale, va_list args) {
 struct libc_wX(vdprintf_data) data;
 data.fd = fd;
 mbstate_reset(&data.state);
 return LIBC_DPRINTF_IS_BUFFERED
      ? libc_format_vcX(bprintf_p_l)((T_pformatprinter)&libc_wX(vdprintf_callback),&data,format,locale,args)
      : libc_format_vcX(printf_p_l)((T_pformatprinter)&libc_wX(vdprintf_callback),&data,format,locale,args);
}
INTERN ssize_t LIBCCALL
libd_vwX(dprintf_p_l)(fd_t fd, T_char const *__restrict format,
                      locale_t locale, va_list args) {
 struct libc_wX(vdprintf_data) data;
 data.fd = fd;
 mbstate_reset(&data.state);
 return LIBC_DPRINTF_IS_BUFFERED
      ? libd_format_vcX(bprintf_p_l)((T_pformatprinter)&libc_wX(vdprintf_callback),&data,format,locale,args)
      : libd_format_vcX(printf_p_l)((T_pformatprinter)&libc_wX(vdprintf_callback),&data,format,locale,args);
}
#endif


INTERN ssize_t
(LIBCCALL libc_vwX(dprintf))(fd_t fd, T_char const *__restrict format, va_list args) {
 return libc_vwX(dprintf_l)(fd,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_vwX(dprintf))(fd_t fd, T_char const *__restrict format, va_list args) {
 return libd_vwX(dprintf_l)(fd,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libc_vwX(dprintf_p))(fd_t fd, T_char const *__restrict format, va_list args) {
 return libc_vwX(dprintf_p_l)(fd,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_vwX(dprintf_p))(fd_t fd, T_char const *__restrict format, va_list args) {
 return libd_vwX(dprintf_p_l)(fd,format,NULL,args);
}

#if CHARACTER_TYPE == CHARACTER_TYPE_CHAR
PRIVATE ssize_t LIBCCALL
libc_format_X(sscanf_getc)(T_char **__restrict data) {
 T_char result = *(*data)++;
 return result ? result : EOF;
}
PRIVATE ssize_t LIBCCALL
libc_format_X(sscanf_ungetc)(int UNUSED(c), T_char **__restrict data) {
 --*data;
 return 0;
}
#else
PRIVATE ssize_t LIBCCALL
libc_format_X(sscanf_getc)(T_char *__restrict pch,
                           T_char **__restrict data) {
 return (*pch = *(*data)++) != 0 ? 0 : -1;
}
PRIVATE ssize_t LIBCCALL
libc_format_X(sscanf_ungetc)(T_char UNUSED(c), T_char **__restrict data) {
 --*data;
 return 0;
}
#endif

INTERN size_t
(LIBCCALL libc_vswX(scanf_l))(T_char const *__restrict src,
                              T_char const *__restrict format,
                              locale_t locale, va_list args) {
 return (size_t)libc_format_vX(scanf_l)((T_pformatgetc)&libc_format_X(sscanf_getc),
                                        (T_pformatungetc)&libc_format_X(sscanf_ungetc),
                                        (void *)&src,format,locale,args);
}
INTERN size_t
(LIBCCALL libd_vswX(scanf_l))(T_char const *__restrict src,
                              T_char const *__restrict format,
                              locale_t locale, va_list args) {
 return (size_t)libd_format_vX(scanf_l)((T_pformatgetc)&libc_format_X(sscanf_getc),
                                        (T_pformatungetc)&libc_format_X(sscanf_ungetc),
                                        (void *)&src,format,locale,args);
}
INTERN size_t
(LIBCCALL libc_vswX(scanf))(T_char const *__restrict src,
                            T_char const *__restrict format, va_list args) {
 return libc_vswX(scanf_l)(src,format,NULL,args);
}
INTERN size_t
(LIBCCALL libd_vswX(scanf))(T_char const *__restrict src,
                            T_char const *__restrict format,
                            va_list args) {
 return libd_vswX(scanf_l)(src,format,NULL,args);
}

struct libc_format_X(snscan_data) {
    T_char const *next;
    T_char const *end;
};
#if CHARACTER_TYPE == CHARACTER_TYPE_CHAR
PRIVATE ssize_t LIBCCALL
libc_format_X(snscanf_getc)(struct libc_format_X(snscan_data) *__restrict data) {
 if (data->next >= data->end)
     return EOF;
 return *data->next++;
}
#else
PRIVATE ssize_t LIBCCALL
libc_format_X(snscanf_getc)(T_char *__restrict pch,
                            struct libc_format_X(snscan_data) *__restrict data) {
 if (data->next >= data->end)
     return -1;
 *pch = *data->next++;
 return 0;
}
#endif
PRIVATE ssize_t LIBCCALL
libc_format_X(snscanf_ungetc)(T_char UNUSED(c),
                              struct libc_format_X(snscan_data) *__restrict data) {
 --data->next;
 return 0;
}

INTERN size_t
(LIBCCALL libc_vsnwX(scanf_l))(T_char const *__restrict src, size_t srclen,
                               T_char const *__restrict format,
                               locale_t locale, va_list args) {
 struct libc_format_X(snscan_data) data;
 data.end = (data.next = src)+srclen;
 return (size_t)libc_format_vX(scanf_l)((T_pformatgetc)&libc_format_X(snscanf_getc),
                                        (T_pformatungetc)&libc_format_X(snscanf_ungetc),
                                        &data,format,locale,args);
}
INTERN size_t
(LIBCCALL libd_vsnwX(scanf_l))(T_char const *__restrict src, size_t srclen,
                               T_char const *__restrict format,
                               locale_t locale, va_list args) {
 struct libc_format_X(snscan_data) data;
 data.end = (data.next = src)+srclen;
 return (size_t)libd_format_vX(scanf_l)((T_pformatgetc)&libc_format_X(snscanf_getc),
                                        (T_pformatungetc)&libc_format_X(snscanf_ungetc),
                                        &data,format,locale,args);
}
INTERN size_t
(LIBCCALL libc_vsnwX(scanf))(T_char const *__restrict src, size_t srclen,
                             T_char const *__restrict format, va_list args) {
 return libc_vsnwX(scanf_l)(src,srclen,format,NULL,args);
}
INTERN size_t
(LIBCCALL libd_vsnwX(scanf))(T_char const *__restrict src, size_t srclen,
                             T_char const *__restrict format, va_list args) {
 return libd_vsnwX(scanf_l)(src,srclen,format,NULL,args);
}



/* STDIO FILE Printers. */
INTERN ssize_t LIBCCALL
libc_vwX(printf_l)(T_char const *__restrict format,
                   locale_t locale, va_list args) {
 return libc_vfwX(printf_l)(stdout,format,locale,args);
}
INTERN ssize_t LIBCCALL
libd_vwX(printf_l)(T_char const *__restrict format,
                   locale_t locale, va_list args) {
 return libd_vfwX(printf_l)(stdout,format,locale,args);
}
INTERN ssize_t LIBCCALL
libc_vwX(printf_p_l)(T_char const *__restrict format,
                     locale_t locale, va_list args) {
 return libc_vfwX(printf_p_l)(stdout,format,locale,args);
}
INTERN ssize_t LIBCCALL
libd_vwX(printf_p_l)(T_char const *__restrict format,
                     locale_t locale, va_list args) {
 return libd_vfwX(printf_p_l)(stdout,format,locale,args);
}

INTERN ssize_t
(LIBCCALL libc_vwX(printf))(T_char const *__restrict format, va_list args) {
 return libc_vwX(printf_l)(format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_vwX(printf))(T_char const *__restrict format, va_list args) {
 return libd_vwX(printf_l)(format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libc_vwX(printf_p))(T_char const *__restrict format, va_list args) {
 return libc_vwX(printf_p_l)(format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_vwX(printf_p))(T_char const *__restrict format, va_list args) {
 return libd_vwX(printf_p_l)(format,NULL,args);
}

#if CHARACTER_TYPE == CHARACTER_TYPE_CHAR
INTERN ssize_t LIBCCALL
libc_vfwX(printf_l)(FILE *__restrict self,
                    T_char const *__restrict format,
                    locale_t locale, va_list args) {
 return libc_format_vcX(printf_l)(&libc_file_printer,
                                 (void *)self,format,locale,args);
}
INTERN ssize_t LIBCCALL
libd_vfwX(printf_l)(FILE *__restrict self,
                    T_char const *__restrict format,
                    locale_t locale, va_list args) {
 return libd_format_vcX(printf_l)(&libc_file_printer,
                                 (void *)self,format,locale,args);
}
INTERN ssize_t LIBCCALL
libc_vfwX(printf_p_l)(FILE *__restrict self,
                      T_char const *__restrict format,
                      locale_t locale, va_list args) {
 return libc_format_vcX(printf_p_l)(&libc_file_printer,
                                   (void *)self,format,locale,args);
}
INTERN ssize_t LIBCCALL
libd_vfwX(printf_p_l)(FILE *__restrict self,
                      T_char const *__restrict format,
                      locale_t locale, va_list args) {
 return libd_format_vcX(printf_p_l)(&libc_file_printer,
                                   (void *)self,format,locale,args);
}
#else
struct libc_wX(vfprintf_data) {
    FILE     *fp;
    mbstate_t state;
};
PRIVATE ssize_t LIBCCALL
libc_wX(vfprintf_callback)(T_char const *__restrict data, size_t datalen,
                           struct libc_wX(vfprintf_data) *__restrict ctx) {
#if CHARACTER_TYPE == CHARACTER_TYPE_CHAR16
 return libc_format_w16sztomb(&libc_file_printer,ctx->fp,
                               data,datalen,&ctx->state,
                               UNICODE_F_NOFAIL|UNICODE_F_NOZEROTERM);
#else
 return libc_format_w32sztomb(&libc_file_printer,ctx->fp,
                               data,datalen,&ctx->state,
                               UNICODE_F_NOFAIL|UNICODE_F_NOZEROTERM);
#endif
}
INTERN ssize_t LIBCCALL
libc_vfwX(printf_l)(FILE *__restrict self, T_char const *__restrict format,
                    locale_t locale, va_list args) {
 struct libc_wX(vfprintf_data) data;
 data.fp = self;
 mbstate_reset(&data.state);
 return libc_format_vcX(printf_l)((T_pformatprinter)&libc_wX(vfprintf_callback),
                                  &data,format,locale,args);
}
INTERN ssize_t LIBCCALL
libd_vfwX(printf_l)(FILE *__restrict self, T_char const *__restrict format,
                    locale_t locale, va_list args) {
 struct libc_wX(vfprintf_data) data;
 data.fp = self;
 mbstate_reset(&data.state);
 return libd_format_vcX(printf_l)((T_pformatprinter)&libc_wX(vfprintf_callback),
                                  &data,format,locale,args);
}
INTERN ssize_t LIBCCALL
libc_vfwX(printf_p_l)(FILE *__restrict self, T_char const *__restrict format,
                      locale_t locale, va_list args) {
 struct libc_wX(vfprintf_data) data;
 data.fp = self;
 mbstate_reset(&data.state);
 return libc_format_vcX(printf_p_l)((T_pformatprinter)&libc_wX(vfprintf_callback),
                                    &data,format,locale,args);
}
INTERN ssize_t LIBCCALL
libd_vfwX(printf_p_l)(FILE *__restrict self, T_char const *__restrict format,
                      locale_t locale, va_list args) {
 struct libc_wX(vfprintf_data) data;
 data.fp = self;
 mbstate_reset(&data.state);
 return libd_format_vcX(printf_p_l)((T_pformatprinter)&libc_wX(vfprintf_callback),
                                    &data,format,locale,args);
}
#endif

INTERN ssize_t
(LIBCCALL libc_vfwX(printf))(FILE *__restrict self,
                             T_char const *__restrict format, va_list args) {
 return libc_vfwX(printf_l)(self,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_vfwX(printf))(FILE *__restrict self,
                             T_char const *__restrict format, va_list args) {
 return libd_vfwX(printf_l)(self,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libc_vfwX(printf_p))(FILE *__restrict self,
                               T_char const *__restrict format, va_list args) {
 return libc_vfwX(printf_p_l)(self,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_vfwX(printf_p))(FILE *__restrict self,
                               T_char const *__restrict format, va_list args) {
 return libd_vfwX(printf_p_l)(self,format,NULL,args);
}


#if CHARACTER_TYPE == CHARACTER_TYPE_CHAR
#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define vfscanf_scanner   libc_fgetc
#define vfscanf_return   libc_ungetc
#else
PRIVATE ssize_t LIBCCALL vfscanf_scanner(FILE *self) { return libc_fgetc(self); }
PRIVATE ssize_t LIBCCALL vfscanf_return(unsigned int c, FILE *self) { return libc_ungetc(c,self); }
#endif

INTERN ssize_t LIBCCALL
libc_vfwX(scanf_l)(FILE *__restrict self, T_char const *__restrict format,
                   locale_t locale, va_list args) {
 return libc_format_vX(scanf_l)((T_pformatgetc)&vfscanf_scanner,
                                (T_pformatungetc)&vfscanf_return,
                                 self,format,locale,args);
}
INTERN ssize_t LIBCCALL
libd_vfwX(scanf_l)(FILE *__restrict self, T_char const *__restrict format,
                   locale_t locale, va_list args) {
 return libd_format_vX(scanf_l)((T_pformatgetc)&vfscanf_scanner,
                                (T_pformatungetc)&vfscanf_return,
                                 self,format,locale,args);
}
#undef vfscanf_scanner
#undef vfscanf_return
#else /* CHARACTER_TYPE == CHARACTER_TYPE_CHAR */
PRIVATE ssize_t LIBCCALL
libc_format_X(vfscanf_scanner)(FILE *__restrict self, T_char *__restrict pch) {
 wint_t result;
#ifdef CONFIG_LIBC_USES_NEW_STDIO
#if CHARACTER_TYPE == CHARACTER_TYPE_CHAR16
 result = libc_fgetwc16(self);
#else
 result = libc_fgetwc32(self);
#endif
#elif CHARACTER_TYPE == CHARACTER_TYPE_CHAR16
 result = libc_16fgetwc(self);
#else
 result = libc_32fgetwc(self);
#endif
 if (result == WEOF)
     return -1;
 *pch = (T_char)result;
 return 0;
}
PRIVATE ssize_t LIBCCALL
libc_format_X(vfscanf_return)(T_char c, FILE *__restrict self) {
#ifdef CONFIG_LIBC_USES_NEW_STDIO
#if CHARACTER_TYPE == CHARACTER_TYPE_CHAR16
 return libc_ungetwc16((wint_t)c,self);
#else
 return libc_ungetwc32((wint_t)c,self);
#endif
#elif CHARACTER_TYPE == CHARACTER_TYPE_CHAR16
 return libc_16ungetwc((wint_t)c,self);
#else
 return libc_32ungetwc((wint_t)c,self);
#endif
}

INTERN ssize_t LIBCCALL
libc_vfwX(scanf_l)(FILE *__restrict self, T_char const *__restrict format,
                   locale_t locale, va_list args) {
 return libc_format_vX(scanf_l)((T_pformatgetc)&libc_format_X(vfscanf_scanner),
                                (T_pformatungetc)&libc_format_X(vfscanf_return),
                                 self,format,locale,args);
}
INTERN ssize_t LIBCCALL
libd_vfwX(scanf_l)(FILE *__restrict self, T_char const *__restrict format,
                   locale_t locale, va_list args) {
 return libd_format_vX(scanf_l)((T_pformatgetc)&libc_format_X(vfscanf_scanner),
                                (T_pformatungetc)&libc_format_X(vfscanf_return),
                                 self,format,locale,args);
}
#endif /* CHARACTER_TYPE != CHARACTER_TYPE_CHAR */
INTERN ssize_t LIBCCALL
libc_vwX(scanf_l)(T_char const *__restrict format,
                  locale_t locale, va_list args) {
 return libc_vfwX(scanf_l)(stdin,format,locale,args);
}
INTERN ssize_t LIBCCALL
libd_vwX(scanf_l)(T_char const *__restrict format,
                  locale_t locale, va_list args) {
 return libd_vfwX(scanf_l)(stdin,format,locale,args);
}

INTERN ssize_t
(LIBCCALL libc_vwX(scanf))(T_char const *__restrict format, va_list args) {
 return libc_vwX(scanf_l)(format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_vwX(scanf))(T_char const *__restrict format, va_list args) {
 return libd_vwX(scanf_l)(format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libc_vfwX(scanf))(FILE *__restrict self, T_char const *__restrict format, va_list args) {
 return libc_vfwX(scanf_l)(self,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_vfwX(scanf))(FILE *__restrict self, T_char const *__restrict format, va_list args) {
 return libd_vfwX(scanf_l)(self,format,NULL,args);
}




#if CHARACTER_TYPE == CHARACTER_TYPE_CHAR
struct libc_stringX(strdup_formatdata) { T_char *start,*iter,*end; };
PRIVATE NONNULL((1,3)) ssize_t
LIBCCALL libc_stringX(strdupf_printer)(T_char const *__restrict data, size_t datalen,
                                       struct libc_stringX(strdup_formatdata) *__restrict fmt) {
 T_char *newiter;
 newiter = fmt->iter+datalen;
 if (newiter > fmt->end) {
  size_t newsize = (size_t)(fmt->end-fmt->start);
  assert(newsize);
  do newsize *= 2;
  while (fmt->start+newsize < newiter);
  /* Realloc the strdup string */
  newiter = (T_char *)libc_realloc(fmt->start,(newsize+1)*sizeof(T_char));
  if unlikely(!newiter) {
   /* If there isn't enough memory, retry
    * with a smaller buffer before giving up. */
   newsize = (fmt->end-fmt->start)+datalen;
   newiter = (T_char *)libc_realloc(fmt->start,(newsize+1)*sizeof(T_char));
   if unlikely(!newiter) return -1; /* Nothing we can do (out of memory...) */
  }
  fmt->iter = newiter+(fmt->iter-fmt->start);
  fmt->start = newiter;
  fmt->end = newiter+newsize;
 }
 libc_memcpy(fmt->iter,data,datalen);
 fmt->iter += datalen;
 return datalen;
}
PRIVATE ATTR_MALLOC T_char *(LIBCCALL libc_vstrX(dupf_l_impl))
(T_char const *__restrict format, locale_t locale,
 va_list args, int type, size_t *plength) {
 struct libc_stringX(strdup_formatdata) data;
 /* Try to do a (admittedly very bad) prediction on the required size. */
 size_t format_length = (libc_strX(len)(format)*3)/2;
 ssize_t error;
 data.start = (T_char *)libc_malloc((format_length+1)*sizeof(T_char));
 if unlikely(!data.start) {
  /* Failed to allocate initial buffer (try with a smaller one) */
  format_length = 1;
  data.start = (T_char *)libc_malloc(2*sizeof(T_char));
  if unlikely(!data.start) return NULL;
 }
 data.end = data.start+format_length;
 data.iter = data.start;
 switch (type) {
 case 0: error = libc_format_vcX(printf_l)((T_pformatprinter)&libc_stringX(strdupf_printer),&data,format,locale,args); break;
 case 1: error = libd_format_vcX(printf_l)((T_pformatprinter)&libc_stringX(strdupf_printer),&data,format,locale,args); break;
 case 2: error = libc_format_vcX(printf_p_l)((T_pformatprinter)&libc_stringX(strdupf_printer),&data,format,locale,args); break;
 case 3: error = libd_format_vcX(printf_p_l)((T_pformatprinter)&libc_stringX(strdupf_printer),&data,format,locale,args); break;
 default: __builtin_unreachable();
 }
 if unlikely(error < 0) {
  libc_free(data.start); /* Out-of-memory */
  return NULL;
 }
 *data.iter = T_NUL;
 format_length = (size_t)(data.iter-data.start);
 if (plength) *plength = format_length;
 if likely(data.iter != data.end) {
  /* Try to realloc the string one last time to save up on memory */
  data.end = (T_char *)libc_realloc(data.start,
                                   (format_length+1)*
                                    sizeof(T_char));
  if likely(data.end) data.start = data.end;
 }
 return data.start;
}
INTERN ATTR_MALLOC T_char *(LIBCCALL libc_vstrX(dupf_l))
(T_char const *__restrict format, locale_t locale, va_list args) {
 return libc_vstrX(dupf_l_impl)(format,locale,args,0,NULL);
}
INTERN ATTR_MALLOC T_char *(LIBCCALL libd_vstrX(dupf_l))
(T_char const *__restrict format, locale_t locale, va_list args) {
 return libc_vstrX(dupf_l_impl)(format,locale,args,1,NULL);
}
INTERN ATTR_MALLOC T_char *(LIBCCALL libc_vstrX(dupf_p_l))
(T_char const *__restrict format, locale_t locale, va_list args) {
 return libc_vstrX(dupf_l_impl)(format,locale,args,2,NULL);
}
INTERN ATTR_MALLOC T_char *(LIBCCALL libd_vstrX(dupf_p_l))
(T_char const *__restrict format, locale_t locale, va_list args) {
 return libc_vstrX(dupf_l_impl)(format,locale,args,3,NULL);
}

PRIVATE NONNULL((1,3)) ssize_t
LIBCCALL libc_stringX(Xstrdupf_printer)(T_char const *__restrict data, size_t datalen,
                                        struct libc_stringX(strdup_formatdata) *__restrict fmt) {
 T_char *newiter;
 newiter = fmt->iter+datalen;
 if (newiter > fmt->end) {
  size_t newsize = (size_t)(fmt->end-fmt->start);
  assert(newsize);
  do newsize *= 2;
  while (fmt->start+newsize < newiter);
  /* Realloc the strdup string */
  LIBC_TRY {
   newiter = (T_char *)libc_Xrealloc(fmt->start,(newsize+1)*sizeof(T_char));
  } LIBC_CATCH (E_BADALLOC) {
   /* If there isn't enough memory, retry
    * with a smaller buffer before giving up. */
   newsize = (fmt->end-fmt->start)+datalen;
   newiter = (T_char *)libc_Xrealloc(fmt->start,(newsize+1)*sizeof(T_char));
  }
  fmt->iter = newiter+(fmt->iter-fmt->start);
  fmt->start = newiter;
  fmt->end = newiter+newsize;
 }
 libc_memcpy(fmt->iter,data,datalen);
 fmt->iter += datalen;
 return datalen;
}
PRIVATE ATTR_RETNONNULL ATTR_MALLOC T_char *(LIBCCALL libc_XvstrX(dupf_l_impl))
(T_char const *__restrict format, locale_t locale,
 va_list args, int type, size_t *plength) {
 struct libc_stringX(strdup_formatdata) data;
 /* Try to do a (admittedly very bad) prediction on the required size. */
 size_t format_length = (libc_strX(len)(format)*3)/2;
 ssize_t error;
 LIBC_TRY {
  data.start = (T_char *)libc_Xmalloc((format_length+1)*sizeof(T_char));
 } LIBC_CATCH(E_BADALLOC) {
  /* Failed to allocate initial buffer (try with a smaller one) */
  data.start = (T_char *)libc_Xmalloc(2*sizeof(T_char));
  format_length = 1;
 }
 data.end = data.start+format_length;
 data.iter = data.start;
 switch (type) {
 case 0: error = libc_format_vcX(printf_l)((T_pformatprinter)&libc_stringX(Xstrdupf_printer),&data,format,locale,args); break;
 case 1: error = libd_format_vcX(printf_l)((T_pformatprinter)&libc_stringX(Xstrdupf_printer),&data,format,locale,args); break;
 case 2: error = libc_format_vcX(printf_p_l)((T_pformatprinter)&libc_stringX(Xstrdupf_printer),&data,format,locale,args); break;
 case 3: error = libd_format_vcX(printf_p_l)((T_pformatprinter)&libc_stringX(Xstrdupf_printer),&data,format,locale,args); break;
 default: __builtin_unreachable();
 }
 if unlikely(error < 0) {
  libc_free(data.start); /* Out-of-memory */
  return NULL;
 }
 *data.iter = T_NUL;
 format_length = (size_t)(data.iter-data.start);
 if (plength) *plength = format_length;
 if likely(data.iter != data.end) {
  /* Try to realloc the string one last time to save up on memory */
  data.end = (T_char *)libc_realloc(data.start,
                                   (format_length+1)*
                                    sizeof(T_char));
  if likely(data.end) data.start = data.end;
 }
 return data.start;
}
INTERN ATTR_RETNONNULL ATTR_MALLOC T_char *(LIBCCALL libc_XvstrX(dupf_l))
(T_char const *__restrict format, locale_t locale, va_list args) {
 return libc_XvstrX(dupf_l_impl)(format,locale,args,0,NULL);
}
INTERN ATTR_RETNONNULL ATTR_MALLOC T_char *(LIBCCALL libd_XvstrX(dupf_l))
(T_char const *__restrict format, locale_t locale, va_list args) {
 return libc_XvstrX(dupf_l_impl)(format,locale,args,1,NULL);
}
INTERN ATTR_RETNONNULL ATTR_MALLOC T_char *(LIBCCALL libc_XvstrX(dupf_p_l))
(T_char const *__restrict format, locale_t locale, va_list args) {
 return libc_XvstrX(dupf_l_impl)(format,locale,args,2,NULL);
}
INTERN ATTR_RETNONNULL ATTR_MALLOC T_char *(LIBCCALL libd_XvstrX(dupf_p_l))
(T_char const *__restrict format, locale_t locale, va_list args) {
 return libc_XvstrX(dupf_l_impl)(format,locale,args,3,NULL);
}

INTERN ssize_t LIBCCALL
libc_vaswX(printf_l)(T_char **__restrict ptr,
                     T_char const *__restrict format,
                     locale_t locale, va_list args) {
 size_t result;
 if ((*ptr = libc_vstrX(dupf_l_impl)(format,locale,args,0,&result)) == NULL)
     return -1;
 return (ssize_t)result;
}
INTERN ssize_t LIBCCALL
libd_vaswX(printf_l)(T_char **__restrict ptr,
                     T_char const *__restrict format,
                     locale_t locale, va_list args) {
 size_t result;
 if ((*ptr = libc_vstrX(dupf_l_impl)(format,locale,args,1,&result)) == NULL)
     return -1;
 return (ssize_t)result;
}
INTERN ssize_t LIBCCALL
libc_vaswX(printf_p_l)(T_char **__restrict ptr,
                       T_char const *__restrict format,
                       locale_t locale, va_list args) {
 size_t result;
 if ((*ptr = libc_vstrX(dupf_l_impl)(format,locale,args,2,&result)) == NULL)
     return -1;
 return (ssize_t)result;
}
INTERN ssize_t LIBCCALL
libd_vaswX(printf_p_l)(T_char **__restrict ptr,
                       T_char const *__restrict format,
                       locale_t locale, va_list args) {
 size_t result;
 if ((*ptr = libc_vstrX(dupf_l_impl)(format,locale,args,3,&result)) == NULL)
     return -1;
 return (ssize_t)result;
}
INTERN ATTR_MALLOC T_char *
(LIBCCALL libc_vstrX(dupf))(T_char const *__restrict format, va_list args) {
 return libc_vstrX(dupf_l)(format,NULL,args);
}
INTERN ATTR_MALLOC T_char *
(LIBCCALL libd_vstrX(dupf))(T_char const *__restrict format, va_list args) {
 return libd_vstrX(dupf_l)(format,NULL,args);
}
INTERN ATTR_MALLOC T_char *
(LIBCCALL libc_vstrX(dupf_p))(T_char const *__restrict format, va_list args) {
 return libc_vstrX(dupf_p_l)(format,NULL,args);
}
INTERN ATTR_MALLOC T_char *
(LIBCCALL libd_vstrX(dupf_p))(T_char const *__restrict format, va_list args) {
 return libd_vstrX(dupf_p_l)(format,NULL,args);
}
INTERN ATTR_RETNONNULL ATTR_MALLOC T_char *
(LIBCCALL libc_XvstrX(dupf))(T_char const *__restrict format, va_list args) {
 return libc_XvstrX(dupf_l)(format,NULL,args);
}
INTERN ATTR_RETNONNULL ATTR_MALLOC T_char *
(LIBCCALL libd_XvstrX(dupf))(T_char const *__restrict format, va_list args) {
 return libd_XvstrX(dupf_l)(format,NULL,args);
}
INTERN ATTR_RETNONNULL ATTR_MALLOC T_char *
(LIBCCALL libc_XvstrX(dupf_p))(T_char const *__restrict format, va_list args) {
 return libc_XvstrX(dupf_p_l)(format,NULL,args);
}
INTERN ATTR_RETNONNULL ATTR_MALLOC T_char *
(LIBCCALL libd_XvstrX(dupf_p))(T_char const *__restrict format, va_list args) {
 return libd_XvstrX(dupf_p_l)(format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libc_vaswX(printf))(T_char **__restrict ptr,
                              T_char const *__restrict format,
                              va_list args) {
 return libc_vaswX(printf_l)(ptr,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_vaswX(printf))(T_char **__restrict ptr,
                              T_char const *__restrict format,
                              va_list args) {
 return libd_vaswX(printf_l)(ptr,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libc_vaswX(printf_p))(T_char **__restrict ptr,
                                T_char const *__restrict format,
                                va_list args) {
 return libc_vaswX(printf_p_l)(ptr,format,NULL,args);
}
INTERN ssize_t
(LIBCCALL libd_vaswX(printf_p))(T_char **__restrict ptr,
                                T_char const *__restrict format,
                                va_list args) {
 return libd_vaswX(printf_p_l)(ptr,format,NULL,args);
}
#endif /* CHARACTER_TYPE == CHARACTER_TYPE_CHAR */


#ifndef NOP_PRINTER_DEFINED
#define NOP_PRINTER_DEFINED 1
PRIVATE ssize_t LIBCCALL
nop_printer(void *UNUSED(data), size_t datalen, void *UNUSED(arg)) {
 return (ssize_t)datalen;
}
#endif /* !NOP_PRINTER_DEFINED */


/* Weird functions required for DOS compatibility */
INTERN size_t LIBCCALL
libc_vsnwX(printf_c_l)(T_char *buf, size_t buflen,
                       T_char const *__restrict format,
                       locale_t locale, va_list args) {
 if (buflen)
     return libc_vsnwX(printf_l)(buf,buflen,format,locale,args);
 /* When ZERO(0) was passed for `buflen', return the required buffer size. */
 return (size_t)libc_format_vcX(printf_l)((T_pformatprinter)&nop_printer,NULL,format,locale,args);
}
INTERN size_t LIBCCALL
libd_vsnwX(printf_c_l)(T_char *buf, size_t buflen,
                       T_char const *__restrict format,
                       locale_t locale, va_list args) {
 if (buflen)
     return libd_vsnwX(printf_l)(buf,buflen,format,locale,args);
 /* When ZERO(0) was passed for `buflen', return the required buffer size. */
 return (size_t)libd_format_vcX(printf_l)((T_pformatprinter)&nop_printer,NULL,format,locale,args);
}
INTERN size_t
(LIBCCALL libc_vsnwX(printf_c))(T_char *buf, size_t buflen,
                                T_char const *__restrict format,
                                va_list args) {
 return libc_vsnwX(printf_c_l)(buf,buflen,format,NULL,args);
}
INTERN size_t
(LIBCCALL libd_vsnwX(printf_c))(T_char *buf, size_t buflen,
                                T_char const *__restrict format,
                                va_list args) {
 return libd_vsnwX(printf_c_l)(buf,buflen,format,NULL,args);
}


/* DOS's count-format functions (simply return how much characters are used by the format string) */
INTDEF size_t LIBCCALL
libc_vscwX(printf_l)(T_char const *__restrict format,
                     locale_t locale, va_list args) {
 return (size_t)libc_format_vcX(printf_l)((T_pformatprinter)&nop_printer,NULL,format,locale,args);
}
INTDEF size_t LIBCCALL
libd_vscwX(printf_l)(T_char const *__restrict format,
                     locale_t locale, va_list args) {
 return (size_t)libd_format_vcX(printf_l)((T_pformatprinter)&nop_printer,NULL,format,locale,args);
}
INTDEF size_t LIBCCALL
libc_vscwX(printf_p_l)(T_char const *__restrict format,
                       locale_t locale, va_list args) {
 return (size_t)libc_format_vcX(printf_p_l)((T_pformatprinter)&nop_printer,NULL,format,locale,args);
}
INTDEF size_t LIBCCALL
libd_vscwX(printf_p_l)(T_char const *__restrict format,
                       locale_t locale, va_list args) {
 return (size_t)libd_format_vcX(printf_p_l)((T_pformatprinter)&nop_printer,NULL,format,locale,args);
}

INTDEF size_t
(LIBCCALL libc_vscwX(printf))(T_char const *__restrict format, va_list args) {
 return libc_vscwX(printf_l)(format,NULL,args);
}
INTDEF size_t
(LIBCCALL libd_vscwX(printf))(T_char const *__restrict format, va_list args) {
 return libd_vscwX(printf_l)(format,NULL,args);
}
INTDEF size_t
(LIBCCALL libc_vscwX(printf_p))(T_char const *__restrict format, va_list args) {
 return libc_vscwX(printf_p_l)(format,NULL,args);
}
INTDEF size_t
(LIBCCALL libd_vscwX(printf_p))(T_char const *__restrict format, va_list args) {
 return libd_vscwX(printf_p_l)(format,NULL,args);
}


#undef T_NUL
#undef T_char
#undef T_stringprinter
#undef T_pformatprinter
#undef T_pformatgetc
#undef T_pformatungetc
#undef T_buffer
#undef T_printer
#undef libc_stringX
#undef libc_XstringX
#undef libc_XwX
#undef libc_wX
#undef libc_vwX
#undef libc_swX
#undef libc_vswX
#undef libc_snwX
#undef libc_vsnwX
#undef libc_scwX
#undef libc_vscwX
#undef libc_aswX
#undef libc_vaswX
#undef libc_fwX
#undef libc_vfwX
#undef libc_strX
#undef libc_vstrX
#undef libc_XstrX
#undef libc_XvstrX
#undef libc_format_cX
#undef libc_format_vcX
#undef libc_format_X
#undef libc_format_vX
#undef libc_format_strX
#undef libd_stringX
#undef libd_XstringX
#undef libd_XwX
#undef libd_wX
#undef libd_vwX
#undef libd_swX
#undef libd_vswX
#undef libd_snwX
#undef libd_vsnwX
#undef libd_scwX
#undef libd_vscwX
#undef libd_aswX
#undef libd_vaswX
#undef libd_fwX
#undef libd_vfwX
#undef libd_strX
#undef libd_vstrX
#undef libd_XstrX
#undef libd_XvstrX
#undef libd_format_cX
#undef libd_format_vcX
#undef libd_format_X
#undef libd_format_vX
#undef CHARACTER_TYPE

DECL_END

