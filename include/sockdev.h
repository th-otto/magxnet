struct magxnet_cookie {
	const char *version;
	const char *author;
	unsigned long magic;
	long (*init_timer)(void);
	void (*checkalarms)(void);
	BASEPAGE *base;
	long (*Fopen)(const char *filename);
	short (*Fclose)(short fd);
	void *o32;
	struct slbuf *dev_table;
	void *o40;
	long o44;
	void *masq;
};

#ifndef __slbuf_defined
#define __slbuf_defined 1
struct slbuf {
	unsigned char flags;
# define SL_INUSE	0x01		/* slbuf in use */
# define SL_SENDING	0x02		/* send in progress */
# define SL_CLOSING	0x04		/* close in progress */

	char		dev[127];	/* device name */
	short fd;
	struct netif	*nif;	/* interface this belongs to */
	short		isize;		/* input ring buffer size */
	char		*ibuf;		/* pointer to input buf */
	short		ihead;		/* input buffer head */
	short		itail;		/* output buffer tail */

	short		osize;		/* ditto for output */
	char		*obuf;		/* ditto for output */
	short		ohead;		/* ditto for output */
	short		otail;		/* ditto for output */

	short		(*send)(struct slbuf *);	/* send more */
	short		(*recv)(struct slbuf *);	/* recv more */
	long		nread;		/* bytes avail for reading */
	long		nwrite;		/* bytes avail for writing */
};
#endif

#ifdef __KERNEL__
extern struct magxnet_cookie cookie;
#endif
