#include "sockets.h"
#include "iov.h"
#include "dummydev.h"
#include "timeout.h"
#include "iov.h"
#include "bpf.h"
#include "rawip.h"
#include "route.h"
#include "icmp.h"
#include "igmp.h"
#include "tcp.h"
#include "udp.h"
#include "masquera.h"
#include "asm_spl.h"


int sleep(int queue, long cond)
{
	UNUSED(queue);
	UNUSED(cond);
	return 0;
}


