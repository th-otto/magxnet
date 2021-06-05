/* soft link routines */

#include <mintbind.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stat.h>
#include "lib.h"


/*
 * If MiNT 0.9 or later is active, use the kernel routines for these;
 * otherwise, try to choose failure modes that applications will best be
 * able to handle
 */

int symlink(old, new)
const char *old,
*new;
{
	char linkname[PATH_MAX];
	char path[PATH_MAX];
	long r;

	_unx2dos(old, linkname, sizeof(linkname));
	_unx2dos(new, path, sizeof(path));
	r = Fsymlink(linkname, path);
	if (r)
	{
		struct stat sb;

		if ((r == -EPATH))
		{
			if (_enoent(path))
				r = -ENOENT;
		} else if ((r == -EACCESS) && (!Fxattr(1, path, &sb)))
			r = -EEXIST;
		errno = (int) -r;
		return -1;
	}
	return (int) r;
}

int readlink(unxname, buf, siz)
char *unxname,
*buf;
int siz;
{
	long r;
	size_t l;
	char filename[PATH_MAX];
	char linkto[PATH_MAX + 1];

	_unx2dos(unxname, filename, sizeof(filename));
	r = Freadlink(PATH_MAX, linkto, filename);
	if (r < 0)
	{
		if (r == -EACCES)
		{
			struct stat sb;

			/* UNIX gives EINVAL, not EACCES, on non-links */
			if ((Fxattr(1, filename, &sb) == 0) && ((sb.st_mode & S_IFMT) != S_IFLNK))
			{
				r = -EINVAL;
			}
		}
		if ((r == -EPATH) && _enoent(filename))
		{
			r = -ENOENT;
		}
		errno = (int) -r;
		return -1;
	}
	linkto[PATH_MAX] = 0;
	_dos2unx(linkto, filename, sizeof(filename));
	l = strlen(filename);
	if (l > siz)
	{
		errno = ERANGE;
		return -1;
	}
	strncpy(buf, filename, siz);
	return (int) l;
}
