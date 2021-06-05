#include <types.h>
#include <unistd.h>
#include <osbind.h>
#include <mintbind.h>
#include <errno.h>

int setegid(x)
int x;
{
	long r;
	static short have_setegid = 1;

	if (have_setegid)
	{
		r = Psetegid(x);
		if (r == -EINVAL)
			have_setegid = 0;
		else if (r < 0)
		{
			errno = (int) -r;
			return -1;
		} else
			return 0;

	}
	return setgid(x);
}
