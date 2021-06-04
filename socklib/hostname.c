/* gethostname -- for now, fake by looking in environment */
/* (written by Eric R. Smith, placed in the public domain) */

/* Modified to support Pure-C, Thorsten Otto. */

#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef __GNUC__
#include <unistd.h>
#endif
#include <errno.h>

#ifndef ENAMETOOLONG
#define ENAMETOOLONG 86
#endif

/* Changed by Guido Flohr:  Include sys/param.h for maximum hostname
   length.  */
#include <sys/param.h>
#include <sys/types.h>

#undef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 127


int gethostname(char *buf, size_t len)
{
	char *foo = 0;
	char xbuf[MAXHOSTNAMELEN + 1];
	int fd, r;

	/* fall back to the old method */
	{
		/* try looking for the file /etc/hostname; if it's present,
		 * it contains the name, otherwise we try the environment
		 */
		fd = open("/etc/hostname", O_RDONLY); /* BUG: does not work with MagiC */
		if (fd >= 0)
		{
			r = (int)read(fd, xbuf, MAXHOSTNAMELEN);
			if (r > 0)
			{
				xbuf[r] = 0;
				foo = xbuf;
				while (*foo)
				{
					if (*foo == '\r' || *foo == '\n')
					{
						*foo = 0;
						/* break; BUG: missing */
					}
					++foo;
				}
				foo = xbuf;
			}
			close(fd);
		}

		if (foo == NULL)
			foo = getenv("HOSTNAME");

#if __GNUC_PREREQ(8, 0)
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

		strncpy(buf, foo ? foo : "unknown", len);

#if 0 /* BUG: missing check */
		if (real_length > len) /* BUG: should be >= */
		{
			__set_errno(ENAMETOOLONG);
			return -1;
		}
#endif

		return 0;
	}
}

