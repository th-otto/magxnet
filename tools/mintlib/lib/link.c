/* make a hard link */

#include <errno.h>
#include <mintbind.h>
#include <param.h>
#include <unistd.h>
#include <stat.h>
#include "lib.h"

/*
 * if MiNT is not active, we try to fail gracefully
 */

int link(_old, _new)
const char *_old,
*_new;
{
	long r;
	char old[MAXPATHLEN],
	 new[MAXPATHLEN];

	_unx2dos(_old, old, sizeof(old));
	_unx2dos(_new, new, sizeof(new));

	r = Flink(old, new);
	if (r < 0 && r != -ENOSYS)
	{
		struct stat sb;

		if ((r == -EPATH))
		{
			if (_enoent(Fxattr(1, old, &sb) ? old : new))
				r = -ENOENT;
		} else if ((r == -EACCESS) && (!Fxattr(1, new, &sb)))
			r = -EEXIST;
		errno = (int) -r;
		return -1;
	} else if (r == -ENOSYS)
	{
		errno = EXDEV;
		return -1;
	}
	return 0;
}
