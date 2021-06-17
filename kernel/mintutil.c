#include "sockets.h"
#include "timeout.h"
#include "bpf.h"
#include "mxkernel.h"

/*
 * various utility functions to mimic a MiNT kernel
 */

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

int sleep(int queue, long cond)
{
	UNUSED(queue);
	UNUSED(cond);
	p_kernel->appl_yield();
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

void wake(int queue, long cond)
{
	/* NOT IMPLEMENTED YET */
	UNUSED(queue);
	UNUSED(cond);
}

/*** ---------------------------------------------------------------------- ***/

void wakeselect(long proc)
{
	/* NOT IMPLEMENTED YET */
	UNUSED(proc);
}

/*** ---------------------------------------------------------------------- ***/

/* unknown, does nothing;; only used by tcpd thread */
void x1bd00(long arg)
{
	UNUSED(arg);
}

/*** ---------------------------------------------------------------------- ***/

long cdecl unixtime(ushort time, ushort date)
{
	int sec, min, hour;
	int mday, mon, year;
	long s;

	static int const mth_start[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

	sec = (time & 31) << 1;
	min = (time >> 5) & 63;
	hour = (time >> 11) & 31;

	mday = date & 31;
	mon = ((date >> 5) & 15) - 1;
	year = 80 + ((date >> 9) & 127);

	/* calculate tm_yday here */
	s = (mday - 1) + mth_start[mon] +	/* leap year correction */
		(((year % 4) != 0) ? 0 : (mon > 1));

	s = (sec) + (min * 60L) + (hour * 3600L) + (s * 86400L) + ((year - 70) * 31536000L) + ((year - 69) / 4) * 86400L;

	return s;
}
