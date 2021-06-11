#ifdef __TURBOC__
#include <sys\times.h>
#else
#include <sys/times.h>
#endif
#include <time.h>
#include <mintbind.h>
#include <errno.h>

extern long _childtime;

/* macro to convert milliseconds into CLK_TCKs */
#define CVRT(x) ((x)/((1000L/CLK_TCK)))

long times(buffer)
struct tms *buffer;
{
	long usage[8],
	 r,
	 real_time;

	real_time = _clock();

	if ((r = Prusage(usage)) != -ENOSYS)
	{
		if (r >= 0 && buffer)
		{
			buffer->tms_cutime = CVRT(usage[3]);
			buffer->tms_cstime = CVRT(usage[2]);
			buffer->tms_utime = CVRT(usage[1]);
			buffer->tms_stime = CVRT(usage[0]);
			return real_time;
		}
		if (!buffer)
			r = -EFAULT;
		errno = (int) -r;
		return -1;
	}

	if (buffer)
	{
		buffer->tms_cstime = (time_t) 0;
		buffer->tms_cutime = (time_t) _childtime;
		buffer->tms_stime = (time_t) 0;
		buffer->tms_utime = (time_t) real_time - _childtime;
	}
	return real_time;
}
