/*  Module p_dmqa.h DmQA V1.0        DECmessageQ Queue Adapter              */
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
/*  IDENT HISTORY:                                                          */
/*                                                                          */
/*  Version     Date    Who     Description                                 */
/*                                                                          */
/*  V1.0FT  08-Mar-1991 kgb     Initial release; Keith Barrett              */
/*  V1.0FT  09-Apr-1991	kgb	Removed several extern statements for	    */
/*				AS/400 support				    */
/*  V1.0FT  22-May-1991	kgb	Added symbol DMQA__force_bracket	    */
/*  V1.0FT  11-Jul-1991 kgb	Added HP definitions                        */
/*                                                                          */
/*                                                                          */
/*                                                                          */

/*  This include file is for interfacing the PAMS compatible application    */
/*  with the Queue Adapter routines. With the exception of including or ex- */
/*  cluding a single "#include" file, application code is compatible with   */
/*  DECmessageQ applications.                                               */
/*                                                                          */
/*  To build against the adapter, the application must first include this   */
/*  header file, and then optional set a few global variables. This include */
/*  file must appear LAST; after all other system & PAMS include files, due */
/*  to references and re-definitions. It also requires that the program     */
/*  previously includes the "stddef.h" file (if any).                       */
/*                                                                          */
/*  After including this file, the following variables must be set up       */
/*  before any calls to the PAMS API. The definitions should be enclosed in */
/*  an "#ifdef DMQ__QADAPTER" statement to ensure code portability.         */
/*                                                                          */
/*  For TCP/IP Communications:                                              */
/*		(char *) DMQA__host	Name of host system (clients only). */
/*	(unsigned short) DMQA__port	Port number for connect (1 - 65535) */
/*                                                                          */
/*	    These should be typically set up once, early in the program.    */
/*	    They may be modified within program execution to allow the      */
/*	    application to communicate w/ multiple TCP/IP servers; However, */
/*	    only one server can be use at a time, and the values CANNOT be  */
/*	    changed until AFTER a PAMS_EXIT was called. One application for */
/*	    this feature might be failover logic.                           */
/*                                                                          */
/*	    (Note: TCP/IP Communications uses STREAM IP data only, so no    */
/*	    provision is made for defining other types).                    */
/*                                                                          */
/*    For Serial communications:                                            */
/*	    <to be determined>                                              */
/*                                                                          */
/*    For DECmessageQ/PAMS Bus Communications:                              */
/*	    <reserved for future use>                                       */
/*                                                                          */
/*    Optional (useable for any communications, set to non-zero):           */
/*	    DMQA__debug			Enable debug mode. May be set or    */
/*					cleared at will.                    */
/*                                                                          */
/*	    DMQA__trans_from_EBCDIC	Translate our msg from EBCDIC into  */
/*					ASCII before putting it on the PAMS */
/*					bus (Default is system dependent)   */
/*                                                                          */
/*	    DMQA__trans_to_EBCDIC	Translate msgs from the PAMS bus    */
/*					into EBCDIC before returning them.  */
/*					(Default is system dependent).      */
/*                                                                          */
/*	    DMQA__argc  and		If set to the values returned by    */
/*	    **DMQA__argv	        argv & argc, AND the 1st parameter  */
/*					in argv is "DMQA", then the remain  */
/*					ing parameters will be used to au-  */
/*					to matically fill in the communica- */
/*					tion specific parameters (such as   */
/*					DMQA__host). See documentation for  */
/*					details.                            */


#ifndef P_DMQA_H
#define P_DMQA_H		       /* This file has now been included     */



			/***** General definitions *****/

#ifndef FALSE
#define	FALSE	0
#define	TRUE	1
#endif

#ifdef VMS
#ifndef SS$_NORMAL
#include <ssdef.h>
#endif
#endif

#ifdef ultrix
#ifndef SS$_NORMAL
#define SS$_NORMAL 1
#endif
#endif

#ifndef PAMS__SUCCESS
#define PAMS__SUCCESS 1		    /* Provide VMS PAMS compatibility */
#endif

#ifndef PAMS__NOTSUPPORTED
#define PAMS__NOTSUPPORTED -95
#endif
/* Safe number to use - it's odd and negative. */

