/*
 * Adopted to Mint-Net 1994, Kay Roemer
 *
 * Modified to support Pure-C, Thorsten Otto.
 */

/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "stsocket.h"

#ifdef __PUREC__
void bcopy (const void *__src, void *__dest, size_t __n);
void bzero (void *__s, size_t __n);
#endif

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
	bzero(buf, sizeof(HEADER));
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
			bcopy(data, cp, datalen);
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
			bcopy(data, cp, datalen);
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
			bcopy(newrr->r_data, cp, newrr->r_size);
			cp += newrr->r_size;
		}
		hp->ancount = htons(0);
		break;
#endif /* ALLOW_UPDATES */
	}
	return (int)(cp - buf);
}


