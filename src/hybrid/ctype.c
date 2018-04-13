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
#ifndef GUARD_HYBRID_CTYPE_C
#define GUARD_HYBRID_CTYPE_C 1

#include <hybrid/compiler.h>
#include "hybrid.h"
#include <bits/dos-ctype.h>
#include <hybrid/byteswap.h>

DECL_BEGIN

INTERN u16 const libc_chattr[256] = {
/*[[[deemon
function attrof(ch) {
    local result = 0;
    if (ASCII_ISCNTRL(ch))  result |= CTYPE_CNTRL;
    if (ASCII_ISBLANK(ch))  result |= CTYPE_BLANK;
    if (ASCII_ISSPACE(ch))  result |= CTYPE_SPACE;
    if (ASCII_ISUPPER(ch))  result |= CTYPE_UPPER;
    if (ASCII_ISLOWER(ch))  result |= CTYPE_LOWER;
    if (ASCII_ISALPHA(ch))  result |= CTYPE_ALPHA;
    if (ASCII_ISDIGIT(ch))  result |= CTYPE_DIGIT;
    if (ASCII_ISXDIGIT(ch)) result |= CTYPE_XDIGIT;
    if (ASCII_ISALNUM(ch))  result |= CTYPE_ALNUM;
    if (ASCII_ISPUNCT(ch))  result |= CTYPE_PUNCT;
    if (ASCII_ISGRAPH(ch))  result |= CTYPE_GRAPH;
    if (ASCII_ISPRINT(ch))  result |= CTYPE_PRINT;
    return result;
}
for (local c = 0; c < 256; ++c) {
    if (!(c % 16)) print c ? "\n    " : "    ",;
    print "CTYPE_SLOT(%#.4x)," % attrof(c),;
}
]]]*/
    CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0320),CTYPE_SLOT(0x0220),CTYPE_SLOT(0x0220),CTYPE_SLOT(0x0220),CTYPE_SLOT(0x0220),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),
    CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),CTYPE_SLOT(0x0200),
    CTYPE_SLOT(0x0160),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),
    CTYPE_SLOT(0x08d8),CTYPE_SLOT(0x08d8),CTYPE_SLOT(0x08d8),CTYPE_SLOT(0x08d8),CTYPE_SLOT(0x08d8),CTYPE_SLOT(0x08d8),CTYPE_SLOT(0x08d8),CTYPE_SLOT(0x08d8),CTYPE_SLOT(0x08d8),CTYPE_SLOT(0x08d8),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),
    CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x08d5),CTYPE_SLOT(0x08d5),CTYPE_SLOT(0x08d5),CTYPE_SLOT(0x08d5),CTYPE_SLOT(0x08d5),CTYPE_SLOT(0x08d5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),
    CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x08c5),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),
    CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x08d6),CTYPE_SLOT(0x08d6),CTYPE_SLOT(0x08d6),CTYPE_SLOT(0x08d6),CTYPE_SLOT(0x08d6),CTYPE_SLOT(0x08d6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),
    CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x08c6),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x04c0),CTYPE_SLOT(0x0200),
    CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),
    CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),
    CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),
    CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),
    CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),
    CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),
    CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),
    CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),CTYPE_SLOT(0x0000),
//[[[end]]]
};


INTERN int (LIBCCALL libc_isalpha)(int ch)  { return libc_isalpha(ch); }
INTERN int (LIBCCALL libc_isupper)(int ch)  { return libc_isupper(ch); }
INTERN int (LIBCCALL libc_islower)(int ch)  { return libc_islower(ch); }
INTERN int (LIBCCALL libc_isdigit)(int ch)  { return libc_isdigit(ch); }
INTERN int (LIBCCALL libc_isxdigit)(int ch) { return libc_isxdigit(ch); }
INTERN int (LIBCCALL libc_isspace)(int ch)  { return libc_isspace(ch); }
INTERN int (LIBCCALL libc_ispunct)(int ch)  { return libc_ispunct(ch); }
INTERN int (LIBCCALL libc_isalnum)(int ch)  { return libc_isalnum(ch); }
INTERN int (LIBCCALL libc_isprint)(int ch)  { return libc_isprint(ch); }
INTERN int (LIBCCALL libc_isgraph)(int ch)  { return libc_isgraph(ch); }
INTERN int (LIBCCALL libc_iscntrl)(int ch)  { return libc_iscntrl(ch); }
INTERN int (LIBCCALL libc_isblank)(int ch)  { return libc_isblank(ch); }
INTERN int (LIBCCALL libc_isascii)(int ch)  { return libc_isascii(ch); }
INTERN int (LIBCCALL libc_toupper)(int ch)  { return ASCII_TOUPPER(ch); }
INTERN int (LIBCCALL libc_tolower)(int ch)  { return ASCII_TOLOWER(ch); }
INTERN int (LIBCCALL libc_isctype)(int ch, int mask) { return libc_isctype(ch,mask); }

