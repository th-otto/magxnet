#include "stsocket.h"

#if 0 /* unused */
static uint32_t _getlong(uint8_t *msgp)
{
	uint8_t *p = msgp;
	uint32_t u;

	u = *p++; u <<= 8;
	u |= *p++; u <<= 8;
	u |= *p++; u <<= 8;
	return u | *p;
}
#endif


static void __putshort(uint16_t s, uint8_t *msgp)
{
	msgp[1] = s;
	msgp[0] = s >> 8;
}


static void __putlong(uint32_t l, uint8_t *msgp)
{
	msgp[3] = l;
	msgp[2] = (l >>= 8);
	msgp[1] = (l >>= 8);
	msgp[0] = l >> 8;
}


/*
 * Set up default settings.  If the configuration file exist, the values
 * there will have precedence.  Otherwise, the server address is set to
 * INADDR_ANY and the default domain name comes from the gethostname().
 *
 * As of 4.4 BSD the default name server address is 127.0.0.1. So we do.
 *
 * The configuration file should only be used if you want to redefine your
 * domain or run without a server on your machine.
 *
 * Return 0 if completes successfully, -1 on error
 */
int res_init(void)
{
	FILE *fp;
	char *cp;
	char **pp;
	int n;
	char buf[BUFSIZ];
	int nserv = 0;						/* number of nameserver records read from file */
	int haveenv = 0;
	int havesearch = 0;

	_res.nsaddr.sin_addr = inet_makeaddr(IN_LOOPBACKNET, 1);
	_res.nsaddr.sin_family = htons(AF_INET);
	_res.nsaddr.sin_port = htons(NAMESERVER_PORT);

	/* Allow user to override the local domain definition */
	if ((cp = getenv("LOCALDOMAIN")) != NULL)
	{
		strncpy(_res.defdname, cp, sizeof(_res.defdname));
		haveenv++;
	}

	if ((fp = fopen(_PATH_RESCONF, "r")) != NULL)
	{
		/* read the config file */
		while (fgets(buf, (int)sizeof(buf), fp) != NULL)
		{
			/* read default domain name */
			if (strncmp(buf, "domain", sizeof("domain") - 1) == 0)
			{
				if (haveenv)			/* skip if have from environ */
					continue;
				cp = buf + sizeof("domain") - 1;
				while (*cp == ' ' || *cp == '\t')
					cp++;
				if ((*cp == '\0') || (*cp == '\n'))
					continue;
				strncpy(_res.defdname, cp, sizeof(_res.defdname) - 1);
				if ((cp = strchr(_res.defdname, '\n')) != NULL)
					*cp = '\0';
				havesearch = 0;
				continue;
			}
			/* set search list */
			if (strncmp(buf, "search", sizeof("search") - 1) == 0)
			{
				if (haveenv)			/* skip if have from environ */
					continue;
				cp = buf + sizeof("search") - 1;
				while (*cp == ' ' || *cp == '\t')
					cp++;
				if ((*cp == '\0') || (*cp == '\n'))
					continue;
				(void) strncpy(_res.defdname, cp, sizeof(_res.defdname) - 1);
				if ((cp = strchr(_res.defdname, '\n')) != NULL)
					*cp = '\0';
				/*
				 * Set search list to be blank-separated strings
				 * on rest of line.
				 */
				cp = _res.defdname;
				pp = _res.dnsrch;
				*pp++ = cp;
				for (n = 0; *cp && pp < _res.dnsrch + MAXDNSRCH; cp++)
				{
					if (*cp == ' ' || *cp == '\t')
					{
						*cp = 0;
						n = 1;
					} else if (n)
					{
						*pp++ = cp;
						n = 0;
					}
				}
				/* null terminate last domain if there are excess */
				while (*cp != '\0' && *cp != ' ' && *cp != '\t')
					cp++;
				*cp = '\0';
				*pp++ = 0;
				havesearch = 1;
				continue;
			}
			/* read nameservers to query */
			if (strncmp(buf, "nameserver", sizeof("nameserver") - 1) == 0 && nserv < MAXNS)
			{
				cp = buf + sizeof("nameserver") - 1;
				while (*cp == ' ' || *cp == '\t')
					cp++;
				if (*cp == '\0' || *cp == '\n')
					continue;
				if ((_res.nsaddr_list[nserv].sin_addr.s_addr = inet_addr(cp)) == INADDR_NONE)
				{
					_res.nsaddr_list[nserv].sin_addr.s_addr = INADDR_ANY;
					continue;
				}
				_res.nsaddr_list[nserv].sin_family = AF_INET;
				_res.nsaddr_list[nserv].sin_port = htons(NAMESERVER_PORT);
				nserv++;
				continue;
			}
		}
		if (nserv > 1)
			_res.nscount = nserv;
		(void) fclose(fp);
	}
	if (_res.defdname[0] == 0)
	{
		if (gethostname(buf, sizeof(_res.defdname)) == 0 && (cp = strchr(buf, '.')) != NULL)
			strcpy(_res.defdname, cp + 1);
	}

	/* find components of local domain that might be searched */
	if (havesearch == 0)
	{
		pp = _res.dnsrch;
		*pp++ = _res.defdname;
		for (cp = _res.defdname, n = 0; *cp; cp++)
			if (*cp == '.')
				n++;
		cp = _res.defdname;
		for (; n >= LOCALDOMAINPARTS && pp < _res.dnsrch + MAXDFLSRCH; n--)
		{
			cp = strchr(cp, '.');
			*pp++ = ++cp;
		}
		*pp++ = 0;
	}
	_res.options |= RES_INIT;
	return 0;
}


