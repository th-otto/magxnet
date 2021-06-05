#ifndef __GNU_STAB__
#define __GNU_STAB__

#ifdef __cplusplus
extern "C" {
#endif

#define __define_stab(NAME, CODE, STRING) NAME=CODE,

enum __stab_debug_code
{
#include "stab.def"
LAST_UNUSED_STAB_CODE
};

#undef __define_stab

#ifdef __cplusplus
}
#endif

#endif /* __GNU_STAB_ */
