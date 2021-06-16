/*
 *	This file contains the stuff needed to send SIGURG to processes
 *	when urgent data arrives.
 *
 *	94/05/12, Kay Roemer.
 */

#include "sockets.h"
#include "tcpsig.h"
#include "bpf.h"
#include "inet.h"
#include "mxkernel.h"
#include <fcntl.h>


static struct {
	short sig;
	short pgrp;
} signals[20];
short tcpd_fd;
static char tcpd_c;
const char *tcpd_pipe_name = "u:\\pipe\\tcpd";


void tcp_sendsig(struct tcb *tcb, short sig)
{
	int i;
	char dummy;
	
	if (tcb->data->sock == 0 || tcb->data->sock->pgrp == 0)
		return;
	if (p_kernel->version >= 4)
		return;
	for (i = 0; i < 20; i++)
	{
		if (signals[i].pgrp == 0)
		{
			signals[i].pgrp = tcb->data->sock->pgrp;
			signals[i].sig = sig;
			Fwrite(tcpd_fd, 1, &dummy);
			break;
		}
	}
}


void tcpd_thread(long arg)
{
	int fd;
	int i;
	
	x1bd00(arg + 0x200);
	Psigblock(-1);
	fd = (int)Fopen(tcpd_pipe_name, O_RDONLY);
	for (;;)
	{
		while (Fread(fd, 1, &tcpd_c) != 1)
			;
		for (i = 0; i < 20; i++)
		{
			if (signals[i].pgrp != 0)
			{
				(void) Pkill(signals[i].pgrp, signals[i].sig);
				signals[i].pgrp = 0;
			}
		}
	}
}
