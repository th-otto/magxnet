/*
 *	Handle allocation of and test for used port numbers.
 *	Note: we assume that tehre is always a free port number
 *	we can use.
 *
 *	01/17/94, kay roemer.
 */

#include "sockets.h"
#include "port.h"
#include "bpf.h"

#include <netinet/in.h>


/*
 * Return true if port `port' is currently in use in the protocol
 * `sock' belongs to.
 */
short port_inuse(struct in_data *sock, in_port_t port)
{
	struct in_data *data;

	for (data = sock->proto->datas; data; data = data->next)
	{
		if (data->flags & IN_HASPORT && data->src.port == port)
			return 1;
	}

	return 0;
}

/*
 * Find the first socket with local port `port'.
 */
struct in_data *port_find(struct in_data *sock, in_port_t port)
{
	struct in_data *data;

	for (data = sock->proto->datas; data; data = data->next)
	{
		if (data->flags & IN_ISBOUND && data->src.port == port)
			break;
	}

	return data;
}

/*
 * Find the first socket with local port `port'.
 */
struct in_data *port_find_with_addr(struct in_data *sock, in_port_t port, in_addr_t addr)
{
	struct in_data *data;

	for (data = sock->proto->datas; data; data = data->next)
	{
		if ((data->flags & IN_ISBOUND) && data->src.port == port && data->src.addr == addr)
		{
			return data;
		}
	}

	return NULL;
}

/*
 * Allocate an unused port number in the range IPPORT_RESERVED <= port
 * <= IPPORT_USERRESERVED for the protocol `sock' belongs to.
 */
in_port_t port_alloc(struct in_data *sock)
{
	static in_port_t lastport = IPPORT_RESERVED - 1;
	struct in_data *data;

	do
	{
		if (++lastport > IPPORT_USERRESERVED)
			lastport = IPPORT_RESERVED;

		data = sock->proto->datas;
		for (; data; data = data->next)
		{
			if (data->flags & IN_HASPORT && data->src.port == lastport)
				break;
		}
	} while (data);

	return lastport;
}
