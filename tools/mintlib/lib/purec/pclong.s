; Pure C long arithmetics

; Diese Routinen sind der PCSTDLIB "mehr als Ñhnlich". Da reine Rechen-
; vorschriften keinem Urheberrechtsschutz unterliegen, habe ich sie
; hier Åbernommen. Im Åbrigen sind sie fÅr alle, die Pure C nicht
; haben, komplett nutzlos...

.globl _ulmul
.globl _uldiv
.globl _ulmod
.globl _lmul
.globl _ldiv
.globl _lmod

.module _ulmul:

	move.l    d0,d2
	swap      d2
	tst.w     d2
	bne.b     L2
	move.l    d1,d2
	swap      d2
	tst.w     d2
	bne.b     L1
	mulu      d1,d0
	rts

L1:	mulu      d0,d2
	swap      d2
	mulu      d1,d0
	add.l     d2,d0
	rts

L2:	mulu      d1,d2
	swap      d2
	mulu      d1,d0
	add.l     d2,d0
	rts

.endmod

.module _uldiv:

	move.l    d1,d2
	swap      d2
	tst.w     d2
	bne.b     L2
	move.l    d0,d2
	swap      d2
	tst.w     d2
	bne.b     L1
	divu      d1,d0
	swap      d0
	clr.w     d0
	swap      d0
	rts

L1:	clr.w     d0
	swap      d0
	swap      d2
	divu      d1,d0
	movea.w   d0,a0
	move.w    d2,d0
	divu      d1,d0
	swap      d0
	move.w    a0,d0
	swap      d0
	rts

L2:	movea.l   d1,a0
	swap      d0
	moveq.l   #$00,d1
	move.w    d0,d1
	clr.w     d0
	moveq.l   #$0f,d2
	add.l     d0,d0
	addx.l    d1,d1
L3:	sub.l     a0,d1
	bcc.b     L4
	add.l     a0,d1
L4:	addx.l    d0,d0
	addx.l    d1,d1
	dbra      d2,L3
	not.w     d0
	rts

.endmod

.module _ulmod:

	move.l    d1,d2
	swap      d2
	tst.w     d2
	bne.b     L2
	move.l    d0,d2
	swap      d2
	tst.w     d2
	bne.b     L1
	divu      d1,d0
	clr.w     d0
	swap      d0
	rts

L1:	clr.w     d0
	swap      d0
	swap      d2
	divu      d1,d0
	move.w    d2,d0
	divu      d1,d0
	clr.w     d0
	swap      d0
	rts

L2:	movea.l   d1,a0
	move.l    d0,d1
	clr.w     d0
	swap      d0
	swap      d1
	clr.w     d1
	moveq.l   #$0f,d2
	add.l     d1,d1
	addx.l    d0,d0
L3:	sub.l     a0,d0
	bcc.b     L4
	add.l     a0,d0
L4:	addx.l    d1,d1
	addx.l    d0,d0
	dbra      d2,L3
	roxr.l    #1,d0
	rts

.endmod

.module _lmul:

	move.l    d0,d2
	bpl.b     L1
	neg.l     d0
L1:	eor.l     d1,d2
	movea.l   d2,a0
	tst.l     d1
	bpl.b     L2
	neg.l     d1
L2:	move.l    d0,d2
	swap      d2
	tst.w     d2
	bne.b     L6
	move.l    d1,d2
	swap      d2
	tst.w     d2
	bne.b     L4
	mulu      d1,d0
	move.l    a0,d2
	bpl.b     L3
	neg.l     d0
L3:	rts

L4:	mulu      d0,d2
	swap      d2
	mulu      d1,d0
	add.l     d2,d0
	move.l    a0,d2
	bpl.b     L5
	neg.l     d0
L5:	rts

L6:	mulu      d1,d2
	swap      d2
	mulu      d1,d0
	add.l     d2,d0
	move.l    a0,d2
	bpl.b     L7
	neg.l     d0
L7:	rts

.endmod

.module _ldiv:

	move.l    d0,d2
	bpl.b     L1
	neg.l     d0
L1:	eor.l     d1,d2
	movea.l   d2,a1
	tst.l     d1
	bpl.b     L2
	neg.l     d1
L2:	move.l    d1,d2
	swap      d2
	tst.w     d2
	bne.b     L6
	move.l    d0,d2
	swap      d2
	tst.w     d2
	bne.b     L4
	divu      d1,d0
	swap      d0
	clr.w     d0
	swap      d0
	move.l    a1,d2
	bpl.b     L3
	neg.l     d0
L3:	rts

L4:	clr.w     d0
	swap      d0
	swap      d2
	divu      d1,d0
	movea.w   d0,a0
	move.w    d2,d0
	divu      d1,d0
	swap      d0
	move.w    a0,d0
	swap      d0
	move.l    a1,d2
	bpl.b     L5
	neg.l     d0
L5:	rts

L6:	movea.l   d1,a0
	swap      d0
	moveq.l   #$00,d1
	move.w    d0,d1
	clr.w     d0
	moveq.l   #$0f,d2
	add.l     d0,d0
	addx.l    d1,d1
L7:	sub.l     a0,d1
	bcc.b     L8
	add.l     a0,d1
L8:	addx.l    d0,d0
	addx.l    d1,d1
	dbra      d2,L7
	not.w     d0
	move.l    a1,d2
	bpl.b     L9
	neg.l     d0
L9:	rts

.endmod

.module _lmod:

	movea.l   d0,a1
	move.l    d0,d2
	bpl.b     L1
	neg.l     d0
L1:	tst.l     d1
	bpl.b     L2
	neg.l     d1
L2:	move.l    d1,d2
	swap      d2
	tst.w     d2
	bne.b     L6
	move.l    d0,d2
	swap      d2
	tst.w     d2
	bne.b     L4
	divu      d1,d0
	clr.w     d0
	swap      d0
	move.l    a1,d2
	bpl.b     L3
	neg.l     d0
L3:	rts

L4:	clr.w     d0
	swap      d0
	swap      d2
	divu      d1,d0
	move.w    d2,d0
	divu      d1,d0
	clr.w     d0
	swap      d0
	move.l    a1,d2
	bpl.b     L5
	neg.l     d0
L5:	rts

L6:	movea.l   d1,a0
	move.l    d0,d1
	clr.w     d0
	swap      d0
	swap      d1
	clr.w     d1
	moveq.l   #$0f,d2
	add.l     d1,d1
	addx.l    d0,d0
L7:	sub.l     a0,d0
	bcc.b     L8
	add.l     a0,d0
L8:	addx.l    d1,d1
	addx.l    d0,d0
	dbra      d2,L7
	roxr.l    #1,d0
	move.l    a1,d2
	bpl.b     L9
	neg.l     d0
L9:	rts

.endmod
