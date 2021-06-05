/* psignal: print an error message describing a signal */
/* this is very TOS specific!! */
/* Written by ERS and placed in the public domain      */
/* check for valid prefix & changed fputs to low level write call (er) */

#include <signal.h>
#include <siglist.h>
#include <string.h>
#include <unistd.h>
#include "lib.h"

void psignal(sig, prefix)
int sig;
const char *prefix;
{
	_write(2, "\r\n", 2L);
	if (prefix && *prefix)
	{
		_write(2, prefix, (long) strlen(prefix));
		_write(2, ": ", 2L);
	}
	if (sig > 0 && sig < NSIG)
		_write(2, sys_siglist[sig], (long) strlen(sys_siglist[sig]));
	else
		_write(2, "unknown signal", 14L);
	_write(2, "\r\n", 2L);
}
