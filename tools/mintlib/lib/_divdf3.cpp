|
| double floating point divide routine
|
#ifndef __M68881__
	.text
	.even
	.globl	__divdf3, ___divdf3
#ifdef	ERROR_CHECK
#include "errbase.h"
	.globl	__infinitydf
LC0:
	.ascii "floating point division by 0\12\15\0"
	.even
#endif	ERROR_CHECK

__divdf3:
___divdf3:
#ifdef	ERROR_CHECK
	tstl	a7@(12)			| check if divisor is 0
	jne	continue
	tstl	a7@(16)
	jne	continue

	pea	pc@(LC0)
	pea	Stderr
	jbsr	_fprintf
	addql	#8,a7

	moveq	#Erange,d0		| set _errno to ERANGE
	Emove	d0,Errno
	moveml	__infinitydf,d0-d1	| return signed infinity
	btst	#31,a7@(4)		| transfer sign of dividend
	jeq	clear			| (mjr++)
	bset	#31,d0			|
	rts				|
clear:					|
	bclr	#31,d0			|
	rts
continue:

#endif	/* ERROR_CHECK */
#ifndef sfp004
|
| written by Kai-Uwe Bloem (I5110401@dbstu1.bitnet).
| Based on a 80x86 floating point packet from comp.os.minix, written by P.Housel
|
| Revision 1.2.4 michal 05-93 (ntomczak@vm.ucs.ualberta.ca)
|   + resynchro with errno codes
|   + code smoothing
|
| patched by Olaf Flebbe (flebbe@tat.physik.uni-tuebingen.de)
|
| Revision 1.2.3 olaf 4-93
|   + Fixed sign for retinf, and retzero: it is in d2.w
|
| Revision 1.2.2 olaf 12-92
|   + added support for NaN and Infinites
|   + added support for -0
|
| Revision 1.2.1 olaf 11-92
|   + prevent the tie rounding case if dividing is not exact.
|      > paranoia now says: "Division appears to round correctly"
|      ** requires _normdf Version 1.6.1 or later
|
| Revision 1.2, kub 01-90 :
| added support for denormalized numbers
|
| Revision 1.1, kub 12-89 :
| Ported over to 68k assembler
|
| Revision 1.0:
| original 8088 code from P.S.Housel

BIAS8	=	0x3FF-1

	lea	sp@(4),a0	| pointer to parameters u and v
	moveml	d2-d7,sp@-	| save registers
	moveml	a0@,d4-d5/d6-d7	| d4-d5 = u, d6-d7 = v

	movel	#0x0fffff,d3
	movel	d4,d0		| d0 = u.exp
	andl	d3,d4		| remove exponent from u.mantissa
	swap	d0
	movew	d0,d2		| d2 = u.sign

	movel	d6,d1		| d1 = v.exp
	andl	d3,d6		| remove exponent from v.mantissa
	swap	d1
	eorw	d1,d2		| d2 = u.sign ^ v.sign (in bit 15)

	moveq	#15,d3
	bclr	d3,d1		| kill sign bit
	bclr	d3,d0		| kill sign bit
	lsrw	#4,d0
	lsrw	#4,d1
|
|
|
	movew	#0x7ff,d3
	cmpw	d3,d0
	jeq	0f		|u == NaN || u== Inf
	cmpw	d3,d1
	jeq	1f		| v == NaN || v == Inf
	tstw	d0
	jne	3f		| u not zero nor denorm
	movel	d5,d3
	orl	d4,d3
	jeq	2f		| 0/ ?

3:	tstw	d1
	jne	nospec

	movel	d7,d3
	orl	d6,d3
	jne	nospec
|	jra	retinf		| x/0 -> +/- Inf
|
|	Return Infinity with correct sign
|
retinf:	moveq	#0,d1
	movel	#0xffe00000,d0
	lslw	#1,d2
	roxrl   #1,d0		| shift in high bit as given by d2
return:	moveml	sp@+,d2-d7
	rts

0:	orl	d5,d4		| u == NaN ?
	jne	retnan		| NaN/ x
	cmpw	d3,d1
	jeq	retnan		| Inf/Inf or Inf/NaN
	jra	retinf		| Inf/x | x != Inf && x != NaN

