/*
 *	Skeleton for the info devices (/dev/unix, /dev/route, ...)
 *
 *	02/28/94, kay roemer.
 */

#include "sockets.h"
#include "dummydev.h"
#include "arpdev.h"
#include "mxkernel.h"

MX_DDEV cdecl_dummydev GNU_ASM_NAME("cdecl_dummydev") = {
	dummydev_open,
	dummydev_close,
	/* ugly hack here: function is not cdecl */
	(long cdecl (*)(MX_DOSFD *, long, void *))arpdev_read,      /* WTF */
	dummydev_write,
	dummydev_stat,
	dummydev_lseek,
	dummydev_datime,
	dummydev_ioctl,
	dummydev_delete,
	0, /* getc */
	0, /* getline */
	0, /* putc */
};

const char *cannot_install = "Cannot install device %S\r\n";
long sprintf_params[10];

/* in assembler part */
extern MX_DDEV dummydev GNU_ASM_NAME("dummydev");

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

long dummydev_init(const char *name, const struct dev_descr *descr)
{
	long r;

	/*
	 * BUG/FIXME: descr is ignored here,
	 * and the dummydev driver has only a single active callback,
	 * which points to arpdev_read
	 */
	UNUSED(descr);
	r = Dcntl(DEV_M_INSTALL, name, (long) &dummydev);
	if (r < 0)
	{
		char message[200];

		sprintf_params[0] = (long)name;
		p_kernel->_sprintf(message, cannot_install, sprintf_params);
		(void) Cconws(message);

		return -1;
	}

	return 0;
}

/*** ---------------------------------------------------------------------- ***/

long cdecl dummydev_open(MX_DOSFD *f)
{
	/* Nothing to do */
	UNUSED(f);
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

long cdecl dummydev_write(MX_DOSFD *f, long nbytes, void *buf)
{
	UNUSED(f);
	UNUSED(nbytes);
	UNUSED(buf);
	return EACCES;
}

/*** ---------------------------------------------------------------------- ***/

long cdecl dummydev_lseek(MX_DOSFD *f, long where, short whence)
{
	switch (whence)
	{
	case SEEK_SET:
		f->fd_fpos = where;
		return f->fd_fpos;

	case SEEK_CUR:
		f->fd_fpos += where;
		return f->fd_fpos;

	case SEEK_END:
		return EACCES;
	}

	return ENOSYS;
}

/*** ---------------------------------------------------------------------- ***/

long cdecl dummydev_ioctl(MX_DOSFD *f, short mode, void *buf)
{
	UNUSED(f);
	switch (mode)
	{
	case FIONREAD:
		*(long *) buf = UNLIMITED;
		return 0;

	case FIONWRITE:
		*(long *) buf = UNLIMITED;
		return 0;

	default:
		return ENOSYS;
	}
}

/*** ---------------------------------------------------------------------- ***/

long cdecl dummydev_datime(MX_DOSFD *f, short *timeptr, short rwflag)
{
	UNUSED(f);
	if (!rwflag)
	{
		timeptr[0] = Tgettime();
		timeptr[1] = Tgetdate();
	}

	return 0;
}

/*** ---------------------------------------------------------------------- ***/

long cdecl dummydev_close(MX_DOSFD *f)
{
	/* Nothing to do */
	UNUSED(f);
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

long cdecl dummydev_delete(MX_DOSFD *f, MX_DOSDIR *dir)
{
	/* Nothing to do */
	UNUSED(f);
	UNUSED(dir);
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

long cdecl dummydev_stat(MX_DOSFD *f, MAGX_UNSEL *unsel, short rwflag, long /* APPL * */ appl)
{
	/* Nothing to do */
	UNUSED(f);
	UNUSED(unsel);
	UNUSED(rwflag);
	UNUSED(appl);
	return 1;
}
