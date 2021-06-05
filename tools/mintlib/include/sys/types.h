#ifndef _TYPES_H
#define _TYPES_H

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

#ifndef __ssize_t_defined
typedef long ssize_t;
# define __ssize_t_defined
#endif

#ifndef _TIME_T
#define _TIME_T long
typedef _TIME_T time_t;
#endif

typedef unsigned short	dev_t;		/* holds a device type */
typedef _GID_T		gid_t;		/* group id type */
typedef unsigned long	ino_t;		/* holds an inode (fake under GEMDOS) */
typedef unsigned short	mode_t;		/* file mode */
typedef short		nlink_t;
typedef long		off_t;
typedef _PID_T		pid_t;		/* process id type */
typedef _UID_T		uid_t;		/* user id type */

#ifndef _POSIX_SOURCE
typedef unsigned char	u_char;
typedef unsigned short	u_short;
typedef unsigned int 	u_int;
typedef unsigned long	u_long;
typedef void *		caddr_t;
#define major(dev)	(((dev) >> 8) & 0xff)
#define minor(dev)	((dev) & 0x00ff)
#define makedev(x,y)	((dev_t) ((((x) & 0xff) << 8) | ((y) & 0xff)))

#ifndef _FD_SET_T
#define _FD_SET_T unsigned long
typedef _FD_SET_T fd_set;
#endif

#define FD_ZERO(set)		(*(set) = 0L)
#define FD_CLR(fd, set)		(*(set) &= ~(1L << (fd)))
#define FD_SET(fd, set)		(*(set) |= (1L << (fd)))
#define FD_ISSET(fd, set)	(*(set) & (1L << (fd)))
#define FD_SETSIZE		32

__EXTERN int truncate	__PROTO((const char *_filename, off_t length));
__EXTERN int ftruncate	__PROTO((int fd, off_t length));

#include <utime.h>	/* sigh! */
#endif /* _POSIX_SOURCE */

#ifdef __cplusplus
}
#endif

#endif /* _TYPES_H */
