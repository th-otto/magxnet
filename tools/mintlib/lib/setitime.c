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

int setitimer(which, new, old)
int which;
const struct itimerval *new;
struct itimerval *old;
{
	long r;
	long newint,
	 newval,
	 oldint = 0,
		oldval = 0;

	if (new)
	{
		newint = (new->it_interval.tv_sec * 1000L + new->it_interval.tv_usec / 1000L);
		newval = (new->it_value.tv_sec * 1000L + new->it_value.tv_usec / 1000L);
	}
	r = Tsetitimer(which, new ? &newint : 0, new ? &newval : 0, old ? &oldint : 0, old ? &oldval : 0);
	if (r < 0)
	{
		errno = -r;
		return -1;
	}

	if (old)
	{
		_ms2tval(oldint, &old->it_interval);
		_ms2tval(oldval, &old->it_value);
	}
	return 0;
}
