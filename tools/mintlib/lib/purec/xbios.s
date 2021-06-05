	.INCLUDE 'osmacros.s'
	
	.EXPORT Initmous,Ssbrk,Physbase,Logbase,Getrez,Setscreen,Setpallete
	.EXPORT Setcolor,Floprd,Flopwr,Flopfmt,Midiws,Mfpint,Iorec,Rsconf
	.EXPORT Keytbl,Random,Protobt,Flopver,Scrdmp,Cursconf,Settime,Gettime
	.EXPORT Bioskeys,Ikbdws,Jdisint,Jenabint,Giaccess,Offgibit,Ongibit
	.EXPORT Xbtimer,Dosound,Setprt,Kbdvbase,Kbrate,Prtblk,Vsync,Supexec
	.EXPORT Puntaes,Floprate,Blitmode,DMAread,DMAwrite,Bconmap,NVMaccess
	.EXPORT Esetshift,Egetshift,EsetBank,EsetColor,EsetPallete,EgetPallete
	.EXPORT EsetGray,EsetSmear

; void    Initmous( int type, _PARAM *par, void (*mousevec)() );
.MODULE Initmous:
	SYS_WLL	XBIOS,#$0,d0,a0,a1
	rts

	.ENDMOD

; void    *Ssbrk( int count );
.MODULE Ssbrk:
	SYS_W	XBIOS,#$1,d0
	movea.l	d0,a0
	rts

	.ENDMOD

; void    *Physbase( void );
.MODULE Physbase:
	SYS_	XBIOS,#$2
	movea.l	d0,a0
	rts

	.ENDMOD

; void    *Logbase( void );
.MODULE Logbase:
	SYS_	XBIOS,#$3
	movea.l	d0,a0
	rts

	.ENDMOD

; int     Getrez( void );
.MODULE Getrez:
	SYS_	XBIOS,#$4
	rts

	.ENDMOD

; void    Setscreen( void *laddr, void *paddr, int rez );
.MODULE Setscreen:
	SYS_LLW	XBIOS,#$5,a0,a1,d0
	rts

	.ENDMOD

; void    Setpalette( void *pallptr );
.MODULE Setpallete:
	SYS_L	XBIOS,#$6,a0
	rts

	.ENDMOD

; int     Setcolor( int colornum, int color );
.MODULE Setcolor:
	SYS_WW	XBIOS,#$7,d0,d1
	rts

	.ENDMOD

; int     Floprd( void *buf, long filler, int devno, int sectno,
;                int trackno, int sideno, int count );
.MODULE Floprd:
	pea	(a2)
	move.w	12(sp),-(sp)
	move.l	10(sp),-(sp)
	move.w	d2,-(sp)
	move.w	d1,-(sp)
	move.l	d0,-(sp)
	pea	(a0)
	move.w	#$8,-(sp)
	trap	#XBIOS
	lea	20(sp),sp
	movea.l	(sp)+,a2
	rts

	.ENDMOD

; int     Flopwr( void *buf, long filler, int devno, int sectno,
;                int trackno, int sideno, int count );
.MODULE Flopwr:
	pea	(a2)
	move.w	12(sp),-(sp)
	move.l	10(sp),-(sp)
	move.w	d2,-(sp)
	move.w	d1,-(sp)
	move.l	d0,-(sp)
	pea	(a0)
	move.w	#$9,-(sp)
	trap	#XBIOS
	lea	20(sp),sp
	movea.l	(sp)+,a2
	rts

	.ENDMOD

; int     Flopfmt( void *buf, long filler, int devno, int spt, int trackno,
;                 int sideno, int interlv, long magic, int virgin );
.MODULE Flopfmt:
	pea	(a2)
	move.l	16(sp),-(sp)
	move.l	16(sp),-(sp)
	move.l	16(sp),-(sp)
	move.w	d2,-(sp)
	move.w	d1,-(sp)
	move.l	d0,-(sp)
	pea	(a0)
	move.w	#$a,-(sp)
	trap	#XBIOS
	lea	26(sp),sp
	movea.l	(sp)+,a2
	rts

	.ENDMOD

