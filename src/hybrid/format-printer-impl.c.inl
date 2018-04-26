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
#define _UTF_SOURCE 1
#define _KOS_SOURCE 1
#include "format-printer.c"
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR
//#define FORMAT_OPTION_LOCALE     1 /* Enable locale support. */
//#define FORMAT_OPTION_WCHAR16    1 /* `%ls' uses 16-bit wide characters. */
//#define FORMAT_OPTION_POSITIONAL 1 /* Enable support for positional arguments. */
#endif

#include <format-printer.h>
#include <except.h>
#include <stdbool.h>
#include <unicode.h>
#include <kos/kdev_t.h>
#ifndef __KERNEL__
#include <alloca.h>
#include <stdlib.h>
#include <unistd.h>
#include "../libs/libc/libc.h"
#include "../libs/libc/err.h"
#include "../libs/libc/format.h"
#include "../libs/libc/unicode.h"
#include "../libs/libc/ushare.h"
#include "../libs/libc/dl.h"
#endif

#ifndef FORMAT_OPTION_CHARTYPE
#error "Must #define FORMAT_OPTION_CHARTYPE"
#endif

#ifndef format_T_char
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#   define format_T_char       char
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define format_T_char       char16_t
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR32
#   define format_T_char       char32_t
#else
#   error "Invalid character type"
#endif
#endif /* !format_T_char */

#ifndef sizeof_T_char
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#   define sizeof_T_char       __SIZEOF_CHAR__
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define sizeof_T_char       2
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR32
#   define sizeof_T_char       4
#else
#   error "Invalid character type"
#endif
#endif /* !sizeof_T_char */

#ifndef LIBC_FORMAT_QUOTE
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#   define LIBC_FORMAT_QUOTE   libc_format_quote
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define LIBC_FORMAT_QUOTE   libc_format_w16quote
#else
#   define LIBC_FORMAT_QUOTE   libc_format_w32quote
#endif
#endif /* !LIBC_FORMAT_QUOTE */

#ifndef LIBC_FORMAT_WIDTH
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_WIDTH libc_format_width
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_WIDTH libc_format_w16width
#   else
#      define LIBC_FORMAT_WIDTH libc_format_w32width
#   endif
#endif /* !LIBC_FORMAT_WIDTH */

#ifndef LIBC_FORMAT_REPEAT
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_REPEAT libc_format_repeat
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_REPEAT libc_format_w16repeat
#   else
#      define LIBC_FORMAT_REPEAT libc_format_w32repeat
#   endif
#endif /* !LIBC_FORMAT_REPEAT */

#ifndef PFORMATPRINTER
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#   define PFORMATPRINTER      pformatprinter
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define PFORMATPRINTER      pw16formatprinter
#else
#   define PFORMATPRINTER      pw32formatprinter
#endif
#endif /* !PFORMATPRINTER */

#ifndef LIBC_FORMAT_VPRINTF
#ifdef FORMAT_OPTION_LOCALE
#   ifdef FORMAT_OPTION_WCHAR16
#      ifdef FORMAT_OPTION_POSITIONAL
#         if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#            define LIBC_FORMAT_PRINTF    libd_format_printf_p_l
#            define LIBC_FORMAT_VPRINTF   libd_format_vprintf_p_l
#         elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#            define LIBC_FORMAT_PRINTF    libd_format_w16printf_p_l
#            define LIBC_FORMAT_VPRINTF   libd_format_vw16printf_p_l
#         else
#            define LIBC_FORMAT_PRINTF    libd_format_w32printf_p_l
#            define LIBC_FORMAT_VPRINTF   libd_format_vw32printf_p_l
#         endif
#      else /* FORMAT_OPTION_POSITIONAL */
#         if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#            define LIBC_FORMAT_PRINTF    libd_format_printf_l
#            define LIBC_FORMAT_VPRINTF   libd_format_vprintf_l
#         elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#            define LIBC_FORMAT_PRINTF    libd_format_w16printf_l
#            define LIBC_FORMAT_VPRINTF   libd_format_vw16printf_l
#         else
#            define LIBC_FORMAT_PRINTF    libd_format_w32printf_l
#            define LIBC_FORMAT_VPRINTF   libd_format_vw32printf_l
#         endif
#      endif /* !FORMAT_OPTION_POSITIONAL */
#   else /* FORMAT_OPTION_WCHAR16 */
#      ifdef FORMAT_OPTION_POSITIONAL
#         if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#            define LIBC_FORMAT_PRINTF    libc_format_printf_p_l
#            define LIBC_FORMAT_VPRINTF   libc_format_vprintf_p_l
#         elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#            define LIBC_FORMAT_PRINTF    libc_format_w16printf_p_l
#            define LIBC_FORMAT_VPRINTF   libc_format_vw16printf_p_l
#         else
#            define LIBC_FORMAT_PRINTF    libc_format_w32printf_p_l
#            define LIBC_FORMAT_VPRINTF   libc_format_vw32printf_p_l
#         endif
#      else /* FORMAT_OPTION_POSITIONAL */
#         if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#            define LIBC_FORMAT_PRINTF    libc_format_printf_l
#            define LIBC_FORMAT_VPRINTF   libc_format_vprintf_l
#         elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#            define LIBC_FORMAT_PRINTF    libc_format_w16printf_l
#            define LIBC_FORMAT_VPRINTF   libc_format_vw16printf_l
#         else
#            define LIBC_FORMAT_PRINTF    libc_format_w32printf_l
#            define LIBC_FORMAT_VPRINTF   libc_format_vw32printf_l
#         endif
#      endif /* !FORMAT_OPTION_POSITIONAL */
#   endif /* !FORMAT_OPTION_WCHAR16 */
#else /* FORMAT_OPTION_LOCALE */
#   ifdef FORMAT_OPTION_WCHAR16
#      ifdef FORMAT_OPTION_POSITIONAL
#         if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#            define LIBC_FORMAT_PRINTF    libd_format_printf_p
#            define LIBC_FORMAT_VPRINTF   libd_format_vprintf_p
#         elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#            define LIBC_FORMAT_PRINTF    libd_format_w16printf_p
#            define LIBC_FORMAT_VPRINTF   libd_format_vw16printf_p
#         else
#            define LIBC_FORMAT_PRINTF    libd_format_w32printf_p
#            define LIBC_FORMAT_VPRINTF   libd_format_vw32printf_p
#         endif
#      else /* FORMAT_OPTION_POSITIONAL */
#         if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#            define LIBC_FORMAT_PRINTF    libd_format_printf
#            define LIBC_FORMAT_VPRINTF   libd_format_vprintf
#         elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#            define LIBC_FORMAT_PRINTF    libd_format_w16printf
#            define LIBC_FORMAT_VPRINTF   libd_format_vw16printf
#         else
#            define LIBC_FORMAT_PRINTF    libd_format_w32printf
#            define LIBC_FORMAT_VPRINTF   libd_format_vw32printf
#         endif
#      endif /* !FORMAT_OPTION_POSITIONAL */
#   else /* FORMAT_OPTION_WCHAR16 */
#      ifdef FORMAT_OPTION_POSITIONAL
#         if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#            define LIBC_FORMAT_PRINTF    libc_format_printf_p
#            define LIBC_FORMAT_VPRINTF   libc_format_vprintf_p
#         elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#            define LIBC_FORMAT_PRINTF    libc_format_w16printf_p
#            define LIBC_FORMAT_VPRINTF   libc_format_vw16printf_p
#         else
#            define LIBC_FORMAT_PRINTF    libc_format_w32printf_p
#            define LIBC_FORMAT_VPRINTF   libc_format_vw32printf_p
#         endif
#      else /* FORMAT_OPTION_POSITIONAL */
#         if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#            define LIBC_FORMAT_PRINTF    libc_format_printf
#            define LIBC_FORMAT_VPRINTF   libc_format_vprintf
#         elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#            define LIBC_FORMAT_PRINTF    libc_format_w16printf
#            define LIBC_FORMAT_VPRINTF   libc_format_vw16printf
#         else
#            define LIBC_FORMAT_PRINTF    libc_format_w32printf
#            define LIBC_FORMAT_VPRINTF   libc_format_vw32printf
#         endif
#      endif /* !FORMAT_OPTION_POSITIONAL */
#   endif /* !FORMAT_OPTION_WCHAR16 */
#endif /* !FORMAT_OPTION_LOCALE */
#endif /* !LIBC_FORMAT_VPRINTF */

