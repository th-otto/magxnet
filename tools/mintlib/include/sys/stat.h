#ifndef _STAT_H
#define _STAT_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif

#ifndef _TYPES_H
#include <types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct stat {
	mode_t	st_mode;
	ino_t	st_ino;		/* must be 32 bits */
	dev_t	st_dev;		/* must be 16 bits */
	short	st_rdev;	/* not supported by the kernel */
	short	st_nlink;
	uid_t	st_uid;		/* must be 16 bits */
	gid_t	st_gid;		/* must be 16 bits */
	off_t	st_size;
	off_t	st_blksize;
	off_t	st_blocks;
	time_t	st_mtime;
	time_t	st_atime;
	time_t	st_ctime;
	short	st_attr;
#ifdef __MINT__
	short	res1;		/* reserved for future kernel use */
	long	res2[2];
#endif
};

#define	S_IFMT			0170000
#define	S_IFCHR			0020000
#define	S_IFDIR			0040000
#define S_IFBLK			0060000
#define	S_IFREG			0100000
#define S_IFIFO			0120000
#define S_IMEM			0140000
#define	S_IFLNK			0160000

#define S_ISCHR(m)		((m & S_IFMT) == S_IFCHR)
#define S_ISDIR(m)		((m & S_IFMT) == S_IFDIR)
#define S_ISBLK(m)		((m & S_IFMT) == S_IFBLK)
#define S_ISREG(m)		((m & S_IFMT) == S_IFREG)
#define S_ISFIFO(m)		((m & S_IFMT) == S_IFIFO)
#ifndef _POSIX_SOURCE
#define S_ISMEM(m)		((m & S_IFMT) == S_IMEM)
#define S_ISLNK(m)		((m & S_IFMT) == S_IFLNK)
#endif /* _POSIX_SOURCE */

#define S_IRWXU			0700
#define S_IRWXG			0070
#define S_IRWXO			0007

#define	S_ISUID			04000
#define	S_ISGID			02000
#define	S_ISVTX			01000
/* file access modes for user, group, and other*/
#define S_IRUSR			0400
#define S_IWUSR			0200
#define S_IXUSR			0100
#define S_IRGRP			0040
#define S_IWGRP			0020
#define S_IXGRP			0010
#define S_IROTH			0004
#define S_IWOTH			0002
#define S_IXOTH			0001

#ifndef _POSIX_SOURCE
#define	S_IREAD			S_IRUSR
#define	S_IWRITE		S_IWUSR
#define	S_IEXEC			S_IXUSR
#define DEV_BSIZE		1024
#endif

/* function definitions */
__EXTERN int	chmod	__PROTO((const char *, int));
__EXTERN int	fstat	__PROTO((int, struct stat *));
#ifndef _POSIX_SOURCE
__EXTERN int	lstat	__PROTO((const char *, struct stat *));
#endif
__EXTERN int	mkdir	__PROTO((const char *, mode_t));
__EXTERN int	mkfifo	__PROTO((const char *, mode_t));
__EXTERN int	stat	__PROTO((const char *, struct stat *));
__EXTERN int	umask	__PROTO((int));

#ifdef __cplusplus
}
#endif

#endif /* _STAT_H */
