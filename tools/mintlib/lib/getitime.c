/*..\getitimer.c last change: Sun Jan 18 10:24:52 1998*/
#include <unistd.h>
#include <errno.h>
#ifdef __TURBOC__
#include <sys\time.h>
#else
#include <sys/time.h>
#endif
#include <mintbind.h>

/* in getrusage.c */
__EXTERN void _ms2tval __PROTO((unsigned long, struct timeval *));

int getitimer(which, old)
int which;
struct itimerval *old;
{
	long r;
	long interval,
	 value;

	r = Tsetitimer(which, 0, 0, &interval, &value);
	if (r < 0)
	{
		errno = -r;
		return -1;
	}
	_ms2tval(interval, &old->it_interval);
	_ms2tval(value, &old->it_value);
	return 0;
}

/* -- end of ..\getitimer.c -- */
