/* from Dale Schumacher's dLibs */

#include <stdio.h>
#include <stddef.h>
#include <assert.h>

int fputs(data, fp)
register const char *data;
register FILE *fp;
{
	register int n = 0;

	while (*data)
	{
		if (fputc(*data++, fp) == EOF)
			return (EOF);
		++n;
	}
	return (n);
}
