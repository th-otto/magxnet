/*
 * semi efficient div and ldiv using gcc struct return conventions
 *	this efficiency is desirable.
 *
 *  NOTE: *not* suitable for use with -fpcc-struct-return option
 *	  for the (slightly) more portable coding, see div.c
 *
 *	++jrb	bammi@dsrgsun.ces.cwru.edu
 */

#ifdef __MSHORT__
/*
 * div_t div(int num, int denom)
 */
	.text
	.even
	.globl	_div
_div:
	movw	sp@(4),d0   | num
	movw	sp@(6),d1   | denom
	extl	d0
	divs	d1,d0
	swap	d0		
	rts		    | d0<31:16> = result.quot  d0<15:0> = result.rem

#else /* !__MSHORT__ */
	.globl _div
_div:			| div is same as ldiv when !__MSHORT__
#endif

/*
 * ldiv_t ldiv(long num, long denom)
 */
	.globl	_ldiv
_ldiv:
	movl	sp@(8),sp@-	| push denom
	movl	sp@(8),sp@-	| push num
	jbsr	___divsi3	| returns quo in d0.l and rem in d1.l
	addqw	#8,sp		| which is how we want it (see fixnum.s)
	rts
