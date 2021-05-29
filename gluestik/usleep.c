#include "stsocket.h"
#include "mintsock.h"


int usleep(__useconds_t dt)
{
	clock_t t;
	clock_t tt;

	tt = ((clock_t) dt) / (((clock_t) 1000000UL) / CLOCKS_PER_SEC);
	t = clock();
	while ((clock() - t) < tt)
		;
	return 0;
}
