	.include "mgx_xfs.inc"

	.IFNE GNUC
	.macro save_regs
	movem.l d2-d7/a2-a6,-(a7)
	.endm
	.macro rest_regs
	movem.l (a7)+,d2-d7/a2-a6
	.endm
	.ELSE
	.macro save_regs 
	movem.l d3-d7/a2-a6,-(a7)
	.endm
	.macro rest_regs
	movem.l (a7)+,d3-d7/a2-a6
	.endm
	.ENDC

hz_200 = 0x4ba

	.xref x1e714
	.xref x1ef60
	.xref x1ef64

	.text

/* MX_KERNEL *ker_getinfo(void); */
	.globl ker_getinfo
ker_getinfo:
	movem.l    a2-a3,-(a7)
	moveq.l    #0,d0
	clr.l      -(a7)
	clr.l      -(a7)
	move.w     #KER_GETINFO,-(a7)
	move.w     #0x0130,-(a7) /* Dcntl */
	trap       #1
	lea.l      12(a7),a7
	tst.l      d0
	bmi.s      ker_getinfo1
	move.l     d0,real_p_kernel
	movea.l    d0,a0
	lea.l      my_kernel,a1
	move.w     mxk_version(a0),mxk_version(a1)
	move.l     mxk_act_pd(a0),mxk_act_pd(a1)
	move.l     mxk_act_appl(a0),mxk_act_appl(a1)
	move.l     mxk_keyb_app(a0),mxk_keyb_app(a1)
	move.l     mxk_pe_slice(a0),mxk_pe_slice(a1)
	move.l     mxk_pe_timer(a0),mxk_pe_timer(a1)
	move.w     mxk_int_msize(a0),mxk_int_msize(a1)
	move.l     a1,d0
ker_getinfo1:
	movea.l    d0,a0
	movem.l    (a7)+,a2-a3
	rts

/* void ker_fast_clrmem(void *from, void *to); */
	.globl ker_fast_clrmem
ker_fast_clrmem:
	.IFNE GNUC
	move.l     4(a7),a0
	move.l     8(a7),a1
	.ENDC
	move.l     a2,-(a7)
	movea.l    real_p_kernel,a2
	movea.l    mxk_fast_clrmem(a2),a2
	jsr        (a2)
	movea.l    (a7)+,a2
	rts

/* char ker_toupper(char c); */
	.globl ker_toupper
