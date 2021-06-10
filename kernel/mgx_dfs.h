/*********************************************************************
*
* Mag!X 3.00
* ==========
*
* Strukturen f�r die Einbindung eines DFS.
* Die korrekten Prototypen f�r die Implementation eines DFS
* in 'C' folgen, so wie die 'C'- Schnittstelle fertig ist.
*
* Da z.T. mehrere Register f�r R�ckgabewerte verwendet werden und
* au�erdem Zeiger in Datenregistern �bergeben werden, ist eine
* Schnittstelle als _cdecl geplant, d.h. s�mtliche Parameter werden
* auf dem Stapel �bergeben, dies erm�glicht die Verwendung eines
* beliebigen Compilers.
*
* MagiC 3.00 onwards
* ==================
*
* Structures for binding in a DFS.
* The correct prototypes for the implementation of a DFS
* in 'C' will follow as soon as the 'C' port is ready.
*
* As at times several registers are used for return values, and in
* addition pointers are passed in data registers, a port as _cdecl
* is planned, i.e. all parameters are passed on the stack, which
* makes possible the use of any desired compiler.
*
* Version: 4.4.94
*
*********************************************************************/

typedef struct _mx_dosfd MX_DOSFD;
typedef struct _mx_dosdir MX_DOSDIR;

typedef struct _mx_ddev {
     LONG cdecl (*ddev_open)(MX_DOSFD *f);
     LONG cdecl (*ddev_close)(MX_DOSFD *f);
     LONG cdecl (*ddev_read)(MX_DOSFD *f, long len, void *buf);
     LONG cdecl (*ddev_write)(MX_DOSFD *f, long len, void *buf);
     LONG cdecl (*ddev_stat)(MX_DOSFD *f, MAGX_UNSEL *unsel, short rwflag, long /* APPL * */ appl);
     LONG cdecl (*ddev_seek)(MX_DOSFD *f, long where, short whence);
     LONG cdecl (*ddev_datime)(MX_DOSFD *f, short *buf, short rwflag);
     LONG cdecl (*ddev_ioctl)(MX_DOSFD *f, short cmd, void *buf);
     LONG cdecl (*ddev_delete)(MX_DOSFD *f, MX_DOSDIR *dir);
     LONG cdecl (*ddev_getc)(MX_DOSFD *f, short mode);
     LONG cdecl (*ddev_getline)(MX_DOSFD *f, char *buf, long size, short mode);
     LONG cdecl (*ddev_putc)(MX_DOSFD *f, short mode, long val);
} MX_DDEV;


struct _mx_dosfd {
/*  0 */     struct _mx_dosdmd	*fd_dmd;
/*  4 */     WORD      fd_refcnt;
/*  6 */     WORD      fd_mode;
/*  8 */     const MX_DEV    *fd_dev;
/* 12 */     const MX_DDEV   *fd_ddev;
/* 16 */     char      fd_name[11];
/* 27 */     char      fd_attr;
/* 28 */     PD        *fd_owner;
/* 32 */     struct _mx_dosfd  *fd_parent;
/* 36 */     struct _mx_dosfd  *fd_children;
/* 40 */     struct _mx_dosfd  *fd_next;
/* 44 */     struct _mx_dosfd  *fd_multi;
/* 48 */     struct _mx_dosfd  *fd_multi1;
/* 52 */     ULONG     fd_fpos;
/* 56 */     char      fd_dirch;
/* 57 */     char      fd_unused;
/* 58 */     WORD      fd_time;
/* 60 */     WORD      fd_date;
/* 62 */     WORD      fd_stcl;
/* 64 */     ULONG     fd_len;
/* 68 */     ULONG     fd_dirpos;
/* 72 */     ULONG     fd_user1;
/* 76 */     ULONG     fd_user2;
/* 80 */     char		*fd_longname;
};

typedef struct _mx_dosdta {
     char      dta_sname[12];
     ULONG     dta_usr1;
     ULONG     dta_usr2;
     char      dta_drive;
     char      dta_attr;
     WORD      dta_time;
     WORD      dta_date;
     ULONG     dta_len;
     char      dta_name[14];
} MX_DOSDTA;


typedef struct _mx_dosdmd {
     MX_XFS    *d_xfs;
     WORD      d_drive;
     MX_DOSFD  *d_root;
     WORD      biosdev;
     LONG      driver;
     LONG      devcode;
     struct _mx_dfs    *d_dfs;
     WORD		d_flags;
} MX_DOSDMD;


struct _mx_dosdir {
     char      dir_name[11];
     char      dir_attr;
     WORD      dir_usr1;
     ULONG     dir_usr2;
     ULONG     dir_usr3;
     WORD      dir_time;
     WORD      dir_date;
     WORD      dir_stcl;
     ULONG     dir_flen;
};



typedef struct _mx_dfs {
     char      dfs_name[8];
     struct _mx_dfs   *dfs_next;
     long      (*dfs_init)(void);
     long      (*dfs_sync)(MX_DOSDMD *d);
     long      (*dfs_drv_open)(MX_DOSDMD *d);
     long      (*dfs_drv_close)(MX_DOSDMD *d, short mode);
     long      (*dfs_dfree)(MX_DOSDMD *, long df[4]);
     long      (*dfs_sfirst)(MX_DOSFD *dd, MX_DOSDIR *dir, LONG pos, MX_DOSDTA *dta, void *link);
     long      (*dfs_snext)(MX_DOSDTA *dta, MX_DOSDMD *d, void *next);
     long      (*dfs_ext_fd)(MX_DOSFD *dd);
     long      (*dfs_fcreate)(MX_DOSFD *fd, MX_DOSDIR *dir, short cmd, long arg);
     long      (*dfs_fxattr)(MX_DOSFD *dd, MX_DOSDIR *dir, short mode, XATTR *xattr, void *link);
     long      (*dfs_dir2index)(MX_DOSFD *dd, MX_DOSDIR *dir, void *link);
     long      (*dfs_readlink)(MX_DOSFD *dd, MX_DOSDIR *dir, void *link);
     long      (*dfs_dir2FD)(MX_DOSFD *dd, MX_DOSDIR *dir, void *link);
     long      (*dfs_fdelete)(MX_DOSFD *dd, MX_DOSDIR *dir, long pos);
     long      (*dfs_pathconf)(MX_DOSFD *dd, short cmd);
} MX_DFS;

/* Supported Dcntl modes */
#define   DFS_GETINFO    0x1100
#define   DFS_INSTDFS    0x1200
#define   DEV_M_INSTALL  0xcd00

/* additional attribute bits */
#ifndef FA_SYMLINK
#define	FA_SYMLINK	0x40
#endif
