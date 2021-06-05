/*
 * textio: read/write routines for text. These can be used in programs
 * where read and write are used instead of the (preferred) stdio routines
 * for manipulating text files, by doing something like
 *  #define read _text_read
 * Written by Eric R. Smith and placed in the public domain.
 *
 * Modified to fix a bug causing a premature EOF signal -
 *   Michal Jaegermann, June 1991.
 */

#include <stdio.h>
#include <unistd.h>
#include <support.h>

int _text_read(fd, buf, nbytes)
int fd;
char *buf;
int nbytes;
{
	char *to,
	*from;
	int r;

	do
	{
		r = read(fd, buf, nbytes);
		if (r <= 0)						/* if EOF or read error - return */
			return r;
		to = from = buf;
		do
		{
			if (*from == '\r')
				from++;
			else
				*to++ = *from++;
		} while (--r);
	} while (buf == to);				/* only '\r's? - try to read next nbytes */
	return (int) (to - buf);
}

int _text_write(fd, from, nbytes)
int fd;
const char *from;
int nbytes;
{
#ifdef __SOZOBON__
	char buf[1024 + 2];
#else
	char buf[BUFSIZ + 2];
#endif
	char *to,
	 c;
	int w,
	 r,
	 bytes_written;

	bytes_written = 0;
	while (bytes_written < nbytes)
	{
		w = 0;
		to = buf;
		while (w < BUFSIZ && bytes_written < nbytes)
		{
			if ((c = *from++) == '\n')
			{
				*to++ = '\r';
				*to++ = c;
				w += 2;
			} else
			{
				*to++ = c;
				w++;
			}
			bytes_written++;
		}
		if ((r = write(fd, buf, w)) != w)
			return (r < 0) ? r : bytes_written - (w - r);
	}
	return bytes_written;
}
