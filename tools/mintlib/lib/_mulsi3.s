| long integer multiplication routine
|
| written by Kai-Uwe Bloem (I5110401@dbstu1.bitnet).
|
|
| Revision 1.1, kub 03-90
| first version, replaces the appropriate routine from fixnum.s.
| This one is longer, but normally faster because __umulsi3 is no longer
| called for multiplication. Rather, the code is inlined here. See the
| comments in _umulsi3.s

	.text
	.even
	.globl	__mulsi3, ___mulsi3, .lmul

.lmul:
__mulsi3:
___mulsi3:
	movel	d2,a0		| save registers
	movel	d3,a1
	movemw	sp@(4),d0-d3	| get the two longs. u = d0-d1, v = d2-d3
	movew	d0,sp@-		| sign flag
	jpl	0f		| is u negative ?
	negw	d1		| yes, force it positive
	negxw	d0
0:	tstw	d2		| is v negative ?
	jpl	0f
	negw	d3		| yes, force it positive ...
	negxw	d2
	notw	sp@		|  ... and modify flag word
0:
	extl	d0		| u.h <> 0 ?
	jeq	1f
	mulu	d3,d0		| r  = v.l * u.h
1:	tstw	d2		| v.h <> 0 ?
	jeq	2f
	mulu	d1,d2		| r += v.h * u.l
	addw	d2,d0
2:	swap	d0
	clrw	d0
	mulu	d3,d1		| r += v.l * u.l
	addl	d1,d0
	movel	a1,d3
	movel	a0,d2
	tstw	sp@+		| should the result be negated ?
	jpl	3f		| no, just return
	negl	d0		| else r = -r
3:	rts
