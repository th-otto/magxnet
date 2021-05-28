				.globl Psysctl
				MODULE	Psysctl
				pea		(a2)
				move.l	d1,-(a7) ; newlen
				move.l	16(a7),-(a7) ; new
				move.l	16(a7),-(a7) ; oldlen
				pea     (a1) ; old
				move.l	d0,-(a7) ; namelen
				pea     (a0) ; name
				move.w	#$15e,-(a7)
				trap #1
				lea		26(a7),a7
				move.l	(a7)+,a2
				rts
				ENDMOD
				
/* long Fsendmsg(short fd, const struct msghdr *msg, long flags); */
				.globl Fsendmsg
				MODULE	Fsendmsg
				pea		(a2)
				move.l	d1,-(a7)
				pea     (a0)
				move.w	d0,-(a7)
				move.w	#$167,-(a7)
				trap #1
				lea		12(a7),a7
				move.l	(a7)+,a2
				rts
				ENDMOD

/* long Frecvfrom(short fd, void *buf, unsigned long len, long flags, struct sockaddr *from, unsigned long *fromlenaddr); */
				.globl Frecvfrom
				MODULE	Frecvfrom
				pea		(a2)
				move.l	8(a7),-(a7)
				pea     (a1)
				move.l	d2,-(a7)
				move.l	d1,-(a7)
				pea     (a0)
				move.w	d0,-(a7)
				move.w	#$168,-(a7)
				trap #1
				lea		24(a7),a7
				move.l	(a7)+,a2
				rts
				ENDMOD

/* long Fsendto(short fd, const void *buf, unsigned long len, long flags, const struct sockaddr *to, unsigned long tolen); */
				.globl Fsendto
				MODULE	Fsendto
				pea		(a2)
				move.l	8(a7),-(a7)
				pea     (a1)
				move.l	d2,-(a7)
				move.l	d1,-(a7)
				pea     (a0)
				move.w	d0,-(a7)
				move.w	#$169,-(a7)
				trap #1
				lea		24(a7),a7
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl Fpoll
				MODULE	Fpoll
				pea		(a2)
				move.l	d1,-(a7)
				move.l	d0,-(a7)
				pea     (a0)
				move.w	#$15a,-(a7)
				trap #1
				lea		14(a7),a7
				move.l	(a7)+,a2
				rts
				ENDMOD
				
