#include <stdio.h>

short getw(fp)
register FILE *fp;
{
	register short n,
	 c;

	if ((c = getc(fp)) == EOF)
		return (EOF);
	n = (c << 8);
	if ((c = getc(fp)) == EOF)
		return (EOF);
	n |= (c & 0xFF);
	return (n);
}

#ifdef __STDC__
short putw(short n, FILE * fp)
#else
short putw(n, fp)
register short n;
register FILE *fp;
#endif
{
	register short m;

	m = (n >> 8);
	if (fputc((m & 0xFF), fp) == EOF)
		return (EOF);
	if (fputc((n & 0xFF), fp) == EOF)
		return (EOF);
	return (n);
}

long getl(fp)
register FILE *fp;
{
	register long n,
	 c;

	if ((c = getw(fp)) == EOF)
		return (EOF);
	n = (c << 16);
	if ((c = getw(fp)) == EOF)
		return (EOF);
	n |= (c & 0xFFFF);
	return (n);
}

long putl(n, fp)
register long n;
register FILE *fp;
{
	register long m;

	m = (n >> 16);
	if (putw((m & 0xFFFFL), fp) == EOF)
		return (EOF);
	if (putw((n & 0xFFFFL), fp) == EOF)
		return (EOF);
	return (n);
}
