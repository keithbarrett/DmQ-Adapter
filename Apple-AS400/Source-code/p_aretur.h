/*  Module  p_aretur.h  DmQA V1.0                                           */
/*                                                                          */
/*                                                                          */
/*  Copyright (c) 1985, 1988, 1990, 1991 by                                 */
/*  Digital Equipment Corporation, Maynard, Mass.                           */
/*                                                                          */
/*  This software is furnished under a license and may be used and copied   */
/*  only  in  accordance  with  the  terms  of such  license and with the   */
/*  inclusion of the above copyright notice. This software or  any  other   */
/*  copies thereof may not be provided or otherwise made available to any   */
/*  other person. No title to and ownership of  the  software  is  hereby   */
/*  transferred.                                                            */
/*                                                                          */
/*  The information in this software is subject to change without  notice   */
/*  and  should  not be  construed  as  a commitment by Digital Equipment   */
/*  Corporation.                                                            */
/*                                                                          */
/*  Digital assumes no responsibility for the use or  reliability  of its   */
/*  software on equipment which is not supplied by Digital.                 */
/*                                                                          */
/*                                                                          */
/*  DECmessageQ Expertise Center                                            */
/*                                                                          */
/*                                                                          */
/*  IDENT HISTORY:                                                          */
/*                                                                          */
/*  Version     Date    Who     Description                                 */
/*                                                                          */
/*                                                                          */


#ifndef P_ARETUR_H
#define P_ARETUR_H

static char *pams_suclist[] = {
                    "PAMS__ZERO_RETURN",
                    "PAMS__SUCCESS",
                    "PAMS__LINK_UP",
                    "PAMS__JOURNAL_ON",
                    "PAMS__NOMOREMSG",
                    "PAMS_FACILITY",
                    "PAMS__DISC_SUCCESS",
                    "PAMS__DISCL_SUCCESS",
                    "PAMS__DLJ_SUCCESS",
                    "PAMS__DLQ_SUCCESS",
                    "PAMS__RTS_SUCCESS",
                    "PAMS__SAF_SUCCESS",
                    "PAMS__RECOVERMODE",
                    "PAMS__NO_UMA",
                    "PAMS__UMA_NA",
                    "PAMS__TRACEBACK",
                    "PAMS__STORED",
                    "PAMS__ENQUEUED",
                    "PAMS__UNATTACHEDQ",
                    "PAMS__CONFIRMREQ",
                    "PAMS__PROPAGATE",
                     "" };
                              

