| mjr: _normsf is not needed if the 68881 is present
| but _infinitysf is retained

|#######################################################################

| single floating point normalization routine
|
| written by Kai-Uwe Bloem (I5110401@dbstu1.bitnet).
| Based on a 80x86 floating point packet from comp.os.minix, written by P.Housel
|
| Revision 1.4.4 michal 05-93 (ntomczak@vm.ucs.ualberta.ca)
|  + restored and ensured future synchronization with errno codes
|  + removed bogus error when normalizing legitimate zero
|  + small mods to shave off few cycles
|
| patched by Olaf Flebbe (flebbe@tat.physik.uni-tuebingen.de)
|
| Revision 1.4.3 olaf 12-92:
|  + added support for -0.
|
| Revision 1.4.2 olaf 11-92
|  + correct stack after overflow.
|
| Revision 1.4.1 olaf 10-92:
|  + corrected rounding in tie case: round up, not down.
|    (needed for enquire 4.3)
|
| Revision 1.4, kub 03-90 :
| export ___normsf entry to C language. Rename the internal entry to a name
| not accessible from C to prevent crashes
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

	.text
	.even
	.globl	__infinitysf
#if !defined (__M68881__) && !defined (sfp004)
	.globl	___normsf
	.globl	norm_sf
#include "errbase.h"
#ifdef	ERROR_CHECK
LC0:
	.ascii "normsf: OVERFLOW\12\15\0"
	.even
#endif	ERROR_CHECK

	| C entry, for procs dealing with the internal representation :
	| float __normsf(long mant, short exp, short sign, short rbits);
___normsf:
	lea	sp@(4),a0	| parameter pointer
	moveml	d2-d5,sp@-	| save working registers
	movel	a0@+,d4		| get mantissa
	movew	a0@+,d0		| get exponent
	movew	a0@+,d2		| get sign
	movew	a0@+,d1		| rounding information

	| internal entry for floating point package, saves time
	| d0=u.exp, d2=u.sign, d1=rounding bits, d4/d5=mantissa
	| registers d2-d5 must be saved on the stack !
norm_sf:
	tstl	d4		| rounding and u.mant == 0 ?
	jne	0f
	tstb	d1
	jeq	retzok
0:
	clrb	d2		| "sticky byte"
1:	movel	#0xff000000,d5
7:	tstw	d0		| divide (shift)
	jle	0f		|  denormalized number
	movel	d4,d3
	andl	d5,d3		|  or until no bits above 23
	jeq	2f
0:	addqw	#1,d0		| increment exponent
	lsrl	#1,d4
	orb	d1,d2		| set "sticky"
	roxrb	#1,d1		| shift into rounding bits
	jra	7b
2:
	andb	#1,d2
	orb	d2,d1		| make least sig bit "sticky"
	asrl	#1,d5		| #0xff800000 -> d5
3:	movel	d4,d3		| multiply (shift) until
	andl	d5,d3		| one in "implied" position
	jne	4f
	subqw	#1,d0		| decrement exponent
	jeq	4f		|  too small. store as denormalized number
	addb	d1,d1		| some doubt about this one *
	addxl	d4,d4
	jra	3b
4:
	tstb	d1		| check rounding bits
	jge	6f		| round down - no action neccessary
	negb	d1
	jvc	5f		| round up
	movew   d4,d1           | tie case - round to even
                                | dont need rounding bits any more
	andw	#1,d1           | check if even
	jeq	6f              | mantissa is even - no action necessary
                                | fall through

5:
	clrw	d1		| zero rounding bits
	addl	#1,d4
	tstw	d0
	jne	0f		| renormalize if number was denormalized
	addw	#1,d0		| correct exponent for denormalized numbers
	jra	1b
0:	movel	d4,d3		| check for rounding overflow
	asll	#1,d5		| #0xff000000 -> d5
	andl	d5,d3
	jne	7b		| go back and renormalize
6:
	tstl	d4		| check if normalization caused an underflow
	jeq	retz
	tstw	d0		| check for exponent overflow or underflow
	jlt	retz
	cmpw	#255,d0
	jge	oflow

	lslw	#8,d0		| re-position exponent - one bit too high
	lslw	#1,d2		| get X bit
	roxrw	#1,d0		| shift it into sign position
	swap	d0		| map to upper word
	clrw	d0
	andl	#0x7fffff,d4	| top mantissa bits
	orl	d4,d0		| insert exponent and sign
	moveml	sp@+,d2-d5
	rts

retz:	moveq	#Erange,d0
	Emove	d0,Errno
retzok:
	moveq	#0,d0
	lslw	#1,d2
	roxrl	#1,d0		| sign of 0 is the same as of d2
	moveml	sp@+,d2-d5
	rts

oflow:

#ifdef	ERROR_CHECK
	movel	d1,sp@-
	pea	pc@(LC0)
	pea	Stderr
	jbsr	_fprintf	|
	addql	#8,sp		|
	movel	sp@+,d1
#endif	ERROR_CHECK

|	movel	pc@(__infinitysf),d0	| return infinity value
	movel   __infinitysf,d0

	tstw	d2		| transfer sign
	jge	ofl_clear	| (mjr++)
	bset	#31,d0		|
ofl_clear:
	moveml	sp@+,d2-d5	| should really cause trap ?!?
	rts

#endif __M68881__

__infinitysf:			| +infinity as proposed by IEEE
	.long	0x7f800000
