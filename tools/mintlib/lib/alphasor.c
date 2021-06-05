/*
**  ALPHASORT
**  Trivial sorting predicate for scandir; puts entries in alphabetical order.
*/

#include <types.h>
#include <dirent.h>
#include <string.h>

int alphasort(d1, d2)
struct dirent **d1;
struct dirent **d2;
{
	return strcmp(d1[0]->d_name, d2[0]->d_name);
}
