/*
 *  ffs.c - find the lowest bit set
 *  Returns a 2-exponent of position + 1 and 0 for 0
 *  Michal Jaegermann, <ntomczak@vm.ucs.ualberta.ca>
 *  10 July 1993
 *  This piece of code is in a Public Domain
 */

#include <support.h>

int ffs(bits)
int bits;
{
	register int count;

	if (0 == bits)
		return 0;

	bits &= -bits;
#ifndef __MSHORT__
	count = (bits & 0x0000ffff ? 16 : 32);
	if (bits & 0x00ff00ff)
		count -= 8;
	if (bits & 0x0f0f0f0f)
		count -= 4;
	if (bits & 0x33333333)
		count -= 2;
	if (bits & 0x55555555)
		count -= 1;
#else
	count = (bits & 0x00ff ? 8 : 16);
	if (bits & 0x0f0f)
		count -= 4;
	if (bits & 0x3333)
		count -= 2;
	if (bits & 0x5555)
		count -= 1;
#endif /* __MSHORT__ */
	return count;
}
