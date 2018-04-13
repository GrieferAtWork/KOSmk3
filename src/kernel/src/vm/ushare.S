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
#include <hybrid/compiler.h>
#include <hybrid/limits.h>
#include <hybrid/asm.h>
#include <kos/ushare.h>

#define PUT16  .word
#define PUT32  .long

/*[[[deemon

#define _KOS_SOURCE 1
#define __USE_KOS 1
#include "../../../../include/errno.h"

local errno = [];
local signl = [];
function __errno(id,name,text) {
	if (id >= #errno) {
		errno.resize(id+1,none);
	}
	errno[id] = pack name,text;
}
function __signl(id,name,text) {
	if (id >= #signl) {
		signl.resize(id+1,none);
	}
	signl[id] = pack name,text;
}

#define ERRNO(id,text) __errno(id,#id,text);
ERRNO(EOK,             "Operation completed successfully")
ERRNO(EPERM,           "Operation not permitted")
ERRNO(ENOENT,          "No such file or directory")
ERRNO(ESRCH,           "No such process")
ERRNO(EINTR,           "Interrupted system call")
ERRNO(EIO,             "I/O error")
ERRNO(ENXIO,           "No such device or address")
ERRNO(E2BIG,           "Argument list too long")
ERRNO(ENOEXEC,         "Exec format error")
ERRNO(EBADF,           "Bad file number")
ERRNO(ECHILD,          "No child processes")
ERRNO(EAGAIN,          "Try again")
ERRNO(ENOMEM,          "Out of memory")
ERRNO(EACCES,          "Permission denied")
ERRNO(EFAULT,          "Bad address")
ERRNO(ENOTBLK,         "Block device required")
ERRNO(EBUSY,           "Device or resource busy")
ERRNO(EEXIST,          "File exists")
ERRNO(EXDEV,           "Cross-device link")
ERRNO(ENODEV,          "No such device")
ERRNO(ENOTDIR,         "Not a directory")
ERRNO(EISDIR,          "Is a directory")
ERRNO(EINVAL,          "Invalid argument")
ERRNO(ENFILE,          "File table overflow")
ERRNO(EMFILE,          "Too many open files")
ERRNO(ENOTTY,          "Not a typewriter")
ERRNO(ETXTBSY,         "Text file busy")
ERRNO(EFBIG,           "File too large")
ERRNO(ENOSPC,          "No space left on device")
ERRNO(ESPIPE,          "Illegal seek")
ERRNO(EROFS,           "Read-only file system")
ERRNO(EMLINK,          "Too many links")
ERRNO(EPIPE,           "Broken pipe")
ERRNO(EDOM,            "Math argument out of domain of func")
ERRNO(ERANGE,          "Math result not representable")
ERRNO(EDEADLK,         "Resource deadlock would occur")
ERRNO(ENAMETOOLONG,    "File name too long")
ERRNO(ENOLCK,          "No record locks available")
ERRNO(ENOSYS,          "Invalid system call number")
ERRNO(ENOTEMPTY,       "Directory not empty")
ERRNO(ELOOP,           "Too many symbolic links encountered")
#undef EWOULDBLOCK
#define EWOULDBLOCK 41
ERRNO(EWOULDBLOCK,     "Operation would block")
ERRNO(ENOMSG,          "No message of desired type")
ERRNO(EIDRM,           "Identifier removed")
ERRNO(ECHRNG,          "Channel number out of range")
ERRNO(EL2NSYNC,        "Level 2 not synchronized")
ERRNO(EL3HLT,          "Level 3 halted")
ERRNO(EL3RST,          "Level 3 reset")
ERRNO(ELNRNG,          "Link number out of range")
ERRNO(EUNATCH,         "Protocol driver not attached")
ERRNO(ENOCSI,          "No CSI structure available")
ERRNO(EL2HLT,          "Level 2 halted")
ERRNO(EBADE,           "Invalid exchange")
ERRNO(EBADR,           "Invalid request descriptor")
ERRNO(EXFULL,          "Exchange full")
ERRNO(ENOANO,          "No anode")
ERRNO(EBADRQC,         "Invalid request code")
#undef EDEADLOCK
#define EDEADLOCK 58
ERRNO(EDEADLOCK,       "Resource deadlock would occur")
ERRNO(EBADSLT,         "Invalid slot")
ERRNO(EBFONT,          "Bad font file format")
ERRNO(ENOSTR,          "Device not a stream")
ERRNO(ENODATA,         "No data available")
ERRNO(ETIME,           "Timer expired")
ERRNO(ENOSR,           "Out of streams resources")
ERRNO(ENONET,          "Machine is not on the network")
ERRNO(ENOPKG,          "Package not installed")
ERRNO(EREMOTE,         "Object is remote")
ERRNO(ENOLINK,         "Link has been severed")
ERRNO(EADV,            "Advertise error")
ERRNO(ESRMNT,          "Srmount error")
ERRNO(ECOMM,           "Communication error on send")
ERRNO(EPROTO,          "Protocol error")
ERRNO(EMULTIHOP,       "Multihop attempted")
ERRNO(EDOTDOT,         "RFS specific error")
ERRNO(EBADMSG,         "Not a data message")
ERRNO(EOVERFLOW,       "Value too large for defined data type")
ERRNO(ENOTUNIQ,        "Name not unique on network")
ERRNO(EBADFD,          "File descriptor in bad state")
ERRNO(EREMCHG,         "Remote address changed")
ERRNO(ELIBACC,         "Can not access a needed shared library")
ERRNO(ELIBBAD,         "Accessing a corrupted shared library")
ERRNO(ELIBSCN,         ".lib section in a.out corrupted")
ERRNO(ELIBMAX,         "Attempting to link in too many shared libraries")
ERRNO(ELIBEXEC,        "Cannot exec a shared library directly")
ERRNO(EILSEQ,          "Illegal byte sequence")
ERRNO(ERESTART,        "Interrupted system call should be restarted")
ERRNO(ESTRPIPE,        "Streams pipe error")
ERRNO(EUSERS,          "Too many users")
ERRNO(ENOTSOCK,        "Socket operation on non-socket")
ERRNO(EDESTADDRREQ,    "Destination address required")
ERRNO(EMSGSIZE,        "Message too long")
ERRNO(EPROTOTYPE,      "Protocol wrong type for socket")
ERRNO(ENOPROTOOPT,     "Protocol not available")
ERRNO(EPROTONOSUPPORT, "Protocol not supported")
ERRNO(ESOCKTNOSUPPORT, "Socket type not supported")
ERRNO(EOPNOTSUPP,      "Operation not supported on transport endpoint")
ERRNO(EPFNOSUPPORT,    "Protocol family not supported")
ERRNO(EAFNOSUPPORT,    "Address family not supported by protocol")
ERRNO(EADDRINUSE,      "Address already in use")
ERRNO(EADDRNOTAVAIL,   "Cannot assign requested address")
ERRNO(ENETDOWN,        "Network is down")
ERRNO(ENETUNREACH,     "Network is unreachable")
ERRNO(ENETRESET,       "Network dropped connection because of reset")
ERRNO(ECONNABORTED,    "Software caused connection abort")
ERRNO(ECONNRESET,      "Connection reset by peer")
ERRNO(ENOBUFS,         "No buffer space available")
ERRNO(EISCONN,         "Transport endpoint is already connected")
ERRNO(ENOTCONN,        "Transport endpoint is not connected")
ERRNO(ESHUTDOWN,       "Cannot send after transport endpoint shutdown")
ERRNO(ETOOMANYREFS,    "Too many references: cannot splice")
ERRNO(ETIMEDOUT,       "Connection timed out")
ERRNO(ECONNREFUSED,    "Connection refused")
ERRNO(EHOSTDOWN,       "Host is down")
ERRNO(EHOSTUNREACH,    "No route to host")
ERRNO(EALREADY,        "Operation already in progress")
ERRNO(EINPROGRESS,     "Operation now in progress")
ERRNO(ESTALE,          "Stale file handle")
ERRNO(EUCLEAN,         "Structure needs cleaning")
ERRNO(ENOTNAM,         "Not a XENIX named type file")
ERRNO(ENAVAIL,         "No XENIX semaphores available")
ERRNO(EISNAM,          "Is a named type file")
ERRNO(EREMOTEIO,       "Remote I/O error")
ERRNO(EDQUOT,          "Quota exceeded")
ERRNO(ENOMEDIUM,       "No medium found")
ERRNO(EMEDIUMTYPE,     "Wrong medium type")
ERRNO(ECANCELED,       "Operation Canceled")
ERRNO(ENOKEY,          "Required key not available")
ERRNO(EKEYEXPIRED,     "Key has expired")
ERRNO(EKEYREVOKED,     "Key has been revoked")
ERRNO(EKEYREJECTED,    "Key was rejected by service")
ERRNO(EOWNERDEAD,      "Owner died")
ERRNO(ENOTRECOVERABLE, "State not recoverable")
ERRNO(ERFKILL,         "Operation not possible due to RF-kill")
ERRNO(EHWPOISON,       "Memory page has hardware error")
#undef ERRNO

__signl(1 -1,"SIGHUP","Hangup (POSIX)");
__signl(2 -1,"SIGINT","Interrupt (ANSI)");
__signl(3 -1,"SIGQUIT","Quit (POSIX)");
__signl(4 -1,"SIGILL","Illegal instruction (ANSI)");
__signl(5 -1,"SIGTRAP","Trace trap (POSIX)");
__signl(6 -1,"SIGABRT","Abort (ANSI)");
__signl(7 -1,"SIGBUS","BUS error (4.2 BSD)");
__signl(8 -1,"SIGFPE","Floating-point exception (ANSI)");
__signl(9 -1,"SIGKILL","Kill, unblockable (POSIX)");
__signl(10-1,"SIGUSR1","User-defined signal 1 (POSIX)");
__signl(11-1,"SIGSEGV","Segmentation violation (ANSI)");
__signl(12-1,"SIGUSR2","User-defined signal 2 (POSIX)");
__signl(13-1,"SIGPIPE","Broken pipe (POSIX)");
__signl(14-1,"SIGALRM","Alarm clock (POSIX)");
__signl(15-1,"SIGTERM","Termination (ANSI)");
__signl(16-1,"SIGSTKFLT","Stack fault");
__signl(17-1,"SIGCHLD","Child status has changed (POSIX)");
__signl(18-1,"SIGCONT","Continue (POSIX)");
__signl(19-1,"SIGSTOP","Stop, unblockable (POSIX)");
__signl(20-1,"SIGTSTP","Keyboard stop (POSIX)");
__signl(21-1,"SIGTTIN","Background read from tty (POSIX)");
__signl(22-1,"SIGTTOU","Background write to tty (POSIX)");
__signl(23-1,"SIGURG","Urgent condition on socket (4.2 BSD)");
__signl(24-1,"SIGXCPU","CPU limit exceeded (4.2 BSD)");
__signl(25-1,"SIGXFSZ","File size limit exceeded (4.2 BSD)");
__signl(26-1,"SIGVTALRM","Virtual alarm clock (4.2 BSD)");
__signl(27-1,"SIGPROF","Profiling alarm clock (4.2 BSD)");
__signl(28-1,"SIGWINCH","Window size change (4.3 BSD, Sun)");
__signl(29-1,"SIGIO","I/O now possible (4.2 BSD)");
__signl(30-1,"SIGPWR","Power failure restart (System V)");
__signl(31-1,"SIGSYS","Bad system call");


#include <file>
#include <util>

local errno_strings = "";
local eno_string_offsets = [none] * #errno;
local sig_string_offsets = [none] * #signl;
for (local i,name_and_text: util::enumerate(errno)) {
	local name,text = name_and_text...;
	local name_offset = errno_strings.find(name);
	if (name_offset == -1) {
		name_offset = #errno_strings;
		errno_strings += name+"\0";
	}
	local text_offset = errno_strings.find(text);
	if (text_offset == -1) {
		text_offset = #errno_strings;
		errno_strings += text+"\0";
	}
	eno_string_offsets[i] = pack name_offset,text_offset;
}
for (local i,name_and_text: util::enumerate(signl)) {
	local name,text = name_and_text...;
	local name_offset = errno_strings.find(name);
	if (name_offset == -1) {
		name_offset = #errno_strings;
		errno_strings += name+"\0";
	}
	local text_offset = errno_strings.find(text);
	if (text_offset == -1) {
		text_offset = #errno_strings;
		errno_strings += text+"\0";
	}
	sig_string_offsets[i] = pack name_offset,text_offset;
}

local header_size = 8*4;
local enotab_offset = header_size;
local sigtab_offset = enotab_offset+4*#eno_string_offsets;
local strtab_offset = sigtab_offset+4*#sig_string_offsets;

print ".section .data.ushare.strerror";
print "PUBLIC_OBJECT(ushare_strerror)";
print "	/" "* etd_version *" "/ PUT32 USHARE_STRERROR_VER_KOSMK3";
print "	/" "* etd_strtab  *" "/ PUT32",strtab_offset;
print "	/" "* etd_enotab  *" "/ PUT32",enotab_offset;
print "	/" "* etd_enocnt  *" "/ PUT32 "+#eno_string_offsets;
print "	/" "* etd_enoent  *" "/ PUT32 4";
print "	/" "* etd_sigtab  *" "/ PUT32",sigtab_offset;
print "	/" "* etd_sigcnt  *" "/ PUT32 "+#sig_string_offsets;
print "	/" "* etd_sigent  *" "/ PUT32 2";

print;
print "/" "* STRERROR_VECTOR *" "/";
local index = 0;
for (local off_name,off_text: eno_string_offsets) {
	print "	/" "* ["+(str index).rjust(3)+"].se_name *" "/ PUT16",off_name;
	print "	/" "* ["+(str index).rjust(3)+"].se_text *" "/ PUT16",off_text;
	++index;
}

print;
print "/" "* STRSIGNAL_VECTOR *" "/";
local index = 0;
for (local off_name,off_text: sig_string_offsets) {
	print "	/" "* ["+(str index).rjust(3)+"].se_name *" "/ PUT16",off_name;
	print "	/" "* ["+(str index).rjust(3)+"].se_text *" "/ PUT16",off_text;
	++index;
}

print;
print "/" "* STRTAB *" "/";
for (local s: errno_strings.split("\0")) {
	if (s) print "	.string",repr s;
}
print ".skip USHARE_STRERROR_FSIZE - (. - ushare_strerror)";
print "SYMEND(ushare_strerror)";

]]]*/
.section .data.ushare.strerror
PUBLIC_OBJECT(ushare_strerror)
	/* etd_version */ PUT32 USHARE_STRERROR_VER_KOSMK3
	/* etd_strtab  */ PUT32 692
	/* etd_enotab  */ PUT32 32
	/* etd_enocnt  */ PUT32 134
	/* etd_enoent  */ PUT32 4
	/* etd_sigtab  */ PUT32 568
	/* etd_sigcnt  */ PUT32 31
	/* etd_sigent  */ PUT32 2