static char *pams_errlist[] = {
                    "PAMS__ZERO_RETURN",
                    "PAMS__NOSEND",
                    "PAMS__WAKEFAIL",
                    "PAMS__TIMERACT",
                    "PAMS__MSGACT",
                    "PAMS__BADDECLARE",
                    "PAMS__BADFREE",
                    "PAMS__TIMEOUT",
                    "PAMS__ACKTMO",
                    "PAMS__MSGUNDEL",
                    "PAMS__NOTALLOCATE",
                    "PAMS__MSGTOBIG",
                    "PAMS__BIGBLKSIZE",
                    "PAMS__BADRECEIVE",
                    "PAMS__BADRECVDEC",
                    "PAMS__BADRECVSFG",
                    "PAMS__BADRECVSNA",
                    "PAMS__BADRECVLNK",
                    "PAMS__INVALIDID",
                    "PAMS__TMPCREMBX",
                    "PAMS__TMPCREMBXS",
                    "PAMS__DCLTMPNOANS",
                    "PAMS__DCLTMPRCVW",
                    "PAMS__DCLTMPTYPE",
                    "PAMS__BADRCVSYNCH",
                    "PAMS__INVFORMAT",
                    "PAMS__INVALIDNUM",
                    "PAMS__NOTACTIVE",
                    "PAMS__EXCEEDQUOTA",
                    "PAMS__BADPRIORITY",
                    "PAMS__BADDELIVERY",
                    "PAMS__BADPROCNUM",
                    "PAMS__BADTMPPROC",
                    "PAMS__BADSYNCHNUM",
                    "PAMS__BADTMPSYNCH",
                    "PAMS__NOTDCL",
                    "PAMS__STATECHANGE",
                    "PAMS__INVUCBCNTRL",
                    "PAMS__NOLINK",
                    "PAMS__CIRACT",
                    "PAMS__PROTOCOL",
                    "PAMS__ABORT",
                    "PAMS__BADASSIGN",
                    "PAMS__TRANLOGNODE",
                    "PAMS__TRANLOGTASK",
                    "PAMS__LOGNAME",
                    "PAMS__LOGNAME2",
                    "PAMS__LOGNAME3",
                    "PAMS__LOGNAME4",
                    "PAMS__LOGNAME5",
                    "PAMS__NOOPEN-",
                    "PAMS__BADSCRIPT",
                    "PAMS__DECNETDEAD",
                    "PAMS__DECLARED",
                    "PAMS__EXHAUSTBLKS",
                    "PAMS__BADTIME",
                    "PAMS__INTERNAL1",
                    "PAMS__INTERNAL2",
                    "PAMS__INTERNAL3",
                    "PAMS__INTERNAL4",
                    "PAMS__BIGMSG",
                    "PAMS__MSGTOSMALL",
                    "PAMS__AREATOSMALL",
                    "PAMS__NOCANSEND",
                    "PAMS__QUECORRUPT",
                    "PAMS__REMQUEFAIL",
                    "PAMS__INSQUEFAIL",
                    "PAMS__CREATEFAIL",
                    "PAMS__DCLTMPFAIL",
                    "PAMS__PAMSDOWN",
                    "PAMS__BADASTPARM",
                    "PAMS__SPARENUM1",
                    "PAMS__SPARENUM2",
                    "PAMS__SPARENUM3",
                    "PAMS__SPARENUM4",
                    "PAMS__SPARENUM5",
                    "PAMS__SPARENUM6",
                    "PAMS__SPARENUM7",
                    "PAMS__EX_Q_LEN",
                    "PAMS__POSSDUPL",
                    "PAMS__STUB",
                    "PAMS__SENDER_TMO_EXPIRED",
                    "PAMS__MRQTBLFULL",
                    "PAMS__NOOBJECT",
                    "PAMS__CANCEL",
                    "PAMS__EXCMAXUNCONF",
                    "PAMS__OBJNOTACTIVE",
                    "PAMS__BUFFEROVF",
                    "PAMS__INVBUFFPTR",
                    "PAMS__BADJOURNAL",
                    "PAMS__COMMERR",
                    "PAMS__BADSELIDX",
                    "PAMS__IDXTBLFULL",
                    "PAMS__BADPARAM",
                    "PAMS__NOMRS",
                    "PAMS__DISC_FAILED",
                    "PAMS__DISCL_FAILED",
                    "PAMS__DLJ_FAILED",
                    "PAMS__DLQ_FAILED",
                    "PAMS__DQF_DEVICE_FAIL",
                    "PAMS__INVUMA",
                    "PAMS__DQF_FULL",
                    "PAMS__INVJH",
                    "PAMS__LINK_DOWN",
                    "PAMS__BADSEQ",
                    "PAMS__NOTJRN",
                    "PAMS__MRS_RES_EXH",
                    "PAMS__NOMOREJH",
                    "PAMS__REJECTED",
                    "PAMS__NOSUCHPCJ",
                    "PAMS__UCBERROR",
                    "PAMS__BADUMA",
                    "PAMS__BADRESPQ",
                    "PAMS__BADARGLIST",
                    "PAMS__NO_DQF",
                    "PAMS__NO_SAF",
                    "PAMS__RTS_FAILED",
                    "PAMS__SAF_DEVICE_FAIL",
                    "PAMS__SAF_FAILED",
                    "PAMS__BADLOGIC",
                    "PAMS__SELRCVACT",
                    "PAMS__NOMRQRESRC",
                    "PAMS__DUPLQNAME",
                    "PAMS__RESRCFAIL",
                    "PAMS__BADTAG",
                    "PAMS__BADTBQHANDLE",
                    "PAMS__INVACCESS",
                    "PAMS__BADNAME",
                    "PAMS__NAMETOOLONG",
                    "PAMS__BADCMD",
                    "PAMS__RESPQREQ",
                    "PAMS__INTERNAL",
                    "PAMS__NOQUOTA",
                    "PAMS__NOTPRIMARYQ",
                    "PAMS__NOTSUPPORTED",
                    "PAMS__NOTSECONDARYQ",
                    "PAMS__FATAL",
                    "PAMS__WRONGDOS",
                    "PAMS__NETERROR",
                    "PAMS__NETNOLINK",
                    "PAMS__NETLINKLOST",
                    "PAMS__BADINITFILE",
                    "PAMS__JOURNAL_FULL",
                    "PAMS__JOURNAL_FAIL",
                    "PAMS__BADQTYPE",
                           ""};
                           
#endif
