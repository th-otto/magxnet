/*
 *	Provide a MiNT kernel interface to device drivers.
 *
 *	(C) 2021 Thorsten Otto.
 */

#include "sockets.h"
#include <stdarg.h>
#include "timeout.h"
#include "kerinfo.h"
#include "bpf.h"
#include "mxkernel.h"
#include <mint/ssystem.h>

#define DOS_MAX 0x160

static Func dos_tab[DOS_MAX];
static Func bios_tab[12];

static long cdecl enosys(void)
{
	return -ENOSYS;
}


static int cdecl ksprintf_old(char *buf, const char *fmt, ...)
{
	va_list args;
	int r;
	
	va_start(args, fmt);
	r = vsprintf(buf, fmt, args);
	va_end(args);
	return r;
}


/*
 * kernel info that is passed to loaded file systems and device drivers
 */
static struct kerinfo kernelinfo =
{
	1 /* MINT_MAJ_VERSION */,
	15 /* MINT_MIN_VERSION */,
	0755 /* DEFAULT_MODE */,
	2, /* MINT_KVERSION */
	bios_tab,
	dos_tab,
	0, /* m_changedrv */
	(void cdecl (*)(const char *, ...))enosys, /* trace */
	(void cdecl (*)(const char *, ...))enosys, /* debug */
	(void cdecl (*)(const char *, ...))enosys, /* alert */
	(void cdecl (*)(const char *, ...))enosys, /* fatal */
	0, /* kmalloc */
	0, /* kfree */
	(void *cdecl (*)(ulong))enosys, /* umalloc */
	(void cdecl (*)(void *))enosys, /* ufree */
	(int cdecl (*)(const char *, const char *, int))enosys, /* kstrnicmp */
	(int cdecl (*)(const char *, const char *))enosys, /* kstricmp */
	(char *cdecl (*)(char *))enosys, /* kstrlwr */
	(char *cdecl (*)(char *))enosys, /* kstrupr */
	ksprintf_old, /* ksprintf */
	(void cdecl (*)(ulong, short *))enosys, /* millis_time */
	unixtime,
	(long cdecl (*)(long))enosys, /* dostime */
	(void cdecl (*)(unsigned))enosys, /* nap */
	(int cdecl (*)(int, long))enosys, /* sleep */
	(void cdecl (*)(int, long))enosys, /* wake */
	(void cdecl (*)(long))enosys, /* wakeselect */
	(int cdecl (*)(void *, void *))enosys, /* denyshare */
	(void *cdecl (*)(ushort, void *, void *))enosys, /* denylock */
	(TIMEOUT *cdecl (*)(long, to_func *))enosys, /* addtimeout */
	(void cdecl (*)(TIMEOUT *))enosys, /* canceltimeout */
	addroottimeout,
	cancelroottimeout,
	(long cdecl (*)(int, ushort))enosys, /* ikill */
	(void cdecl (*)(int, long, short))enosys, /* iwake */
	NULL, /* bio */
	/* version 1 extension */
	NULL, /* xtime */
	0, /* res */

	/* version 2
	 */

	0, /* add_rsvfentry */
	0, /* del_rsvfentry */
	0, /* killgroup */
	NULL, /* dma */
	NULL, /* loops_per_sec */
	0, /* get_toscookie */

	0, /* so_register */
	0, /* so_unregister */
	0, /* so_release */
	0, /* so_sockpair */
	0, /* so_connect */
	0, /* so_accept */
	0, /* so_create */
	0, /* so_dup */
	0, /* so_free */

	0, /* load_modules */
	0, /* kthread_create */
	0, /* kthread_exit */

	NULL, /* dmabuf_alloc, */
	NULL, /* nf_ops */

	0, /* remaining_proc_time */

	{
		0
	}
};


static short cdecl sys_c_conws(const char *str)
{
	return (short) Cconws(str);
}


static unsigned short cdecl sys_t_getdate(void)
{
	return Tgetdate();
}


static unsigned short cdecl sys_t_gettime(void)
{
	return Tgettime();
}


#if defined(__GNUC__) && !defined(__MSHORT__)
struct setexc_args {
	short number;
	void *exchdlr;
};
static void (*cdecl sys_b_setexc(struct setexc_args args)) (void)
{
	return Setexc(args.number, args.exchdlr);
}
#else
static void (*cdecl sys_b_setexc(short number, void (*exchdlr)(void) )) (void)
{
	return Setexc(number, exchdlr);
}
#endif


static void *cdecl m_kmalloc(unsigned long size)
{
	return kmalloc(size);
}


#if defined(__GNUC__) && !defined(__MSHORT__)
struct mxalloc_args {
	unsigned long size;
	short mode;
};
static void *cdecl sys_m_xalloc(struct mxalloc_args args)
{
	return p_kernel->mxalloc(args.size, args.mode, _BasPag);
}
#else
static void *cdecl sys_m_xalloc(unsigned long size, short mode)
{
	return p_kernel->mxalloc(size, mode, _BasPag);
}
#endif


static void cdecl m_kfree(void *ptr)
{
	kfree(ptr);
}


static long cdecl sys_s_system(short mode, ulong arg1, ulong arg2)
{
	long r = E_OK;
	long values[2];
	
	switch (mode)
	{
	case -1:
		TRACE(("exit s_system(): call exists"));
		break;
	case S_OSNAME:
		r = 0x4d616758L; 	/* MagX */
		break;
	case S_GETCOOKIE:
		if (get_cookie(arg1, (long *)arg2) == NULL)
			r = EERROR;
		break;
	case S_SETCOOKIE:
		values[0] = arg1;
		values[1] = arg2;
		r = install_cookie(values) == FALSE ? 1 : 0;
		/* BUG: falls through */
	default:
		r = EACCES;
		break;
	}
	return r;
}


static long cdecl sys_f_write(short fd, long count, const void *buf)
{
	return Fwrite(fd, count, buf);
}


/* FIXME: should return pointer */
long init_kerinfo(void)
{
	int i;
	
	for (i = 0; i < 12; i++)
		bios_tab[i] = enosys;
	
	dos_tab[0x009] = (Func) sys_c_conws;
	dos_tab[0x02a] = (Func) sys_t_getdate;
	dos_tab[0x02c] = (Func) sys_t_gettime;
	dos_tab[0x040] = (Func) sys_f_write;
	dos_tab[0x044] = (Func) sys_m_xalloc;
	dos_tab[0x154] = (Func) sys_s_system;

	bios_tab[5] = (Func) sys_b_setexc;

	kernelinfo.drvchng = (void *)p_kernel; /* WTF */
	kernelinfo.kmalloc = m_kmalloc;
	kernelinfo.kfree = m_kfree;
	
	return (long)&kernelinfo;
}
