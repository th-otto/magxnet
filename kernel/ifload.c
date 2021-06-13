/*
 *	Load network interfaces (*.xif) from /mint/ and /multitos/.
 *
 *	06/22/94, Kay Roemer.
 */

#include "sockets.h"
#include "ifload.h"

/* Keep netinfo.h from defining it */
#define NETINFO
#include "netinfo.h"

#include "bpf.h"
#include "inet.h"
#include "inetutil.h"
#include "ifeth.h"


struct netinfo netinfo = {
	buf_alloc,
	buf_free,
	buf_reserve,
	buf_deref,

	if_enqueue,
	if_dequeue,
	if_register,
	if_input,
	if_flushq,

	(short cdecl (*)(void *, short))chksum, /* BUG: function is not cdecl */
	if_getfreeunit,

	eth_build_hdr,
	eth_remove_hdr,

	NULL,

	bpf_input,

	if_deregister,

	{ 0, 0, 0, 0 }
};


#if 0
static long xif_module_init(void *initfunc, struct kerinfo *k, struct netinfo *n)
{
	register long ret __asm__("d0");

	__asm__ volatile (
		"\tmovl	%3,sp@-\n"
		"\tmovl	%2,sp@-\n"
		"\tmovl	%1,a0\n"
		"\tjsr	a0@\n"
		"\taddql	#8,sp\n"
		: "=r"(ret)	/* outputs */
		: "r"(initfunc), "r"(k), "r"(n)	/* inputs  */
		: __CLOBBER_RETURN("d0") "d1", "d2", "d3", "d4", "d5", "d6", "d7", "a0", "a1", "a2",	/* clobbered regs */
		   "memory");

	return ret;
}


static long load_xif(struct basepage *b, const char *name, short *class, short *subclass)
{
	long r;

	DEBUG(("load_xif: enter (0x%lx, %s)", (unsigned long) b, name));
	DEBUG(("load_xif: init 0x%lx, size %li", (unsigned long) b->p_tbase, (b->p_tlen + b->p_dlen + b->p_blen)));

	/* pass a pointer to the drivers file name on to the
	 * driver.
	 */
	netinfo.fname = name;
	*class = MODCLASS_XIF;
	*subclass = 0;
	r = xif_module_init((void *) b->p_tbase, KERNEL, &netinfo);
	netinfo.fname = NULL;

	return r;
}
#endif


void if_load(void)
{
	_DTA *old_dta;
	
	old_dta = Fgetdta();
	
	(void) Cconws("Loading interfaces:\r\n");
	
	Fsetdta(old_dta);
}
