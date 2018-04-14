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
#ifndef GUARD_LIBS_LIBC_USHARE_C
#define GUARD_LIBS_LIBC_USHARE_C 1
#define __UTSNAME_EXPOSE_HOSTNAME 1

#include "libc.h"
#include "format.h"
#include "ushare.h"
#include "errno.h"
#include "system.h"
#include "vm.h"
#include "rtl.h"
#include "posix_signal.h"
#include "unicode.h"

#include <kos/ushare.h>
#include <hybrid/section.h>
#include <hybrid/atomic.h>
#include <errno.h>
#include <assert.h>
#include <sys/utsname.h>
#include <except.h>
#include <syslog.h>
#include <unicode.h>

DECL_BEGIN

CRT_RARE uintptr_t LIBCCALL map_ushare(uintptr_t name, size_t size) {
 struct mmap_info info;
 /* Map the ushare segment into our address space. */
 info.mi_addr            = (void *)USHARE_ADDRESS_HINT;
 info.mi_prot            = PROT_READ|PROT_EXEC;
 info.mi_flags           = MAP_PRIVATE;
 info.mi_xflag           = XMAP_USHARE;
 info.mi_size            = size;
 info.mi_align           = PAGESIZE;
 info.mi_gap             = 0;
 info.mi_tag             = NULL;
 info.mi_ushare.mu_name  = name;
 info.mi_ushare.mu_start = 0;
 return (uintptr_t)libc_xmmap1(&info);
}
CRT_EXCEPT uintptr_t LIBCCALL Xmap_ushare(uintptr_t name, size_t size) {
 struct mmap_info info;
 /* Map the ushare segment into our address space. */
 info.mi_addr            = (void *)USHARE_ADDRESS_HINT;
 info.mi_prot            = PROT_READ|PROT_EXEC;
 info.mi_flags           = MAP_PRIVATE;
 info.mi_xflag           = XMAP_USHARE;
 info.mi_size            = size;
 info.mi_align           = PAGESIZE;
 info.mi_gap             = 0;
 info.mi_tag             = NULL;
 info.mi_ushare.mu_name  = name;
 info.mi_ushare.mu_start = 0;
 return (uintptr_t)libc_Xxmmap1(&info);
}


CRT_DRARE uintptr_t ushare_strerror_data = (uintptr_t)-1;
CRT_RARE struct ushare_strerror *
LIBCCALL get_ushare_strerror(void) {
 uintptr_t old_address,new_address;
 /* Quick check: has the strerror segment already been loaded? */
 new_address = ushare_strerror_data;
 if (new_address != (uintptr_t)-1)
     return (struct ushare_strerror *)new_address;
 new_address = map_ushare(USHARE_STRERROR_FNAME,USHARE_STRERROR_FSIZE);
 if unlikely(new_address == (uintptr_t)-1)
    return (struct ushare_strerror *)-1;
 old_address = ATOMIC_CMPXCH_VAL(ushare_strerror_data,(uintptr_t)-1,new_address);
 if (old_address == (uintptr_t)-1)
     return (struct ushare_strerror *)new_address;
 sys_munmap((void *)new_address,USHARE_STRERROR_FSIZE);
 return (struct ushare_strerror *)old_address;
}


#if USHARE_STRERROR_VERSION < USHARE_STRERROR_VER_KOSMK3
#error "Unsupported strerror() version"
#endif