ker_toupper:
	.IFNE GNUC
	.IFNE MSHORT
	move.w     4(a7),d0
	.ELSE
	move.l     4(a7),d0
	.ENDC
	.ENDC
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_toupper(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* void ker_sprintf(char *dest, const char *format, long *params); */
	.globl ker_sprintf
ker_sprintf:
	.IFNE GNUC
	move.l     4(a7),a0
	move.l     8(a7),a1
	move.l     12(a7),d0
	.ELSE
	move.l     4(a7),d0
	.ENDC
	save_regs
	/*
	 * In the kernel, that functions expects arguments on the stack.
	 * We call it in a way that works in both cases.
	 */
	move.l     d0,-(a7)
	move.l     a1,-(a7)
	move.l     a0,-(a7)
	movea.l    real_p_kernel,a6
	movea.l    mxk__sprintf(a6),a6
	jsr        (a6)
	lea.l      12(a7),a7
	rest_regs
	rts

/* void ker_appl_yield(void); */
	.globl ker_appl_yield
ker_appl_yield:
	move.l     a2,-(a7)
	movea.l    real_p_kernel,a2
	movea.l    mxk_appl_yield(a2),a2
	jsr        (a2)
	movea.l    (a7)+,a2
	rts

/* void ker_appl_suspend(void); */
	.globl ker_appl_suspend
ker_appl_suspend:
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_appl_suspend(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* void ker_appl_begcritic(void); */
	.globl ker_appl_begcritic
ker_appl_begcritic:
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_appl_begcritic(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* void ker_appl_endcritic(void); */
	.globl ker_appl_endcritic
ker_appl_endcritic:
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_appl_endcritic(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* long ker_evnt_IO(LONG ticks_50hz, MAGX_UNSEL *unsel); */
	.globl ker_evnt_IO
ker_evnt_IO:
	.IFNE GNUC
	move.l     4(a7),d0
	move.l     8(a7),a0
	.ENDC
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_evnt_IO(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* void ker_evnt_mIO(LONG ticks_50hz, MAGX_UNSEL *unsel, WORD cnt); */
	.globl ker_evnt_mIO
ker_evnt_mIO:
	.IFNE GNUC
	move.l     4(a7),d0
	move.l     8(a7),a0
	.IFNE MSHORT
	move.w     12(a7),d1
	.ELSE
	move.l     12(a7),d1
	.ENDC
	.ENDC
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_evnt_mIO(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* void ker_evnt_emIO(APPL *ap); */
	.globl ker_evnt_emIO
ker_evnt_emIO:
	.IFNE GNUC
	move.l     4(a7),a0
	.ENDC
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_evnt_emIO(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* void ker_appl_IOcomplete(APPL *ap); */
	.globl ker_appl_IOcomplete
ker_appl_IOcomplete:
	.IFNE GNUC
	move.l     4(a7),a0
	.ENDC
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_appl_IOcomplete(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* long ker_evnt_sem(WORD mode, void *sem, LONG timeout); */
	.globl ker_evnt_sem
ker_evnt_sem:
	.IFNE GNUC
	.IFNE MSHORT
	move.w 4(a7),d0
	move.l 6(a7),a0
	move.l 10(a7),d1
	.ELSE
	move.l 4(a7),d0
	move.l 8(a7),a0
	move.l 12(a7),d1
	.ENDC
	.ENDC
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_evnt_sem(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* void ker_Pfree(PD *pd); */
	.globl ker_Pfree
ker_Pfree:
	.IFNE GNUC
	move.l 4(a7),a0
	.ENDC
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_Pfree(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* void *ker_int_malloc(void *memblock); */
	.globl ker_int_malloc
ker_int_malloc:
	.IFNE GNUC
	move.l 4(a7),a0
	.ENDC
	move.l     a2,-(a7)
	movea.l    real_p_kernel,a2
	movea.l    mxk_int_malloc(a2),a2
	jsr        (a2)
	/* note: return value from kernel in D0, not A0 */
	movea.l    d0,a0
	movea.l    (a7)+,a2
	rts

/* void ker_int_mfree(void *memblk); */
	.globl ker_int_mfree
ker_int_mfree:
	.IFNE GNUC
	move.l 4(a7),a0
	.ENDC
	move.l     a2,-(a7)
	movea.l    real_p_kernel,a2
	movea.l    mxk_int_mfree(a2),a2
	jsr        (a2)
	movea.l    (a7)+,a2
	rts

/* void ker_resv_intmem(void *mem, LONG bytes); */
	.globl ker_resv_intmem
ker_resv_intmem:
	.IFNE GNUC
	move.l 4(a7),a0
	move.l 8(a7),d0
	.ENDC
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_resv_intmem(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* LONG ker_diskchange(WORD drv); */
	.globl ker_diskchange
ker_diskchange:
	.IFNE GNUC
	.IFNE MSHORT
	move.w     8(a7),d0
	.ELSE
	move.l     8(a7),d0
	.ENDC
	.ENDC
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_diskchange(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* LONG ker_DMD_rdevinit(DMD *dmd); */
	.globl ker_DMD_rdevinit
ker_DMD_rdevinit:
	.IFNE GNUC
	move.l     4(a7),a0
	.ENDC
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_DMD_rdevinit(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* LONG ker_proc_info(WORD code, PD *pd); */
	.globl ker_proc_info
ker_proc_info:
	.IFNE GNUC
	.IFNE MSHORT
	move.w     4(a7),d0
	move.l     6(a7),a0
	.ELSE
	move.l     4(a7),d0
	move.l     8(a7),a0
	.ENDC
	.ENDC
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_ker_proc_info(a6),a6
	jsr        (a6)
	rest_regs
	rts

/* void *ker_mxalloc(LONG amount, WORD mode, PD *pd); */
	.globl ker_mxalloc
ker_mxalloc:
	.IFNE GNUC
	.IFNE MSHORT
	move.l     4(a7),d0
	move.w     8(a7),d1
	move.l     10(a7),a0
	.ELSE
	move.l     4(a7),d0
	move.l     8(a7),d1
	move.l     12(a7),a0
	.ENDC
	.ENDC
	move.l     a2,-(a7)
	movea.l    real_p_kernel,a2
	movea.l    mxk_ker_mxalloc(a2),a2
	jsr        (a2)
	movea.l    (a7)+,a2
	/* note: return value from kernel in D0, not A0 */
	movea.l    d0,a0
	rts

/* LONG ker_mfree(void *block); */
	.globl ker_mfree
ker_mfree:
	.IFNE GNUC
	move.l     4(a7),a0
	.ENDC
	move.l     a2,-(a7)
	movea.l    real_p_kernel,a2
	movea.l    mxk_ker_mfree(a2),a2
	jsr        (a2)
	movea.l    (a7)+,a2
	rts

/* void *ker_mshrink(void *block, LONG newlen); */
	.globl ker_mshrink
ker_mshrink:
	.IFNE GNUC
	move.l     4(a7),a0
	move.l     8(a7),d0
	.ENDC
	save_regs
	movea.l    real_p_kernel,a6
	movea.l    mxk_ker_mshrink(a6),a6
	jsr        (a6)
	/* note: return value from kernel in D0, not A0 */
	movea.l    d0,a0
	rest_regs
	rts

get_longframe:
	move.w     (0x59e).w,d0
	rts

	.globl install_bios_handler
install_bios_handler:
	.IFNE GNUC
	move.l     4(a7),a0
	move.l     8(a7),a1
	.ENDC
	movem.l    d1-d2/a0-a2,-(a7)
	move.l     a0,x11968
	move.l     a1,x1196c
	move.l     #-1,-(a7)
	move.w     #0x45,-(a7) /* get old bios trap */
	move.w     #5,-(a7)
	trap       #13
	addq.l     #8,a7
	move.l     d0,new_bios_shortframe-4
	move.l     d0,new_bios_longframe-4
	pea.l      get_longframe
	move.w     #0x26,-(a7) /* Supexec */
	trap       #14
	addq.l     #6,a7
	tst.w      d0
	beq.s      xbios_short
	move.w     #3,bios_intervall
	pea.l      new_bios_longframe
	bra.s      install_bios1
xbios_short:
	pea.l      new_bios_shortframe
install_bios1:
	move.w     #0x45,-(a7) /* install new bios handler */
	move.w     #5,-(a7)
	trap       #13
	addq.l     #8,a7
	movem.l    (a7)+,d1-d2/a0-a2
	move.l     a7,x11964
	rts

	.dc.l 0x58425241 /* 'XBRA' */
	.dc.l 0x53434b4d /* 'SCKM' */
	.dc.l 0
new_bios_longframe:
	subq.w     #1,bios_counter
	bne.s      new_bios_l2
	move.w     bios_intervall(pc),bios_counter
	tas.b      bios_sema
	bmi.s      new_bios_l2
	btst       #2,(a7)
	bne.s      new_bios_l1
	move.l     a0,-(a7)
	movea.l    real_p_kernel,a0
	movea.l    mxk_pe_slice(a0),a0
	move.w     (a0),save_slice
	move.w     #-1,(a0)
	movea.l    (a7)+,a0
	clr.w      -(a7) /* push dummy frame word */
	pea.l      ret_from_bios
	move.w     sr,-(a7)
	move.l     new_bios_shortframe-4,-(a7)
	rts
new_bios_l1:
	sf         bios_sema
new_bios_l2:
	move.l     new_bios_longframe-4,-(a7)
	rts

	.dc.l 0x58425241 /* 'XBRA' */
	.dc.l 0x53434b4d /* 'SCKM' */
	.dc.l 0

new_bios_shortframe:
	subq.w     #1,bios_counter
	bne.s      new_bios_s2
	move.w     bios_intervall(pc),bios_counter
	tas.b      bios_sema
	bmi.s      new_bios_s2
	btst       #2,(a7)
	bne.s      new_bios_s1
	move.l     a0,-(a7)
	movea.l    real_p_kernel,a0
	movea.l    mxk_pe_slice(a0),a0
	move.w     (a0),save_slice
	move.w     #-1,(a0)
	movea.l    (a7)+,a0
	pea.l      ret_from_bios
	move.w     sr,-(a7)
	move.l     new_bios_shortframe-4,-(a7)
	rts
new_bios_s1:
	sf         bios_sema
new_bios_s2:
	move.l     new_bios_shortframe-4,-(a7)
	rts

ret_from_bios:
	movem.l    d0-d7/a0-a6,-(a7)
	move.w     60(a7),d0
	bset       #13,d0
	move.w     d0,sr
	move.w     x1e714,d0
	bne        ret_b10
	lea.l      x1ef60,a2
ret_b1:
	move.l     (a2),d1
	beq.s      ret_b3
	movea.l    d1,a0
	move.l     8(a0),d4
	bgt.s      ret_b3
	move.w     sr,d5
	ori.w      #0x0700,sr
	move.l     18(a0),d3
	movea.l    4(a0),a3
	movea.l    12(a0),a4
	move.l     (a0),(a2)
	move.l     (a2),d0
	beq.s      ret_b2
	movea.l    d0,a1
	add.l      d4,8(a1)
ret_b2:
	move.l     4(a2),(a0)
	move.l     (hz_200).w,d0
	add.l      #400,d0
	move.l     d0,8(a0)
	move.l     a0,4(a2)
	move.w     d5,sr
	move.l     d3,d0
	movea.l    a3,a0
	jsr        (a4)
	bra.s      ret_b1
ret_b3:
	lea.l      x1ef64,a2
	move.l     (a2),d1
	beq.s      ret_b10
	move.w     sr,d4
	ori.w      #0x0700,sr
	movea.l    d1,a3
	move.l     (hz_200).w,d3
ret_b4:
	cmp.l      8(a3),d3
	ble.s      ret_b8
	clr.l      (a2)
	move.w     d4,sr
ret_b5:
	moveq.l    #2,d0
	and.w      16(a3),d0
	beq.s      ret_b6
	andi.w     #-2,16(a3)
	bra.s      ret_b7
ret_b6:
	movea.l    a3,a0
	subq.l     #2,a0
	clr.b      (a0)
ret_b7:
	movea.l    (a3),a3
	move.l     a3,d0
	beq.s      ret_b10
	bra.s      ret_b5
ret_b8:
	movea.l    a3,a2
	move.l     (a2),d1
	beq.s      ret_b9
	movea.l    d1,a3
	bra.s      ret_b4
ret_b9:
	move.w     d4,sr
ret_b10:
	movea.l    real_p_kernel,a0
	movea.l    mxk_pe_slice(a0),a0
	move.w     save_slice,(a0)
	movem.l    (a7)+,d0-d7/a0-a6
	sf         bios_sema
	rte

	.dc.w 0
bios_counter:
	.dc.w 1
bios_intervall:
	.dc.w 4
bios_sema:
	.dc.w 0
save_slice:
	.dc.w 1
	.dc.w 0
x11964:
	.dc.l 0
x11968:
	.dc.l 0
x1196c:
	.dc.l 0
x11970:
	.ds.b 66

	.data
my_kernel:
	.dc.w 0
	.dc.l ker_fast_clrmem
	.dc.l ker_toupper
	.dc.l ker_sprintf
	.dc.l 0
	.dc.l 0
	.dc.l 0
	.dc.l 0
	.dc.l 0
	.dc.l ker_appl_yield
	.dc.l ker_appl_suspend
	.dc.l ker_appl_begcritic
	.dc.l ker_appl_endcritic
	.dc.l ker_evnt_IO
	.dc.l ker_evnt_mIO
	.dc.l ker_evnt_emIO
	.dc.l ker_appl_IOcomplete
	.dc.l ker_evnt_sem
	.dc.l ker_Pfree
	.dc.w 0
	.dc.l ker_int_malloc
	.dc.l ker_int_mfree
	.dc.l ker_resv_intmem
	.dc.l ker_diskchange
	.dc.l ker_DMD_rdevinit
	.dc.l ker_proc_info
	.dc.l ker_mxalloc
	.dc.l ker_mfree
	.dc.l ker_mshrink

	.globl real_p_kernel
real_p_kernel:
	.dc.l 0
