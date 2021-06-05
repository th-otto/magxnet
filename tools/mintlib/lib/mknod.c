/* fake mknod -- this always fails, except with MinixFS 0.60pl10 or later
 * -- or for ptys in /dev and named pipes, they are `emulated' (partly)
 * using symlinks to /pipe and some kludges in open()...
 */

#include <errno.h>
#include <support.h>
#include <stat.h>
#include <limits.h>
#include <types.h>
#include <mintbind.h>
#include <device.h>
#include "lib.h"

#define MFS_MKNOD 0x10f

#ifndef PIPE_RDEV
#define PIPE_RDEV 0x7e00
#endif

int mknod(path, mode, dev)
const char *path;
int mode,
	dev;
{
	long err;
	char _path[PATH_MAX];
	extern int __mint;

	if (S_ISDIR(mode))
	{
		return (mkdir(path, (mode_t) mode));
	}
	if (S_ISFIFO(mode) || ((mode & S_IFMT) == 010000))
	{
		return (mkfifo(path, (mode_t) mode));
	}
	_unx2dos(path, _path, sizeof(_path));
	err = Dcntl(MFS_MKNOD, _path, ((long) mode & 0xffff) | ((long) dev << 16));
	if (err >= 0)
		return 0;
	if (__mint >= 9 && S_ISCHR(mode) && major(dev) == major(PIPE_RDEV))
	{
		/* create symlink to /pipe/c$ mode Fcreate-mode flag id (see open.c)
		   use minor | 0x80 for /dev/ptyp* (flag == '-') and minor & 0x7f
		   for ttyp* (flag = '$'), id is dev & ~0x80
		 */
		char linkf[30] = "u:\\pipe\\c$",
			*s;

		s = linkf + sizeof "u:\\pipe\\c$" - 1;
		*s++ = ((mode >> 6) & 7) + '0';
		*s++ = ((mode >> 3) & 7) + '0';
		*s++ = (mode & 7) + '0';
#define FMODE (FA_HIDDEN|FA_SYSTEM)
		*s++ = ((FMODE & 0xf0) >> 4) + '@';
		*s++ = (FMODE & 0xf) + '@';
		*s++ = '$';
		if (dev & 0x80)
		{
			dev &= ~0x80;
			s[-1] = '-';
		}
		_ultoa(dev, s, 16);

		err = Fsymlink(linkf, _path);
		if (err)
		{
			struct stat sb;

			if ((err == -EPATH))
			{
				if (_enoent(_path))
					err = -ENOENT;
			} else if ((err == -EACCESS) && (!Fxattr(1, _path, &sb)))
				err = -EEXIST;
			errno = (int) -err;
			return -1;
		}
		return (int) err;
	}
	errno = -(int)err;
	return -1;
}
