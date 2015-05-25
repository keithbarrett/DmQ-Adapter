/*  Module p_return.h	DmQA V1.0    	PAMS Status Return Codes            */
/*                                                                          */
/*                                                                          */
/*  Copyright (c) 1990, 1991 by                                             */
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
/*  This file contains a summation of all the status codes that exist in    */
/*  DECmessageQ for VMS & non-VMS systems. In cases where the sybol existed */
/*  on both platforms (or only VMS), the value for the symbol is from the   */
/*  VMS definition. Symbols that are only in the non-VMS DmQ products have  */
/*  the value from that product (usually a negative no.). VMS values were   */
/*  more important due to the fact the Queue Adapter Server itself runs     */
/*  on the VMS system.                                                      */
/*                                                                          */
/*                                                                          */
/*  IDENT HISTORY:                                                          */
/*                                                                          */
/*  Version     Date    Who     Description                                 */
/*                                                                          */
/*  V1.0    08-Mar-1991 kgb     Release                                     */
/*                                                                          */
/*                                                                          */


#ifndef	P_RETURN_H			/* Have we already been included?   */
#define P_RETURN_H


#define PAMS_FACILITY 		    2132

#ifdef ultrix
#ifndef SS$_NORMAL
#define SS$_NORMAL 		    1
#endif
#endif

#define PAMS_LAST_SUCCESS	    39		/* non-VMS only */
#define PAMS_LAST_FAIL 		    -288    	/* Largest non-VMS err code */
#define PAMS__SUC 		    1
#define PAMS__SUCCESS 		    1

#define PAMS__CONFIRMREQ	    139756323
#define PAMS__DISC_SUCCESS	    139755537
#define PAMS__DISCL_SUCCESS	    139755529
#define PAMS__DLJ_SUCCESS	    139755545
#define PAMS__DLQ_SUCCESS	    139755553
#define PAMS__ENQUEUED		    139756331
#define PAMS__JOURNAL_ON	    5		/* non-VMS only */
#define PAMS__LINK_UP		    3		/* non-VMS only */
#define PAMS__MSG		    139755561
#define PAMS__NO_UMA		    139756371
#define PAMS__NOMOREMSG		    139756347
#define PAMS__NOMSG		    139756355
#define PAMS__PROPAGATE		    139756379
#define PAMS__RECOVERMODE	    139755569
#define PAMS__RTS_SUCCESS	    139755577
#define PAMS__SAF_SUCCESS	    139755585
#define PAMS__STORED		    139756387
#define PAMS__TRACEBACK		    139756403
#define PAMS__UMA_NA		    139756411
#define PAMS__UNATTACHEDQ	    139756419

#define PAMS__TIMERACT		    139756395
#define PAMS__MSGACT		    139756339
#define PAMS__NOSEND		    139756363
#define PAMS__WAKEFAIL		    139756427
/* Note: For DECmessageQ V1.0 on non-vms systems, these */
/*       symbols are defined with non-success values.   */


