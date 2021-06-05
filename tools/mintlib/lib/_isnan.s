| double floating point predicates as proposed by IEEE (appendix)
|
|  Isnan(x)  returns true iff x is a NaN
|  Finite(x) returns trus iff x is finite (i.e. not an infiniy or a NaN)
|
| written by Olaf Flebbe (flebbe@tat.physik.uni-tuebingen.de)
|
| Revision 1.0 olaf 12-92
| 
	.text
	.even
	.globl	_Isnan
	.globl	_Finite

_Isnan:	moveml	sp@(4),d0/d1
	bclr	#31,d0
	cmpl	#0x7ff00000,d0
	jle	No
	tstl	d1
	jeq	No
	moveql	#1,d0
	rts
No:	clrl	d0
	rts

_Finite: moveml	sp@(4),d0/d1
	bclr	#31,d0
	cmpl	#0x7ff00000,d0
	jlt	Yes
	clrl	d0
	rts
Yes:	moveql	#1,d0
	rts
