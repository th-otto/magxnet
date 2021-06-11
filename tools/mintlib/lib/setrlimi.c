/* get/set resource limits */
/* written by Eric R. Smith and placed in the public domain */

#include <time.h>
#include <types.h>
#include <resource.h>
#include <mintbind.h>
#include <errno.h>

int setrlimit(kind, rl)
int kind;
struct rlimit *rl;
{
	unsigned long limit;
	long r;
	int mode;

	limit = rl->rlim_cur;

	if (limit >= RLIM_INFINITY)
		limit = 0;
	else if (limit == 0)
		limit = 1;

	switch (kind)
	{
	case RLIMIT_CPU:
		mode = 1;
		break;
	case RLIMIT_RSS:
		mode = 2;
		break;
	case RLIMIT_DATA:
		mode = 3;
		break;
	default:
		errno = ENOSYS;
		return -1;
	}
	r = Psetlimit(mode, limit);

	if (r < 0)
	{
		errno = (int) -r;
		return -1;
	}
	return 0;
}

int getrlimit(kind, rl)
int kind;
struct rlimit *rl;
{
	long limit;
	int mode;


	switch (kind)
	{
	case RLIMIT_CPU:
		mode = 1;
		break;
	case RLIMIT_RSS:
		mode = 2;
		break;
	case RLIMIT_DATA:
		mode = 3;
		break;
	default:
		errno = ENOSYS;
		return -1;
	}
	limit = Psetlimit(mode, -1L);

	if (limit < 0)
	{
		errno = (int) -limit;
		return -1;
	}

	if (limit == 0)
		limit = RLIM_INFINITY;

	rl->rlim_cur = limit;
	rl->rlim_max = RLIM_INFINITY;
	return 0;
}
