/*
 * div
 *	this one should be compat with -fpcc-struct-return
 *
 *	++jrb	bammi@dsrgsun.ces.cwru.edu
 */
#include <stdlib.h>

#ifdef __GNUC__

long __divsi3(long, long);				/* returns: quot in d0.l  remainder in d1.l */

#ifdef __MSHORT__
div_t div(int num, int denom)
{
	div_t result;

	__asm__ volatile ("\
		divs	%4,%3	| %3/%2 must be a data reggie
		movw	%2,%0	| %2<31:16> == rem    %2<15:0> == quot
		swap	%2
		movw	%2,%1":"=g" (result.quot), "=g"(result.rem), "=d"((long) num):"2"((long) num), "d"(denom));

	return result;
}
#else /* !__MSHORT__ */
__asm__(".stabs \"_div\",5,0,0,_ldiv");
#endif
#else /* !__GNUC__ */

div_t div(num, denom)
int num,
	denom;
{
	div_t res;

	res.quot = num / denom;
	res.rem = num % denom;

	return res;
}

#endif /* !__GNUC__ */
