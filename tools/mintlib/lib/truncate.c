/*

Here is an implementation of truncate/ftruncate for the MiNTlib. There
is a special case for truncate, if the filesystem does not recognize
FTRUNCATE and the length is zero, Fcreate is used to truncate the
file. This only works for tosfs if the file isn't already open. The
rest is quite straight forward.

Andreas.

Andreas Schwab <schwab@issan.informatik.uni-dortmund.de>

*/

#include <compiler.h>
#include <limits.h>
#include <errno.h>
#include <mintbind.h>
#include <ioctl.h>
#ifdef __TURBOC__
#include <sys\types.h>
#else
#include <sys/types.h>
#endif
#include "lib.h"

int truncate(_filename, length)
const char *_filename;
off_t length;
{
	int fh,
	 res;
	char filename[PATH_MAX];

	(void) _unx2dos(_filename, filename, sizeof(filename));
	res = -EINVAL;

	res = (int) Dcntl(FTRUNCATE, filename, (long) &length);

	if (res == 0)
		return res;
	if (res != EINVAL)
	{
		if (res < 0)
		{
			if ((res == -EPATH) && (_enoent(filename)))
				res = -ENOENT;
			errno = (int) -res;
			return -1;
		}
		return 0;
	}

	fh = (int) Fopen(filename, 2);
	if (fh < 0)
	{
		if ((fh == -EPATH) && (_enoent(filename)))
			fh = -ENOENT;
		errno = -fh;
		return -1;
	}

	res = (int) Fcntl(fh, (long) &length, FTRUNCATE);
	Fclose(fh);

	if (res == -EINVAL && length == 0)
	{
		res = (int) Fcreate(filename, 0);
		if (res >= 0)
			Fclose(res);
	}
	if (res < 0)
	{
		errno = -res;
		return -1;
	}
	return 0;
}

int ftruncate(fd, length)
int fd;
off_t length;
{
	int res;

	res = (int) Fcntl(fd, (long) &length, FTRUNCATE);

	if (res < 0)
	{
		errno = -res;
		return -1;
	}
	return 0;
}
