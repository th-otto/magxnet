# Makefile for STinG, MagiCNet, MINTNet network drivers for NE2000
# on the EtherNEA or EtherNEC BUS interface
#
# Copyright 2002 Dr. Thomas Redelberger
# Use it under the terms of the GNU General Public License
# (See file COPYING.TXT)
#
#$Id: makefile 1.3 2002/06/08 16:38:20 Thomas Exp Thomas $
#

#### Pure-C 1.1
CC=bin\pcc.ttp

LINK=bin\plink.ttp
LFLAGS=
LIBDIR=lib

#### DEVPAC 2.0
#AS=bin\genst2.ttp
#### DEVPAC 3.10
#AS=bin\gen.ttp
#AFLAGS=-L2
### PASM
#AS=bin\pasm.ttp
#TOSLIB=pctoslib.lib
#NO_STACK=-s=0
#MINI_STACK=-s=256
#AFLAGS=-S
# GAS
include ../../configvars
EVAL_MSHORT = \
	if echo "" | $(CC) $(CFLAGS) -dM -E - | grep -q __MSHORT__; then echo 1; else echo 0; fi
AFLAGS= $(CFLAGS) -Wa,-defsym,GNUC=1 -Wa,-defsym,MSHORT=$(shell $(EVAL_MSHORT))
NO_STACK= -Wl,--stacksize,0
MINI_STACK= -Wl,--stacksize,256

CFLAGS += -I../../include -Iinc_stng

#### Mupfel
RM=rm -f
CP=cp
MV=mv
MAKE=make
ZIP=zip
ECHO=echo
#BINDIR=..\bin\
BINDIR=../bin/

OBJS_STNG = startup.o enestng.o nestng.o ne.o uti.o version.o
OBJS_MNET = enemnet.o nemnet.o ne.o uti.o version.o
OBJS_HT3  = ht3ene.o ne.o uti.o
OBJS = $(OBJS_STNG) $(OBJS_MNET) $(OBJS_HT3)


INC_STNG= inc_stng
INC_MNET= inc_mnet

default:	all

drivers:	etherne.stx etherne.mif


etherne.stx:	$(OBJS_STNG)
	$(LINK) $(LFLAGS) -o $@ $(NO_STACK) $(OBJS_STNG) $(TOSLIB)


etherne.mif:	$(OBJS_MNET)
	$(LINK) $(LFLAGS) $(NO_STACK) -o $@ $(OBJS_MNET)


ht1ene.tos:	ht1ene.o
	$(LINK) $(LFLAGS) -s=256 -o=$@ $<

ht2ene.tos:	ht2ene.o
	$(LINK) $(LFLAGS) -s=256 -o=$@ $<


ht3ene.tos:	$(OBJS_HT3)
	$(LINK) $(LFLAGS) -s=256 -o=$@ $(OBJS_HT3)


ht4ene.tos:	ht4ene.o uti.i bus.i uti.s
	$(LINK) $(LFLAGS) -s=256 -o=$@ $<


enemnet.o:	$(INC_MNET)/buf.i $(INC_MNET)/if.i $(INC_MNET)/netinfo.i uti.i

enestng.o:	../../include/transprt.h $(INC_STNG)/port.h ne.h uti.h version.h

nestng.o nemnet.o ne.o:	devswit.i uti.i bus.i 8390.i

nemnet.o:	bus.i $(INC_MNET)/buf.i $(INC_MNET)/if.i $(INC_MNET)/netinfo.i

ne.o:		netdev.i bus.i

uti.o:		uti.i uti.h

ht1ene.o:       ht1ene.s uti.i bus.i 8390.i uti.s 
ht2ene.o:       ht2ene.s uti.i bus.i uti.s 
ht3ene.o:       ht3ene.s uti.i bus.i 8390.i
ht4ene.o:       ht4ene.s uti.i bus.i uti.s 

#################################################################

clean:
	$(RM) *.tos *.stx *.mif *.o


package:
	cd ..; rm -f etherne.zip; $(ZIP) -r etherne.zip . -x "*.o" "*.pdb" "*.git/*" .gitignore


all::	eneaf eneas enec enec3 enem eneh enep2 enex


#################################################################
# do the various IP stack &  hardware configurations/combinations