#undef __chattr
EXPORT(__chattr,libc_chattr);
EXPORT(isalpha,libc_isalpha);
EXPORT(isupper,libc_isupper);
EXPORT(islower,libc_islower);
EXPORT(isdigit,libc_isdigit);
EXPORT(isxdigit,libc_isxdigit);
EXPORT(isspace,libc_isspace);
EXPORT(ispunct,libc_ispunct);
EXPORT(isalnum,libc_isalnum);
EXPORT(isprint,libc_isprint);
EXPORT(isgraph,libc_isgraph);
EXPORT(iscntrl,libc_iscntrl);
EXPORT(isblank,libc_isblank);
EXPORT(toupper,libc_toupper);
EXPORT(tolower,libc_tolower);
EXPORT(isctype,libc_isctype);
EXPORT(isascii,libc_isascii);
#ifndef CONFIG_LIBC_LIMITED_API
EXPORT(__isascii,libc_isascii);
#endif



#ifndef CONFIG_LIBC_LIMITED_API
INTERN int (LIBCCALL libc__tolower)(int ch) { return ch+0x20; }
INTERN int (LIBCCALL libc__toupper)(int ch) { return ch-0x20; }
INTERN int LIBCCALL libc_toascii(int ch) { return ch&0x7f; }
INTERN int LIBCCALL libc_iscsym(int ch) { return libc_isctype(ch,CTYPE_SLOT(CTYPE_ALPHA|CTYPE_DIGIT)) || ch == '_'; }
INTERN int LIBCCALL libc_iscsymf(int ch) { return libc_isctype(ch,CTYPE_SLOT(CTYPE_ALPHA)) || ch == '_'; }
INTERN int LIBCCALL libc_isleadbyte(int wc) { return wc >= 192 && wc <= 255; }
INTERN int LIBCCALL libc_dos_isctype(int ch, int mask) {
 int check_mask = 0,result;
 if (mask&__DOS_UPPER) check_mask |= CTYPE_SLOT(CTYPE_UPPER);
 if (mask&__DOS_LOWER) check_mask |= CTYPE_SLOT(CTYPE_LOWER);
 if (mask&__DOS_DIGIT) check_mask |= CTYPE_SLOT(CTYPE_DIGIT);
 if (mask&__DOS_SPACE) check_mask |= CTYPE_SLOT(CTYPE_SPACE);
 if (mask&__DOS_PUNCT) check_mask |= CTYPE_SLOT(CTYPE_PUNCT);
 if (mask&__DOS_CONTROL) check_mask |= CTYPE_SLOT(CTYPE_CNTRL);
 if (mask&__DOS_BLANK) check_mask |= CTYPE_SLOT(CTYPE_BLANK);
 if (mask&__DOS_HEX) check_mask |= CTYPE_SLOT(CTYPE_XDIGIT);
 if (mask&__DOS_ALPHA) check_mask |= CTYPE_SLOT(CTYPE_XDIGIT);
 result = libc_isctype(ch,check_mask);
 if (mask&__DOS_LEADBYTE) result |= libc_isleadbyte(ch);
 return result;
}
#if __SIZEOF_WINT_T__ == __SIZEOF_INT__ || defined(__x86_64__) || defined(__arm__)
DEFINE_INTERN_ALIAS(libc_iswalnum,libc_isalnum);
DEFINE_INTERN_ALIAS(libc_iswalpha,libc_isalpha);
DEFINE_INTERN_ALIAS(libc_iswcntrl,libc_iscntrl);
DEFINE_INTERN_ALIAS(libc_iswdigit,libc_isdigit);
DEFINE_INTERN_ALIAS(libc_iswgraph,libc_isgraph);
DEFINE_INTERN_ALIAS(libc_iswlower,libc_islower);
DEFINE_INTERN_ALIAS(libc_iswprint,libc_isprint);
DEFINE_INTERN_ALIAS(libc_iswpunct,libc_ispunct);
DEFINE_INTERN_ALIAS(libc_iswspace,libc_isspace);
DEFINE_INTERN_ALIAS(libc_iswupper,libc_isupper);
DEFINE_INTERN_ALIAS(libc_iswxdigit,libc_isxdigit);
DEFINE_INTERN_ALIAS(libc_iswblank,libc_isblank);
DEFINE_INTERN_ALIAS(libc_iswascii,libc_isascii);
DEFINE_INTERN_ALIAS(libc_iswcsym,libc_iscsym);
DEFINE_INTERN_ALIAS(libc_iswcsymf,libc_iscsymf);
DEFINE_INTERN_ALIAS(libc_iswctype,libc_isctype);
#else
INTERN int LIBCCALL libc_iswalnum(wint_t wc) { return libc_isalnum((int)wc); }
INTERN int LIBCCALL libc_iswalpha(wint_t wc) { return libc_isalpha((int)wc); }
INTERN int LIBCCALL libc_iswcntrl(wint_t wc) { return libc_iscntrl((int)wc); }
INTERN int LIBCCALL libc_iswdigit(wint_t wc) { return libc_isdigit((int)wc); }
INTERN int LIBCCALL libc_iswgraph(wint_t wc) { return libc_isgraph((int)wc); }
INTERN int LIBCCALL libc_iswlower(wint_t wc) { return libc_islower((int)wc); }
INTERN int LIBCCALL libc_iswprint(wint_t wc) { return libc_isprint((int)wc); }
INTERN int LIBCCALL libc_iswpunct(wint_t wc) { return libc_ispunct((int)wc); }
INTERN int LIBCCALL libc_iswspace(wint_t wc) { return libc_isspace((int)wc); }
INTERN int LIBCCALL libc_iswupper(wint_t wc) { return libc_isupper((int)wc); }
INTERN int LIBCCALL libc_iswxdigit(wint_t wc) { return libc_isxdigit((int)wc); }
INTERN int LIBCCALL libc_iswblank(wint_t wc) { return libc_isblank((int)wc); }
INTERN int LIBCCALL libc_iswascii(wint_t wc) { return libc_isascii((int)wc); }
INTERN int LIBCCALL libc_iswcsym(wint_t wc) { return libc_iscsym((int)wc); }
INTERN int LIBCCALL libc_iswcsymf(wint_t wc) { return libc_iscsymf((int)wc); }
INTERN int LIBCCALL libc_iswctype(wint_t wc, wctype_t desc) { return libc_isctype((int)wc,(int)desc); }
#endif /* __SIZEOF_WINT_T__ != __SIZEOF_INT__ */
#if __SIZEOF_WINT_T__ == __SIZEOF_INT__
DEFINE_INTERN_ALIAS(libc_towlower,libc_tolower);
DEFINE_INTERN_ALIAS(libc_towupper,libc_toupper);
#elif __SIZEOF_WINT_T__ > __SIZEOF_INT__
INTERN wint_t LIBCCALL libc_towlower(wint_t wc) { return wc < 256 ? (wint_t)libc_tolower((int)wc) : wc; }
INTERN wint_t LIBCCALL libc_towupper(wint_t wc) { return wc < 256 ? (wint_t)libc_toupper((int)wc) : wc; }
#else
INTERN wint_t LIBCCALL libc_towlower(wint_t wc) { return (wint_t)libc_tolower((int)wc); }
INTERN wint_t LIBCCALL libc_towupper(wint_t wc) { return (wint_t)libc_toupper((int)wc); }
#endif

