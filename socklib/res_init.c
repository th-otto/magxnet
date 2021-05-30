#include "stsocket.h"

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
	_res.nscount = 1;

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
				if ((cp = index(_res.defdname, '\n')) != NULL)
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
				if ((cp = index(_res.defdname, '\n')) != NULL)
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
				if ((_res.nsaddr_list[nserv].sin_addr.s_addr = inet_addr(cp)) == 0xffffUL) /* BUG: should be INADDR_NONE */
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
		if (gethostname(buf, sizeof(_res.defdname)) == 0 && (cp = index(buf, '.')) != NULL)
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
			cp = index(cp, '.');
			*pp++ = ++cp;
		}
		*pp++ = 0;
	}
	_res.options |= RES_INIT;
	return 0;
}
