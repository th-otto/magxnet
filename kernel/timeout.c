/*
 * This file has been modified as part of the FreeMiNT project. See
 * the file Changes.MH for details and dates.
 * 
 * 
 * Copyright 1990,1991,1992 Eric R. Smith.
 * Copyright 1992,1993,1994 Atari Corporation.
 * All rights reserved.
 * 
 */

#include "sockets.h"
#include <stddef.h>
#include <stdlib.h>
#include "timeout.h"
#include "kerinfo.h"
#include "bpf.h"
#include "mxkernel.h"
#include "asm_spl.h"

#define TIMEOUTS		64				/* # of static timeout structs */
#define TIMEOUT_USED		0x01		/* timeout struct is in use */
#define TIMEOUT_STATIC		0x02		/* this is a static timeout */

/* This gets implicitly initialized to zero, thus the flags are
 * set up correctly.
 */
static TIMEOUT timeouts[TIMEOUTS] = { { 0, 0, 0, 0, 0, 0 } }; /* FIXME: why initialized? */
TIMEOUT *tlist GNU_ASM_NAME("tlist") = NULL;
TIMEOUT *expire_list GNU_ASM_NAME("expire_list") = NULL;

/* Number of ticks after that an expired timeout is considered to be old
 * and disposed automatically.
 */
#define TIMEOUT_EXPIRE_LIMIT	400		/* 2 secs */

static TIMEOUT *newtimeout(short fromlist)
{
	if (!fromlist)
	{
		TIMEOUT *t;

		t = timeout_alloc();
		if (t)
		{
			t->flags = 0;
			t->arg = 0;
			return t;
		}
	}

	{
		int i;
		short sr;

#ifdef __PUREC__
		/* only for binary equivalence; might as well use splhigh() */
		sr = getsr();
		setipl7();
#else
		sr = splhigh();
#endif
		for (i = 0; i < TIMEOUTS; i++)
		{
			if (!(timeouts[i].flags & TIMEOUT_USED))
			{
				timeouts[i].flags |= (TIMEOUT_STATIC | TIMEOUT_USED);
				spl(sr);
				timeouts[i].arg = 0;
				return &timeouts[i];
			}
		}
		spl(sr);
	}

	return 0;
}


#define TIMEOUT_POOL(t) \
	((struct timeout_pool *)((char *)(t) - offsetof(struct timeout_pool, tmout)))

static void disposetimeout(TIMEOUT *t)
{
	if (t->flags & TIMEOUT_STATIC)
		t->flags &= ~TIMEOUT_USED;
	else
		TIMEOUT_POOL(t)->inuse = 0;
}


void dispose_old_timeouts(void)
{
	TIMEOUT *t;
	TIMEOUT **prev;
	long now = *((volatile long *)0x4ba);
	short sr;
	
#ifdef __PUREC__
	/* only for binary equivalence; might as well use splhigh() */
	sr = getsr();
	setipl7();
#else
	sr = splhigh();
#endif

	for (prev = &expire_list, t = *prev; t; prev = &t->next, t = *prev)
	{
		if (t->when < now)
		{
			/* This and the following timeouts are too old.
			 * Throw them away.
			 */

			*prev = 0;
			spl(sr);
			while (t)
			{
#if 0
				TIMEOUT *old;

				old = t;
				t = t->next;
				disposetimeout(old);
#else
				/* inlined disposetimeout */
				if (t->flags & TIMEOUT_STATIC)
					t->flags &= ~TIMEOUT_USED;
				else
					TIMEOUT_POOL(t)->inuse = 0;
				t = t->next;
#endif
			}
			return;
		}
	}

	spl(sr);
}


