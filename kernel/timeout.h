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

extern TIMEOUT *tmout;

TIMEOUT *addtimeout(struct proc *p, long delta, void (*func)(struct proc *, long));
TIMEOUT *addtimeout_curproc(long delta, void (*func)(struct proc *, long));
TIMEOUT *cdecl addroottimeout(long delta, void cdecl (*func)(struct proc *, long), ushort flags);
void cancelalltimeouts (void);
void canceltimeout(TIMEOUT *which);
void cdecl cancelroottimeout(TIMEOUT *which);


#endif /* _timeout_h */
