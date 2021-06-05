/* based upon Dale Schumacher's dLibs library */
/* extensively modified by ers */

#include <compiler.h>
#include <osbind.h>
#include <mintbind.h>
#include <limits.h>
#include <fcntl.h>
#include <ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stat.h>
#include <device.h>
#include "lib.h"
#ifndef PIPE_RDEV
#define PIPE_RDEV 0x7e00
#endif

/*
 * the MiNT kernel uses 0x08 for O_APPEND. For
 * backwards compatibility with old .o files,
 * we leave the definition in fcntl.h the same,
 * but adjust the file masks here.
 */

static int __umask = -1;
extern int __mint;

static void _get_umask __PROTO((void));

/*
 * function to set the initial value of __umask
 */

static void _get_umask()
{
	if (__mint < 9)
	{
		__umask = 0;
	} else
	{
		__umask = Pumask(0);
		(void) Pumask(__umask);
	}
}

/* NOTE: we assume here that every compiler that can handle __PROTO
 *       is __STDC__ and can handle the >>...<< ellipsis
 *       (see also unistd.h)
 */

int open(const char *_filename, int iomode, ...)
{
	int rv;
	int modemask;						/* which bits get passed to the OS? */
	char filename[PATH_MAX];
	long fcbuf;							/* a temporary buffer for Fcntl */
	struct stat sb;

	unsigned pmode;
	va_list argp;

	va_start(argp, iomode);
	pmode = va_arg(argp, unsigned int);

	va_end(argp);

	_unx2dos(_filename, filename, sizeof(filename));

/* use the umask() setting to get the right permissions */
	if (__umask == -1)
		_get_umask();
	pmode &= ~__umask;

/* set the file access modes correctly */
	iomode = iomode & ~O_SHMODE;

	if (__mint >= 9)
	{
		modemask = O_ACCMODE | O_SHMODE | O_SYNC | O_NDELAY | O_CREAT | O_TRUNC | O_EXCL;
		iomode |= O_DENYNONE;
/*
		if (__mint >= 96)
*/
		{
			modemask |= _REALO_APPEND;
			if (iomode & O_APPEND)
				iomode |= _REALO_APPEND;
		}
		rv = (int)Fxattr(0, filename, &sb);
	} else
	{
		modemask = O_ACCMODE;
		rv = Fattrib(filename, 0, 0);
	}

	if (rv >= 0)						/* file exists */
	{
		if ((iomode & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
		{
			errno = EEXIST;
			return __SMALLEST_VALID_HANDLE - 1;
		}
		rv = (int) Fopen(filename, iomode & modemask & ~O_CREAT);
		if (rv == -ENOENT)
			/* race: file can disappear between stat and open... */
			goto noent;
		if (rv >= 0 && __mint >= 9 && (	/* __mint <= 0x109 ?
										   S_ISFIFO(sb.st_mode) : */
										  (major(sb.st_rdev) == major(PIPE_RDEV)))
			&& ((iomode & O_WRONLY) ? Foutstat(rv) : Finstat(rv)) < 0)
		{
			/* /pipe/file still open but noone at other end */
			(void) Fclose(rv);
			errno = ENXIO;
			return __SMALLEST_VALID_HANDLE - 1;
		}
		if ((iomode & ~modemask & O_TRUNC) && (rv >= 0))
		{
			/* Give up if the mode flags conflict */
			if ((iomode & O_ACCMODE) == O_RDONLY)
			{
				(void) Fclose(rv);
				errno = EACCES;
				return __SMALLEST_VALID_HANDLE - 1;
			}
			/* Try the FTRUNCATE first.  If it fails, have GEMDOS
			   truncate it, then reopen with the correct modes.
			 */
			fcbuf = 0L;
			if ((__mint <= 90) || (Fcntl(rv, (long) &fcbuf, FTRUNCATE) < 0))
			{
				(void) Fclose(rv);
				rv = (int) Fcreate(filename, 0x00);
				if (rv < 0)
				{
					errno = -rv;
					return __SMALLEST_VALID_HANDLE - 1;
				}
				(void) Fclose(rv);
				rv = (int) Fopen(filename, iomode & modemask);
			}
		}
	} else								/* file doesn't exist */
	{
		char linkf[40];
		long l;

	  noent:
		if (iomode & O_CREAT)
		{
			if (__mint >= 9)
				rv = (int) Fopen(filename, iomode & modemask);
			else
			{
				rv = (int) Fcreate(filename, 0x00);
				if (rv >= 0)
				{
					sb.st_mode = 0;
					if (fstat(rv, &sb) != 0 || !S_ISFIFO(sb.st_mode))
					{
						(void) Fclose(rv);
						rv = (int) Fopen(filename, iomode & modemask);
					}
				}
			}
			if (rv >= 0 && __mint >= 9)
				(void) Fchmod(filename, pmode);
		}
		/* difference between MiNT and unix:  unix can create named pipes
		   (fifos) and ptys anywhere in the filesystem with mknod/mkfifo
		   and open both ends with standard open(), without O_CREAT.
		   MiNT keeps all this in /pipe, creating and first open
		   have to be done thru Fcreate with mode flags encoded as dos
		   `attributes' and the kernel doesn't know ENXIO. also ptys
		   use the same filename for both sides and everything in /pipe
		   unlinks itself at the last close.

		   the idea is when open would return ENOENT or EACCESS on
		   creating see if its a symlink to /pipe/[cn]$* and then try
		   the right Fcreate/chmod here...  so mknod/mkfifo can create
		   a symlink and pass the mknod info in the link:
		   /pipe/c$ pmode Fcreate-mode [flag] id
		   /pipe/n$ pmode [flag] long-id    (Fcreate-mode == pipe)

		   pmode is octal, Fcreate-mode is 2 nibbles + '@', flag is '-'
		   for pty server side (/dev/ptyp*) or '$' for pty client (ttyp*,
		   open server side Fcreates this).  the ids only have to be uniqe
		   and long-id not start with an octal digit...
		 */
		if (__mint >= 9 &&
			rv == ((iomode & O_CREAT) ? -EACCESS : -ENOENT) &&
			Freadlink((int)sizeof(linkf) - 1, linkf, filename) >= 0 &&
			!strncmp(linkf, "u:\\pipe\\", 8) &&
			linkf[9] == '$' && (linkf[8] == 'c' || linkf[8] == 'n') && linkf[10] >= '0' && linkf[10] <= '7')
		{
			char *p,
			*q;

			pmode = (unsigned)strtoul(linkf + 10, &q, 8);
			p = q;
			if (linkf[8] == 'n')
				l = (FA_HIDDEN | 0x80);
			else
			{
				l = (*p++ & 0xf) << 4;
				l |= *p++ & 0xf;
			}
			if (((l & 0x80) && ((iomode & (O_ACCMODE | O_NDELAY)) ==
								(O_WRONLY | O_NDELAY))) || ((l & FA_SYSTEM) && *p == '$'))
			{
				errno = ENXIO;
				return __SMALLEST_VALID_HANDLE - 1;
			}
			if ((iomode & O_ACCMODE) == O_WRONLY)
			{
				l |= FA_RDONLY;
			}
			if (*p == '-')
				*p = '$';
			if (!(l & ~0x80))
			{
				rv = (int) Fopen(filename, (iomode & modemask) | O_CREAT | O_EXCL);
			} else
			{
				rv = (int) Fcreate(linkf, (int)(l & ~0x80));
				if (rv >= 0 && (iomode & O_NDELAY))
					Fcntl(rv, iomode & modemask, F_SETFL);
			}
			if (rv >= 0)
			{
				if (!Fchmod(linkf, pmode) && !Fxattr(1, filename, &sb))
					(void) Fchown(linkf, sb.st_uid, sb.st_gid);
			}
		}
	}

	if (rv < (__SMALLEST_VALID_HANDLE))
	{
		if ((rv == -EPATH) && (_enoent(filename)))
			rv = -ENOENT;
		errno = -rv;
		return __SMALLEST_VALID_HANDLE - 1;
	}
	if (__mint)
	{
		/* Relocate the handle to the lowest positive numbered handle
		   available
		 */
		fcbuf = Fcntl(rv, (long) 0, F_DUPFD);
		if (fcbuf >= 0)
		{
			if (fcbuf < rv)
			{
				(void) Fclose(rv);
				rv = (int) fcbuf;
			} else
			{
				(void) Fclose((int) fcbuf);
			}
		}
		/* clear the close-on-exec flag */
		fcbuf = (long) Fcntl(rv, (long) 0, F_GETFD);
		if (fcbuf & FD_CLOEXEC)
			(void) Fcntl(rv, fcbuf & ~FD_CLOEXEC, F_SETFD);
	}
	if ((iomode & O_APPEND) && !(modemask & _REALO_APPEND))
		(void) Fseek(0L, rv, SEEK_END);

	/* fix the case `isatty() called before and not closed thru close()' */
	if (__OPEN_INDEX(rv) < __NHANDLES)
		__open_stat[__OPEN_INDEX(rv)].status = FH_UNKNOWN;
	/* Important side effect:  isatty(rv) sets up flags under TOS */
	if (isatty(rv) && (!(iomode & O_NOCTTY)) && (!(isatty(-1))))
	{
		/* If the process is a session leader with no controlling tty,
		   and the tty that was opened is not already the controlling
		   tty of another process, the tty becomes the controlling tty
		   of the process.  Note that MiNT has no concept of a session
		   leader so we really only check that it is a process group
		   leader.
		 */
		if ((!__mint) || ((Pgetpgrp() == Pgetpid()) && (Fcntl(rv, &fcbuf, TIOCGPGRP) >= 0) && (fcbuf == 0)))
		{
			(void) Fforce(-1, rv);		/* new controlling tty */
			__open_stat[__OPEN_INDEX(-1)] = __open_stat[__OPEN_INDEX(rv)];
		}
	}

	return (rv);
}

int creat(name, mode)
const char *name;
unsigned mode;
{
	return open(name, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

/* umask -- change default file creation mask */

int umask(complmode)
int complmode;
{
	int old_umask;

	if (__umask == -1)
		_get_umask();
	old_umask = __umask;
	__umask = complmode;
	if (__mint >= 9)
		return Pumask(complmode);
	return old_umask;
}
