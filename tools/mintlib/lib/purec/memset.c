/* from Henry Spencer's stringlib */
#include <stddef.h>
#include <string.h>

/*
 * memset - set bytes
 *
 * CHARBITS should be defined only if the compiler lacks "unsigned char".
 * It should be a mask, e.g. 0377 for an 8-bit machine.
 */

#ifndef CHARBITS
#	define	UNSCHAR(c)	((unsigned char)(c))
#else
#	define	UNSCHAR(c)	((c)&CHARBITS)
#endif

void *
memset(s, ucharfill, size)
void * s;
register int ucharfill;
size_t size;
{
	register char *scan;
	register size_t n;
	register int uc;

	scan = (char *)s;
	uc = UNSCHAR(ucharfill);
	for (n = size; n > 0; n--)
		*scan++ = uc;

	return(s);
}