struct prop { char name[8]; };
PRIVATE struct prop const prop_names[12] = {
    { "upper"  }, /* UPPERCASE. */
    { "lower"  }, /* lowercase. */
    { "alpha"  }, /* Alphabetic. */
    { "digit"  }, /* Numeric. */
    { "xdigit" }, /* Hexadecimal numeric. */
    { "space"  }, /* Whitespace. */
    { "print"  }, /* Printing. */
    { "graph"  }, /* Graphical. */
    { "blank"  }, /* Blank (usually SPC and TAB). */
    { "cntrl"  }, /* Control character. */
    { "punct"  }, /* Punctuation. */
    { "alnum"  }  /* Alphanumeric. */
};
INTERN wctype_t LIBCCALL libc_wctype(char const *prop) {
 struct prop const *iter = prop_names;
 for (; iter != COMPILER_ENDOF(prop_names); ++iter) {
  if (!libc_strcmp(iter->name,prop)) {
   int bit = (int)(iter-prop_names);
   return (u16)BSWAP_H2BE16(1 << bit);
  }
 }
 return 0;
}
struct trans { char name[8]; };
#define TRANS_LOWER 0
#define TRANS_UPPER 1
PRIVATE struct trans const trans_names[2] = {
    [TRANS_LOWER] = { "tolower" },
    [TRANS_UPPER] = { "toupper" },
};
INTERN wctrans_t LIBCCALL libc_wctrans(char const *prop) {
 struct trans const *iter = trans_names;
 for (; iter != COMPILER_ENDOF(trans_names); ++iter) {
  if (!libc_strcmp(iter->name,prop))
       return (wctrans_t)((uintptr_t)(iter-trans_names));
 }
 return 0;
}
INTERN wint_t LIBCCALL libc_towctrans(wint_t wc, wctrans_t desc) {
 switch ((uintptr_t)desc) {
 case TRANS_LOWER: return libc_towlower(wc);
 case TRANS_UPPER: return libc_towupper(wc);
 default: break;
 }
 return wc;
}



