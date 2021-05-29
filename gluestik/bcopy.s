/*
 *	new version of bcopy, memcpy and memmove
 *	handles overlap, odd/even alignment
 *	uses movem to copy 256 bytes blocks faster.
 *	Alexander Lehmann	alexlehm@iti.informatik.th-darmstadt.de
 *	sortof inspired by jrbs bcopy
 */

	.text
	.even
	.globl bcopy
	.globl _bcopy

/* memcpy: */
	exg a0,a1
/*
 *	void bcopy( const void *src, void *dest, size_t length );
 *	void _bcopy( const void *src, void *dest, unsigned long length );
 *	return value not used (returns src)
 *	functions are aliased (except for HSC -- sb)
 */
bcopy:
_bcopy:
	/* a0 src, a1 dest, d0.l length */
	pea (a1)

	tst.l	d0	/* length */
	beq	exit		/* length==0? (size_t) */

	/* overlay ? */
	cmp.l	a0,a1
	bgt	top_down

	move.w	a0,d1		/* test for alignment */
	move.w	a1,d2
	eor.w	d2,d1
	btst	#0,d1		/* one odd one even ? */
	bne	slow_copy
	btst	#0,d2		/* both even ? */
	beq	both_even
	move.b	(a0)+,(a1)+	/* copy one byte, now we are both even */
	subq.l	#1,d0
both_even:
	/* BUG: d0[8-15] may contain garbage */
	move.b	d0,d1		/* save length less 256 */
	lsr.l	#8,d0		/* number of 256 bytes blocks */
	beq	less256
	movem.l	d1/d3-d7/a2-a6,-(a7)	/* d2 is already saved */
copy256:
	movem.l	(a0)+,d1-d7/a2-a6	/* copy 5*48+16=256 bytes */
	movem.l	d1-d7/a2-a6,(a1)
	movem.l	(a0)+,d1-d7/a2-a6
	movem.l	d1-d7/a2-a6,48(a1)
	movem.l	(a0)+,d1-d7/a2-a6
	movem.l	d1-d7/a2-a6,96(a1)
	movem.l	(a0)+,d1-d7/a2-a6
	movem.l	d1-d7/a2-a6,144(a1)
	movem.l	(a0)+,d1-d7/a2-a6
	movem.l	d1-d7/a2-a6,192(a1)
	movem.l	(a0)+,d1-d4
	movem.l	d1-d4,240(a1)
	lea	256(a1),a1		/* increment dest, src is already */
	subq.l	#1,d0
	bne	copy256 		/* next, please */
	movem.l	(sp)+,d1/d3-d7/a2-a6
less256:			/* copy 16 bytes blocks */
	move.b	d1,d0
	lsr.b	#2,d0		/* number of 4 bytes blocks */
	beq	less4		/* less that 4 bytes left */
	move.w	d0,d2
	subq.b	#1,d0
	lsr.b	#2,d0		/* number of 16 bytes blocks minus 1, if d2==0 */
	neg.w	d2
	and.w	#3,d2		/* d2 = number of bytes below 16 (-n)&3 */
	add.w	d2,d2		/* offset in code (movl two bytes) */
	jmp	2(pc,d2.w)	/* jmp into loop */
copy16:
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	dbra	d0,copy16
less4:
	btst	#1,d1
	beq	less2
	move.w	(a0)+,(a1)+
less2:
	btst	#0,d1
	beq	none
	move.b	(a0),(a1)
none:
exit_d2:
exit:
	move.l 	(sp)+,a0		/* return dest (for memcpy only) */
	rts

slow_copy:			/* byte by bytes copy */
	move.w	d0,d1
	neg.w	d1
	and.w	#7,d1		/* d1 = number of bytes blow 8 (-n)&7 */
	addq.l	#7,d0
	lsr.l	#3,d0		/* number of 8 bytes block plus 1, if d1!=0 */
	add.w	d1,d1		/* offset in code (move.b two bytes) */
	jmp	2(pc,d1.w)	/* jump into loop */
scopy:
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	subq.l	#1,d0
	bne	scopy
	bra	exit_d2

top_down:
	add.l	d0,a0		/* a0 byte after end of src */
	add.l	d0,a1		/* a1 byte after end of dest */

	move.w	a0,d1		/* exact the same as above, only with predec */
	move.w	a1,d2
	eor.w	d2,d1
	btst	#0,d1
	bne	slow_copy_d

	btst	#0,d2
	beq	both_even_d
	move.b	-(a0),-(a1)
	subq.l	#1,d0
both_even_d:
	/* BUG: d0[8-15] may contain garbage */
	move.b	d0,d1
	lsr.l	#8,d0
	beq	less256_d
	movem.l	d1/d3-d7/a2-a6,-(a7)
copy256_d:
	movem.l	-48(a0),d1-d7/a2-a6
	movem.l	d1-d7/a2-a6,-(a1)
	movem.l	-96(a0),d1-d7/a2-a6
	movem.l	d1-d7/a2-a6,-(a1)
	movem.l	-144(a0),d1-d7/a2-a6
	movem.l	d1-d7/a2-a6,-(a1)
	movem.l	-192(a0),d1-d7/a2-a6
	movem.l	d1-d7/a2-a6,-(a1)
	movem.l	-240(a0),d1-d7/a2-a6
	movem.l	d1-d7/a2-a6,-(a1)
	movem.l	-256(a0),d1-d4
	movem.l	d1-d4,-(a1)
	lea	-256(a0),a0
	subq.l	#1,d0
	bne	copy256_d
	movem.l	(sp)+,d1/d3-d7/a2-a6
less256_d:
	move.b	d1,d0
	lsr.b	#2,d0
	beq	less4_d
	move.w	d0,d2
	subq.b	#1,d0
	lsr.b	#2,d0
	neg.w	d2
	and.w	#3,d2
	add.w	d2,d2
	jmp	2(pc,d2.w)
copy16_d:
	move.l	-(a0),-(a1)
	move.l	-(a0),-(a1)
	move.l	-(a0),-(a1)
	move.l	-(a0),-(a1)
	dbra	d0,copy16_d
less4_d:
	btst	#1,d1
	beq	less2_d
	move.w	-(a0),-(a1)
less2_d:
	btst	#0,d1
	beq	exit_d2
	move.b	-(a0),-(a1)
	bra	exit_d2
slow_copy_d:
	move.w	d0,d1
	neg.w	d1
	and.w	#7,d1
	addq.l	#7,d0
	lsr.l	#3,d0
	add.w	d1,d1
	jmp	2(pc,d1.w)
scopy_d:
	move.b	-(a0),-(a1)
	move.b	-(a0),-(a1)
	move.b	-(a0),-(a1)
	move.b	-(a0),-(a1)
	move.b	-(a0),-(a1)
	move.b	-(a0),-(a1)
	move.b	-(a0),-(a1)
	move.b	-(a0),-(a1)
	subq.l	#1,d0
	bne	scopy_d
	bra	exit_d2
