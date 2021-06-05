|
| long integer to double float conversion routine
|
	.text
	.even
	.globl	__floatsidf, ___floatsidf

__floatsidf:
___floatsidf:
#ifdef __M68881__
|
| Written by M.Ritzert
| 22.11.91
| ritzert@dfg.dbp.de
|
	fintrzd a7@(4),fp0		| load long int to fp0
	fmoved	fp0,a7@-		| get double from fp0
	moveml	a7@+,d0-d1
 	rts

#endif	__M68881__
#ifdef	sfp004
|
| Written by M.Ritzert
| 5.10.90
| ritzert@dfg.dbp.de
|

| addresses of the 68881 data port. This choice is fastest when much data is
| transferred between the two processors.

comm =	 -6	|	fpu command reg
resp =	-16	|	fpu response reg
zahl =	  0	|	fpu data reg

| waiting loop ...
|
| wait:
| ww:	cmpiw	#0x8900,a1@(resp)
| 	beq	ww
| is coded directly by
|	.long	0x0c688900, 0xfff067f8

	lea	0xfffffa50:w,a0
	movew	#0x4000,a0@(comm)	| load long int to fp0
	cmpiw	#0x8900,a0@(resp)	| check
	movel	a7@(4),a0@
	movew	#0x7400,a0@(comm)	| get double from fp0
	.long	0x0c688900, 0xfff067f8
	movel	a0@,d0
	movel	a0@,d1
	rts

#endif	sfp004
#if !defined	(__M68881__) && !defined (sfp004)
|
| written by Kai-Uwe Bloem (I5110401@dbstu1.bitnet).
| Based on a 80x86 floating point packet from comp.os.minix, written by P.Housel
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

	movel	sp@(4),d0	| get the 4-byte integer
	moveml	d2-d7,sp@-	| save registers to make norm_df happy

	movel	d0,d4		| prepare result mantissa
	moveq	#0,d5
	movew	#BIAS8+32-11,d0	| radix point after 32 bits
0:
	movel	d4,d2		| set sign flag
	jge	1f		| nonnegative
	negl	d4		| take absolute value
1:
	swap	d2		| follow norm_df conventions
	clrw	d1		| set rounding = 0
	jmp	norm_df

#endif	/* !__M68881__ && !sfp004	*/
