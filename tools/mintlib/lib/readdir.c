/* readdir routine */

/* under MiNT (v0.9 or better) these use the appropriate system calls.
 * under TOS or older versions of MiNT, they use Fsfirst/Fsnext
 *
 * Written by Eric R. Smith and placed in the public domain
 */

#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <osbind.h>
#include <mintbind.h>
#include "lib.h"


/* Important note: under Metados, some file systems can have opendir/readdir/
 * closdir, so we must not have a status variable for these ones.
 * Instead, check the directory struct if there was an opendir call.
 */

extern ino_t __inode;					/* in stat.c */

/* a new value for DIR->status, to indicate that the file system is not
 * case sensitive.
 */
#define _NO_CASE  8


struct dirent *readdir(d)
DIR *d;
{
	struct dbuf
	{
		long ino;
		char name[NAME_MAX + 1];
	} dbuf;
	long r;
	_DTA *olddta;
	struct dirent *dd = &d->buf;

	if (d->handle != 0xff000000L)
	{
		/* The directory descriptor was optained by calling Dopendir(), as
		 * there is a valid handle.
		 */
		r = (int) Dreaddir((int) (NAME_MAX + 1 + sizeof(long)), d->handle, (char *) &dbuf);
		if (r == -ENMFIL)
			return 0;
		else if (r)
		{
			errno = (int) -r;
			return 0;
		} else
		{
			dd->d_ino = dbuf.ino;
			dd->d_off++;
			dd->d_reclen = (short) strlen(dbuf.name);
			strcpy(dd->d_name, dbuf.name);

			/* if file system is case insensitive, transform name to lowercase */
			if (d->status == _NO_CASE)
				strlwr(dd->d_name);

			return dd;
		}
	}
/* ordinary TOS search, using Fsnext. Note that the first time through,
 * Fsfirst has already provided valid data for us; for subsequent
 * searches, we need Fsnext.
 */
	if (d->status == _NMFILE)
		return 0;
	if (d->status == _STARTSEARCH)
	{
		d->status = _INSEARCH;
	} else
	{
		olddta = Fgetdta();
		Fsetdta(&(d->dta));
		r = Fsnext();
		Fsetdta(olddta);
		if (r == -ENMFIL)
		{
			d->status = _NMFILE;
			return 0;
		} else if (r)
		{
			errno = (int) -r;
			return 0;
		}
	}
	dd->d_ino = __inode++;
	dd->d_off++;
	_dos2unx(d->dta.dta_name, dd->d_name, sizeof(dd->d_name));
	dd->d_reclen = (short) strlen(dd->d_name);
	return dd;
}