static void inserttimeout(TIMEOUT *t, long delta)
{
	TIMEOUT **prev;
	TIMEOUT *cur;
	short sr;
	
#ifdef __PUREC__
	/* only for binary equivalence; might as well use splhigh() */
	sr = getsr();
	setipl7();
#else
	sr = splhigh();
#endif

	cur = tlist;
	prev = &tlist;
	while (cur)
	{
		if (cur->when >= delta)
		{
			cur->when -= delta;
			t->next = cur;
			t->when = delta;
			*prev = t;
			spl(sr);
			return;
		}
		delta -= cur->when;
		prev = &cur->next;
		cur = cur->next;
	}

#ifndef __GNUC__
	/* BUG: forgotten assert(delta >= 0) */
#undef assert
#define assert(expr)\
	((void)((expr)||(fprintf(stderr, \
	"\nAssertion failed: %s, file %s, line %d\n",\
	 #expr, "H:\\MAGICNET\\NET\\INET\\TIMEOUT.C", 173),\
	 ((int (*)(void))abort)())))
	assert(delta >= 0);
#endif
	
	t->when = delta;
	t->next = cur;
	*prev = t;

	spl(sr);
}

/*
 * addroottimeout(long delta, void (*)(PROC *), short flags);
 * Same as addtimeout(), except that the timeout is attached to Pid 0 (MiNT).
 * This means the timeout won't be cancelled if the process which was
 * running at the time addroottimeout() was called exits.
 *
 * Currently only bit 0 of `flags' is used. Meaning:
 * Bit 0 set: Call from interrupt (cannot use kmalloc, use statically
 *	allocated `struct timeout' instead).
 * Bit 0 clear: Not called from interrupt, can use kmalloc.
 *
 * Thus addroottimeout() can be called from interrupts (bit 0 of flags set),
 * which makes it *extremly* useful for device drivers.
 * A serial device driver would make an addroottimeout(0, check_keys, 1)
 * if some bytes have arrived.
 * check_keys() is then called at the next context switch, can use all
 * the kernel functions and can do time cosuming jobs.
 */

TIMEOUT *cdecl addroottimeout(long delta, to_func *func, ushort flags)
{
	TIMEOUT *t;
	TIMEOUT **prev;
	ushort sr;

#ifdef __PUREC__
	/* only for binary equivalence; might as well use splhigh() */
	sr = getsr();
	setipl7();
#else
	sr = splhigh();
#endif

	/* Try to reuse an already expired timeout that had the
	 * same function attached
	 */
	prev = &expire_list;
	t = *prev;
	for (; t != NULL; )
	{
		if (/* t->proc == p && */ t->func == func)
		{
			*prev = t->next;
			break;
		}
		prev = &t->next;
		t = *prev;
	}

	spl(sr);

	if (t == NULL)
		t = newtimeout(flags & 1);

	if (t)
	{
		/* t->proc = p; */
		t->func = func;
		inserttimeout(t, delta);
	}

	return t;
}

#if 0
TIMEOUT *cdecl addtimeout(PROC *p, long delta, to_func *func)
{
	return __addtimeout(p, delta, func, 0);
}
#endif


/*
 * cancelalltimeouts(): cancels all pending timeouts for the current
 * process
 */

void _cdecl cancelalltimeouts(void)
{
	TIMEOUT *cur;
	TIMEOUT **prev;
	TIMEOUT *old;
	long delta;
	short sr;
	
#ifdef __PUREC__
	/* only for binary equivalence; might as well use splhigh() */
	sr = getsr();
	setipl7();
#else
	sr = splhigh();
#endif

	cur = tlist;
	prev = &tlist;
	while (cur)
	{
		if (cur->proc == (PROC *)get_curproc())
		{
			delta = cur->when;
			old = cur;
			*prev = cur = cur->next;
			if (cur)
				cur->when += delta;
			spl(sr);
			disposetimeout(old);
#ifdef __PUREC__
			/* only for binary equivalence; might as well use splhigh() */
			sr = getsr();
			setipl7();
#else
			sr = splhigh();
#endif

			/* ++kay: just in case an interrupt handler installed a
			 * timeout right after `prev' and before `cur'
			 */
			cur = *prev;
		} else
		{
			prev = &cur->next;
			cur = cur->next;
		}
	}

	prev = &expire_list;
	for (cur = *prev; cur; cur = *prev)
	{
		if (cur->proc == (PROC *)get_curproc())
		{
			*prev = cur->next;
			spl(sr);
			disposetimeout(cur);
#ifdef __PUREC__
			/* only for binary equivalence; might as well use splhigh() */
			sr = getsr();
			setipl7();
#else
			sr = splhigh();
#endif
		} else
		{
			prev = &cur->next;
		}
	}

	spl(sr);
}

/*
 * Cancel a specific timeout. If the timeout isn't on the list, or isn't
 * for this process, we do nothing; otherwise, we cancel the time out
 * and then free the memory it used. *NOTE*: it's very possible (indeed
 * likely) that "this" was already removed from the list and disposed of
 * by the timeout processing routines, so it's important that we check
 * for it's presence in the list and do absolutely nothing if we don't
 * find it there!
 */

void _cdecl cancelroottimeout(TIMEOUT *this)
{
	TIMEOUT *cur;
	TIMEOUT **prev;
	short sr;
	
#ifdef __PUREC__
	/* only for binary equivalence; might as well use splhigh() */
	sr = getsr();
	setipl7();
#else
	sr = splhigh();
#endif

	/* First look at the list of expired timeouts */
	prev = &expire_list;
	for (cur = *prev; cur; cur = *prev)
	{
		if (cur == this /* && cur->proc == p */)
		{
			*prev = cur->next;
			spl(sr);
			disposetimeout(this);
			return;
		}
		prev = &cur->next;
	}

	prev = &tlist;
	for (cur = tlist; cur; cur = cur->next)
	{
		if (cur == this /* && cur->proc == p */)
		{
			*prev = cur->next;
			if (cur->next)
			{
				cur->next->when += this->when;
			}
			spl(sr);
			disposetimeout(this);
			return;
		}
		prev = &cur->next;
	}

	spl(sr);
}

#if 0
void _cdecl canceltimeout(TIMEOUT *this)
{
	__canceltimeout(this, (PROC *)get_curproc());
}
#endif


/*
 * timeout: called every 20 ms or so by GEMDOS, this routine
 * is responsible for maintaining process times and such.
 * it should also decrement the "proc_clock" variable, but
 * should *not* take any action when it reaches 0 (the state of the
 * stack is too uncertain, and time is too critical). Instead,
 * a vbl routine checks periodically and if "proc_clock" is 0
 * suspends the current process
 */

/* moved to intr.spp (speed reasons)
 * was called by mint_timer()
 */
#if 0

static volatile int our_clock = 1000;

void _cdecl timeout(void)
{
	short ms;

	c20ms++;

	kintr = keyrec->head != keyrec->tail;

	if (proc_clock)
		proc_clock--;

	ms = *((short *) 0x442L);
	our_clock -= ms;

	if (tlist)
		tlist->when -= ms;
}
#endif

/*
 * sleep() calls this routine to check on alarms and other sorts
 * of time-outs on every context switch.
 */

void checkalarms(void)
{
	ushort sr;
	long delta;

#ifdef NOTYET
	/* do the once per second things */
	while (our_clock < 0)
	{
		our_clock += 1000;

		/* Updates timestamp and datestamp. */
		synch_timers();

		searchtime++;
		reset_priorities();
	}
#endif

#ifdef __PUREC__
	/* only for binary equivalence; might as well use splhigh() */
	sr = getsr();
	setipl7();
#else
	sr = splhigh();
#endif

	/* see if there are outstanding timeout requests to do */
	while (tlist && ((delta = tlist->when) <= 0))
	{
		/* hack: pass an extra long as args, those intrested in it will
		 * need a cast and have to place it in t->arg themselves but
		 * that way everything else still works without change -nox
		 */
#ifdef __GNUC__
		register long args __asm__("d0") = tlist->arg;
		register PROC *p __asm__("a0") = tlist->proc;
#else
		long args = tlist->arg;
		PROC *p = tlist->proc;
#endif
		to_func *evnt = tlist->func;
		TIMEOUT *old = tlist;

		tlist = tlist->next;

		/* if delta < 0, it's possible that the time has come for the
		 * next timeout to occur.
		 * ++kay: moved this before the timeout fuction is called, in
		 * case the timeout function installs a new timeout.
		 */
		if (tlist)
			tlist->when += delta;

		old->next = expire_list;
		old->when = *(long *) 0x4ba + TIMEOUT_EXPIRE_LIMIT;
		expire_list = old;

		spl(sr);

		/* ++kay: debug output at spl7 hangs the system, so moved it
		 * here
		 */
		TRACE(("doing timeout code for pid %d", p->pid));

		/* call the timeout function */
#ifdef __GNUC__
		/*
		 * take care to call it in a way that works both for cdecl
		 * and Pure-C calling conventions, since there seem
		 * to be drivers around that were compiled by it.
		 */
		__asm__ __volatile__(
			"\tmove.l %1,-(%%a7)\n"
			"\tmove.l %0,-(%%a7)\n"
			"\tjsr (%2)\n"
#ifdef __mcoldfire__
			"\taddq.l #8,%%a7\n"
#else
			"\taddq.w #8,%%a7\n"
#endif
			: /* no outputs */
			: "a"(p), "d"(args), "a"(evnt)
			: "d1", "d2", "a1", "a2", "cc", "memory");
#else
		evnt(p, args);
#endif

#ifdef __PUREC__
		/* only for binary equivalence; might as well use splhigh() */
		sr = getsr();
		setipl7();
#else
		sr = splhigh();
#endif
	}

	spl(sr);

	/* Now look at the expired timeouts if some are getting old */
	dispose_old_timeouts();
}
