/* nlist for mintlib/toslib.
 * Written by S.N. Henson and released into the public domain.
 *
 * Currently uses just standard and GST extended symbol formats.
 */

#include <st-out.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <macros.h>
#include <nlist.h>
#include <string.h>

#define SYM_BUFSIZE 100
#define SYMSIZE sizeof(struct asym)
#define GSYM_SIZE 23

static int sym_fd;

static long symcount;

static int getsym __PROTO((struct asym * sptr));

/* Read a symbol from a file and put it in sptr */
static int getsym(sptr)
struct asym *sptr;
{
	static struct asym symbuf[SYM_BUFSIZE],
	*symptr;
	static long symread;

	if (!symread)
	{
		symread = min(sizeof(symbuf), symcount);
		if ((int) symread != read(sym_fd, symbuf, (int) symread))
		{
			close(sym_fd);
			symread = 0;
			errno = EREAD;				/* Premature EOF */
			return -1;
		}
		symptr = symbuf;
	}

	*sptr = *symptr++;
	symread -= SYMSIZE;

	if (symread < 0)
	{
		symread = 0;
		errno = EREAD;
		return -1;
	}

	return 0;
}

int nlist(file, nl)
char *file;
struct nlist *nl;
{

	struct nlist *p;

	struct aexec hbuf;

	struct asym sym;

	long nl_count;

	if (!file || !nl)
	{
		errno = EFAULT;
		return -1;
	}

	if ((sym_fd = open(file, O_RDONLY)) == -1)
		return -1;

	/* Read in file header */
	read(sym_fd, &hbuf, (int) sizeof(hbuf));

	/* Executable file ? */
	if (A_BADMAG(hbuf))
	{
		close(sym_fd);
		errno = ENOEXEC;
		return -1;
	}

	/* Any symbols? */
	if ((symcount = hbuf.a_syms) == 0)
	{
		errno = EDOM;
		return -1;
	}

	if (symcount < sizeof(struct asym))
	{
		close(sym_fd);
		errno = EREAD;
		return -1;
	}

	/* Seek to symbol table */
	if (A_SYMOFF(hbuf) != lseek(sym_fd, A_SYMOFF(hbuf), SEEK_SET))
	{
		close(sym_fd);
		errno = EREAD;
		return -1;
	}

	/* Count number of symbols to match */
	for (p = nl, nl_count = 0; p->n_name; p++)
	{
		nl_count++;
		p->n_value = p->n_type = 0;
	}

	if (!nl_count)
		return 0;

	do
	{
		char name[GSYM_SIZE];

		/* Get one symbol from file */
		if (getsym(&sym))
			return -1;

		symcount -= SYMSIZE;

		strncpy(name, sym.a_name, 8);

		/* If extended symbol, get rest of name */
		if ((sym.a_type & A_LNAM) == A_LNAM)
		{
			if ((symcount <= 0) || getsym((struct asym *) (name + 8)))
				return -1;
			symcount -= SYMSIZE;
			name[22] = 0;
		} else
			name[8] = 0;

		/* Check for match */
		for (p = nl; p->n_name; p++)
		{
			if (p->n_type || strcmp(name, p->n_name))
				continue;
			p->n_value = sym.a_value;
			p->n_type = sym.a_type;
			if (--nl_count == 0)
				return 0;
		}
	}
	while (symcount > 0);

	return (int) nl_count;
}