#ifndef LIBC_FORMAT_HEXDUMP
#ifdef FORMAT_OPTION_LOCALE
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_HEXDUMP   libc_format_hexdump_l
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_HEXDUMP   libc_format_w16hexdump_l
#   else
#      define LIBC_FORMAT_HEXDUMP   libc_format_w32hexdump_l
#   endif
#else /* FORMAT_OPTION_LOCALE */
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_HEXDUMP   libc_format_hexdump
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_HEXDUMP   libc_format_w16hexdump
#   else
#      define LIBC_FORMAT_HEXDUMP   libc_format_w32hexdump
#   endif
#endif /* !FORMAT_OPTION_LOCALE */
#endif /* !LIBC_FORMAT_HEXDUMP */

#if FORMAT_OPTION_CHARTYPE != CHARACTER_TYPE_CHAR
#ifndef STRUCT_PRINTER
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define STRUCT_PRINTER  struct w16printer
#else
#   define STRUCT_PRINTER  struct w32printer
#endif
#endif /* !STRUCT_PRINTER */
#ifndef STRUCT_PRINTER_INIT
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define STRUCT_PRINTER_INIT  libc_w16printer_init
#else
#   define STRUCT_PRINTER_INIT  libc_w32printer_init
#endif
#endif /* !STRUCT_PRINTER_INIT */
#ifndef STRUCT_PRINTER_FINI
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define STRUCT_PRINTER_FINI  libc_w16printer_fini
#else
#   define STRUCT_PRINTER_FINI  libc_w32printer_fini
#endif
#endif /* !STRUCT_PRINTER_FINI */
#ifndef STRUCT_PRINTER_PRINT
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define STRUCT_PRINTER_PRINT libc_w16printer_print
#else
#   define STRUCT_PRINTER_PRINT libc_w32printer_print
#endif
#endif /* !STRUCT_PRINTER_PRINT */
#endif /* FORMAT_OPTION_CHARTYPE != CHARACTER_TYPE_CHAR */


DECL_BEGIN

#ifndef FORMAT_PRINTER_CONFIGURATION_DEFINED
#define FORMAT_PRINTER_CONFIGURATION_DEFINED 1
#define VA_SIZE  __SIZEOF_INT__

#if VA_SIZE == 8
#   define LENGTH_I64  0
#   define LENGTH_I32  0
#   define LENGTH_I16  0
#   define LENGTH_I8   0
#elif VA_SIZE == 4
#   define LENGTH_I64  1
#   define LENGTH_I32  0
#   define LENGTH_I16  0
#   define LENGTH_I8   0
#elif VA_SIZE == 2
#   define LENGTH_I64  2
#   define LENGTH_I32  1
#   define LENGTH_I16  0
#   define LENGTH_I8   0
#elif VA_SIZE == 1
#   define LENGTH_I64  3
#   define LENGTH_I32  2
#   define LENGTH_I16  1
#   define LENGTH_I8   0
#else
#   error "Error: Unsupported `VA_SIZE'"
#endif

#define LENGTH_L    'L'
#define LENGTH_Z    'z'
#define LENGTH_T    't'
#define LENGTH_J    LENGTH_I64 /* intmax_t */
#define LENGTH_SIZE PP_CAT2(LENGTH_I,PP_MUL8(__SIZEOF_POINTER__))
#define LENGTH_HH   PP_CAT2(LENGTH_I,PP_MUL8(__SIZEOF_CHAR__))
#define LENGTH_H    PP_CAT2(LENGTH_I,PP_MUL8(__SIZEOF_SHORT__))
#define LENGTH_l    PP_CAT2(LENGTH_I,PP_MUL8(__SIZEOF_LONG__))
#ifdef __SIZEOF_LONG_LONG__
#define LENGTH_LL   PP_CAT2(LENGTH_I,PP_MUL8(__SIZEOF_LONG_LONG__))
#else
#define LENGTH_LL   LENGTH_I64
#endif

