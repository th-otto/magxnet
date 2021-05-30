#include <arpa/inet.h>

/*
 * Formulate an Internet address from network + host.  Used in
 * building addresses stored in the ifnet structure.
 */
struct in_addr inet_makeaddr(in_addr_t net, in_addr_t host)
{
	struct in_addr addr;

	if (net < 128)
		addr.s_addr = (net << IN_CLASSA_NSHIFT) | (host & IN_CLASSA_HOST);
	else if (net < 65536UL)
		addr.s_addr = (net << IN_CLASSB_NSHIFT) | (host & IN_CLASSB_HOST);
	else if (net < 16777216L)
		addr.s_addr = (net << IN_CLASSC_NSHIFT) | (host & IN_CLASSC_HOST);
	else
		addr.s_addr = net | host;
	addr.s_addr = htonl(addr.s_addr);
	return addr;
}
