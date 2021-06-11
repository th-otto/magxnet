/* fake mkfifo -- should even work sometimes now :)  */

#include <mintbind.h>
#include <errno.h>
#include <support.h>
#include <types.h>
#include <limits.h>
#include <stat.h>
#include <unistd.h>
#include "lib.h"

int mkfifo(_path, mode)
const char *_path;
mode_t mode;
{
	static unsigned long timelast;
	char path[PATH_MAX],
	 linkf[30] = "u:\\pipe\\n$",
		*s,
		c;
	unsigned long l;
	unsigned short x;
	long r;
	int i;

	/* create symlink to /pipe/n$ mode id (see open.c)
	   for id code pid&0x7fff and time>>1 in base-36, this just fits
	   in pipefs' limit of 14 chars...  */

	_unx2dos(_path, path, sizeof(path));
	s = linkf + sizeof "u:\\pipe\\n$" - 1;
	*s++ = ((mode >> 6) & 7) + '0';
	*s++ = ((mode >> 3) & 7) + '0';
	*s++ = (mode & 7) + '0';
#if 0
#define FMODE (FA_HIDDEN)
	*s++ = ((FMODE & 0xf0) >> 4) + '@';
	*s++ = (FMODE & 0xf) + '@';
#endif
	x = (unsigned short) 36 *(unsigned short) 36 *(unsigned short) 36 - 1 - (unsigned short) (getpid() & 0x7fff);

	c = x % 36;
	if (c > 9)
		c += 'a' - '9' - 1;
	s[2] = c + '0';
	x /= 36;
	c = x % 36;
	if (c > 9)
		c += 'a' - '9' - 1;
	s[1] = c + '0';
	*s = (x / 36) - 10 + 'a';

	/* BUG:  14 char names in /pipe should work but they do not
	   everywhere... (MiNT 1.10, readdir gets no \0 char)  ok so
	   then try to squeeze the pid in 2 chars, this is possible
	   because they are always <= 999 at least in 1.10.  */
	if (*s == 'z' && s[1] > '7')
	{
		*s = s[1];
		s[1] = s[2];
		--s;
	}
	s += 3;
	/* make sure time is uniqe, if necessary sleep. */
	for (; (l = time((long *) 0) >> 1) == timelast; Fselect(1000, 0L, 0L, 0L))
		;
	timelast = l;
	for (i = 0; i < 6; ++i, l /= 36)
	{
		c = l % 36;
		if (c > 9)
			c += 'a' - '9' - 1;
		*s++ = c + '0';
	}
	*s = 0;
	r = Fsymlink(linkf, path);
	if (r == -ENOSYS)
	{
		errno = ENOSYS;
		return -1;
	}
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
