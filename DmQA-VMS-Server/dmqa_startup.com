$  SET NOVERIFY
$! DMQA_STARTUP.COM
$!
$!*****************************************************************************
$!*									      *
$!*  COPYRIGHT (c) 1990, 1991 BY			      	              *
$!*  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.		      *
$!*  ALL RIGHTS RESERVED.						      *
$!* 									      *
$!*  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED    *
$!*  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE    *
$!*  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER    *
$!*  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY    *
$!*  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY    *
$!*  TRANSFERRED.							      *
$!* 									      *
$!*  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE    *
$!*  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT    *
$!*  CORPORATION.							      *
$!* 									      *
$!*  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS    *
$!*  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.		      *
$!* 									      *
$!*									      *
$!*****************************************************************************
$!
$! This command procedure starts the VMS DECmessageQ Queue Adapter Server for
$! the specified port. It requires some privileges.
$!
$! P1 = [optional] Program name (defaults to DMQA_SERVER.EXE)
$! P2 = [optional] user to send mail to if adapter fails
$! P3 = [optional] output (defaults to NL:). Also;
$!	if specified as "INTERACTIVE", then the server will run
$!	interactively w/o a logfile instead of detached, and it will NOT
$!	create offspring.
$! P4 = [optional] value for DMQ$DEBUG
$! P5 - P8 are passed to the program startup as P2 - P5 (P1 is 'DMQA')
$!
$!
$! Revisions:
$!
$!	23-Apr-1991	kgb	Initial release; Keith Barrett
$!	24-Apr-1991	kgb	Improved mail message logic
$!	06-May-1991	kgb	Changed privs needed
$!	07-May-1991	kgb	Added INTERACTIVE indication
$!	08-May-1991	kgb	INTERACTIVE now supresses offspring.
$!	22-Jul-1991	kgb	Added test for DNS$ images
$
$
$	if f$trnlnm("DMQA$DEBUG") .nes. "" then set verify
$	on control_y then goto abort
$	on error then goto abort
$	privs_needed = -
	"CMKRNL,DETACH,EXQUOTA,ALTPRI,OPER,NETMBX,TMPMBX,BYPASS"
$	old_privs	= f$setprv("''privs_needed'")
$	bell[0,7]	= 7
$	say		= "write sys$output"
$	output		= "write outfile"
$	cur_dir		= f$environment("DEFAULT")
$	pid		= f$getjpi("","pid")
$	tmp_file	= "tmp''pid'.tmp"
$	this_procedure	= f$environment("PROCEDURE")
$	dmqa_server	= f$parse("''P1'", "''procedure'",, "DEVICE") + -
			  f$parse("''P1'", "''procedure'",, "DIRECTORY") + -
			  f$parse("''P1'", "DMQA_SERVER.EXE",, "NAME") +-
			  f$parse("''P1'", "DMQA_SERVER.EXE",, "TYPE")
$	bus_group	= f$trnlnm("DMQ$BUS_GROUP")
$	uic		= f$user()
$	owner_process	= ""
$	debug		= 0
$	param_prefix	= "dmqa"
$	node		= f$getsyi("SCSNODE") - ":" - ":"
$	node		= f$edit(node, "COLLAPSE")
$	priority	= "/priority=5"
$	interactive_flag= 0
$	efc_name	= "DMQA_''p5'_EFC"
$	if p7 .nes. "" then efc_name = p7
$	if "''dmqa$debug'" .nes. "" then debug = 1
$
$	say ""
$	say "DMQA_STARTUP - DECmessageQ Queue Adapter Startup Procedure"
$	say ""
$
$	if .not. f$privilege(privs_needed) then goto no_privs
$
$	if .not. f$file_attributes("SYS$LIBRARY:DNS$SHARE.EXE","KNOWN") then -
		goto no_images
$	if .not. f$file_attributes("SYS$LIBRARY:DNS$RTL.EXE","KNOWN") then -
		goto no_images
$
$	if debug then say "%DEBUG is ENABLED!"
$
$	if "''p1'''p2'''p3'''p4'''p5'''p6'''p7'''p8'" .nes. "" -
		then goto test_filename
