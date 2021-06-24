/*
 *	This file implements some sort of line discipline stuff using
 *	select and a separate process.
 *
 *	06/12/94, Kay Roemer.
 */

#define __KERNEL_XDD__

#include "sockets.h"
#include "sockdev.h"
#include "bpf.h"
#include "netinfo.h"
#include "buf.h"
#include "serial.h"
#include "mxkernel.h"
#include "slip.h"

#ifndef MX_KEEP
# define MX_KEEP 0x4000
#endif

#define NSLBUFS		4

#define FDZERO(fd)		((fd) = 0)
#define FDSET(fd, set)		((set) |= 1L << (fd))
#define FDCLR(fd, set)		((set) &= ~(1L << (fd)))
#define FDISSET(fd, set)	((set) & (1L << (fd)))


static struct slbuf allslbufs[NSLBUFS];
struct magxnet_cookie *sockets_dev;
struct magxnet_cookie *sockets_dev_cookie;


#define f_open(dev, mode) (*sockets_dev->Fopen)(dev)
#define f_close(fd) (*sockets_dev->Fclose)(fd)
#undef _BasPag
#define _BasPag sockets_dev->base


#if 0
static short glfd;
static char *pname = "u:\\pipe\\sld";
static struct proc *sld_p;

/* moved to magx_sld.ovl  for MagiCNet */
static void _sld(void)
{
	struct slcmd cmd;
	struct slbuf *sl;
	short i;
	short pipefd;
	short p;
	short tmout = 0;
	long r;
	long rset;
	long wset;
	long wready;
	long rready;
	long space;

	pipefd = f_open(pname, O_RDONLY | O_NDELAY);
	if (pipefd < 0)
	{
		char buf[128];

		ksprintf(buf, "sld: PANIC: Cannot open pipe (-> %i)\r\n", pipefd);
		c_conws(buf);

		return;
	}

	FDZERO(rset);
	FDZERO(wset);
	FDSET(pipefd, rset);

	for (;;)
	{
		wready = wset;
		rready = rset;

		/*
		 * Poll if some channels are active
		 */
		tmout = (rset & ~(1L << pipefd)) ? SL_VTIME : 0;
		r = f_select(tmout, &rready, &wready, 0);
		if (r < 0)
		{
			char buf[128];

			ksprintf(buf, "sld: Fselect failed (%li)\r\n", r);
			c_conws(buf);

			f_select(1000, 0L, 0L, 0L);

			continue;
		}

		if (FDISSET(pipefd, rready))
		{
			r = f_read(pipefd, 1, (char *) &cmd);
			if (r == 1 && cmd.slnum < NSLBUFS)
			{
				sl = &allslbufs[cmd.slnum];
				if (sl->flags & SL_INUSE)
					switch (cmd.cmd)
					{
					case SLCMD_OPEN:
						{
#ifdef SLD_DEBUG
							char buf[128];

							ksprintf(buf, "sld: SLCMD_OPEN (sl->fd = %i)\r\n", sl->fd);
							c_conws(buf);
#endif

							r = sl->fd;
							sl->fd = f_dup(r);
							f_close(r);

							if (sl->fd < 0)
							{
								c_conws("sld: PANIC: Fdup failed\r\n");
								return;
							}
#ifdef SLD_DEBUG

							ksprintf(buf, "sld: SLCMD_OPEN -> sl->fd = %i (0x%lx)\r\n", sl->fd,
									 sld_p->p_fd->ofiles[sl->fd]);
							c_conws(buf);
#endif

							FDSET(sl->fd, rset);
							break;
						}
					case SLCMD_CLOSE:
						{
#ifdef SLD_DEBUG
							char buf[128];

							ksprintf(buf, "sld: SLCMD_CLOSE (sl->fd = %i)\r\n", sl->fd);
							c_conws(buf);
#endif

							FDCLR(sl->fd, rset);
							FDCLR(sl->fd, wset);
							FDCLR(sl->fd, wready);
							FDCLR(sl->fd, rready);
							f_close(sl->fd);
							sl->flags &= ~(SL_INUSE | SL_SENDING | SL_CLOSING);
							break;
						}
					case SLCMD_SEND:
						{
							FDSET(sl->fd, wset);
							FDSET(sl->fd, wready);	/* pretend we can write */
							(*sl->send) (sl);
							break;
						}
					}
			}
		}

		for (i = 0; i < NSLBUFS; ++i)
		{
			long nr;

			sl = &allslbufs[i];
			if ((sl->flags & (SL_INUSE | SL_CLOSING)) != SL_INUSE)
				continue;

			/*
			 * We try to read something if
			 * 1) >= SL_VMIN chars arrived, or
			 * 2) no more input arrived in the last SL_VTIME msecs.
			 */
			nr = 0;
			f_cntl(sl->fd, (long) &nr, FIONREAD);
			if (FDISSET(sl->fd, rready) || (nr && nr <= sl->nread))
			{
				p = sl->ihead;
				r = 1;
				if (p >= sl->itail)
				{
					space = sl->isize - p;
					if (sl->itail == 0)
						--space;
					if (space > 0)
					{
						r = f_read(sl->fd, space, &sl->ibuf[p]);
						if (r > 0)
						{
							p = (p + (short) r) & (sl->isize - 1);
							nr -= r;
						}
					}
				}
				if (r > 0 && p + 1 < sl->itail)
				{
					space = sl->itail - p - 1;
					if (space > 0)
					{
						r = f_read(sl->fd, space, &sl->ibuf[p]);
						if (r > 0)
						{
							p = (p + (short) r) & (sl->isize - 1);
							nr -= r;
						}
					}
				}
				if (r < 0)
					c_conws("sld: Fread failed\r\n");
				sl->ihead = p;
			}
			sl->nread = MAX(0, nr);
			if (sl->ihead != sl->itail)
				(*sl->recv) (sl);

			if (!(sl->flags & SL_SENDING))
				continue;

			/*
			 * We send something if
			 * 1) connection was idle some time, or
			 * 2) space did not increase for SL_VTIME, or
			 * 3) can send whole buf, or
			 * 4) can send at least SL_VMIN
			 */
			nr = 0;
			f_cntl(sl->fd, (long) &nr, FIONWRITE);
			if (FDISSET(sl->fd, wready) || (nr && nr <= sl->nwrite) || nr >= SL_VMIN || nr >= SL_OUSED(sl))
			{
				FDSET(sl->fd, wset);
				p = sl->otail;
				r = 1;
				if (p > sl->ohead)
				{
					space = sl->osize - p;
					if (space > 0)
					{
						r = f_write(sl->fd, space, &sl->obuf[p]);
						if (r > 0)
						{
							p = (p + (short) r) & (sl->osize - 1);
							nr -= r;
						}
					}
				}
				if (r > 0 && p < sl->ohead)
				{
					space = sl->ohead - p;
					if (space > 0)
					{
						r = f_write(sl->fd, space, &sl->obuf[p]);
						if (r > 0)
						{
							p = (p + (short) r) & (sl->osize - 1);
							nr -= r;
						}
					}
				}
				if (r < 0)
					c_conws("sld: Fwrite failed\r\n");
				sl->otail = p;
				/*
				 * Produce more output if at least a quarter of
				 * our output buffer is free
				 */
				if (4 * SL_OFREE(sl) >= sl->osize)
				{
					if (sl->ohead == sl->otail)
					{
						sl->flags &= ~SL_SENDING;
						FDCLR(sl->fd, wset);
					}
					(*sl->send) (sl);
					if (sl->ohead != sl->otail)
					{
						sl->flags |= SL_SENDING;
						FDSET(sl->fd, wset);
					}
				}
			}
			sl->nwrite = MAX(0, nr);
			/*
			 * If buffer is tiny then turn off this descriptor
			 * and wait up to SL_VMIN until next retry.
			 * The descriptor is turned on again after the
			 * next Fselect().
			 */
			if (sl->nwrite < SL_OUSED(sl) && sl->nwrite < SL_VMIN)
				FDCLR(sl->fd, wset);
		}
	}
}

