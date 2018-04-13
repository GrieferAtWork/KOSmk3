#ifndef _ASM_GENERIC_IOCTLS_H
#define _ASM_GENERIC_IOCTLS_H

/* DISCLAIMER: _STRONGLY_ Based on '/usr/include/asm-generic/ioctls.h'
 * NOTE: Do not add a license header to this file! */

#include <linux/ioctl.h>

#define TCGETS             0x5401
#define TCSETS             0x5402
#define TCSETSW            0x5403
#define TCSETSF            0x5404
#define TCGETA             0x5405
#define TCSETA             0x5406
#define TCSETAW            0x5407
#define TCSETAF            0x5408
#define TCSBRK             0x5409
#define TCXONC             0x540a
#define TCFLSH             0x540b

#define TIOCEXCL           0x540c
#define TIOCNXCL           0x540d
#define TIOCSCTTY          0x540e
#define TIOCGPGRP          0x540f
#define TIOCSPGRP          0x5410
#define TIOCOUTQ           0x5411
#define TIOCSTI            0x5412
#define TIOCGWINSZ         0x5413
#define TIOCSWINSZ         0x5414
#define TIOCMGET           0x5415
#define TIOCMBIS           0x5416
#define TIOCMBIC           0x5417
#define TIOCMSET           0x5418

#define TIOCGSOFTCAR       0x5419
#define TIOCSSOFTCAR       0x541a
#define FIONREAD           0x541b
#define TIOCINQ            FIONREAD
#define TIOCLINUX          0x541c
#define TIOCCONS           0x541d
#define TIOCGSERIAL        0x541e
#define TIOCSSERIAL        0x541f
#define TIOCPKT            0x5420
#define FIONBIO            0x5421
#define TIOCNOTTY          0x5422
#define TIOCSETD           0x5423
#define TIOCGETD           0x5424
#define TCSBRKP            0x5425 /* Needed for POSIX tcsendbreak() */
#define TIOCSBRK           0x5427 /* BSD compatibility */
#define TIOCCBRK           0x5428 /* BSD compatibility */
#define TIOCGSID           0x5429 /* Return the session ID of FD */
#define TCGETS2            _IOR('T',0x2a,struct termios2)
#define TCSETS2            _IOW('T',0x2b,struct termios2)
#define TCSETSW2           _IOW('T',0x2c,struct termios2)
#define TCSETSF2           _IOW('T',0x2d,struct termios2)
#define TIOCGRS485         0x542e
#ifndef TIOCSRS485
#define TIOCSRS485         0x542f
#endif
#define TIOCGPTN           _IOR('T',0x30,unsigned int) /* Get Pty Number (of pty-mux device) */
#define TIOCSPTLCK         _IOW('T',0x31,int)  /* Lock/unlock Pty */
#define TIOCGDEV           _IOR('T',0x32,unsigned int) /* Get primary device node of /dev/console */
#define TCGETX             0x5432 /* SYS5 TCGETX compatibility */
#define TCSETX             0x5433
#define TCSETXF            0x5434
#define TCSETXW            0x5435
#define TIOCSIG            _IOW('T',0x36,int) /* pty: generate signal */
#define TIOCVHANGUP        0x5437
#define TIOCGPKT           _IOR('T',0x38,int) /* Get packet mode state */
#define TIOCGPTLCK         _IOR('T',0x39,int) /* Get Pty lock state */
#define TIOCGEXCL          _IOR('T',0x40,int) /* Get exclusive mode state */

#define FIONCLEX           0x5450
#define FIOCLEX            0x5451
#define FIOASYNC           0x5452
#define TIOCSERCONFIG      0x5453
#define TIOCSERGWILD       0x5454
#define TIOCSERSWILD       0x5455
#define TIOCGLCKTRMIOS     0x5456
#define TIOCSLCKTRMIOS     0x5457
#define TIOCSERGSTRUCT     0x5458 /* For debugging only */
#define TIOCSERGETLSR      0x5459 /* Get line status register */
#define TIOCSERGETMULTI    0x545a /* Get multiport config  */
#define TIOCSERSETMULTI    0x545b /* Set multiport config */

#define TIOCMIWAIT         0x545c /* wait for a change on serial input line(s) */
#define TIOCGICOUNT        0x545d /* read serial port inline interrupt counts */

#ifndef FIOQSIZE
#define FIOQSIZE           0x5460
#endif

/* Used for packet mode */
#define TIOCPKT_DATA       0x0000
#define TIOCPKT_FLUSHREAD  0x0001
#define TIOCPKT_FLUSHWRITE 0x0002
#define TIOCPKT_STOP       0x0004
#define TIOCPKT_START      0x0008
#define TIOCPKT_NOSTOP     0x0010
#define TIOCPKT_DOSTOP     0x0020
#define TIOCPKT_IOCTL      0x0040

#define TIOCSER_TEMT       0x01 /* Transmitter physically empty */

#endif /* !_ASM_GENERIC_IOCTLS_H */
