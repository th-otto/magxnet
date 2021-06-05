/* time.c : return the elapsed seconds since midnight Jan 1 1970 GMT */
/* written by Eric R. Smith and placed in the public domain */


/* 
 * changes for UTC kernel time by jerry g geiger Apr 97
 * using recommanded 'CLOK' cookie:
 * val:		&xff000000   bitfield
 			&x00000000   reserved
			&x00ffffff   offset from UTC in seconds
 */

#include <time.h>
#include <mintbind.h>
#include "lib.h"


static struct tm this_tm;
int _dst;
static long CLOCK_cookie = -1;			/* not initialized  */

/* CLOCK_cookie defines: */
#define CLOCK_IS_UTC	(2L<<24)
#define CLOCK_IS_LMT	(1L<<24)
#define OS_HAS_TgetTOD	(16L<<24)
#define TgetTOD_HAS_TZP (32L<<24)

/* CLOCK_cookie & 0xffffff:  offset from UTC in seconds */

/* from mint sources: unixtim()
	calculate unixtime from dostime values, if those are straight UTC
	the value should be correct
*/

static int const mth_start[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

/* if KERNEL_HAS_UTC
 */
static time_t utc_unixtime(unsigned int dostime, unsigned int dosdate)
{
	int sec,
	 min,
	 hour;
	int mday,
	 mon,
	 year;
	long y,
	 s;

	sec = (dostime & 31) << 1;
	min = (dostime >> 5) & 63;
	hour = (dostime >> 11) & 31;
	mday = dosdate & 31;
	mon = ((dosdate >> 5) & 15) - 1;
	year = 80 + ((dosdate >> 9) & 127);

/* calculate tm_yday here */
	y = (mday - 1) + mth_start[mon] +	/* leap year correction */
		(((year % 4) != 0) ? 0 : (mon > 1));

	s = (sec) + (min * 60L) + (hour * 3600L) + (y * 86400L) + ((year - 70) * 31536000L) + ((year - 69) / 4) * 86400L;

	return s;
}


/* kernel runs in local mean time (including DST??)
 * unixtime: convert a DOS time/date pair into a unix time 
 * in case people were wondering about whether or not DST is applicable
 * right now, we set the external variable _dst to the value
 * returned from mktime
 */
static time_t lmt_unixtime(unsigned int dostime, unsigned int dosdate)
{
	time_t t;
	struct tm *stm = &this_tm;

	stm->tm_sec = (dostime & 31) << 1;
	stm->tm_min = (dostime >> 5) & 63;
	stm->tm_hour = (dostime >> 11) & 31;
	stm->tm_mday = dosdate & 31;
	stm->tm_mon = ((dosdate >> 5) & 15) - 1;
	stm->tm_year = 80 + ((dosdate >> 9) & 127);
	if (CLOCK_cookie & CLOCK_IS_LMT)
		stm->tm_isdst = 0;				/* we know about DST: unset */
	else
		stm->tm_isdst = -1;				/* we don't know about DST: see TZ variable */
	stm->tm_wday = stm->tm_yday = -1;	/* or about these */
#if HAS_BSD_TIME
	stm->tm_zone = 0;					/* abbreviation of timezone name */
	stm->tm_gmtoff = 0;					/* offset from UTC in seconds */
	/*  maybe that works:
	   if(CLOCK_cookie & CLOCK_IS_LMT) {
	   stm->tm_zone = "LMT";
	   stm->tm_gmtoff = CLOCK_cookie & 0xffffff; 
	   if(CLOCK_cookie & 0x800000)
	   stm->tm_gmtoff |= 0xff000000;
	   }
	 */
#endif

/* mktime expects a local time, which is what we're giving it */
	t = mktime(stm);

	_dst = (stm->tm_isdst == 1) ? 1 : 0;

	return t;
}



time_t _unixtime(dostime, dosdate)
unsigned dostime,
	dosdate;

/*
 * unixtim(time, date): convert a Dos style (time, date) pair into
 * a Unix time (seconds from midnight Jan 1., 1970)
 */
{

	if (CLOCK_cookie & CLOCK_IS_UTC)
		return (utc_unixtime(dostime, dosdate));
	/* else */
	return (lmt_unixtime(dostime, dosdate));
}



time_t time(t)
time_t *t;
{
	unsigned dostime,
	 dosdate;
	time_t made;
	struct timeval tv;

	if (CLOCK_cookie == -1)
		Getcookie(0x434c4f4bL, &CLOCK_cookie);	/* 'CLOK' */

	if (CLOCK_cookie & OS_HAS_TgetTOD)
	{
		Tgettimeofday(&tv, NULL);
		made = (time_t) tv.tv_sec;
	} else
	{
		dostime = Tgettime();
		dosdate = Tgetdate();

		made = _unixtime(dostime, dosdate);
	}

	if (t)
		*t = made;
	return made;
}
