/*
 *
 * Strukturen und Konstanten fuer den Assembler-Teil
 * eines MagiX-XFS
 *
 * Version: Andreas Kromke, 23.5.97
 *
 */

	.IFEQ GNUC
MSHORT = 1
	.ENDC

     .OFFSET 0

mxk_version:        DS.W      1
mxk_fast_clrmem:    DS.L      1
mxk_toupper:        DS.L      1
mxk__sprintf:       DS.L      1
mxk_act_pd:         DS.L      1
mxk_act_appl:       DS.L      1
mxk_keyb_app:       DS.L      1
mxk_pe_slice:       DS.L      1
mxk_pe_timer:       DS.L      1
mxk_appl_yield:     DS.L      1
mxk_appl_suspend:   DS.L      1
mxk_appl_begcritic: DS.L      1
mxk_appl_endcritic: DS.L      1
mxk_evnt_IO:        DS.L      1
mxk_evnt_mIO:       DS.L      1
mxk_evnt_emIO:      DS.L      1
mxk_appl_IOcomplete:DS.L      1
mxk_evnt_sem:       DS.L      1
mxk_Pfree:          DS.L      1
mxk_int_msize:      DS.W      1
mxk_int_malloc:     DS.L      1
mxk_int_mfree:      DS.L      1
mxk_resv_intmem:    DS.L      1
mxk_diskchange:     DS.L      1
mxk_DMD_rdevinit:	DS.L      1
mxk_ker_proc_info:	DS.L      1
mxk_ker_mxalloc:	DS.L      1
mxk_ker_mfree:		DS.L      1
mxk_ker_mshrink:	DS.L      1

	.OFFSET 0

dfsk_version:		DS.W	1
dfsk_dir_srch:		DS.L	1
dfsk_reopen_FD:	DS.L	1
dfsk_close_DD:		DS.L	1
dfsk_match_8_3:	DS.L	1
dfsk_conv_8_3:		DS.L	1
dfsk_init_DTA:		DS.L	1
dfsk_rcnv_8_3:		DS.L	1

     .OFFSET 0

dev_close:     DS.L      1
dev_read:      DS.L      1
dev_write:     DS.L      1
dev_stat:      DS.L      1
dev_seek:      DS.L      1
dev_datime:    DS.L      1
dev_ioctl:     DS.L      1
dev_getc:      DS.L      1
dev_getline:   DS.L      1
dev_putc:      DS.L      1

     .OFFSET 0

dd_dmd:        DS.L      1    /* 0x00: Zeiger auf DMD                      */
dd_refcnt:     DS.W      1    /* 0x04: Ref.zaehler fuer Standardpfade        */

     .OFFSET 0

fd_dmd:        DS.L      1    /* 0x00: Zeiger auf DMD                      */
fd_refcnt:     DS.W      1    /* 0x04: Ref.zaehler fuers Schliessen oder -1   */
fd_mode:       DS.W      1    /* 0x06: Open- Modus (0,1,2) und flags       */
fd_dev:        DS.L      1    /* 0x08: Zeiger auf MX_DEV                   */

     .OFFSET 0

dhd_dmd:       DS.L      1    /* 0x00: Zeiger auf DMD                      */

     .OFFSET 0

dta_res1:      DS.B      20
dta_drive:     DS.B      1    /* 0x14: */
dta_res2:      DS.B      1    /* 0x15: */
dta_time:      DS.W      1    /* 0x16: gefundene Zeit                      */
dta_date:      DS.W      1    /* 0x18: gefundenes Datum                    */
dta_len:       DS.L      1    /* 0x1a: gefundene Laenge                     */
dta_name:      DS.B      14   /* 0x1e: gefundener Dateiname                */

     .OFFSET 0

d_xfs:         DS.L      1    /* 0x00: Der Dateisystemtreiber              */
d_drive:       DS.W      1    /* 0x04: Laufwerknummer 0..31                */
d_root:        DS.L      1    /* 0x06: Zeiger auf FD der Root              */
d_biosdev:     DS.W      1    /* 0x0a: BIOS-Laufwerk oder -1               */
d_driver:      DS.L      1    /* 0x0c: def. zus. mit devcode das Medium    */
d_devcode:     DS.L      1    /* 0x10: z.B. SCSI Target & Laufwerk         */

     .OFFSET 0

