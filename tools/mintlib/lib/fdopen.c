/* from Dale Schumacher's dLibs */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

extern int __mint;

FILE *fdopen(h, mode)
register int h;
register const char *mode;
{
	extern int __default_mode__;		/* see defmode.c */
	register int i,
	 iomode = 0,
		f = __default_mode__;
	register FILE *fp = NULL;
	void _getbuf __PROTO((FILE *));

	for (i = 0; (!fp && (i < _NFILE)); ++i)
		if (!(_iob[i]._flag & (_IORW | _IOREAD | _IOWRT)))
			fp = &_iob[i];				/* empty slot */
	if (!fp)
	{
		errno = EMFILE;
		return (NULL);
	}

	while (*mode)
	{
		switch (*mode++)
		{
		case 'r':
			f |= _IOREAD;
			break;
		case 'w':
			f |= _IOWRT;
			iomode |= (O_CREAT | O_TRUNC);
			break;
		case 'a':
			f |= _IOWRT;
			iomode |= (O_CREAT | O_APPEND);
			break;
		case '+':
			f &= ~(_IOREAD | _IOWRT);
			f |= _IORW;
			break;
		case 'b':
			f |= _IOBIN;
			break;
		case 't':
			f &= ~_IOBIN;
			break;
		default:						/* illegal file mode */
			return (NULL);
		}
	}
	if ((i = (f & (_IORW | _IOREAD | _IOWRT))) == 0)
		return (NULL);
	else if (i == _IOREAD)
		iomode |= O_RDONLY;
	else if (i == _IOWRT)
		iomode |= O_WRONLY;
	else
		iomode |= O_RDWR;

	if (isatty(h))
		f |= __mint ? (_IODEV | _IONBF | _IOBIN) : (_IODEV | _IONBF);
	else
		f |= _IOFBF;
	fp->_file = h;						/* file handle */
	fp->_flag = f;						/* file status flags */
	_getbuf(fp);
	return (fp);
}
