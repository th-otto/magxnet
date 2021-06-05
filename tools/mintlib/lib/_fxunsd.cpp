|
| double float to unsigned long conversion routine
| does not really return unsigned long: max result is 0x7fffffff
| mjr
	.text
	.even
	.globl	__fixunsdfsi, ___fixunsdfsi

#ifdef ERROR_CHECK
#include "errbase.h"
_Overflow:
	.ascii "OVERFLOW\0"
_Negative:
	.ascii "NEGATIVE NUMBER\0"
_Error_String:
	.ascii "_fixunsdfsi: %s error\n\0"
.even
#endif ERROR_CHECK

__fixunsdfsi:
___fixunsdfsi:

#ifdef ERROR_CHECK
	tstl	sp@(4)			| negative?
	jeq	Continue
	moveq	#Erange,d0
	Emove	d0,Errno
	pea	pc@(_Negative)
	jra	error_exit
Continue:
#endif /* ERROR_CHECK */
#ifdef __M68881__

	fintrzd sp@(4),fp0		| convert
	fmovel	fp0,d0

#endif __M68881__

#ifdef	sfp004
| double float to unsigned long conversion routine
| does not really return unsigned long: max result is 0x7fffffff
| mjr

comm =	 -6
resp =	-16
zahl =	  0

	lea	0xfffffa50:w,a0
	movew	#0x5403,a0@(comm)	| fintrz to fp0
	cmpiw	#0x8900,a0@(resp)	| check
	movel	sp@(4),a0@
	movel	sp@(8),a0@
	movew	#0x6000,a0@(comm)	| result to d0
	.long	0x0c688900, 0xfff067f8
	movel	a0@,d0
#endif	sfp004

# if !defined (sfp004) && !defined (__M68881__)

| double float to unsigned long conversion routine
|
| written by Kai-Uwe Bloem (I5110401@dbstu1.bitnet).
| Based on a 80x86 floating point packet from comp.os.minix, written by P.Housel
|
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
	moveml	d4/d5,sp@-	| save registers
	moveml	a0@,d4-d5	| get the number
	movew	a0@,d0		| extract exp
	bclr	#15,d0		| kill sign bit
	lsrw	#4,d0

	andl	#0x0fffff,d4	| remove exponent from mantissa
	bset	#20,d4		| restore implied leading "1"

	cmpw	#BIAS8,d0	| check exponent
	jlt	zero		| strictly fractional, no integer part ?
	cmpw	#BIAS8+32,d0	| is it too big to fit in a 32-bit integer ?
	jgt	toobig

	subw	#BIAS8+21,d0	| adjust exponent
	jgt	2f		| shift up
	jeq	3f		| no shift

	cmpw	#-8,d0		| replace far shifts by swap
	jgt	1f
	movew	d4,d5		| shift fast, 16 bits
	swap	d5
	clrw	d4
	swap	d4
	addw	#16,d0		| account for swap
	jgt	2f
	jeq	3f

1:	lsrl	#1,d4		| shift down to align radix point;
	addqw	#1,d0		| extra bits fall off the end (no rounding)
	jlt	1b		| shifted all the way down yet ?
	jra	3f

2:	addl	d5,d5		| shift up to align radix point
	addxl	d4,d4
	subqw	#1,d0
	jgt	2b
3:
	movel	d4,d0		| put integer into result register
7:
	moveml	sp@+,d4/d5
	rts

zero:
	clrl	d0		| make the whole thing zero
	jra	7b

toobig:
	moveml	sp@+,d4/d5
	moveq	#-1,d0		| ugh. Should cause a trap here.
	bclr	#31,d0
#endif

#ifdef	ERROR_CHECK
	cmpil	#0x7fffffff,d0	| >= long_max
	jge	error_plus	|
	rts
error_plus:
	moveml	d0-d1,sp@-
	moveq	#Erange,d0
	Emove	d0,Errno
	pea	pc@(_Overflow)	| for printf
error_exit:
	pea	pc@(_Error_String)	|
	pea	Stderr
	jbsr	_fprintf	|
	addl	#12,sp		|
	moveml	sp@+,d0-d1
#endif	ERROR_CHECK
	rts

