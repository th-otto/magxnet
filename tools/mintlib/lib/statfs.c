/*
 * statfs() emulation for MiNT/TOS
 *
 * Written by Adrian Ashley (adrian@secret.uucp)
 * and placed in the public domain.
 *
 * Modified for MiNT 1.15 by Chris Felsch
 */

#include <errno.h>
#include <stat.h>
#include <osbind.h>
#include <mintbind.h>
#include <unistd.h>
#include <limits.h>
#include <support.h>
#include <sys/statfs.h>
#include <string.h>

/* from kernel source! */
#include <dcntl.h>

extern int __mint;

int statfs(const char *path, struct statfs *buf)
{
	long r;
	struct stat statbuf;
	struct fs_info info;
	struct fs_usage usage;

	if (!buf || !path)
	{
		errno = EFAULT;
		return -1;
	}

	r = stat(path, &statbuf);
	if (r == -1)
		return -1;

	r = Dcntl(FS_INFO, path, (long) &info);
	if (r == E_OK)
	{
		buf->f_type = info.type;
	} else
		buf->f_type = FS_OLDTOS;		/* default: TOS */

	r = Dcntl(FS_USAGE, path, (long) &usage);
	if (r == E_OK)
	{
		buf->f_blocks = usage.blocks.lo;
		buf->f_bsize = usage.blocksize;
		buf->f_bfree = buf->f_bavail = usage.free_blocks.lo;
		buf->f_files = usage.inodes.lo;
		buf->f_ffree = usage.free_inodes.lo;
		buf->f_fsid.val[0] = buf->f_fsid.val[1] = -1L;
	} else if (r == -ENOSYS)
	{
		_DISKINFO free;

		/* Hack by HPP 02/06/1993: since MiNT 0.99 only returns     */
		/* valid dfree info for pseudo-drives if they are the       */
		/* current directory, change directories for the occasion.  */
		if ((__mint >= 99) && (statbuf.st_dev >= 32))
		{
			char oldpath[PATH_MAX];

			if (getcwd(oldpath, PATH_MAX) != NULL)
			{
				chdir(path);
				r = Dfree(&free, statbuf.st_dev + 1);
				chdir(oldpath);
			} else
				r = Dfree(&free, statbuf.st_dev + 1);
		} else
			r = Dfree(&free, statbuf.st_dev + 1);

		if (r == E_OK)
		{
			buf->f_bsize = free.b_secsiz * free.b_clsiz;
			buf->f_blocks = free.b_total;
			buf->f_bfree = buf->f_bavail = free.b_free;
			buf->f_files = buf->f_ffree = -1L;
			buf->f_fsid.val[0] = buf->f_fsid.val[1] = -1L;
		}
	}

	/* An error occured (e.g. no medium in removable drive) */
	if (r < 0)
	{
		errno = -(int)r;
		return -1;
	}

	return 0;
}

int get_fsname(const char *path, char *xfs_name, char *type_name)
{
	struct fs_info info;
	char xname[32];
	char tname[32];

	if (!path)
	{
		errno = EFAULT;
		return -1;
	}

	if (Dcntl(FS_INFO, path, (long) (&info)) >= 0)	/* MiNT 1.15 */
	{
		strcpy(xname, info.name);
		strcpy(tname, info.type_asc);
	} else
	{
		if (Dcntl(MX_KER_XFSNAME, path, (long)xname) >= 0)	/* MagiC: only one name available */
			strcpy(tname, xname);
		else
		{
			strcpy(xfs_name, "tos-fs");
			strcpy(type_name, "tos");
		}
	}

	if (xfs_name)
		strcpy(xfs_name, xname);
	if (type_name)
		strcpy(type_name, tname);
	return 0;
}
