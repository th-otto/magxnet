#include <osbind.h>
#include <unistd.h>
#include <basepage.h>
#include <mintbind.h>
#include <errno.h>


int getppid()
{
	long r;
	static short have_getppid = 1;

	if (have_getppid)
	{
		r = Pgetppid();
		if (r == -ENOSYS)
			have_getppid = 0;
		else
			return (int) r;
	}
	return (int) (((long) (_base->p_parent)) >> 8);
}