#ifndef PAMS__COMMERR
#define PAMS__COMMERR -180
#endif

#ifndef	PAMS__NETERROR
#define PAMS__NETERROR PAMS__COMMERR
#endif


#ifndef abs
#define abs(x) ((x)<0?-(x):(x))
#endif

#ifdef __STDC__
#ifndef __USE_ARGS
#define __USE_ARGS TRUE
#endif
#endif

#ifdef __ANSI__
#ifndef __USE_ARGS
#define __USE_ARGS TRUE
#endif
#endif

#ifdef __SAA__
#ifndef __USE_ARGS
#define __USE_ARGS TRUE
#endif
#endif
/* IBM */

#ifdef __SAA_L2__
#ifndef __USE_ARGS
#define __USE_ARGS TRUE
#endif
#endif
/* IBM */

#ifdef __EXTENDED__
#ifndef __USE_ARGS
#define __USE_ARGS TRUE
#endif
#endif
/* IBM */

#ifdef VMS
#ifndef __USE_ARGS
#define __USE_ARGS TRUE
#endif
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef CONST
#define CONST const
#endif

#ifndef ANY
#define ANY VOID
#endif

#ifndef QUADWORD
#define QUADWORD long
#endif


#define DMQ__QADAPTER TRUE	       /* General def to declare our presence */
#define DMQA__IDENT "V02-01 "  	       /* ident (must be 7 chars long)        */
#define DMQA__PROTOCOL	    4	       /* Protocol revision (must be numeric) */
#define DMQA__CLIENT_ID	    1	       /* Client Process (must be numeric)    */
#define DMQA__SERVER_ID	    2	       /* Server Process (must be numeric)    */
#define DMQA__DBG_PREFIX    "_DmQA:"

#ifndef DMQA__OFFSPRING	       	       /* Maximum number of server children   */
#define DMQA__OFFSPRING 15	       /* (must be from 0 to 15)              */
#endif

 struct DMQA__HEADER_REC
	{
            long            lbyte_order;        /* 4 char array of byte order */
	    long	    pams_status;	/* PAMS returned status	      */
	    long	    error;		/* Remote errno status	      */
	    unsigned long   task_id;		/* Task ID of Program	      */
	    unsigned long   msg_seq_no;		/* Message seqence number     */
	    long	    flags;		/* Bit flags:		      */
						/*  0	msg is in EBCDIC      */
	    					/*  1	os swaps bytes	      */
						/*  2	os aligns datatypes   */
						/*  3   trans_from_ebcdic     */
						/*  4   trans_to_ebcdic       */
						/*  5	header is in EBCDIC   */
						/*  6	msg is text only      */
						/*  7   Reserved; must be 0   */
						/*  8   Internal message      */
						/*  9	Force EBCDIC bracket? */
						/*10-30 <unused>	      */
						/*  31  Reserved; must be 0   */
	    long	    reserved_long;	/* Reserved for future use    */
	    short	    msg_length;		/* Expected length in bytes   */
            short           sbyte_order;        /* 2 char array of byte order */
	    short	    checksum;		/* Reserved for async chksum  */
	    short	    max_packetsize;	/* Max packet size allowed    */
	    unsigned short  offspring_link;	/* link to server's offspring */
	    short	    reserved_short;	/* Reserved for future use    */
	    char	    ident[8];		/* Ident string               */
	    char	    id;			/* Message ID (client/server) */
	    char	    os;			/* Operating system id	      */
	    char	    protocol;		/* Protocol Revision	      */
	    char	    cmd;		/* PAMS command value         */
	    char	    type;		/* msg type - Reserved        */
	    char	    reserved_char1;	/* Reserved for future use    */
	    char	    reserved_char2;	/* Reserved for future use    */
	    char	    reserved_char3;	/* Reserved for future use    */
	};
	/* Note: This structure must END on a longword boundry */

#define DMQA__HEADER struct DMQA__HEADER_REC

#define DMQA__HEADERSIZE 56
/* NOTE: This value must be hard-coded and match the size of the structure   */
/*       above. Using "sizeof()" in a "#define" is not portable.             */

