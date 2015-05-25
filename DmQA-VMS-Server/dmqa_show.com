$! DMQA_SHOW.COM
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
$! P1 [optional]  If specified, will cause the display to "update" at that
$!		  interval.
$!
$! Revisions:
$!
$!	25-Apr-1991	kgb	Initial release; Keith Barrett
$!
$
$	esc[0,7]	= 27
$	bell[0,7]	= 7
$	say		= "write sys$output"
$	output		= "write outfile"
$	cur_dir		= f$environment("DEFAULT")
$	debug		= 0
$	node		= f$getsyi("SCSNODE") - ":" - ":"
$	node		= f$edit(node, "COLLAPSE")
$	old_privs	= f$setprv("WORLD,SYSPRV,GROUP")
$	if "''dmqa$debug'" .nes. "" then debug = 1
$
$
$	if p1 .nes. "" then say "''esc'[H''esc'[J"
$
$ begin:
$	proc_count	= 0
$	server_name1	= ""
$	pid_list1	= ""
$	index		= 0
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
$	previous = ""
$
$ display_loop:
$	index = index + 1
$	if index .gt. proc_count then goto end_display_loop
$	item = server_name'index'
$	pid  = pid_list'index'
$	type = f$edit(f$extract(5, 1, item), "UPCASE")
$	desc = "(server)"
$	if type .eqs. "O" then desc = "(offspring)"
$	if previous .eqs. "" then previous = type
$!	if previous .eqs. "O" .and. type .eqs. "S" then say ""
$	previous = type
$	say "   ''pid'	''item'	''desc'"
$	goto display_loop
$
$ end_display_loop:
$	if p1 .eqs. "" then goto exit
$	wait 00:00:'p1'
$	say "''esc'[H''esc'[J"
$	goto begin
$
$ exit:
$	privs	= f$setprv(old_privs)
$	say ""
$ end:	EXIT