int res_send(const uint8_t *buf, int buflen, uint8_t *answer, int anslen)
{
	int n = 0;					/* Shut up compiler warning.  */
	int try;
	int v_circuit;
	int resplen;
	int ns;
	int gotsomewhere = 0;
	int connected = 0;
	int connreset = 0;
	uint16_t id;
	uint16_t len;
	uint8_t *cp;
	fd_set dsmask;
	struct timeval timeout;
	const HEADER *hp = (const HEADER *) buf;
	HEADER *anhp = (HEADER *) answer;
	struct iovec iov[2];
	int terrno = ETIMEDOUT;
	char junk[512];

	static int s = -1;	/* socket used for communications */
	static struct sockaddr no_addr;

	if ((_res.options & RES_INIT) == 0)
		res_init();
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
						Fclose(s);
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

					if ((n = (int)Fsendmsg(s, &msg, 0)) != (int)sizeof(len) + buflen)
					{
						terrno = -n;
						Fclose(s);
						s = -1;
						continue;
					}
				}
				/*
				 * Receive length & response
				 */
				cp = answer;
				len = sizeof(uint16_t);
				while (len != 0 && (n = (int)Fread(s, len, cp)) > 0)
				{
					cp += n;
					len -= n;
				}
				if (n <= 0)
				{
					terrno = -n;
					Fclose(s);
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
				while (len != 0 && (n = (int)Fread(s, len, cp)) > 0)
				{
					cp += n;
					len -= n;
				}
				if (n <= 0)
				{
					terrno = -n;
					Fclose(s);
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
						if ((n = (int)Fread(s, n, junk)) > 0)
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
				FD_ZERO(&dsmask);
				FD_SET(s, &dsmask);
				n = select(s + 1, &dsmask, NULL, NULL, &timeout);
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
					Fclose(s);
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
				Fclose(s);
				s = -1;
			}
			return resplen;
		}
	}
	if (s >= 0)
	{
		Fclose(s);
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
 * Formulate a normal query, send, and await answer.
 * Returned answer is placed in supplied buffer "answer".
 * Perform preliminary check of answer, returning success only
 * if no error is indicated and the answer count is nonzero.
 * Return the size of the response on success, -1 on error.
 * Error number is left in h_errno.
 * Caller must parse answer and determine whether it answers the question.
 */
int res_query(const char *name,			/* domain name */
			  int class, int type,		/* class and type of query */
			  uint8_t * answer,			/* buffer to put answer */
			  int anslen)				/* size of answer buffer */
{
	uint8_t buf[MAXPACKET];
	HEADER *hp;
	int n;

	if ((_res.options & RES_INIT) == 0)
		res_init();
	n = res_mkquery(QUERY, name, class, type, NULL, 0, NULL, buf, (int)sizeof(buf));

	if (n <= 0)
	{
		h_errno = NO_RECOVERY;
		return n;
	}
	n = res_send(buf, n, answer, anslen);
	if (n < 0)
	{
		h_errno = TRY_AGAIN;
		return n;
	}

	hp = (HEADER *) answer;
	if (hp->rcode != NOERROR || ntohs(hp->ancount) == 0)
	{
		switch (hp->rcode)
		{
		case NXDOMAIN:
			h_errno = HOST_NOT_FOUND;
			break;
		case SERVFAIL:
			h_errno = TRY_AGAIN;
			break;
		case NOERROR:
			h_errno = NO_DATA;
			break;
		case FORMERR:
		case NOTIMP:
		case REFUSED:
		default:
			h_errno = NO_RECOVERY;
			break;
		}
		return -1;
	}
	return n;
}


/*
 * Form all types of queries.
 * Returns the size of the result or -1.
 */
int res_mkquery(int op,					/* opcode of query */
				const char *dname,		/* domain name */
				int class, int type,	/* class and type of query */
				char *data,				/* resource record data */
				int datalen,			/* length of data */
				struct rrec *newrr,		/* new rr for modify or append */
				uint8_t *buf,				/* buffer to put query */
				int buflen)				/* size of buffer */
{
	HEADER *hp;
	uint8_t *cp;
	int n;
	uint8_t *dnptrs[10];
	uint8_t **dpp;
	uint8_t **lastdnptr;

	(void) newrr;

	/*
	 * Initialize header fields.
	 */
	if (buf == NULL || buflen < (int)sizeof(HEADER))
		return -1;
	memset(buf, 0, sizeof(HEADER));
	hp = (HEADER *) buf;
	hp->id = htons(++_res.id);
	hp->opcode = op;
	hp->pr = (_res.options & RES_PRIMARY) != 0;
	hp->rd = (_res.options & RES_RECURSE) != 0;
	hp->rcode = NOERROR;
	cp = buf + sizeof(HEADER);
	buflen -= (int)sizeof(HEADER);
	dpp = dnptrs;
	*dpp++ = buf;
	*dpp++ = NULL;
	lastdnptr = dnptrs + sizeof(dnptrs) / sizeof(dnptrs[0]);
	/*
	 * perform opcode specific processing
	 */
	switch (op)
	{
	case QUERY:
		if ((buflen -= NS_QFIXEDSZ) < 0)
			return -1;
		if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
			return -1;
		cp += n;
		buflen -= n;
		__putshort(type, cp);
		cp += 2;
		__putshort(class, cp);
		cp += 2;
		hp->qdcount = htons(1);
		if (op == QUERY || data == NULL)
			break;
		/*
		 * Make an additional record for completion domain.
		 */
		buflen -= NS_RRFIXEDSZ;
		if ((n = dn_comp(data, cp, buflen, dnptrs, lastdnptr)) < 0)
			return -1;
		cp += n;
		buflen -= n;
		__putshort(T_NULL, cp);
		cp += 2;
		__putshort(class, cp);
		cp += 2;
		__putlong(0, cp);
		cp += 4;
		__putshort(0, cp);
		cp += 2;
		hp->arcount = htons(1);
		break;

	case IQUERY:
		/*
		 * Initialize answer section
		 */
		if (buflen < 1 + NS_RRFIXEDSZ + datalen)
			return -1;
		*cp++ = '\0';					/* no domain name */
		__putshort(type, cp);
		cp += 2;
		__putshort(class, cp);
		cp += 2;
		__putlong(0, cp);
		cp += 4;
		__putshort(datalen, cp);
		cp += 2;
		if (datalen)
		{
			memcpy(cp, data, datalen);
			cp += datalen;
		}
		hp->ancount = htons(1);
		break;

#ifdef ALLOW_UPDATES
		/*
		 * For UPDATEM/UPDATEMA, do UPDATED/UPDATEDA followed by UPDATEA
		 * (Record to be modified is followed by its replacement in msg.)
		 */
	case UPDATEM:
	case UPDATEMA:

	case UPDATED:
		/*
		 * The res code for UPDATED and UPDATEDA is the same; user
		 * calls them differently: specifies data for UPDATED; server
		 * ignores data if specified for UPDATEDA.
		 */
	case UPDATEDA:
		buflen -= RRFIXEDSZ + datalen;
		if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
			return -1;
		cp += n;
		__putshort(type, cp);
		cp += 2;
		__putshort(class, cp);
		cp += 2;
		__putlong(0, cp);
		cp += 4;
		__putshort(datalen, cp);
		cp += 2;
		if (datalen)
		{
			memcpy(cp, data, datalen);
			cp += datalen;
		}
		if ((op == UPDATED) || (op == UPDATEDA))
		{
			hp->ancount = htons(0);
			break;
		}
		/* Else UPDATEM/UPDATEMA, so drop into code for UPDATEA */

	case UPDATEA:						/* Add new resource record */
		buflen -= RRFIXEDSZ + datalen;
		if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
			return -1;
		cp += n;
		__putshort(newrr->r_type, cp);
		cp += 2;
		__putshort(newrr->r_class, cp);
		cp += 2;
		__putlong(0, cp);
		cp += 4;
		__putshort(newrr->r_size, cp);
		cp += 2;
		if (newrr->r_size)
		{
			memcpy(cp, newrr->r_data, newrr->r_size);
			cp += newrr->r_size;
		}
		hp->ancount = htons(0);
		break;
#endif /* ALLOW_UPDATES */
	}
	return (int)(cp - buf);
}


/*
 * Perform a call on res_query on the concatenation of name and domain,
 * removing a trailing dot from name if domain is NULL.
 */
int res_querydomain(
	const char *name,
	const char *domain,
	int class, int type,	/* class and type of query */
	uint8_t *answer,	/* buffer to put answer */
	int anslen)			/* size of answer */
{
	char nbuf[2 * NS_MAXDNAME + 2];
	const char *longname = nbuf;
	int n;

	if (domain == NULL)
	{
		/*
		 * Check for trailing '.';
		 * copy without '.' if present.
		 */
		n = (int)strlen(name) - 1;
		if (name[n] == '.' && n < (int) sizeof(nbuf) - 1)
		{
			memcpy(nbuf, name, n);
			nbuf[n] = '\0';
		} else
		{
			longname = name;
		}
	} else
	{
		sprintf(nbuf, "%.*s.%.*s", NS_MAXDNAME, name, NS_MAXDNAME, domain);
	}

	return res_query(longname, class, type, answer, anslen);
}


char *__hostalias(const char *name)
{
	char *C1,
	*C2;
	FILE *fp;
	char *file;
	char buf[BUFSIZ];
	static char abuf[NS_MAXDNAME];

	file = getenv("HOSTALIASES");
	if (file == NULL || (fp = fopen(file, "r")) == NULL)
		return NULL;
	buf[sizeof(buf) - 1] = '\0';
	while (fgets(buf, (int)sizeof(buf), fp))
	{
		for (C1 = buf; *C1 && !isspace(*C1); ++C1) ;
		if (!*C1)
			break;
		*C1 = '\0';
		if (strcasecmp(buf, name) == 0)
		{
			while (isspace(*++C1))
				;
			if (!*C1)
				break;
			for (C2 = C1 + 1; *C2 && !isspace(*C2); ++C2)
				;
			abuf[sizeof(abuf) - 1] = *C2 = '\0';
			strncpy(abuf, C1, sizeof(abuf) - 1);
			fclose(fp);
			return abuf;
		}
	}
	fclose(fp);
	return NULL;
}


/*
 * Formulate a normal query, send, and retrieve answer in supplied buffer.
 * Return the size of the response on success, -1 on error.
 * If enabled, implement search rules until answer or unrecoverable failure
 * is detected.  Error number is left in h_errno.
 * Only useful for queries in the same name hierarchy as the local host
 * (not, for example, for host address-to-name lookups in domain in-addr.arpa).
 */
int res_search(const char *name,		/* domain name */
			   int class, int type,		/* class and type of query */
			   uint8_t *answer,			/* buffer to put answer */
			   int anslen)				/* size of answer */
{
	const char *cp;
	char **domain;
	int n;
	int ret;
	int got_nodata = 0;

	if ((_res.options & RES_INIT) == 0)
		res_init();

	__set_errno(0);
	h_errno = HOST_NOT_FOUND;			/* default, if we never query */
	for (cp = name, n = 0; *cp; cp++)
		if (*cp == '.')
			n++;
	if (n == 0 && (cp = __hostalias(name)) != NULL)
		return res_query(cp, class, type, answer, anslen);

	/*
	 * We do at least one level of search if
	 *  - there is no dot and RES_DEFNAME is set, or
	 *  - there is at least one dot, there is no trailing dot,
	 *    and RES_DNSRCH is set.
	 */
	if ((n == 0 && (_res.options & RES_DEFNAMES)) ||
		(n != 0 && *--cp != '.' && (_res.options & RES_DNSRCH)))
	{
		for (domain = _res.dnsrch; *domain; domain++)
		{
			ret = res_querydomain(name, *domain, class, type, answer, anslen);
			if (ret > 0)
				return ret;
			/*
			 * If no server present, give up.
			 * If name isn't found in this domain,
			 * keep trying higher domains in the search list
			 * (if that's enabled).
			 * On a NO_DATA error, keep trying, otherwise
			 * a wildcard entry of another type could keep us
			 * from finding this entry higher in the domain.
			 * If we get some other error (negative answer or
			 * server failure), then stop searching up,
			 * but try the input name below in case it's fully-qualified.
			 */
			if (errno == ECONNREFUSED)
			{
				h_errno = TRY_AGAIN;
				return -1;
			}
			if (h_errno == NO_DATA)
				got_nodata++;
			if ((h_errno != HOST_NOT_FOUND && h_errno != NO_DATA) || (_res.options & RES_DNSRCH) == 0)
				break;
		}
	}
	/*
	 * If the search/default failed, try the name as fully-qualified,
	 * but only if it contained at least one dot (even trailing).
	 * This is purely a heuristic; we assume that any reasonable query
	 * about a top-level domain (for servers, SOA, etc) will not use
	 * res_search.
	 */
	if (n && (ret = res_querydomain(name, NULL, class, type, answer, anslen)) > 0)
		return ret;
	if (got_nodata)
		h_errno = NO_DATA;
	return -1;
}
