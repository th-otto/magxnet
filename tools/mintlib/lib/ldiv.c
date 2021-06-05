/*
 * ldiv
 *	this one should be compat with -fpcc-struct-return
 *
 *	++jrb	bammi@dsrgsun.ces.cwru.edu
 */
#include <stdlib.h>

#ifdef __GNUC__

long __divsi3(long, long);				/* returns: quot in d0.l  remainder in d1.l */

ldiv_t ldiv(long num, long denom)
{
	ldiv_t result;

	__asm__ volatile ("\
 		movl	%3,sp@-
		movl	%2,sp@-
		jsr	___divsi3
		addqw	#8,sp
		movl	d0,%0
		movl	d1,%1":"=g" (result.quot), "=g"(result.rem):"r"(num), "r"(denom));	/* compiler dependency, dont tell gcc about d0,d1 clobb */

	return result;
}

#else /* !__GNUC__ */

ldiv_t ldiv(num, denom)
long num,
	denom;
{
	ldiv_t res;

	res.quot = num / denom;
	res.rem = num % denom;

	return res;
}

#endif /* !__GNUC__ */
