#include "stsocket.h"

#ifdef __PUREC__
void bcopy (const void *__src, void *__dest, size_t __n);
#endif

#ifdef __PUREC__
extern const unsigned char *_ctype;
#define	_ISspace	0x10		/* whitespace */
#undef isspace
#define	isspace(c)	(_ctype[(unsigned char)((c))]&_ISspace)
#endif


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
			  uint8_t *answer,			/* buffer to put answer */
			  int anslen)				/* size of answer buffer */
{
	uint8_t buf[MAXPACKET];
	HEADER *hp;
	int n;

	if ((_res.options & RES_INIT) == 0 && res_init() == -1)
		return -1;
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

	if ((_res.options & RES_INIT) == 0 && res_init() == -1)
		return -1;

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
		(n < 2 && *--cp != '.' && (_res.options & RES_DNSRCH))) /* BUG? < 2 ? */
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
			bcopy(name, nbuf, n);
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


const char *__hostalias(const char *name)
{
	char *C1;
	char *C2;
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
		for (C1 = buf; *C1 && !isspace(*C1); ++C1)
			;
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
