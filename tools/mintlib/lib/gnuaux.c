/* 
    Selected parts of gnulib for atariST g++
     ++jrb	bammi@cadence.com
*/

#ifndef _COMPILER_H
#include <compiler.h>
#endif

#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <string.h>

/* frills for C++ */

#ifdef L_builtin_new
typedef void (*vfp)();

extern vfp __new_handler;

char *__builtin_new(sz)
size_t sz;
{
	char *p;

	p = (char *) malloc(sz);
	if (p == 0)
		(*__new_handler) ();
	return p;
}
#endif

#ifdef L_builtin_New
typedef void (*vfp)();

static void default_new_handler();

vfp __new_handler = default_new_handler;

char *__builtin_vec_new(p, maxindex, size, ctor)
char *p;
size_t maxindex,
	size;
void (*ctor)();
{
	char *__builtin_new(size_t);
	size_t i,
	 nelts = maxindex + 1;
	char *rval;

	if (p == 0)
		p = (char *) __builtin_new(nelts * size);

	rval = p;

	for (i = 0; i < nelts; i++)
	{
		(*ctor) (p);
		p += size;
	}

	return rval;
}

vfp __set_new_handler(handler)
vfp handler;
{
	vfp prev_handler;

	prev_handler = __new_handler;
	if (handler == 0)
		handler = default_new_handler;
	__new_handler = handler;
	return prev_handler;
}

vfp set_new_handler(handler)
vfp handler;
{
	return __set_new_handler(handler);
}

static void default_new_handler()
{
	/* don't use fprintf (stderr, ...) because it may need to call malloc.  */
	write(2, "default_new_handler: out of memory... aaaiiiiiieeeeeeeeeeeeee!\n", 65);
	/* don't call exit () because that may call global destructors which
	   may cause a loop.  */
	_exit(-1);
}
#endif

#ifdef L_builtin_del
typedef void (*vfp)();

void __builtin_delete(ptr)
char *ptr;
{
	if (ptr)
		free(ptr);
}

void __builtin_vec_delete(ptr, maxindex, size, dtor, auto_delete_vec, auto_delete)
char *ptr;
size_t maxindex,
	size;
void (*dtor)();
int auto_delete;
{
	size_t i,
	 nelts = maxindex + 1;
	char *p = ptr;

	ptr += nelts * size;

	for (i = 0; i < nelts; i++)
	{
		ptr -= size;
		(*dtor) (ptr, auto_delete);
	}

	if (auto_delete_vec)
		free(p);
}

#endif
