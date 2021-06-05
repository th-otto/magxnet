#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "lib.h"

/* new file 4/15/92 sb
   This function had to be split from the [vf]printf functions to accommodate
   Sozobon's floating point libs.  Originally, anything that used the assert()
   macro [like malloc()] would pull in this function and bring with it
   definitions of printf et.al.; if we've already included the printf() from
   libm.a, this will cause a conflict.

   5/2/92 sb
   Modified to do its own work rather than call fprintf().

   5/4/93 uo
   Threw away a static buffer.
*/

static void _say __PROTO((const char *s));

static void _say(s)
const char *s;
{
	_write(2, s, (long) (strlen(s)));
}

/* This is used by the `assert' macro.  */
void __eprintf(expression, line, filename)
const char *expression;
const long line;
const char *filename;
{
	char buf[20];

	_ltoa(line, buf, 10);
	_say("assertion `");
	_say(expression);
	_say("' failed at line ");
	_say(buf);
	_say(" of file ");
	_say(filename);
	_say("\r\n");
}
