#include <types.h>
#include <unistd.h>
#include <osbind.h>
#include <mintbind.h>
#include <errno.h>

int setreuid(ruid, euid)
int ruid,
	euid;
{
	long r;
	static short have_setreuid = 1;

	if (have_setreuid)
	{
		r = Psetreuid(ruid, euid);
		if (r == -ENOSYS)
			have_setreuid = 0;
		else if (r < 0)
		{
			errno = (int) -r;
			return -1;
		} else
			return 0;

	}
	return setuid(euid);
}
