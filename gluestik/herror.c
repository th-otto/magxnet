#include "stsocket.h"
#include "mintsock.h"

#if !MAGIC_ONLY
static const char *h_errlist[] =
{
	"Resolver Error 0 (no error)",
	"Unknown host",				/* 1 HOST_NOT_FOUND */
	"Host name lookup failure",		/* 2 TRY_AGAIN */
	"Unknown server error",			/* 3 NO_RECOVERY */
	"No address associated with name",	/* 4 NO_ADDRESS */
};
#define h_nerr (int)(sizeof h_errlist / sizeof h_errlist[0])


const char *hstrerror(int err)
{
	if (err < 0)
		return "Resolver internal error";
	else if (err < h_nerr)
		return h_errlist[err];
	
	return "Unknown resolver error";
}
#endif