#define F_NONE     0x0000
#define F_UPPER    0x0001 /* Print upper-case hex-characters. */
#define F_LJUST    0x0002 /* '%-'. */
#define F_SIGN     0x0004 /* '%+'. */
#define F_SPACE    0x0008 /* '% '. */
#define F_PADZERO  0x0010 /* '%0'. */
#define F_HASWIDTH 0x0020 /* '%123'. */
#define F_HASPREC  0x0040 /* `%.123'. */
#define F_PREFIX   0x0080 /* `%#'. */
#define F_SIGNED   0x0100
#define F_FIXBUF   0x0200
#endif /* !FORMAT_PRINTER_CONFIGURATION_DEFINED */
 
#define DO_PRINT(p,s) \
do{ if unlikely((temp = (*printer)(p,s,closure)) < 0) goto err; \
    result += temp; \
}__WHILE0
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#define DO_PRINT_ASCII(p,s) \
do{ if unlikely((temp = (*printer)(p,s,closure)) < 0) goto err; \
    result += temp; \
}__WHILE0
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#define DO_PRINT_ASCII(p,s) \
do{ if unlikely((temp = impl_do_print_ascii16(printer,closure,p,s)) < 0) goto err; \
    result += temp; \
}__WHILE0
#ifndef IMPL_DO_PRINT_ASCII16_DEFINED
#define IMPL_DO_PRINT_ASCII16_DEFINED 1
PRIVATE ssize_t LIBCCALL
impl_do_print_ascii16(PFORMATPRINTER printer, void *closure,
                      char const *__restrict p, size_t s) {
 ssize_t COMPILER_IGNORE_UNINITIALIZED(result);
 STRUCT_PRINTER fwd = W16PRINTER_INIT(printer,closure);
 TRY {
  result = STRUCT_PRINTER_PRINT(p,s,&fwd);
 } FINALLY {
  STRUCT_PRINTER_FINI(&fwd);
 }
 return result;
}
#endif /* !IMPL_DO_PRINT_ASCII16_DEFINED */
#else
#define DO_PRINT_ASCII(p,s) \
do{ if unlikely((temp = impl_do_print_ascii32(printer,closure,p,s)) < 0) goto err; \
    result += temp; \
}__WHILE0
#ifndef IMPL_DO_PRINT_ASCII32_DEFINED
#define IMPL_DO_PRINT_ASCII32_DEFINED 1
PRIVATE ssize_t LIBCCALL
impl_do_print_ascii32(PFORMATPRINTER printer, void *closure,
                      char const *__restrict p, size_t s) {
 ssize_t COMPILER_IGNORE_UNINITIALIZED(result);
 STRUCT_PRINTER fwd = W32PRINTER_INIT(printer,closure);
 TRY {
  result = STRUCT_PRINTER_PRINT(p,s,&fwd);
 } FINALLY {
  STRUCT_PRINTER_FINI(&fwd);
 }
 return result;
}
#endif /* !IMPL_DO_PRINT_ASCII32_DEFINED */
#endif
#ifdef FORMAT_OPTION_LOCALE
#define DO_PRINTF(format,...) \
do{ if unlikely((temp = LIBC_FORMAT_PRINTF(printer,closure,format,locale,##__VA_ARGS__)) < 0) goto err; \
    result += temp; \
}__WHILE0
#else
#define DO_PRINTF(...) \
do{ if unlikely((temp = LIBC_FORMAT_PRINTF(printer,closure,__VA_ARGS__)) < 0) goto err; \
    result += temp; \
}__WHILE0
#endif


#ifndef VA_CONT_DEFINED
#define VA_CONT_DEFINED 1
struct va_cont { va_list args; };
#endif /* !VA_CONT_DEFINED */


#define UNIQUE(x) PP_CAT2(LIBC_FORMAT_PRINTF,x)

#ifndef FORMAT_OPTION_POSITIONAL
#ifdef __KERNEL__
#define KERNEL_XFORMAT(x) x
#define USER_XFORMAT(x)   /* nothing */
#else
#define KERNEL_XFORMAT(x) /* nothing */
#define USER_XFORMAT(x)   x
#endif

#define FOREACH_XFORMAT(callback) \
                   callback(UNIQUE(xformat_hex_name),UNIQUE(xformat_hex)) \
                   callback(UNIQUE(xformat_dev_name),UNIQUE(xformat_dev)) \
                   callback(UNIQUE(xformat_vinfo_name),UNIQUE(xformat_vinfo)) \
    KERNEL_XFORMAT(callback(UNIQUE(xformat_path_name),UNIQUE(xformat_path))) \
      USER_XFORMAT(callback(UNIQUE(xformat_errno_name),UNIQUE(xformat_errno))) \

PRIVATE format_T_char const UNIQUE(xformat_hex_name)[] = { 'h', 'e', 'x' };
PRIVATE ssize_t LIBCCALL
UNIQUE(xformat_hex)(PFORMATPRINTER printer, void *closure,
                    format_T_char const *arg, unsigned int length,
                    unsigned int flags, size_t precision,
#ifdef FORMAT_OPTION_LOCALE
                    locale_t locale,
#endif
                    struct va_cont *args) {
 void *p = va_arg(args->args,void *);
 (void)length;
 if (!(flags & F_FIXBUF))
       precision = libc_strnlen((char *)p,precision);
 /* TODO: Parse `arg' */
#ifdef FORMAT_OPTION_LOCALE
 return LIBC_FORMAT_HEXDUMP(printer,closure,p,precision,0,
                            FORMAT_HEXDUMP_FLAG_ADDRESS,locale);
#else
 return LIBC_FORMAT_HEXDUMP(printer,closure,p,precision,0,
                            FORMAT_HEXDUMP_FLAG_ADDRESS);
#endif
}

PRIVATE format_T_char const UNIQUE(xformat_dev_name)[] = { 'd', 'e', 'v' };
PRIVATE format_T_char const UNIQUE(dev_format)[] = {
    '%','.','2','x',':','%','.','2','x',0
};
PRIVATE ssize_t LIBCCALL
UNIQUE(xformat_dev)(PFORMATPRINTER printer, void *closure,
                    format_T_char const *UNUSED(arg), unsigned int UNUSED(length),
                    unsigned int UNUSED(flags), size_t UNUSED(precision),
#ifdef FORMAT_OPTION_LOCALE
                    locale_t locale,
#endif
                    struct va_cont *args) {
 dev_t dev = va_arg(args->args,dev_t);
 return LIBC_FORMAT_PRINTF(printer,
                           closure,
                           UNIQUE(dev_format),
#ifdef FORMAT_OPTION_LOCALE
                           locale,
#endif
                           MAJOR(dev),
                           MINOR(dev));
}

#ifdef __KERNEL__
#include <fs/linker.h>
#include <fs/driver.h>
#include <kernel/vm.h>
#include <unwind/debug_line.h>
#endif /* __KERNEL__ */

#ifndef VINFO_USE_MAGIC_DEFINED
#define VINFO_USE_MAGIC_DEFINED 1

/* Recognized values for `CONFIG_VINFO_USE_MAGIC':
 * <  0: Don't use magic
 * == 0: Automatically determine at runtime
 * >  0: Do use magic
 */
#ifndef CONFIG_VINFO_USE_MAGIC
#ifdef NDEBUG
#define CONFIG_VINFO_USE_MAGIC  (-1)
#endif
#elif (CONFIG_VINFO_USE_MAGIC+0) == 0
#undef CONFIG_VINFO_USE_MAGIC
#endif

#ifdef CONFIG_VINFO_USE_MAGIC
#if (CONFIG_VINFO_USE_MAGIC+0) > 0
#define vinfo_use_magic()   0
#else
#define vinfo_use_magic()   1
#endif
#else
INTDEF bool LIBCCALL vinfo_use_magic(void);
#endif
#endif


PRIVATE format_T_char const UNIQUE(xformat_vinfo_name)[] = { 'v', 'i', 'n', 'f', 'o' };
PRIVATE ssize_t LIBCCALL
UNIQUE(xformat_vinfo)(PFORMATPRINTER printer, void *closure,
                      format_T_char const *arg, unsigned int length,
                      unsigned int flags, size_t precision,
#ifdef FORMAT_OPTION_LOCALE
                      locale_t locale,
#endif
                      struct va_cont *args) {
 PRIVATE format_T_char const default_format[] = {
  '%','f','(','%','l',',','%','c',')',' ',':',' ','%','n',0
 };
#ifndef CONFIG_VINFO_USE_MAGIC
 if (vinfo_use_magic())
#endif
#if !defined(CONFIG_VINFO_USE_MAGIC) || (CONFIG_VINFO_USE_MAGIC+0) > 0
 {
#ifdef __KERNEL__
  PRIVATE format_T_char const magic_format[] = {
   '%','%','{','v','i','n','f','o',':','%','[','p','a','t',
   'h',']',':','%','p',':','%','p',':','%','$','s','}',0
  };
  uintptr_t addr = va_arg(args->args,uintptr_t);
  size_t arglen = 0; unsigned int recursion = 1;
  ssize_t COMPILER_IGNORE_UNINITIALIZED(result);
  struct vm *effective_vm = &vm_kernel;
  bool has_lock; struct vm_node *node;
  REF struct application *EXCEPT_VAR app = NULL;
  if (addr < KERNEL_BASE) effective_vm = THIS_VM;
  if (!arg) arg = default_format;
  for (; arg[arglen]; ++arglen) {
   /* */if (arg[arglen] == '[') ++recursion;
   else if (arg[arglen] == ']') { if (!--recursion) break; }
  }
  has_lock = vm_tryacquire(effective_vm);
  if (!has_lock) PREEMPTION_DISABLE();
  node = vm_getnode(VM_ADDR2PAGE(addr));
  if (node && node->vn_notify == &application_notify) {
   app = (REF struct application *)node->vn_closure;
   application_incref(app);
  }
  if (!has_lock) PREEMPTION_ENABLE();
  else vm_release(effective_vm);
  if (!app) {
   app = &kernel_driver.d_app;
   application_incref(app);
  }
  TRY {
   struct application *used_app = app;
   if (!app->a_module->m_path)
        used_app = &kernel_driver.d_app;
   result = LIBC_FORMAT_PRINTF(printer,closure,
                               magic_format,
#ifdef FORMAT_OPTION_LOCALE
                               locale,
#endif
                               used_app->a_module->m_path,
                               addr-app->a_loadaddr,addr,arglen,arg);
  } FINALLY {
   application_decref(app);
  }
  return result;
#else
  PRIVATE format_T_char const magic_format[] = {
   '%','%','{','v','i','n','f','o',':','%','s',
   ':','%','p',':','%','p',':','%','$','s','}',0
  };
  char pathbuf[80]; size_t arglen = 0;
  unsigned int recursion = 1;
  char *path = pathbuf; char *free_path = NULL;
  struct module_path_info query; ssize_t result;
  uintptr_t addr = va_arg(args->args,uintptr_t);
  if (!arg) arg = default_format;
  for (; arg[arglen]; ++arglen) {
   /* */if (arg[arglen] == '[') ++recursion;
   else if (arg[arglen] == ']') { if (!--recursion) break; }
  }
  query.pi_format  = REALPATH_FPATH;
  query.pi_path    = pathbuf;
  query.pi_pathlen = sizeof(pathbuf);
  if (libc_xdlmodule_info((void *)addr,MODULE_INFO_CLASS_PATH,&query,sizeof(query)) != sizeof(query)) {
missing_path:
   path = libc_program_invocation_name(); /* Better than nothing... */
   query.pi_loadaddr = 0;
  } else if (query.pi_pathlen > sizeof(pathbuf)) {
   /* Allocate a larger buffer for the module path. */
   __malloca_nofail(free_path,query.pi_pathlen);
   query.pi_path = path = free_path;
   if (libc_xdlmodule_info((void *)addr,MODULE_INFO_CLASS_PATH,&query,sizeof(query)) != sizeof(query))
       goto missing_path;
  }
  result = LIBC_FORMAT_PRINTF(printer,closure,magic_format,
#ifdef FORMAT_OPTION_LOCALE
                              locale,
#endif
                              path,
                              addr-query.pi_loadaddr,
                              addr,arglen,arg);
  if (free_path) freea(free_path);
  return result;
#endif
 }
#endif
#ifndef CONFIG_VINFO_USE_MAGIC
 else
#endif
#if !defined(CONFIG_VINFO_USE_MAGIC) || (CONFIG_VINFO_USE_MAGIC+0) < 0
 {
  PRIVATE format_T_char const slash[] = { '/', 0 };
  PRIVATE format_T_char const questionmasks[] = { '?', '?', '?', 0 };
  PRIVATE format_T_char const pointer_format[] = { '%', 'p', 0 };
  PRIVATE format_T_char const unsigned_format[] = { '%', 'u', 0 };
  ssize_t temp,result = 0; format_T_char ch;
  format_T_char const *flush_start; void *addr;
#ifdef __KERNEL__
  uintptr_t load_addr;
  struct dl_addr2line info;
#else
  struct {
     struct dl_addr2line _info;
     char                _strings[64];
  }  _info_buffer;
#ifndef __INTELLISENSE__
#define info   _info_buffer._info
#else
  struct dl_addr2line info;
#endif
  struct dl_addr2line *dyninfo = NULL;
  ssize_t req_size;
#endif
  unsigned int recursion = 1;
  addr = va_arg(args->args,void *);
#ifdef __KERNEL__
  load_addr = linker_debug_query((uintptr_t)addr,&info);
  if (load_addr == (uintptr_t)-1)
      load_addr = 0,libc_memset(&info,0,sizeof(info));
#else
  req_size = libc_xdladdr2line((void *)addr,&_info_buffer._info,sizeof(_info_buffer));
  if ((size_t)req_size > sizeof(_info_buffer)) {
   __malloca_nofail(dyninfo,(size_t)req_size);
   req_size = libc_xdladdr2line((void *)addr,dyninfo,(size_t)req_size);
   libc_memcpy(&_info_buffer._info,dyninfo,sizeof(struct dl_addr2line));
  }
  if unlikely(req_size <= 0)
     libc_memset(&_info_buffer._info,0,sizeof(struct dl_addr2line));
#endif
  /* Format options:
   *    %%: Emit a '%' character.
   *    %f: Source file name. (Or `"???"')
   *    %l: Source line number (1-based). (Or `"0"')
   *    %c: Source column offset (1-based). (Or `"0"')
   *    %n: Nearest symbol name. (Or `"???"')
   *    %<: Next lower address. (Or 0)
   *    %>: Next greater address. (Or 0)
   *    %p: The requested address.
   * Example:
   * >>my_label:
   * >>    syslog(LOG_DEBUG,"%[vinfo:%f(%l,%c) : %n] : This is me\n",&&my_label);
   */
  /* Use a default representation if no argument was given. */
  if (!arg) arg = default_format;
  flush_start = arg;
next:
  ch = *arg++;
  /* */if unlikely(!ch) goto end;
  else if (ch == '[') ++recursion;
  else if (ch == ']') { if (!--recursion) goto end; }
  if (ch != '%') goto next;
  if (arg-1 != flush_start)
      DO_PRINT(flush_start,(size_t)((arg-1)-flush_start));
/*nextfmt:*/
  ch = *arg++;
  switch (ch) {

  case 'f':
   if (info.d_base) {
    size_t baselen = libc_strlen(info.d_base);
    DO_PRINT_ASCII(info.d_base,baselen);
    if (info.d_base[baselen-1] != '/' &&
       (info.d_path || info.d_file)) DO_PRINT(slash,1);
   }
   if (info.d_path) {
    size_t pathlen = libc_strlen(info.d_path);
    DO_PRINT_ASCII(info.d_path,pathlen);
    if (info.d_path[pathlen-1] != '/' &&
        info.d_file) DO_PRINT(slash,1);
   }
   if (info.d_file) {
    size_t filelen = libc_strlen(info.d_file);
    DO_PRINT_ASCII(info.d_file,filelen);
   }
   if (!info.d_base && !info.d_path && !info.d_file)
        DO_PRINT(questionmasks,COMPILER_STRLEN(questionmasks));
   break;

  {
   int integer;
  case 'l':
   integer = info.d_line;
 do_print_int:
   DO_PRINTF(unsigned_format,integer);
   break;
  case 'c':
   integer = info.d_column;
   goto do_print_int;
  }

  case 'n':
   if (!info.d_name) {
    DO_PRINT(questionmasks,COMPILER_STRLEN(questionmasks));
   } else {
    DO_PRINT_ASCII(info.d_name,libc_strlen(info.d_name));
   }
   break;

  {
   uintptr_t ptr;
  case '<':
   ptr = (uintptr_t)info.d_begin;
do_print_ptr:
   DO_PRINTF(pointer_format,ptr);
   break;
  case '>':
   ptr = (uintptr_t)info.d_end;
   goto do_print_ptr;
  case 'p':
   ptr = (uintptr_t)addr;
   goto do_print_ptr;
  }

  case '%':
   flush_start = arg-1;
   goto next;
  case '\0':
  case ']':
   goto end;
 
  default:
 /*broken_format:*/
   arg = flush_start;
   --flush_start;
   goto next;
  }
  flush_start = arg;
  goto next;
end:
  --arg;
  if (flush_start != arg) {
   temp = (*printer)(flush_start,(size_t)(arg-flush_start),closure);
   if unlikely(temp < 0) goto err;
   result += temp;
  }
#ifndef __KERNEL__
  if (dyninfo) freea(dyninfo);
#endif /* !__KERNEL__ */
  return result;
err:
#ifndef __KERNEL__
  if (dyninfo) freea(dyninfo);
#undef info
#endif /* !__KERNEL__ */
  return temp;
 }
#endif
}


#ifdef __KERNEL__
#include <fs/path.h>
#include <fs/node.h>

PRIVATE ssize_t LIBCCALL
print_path_r(pformatprinter printer, void *closure,
             struct path const *__restrict entry,
             struct path const *__restrict fs_root) {
 ssize_t result = 0,temp; assert(entry);
 if (entry == fs_root || !entry->p_parent) goto end;
 temp = print_path_r(printer,closure,
                            entry->p_parent,
                            fs_root);
 if unlikely(temp < 0) return temp;
 result += temp;
 temp = (*printer)("/",1,closure);
 if unlikely(temp < 0) return temp;
 result += temp;
 temp = (*printer)(entry->p_dirent->de_name,
                   entry->p_dirent->de_namelen,
                   closure);
 if unlikely(temp < 0) return temp;
 result += temp;
end:
 return result;
}
PRIVATE ssize_t LIBCCALL
print_path(pformatprinter printer, void *closure,
           struct path const *__restrict entry) {
 struct path const *root = THIS_FS->fs_root;
 if (entry == root) return (*printer)("/",1,closure);
 return print_path_r(printer,closure,entry,root);
}

PRIVATE format_T_char const UNIQUE(xformat_path_name)[] = { 'p', 'a', 't', 'h' };
PRIVATE ssize_t LIBCCALL
UNIQUE(xformat_path)(PFORMATPRINTER printer, void *closure,
                     format_T_char const *arg, unsigned int length,
                     unsigned int flags, size_t precision,
#ifdef FORMAT_OPTION_LOCALE
                     locale_t locale,
#endif
                     struct va_cont *args) {
 struct path *p = va_arg(args->args,struct path *);
 if (!p) return (*printer)("/",1,closure);
 return print_path(printer,closure,p);
}
#endif /* __KERNEL__ */


#ifndef __KERNEL__
PRIVATE format_T_char const UNIQUE(xformat_errno_name)[] = { 'e', 'r', 'r', 'n', 'o' };
PRIVATE ssize_t LIBCCALL
UNIQUE(xformat_errno)(PFORMATPRINTER printer, void *closure,
                      format_T_char const *arg, unsigned int length,
                      unsigned int flags, size_t precision,
#ifdef FORMAT_OPTION_LOCALE
                     locale_t locale,
#endif
                     struct va_cont *args) {
 PRIVATE format_T_char const str_unknown_error[] = {
  'U','n','k','n','o','w','n',' ','e','r','r','o','r',' ','%','d',0 };
 PRIVATE format_T_char const str_name_and_desc[] = {
  '%','s','(','%','s',')',0 };
 errno_t error = va_arg(args->args,errno_t);
 char const *msg = libc_strerror_s(error);
 (void)length,(void)precision,(void)flags;
#ifdef FORMAT_OPTION_LOCALE
 if (!msg) return LIBC_FORMAT_PRINTF(printer,closure,str_unknown_error,locale,error);
 return LIBC_FORMAT_PRINTF(printer,closure,str_name_and_desc,locale,libc_strerrorname_s(error),msg);
#else
 if (!msg) return LIBC_FORMAT_PRINTF(printer,closure,str_unknown_error,error);
 return LIBC_FORMAT_PRINTF(printer,closure,str_name_and_desc,libc_strerrorname_s(error),msg);
#endif
}
#endif /* !__KERNEL__ */



#endif /* !FORMAT_OPTION_POSITIONAL */





INTERN ssize_t LIBCCALL
LIBC_FORMAT_VPRINTF(PFORMATPRINTER printer, void *closure,
                    format_T_char const *__restrict format,
#ifdef FORMAT_OPTION_LOCALE
                    locale_t locale,
#endif
                    va_list args) {
 ssize_t result = 0,COMPILER_IGNORE_UNINITIALIZED(temp);
 format_T_char const *flush_start;
 format_T_char ch;
 size_t width,precision;
 unsigned int flags;
 unsigned int length;
#ifdef FORMAT_OPTION_LOCALE
 (void)locale;
 /* TODO */
#endif
#ifdef FORMAT_OPTION_POSITIONAL
 /* TODO */
#endif
 flush_start = format;
next:
 ch = *format++;
 if unlikely(!ch) goto end;
 if (ch != '%') goto next;
 if (format-1 != flush_start)
     DO_PRINT(flush_start,(size_t)((format-1)-flush_start));
 flush_start = format;
 
 /* Format option. */
 flags     = F_NONE;
 length    = 0;
 width     = 0;
 precision = 0;
nextfmt:
 ch = *format++;
 switch (ch) {

 case '%':
  flush_start = format-1;
  goto next;

 case '\0': goto end;
 case '-': flags |= F_LJUST;   goto nextfmt;
 case '+': flags |= F_SIGN;    goto nextfmt;
 case ' ': flags |= F_SPACE;   goto nextfmt;
 case '#': flags |= F_PREFIX;  goto nextfmt;
 case '0': flags |= F_PADZERO; goto nextfmt;

 case '?':
#if __SIZEOF_POINTER__ > VA_SIZE
  width = va_arg(args,size_t);
  goto nextfmt;
#endif
 case '*':
  width = (size_t)va_arg(args,unsigned int);
  goto nextfmt;

 case ':': /* Precision. */
  flags |= F_FIXBUF;
  ATTR_FALLTHROUGH;
 case '.': /* Precision. */
  ch = *format++;
#if __SIZEOF_POINTER__ > VA_SIZE
  if (ch == '*') {
   precision = (size_t)va_arg(args,unsigned int);
  } else if (ch == '?')
#else
  if (ch == '*' || ch == '?')
#endif
  { __IF0 { case '$': flags |= F_FIXBUF; }
    precision = va_arg(args,size_t);
  } else if (ch >= '0' && ch <= '9') {
   /* decimal-encoded precision modifier. */
   precision = (size_t)(ch - '0');
   while ((ch = *format,ch >= '0' && ch <= '9')) {
    precision = precision*10 + (size_t)(ch - '0');
    ++format;
   }
  } else {
   goto broken_format;
  }
  flags |= F_HASPREC;
  goto nextfmt;

 case 'h':
  if (*format != 'h') length = LENGTH_H;
  else { ++format; length = LENGTH_HH; }
  goto nextfmt;

 case 'l':
  if (*format != 'l') length = LENGTH_l;
  else { ++format; length = LENGTH_LL; }
  goto nextfmt;

 case 'z': case 't': case 'L':
  length = (unsigned int)ch;
  goto nextfmt;

 case 'I':
  ch = *format++;
  /* */if (ch == '8') length = LENGTH_I8;
  else if (ch == '1' && *format == '6') { ++format; length = LENGTH_I16; }
  else if (ch == '3' && *format == '2') { ++format; length = LENGTH_I32; }
  else if (ch == '6' && *format == '4') { ++format; case 'j': length = LENGTH_I64; }
  else { --format; length = LENGTH_SIZE; }
  goto nextfmt;

 { unsigned int numsys; char const *dec; bool is_neg;
   union { u64 u; s64 i; } data;
   size_t print_width,space_width;
   format_T_char *iter,buffer[67]; /* 64-bit binary, w/prefix + sign */
   __IF0 { case 'b': numsys = 2; }
   __IF0 { case 'o': numsys = 8; }
   __IF0 { case 'u': numsys = 10; if unlikely(length == LENGTH_T) flags |= F_SIGNED; }
   __IF0 { case 'd': case 'i': numsys = 10; if likely(length != LENGTH_Z) flags |= F_SIGNED; }
   __IF0 { case 'p': if (!(flags&F_HASPREC)) { precision = sizeof(void *)*2; flags |= F_HASPREC; }
#if __SIZEOF_POINTER__ > VA_SIZE
                     if (!length) length = LENGTH_SIZE;
#endif
           case 'X': flags |= F_UPPER;
           case 'x': numsys = 16; if unlikely(length == LENGTH_T) flags |= F_SIGNED; }
#if VA_SIZE < 2
if (length == LENGTH_I8) {
    data.u = (u64)va_arg(args,u8);
    if (flags&F_SIGNED) data.i = (s64)(s8)(u8)data.u;
   } else
#endif
#if VA_SIZE < 4
if (length == LENGTH_I16) {
    data.u = (u64)va_arg(args,u16);
    if (flags&F_SIGNED) data.i = (s64)(s16)(u16)data.u;
   } else
#endif
#if VA_SIZE < 8
   if (length == LENGTH_I32) {
    data.u = (u64)va_arg(args,u32);
    if (flags&F_SIGNED) data.i = (s64)(s32)(u32)data.u;
   } else
#endif
   { data.u = va_arg(args,u64); }

#if F_UPPER == 1
   dec = libc_decimals[flags&F_UPPER];
#else
   dec = libc_decimals[!!(flags&F_UPPER)];
#endif
   is_neg = false;
   if (flags&F_SIGNED && data.i < 0) {
    is_neg = true;
    data.i = -data.i;
   }
   iter = COMPILER_ENDOF(buffer);
   /* Actually translate the given input integer. */
   do *--iter = dec[data.u % numsys];
   while ((data.u /= numsys) != 0);
   if (flags&F_PREFIX && numsys != 10) {
    /* */if (numsys == 16) *--iter = dec[16]; /* X/x */
    else if (numsys == 2)  *--iter = dec[11]; /* B/b */
    *--iter = '0';
   }
   space_width = 0;
   print_width = (size_t)(COMPILER_ENDOF(buffer)-iter);
   if ((flags&F_HASPREC) && precision > print_width)
        print_width = precision;
   if (is_neg || (flags&F_SIGN)) ++print_width;
   if ((flags&F_HASWIDTH) && width > print_width) {
    space_width = width-print_width;
    if (!(flags & F_LJUST)) {
     temp = LIBC_FORMAT_REPEAT(printer,closure,' ',space_width);
     if unlikely(temp < 0) goto err;
     result += temp;
     space_width = 0;
    }
   }
   {
    format_T_char sign[] = { 0 };
    /* */if (is_neg) sign[0] = '-';
    else if (flags&F_SIGN) sign[0] = '+';
    if (sign[0]) {
     temp = (*printer)(sign,1,closure);
     if unlikely(temp < 0) goto err;
     result += temp;
    }
   }
   print_width = (size_t)(COMPILER_ENDOF(buffer)-iter);
   if ((flags&F_HASPREC) && precision > print_width) {
    temp = LIBC_FORMAT_REPEAT(printer,closure,'0',precision-print_width);
    if unlikely(temp < 0) goto err;
    result += temp;
   }
   temp = (*printer)(iter,print_width,closure);
   if unlikely(temp < 0) goto err;
   result += temp;
   if (space_width) {
    temp = LIBC_FORMAT_REPEAT(printer,closure,' ',space_width);
    if unlikely(temp < 0) goto err;
    result += temp;
   }
 } break;

 {
  char *string;
  size_t string_length;
  size_t string_width;
  char given_char[1];
 case 'c':
  given_char[0] = (char)va_arg(args,int);
  string        = given_char;
  string_length = 1;
  goto print_string;
 case 'q':
 case 's':
  string = va_arg(args,char *);
  if (!string) string = "(null)";
  if (!(flags&F_HASPREC)) precision = (size_t)-1;
  if (flags&F_FIXBUF) string_length = precision;
  else string_length = libc_strnlen(string,precision);
print_string:
  if (ch == 'q') {
   string_width = (size_t)libc_format_quote(&libc_format_width,NULL,string,string_length,
#if F_PREFIX == FORMAT_QUOTE_FLAG_PRINTRAW
                                            flags&F_PREFIX
#else
                                            flags&F_PREFIX
                                            ? FORMAT_QUOTE_FLAG_PRINTRAW
                                            : FORMAT_QUOTE_FLAG_NONE
#endif
                                            );
  } else {
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
   string_width = string_length;
#else
   if (!(flags & F_HASWIDTH)) {
    string_width = (size_t)-1;
#if 1 /* Optimization for `printf("%*s",(int)num_spaces,"");' */
   } else if (!string_length) {
    string_width = 0;
#endif
   } else {
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
    mbstate_t state = MBSTATE_INIT;
    string_width = libc_utf8to16(string,string_length,NULL,0,&state,
                                 UNICODE_F_NOZEROTERM|
                                 UNICODE_F_NOFAIL);
#else
    mbstate_t state = MBSTATE_INIT;
    string_width = libc_utf8to32(string,string_length,NULL,0,&state,
                                 UNICODE_F_NOZEROTERM|
                                 UNICODE_F_NOFAIL);
#endif
   }
#endif
  }
  if ((width > string_width) && !(flags&F_LJUST)) {
   temp = LIBC_FORMAT_REPEAT(printer,closure,' ',width-string_width);
   if unlikely(temp < 0) goto err;
   result += temp;
  }
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
  if (ch == 'q') {
   temp = libc_format_quote(printer,closure,string,string_length,
#if F_PREFIX == FORMAT_QUOTE_FLAG_PRINTRAW
                            flags&F_PREFIX
#else
                            flags&F_PREFIX
                            ? FORMAT_QUOTE_FLAG_PRINTRAW
                            : FORMAT_QUOTE_FLAG_NONE
#endif
                            );
  } else {
   temp = (*printer)(string,string_length,closure);
  }
#else
  {
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
   STRUCT_PRINTER fwd = W16PRINTER_INIT(printer,closure);
#else
   STRUCT_PRINTER fwd = W32PRINTER_INIT(printer,closure);
#endif
   TRY {
    if (ch == 'q') {
     temp = libc_format_quote(&STRUCT_PRINTER_PRINT,
                              &fwd,string,string_length,
#if F_PREFIX == FORMAT_QUOTE_FLAG_PRINTRAW
                              flags&F_PREFIX
#else
                              flags&F_PREFIX
                              ? FORMAT_QUOTE_FLAG_PRINTRAW
                              : FORMAT_QUOTE_FLAG_NONE
#endif
                              );
    } else {
     temp = STRUCT_PRINTER_PRINT(string,string_length,&fwd);
    }
   } FINALLY {
    STRUCT_PRINTER_FINI(&fwd);
   }
  }
#endif
  if unlikely(temp < 0) goto err;
  result += temp;
  if ((width > string_width) && (flags&F_LJUST)) {
   temp = LIBC_FORMAT_REPEAT(printer,closure,' ',width-string_width);
   if unlikely(temp < 0) goto err;
   result += temp;
  }
 } break;

#ifndef FORMAT_OPTION_POSITIONAL
 {
  format_T_char const *xformat_start;
  format_T_char const *xformat_arg;
  size_t xformat_size; unsigned int i;
 case '[':
  xformat_start = format;
  xformat_arg = NULL;
  i = 1;
  while ((ch = *format++) != 0) {
   /* */if (ch == '[') ++i;
   else if (ch == ']') { if (!--i) break; }
   else if (ch == ':' && i == 1 && !xformat_arg) xformat_arg = format;
  }
  if (xformat_arg)
   xformat_size = (size_t)((xformat_arg-1) - xformat_start);
  else {
   xformat_size = (size_t)(format - xformat_start);
   if (ch) --xformat_size;
  }
#if sizeof_T_char == 2
#define COMPARE_NAME(x) (COMPILER_LENOF(x) == xformat_size && !libc_memcmpw(x,xformat_start,COMPILER_LENOF(x)))
#elif sizeof_T_char == 4
#define COMPARE_NAME(x) (COMPILER_LENOF(x) == xformat_size && !libc_memcmpl(x,xformat_start,COMPILER_LENOF(x)))
#elif sizeof_T_char == 8
#define COMPARE_NAME(x) (COMPILER_LENOF(x) == xformat_size && !libc_memcmpq(x,xformat_start,COMPILER_LENOF(x)))
#else
#define COMPARE_NAME(x) (COMPILER_LENOF(x) == xformat_size && !libc_memcmp(x,xformat_start,COMPILER_LENOF(x)*sizeof(format_T_char)))
#endif
#ifdef FORMAT_OPTION_LOCALE
#ifdef __VA_LIST_IS_ARRAY
#define INVOKE_XFORMAT(func) \
 if ((temp = func(printer,closure,xformat_arg,length,flags, \
                  precision,locale,(struct va_cont *)args)) < 0) goto err; \
 result += temp;
#else
#define INVOKE_XFORMAT(func) \
 if ((temp = func(printer,closure,xformat_arg,length,flags, \
                  precision,locale,(struct va_cont *)&args)) < 0) goto err; \
 result += temp;
#endif
#else
#ifdef __VA_LIST_IS_ARRAY
#define INVOKE_XFORMAT(func) \
 if ((temp = func(printer,closure,xformat_arg,length, \
                  flags,precision,(struct va_cont *)args)) < 0) goto err; \
 result += temp;
#else
#define INVOKE_XFORMAT(func) \
 if ((temp = func(printer,closure,xformat_arg,length, \
                  flags,precision,(struct va_cont *)&args)) < 0) goto err; \
 result += temp;
#endif
#endif
#define ENUM_XFORMAT(name,func) \
  if (COMPARE_NAME(name)) { INVOKE_XFORMAT(func) break; }
  FOREACH_XFORMAT(ENUM_XFORMAT)
#undef ENUM_XFORMAT
#undef INVOKE_XFORMAT
#undef COMPARE_NAME
  goto broken_format;
 } break;
#endif /* !FORMAT_OPTION_POSITIONAL */
 
 default:
  if (ch >= '0' && ch <= '9') {
   /* decimal-encoded width modifier. */
   width = (size_t)(ch - '0');
   while ((ch = *format,ch >= '0' && ch <= '9')) {
    width = width*10 + (size_t)(ch - '0');
    ++format;
   }
   flags |= F_HASWIDTH;
   goto nextfmt;
  }
broken_format:
  format = flush_start;
  --flush_start;
  goto next;
 }
 flush_start = format;
 goto next;
end:
 --format;
 assert(!*format);
 if (flush_start != format) {
  temp = (*printer)(flush_start,(size_t)(format-flush_start),closure);
  if unlikely(temp < 0) goto err;
  result += temp;
 }
 return result;
err:
 return temp;
}

#undef DO_PRINTF
#undef DO_PRINT
#undef DO_PRINT_ASCII
#undef UNIQUE

#undef sizeof_T_char
#undef format_T_char
#undef LIBC_FORMAT_QUOTE
#undef PFORMATPRINTER
#undef LIBC_FORMAT_HEXDUMP
#undef LIBC_FORMAT_PRINTF
#undef LIBC_FORMAT_VPRINTF
#undef LIBC_FORMAT_REPEAT
#undef LIBC_FORMAT_WIDTH

#if FORMAT_OPTION_CHARTYPE != CHARACTER_TYPE_CHAR
#undef STRUCT_PRINTER
#undef STRUCT_PRINTER_INIT
#undef STRUCT_PRINTER_FINI
#undef STRUCT_PRINTER_PRINT
#endif

#undef FORMAT_OPTION_CHARTYPE
#undef FORMAT_OPTION_LOCALE
#undef FORMAT_OPTION_WCHAR16
#undef FORMAT_OPTION_POSITIONAL

DECL_END
