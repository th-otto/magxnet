|
| single floating point divide routine
|
#ifndef __M68881__
	.text
	.even
	.globl	__divsf3, ___divsf3
#ifdef	ERROR_CHECK
#include "errbase.h"
	.globl	__infinitysf

LC0:
	.ascii "floating point division by 0\12\15\0"
	.even
#endif	ERROR_CHECK

__divsf3:
___divsf3:

#ifdef	ERROR_CHECK
	tstl	sp@(8)			| check if divisor is 0
	jne	no_exception

	pea	pc@(LC0)
	pea	Stderr
	jbsr	_fprintf
	addql	#8,sp

	moveq	#Erange,d0		| set _errno to ERANGE
	Emove	d0,Errno
	movel	__infinitysf,d0		| return signed infinity
	btst	#31,sp@(4)		| transfer sign of dividend
	jeq	clear			| (mjr++)
	bset	#31,d0			|
clear:					|
	rts

no_exception:
#endif	ERROR_CHECK

#ifndef sfp004
|
| written by Kai-Uwe Bloem (I5110401@dbstu1.bitnet).
| Based on a 80x86 floating point packet from comp.os.minix, written by P.Housel
| Revision 1.2.4 michal 05-93 (ntomczak@vm.ucs.ualberta.ca)
|   + resynchro with errno codes
|   + code smoothing
|   + removed extra code 'retz:' and 'divz:'; these cases are
|     going back through 'retzero:' and 'retinf:'
|
| patched by Olaf Flebbe (flebbe@tat.physik.uni-tuebingen.de)
|
| Revision 1.2.3 olaf 5-93
|   + correct sign bug for retinf and retzero
|
| Revision 1.2.2 olaf 12-92
|   + added support for NaN and Infinites
|   + added support for -0
|
| Revision 1.2.1 olaf 11-92
|   + prevent the tie rounding case if dividing is not exact.
|      > paranoia now says: "Division appears to round correctly"
|      ** requires _normsf Version 1.4.2 or later
|
| Revision 1.2, kub 01-90 :
| added support for denormalized numbers
|
| Revision 1.1, kub 12-89 :
| Created single float version for 68000
|
| Revision 1.0:
| original 8088 code from P.S.Housel for double floats

BIAS4	=	0x7F-1

	lea	sp@(4),a0	| pointer to parameters u and v
	moveml	d2-d5,sp@-	| save registers
	moveml	a0@,d4/d5	| d4 = u, d5 = v

	movel	#0x7fffff,d3
	movel	d4,d0		| d0 = u.exp
	andl	d3,d4		| remove exponent from u.mantissa
	swap	d0
	movew	d0,d2		| d2 = u.sign

	movel	d5,d1		| d1 = v.exp
	andl	d3,d5		| remove exponent from v.mantissa
	swap	d1
	eorw	d1,d2		| d2 = u.sign ^ v.sign (in bit 15)

	moveq	#15,d3
	bclr	d3,d0		| kill sign bit
	bclr	d3,d1		| kill sign bit
	lsrw	#7,d0
	lsrw	#7,d1
|
|
|
	moveq	#-1,d3
	cmpb	d3,d0		| comparison with #0xff
	jeq	0f		| u == NaN || u== Inf
	cmpb	d3,d1
	jeq	1f		| v == NaN || v == Inf
	tstb	d0
	jne	3f		| u not zero nor denorm
	tstl	d4
	jeq	2f		| 0/ ?

3:	tstw	d1
	jne	nospec

	tstl	d5
	jne	nospec
	jra	retinf		| x/0 -> +/- Inf

0:	tstl	d4		| u == NaN ?
	jne	retnan		| NaN/ x
	cmpb	d3,d1
	jeq	retnan		| Inf/Inf or Inf/NaN
|	jra	retinf		| Inf/x | x != Inf && x != NaN
|
|	Return Infinity with correct sign
|
retinf:	movel	#0xff000000,d0
	lslw	#1,d2
	roxrl   #1,d0		| shift in high bit as given by d2
return:	moveml	sp@+,d2-d5
	rts

1:	tstl	d5
	jne	retnan		| x/NaN