EXPORT(__KSYM(strerror_s),libc_strerror_s);
CRT_RARE char const *LIBCCALL libc_strerror_s(errno_t errnum) {
 struct ushare_strerror *ushare;
 struct ushare_strerror_entry *entry;
 ushare = get_ushare_strerror();
 if unlikely((uintptr_t)ushare == (uintptr_t)-1 ||
             ushare->se_version < USHARE_STRERROR_VER_KOSMK3)
    return NULL; /* No ushare segment, or unsupported version. */
 if unlikely((u32)errnum >= ushare->se_enocnt)
    return NULL; /* Invalid error number. */
 entry = (struct ushare_strerror_entry *)((uintptr_t)ushare+ushare->se_enotab+
                                          (uintptr_t)errnum*ushare->se_enoent);
 return (char const *)((uintptr_t)ushare+ushare->se_strtab+entry->se_text);
}
EXPORT(__KSYM(strerrorname_s),libc_strerrorname_s);
CRT_RARE char const *LIBCCALL libc_strerrorname_s(errno_t errnum) {
 struct ushare_strerror *ushare;
 struct ushare_strerror_entry *entry;
 ushare = get_ushare_strerror();
 if unlikely((uintptr_t)ushare == (uintptr_t)-1 ||
             ushare->se_version < USHARE_STRERROR_VER_KOSMK3)
    return NULL; /* No ushare segment, or unsupported version. */
 if unlikely((u32)errnum >= ushare->se_enocnt)
    return NULL; /* Invalid error number. */
 entry = (struct ushare_strerror_entry *)((uintptr_t)ushare+ushare->se_enotab+
                                          (uintptr_t)errnum*ushare->se_enoent);
 return (char const *)((uintptr_t)ushare+ushare->se_strtab+entry->se_name);
}
EXPORT(__KSYM(strsignal_s),libc_strsignal_s);
CRT_RARE char const *LIBCCALL libc_strsignal_s(int signo) {
 struct ushare_strerror *ushare;
 struct ushare_strerror_signl *entry;
 ushare = get_ushare_strerror();
 if unlikely((uintptr_t)ushare == (uintptr_t)-1 ||
             ushare->se_version < USHARE_STRERROR_VER_KOSMK3)
    return NULL; /* No ushare segment, or unsupported version. */
 if unlikely((u32)signo >= ushare->se_sigent)
    return NULL; /* Invalid error number. */
 entry = (struct ushare_strerror_signl *)((uintptr_t)ushare+ushare->se_sigtab+
                                          (uintptr_t)signo*ushare->se_sigent);
 return (char const *)((uintptr_t)ushare+ushare->se_strtab+entry->se_name);
}
EXPORT(__KSYM(strsignaltext_s),libc_strsignaltext_s);
CRT_RARE char const *LIBCCALL libc_strsignaltext_s(int signo) {
 struct ushare_strerror *ushare;
 struct ushare_strerror_signl *entry;
 ushare = get_ushare_strerror();
 if unlikely((uintptr_t)ushare == (uintptr_t)-1 ||
             ushare->se_version < USHARE_STRERROR_VER_KOSMK3)
    return NULL; /* No ushare segment, or unsupported version. */
 if unlikely((u32)signo >= ushare->se_sigent)
    return NULL; /* Invalid error number. */
 entry = (struct ushare_strerror_signl *)((uintptr_t)ushare+ushare->se_sigtab+
                                          (uintptr_t)signo*ushare->se_sigent);
 return (char const *)((uintptr_t)ushare+ushare->se_strtab+entry->se_text);
}

PRIVATE ATTR_RAREBSS char strerror_buf[64*4];
PRIVATE ATTR_RAREBSS char strsignal_buf[32];
#define STRERROR_BUF_LENGTH  64

EXPORT(__KSYM(__xpg_strerror_r),libc_xpg_strerror_r);
CRT_RARE int LIBCCALL
libc_xpg_strerror_r(errno_t errnum,
                    char *buf,
                    size_t buflen) {
 size_t msg_len; char const *string;
 string = libc_strerror_s(errnum);
 if (!buf) buflen = 0;
 if (!string)
     return EINVAL;
 /* Copy the descriptor text. */
 msg_len = (libc_strlen(string)+1)*sizeof(char);
 if (msg_len > buflen)
     return ERANGE;
 libc_memcpy(buf,string,msg_len);
 return 0;
}


CRT_RARE_RODATA char const unknown_error_format[] = "Unknown error %d";
CRT_RARE_RODATA char const unknown_signal_format[] = "Unknown signal %d";