; void    Midiws( int cnt, void *ptr );
.MODULE Midiws:
	SYS_WL	XBIOS,#$c,d0,a0
	rts

	.ENDMOD

; void    Mfpint( int erno, void (*vector)() );
.MODULE Mfpint:
	SYS_WL	XBIOS,#$d,d0,a0
	rts

	.ENDMOD

; _IOREC   *Iorec( int dev );
.MODULE Iorec:
	SYS_W	XBIOS,#$e,d0
	movea.l	d0,a0
	rts

	.ENDMOD

; long    Rsconf( int baud, int ctr, int ucr, int rsr, int tsr, int scr );
.MODULE Rsconf:
	pea	(a2)
	move.w	12(sp),-(sp)
	move.l	10(sp),-(sp)
	movem.w	d0-d2,-(sp)
	move.w	#$f,-(sp)
	trap	#XBIOS
	lea	14(sp),sp
	movea.l	(sp)+,a2
	rts

	.ENDMOD

; _KEYTAB  *Keytbl( void *unshift, void *shift, void *capslock );
.MODULE Keytbl:
	SYS_LLL	XBIOS,#$10,a0,a1,REGSIZE+4(sp)
	movea.l	d0,a0
	rts

	.ENDMOD

; long    Random( void );
.MODULE Random:
	SYS_	XBIOS,#$11
	rts

	.ENDMOD

; void    Protobt( void *buf, long serialno, int disktype, int execflag );
.MODULE Protobt:
	SYS_LLWW	XBIOS,#$12,a0,d0,d1,d2
	rts

	.ENDMOD

; int     Flopver( void *buf, long filler, int devno, int sectno,
;                 int trackno, int sideno, int count );
.MODULE Flopver:
	pea	(a2)
	move.w	12(sp),-(sp)
	move.l	10(sp),-(sp)
	move.w	d2,-(sp)
	move.w	d1,-(sp)
	move.l	d0,-(sp)
	pea	(a0)
	move.w	#$13,-(sp)
	trap	#XBIOS
	lea	20(sp),sp
	movea.l	(sp)+,a2
	rts

	.ENDMOD

; void    Scrdmp( void );
.MODULE Scrdmp:
	SYS_	XBIOS,#$14
	rts

	.ENDMOD

; int     Cursconf( int func, int rate );
.MODULE Cursconf:
	SYS_WW	XBIOS,#$15,d0,d1
	rts

	.ENDMOD

; void    Settime( unsigned long time );
.MODULE Settime:
	SYS_L	XBIOS,#$16,d0
	rts

	.ENDMOD

; unsigned long  Gettime( void );
.MODULE Gettime:
	SYS_	XBIOS,#$17
	rts

	.ENDMOD

; void    Bioskeys( void );
.MODULE Bioskeys:
	SYS_	XBIOS,#$18
	rts

	.ENDMOD

; void    Ikbdws( int count, void *ptr );
.MODULE Ikbdws:
	SYS_WL	XBIOS,#$19,d0,a0
	rts

	.ENDMOD

; void    Jdisint( int number );
.MODULE Jdisint:
	SYS_W	XBIOS,#$1a,d0
	rts

	.ENDMOD

; void    Jenabint( int number );
.MODULE Jenabint:
	SYS_W	XBIOS,#$1b,d0
	rts

	.ENDMOD

; char    Giaccess( char data, int regno );
.MODULE Giaccess:
	SYS_WW	XBIOS,#$1c,d0,d1
	rts

	.ENDMOD

; void    Offgibit( int bitno );
.MODULE Offgibit:
	SYS_W	XBIOS,#$1d,d0
	rts

	.ENDMOD

; void    Ongibit( int bitno );
.MODULE Ongibit:
	SYS_W	XBIOS,#$1e,d0
	rts

	.ENDMOD

