#
# Makefile for gluestik, a stik emulator for magicnet/mintnet
#

CROSS = m68k-atari-mint-
CC = $(CROSS)gcc
AS = $(CROSS)gcc
CFLAGS = -O2 -fomit-frame-pointer -Wall -W -Wshadow -Werror
LDFLAGS = -s -Wl,--mprg-flags,0x17

PROGRAMS = gluestik.prg

OBJS = \
	gs_conf.o \
	gs_func.o \
	gs_mem.o \
	gs_stik.o \
	main.o \
	bzero.o \

ifneq ($(CROSS),)
OBJS += mt_aes.o
#OBJS += socket.o gethost.o hostname.o
endif

all: $(PROGRAMS)

gluestik.prg: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lm

$(OBJS): gs.h gs_conf.h gs_func.h gs_mem.h gs_stik.h transprt.h
gs_stik.o main.o: version.h

mt_aes.o: mt_aes.s
	$(AS) -c -o $@ $<

clean::
	rm -f *.o *.pdb *.prg *.ttp *.map $(PROGRAMS)
