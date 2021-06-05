/* dummy implementations for strcoll and strxfrm, that doesn't know about
   locales */

#include <string.h>

int strcoll(scan1, scan2)
const char *scan1;
const char *scan2;
{
	return strcmp(scan1, scan2);
}

size_t strxfrm(to, from, maxsize)
char *to;
const char *from;
size_t maxsize;
{
	(void) strncpy(to, from, maxsize - 1);
	to[maxsize] = '\0';
	return strlen(to);
}