$	say ""
$	say "This command procedure starts the VMS DECmessageQ Queue Adapter Server."
$	say ""
$	say "Parameters:"
$	say "	P1 = [optional] Program name (defaults to DMQA_SERVER.EXE)"
$	say "	P2 = [optional] user to send mail to if the server fails"
$	say "	     (default is to send a message to OPA0:)"
$	say "	P3 = [optional] logfile output (default is no logfile)"
$	say "	     If specified as INTERACTIVE, then the server will"
$	say "	     run interactively instead of detached, and will NOT"
$	say "	     create offspring."
$	say "	P4 = [optional] value for DMQ$DEBUG"
$	say ""
$	say "	P5 - P8 are passed to the server program as P2 - P5"
$	say "	(P1 will be set to 'DMQA'). These parameters vary, depending"
$	say "	on the communications platform being used."
$	say ""
$	say "	For TCP/IP Communications:"
$	say "		P5 	= port number to use (1 - 65535)"
$	say "		P6 	= debug mode"
$	say "		P7 - P8 = <reserved for internal use>"
$	say ""
$	say "		Example of TCP/IP Server startup for port 8192:"
$	say ""
$	say "		@DMQA_STARTUP """" ""user"" """" """" 8192 "
$	goto exit
$
$ test_filename:
$	if f$search("''dmqa_server'") .nes. "" then goto test_dmq
$	say ""
$	say "?Image not found - ''dmqa_server'"
$	goto exit
$
$ test_dmq:
$	if bus_group .nes. "" then goto process_bus_group
$	say ""
$	say "?You are not a member of a valid DECmessageQ BUS and group"
$	goto exit
$
$ process_bus_group:
$	bus	= f$element(0, "_", bus_group)
$	group	= f$element(1, "_", bus_group)
$	dmq_dir	= f$trnlnm("DMQ$EXE") - "DMQ$DISK:"
$	dmq_disk= f$trnlnm("DMQ$DISK")
$	dmq_loc	= f$parse("''dmq_dir'", "''dmq_disk'",, "DEVICE") + -
		  f$parse("''dmq_dir'", "''dmq_disk'",, "DIRECTORY")
$
$ test_params:
$	if p3 .nes. "INTERACTIVE" then goto test_more
$	interactive_flag = -1
$	p3 = "DMQA_SERVER.LOG"
$	param_prefix = "dmqa_o"
$	if p8 .eqs. "" then p8 = "64"
$
$ test_more:
$	if P3 .eqs. "" then logfile = "NL:"
$	if "''P5'''P6'''P7'''P8'" .nes. "" then goto begin
$
$	say ""
$	say "?No P5 to P8 parameters specified"
$	goto exit
$
$
$ begin:
$	process_name = "DmQA_S_" + P5
$ 	process_name = f$extract(0, 15, process_name)
$
$	open/write	outfile	'tmp_file'
$	output  "$ set noverify"
$	output	"$ set noon"
$	output	"$ set proc/priv=(all)
$	output	"$ @''dmq_loc'dmq$set_lnm_table ''bus' ''group'"
$	output  "$ set verify"
$	output	"$ DMQA_BUSGROUP == ""$@''dmq_loc'DMQ$SET_LNM_TABLE ''bus' ''group'"""
$	output	"$ DMQA_DEBUG == ""$!"""
$	if p4 .nes. "" then -
$		output	"$ DMQA_DEBUG == ""$DEFINE DMQ$DEBUG """"''p4'"""""
$	output	"$ DMQA_DIR == ""''cur_dir'"""
$	output	"$ DMQA_EXE == ""$server = """"$''dmqa_server'"""""
$
$	index = 0
$ logfile_loop:
$	index = index + 1
$	if index .gt. 15 then goto logfile_loop_exit
$	if p3 .nes. "" .and. p3 .nes. "NL:" then goto process_log
$	output	"$ DMQA_LOG''index' == ""NL:"""
$	goto logfile_loop
$
$ process_log:
$	tmp = index + 64
$	tmp_str = ""
$	tmp_str[0,8] = tmp
$	suffix = "_''p5'''tmp_str'"
$	tmpfile	= f$parse("''P3'", "''cur_dir'DMQA_SERVER.LOG",, "DEVICE") + -
		  f$parse("''P3'", "''cur_dir'DMQA_SERVER.LOG",, "DIRECTORY") + -
		  f$parse("''P3'", "''cur_dir'DMQA_SERVER.LOG",, "NAME") + -
		  suffix + -
		  f$parse("''P3'", "''cur_dir'DMQA_SERVER.LOG",, "TYPE")
$	output	"$ DMQA_LOG''index' == ""''tmpfile'"""
$	goto logfile_loop
$
$ logfile_loop_exit:
$	if p3 .nes. "" .and. p3 .nes. "NL:" then -
	logfile	= f$parse("''P3'", "''cur_dir'DMQA_SERVER.LOG",, "DEVICE") + -
		  f$parse("''P3'", "''cur_dir'DMQA_SERVER.LOG",, "DIRECTORY") + -
		  f$parse("''P3'", "''cur_dir'DMQA_SERVER.LOG",, "NAME") + -
		  "_''p5'" + -
		  f$parse("''P3'", "''cur_dir'DMQA_SERVER.LOG",, "TYPE")
