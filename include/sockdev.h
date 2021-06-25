struct magxnet_cookie {
	const char *version;
	const char *author;
	unsigned long magic;
	long (*init_timer)(void);
	void (*checkalarms)(void);
	BASEPAGE *base;
	long (*Fopen)(const char *filename);
	short (*Fclose)(short fd);
	void *o32;              /* unused */
	void /* struct slbuf */ *allslbufs;
	void *o40;              /* unknown function pointer; not used */
	long initialized;       /* whether sockets.dev has been initialized by magxconf */
	void *masq;             /* pointer to MASQ_GLOBAL_INFO */
};

#ifdef __KERNEL__
extern struct magxnet_cookie cookie;
#endif
