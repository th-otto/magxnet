#include "sockets.h"
#include "mxkernel.h"
#include "sockdev.h"
#include <fcntl.h>

struct dom_ops *alldomains = NULL;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

struct socket *so_alloc(void)
{
	struct socket *s;
	
	s = p_kernel->mxalloc(sizeof(*s), MX_PREFTTRAM, _BasPag);
	if (s == NULL)
		return NULL;
	s->type = SOCK_NONE;
	s->state = SS_VIRGIN;
	s->flags = 0;
	s->conn = NULL;
	s->iconn_q = NULL;
	s->next = NULL;
	s->ops = NULL;
	s->data = NULL;
	s->error = 0;
	s->pgrp = 0;
	s->rsel = 0;
	s->wsel = 0;
	s->xsel = 0;
	/* BUG: date/time not initialized */
	s->lockpid = 0;
	return s;
}

/*** ---------------------------------------------------------------------- ***/

long so_release(struct socket *so)
{
	struct socket *peer;
	short ostate;
	long r = 0;

	ostate = so->state;
	if (ostate != SS_VIRGIN && ostate != SS_ISDISCONNECTED)
	{
		so->state = SS_ISDISCONNECTING;
		so->flags |= SO_CLOSING;

		/* Tell all clients waiting for connections that we are
		 * closing down. This is done by setting there `conn'-field
		 * to zero and waking them up.
		 * NOTE: setting clients state to SS_ISDISCONNECTING here
		 * causes the client not to be able to try a second connect(),
		 * unless somewhere else its state is reset to SS_ISUNCONNECTED
		 */
		if (so->flags & SO_ACCEPTCON)
		{
			while ((peer = so->iconn_q) != NULL)
			{
				so->iconn_q = peer->next;
				peer->state = SS_ISDISCONNECTING;
				peer->conn = 0;
				peer->ops->abort(peer, SS_ISCONNECTING);
			}
		}

		/* Remove ourselves from the incomplete connection queue of
		 * some server. If we are on any queue, so->state is the
		 * server we are connecting to.
		 * so->state is set to 0 afterwards to indicate that
		 * connect() failed.
		 */
		if (ostate == SS_ISCONNECTING)
		{
			struct socket *last, *server;

			server = so->conn;
			if (server)
			{
				last = server->iconn_q;
				if (last == so)
				{
					server->iconn_q = so->next;
				} else
				{
					while (last && (last->next != so))
						last = last->next;
					if (last)
						last->next = so->next;
				}
				so->conn = 0;
			}
		}

		/* Tell the peer we are closing down, but let the underlying
		 * protocol do its part first.
		 */
		if (ostate == SS_ISCONNECTED && so->conn)
		{
			peer = so->conn;
			so->conn = 0;
			peer->state = SS_ISDISCONNECTING;
			so->ops->abort(peer, SS_ISCONNECTED);
		}

		r = so->ops->detach(so);
		if (r == 0)
		{
			/* No protocol data attached anymore, so we are
			 * disconnected.
			 */
			so->state = SS_ISDISCONNECTED;

			/* Wake anyone waiting for `so', since its state
			 * changed.
			 */
			wake(IO_Q, (long) so);
			so_wakersel(so);
			so_wakewsel(so);
			so_wakexsel(so);
		} else
		{
			ALERT("so_release: so->ops->detach failed!");
		}
	}

	return r;
}

/*** ---------------------------------------------------------------------- ***/

void so_sockpair(struct socket *so1, struct socket *so2)
{
	so1->conn = so2;
	so2->conn = so1;
	so1->state = SS_ISCONNECTED;
	so2->state = SS_ISCONNECTED;
}

/*** ---------------------------------------------------------------------- ***/

/* Put `client' on the queue of incomplete connections of `server'.
 * Blocks until the connection is accepted or it's impossible to
 * establish the connection, unless `nonblock' != 0.
 * `Backlog' is the number of pending connections allowed for `server'.
 * so_connect() will fail if there are already `backlog' clients on the
 * server queue.
 * NOTE: Before using this function to connect `client' to `server',
 * you should do the following:
 *	if (client->state == SS_ISCONNECTING) return EALREADY;
 */
long so_connect(struct socket *server, struct socket *client, short backlog, short nonblock, short wakeup)
{
	struct socket *last;
	short clients;

	if (!(server->flags & SO_ACCEPTCON))
	{
		DEBUG(("sockdev: so_connect: server is not listening"));
		return ENOSYS; /* BUG: should be EINVAL */
	}

	/* Put client on the incomplete connection queue of server. */
	client->next = 0;
	last = server->iconn_q;
	if (last)
	{
		for (clients = 1; last->next; last = last->next)
			++clients;
		if (clients >= backlog)
		{
			DEBUG(("sockdev: so_connect: backlog exeeded"));
			return ETIMEDOUT;
		}
		last->next = client;
	} else
	{
		if (backlog == 0)
		{
			DEBUG(("sockdev: so_connect: backlog exeeded"));
			return ETIMEDOUT;
		}
		server->iconn_q = client;
	}
	client->state = SS_ISCONNECTING;
	client->conn = server;

	/* Wake proc's selecting for reading on server, which are waiting
	 * for a connection request on the listening server.
	 */
	if (wakeup)
	{
		so_wakersel (server);
		wake(IO_Q, (long) server);
	}

	if (nonblock)
		return EINPROGRESS;

	while (client->state == SS_ISCONNECTING)
	{
		if (sleep(IO_Q, (long) client))
		{
			/* Maybe someone closed the client on us. */
			return EINTR;
		}
		if (!client->conn)
		{
			/* Server rejected us from its queue. */
			DEBUG(("sockdev: so_connect: connection refused"));
			return ECONNREFUSED;
		}
	}

	/* Now we are (at least were) connected to server. */
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

/* Register a new domain `domain'. Note that one can register several
 * domains with the same `domain' value. When looking up a domain, the
 * one which was last installed is chosen.
 */
void so_register(short domain, struct dom_ops *ops)
{
	DEBUG(("sockets: registering domain %i (ops %p)", domain, ops));

	ops->domain = domain;
	ops->next = alldomains;
	alldomains = ops;
}

/*** ---------------------------------------------------------------------- ***/

long so_rselect(struct socket *so, long proc)
{
	if (so->rsel)
		return 0; /* BUG: should return error */
	
	so->rsel = (void *)proc;
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

long so_wselect(struct socket *so, long proc)
{
	if (so->wsel)
		return 0; /* BUG: should return error */
	
	so->wsel = (void *)proc;
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

long so_xselect(struct socket *so, long proc)
{
	if (so->xsel)
		return 0; /* BUG: should return error */
	
	so->xsel = (void *)proc;
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

void so_wakersel(struct socket *so)
{
	if (so->rsel)
		wakeselect((long)so->rsel);
}

/*** ---------------------------------------------------------------------- ***/

void so_wakewsel(struct socket *so)
{
	if (so->wsel)
		wakeselect((long)so->wsel);
}

/*** ---------------------------------------------------------------------- ***/

void so_wakexsel(struct socket *so)
{
	if (so->xsel)
#if 0
		wakeselect((long)so->xsel);
#else
		(void) Cconws("I'll wakexselect you later...\r\n"); /* WTF? */
#endif
}