/* STRERROR_VECTOR */
	/* [  0].se_name */ PUT16 0
	/* [  0].se_text */ PUT16 4
	/* [  1].se_name */ PUT16 37
	/* [  1].se_text */ PUT16 43
	/* [  2].se_name */ PUT16 67
	/* [  2].se_text */ PUT16 74
	/* [  3].se_name */ PUT16 100
	/* [  3].se_text */ PUT16 106
	/* [  4].se_name */ PUT16 122
	/* [  4].se_text */ PUT16 128
	/* [  5].se_name */ PUT16 152
	/* [  5].se_text */ PUT16 156
	/* [  6].se_name */ PUT16 166
	/* [  6].se_text */ PUT16 172
	/* [  7].se_name */ PUT16 198
	/* [  7].se_text */ PUT16 204
	/* [  8].se_name */ PUT16 227
	/* [  8].se_text */ PUT16 235
	/* [  9].se_name */ PUT16 253
	/* [  9].se_text */ PUT16 259
	/* [ 10].se_name */ PUT16 275
	/* [ 10].se_text */ PUT16 282
	/* [ 11].se_name */ PUT16 301
	/* [ 11].se_text */ PUT16 308
	/* [ 12].se_name */ PUT16 318
	/* [ 12].se_text */ PUT16 325
	/* [ 13].se_name */ PUT16 339
	/* [ 13].se_text */ PUT16 346
	/* [ 14].se_name */ PUT16 364
	/* [ 14].se_text */ PUT16 371
	/* [ 15].se_name */ PUT16 383
	/* [ 15].se_text */ PUT16 391
	/* [ 16].se_name */ PUT16 413
	/* [ 16].se_text */ PUT16 419
	/* [ 17].se_name */ PUT16 443
	/* [ 17].se_text */ PUT16 450
	/* [ 18].se_name */ PUT16 462
	/* [ 18].se_text */ PUT16 468
	/* [ 19].se_name */ PUT16 486
	/* [ 19].se_text */ PUT16 172
	/* [ 20].se_name */ PUT16 493
	/* [ 20].se_text */ PUT16 501
	/* [ 21].se_name */ PUT16 517
	/* [ 21].se_text */ PUT16 524
	/* [ 22].se_name */ PUT16 539
	/* [ 22].se_text */ PUT16 546
	/* [ 23].se_name */ PUT16 563
	/* [ 23].se_text */ PUT16 570
	/* [ 24].se_name */ PUT16 590
	/* [ 24].se_text */ PUT16 597
	/* [ 25].se_name */ PUT16 617
	/* [ 25].se_text */ PUT16 624
	/* [ 26].se_name */ PUT16 641
	/* [ 26].se_text */ PUT16 649
	/* [ 27].se_name */ PUT16 664
	/* [ 27].se_text */ PUT16 670
	/* [ 28].se_name */ PUT16 685
	/* [ 28].se_text */ PUT16 692
	/* [ 29].se_name */ PUT16 716
	/* [ 29].se_text */ PUT16 723
	/* [ 30].se_name */ PUT16 736
	/* [ 30].se_text */ PUT16 742
	/* [ 31].se_name */ PUT16 764
	/* [ 31].se_text */ PUT16 771
	/* [ 32].se_name */ PUT16 786
	/* [ 32].se_text */ PUT16 792
	/* [ 33].se_name */ PUT16 804
	/* [ 33].se_text */ PUT16 809
	/* [ 34].se_name */ PUT16 845
	/* [ 34].se_text */ PUT16 852
	/* [ 35].se_name */ PUT16 882
	/* [ 35].se_text */ PUT16 890
	/* [ 36].se_name */ PUT16 920
	/* [ 36].se_text */ PUT16 933
	/* [ 37].se_name */ PUT16 952
	/* [ 37].se_text */ PUT16 959
	/* [ 38].se_name */ PUT16 985
	/* [ 38].se_text */ PUT16 992
	/* [ 39].se_name */ PUT16 1019
	/* [ 39].se_text */ PUT16 1029
	/* [ 40].se_name */ PUT16 1049
	/* [ 40].se_text */ PUT16 1055
	/* [ 41].se_name */ PUT16 1091
	/* [ 41].se_text */ PUT16 1103
	/* [ 42].se_name */ PUT16 1125
	/* [ 42].se_text */ PUT16 1132
	/* [ 43].se_name */ PUT16 1159
	/* [ 43].se_text */ PUT16 1165
	/* [ 44].se_name */ PUT16 1184
	/* [ 44].se_text */ PUT16 1191
	/* [ 45].se_name */ PUT16 1219
	/* [ 45].se_text */ PUT16 1228
	/* [ 46].se_name */ PUT16 1253
	/* [ 46].se_text */ PUT16 1260
	/* [ 47].se_name */ PUT16 1275
	/* [ 47].se_text */ PUT16 1282
	/* [ 48].se_name */ PUT16 1296
	/* [ 48].se_text */ PUT16 1303
	/* [ 49].se_name */ PUT16 1328
	/* [ 49].se_text */ PUT16 1336
	/* [ 50].se_name */ PUT16 1365
	/* [ 50].se_text */ PUT16 1372
	/* [ 51].se_name */ PUT16 1399
	/* [ 51].se_text */ PUT16 1406
	/* [ 52].se_name */ PUT16 1421
	/* [ 52].se_text */ PUT16 1427
	/* [ 53].se_name */ PUT16 1444
	/* [ 53].se_text */ PUT16 1450
	/* [ 54].se_name */ PUT16 1477
	/* [ 54].se_text */ PUT16 1484
	/* [ 55].se_name */ PUT16 1498
	/* [ 55].se_text */ PUT16 1505
	/* [ 56].se_name */ PUT16 1514
	/* [ 56].se_text */ PUT16 1522
	/* [ 57].se_name */ PUT16 1543
	/* [ 57].se_text */ PUT16 1551
	/* [ 58].se_name */ PUT16 1564
	/* [ 58].se_text */ PUT16 890
	/* [ 59].se_name */ PUT16 1574
	/* [ 59].se_text */ PUT16 1581
	/* [ 60].se_name */ PUT16 1602
	/* [ 60].se_text */ PUT16 1609
	/* [ 61].se_name */ PUT16 1629
	/* [ 61].se_text */ PUT16 1637
	/* [ 62].se_name */ PUT16 1655
	/* [ 62].se_text */ PUT16 1661
	/* [ 63].se_name */ PUT16 1675
	/* [ 63].se_text */ PUT16 1681
	/* [ 64].se_name */ PUT16 1706
	/* [ 64].se_text */ PUT16 1713
	/* [ 65].se_name */ PUT16 1743
	/* [ 65].se_text */ PUT16 1750
	/* [ 66].se_name */ PUT16 1772
	/* [ 66].se_text */ PUT16 1780
	/* [ 67].se_name */ PUT16 1797
	/* [ 67].se_text */ PUT16 1805
	/* [ 68].se_name */ PUT16 1827
	/* [ 68].se_text */ PUT16 1832
	/* [ 69].se_name */ PUT16 1848
	/* [ 69].se_text */ PUT16 1855
	/* [ 70].se_name */ PUT16 1869
	/* [ 70].se_text */ PUT16 1875
	/* [ 71].se_name */ PUT16 1903
	/* [ 71].se_text */ PUT16 1910
	/* [ 72].se_name */ PUT16 1925
	/* [ 72].se_text */ PUT16 1935
	/* [ 73].se_name */ PUT16 1954
	/* [ 73].se_text */ PUT16 1962
	/* [ 74].se_name */ PUT16 1981
	/* [ 74].se_text */ PUT16 1989
	/* [ 75].se_name */ PUT16 2008
	/* [ 75].se_text */ PUT16 2018
	/* [ 76].se_name */ PUT16 2056
	/* [ 76].se_text */ PUT16 2065
	/* [ 77].se_name */ PUT16 2092
	/* [ 77].se_text */ PUT16 2099
	/* [ 78].se_name */ PUT16 2128
	/* [ 78].se_text */ PUT16 2136
	/* [ 79].se_name */ PUT16 2159
	/* [ 79].se_text */ PUT16 2167
	/* [ 80].se_name */ PUT16 2206
	/* [ 80].se_text */ PUT16 2214
	/* [ 81].se_name */ PUT16 2251
	/* [ 81].se_text */ PUT16 2259
	/* [ 82].se_name */ PUT16 2291
	/* [ 82].se_text */ PUT16 2299
	/* [ 83].se_name */ PUT16 2347
	/* [ 83].se_text */ PUT16 2356
	/* [ 84].se_name */ PUT16 2394
	/* [ 84].se_text */ PUT16 2401
	/* [ 85].se_name */ PUT16 2423
	/* [ 85].se_text */ PUT16 2432
	/* [ 86].se_name */ PUT16 2476
	/* [ 86].se_text */ PUT16 2485
	/* [ 87].se_name */ PUT16 2504
	/* [ 87].se_text */ PUT16 2511
	/* [ 88].se_name */ PUT16 2526
	/* [ 88].se_text */ PUT16 2535
	/* [ 89].se_name */ PUT16 2566
	/* [ 89].se_text */ PUT16 2579
	/* [ 90].se_name */ PUT16 2608
	/* [ 90].se_text */ PUT16 2617
	/* [ 91].se_name */ PUT16 2634
	/* [ 91].se_text */ PUT16 2645
	/* [ 92].se_name */ PUT16 2676
	/* [ 92].se_text */ PUT16 2688
	/* [ 93].se_name */ PUT16 2711
	/* [ 93].se_text */ PUT16 2727
	/* [ 94].se_name */ PUT16 2750
	/* [ 94].se_text */ PUT16 2766
	/* [ 95].se_name */ PUT16 2792
	/* [ 95].se_text */ PUT16 2803
	/* [ 96].se_name */ PUT16 2849
	/* [ 96].se_text */ PUT16 2862
	/* [ 97].se_name */ PUT16 2892
	/* [ 97].se_text */ PUT16 2905
	/* [ 98].se_name */ PUT16 2946
	/* [ 98].se_text */ PUT16 2957
	/* [ 99].se_name */ PUT16 2980
	/* [ 99].se_text */ PUT16 2994
	/* [100].se_name */ PUT16 3026
	/* [100].se_text */ PUT16 3035
	/* [101].se_name */ PUT16 3051
	/* [101].se_text */ PUT16 3063
	/* [102].se_name */ PUT16 3086
	/* [102].se_text */ PUT16 3096
	/* [103].se_name */ PUT16 3140
	/* [103].se_text */ PUT16 3153
	/* [104].se_name */ PUT16 3186
	/* [104].se_text */ PUT16 3197
	/* [105].se_name */ PUT16 3222
	/* [105].se_text */ PUT16 3230
	/* [106].se_name */ PUT16 3256
	/* [106].se_text */ PUT16 3264
	/* [107].se_name */ PUT16 3304
	/* [107].se_text */ PUT16 3313
	/* [108].se_name */ PUT16 3349
	/* [108].se_text */ PUT16 3359
	/* [109].se_name */ PUT16 3405
	/* [109].se_text */ PUT16 3418
	/* [110].se_name */ PUT16 3453
	/* [110].se_text */ PUT16 3463
	/* [111].se_name */ PUT16 3484
	/* [111].se_text */ PUT16 3497
	/* [112].se_name */ PUT16 3516
	/* [112].se_text */ PUT16 3526
	/* [113].se_name */ PUT16 3539
	/* [113].se_text */ PUT16 3552
	/* [114].se_name */ PUT16 3569
	/* [114].se_text */ PUT16 3578
	/* [115].se_name */ PUT16 3608
	/* [115].se_text */ PUT16 3620
	/* [116].se_name */ PUT16 3646
	/* [116].se_text */ PUT16 3653
	/* [117].se_name */ PUT16 3671
	/* [117].se_text */ PUT16 3679
	/* [118].se_name */ PUT16 3704
	/* [118].se_text */ PUT16 3712
	/* [119].se_name */ PUT16 3740
	/* [119].se_text */ PUT16 3748
	/* [120].se_name */ PUT16 3778
	/* [120].se_text */ PUT16 3785
	/* [121].se_name */ PUT16 3806
	/* [121].se_text */ PUT16 3816
	/* [122].se_name */ PUT16 3833
	/* [122].se_text */ PUT16 3840
	/* [123].se_name */ PUT16 3855
	/* [123].se_text */ PUT16 3865
	/* [124].se_name */ PUT16 3881
	/* [124].se_text */ PUT16 3893
	/* [125].se_name */ PUT16 3911
	/* [125].se_text */ PUT16 3921
	/* [126].se_name */ PUT16 3940
	/* [126].se_text */ PUT16 3947
	/* [127].se_name */ PUT16 3974
	/* [127].se_text */ PUT16 3986
	/* [128].se_name */ PUT16 4002
	/* [128].se_text */ PUT16 4014
	/* [129].se_name */ PUT16 4035
	/* [129].se_text */ PUT16 4048
	/* [130].se_name */ PUT16 4076
	/* [130].se_text */ PUT16 4087
	/* [131].se_name */ PUT16 4098
	/* [131].se_text */ PUT16 4114
	/* [132].se_name */ PUT16 4136
	/* [132].se_text */ PUT16 4144
	/* [133].se_name */ PUT16 4182
	/* [133].se_text */ PUT16 4192

