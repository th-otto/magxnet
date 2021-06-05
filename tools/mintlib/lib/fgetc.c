#include <stdio.h>

int fgetc(fp)
FILE *fp;
{
	int c;

	do
	{
		c = --fp->_cnt >= 0 ? ((int) *fp->_ptr++) : _filbuf(fp);
	} while ((!(fp->_flag & _IOBIN)) && (c == '\r'));
	return c;
}
