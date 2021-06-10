/*********************************************************************
*
* Mag!X 3.00
* ==========
*
* Strukturen fr die Einbindung eines XFS.
* Die korrekten Prototypen fr die Implementation eines XFS
* in 'C' folgen, so wie die 'C'- Schnittstelle fertig ist.
*
* Da z.T. mehrere Register fr Rckgabewerte verwendet werden und
* aužerdem Zeiger in Datenregistern bergeben werden, ist eine
* Schnittstelle als _cdecl geplant, d.h. s„mtliche Parameter werden
* auf dem Stapel bergeben, dies erm”glicht die Verwendung eines
* beliebigen Compilers.
*
* MagiC 3.00 onwards
* ==================
*
* Structures for binding in an XFS.
* The correct prototypes for the implementation of an XFS
* in 'C' will follow as soon as the 'C' interface is ready.
*
* As at times several registers are used for return values and
* in addition pointers are passed in data registers, an interface
* is planned as _cdecl, i.e. all parameters are passed on the stack.
* This permits the use of any desired compiler.
*
* Version: 3.10.94
*
*********************************************************************/

#ifndef __MX_XFSC_H__
#define __MX_XFSC_H__ 1

#include "mgx_dfsc.h"

typedef struct {
     WORD version;
     void (*fast_clrmem)      ( void *from, void *to ); /* parameters in register */
     char (*toupper)          ( char c );               /* parameters in register */
     void (*_sprintf)         ( char *dest, char *source, LONG *p );  /* parameters on stack */
     void *act_pd;
     APPL **act_appl;
     APPL **keyb_app;
     WORD *pe_slice;
/* 30 */     WORD *pe_timer;
     void (*appl_yield)       ( void );
     void (*appl_suspend)     ( void );
     void (*appl_begcritic)   ( void );
/* 46 */     void (*appl_endcritic)   ( void );
     long (*evnt_IO)          ( LONG ticks_50hz, void *unsel ); /* parameters in register */
     void (*evnt_mIO)         ( LONG ticks_50hz, void *unsel, WORD cnt ); /* parameters in register */
     void (*evnt_emIO)        ( APPL *ap ); /* parameters in register */
/* 62 */     void (*appl_IOcomplete)  ( APPL *ap ); /* parameters in register */
     long (*evnt_sem)         ( WORD mode, void *sem, LONG timeout ); /* parameters in register */
     void (*Pfree)            ( void *pd ); /* parameters in register */
     WORD int_msize;
/* 76 */     void *int_malloc         ( void );
     void int_mfree           ( void *memblk ); /* parameters in register */
     void resv_intmem         ( void *mem, LONG bytes ); /* parameters in register */
     LONG diskchange          ( WORD drv ); /* parameters in register */
/* 92 */     LONG DMD_rdevinit		( DMD *dmd ); /* parameters in register */
     LONG proc_info			( WORD code, PD *pd ); /* parameters in register */
	LONG mxalloc			( LONG amount, WORD mode, PD *pd ); /* parameters in register */
	LONG mfree			( void *block ); /* parameters in register */
	LONG mshrink			( void *block, LONG newlen ); /* parameters in register */
/* 112 */
} MX_KERNEL;


typedef struct _mx_dev {
     long      (*dev_close)(MX_FD *f);
     long      (*dev_read)();
     long      (*dev_write)();
     long      (*dev_stat)();
     long      (*dev_seek)();
     long      (*dev_datime)();
     long      (*dev_ioctl)();
     long      (*dev_getc)();
     long      (*dev_getline)();
     long      (*dev_putc)();
} MX_DEV;


typedef struct _mx_dd {
     struct _mx_dmd *dd_dmd;
     WORD      dd_refcnt;
} MX_DD;


typedef struct _mx_fd {
     struct _mx_dmd *fd_dmd;
     WORD      fd_refcnt;
     WORD      fd_mode;
     MX_DEV    *fd_dev;
} MX_FD;


