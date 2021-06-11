/* a public domain rename, by ERS */

#include <limits.h>
#include <errno.h>
#include <osbind.h>
#include <string.h>
#include <unistd.h>
#include <stat.h>
#include <mintbind.h>
#include "lib.h"

int rename(_oldname, _newname)
const char *_oldname,
*_newname;
{
	char oldname[PATH_MAX],
	 newname[PATH_MAX];
	int rval,
	 r;
	long xattr;
	struct stat oldstat;
	struct stat newstat;

	_unx2dos(_oldname, oldname, sizeof(oldname));
	_unx2dos(_newname, newname, sizeof(newname));

	if (((xattr = Fxattr(1, newname, &newstat)) != -ENOSYS)
		&& (xattr == 0)
		&& (Fxattr(1, newname, &newstat) == 0)
		&& (Fxattr(1, oldname, &oldstat) == 0)
		&& (newstat.st_dev == oldstat.st_dev) && (newstat.st_ino == oldstat.st_ino))
	{
		/* rename("foo", "./foo"); */
		errno = EEXIST;
		return -1;
	}

	rval = Frename(0, oldname, newname);
	if (rval == 0)
		return 0;
	if (rval != -EXDEV && rval != -ENOENT && rval != -EPATH)
	{
		if (!strcmp(newname, oldname))
		{
/* on inode-less filesystems do at least catch rename("foo", "foo"); */
			errno = EEXIST;
			return -1;
		}
		if ((r = Fdelete(newname)) == 0)
			rval = Frename(0, oldname, newname);
/* kludge for kernel versions that suffer from `too much DOS' :)  i.e.
 * still look at the files write permission instead of the directories
 * even on real filesystems and when running in the MiNT domain:
 * if it says EACCESS and the dest.dir exists try the old link/unlink way...
 */
		if (rval == -EACCESS && r != -EPATH)
		{
			if ((rval = (int)Flink(oldname, newname)) == 0)
			{
				if ((r = Fdelete(oldname)) != 0 && r != -ENOENT && r != -EPATH)
				{
					(void) Fdelete(newname);
				} else
				{
					rval = 0;			/* alright that did it! */
				}
			} else if (rval == -ENOSYS)
			{
				rval = -EACCESS;		/* filesystem doesn't know link()... */
			}
		}
	}

	if (rval < 0)
	{
		if ((rval == -EPATH) && (xattr != -ENOSYS) && (_enoent(Fxattr(1, oldname, &oldstat) ? oldname : newname)))
			rval = -ENOENT;
		errno = -rval;
		rval = -1;
	}
	return rval;
}
