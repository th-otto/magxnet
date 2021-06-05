|
| Initialization code; this is common to both 16 and 32 bit libraries,
| so be careful!
|
	.globl	__app		| short, declared in crtinit.c
	.globl	__base		| BASEPAGE *, declared in crtinit.c
	.globl	__heapbase	| void *
	.globl	__stksize	| long, declared by user or in stksiz.c

|
| externs to pull ident strings of all used libraries into the
| executable; if a library is not used, then the extern is
| satisfied by a dummy in the library

	.globl	___Ident_libg
	.globl	___Ident_curses
	.globl	___Ident_widget
	.globl	___Ident_gem
	.globl	___Ident_pml
	.globl	___Ident_gnulib

|
| Assumption: basepage is passed in a0 for accessories; for programs
| a0 is always 0.

	.text
	.even
	.globl	__start
__start:

|
| If compiled for base-relative, get address of data segment from
| basepage and store data+32K in base register.  -- hyc

#ifdef __MBASE__

#define Base	__MBASE__@(__base:w)
#define Heapbase	__MBASE__@(__heapbase:w)
#define Stksize	__MBASE__@(__stksize:w)

	movl	a0, a1
	cmpw	#0, a1		
	jne	skip0
	movl	sp@(4), a1
skip0:	movl	a1@(16), __MBASE__
	subw	#32768, __MBASE__	| 32K == -32K, so subtract to add
#else

#define Base	__base
#define Heapbase	__heapbase
#define Stksize	__stksize

#endif
	subl	a6, a6		| clear a6 for debuggers
	cmpw	#0, a0		| test if acc or program
	jeq	__startprg	| if a program, go elsewhere
	tstl	a0@(36)		| also test parent basepage pointer
	jne	__startprg	| for accs, it must be 0
	movel	a0, Base	| acc basepage is in A0
	lea	a0@(252), sp	| use the command line as a temporary stack
	jmp	__acc_main	| function is in crtinit.c
|
| program startup code: doesn''t actually do much, other than push
| the basepage onto the stack and call _start1 in crtinit.c
|
__startprg:
	movel	sp@(4), a0	| get basepage
	movel	a0, Base	| save it
	movel	a0@(4), d0	| get _base->p_hitpa
	bclr	#0, d0		| round off
	movel	d0, sp		| set stack (temporarily)
	jmp	__crtinit	| in crtinit.c

|
| _setstack: changes the stack pointer; called as
|     void setstack( void *newsp )
| called from crtinit.c once the new stack size has been decided upon
|
| WARNING WARNING WARNING: after you do this, local variables may no longer
| be accessible!
| destroys a0 and a7

	.globl	__setstack
__setstack:
	movel	sp@+, a0	| save return address
	movel	sp@, sp		| new stack pointer
	subql	#4, sp		| fixup for tidy upon return
	jmp	a0@		| back to caller

|
| interfaces for gprof: for crt0.o, does nothing, but for gcrt0.o branches
| to the appropriate subroutines
|
	.globl 	__monstartup
	.globl	__moncontrol
	.globl 	___mcleanup

#ifdef GCRT0
	.globl	_monstartup
	.globl	_moncontrol
	.globl	__mcleanup

__monstartup:
	jmp	_monstartup
__moncontrol:
	jmp	_moncontrol
___mcleanup:
	jmp	__mcleanup
#else
__monstartup:
__moncontrol:
___mcleanup:
	rts
#endif /* GCRT0 */