#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_toascii_l,libc_toascii);
DEFINE_INTERN_ALIAS(libc_isascii_l,libc_isascii);
DEFINE_INTERN_ALIAS(libc_isalpha_l,libc_isalpha);
DEFINE_INTERN_ALIAS(libc_isupper_l,libc_isupper);
DEFINE_INTERN_ALIAS(libc_islower_l,libc_islower);
DEFINE_INTERN_ALIAS(libc_isdigit_l,libc_isdigit);
DEFINE_INTERN_ALIAS(libc_isxdigit_l,libc_isxdigit);
DEFINE_INTERN_ALIAS(libc_isspace_l,libc_isspace);
DEFINE_INTERN_ALIAS(libc_ispunct_l,libc_ispunct);
DEFINE_INTERN_ALIAS(libc_isalnum_l,libc_isalnum);
DEFINE_INTERN_ALIAS(libc_isprint_l,libc_isprint);
DEFINE_INTERN_ALIAS(libc_isgraph_l,libc_isgraph);
DEFINE_INTERN_ALIAS(libc_iscntrl_l,libc_iscntrl);
DEFINE_INTERN_ALIAS(libc_iscsym_l,libc_iscsym);
DEFINE_INTERN_ALIAS(libc_iscsymf_l,libc_iscsymf);
DEFINE_INTERN_ALIAS(libc_isblank_l,libc_isblank);
DEFINE_INTERN_ALIAS(libc_isctype_l,libc_isctype);
DEFINE_INTERN_ALIAS(libc_isleadbyte_l,libc_isleadbyte);
DEFINE_INTERN_ALIAS(libc_toupper_l,libc_toupper);
DEFINE_INTERN_ALIAS(libc_tolower_l,libc_tolower);
DEFINE_INTERN_ALIAS(libc_iswalnum_l,libc_iswalnum);
DEFINE_INTERN_ALIAS(libc_iswalpha_l,libc_iswalpha);
DEFINE_INTERN_ALIAS(libc_iswcntrl_l,libc_iswcntrl);
DEFINE_INTERN_ALIAS(libc_iswdigit_l,libc_iswdigit);
DEFINE_INTERN_ALIAS(libc_iswgraph_l,libc_iswgraph);
DEFINE_INTERN_ALIAS(libc_iswlower_l,libc_iswlower);
DEFINE_INTERN_ALIAS(libc_iswprint_l,libc_iswprint);
DEFINE_INTERN_ALIAS(libc_iswpunct_l,libc_iswpunct);
DEFINE_INTERN_ALIAS(libc_iswspace_l,libc_iswspace);
DEFINE_INTERN_ALIAS(libc_iswupper_l,libc_iswupper);
DEFINE_INTERN_ALIAS(libc_iswxdigit_l,libc_iswxdigit);
DEFINE_INTERN_ALIAS(libc_iswblank_l,libc_iswblank);
DEFINE_INTERN_ALIAS(libc_iswcsym_l,libc_iswcsym);
DEFINE_INTERN_ALIAS(libc_iswcsymf_l,libc_iswcsymf);
DEFINE_INTERN_ALIAS(libc_iswctype_l,libc_iswctype);
DEFINE_INTERN_ALIAS(libc_towlower_l,libc_towlower);
DEFINE_INTERN_ALIAS(libc_towupper_l,libc_towupper);
DEFINE_INTERN_ALIAS(libc_wctype_l,libc_wctype);
DEFINE_INTERN_ALIAS(libc_wctrans_l,libc_wctrans);
DEFINE_INTERN_ALIAS(libc_towctrans_l,libc_towctrans);
#else
INTERN int LIBCCALL libc_toascii_l(int ch, locale_t UNUSED(locale)) { return libc_toascii(ch); }
INTERN int LIBCCALL libc_isascii_l(int ch, locale_t UNUSED(locale)) { return libc_isascii(ch); }
INTERN int LIBCCALL libc_isalpha_l(int ch, locale_t UNUSED(locale)) { return libc_isalpha(ch); }
INTERN int LIBCCALL libc_isupper_l(int ch, locale_t UNUSED(locale)) { return libc_isupper(ch); }
INTERN int LIBCCALL libc_islower_l(int ch, locale_t UNUSED(locale)) { return libc_islower(ch); }
INTERN int LIBCCALL libc_isdigit_l(int ch, locale_t UNUSED(locale)) { return libc_isdigit(ch); }
INTERN int LIBCCALL libc_isxdigit_l(int ch, locale_t UNUSED(locale)) { return libc_isxdigit(ch); }
INTERN int LIBCCALL libc_isspace_l(int ch, locale_t UNUSED(locale)) { return libc_isspace(ch); }
INTERN int LIBCCALL libc_ispunct_l(int ch, locale_t UNUSED(locale)) { return libc_ispunct(ch); }
INTERN int LIBCCALL libc_isalnum_l(int ch, locale_t UNUSED(locale)) { return libc_isalnum(ch); }
INTERN int LIBCCALL libc_isprint_l(int ch, locale_t UNUSED(locale)) { return libc_isprint(ch); }
INTERN int LIBCCALL libc_isgraph_l(int ch, locale_t UNUSED(locale)) { return libc_isgraph(ch); }
INTERN int LIBCCALL libc_iscntrl_l(int ch, locale_t UNUSED(locale)) { return libc_iscntrl(ch); }
INTERN int LIBCCALL libc_iscsym_l(int wc, locale_t UNUSED(locale)) { return libc_iscsym(wc); }
INTERN int LIBCCALL libc_iscsymf_l(int wc, locale_t UNUSED(locale)) { return libc_iscsymf(wc); }
INTERN int LIBCCALL libc_isblank_l(int ch, locale_t UNUSED(locale)) { return libc_isblank(ch); }
INTERN int LIBCCALL libc_isctype_l(int ch, int mask, locale_t UNUSED(locale)) { return libc_isctype(ch,mask); }
INTERN int LIBCCALL libc_isleadbyte_l(int wc, locale_t UNUSED(locale)) { return libc_isleadbyte(wc); }
INTERN int LIBCCALL libc_toupper_l(int ch, locale_t UNUSED(locale)) { return libc_toupper(ch); }
INTERN int LIBCCALL libc_tolower_l(int ch, locale_t UNUSED(locale)) { return libc_tolower(ch); }
INTERN int LIBCCALL libc_iswalnum_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswalnum(wc); }
INTERN int LIBCCALL libc_iswalpha_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswalpha(wc); }
INTERN int LIBCCALL libc_iswcntrl_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswcntrl(wc); }
INTERN int LIBCCALL libc_iswdigit_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswdigit(wc); }
INTERN int LIBCCALL libc_iswgraph_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswgraph(wc); }
INTERN int LIBCCALL libc_iswlower_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswlower(wc); }
INTERN int LIBCCALL libc_iswprint_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswprint(wc); }
INTERN int LIBCCALL libc_iswpunct_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswpunct(wc); }
INTERN int LIBCCALL libc_iswspace_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswspace(wc); }
INTERN int LIBCCALL libc_iswupper_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswupper(wc); }
INTERN int LIBCCALL libc_iswxdigit_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswxdigit(wc); }
INTERN int LIBCCALL libc_iswblank_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswblank(wc); }
INTERN int LIBCCALL libc_iswcsym_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswcsym(wc); }
INTERN int LIBCCALL libc_iswcsymf_l(wint_t wc, locale_t UNUSED(locale)) { return libc_iswcsymf(wc); }
INTERN int LIBCCALL libc_iswctype_l(wint_t wc, wctype_t desc, locale_t UNUSED(locale)) { return libc_iswctype(wc,desc); }
INTERN wint_t LIBCCALL libc_towlower_l(wint_t wc, locale_t UNUSED(locale)) { return libc_towlower(wc); }
INTERN wint_t LIBCCALL libc_towupper_l(wint_t wc, locale_t UNUSED(locale)) { return libc_towupper(wc); }
INTERN wctype_t LIBCCALL libc_wctype_l(char const *prop, locale_t UNUSED(locale)) { return libc_wctype(prop); }
INTERN wctrans_t LIBCCALL libc_wctrans_l(char const *prop, locale_t UNUSED(locale)) { return libc_wctrans(prop); }
INTERN wint_t LIBCCALL libc_towctrans_l(wint_t wc, wctrans_t desc, locale_t UNUSED(locale)) { return libc_towctrans(wc,desc); }
#endif