/* STRSIGNAL_VECTOR */
	/* [  0].se_name */ PUT16 4223
	/* [  0].se_text */ PUT16 4230
	/* [  1].se_name */ PUT16 4245
	/* [  1].se_text */ PUT16 4252
	/* [  2].se_name */ PUT16 4269
	/* [  2].se_text */ PUT16 4277
	/* [  3].se_name */ PUT16 4290
	/* [  3].se_text */ PUT16 4297
	/* [  4].se_name */ PUT16 4324
	/* [  4].se_text */ PUT16 4332
	/* [  5].se_name */ PUT16 4351
	/* [  5].se_text */ PUT16 4359
	/* [  6].se_name */ PUT16 4372
	/* [  6].se_text */ PUT16 4379
	/* [  7].se_name */ PUT16 4399
	/* [  7].se_text */ PUT16 4406
	/* [  8].se_name */ PUT16 4438
	/* [  8].se_text */ PUT16 4446
	/* [  9].se_name */ PUT16 4472
	/* [  9].se_text */ PUT16 4480
	/* [ 10].se_name */ PUT16 4510
	/* [ 10].se_text */ PUT16 4518
	/* [ 11].se_name */ PUT16 4548
	/* [ 11].se_text */ PUT16 4556
	/* [ 12].se_name */ PUT16 4586
	/* [ 12].se_text */ PUT16 4594
	/* [ 13].se_name */ PUT16 4614
	/* [ 13].se_text */ PUT16 4622
	/* [ 14].se_name */ PUT16 4642
	/* [ 14].se_text */ PUT16 4650
	/* [ 15].se_name */ PUT16 4669
	/* [ 15].se_text */ PUT16 4679
	/* [ 16].se_name */ PUT16 4691
	/* [ 16].se_text */ PUT16 4699
	/* [ 17].se_name */ PUT16 4732
	/* [ 17].se_text */ PUT16 4740
	/* [ 18].se_name */ PUT16 4757
	/* [ 18].se_text */ PUT16 4765
	/* [ 19].se_name */ PUT16 4791
	/* [ 19].se_text */ PUT16 4799
	/* [ 20].se_name */ PUT16 4821
	/* [ 20].se_text */ PUT16 4829
	/* [ 21].se_name */ PUT16 4862
	/* [ 21].se_text */ PUT16 4870
	/* [ 22].se_name */ PUT16 4902
	/* [ 22].se_text */ PUT16 4909
	/* [ 23].se_name */ PUT16 4946
	/* [ 23].se_text */ PUT16 4954
	/* [ 24].se_name */ PUT16 4983
	/* [ 24].se_text */ PUT16 4991
	/* [ 25].se_name */ PUT16 5026
	/* [ 25].se_text */ PUT16 5036
	/* [ 26].se_name */ PUT16 5066
	/* [ 26].se_text */ PUT16 5074
	/* [ 27].se_name */ PUT16 5106
	/* [ 27].se_text */ PUT16 5115
	/* [ 28].se_name */ PUT16 5149
	/* [ 28].se_text */ PUT16 5155
	/* [ 29].se_name */ PUT16 5182
	/* [ 29].se_text */ PUT16 5189
	/* [ 30].se_name */ PUT16 5222
	/* [ 30].se_text */ PUT16 5229

