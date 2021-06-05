/* if the user doesn't link PCFLTLIB.LIB, we have to provide
   a dummy function _fpuinit();                hohmuth 31 Aug 92 */

#ifndef __NO_FLOAT__
void _fpuinit(void) { }			/* called from main.c */
#endif
