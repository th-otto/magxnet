/*
 * system(): execute a command, passed as a string
 *
 * Written by Eric R. Smith and placed in the public domain.
 *
 * Modified by Allan Pratt to call _unx2dos on redirect file names
 * and to call spawnvp() without calling fork() -- why bother?
 *
 * Modified by Frank Ridderbusch in _parseargs() to handle the case
 * >'file'. Without the modification, the quotes would end up in the
 * filename for redirection
 *
 */

#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <process.h>
#include <errno.h>
#include <file.h>
#include <osbind.h>
#include <unistd.h>
#include "lib.h"

#define isquote(c) ((c) == '\"' || (c) == '\'' || (c) == '`')
#define ARG_ERR	   ( (Argentry *) -1L )

/* struct. used to build a list of arguments for the command */

typedef struct argentry
{
	struct argentry *next;
	char string[1];
} Argentry;

static Argentry *_argalloc __PROTO((const char *s));
static void _argfree __PROTO((Argentry * p));
static Argentry *_parseargs __PROTO((const char *s));

/* allocate an Argentry that will hold the string "s" */

static Argentry *_argalloc(s)
const char *s;
{
	Argentry *x;

	x = (Argentry *) malloc((size_t) (sizeof(Argentry) + strlen(s) + 1));
	if (!x)
		return ARG_ERR;
	x->next = (Argentry *) 0;
	strcpy(x->string, s);
	return x;
}

/* free a list of Argentries */

static void _argfree(p)
Argentry *p;
{
	Argentry *oldp;

	while (p)
	{
		oldp = p;
		p = p->next;
		free(oldp);
	}
}

/* parse a string into a list of Argentries. Words are defined to be
 * (1) any sequence of non-blank characters
 * (2) any sequence of characters starting with a ', ", or ` and ending
 *     with the same character. These quotes are stripped off.
 * (3) any spaces after an unquoted > or < are skipped, so
 *     "ls > junk" is parsed as 'ls' '>junk'.
 */

static Argentry *_parseargs(s)
const char *s;
{
	Argentry *cur,
	*res;
	char buf[1024];
	char *t,
	 quote;

	res = cur = _argalloc("");

	for (;;)
	{
		t = buf;
	  again:
		while (isspace(*s))
			s++;
		if (!*s)
			break;
		if (isquote(*s))
		{
			quote = *s++;
			while (*s && *s != quote)
				*t++ = *s++;
			if (*s)
				s++;					/* skip final quote */
		} else
		{
			while (*s && !isspace(*s))
			{
				*t++ = *s++;
				if (isquote(*s))
					goto again;
			}
			if (*s && (*(s - 1) == '>' || *(s - 1) == '<'))
				goto again;
		}
		*t = 0;
		cur->next = _argalloc(buf);
		if ((cur = cur->next) == NULL)	/* couldn't alloc() */
			return ARG_ERR;
	}
	cur->next = (Argentry *) 0;
	cur = res;
	res = res->next;
	free(cur);
	return res;
}


/* Here is system() itself.
 * FIXME: we probably should do wildcard expansion.
 * also, should errno get set here??
 */

int system(s)
const char *s;
{
	Argentry *al,
	*cur;
	char **argv,
	*p;
	int argc,
	 i;
	char const *infile,
	*outfile;
	int infd = 0,
		outfd = 1,
		append = 0;
	int oldin = 0,
		oldout = 1;						/* hold the Fdup'd in, out */
	char path[PATH_MAX];
	int retval;

	if (!s)								/* check for system() supported ?? */
		return 1;
	al = _parseargs(s);					/* get a list of args */
	if (al == ARG_ERR)
	{									/* not enough memory */
		errno = ENOMEM;
		return -1;
	}

	infile = outfile = "";

/* convert the list returned by _parseargs to the normal char *argv[] */
	argc = i = 0;
	for (cur = al; cur; cur = cur->next)
		argc++;
	if ((argv = (char **) malloc((size_t) (argc * sizeof(char *)))) == NULL)
	{
		errno = ENOMEM;
		return -1;
	}
	for (cur = al; cur; cur = cur->next)
	{
		p = cur->string;
		if (*p == '>')
		{
			outfile = p + 1;
			if (*outfile == '>')
			{
				outfile++;
				append = 1;
			} else
				append = 0;
		} else if (*p == '<')
		{
			infile = p + 1;
		} else
			argv[i++] = p;
	}
	argv[i] = (char *) 0;

/* now actually run the program */

	if (*infile)
	{
		(void) _unx2dos(infile, path, sizeof(path));
		infd = (int) Fopen(path, 0);
		if (infd < __SMALLEST_VALID_HANDLE)
		{
			perror(infile);
			return (2);
		}
		oldin = (int) Fdup(0);
		(void) Fforce(0, infd);
	}
	if (*outfile)
	{
		(void) _unx2dos(outfile, path, sizeof(path));
		if (append)
		{
			outfd = (int) Fopen(path, 2);
			if (outfd < __SMALLEST_VALID_HANDLE)
				outfd = (int) Fcreate(path, 0);
			else
				(void) Fseek(0L, outfd, 2);
		} else
			outfd = (int) Fcreate(path, 0);
		if (outfd < __SMALLEST_VALID_HANDLE)
		{
			perror(outfile);
			return (2);
		}
		oldout = (int) Fdup(1);
		(void) Fforce(1, outfd);
	}

	retval = spawnvp(P_WAIT, argv[0], argv);

	if (*infile)
		(void) (Fforce(0, oldin), Fclose(oldin), Fclose(infd));
	if (*outfile)
		(void) (Fforce(1, oldout), Fclose(oldout), Fclose(outfd));
	free(argv);
	_argfree(al);
	return retval;
}
