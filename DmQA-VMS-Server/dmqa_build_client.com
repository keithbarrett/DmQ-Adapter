$! This procedure builds the DMQA CLIENT programs on VMS. To run this,
$! You need to have access to the P_DMQA.H file, and also you must be in
$! a valid DMQ group so the DMQ$LIB logical functions. The procedure assumes
$! UCX is available.
$
$ DEBUG_C = ""
$ DEBUG_L = ""
$ IF P1 .NES. "" THEN DEBUG_C = "/DEB/NOOPT"
$ IF P1 .NES. "" THEN DEBUG_L = "/DEB"
$ assign sys$library:vaxcrtl lnk$library
$
$ CC/standard=portable DMQPP
$ CC/standard=portable DMQIN
$ CC/standard=portable'DEBUG_C' DMQA_TSO+DMQ$LIB:DMQ.TLB/LIB
$ CC/standard=portable'DEBUG_C' DMQA_API+DMQ$LIB:DMQ.TLB/LIB
$
$! BUILD TEST PROGRAMS ON VMS
$
$ CC/standard=portable'DEBUG_C' DMQA_API+DMQ$LIB:DMQ.TLB/LIB
$ CC/standard=portable'DEBUG_C' DMQATEST+DMQ$LIB:DMQ.TLB/LIB
$ if f$search("bounce.c") .nes. "" then -
	CC/standard=portable/define=("QADAPTER_TEST 1")'DEBUG_C' -
	BOUNCE+DMQ$LIB:DMQ.TLB/LIB
$ if f$search("roundtrip.c") .nes. "" then -
	CC/standard=portable/define=("QADAPTER_TEST 1")'DEBUG_C' -
	ROUNDTRIP+DMQ$LIB:DMQ.TLB/LIB
$ LINK'DEBUG_L' -
	 DMQATEST+DMQA_API+DMQA_TSO+DMQPP+DMQIN+SYS$LIBRARY:UCX$IPC.OLB/LIB
$ if f$search("bounce.obj") .nes. "" then -
	LINK'DEBUG_L' -
	BOUNCE+DMQA_API+DMQA_TSO+DMQPP+DMQIN+SYS$LIBRARY:UCX$IPC.OLB/LIB
$ if f$search("roundtrip.obj") .nes. "" then -
	LINK'DEBUG_L' -
	ROUNDTRIP+DMQA_API+DMQA_TSO+DMQPP+DMQIN+SYS$LIBRARY:UCX$IPC.OLB/LIB
$ EXIT
