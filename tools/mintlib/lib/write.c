/*
 * _write: like write, but takes a long instead of an int. Written by
 * Eric R. Smith and placed in the public domain.
 */

/* BUG: under TOS, CRMOD doesn't work unless RAW is on or SIGINT is
 * being caught
 */

#include <osbind.h>
#include <fcntl.h>
#include <ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stat.h>
#include <mintbind.h>
#include "lib.h"

extern __Sigfunc _sig_handler[];		/* TOS fakes for signal handling */

long _write(fd, buf, size)
int fd;
const void *buf;
unsigned long size;
{
	unsigned char c,
	*foo;
	unsigned flags;
	long r;
	extern int __mint;
	struct stat statbuf;
	_DOSTIME timebuf;

	if (__mint == 0 && isatty(fd))
	{
		r = __OPEN_INDEX(fd);
		if (r < 0 || r >= __NHANDLES)
			r = __NHANDLES - 1;
		flags = __open_stat[r].flags;
		if ((flags & RAW) || _sig_handler[SIGINT] != SIG_DFL)
		{
			foo = (unsigned char *) buf;
			r = size;
			while (r-- > 0)
			{
				c = *foo++;
				if (c == '\n' && (flags & CRMOD))
					_console_write_byte(fd, '\r');
				_console_write_byte(fd, c);
			}
			return size;
		}
	}

	r = Fwrite(fd, size, buf);
	if (r < 0)
	{
		errno = (int) -r;
		return -1;
	}
	if (size && r == 0)
	{
		if (__mint >= 9)
		{
			if ((r = Fcntl(fd, &statbuf, FSTAT)) < 0)
			{
				errno = (int) -r;
				return -1;
			}
			if ((statbuf.st_mode & S_IFMT) == S_IFREG)
			{
				errno = ENOSPC;
				return -1;
			}
		} else if (Fdatime(&timebuf, fd, 0) == 0 && Fseek(0L, fd, SEEK_CUR) >= 0)
		{
			errno = ENOSPC;
			return -1;
		}
	}

	return r;
}

#if defined (__GNUC__) && !defined (__MSHORT__)
__asm__(".stabs \"_write\",5,0,0,__write");
#else
int write(fd, buf, size)
int fd;
const void *buf;
unsigned size;
{
	return (int) _write(fd, buf, (unsigned long) size);
}
#endif
