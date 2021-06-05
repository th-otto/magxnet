/*
 * added _binmode()
 *	if called with TRUE, then subsequently all fopens have _IOBIN
 *	by default  on open. This will make life much easier for
 *	people who have been using the Gnu lib (any my job with the compiler
 *	much easier too, dont't have to go hunting for fopens())
 *				++jrb;
 */

#include <stdio.h>

extern int __default_mode__;

void _binmode(force)
int force;
{
	if (force)
		__default_mode__ = _IOBIN;
	else
		__default_mode__ = 0;
}
