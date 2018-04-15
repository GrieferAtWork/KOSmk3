#ifndef _ASM_GENERIC_IOCTLS_H
#define _ASM_GENERIC_IOCTLS_H

/* DISCLAIMER: _STRONGLY_ Based on '/usr/include/asm-generic/ioctls.h'
 * NOTE: Do not add a license header to this file! */

#include <linux/ioctl.h>

#define TCGETS             _IO('T',0x01)
#define TCSETS             _IO('T',0x02)
#define TCSETSW            _IO('T',0x03)
#define TCSETSF            _IO('T',0x04)
#define TCGETA             _IO('T',0x05)
#define TCSETA             _IO('T',0x06)
#define TCSETAW            _IO('T',0x07)
#define TCSETAF            _IO('T',0x08)
#define TCSBRK             _IO('T',0x09)
#define TCXONC             _IO('T',0x0a)
#define TCFLSH             _IO('T',0x0b)

#define TIOCEXCL           _IO('T',0x0c)
#define TIOCNXCL           _IO('T',0x0d)
#define TIOCSCTTY          _IO('T',0x0e)
#define TIOCGPGRP          _IO('T',0x0f)
#define TIOCSPGRP          _IO('T',0x10)
#define TIOCOUTQ           _IO('T',0x11)
#define TIOCSTI            _IO('T',0x12)
#define TIOCGWINSZ         _IO('T',0x13)
#define TIOCSWINSZ         _IO('T',0x14)
#define TIOCMGET           _IO('T',0x15)
#define TIOCMBIS           _IO('T',0x16)
#define TIOCMBIC           _IO('T',0x17)
#define TIOCMSET           _IO('T',0x18)

#define TIOCGSOFTCAR       _IO('T',0x19)
#define TIOCSSOFTCAR       _IO('T',0x1a)
#define FIONREAD           _IO('T',0x1b)
#define TIOCINQ            FIONREAD
#define TIOCLINUX          _IO('T',0x1c)
#define TIOCCONS           _IO('T',0x1d)
#define TIOCGSERIAL        _IO('T',0x1e)
#define TIOCSSERIAL        _IO('T',0x1f)
#define TIOCPKT            _IO('T',0x20)
#define FIONBIO            _IO('T',0x21)
#define TIOCNOTTY          _IO('T',0x22)
#define TIOCSETD           _IO('T',0x23)
#define TIOCGETD           _IO('T',0x24)
#define TCSBRKP            _IO('T',0x25) /* Needed for POSIX tcsendbreak() */
#define TIOCSBRK           _IO('T',0x27) /* BSD compatibility */
#define TIOCCBRK           _IO('T',0x28) /* BSD compatibility */
#define TIOCGSID           _IO('T',0x29) /* Return the session ID of FD */
#define TCGETS2            _IOR('T',0x2a,struct termios2)
#define TCSETS2            _IOW('T',0x2b,struct termios2)
#define TCSETSW2           _IOW('T',0x2c,struct termios2)
#define TCSETSF2           _IOW('T',0x2d,struct termios2)
#define TIOCGRS485         _IO('T',0x2e)
#ifndef TIOCSRS485
#define TIOCSRS485         _IO('T',0x2f)
#endif
#define TIOCGPTN           _IOR('T',0x30,unsigned int) /* Get Pty Number (of pty-mux device) */
#define TIOCSPTLCK         _IOW('T',0x31,int)  /* Lock/unlock Pty */
#define TIOCGDEV           _IOR('T',0x32,unsigned int) /* Get primary device node of /dev/console */
#define TCGETX             _IO('T',0x32) /* SYS5 TCGETX compatibility */
#define TCSETX             _IO('T',0x33)
#define TCSETXF            _IO('T',0x34)
#define TCSETXW            _IO('T',0x35)
#define TIOCSIG            _IOW('T',0x36,int) /* pty: generate signal */
#define TIOCVHANGUP        _IO('T',0x37)
#define TIOCGPKT           _IOR('T',0x38,int) /* Get packet mode state */
#define TIOCGPTLCK         _IOR('T',0x39,int) /* Get Pty lock state */
#define TIOCGEXCL          _IOR('T',0x40,int) /* Get exclusive mode state */

#define FIONCLEX           _IO('T',0x50)
#define FIOCLEX            _IO('T',0x51)
#define FIOASYNC           _IO('T',0x52)
#define TIOCSERCONFIG      _IO('T',0x53)
#define TIOCSERGWILD       _IO('T',0x54)
#define TIOCSERSWILD       _IO('T',0x55)
#define TIOCGLCKTRMIOS     _IO('T',0x56)
#define TIOCSLCKTRMIOS     _IO('T',0x57)
#define TIOCSERGSTRUCT     _IO('T',0x58) /* For debugging only */
#define TIOCSERGETLSR      _IO('T',0x59) /* Get line status register */
#define TIOCSERGETMULTI    _IO('T',0x5a) /* Get multiport config  */
#define TIOCSERSETMULTI    _IO('T',0x5b) /* Set multiport config */

#define TIOCMIWAIT         _IO('T',0x5c) /* wait for a change on serial input line(s) */
#define TIOCGICOUNT        _IO('T',0x5d) /* read serial port inline interrupt counts */

#ifndef FIOQSIZE
#define FIOQSIZE           _IO('T',0x60)
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
