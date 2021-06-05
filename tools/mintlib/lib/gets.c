#include <stdio.h>
#include <assert.h>

char *gets(data)
register char *data;
{
	register char *p = data;
	register int c;

	while (((c = getc(stdin)) != EOF) && (c != '\n'))
	{
		if (c == '\b')
		{
			if (p > data)
				--p;
		} else
			*p++ = c;
	}
	*p = '\0';
	return (((c == EOF) && (p == data)) ? NULL : data);	/* NULL == EOF */
}
