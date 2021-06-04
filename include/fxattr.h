/* structure for Fxattr */
#ifndef __XATTR
#define __XATTR
typedef struct xattr		XATTR;

struct xattr
{
	unsigned short	mode;
	long	index;
	unsigned short	dev;
	unsigned short	rdev;		/* "real" device */
	unsigned short	nlink;
	unsigned short	uid;
	unsigned short	gid;
	long	st_size;
	long	blksize;
	long	nblocks;
	unsigned short	mtime, mdate;
	unsigned short	atime, adate;
	unsigned short	ctime, cdate;
	short	attr;
	short	reserved2;
	long	reserved3[2];
};
#endif
