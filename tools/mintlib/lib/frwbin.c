/*
 * this flag controls the behaviour fread/fwrite:
 *
 *  when non-zero, fread/fwrite will not do CR processing
 *  even for text mode files (ie: ignore file mode).
 *
 * when zero fread/fwrite is process according to file mode
 *
 * this file is linked in when the user does not specify the variable.
 */
short __FRW_BIN__ = 0;