static void sld(void *bp)
{
	p_domain(1);
	p_nice(-5);
	p_sigblock(-1L);

	_sld();

	kthread_exit(1);
}


long serial_init(void)
{
	long r;

	if (!kthread_create)
	{
		ALERT(("This slip.xif requires an uptodate 1.16 kernel!"));
		return -1;
	}

	glfd = f_open(pname, O_NDELAY | O_WRONLY | O_CREAT | O_GLOBAL);
	if (glfd < 100)
		glfd += 100;

	f_chmod(pname, 0600);

	r = kthread_create(sld, (void *) 0x1, &sld_p, "sld");
	if (r != 0)
		f_close(glfd);

	return r;
}

#else

static long get_jar(void)
{
	return *((long *)0x5a0);
}


long *get_cookie(long id, long *value)
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


long sockets_dev_init(void)
{
	if (get_cookie(C_SCKM, (long *)&sockets_dev_cookie) == NULL)
		return FALSE;
	if (sockets_dev_cookie)
		sockets_dev = (struct magxnet_cookie *)sockets_dev_cookie;
	sockets_dev->dev_table = allslbufs;

#ifdef __PUREC__
	/* FIXME: some relict of function above? */
	(void)"sld: Fread failed\r\n";
	(void)"sld: Fwrite failed\r\n";
#endif

	return TRUE;
}

#endif


#ifdef NOTYET
static long serial_make_raw(short fd)
{
	short vmin[2];
	struct sgttyb sg;

	if (f_cntl(fd, (long) &sg, TIOCGETP) < 0)
		return -1;

	sg.sg_flags &= ~(T_CRMOD | T_CBREAK | T_ECHO | T_XKEY | T_TOSTOP | T_EVENP | T_ODDP);
	sg.sg_flags |= T_RAW;
	if (f_cntl(fd, (long) &sg, TIOCSETP) < 0)
		return -1;

	/*
	 * VTIME we (must) do ourselves...
	 */
	vmin[0] = SL_VMIN;
	vmin[1] = 0;
	if (f_cntl(fd, (long) vmin, TIOCSVMIN) < 0)
		return -1;

	return 0;
}
#endif


