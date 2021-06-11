#include <types.h>
#include <unistd.h>
#include <osbind.h>
#include <mintbind.h>
#include <errno.h>

int seteuid(x)
int x;
{
	long r;
	static short have_seteuid = 1;

	if (have_seteuid)
	{
		r = Pseteuid(x);
		if (r == -ENOSYS)
			have_seteuid = 0;
		else if (r < 0)
		{
			errno = (int) -r;
			return -1;
		} else
			return 0;

	}
	return setuid(x);
}
