/**
 * Representation of an timeout event.
 * The timeout events are stored in a callout list (single linked).
 * The events are sorted in the order of their occurence. Therefor
 * the time for searching the list is saved. The insertion of he event takes
 * longer, but does't happend too often, compared with the searching, which
 * happens at every clock tick. The time when to trigger the event is given
 * relative to the event in the list before this event. So decreasing the
 * counter for the first event in the list decereses the counter for the other
 * events too.
 */
# ifndef _timeout_h
# define _timeout_h

typedef struct timeout TIMEOUT;

typedef struct proc PROC;
struct proc { int dummy; };

/* BUG: in MiNT, this is cdecl */
typedef void to_func (PROC *, long arg);

struct timeout
{
	TIMEOUT	*next;		/**< link to next event in the list.				*/
	PROC	*proc;		/**< This process registerd this timeout event.		*/
	long	when;		/**< Difference to the event before this in the list.*/
	to_func	*func;		/**< Function to call at timeout					*/
	ushort	flags;
	long	arg;		/**< Argument to the function which gets called.	*/
};

struct timeout_pool {
	struct timeout_pool *next;
	char inuse;
	TIMEOUT tmout;
};

extern TIMEOUT *tmout;
extern struct timeout_pool timeout_pool[128];

void timeout_init(struct timeout_pool *pool, size_t size, size_t elemsize) GNU_ASM_NAME("timeout_init");
TIMEOUT *timeout_alloc(void) GNU_ASM_NAME("timeout_alloc");
void checkalarms(void);

TIMEOUT *cdecl addtimeout(struct proc *p, long delta, to_func *func);
TIMEOUT *addtimeout_curproc(long delta, to_func *func);
TIMEOUT *cdecl addroottimeout(long delta, to_func *func, ushort flags);
void cdecl cancelalltimeouts(void);
void cdecl canceltimeout(TIMEOUT *which);
void cdecl cancelroottimeout(TIMEOUT *which);


#endif /* _timeout_h */
