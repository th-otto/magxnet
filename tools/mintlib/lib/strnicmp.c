/* derived from strncmp from Henry Spencer's stringlib */
/* revised by ERS */
/* i-changes by Alexander Lehmann */

#include <string.h>
#include <ctype.h>

/*
 * strnicmp - compare at most n characters of string s1 to s2 without case
 *           result is equivalent to strcmp(strupr(s1),s2)),
 *           but doesn't change anything
 */

#ifdef __GNUC__
asm(".stabs \"_strncmpi\",5,0,0,_strnicmp");	/* dept of clean tricks */
asm(".stabs \"_strncasecmp\",5,0,0,_strnicmp");
#endif

int										/* <0 for <, 0 for ==, >0 for > */
strnicmp(scan1, scan2, n)
register const char *scan1;
register const char *scan2;
size_t n;
{
	register char c1,
	 c2;
	register long count;

	if (!scan1)
	{
		return scan2 ? -1 : 0;
	}
	if (!scan2)
		return 1;
	count = n;
	do
	{
		c1 = *scan1++;
		c1 = toupper(c1);
		c2 = *scan2++;
		c2 = toupper(c2);
	} while (--count >= 0 && c1 && c1 == c2);

	if (count < 0)
		return (0);

	/*
	 * The following case analysis is necessary so that characters
	 * which look negative collate low against normal characters but
	 * high against the end-of-string NUL.
	 */
	if (c1 == c2)
		return (0);
	else if (c1 == '\0')
		return (-1);
	else if (c2 == '\0')
		return (1);
	else
		return (c1 - c2);
}

#ifndef __GNUC__
int strncmpi(scan1, scan2, n)
register const char *scan1;
register const char *scan2;
size_t n;
{
	return strnicmp(scan1, scan2, n);
}

int strncasecmp(scan1, scan2, n)
register const char *scan1;
register const char *scan2;
size_t n;
{
	return strnicmp(scan1, scan2, n);
}
#endif