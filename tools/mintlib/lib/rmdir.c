/* rmdir -- remove a directory */
/* written by Eric R. Smith and placed in the public domain */

#include <limits.h>
#include <mintbind.h>
#include <errno.h>
#include <unistd.h>
#include "lib.h"

int rmdir(_path)
const char *_path;
{
	char path[PATH_MAX];
	int r;

	_unx2dos(_path, path, sizeof(path));
	r = Ddelete(path);
	if (r < 0)
	{
		long d;

		if ((r == -EPATH))
		{
			if (_enoent(path))
				r = -ENOENT;
		} else if ((r == -EACCESS) && (((d = Dopendir(path, 0)) & 0xff000000L) != 0xff000000L))
		{
			char *name;
			int rd;

			do
				rd = (int) Dreaddir((int) (PATH_MAX), d, path);
			while (rd >= 0 && *(name = path + sizeof(long)) == '.' && (!*++name || (*name == '.' && !*++name)));
			if (rd != -ENMFIL)
				r = -ENOTEMPTY;
			(void) Dclosedir(d);
		}
		errno = -r;
		r = -1;
	}
	return r;
}
