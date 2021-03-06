|
|  double float to long conversion routine
|
| M. Ritzert (ritzert@DFG.DBP.DE)
	.text
	.even
	.globl	__fixdfsi, ___fixdfsi

#ifdef ERROR_CHECK
#include "errbase.h"
_Overflow:
	.ascii "OVERFLOW\0"
_Error_String:
	.ascii "_fixdfsi: %s error\n\0"
.even
#endif ERROR_CHECK

__fixdfsi:
___fixdfsi:

#ifdef sfp004
|
| 4.10.1990/10.1.1992
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
	movew	#0x5403,a0@(comm)	| fintrz to fp0
	cmpiw	#0x8900,a0@(resp)	| check
	movel	sp@(4),a0@
	movel	sp@(8),a0@
	movew	#0x6000,a0@(comm)	| result to d0
	.long	0x0c688900, 0xfff067f8
	movel	a0@,d0

#endif	sfp004
#ifdef	__M68881__

|
| floating point stuff for Atari-gcc using
| an 68030/68881
| developed with gcc/gas
|
| double float to long conversion routine
|
| M. Ritzert (mjr@dfg.dbp.de)
|
| 30.11.1991/10.1.1992
|

	fintrzd sp@(4),fp0		| convert the arg
	fmovel fp0,d0			| return

#endif	__M68881__


# if !defined (sfp004) && !defined (__M68881__)

| double float to long conversion routine
|
| written by Kai-Uwe Bloem (I5110401@dbstu1.bitnet).
| Based on a 80x86 floating point packet from comp.os.minix, written by P.Housel
|
| modified by Andreas Schwab (schwab@ls5.informatik.uni-dortmund.de):
| no loop needed for the shift-down case
| testing for far shifts now too slow
| use different registers to gain speed
|
| Revision 1.3, kub 01-90 :
| added support for denormalized numbers
|
| Revision 1.2, kub 01-90 :
| replace far shifts by swaps to gain speed
|
| Revision 1.1, kub 12-89 :
| Ported over to 68k assembler
|
| Revision 1.0:
| original 8088 code from P.S.Housel

BIAS8	=	0x3FF-1

	lea	sp@(4),a0	| pointer to parameters
	moveml	d2/d3,sp@-	| save registers
	moveml	a0@,d0-d1	| get the number
	movew	a0@,d2		| extract exp
	movew	d2,d3		| extract sign
	bclr	#15,d2		| kill sign bit
	lsrw	#4,d2

	andl	#0x0fffff,d0	| remove exponent from mantissa
	bset	#20,d0		| restore implied leading "1"

	cmpw	#BIAS8,d2	| check exponent
	jlt	zero		| strictly fractional, no integer part ?
	cmpw	#BIAS8+32,d2	| is it too big to fit in a 32-bit integer ?
	jgt	toobig

	subw	#BIAS8+21,d2	| adjust exponent
	jgt	2f		| shift up
	jeq	7f		| no shift (never too big)

	negw	d2
	lsrl	d2,d0		| shift down to align radix point;
				| extra bits fall off the end (no rounding)
	jra	7f		| never too big

2:	addl	d1,d1		| shift up to align radix point
	addxl	d0,d0
	subqw	#1,d2
	jgt	2b

3:	cmpl	#0x80000000,d0	| -2147483648 is a nasty evil special case
	jne	6f
	tstw	d3		| this had better be -2^31 and not 2^31
	jpl	toobig
	jra	8f
6:	tstl	d0		| sign bit set ? (i.e. too big)
	jmi	toobig
7:
	tstw	d3		| is it negative ?
	jpl	8f
	negl	d0		| negate
8:
	moveml	sp@+,d2/d3
	rts

zero:
	clrl	d0		| make the whole thing zero
	jra	8b

toobig:
	moveq	#-1,d0		| ugh. Should cause a trap here.
	bclr	#31,d0
	moveml	sp@+,d2/d3

#endif /* !defined (sfp004) && !defined (__M68881__) */

#ifdef	ERROR_CHECK

| all three versions
	cmpil	#0x7fffffff,d0	| >= long_max
	jge	error_msg
	cmpil	#-0x7fffffff,d0	| <= long_min ?
	jle	error_msg
	rts
error_msg:
	moveml	d0-d1,sp@-
	moveq	#Erange,d0
	Emove	d0,Errno
	pea	pc@(_Overflow)	| for printf
	pea	pc@(_Error_String)
	pea	Stderr
	jbsr	_fprintf
	addl	#12,sp
	moveml	sp@+,d0-d1
#endif	/* ERROR_CHECK */

	rts
