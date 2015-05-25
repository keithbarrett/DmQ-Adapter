$! DMQA_SHUTDOWN.COM
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
$! the specified port. It may require privileges to examine and delete
$! processes.
$!
$! P1 = [optional] "link" number to remove , or "ALL" for all
$!
$! Revisions:
$!
$!	25-Apr-1991	kgb	Initial release; Keith Barrett
$!	16-May-1991	kgb	Cleaned up file deleteion logic
$!	23-May-1991	kgb	Added P2 parameter
$!	01-Jun-1991	kgb	Check for privs
$!
$!
$
$	bell[0,7]	= 7
$	say		= "write sys$output"
$	output		= "write outfile"
$	privs_needed	= "BYPASS,CMKRNL"
$	old_privs	= f$setprv("''privs_needed'")
$	cur_dir		= f$environment("DEFAULT")
$	debug		= 0
$	node		= f$getsyi("SCSNODE") - ":" - ":"
$	node		= f$edit(node, "COLLAPSE")
$	our_pid		= f$getjpi("","PID")
$	if "''dmqa$debug'" .nes. "" then debug = 1
$	procedure	= f$environment("PROCEDURE")
$	shutdown_prog =	-
		f$parse("DMQA_SHUTDOWN.EXE", "''procedure'",, "DEVICE") + -
		f$parse("DMQA_SHUTDOWN.EXE", "''procedure'",, "DIRECTORY") + -
		f$parse("DMQA_SHUTDOWN.EXE", "''procedure'",, "NAME") + -
		f$parse("DMQA_SHUTDOWN.EXE", "''procedure'",, "TYPE")
$	dmqa_shutdown_efn = ""
$	if f$search("''shutdown_prog'") .nes. "" then -
		dmqa_shutdown_efn = "$" + shutdown_prog
$
$	proc_count	= 0
$	server_name1	= ""
$	pid_list1	= ""
$	index		= 0
$	install = "$sys$system:install/command_mode"
$	image_removed	= 0
$
$
$	say ""
$	say "DMQA_SHUTDOWN - DECmessageQ Queue Adapter Shutdown Procedure"
$	say ""
$
$	if .not. f$privilege(privs_needed) then goto no_privs
$	if debug then say "%DEBUG is ENABLED!"
$	p1 = f$edit("''p1'", "UPCASE,COLLAPSE")
$	if (p1 .nes. "HELP") .and. (p1 .nes. "") then goto begin
$
$	say ""
$	say "This procedure will shutdown the DECmessageQ Queue Adapter Servers"
$	say ""
$	say "	Parameters:"
$	say ""
$	say "	P1	[optional] link to bring down, or "ALL" for all."
$	say "	P2	[optional] if "UNINSTALL", then the program image"
$	say "		will be un-installed."
$	say ""
$	goto exit
$
$
$ begin:
$	context = ""
$
$ pid_loop:
$	pid = f$pid(context)
$	if pid .eqs. "" then goto end_pid_loop
$	server_name = f$edit(f$getjpi(pid,"PRCNAM"), "TRIM")
$	work_server_name = f$edit(server_name, "UPCASE")
$	if (f$extract(0, 7, work_server_name) .nes. "DMQA_S_") .and. -
	   (f$extract(0, 7, work_server_name) .nes. "DMQA_O_") then goto pid_loop
$	proc_count = proc_count + 1
$	server_name'proc_count' = server_name
$	pid_list'proc_count'	= pid
$	goto pid_loop
$
$ end_pid_loop:
$	if proc_count .gt. 0 then goto sort_servers
$	say ""
$	say "?There are no DmQA Servers running on this node"
$	say ""
$	goto exit
$
$ sort_servers:
$	index1 = 0
$
$ sort_loop:
$	index1 = index1 + 1
$	if index1 .gt. proc_count then goto end_sort_loop
$	index2 = index1
$
$ sub_loop:
$	index2 = index2 + 1
$	if index2 .gt. proc_count then goto sort_loop
$	item1 = server_name'index1'
$	item2 = server_name'index2'
$	if f$extract(7, 8, item1) .les. f$extract(7, 8, item2) then -
		goto sub_loop
