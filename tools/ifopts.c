/*
 * option file parser for ifconfig.
 *
 * (w) 1995, Kay Roemer.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "ifopts.h"

static int line = 0;
static char *file = "<argv>";


#if __GNUC_PREREQ(8, 1)
/* ignore warnings from strncpy(), we *do* want to truncate these */
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif


int parse_hwaddr(char *hw, char *addr)
{
	static char parse_hwaddr_str[128];
	unsigned int x;
	unsigned int i;
	char *cp;

	strncpy(parse_hwaddr_str, addr, sizeof(parse_hwaddr_str));
	cp = strtok(parse_hwaddr_str, ":");
	for (i = 0; i < 6 && cp; ++i, cp = strtok(NULL, ":"))
	{
		if (sscanf(cp, "%i", &x) != 1 || x > 255)
			return -1;
		hw[i] = (char) x;
	}

	if (i < 6 || cp)
		return -1;

	return 0;
}


static int opt_parse(struct ifopt *ifo, char *option, char *value)
{
	char *cp;

	for (cp = option; *cp && isalnum(*cp); ++cp)
		;

	if (*cp)
	{
		fprintf(stderr, "%s:%d: warning: option '%s' contains " "non-alphanumerical characters\n", file, line, option);
	}

	if (strlen(option) >= sizeof(ifo->option))
	{
		fprintf(stderr, "%s:%d: option name '%s' too long, "
				"truncating to %ld chars\n", file, line, option, sizeof(ifo->option) - 1);
	}

	strncpy(ifo->option, option, sizeof(ifo->option));
	ifo->option[sizeof(ifo->option) - 1] = '\0';

	cp = value;
	if (*cp == '"')
	{
		/*
		 * a string
		 */
		++value;
		for (++cp; *cp && *cp != '"'; ++cp)
			;
		if (!*cp)
		{
			fprintf(stderr, "%s:%d: unterminated string\n", file, line);
			return -1;
		}

		*cp = '\0';
		if (strlen(value) >= sizeof(ifo->ifo_string))
		{
			fprintf(stderr, "%s:%d: string too long, skipping\n", file, line);
			return -1;
		}

		strcpy(ifo->ifo_string, value);
		ifo->valtype = IFO_STRING;
	} else if (*cp == '+' || *cp == '-' || isdigit(*cp))
	{
		/*
		 * Integer or address
		 */
		if (!parse_hwaddr(ifo->ifo_string, cp))
		{
			/*
			 * hw address
			 */
			ifo->valtype = IFO_HWADDR;
		} else if (sscanf(cp, "%li", &ifo->ifo_long) == 1)
		{
			/*
			 * integer
			 */
			ifo->valtype = IFO_INT;
		} else
		{
			fprintf(stderr, "%s:%d: invalid value for option " "'%s'\n", file, line, option);
			return -1;
		}
	} else
	{
		fprintf(stderr, "%s:%d: invalid value for option '%s'\n", file, line, option);
		return -1;
	}

	return 0;
}


static int opt_set(char *ifname, struct ifopt *ifo)
{
	struct ifreq ifr;

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_data = (void *) ifo;
	if (ioctl(sockfd, SIOCSIFOPT, &ifr) < 0)
	{
		switch (errno)
		{
		case ENOSYS:
			fprintf(stderr, "%s:%d: option '%s' not supported on " "interface %s\n", file, line, ifo->option, ifname);
			break;

		case ENOENT:
			fprintf(stderr, "%s:%d: bad value for option '%s'\n", file, line, ifo->option);
			break;

		default:
			fprintf(stderr, "%s:%d: option '%s': %s\n", file, line, ifo->option, strerror(errno));
			break;
		}

		return -1;
	}

	return 0;
}

#undef LINE_MAX
#define LINE_MAX 1024

int opt_file(char *fname, char *ifname)
{
	FILE *fp;
	static char opt_file_buf[LINE_MAX];
	char *cp;
	char *opt;
	char *val;
	struct ifopt ifo;

	file = fname;
	line = 1;
	fp = fopen(file, "r");
	if (!fp)
	{
		fprintf(stderr, "cannot open input file '%s'\n", file);
		return -1;
	}

	for (; fgets(opt_file_buf, LINE_MAX, fp); ++line)
	{
		if (strlen(opt_file_buf) > LINE_MAX - 10)
		{
			fprintf(stderr, "%s:%d: skipping too long line\n", file, line);
			continue;
		}

		/*
		 * remove comments
		 */
		if ((cp = strchr(opt_file_buf, '#')) != NULL)
			*cp = '\0';

		/*
		 * skip leading white space
		 */
		for (cp = opt_file_buf; *cp && isspace(*cp); ++cp)
			;

		if (!*cp)
			continue;

		/*
		 * first string of chars is option name
		 */
		opt = cp;

		for (; *cp && !isspace(*cp); ++cp)
			;

		if (!*cp)
		{
			fprintf(stderr, "%s:%d: option '%s' has no value\n", file, line, opt);
			continue;
		}

		*cp++ = '\0';

		/*
		 * skip white space between option name and value
		 */
		for (; *cp && isspace(*cp); ++cp)
			;

		if (!*cp)
		{
			fprintf(stderr, "%s:%d: option '%s' has no value\n", file, line, opt);
			continue;
		}
		val = cp;

		if (opt_parse(&ifo, opt, val) < 0)
			continue;

		opt_set(ifname, &ifo);
	}

	if (ferror(fp))
	{
		fprintf(stderr, "%s:%d: error reading input file\n", file, line);
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}
