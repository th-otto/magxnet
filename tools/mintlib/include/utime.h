#ifndef _U_TIME_H
#define _U_TIME_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif

struct utimbuf {			/* type for times() call */
	time_t	actime;
	time_t	modtime;
};

__EXTERN int utime __PROTO((const char *path, const struct utimbuf *times));

#endif  /* _U_TIME_H */
