	.INCLUDE 'osmacros.s'
	
	.EXPORT	Pterm0,Cconin,Cconout,Cauxin,Cauxout,Cprnout,Crawio
	.EXPORT Crawcin,Cnecin,Cconws,Cconrs,Cconis,Dsetdrv,Cconos
	.EXPORT Cprnos,Cauxis,Cauxos,Dgetdrv,Fsetdta,Super
	.EXPORT Tgetdate,Tsetdate,Tgettime,Tsettime,Fgetdta,Sversion
	.EXPORT Ptermres,Dfree,Dcreate,Ddelete,Dsetpath,Fcreate,Fopen
	.EXPORT Fclose,Fread,Fwrite,Fdelete,Fseek,Fattrib,Fdup,Fforce
	.EXPORT Dgetpath,Malloc,Mfree,Mshrink,Pexec,Pterm,Fsfirst
	.EXPORT Fsnext,Frename,Fdatime,Mxalloc,Maddalt,Flock

; void    Pterm0( void );
.MODULE Pterm0:
	clr.w	-(sp)
	trap	#GEMDOS

	.ENDMOD

; long    Cconin( void );
.MODULE Cconin:
	SYS_	GEMDOS,#$1
	rts
	
	.ENDMOD

; void    Cconout( int c );
.MODULE Cconout:
	SYS_W	GEMDOS,#$2,d0
	rts
	
	.ENDMOD

; int     Cauxin( void );
.MODULE Cauxin:
	SYS_	GEMDOS,#$3
	rts
	
	.ENDMOD

; void    Cauxout( int c );
.MODULE Cauxout:
	SYS_W	GEMDOS,#$4,d0
	rts
	
	.ENDMOD

; int     Cprnout( int c );
.MODULE Cprnout:
	SYS_W	GEMDOS,#$5,d0
	rts
	
	.ENDMOD

; long    Crawio( int w );
.MODULE Crawcio:
	SYS_W	GEMDOS,#$6,d0
	rts
	
	.ENDMOD

; long    Crawcin( void );
.MODULE Crawcin:
	SYS_	GEMDOS,#$7
	rts
	
	.ENDMOD

; long    Cnecin( void );
.MODULE Cnecin:
	SYS_	GEMDOS,#$8
	rts
	
	.ENDMOD

; int     Cconws( const char *buf );
.MODULE Cconws:
	SYS_L	GEMDOS,#$9,a0
	rts
	
	.ENDMOD

; void    Cconrs( LINE *buf );
.MODULE Cconrs:
	SYS_L	GEMDOS,#$A,a0
	rts
	
	.ENDMOD

; int     Cconis( void );
.MODULE Cconis:
	SYS_	GEMDOS,#$B
	rts
	
	.ENDMOD

; long    Dsetdrv( int drv );
.MODULE Dsetdrv:
	SYS_W	GEMDOS,#$E,d0
	rts
	
	.ENDMOD

; int     Cconos( void );
.MODULE Cconos:
	SYS_	GEMDOS,#$10
	rts
	
	.ENDMOD

; int     Cprnos( void );
.MODULE Cprnos:
	SYS_	GEMDOS,#$11
	rts
	
	.ENDMOD

; int     Cauxis( void );
.MODULE Cauxis:
	SYS_	GEMDOS,#$12
	rts
	
	.ENDMOD

; int     Cauxos( void );
.MODULE Cauxos:
	SYS_	GEMDOS,#$13
	rts
	
	.ENDMOD

; int     Dgetdrv( void );
.MODULE Dgetdrv:
	SYS_	GEMDOS,#$19
	rts
	
	.ENDMOD

; void    Fsetdta( _DTA *buf );
.MODULE Fsetdta:
	SYS_L	GEMDOS,#$1a,a0
	rts
	
	.ENDMOD

; long    Super( void *stack );
.MODULE Super:
	SYS_L	GEMDOS,#$20,a0
	rts
	
	.ENDMOD

; unsigned int  Tgetdate( void );
.MODULE Tgetdate:
	SYS_	GEMDOS,#$2a
	rts
	
	.ENDMOD

; unsigned int  Tsetdate( unsigned int date );
.MODULE Tsetdate:
	SYS_W	GEMDOS,#$2b,d0
	rts
	
	.ENDMOD

; unsigned int  Tgettime( void );
.MODULE Tgettime:
	SYS_	GEMDOS,#$2c
	rts
	
	.ENDMOD

; unsigned int  Tsettime( unsigned int time );
.MODULE Tsettime:
	SYS_W	GEMDOS,#$2d,d0
	rts
	
	.ENDMOD

; _DTA    *Fgetdta( void );
.MODULE Fgetdta:
	SYS_	GEMDOS,#$2f
	movea.l	d0,a0
	rts
	
	.ENDMOD

; int     Sversion( void );
.MODULE Sversion:
	SYS_	GEMDOS,#$30
	rts
	
	.ENDMOD

; void    Ptermres( long keepcnt, int retcode );
.MODULE Ptermres:
	SYS_LW	GEMDOS,#$31,d0,d1
	rts
	
	.ENDMOD

