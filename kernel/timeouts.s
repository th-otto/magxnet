	.offset 0
tm_next:	.ds.l 1
tm_inuse:	.ds.b 1
	.ds.b 1
tm_head:

	.text

	.globl timeout_init
timeout_init:
	sub.l      d1,d0
	bcs.s      timeout1
	move.l     timeout_list,tm_next(a0)
	move.l     a0,timeout_list
	clr.b      tm_inuse(a0)
	adda.l     d1,a0
	bra.s      timeout_init
timeout1:
	rts

	.globl timeout_alloc
timeout_alloc:
	move.w     sr,d1
	ori.w      #0x0700,sr
	movea.l    timeout_list,a0
	bra.s      timeout_alloc2
timeout_alloc1:
	tst.b      tm_inuse(a0)
	beq.s      timeout_alloc3
	movea.l    tm_next(a0),a0
timeout_alloc2:
	move.l     a0,d0
	bne.s      timeout_alloc1
	illegal
timeout_alloc3:
	st         tm_inuse(a0)
	addq.l     #tm_head,a0
	move.w     d1,sr
	rts

timeout_free:
	subq.l     #2,a0
	tst.b      (a0)
	beq.s      timeout_free1
	clr.b      (a0)
	rts
timeout_free1:
	illegal
	rts


timeout_list:
	.dc.l timeout_sentinel
timeout_sentinel:
	.ds.b 28 /* sizeof(timeout_pool) */
