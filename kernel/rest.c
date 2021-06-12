#include "sockets.h"
#include "iov.h"
#include "dummydev.h"
#include "timeout.h"
#include "iov.h"

void inet4_init(void)
{
}


void x1c022(void)
{
	so_connect(0, 0, 0, 0, 0); /* XXX */
	so_register(0, 0);
	so_rselect(0, 0);
	so_wselect(0, 0);
	so_xselect(0, 0);
	iov2buf_cpy(0, 0, 0, 0, 0);
	buf2iov_cpy(0, 0, 0, 0, 0);
	dummydev_init(0, 0);
}


int sleep(int queue, long cond)
{
	UNUSED(queue);
	UNUSED(cond);
	return 0;
}


void wake(int queue, long cond)
{
	/* NOT IMPLEMENTED YET */
	UNUSED(queue);
	UNUSED(cond);
}

void wakeselect(long proc)
{
	/* NOT IMPLEMENTED YET */
	UNUSED(proc);
}


TIMEOUT *addroottimeout(long delta, void (*func)(struct proc *, long), ushort flags)
{
	(void)delta;
	(void)func;
	(void)flags;
	return 0;
}


void cancelroottimeout(TIMEOUT *which)
{
	(void)which;
}


long iov_size (const struct iovec *iov, long n)
{
	(void)iov;
	return n;
}


long unixtime(unsigned short time, unsigned short date)
{
	(void)time;
	(void)date;
	return 0;
}


long so_free(struct socket *so)
{
	(void)so;
	return 0;
}
