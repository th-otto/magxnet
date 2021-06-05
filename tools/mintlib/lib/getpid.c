#include <osbind.h>
#include <unistd.h>
#include <basepage.h>
#include <mintbind.h>
#include <errno.h>

int getpid()
{
	int r;
	static short have_getpid = 1;

	if (have_getpid)
	{
		r = (int) Pgetpid();
		if (r == -EINVAL)
			have_getpid = 0;
		else
			return r;
	}
	return ((int) (((long) _base) >> 8));
}