EXPORT_STRONG(__strerror_r,libc_strerror_r); /* GLibc alias */
EXPORT(__KSYM(strerror_r),libc_strerror_r);
CRT_RARE char *LIBCCALL
libc_strerror_r(errno_t errnum,
                char *buf,
                size_t buflen) {
 char const *string;
 string = libc_strerror_s(errnum);
 if (!buf || !buflen) {
  buf    = strerror_buf;
  buflen = STRERROR_BUF_LENGTH;
 }
 if (string) {
  /* Copy the descriptor text. */
  size_t msg_len = (libc_strlen(string)+1)*sizeof(char);
  if (msg_len > buflen) {
   buf    = strerror_buf;
   buflen = STRERROR_BUF_LENGTH;
   if unlikely(msg_len > buflen) {
    msg_len = buflen-1;
    buf[msg_len] = '\0';
   }
  }
  libc_memcpy(buf,string,msg_len);
 } else {
again_unknown:
  if (libc_snprintf(buf,buflen,unknown_error_format,errnum) >= buflen) {
   assert(buf != strerror_buf);
   buf    = strerror_buf;
   buflen = STRERROR_BUF_LENGTH;
   goto again_unknown;
  }
 }
 return buf;
}

EXPORT(__KSYM(strsignal),libc_strsignal);
CRT_RARE char *LIBCCALL
libc_strsignal(int signo) {
 char const *string;
 string = libc_strsignal_s(signo);
 if (string) {
  /* Copy the descriptor text. */
  strsignal_buf[COMPILER_LENOF(strsignal_buf)-1] = '\0';
  libc_strncpy(strsignal_buf,string,COMPILER_LENOF(strsignal_buf)-1);
 } else {
  libc_sprintf(strsignal_buf,unknown_signal_format,signo);
 }
 return strsignal_buf;
}

EXPORT(__KSYM(strerror),libc_strerror);
CRT_RARE char *LIBCCALL
libc_strerror(errno_t errnum) {
 char const *string;
 string = libc_strerror_s(errnum);
 if (string) {
  /* Copy the descriptor text. */
  strerror_buf[STRERROR_BUF_LENGTH-1] = '\0';
  libc_strncpy(strerror_buf,string,STRERROR_BUF_LENGTH-1);
 } else {
  libc_sprintf(strerror_buf,unknown_error_format,errnum);
 }
 return strerror_buf;
}

EXPORT(__KSYM(strerror_l),libc_strerror_l);
CRT_RARE char *LIBCCALL
libc_strerror_l(errno_t errnum, locale_t locale) {
 (void)locale; /* ??? */
 return libc_strerror(errnum);
}


EXPORT(__DSYM(strerror_l),libd_strerror_l);
CRT_DOS char *LIBCCALL
libd_strerror_l(derrno_t errnum, locale_t locale) {
 (void)locale; /* ??? */
 return libd_strerror(errnum);
}
EXPORT(__DSYM(__xpg_strerror_r),libd_xpg_strerror_r);
CRT_DOS int LIBCCALL libd_xpg_strerror_r(errno_t errnum, char *buf, size_t buflen) {
 return libc_xpg_strerror_r(libc_errno_dos2kos(errnum),buf,buflen);
}
EXPORT(__DSYM(strerror_r),libd_strerror_r);
CRT_DOS char *LIBCCALL libd_strerror_r(errno_t errnum, char *buf, size_t buflen) {
 return libc_strerror_r(libc_errno_dos2kos(errnum),buf,buflen);
}
EXPORT(__DSYM(strerror_s),libd_strerror_s);
CRT_DOS char const *LIBCCALL libd_strerror_s(errno_t errnum) {
 return libc_strerror_s(libc_errno_dos2kos(errnum));
}
EXPORT(__DSYM(strerrorname_s),libd_strerrorname_s);
CRT_DOS char const *LIBCCALL libd_strerrorname_s(errno_t errnum) {
 return libc_strerrorname_s(libc_errno_dos2kos(errnum));
}
EXPORT(__DSYM(strerror),libd_strerror);
CRT_DOS char *LIBCCALL libd_strerror(derrno_t errnum) {
 return libc_strerror(libc_errno_dos2kos(errnum));
}
EXPORT(__DSYM(strsignal_s),libd_strsignal_s);
CRT_DOS char const *LIBCCALL libd_strsignal_s(int signo) {
 return libc_strsignal_s(libc_signo_dos2kos(signo));
}
EXPORT(__DSYM(strsignaltext_s),libd_strsignaltext_s);
CRT_DOS char const *LIBCCALL libd_strsignaltext_s(int signo) {
 return libc_strsignaltext_s(libc_signo_dos2kos(signo));
}
EXPORT(__DSYM(strsignal),libd_strsignal);
CRT_DOS char *LIBCCALL libd_strsignal(int signo) {
 return libc_strsignal(libc_signo_dos2kos(signo));
}






