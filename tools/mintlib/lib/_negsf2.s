| double floating point negation routine
|
| written by Kai-Uwe Bloem (I5110401@dbstu1.bitnet).
| Based on a 80x86 floating point packet from comp.os.minix, written by P.Housel
| patched by Olaf Flebbe (flebbe@tat.physik.uni-tuebingen.de)
|
| revision 1.1.1, olaf 12-92
|  + Since sign of NaN is ignored, we can flip it.
|  + -0. is different from 0., but compares equal!
|
| Revision 1.1, kub 12-89 :
| Created single float version for 68000
|
| Revision 1.0:
| original 8088 code from P.S.Housel for double floats

	.text
	.even
	.globl	__negsf2, ___negsf2

__negsf2:			| floating point negate
___negsf2:
	movel	sp@(4),d0
	bchg	#31,d0		| flip sign bit
	rts
