/* public domain pause(), by ers */

#include <mintbind.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

extern int __mint;

int pause()
{
	if (__mint)
		(void) Pause();
	/* do nothing for TOS */
	errno = EINTR;
	return -1;
}

/* Public domain sigpause() - AGK */

void sigpause(mask)
long mask;
{
	long oldmask;

	if (__mint == 0)
	{
		/* for TOS, we just toggle the signal mask -- maybe
		 * there's a pending signal that we can receive.
		 */
		oldmask = sigsetmask(mask);
		sigsetmask(oldmask);
	}
/*
	else if (__mint <= 94) {
		oldmask = Psigsetmask(mask);
		(void)Pause();
		(void)Psigsetmask(oldmask);
	}
*/
	else
		(void) Psigpause(mask);
}
