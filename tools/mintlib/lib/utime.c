/* utime -- set the file modification time of the given file
 * according to the time given; a time of 0 means the current
 * time.
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

time_t _dostime __PROTO((time_t t));

/* convert a Unix time into a DOS time. The longword returned contains
   the time word first, then the date word */

time_t _dostime(t)
time_t t;
{
	time_t time,
	 date;
	struct tm *ctm;

	if ((ctm = localtime(&t)) == NULL)
		return 0;
	time = (ctm->tm_hour << 11) | (ctm->tm_min << 5) | (ctm->tm_sec >> 1);
	date = ((ctm->tm_year - 80) & 0x7f) << 9;
	date |= ((ctm->tm_mon + 1) << 5) | (ctm->tm_mday);
	return (time << 16) | date;
}

int utime(_filename, tset)
const char *_filename;
const struct utimbuf *tset;
{
	int fh;
	long ret;
	unsigned long actime,
	 modtime;
	unsigned long dtime;				/* dos time equivalent */

	char filename[PATH_MAX];
	struct _mutimbuf settime;
	long res;

	if (tset)
	{
		modtime = _dostime(tset->modtime);
		actime = _dostime(tset->actime);
	} else
	{
		actime = ((long) Tgettime() << 16) | (Tgetdate() & 0xFFFF);
		modtime = actime;
	}

	(void) _unx2dos(_filename, filename, sizeof(filename));

	settime.actime = (unsigned short) ((actime >> 16) & 0xFFFF);
	settime.acdate = (unsigned short) (actime & 0xFFFF);
	settime.modtime = (unsigned short) ((modtime >> 16) & 0xFFFF);
	settime.moddate = (unsigned short) (modtime & 0xFFFF);
	res = -ENOSYS;

	if (tset)
		res = Dcntl(FUTIME, filename, (long) (&settime));
	else
		res = Dcntl(FUTIME, filename, (long) 0);

	if (res != -ENOSYS)
	{
		if (res < 0)
		{
			if ((res == -EPATH) && (_enoent(filename)))
				res = -ENOENT;
			errno = (int) -res;
			return -1;
		}
		return 0;
	}
	fh = (int) Fopen(filename, 2);
	if (fh < 0)
	{
#if 1
		/* Kludge:  on dos filesystems return success for dirs
		   even though we failed */
		if ((fh == -ENOENT) &&
			(((ret = Dpathconf(filename, 5)) == 2) || (ret == -ENOSYS)) && (Fattrib(filename, 0, 0) == FA_DIR))
			return 0;
#endif
		if ((fh == -EPATH) && (_enoent(filename)))
			fh = -ENOENT;
		errno = -fh;
		return -1;
	}

	if (tset)
		res = Fcntl(fh, (long) &settime, FUTIME);
	else
		res = Fcntl(fh, (long) 0, FUTIME);

	if (res == -ENOSYS)
	{
		dtime = modtime;
		(void) Fdatime((_DOSTIME *) & dtime, fh, 1);
	}

	if ((fh = Fclose(fh)) != 0)
	{
		errno = -fh;
		return -1;
	}
	return 0;
}
