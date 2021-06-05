/* from Henry Spencer's stringlib */
#include <string.h>

/* static char nullstr[] = "unknown error"; */
static char const _nullstr[] = "unknown error";

char const *const sys_errlist[] = {
	"OK",								/* 0 */
	"error",							/* 1 */
	"drive not ready",					/* 2 */
	"unknown command",					/* 3 */
	"crc error",						/* 4 */
	"bad request",						/* 5 */
	"seek error",						/* 6 */
	"unknown media",					/* 7 */
	"sector not found",					/* 8 */
	"out of paper",						/* 9 */
	"write failure",					/* 10 */
	"read failure",						/* 11 */
	"general mishap",					/* 12 */
	"media write protected",			/* 13 */
	"media changed",					/* 14 */
	"unknown device",					/* 15 */
	"bad sectors on format",			/* 16 */
	"disk swap request",				/* 17 */
	_nullstr,							/* 18 */
	_nullstr,							/* 19 */
	_nullstr,							/* 20 */
	_nullstr,							/* 21 */
	_nullstr,							/* 22 */
	_nullstr,							/* 23 */
	_nullstr,							/* 24 */
	_nullstr,							/* 25 */
	_nullstr,							/* 26 */
	_nullstr,							/* 27 */
	_nullstr,							/* 28 */
	_nullstr,							/* 29 */
	_nullstr,							/* 30 */
	_nullstr,							/* 31 */
	"invalid function number",			/* 32 */
	"file not found",					/* 33 */
	"path not found",					/* 34 */
	"no more handles",					/* 35 */
	"permission denied",				/* 36 */
	"invalid handle",					/* 37 */
	_nullstr,							/* 38 */
	"out of memory",					/* 39 */
	"invalid memory block",				/* 40 */
	_nullstr,							/* 41 */
	_nullstr,							/* 42 */
	_nullstr,							/* 43 */
	_nullstr,							/* 44 */
	_nullstr,							/* 45 */
	"invalid drive id",					/* 46 */
	_nullstr,							/* 47 */
	"link across devices",				/* 48 */
	"no more files",					/* 49 */
	_nullstr,							/* 50 */
	_nullstr,							/* 51 */
	_nullstr,							/* 52 */
	_nullstr,							/* 53 */
	_nullstr,							/* 54 */
	_nullstr,							/* 55 */
	_nullstr,							/* 56 */
	_nullstr,							/* 57 */
	"locking conflict",					/* 58 */
	_nullstr,							/* 59 */
	_nullstr,							/* 60 */
	_nullstr,							/* 61 */
	_nullstr,							/* 62 */
	_nullstr,							/* 63 */
	"range error/bad argument",			/* 64 */
	"internal error",					/* 65 */
	"bad executable format",			/* 66 */
	"memory block growth failure",		/* 67 */
	_nullstr,							/* 68 */
	_nullstr,							/* 69 */
	_nullstr,							/* 70 */
	_nullstr,							/* 71 */
	_nullstr,							/* 72 */
	_nullstr,							/* 73 */
	_nullstr,							/* 74 */
	_nullstr,							/* 75 */
	_nullstr,							/* 76 */
	_nullstr,							/* 77 */
	_nullstr,							/* 78 */
	_nullstr,							/* 79 */
	"too many symbolic links",			/* 80 */
	"broken pipe",						/* 81 */
	_nullstr,							/* 82 */
	_nullstr,							/* 83 */
	_nullstr,							/* 84 */
	"file exists",						/* 85 */
	"name too long",					/* 86 */
	"not a tty",						/* 87 */
	"range error",						/* 88 */
	"domain error",						/* 89 */
	"I/O error",						/* 90 */
	"no space on device",				/* 91 */
	_nullstr,							/* 92 */
	_nullstr,							/* 93 */
	_nullstr,							/* 94 */
	_nullstr,							/* 95 */
	_nullstr, _nullstr, _nullstr, _nullstr, _nullstr,	/* 96 - 100 */
	_nullstr, _nullstr, _nullstr, _nullstr, _nullstr,	/* 101 - 105 */
	_nullstr, _nullstr, _nullstr, _nullstr, _nullstr,	/* 106 - 110 */
	_nullstr, _nullstr, _nullstr, _nullstr, _nullstr,	/* 111 - 115 */
	_nullstr, _nullstr, _nullstr, _nullstr, _nullstr,	/* 116 - 120 */
	_nullstr, _nullstr, _nullstr, _nullstr, _nullstr,	/* 121 - 125 */
	_nullstr,							/* 126 */
	_nullstr,							/* 127 */
	"interrupted system call"			/* 128 */
};

short sys_nerr = (short) (sizeof(sys_errlist) / sizeof(sys_errlist[0]));

#ifdef __MINT__

/* Support for Kay Roemer's socket library */

char const *const _sock_errlist[] = {
	"Socket operation on non-socket",	/* 300 */
	"Destination address required",
	"Message too long",
	"Protocol wrong type for socket",
	"Protocol not available",
	"Protocol not supported",
	"Socket type not supported",
	"Operation not supported",
	"Protocol family not supported",
	"Address family not supported by protocol",
	"Address already in use",
	"Cannot assign requested address",
	"Network is down",
	"Network is unreachable",
	"Network dropped conn. because of reset",
	"Software caused connection abort",
	"Connection reset by peer",
	"Socket is already connected",
	"Socket is not connected",
	"Cannot send after shutdown",
	"Connection timed out",
	"Connection refused",
	"Host is down",
	"No route to host",
	"Operation already in progress",
	"Operation now in progress",
	"Operation would block"
};

short _sock_nerr = (short) (sizeof(_sock_errlist) / sizeof(char *));

#define MINSOCKERR	300
#define MAXSOCKERR	(MINSOCKERR + _sock_nerr)

#endif

/*
 * strerror - map error number to descriptive string
 *
 */

char *strerror(errnum)
short errnum;
{
	extern short sys_nerr;
	extern char const *const sys_errlist[];

#ifdef __MINT__
	extern char const *const _sock_errlist[];
#endif
	if (errnum >= 0 && errnum < sys_nerr)
		return ((char *) sys_errlist[errnum]);
#ifdef __MINT__
	else if (errnum >= MINSOCKERR && errnum < MAXSOCKERR)
		return ((char *) _sock_errlist[errnum - MINSOCKERR]);
#endif
	else
		return ((char *) _nullstr);
}