EXPORT(_tolower,libc__tolower);
EXPORT(_toupper,libc__toupper);
EXPORT(toascii,libc_toascii);
EXPORT(isleadbyte,libc_isleadbyte);
EXPORT(iswalnum,libc_iswalnum);
EXPORT(iswalpha,libc_iswalpha);
EXPORT(iswcntrl,libc_iswcntrl);
EXPORT(iswdigit,libc_iswdigit);
EXPORT(iswgraph,libc_iswgraph);
EXPORT(iswlower,libc_iswlower);
EXPORT(iswprint,libc_iswprint);
EXPORT(iswpunct,libc_iswpunct);
EXPORT(iswspace,libc_iswspace);
EXPORT(iswupper,libc_iswupper);
EXPORT(iswxdigit,libc_iswxdigit);
EXPORT(iswblank,libc_iswblank);
EXPORT(iswascii,libc_iswascii);
EXPORT(iswctype,libc_iswctype);
EXPORT(iswcsym,libc_iswcsym);
EXPORT(iswcsymf,libc_iswcsymf);
EXPORT(towlower,libc_towlower);
EXPORT(towupper,libc_towupper);
EXPORT(wctype,libc_wctype);
EXPORT(wctrans,libc_wctrans);
EXPORT(towctrans,libc_towctrans);

