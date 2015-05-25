$! This procedure rebuilds the DMQA SERVER from source. To run this,
$! You need to have access to the P_DMQA.H file, and also you must be in
$! a valid DMQ group so the DMQ$LIB logical functions. It also assumes UCX
$! is on the system.
$
$ DEBUG_C = ""
$ DEBUG_L = ""
$ IF P1 .NES. "" THEN DEBUG_C = "/DEB/NOOPT"
$ IF P1 .NES. "" THEN DEBUG_L = "/DEB"
$ COMMON_LOC = ""
$ IF F$SEARCH("''COMMON_LOC'DMQIN.C") .EQS. "" THEN -
	COMMON_LOC = "[-.CLIENT]"
$ assign sys$library:vaxcrtl lnk$library
$
$ IF F$SEARCH("SYS$LIBRARY:UCX$IPC.OLB") .NES. "" THEN GOTO TEST_DMQ
$ WRITE SYS$OUTPUT "UCX has not been installed on this system"
$ GOTO EXIT
$
$ TEST_DMQ:
$ IF F$TRNLNM("DMQ$LIB") .NES. "" THEN GOTO DO_IT
$ WRITE SYS$OUTPUT "?DMQ$LIB logical is undefined"
$ WRITE SYS$OUTPUT ""
$ WRITE SYS$OUTPUT "Please select a valid DECmessageQ BUS and GROUP"
$ WRITE SYS$OUTPUT "before executing this procedure."
$ GOTO EXIT
$
$ DO_IT:
$ if f$search("[-.client]p_dmqa.h") .nes. "" then -
	copy [-.client]p_dmqa.h []/lo
$ CC/standard=portable 'COMMON_LOC'DMQPP
$ CC/standard=portable 'COMMON_LOC'DMQIN
$ if f$search("dmqa_shutdown.c") .nes. "" then -
	CC/standard=portable DMQA_SHUTDOWN
$ CC/standard=portable'DEBUG_C' DMQA_SERVER+DMQ$LIB:DMQ.TLB/LIB
$ CC/standard=portable'DEBUG_C' 'COMMON_LOC'DMQA_TSO+DMQ$LIB:DMQ.TLB/LIB
$ if f$search("DMQA_IPI.C") .nes. "" then -
	CC/standard=portable'DEBUG_C' DMQA_IPI+DMQ$LIB:DMQ.TLB/LIB
$ LINK DMQA_SHUTDOWN
$ IF P1 .EQS. "" THEN -
  LINK DMQA_SERVER+DMQA_IPI+DMQA_TSO+DMQPP+DMQIN+SYS$LIBRARY:UCX$IPC.OLB/LIB+DMQ$LIB:DMQ.OPT/OPT
$ IF P1 .NES. "" THEN -
  LINK'DEBUG_L'/EXE=DMQA_DEBUG DMQA_SERVER+DMQA_IPI+DMQA_TSO+DMQPP+DMQIN+SYS$LIBRARY:UCX$IPC.OLB/LIB+DMQ$LIB:DMQ.OPT/OPT
$
$ EXIT:
$   WRITE SYS$OUTPUT ""
$   exit
