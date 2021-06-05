/*
 *
 *	STDIO.H		Standard i/o include file
 *
 */

#ifndef _STDIO_H
#define	_STDIO_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SIZE_T
#define _SIZE_T __SIZE_TYPEDEF__
typedef _SIZE_T size_t;
#endif

/*
 *	CONSTANTS:
 */

#ifndef STREAM_MAX
#define STREAM_MAX	_NFILE
#endif

#define FOPEN_MAX	_NFILE
#define	FILENAME_MAX	(128)		/* maximum filename size */

#ifndef NULL
#define NULL		__NULL
#endif

#define	BUFSIZ		((size_t)1024)	/* default buffer size */
			/* change here must be reflected in crt0.c too */

#define	EOF		(-1)		/* end-of-file indicator */
#ifndef __STRICT_ANSI__
# ifndef _POSIX_SOURCE
#define	EOS		'\0'		/* end-of-string indicator */
# endif
#endif

#ifndef SEEK_SET
/* lseek() origins */
#define	SEEK_SET	0		/* from beginning of file */
#define	SEEK_CUR	1		/* from current location */
#define	SEEK_END	2		/* from end of file */
#endif

/* FILE structure flags */
#define	_IOREAD		0x0001		/* file may be read from */
#define	_IOWRT		0x0002		/* file may be written to */
#define	_IOBIN		0x0004		/* file is in "binary" mode */
#define	_IODEV		0x0008		/* file is a character device */
#define	_IORW		0x0080		/* file is open for update (r+w) */
#define	_IOFBF		0x0100		/* i/o is fully buffered */
#define	_IOLBF		0x0200		/* i/o is line buffered */
#define	_IONBF		0x0400		/* i/o is not buffered */
#define	_IOMYBUF	0x0800		/* standard buffer */
#define	_IOEOF		0x1000		/* EOF has been reached */
#define	_IOERR		0x4000		/* an error has occured */
#define _IOSTRING	0x8000		/* really a string buffer   */

typedef	struct			/* FILE structure */
	{
	long		_cnt;		/* # of bytes in buffer */
	unsigned char	*_ptr;		/* current buffer pointer */
	unsigned char	*_base;		/* base of file buffer */
	unsigned int	_flag;		/* file status flags */
	int		_file;		/* file handle */
	long		_bsiz;		/* buffer size */
	unsigned char	_ch;		/* tiny buffer, for "unbuffered" i/o */
	}
	FILE;

/* object of type capable of recording uniquely every position in a file */
typedef unsigned long fpos_t;

/* lengths of various things */
#define L_ctermid	128
#define	L_tmpnam	128
#ifdef _SYSV_SOURCE
#define L_cuserid	80
#endif
#define	TMP_MAX		100

extern	FILE	_iob[];

/* standard streams */
#define stdin	(&_iob[0])
#define stdout	(&_iob[1])
#define stderr	(&_iob[2])

/* stream macros */
#define clearerr(fp)	((void) ((fp)->_flag &= ~(_IOERR|_IOEOF)))
#define feof(fp)	((fp)->_flag & _IOEOF)
#define ferror(fp)	((fp)->_flag & _IOERR)
#define fileno(fp)	((fp)->_file)


/* function definitions */

__EXTERN char *	ctermid	__PROTO((char *));
#ifdef _SYSV_SOURCE
__EXTERN char *	cuserid __PROTO((char *));
#endif /* _SYSV_SOURCE */

__EXTERN int	remove	__PROTO((const char *));
__EXTERN int	rename	__PROTO((const char *, const char *));
__EXTERN char *	tmpnam	__PROTO((char *));
__EXTERN FILE *	tmpfile	__PROTO((void));

__EXTERN int	fclose	__PROTO((FILE *));
__EXTERN int	fflush	__PROTO((FILE *));

__EXTERN FILE *	fopen	__PROTO((const char *, const char *));
__EXTERN FILE *	freopen	__PROTO((const char *, const char *, FILE *));

__EXTERN void	setbuf	__PROTO((FILE *, char *));
__EXTERN int	setvbuf	__PROTO((FILE *, char *, int, size_t));

