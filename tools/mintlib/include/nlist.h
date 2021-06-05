#ifndef _NLIST_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif


struct nlist
{
	char *n_name;
	unsigned short n_type;
	long n_value;
};

__EXTERN int nlist __PROTO((char *file, struct nlist *nl));


#define _NLIST_H
#endif
