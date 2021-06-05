|
|  single float to long conversion routine
|
| Andreas Schwab (schwab@ls5.informatik.uni-dortmund.de)
|  mostly copied from _fixdfsi.cpp
|  (error check removed because no checking possible)

#ifndef __M68881__

	.text
	.even
	.globl	___fixsfsi

___fixsfsi:

#ifdef sfp004

comm =	 -6
resp =	-16
zahl =	  0

	lea	0xfffffa50:w,a0
	movew	#0x4403,a0@(comm)	| fintrz to fp0
	cmpiw	#0x8900,a0@(resp)	| check
	movel	a7@(4),a0@
	movew	#0x6000,a0@(comm)	| result to d0
| waiting loop is NOT coded directly
1:	cmpiw	#0x8900,a0@(resp)
	jeq	1b
	movel	a0@,d0
	rts

#else /* !sfp004 */

BIAS4	=	0x7F-1

	movel	sp@(4),d0	| get number
	movel	d2,sp@-		| save register
	movel	d0,d1
	swap	d1		| extract exp
	movew	d1,d2		| extract sign
	bclr	#15,d1		| kill sign bit
	lsrw	#7,d1

	andl	#0x7fffff,d0	| remove exponent from mantissa
	bset	#23,d0		| restore implied leading "1"

	cmpw	#BIAS4,d1	| check exponent
	jlt	zero		| strictly factional, no integer part ?
	cmpw	#BIAS4+32,d1	| is it too big to fit in a 32-bit integer ?
	jgt	toobig

	subw	#BIAS4+24,d1	| adjust exponent
	jgt	2f		| shift up
	jeq	7f		| no shift (never too big)

1:	negw	d1
	lsrl	d1,d0		| shift down to align radix point;
				| extra bits fall off the end (no rounding)
	jra	7f		| never too big

2:	lsll	d1,d0		| shift up to align radix point

3:	cmpl	#0x80000000,d0	| -2147483648 is a nasty evil special case
	jne	6f
	tstw	d2		| this had better be -2^31 and not 2^31
	jpl	toobig
	jra	8f
6:	tstl	d0		| sign bit set ? (i.e. too big)
	jmi	toobig
7:
	tstw	d2		| is it negative ?
	jpl	8f
	negl	d0		| negate
8:
	movel	sp@+,d2
	rts

zero:
	clrl	d0		| make the whole thing zero
	jra	8b

toobig:
	moveq	#-1,d0		| ugh. Should cause a trap here.
	bclr	#31,d0		| make it #0x7fffffff
	jra	8b

#endif /* !sfp004*/

#endif /* !__M68881__ */
