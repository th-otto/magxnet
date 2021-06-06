#
# Makefile for gluestik, a stik emulator for magicnet/mintnet
#

include ../configvars

PROGRAMS = libsock.a

OBJS = \
	herror.o \
	accept.o \
	bind.o \
	connect.o \
	sockglob.o \
	gethost.o \
	sockname.o \
	inetaddr.o \
	makeaddr.o \
	listen.o \
	res_comp.o \
	recv.o \
	res_init.o \
	res_mkq.o \
	res_quer.o \
	res_send.o \
	send.o \
	sendmsg.o \
	sendto.o \
	socket.o \
	hostname.o \
	recvfrom.o \
	poll.o \
	select.o \
	getnet.o \
	getproto.o \
	getserv.o \
	inetneto.o \
	inetnetw.o \
	sethost.o \
	$(empty)

all:: $(PROGRAMS)

libsock.a: $(OBJS)
	$(RM) $@
	$(AR) rcs $@ $^

$(OBJS): ../include/stsocket.h ../include/mintsock.h

clean::
	rm -f *.o *.a *.pdb *.prg *.ttp *.map $(PROGRAMS)
