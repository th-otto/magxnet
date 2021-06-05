/* from Dale Schumacher's dLibs */

#include <stdio.h>
#include <stddef.h>
#include <assert.h>

int puts(data)
const char *data;
{
	register int n;

	if (((n = fputs(data, stdout)) == EOF) || (fputc('\n', stdout) == EOF))
		return (EOF);
	return (++n);
}
