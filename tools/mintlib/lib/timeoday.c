/* BSDish gettimeofday() and settimeofday() calls */
/* also ftime(), which seems to be similar to gettimeofday() */

/* changed by jerry, Fri, July 5 1996

   using  portlib's __5ms_gettimeofday funcs 
   to get a reasonable usec value returned by gettimeofday().
   Of course this could easily be done in MiNT itself, returning
   sec and usec values based on the the 5ms counter 
   in a Tgettimeofday function . :-)

   The problem with this `solution' is: A process won't know
   the new time if a settime() call was performed by another process
   during it's program's runtime !!

   last change -jerry: 1997/07/02
	
*/

#include <types.h>
#include <time.h>
#include <unistd.h>
#ifdef __TURBOC__
#include <sys\timeb.h>
#else
#include <sys/timeb.h>
#endif
#include <mintbind.h>
#include <ssystem.h>

#ifdef HAS_BSD_TIME
extern time_t timezone;					/* in localtime.c */
extern int daylight;					/* in localtime.c */
#else
extern int _dst;						/* in time.c */
extern long _timezone;					/* in localtim.c */
#endif

extern idst;							/* in time.c */

int _t_o_day_first = 1;					/* changed by stime(),settimeofday(), if  successfull */

static struct timeval the_time;
static clock_t oticks;

static unsigned long getnow()
{
	unsigned long now;

	now = *((unsigned long *) 0x4baL);
	return now;
}

#ifdef HAS_SSYSTEM
clock_t getticks(void)
{
	if (Ssystem(-1, NULL, NULL))
		return (clock_t) Supexec(getnow);
	else
		return (clock_t) Ssystem(S_GETLVAL, 0x000004baL, NULL);
}
#else
#define getticks()  (clock_t)Supexec(getnow)
#endif

static void tvadd(struct timeval *tv1, struct timeval *tv2)
{
	tv1->tv_usec += tv2->tv_usec;
	if ((unsigned long) tv1->tv_usec >= 1000000L)
	{
		tv1->tv_usec -= 1000000L;
		tv1->tv_sec++;
	}
	tv1->tv_sec += tv2->tv_sec;
}


int gettimeofday(tv, tzp)
struct timeval *tv;
struct timezone *tzp;
{
	struct timeval this_time;
	int r;
	clock_t diff,
	 ticks;
	long clockcookie;

	Getcookie(0x434c4f4bL, &clockcookie);	/* 'CLOK'   */
	if (!(clockcookie & (16L << 24)))
	{									/* OS_HAS_TgetTOD   */
		ticks = getticks();				/* DON'T replace by Supexec(getnow) ! */

		if (_t_o_day_first)
		{
			_t_o_day_first = 0;
			oticks = ticks;
			the_time.tv_sec = time((time_t *) 0);
			/* we have to adjust the resolution of time : 2 * CLK_TCK
			   to our resolution: usec is the time since what time returned.
			   Of course it would be better to use some internal ticks variable 
			   '_was_now', that is filled in by time() when it gets the seconds.
			   This variable coud be used then as initial 'oticks' value in
			   this function.
			 */
			the_time.tv_usec = (ticks % (2 * CLK_TCK)) * (1000000L / CLK_TCK);
			if (the_time.tv_usec >= 1000000L)
			{
				the_time.tv_usec -= 1000000L;
				the_time.tv_sec++;
			}
			ticks = getticks();
		}

		diff = ticks - oticks;

		oticks = ticks;

		this_time.tv_sec = diff / CLK_TCK;
		this_time.tv_usec = ((diff % CLK_TCK) * 1000000L) / CLK_TCK;
		tvadd(&the_time, &this_time);

		if (tv)
		{
			tv->tv_sec = the_time.tv_sec;
			tv->tv_usec = the_time.tv_usec;
		}

		r = 0;
	} else
	{									/* OS supports Tgettimeofday    */
		r = Tgettimeofday(tv, tzp) ? 1 : 0;
	}
	if (tzp)
	{
		if (!(clockcookie & (32L << 24)))
		{								/* TgetTOD_HAS_TZP  */
			tzset();					/* we now need tzset() !!!! */
#ifdef HAS_BSD_TIME
			tzp->tz_minuteswest = (int) (timezone / 60);
			tzp->tz_dsttime = (daylight) ? 1 : 0;
#else
			tzp->tz_minuteswest = (int) (_timezone / 60);
			tzp->tz_dsttime = (_dst) ? 1 : 0;
#endif
		}
	}
	return r;
}


int ftime(tp)
struct timeb *tp;
{
	struct timeval tv;
	struct timezone tz;

	if (!tp || gettimeofday(&tv, &tz))
		return (1);

	tp->time = tv.tv_sec;
	tp->millitm = tv.tv_usec / 1000;

#ifdef HAS_BSD_TIME
	tp->timezone = (int) (timezone / 60);
	tp->dstflag = (daylight) ? 1 : 0;
#else
	tp->timezone = (int) (_timezone / 60);
	tp->dstflag = (_dst) ? 1 : 0;
#endif
	return 0;
}
