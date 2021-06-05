/*
	execxxx stuff for MiNT/TOS; written by Eric R. Smith, and
	placed in the public domain

	uo, 3.5.93: replaced findfile -> buffindfile (no static buffer
		anymore)
*/

#include <stdarg.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <support.h>

/* execvp, execlp: try to execute a program on the default system
   execution path. WARNING: the current directory is always searched
   first.
*/

static char const *const extensions[] = { "ttp", "prg", "tos", NULL };

#ifdef __STDC__
int execvp(const char *name, char *const *argv)
#else
int execvp(name, argv)
char *name;
char **argv;
#endif
{
	const char *execname;
	char buffer[PATH_MAX];

	execname = _buffindfile(name, getenv("PATH"), extensions, buffer);
	if (!execname)
	{
		errno = ENOENT;
		return -1;						/* file not found */
	}
	return _spawnve(P_OVERLAY, execname, argv, (char **) NULL);
}

#ifdef __STDC__
int execlp(const char *name, ...)
#else
int execlp(name)
char *name;
#endif
{
	va_list args;

	va_start(args, name);
	return execvp(name, (char **) args);
}
