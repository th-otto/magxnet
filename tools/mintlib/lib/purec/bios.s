	.INCLUDE 'osmacros.s'
	
	.EXPORT Getmpb,Bconstat,Bconin,Bconout,Rwabs,Setexc
	.EXPORT Tickcal,Getbpb,Bcostat,Mediach,Drvmap,Kbshift

; void    Getmpb( _MPB *ptr );
.MODULE Getmpb:
	SYS_L	BIOS,#$0,a0
	rts

	.ENDMOD

; int     Bconstat( int dev );	
.MODULE Bconstat:
	SYS_W	BIOS,#$1,d0
	rts

	.ENDMOD

; long    Bconin( int dev );	
.MODULE Bconin:
	SYS_W	BIOS,#$2,d0
	rts

	.ENDMOD

; long    Bconout( int dev, int c );	
.MODULE Bconout:
	SYS_WW	BIOS,#$3,d0,d1
	rts

	.ENDMOD

; long    Rwabs( int rwflag, void *buf, int cnt, int recnr, int dev );	
; pass one extra arg from stack:  lrecno
.MODULE Rwabs:
	pea	(a2)
	move.l	10(sp),-(sp)
	move.w	12(sp),-(sp)
	move.w	d2,-(sp)
	move.w	d1,-(sp)
	pea	(a0)
	move.w	d0,-(sp)
	move.w	#$4,-(sp)
	trap	#BIOS
	lea	18(sp),sp
	movea.l	(sp)+,a2
	rts

	.ENDMOD

; void    (*Setexc( int number, void (*exchdlr)() )) ();	
.MODULE Setexc:
	SYS_WL	BIOS,#$5,d0,a0
	movea.l	d0,a0
	rts

	.ENDMOD

; long    Tickcal( void );	
.MODULE Tickcal:
	SYS_	BIOS,#$6
	rts

	.ENDMOD

; _BPB    *Getbpb( int dev );	
.MODULE Getbpb:
	SYS_W	BIOS,#$7,d0
	movea.l	d0,a0
	rts

	.ENDMOD

; long    Bcostat( int dev );	
.MODULE Bcostat:
	SYS_W	BIOS,#$8,d0
	rts

	.ENDMOD

; long    Mediach( int dev );	
.MODULE Mediach:
	SYS_W	BIOS,#$9,d0
	rts

	.ENDMOD

; long    Drvmap( void );	
.MODULE Drvmap:
	SYS_	BIOS,#$a
	rts

	.ENDMOD

; long    Kbshift( int mode );	
.MODULE Kbshift:
	SYS_W	BIOS,#$b,d0
	rts

	.ENDMOD