#define DMQA__OVERHEAD (DMQA__HEADERSIZE+37+42+40)
/* Header + 37 (max params) + biggest psb (40+2) + showbuffer min (40)       */
/* This is an estimated overhead for the DMQA protocol                       */


#ifndef DMQA__INTERNAL		       /* Application definitions	      */

    extern  char	    *DMQA__host;
    extern  char	    *DMQA__secondary_host;
    extern  unsigned short  DMQA__port;
    extern  unsigned short  DMQA__secondary_port;
    extern  unsigned short  DMQA__highest_port;
    /* TCP/IP server paramters */

    extern int		    DMQA__debug;		/* debug mode         */
    extern int		    DMQA__argc;			/* copy of argc       */
    extern char	    	    **DMQA__argv;		/* copy of argv       */
    extern unsigned long    DMQA__task_id;		/* task-id            */
    extern long	    	    DMQA__errno;		/* copy of errno      */
    extern int		    DMQA__trans_to_EBCDIC;
    extern int		    DMQA__trans_from_EBCDIC;
    extern int		    DMQA__msg_is_in_EBCDIC;
    extern int              DMQA__offspring_flag;       /* Are we offspring?  */
    extern int		    DMQA__force_bracket;	/* Do EBCDIC like ftp */
    /* General parameters */
        
#ifdef __USE_ARGS
    extern long	    DMQA__test(char *, short, short *, char *, long);
#else
    extern long	    DMQA__test();
#endif
/* Additional Prototypes */



#else				    /* Internal definitions		      */



			/***** Operating Systems *****/


#define	DMQA__VMS_OS	  1	    /* VMS			    */
#define	DMQA__MSDOS_OS    2	    /* MSDOS			    */
#define	DMQA__ULTRIX_OS   3	    /* Ultrix			    */
#define	DMQA__SUN_OS	  4	    /* SUN			    */
#define DMQA__OS2_OS	  5	    /* OS/2			    */
#define DMQA__AMIGA_OS    6	    /* Amiga			    */
#define DMQA__AS400_OS    7	    /* AS400			    */
#define DMQA__MAC_OS	  8	    /* MAC			    */
#define DMQA__HP_OS	  9         /* HP			    */
/*			  10-88	       Reserved for Digital	    */
#define	DMQA__RSTSE_OS    89	    /* RSTS/E   		    */
#define	DMQA__RSX_OS      90	    /* RSX			    */
/*			  91-98	       Reserved for Customer use    */
/*			  99	       Historical (reserved)	    */
/*			  100-126      Reserved for Customer use    */
#define	DMQA__UNKNOWN_OS  127	    /* unknown system		    */


		    /***** PAMS Entry point definitions *****/

#define	DMQA__PAMS_ATTACH_Q		1	/* Send msg via buffer        */
#define	DMQA__PAMS_CANCEL_SELECT	2	/* cancel msg selection       */
#define	DMQA__PAMS_CANCEL_GET		3	/* Cancel Async GETS          */
#define	DMQA__PAMS_CLOSE_JRN		4	/* close DLJ/PCJ	      */
#define	DMQA__PAMS_CONFIRM_MSG		5	/* Confirm an MRS message     */
#define	DMQA__PAMS_EXIT			6	/* exit			      */
#define	DMQA__PAMS_GET_MSG		7	/* NonWait get msg via buffer */
#define	DMQA__PAMS_GET_MSGA		8	/* get msg via buffer w/ AST  */
#define	DMQA__PAMS_GET_MSGW		9	/* Wait get msg via buffer    */
#define	DMQA__PAMS_LOCATE_Q		10	/* Send msg via buffer        */
#define	DMQA__PAMS_OPEN_JRN		11	/* Open a DLJ/PCJ	      */
#define	DMQA__PAMS_PUT_MSG		12	/* Send msg via buffer        */
#define	DMQA__PAMS_READ_JRN		13	/* read DLJ/PCJ		      */
#define	DMQA__PAMS_SET_SELECT		14	/* Get a msg selection index  */
#define	DMQA__PAMS_SET_TIMER		15	/* Set timer                  */
/* Standard set */

