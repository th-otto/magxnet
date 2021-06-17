#include "sockets.h"
#include "mxkernel.h"
#include "sockdev.h"
#include "bpf.h"
#include "timeout.h"
#include "version.h"

#ifndef SuperToUser
#define SuperToUser(sp) Super((void *)(sp))
#endif

MX_KERNEL *p_kernel_copy;
long x1f1dc;
long x1f1e0;
long x1f1e4;
MX_KERNEL *p_kernel;
long cookie_values[2];
struct magxnet_cookie cookie;
struct timeout_pool timeout_pool[128];
const char *socket_devname = "u:\\dev\\socket";
typedef void (*init_func)(void);
static init_func init_funcs[] = { inet4_init, bpf_init, 0 };

short in_tcp_send;
void *x1ef60 GNU_ASM_NAME("x1ef60");
void *x1ef64 GNU_ASM_NAME("x1ef64");



extern void new_etv_timer(void) GNU_ASM_NAME("new_etv_timer");
extern void *old_etv_timer GNU_ASM_NAME("old_etv_timer");


/* forward declarations */
static void print_banner(void);


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


int install_cookie(long *values)
{
	unsigned long *jar;
	int size;
	unsigned long value;

	jar = (unsigned long *)Supexec(get_jar);
	size = 0;
	if (jar != NULL)
	{
		while ((value = jar[0]) != 0)
		{
			jar += 2;
			size++;
		}
		if ((unsigned long)size < jar[1])
		{
			jar[2] = value;
			jar[3] = jar[1];
			*jar++ = *values++;
			*jar++ = *values++;
			return TRUE;
		}
	}
	return FALSE;
}


#if defined(__PUREC__)
static void nop(void) 0x4e71;
#elif defined(__GNUC__)
static void nop(void)
{
	__asm__ __volatile__("\tnop\n");
}
#else
#define nop()
#endif


static void setvec(void **old, long vec, void *new)
{
	void **vecptr = (void **)vec;
	*old = *vecptr;
	*vecptr = new;
	nop(); /* WTF? */
	nop();
}


static long set_etv_timer(void)
{
	setvec(&old_etv_timer, 0x400, new_etv_timer);
	return 0;
}



int main(void)
{
	long sp;
	long r;
	int i;
	
	sp = 0;
	print_banner();

	/* MagiC running ? */
	if (get_cookie(C_MagX, NULL) == NULL)
		return -1;

	p_kernel = ker_getinfo();
	if (p_kernel == 0 || p_kernel == (void *)-32)
	{
		(void) Cconws("Cannot install socket device\r\n");
		Pterm(1); /* FIXME: just return */
	}
	if (p_kernel->version < 4 || p_kernel->int_msize < 34)
	{
		(void) Cconws(" MAGX-NeT only works with MagiC kernel 4 or newer!\r\n");
		(void) Cconws("Press any key...");
		Cnecin();
		return -1;
	}
	r = Dcntl(DEV_M_INSTALL, socket_devname, (long)&socket_dev);
	if (r < 0)
		return (int)r;
	p_kernel_copy = p_kernel;
	timeout_init(timeout_pool, sizeof(timeout_pool), sizeof(timeout_pool[0]));
	cookie.version = MSG_VERSION;
	cookie.author = "Vassilis Papathanassiou";
	cookie.magic = 0x4D475853L; /* 'MGXS' */
	cookie.init_timer = set_etv_timer;
	cookie.checkalarms = checkalarms;
	cookie.base = _BasPag;
	cookie_values[0] = C_SCKM;
	cookie_values[1] = (long)&cookie;
	if (install_cookie(cookie_values) == FALSE)
	{
		(void) Cconws("What ? No place for cookie !\r\n");
		Fdelete(socket_devname);
		return -1;
	}

	if (Super((void *)1) == 0)
		sp = Super(NULL);
	for (i = 0; init_funcs[i] != 0; i++)
		init_funcs[i]();
	install_bios_handler(cookie.checkalarms, cookie.o40);
	if (sp != 0)
		SuperToUser(sp);
	
	Ptermres(-1, 0); /* keep all memory */
	
	return 0;
}


void printstr(const char *str)
{
	while (*str)
		Bconout(2, (unsigned char)*str++);
}


static void print_banner(void)
{
	const char *str; /* FIXME: unneeded */
	str = "\r\n\033pMAGX-NeT dated Jan 22 2003\033q\r\n";
	printstr(str);
	str = "MagX-Net " __STRINGIFY(VER_MAJOR) "." __STRINGIFY(VER_MINOR) " PL " __STRINGIFY(VER_PL)
#if defined(ALPHA)
		", alpha"
#elif defined(BETA)
		", beta"
#endif
		"\r\n";
	printstr(str);
	str = "\275 1998-2003  Vassilis Papathanassiou \r\n\r\n";
	printstr(str);
}