xattr_mode:    DS.W      1    /* %ttttsssrwxrwxrwx                         */
                              /* Bit 12,13,14,15: Dateityp                 */
                              /*         2: BIOS special file              */
                              /*         4: Directory file                 */
                              /*         8: regular file                   */
                              /*        10: fifo                           */
                              /*        12: memory region or process       */
                              /*        14: symbolic link                  */
                              /* Bit 9,10,11: special bits                 */
                              /*         1: sticky bit (?)                 */
                              /*         2: setgid (?)                     */
                              /*         4: setuid (?)                     */
                              /* Bit 0..8: access modes                    */
                              /*         rwx fuer user/group/world          */
xattr_index:   DS.L      1
xattr_dev:     DS.W      1
xattr_res1:    DS.W      1
xattr_nlink:   DS.W      1
xattr_uid:     DS.W      1
xattr_gid:     DS.W      1
xattr_size:    DS.L      1
xattr_blksize: DS.L      1
xattr_nblocks: DS.L      1
xattr_mtime:   DS.W      1
xattr_mdate:   DS.W      1
xattr_atime:   DS.W      1
xattr_adate:   DS.W      1
xattr_ctime:   DS.W      1
xattr_cdate:   DS.W      1
xattr_attr:    DS.W      1
xattr_res2:    DS.W      1
xattr_res3:    DS.L      2
xattr_sizeof:

     .OFFSET 0

xfs_name:      DS.B      8
xfs_next:      DS.L      1
xfs_flags:     DS.L      1
xfs_init:      DS.L      1
xfs_sync:      DS.L      1
xfs_pterm:     DS.L      1
xfs_garbcoll:  DS.L      1
xfs_freeDD:    DS.L      1
xfs_drv_open:  DS.L      1
xfs_drv_close: DS.L      1
xfs_path2DD:   DS.L      1
xfs_sfirst:    DS.L      1
xfs_snext:     DS.L      1
xfs_fopen:     DS.L      1
xfs_fdelete:   DS.L      1
xfs_link:      DS.L      1
xfs_xattr:     DS.L      1
xfs_attrib:    DS.L      1
xfs_chown:     DS.L      1
xfs_chmod:     DS.L      1
xfs_dcreate:   DS.L      1
xfs_ddelete:   DS.L      1
xfs_DD2name:   DS.L      1
xfs_dopendir:  DS.L      1
xfs_dreaddir:  DS.L      1
xfs_drewinddir: DS.L     1
xfs_dclosedir: DS.L      1
xfs_dpathconf: DS.L      1
xfs_dfree:     DS.L      1
xfs_wlabel:    DS.L      1
xfs_rlabel:    DS.L      1
xfs_symlink:   DS.L      1
xfs_readlink:  DS.L      1
xfs_dcntl:     DS.L      1
xfs_sizeof:

     .text

/* Standardhandles */

STDIN               =  0
STDOUT              =  1
STDAUX              =  2
STDPRN              =  3
STDHDL4             =  4
STDHDL5             =  5

/* Schreib-/Lesemodi fuer Fgetchar und Fputchar */

CMODE_RAW           =  0
CMODE_COOKED        =  1
BCMODE_COOKED       =  0
CMODE_ECHO          =  2
BCMODE_ECHO         =  1

/* Open- Modus von Dateien (Mag!X- intern)                                 */
/* NOINHERIT wird nicht unterstuetzt, weil nach TOS- Konvention nur die     */
/* Handles 0..5 vererbt werden                                             */
/* HiByte wie unter MiNT verwendet                                         */

OM_RPERM       =  1         /* Datei ist zum Lesen geoeffnet              */
BOM_RPERM      =  0
OM_WPERM       =  2         /* Datei ist zum Schreiben geoeffnet          */
BOM_WPERM      =  1
OM_EXEC        =  4         /* Datei ist zum Ausfuehren geoeffnet          */
BOM_EXEC       =  2
OM_APPEND      =  8         /* Schreibzugriffe ans Ende (Kernel!)        */
BOM_APPEND     =  3
OM_RDENY       =  16        /* andere duerfen nicht gleichz. lesen        */
BOM_RDENY      =  4
OM_WDENY       =  32        /* andere duerfen nicht gleichz. schreiben    */
BOM_WDENY      =  5
OM_NOCHECK     =  64        /* KEINE ueberpruefung durch den Kernel        */
BOM_NOCHECK    =  6