; int     Dfree( _DISKINFO *buf, int driveno );
.MODULE Dfree:
	SYS_LW	GEMDOS,#$36,a0,d0
	rts
	
	.ENDMOD

; int     Dcreate( const char *path );
.MODULE Dcreate:
	SYS_L	GEMDOS,#$39,a0
	rts
	
	.ENDMOD

; int     Ddelete( const char *path );
.MODULE Ddelete:
	SYS_L	GEMDOS,#$3a,a0
	rts
	
	.ENDMOD

; int     Dsetpath( const char *path );
.MODULE Dsetpath:
	SYS_L	GEMDOS,#$3b,a0
	rts
	
	.ENDMOD

; long    Fcreate( const char *filename, int attr );
.MODULE Fcreate:
	SYS_LW	GEMDOS,#$3c,a0,d0
	rts
	
	.ENDMOD

; long    Fopen( const char *filename, int mode );
.MODULE Fopen:
	SYS_LW	GEMDOS,#$3d,a0,d0
	rts
	
	.ENDMOD

; int     Fclose( int handle );
.MODULE Fclose:
	SYS_W	GEMDOS,#$3e,d0
	rts
	
	.ENDMOD

; long    Fread( int handle, long count, void *buf );
.MODULE Fread:
	SYS_WLL	GEMDOS,#$3f,d0,d1,a0
	rts
	
	.ENDMOD

; long    Fwrite( int handle, long count, void *buf );
.MODULE Fwrite:
	SYS_WLL	GEMDOS,#$40,d0,d1,a0
	rts
	
	.ENDMOD

; int     Fdelete( const char *filename );
.MODULE Fdelete:
	SYS_L	GEMDOS,#$41,a0
	rts
	
	.ENDMOD

; long    Fseek( long offset, int handle, int seekmode );
.MODULE Fseek:
	SYS_LWW	GEMDOS,#$42,d0,d1,d2
	rts
	
	.ENDMOD

; int     Fattrib( const char *filename, int wflag, int attrib );
.MODULE Fattrib:
	SYS_LWW	GEMDOS,#$43,a0,d0,d1
	rts
	
	.ENDMOD

; long    Fdup( int handle );
.MODULE Fdup:
	SYS_W	GEMDOS,#$45,d0
	rts
	
	.ENDMOD

; long    Fforce( int stch, int nonstdh );
.MODULE Fforce:
	SYS_WW	GEMDOS,#$46,d0,d1
	rts
	
	.ENDMOD

; int     Dgetpath( char *path, int driveno );
.MODULE Dgetpath:
	SYS_LW	GEMDOS,#$47,a0,d0
	rts
	
	.ENDMOD

; void    *Malloc( long number );
.MODULE Malloc:
	SYS_L	GEMDOS,#$48,d0
	movea.l	d0,a0
	rts
	
	.ENDMOD

; int     Mfree( void *block );
.MODULE Mfree:
	SYS_L	GEMDOS,#$49,a0
	rts
	
	.ENDMOD

; int     Mshrink( int zero, void *ptr, long size );
.MODULE Mshrink:
	SYS_WLL	GEMDOS,#$4a,d0,a0,d1
	rts
	
	.ENDMOD

; long    Pexec( int mode, char *ptr1, void *ptr2, void *ptr3 );
.MODULE Pexec:
	SYS_WLLL	GEMDOS,#$4b,d0,a0,a1,REGSIZE+4(sp)
	rts
	
	.ENDMOD

; void    Pterm( int retcode );
.MODULE Pterm:
	move.w	d0,-(sp)
	move.w	#$4c,-(sp)
	trap	#GEMDOS
	
	.ENDMOD

; int     Fsfirst( const char *filename, int attr );
.MODULE Fsfirst:
	SYS_LW	GEMDOS,#$4e,a0,d0
	rts
	
	.ENDMOD

; int     Fsnext( void );
.MODULE Fsnext:
	SYS_	GEMDOS,#$4f
	rts
	
	.ENDMOD

; int     Frename( int zero, const char *oldname, const char *newname );
.MODULE Frename:
	SYS_WLL	GEMDOS,#$56,d0,a0,a1
	rts
	
	.ENDMOD

; int     Fdatime( _DOSTIME *timeptr, int handle, int wflag );
.MODULE Fdatime:
	SYS_LWW	GEMDOS,#$57,a0,d0,d1
	rts
	
	.ENDMOD

; void    *Mxalloc( long number, int mode );
.MODULE Mxalloc:
	SYS_LW	GEMDOS,#$44,d0,d1
	movea.l	d0,a0
	rts
	
	.ENDMOD

; long    Maddalt( void *start, long size );
.MODULE Maddalt:
	SYS_LL	GEMDOS,#$14,a0,d0
	rts
	
	.ENDMOD

; long	Flock( int handle, int mode, long start, long length );
.MODULE Flock:
	SYS_WWLL	GEMDOS,#$5c,d0,d1,d2,REGSIZE+4(sp)
	rts
	
	.ENDMOD

