/* from Dale Schumacher's dLibs */
/* heavily modified by ers and jrb */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <osbind.h>
#include <mintbind.h>
#include <memory.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <param.h>
#include <ssystem.h>
#include "lib.h"

int errno;
int __mint;								/* 0 for TOS, MiNT version number otherwise */
int _pdomain;							/* errorcode of Pdomain call */

char _rootdir;							/* user's preferred root directory */

clock_t _starttime;						/* 200 HZ tick when we started the program */
clock_t _childtime;						/* time consumed so far by our children */
FILE _iob[_NFILE];						/* stream buffers initialized below */

/* functions registered by user for calling at exit */
typedef void (*ExitFn) __PROTO( (void));
ExitFn *_at_exit;
int _num_at_exit;						/* number of functions registered - 1 */
int no_ssystem;

/*
 * get MiNT version number. Since this has to be done in supervisor mode,
 * we might as well set the start-up time of the system here, too.
 */
/* this function is called in user mode if the kernel supports Ssystem() */

#ifndef ARP_HACK
static long getMiNT(void)
{
	long *cookie;

/* get the system time in 200HZ ticks from the BIOS _hz_200 variable */
	if (no_ssystem)
		_starttime = *((unsigned long *) 0x4baL);
	else
		_starttime = Ssystem(S_GETLVAL, 0x000004baL, NULL);

	_childtime = 0;

	if (no_ssystem)
	{
		cookie = *((long **) 0x5a0L);
		if (!cookie)
			__mint = 0;
		else
		{
			while (*cookie)
			{
				if (*cookie == 0x4d694e54L)
				{						/* MiNT */
					__mint = (int) cookie[1];
					return 0;
				}
				cookie += 2;
			}

		}

	} else
	{
		__mint = (int) Ssystem(S_GETCOOKIE, 0x4d694e54L, NULL);
		return 0;
	}
	__mint = 0;
	return 0;
}
#endif

/* supplied by the user */
__EXTERN int main __PROTO((int, char **, char **));

#if __GNUC__ > 1
/* in libgcc2.c */
__EXTERN void __do_global_dtors __PROTO((void));
#endif

#if defined(__TURBOC__) && !defined(__NO_FLOAT__)
void _fpuinit(void);					/* in PCFLTLIB.LIB */

long _fpuvect[10];
long _pfumode;
long _fpuctrl;
#endif

void _main __PROTO((long, char **, char **));

