*      PCSTART.S
*
*      Pure C Startup Code
*
*      Copyright (c) Borland International 1988/89/90
*      All Rights Reserved.


*>>>>>> Export references <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

        .EXPORT exit, __exit

        .EXPORT _app
        .EXPORT errno
        .xref _AtExitVec
        .xref _FilSysVec
        .EXPORT _RedirTab
        .EXPORT _StkLim
        .EXPORT _PgmSize

        .EXPORT __text, __data, __bss

*>>>>>> Import references <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

        .IMPORT main
        .IMPORT _fpuinit
        .IMPORT _StkSize
        .IMPORT _FreeAll
		.xref _fpumode



*>>>>>> Data structures <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


* Base page structure

        .OFFSET 0

TpaStart:
        .DS.L   1
TpaEnd:
        .DS.L   1
TextSegStart:
        .DS.L   1
TextSegSize:
        .DS.L   1
DataSegStart:
        .DS.L   1
DataSegSize:
        .DS.L   1
BssSegStart:
        .DS.L   1
BssSegSize:
        .DS.L   1
DtaPtr:
        .DS.L   1
PntPrcPtr:
        .DS.L   1
Reserved0:
        .DS.L   1
EnvStrPtr:
        .DS.L   1
Reserved1:
        .DS.B   7
CurDrv:
        .DS.B   1
Reserved2:
        .DS.L   18
CmdLine:
        .DS.B   128
BasePageSize:
        .DS     0



*>>>>>>> Code segment <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

        .CODE
__text:


******** PcStart ********************************************************

Start:
        BRA.B   Start0



******* Configuration data


* Redirection array pointer

        .DC.L   _RedirTab


* Stack size entry

        .DC.L   _StkSize



******* Pc library copyright note

        .ALIGN  16

        .ascii  "Pure C"
		.dc.w 0
		.dc.w 0
		.dc.w 0
		.dc.w 0
EmpStr:
		.dc.w 0



******** Pc startup code

* Setup pointer to base page

Start0:
        MOVE.L  A0, D0
        BNE     ACC

        MOVE.L  4(A7), A3   ; BasePagePointer from Stack
        MOVEQ.L #1, D0      ; Program is Application
        BRA     APP
ACC:
		move.l  a0,a3
        CLR.W   D0          ; Program is DeskAccessory

APP:

        MOVE.L  A3, _BasPag

* Setup applikation flag

        MOVE.W  D0,_app


* Compute size of required memory
* := text segment size + data segment size + bss segment size
*  + stack size + base page size
* (base page size includes stack size)

        MOVE.L  TextSegSize(A3),d1
        ADD.L   DataSegSize(A3),d1
        ADD.L   BssSegSize(A3),d1
        ADD.l   #BasePageSize,d1
        MOVE.L  d1, _PgmSize

* Setup longword aligned application stack

		move.l d1,d2
        add.L  A3,D2
        AND.B   #$FC,D2
        MOVE.L  D2,A7

* check application flag

        TST.W   d0
        BEQ     Start11  * No environment and no arguments

* Free not required memory

        MOVE.L  d1,-(A7)
        MOVE.L  A3,-(A7)
        clr.w   -(A7)
        MOVE.W  #74,-(A7)
        TRAP    #1
        LEA.L   12(A7),A7

* scan environment
		move.l  a7,d0
        SUB.L   #_StkSize-4, D0
        AND.B   #$FC, D0
        MOVE.L  D0, A1
        MOVE.L  A1, A4
        MOVE.L  EnvStrPtr(A3), A2
ScanEnvLoop:
        MOVE.L  A2, (A1)+
        move.l  a2,a5
        TST.B   (A2)+
        beq     ScanEnvExit
Start1:
        TST.B   (A2)+
        BNE     Start1
        movep.w    0(a5),d0
        swap d0
        movep.w    1(a5),d0
        cmpi.l     #'AGRV',d0
		BNE		ScanEnvLoop
		cmpi.b     #'=',4(a5)
		BNE		ScanEnvLoop
		cmpi.b     #127,128(a3)
		BNE		ScanEnvLoop
		* now we have found extended arguments
		CLR.B	(A5)
		CLR.L	-4(A1)
		MOVE.L	A1, A5			* this is argv
		moveq	#0,D3				* this is argc		
		MOVE.L	A2, (A1)+