EXPORT(isalpha_l,libc_isalpha_l);
EXPORT(isupper_l,libc_isupper_l);
EXPORT(islower_l,libc_islower_l);
EXPORT(isdigit_l,libc_isdigit_l);
EXPORT(isxdigit_l,libc_isxdigit_l);
EXPORT(isspace_l,libc_isspace_l);
EXPORT(ispunct_l,libc_ispunct_l);
EXPORT(isalnum_l,libc_isalnum_l);
EXPORT(isprint_l,libc_isprint_l);
EXPORT(isgraph_l,libc_isgraph_l);
EXPORT(iscntrl_l,libc_iscntrl_l);
EXPORT(isblank_l,libc_isblank_l);
EXPORT(isctype_l,libc_isctype_l);
EXPORT(isleadbyte_l,libc_isleadbyte_l);
EXPORT(toupper_l,libc_toupper_l);
EXPORT(tolower_l,libc_tolower_l);

EXPORT(iswalnum_l,libc_iswalnum_l);
EXPORT(iswalpha_l,libc_iswalpha_l);
EXPORT(iswcntrl_l,libc_iswcntrl_l);
EXPORT(iswdigit_l,libc_iswdigit_l);
EXPORT(iswgraph_l,libc_iswgraph_l);
EXPORT(iswlower_l,libc_iswlower_l);
EXPORT(iswprint_l,libc_iswprint_l);
EXPORT(iswpunct_l,libc_iswpunct_l);
EXPORT(iswspace_l,libc_iswspace_l);
EXPORT(iswupper_l,libc_iswupper_l);
EXPORT(iswxdigit_l,libc_iswxdigit_l);
EXPORT(iswblank_l,libc_iswblank_l);
EXPORT(iswctype_l,libc_iswctype_l);
EXPORT(iswcsym_l,libc_iswcsym_l);
EXPORT(iswcsymf_l,libc_iswcsymf_l);
EXPORT(towlower_l,libc_towlower_l);
EXPORT(towupper_l,libc_towupper_l);
EXPORT(wctype_l,libc_wctype_l);
EXPORT(wctrans_l,libc_wctrans_l);
EXPORT(towctrans_l,libc_towctrans_l);


