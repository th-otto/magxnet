#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "lib.h"

/*
 * char *getwd(char *buf)
 *	return cwd in buf
 */
char *getwd(buf)
char *buf;
{
	char *ret = getcwd(buf, PATH_MAX);

	if (ret)
		return ret;
	strcpy(buf, strerror(errno));
	return NULL;
}
