; Pure C calls this, if stack checking fails: 
; report stack overflow and abort
; 17-6-92 um

.globl _StkOver
.globl _StkLim
.globl abort

.text

_StkOver:
	movea.l _StkLim, a7	; get some stack...
	pea.l _overflow_message
	move.w #9, -(a7)
	trap #1				; Cconws
	addq.l #6, a7		; well...
	jmp abort

.data

_overflow_message:
	.asciiz "Stack overflow\r\n"