/* UTSNAME / HOSTNAME USHARE data */
PRIVATE uintptr_t ushare_utsname_data = (uintptr_t)-1;
CRT_EXCEPT struct utsname *LIBCCALL Xget_ushare_utsname(void) {
 uintptr_t old_address,new_address;
 /* Quick check: has the strerror segment already been loaded? */
 new_address = ushare_utsname_data;
 if (new_address != (uintptr_t)-1)
     return (struct utsname *)new_address;
 new_address = Xmap_ushare(USHARE_UTSNAME_FNAME,USHARE_UTSNAME_FSIZE);
 old_address = ATOMIC_CMPXCH_VAL(ushare_utsname_data,(uintptr_t)-1,new_address);
 if (old_address == (uintptr_t)-1)
     return (struct utsname *)new_address;
 sys_munmap((void *)new_address,USHARE_UTSNAME_FSIZE);
 return (struct utsname *)old_address;
}

EXPORT(Xuname,libc_Xuname);
CRT_EXCEPT void LIBCCALL
libc_Xuname(struct utsname *name) {
 /* Don't include `hostname' in uname information.
  * That's what `gethostname()' is for! */
 libc_memcpy(name,
             Xget_ushare_utsname(),
             sizeof(struct utsname));
}

EXPORT(uname,libc_uname);
CRT_COLD int LIBCCALL
libc_uname(struct utsname *name) {
 LIBC_TRY {
  libc_Xuname(name);
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return 0;
}

EXPORT(Xgethostname,libc_Xgethostname);
CRT_EXCEPT void LIBCCALL
libc_Xgethostname(char *name, size_t buflen) {
 size_t reqlen;
 struct utsname *uts = Xget_ushare_utsname();
 reqlen = libc_strnlen(uts->nodename,_UTSNAME_NODENAME_LENGTH);
 if (buflen < (reqlen+1)*sizeof(char))
     libc_throw_buffer_too_small(reqlen,buflen);
 libc_memcpy(name,uts->nodename,reqlen*sizeof(char));
 name[reqlen] = '\0';
}

EXPORT(gethostname,libc_gethostname);
CRT_COLD int LIBCCALL
libc_gethostname(char *name, size_t buflen) {
 LIBC_TRY {
  size_t reqlen;
  struct utsname *uts = Xget_ushare_utsname();
  reqlen = libc_strnlen(uts->nodename,_UTSNAME_NODENAME_LENGTH);
  if (buflen < (reqlen+1)*sizeof(char)) { libc_seterrno(ENAMETOOLONG); return -1; }
  libc_memcpy(name,uts->nodename,reqlen*sizeof(char));
  name[reqlen] = '\0';
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return 0;
}

EXPORT(Xgetdomainname,libc_Xgetdomainname);
CRT_EXCEPT void LIBCCALL
libc_Xgetdomainname(char *name, size_t buflen) {
 size_t reqlen;
 struct utsname *uts = Xget_ushare_utsname();
 reqlen = libc_strnlen(uts->domainname,_UTSNAME_DOMAIN_LENGTH);
 if (buflen < (reqlen+1)*sizeof(char))
     libc_throw_buffer_too_small(reqlen,buflen);
 libc_memcpy(name,uts->domainname,reqlen*sizeof(char));
 name[reqlen] = '\0';
}

EXPORT(getdomainname,libc_getdomainname);
CRT_COLD int LIBCCALL
libc_getdomainname(char *name, size_t buflen) {
 LIBC_TRY {
  size_t reqlen;
  struct utsname *uts = Xget_ushare_utsname();
  reqlen = libc_strnlen(uts->domainname,_UTSNAME_DOMAIN_LENGTH);
  if (buflen < (reqlen+1)*sizeof(char)) { libc_seterrno(ENAMETOOLONG); return -1; }
  libc_memcpy(name,uts->domainname,reqlen*sizeof(char));
  name[reqlen] = '\0';
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return 0;
}

EXPORT(sethostname,libc_sethostname);
CRT_COLD int LIBCCALL
libc_sethostname(char const *name, size_t len) {
 return FORWARD_SYSTEM_ERROR(sys_sethostname(name,len));
}

EXPORT(setdomainname,libc_setdomainname);
CRT_COLD int LIBCCALL
libc_setdomainname(char const *name, size_t len) {
 return FORWARD_SYSTEM_ERROR(sys_setdomainname(name,len));
}


EXPORT(_strerror_s,libc_dos_strerrorm_s);
CRT_DOS char *LIBCCALL
libc_dos_strerrorm_s(char *__restrict buf, size_t buflen, char const *message) {
 char const *string;
 string = libc_strerror_s(libc_geterrno());
 if (string) {
  if (message) {
   libc_snprintf(buf,buflen,"%s: %s\n",message,string);
  } else {
   libc_snprintf(buf,buflen,"%s\n",string);
  }
 } else if (message) {
  libc_snprintf(buf,buflen,"%s: Unknown error %d\n",message,libc_dos_geterrno());
 } else {
  libc_snprintf(buf,buflen,"Unknown error %d\n",libc_dos_geterrno());
 }
 return buf;
}

EXPORT(_strerror,libc_dos_strerrorm);
CRT_DOS char *LIBCCALL libc_dos_strerrorm(char const *message) {
 return libc_dos_strerrorm_s(strerror_buf,STRERROR_BUF_LENGTH,message);
}

CRT_RARE_RODATA char16_t const raw_error_format16[] = { '%','s','\n',0};
CRT_RARE_RODATA char32_t const raw_error_format32[] = { '%','s','\n',0};
CRT_RARE_RODATA char16_t const user_error_format16[] = { '%','l','s',':',' ','%','s','\n',0};
CRT_RARE_RODATA char32_t const user_error_format32[] = { '%','l','s',':',' ','%','s','\n',0};
CRT_RARE_RODATA char16_t const unknown_user_error_format16[] = {
 '%','l','s',':',' ','U','n','k','n','o','w','n',' ','e','r','r','o','r',' ','%','d','\n',0};
CRT_RARE_RODATA char32_t const unknown_user_error_format32[] = {
 '%','l','s',':',' ','U','n','k','n','o','w','n',' ','e','r','r','o','r',' ','%','d','\n',0};
CRT_RARE_RODATA char16_t const unknown_error_format16[] = {
 'U','n','k','n','o','w','n',' ','e','r','r','o','r',' ','%','d','\n',0};
CRT_RARE_RODATA char32_t const unknown_error_format32[] = {
 'U','n','k','n','o','w','n',' ','e','r','r','o','r',' ','%','d','\n',0};

CRT_DOS errno_t LIBCCALL
libc_impl_w16error_s(char16_t *__restrict buf, size_t buflen,
                     errno_t errnum, char16_t const *message) {
 char const *string;
 string = libc_strerror_s(errnum);
 if (string) {
  if (message) {
   if (libd_snw16printf(buf,buflen,user_error_format16,message,string) < 0)
       goto fail;
  } else {
   if (libd_snw16printf(buf,buflen,raw_error_format16,string) < 0)
       goto fail;
  }
 } else if (message) {
  if (libd_snw16printf(buf,buflen,unknown_user_error_format16,message,errnum) < 0)
      goto fail;
 } else {
  libd_snw16printf(buf,buflen,unknown_error_format16,errnum);
 }
 return 0;
fail:
 return libc_geterrno();
}
CRT_DOS_EXT errno_t LIBCCALL
libc_impl_w32error_s(char32_t *__restrict buf, size_t buflen,
                     errno_t errnum, char32_t const *message) {
 char const *string;
 string = libc_strerror_s(errnum);
 if (string) {
  if (message) {
   if (libc_snw32printf(buf,buflen,user_error_format32,message,string) < 0)
       goto fail;
  } else {
   if (libd_snw32printf(buf,buflen,raw_error_format32,string) < 0)
       goto fail;
  }
 } else if (message) {
  if (libc_snw32printf(buf,buflen,unknown_user_error_format32,message,errnum) < 0)
      goto fail;
 } else {
  libc_snw32printf(buf,buflen,unknown_error_format32,errnum);
 }
 return 0;
fail:
 return libc_geterrno();
}



EXPORT(__KSYMw16(wcserror),libc_w16error);
EXPORT(__KSYMw32(wcserror),libc_w32error);
EXPORT(__DSYMw16(_wcserror),libc_dos_w16error);
EXPORT(__DSYMw32(wcserror),libc_dos_w32error);
EXPORT(__SYMw16(__wcserror),libc_w16errorm);
EXPORT(__SYMw32(__wcserror),libc_w32errorm);
EXPORT(__KSYMw16(wcserror_s),libc_w16error_s);
EXPORT(__KSYMw32(wcserror_s),libc_w32error_s);
EXPORT(__DSYMw16(_wcserror_s),libc_dos_w16error_s);
EXPORT(__DSYMw32(wcserror_s),libc_dos_w32error_s);
EXPORT(__SYMw16(__wcserror_s),libc_w16errorm_s);
EXPORT(__SYMw32(__wcserror_s),libc_w32errorm_s);
CRT_DOS errno_t LIBCCALL libc_w16error_s(char16_t *__restrict buf, size_t buflen, errno_t errnum) { return libc_impl_w16error_s(buf,buflen,errnum,NULL); }
CRT_DOS_EXT errno_t LIBCCALL libc_w32error_s(char32_t *__restrict buf, size_t buflen, errno_t errnum) { return libc_impl_w32error_s(buf,buflen,errnum,NULL); }
CRT_DOS errno_t LIBCCALL libc_w16errorm_s(char16_t *__restrict buf, size_t buflen, char16_t const *message) { return libc_impl_w16error_s(buf,buflen,libc_geterrno(),message); }
CRT_DOS_EXT errno_t LIBCCALL libc_w32errorm_s(char32_t *__restrict buf, size_t buflen, char32_t const *message) { return libc_impl_w32error_s(buf,buflen,libc_geterrno(),message); }
CRT_DOS derrno_t LIBCCALL libc_dos_w16error_s(char16_t *__restrict buf, size_t buflen, derrno_t errnum) { return libc_errno_kos2dos(libc_impl_w16error_s(buf,buflen,libc_errno_dos2kos(errnum),NULL)); }
CRT_DOS_EXT derrno_t LIBCCALL libc_dos_w32error_s(char32_t *__restrict buf, size_t buflen, derrno_t errnum) { return libc_errno_kos2dos(libc_impl_w32error_s(buf,buflen,libc_errno_dos2kos(errnum),NULL)); }
CRT_DOS char16_t *LIBCCALL libc_w16error(errno_t errnum) { return libc_impl_w16error_s((char16_t *)strerror_buf,STRERROR_BUF_LENGTH,errnum,NULL) ? NULL : (char16_t *)strerror_buf; }
CRT_DOS_EXT char32_t *LIBCCALL libc_w32error(errno_t errnum) { return libc_impl_w32error_s((char32_t *)strerror_buf,STRERROR_BUF_LENGTH,errnum,NULL) ? NULL : (char32_t *)strerror_buf; }
CRT_DOS char16_t *LIBCCALL libc_w16errorm(char16_t const *message) { return libc_impl_w16error_s((char16_t *)strerror_buf,STRERROR_BUF_LENGTH,libc_geterrno(),message) ? NULL : (char16_t *)strerror_buf; }
CRT_DOS_EXT char32_t *LIBCCALL libc_w32errorm(char32_t const *message) { return libc_impl_w32error_s((char32_t *)strerror_buf,STRERROR_BUF_LENGTH,libc_geterrno(),message) ? NULL : (char32_t *)strerror_buf; }
CRT_DOS char16_t *LIBCCALL libc_dos_w16error(derrno_t errnum) { return libc_w16error(libc_errno_dos2kos(errnum)); }
CRT_DOS_EXT char32_t *LIBCCALL libc_dos_w32error(derrno_t errnum) { return libc_w32error(libc_errno_dos2kos(errnum)); }




DECL_END

#endif /* !GUARD_LIBS_LIBC_USHARE_C */