struct slbuf *serial_open(struct netif *nif, char *device, short (*f_send)(struct slbuf *), short(*f_recv)(struct slbuf *))
{
	struct slbuf *sl;
	short i;

	sl = 0;
	for (i = 0; i < NSLBUFS; ++i)
	{
		if (!(allslbufs[i].flags & SL_INUSE))
		{
			sl = &allslbufs[i];
			if (sl->ibuf)
				kfree(sl->ibuf);
			if (sl->obuf)
				kfree(sl->obuf);
			sl->ibuf = sl->obuf = 0;
		}
	}

	if (!sl)
	{
		DEBUG(("serial_open: out of sl bufs"));
		(void) Cconws("serial_open: out of sl bufs\r\n"); /* BUG: forgotten debug output */
		return 0;
	}

	sl->flags = SL_INUSE;
	sl->nif = nif;
	sl->send = f_send;
	sl->recv = f_recv;
	sl->nread = 0;
	sl->nwrite = 0;
	sl->isize = SL_IBUFSIZE;
	sl->osize = SL_OBUFSIZE;
	sl->ihead = sl->itail = 0;
	sl->ohead = sl->otail = 0;
	sl->ibuf = p_kernel->mxalloc(SL_IBUFSIZE, MX_PREFTTRAM|MX_KEEP, _BasPag);
	sl->obuf = p_kernel->mxalloc(SL_OBUFSIZE, MX_PREFTTRAM|MX_KEEP, _BasPag);

	if (!sl->ibuf || !sl->obuf)
	{
		DEBUG(("serial_open: no mem for buffers"));
		(void) Cconws("serial_open: no mem for buffers"); /* BUG: forgotten debug output */
		if (sl->ibuf)
			kfree(sl->ibuf);
		if (sl->obuf)
			kfree(sl->obuf);
		sl->ibuf = sl->obuf = 0;
		sl->flags &= ~SL_INUSE;
		return 0;
	}

	strncpy(sl->dev, device, sizeof(sl->dev));
	sl->dev[sizeof(sl->dev) - 1] = '\0';

	if (sockets_dev->Fopen)
		sl->fd = f_open(sl->dev, O_RDWR | O_NDELAY | O_GLOBAL);
	else
		sl->fd = -1;
	if (sl->fd < 0)
	{
		DEBUG(("serial_open: fopen(%s) returned %d", sl->fd));
		kfree(sl->ibuf);
		kfree(sl->obuf);
		sl->ibuf = sl->obuf = 0;
		sl->flags &= ~SL_INUSE;
		return 0;
	}

#ifdef NOTYET
	if (sl->fd < 100)
		sl->fd += 100;

	if (serial_make_raw(sl->fd))
	{
		DEBUG(("serial_open: cannot make tty raw."));
		f_close(sl->fd);
		kfree(sl->ibuf);
		kfree(sl->obuf);
		sl->ibuf = sl->obuf = 0;
		sl->flags &= ~SL_INUSE;
		return 0;
	}
#endif

#ifdef NOTYET
	{
	struct slcmd cmd;
	long r;
	cmd.slnum = (unsigned int)(sl - allslbufs);
	cmd.cmd = SLCMD_OPEN;
	r = f_write(glfd, 1, (char *) &cmd);
	if (r != 1)
	{
		DEBUG(("serial_open: fwrite() returned %ld", r));
		f_close(sl->fd);
		kfree(sl->ibuf);
		kfree(sl->obuf);
		sl->ibuf = sl->obuf = 0;
		sl->flags &= ~SL_INUSE;
		return 0;
	}
	}
#endif

	return sl;
}

long serial_close(struct slbuf *sl)
{
#ifdef NOTYET
	struct slcmd cmd;
	long r;

	cmd.slnum = (unsigned int)(sl - allslbufs);
	cmd.cmd = SLCMD_CLOSE;
	r = f_write(glfd, 1, (char *) &cmd);
	if (r != 1)
	{
		DEBUG(("serial_close: fwrite() returned %ld", r));
		return r ? r : -1;
	}
	sl->flags |= SL_CLOSING;
#else
	if (sockets_dev->Fclose)
		sl->fd = (*sockets_dev->Fclose)(sl->fd);
	if (sl->fd == 0)
		sl->flags &= ~(SL_CLOSING|SL_SENDING|SL_INUSE);
	else
		sl->flags |= SL_CLOSING;
		
#endif

	return 0;
}

long serial_send(struct slbuf *sl)
{
	if (sl->flags & SL_SENDING)
		return 0;

#ifdef NOTYET
	{
	struct slcmd cmd;
	long r;

	cmd.slnum = (unsigned int)(sl - allslbufs);
	cmd.cmd = SLCMD_SEND;
	r = f_write(glfd, 1, (char *) &cmd);
	if (r != 1)
	{
		DEBUG(("serial_send: fwrite() returned %ld", r));
		return r ? r : -1;
	}
	}
#else
	sl->send(sl);
#endif

	sl->flags |= SL_SENDING;
	return 0;
}
