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
| Ported over to 68k assembler
|
| Revision 1.0:
| original 8088 code from P.S.Housel

	.text
	.even
	.globl	__negdf2, ___negdf2

__negdf2:			| floating point negate
___negdf2:
	moveml	sp@(4),d0-d1
	bchg	#31,d0		| flip sign bit
	rts
