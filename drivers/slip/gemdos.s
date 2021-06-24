				.globl _gemdos
				MODULE	_gemdos
				move.l	(a7)+,save_pc
				move.l	a2,save_a2
				trap #1
				move.l	save_a2,a2
				move.l	save_pc,a1
				jmp (a1)
				BSS
save_pc:		ds.l	1
save_a2:		ds.l	1
				TEXT
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

				.globl Tgettime
				MODULE	Tgettime
				pea		(a2)
				move.w	#$2C,-(a7)
				trap #1
				addq.w	#2,a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Tgetdate
				MODULE	Tgetdate
				pea		(a2)
				move.w	#$2A,-(a7)
				trap #1
				addq.w	#2,a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Cconws
				MODULE	Cconws
				pea		(a2)
				pea		(a0)
				move.w	#$09,-(a7)
				trap #1
				addq.w	#6,a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Supexec
				MODULE	Supexec				
				pea     (a2)
				pea     (a0)
				move.w	#$26,-(a7)
				trap #14
				addq.w	#6,a7
				movea.l	(a7)+,a2
				rts
				ENDMOD

