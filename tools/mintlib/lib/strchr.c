/* from Henry Spencer's stringlib */
/* modified by ERS */
#include <string.h>

/*
 * strchr - find first occurrence of a character in a string
 */
#ifdef __GNUC__
asm(".stabs \"_index\",5,0,0,_strchr");	/* dept of clean tricks */
#else
char *index(s, charwanted)
const char *s;
int charwanted;
{
	return strchr(s, charwanted);
}
#endif

char *									/* found char, or NULL if none */
strchr(s, charwanted)
const char *s;
register int charwanted;
{
	register char c;

	/*
	 * The odd placement of the two tests is so NUL is findable.
	 */
	while ((c = *s++) != (char) charwanted)
		if (c == 0)
			return NULL;
	return ((char *) --s);
}