xArgLoop:
		TST.B	(A2)+
		BNE		xArgLoop
        MOVE.L  A2, (A1)+
		ADDQ.W	#1, D3
        TST.B   (A2)
        BNE     xArgLoop
		BRA		Start10			* we don't need to parse basepage's tail
ScanEnvExit:
        CLR.L   -4(A1)

* scan commandline
        LEA     CmdLine(A3), A0
        MOVE.B  (A0), D1
        EXT.W   D1
		move.l a1,a5
		move.l     a0,(a1)+
		clr.b   (a0)+
		
vallidLength:
        MOVEQ   #1, D3
		move.l  a0,(a1)+
		moveq.l    #' ',d4
		moveq.l    #$27,d5
		moveq.l    #$22,d6
        BRA     Start9

Start2:                         * testing blank (seperator)
		move.b (a0)+,d0
        CMP.B   d4,d0
        BHI     Start21
        tst.b   -2(a0)
		bne.s      Start7
		addq.l     #1,-4(a1)
		bra.s      Start8
Start21:
		cmp.b      d5,d0
		bne.s      Start5
		move.l     -4(a1),d0
		addq.l     #1,d0
		cmpa.l     d0,a0
		bne.s      Start5
		addq.l     #1,-4(a1)
Start3:
		cmp.b      (a0)+,d5
		dbeq       d1,Start3
		subq.w     #1,d1
		bmi.s      Start10
		beq.s      Start7
		cmp.b      (a0),d5
		bne.s      Start7
		movea.l    -(a1),a2
		movea.l    a0,a3
Start4:
		move.b     -2(a3),-(a3)
		cmpa.l     a2,a3
		bhi.s      Start4
		addq.l     #1,a0
		subq.w     #1,d1
		addq.l     #1,(a1)+
		bra.s      Start3
Start5:
		cmp.b      d6,d0
		bne.s      Start9
		addq.l     #1,-4(a1)
Start6:
		cmp.b      (a0)+,d6
		dbeq       d1,Start6
		subq.w     #1,d1
		bmi.s      Start10
Start7:
		move.l     a0,(a1)+
		addq.w     #1,d3
Start8:
		clr.b      -1(a0)
Start9:
		subq.w     #1,d1
		bpl.s      Start2
		tst.b      -1(a0)
		beq.s      Start10
		addq.w     #1,d3
		clr.b      (a0)
		addq.l     #4,a1
Start10:
		movea.l    a1,a6
		clr.l      -(a1)
Start11:
		clr.w      _fpumode
		lea.l      256(a6),a6
		move.l     a6,_StkLim

Start12:
        ;JSR     _fpuinit
		clr.w errno
		clr.l _AtExitVec
		clr.l _FilSysVec
		
******* Execute main program *******************************************
*
* Parameter passing:
*   <D0.W> = Command line argument count (argc)
*   <A0.L> = Pointer to command line argument pointer array (argv)
*   <A1.L> = Pointer to tos environment string (env)

        MOVE    D3, D0
        MOVE.L  A5, A0
        MOVE.L  A4, A1
        JSR     main



******** exit ***********************************************************
*
* Terminate program
*
* Entry parameters:
*   <D0.W> = Termination status : Integer
* Return parameters:
*   Never returns

exit:
        MOVE.W  D0,-(A7)

* Execute all registered atexit procedures

        MOVE.L  _AtExitVec,D0
        BEQ     __exit

        MOVE.L  D0,A0
        JSR     (A0)


* Deinitialize file system

__exit:
        MOVE.L  _FilSysVec,D0
        BEQ     Exit1

        MOVE.L  D0,A0
        JSR     (A0)


* Deallocate all heap blocks

Exit1:
        JSR     _FreeAll


* Program termination with return code

        MOVE.W  #76,-(A7)
        TRAP    #1


*>>>>>>> Data segment <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

        .BSS
__bss:

* Pointer to base page

        .globl _BasPag
_BasPag:
		.globl _base
_base:
        .DS.L   1


* Applikation flag

_app:
        .DS.W   1


* Stack limit

_StkLim:
        .DS.L   1

* Program size

_PgmSize:
        .DS.L   1

* Redirection address table

_RedirTab:
        .DS.L   6

* Global error variable

errno:
        .DS.W   1


* Vector for atexit

_AtExitVec:
        .DS.L   1


* Vector for file system deinitialization

_FilSysVec:
        .DS.L   1
