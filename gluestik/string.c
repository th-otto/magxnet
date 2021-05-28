#include <string.h>


char *index(const char *str, int c)
{
	return strchr(str, c);
}


char *rindex(const char *str, int c)
{
	return strrchr(str, c);
}