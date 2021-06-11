#include <errno.h>
#include <string.h>
#include <stat.h>
#include <mintbind.h>
#include "lib.h"

/*
Given a pathname for which some system call returned EPATH, this function
decides if UNIX would have returned ENOENT instead.
Warning: path must be in dos format.
*/

int _enoent(path)
char *path;
{
	register char *s;
	struct stat st;
	long oldmask,
	 xattr;

	for (s = path; *s; s++)
		/* nop */ ;
	oldmask = Psigblock(~0L);

	for (; s != path; s--)
	{
		if (*s == '\\')
		{
			*s = '\0';
			if ((xattr = Fxattr(0, path, &st)) == -ENOSYS)
			{
				(void) Psigsetmask(oldmask);
				return 0;
			}
			if ((xattr == 0) && ((st.st_mode & S_IFMT) != S_IFDIR))
			{
				*s = '\\';
				(void) Psigsetmask(oldmask);
				return 0;				/* existing non-directory file in path, ENOTDIR ok */
			}
			*s = '\\';
		}
	}
	(void) Psigsetmask(oldmask);
	return 1;							/* should have been ENOENT */
}
