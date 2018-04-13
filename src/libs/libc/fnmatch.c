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
#ifndef GUARD_LIBS_LIBC_FNMATCH_C
#define GUARD_LIBS_LIBC_FNMATCH_C 1

#include "libc.h"
#include "fnmatch.h"

#include <fnmatch.h>

DECL_BEGIN

INTERN int LIBCCALL
libc_fnmatch(char const *pattern,
             char const *name,
             int flags) {
 char card_post;
 for (;;) {
  if (!*name) {
   /* End of name (if the patter is empty, or only contains '*', we have a match) */
   while (*pattern == '*') ++pattern;
   return FNM_NOMATCH;
  }
  if (!*pattern)
      return FNM_NOMATCH; /* Pattern end doesn't match */
  if (*pattern == '*') {
   /* Skip starts */
   do ++pattern; while (*pattern == '*');
   if ((card_post = *pattern++) == '\0')
        return 0; /* Pattern ends with '*' (matches everything) */
   if (card_post == '?') goto next; /* Match any --> already found */
   for (;;) {
    char ch = *name++;
    if (ch == card_post ||
      ((flags & FNM_CASEFOLD) && libc_tolower(ch) == libc_tolower(card_post))) {
     /* Recursively check if the rest of the name and pattern match */
     if (!libc_fnmatch(name,pattern,flags))
          return 0;
    } else if (!ch) {
     return FNM_NOMATCH; /* Wildcard suffix not found */
    } else if (ch == '/') {
     if ((flags & FNM_PATHNAME))
         return FNM_NOMATCH;
     if ((flags & FNM_PERIOD) && name[0] == '.' && card_post != '.')
         return FNM_NOMATCH;
    }
   }
  }
  if (*pattern == *name) {
next:
   ++name,++pattern;
   continue; /* single character match */
  }
  if (*pattern == '?') {
   if (*name == '/') {
    if ((flags & FNM_PATHNAME))
        return FNM_NOMATCH;
    if ((flags & FNM_PERIOD) && name[1] == '.' && pattern[1] != '.')
        return FNM_NOMATCH;
   }
   goto next;
  }
  break; /* mismatch */
 }
 return FNM_NOMATCH;
}


EXPORT(fnmatch,libc_fnmatch);


DECL_END

#endif /* !GUARD_LIBS_LIBC_FNMATCH_C */
