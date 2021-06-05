#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

__EXTERN long strtol __PROTO((const char *, char **, int));

int atoi(str)
const char *str;
{
	return (int) strtol(str, (char **) 0, 10);
}
