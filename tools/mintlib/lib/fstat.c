#include <limits.h>
#include <types.h>
#include <stat.h>
#include <ctype.h>
#include <errno.h>
#include <osbind.h>
#include <mintbind.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <support.h>
#include <ioctl.h>						/* for FSTAT */
#include "lib.h"

extern ino_t __inode;

__EXTERN int _do_stat __PROTO((const char *_path, struct stat * st, int lflag));

/* 
 * fstat: if we're not running under MiNT, this is pretty bogus.
 * what we can really find is:
 * modification time: via Fdatime()
 * file size: via Fseek()
 * fortunately, these are the things most programs are interested in.
 * BUG: passing an invalid file descriptor gets back a stat structure for
 * a tty.
 */

int fstat(fd, st)
int fd;
struct stat *st;
{
	long oldplace,
	 r;
	_DOSTIME timeptr;
	short magic;

	if ((r = Fcntl(fd, (long) st, FSTAT)) != -ENOSYS)
	{
		if (r)
		{
			errno = (int) -r;
			return -1;
		}
		__UNIXTIME(st->st_mtime);
		__UNIXTIME(st->st_atime);
		__UNIXTIME(st->st_ctime);
		st->st_blocks = (st->st_blocks * st->st_blksize) / 512;
		return 0;
	}

	r = Fdatime(&timeptr, fd, 0);
	if (r < 0)
	{									/* assume TTY */
		st->st_mode = S_IFCHR | 0600;
		st->st_attr = 0;
		st->st_mtime = st->st_ctime = st->st_atime = time((time_t *) 0) - 2;
		st->st_size = 0;
	} else
	{
		st->st_mtime = st->st_atime = st->st_ctime = _unixtime(timeptr.time, timeptr.date);
		st->st_mode = S_IFREG | 0644;	/* this may be false */
		st->st_attr = 0;				/* because this is */

		/* get current file location */
		oldplace = Fseek(0L, fd, SEEK_CUR);
		if (oldplace < 0)
		{								/* can't seek -- must be pipe */
			st->st_mode = S_IFIFO | 0644;
			st->st_size = 1024;
		} else
		{
			r = Fseek(0L, fd, SEEK_END);	/* go to end of file */
			st->st_size = r;
			(void) Fseek(0L, fd, SEEK_SET);	/* go to start of file */
			/* check for executable file */
			if (Fread(fd, 2, (char *) &magic) == 2)
			{
				if (magic == 0x601a || magic == 0x2321)
					st->st_mode |= 0111;
			}
			(void) Fseek(oldplace, fd, SEEK_SET);
		}
	}

/* all this stuff is likely bogus as well. sigh. */
	st->st_dev = Dgetdrv();
	st->st_rdev = 0;
	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_blksize = 1024;
/* note: most Unixes measure st_blocks in 512 byte units */
	st->st_blocks = (st->st_size + 511) / 512;
	st->st_ino = __inode++;
	st->st_nlink = 1;
	return 0;
}