void _main(_argc, _argv, _envp)
long _argc;
char **_argv,
**_envp;
{
	register FILE *f;
	register int i;
	char *s,
	*pconv;
	extern int __default_mode__;		/* in defmode.c or defined by user */
	extern short _app;					/* tells if we're an application or acc */

#ifdef ARP_HACK
	long value;
#endif
	char *p,
	*tmp;
	size_t len,
	 cnt;

	_num_at_exit = 0;
	errno = 0;

#if defined(__TURBOC__) && !defined(__NO_FLOAT__)
	_fpuinit();
#endif

/*
 * check for MiNT
 */
	no_ssystem = (int)Ssystem(-1, NULL, NULL);
#ifdef ARP_HACK
	_starttime = get_sysvar((void *)0x4baL);
	_childtime = 0;
	if (Getcookie(0x4d694e54L, &value) == 0)
		__mint = (int)value;
	else
		__mint = 0;
#else
	if (no_ssystem)
		(void) Supexec(getMiNT);
	else
		(void) getMiNT();
#endif

	if (_app)
		_pdomain = Pdomain(1);			/* set MiNT domain */

/*
 * initialize UNIXMODE stuff. Note that this library supports only
 * a few of the UNIXMODE variables, namely "b" (binary mode default)
 * and "r<c>" (default root directory).
 */
	if ((s = getenv("UNIXMODE")) != 0)
	{
		while (*s)
		{
			if (*s == 'b')
				__default_mode__ = _IOBIN;
			else if (*s == 'r' && s[1])
				_rootdir = *++s;
			else if (*s == '.' && s[1])
				s++;					/* ignore */
			s++;
		}
	}

	if (_rootdir >= 'A' && _rootdir <= 'Z')
		_rootdir = _rootdir - 'A' + 'a';

/*
 * if we're running under MiNT, and the current drive is U:, then this
 * must be our preferred drive
 */
	if (!_rootdir && __mint >= 9)
	{
		if (Dgetdrv() == 'U' - 'A')
			_rootdir = 'u';
	}

	/* clear isatty status for dumped programs */
	for (i = 0; i < __NHANDLES; i++)
		__open_stat[i].status = FH_UNKNOWN;

/* if stderr is not re-directed to a file, force 2 to console
 * (UNLESS we've been run from a shell we trust, i.e. one that supports
 *  the official ARGV scheme, in which case we leave stderr be).
 */
	if (!*_argv[0] && isatty(2))
		(void) Fforce(2, -1);

	stdin->_flag = _IOREAD | _IOFBF | __default_mode__;
	stdout->_flag = _IOWRT | _IOLBF | __default_mode__;
	stderr->_flag = _IORW | _IONBF | __default_mode__;
	/* some brain-dead people read from stderr */

	for (i = 0, f = _iob; i < 3; ++i, ++f)
	{									/* flag device streams */
		if (isatty(f->_file = i))
			f->_flag |= (__mint ? _IODEV | _IOBIN : _IODEV);
		else if (f == stdout)
		{								/* stderr is NEVER buffered */
			/* if stdout re-directed, make it full buffered */
			f->_flag &= ~(_IOLBF | _IONBF);
			f->_flag |= _IOFBF;
		}
		_getbuf(f);						/* get a buffer */
	}
	for (i = 3; i < _NFILE; i++, f++)
	{
		f->_flag = 0;					/* clear flags, if this is a dumped program */
	}

	/* Fix up environment, if necessary. All variables listed in PCONVERT
	 * are affected (by default, only PATH will be converted).
	 * The "standard" path separators for PATH are
	 * ',' and ';' in the Atari world, but POSIX mandates ':'. This
	 * conflicts with the use of ':' as a drive separator, so we
	 * also convert names like A:\foo to /dev/A/foo
	 * NOTE: this conversion must be undone in spawn.c so that
	 * old fashioned programs will understand us!
	 */

	for (i = 0; (pconv = _envp[i]) != 0; i++)
	{
		if (!strncmp(pconv, "PCONVERT=", 9))
		{
			pconv += 9;
			break;
		}
	}

	for (i = 0; (s = _envp[i]) != 0; i++)
	{

		if (pconv)
		{

			p = pconv;
			while (*p)
			{

				tmp = p;
				len = 0;
				while (*tmp && *tmp != ',')
				{
					tmp++;
					len++;
				}

				if (!strncmp(s, p, len) && s[len] == '=')
				{
					size_t size;

					len++;
					tmp = s + len;		/* tmp now after '=' */
					cnt = 1;
					while (*tmp)
					{					/* count words */
						if (*tmp == ';' || *tmp == ',')
							cnt++;
						tmp++;
					}
					size = tmp - s + cnt * 5;
					_envp[i] = (char *) malloc(size);
					strncpy(_envp[i], s, len);
					_path_dos2unx(s + len, _envp[i] + len, size - len);
					_envp[i] = (char *) realloc(_envp[i], strlen(_envp[i]) + 1);
					break;
				}

				if (!*tmp)
					break;
				p = tmp + 1;
			}
		} else							/* ! pconv */
		{
			/* PATH is always converted */
			if (s[0] == 'P' && s[1] == 'A' && s[2] == 'T' && s[3] == 'H' && s[4] == '=')
			{
				size_t size;

				tmp = s + 5;			/* tmp now after '=' */
				cnt = 1;
				while (*tmp)
				{
					/* count words */
					if (*tmp == ';' || *tmp == ',')
						cnt++;
					tmp++;
				}
				size = tmp - s + cnt * 5;
				_envp[i] = (char *) malloc(size);
				strncpy(_envp[i], s, 5);
				_path_dos2unx(s + 5, _envp[i] + 5, size - 5);
				_envp[i] = (char *) realloc(_envp[i], strlen(_envp[i]) + 1);
				break;
			}
		}
	}

	/* ANSI-Draft: A return from the initial call to the main 
	 * function is equivalent to calling the exit function with
	 * the value returned by the main function as its argument. If
	 * the main function executes a return that specifies no
	 * value, the termination status returned to the host
	 * environment is undefined. [section 2.1.2.2]
	 */
	exit(main((int) _argc, _argv, _envp));
}

void _fclose_all_files()
{
	register int i,
	 f;

	for (i = 0; i < _NFILE; ++i)
	{
		f = _iob[i]._flag;
		if (f & (_IORW | _IOREAD | _IOWRT))
			if (_iob[i]._file <= 2)		/* only flush std. streams */
				fflush(&_iob[i]);
			else
				fclose(&_iob[i]);
	}
}

__EXITING exit(status)
int status;
{
	register int i;

	for (i = _num_at_exit - 1; i >= 0; --i)
		(*_at_exit[i]) ();

#if __GNUC__ > 1
	__do_global_dtors();
#endif

	_fclose_all_files();
	_exit(status);
}
