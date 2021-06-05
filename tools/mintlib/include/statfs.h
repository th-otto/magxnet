/* 
 * statfs.h -- structure for statfs() call.
 */

#ifndef _SYS_STATFS_H
#define _SYS_STATFS_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
	long val[2];
} fsid_t;

struct statfs 
{
	long		f_type;		/* type of info - defined in dcntl.h */
	long		f_bsize;		/* fundamental file system block size */
	long		f_blocks;	/* total blocks in file system */
	long		f_bfree;		/* free blocks */
	long 		f_bavail;	/* free blocks available to non-super-user */
	long 		f_files;		/* total file nodes in file system */
	long 		f_ffree;		/* free file nodes in fs */
	fsid_t	f_fsid;		/* file system id */
	long		f_spare[7];	/* spare for later */
};

__EXTERN int	statfs		__PROTO((const char *path, struct statfs *buf));
__EXTERN int	get_fsname	__PROTO((const char *path, char *xfs_name, char *type_name));

#ifdef __cplusplus
}
#endif

#endif	/* _SYS_STATFS_H */
