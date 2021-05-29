/*
 * BUG: read() in this module apparently had a wrong prototype
 */
#define read real_read
/*
 * BUG: struct msghdr is wrong
 */
#define msghdr real_msghdr
#include "stsocket.h"
#undef read
#undef msghdr
int read(int fd, void *buf, unsigned int len);
struct msghdr
  {
    __ptr_t msg_name;		/* Address to send to/receive from.  */
    unsigned long msg_namelen;	/* Length of address data.  */

    struct iovec *msg_iov;	/* Vector of data to send/receive into.  */
    long msg_iovlen;		/* Number of elements in the vector.  */

    __ptr_t msg_control;	/* Access rights information.  */
    unsigned long msg_controllen;	/* Length of access rights information.  */
  };


static int s = -1;	/* socket used for communications */
static struct sockaddr no_addr;


int res_send(const uint8_t *buf, int buflen, uint8_t *answer, int anslen)
{
	/* try: d3 */
	/* n: d4 */
	/* ns: d5 */
	/* resplen: d6 */
	/* buflen: d7 */
	/* hp: a6 */
	/* cp: a4 */

	/* msg: 570 */
	/* truncate: 568 */
	/* buf: 564 */
	/* answer: 560 */
	/* anslen: 558 */
	/* v_circuit: 556 */
	/* gotsomewhere: 554 */
	/* connected: 552 */
	/* connreset: 550 */
	/* id: 548 */
	/* len: 546 */
	/* dsmask: 542 */
	/* timeout: 534 */
	/* anhp: 530 */
	/* iovec: 514 */
	/* terrno: 512 */
	
	int n;					/* Shut up compiler warning.  */
	int try, v_circuit, resplen, ns;
	int gotsomewhere = 0;
	int connected = 0;
	int connreset = 0;
	uint16_t id, len;
	uint8_t *cp;
	unsigned long dsmask;
	struct timeval timeout;
	const HEADER *hp = (const HEADER *) buf;
	HEADER *anhp = (HEADER *) answer;
	struct iovec iov[2];
	int terrno = ETIMEDOUT;
	char junk[512];

	if ((_res.options & RES_INIT) == 0)
		if (res_init() == -1)
		{
			return -1;
		}
	v_circuit = (_res.options & RES_USEVC) || buflen > NS_PACKETSZ;
	id = hp->id;
	/*
	 * Send request, RETRY times, or until successful
	 */
	for (try = 0; try < _res.retry; try++)
	{
		for (ns = 0; ns < _res.nscount; ns++)
		{
		  usevc:
			if (v_circuit)
			{
				int truncated = 0;

				/*
				 * Use virtual circuit;
				 * at most one attempt per server.
				 */
				try = _res.retry;
				if (s < 0)
				{
					s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
					if (s < 0)
					{
						terrno = errno;
						continue;
					}
					if (connect(s, (struct sockaddr *) &(_res.nsaddr_list[ns]), (socklen_t)sizeof(struct sockaddr)) < 0)
					{
						terrno = errno;
						close(s);
						s = -1;
						continue;
					}
				}
				/*
				 * Send length & message
				 */
				len = htons((uint16_t) buflen);
				iov[0].iov_base = &len;
				iov[0].iov_len = sizeof(len);
				iov[1].iov_base = (void *)buf;
				iov[1].iov_len = buflen;
				{
					struct msghdr msg;

					msg.msg_name = 0;
					msg.msg_namelen = 0;
					msg.msg_iov = iov;
					msg.msg_iovlen = 2;
					msg.msg_control = 0;
					msg.msg_controllen = 0;

					if (sendmsg(s, (void *)&msg, 0) != sizeof(len) + buflen)
					{
						terrno = errno;
						close(s);
						s = -1;
						continue;
					}
				}
				/*
				 * Receive length & response
				 */
				cp = answer;
				len = sizeof(uint16_t);
				while (len != 0 && (n = (int)read(s, cp, len)) > 0)
				{
					cp += n;
					len -= n;
				}
				if (n <= 0)
				{
					terrno = errno;
					close(s);
					s = -1;
					/*
					 * A long running process might get its TCP
					 * connection reset if the remote server was
					 * restarted.  Requery the server instead of
					 * trying a new one.  When there is only one
					 * server, this means that a query might work
					 * instead of failing.  We only allow one reset
					 * per query to prevent looping.
					 */
					if (terrno == ECONNRESET && !connreset)
					{
						connreset = 1;
						ns--;
					}
					continue;
				}
				cp = answer;
				if ((resplen = ntohs(*(uint16_t *) cp)) > anslen)
				{
					len = anslen;
					truncated = 1;
				} else
				{
					len = resplen;
				}
				while (len != 0 && (n = (int)read(s, cp, len)) > 0)
				{
					cp += n;
					len -= n;
				}
				if (n <= 0)
				{
					terrno = errno;
					close(s);
					s = -1;
					continue;
				}
				if (truncated)
				{
					/*
					 * Flush rest of answer
					 * so connection stays in synch.
					 */
					anhp->tc = 1;
					len = resplen - anslen;
					while (len != 0)
					{
						n = len > sizeof(junk) ? (int)sizeof(junk) : len;
						if ((n = (int)read(s, junk, n)) > 0)
							len -= n;
						else
							break;
					}
				}
			} else
			{
				/*
				 * Use datagrams.
				 */
				if (s < 0)
				{
					s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
					if (s < 0)
					{
						terrno = errno;
						continue;
					}
				}
				/*
				 * I'm tired of answering this question, so:
				 * On a 4.3BSD+ machine (client and server,
				 * actually), sending to a nameserver datagram
				 * port with no nameserver will cause an
				 * ICMP port unreachable message to be returned.
				 * If our datagram socket is "connected" to the
				 * server, we get an ECONNREFUSED error on the next
				 * socket operation, and select returns if the
				 * error message is received.  We can thus detect
				 * the absence of a nameserver without timing out.
				 * If we have sent queries to at least two servers,
				 * however, we don't want to remain connected,
				 * as we wish to receive answers from the first
				 * server to respond.
				 */
				if (_res.nscount == 1 || (try == 0 && ns == 0))
				{
					/*
					 * Don't use connect if we might
					 * still receive a response
					 * from another server.
					 */
					if (connected == 0)
					{
						if (connect(s, (struct sockaddr *) &_res.nsaddr_list[ns], (socklen_t)sizeof(struct sockaddr)) < 0)
						{
							continue;
						}
						connected = 1;
					}
					if (send(s, buf, buflen, 0) != buflen)
					{
						continue;
					}
				} else
				{
					/*
					 * Disconnect if we want to listen
					 * for responses from more than one server.
					 */
					if (connected)
					{
						connect(s, &no_addr, (socklen_t)sizeof(no_addr));
						connected = 0;
					}
					if (sendto(s, buf, buflen, 0,
							   (struct sockaddr *) &_res.nsaddr_list[ns], (socklen_t)sizeof(struct sockaddr)) != buflen)
					{
						continue;
					}
				}

				/*
				 * Wait for reply
				 */
				timeout.tv_sec = _res.retrans << try;
				if (try > 0)
					timeout.tv_sec /= _res.nscount;
				if (timeout.tv_sec <= 0)
					timeout.tv_sec = 1;
				timeout.tv_usec = 0;
			  wait:
			  	/* ugly hack here */
				dsmask = 0;
				dsmask |= 1L << s;
				n = select(s + 1, (fd_set *)&dsmask, NULL, NULL, &timeout);
				if (n < 0)
				{
					continue;
				}
				if (n == 0)
				{
					/*
					 * timeout
					 */
					gotsomewhere = 1;
					continue;
				}
				if ((resplen = recv(s, answer, anslen, 0)) <= 0)
				{
					continue;
				}
				gotsomewhere = 1;
				if (id != anhp->id)
				{
					/*
					 * response from old query, ignore it
					 */
					goto wait;
				}
				if (!(_res.options & RES_IGNTC) && anhp->tc)
				{
					/*
					 * get rest of answer;
					 * use TCP with same server.
					 */
					close(s);
					s = -1;
					v_circuit = 1;
					goto usevc;
				}
			}
			/*
			 * If using virtual circuits, we assume that the first server
			 * is preferred * over the rest (i.e. it is on the local
			 * machine) and only keep that one open.
			 * If we have temporarily opened a virtual circuit,
			 * or if we haven't been asked to keep a socket open,
			 * close the socket.
			 */
			if ((v_circuit && ((_res.options & RES_USEVC) == 0 || ns != 0)) || (_res.options & RES_STAYOPEN) == 0)
			{
				close(s);
				s = -1;
			}
			return resplen;
		}
	}
	if (s >= 0)
	{
		close(s);
		s = -1;
	}
	if (v_circuit == 0)
	{
		if (gotsomewhere == 0)
		{
			__set_errno(ECONNREFUSED);	/* no nameservers found */
		} else
		{
			__set_errno(ETIMEDOUT);		/* no answer obtained */
		}
	} else
	{
		__set_errno(terrno);
	}
	return -1;
}


/*
 * This routine is for closing the socket if a virtual circuit is used and
 * the program wants to close it.  This provides support for endhostent()
 * which expects to close the socket.
 *
 * This routine is not expected to be user visible.
 */
void _res_close(void)
{
	if (s != -1)
	{
		close(s);
		s = -1;
	}
}
