				.globl Setexc
				MODULE	Setexc
				movem.l a0/a2/d0,-(a7)
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

				.globl Super
				MODULE	Super
				pea		(a2)
				pea		(a0)
				move.w	#$20,-(a7)
				trap #1
				addq.w	#6,a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Ptermres
				MODULE	Ptermres
				move.w	d1,-(a7)
				move.l	d0,-(a7)
				move.w	#$31,-(a7)
				trap #1
				ENDMOD

				.globl Pterm
				MODULE	Pterm
				move.w	d0,-(a7)
				move.w	#$4C,-(a7)
				trap #1
				ENDMOD

				.globl _Mshrink
				MODULE	_Mshrink
				pea		(a2)				
				move.l	d1,-(a7)
				pea		(a0)
				move.w	d0,-(a7)
				move.w	#$4A,-(a7)
				trap	#1
				lea		12(a7),a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Mfree
				MODULE	Mfree
				pea		(a2)
				pea		(a0)
				move.w	#$49,-(a7)
				trap #1
				addq.w	#6,a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Fwrite
				MODULE	Fwrite
				movem.l d0-d1/a0/a2,-(a7)
				move.w	#$40,(a7)
				trap #1
				lea		12(a7),a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Fsetdta
				MODULE	Fsetdta
				pea		(a2)
				pea		(a0)
				move.w	#$1A,-(a7)
				trap #1
				addq.w	#6,a7
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

				.globl Fgetdta
				MODULE	Fgetdta
				pea		(a2)
				move.w	#$2F,-(a7)
				trap #1
				addq.w	#2,a7
				movea.l	d0,a0
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

				.globl Dsetdrv
				MODULE	Dsetdrv
				pea		(a2)
				move.w	d0,-(a7)
				move.w	#$0E,-(a7)
				trap #1
				addq.w	#4,a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Dgetdrv
				MODULE	Dgetdrv
				pea		(a2)
				move.w	#$19,-(a7)
				trap #1
				addq.w	#2,a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Cnecin
				MODULE	Cnecin
				pea		(a2)
				move.w	#$08,-(a7)
				trap #1
				addq.w	#2,a7
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
				
				.globl Pkill
				MODULE	Pkill
				pea		(a2)
				move.w	d1,-(a7)
				move.w	d0,-(a7)
				move.w	#$111,-(a7)
				trap #1
				addq.w	#6,a7
				move.l	(a7)+,a2
				rts
				ENDMOD
				
				.globl Psigblock
				MODULE	Psigblock
				pea		(a2)
				move.l	d0,-(a7)
				move.w	#$116,-(a7)
				trap #1
				addq.w	#6,a7
				move.l	(a7)+,a2
				rts
				ENDMOD
				
				.globl Pexec
				MODULE	Pexec
				pea		(a2)
				move.l	8(a7),-(a7)
				pea		(a1)
				pea		(a0)
				move.w	d0,-(a7)
				move.w	#$4B,-(a7)
				trap #1
				lea		16(a7),a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Fsnext
				MODULE	Fsnext
				pea		(a2)
				move.w	#$4F,-(a7)
				trap #1
				addq.w	#2,a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Fsfirst
				MODULE	Fsfirst
				pea		(a2)
				move.w	d0,-(a7)
				pea		(a0)
				move.w	#$4E,-(a7)
				trap #1
				addq.w	#8,a7
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

				.globl Fdelete
				MODULE	Fdelete
				pea		(a2)
				pea		(a0)
				move.w	#$41,-(a7)
				trap #1
				addq.w	#6,a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Dsetpath
				MODULE	Dsetpath
				pea		(a2)
				pea		(a0)
				move.w	#$3B,-(a7)
				trap #1
				addq.w	#6,a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Dgetpath
				MODULE	Dgetpath
				pea		(a2)
				move.w	d0,-(a7)
				pea		(a0)
				move.w	#$47,-(a7)
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

				.globl Dcntl
				MODULE	Dcntl
				pea		(a2)
				move.l	d1,-(a7)
				pea		(a0)
				move.w	d0,-(a7)
				move.w	#$130,-(a7)
				trap #1
				lea		12(a7),a7
				move.l	(a7)+,a2
				rts
				ENDMOD
				
				.globl Fchmod
				MODULE	Fchmod
				pea		(a2)
				move.w	d0,-(a7)
				pea		(a0)
				move.w	#$132,-(a7)
				trap #1
				addq.w	#8,a7
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

				.globl Pgeteuid
				MODULE	Pgeteuid
				pea		(a2)
				move.w	#$138,-(a7)
				trap #1
				addq.w	#2,a7
				move.l	(a7)+,a2
				rts
				ENDMOD
				
				.globl Pgetpid
				MODULE	Pgetpid
				pea		(a2)
				move.w	#$10b,-(a7)
				trap #1
				addq.w	#2,a7
				move.l	(a7)+,a2
				rts
				ENDMOD
				
