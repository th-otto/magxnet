#include <errno.h>
#include <osbind.h>
#include <stddef.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include "lib.h"

/***************************************************************
chdir: change the directory and (possibly) the drive.
By ERS: it's in the public domain.
****************************************************************/

int chdir(dir)
const char *dir;
{
	int drv,
	 old;
	int r;
	char tmp[PATH_MAX];
	register char *d;

	(void) _unx2dos(dir, tmp, sizeof(tmp));	/* convert Unix filename to DOS */
	d = tmp;
	old = Dgetdrv();
	if (*d && *(d + 1) == ':')
	{
		drv = toupper(*d) - 'A';
		d += 2;
		(void) Dsetdrv(drv);
	}

	if (!*d)
	{									/* empty path means root directory */
		*d = '\\';
		*(d + 1) = '\0';
	}
	if ((r = Dsetpath(d)) < 0)
	{
		(void) Dsetdrv(old);
		if ((r == -EPATH) && _enoent(tmp))
		{
			r = -ENOENT;
		}
		errno = -r;
		return -1;
	}
	return 0;
}
