*********************************************************************************
* Bus access macros for ST/TT/Falcon Cartridge Port for register base hardware	*
* Version for Lyndon Amsdon's and mine NE2000 interface hardware.        	*
*										*
*	Copyright 2002 Dr. Thomas Redelberger					*
*	Use it under the terms of the GNU General Public License		*
*	(See file COPYING.TXT)							*
*										*
* Tabsize 8, developed with DEVPAC assembler 2.0.				*
*										*
*********************************************************************************

BUS_ACSI              = 0
BUS_CARTRIDGE         = 1
BUS_ISA_MILAN         = 2
BUS_ISA_HADES_ET4000  = 3
BUS_ISA_HADES_TSENG   = 4
BUS_ISA_HADES_MACH64  = 5
BUS_ISA_HADES_MACH32  = 6
BUS_FALCON_EXPANSION  = 7


	IFEQ BUS-BUS_ACSI
	.INCLUDE "busenea.i"
	ENDC

	IFEQ BUS-BUS_CARTRIDGE
	.INCLUDE "busenec.i"
	ENDC

	IFEQ BUS-BUS_ISA_MILAN
	.INCLUDE "busenem.i"
	ENDC

	IFEQ BUS-BUS_ISA_HADES_ET4000
	.INCLUDE "buseneh.i"
	ENDC

	IFEQ BUS-BUS_ISA_HADES_TSENG
	.INCLUDE "buseneh.i"
	ENDC

	IFEQ BUS-BUS_ISA_HADES_MACH32
	.INCLUDE "buseneh.i"
	ENDC

	IFEQ BUS-BUS_ISA_HADES_MACH64
	.INCLUDE "buseneh.i"
	ENDC

	IFEQ BUS-BUS_FALCON_EXPANSION
	.INCLUDE "busenex.i"
	ENDC
