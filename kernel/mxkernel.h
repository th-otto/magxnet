/*
 * wrapper functions to call the callbacks
 * exported via the MX_KERNEL structure.
 * Even for Pure-C, we need these to preserve a2 across
 * the call (implementation in mxkernel.s)
 * For GNU-C, some inline asm magic is used.
 * For other compilers, this has to be done.
 */

#ifndef _mx_kernel_h
#define _mx_kernel_h 1

/*
 * points to our wrapper interfcae
 */
extern MX_KERNEL *p_kernel;
/*
 * points to the structure obtained from Dcntl(KER_GETINFO)
 */
extern MX_KERNEL *real_p_kernel GNU_ASM_NAME("real_p_kernel");

MX_KERNEL *ker_getinfo(void) GNU_ASM_NAME("ker_getinfo");

#if 0
void ker_fast_clrmem(void *from, void *to);
char ker_toupper(char c);
void ker_sprintf(char *dest, const char *format, long *params);
void ker_appl_yield(void);
void ker_appl_suspend(void);
void ker_appl_begcritic(void);
void ker_appl_endcritic(void);
long ker_evnt_IO(LONG ticks_50hz, MAGX_UNSEL *unsel);
void ker_evnt_mIO(LONG ticks_50hz, MAGX_UNSEL *unsel, WORD cnt);
void ker_evnt_emIO(APPL *ap);
void ker_appl_IOcomplete(APPL *ap);
long ker_evnt_sem(WORD mode, void *sem, LONG timeout);
void ker_Pfree(PD *pd);
void *ker_int_malloc(void *memblock);
void ker_int_mfree(void *memblk);
void ker_resv_intmem(void *mem, LONG bytes);
LONG ker_diskchange(WORD drv);
LONG ker_DMD_rdevinit(DMD *dmd);
LONG ker_proc_info(WORD code, PD *pd);
void *ker_mxalloc(LONG amount, WORD mode, PD *pd);
LONG ker_mfree(void *block);
void *ker_mshrink(void *block, LONG newlen);
#endif

#define kmalloc(size) p_kernel->mxalloc(size, MX_PREFTTRAM, _BasPag)
#define kfree(ptr) p_kernel->mfree(ptr)
#define mint_bzero(ptr, size) p_kernel->fast_clrmem(ptr, (char *)(ptr) + size)
#ifdef __PUREC__
/*
 * FIXME: only for binary comparison.
 * FIXME2: should really call OS instead
 * (Pgeteuid available since magic 1999/12/30)
 */
static short p_geteuid(void) 0x7000; /* moveq #0,d0 */
#else
#define p_geteuid() 0L
#endif
#define p_kill(pid, sig) (void) Pkill(pid, sig)
#define p_getpid() p_kernel->proc_info(2, *(real_p_kernel->act_pd))

extern long sprintf_params[];
extern short bios_sema GNU_ASM_NAME("bios_sema");
extern short in_tcp_send GNU_ASM_NAME("in_tcp_send");

#endif /* _mx_kernel_h */

