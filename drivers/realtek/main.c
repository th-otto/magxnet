/*
 *	Loadable MintNet network interface startup code.
 *
 *	06/22/94, Kay Roemer.
 */

#include "sockets.h"
#include "bpf.h"
#include "netinfo.h"
#include "timeout.h"
#include "kerinfo.h"
#include "mxkernel.h"


long cdecl init (struct kerinfo *ker, struct netinfo *net);
long driver_init(void);


long cdecl init (struct kerinfo *k, struct netinfo *n)
{
	KERNEL = k;
	p_kernel = (void *)k->drvchng; /* sockets.dev passes magic kernel info here */
	NETINFO = n;
	
	return (driver_init () != 0) ? 1 : 0;
}

struct netinfo *NETINFO;
struct kerinfo *KERNEL;
MX_KERNEL *p_kernel; /* MagiC kernel info */

