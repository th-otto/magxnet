#NO_APP
gcc2_compiled.:
___gnu_compiled_c:
.stabs "/home/extase/root/mintlib/48/lib/",100,0,0,Ltext0
.stabs "filbuf.c",100,0,0,Ltext0
.text
Ltext0:
.stabs "int:t1=r1;-2147483648;2147483647;",128,0,0,0
.stabs "char:t2=r2;0;127;",128,0,0,0
.stabs "long int:t3=r1;-2147483648;2147483647;",128,0,0,0
.stabs "unsigned int:t4=r1;0;-1;",128,0,0,0
.stabs "long unsigned int:t5=r1;0;-1;",128,0,0,0
.stabs "long long int:t6=r1;01000000000000000000000;0777777777777777777777;",128,0,0,0
.stabs "long long unsigned int:t7=r1;0000000000000;01777777777777777777777;",128,0,0,0
.stabs "short int:t8=r1;-32768;32767;",128,0,0,0
.stabs "short unsigned int:t9=r1;0;65535;",128,0,0,0
.stabs "signed char:t10=r1;-128;127;",128,0,0,0
.stabs "unsigned char:t11=r1;0;255;",128,0,0,0
.stabs "float:t12=r1;4;0;",128,0,0,0
.stabs "double:t13=r1;8;0;",128,0,0,0
.stabs "long double:t14=r1;8;0;",128,0,0,0
.stabs "complex int:t15=s8real:1,0,32;imag:1,32,32;;",128,0,0,0
.stabs "complex float:t16=r16;4;0;",128,0,0,0
.stabs "complex double:t17=r17;8;0;",128,0,0,0
.stabs "complex long double:t18=r18;8;0;",128,0,0,0
.stabs "void:t19=19",128,0,0,0
.stabs "size_t:t5",128,0,20,0
.stabs "FILE:t20=s26_cnt:3,0,32;_ptr:21=*11,32,32;\\",128,0,0,0
.stabs "_base:21,64,32;_flag:4,96,32;_file:1,128,32;\\",128,0,0,0
.stabs "_bsiz:3,160,32;_ch:11,192,8;;",128,0,79,0
.stabs "fpos_t:t5",128,0,82,0
.stabs "wchar_t:t9",128,0,23,0
.stabs "div_t:t22=s8quot:1,0,32;rem:1,32,32;;",128,0,47,0
.stabs "ldiv_t:t23=s8quot:3,0,32;rem:3,32,32;;",128,0,52,0
.stabs "time_t:t3",128,0,28,0
.stabs "clock_t:t5",128,0,36,0
.stabs "tm:T24=s44tm_sec:1,0,32;tm_min:1,32,32;\\",128,0,0,0
.stabs "tm_hour:1,64,32;tm_mday:1,96,32;tm_mon:1,128,32;\\",128,0,0,0
.stabs "tm_year:1,160,32;tm_wday:1,192,32;tm_yday:1,224,32;\\",128,0,0,0
.stabs "tm_isdst:1,256,32;tm_zone:25=*2,288,32;tm_gmtoff:3,320,32;;",128,0,0,0
.stabs "timeval:T26=s8tv_sec:3,0,32;tv_usec:3,32,32;;",128,0,0,0
.stabs "timezone:T27=s8tz_minuteswest:1,0,32;tz_dsttime:1,32,32;;",128,0,0,0
.stabs "itimerval:T28=s16it_interval:26,0,64;it_value:26,64,64;;",128,0,0,0
.stabs "fd_set:t5",128,0,116,0
.stabs "fnmapfunc_t:t29=*30=f1",128,0,17,0
.stabs "mem_chunk:T31=s12valid:3,0,32;next:32=*31,32,32;\\",128,0,0,0
.stabs "size:5,64,32;;",128,0,0,0
.stabs "__open_file:T33=s4status:8,0,16;flags:8,16,16;;",128,0,0,0
	.even
.globl __filbuf
__filbuf:
	.stabd 68,0,22
	movel a2,sp@-
	movel d2,sp@-
	movel sp@(12),a2
	.stabd 68,0,23
LBB2:
	.stabd 68,0,26
	movel a2@(12),d2
	.stabd 68,0,27
	tstb d2
	jge L2
	moveq #1,d0
	orl d2,d0
	movel d0,a2@(12)
	movel d0,d2
L2:
	.stabd 68,0,28
	movel d2,d0
	andl #20481,d0
	moveq #1,d1
	cmpl d0,d1
	jeq L3
	.stabd 68,0,29
	moveq #-1,d0
	jra L9
L3:
	.stabd 68,0,32
	lea a4@(__iob:w),a1
	movel a1,d0
	cmpl a2,d0
	jne L4
	btst #3,d2
	jeq L4
	lea a2@(26),a0
	btst #1,a2@(40)
	jeq L4
	.stabd 68,0,33
	movel a0,sp@-
	jbsr _fflush
	addqw #4,sp
L4:
	.stabd 68,0,35
	movel a2@(8),a2@(4)
	.stabd 68,0,36
	movel a2@(20),sp@-
	movel a2@(8),sp@-
	movel a2@(16),sp@-
	jbsr __read
	movel d0,a0
	addqw #8,sp
	addqw #4,sp
	tstl a0
	jgt L5
	.stabd 68,0,38
	movel a2@(12),d0
	tstl a0
	jne L6
	movel d0,a2@(12)
	btst #3,d2
	jne L7
	orw #4096,a2@(14)
	jra L7
L6:
	orw #16384,d0
	movel d0,a2@(12)
L7:
	.stabd 68,0,39
	clrl a2@
	.stabd 68,0,40
	moveq #-1,d0
	jra L9
	.stabd 68,0,41
L5:
	.stabd 68,0,42
	subqw #1,a0
	movel a0,a2@
	.stabd 68,0,43
	movel a2@(4),a0
	clrl d0
	moveb a0@,d0
	addql #1,a2@(4)
L9:
	.stabd 68,0,44
LBE2:
	movel sp@+,d2
	movel sp@+,a2
	rts
.stabs "_filbuf:F1",36,0,21,__filbuf
.stabs "fp:p34=*20",160,0,21,12
.stabs "fp:r34",64,0,21,10
.stabs "f:r4",64,0,23,2
.stabs "got:r3",64,0,24,8
.stabn 192,0,0,LBB2
.stabn 224,0,0,LBE2
