#include "stsocket.h"
#include "mintsock.h"

/* Check the first NFDS descriptors each in READFDS (if not NULL) for read
   readiness, in WRITEFDS (if not NULL) for write readiness, and in EXCEPTFDS
   (if not NULL) for exceptional conditions.  If TIMEOUT is not NULL, time out
   after waiting the interval specified therein.  Returns the number of ready
   descriptors, or -1 for errors.  

   The function is currently emulated by means of poll().  This is 
   sub-optimal as long as NFDS is less or equal 32 because then we 
   waste quite some time by copying the file descriptor masks into
   struct poll.  Better poll will only be called when the native
   Fselect is not able to handle the call.  For the time being I want
   to test poll() and therefore ignore this problem.  
   
   If poll() has set POLLERR, POLLHUP, POLLNVAL or POLLMSG for any of the 
   polled descriptors, we simply mark that descriptor as ready for reading,
   writing or urgent reading so that the caller will get informed.  */
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	int retval;
	unsigned long msec_timeout;
	unsigned short this_timeout;
	unsigned long rmask;
	unsigned long wmask;
	unsigned long xmask;

	(void)nfds; /* not supported with Fselect() */
	rmask = 0;
	wmask = 0;
	xmask = 0;	
	/* Three loops are more efficient than one here.  */
	if (readfds != NULL)
		rmask = readfds->__fds_bits[0];
	if (writefds != NULL)
		wmask = writefds->__fds_bits[0];
	if (exceptfds != NULL)
		xmask = exceptfds->__fds_bits[0];

	if (timeout != NULL)
	{
		msec_timeout = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
		if (msec_timeout == 0)
			msec_timeout = 1;
	} else
	{
		msec_timeout = 0;
	}

	for (;;)
	{
		if (msec_timeout > USHRT_MAX)
		{
			this_timeout = USHRT_MAX;
		} else
		{
			this_timeout = msec_timeout;
		}
		msec_timeout -= this_timeout;
		retval = (int)Fselect(this_timeout, (void *)readfds->__fds_bits, (void *)writefds->__fds_bits, (void *)exceptfds->__fds_bits);
		if (retval < 0)
		{
			__set_errno(-retval);
			return -1;
		}
		if (retval != 0)
			break;
		if (msec_timeout == 0)
			break;
		if (readfds != NULL)
			readfds->__fds_bits[0] = rmask;
		if (writefds != NULL)
			writefds->__fds_bits[0] = wmask;
		if (exceptfds != NULL)
			exceptfds->__fds_bits[0] = xmask;
	}

	return retval;
}