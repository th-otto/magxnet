#ifndef _OSTRUCT_H
#define _OSTRUCT_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * General OS specific codes here
 *
 */

/*
 * GEMDOS defines and structures
 */

/* Structure used by Dfree() */
typedef struct {
    long b_free;	/* number of free clusters */
    long b_total;	/* total number of clusters */
    long b_secsiz;	/* number of bytes per sector */
    long b_clsiz;	/* number of sectors per cluster */
} _DISKINFO;

/* Structure returned by Fdatime() */
typedef struct {
  short time;
  short date;
} _DOSTIME;

/* Structure used by Cconrs */
typedef struct
{
        unsigned char maxlen;
        unsigned char actuallen;
        char    buffer[255];
} _CCONLINE;

#ifdef __TURBOC__
#define LINE _CCONLINE
#endif

/* Structure used by Fgetdta(), Fsetdta(), Fsfirst(), Fsnext() */
typedef struct _dta {
    char 	    dta_buf[21];	/* reserved */
    char            dta_attribute;	/* file attribute */
    unsigned short  dta_time;		/* file time stamp */
    unsigned short  dta_date;		/* file date stamp */
    long            dta_size;		/* file size */
    char            dta_name[14];	/* file name */
} _DTA;

/* Codes used with Fsfirst() */

#define        FA_RDONLY           0x01
#define        FA_HIDDEN           0x02
#define        FA_SYSTEM           0x04
#define        FA_LABEL            0x08
#define        FA_DIR              0x10
#define        FA_CHANGED          0x20

/* This is what Fxattr expects.  */
/* old version first */
#if 0
struct xattr
{
	unsigned short st_mode;
	long           st_ino;	/* must be 32 bits */
	unsigned short st_dev;	/* must be 16 bits */
	short          st_rdev;	/* not supported by the kernel */
	unsigned short st_nlink;
	unsigned short st_uid;	/* must be 16 bits */
	unsigned short st_gid;	/* must be 16 bits */
	long           st_size;
	long           st_blksize;
	long           st_blocks;
	unsigned long  st_mtime;
	unsigned long  st_atime;
	unsigned long  st_ctime;
	short          st_attr;
	short res1;		/* reserved for future kernel use */
	long res2[2];
};
#endif

#ifndef __XATTR
#define __XATTR
typedef struct xattr {
	unsigned short mode;
	/* file types */
#define S_IFMT	0170000		/* mask to select file type */
#define S_IFCHR	0020000		/* BIOS special file */
#define S_IFDIR	0040000		/* directory file */
#define S_IFREG 0100000		/* regular file */
#define S_IFIFO 0120000		/* FIFO */
#define S_IMEM	0140000		/* memory region or process */
#define S_IFLNK	0160000		/* symbolic link */

	/* special bits: setuid, setgid, sticky bit */
#define S_ISUID	04000
#define S_ISGID 02000
#define S_ISVTX	01000

	/* file access modes for user, group, and other*/
#define S_IRUSR	0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRGRP 0040
#define S_IWGRP	0020
#define S_IXGRP	0010
#define S_IROTH	0004
#define S_IWOTH	0002
#define S_IXOTH	0001
#define DEFAULT_DIRMODE (0777)
#define DEFAULT_MODE	(0666)
	long index;
	unsigned short dev;
	unsigned short reserved1;
	unsigned short nlink;
	unsigned short uid;
	unsigned short gid;
	long st_size;
	long blksize, nblocks;
	unsigned short mtime, mdate;
	unsigned short atime, adate;
	unsigned short ctime, cdate;
	short attr;
	short reserved2;
	long reserved3[2];
} XATTR;
#endif

/* Codes used with Pexec */

#define        PE_LOADGO           0           /* load & go */
#define        PE_LOAD             3           /* just load */
#define        PE_GO               4           /* just go */
#define        PE_CBASEPAGE        5           /* just create basepage */
/* Tos 1.4: like 4, but memory ownership changed to child, and freed
   on exit
 */