; void    Xbtimer( int timer, int control, int data, void (*vector)() );
.MODULE Xbtimer:
	SYS_WWWL	XBIOS,#$1f,d0,d1,d2,a0
	rts

	.ENDMOD

; void    *Dosound( void *buf );
.MODULE Dosound:
	SYS_L	XBIOS,#$20,a0
	movea.l	d0,a0
	rts

	.ENDMOD

; int     Setprt( int config );
.MODULE Setprt:
	SYS_W	XBIOS,#$21,d0
	rts

	.ENDMOD

; _KBDVECS *Kbdvbase( void );
.MODULE Kbdvbase:
	SYS_	XBIOS,#$22
	movea.l	d0,a0
	rts

	.ENDMOD

; int     Kbrate( int initial, int repeat );
.MODULE Kbrate:
	SYS_WW	XBIOS,#$23,d0,d1
	rts

	.ENDMOD

; void    Prtblk( _PBDEF *par );
.MODULE Prtblk:
	SYS_L	XBIOS,#$24,a0
	rts

	.ENDMOD

; void    Vsync( void );
.MODULE Vsync:
	SYS_	XBIOS,#$25
	rts

	.ENDMOD

; long    Supexec( long (*func)() );
.MODULE Supexec:
	SYS_L	XBIOS,#$26,a0
	rts

	.ENDMOD

; void    Puntaes( void );
.MODULE Puntaes:
	SYS_	XBIOS,#$27
	rts

	.ENDMOD

; int     Floprate( int devno, int newrate );
.MODULE Floprate:
	SYS_WW	XBIOS,#$29,d0,d1
	rts

	.ENDMOD

; int     Blitmode( int mode );
.MODULE Blitmode:
	SYS_W	XBIOS,#$40,d0
	rts

	.ENDMOD

; int     DMAread( long sector, int count, void *buffer, int devno );
.MODULE DMAread:
	SYS_LWLW	XBIOS,#$2a,d0,d1,a0,d2
	rts

	.ENDMOD

; int     DMAwrite( long sector, int count, void *buffer, int devno );
.MODULE DMAwrite:
	SYS_LWLW	XBIOS,#$2b,d0,d1,a0,d2
	rts

	.ENDMOD

; long    Bconmap( int devno );
.MODULE Bconmap:
	SYS_W	XBIOS,#$2c,d0
	rts

	.ENDMOD

; int     NVMaccess( int opcode, int start, int count, void *buffer );
.MODULE NVMaccess:
	SYS_WWWL	XBIOS,#$2e,d0,d1,d2,a0
	rts

	.ENDMOD

; int     Esetshift( int shftMode );
.MODULE Esetshift:
	SYS_W	XBIOS,#$50,d0
	rts

	.ENDMOD

; int     Egetshift( void );
.MODULE Egetshift:
	SYS_	XBIOS,#$51
	rts

	.ENDMOD

; int     EsetBank( int bankNum );
.MODULE EsetBank:
	SYS_W	XBIOS,#$52,d0
	rts

	.ENDMOD

; int     EsetColor( int colorNum, int color );
.MODULE EsetColor:
	SYS_WW	XBIOS,#$53,d0,d1
	rts

	.ENDMOD

; void    EsetPalette( int colorNum, int count, int *palettePtr );
.MODULE EsetPallete:
	SYS_WWL	XBIOS,#$54,d0,d1,a0
	rts

	.ENDMOD

; void    EgetPalette( int colorNum, int count, int *palettePtr );
.MODULE EgetPallete:
	SYS_WWL	XBIOS,#$55,d0,d1,a0
	rts

	.ENDMOD

; int     EsetGray( int swtch );
.MODULE EsetGray:
	SYS_W	XBIOS,#$56,d0
	rts

	.ENDMOD

; int     EsetSmear( int swtch );
.MODULE EsetSmear:
	SYS_W	XBIOS,#$57,d0
	rts

	.ENDMOD

