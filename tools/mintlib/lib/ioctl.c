/*
 * ioctl() emulation for MiNT; written by Eric R. Smith and placed
 * in the public domain
 */

#include <errno.h>
#include <mintbind.h>
#include <ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linea.h>						/* for TIOCGWINSZ under TOS */
#include <support.h>
#include <stat.h>
#include <ssystem.h>
#include <types.h>
#include "lib.h"						/* for __open_stat */

extern int __mint;						/* MiNT version */
int _ttydisc = NTTYDISC;
int _ldisc = LLITOUT;

/* in read.c */
extern struct tchars __tchars;
extern struct ltchars __ltchars;

int ioctl(fd, cmd, arg)
int fd,
	cmd;
void *arg;
{
	long r;
	int istty = isatty(fd);
	struct sgttyb *sg = (struct sgttyb *) arg;
	int null_fd;
	long baud;

	if (istty)
	{
		switch (cmd)
		{
		case TIOCGETD:
			*((int *) arg) = _ttydisc;
			return 0;
		case TIOCSETD:
			_ttydisc = *((int *) arg);
			return 0;
		case TIOCLGET:
			*((int *) arg) = _ldisc;
			return 0;
		case TIOCLSET:
			_ldisc = *((int *) arg);
			return 0;
		case TIOCLBIS:
			_ldisc |= *((int *) arg);
			return 0;
		case TIOCLBIC:
			_ldisc &= ~(*((int *) arg));
			return 0;
		case TIOCSWINSZ:
			if (__mint < 9)
				return 0;
			break;
		case TIOCGWINSZ:
			if (__mint < 9)
			{
				struct winsize *win = (struct winsize *) arg;

				linea0();
				win->ws_row = V_CEL_MY + 1;
				win->ws_col = V_CEL_MX + 1;
				win->ws_xpixel = V_X_MAX;
				win->ws_ypixel = V_Y_MAX;
				return 0;
			}
			break;
#ifdef __MINT__
		case TIOCNOTTY:
			if (__mint)
			{
				if ((fd < 0) || !(_isctty(fd)))
				{
					errno = EBADF;
					return -1;
				}
				(void) Fclose(fd);
				null_fd = (int) Fopen(	/* __mint < 9 ? "V:\\null"
										   : */ "U:\\dev\\null", O_RDWR);
				(void) Fforce(-1, null_fd);
				__open_stat[__OPEN_INDEX(-1)].status = FH_UNKNOWN;
				__open_stat[__OPEN_INDEX(fd)].status = FH_UNKNOWN;
				if (null_fd != fd)
				{
					(void) Fforce(fd, null_fd);
					(void) Fclose(null_fd);
				}
				return 0;
			}
			break;
#endif /* __MINT__ */
		default:
			break;
		}
	}

	if (__mint)
	{
		switch (cmd)
		{
		case TIOCCDTR:
			baud = 0;
			r = Fcntl(fd, &baud, TIOCOBAUD);
			if (r < 0)
			{
				errno = (int) -r;
				return -1;
			}
			return 0;
		case TIOCSDTR:
			baud = -1;
			r = Fcntl(fd, &baud, TIOCOBAUD);
			if (r < 0)
			{
				errno = (int) -r;
				return -1;
			}
			r = Fcntl(fd, &baud, TIOCOBAUD);
			if (r < 0)
			{
				errno = (int) -r;
				return -1;
			}
			return 0;
		case TIOCMGET:
/*
	      if (__mint >= 0x10a)
*/
			{
				char g;

#ifdef __LATTICE__
				void *ssp;
#else
				long ssp;
#endif
				short *mfp;
				short m;
				struct stat sb;
				long *msig;

				msig = (long *) arg;
				r = Fcntl(fd, (long) &sb, FSTAT);
				if (r < 0)
				{
					errno = (int) -r;
					return -1;
				}
				if (((sb.st_mode & S_IFMT) == S_IFCHR) && (sb.st_rdev == 257))
				{
					*msig = TIOCM_DSR;
					g = Giaccess(0, 14);
					*msig |= ((g & (1 << 3)) ? 0 : TIOCM_RTS);
					*msig |= ((g & (1 << 4)) ? 0 : TIOCM_DTR);
					mfp = ((short *) 0xfffffa00L);
					if (Ssystem(-1, NULL, NULL))
					{
						ssp = Super(0L);
						m = *mfp & 0xff;
						Super((void *)ssp);
					} else
						m = (short) (Ssystem(TIOCMGET, (unsigned long) mfp, NULL));
					*msig |= ((m & (1 << 1)) ? 0 : TIOCM_CAR);
					*msig |= ((m & (1 << 2)) ? 0 : TIOCM_CTS);
					*msig |= ((m & (1 << 6)) ? 0 : TIOCM_RNG);
					return 0;
				}
				errno = ENOSYS;
				return -1;
			}
		case TIOCSETP:
/*
	      if (__mint <= 0x10a) {
		r = Fcntl(fd, arg, cmd);
		if (r != -ENOSYS)
		  break;
		cmd = TIOCSETN;
	      }
*/
		 /*FALLTHRU*/ default:
			r = Fcntl(fd, arg, cmd);
			break;
		}
	} else if (istty)
	{
		r = 0;
		switch (cmd)
		{
		case TIOCSETP:
			fd = __OPEN_INDEX(fd);
			if (fd < 0 || fd >= __NHANDLES)
				fd = __NHANDLES - 1;
			__open_stat[fd].flags = sg->sg_flags;
			break;
		case TIOCGETP:
			fd = __OPEN_INDEX(fd);
			if (fd < 0 || fd >= __NHANDLES)
				fd = __NHANDLES - 1;
			sg->sg_flags = __open_stat[fd].flags;
			sg->sg_ispeed = sg->sg_ospeed = 0;
			sg->sg_erase = 'H' & 0x1f;
			sg->sg_kill = 'U' & 0x1f;
			break;
		case TIOCGETC:
			*((struct tchars *) arg) = __tchars;
			break;
		case TIOCSETC:
			__tchars = *((struct tchars *) arg);
			break;
		case TIOCGLTC:
			*((struct ltchars *) arg) = __ltchars;
			break;
		case TIOCSLTC:
			__ltchars = *((struct ltchars *) arg);
			break;
		case TIOCGPGRP:
			*((long *) arg) = 0;
			break;
		case TIOCSPGRP:
			break;
		default:
			r = -ENOSYS;
		}
	} else
		r = -ENOSYS;

	if (r < 0)
	{
		errno = (int) -r;
		return -1;
	}
	return (int) r;
}
