#include <stdio.h>
#include <fcntl.h>
#if defined(__PUREC__) && defined(__STDIO)
# define SYSHDR OSHEADER
# define _run p_run
# include <tos.h>
# define _base _BasPag
#else
# include <mint/mintbind.h>
# include <mint/sysvars.h>
# include <mint/basepage.h>
#endif
#if defined(__PUREC__)
# include "../include/mt_aes.h"
#else
# include <gemx.h>
#endif
struct netif { int dummy; };
#include "sockdev.h"

#define C_SCKM 0x53434B4DL     /* MagXNet (SOCKET.DEV) */
#define FIONREAD	(('F'<< 8) | 1)
#define FIONWRITE	(('F'<< 8) | 2)

#define NSLBUFS		4

static struct magxnet_cookie *sockets_dev;
static long cookie;
static short apid;
static struct slbuf *dev_table;
extern struct slbuf *currdev;

static char const not_installed[] = " MagiCNet device driver NOT installed!\r\n";


static void do_work(void);


static long get_jar(void)
{
	return *((long *)0x5a0);
}


static long *get_cookie(long id, long *value)
{
	long *jar;
	
	jar = (long *)Supexec(get_jar);
	if (jar != NULL)
	{
		while (jar[0] != 0)
		{
			if (jar[0] == id)
			{
				if (value)
					*value = *++jar;
				return jar;
			}
			jar += 2;
		}
	}
	return NULL;
}


static void mainloop(void)
{
	EVNT event;
	int done;
	
	done = 0;
	do
	{
		mt_EVNT_multi(MU_TIMER | MU_MESAG,
			1, 1, 0,
			NULL, NULL,
			30,
			&event, NULL);
		if (event.mwhich & MU_MESAG)
		{
			if (event.msg[0] == AP_TERM)
			{
				mt_evnt_timer(1000, NULL);
				sockets_dev->Fopen = NULL;
				sockets_dev->Fclose = NULL;
				done = 1;
				break;
			}
		}
		if (event.mwhich & MU_TIMER)
		{
			do_work();
		}
	} while (!done);
}


static long my_Fopen(const char *filename)
{
	void **p_run;
	void *run_save;
	long ret;
	OSHEADER *osheader;

#ifdef __GNUC__
	/*
	 * function is called with Pure-C calling convention,
	 * with filename being passed in A0
	 */
	{
		register void *a0 __asm__("a0");
		__asm__ __volatile__(
			"move.l %1,%0\n"
		: "=r"(filename)
		: "r"(a0)
		: "cc");
	}
#endif
	osheader = *((OSHEADER **)0x4f2);
	p_run = (void **)osheader->p_run;
	run_save = *p_run;
	*p_run = _base;
	ret = Fopen(filename, O_RDWR);
	*p_run = run_save;
	return ret;
}


static short my_Fclose(short fd)
{
	void **p_run;
	void *run_save;
	short ret;
	OSHEADER *osheader;
	
#ifdef __GNUC__
	/*
	 * function is called with Pure-C calling convention,
	 * with fd being passed in D0
	 */
	{
		register short d0 __asm__("d0");
		__asm__ __volatile__(
			"move.w %1,%0\n"
		: "=r"(fd)
		: "r"(d0)
		: "cc");
	}
#endif
	osheader = *((OSHEADER **)0x4f2);
	p_run = (void **)osheader->p_run;
	run_save = *p_run;
	*p_run = _base;
	ret = Fclose(fd);
	*p_run = run_save;
	return ret;
}


int main(void)
{
	apid = mt_appl_init(NULL);
	
	if (apid == -1)
		return apid;
	if (get_cookie(C_SCKM, &cookie) == NULL)
	{
		(void) Cconws(not_installed);
	} else if (cookie)
	{
		sockets_dev = (struct magxnet_cookie *)cookie;
		sockets_dev->Fopen = my_Fopen;
		sockets_dev->Fclose = my_Fclose;
		dev_table = sockets_dev->dev_table;
		if (dev_table)
			mainloop();
	}
	mt_appl_exit(NULL);
	return 0;
}


static long write_dev(void)
{
#if defined(__PUREC__)
	/* BUG: called with Pure-C calling convention */
	currdev->send(currdev);
#elif defined(__GNUC__)
	{
		/*
		 * take care to call it in a way that works both for cdecl
		 * and Pure-C calling conventions, since there seem
		 * to be drivers around that were compiled by it.
		 */
		register void *a0 __asm__("a0") = currdev;
		register void *a1 __asm__("a1") = currdev->send;
		__asm__ __volatile__(
			"\tmove.l %0,-(%%sp)\n"
			"\tjsr (%1)\n"
			"\taddq.w #4,%%sp\n"
		:
		: "r"(a0), "r"(a1)
		: "cc", "d2", "memory");
	}
#else
#error "you loose"
#endif
	return 0;
}


