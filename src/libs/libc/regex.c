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
#ifndef GUARD_LIBS_LIBC_REGEX_C
#define GUARD_LIBS_LIBC_REGEX_C 1

#include "libc.h"
#include "regex.h"

#include <regex.h>
#include <hybrid/section.h>
#include <hybrid/xch.h>

DECL_BEGIN

#define ATTR_RE_TEXT  ATTR_SECTION(".text.regex")


PUBLIC ATTR_RAREDATA reg_syntax_t re_syntax_options = 0;
INTERN ATTR_RE_TEXT reg_syntax_t LIBCCALL libc_re_set_syntax(reg_syntax_t syntax) { return XCH(re_syntax_options,syntax); }

INTERN ATTR_RE_TEXT char const *LIBCCALL libc_re_compile_pattern(char const *pattern, size_t length, struct re_pattern_buffer *buffer) { return NULL; }
INTERN ATTR_RE_TEXT int LIBCCALL libc_re_compile_fastmap(struct re_pattern_buffer *buffer) { return REG_ENOSYS; }
INTERN ATTR_RE_TEXT int LIBCCALL libc_re_search(struct re_pattern_buffer *buffer, char const *string, int length, int start, int range, struct re_registers *regs) { return REG_ENOSYS; }
INTERN ATTR_RE_TEXT int LIBCCALL libc_re_search_2(struct re_pattern_buffer *buffer, char const *string1, int length1, char const *string2, int length2, int start, int range, struct re_registers *regs, int stop) { return REG_ENOSYS; }
INTERN ATTR_RE_TEXT int LIBCCALL libc_re_match(struct re_pattern_buffer *buffer, char const *string, int length, int start, struct re_registers *regs) { return REG_ENOSYS; }
INTERN ATTR_RE_TEXT int LIBCCALL libc_re_match_2(struct re_pattern_buffer *buffer, char const *string1, int length1, char const *string2, int length2, int start, struct re_registers *regs, int stop) { return REG_ENOSYS; }
INTERN ATTR_RE_TEXT void LIBCCALL libc_re_set_registers(struct re_pattern_buffer *buffer, struct re_registers *regs, unsigned int num_regs, regoff_t *starts, regoff_t *ends) { }
INTERN ATTR_RE_TEXT char *LIBCCALL libc_re_comp(char const *str) { return NULL; }
INTERN ATTR_RE_TEXT int LIBCCALL libc_re_exec(char const *str) { return REG_ENOSYS; }
INTERN ATTR_RE_TEXT int LIBCCALL libc_regcomp(regex_t *__restrict preg, char const *__restrict pattern, int cflags) { return REG_ENOSYS; }
INTERN ATTR_RE_TEXT int LIBCCALL libc_regexec(regex_t const *__restrict preg, char const *__restrict string, size_t nmatch, regmatch_t pmatch[__restrict_arr], int eflags) { return REG_ENOSYS; }
INTERN ATTR_RE_TEXT size_t LIBCCALL libc_regerror(int errcode, regex_t const *__restrict preg, char *__restrict errbuf, size_t errbuf_size) { return 0; }
INTERN ATTR_RE_TEXT void LIBCCALL libc_regfree(regex_t *preg) { }



EXPORT(re_set_syntax,libc_re_set_syntax);
EXPORT(re_compile_pattern,libc_re_compile_pattern);
EXPORT(re_compile_fastmap,libc_re_compile_fastmap);
EXPORT(re_search,libc_re_search);
EXPORT(re_search_2,libc_re_search_2);
EXPORT(re_match,libc_re_match);
EXPORT(re_match_2,libc_re_match_2);
EXPORT(re_set_registers,libc_re_set_registers);
EXPORT(re_comp,libc_re_comp);
EXPORT(re_exec,libc_re_exec);
EXPORT(regcomp,libc_regcomp);
EXPORT(regexec,libc_regexec);
EXPORT(regerror,libc_regerror);
EXPORT(regfree,libc_regfree);


DECL_END

#endif /* !GUARD_LIBS_LIBC_REGEX_C */