/* Open- Modus von Dateien (MiNT) */
O_RWMODE       =  3         /* Maske fuer r/w- Modi */
O_RDONLY       =  0
O_WRONLY       =  1
O_RDWR         =  2
O_EXEC         =  3

O_APPEND       =  8         /* Schreiben geht immer ans Dateiende   */
BO_APPEND      =  3

O_SHMODE       =  0x70
O_COMPAT       =  0
O_DENYRW       =  0x10
O_DENYW        =  0x20
O_DENYR        =  0x30
O_DENYNONE     =  0x40

O_NOINHERIT    =  0x80       /* von Mag!X nicht unterstuetzt */
O_NDELAY       =  0x100      /* von Mag!X nicht unterstuetzt */
O_CREAT        =  0x200
BO_CREAT       =  9
O_TRUNC        =  0x400
BO_TRUNC       =  10
O_EXCL         =  0x800
BO_EXCL        =  11
O_TTY          =  0x2000     /* von Mag!X nicht unterstuetzt */
O_HEAD         =  0x4000     /* von Mag!X nicht unterstuetzt */
O_LOCK         =  0x8000     /* von Mag!X nicht unterstuetzt */

/* unterstuetzte Dcntl- Modi (Mag!X- spezifisch!) */
KER_GETINFO    =  0x0100     /* Kernel: gib Zeiger auf Kernel zurueck    */
KER_DOSLIMITS  =  0x0101     /* Kernel: Limits fuer FAT-Dateisysteme */
KER_INSTXFS    =  0x0200     /* Kernel: installiere ein XFS */
DFS_GETINFO    =  0x1100     /* XFS_DOS: gib Zeiger auf DFS-Kernel */
DFS_INSTDFS    =  0x1200     /* XFS_DOS: installiere ein DFS */
DEV_M_INSTALL  =  0xcd00     /* DFS_U */

/* unterstuetzte Fcntl- Modi */
FSTAT          =  0x4600
FIONREAD       =  0x4601
FIONWRITE      =  0x4602
FUTIME         =  0x4603
FTRUNCATE      =  0x4604
SHMGETBLK      =  0x4d00
SHMSETBLK      =  0x4d01
PBASEADDR      =  0x5002

/* Modi und Codes fuer Dpathconf() */

DP_MAXREQ      =  -1
DP_IOPEN       =  0    /* internal limit on # of open files */
DP_MAXLINKS    =  1    /* max number of hard links to a file */
DP_PATHMAX     =  2    /* max path name length */
DP_NAMEMAX     =  3    /* max length of an individual file name */
DP_ATOMIC      =  4    /* # of bytes that can be written atomically */
DP_TRUNC       =  5    /* file name truncation behavior */
 DP_NOTRUNC    =  0    /* long filenames give an error */
 DP_AUTOTRUNC  =  1    /* long filenames truncated */
 DP_DOSTRUNC   =  2    /* DOS truncation rules in effect */
DP_CASE        =  6
 DP_CASESENS   =  0    /* case sensitive */
 DP_CASECONV   =  1    /* case always converted */
 DP_CASEINSENS =  2    /* case insensitive, preserved */
DP_MODEATTR    =  7
 DP_ATTRBITS   =  0x000000ff
 DP_MODEBITS   =  0x000fff00
 DP_FILETYPS   =  0xfff00000
  DP_FT_DIR    =  0x00100000
  DP_FT_CHR    =  0x00200000
  DP_FT_BLK    =  0x00400000
  DP_FT_REG    =  0x00800000
  DP_FT_LNK    =  0x01000000
  DP_FT_SOCK   =  0x02000000
  DP_FT_FIFO   =  0x04000000
  DP_FT_MEM    =  0x08000000
DP_XATTRFIELDS =  8
 DP_INDEX      =  0x0001
 DP_DEV        =  0x0002
 DP_RDEV       =  0x0004
 DP_NLINK      =  0x0008
 DP_UID        =  0x0010
 DP_GID        =  0x0020
 DP_BLKSIZE    =  0x0040
 DP_SIZE       =  0x0080
 DP_NBLOCKS    =  0x0100
 DP_ATIME      =  0x0200
 DP_CTIME      =  0x0400
 DP_MTIME      =  0x0800

/* Fuer Psemaphore, die Modi 0/1 werden z.Zt. nicht unterstuetzt */

PSEM_CRGET     =  0    /* create and get */
PSEM_DESTROY   =  1
PSEM_GET       =  2
PSEM_RELEASE   =  3
