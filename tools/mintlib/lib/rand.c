/*  rand(3)
 *
 *  Author: Terrence W. Holm          Nov. 1988
 *
 *
 *  A prime modulus multiplicative linear congruential
 *  generator (PMMLCG), or "Lehmer generator".
 *  Implementation directly derived from the article:
 *
 *	S. K. Park and K. W. Miller
 *	Random Number Generators: Good Ones are Hard to Find
 *	CACM vol 31, #10. Oct. 1988. pp 1192-1201.
 *
 *
 *  Using the following multiplier and modulus, we obtain a
 *  generator which:
 *
 *	1)  Has a full period: 1 to 2^31 - 2.
 *	2)  Is testably "random" (see the article).
 *	3)  Has a known implementation by E. L. Schrage.
 */

#include <stddef.h>
#include <stdlib.h>

#define  A	  16807L					/*  A "good" multiplier   */
#define  M   2147483647L				/*  Modulus: 2^31 - 1     */
#define  Q       127773L				/*  M / A         */
#define  R         2836L				/*  M % A         */


static long _lseed = 1L;


void srand(seed)
unsigned int seed;

{
	_lseed = seed;
}


int rand()
{
	_lseed = A * (_lseed % Q) - R * (_lseed / Q);

	if (_lseed < 0)
		_lseed += M;

#ifdef __MSHORT__
	return ((int) (_lseed & 0x7fffL));	/* how "good" is it now ? */
#else
	return ((int) _lseed);
#endif
}