#define        PE_GO_FREE          6           /* just go, then free */

#ifdef __MINT__
/* ers: what exactly does mode 7 do ??? */
#  define	PE_ASYNC_LOADGO	   100	       /* load and asynchronously go */
#  define       PE_ASYNC_GO	   104	       /* asynchronously go	     */
#  define       PE_ASYNC_GO_FREE   106	       /* asynchronously go and free */
#  define       PE_OVERLAY	   200	       /* load and overlay	     */
#endif

/*
 * BIOS defines and structures
 */

/* Device codes for Bconin(), Bconout(), Bcostat(), Bconstat() */
#define _PRT    0
#define _AUX    1
#define _CON    2
#define _MIDI   3
#define _IKBD   4
#define _RAWCON 5

/* Structure returned by Getbpb() */
typedef struct {
  short recsiz;         /* bytes per sector */
  short clsiz;          /* sectors per cluster */
  short clsizb;         /* bytes per cluster */
  short rdlen;          /* root directory size */
  short fsiz;           /* size of file allocation table */
  short fatrec;         /* startsector of second FAT */
  short datrec;         /* first data sector */
  short numcl;          /* total number of clusters */
  short bflags;         /* some flags */
} _BPB;

/* Structures used by Getmpb() */

/* Memory descriptor */
typedef struct _md {
    struct _md	*md_next;	/* next descriptor in the chain */
    long	 md_start;	/* starting address of block */
    long	 md_length;	/* length of the block */
    long	 md_owner;	/* owner's process descriptor */
} _MD;

/* Memory parameter block */
typedef struct {
    _MD *mp_free;		/* free memory chunks */
    _MD *mp_used;		/* used memory descriptors */
    _MD *mp_rover;		/* rover memory descriptor */
} _MPB;


/*
 * XBIOS defines and structures
 */

/* Codes used with Cursconf() */
#define CURS_HIDE   	0
#define CURS_SHOW   	1
#define CURS_BLINK  	2
#define CURS_NOBLINK    3
#define CURS_SETRATE    4
#define CURS_GETRATE    5

/* Structure returned by Iorec() */
typedef struct {
    char    *ibuf;
    short   ibufsiz;
    volatile short   ibufhd;
    volatile short   ibuftl;
    short   ibuflow;
    short   ibufhi;
} _IOREC;

/* Structure used by Initmouse() */
typedef struct {
	char	topmode;
	char	buttons;
	char	xparam;
	char	yparam;
	short	xmax;
	short	ymax;
	short	xstart;
	short	ystart;
} _PARAM;

/* Structure returned by Kbdvbase() */
typedef struct {
    void    (*midivec)	__PROTO((void));
    void    (*vkbderr)	__PROTO((void));
    void    (*vmiderr)	__PROTO((void));
    void    (*statvec)	__PROTO((void *));
    void    (*mousevec)	__PROTO((void *));
    void    (*clockvec)	__PROTO((void *));
    void    (*joyvec)	__PROTO((void *));
    long    (*midisys)	__PROTO((void));
    long    (*ikbdsys)	__PROTO((void));
    char    kbstate;
} _KBDVECS;

/* Structure returned by Keytbl() */
typedef struct {
    void *unshift;	/* pointer to unshifted keys */
    void *shift;	/* pointer to shifted keys */
    void *caps;		/* pointer to capslock keys */
} _KEYTAB;

/* Structure used by Prtblk() */
typedef struct
{
        void    *pb_scrptr;
        int     pb_offset;
        int     pb_width;
        int     pb_height;
        int     pb_left;
        int     pb_right;
        int     pb_screz;
        int     pb_prrez;
        void    *pb_colptr;
        int     pb_prtype;
        int     pb_prport;
        void    *pb_mask;
} _PBDEF;

#ifdef __cplusplus
}
#endif

#endif /* _OSTRUCT_H */
