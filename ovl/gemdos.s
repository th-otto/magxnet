				.globl Fwrite
				MODULE	Fwrite
				movem.l d0-d1/a0/a2,-(a7)
				move.w	#$40,(a7)
				trap #1
				lea		12(a7),a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Fread
				MODULE	Fread
				movem.l d0-d1/a0/a2,-(a7)
				move.w	#$3F,(a7)
				trap #1
				lea		12(a7),a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Fclose
				MODULE	Fclose
				pea		(a2)
				move.w	d0,-(a7)
				move.w	#$3E,-(a7)
				trap #1
				addq.w	#4,a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Fcntl
				MODULE	Fcntl
				pea		(a2)
				move.w	d2,-(a7)
				move.l	d1,-(a7)
				move.w	d0,-(a7)
				move.w	#$104,-(a7)
				trap #1
				lea		10(a7),a7
				move.l	(a7)+,a2
				rts
				ENDMOD
				
				.globl Fopen
				MODULE	Fopen
				pea		(a2)
				move.w	d0,-(a7)
				pea		(a0)
				move.w	#$3D,-(a7)
				trap #1
				addq.w	#8,a7
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

