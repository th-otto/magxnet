/* mkdir: make a new directory
 * written by Eric R. Smith and placed in the public domain
 * modified by Alan Hourihane, to check for directory and return EEXIST.
 */

#include <errno.h>
#include <limits.h>
#include <osbind.h>
#include <mintbind.h>
#include <types.h>
#include <stat.h>
#include <unistd.h>
#include "lib.h"

extern int errno;

int mkdir(_path, mode)
const char *_path;
mode_t mode;
{
	struct stat statbuf;
	int rv,
	 umask;
	char path[PATH_MAX];

	_unx2dos(_path, path, sizeof(path));

	rv = stat(path, &statbuf);			/* Stat directory */
	if (rv == 0)
	{									/* Does it exist ? */
		errno = EEXIST;					/* Yes, so tell user. */
		return -1;
	}

	if (errno != ENOENT)
	{									/* Return stat error, if other than */
		/* File not found. */
		if ((errno == EPATH) && (_enoent(path)))
			errno = ENOENT;
		return -1;
	}

	rv = Dcreate(path);
	if (rv < 0)
	{
		errno = -rv;
		return -1;
	}

	if ((umask = Pumask(0)) != -ENOSYS)
	{
		(void) Pumask(umask);
		(void) Fchmod(path, mode & ~umask);
	}
	return 0;
}
