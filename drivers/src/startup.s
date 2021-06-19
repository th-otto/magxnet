/*
 * Startup code for STinG drivers written in C
 */
	.xref sting_main
	.text
	move.l 4(sp),a0
	bra sting_main
