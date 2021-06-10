struct magxnet_cookie {
	const char *version;
	const char *author;
	unsigned long magic;
	long (*init_timer)(void);
	void (*o16)(void);
	BASEPAGE *base;
	long (*Fopen)(const char *filename);
	short (*Fclose)(short fd);
	void *o32;
	struct sockdev *dev_table;
	void *o40;
	long o44;
	long o48;
};


struct sockdev {
	unsigned char flags;
	char o1[127];
	short fd;
	char o130[4];
	short inbuf_size;
	char *inbuf_ptr;
	short input_tail;
	short input_head;
	short outbuf_size;
	char *outbuf_ptr;
	short output_tail;
	short output_head;
	void (*write_dev)(struct sockdev *);
	void (*read_dev)(struct sockdev *);
	long input_avail;
	long output_avail;
};
