/* killpg() for MiNT */

#include <stdlib.h>
#include <errno.h>
#include <osbind.h>
#include <signal.h>
#include <unistd.h>
#include <mintbind.h>

/* vector of signal handlers (for TOS) */
extern __Sigfunc _sig_handler[];		/* in signal.c */

/* vector giving which signals are currently blocked from delivery (for TOS) */
extern long _sigmask;					/* in signal.c */

/* which signals are pending? */
extern long _sigpending;

extern short _have_pkill;

int killpg(pgrp, sig)
int pgrp,
	sig;
{
	long r;

	if (pgrp < 0)
	{
		errno = ERANGE;
		return -1;
	}
	r = Pkill(-pgrp, sig);
	/* (void)Syield(); */

	if (r == -EINVAL)
	{
		_have_pkill = 0;
		if (pgrp == 0 || pgrp == getpgrp())
			return kill(getpid(), sig);
	}
	if (r < 0)
	{
		errno = (int) -r;
		return -1;
	}
	return 0;
}
