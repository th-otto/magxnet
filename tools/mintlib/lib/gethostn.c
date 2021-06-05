/* gethostname -- for now, fake by looking in environment */
/* (written by Eric R. Smith, placed in the public domain) */

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <support.h>

#define MAXLEN 127

int gethostname(buf, len)
char *buf;
size_t len;
{
	char *foo = 0;
	char xbuf[MAXLEN + 1];
	int fd,
	 r;

#if 0
	if (!foo)
	{
#endif
/* try looking for the file /etc/hostname; if it's present,
 * it contains the name, otherwise we try the environment
 */
		fd = open("/etc/hostname", O_RDONLY);
		if (fd >= 0)
		{
			r = read(fd, xbuf, MAXLEN);
			if (r > 0)
			{
				xbuf[r] = 0;
				foo = xbuf;
				while (*foo)
				{
					if (*foo == '\r' || *foo == '\n')
						*foo = 0;
					foo++;
				}
				foo = xbuf;
			}
			close(fd);
		}
#if 0
	}
#endif
	if (!foo)
		foo = getenv("HOSTNAME");
	strncpy(buf, foo ? foo : "unknown", len);
	return 0;
}
