| mjr: not needed on the TT

#ifndef	__M68881__
	.text
	.even
	.globl	__muldf3, ___muldf3

__muldf3:
___muldf3:

# ifdef	sfp004

| double precision floating point stuff for Atari-gcc using the SFP004
| developed with gas
|
| double precision multiplication
|
| M. Ritzert (mjr at dfg.dbp.de)
|
| 4.10.1990
|
| no NAN checking implemented since the 68881 treats this situation "correctly",
| i.e. according to IEEE

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
	movew	#0x5423,a0@(comm)
	.long	0x0c688900, 0xfff067f8
	movel	a7@(12),a0@
	movel	a7@(16),a0@
	movew	#0x7400,a0@(comm)	| result to d0/d1
	.long	0x0c688900, 0xfff067f8
	movel	a0@,d0
	movel	a0@,d1
	rts

# else	sfp004

| double floating point multiplication routine
|
| written by Kai-Uwe Bloem (I5110401@dbstu1.bitnet).
| Based on a 80x86 floating point packet from comp.os.minix, written by P.Housel
| Revision 1.2.4 michal 05-93 (ntomczak@vm.ucs.ualberta.ca)
|   + ensure that Inf * NaN == NaN * Inf == NaN
|     and 0 * Inf = Inf * 0 = NaN
|
| Revision 1.2.3 michal 05-93 (ntomczak@vm.ucs.ualberta.ca)
|   + code smoothing
|
| patched by Olaf Flebbe (flebbe@tat.physik.uni-tuebingen.de)
|
| Revision 1.2.2 olaf 05-93:
|   + fixed a bug for signed bug for 0.
|
| Revision 1.2.1 olaf 12-92:
|   + added support for NaN and Infinites
|   + added support for -0
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

	lea	sp@(4),a0
	moveml	d2-d7,sp@-
	moveml	a0@,d4-d5/d6-d7 | d4-d5 = v, d6-d7 = u

	movel	#0x0fffff,d3
	movel	d6,d0		| d0 = u.exp
	andl	d3,d6		| remove exponent from u.mantissa
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
	jne	0f		| if not the first one then maybe the second
	tstl	d7
	jeq	1f
0:	tstl	d1
	jne	1f
	tstl	d5
1:	seq	d2		| 'one of factors is 0' flag in the lowest byte
	lsrw	#4,d0		| keep in d0, d1 exponents only
	lsrw	#4,d1
|
| Testing for NaN and Infinities
|
	movew	#0x7ff,d3
	cmpw	d3,d0
	jeq	0f
	cmpw	d3,d1
	jne	nospec
	jra	1f
|	first operand is special
|	Nan?
0:	orl	d7,d6
	jne	retnan
1:	tstb	d2		| 0 times special or special times 0 ?
	jne	retnan		| yes -> NaN
	cmpw	d3,d1		| is the other special?
	jeq	2f		| maybe it is NaN
|
|	Return Infinity with correct sign
|
retinf:	moveq	#0,d1
	movel	#0xffe00000,d0	| we will return #0xfff00000 or #0x7ff00000
	lslw	#1,d2
	roxrl   #1,d0		| shift in high bit as given by d2
return:	moveml	sp@+,d2-d7
	rts
|
| v is special
|
2:	orl	d5,d4		| is v NaN?
	jeq	retinf		| if not then we have (not-NaN * Inf)
|
|	Return NaN
|
retnan: moveql	#-1,d1
	movel	d1,d0
	lsrl	#1,d0		| 0x7fffffff -> d0
	jra	return
|
| end of NaN and Inf.
|
nospec:	tstb	d2		| not needed - but we can waste two instr.
	jne	retzz		| return signed 0 if one of factors is 0
	lea	sp@(-16),sp	| multiplication accumulator

	moveq	#20,d3
	bset	d3,d6		| restore implied leading "1"
	tstw	d0		| check for zero exponent - no leading "1"
	jne	1f
	bclr	d3,d6		| remove it
	addqw	#1,d0		| "normalize" exponent
1:	movel	d6,d3
	orl	d7,d3
	jeq	retz		| multiplying by zero

	moveq	#20,d3
	bset	d3,d4		| restore implied leading "1"
	tstw	d1		| check for zero exponent - no leading "1"
	jne	1f
	bclr	d3,d4		| remove it
	addqw	#1,d1		| "normalize" exponent
1:	movel	d4,d3
	orl	d5,d3
	jeq	retz		| multiplying by zero

	addw	d1,d0		| add exponents,
	subw	#BIAS8+16-11,d0	| remove excess bias, acnt for repositioning

	lea	sp@,a1		| initialize 128-bit product to zero
	moveq	#0,d3
	movel	d3,a1@+
	movel	d3,a1@+
	movel	d3,a1@+
	movel	d3,a1@
	subqw	#4,a1		| an address of sp@(8) in a1

| see Knuth, Seminumerical Algorithms, section 4.3. algorithm M

	swap	d2
	movew	#4-1,d2
1:
	movew	d5,d3
	mulu	d7,d3		| mulitply with bigit from multiplier
	addl	d3,a1@(4)	| store into result
	movew	d4,d3
	mulu	d7,d3
	movel	a1@,d1		| add to result
	addxl	d3,d1
	movel	d1,a1@
	roxlw	a1@-		| rotate carry in

	movel	d5,d3
	swap	d3
	mulu	d7,d3
	addl	d3,a1@(4)	| add to result
	movel	d4,d3
	swap	d3
	mulu	d7,d3
	movel	a1@,d1		| add to result
	addxl	d3,d1
	movel	d1,a1@

	movew	d6,d7
	swap	d6
	swap	d7
	dbra	d2,1b

	swap	d2		| [TOP 16 BITS SHOULD BE ZERO !]

	moveml	sp@(2),d4-d7	| get the 112 valid bits
	clrw	d7		| (pad to 128)
	movel	#0x0000ffff,d3
2:
	cmpl	d3,d4		| multiply (shift) until
	jhi	3f		|  1 in upper 16 result bits
	cmpw	#9,d0		| give up for denormalized numbers
	jle	3f
	swap	d4		| (we''re getting here only when multiplying
	swap	d5		|  with a denormalized number; there''s an
	swap	d6		|  eventual loss of 4 bits in the rounding
	swap	d7		|  byte -- what a pity 8-)
	movew	d5,d4
	movew	d6,d5
	movew	d7,d6
	clrw	d7
	subqw	#8,d0		| decrement exponent
	subqw	#8,d0
	jra	2b
3:
	movel	d6,d1		| get rounding bits
	roll	#8,d1
	movel	d1,d3		| see if sticky bit should be set
	orl	d7,d3		| (lower 16 bits of d7 are guaranteed to be 0)
	andl	#0xffffff00,d3
	jeq	4f
	orb	#1,d1		| set "sticky bit" if any low-order set
4:	lea	sp@(16),sp	| remove accumulator from stack
	jmp	norm_df		| (result in d4/d5)
|				| norm_df does not return here
retz:	lea	sp@(16),sp	| drop accumulator space
retzz:	moveq	#0,d0		| save zero as result
	movel	d0,d1
	lslw	#1,d2		| fill X bit
	roxrl	#1,d0		| set high bit of d0 accordingly
	moveml	sp@+,d2-d7
	rts			| no normalizing neccessary

# endif	sfp004
#endif	__M68881__
