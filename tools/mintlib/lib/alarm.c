/*
 * alarm: a public domain alarm for MiNT (by ers)
 */

#include <errno.h>
#include <mintbind.h>
#include <limits.h>
#include <unistd.h>

unsigned int alarm(secs)
unsigned secs;
{
	long r;
	static short have_talarm = 1;

	if (have_talarm)
	{
#ifndef __MSHORT__
		if (secs > ((unsigned int) (LONG_MAX / 1000)))
			secs = ((unsigned int) (LONG_MAX / 1000));
#endif
		r = Talarm((long) secs);
		if (r == -EINVAL)
			have_talarm = 0;
		else
			return (unsigned int) r;
	}
	return 0;

}