|	jra	retzero		| x/Inf -> +/- 0
|
|	Return correct signed zero
|
retzero:moveq	#0,d0		| zero destination
	lslw	#1,d2		| set X bit accordingly
	roxrl	#1,d0
	jra	return

2:	tstw	d1
	jne	retzero		| 0/x ->+/- 0
	tstl	d4
	jne	retzero		| 0/x
|	jra	retnan		| 0/0
|
|	Return NaN
|
retnan: movel	d3,d0		| d3 contains 0xffffffff
	lsrl	#1,d0
	jra	return
|
|	End of special handling
|
nospec:	moveq	#23,d3
	bset	d3,d4		| restore implied leading "1"
	tstw	d0		| check for zero exponent - no leading "1"
	jne	1f
	bclr	d3,d4		| remove it
	addw	#1,d0		| "normalize" exponent
1:
	tstl	d4
	jeq	retzero		| dividing zero

	bset	d3,d5		| restore implied leading "1"
	tstw	d1		| check for zero exponent - no leading "1"
	jne	1f
	bclr	d3,d5		| remove it
	addw	#1,d1		| "normalize" exponent
1:
# ifndef ERROR_CHECK
	tstl	d5
	jeq	retinf		| divide by zero
# endif	ERROR_CHECK

	subw	d1,d0		| subtract exponents,
	addw	#BIAS4-8+1,d0	| add bias back in, account for shift
	addw	#34,d0		| add loop offset, +2 for extra rounding bits
				| for denormalized numbers (2 implied by dbra)
	movew	#27,d1		| bit number for "implied" pos (+4 for rounding)
	moveql	#-1,d3		| zero quotient (for speed a one''s complement)
	subl	d5,d4		| initial subtraction, u = u - v
2:
	btst	d1,d3		| divide until 1 in implied position
	jeq	5f

	addl	d4,d4
	jcs	4f		| if carry is set, add, else subtract

	addxl	d3,d3		| shift quotient and set bit zero
	subl	d5,d4		| subtract, u = u - v
	dbra	d0,2b		| give up if result is denormalized
	jra	5f
4:
	addxl	d3,d3		| shift quotient and clear bit zero
	addl	d5,d4		| add (restore), u = u + v
	dbra	d0,2b		| give up if result is denormalized
5:	subqw	#2,d0		| remove rounding offset for denormalized nums
	notl	d3		| invert quotient to get it right

	clrl	d1		| zero rounding bits
	tstl 	d4		| check for exact result
	jeq	1f
	moveql	#-1,d1		| prevent tie case
1:	movel	d3,d4		| save quotient mantissa
	jmp	norm_sf		| (registers on stack removed by norm_sf)

#if 0
| this  is dead code right now - retzero and retinf used (mj)
# ifndef ERROR_CHECK
retz:	moveq	#0,d0		| zero destination
	moveml	sp@+,d2-d5
	rts			| no normalization needed

divz:	movel	__infinitysf,d0	| return infinity value
	moveml	sp@+,d2-d5	| should really cause trap ?!?
	btst	#31,sp@(4)	| transfer sign of dividend
	jeq	clear		| (mjr++)
	bset	#31,d0		|
	rts			|
clear:				|
	bclr	#31,d0		|
	rts
# endif	ERROR_CHECK
#endif

#else

| single precision floating point stuff for Atari-gcc using the SFP004
| or compatible boards with a memory mapped 68881
| developed with gas
|
|  single floating point divide routine
|
| M. Ritzert (mjr at dmzrzu71)
|            (ritzert@dfg.dbp.de)
| 4.10.1990
|
| +_infinitysf returned instead of a NAN
| the DOMAIN exception is not supported yet. In case of an exception
| _errno is always set to ERANGE

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
	movew	#0x4400,a0@(comm)	| load first argument to fp0
	cmpiw	#0x8900,a0@(resp)	| check
	movel	sp@(4),a0@
	movew	#0x4424,a0@(comm)
	.long	0x0c688900, 0xfff067f8
	movel	sp@(8),a0@
	movew	#0x6400,a0@(comm)	| result to d0
	.long	0x0c688900, 0xfff067f8
	movel	a0@,d0			| REMARK: 0/0 returns a NAN
	rts				| if ERROR_CHECK is disabled

#endif	sfp004
#endif /* !__M68881__ */
