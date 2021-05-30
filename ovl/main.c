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
#include "sockdev.h"

#define C_SCKM 0x53434B4DL     /* MagXNet (SOCKET.DEV) */
#define FIONREAD	(('F'<< 8) | 1)
#define FIONWRITE	(('F'<< 8) | 2)


static struct magxnet_cookie *sockets_dev;
static long cookie;
static short apid;
static struct sockdev *dev_table;
extern struct sockdev *currdev;

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
	currdev->write_dev(currdev);
#elif defined(__GNUC__)
	{
		register void *a0 __asm__("a0") = currdev;
		register void *a1 __asm__("a1") = currdev->write_dev;
		__asm__ __volatile__(
			"jsr (%1)\n"
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
	currdev->read_dev(currdev);
#elif defined(__GNUC__)
	{
		register void *a0 __asm__("a0") = currdev;
		register void *a1 __asm__("a1") = currdev->read_dev;
		__asm__ __volatile__(
			"jsr (%1)\n"
		:
		: "r"(a0), "r"(a1)
		: "cc", "d2", "memory");
	}
#else
#error "you loose"
#endif
	return 0;
}


static void do_work(void)
{
	struct sockdev *devptr;
	int loop;
	struct sockdev *dev;
	long navail;
	short offset;
	long nbytes;
	long bufsize;
	
	for (devptr = dev_table, loop = 0; loop < 4; loop++, devptr++)
	{
		dev = devptr;
		if ((dev->flags & 0x05) != 0x01)
			continue;
		navail = 0;
		Fcntl(dev->fd, (long)&navail, FIONREAD);
		if (navail != 0 && navail <= dev->input_avail)
		{
			offset = dev->input_tail;
			nbytes = 1;
			/*
			 * read from tail to end of buffer
			 */
			if (offset >= dev->input_head)
			{
				bufsize = dev->inbuf_size - offset;
				if (dev->input_head == 0)
					bufsize -= nbytes;
				if (bufsize > 0)
				{
					nbytes = Fread(dev->fd, bufsize, dev->inbuf_ptr + offset);
					if (nbytes > 0)
					{
						offset = (offset + nbytes) & (dev->inbuf_size - 1);
						navail -= nbytes;
					}
				}
			}
			
			/*
			 * read from start of buffer to 1 byte before head
			 * BUG: we may have already read all available bytes above
			 */
			if (nbytes > 0 && (offset + 1) < dev->input_head)
			{
				bufsize = dev->input_head - offset - 1;
				if (bufsize > 0)
				{
					nbytes = Fread(dev->fd, bufsize, dev->inbuf_ptr + offset);
					if (nbytes > 0)
					{
						offset = (offset + nbytes) & (dev->inbuf_size - 1);
						navail -= nbytes;
					}
				}
			}
			if (nbytes < 0)
				(void) Cconws("sld: Fread failed\r\n");
			dev->input_tail = offset;
		}

		dev->input_avail = navail < 0 ? 0 : navail;
		if (dev->input_tail != dev->input_head)
		{
			currdev = dev;
			Supexec(read_dev);
		}

		if (dev->flags & 0x02)
		{
			navail = 0;
			Fcntl(dev->fd, (long)&navail, FIONWRITE);
			if ((navail != 0 && navail <= dev->output_avail) ||
				navail >= 100 ||
				((long)(void *)navail >= (long)(void *)((dev->output_tail - dev->output_head) & (dev->outbuf_size - 1)))) /* WTF? why cast? */
			{
				offset = dev->output_head;
				nbytes = 1;
				/*
				 * write from head to end of buffer
				 */
				if (offset > dev->output_tail)
				{
					bufsize = dev->outbuf_size - offset;
					if (bufsize > 0)
					{
						nbytes = Fwrite(dev->fd, bufsize, dev->outbuf_ptr + offset);
						if (nbytes > 0)
						{
							offset = (offset + nbytes) & (dev->outbuf_size - 1);
							navail -= nbytes;
						}
					}
				}

				/*
				 * write from start of buffer to tail
				 * BUG: we may have already written all available bytes above
				 */
				if (nbytes > 0 && offset < dev->output_tail)
				{
					bufsize = dev->output_tail - offset;
					if (bufsize > 0)
					{
						nbytes = Fwrite(dev->fd, bufsize, dev->outbuf_ptr + offset);
						if (nbytes > 0)
						{
							offset = (offset + nbytes) & (dev->outbuf_size - 1);
							navail -= nbytes;
						}
					}
				}

				if (nbytes < 0)
					(void) Cconws("sld: Fwrite failed\r\n");
				dev->output_head = offset;
				
				if ((((dev->outbuf_size - 1) - ((dev->output_tail - offset) & (dev->outbuf_size - 1))) << 2) >= dev->outbuf_size)
				{
					short d2; /* XXX */
					d2 = dev->output_tail;
					if (offset == d2)
						dev->flags &= ~0x02;
					currdev = dev;
					Supexec(write_dev);
					if (dev->output_tail != dev->output_head)
						dev->flags |= 0x02;
				}
			}
			dev->output_avail = navail < 0 ? 0 : navail;
		}
	}
}

/* XXX temporary to get identical results */
struct sockdev *currdev;