#ifdef __SRC__
__EXTERN int  fscanf  __PROTO((FILE *, const char *, char *));
__EXTERN int  scanf   __PROTO((const char *, char *));
__EXTERN int  sscanf  __PROTO((const char *, const char *, int));
#else /* not __SRC__ */
__EXTERN int  fscanf  __PROTO((FILE *, const char *, ...));
__EXTERN int  scanf   __PROTO((const char *, ...));
__EXTERN int  sscanf  __PROTO((const char *, const char *, ...));
#endif /* not __SRC__ */

__EXTERN int	fprintf	__PROTO((FILE *, const char *, ...));
__EXTERN int	printf	__PROTO((const char *, ...));
__EXTERN int	sprintf	__PROTO((char *, const char *, ...));

__EXTERN int 	vfprintf __PROTO((FILE *, const char *, __VA_LIST__));
__EXTERN int 	vprintf	 __PROTO((const char *, __VA_LIST__));
__EXTERN int 	vsprintf __PROTO((char *, const char *, __VA_LIST__));
#ifndef _POSIX_SOURCE
__EXTERN int	vscanf  __PROTO((const char *, __VA_LIST__));
__EXTERN int	vfscanf __PROTO((FILE *, const char *, __VA_LIST__));
__EXTERN int	vsscanf	__PROTO((const char *, const char *, __VA_LIST__));
#endif /* _POSIX_SOURCE */

__EXTERN int	fgetc	__PROTO((FILE *));
__EXTERN char	*fgets	__PROTO((char *, int, FILE *));
__EXTERN char	*gets	__PROTO((char *));
__EXTERN int	fputc	__PROTO((int c, FILE *));
__EXTERN int	fputs	__PROTO((const char *, FILE *));
__EXTERN int	puts	__PROTO((const char *));

__EXTERN size_t	fread	__PROTO((void *, size_t, size_t, FILE *));
__EXTERN size_t	fwrite	__PROTO((const void *, size_t, size_t, FILE *));

__EXTERN int	fgetpos	__PROTO((FILE *, fpos_t *));
__EXTERN int	fsetpos	__PROTO((FILE *, fpos_t *));

__EXTERN int	fseek	__PROTO((FILE *, long, int));
__EXTERN long	ftell	__PROTO((FILE *));
__EXTERN void	rewind	__PROTO((FILE *));

__EXTERN void	perror	__PROTO((const char *));

#ifndef __STRICT_ANSI__
__EXTERN FILE	*fdopen	__PROTO((int, const char *));

# ifndef _POSIX_SOURCE
__EXTERN FILE *	fopenp	__PROTO((const char *, const char *));
__EXTERN int 	fungetc	__PROTO((int, FILE *));
__EXTERN int	pclose	__PROTO((FILE *));
__EXTERN FILE *	popen	__PROTO((const char *, const char *));
__EXTERN void	setlinebuf	__PROTO((FILE *));

__EXTERN void	_binmode	__PROTO((int));		/* ++jrb */
__EXTERN long 	getl	__PROTO((FILE *));
__EXTERN long 	putl	__PROTO((long, FILE *));
__EXTERN short 	getw	__PROTO((FILE *));
__EXTERN short 	putw	__PROTO((short, FILE *));
__EXTERN void	_getbuf	__PROTO((FILE *fp));
# endif /* _POSIX_SOURCE */

#endif /* __STRICT_ANSI__ */


/* aliases */

__EXTERN int	_filbuf	__PROTO((FILE *));	/* needed for getc */

#ifdef __GNUC_INLINE__
#define getc(__fp) \
({   int __c; \
     typedef _tfp = (__fp); \
     _tfp __lfp = (__fp); \
    do { \
	__c = (--__lfp->_cnt >= 0) ? ((int)*__lfp->_ptr++) : _filbuf(__lfp); \
    } while ((!(__lfp->_flag & _IOBIN)) && (__c == '\r')); \
    __c; \
})
#else
#define	getc(fp)		fgetc(fp)
#endif

#define	ungetc			fungetc
#define	putc			fputc
#define	getchar()		getc(stdin)
#define	ungetchar(c)		fungetc((c),stdin)
#define	putchar(c)		fputc((c),stdout)

#ifdef __cplusplus
}
#endif

#endif /* _STDIO_H */
