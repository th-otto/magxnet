/*
 * wrapper functions to call the callbacks
 * exported via the MX_KERNEL structure.
 * Even for Pure-C, we need these to preserve a2 across
 * the call (implementation in mxkernel.s)
 * For GNU-C, some inline asm magic is used.
 * For other compilers, this has to be done.
 */

extern MX_KERNEL *p_kernel;

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
