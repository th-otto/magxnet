#
# Makefile for sockets.dev
#

include ../configvars

PROGRAMS = sockets.dev

OBJS = \
	main.o \
	sockdev.o \
	mxdev.o \
	ipcutil.o \
	iov.o \
	dummydev.o \
	dummyasm.o \
	etvtimer.o \
	mxkernel.o \
	arpdev.o \
	inet.o \
	arp.o \
	ip.o \
	masquera.o \
	masqdev.o \
	udp.o \
	tcp.o \
	tcpin.o \
	tcpout.o \
	tcpsig.o \
	tcpdinit.o \
	tcputil.o \
	timer.o \
	buf.o \
	inetutil.o \
	route.o \
	routeasm.o \
	routedev.o \
	if.o \
	loopback.o \
	icmp.o \
	igmp.o \
	bpf.o \
	bpfasm.o \
	bpf_filt.o \
	inetasm.o \
	inetdev.o \
	port.o \
	ifload.o \
	ifeth.o \
	rawip.o \
	x28.o \
	rest.o \
	$(empty)

# OBJS += masqasm.o

all:: $(PROGRAMS)

EVAL_MSHORT = \
	if echo "" | $(CC) $(CFLAGS) -dM -E - | grep -q __MSHORT__; then echo 1; else echo 0; fi

.s.o:
	$(CC) -c $(CFLAGS) -Wa,-defsym,GNUC=1 -Wa,-defsym,MSHORT=$(shell $(EVAL_MSHORT)) -o $@ $<

sockets.dev: $(OBJS) $(SOCKLIB_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(SOCKLIB)

clean::
	rm -f *.o *.pdb *.prg *.ttp *.map $(PROGRAMS)
