#include <types.h>
#include <unistd.h>
#include <osbind.h>
#include <mintbind.h>
#include <errno.h>

extern uid_t __uid;

uid_t getuid()
{
	long r;
	static short have_getuid = 1;

	if (have_getuid)
	{
		r = Pgetuid();
		if (r == -EINVAL)
			have_getuid = 0;
		else
			return (uid_t) r;
	}
	return __uid;
}