#define	DMQA__PUTIL_SHOW_PENDING	100	/* Get msg pending count      */
#define	DMQA__PUTIL_PRINT_STS		101	/* Print formatted msg        */
/* PUTILs */

#define	DMQA__PAMS_ALLOC_MSG		70	/* Obtain msg pointer	      */
#define	DMQA__PAMS_CANCEL_SYNCH		71	/* Cancel Synch		      */
#define	DMQA__PAMS_DCL_PROCESS		72	/* Declare Process	      */
#define	DMQA__PAMS_DCL_SYNCH		73	/* declare synch channel      */
#define	DMQA__PAMS_FREE_MSG		74	/* Release msg ptr after rcv  */
#define	DMQA__PAMS_RCV_MSG		75	/* NonWait msg rcv via ptr    */
#define	DMQA__PAMS_RCV_MSGW		76	/* Wait msg rcv via ptr       */
#define	DMQA__PAMS_SEND_MSG		77	/* Send msg via pointer	      */
#define	DMQA__PAMS_SET_SYNCH		78	/* Enable synch		      */
#define	DMQA__PAMS_SHOW_MSG		79	/* Get msg header info        */
#define	DMQA__PAMS_WAIT_TIMER		80	/* Wait for timer	      */
/* Obsoletes */

#define	DMQA__PAMS_UNKNOWN		0	/* Undefined		      */
#define DMQA__INTERNAL_MSG              126     /* Internal use               */
#define	DMQA__TEST_MSG			127	/* Test message               */
/* Reserved - must have these values */


			    /***** Bit values *****/


#define	DMQA_BIT__MSG_IN_EBCDIC	    1	    /* Msg is in EBCDIC		    */
#define DMQA_BIT__OS_SWAPS_BYTES    2	    /* OS stores bytes backwards    */
#define DMQA_BIT__OS_ALIGNS_DATA    4	    /* RISC system		    */
#define DMQA_BIT__TRANS_FROM_EBCDIC 8	    /* Msg is in EBCDIC - translate */
#define DMQA_BIT__TRANS_TO_EBCDIC   16	    /* Response must be in EBCDIC   */
#define	DMQA_BIT__HEADER_IS_EBCDIC  32	    /* Header is in EBCDIC	    */
#define DMQA_BIT__MSG_IS_TEXT_ONLY  64	    /* Message is text only         */
#define	DMQA_BIT__RESERVED1	    128	    /* Reserved, must be zero       */
#define	DMQA_BIT__INTERNAL_MSG	    256	    /* Message is DmQA internal     */
#define DMQA_BIT__FORCE_BRACKET	    512	    /* Force EBCDIC "[]" like ftp   */
/* Header flags */


		    /***** DMQA Configuration definitions *****/


#ifdef DMQA__PAMSV25			/* pre-defined by adapter API/IPI   */
#define DMQA__USE_PAMS
#endif

#ifdef DMQA__PAMSV30			/* pre-defined by adapter API/IPI   */
#define DMQA__USE_PAMS
#endif

#ifdef DMQA__DMQ010
#define DMQA__USE_PAMS			/* pre-defined by adapter API/IPI   */
#endif

#ifdef RSX
#define DMQA__OS DMQA__RSX_OS		/* RSX */
#define DMQA__SIZMAX 2048		/* Maximum communications msg size */
#endif

#ifdef RSTSE
#define DMQA__OS DMQA__RSTSE_OS		/* RSTSE */
#define DMQA__SIZMAX 2048		/* Maximum communications msg size */
#endif

#ifdef VMS
#define	DMQA__OS DMQA__VMS_OS		/* VMS */
#define DMQA__SIZMAX 32767		/* Maximum communications msg size */
#define DMQA__MSGMAX 32000		/* maximum PAMS msg size */
#endif

#ifdef MSDOS
#define DMQA__SIZMAX 4096		/* Maximum communications msg size */
#ifdef OS2
#define DMQA__OS DMQA__OS2_OS		/* OS2 */
#else
#define DMQA__OS DMQA__MSDOS_OS		/* MSDOS */
#endif
#endif

