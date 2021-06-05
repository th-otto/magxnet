/* moved to a separate module, so that people who want to can put
 * __default_mode__ = _IOBIN in their main program without getting
 * a link conflict; moreover, this allows stdin and stdout to be
 * binary mode by default as well. ++ERS
 *
 * moved into a module independent of binmode so that if its undefined
 * by the user it is pulled in from here without pulling in binmode() too.
 *
 */
int __default_mode__ = 0;