$	server_name'index2' = item1
$	server_name'index1' = item2
$	tmp_pid = pid_list'index2'
$	pid_list'index2' = pid_list'index1'
$	pid_list'index1' = tmp_pid
$	goto sub_loop
$
$ end_sort_loop:
$	say ""
$	say "The following servers are running at ''f$time()'"
$	say ""
$
$	index = 0
$
$ display_loop:
$	index = index + 1
$	if index .gt. proc_count then goto end_display_loop
$	item = server_name'index'
$	pid  = pid_list'index'
$	type = f$edit(f$extract(5, 1, item), "UPCASE")
$	desc = "(server)"
$	if type .eqs. "O" then desc = "(offspring)"
$	say "   ''pid'	''item'	''desc'"
$	goto display_loop
$
$ end_display_loop:
$	say ""
$	say ""
$	if p1 .nes. "ALL" then say "Shutting down link ''p1'"
$	if p1 .eqs. "ALL" then say "Shutting down all Servers"
$	say ""
$
$	index 	= 0
$	length 	= f$length("''p1'")
$	delete_count = 0
$
$ delete_loop:
$	index = index + 1
$	if index .gt. proc_count then goto end_delete_loop
$	item = server_name'index'
$	pid  = pid_list'index'
$	if p1 .eqs. "ALL" then goto delete_it
$	if f$extract(7, 15, item) .eqs. p1 then goto delete_it
$	tmp = f$length(item)
$	tmp_str = f$extract(0, tmp - 1, item)
$	if f$extract(7, 15, tmp_str) .eqs. p1 then goto delete_it
$	goto delete_loop
$
$ delete_it:
$	image_name = f$getjpi(pid,"IMAGNAME")
$	say "    Deleting ''pid' (''item')"
$	parent = f$getjpi(pid,"OWNER")
$	stop/id='pid'
$	if item - "_S_" .nes. item then goto cont_delete
$	efc_name = item - "DMQA_O_" - "DmQA_O_"
$	efn = f$extract(f$length(efc_name) - 1, 1, efc_name)
$	efc_name = "DMQA_" + -
		f$extract(0, f$length(efc_name) - 1, efc_name) + -
		"_EFC"
$	if (dmqa_shutdown_efn .nes. "") .and. (p1 .nes. "ALL") then -
		dmqa_shutdown_efn "''efc_name'" "''efn'"
$
$ cont_delete:
$	delete_count = delete_count + 1
$	loc = f$parse(image_name,,,"DEVICE") + -
		  f$parse(image_name,,,"DIRECTORY")
$	file = item - "DmQA_O" - "DmQA_S" - "DMQA_S" - "DMQA_O"
$	if parent .eqs. "" then parent = pid
$	if debug then say "_DmQA: Deleting ''loc'OFFSPRING''file'.tmp;*"
$	assign/nolog/user nl: sys$output
$	assign/nolog/user nl: sys$error
$	assign/nolog/user nl: sys$message
$	delete/noconf/nolog 'loc'OFFSPRING'file'.tmp;*
$
$!	if debug then say "_DmQA: Deleting ''loc'TMP''parent'.tmp;*"
$!	assign/nolog/user nl: sys$output
$!	assign/nolog/user nl: sys$error
$!	assign/nolog/user nl: sys$message
$!	delete/noconf/nolog 'loc'tmp'parent'.tmp;*
$
$	if debug then say "_DmQA: Deleting OFFSPRING''file'.tmp;*"
$	assign/nolog/user nl: sys$output
$	assign/nolog/user nl: sys$error
$	assign/nolog/user nl: sys$message
$	delete/noconf/nolog OFFSPRING'file'.tmp;*
$
$!	if debug then say "_DmQA: Deleting TMP''parent'.tmp;*"
$!	assign/nolog/user nl: sys$output
$!	assign/nolog/user nl: sys$error
$!	assign/nolog/user nl: sys$message
$!	delete/noconf/nolog tmp'parent'.tmp;*
$
$	if (p1 .eqs. "ALL") .or. (proc_count - delete_count .eq. 0) -
		.or. (p2 .nes. "") then -
		goto remove_image
$	goto delete_loop
$
$ remove_image:
$	if f$search("''image_name'") .eqs. "" then -
		goto delete_loop
$	if .not. f$file_attributes("''image_name'","KNOWN") then -
		goto delete_loop
$	if debug then say "_DmQA: Removing ''image_name'"
$	assign/nolog/user nl: sys$output
$	assign/nolog/user nl: sys$message
$	assign/nolog/user nl: sys$error
$	install remove 'image_name'
$	if .not. f$file_attributes("''image_name'","KNOWN") then -
		image_removed = -1
$
$	goto delete_loop
$
$ end_delete_loop:
$
$	if image_removed .eq. 0 then goto check_count
$	say ""
$	say "%Server image(s) de-installed from system"
$	goto exit
$
$ check_count:
$	if delete_count .eq. 0 then goto none_found
$	say "''delete_count" processes removed"
$	goto exit
$
$ NO_PRIVS:
$	say ""
$	say "?This program needs the following privileges:"
$	say ""
$	say "''privs_needed'"
$	say ""
$	goto exit
$
$ none_found:
$	say "?No qualifying processes found to delete"
$
$ exit:
$	say ""
$	privs = f$setprv("''old_privs'")
$ 	EXIT
