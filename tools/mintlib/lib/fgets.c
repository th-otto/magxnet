/* from Dale Schumacher's dLibs */

#include <stdio.h>
#include <stddef.h>
#include <assert.h>

char *fgets(data, limit, fp)
char *data;
register int limit;
register FILE *fp;
{
	register char *p = data;
	register int c = EOF;

	while ((--limit > 0) && ((c = getc(fp)) != EOF))
		if ((*p++ = c) == '\n')
			break;
	*p = '\0';
	return ((c == EOF && p == data) ? NULL : data);	/* NULL == EOF */
}
