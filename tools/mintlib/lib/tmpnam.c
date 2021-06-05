/* tmpnam.c : return a temporary file name */
/* written by Eric R. Smith and placed in the public domain */
/**
 *  - retuned name can be passed outside via system(); other programs
 *    may not dig '/' as a path separator
 *  - somehow more frugal in a memory use
 *    (mj - October 1990)
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char pattern[] = "\\__XXXXXX";

char *tmpnam(buf)
char *buf;
{
	char *tmpdir;
	size_t tlen;
	extern char *mktemp __PROTO((char *));

	if (((tmpdir = getenv("TEMP")) == NULL) && ((tmpdir = getenv("TMPDIR")) == NULL) &&
		((tmpdir = getenv("TMP")) == NULL) && ((tmpdir = getenv("TEMPDIR")) == NULL))
		tmpdir = ".";

	tlen = strlen(tmpdir);

	if (!buf)
	{
		size_t blen;

		blen = tlen + sizeof(pattern);
		if (NULL == (buf = (char *) malloc(blen)))
			return NULL;
	}
	strcpy(buf, tmpdir);
	if (tmpdir[tlen - 1] == '/' || tmpdir[tlen - 1] == '\\')
		--tlen;
	strcpy(buf + tlen, pattern);
	return (mktemp(buf));
}
