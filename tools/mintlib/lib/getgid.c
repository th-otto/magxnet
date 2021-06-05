#include <types.h>
#include <unistd.h>
#include <mintbind.h>
#include <errno.h>

extern gid_t __gid;

gid_t getgid()
{
	long r;
	static short have_getgid = 1;

	if (have_getgid)
	{
		r = Pgetgid();
		if (r == -EINVAL)
			have_getgid = 0;
		else
			return (gid_t) r;
	}
	return __gid;
}