/* STRTAB */
	.string "EOK"
	.string "Operation completed successfully"
	.string "EPERM"
	.string "Operation not permitted"
	.string "ENOENT"
	.string "No such file or directory"
	.string "ESRCH"
	.string "No such process"
	.string "EINTR"
	.string "Interrupted system call"
	.string "EIO"
	.string "I/O error"
	.string "ENXIO"
	.string "No such device or address"
	.string "E2BIG"
	.string "Argument list too long"
	.string "ENOEXEC"
	.string "Exec format error"
	.string "EBADF"
	.string "Bad file number"
	.string "ECHILD"
	.string "No child processes"
	.string "EAGAIN"
	.string "Try again"
	.string "ENOMEM"
	.string "Out of memory"
	.string "EACCES"
	.string "Permission denied"
	.string "EFAULT"
	.string "Bad address"
	.string "ENOTBLK"
	.string "Block device required"
	.string "EBUSY"
	.string "Device or resource busy"
	.string "EEXIST"
	.string "File exists"
	.string "EXDEV"
	.string "Cross-device link"
	.string "ENODEV"
	.string "ENOTDIR"
	.string "Not a directory"
	.string "EISDIR"
	.string "Is a directory"
	.string "EINVAL"
	.string "Invalid argument"
	.string "ENFILE"
	.string "File table overflow"
	.string "EMFILE"
	.string "Too many open files"
	.string "ENOTTY"
	.string "Not a typewriter"
	.string "ETXTBSY"
	.string "Text file busy"
	.string "EFBIG"
	.string "File too large"
	.string "ENOSPC"
	.string "No space left on device"
	.string "ESPIPE"
	.string "Illegal seek"
	.string "EROFS"
	.string "Read-only file system"
	.string "EMLINK"
	.string "Too many links"
	.string "EPIPE"
	.string "Broken pipe"
	.string "EDOM"
	.string "Math argument out of domain of func"
	.string "ERANGE"
	.string "Math result not representable"
	.string "EDEADLK"
	.string "Resource deadlock would occur"
	.string "ENAMETOOLONG"
	.string "File name too long"
	.string "ENOLCK"
	.string "No record locks available"
	.string "ENOSYS"
	.string "Invalid system call number"
	.string "ENOTEMPTY"
	.string "Directory not empty"
	.string "ELOOP"
	.string "Too many symbolic links encountered"
	.string "EWOULDBLOCK"
	.string "Operation would block"
	.string "ENOMSG"
	.string "No message of desired type"
	.string "EIDRM"
	.string "Identifier removed"
	.string "ECHRNG"
	.string "Channel number out of range"
	.string "EL2NSYNC"
	.string "Level 2 not synchronized"
	.string "EL3HLT"
	.string "Level 3 halted"
	.string "EL3RST"
	.string "Level 3 reset"
	.string "ELNRNG"
	.string "Link number out of range"
	.string "EUNATCH"
	.string "Protocol driver not attached"
	.string "ENOCSI"
	.string "No CSI structure available"
	.string "EL2HLT"
	.string "Level 2 halted"
	.string "EBADE"
	.string "Invalid exchange"
	.string "EBADR"
	.string "Invalid request descriptor"
	.string "EXFULL"
	.string "Exchange full"
	.string "ENOANO"
	.string "No anode"
	.string "EBADRQC"
	.string "Invalid request code"
	.string "EBADSLT"
	.string "Invalid slot"
	.string "EDEADLOCK"
	.string "EBFONT"
	.string "Bad font file format"
	.string "ENOSTR"
	.string "Device not a stream"
	.string "ENODATA"
	.string "No data available"
	.string "ETIME"
	.string "Timer expired"
	.string "ENOSR"
	.string "Out of streams resources"
	.string "ENONET"
	.string "Machine is not on the network"
	.string "ENOPKG"
	.string "Package not installed"
	.string "EREMOTE"
	.string "Object is remote"
	.string "ENOLINK"
	.string "Link has been severed"
	.string "EADV"
	.string "Advertise error"
	.string "ESRMNT"
	.string "Srmount error"
	.string "ECOMM"
	.string "Communication error on send"
	.string "EPROTO"
	.string "Protocol error"
	.string "EMULTIHOP"
	.string "Multihop attempted"
	.string "EDOTDOT"
	.string "RFS specific error"
	.string "EBADMSG"
	.string "Not a data message"
	.string "EOVERFLOW"
	.string "Value too large for defined data type"
	.string "ENOTUNIQ"
	.string "Name not unique on network"
	.string "EBADFD"
	.string "File descriptor in bad state"
	.string "EREMCHG"
	.string "Remote address changed"
	.string "ELIBACC"
	.string "Can not access a needed shared library"
	.string "ELIBBAD"
	.string "Accessing a corrupted shared library"
	.string "ELIBSCN"
	.string ".lib section in a.out corrupted"
	.string "ELIBMAX"
	.string "Attempting to link in too many shared libraries"
	.string "ELIBEXEC"
	.string "Cannot exec a shared library directly"
	.string "EILSEQ"
	.string "Illegal byte sequence"
	.string "ERESTART"
	.string "Interrupted system call should be restarted"
	.string "ESTRPIPE"
	.string "Streams pipe error"
	.string "EUSERS"
	.string "Too many users"
	.string "ENOTSOCK"
	.string "Socket operation on non-socket"
	.string "EDESTADDRREQ"
	.string "Destination address required"
	.string "EMSGSIZE"
	.string "Message too long"
	.string "EPROTOTYPE"
	.string "Protocol wrong type for socket"
	.string "ENOPROTOOPT"
	.string "Protocol not available"
	.string "EPROTONOSUPPORT"
	.string "Protocol not supported"
	.string "ESOCKTNOSUPPORT"
	.string "Socket type not supported"
	.string "EOPNOTSUPP"
	.string "Operation not supported on transport endpoint"
	.string "EPFNOSUPPORT"
	.string "Protocol family not supported"
	.string "EAFNOSUPPORT"
	.string "Address family not supported by protocol"
	.string "EADDRINUSE"
	.string "Address already in use"
	.string "EADDRNOTAVAIL"
	.string "Cannot assign requested address"
	.string "ENETDOWN"
	.string "Network is down"
	.string "ENETUNREACH"
	.string "Network is unreachable"
	.string "ENETRESET"
	.string "Network dropped connection because of reset"
	.string "ECONNABORTED"
	.string "Software caused connection abort"
	.string "ECONNRESET"
	.string "Connection reset by peer"
	.string "ENOBUFS"
	.string "No buffer space available"
	.string "EISCONN"
	.string "Transport endpoint is already connected"
	.string "ENOTCONN"
	.string "Transport endpoint is not connected"
	.string "ESHUTDOWN"
	.string "Cannot send after transport endpoint shutdown"
	.string "ETOOMANYREFS"
	.string "Too many references: cannot splice"
	.string "ETIMEDOUT"
	.string "Connection timed out"
	.string "ECONNREFUSED"
	.string "Connection refused"
	.string "EHOSTDOWN"
	.string "Host is down"
	.string "EHOSTUNREACH"
	.string "No route to host"
	.string "EALREADY"
	.string "Operation already in progress"
	.string "EINPROGRESS"
	.string "Operation now in progress"
	.string "ESTALE"
	.string "Stale file handle"
	.string "EUCLEAN"
	.string "Structure needs cleaning"
	.string "ENOTNAM"
	.string "Not a XENIX named type file"
	.string "ENAVAIL"
	.string "No XENIX semaphores available"
	.string "EISNAM"
	.string "Is a named type file"
	.string "EREMOTEIO"
	.string "Remote I/O error"
	.string "EDQUOT"
	.string "Quota exceeded"
	.string "ENOMEDIUM"
	.string "No medium found"
	.string "EMEDIUMTYPE"
	.string "Wrong medium type"
	.string "ECANCELED"
	.string "Operation Canceled"
	.string "ENOKEY"
	.string "Required key not available"
	.string "EKEYEXPIRED"
	.string "Key has expired"
	.string "EKEYREVOKED"
	.string "Key has been revoked"
	.string "EKEYREJECTED"
	.string "Key was rejected by service"
	.string "EOWNERDEAD"
	.string "Owner died"
	.string "ENOTRECOVERABLE"
	.string "State not recoverable"
	.string "ERFKILL"
	.string "Operation not possible due to RF-kill"
	.string "EHWPOISON"
	.string "Memory page has hardware error"
	.string "SIGHUP"
	.string "Hangup (POSIX)"
	.string "SIGINT"
	.string "Interrupt (ANSI)"
	.string "SIGQUIT"
	.string "Quit (POSIX)"
	.string "SIGILL"
	.string "Illegal instruction (ANSI)"
	.string "SIGTRAP"
	.string "Trace trap (POSIX)"
	.string "SIGABRT"
	.string "Abort (ANSI)"
	.string "SIGBUS"
	.string "BUS error (4.2 BSD)"
	.string "SIGFPE"
	.string "Floating-point exception (ANSI)"
	.string "SIGKILL"
	.string "Kill, unblockable (POSIX)"
	.string "SIGUSR1"
	.string "User-defined signal 1 (POSIX)"
	.string "SIGSEGV"
	.string "Segmentation violation (ANSI)"
	.string "SIGUSR2"
	.string "User-defined signal 2 (POSIX)"
	.string "SIGPIPE"
	.string "Broken pipe (POSIX)"
	.string "SIGALRM"
	.string "Alarm clock (POSIX)"
	.string "SIGTERM"
	.string "Termination (ANSI)"
	.string "SIGSTKFLT"
	.string "Stack fault"
	.string "SIGCHLD"
	.string "Child status has changed (POSIX)"
	.string "SIGCONT"
	.string "Continue (POSIX)"
	.string "SIGSTOP"
	.string "Stop, unblockable (POSIX)"
	.string "SIGTSTP"
	.string "Keyboard stop (POSIX)"
	.string "SIGTTIN"
	.string "Background read from tty (POSIX)"
	.string "SIGTTOU"
	.string "Background write to tty (POSIX)"
	.string "SIGURG"
	.string "Urgent condition on socket (4.2 BSD)"
	.string "SIGXCPU"
	.string "CPU limit exceeded (4.2 BSD)"
	.string "SIGXFSZ"
	.string "File size limit exceeded (4.2 BSD)"
	.string "SIGVTALRM"
	.string "Virtual alarm clock (4.2 BSD)"
	.string "SIGPROF"
	.string "Profiling alarm clock (4.2 BSD)"
	.string "SIGWINCH"
	.string "Window size change (4.3 BSD, Sun)"
	.string "SIGIO"
	.string "I/O now possible (4.2 BSD)"
	.string "SIGPWR"
	.string "Power failure restart (System V)"
	.string "SIGSYS"
	.string "Bad system call"
.skip USHARE_STRERROR_FSIZE - (. - ushare_strerror)
SYMEND(ushare_strerror)
//[[[end]]]