#ifdef ultrix
#define DMQA__OS DMQA__ULTRIX_OS	/* ULTRIX */
#ifdef	DMQA__USE_PAMS
#define DMQA__SIZMAX 4096		/* Maximum communications msg size */
#else
#define DMQA__SIZMAX 32767		/* Maximum communications msg size */
#define DMQA__MSGMAX 32000		/* maximum PAMS msg size */
#endif
#endif

#ifdef sun
#define DMQA__OS DMQA__SUN_OS		/* SUN */
#define DMQA__SIZMAX 4096		/* Maximum communications msg size */
#endif

#ifdef AMIGA
#define DMQA__OS DMQA__AMIGA_OS		/* AMIGA */
#define DMQA__SIZMAX 4096		/* Maximum communications msg size */
#endif

#ifdef THINK_C
#define DMQA__OS DMQA__MAC_OS		/* MAC */
#define DMQA__SIZMAX 4096		/* Maximum communications msg size */
#endif

#ifdef HP				/* must be defined on compile */
#define DMQA__OS DMQA__HP_OS		/* HP */
#define DMQA__SIZMAX 32767		/* Maximum communications msg size */
#define DMQA__MSGMAX 32000		/* maximum PAMS msg size */
#endif

#ifdef AS400				/* must be defined on compile */
#define DMQA__OS DMQA__AS400_OS		/* AS400 */
#define DMQA__SIZMAX 32767		/* Maximum communications msg size */
#define DMQA__MSGMAX 32000		/* maximum PAMS msg size */
#define DMQA__EBCDIC TRUE
#endif

#ifndef DMQA__OS
#define DMQA__OS DMQA__UNKNOWN_OS	/* ?UNKNOWN? */
#define DMQA__SIZMAX 32767		/* Maximum communications msg size */
#define DMQA__MSGMAX 32000		/* maximum PAMS msg size */
#endif

#ifndef DMQA__MSGMAX
#define DMQA__MSGMAX (DMQA__SIZMAX - DMQA__OVERHEAD)
#endif

#ifndef DMQA__SHOWBUFMAX
#define DMQA__SHOWBUFMAX    128        /* Maximum showbuffer (in bytes); must */
#endif				       /* be multiple of 4 - Minimum is 60    */

#ifndef DMQA__NAMELISTMAX              /* Maximum number of name space        */
#define DMQA__NAMELISTMAX   128        /* entries (each takes 3 longword)     */
#endif                                 /* for pams_attach_q and pams_locate_q */

#ifndef DMQA__SELARRAYMAX              /* Maximum number of selection array   */
#define DMQA__SELARRAYMAX   ((DMQA__MSGMAX - 6) / 52)
#endif				       /* for pams_set_select		      */

/* Note: Communications modules also define DMQA__SENDMAX and DMQA__RECVMAX   */
/* which set the maximum packet size on a send or receive.                    */

#ifndef DMQA__EBCDIC
#define DMQA__EBCDIC FALSE
#endif


                     /***** Other definitions *****/


#define DMQA__TCPIP_BACKLOG 1

#ifdef __USE_ARGS
extern void		DMQA__push_long(char **, char *);
extern void		DMQA__push_short(char **, char *);
extern void		DMQA__push_char(char **, char *);
extern void		DMQA__push_psb(char **, char *, short);
extern unsigned long	DMQA__pop_long(char **);
extern unsigned short   DMQA__pop_short(char **);
extern void		DMQA__pop_psb(char **, char *, short);
extern VOID             *memcpy(VOID *, CONST VOID *, size_t);
#else
extern void		DMQA__push_long();
extern void		DMQA__push_short();
extern void		DMQA__push_char();
extern void		DMQA__push_psb();
extern unsigned long	DMQA__pop_long();
extern unsigned short   DMQA__pop_short();
extern void		DMQA__pop_psb();
extern VOID             *memcpy();
#endif
/* Global internal functions */

#define DMQA__DBG_EXIT(x) printf("_DmQA: -- Exiting call with status %d --\n",x)
#define DMQA__BUFFEROVF   PAMS__BUFFEROVF
#define DMQA__BIGGEST_PSB 40

#endif
#endif