1:	orl	d7,d6
	jne	retnan		| x/NaN
|	jra	retzero		| x/Inf -> +/- 0
|
|	Return correct signed zero
|
retzero:moveq	#0,d0		| zero destination
	movel	d0,d1
	lslw	#1,d2		| we need an extension bit
	roxrl	#1,d0
	jra	return

2:	tstw	d1
	jne	retzero		| 0/x ->+/- 0
	orl	d5,d4
	jne	retzero		| 0/x
|	jra	retnan		| 0/0
|
|	Return NaN
|
retnan: moveql	#-1,d1
	movel	d1,d0
	lsrl	#1,d0		| 0x7fffffff -> d0
	jra	return
|
|	End of special handling
|
nospec:	moveq	#20,d3
	bset	d3,d4		| restore implied leading "1"
	tstw	d0		| check for zero exponent - no leading "1"
	jne	1f
	bclr	d3,d4		| remove it
	addw	#1,d0		| "normalize" exponent

1:	bset	d3,d6		| restore implied leading "1"
	tstw	d1		| check for zero exponent - no leading "1"
	jne	1f
	bclr	d3,d6		| remove it
0:	addw	#1,d1		| "normalize" exponent

1:	movew	d2,a0		| save sign

	subw	d1,d0		| subtract exponents,
	addw	#BIAS8-11+1,d0	|  add bias back in, account for shift
	addw	#66,d0		|  add loop offset, +2 for extra rounding bits
				|   for denormalized numbers (2 implied by dbra)
	movew	#24,d1		| bit number for "implied" pos (+4 for rounding)
	moveq	#-1,d2		| zero the quotient
	moveq	#-1,d3		|  (for speed it is a one''s complement)
	subl	d7,d5		| initial subtraction,
	subxl	d6,d4		| u = u - v
2:
	btst	d1,d2		| divide until 1 in implied position
	jeq	5f

	addl	d5,d5
	addxl	d4,d4
	jcs	4f		| if carry is set, add, else subtract

	addxl	d3,d3		| shift quotient and set bit zero
	addxl	d2,d2
	subl	d7,d5		| subtract
	subxl	d6,d4		| u = u - v
	dbra	d0,2b		| give up if result is denormalized
	jra	5f
4:
	addxl	d3,d3		| shift quotient and clear bit zero
	addxl	d2,d2
	addl	d7,d5		| add (restore)
	addxl	d6,d4		| u = u + v
	dbra	d0,2b		| give up if result is denormalized
5:	subqw	#2,d0		| remove rounding offset for denormalized nums
	notl	d2		| invert quotient to get it right
	notl	d3

	movel   d5,d1
	orl     d4,d1           | check for exact result
	jeq     1f
	moveql  #-1,d1          | Set rounding bits for tie case
1:	movel	d2,d4		| save quotient mantissa
	movel	d3,d5
	movew	a0,d2		| get sign back
	jmp	norm_df		| (registers on stack removed by norm_df)

#else

| double precision floating point stuff for Atari-gcc using the SFP004
| developed with gas
|
| double precision division
|
| M. Ritzert (mjr at dmzrzu71)
|
| 4.10.1990
|
| no NAN checking implemented to gain compatibility with the TT-lib
|
| addresses of the 68881 data port. This choice is fastest when much data is
| transferred between the two processors.

comm =	 -6
resp =	-16
zahl =	  0

| waiting loop ...
|
| wait:
| ww:	cmpiw	#0x8900,a0@(resp)
| 	beq	ww
| is coded directly by
|	.long	0x0c688900, 0xfff067f8

	lea	0xfffffa50:w,a0
	movew	#0x5400,a0@(comm)	| load first argument to fp0
	cmpiw	#0x8900,a0@(resp)	| check
	movel	a7@(4),a0@
	movel	a7@(8),a0@
	movew	#0x5420,a0@(comm)
	.long	0x0c688900, 0xfff067f8
	movel	a7@(12),a0@
	movel	a7@(16),a0@
	movew	#0x7400,a0@(comm)	| result to d0
	.long	0x0c688900, 0xfff067f8
	movel	a0@,d0
	movel	a0@,d1
 	rts
#endif	sfp004
#endif /* !__M68881__ */