$	output "$ server == ""$''dmqa_server'"""
$	if "''P4'" .nes. "" then -
		output "$ define DMQ$DEBUG ''P4'"
$	output	"$ server ''param_prefix' ""''p5'"" ""''p6'"" ""''efc_name'"" ""''p8'"""
$
$	if P2 .eqs. ""  then goto use_reply
$	output "$copy sys$input mail.tmp"
$	output ""
$	output "The DECmessageQ Queue Adapter Server ''process_name' has"
$	output "unexpectedly terminated."
$	output ""
$	if (logfile - "NL:" .eqs. "") .and. (interactive_flag .eq. 0) then -
	output "The server was started without a log file - no further information exists."
$	if logfile - "NL:" .nes. "" then -
	output "The server's logfile (''logfile') should contain more information."
$	output ""
$	output "The server will have to be re-started to allow further"
$	output "connections on that link."
$	output ""
$	output "The command line used was:"
$	output ""
$	if node .nes. "" then output "(on node ''node'::)"
$	output  -
	"	@''this_procedure' ''dmqa_server' ''p2' ''logfile' ""''p4'"" ""''p5'""  ""''p6'"" ""''p7'"" ""''p8'"""
$	output ""
$	output "$EOD"
$	output -
	"$ mail/noedit/subj=""DmQ Queue Adapter Server ''process_name' failed"" mail.tmp ''p2'"
$	output "$delete mail.tmp;/lo"
$	goto continue
$
$ use_reply:
$	if (p2 .eqs. "") .and. (interactive_flag .eq. 0) then output -
	"$ repl/bell/term=opa0: ""DECmessageQ Queue Adapter Server ''process_name' has failed"""
$
$ continue:
$	output	"$ set noverify"
$	output	"$ exit"
$	close outfile
$
$	say "Starting DECmessageQ Queue Adapter Server at ''f$time()'"
$	say ""
$	say "          Image: ''DMQA_SERVER'"
$	say "        Process: ''process_name'"
$	say "   DmQ location: ''dmq_loc'"
$	say "            Bus: ''bus'"
$	say "          Group: ''group'"
$	if p4 .nes. "" then say "      DMQ$DEBUG: ''P4'"
$	if p4 .eqs. "" then say "      DMQ$DEBUG: <none>"
$	if interactive_flag .eq. 0 then say "         Output: ''logfile'"
$	if interactive_flag .ne. 0 then say "         Output: INTERACTIVE"
$	say "         Params: ''param_prefix', ''p5', ''p6', ''p7', ''p8'"
$	say ""
$
$ ask_user:
$	if f$environment("DEPTH") .gt. 1 then goto no_ask
$	inquire tmp_str "Ok to start this server (Yes/No) <No>? "
$	tmp_str = f$extract(0, 1, f$edit(tmp_str,"UPCASE,COLLAPSE"))
$	if tmp_str .eqs. "" then tmp_str = "N"
$	if tmp_str .eqs. "N" then goto abort
$	if tmp_str .nes. "Y" then goto ask_user
$	say ""
$
$ no_ask:
$	if interactive_flag then goto start_interactive
$	if f$file_attributes("''dmqa_server'","KNOWN") then goto do_it
$	say "%Installing ''dmqa_server' image..."
$	install = "$sys$system:install/command_mode"
$	install add 'dmqa_server'/open/head
$	say ""
$
$ do_it:
$	if interactive_flag then goto start_interactive
$	set noon
$	run/detach/uic='uic'/proc="''process_name'" -
     		/input='cur_dir''tmp_file' -
     		/output='logfile' 'priority' sys$system:loginout 
$	set on
$
$	wait 00:00:10	! Give startup a chance to mail us a failure message
$	goto rundown
$
$ start_interactive:
$	@'tmp_file'
$	set noverify
$
$ rundown:
$	if debug .eq. 0 then delete 'tmp_file';*
$
$	say ""
$	say "Process completed at ''f$time()'"
$
$	goto exit
$
$ no_images:
$	say ""
$	say "?This bus/group is not running (DNS$SHARE is not installed)"
$	goto abort
$
$ no_privs:
$	say ""
$	say "?This program needs the following privileges:"
$	say ""
$	say "''privs_needed'"
$	say ""
$
$ abort:
$	say ""
$	say "?Aborting..."
$
$ exit:
$	old_privs = f$privilege(old_privs)
$	say ""
$ 	EXIT
