/*
 * _read: like read, but takes a long instead of an int. Written by
 * Eric R. Smith and placed in the public domain.
 */

#include <fcntl.h>
#include <osbind.h>
#include <errno.h>
#include <ioctl.h>
#include <signal.h>
#include <unistd.h>
#include <support.h>
#include <mintbind.h>
#include "lib.h"

#define CTRL(x) (x & 0x1f)
#define UNDEF -1

struct tchars __tchars = {
	CTRL('C'),							/* interrupt */
	CTRL('\\'),							/* quit */
	CTRL('Q'),							/* start */
	CTRL('S'),							/* stop */
	CTRL('D'),							/* EOF */
	'\r'								/* alternate end of line */
};

struct ltchars __ltchars = {
	CTRL('Z'),							/* suspend */
	CTRL('Y'),							/* suspend after read */
	CTRL('R'),							/* reprint */
	UNDEF,								/* flush output */
	UNDEF,								/* erase word */
	UNDEF								/* quote next char */
};

/*
 * BUGS: in tos, turning ECHO off but leaving RAW and CBREAK alone doesn't
 * work properly
 */

long _read(fd, buf, size)
int fd;
void *buf;
unsigned long size;
{
	char *foo;
	long r;
	extern int __mint;
	int indx;
	int flags;

#ifdef EIO
	long tty_pgrp;
	long omask;
	__Sigfunc osigt;
#endif
#ifdef EAGAIN
	long wasready;
#endif

	if (isatty(fd))
	{
/* work around a bug in TOS; 4096 bytes should be plenty for terminal reads */
		if (size > 4096)
			size = 4096;
		indx = __OPEN_INDEX(fd);
		if (indx < 0 || indx >= __NHANDLES)
			indx = __NHANDLES - 1;
		flags = __open_stat[indx].flags;
	} else
		flags = -1;

	if ((__mint > 0) || (flags == -1) || (((flags & (RAW | CBREAK | ECHO)) == ECHO)))
	{
#ifdef EIO
		if (__mint && _isctty(fd))
		{
			(void) Fcntl(fd, (long) &tty_pgrp, TIOCGPGRP);
			if (tty_pgrp != Pgetpgrp())
			{
#if 0
				/* This isn't really what we mean here...we really want to
				   know if our process group has no controlling terminal.
				 */
				if (fd == -1 && __open_stat[indx] == FH_ISAFILE)
				{
					errno = EIO;
					return -1;
				}
#endif
				omask = Psigblock(~0L);
				osigt = (__Sigfunc) Psignal(SIGTTIN, (long) SIG_IGN);
				(void) Psignal(SIGTTIN, (long) osigt);
				(void) Psigsetmask(omask);
				if ((omask & sigmask(SIGTTIN)) || (osigt == SIG_IGN))
				{
					errno = EIO;
					return -1;
				}
			}
		}
#endif /* EIO */
#ifdef EAGAIN
		if (__mint)
		{
			(void) Fcntl(fd, &wasready, FIONREAD);
		}
#endif
		r = Fread(fd, size, buf);
#ifdef EAGAIN
		if (__mint && r == 0 && wasready == 0 && (Fcntl(fd, 0, F_GETFL) | O_NDELAY))
		{
			r = -EAGAIN;
		}
#endif
		if (r < 0)
		{
			errno = (int) -r;
			return -1;
		}

		/* watch out for TTYs */
		if (__mint == 0 && isatty(fd))
		{
			foo = (char *) buf;
			if (*foo == __tchars.t_eofc)	/* EOF character? */
				return 0;
			/* for multibyte reads terminated by a CR, we add
			   the CR since TOS doesn't put it in for us
			 */
			if ((r < size) && (foo[r - 1] != '\r'))
			{
				foo[r] = '\r';
				r++;
			}
			/* If the last char is a CR (either added above
			   or read from a single-byte Fread()) we translate
			   it according to the CRMOD setting
			 */
			if ((flags & CRMOD) && r && (foo[r - 1] == '\r'))
			{
				foo[r - 1] = '\n';
				Cconout('\n');			/* not quite right if fd != 0 */
			}
		}
	} else
	{
	  again:
		r = _console_read_byte(fd) & 0x00ff;
		if (flags & ECHO)
		{
			_console_write_byte(fd, (int) r);
		}
		if (flags & CRMOD)
		{
			if (r == '\r')
				r = '\n';
		}
		if (!(flags & RAW))
		{
			if (r == __tchars.t_intrc)
			{
				raise(SIGINT);
				goto again;
			} else if (r == __tchars.t_quitc)
			{
				raise(SIGQUIT);
				goto again;
			}
		}
		*((char *) buf) = r;
		r = 1;
	}

	return r;
}

#if defined (__GNUC__) && !defined (__MSHORT__)
__asm__(".stabs \"_read\",5,0,0,__read");
#else
int read(fd, buf, size)
int fd;
void *buf;
unsigned size;
{
	return (int) _read(fd, buf, (unsigned long) size);
}
#endif