/* DOS-specific functions. */
EXPORT(_isctype,libc_dos_isctype);
EXPORT(__iscsym,libc_iscsym);
EXPORT(__iscsymf,libc_iscsymf);
EXPORT(__iswcsym,libc_iswcsym);
EXPORT(__iswcsymf,libc_iswcsymf);

/* DOS name aliases. */
EXPORT(_isalpha_l,libc_isalpha_l);
EXPORT(_isupper_l,libc_isupper_l);
EXPORT(_islower_l,libc_islower_l);
EXPORT(_isdigit_l,libc_isdigit_l);
EXPORT(_isxdigit_l,libc_isxdigit_l);
EXPORT(_isspace_l,libc_isspace_l);
EXPORT(_ispunct_l,libc_ispunct_l);
EXPORT(_isalnum_l,libc_isalnum_l);
EXPORT(_isprint_l,libc_isprint_l);
EXPORT(_isgraph_l,libc_isgraph_l);
EXPORT(_iscntrl_l,libc_iscntrl_l);
EXPORT(_isblank_l,libc_isblank_l);
EXPORT(_isctype_l,libc_isctype_l);
EXPORT(_isleadbyte_l,libc_isleadbyte_l);
EXPORT(_toupper_l,libc_toupper_l);
EXPORT(_tolower_l,libc_tolower_l);
EXPORT(__toascii,libc_toascii);

EXPORT(_iswalnum_l,libc_iswalnum_l);
EXPORT(_iswalpha_l,libc_iswalpha_l);
EXPORT(_iswblank_l,libc_iswblank_l);
EXPORT(_iswcntrl_l,libc_iswcntrl_l);
EXPORT(_iswcsym_l,libc_iswcsym_l);
EXPORT(_iswcsymf_l,libc_iswcsymf_l);
EXPORT(_iswctype_l,libc_iswctype_l);
EXPORT(_iswdigit_l,libc_iswdigit_l);
EXPORT(_iswgraph_l,libc_iswgraph_l);
EXPORT(_iswlower_l,libc_iswlower_l);
EXPORT(_iswprint_l,libc_iswprint_l);
EXPORT(_iswpunct_l,libc_iswpunct_l);
EXPORT(_iswspace_l,libc_iswspace_l);
EXPORT(_iswupper_l,libc_iswupper_l);
EXPORT(_iswxdigit_l,libc_iswxdigit_l);
EXPORT(_towlower_l,libc_towlower_l);
EXPORT(_towupper_l,libc_towupper_l);



