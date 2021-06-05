/* As suggested by Harbison & Steele */

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

__EXTERN long strtol __PROTO((const char *, char **, int));

long atol(str)
const char *str;
{
	return strtol(str, (char **) 0, 10);
}
