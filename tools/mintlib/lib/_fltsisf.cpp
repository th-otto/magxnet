|
| long integer to single float conversion routine
|
| Andreas Schwab (schwab@ls5.informatik.uni-dortmund.de)
|  mostly copied from _floatsi.cpp

#ifndef __M68881__
	.text
	.even
	.globl	___floatsisf

___floatsisf:
#ifdef	sfp004

| addresses of the 68881 data port. This choice is fastest when much data is
| transferred between the two processors.

comm =	 -6	|	fpu command reg
resp =	-16	|	fpu response reg
zahl =	  0	|	fpu data reg

	lea	0xfffffa50:w,a0
	movew	#0x4000,a0@(comm)	| load long int to fp0
	cmpiw	#0x8900,a0@(resp)	| check
	movel	a7@(4),a0@
	movew	#0x6400,a0@(comm)	| get single from fp0
| wait loop is NOT coded directly
1:	cmpw	#0x8900,a0@(resp)
	jeq	1b
	movel	a0@,d0
	rts

#else /* !sfp004 */

BIAS4	=	0x7F-1

	moveml	d2-d5,sp@-	| save registers to make norm_sf happy

	movel	sp@(20),d4	| prepare result mantissa
	movew	#BIAS4+32-8,d0	| radix point after 32 bits
	movel	d4,d2		| set sign flag
	jge	1f		| nonnegative
	negl	d4		| take absolute value
1:
	swap	d2		| follow norm_sf conventions
	clrw	d1		| set rounding = 0
	jmp	norm_sf

#endif /* !sfp004 */

#endif /* !__M68881__ */
