| double floating point compare routine
|
| written by Olaf Flebbe (flebbe@tat.physik.uni-tuebingen.de)
| Based on a 68k floating point packet from Kai-Uwe Bloem, itself based
| on a 80x86 floating point packet from comp.os.minix, written by P.Housel
|
| Revision 2.0: olaf 12-92.
| Revision 2.1: michal 05-93 allow for splitting in separate objects in make

#ifdef __DEF_ALL__	/* this def'ed when making on the ST */

/* gcc-2.0 stuff */
#define L_eqdf2
#define L_gtdf2
#define L_ledf2
#define L_gedf2
#define L_ltdf2

#endif /* __DEF_ALL__ */

#ifdef L_eqdf2
	.text
	.even
	.globl	___eqdf2, ___nedf2
	.globl	__cmpdf2, ___cmpdf2

__cmpdf2:
___cmpdf2:
___eqdf2:
___nedf2:
	moveml	sp@(4),d0-d1/a0-a1		| get numbers to compare with
	tstl	d0		| check sign bit
	jpl	1f
	negl	d1		| negate
	negxl	d0
	bchg	#31,d0		| toggle sign bit

1:	exg	a0,d0
	exg	a1,d1
	tstl	d0		| check sign bit
	jpl	2f
	negl	d1		| negate
	negxl	d0
	bchg	#31,d0		| toggle sign bit

2:	cmpl	d1,a1
	jne	4f
	cmpl	d0,a0
	jne	4f
	bclr	#31,d0
	cmpl	#0x7ff00000,d0		| NaN is not equal NaN !
	jgt	4f
	jlt	3f
	tstl	d1
	jne	4f
3:	moveq	#0,d0
	rts

4:	moveql	#1,d0
	rts
#endif /* L_eqdf2 */

#ifdef L_gtdf2
	.text
	.even
	.globl	___gtdf2
___gtdf2:
	moveml	sp@(4),d0-d1/a0-a1		| get numbers to compare with
	tstl	d0		| check sign bit
	jpl	1f
	negl	d1		| negate
	negxl	d0
	bchg	#31,d0		| toggle sign bit

1:	exg	a0,d0
	exg	a1,d1
	tstl	d0		| check sign bit
	jpl	2f
	negl	d1		| negate
	negxl	d0
	bchg	#31,d0		| toggle sign bit

2:	exg	a0,d0
	exg	a1,d1
	cmpl	a0,d0
	jgt	4f		| d0 > a0 Test if NaN (should be false!)
	jlt	3f		|
	cmpl	a1,d1
	jhi	4f
3:	moveq	#0,d0		| Test is false.
	rts
4:	bclr	#31,d0
	cmpl	#0x7ff00000,d0	| First operand == NaN =?
	jgt	3b
	jlt	5f		| It is finite!
	tstl	d1
	jne	3b		| It *is* a NaN
5:	moveql	#1,d0		| Test True
	rts
#endif /* L_gtdf2 */

#ifdef L_ledf2
	.text
	.even
	.globl	___ledf2
___ledf2:
	moveml	sp@(4),d0-d1/a0-a1		| get numbers to compare with
	tstl	d0		| check sign bit
	jpl	1f
	negl	d1		| negate
	negxl	d0
	bchg	#31,d0		| toggle sign bit

1:	exg	a0,d0
	exg	a1,d1
	tstl	d0		| check sign bit
	jpl	2f
	negl	d1		| negate
	negxl	d0
	bchg	#31,d0		| toggle sign bit

2:	cmpl	d0,a0
	jlt	4f
	jgt	3f
	cmpl	d1,a1
	jls	4f	        | <= !
3:	moveql	#1,d0		| Test is false
	rts
4:	bclr	#31,d0
	cmpl	#0x7ff00000,d0	| First operand == NaN =?
	jgt	3b
	jlt	5f		| It is finite!
	tstl	d1
	jne	3b		| It *is* a NaN
5:	moveq	#0,d0		| Test true
	rts
#endif /* L_ledf2 */


#ifdef L_gedf2
	.text
	.even
	.globl	___gedf2
___gedf2:
	moveml	sp@(4),d0-d1/a0-a1		| get numbers to compare with
	tstl	d0		| check sign bit
	jpl	1f
	negl	d1		| negate
	negxl	d0
	bchg	#31,d0		| toggle sign bit

1:	exg	a0,d0
	exg	a1,d1
	tstl	d0		| check sign bit
	jpl	2f
	negl	d1		| negate
	negxl	d0
	bchg	#31,d0		| toggle sign bit

2:	exg	a0,d0
	exg	a1,d1
	cmpl	a0,d0
	jgt	4f
	jlt	3f
	cmpl	a1,d1
	jeq	4f		| >= !
	jhi	4f
3:	moveql	#-1,d0		| False
	rts
4:	bclr	#31,d0
	cmpl	#0x7ff00000,d0	| First operand == NaN =?
	jgt	3b
	jlt	5f		| It is finite!
	tstl	d1
	jne	3b		| It *is* a NaN
5:	moveq	#0,d0		| Test True
	rts
#endif /* L_gedf2 */

#ifdef L_ltdf2
	.text
	.even
	.globl	___ltdf2
___ltdf2:
	moveml	sp@(4),d0-d1/a0-a1		| get numbers to compare with
	tstl	d0		| check sign bit
	jpl	1f
	negl	d1		| negate
	negxl	d0
	bchg	#31,d0		| toggle sign bit

1:	exg	a0,d0
	exg	a1,d1
	tstl	d0		| check sign bit
	jpl	2f
	negl	d1		| negate
	negxl	d0
	bchg	#31,d0		| toggle sign bit

2:	cmpl	d0,a0
	jlt	4f
	jgt	3f
	cmpl	d1,a1
	jeq	3f
	jls	4f
3:	moveq	#0,d0
	rts
4:	bclr	#31,d0
	cmpl	#0x7ff00000,d0	| First operand == NaN =?
	jgt	3b
	jlt	5f		| It is finite!
	tstl	d1
	jne	3b		| It *is* a NaN
5:	moveql	#-1,d0		| Test True
	rts
#endif /* L_ltdf2 */
