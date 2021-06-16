/*
 *	Provide a MiNT kernel interface to device drivers.
 *
 *	(C) 2021 Thorsten Otto.
 */

#include "sockets.h"
#define NETINFO
#include "timeout.h"
#include "kerinfo.h"
#include "bpf.h"

long init_kerinfo(void)
{
	return 0;
}
