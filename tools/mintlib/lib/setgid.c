#include <types.h>
#include <unistd.h>
#include <osbind.h>
#include <mintbind.h>
#include <errno.h>

extern gid_t __gid;

int setgid(x)
int x;
{
	long r;
	static short have_setgid = 1;

	if (have_setgid)
	{
		r = Psetgid(x);
		if (r == -EINVAL)
		{
			__gid = x;
			have_setgid = 0;
		} else if (r < 0)
		{
			errno = (int) -r;
			return -1;
		}
		return 0;
	}
	__gid = x;
	return 0;
}