typedef struct _mx_dhd {
     struct _mx_dmd *dhd_dmd;
} MX_DHD;


typedef struct _mx_dta {
     char      dta_res1[20];
     char      dta_drive;
     char      dta_attribute;
     WORD      dta_time;
     WORD      dta_date;
     ULONG     dta_len;
     char      dta_name[14];
} MX_DTA;


typedef struct _mx_dmd {
     struct _mx_xfs *d_xfs;
     WORD      d_drive;
     MX_DD     *d_root;
     WORD      biosdev;
     LONG      driver;
     LONG      devcode;
} MX_DMD;


typedef struct _mx_xfs {
     char      xfs_name[8];
     struct    _mx_xfs *xfs_next;
     ULONG     xfs_flags;
     long      (*xfs_init)();
     long      (*xfs_sync)();
     long      (*xfs_pterm)();
     long      (*xfs_garbcoll)();
     long      (*xfs_freeDD)();
     long      (*xfs_drv_open)();
     long      (*xfs_drv_close)();
     long      (*xfs_path2DD)();
     long      (*xfs_sfirst)();
     long      (*xfs_snext)();
     long      (*xfs_fopen)();
     long      (*xfs_fdelete)();
     long      (*xfs_link)();
     long      (*xfs_xattr)();
     long      (*xfs_attrib)();
     long      (*xfs_chown)();
     long      (*xfs_chmod)();
     long      (*xfs_dcreate)();
     long      (*xfs_ddelete)();
     long      (*xfs_DD2name)();
     long      (*xfs_dopendir)();
     long      (*xfs_dreaddir)();
     long      (*xfs_drewinddir)();
     long      (*xfs_dclosedir)();
     long      (*xfs_dpathconf)();
     long      (*xfs_dfree)();
     long      (*xfs_wlabel)();
     long      (*xfs_rlabel)();
     long      (*xfs_symlink)();
     long      (*xfs_readlink)();
     long      (*xfs_dcntl)();
} MX_XFS;

/* Write/Read modes for Fgetchar and Fputchar */

#define   CMODE_RAW      0
#define   CMODE_COOKED   1
#define   CMODE_ECHO     2

/* Open mode of files (MagiC-internal)                                    */
/* NOINHERIT is not supported, because in the TOS convention only the     */
/* handles 0..5 are inherited                                             */
/* High byte used as under MiNT                                           */

#define   OM_RPERM       1
#define   OM_WPERM       2
#define   OM_EXEC        4
#define   OM_APPEND      8
#define   OM_RDENY       16
#define   OM_WDENY       32
#define   OM_NOCHECK     64


/* Supported Dcntl modes (MagiC-specific!) */
#define   KER_GETINFO    0x0100
#define   KER_INSTXFS    0x0200
#define   KER_SETWBACK   0x0300
#define   DFS_GETINFO    0x1100
#define   DFS_INSTDFS    0x1200
#define   DEV_M_INSTALL  0xcd00

/* Supported Fcntl modes */
#define   FTRUNCATE      0x4604

/* Modes and codes for Dpathconf() (-> MiNT) */

#ifndef DP_MAXREQ
#define   DP_MAXREQ      -1
#define   DP_IOPEN       0
#define   DP_MAXLINKS    1
#define   DP_PATHMAX     2
#define   DP_NAMEMAX     3
#define   DP_ATOMIC      4
#define   DP_TRUNC       5
#define    DP_NOTRUNC    0
#define    DP_AUTOTRUNC  1
#define    DP_DOSTRUNC   2
#define   DP_CASE        6
#define    DP_CASESENS   0
#define    DP_CASECONV   1
#define    DP_CASEINSENS 2
#endif

/* For Psemaphore, the modes 0/1 are not supported at present */

#define   PSEM_CRGET     0
#define   PSEM_DESTROY   1
#define   PSEM_GET       2
#define   PSEM_RELEASE   3

#endif /* __MX_XFSC_H__ */