/*
    _ismbbalnum
    _ismbbalnum_l
    _ismbbalpha
    _ismbbalpha_l
    _ismbbblank
    _ismbbblank_l
    _ismbbgraph
    _ismbbgraph_l
    _ismbbkalnum
    _ismbbkalnum_l
    _ismbbkana
    _ismbbkana_l
    _ismbbkprint
    _ismbbkprint_l
    _ismbbkpunct
    _ismbbkpunct_l
    _ismbblead
    _ismbblead_l
    _ismbbprint
    _ismbbprint_l
    _ismbbpunct
    _ismbbpunct_l
    _ismbbtrail
    _ismbbtrail_l
    _ismbcalnum
    _ismbcalnum_l
    _ismbcalpha
    _ismbcalpha_l
    _ismbcblank
    _ismbcblank_l
    _ismbcdigit
    _ismbcdigit_l
    _ismbcgraph
    _ismbcgraph_l
    _ismbchira
    _ismbchira_l
    _ismbckata
    _ismbckata_l
    _ismbcl0
    _ismbcl0_l
    _ismbcl1
    _ismbcl1_l
    _ismbcl2
    _ismbcl2_l
    _ismbclegal
    _ismbclegal_l
    _ismbclower
    _ismbclower_l
    _ismbcprint
    _ismbcprint_l
    _ismbcpunct
    _ismbcpunct_l
    _ismbcspace
    _ismbcspace_l
    _ismbcsymbol
    _ismbcsymbol_l
    _ismbcupper
    _ismbcupper_l
    _ismbslead
    _ismbslead_l
    _ismbstrail
    _ismbstrail_l
*/


/* These are also exported by GLibc */
EXPORT_STRONG(__isctype,libc_isctype);
EXPORT_STRONG(__isalnum_l,libc_isalnum_l);
EXPORT_STRONG(__isalpha_l,libc_isalpha_l);
EXPORT_STRONG(__isascii_l,libc_isascii_l);
EXPORT_STRONG(__isblank_l,libc_isblank_l);
EXPORT_STRONG(__iscntrl_l,libc_iscntrl_l);
EXPORT_STRONG(__isdigit_l,libc_isdigit_l);
EXPORT_STRONG(__isgraph_l,libc_isgraph_l);
EXPORT_STRONG(__islower_l,libc_islower_l);
EXPORT_STRONG(__isprint_l,libc_isprint_l);
EXPORT_STRONG(__ispunct_l,libc_ispunct_l);
EXPORT_STRONG(__isspace_l,libc_isspace_l);
EXPORT_STRONG(__isupper_l,libc_isupper_l);
EXPORT_STRONG(__isxdigit_l,libc_isxdigit_l);
EXPORT_STRONG(__iswctype,libc_iswctype);
EXPORT_STRONG(__iswctype_l,libc_iswctype_l);
EXPORT_STRONG(__iswalnum_l,libc_iswalnum_l);
EXPORT_STRONG(__iswalpha_l,libc_iswalpha_l);
EXPORT_STRONG(__iswblank_l,libc_iswblank_l);
EXPORT_STRONG(__iswcntrl_l,libc_iswcntrl_l);
EXPORT_STRONG(__iswdigit_l,libc_iswdigit_l);
EXPORT_STRONG(__iswgraph_l,libc_iswgraph_l);
EXPORT_STRONG(__iswlower_l,libc_iswlower_l);
EXPORT_STRONG(__iswprint_l,libc_iswprint_l);
EXPORT_STRONG(__iswpunct_l,libc_iswpunct_l);
EXPORT_STRONG(__iswspace_l,libc_iswspace_l);
EXPORT_STRONG(__iswupper_l,libc_iswupper_l);
EXPORT_STRONG(__iswxdigit_l,libc_iswxdigit_l);
EXPORT_STRONG(__toascii_l,libc_toascii_l);
EXPORT_STRONG(__tolower_l,libc_tolower_l);
EXPORT_STRONG(__toupper_l,libc_toupper_l);
EXPORT_STRONG(__towctrans,libc_towctrans);
EXPORT_STRONG(__towctrans_l,libc_towctrans_l);
EXPORT_STRONG(__towlower_l,libc_towlower_l);
EXPORT_STRONG(__towupper_l,libc_towupper_l);
EXPORT_STRONG(__wctrans_l,libc_wctrans_l);
EXPORT_STRONG(__wctype_l,libc_wctype_l);
EXPORT_STRONG(__ctype_tolower,libc_tolower);
EXPORT_STRONG(__ctype_toupper,libc_toupper);


#endif /* !CONFIG_LIBC_LIMITED_API */


DECL_END

#endif /* !GUARD_HYBRID_CTYPE_C */
