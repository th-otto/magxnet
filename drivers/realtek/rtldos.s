				.globl Setexc
				MODULE	Setexc
				movem.l d0/a0/a2,-(a7)
				move.w	#5,(a7)
				trap #13
				addq.w	#8,a7
				movea.l	d0,a0
				movea.l	(a7)+,a2
				rts
				ENDMOD

				.globl Bconout
				MODULE	Bconout
				pea     (a2)
				move.w	d1,-(a7)
				move.w	d0,-(a7)
				move.w	#3,-(a7)
				trap #13
				addq.w	#6,a7
				movea.l	(a7)+,a2
				rts
				ENDMOD

				.globl Mfpint
				MODULE	Mfpint				
				movem.l d0/a0/a2,-(a7)
				move.w	#13,(a7)
				trap #14
				addq.w	#8,a7
				movea.l	(a7)+,a2
				rts
				ENDMOD
