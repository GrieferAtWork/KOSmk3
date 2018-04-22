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
#ifndef GUARD_KERNEL_MODULES_PROCFS_STRINGPRINTER_C
#define GUARD_KERNEL_MODULES_PROCFS_STRINGPRINTER_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <kernel/malloc.h>
#include <except.h>
#include <string.h>
#include <assert.h>

#include "stringprinter.h"

DECL_BEGIN

/* String printer functions for construction of C-style NUL-terminated strings. */
INTERN void KCALL
StringPrinter_Init(struct stringprinter *__restrict self,
                   size_t hint) {
 if (!hint) hint = 16*sizeof(void *);
 self->sp_buffer = (char *)kmalloc((hint+1)*sizeof(char),
                                    GFP_SHARED);
 self->sp_bufpos = self->sp_buffer;
 self->sp_bufend = self->sp_buffer+hint;
 self->sp_bufend[0] = 0;
}
INTERN ATTR_RETNONNULL ATTR_MALLOC char *KCALL
StringPrinter_Pack(struct stringprinter *__restrict EXCEPT_VAR self) {
 char *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(result);
 size_t EXCEPT_VAR result_size;
 assert(self->sp_bufpos >= self->sp_buffer);
 assert(self->sp_bufpos <= self->sp_bufend);
 result_size = (size_t)(self->sp_bufpos-self->sp_buffer);
 if (self->sp_bufpos != self->sp_bufend) {
  TRY {
   result = (char *)krealloc(self->sp_buffer,(result_size+1)*sizeof(char),GFP_SHARED);
  } CATCH (E_BADALLOC) {
   result = self->sp_buffer;
  }
 } else {
  result = self->sp_buffer;
 }
 result[result_size] = 0;
 self->sp_buffer = NULL;
 return result;
}
INTERN void KCALL
StringPrinter_Fini(struct stringprinter *__restrict self) {
 kfree(self->sp_buffer);
}
INTERN ssize_t KCALL
StringPrinter_Print(char const *__restrict data,
                    size_t datalen, void *closure) {
 struct stringprinter *self;
 size_t size_avail,newsize,reqsize;
 char *COMPILER_IGNORE_UNINITIALIZED(new_buffer);
 self = (struct stringprinter *)closure;
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
  TRY {
   new_buffer = (char *)krealloc(self->sp_buffer,(newsize+1)*sizeof(char),GFP_SHARED);
  } CATCH (E_BADALLOC) {
   newsize    = reqsize;
   new_buffer = (char *)krealloc(self->sp_buffer,(newsize+1)*sizeof(char),GFP_SHARED);
  }
  self->sp_bufpos = new_buffer+(self->sp_bufpos-self->sp_buffer);
  self->sp_bufend = new_buffer+newsize;
  self->sp_buffer = new_buffer;
 }
 memcpy(self->sp_bufpos,data,datalen*sizeof(char));
 self->sp_bufpos += datalen;
 assert(self->sp_bufpos <= self->sp_bufend);
 return (ssize_t)datalen;
}

DECL_END

#endif /* !GUARD_KERNEL_MODULES_PROCFS_STRINGPRINTER_C */
