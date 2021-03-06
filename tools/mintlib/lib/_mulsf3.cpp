| mjr: not needed on the TT

#ifndef	__M68881__

	.text
	.even
	.globl	__mulsf3, ___mulsf3

__mulsf3:
___mulsf3:

# ifdef	sfp004

| single precision floating point stuff for Atari-gcc using the SFP004
| developed with gas
|
| single floating point multiplication routine
|
| M. Ritzert (mjr at dfg.dbp.de)
|
| 7. Juli 1989
|
| revision 1.1: adapted to the gcc lib patchlevel 58
| 4.10.90

comm =	 -6
resp =	-16
zahl =	  0

	lea	0xfffffa50:w,a0
	movew	#0x4400,a0@(comm)	| load first argument to fp0
	cmpiw	#0x8900,a0@(resp)	| check
	movel	sp@(4),a0@
	movew	#0x4427,a0@(comm)
	.long	0x0c688900, 0xfff067f8
	movel	sp@(8),a0@
	movew	#0x6400,a0@(comm)	| result to d0
	.long	0x0c688900, 0xfff067f8
	movel	a0@,d0
	rts

# else	sfp004

| single floating point multiplication routine
|
| written by Kai-Uwe Bloem (I5110401@dbstu1.bitnet).
| Based on a 80x86 floating point packet from comp.os.minix, written by P.Housel
|
| Revision 1.2.4 michal 05-93 (ntomczak@vm.ucs.ualberta.ca)
|   + ensure that Inf * NaN == NaN * Inf == NaN
|     and 0 * Inf = Inf * 0 = NaN
|
| Revision 1.2.3 michal 05-93 (ntomczak@vm.ucs.ualberta.ca)
|   + code smoothing
|
| patched by Olaf Flebbe (flebbe@tat.physik.uni-tuebingen.de)
|
| Revision 1.2.2 olaf 05-93
|   + fixed a bug with -0.
|
| Revision 1.2.1 olaf 12-92:
|   + added support for NaN and Infinites
|   + added support for -0
|
| Revision 1.2, kub 01-90 :
| added support for denormalized numbers
|
| Revision 1.1, kub 12-89 :
| Created single float version for 68000. Code could be speed up by having
| the accumulator in the 68000 register set ...
|
| Revision 1.0:
| original 8088 code from P.S.Housel for double floats

BIAS4	=	0x7F-1

	lea	sp@(4),a0
	moveml	d2-d5,sp@-
	moveml	a0@,d4/d5	| d4 = v, d5 = u

	movel	#0x7fffff,d3
	movel	d5,d0		| d0 = u.exp
	andl	d3,d5		| remove exponent from u.mantissa
	swap	d0
	movew	d0,d2		| d2 = u.sign

	movel	d4,d1		| d1 = v.exp
	andl	d3,d4		| remove exponent from v.mantissa
	swap	d1
	eorw	d1,d2		| d2 = u.sign ^ v.sign (in bit 15)
	
	moveq	#15,d3
	bclr	d3,d0		| kill sign bit
	bclr	d3,d1		| kill sign bit
	tstl	d0		| test if one of factors is 0
	jeq	1f
	tstl	d1
1:	seq	d2		| 'one of factors is 0' flag in the lowest byte
	lsrw	#7,d0		| keep here exponents only
	lsrw	#7,d1

|
| Testing for NaN and Infinities
|
	moveq	#-1,d3
	cmpb	d3,d0
	jeq	0f
	cmpb	d3,d1
	jne	nospec
	jra	1f
|
|	first operand is special
|
0:	tstl	d5		| is it NaN?
	jne	retnan
1:	tstb	d2		| 0 times special or special times 0 ?
	jne	retnan		| yes -> NaN
	cmpb	d3,d1		| is the other special ?
	jeq	2f		| maybe it is NaN
|
|	Return Infinity with correct sign
|
retinf: movel	#0xff000000,d0  | we will return #0xff800000 or #0x7f800000
	lslw	#1,d2
	roxrl   #1,d0		| shift in high bit as given by d2
return:	moveml	sp@+,d2-d5
	rts

|
| v is special
|
	
2:	tstl	d4		| is this NaN?
	jeq	retinf		| we know that the other is not zero
retnan: moveql	#-1,d0
	lsrl	#1,d0		| 0x7fffffff -> d0
	jra	return
|
| end of NaN and Inf.
|
nospec:	tstb	d2		| not needed - but we can waste two instr.
	jne	retzz		| return signed 0 if one of factors is 0
	moveq	#23,d3
	bset	d3,d5		| restore implied leading "1"
	subqw	#8,sp		| multiplication accumulator
	tstw	d0		| check for zero exponent - no leading "1"
	jne	1f
	bclr	d3,d5		| remove it
	addqw	#1,d0		| "normalize" exponent
1:	tstl	d5
	jeq	retz		| multiplying zero

	moveq	#23,d3
	bset	d3,d4		| restore implied leading "1"
	tstw	d1		| check for zero exponent - no leading "1"
	jne	1f
	bclr	d3,d4		| remove it
	addqw	#1,d1		| "normalize" exponent
1:	tstl	d4
	jeq	retz		| multiply by zero

	addw	d1,d0		| add exponents,
	subw	#BIAS4+16-8,d0	| remove excess bias, acnt for repositioning

	clrl	sp@		| initialize 64-bit product to zero
	clrl	sp@(4)

| see Knuth, Seminumerical Algorithms, section 4.3. algorithm M

	movew	d4,d3
	mulu	d5,d3		| mulitply with bigit from multiplier
	movel	d3,sp@(4)	| store into result

	movel	d4,d3
	swap	d3
	mulu	d5,d3
	addl	d3,sp@(2)	| add to result

	swap	d5		| [TOP 8 BITS SHOULD BE ZERO !]

	movew	d4,d3
	mulu	d5,d3		| mulitply with bigit from multiplier
	addl	d3,sp@(2)	| store into result (no carry can occur here)

	movel	d4,d3
	swap	d3
	mulu	d5,d3
	addl	d3,sp@		| add to result
				| [TOP 16 BITS SHOULD BE ZERO !]
	moveml	sp@(2),d4-d5	| get the 48 valid mantissa bits
	clrw	d5		| (pad to 64)

	movel	#0x0000ffff,d3
2:
	cmpl	d3,d4		| multiply (shift) until
	jhi	3f		|  1 in upper 16 result bits
	cmpw	#9,d0		| give up for denormalized numbers
	jle	3f
	swap	d4		| (we''re getting here only when multiplying
	swap	d5		|  with a denormalized number; there''s an
	movew	d5,d4		|  eventual loss of 4 bits in the rounding
	clrw	d5		|  byte -- what a pity 8-)
	subqw	#8,d0		| decrement exponent
	subqw	#8,d0
	jra	2b
3:
	movel	d5,d1		| get rounding bits
	roll	#8,d1
	movel	d1,d3		| see if sticky bit should be set
	andl	#0xffffff00,d3
	jeq	4f
	orb	#1,d1		| set "sticky bit" if any low-order set
4:	addqw	#8,sp		| remove accumulator from stack
	jmp	norm_sf		| (result in d4)

retz:	addqw	#8,sp		| release accumulator space
retzz:	moveq	#0,d0		| save zero as result
	lslw	#1,d2		| and set it sign as for d2
	roxrl	#1,d0
	moveml	sp@+,d2-d5
	rts			| no normalizing neccessary

# endif	sfp004
#endif	__M68881__
