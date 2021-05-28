#include <string.h>

static void _real_bzero(char *p, size_t size)
{
	size_t longs;
	size_t words;
	
	if ((long)p & 1)
	{
		*p++ = 0;
		size--;
	}
	longs = size >> 2;
	size -= longs << 2;
	words = size >> 1;
	size -= words << 1;
	while (longs-- != 0)
		*((long *)p++) = 0;
	while (words-- != 0)
		*((short *)p++) = 0;
	while (size-- != 0)
		*p++ = 0;
}

void bzero(void *p, size_t size)
{
	_real_bzero(p, size);
}
