#
# Makefile for gluestik, a stik emulator for magicnet/mintnet
#

include ../configvars

PROGRAMS = libsock.a

OBJS = \
	accept.o \
	bind.o \
	connect.o \
	gethost.o \
	getnet.o \
	getprote.o \
	getprotn.o \
	getproto.o \
	getserv.o \
	getservn.o \
	herror.o \
	hostname.o \
	hstrerr.o \
	inetaddr.o \
	inetlnao.o \
	inetneto.o \
	inetnetw.o \
	listen.o \
	makeaddr.o \
	poll.o \
	recv.o \
	recvfrom.o \
	res_comp.o \
	res_init.o \
	res_mkq.o \
	res_quer.o \
	res_send.o \
	select.o \
	send.o \
	sendmsg.o \
	sendto.o \
	sethost.o \
	socket.o \
	sockglob.o \
	sockname.o \
	sockpair.o \
	sockopt.o \
	$(empty)

all:: $(PROGRAMS)

libsock.a: $(OBJS)
	$(RM) $@
	$(AR) rcs $@ $^

$(OBJS): ../include/stsocket.h ../include/mintsock.h

clean::
	rm -f *.o *.a *.pdb *.prg *.ttp *.map $(PROGRAMS)
