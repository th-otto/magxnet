#ifndef _UTMP_H
#define _UTMP_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif

/*
 * Structure of utmp and wtmp files.
 *
 */
struct utmp {
	char	ut_line[8];		/* tty name */
	char	ut_name[8];		/* user id */
	char	ut_host[16];		/* host name, if remote */
	long	ut_time;		/* time on */
};

/*
 * This is to determine if a utmp entry does not correspond to a genuine user
 * (pseudo tty)
 */

#define nonuser(ut) ((ut).ut_host[0] == 0 && \
		strncmp((ut).ut_line, "tty", 3) == 0 \
			&& ((ut).ut_line[3] == 'p' \
			|| (ut).ut_line[3] == 'q' \
			|| (ut).ut_line[3] == 'r' \
			|| (ut).ut_line[3] == 's'))

/* Prototypes */

__EXTERN void _write_utmp __PROTO((const char *line, const char *name,
					const char *host, unsigned long time));
__EXTERN void _write_wtmp __PROTO((const char *line, const char *name,
					const char *host, unsigned long time));

#endif /* _UTMP_H */
