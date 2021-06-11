/*
 * stat, fstat, lstat emulation for TOS
 * written by Eric R. Smith and placed in the public domain
 */

#include <limits.h>
#include <types.h>
#include <stat.h>
#include <ctype.h>
#include <errno.h>
#include <osbind.h>
#include <mintbind.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <support.h>
#include <ioctl.h>						/* for FSTAT */
#include "lib.h"

/* extern int __mint; */

extern ino_t __inode;

/* for backwards compatibilty: if nonzero, files are checked to see if
 * they have the TOS executable magic number in them
 */

int _x_Bit_set_in_stat = 0;

/* date for files (like root directories) that don't have one */
#define OLDDATE _unixtime(0,0)

/*
 * common routine for stat() and lstat(); if "lflag" is 0, then symbolic
 * links are automatically followed (like stat), if 1 then they are not
 * (like lstat)
 */

__EXTERN int _do_stat __PROTO((const char *_path, struct stat * st, int lflag));

int _do_stat(_path, st, lflag)
const char *_path;
struct stat *st;
int lflag;
{
	long r;
	_DTA *olddta;
	int nval;
	char path[PATH_MAX];
	char *ext,
	 drv;
	int fd;
	short magic;
	_DTA d;
	int isdot = 0;


	if (!_path)
	{
		errno = EFAULT;
		return -1;
	}

	/*
	 * _unx2dos returns 1 for device names (like /dev/con)
	 */
	nval = _unx2dos(_path, path, sizeof(path));

	/* try to use the build in stat() call, but if the system does not
	 * have it, record that and never try again
	 */

	/* actually we can't do that, because Fxattr() works for MetaDOS devices
	 * but returns -ENOSYS for other GEMDOS devices. Really unhappy solution.
	 * Since I don't want to patch chdir() and other calls I simply have to
	 * test the presence of Fxattr() every time the stat() is called.
	 * PS 970606
	 */

	r = Fxattr(lflag, path, st);
	if (r != -ENOSYS)
	{
		if (r)
		{
			if ((r == -EPATH) && _enoent(path))
			{
				r = -ENOENT;
			}
			errno = (int) -r;
			return -1;
		} else
		{
			__UNIXTIME(st->st_mtime);
			__UNIXTIME(st->st_atime);
			__UNIXTIME(st->st_ctime);
			/* Most versions of Unix count in 512 byte blocks */
			st->st_blocks = (st->st_blocks * st->st_blksize) / 512;
			/* if we hit a symbolic link, try to get its size right */
			if (lflag && ((st->st_mode & S_IFMT) == S_IFLNK))
			{
				char buf[PATH_MAX + 1];
				char buf1[PATH_MAX + 1];

				r = Freadlink(PATH_MAX, buf, path);
				if (r < 0)
				{
					errno = (int) -r;
					return -1;
				}
				buf[PATH_MAX] = 0;
				_dos2unx(buf, buf1, sizeof(buf1));
				st->st_size = strlen(buf1);
			}
			return 0;
		}
	}

	/* otherwise, check to see if we have a name like CON: or AUX: */
	if (nval == 1)
	{
		st->st_mode = S_IFCHR | 0600;
		st->st_attr = 0;
		st->st_ino = __inode++;
		st->st_rdev = 0;
		st->st_mtime = st->st_ctime = st->st_atime = time((time_t *) 0) - 2;
		st->st_dev = 0;
		st->st_nlink = 1;
		st->st_uid = geteuid();
		st->st_gid = getegid();
		st->st_size = st->st_blocks = 0;
		st->st_blksize = 1024;
		return 0;
	}

	/* A file name: check for root directory of a drive */
	if (path[0] == '\\' && path[1] == 0)
	{
		drv = Dgetdrv() + 'A';
		goto rootdir;
	}

	if (((drv = path[0]) != 0) && path[1] == ':' && (path[2] == 0 || (path[2] == '\\' && path[3] == 0)))
	{
	  rootdir:
		st->st_mode = S_IFDIR | 0755;
		st->st_attr = FA_DIR;
		st->st_dev = isupper(drv) ? drv - 'A' : drv - 'a';
		st->st_ino = 2;
		st->st_mtime = st->st_ctime = st->st_atime = OLDDATE;
		goto fill_dir;
	}

	/* forbid wildcards in path names */
	if (index(path, '*') || index(path, '?'))
	{
		errno = ENOENT;
		return -1;
	}

	/* OK, here we're going to have to do an Fsfirst to get the date */
	/* NOTE: Fsfirst(".",-1) or Fsfirst("..",-1) both fail under TOS,
	 * so we kludge around this by using the fact that Fsfirst(".\*.*"
	 * or "..\*.*" will return the correct file first (except, of course,
	 * in root directories :-( ).
	 * NOTE2: Some versions of TOS don't like Fsfirst("RCS\\", -1) either,
	 * so we do the same thing if the path ends in '\\'.
	 */

	/* find the end of the string */
	for (ext = path; ext[0] && ext[1]; ext++) ;

/* add appropriate kludge if necessary */
	if (*ext == '.' && (ext == path || ext[-1] == '\\' || ext[-1] == '.'))
	{
		isdot = 1;
		strcat(path, "\\*.*");
	} else if (*ext == '\\')
	{
		isdot = 1;
		strcat(path, "*.*");
	}
	olddta = Fgetdta();
	Fsetdta(&d);
	r = Fsfirst(path, 0xff);
	Fsetdta(olddta);
	if (r < 0)
	{
		if (isdot && r == -ENOENT)
			goto rootdir;
		errno = (int) -r;
		return -1;
	}

	if (isdot && ((d.dta_name[0] != '.') || (d.dta_name[1])))
	{
		goto rootdir;
	}

	st->st_mtime = st->st_ctime = st->st_atime = _unixtime(d.dta_time, d.dta_date);
	if (((drv = *path) != 0) && path[1] == ':')
		st->st_dev = toupper(drv) - 'A';
	else
		st->st_dev = Dgetdrv();

	st->st_ino = __inode++;
	st->st_attr = d.dta_attribute;
/*
	if (__mint && st->st_dev == ('Q' - 'A'))
			st->st_mode = 0644 | S_IFIFO;
	else
*/
	{
		st->st_mode = 0644 | (st->st_attr & FA_DIR ? S_IFDIR | 0111 : S_IFREG);
	}

	if (st->st_attr & FA_RDONLY)
		st->st_mode &= ~0222;			/* no write permission */
	if (st->st_attr & FA_HIDDEN)
		st->st_mode &= ~0444;			/* no read permission */

/* check for a file with an executable extension */
	ext = strrchr(_path, '.');
	if (ext)
	{
		if (!strcmp(ext, ".ttp") || !strcmp(ext, ".prg") ||
			!strcmp(ext, ".tos") || !strcmp(ext, ".g") || !strcmp(ext, ".sh") || !strcmp(ext, ".bat"))
		{
			st->st_mode |= 0111;
		}
	}
	if ((st->st_mode & S_IFMT) == S_IFREG)
	{
		if (_x_Bit_set_in_stat)
		{
			if ((fd = (int) Fopen(path, 0)) < 0)
			{
				errno = -fd;
				return -1;
			}
			magic = 0;
			(void) Fread(fd, 2, (char *) &magic);
			(void) Fclose(fd);
			if (magic == 0x601A			/* TOS executable */
				|| magic == 0x2321)		/* "#!" shell file */
				st->st_mode |= 0111;
		}
		st->st_size = d.dta_size;
		/* in Unix, blocks are measured in 512 bytes */
		st->st_blocks = (st->st_size + 511) / 512;
		st->st_nlink = 1;				/* we dont have hard links */
	} else
	{
	  fill_dir:
		st->st_size = 1024;
		st->st_blocks = 2;
		st->st_nlink = 2;				/* "foo" && "foo/.." */
	}

	st->st_rdev = 0;
	st->st_uid = geteuid();				/* the current user owns every file */
	st->st_gid = getegid();
	st->st_blksize = 1024;
	return 0;
}