#define PAMS__ABORT		    139757120
#define PAMS__ACKTMO		    139757128
#define PAMS__AMBIG		    139758722
#define PAMS__AREATOSMALL	    139758730
#define PAMS__BADARGLIST	    139758738
#define PAMS__BADASSIGN		    139761924
#define PAMS__BADASTPARM	    139761932
#define PAMS__BADCMD		    139758746
#define PAMS__BADDECLARE	    139757136
#define PAMS__BADDELIVERY	    139758754
#define PAMS__BADFREE		    139757144
#define PAMS__BADINITFILE 	    -282	/* New for PC/ULTRIX/SUN */
#define PAMS__BADJOURNAL	    139758762
#define PAMS__BADLOGIC		    139758770
#define PAMS__BADMSGBUF		    139761940	/* VMS Only */
#define PAMS__BADNAME		    139758778
#define PAMS__BADPARAM		    139758786
#define PAMS__BADPRIORITY	    139758794
#define PAMS__BADPROCNUM	    139758802
#define PAMS__BADQTYPE 		    -288	/* New DmQ Code */
#define PAMS__BADRCVSYNCH           -48		/* non-VMS only */
#define PAMS__BADRECEIVE	    139758810
#define PAMS__BADRECVDEC            -28		/* non-VMS only */
#define PAMS__BADRECVLNK            -34		/* non-VMS only */
#define PAMS__BADRECVSFG            -30		/* non-VMS only */
#define PAMS__BADRECVSNA            -32		/* non-VMS only */
#define PAMS__BADRESPQ		    139758818
#define PAMS__BADSCRIPT		    139761948
#define PAMS__BADSELIDX		    139758826
#define PAMS__BADSEQ		    139758834
#define PAMS__BADSYNCHNUM	    139758842
#define PAMS__BADTAG		    139758850
#define PAMS__BADTBQHANDLE	    139758858
#define PAMS__BADTIME		    139758866
#define PAMS__BADTMPPROC	    139758874
#define PAMS__BADTMPSYNCH	    139758882
#define PAMS__BADUMA		    139758890
#define PAMS__BIGBLKSIZE	    139758898
#define PAMS__BIGMSG		    139758906
#define PAMS__BUFFEROVF		    139757152
#define PAMS__CANCEL		    139757160
#define PAMS__CIRACT		    139758914
#define PAMS__COMMERR		    139758922
#define PAMS__CREATEFAIL	    139761956
#define PAMS__DCLTMPFAIL	    139761964
#define PAMS__DCLTMPNOANS           -42		/* non-VMS only */
#define PAMS__DCLTMPRCVW            -44		/* non-VMS only */
#define PAMS__DCLTMPTYPE            -46		/* non-VMS only */
#define PAMS__DECLARED		    139761972
#define PAMS__DECNETDEAD            -104	/* non-VMS only */
#define PAMS__DISCL_FAILED	    139758930
#define PAMS__DISC_FAILED	    139758938
#define PAMS__DLJ_FAILED	    139758946
#define PAMS__DLQ_FAILED	    139758954
#define PAMS__DNSCLASSBAD	    139758962	/* VMS Only */
#define PAMS__DNSDIRFAIL	    139758970	/* VMS Only */
#define PAMS__DNSFMTBAD		    139758978	/* VMS Only */
#define PAMS__DQF_DEVICE_FAIL	    139758986
#define PAMS__DQF_FULL		    139758994
#define PAMS__DUPLMSGCLASS	    139759002	/* VMS Only */
#define PAMS__DUPLMSGFROM	    139759010	/* VMS Only */
#define PAMS__DUPLMSGTO		    139759018	/* VMS Only */
#define PAMS__DUPLMSGTYPE	    139759026	/* VMS Only */
#define PAMS__DUPLQNAME		    139759034
#define PAMS__ENDOFSOURCE	    139757168	/* VMS Only */
#define PAMS__ERRORS		    139759042	/* VMS Only */
#define PAMS__EXCEEDQUOTA	    139759050
#define PAMS__EXCMAXUNCONF	    139757176
#define PAMS__EXHAUSTBLKS	    139761980
#define PAMS__EXTRA		    139759058	/* VMS Only */
#define PAMS__EX_Q_LEN		    139757184
#define PAMS__FATAL                 -272	/* New for PC/ULTRIX/SUN */
#define PAMS__IDXTBLFULL	    139759066
#define PAMS__INSQUEFAIL	    139761988
#define PAMS__INTERNAL		    139761996
#define PAMS__INTERNAL1             -112	/* non-VMS only */
#define PAMS__INTERNAL2             -114	/* non-VMS only */
#define PAMS__INTERNAL3             -116	/* non-VMS only */
#define PAMS__INTERNAL4             -118	/* non-VMS only */
#define PAMS__INVACCESS		    139759074
#define PAMS__INVALIDID		    139759082
#define PAMS__INVALIDNUM	    139759090
#define PAMS__INVBUFFPTR	    139759098
#define PAMS__INVFORMAT		    139759106
#define PAMS__INVJH		    139759114
#define PAMS__INVUCBCNTRL	    139759122
#define PAMS__INVUMA		    139759130
#define PAMS__IVDATAFORMAT	    139759138	/* VMS Only */
#define PAMS__IVDATATYPE	    139759146	/* VMS Only */
#define PAMS__IVECHOLIM		    139759154	/* VMS Only */
#define PAMS__IVLOGLIM		    139759162	/* VMS Only */
#define PAMS__IVMSGCLASS	    139759170	/* VMS Only */
#define PAMS__IVMSGJOU		    139759178	/* VMS Only */
#define PAMS__IVMSGPRI		    139759186	/* VMS Only */
#define PAMS__IVMSGSRC		    139759194	/* VMS Only */
#define PAMS__IVMSGTARG		    139759202	/* VMS Only */
#define PAMS__IVMSGTYPE		    139759210	/* VMS Only */
#define PAMS__IVREPVAL		    139759218	/* VMS Only */
#define PAMS__IVSETKEY		    139759226	/* VMS Only */
#define PAMS__IVSETLOG		    139759234	/* VMS Only */
#define PAMS__IVSETSR		    139759242	/* VMS Only */
#define PAMS__IVSETSRWHO	    139759250	/* VMS Only */
#define PAMS__IVTIME		    139759258	/* VMS Only */
#define PAMS__JOURNAL_FAIL          -286	/* New for PC/ULTRIX/SUN */
#define PAMS__JOURNAL_FULL          -284	/* New for PC/ULTRIX/SUN */
#define PAMS__LINK_DOWN		    139759266
#define PAMS__LOGFOPENFAI	    139759274	/* VMS Only */
#define PAMS__LOGFWRITERR	    139759282	/* VMS Only */
#define PAMS__LOGNAME		    139762004
#define PAMS__LOGNAME2		    139762012
#define PAMS__LOGNAME3		    139762020
#define PAMS__LOGNAME4		    139762028
#define PAMS__LOGNAME5		    139762036
#define PAMS__MISENDC		    139759290	/* VMS Only */
#define PAMS__MISENDR		    139759298	/* VMS Only */
#define PAMS__MISEOM		    139759306	/* VMS Only */
#define PAMS__MISQUOTE		    139759314	/* VMS Only */
#define PAMS__MISREPEAT		    139759322	/* VMS Only */
#define PAMS__MRQTBLFULL	    139757192
#define PAMS__MRS_RES_EXH	    139759330
#define PAMS__MSGCONTENT	    139759338	/* VMS Only */
#define PAMS__MSGFMT		    139759346	/* VMS Only */
#define PAMS__MSGTOBIG		    139759354
#define PAMS__MSGTOOLONG	    139759362	/* VMS Only */
#define PAMS__MSGTOSMALL	    139759370
#define PAMS__MSGUNDEL		    139757200
#define PAMS__NAMETOOLONG	    139759378
#define PAMS__NETERROR              -276	/* New for PC/ULTRIX/SUN */
#define PAMS__NETLINKLOST           -280	/* New for PC/ULTRIX/SUN */
#define PAMS__NETNOLINK             -278	/* New for PC/ULTRIX/SUN */
#define PAMS__NOACCESS		    139762124	/* VMS Only */
#define PAMS__NOACL		    139762116	/* VMS Only */
#define PAMS__NOCANSEND		    139759386
#define PAMS__NOLINK		    139759394
#define PAMS__NOLOGFILE		    139759402	/* VMS Only */
#define PAMS__NOMOREJH		    139759410
#define PAMS__NOMRQRESRC	    139759418
#define PAMS__NOMRS		    139759426
#define PAMS__NOMSGCLASS	    139759434	/* VMS Only */
#define PAMS__NOMSGFROM		    139759442	/* VMS Only */
#define PAMS__NOMSGTO		    139759450	/* VMS Only */
#define PAMS__NOMSGTYPE		    139759458	/* VMS Only */
#define PAMS__NOOBJECT		    139757208
#define PAMS__NOOPEN		    139762044
#define PAMS__NOPRIV		    139762052	/* VMS Only */
#define PAMS__NOQUOTA		    139762060
#define PAMS__NOSUCHPCJ		    139759466
#define PAMS__NOTACTIVE		    139759474
#define PAMS__NOTALLOCATE	    139759482
#define PAMS__NOTASTMT		    139759490
#define PAMS__NOTDCL		    139759498
#define PAMS__NOTDECIMAL	    139759506	/* VMS Only */
#define PAMS__NOTHEX		    139759514	/* VMS Only */
#define PAMS__NOTJRN		    139759522
#define PAMS__NOTOCTAL		    139759530
#define PAMS__NOTPRIMARYQ	    139762068
#define PAMS__NOTSECONDARYQ	    139762076
#define PAMS__NOTSUPPORTED	    139762084
#define PAMS__NOTYETIMP		    139757216
#define PAMS__NO_DQF		    139759538
#define PAMS__NO_SAF		    139759546
#define PAMS__OBJNOTACTIVE	    139757224
#define PAMS__PAMSDOWN		    139762092
#define PAMS__PNUMNOEXIST	    139759554
#define PAMS__POSSDUPL		    139757232
#define PAMS__PROTOCOL		    139759562	/* VMS Only */
#define PAMS__QUECORRUPT	    139762100
#define PAMS__REJECTED		    139759570
#define PAMS__REMQUEFAIL	    139762108
#define PAMS__RESPQREQ		    139759578
#define PAMS__RESRCFAIL		    139759586
#define PAMS__RTS_FAILED	    139759594
#define PAMS__SAF_DEVICE_FAIL	    139759602
#define PAMS__SAF_FAILED	    139759610	/* VMS Only */
#define PAMS__SAF_FORCED	    139757264
#define PAMS__SELRCVACT		    139759618
#define PAMS__SENDER_TMO_EXPIRED    139757240
#define PAMS__SPARENUM1             -142	/* non-VMS only */
#define PAMS__SPARENUM2             -144	/* non-VMS only */
#define PAMS__SPARENUM3             -146	/* non-VMS only */
#define PAMS__SPARENUM4             -148	/* non-VMS only */
#define PAMS__SPARENUM5             -150	/* non-VMS only */
#define PAMS__SPARENUM6             -152	/* non-VMS only */
#define PAMS__SPARENUM7             -154	/* non-VMS only */
#define PAMS__STATECHANGE	    139759626
#define PAMS__STRINGTOOLONG	    139759634	/* VMS Only */
#define PAMS__STUB		    139757248
#define PAMS__SYNTAX		    139759642	/* VMS Only */
#define PAMS__TIMEOUT		    139757256
#define PAMS__TMPCREMBX             -38		/* non-VMS only */
#define PAMS__TMPCREMBXS            -40		/* non-VMS only */
#define PAMS__TOOMANYLOGF	    139759650	/* VMS Only */
#define PAMS__TOOMANYREPEAT	    139759658	/* VMS Only */
#define PAMS__TOOMANYSETS	    139759666	/* VMS Only */
#define PAMS__TRANLOGNODE           -86		/* non-VMS only */
#define PAMS__TRANLOGTASK           -88	    	/* non-VMS only */
#define PAMS__UCBERROR		    139759674
#define PAMS__VALUEOVERFL	    139759682	/* VMS Only */
#define PAMS__WRONGDOS              -274	/* New for PC/ULTRIX/SUN */
#define PAMS__ZEROREPEAT	    139759690	/* VMS Only */

#endif
