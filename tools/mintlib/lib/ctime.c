#include <time.h>
#include <string.h>
#include <stdio.h>

/* taken from Dale Schumacher's dLibs library */

static char timebuf[26];

static char *day[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

static char *month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};


#ifdef __GNUC__
/* 
 * macro to produce two decimal digits of mval and write them
 * at position pointed by mptr - moves mptr to the next free position;
 * mval has to be in range 0-99, or banshee will get you
 */
#define _TWODIG(mptr,mval)		\
{					\
        long _i = (long)(mval);		\
     __asm__ volatile(			\
	" divu	#10,%1\n"			\
	" addb	#48,%1\n"			\
	" moveb	%1,%0@+\n"			\
	" swap	%1\n"			\
	" addb	#48,%1\n"			\
	" moveb	%1,%0@+\n"			\
	      : "=a"(mptr), "=d"(_i)		\
	      : "1"(_i), "0"(mptr));		\
}

#else

static char *two_dig __PROTO((char *buf, unsigned short num));

static char *two_dig(buf, num)
char *buf;
unsigned short num;
{
	unsigned int rem;

	rem = num % 10;
	*buf++ = '0' + (num / 10);
	*buf++ = '0' + rem;
	return buf;
}

#endif /* __GNUC__ */

char *asctime(time)
register const struct tm *const time;

/*
 *      Convert <time> structure value to a string.  The same format, and
 *      the same internal buffer, as for ctime() is used for this function.
 */
{
	unsigned short values[6],
	*vpos,
	*valp;
	int i;
	char *ptr = timebuf;

	(void) strcpy(ptr, "??? ??? ?? ??:??:?? ????\n");
	if (time != NULL)
	{
		vpos = values;
		*vpos++ = time->tm_mday;
		*vpos++ = time->tm_hour;
		*vpos++ = time->tm_min;
		*vpos++ = time->tm_sec;
		i = 1900 + time->tm_year;
		*vpos++ = i / 100;
		*vpos = i % 100;
		ptr = (char *) memcpy(ptr, day[time->tm_wday], 3) + 3;
		ptr += 1;
		ptr = (char *) memcpy(ptr, month[time->tm_mon], 3) + 3;
		valp = values;
		do
		{
			ptr += 1;
#ifdef __GNUC__
			_TWODIG(ptr, *valp++);
#else
			ptr = two_dig(ptr, *valp++);
#endif /* __GNUC__ */
		}
		while (valp < vpos);
#ifdef __GNUC__
		_TWODIG(ptr, *valp);
#else
		two_dig(ptr, *valp);
#endif /* __GNUC__ */
		if ('0' == timebuf[8])
			timebuf[8] = ' ';			/* blank out leading zero on a day */

	}
	return (timebuf);
}


char *ctime(rawtime)
const time_t *const rawtime;

/*
 *      Convert <rawtime> to a string.  A 26 character fixed field string
 *      is created from the raw time value.  The following is an example
 *      of what this string might look like:
 *              "Wed Jul  8 18:43:07 1987\n\0"
 *      A 24-hour clock is used, and due to a limitation in the ST system
 *      clock value, only a resolution of 2 seconds is possible.  A pointer
 *      to the formatted string, which is held in an internal buffer, is
 *      returned.
 */
{
	return (asctime(localtime(rawtime)));
}