eneaf:
	$(RM) *.o *.tos *.stx *.mif
	$(MAKE)	DEFS=-Wa,-defsym,BUS=0 EXTRADEFS=-Wa,-defsym,ACSI_SLOW_ACCESS=0 drivers ht1ene.tos ht2ene.tos ht3ene.tos
	$(MV)	etherne.mif $(BINDIR)eneaf.mif	# save these drivers
	$(MV)	etherne.stx $(BINDIR)eneaf.stx
	$(MV)	ht1ene.tos $(BINDIR)ht1eneaf.tos
	$(MV)	ht2ene.tos $(BINDIR)ht2eneaf.tos
	$(MV)	ht3ene.tos $(BINDIR)ht3eneaf.tos


eneas:
	$(RM) *.o *.tos *.stx *.mif
	$(MAKE)	DEFS=-Wa,-defsym,BUS=0 EXTRADEFS=-Wa,-defsym,ACSI_SLOW_ACCESS=1 drivers ht1ene.tos ht2ene.tos ht3ene.tos
	$(MV)	etherne.mif $(BINDIR)eneas.mif
	$(MV)	etherne.stx $(BINDIR)eneas.stx
	$(MV)	ht1ene.tos $(BINDIR)ht1eneas.tos
	$(MV)	ht2ene.tos $(BINDIR)ht2eneas.tos
	$(MV)	ht3ene.tos $(BINDIR)ht3eneas.tos


enec:
	$(RM) *.o *.tos *.stx *.mif
	$(MAKE)	DEFS=-Wa,-defsym,BUS=1 EXTRADEFS=-Wa,-defsym,CPU020=0 drivers ht1ene.tos ht2ene.tos ht3ene.tos ht4ene.tos
	$(MV)	etherne.mif $(BINDIR)enec.mif
	$(MV)	etherne.stx $(BINDIR)enec.stx
	$(MV)	ht1ene.tos $(BINDIR)ht1enec.tos
	$(MV)	ht2ene.tos $(BINDIR)ht2enec.tos
	$(MV)	ht3ene.tos $(BINDIR)ht3enec.tos
	$(MV)	ht4ene.tos $(BINDIR)ht4enec.tos


enec3:
	$(RM) *.o *.tos *.stx *.mif
	$(MAKE)	DEFS=-Wa,-defsym,BUS=1 EXTRADEFS=-Wa,-defsym,CPU020=1 drivers
	$(MV)	etherne.mif $(BINDIR)enec3.mif
	$(MV)	etherne.stx $(BINDIR)enec3.stx


enem:
	$(RM) *.o *.tos *.stx *.mif
	$(MAKE)	DEFS=-Wa,-defsym,BUS=2 drivers ht2ene.tos
	$(MV)	etherne.mif $(BINDIR)enem.mif
	$(MV)	etherne.stx $(BINDIR)enem.stx
	$(MV)	ht2ene.tos $(BINDIR)ht2enem.tos


eneh:
	$(RM) *.o *.tos *.stx *.mif
	$(MAKE)	DEFS=-Wa,-defsym,BUS=3 drivers ht2ene.tos
	$(MV)	etherne.mif $(BINDIR)eneh.mif
	$(MV)	etherne.stx $(BINDIR)eneh.stx
	$(MV)	ht2ene.tos $(BINDIR)ht2eneh.tos


enep2:
	$(RM) *.o *.tos *.stx *.mif
	$(MAKE)	DEFS=-Wa,-defsym,BUS=6 drivers ht2ene.tos
	$(MV)	etherne.mif $(BINDIR)enep2.mif
	$(MV)	etherne.stx $(BINDIR)enep2.stx
	$(MV)	ht2ene.tos $(BINDIR)ht2enep2.tos


enex:
	$(RM) *.o *.tos *.stx *.mif
	$(MAKE)	DEFS=-Wa,-defsym,BUS=7 drivers ht2ene.tos
	$(MV)	etherne.mif $(BINDIR)enex.mif
	$(MV)	etherne.stx $(BINDIR)enex.stx
	$(MV)	ht2ene.tos $(BINDIR)ht2enex.tos


# development driver

development:
	$(RM) $(OBJS)
	$(CP) devswitd.i devswit.i	# replace the relevant include file
	$(MAKE)

# production driver

production:
	$(RM) $(OBJS)
	$(CP) devswitp.i devswit.i	# replace the relevant include file
	$(MAKE)


#### general stuff ##############################################

# order of file and option specific to DEVPAC assembler
.s.o:
	$(AS) -c $< $(AFLAGS) $(DEFS) $(EXTRADEFS)

.c.o:
	$(CC) -c $(CFLAGS) $<

#### end of makefile ############################################

