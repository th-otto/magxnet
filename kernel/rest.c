#include "sockets.h"
#include "iov.h"

void x12306(void)
{
}


void x1b26e(void)
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