static long read_dev(void)
{
#if defined(__PUREC__)
	/* BUG: called with Pure-C calling convention */
	currdev->recv(currdev);
#elif defined(__GNUC__)
	{
		/*
		 * take care to call it in a way that works both for cdecl
		 * and Pure-C calling conventions, since there seem
		 * to be drivers around that were compiled by it.
		 */
		register void *a0 __asm__("a0") = currdev;
		register void *a1 __asm__("a1") = currdev->recv;
		__asm__ __volatile__(
			"\tmove.l %0,-(%%sp)\n"
			"\tjsr (%1)\n"
			"\taddq.w #4,%%sp\n"
		:
		: "r"(a0), "r"(a1)
		: "cc", "d2", "memory");
	}
#else
#error "you loose"
#endif
	return 0;
}


/*
 * worker function.
 * almost identical to the inner loop of _sld() in serial.c
 */
static void do_work(void)
{
	struct slbuf *devptr;
	int loop;
	struct slbuf *sl;
	long nr;
	short offset;
	long r;
	long space;
	
	for (devptr = dev_table, loop = 0; loop < NSLBUFS; loop++, devptr++)
	{
		sl = devptr;
		if ((sl->flags & (SL_INUSE|SL_CLOSING)) != SL_INUSE)
			continue;
		nr = 0;
		Fcntl(sl->fd, (long)&nr, FIONREAD);
		if (nr != 0 && nr <= sl->nread)
		{
			offset = sl->ihead;
			r = 1;
			/*
			 * read from head to end of buffer
			 */
			if (offset >= sl->itail)
			{
				space = sl->isize - offset;
				if (sl->itail == 0)
					space -= r;
				if (space > 0)
				{
					r = Fread(sl->fd, space, sl->ibuf + offset);
					if (r > 0)
					{
						offset = (offset + r) & (sl->isize - 1);
						nr -= r;
					}
				}
			}
			
			/*
			 * read from start of buffer to 1 byte before tail
			 * BUG: we may have already read all available bytes above
			 */
			if (r > 0 && (offset + 1) < sl->itail)
			{
				space = sl->itail - offset - 1;
				if (space > 0)
				{
					r = Fread(sl->fd, space, sl->ibuf + offset);
					if (r > 0)
					{
						offset = (offset + r) & (sl->isize - 1);
						nr -= r;
					}
				}
			}
			if (r < 0)
				(void) Cconws("sld: Fread failed\r\n");
			sl->ihead = offset;
		}

		sl->nread = nr < 0 ? 0 : nr;
		if (sl->ihead != sl->itail)
		{
			currdev = sl;
			Supexec(read_dev);
		}

		if (sl->flags & SL_SENDING)
		{
			nr = 0;
			Fcntl(sl->fd, (long)&nr, FIONWRITE);
			if ((nr != 0 && nr <= sl->nwrite) ||
				nr >= 100 ||
				((long)(void *)nr >= (long)(void *)((sl->ohead - sl->otail) & (sl->osize - 1)))) /* WTF? why cast? */
			{
				offset = sl->otail;
				r = 1;
				/*
				 * write from tail to end of buffer
				 */
				if (offset > sl->ohead)
				{
					space = sl->osize - offset;
					if (space > 0)
					{
						r = Fwrite(sl->fd, space, sl->obuf + offset);
						if (r > 0)
						{
							offset = (offset + r) & (sl->osize - 1);
							nr -= r;
						}
					}
				}

				/*
				 * write from start of buffer to head
				 * BUG: we may have already written all available bytes above
				 */
				if (r > 0 && offset < sl->ohead)
				{
					space = sl->ohead - offset;
					if (space > 0)
					{
						r = Fwrite(sl->fd, space, sl->obuf + offset);
						if (r > 0)
						{
							offset = (offset + r) & (sl->osize - 1);
							nr -= r;
						}
					}
				}

				if (r < 0)
					(void) Cconws("sld: Fwrite failed\r\n");
				sl->otail = offset;
				
				if ((((sl->osize - 1) - ((sl->ohead - offset) & (sl->osize - 1))) << 2) >= sl->osize)
				{
					short d2; /* XXX */
					d2 = sl->ohead;
					if (offset == d2)
						sl->flags &= ~SL_SENDING;
					currdev = sl;
					Supexec(write_dev);
					if (sl->ohead != sl->otail)
						sl->flags |= SL_SENDING;
				}
			}
			sl->nwrite = nr < 0 ? 0 : nr;
		}
	}
}

/* XXX temporary to get identical results */
struct slbuf *currdev;
