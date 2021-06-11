#include <types.h>
#include <unistd.h>
#include <mintbind.h>
#include <errno.h>

gid_t getegid()
{
	long r;
	static short have_getegid = 1;

	if (have_getegid)
	{
		r = Pgetegid();
		if (r == -ENOSYS)
			have_getegid = 0;
		else
			return (gid_t) r;
	}
	return getgid();
}
