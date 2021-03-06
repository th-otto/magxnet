#include "sockets.h"
#include "tcpsig.h"
#include "bpf.h"
#include "inet.h"
#include "mxkernel.h"
#include <fcntl.h>

extern short tcpd_fd;
const char *tcpd_pipe_name = "u:\\pipe\\tcpd";


extern void tcpd_thread(long arg);

#undef O_NONBLOCK
#define O_NONBLOCK 0x100
#undef O_CREAT
#define O_CREAT 0x200
#undef O_GLOBAL
#define O_GLOBAL 0x1000

#ifdef __PUREC__
/* uses binding with explicit hidden arg */
short _Mshrink(short zero, void *ptr, long size);
#define Mshrink(ptr, size) _Mshrink(0, ptr, size)
#endif



void tcpd_init(void)
{
	PD *pd;
	
	if (p_kernel->version >= 4)
		return;
	tcpd_fd = Fopen(tcpd_pipe_name, O_WRONLY | O_NONBLOCK | O_CREAT | O_GLOBAL);
	if (tcpd_fd < 100)
		tcpd_fd += 100;
	Fchmod(tcpd_pipe_name, 0600);
	pd = (PD *)Pexec(5, NULL, "", NULL);
	Mshrink(pd, 0x300);
	pd->p_tbase = (void *)tcpd_thread;
	pd->p_hitpa = (char *)pd + 0x300;
	Pexec(104, "tcpd", pd, NULL);
}



