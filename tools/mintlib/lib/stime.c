/* stime -- set the current time to the value given.
 *
 * All times are in Unix format, i.e. seconds since to
 * midnight, January 1, 1970 GMT
 *
 * written by Eric R. Smith, and placed in the public domain.
 *
 */

#include <compiler.h>
#include <limits.h>
#include <time.h>
#include <errno.h>
#include <osbind.h>
#include <mintbind.h>
#include <ioctl.h>
#include <assert.h>
#include <unistd.h>
#ifdef __TURBOC__
#include <sys\types.h>
#else
#include <sys/types.h>
#endif
#include "lib.h"

/* extern int __mint; */
extern int _t_o_day_first;				/* in timeoday.c */

extern time_t _dostime __PROTO((time_t t));	/* in utime.c */


int stime(t)
time_t *t;
{
	unsigned long dtime;
	unsigned date,
	 time;
	long r;

	assert(t != 0);
	dtime = _dostime(*t);
	date = (int) (dtime & 0xffff);
	time = (int) (dtime >> 16) & 0xffff;

	if (((r = Tsetdate(date)) != 0) || ((r = Tsettime(time)) != 0))
	{
		errno = r == -1 ? EBADARG : (int) -r;
		return -1;
	}
	/* we just make gettimeofday initate itself again */
	_t_o_day_first = 1;

	return 0;
}


int settimeofday(tv, tzp)
struct timeval *tv;
struct timezone *tzp;
{
	return (stime(&tv->tv_sec));
}
