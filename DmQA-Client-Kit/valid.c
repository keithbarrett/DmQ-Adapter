/*************************/
/* compile time switches */
/*************************/

/*****************************************************************************/
/*  DECMessageQ for VMS							     */
/*                                                                           */
/*  Copyright (c) 1990                    			             */
/*  by DIGITAL Equipment Corporation, Maynard, Mass.			     */
/*									     */
/*  This software is furnished under a license and may be used and  copied   */
/*  only  in  accordance  with  the  terms  of  such  license and with the   */
/*  inclusion of the above copyright notice.  This software or  any  other   */
/*  copies  thereof may not be provided or otherwise made available to any   */
/*  other person.  No title to and ownership of  the  software  is  hereby   */
/*  transferred.							     */
/*									     */
/*  The information in this software is subject to change  without  notice   */
/*  and  should  not  be  construed  as  a commitment by DIGITAL Equipment   */
/*  Corporation.							     */
/*									     */
/*  DIGITAL assumes no responsibility for the use or  reliability  of  its   */
/*  software on equipment which is not supplied by DIGITAL.		     */
/*									     */
/*****************************************************************************/



/*--------------------------------------------------------------------------*/
/* Source File:	valid.c							    */
/* Module:		valid                                               */
/* Author:		R Skelding                                          */
/* Date Written:	August 1989                                         */
/*                                                                          */
/* Descripton:                                                              */
/*                                                                          */
/*   Portable test package for testing Success and Failure conditions       */
/*   for a range of arguments to PAMS_PUT_MSG.  It will also exercise       */
/*   the successful reception of a variety of delivery modes.  This same    */
/*   program is used to Declare as a sender or a receiver.                  */
/*                                                                          */
/*                                                                          */
/* Revisions:                                                               */
/*                                                                          */
/*	29-Nov-1990	KGB	Changed PAMS to DECmessageQ                 */
/*				Added 3 minute TMO on receive               */
/*                                                                          */
/*      04-Apr-1991     RTS     Modififications so VALID can                */
/*                              test the QADAPTER.                          */
/*                                                                          */
/*	18-Apr-1991	KGB	Further changes for DmQA and AS/400 systems */
/*                                                                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#ifdef VMS
/*************************/
/* Standard DMQ includes */
/*************************/

#pragma nostandard
#include pams_c_entry_point
#include pams_c_process
#include pams_c_group  
#include pams_c_type_class
#include pams_c_return_status_def
#include pams_c_symbol_def
#pragma standard

#else
/*******************************/
/* port-o-pams flavor includes */
/*******************************/
#include "p_entry.h"
#include "p_proces.h"
#include "p_group.h"
#include "p_typecl.h"
#include "p_return.h"
#include "p_symbol.h"

#endif

#ifndef VMS
#ifndef USE_QADAPTER
#define USE_QADAPTER 1
#endif
#endif

#ifdef USE_QADAPTER
#include "p_dmqa.h"
#endif

typedef union
{
   long all;
   struct
   {
      short process;
      short group;
   } au;	

} PAMS_ADDR;

/* Override the QADAPTER's definition of struct psb     */
/* We like ours better, and this gets us by any casting */
/* errors flagged by the compiler.                      */

struct PAMS_PSB
{
   short int type;
   short int cd;
   long int del_status;
   long int seq[2];
   long int uma_status;
   long int unused[3];
};

#define PAMS_VALID_S 90
#define PAMS_VALID_R 94

#define WF 1
#define AK 2
#define NN 3

#ifdef VMS
#define MSG_CLAS_TEST_DATA     	1000
#define MSG_CLAS_TEST_CONTROL   1001
#define MSG_TYPE_TEST_SIGNON	1010
#define MSG_TYPE_TEST_REQ   	1020
#define MSG_TYPE_TEST_ACK    	1030
#define MSG_TYPE_TEST_DATA	1040
#define MSG_TYPE_TEST_STATUS    1050
#define MSG_TYPE_TEST_SIGNOFF   1060
#define MSG_CLAS_MRS              28
#define MSG_TYPE_MRS_ACK       - 801
#endif

#define NUM_TEST_ELEMENTS 59
#define MSTR(x,y) case(x): mstrc = y; break;
#define PDEL_UMA_0   0

#ifndef PAMS__SUCCESS
#define PAMS__SUCCESS 1
#endif

#define PAMS__0      0

#define TEST_DESC_SIZE 6		/* RISC pads sizeof to 8 */

typedef struct
{
   long int test_number;
   char delivery;
   char uma;
} TEST_DESCRIP;

#define SUCCESS 1
#define FAILURE 0

#define         MAX_MSG_SIZE      4096		/* Max is 65535 */
char msg_area  [MAX_MSG_SIZE];

char cmd_line_buf[80];

PAMS_ADDR reqpamsid, mypamsid;
short int receiver_group;
short int sender_group;
short int partner_active;

long int test_no = 0;
long int test_status = 0;

int loop  = 1;

/******************************************************/
/* Pattern of expected test results for VAX PAMS V2.5 */
/*  1 means PAMS__SUCCESS                             */
/*  0 means PAMS__BADDELIVERY                         */
/* -1 means PAMS__BADUMA                              */
/******************************************************/

char valid [] = 
{ 	1,
	1, -1, -1, -1, -1, -1,
	0,  0,  0,  0,  0,  0,
	1,  1, -1, -1,  1, -1,
	1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,
	1,  1,  1,  1
};

/******************************************************/
/* Test descriptions ... parallel with array valid [] */
/******************************************************/

TEST_DESCRIP tcary [] =          
{
      	{  0,	0,			0 		},

	{  1,	PDEL_MODE_WF_MEM, 	PDEL_UMA_DISC	},   
	{  2,	PDEL_MODE_WF_MEM,	PDEL_UMA_DISCL	},
	{  3,	PDEL_MODE_WF_MEM,	PDEL_UMA_DLQ	},
	{  4,	PDEL_MODE_WF_MEM,	PDEL_UMA_DLJ	},
	{  5,	PDEL_MODE_WF_MEM,	PDEL_UMA_RTS	},
	{  6,	PDEL_MODE_WF_MEM,	PDEL_UMA_SAF	},

	{  7,	PDEL_MODE_AK_MEM,	PDEL_UMA_DISC	},
	{  8,	PDEL_MODE_AK_MEM,	PDEL_UMA_DISCL	},
	{  9,	PDEL_MODE_AK_MEM,	PDEL_UMA_DLQ	},
	{ 10,	PDEL_MODE_AK_MEM,	PDEL_UMA_DLJ	},
	{ 11,	PDEL_MODE_AK_MEM,	PDEL_UMA_RTS	},
	{ 12,	PDEL_MODE_AK_MEM,	PDEL_UMA_SAF	},

	{ 13,	PDEL_MODE_NN_MEM,	PDEL_UMA_DISC	},
	{ 14,	PDEL_MODE_NN_MEM,	PDEL_UMA_DISCL	},
	{ 15,	PDEL_MODE_NN_MEM,	PDEL_UMA_DLQ	},
	{ 16,	PDEL_MODE_NN_MEM,	PDEL_UMA_DLJ	},
	{ 17,	PDEL_MODE_NN_MEM,	PDEL_UMA_RTS	},
	{ 18,	PDEL_MODE_NN_MEM,	PDEL_UMA_SAF	},

	{ 19,	PDEL_MODE_WF_DQF,	PDEL_UMA_DISC	},
	{ 20,	PDEL_MODE_WF_DQF,	PDEL_UMA_DISCL	},
	{ 21,	PDEL_MODE_WF_DQF,	PDEL_UMA_DLQ	},
	{ 22,	PDEL_MODE_WF_DQF,	PDEL_UMA_DLJ	},
	{ 23,	PDEL_MODE_WF_DQF,	PDEL_UMA_RTS	},
	{ 24,	PDEL_MODE_WF_DQF,	PDEL_UMA_SAF	},

	{ 25,	PDEL_MODE_AK_DQF,	PDEL_UMA_DISC	},
	{ 26,	PDEL_MODE_AK_DQF,	PDEL_UMA_DISCL	},
	{ 27,	PDEL_MODE_AK_DQF,	PDEL_UMA_DLQ	},
	{ 28,	PDEL_MODE_AK_DQF,	PDEL_UMA_DLJ	},
	{ 29,	PDEL_MODE_AK_DQF,	PDEL_UMA_RTS	},
	{ 30,	PDEL_MODE_AK_DQF,	PDEL_UMA_SAF	},

	{ 31,	PDEL_MODE_NN_DQF,	PDEL_UMA_DISC	},
	{ 32,	PDEL_MODE_NN_DQF,	PDEL_UMA_DISCL	},
	{ 33,	PDEL_MODE_NN_DQF,	PDEL_UMA_DLQ	},
	{ 34,	PDEL_MODE_NN_DQF,	PDEL_UMA_DLJ	},
	{ 35,	PDEL_MODE_NN_DQF,	PDEL_UMA_RTS	},
	{ 36,	PDEL_MODE_NN_DQF,	PDEL_UMA_SAF	},
                
	{ 37,	PDEL_MODE_WF_SAF,	PDEL_UMA_DISC	},
	{ 38,	PDEL_MODE_WF_SAF,	PDEL_UMA_DISCL	},
	{ 39,	PDEL_MODE_WF_SAF,	PDEL_UMA_DLQ	},
	{ 40,	PDEL_MODE_WF_SAF,	PDEL_UMA_DLJ	},
	{ 41,	PDEL_MODE_WF_SAF,	PDEL_UMA_RTS	},
	{ 42,	PDEL_MODE_WF_SAF,	PDEL_UMA_SAF	},

	{ 43,	PDEL_MODE_AK_SAF,	PDEL_UMA_DISC	},
	{ 44,	PDEL_MODE_AK_SAF,	PDEL_UMA_DISCL	},
	{ 45,	PDEL_MODE_AK_SAF,	PDEL_UMA_DLQ	},
	{ 46,	PDEL_MODE_AK_SAF,	PDEL_UMA_DLJ	},
	{ 47,	PDEL_MODE_AK_SAF,	PDEL_UMA_RTS	},
	{ 48,	PDEL_MODE_AK_SAF,	PDEL_UMA_SAF	},

	{ 49,	PDEL_MODE_NN_SAF,	PDEL_UMA_DISC	},
	{ 50,	PDEL_MODE_NN_SAF,	PDEL_UMA_DISCL	},
	{ 51,	PDEL_MODE_NN_SAF,	PDEL_UMA_DLQ	},
	{ 52,	PDEL_MODE_NN_SAF,	PDEL_UMA_DLJ	},
	{ 53,	PDEL_MODE_NN_SAF,	PDEL_UMA_RTS	},
	{ 54,	PDEL_MODE_NN_SAF,	PDEL_UMA_SAF	},


	{ 55,	PDEL_MODE_DG_LOG,		0	},
	{ 56,	PDEL_MODE_DG,			0	},
	{ 57,   PDEL_MODE_RTS,			0	},
	{ 58,	PDEL_MODE_WFQ,			0	},
            
};



char *get_sym_str( sym )
long int sym;
{
	char *mstrc;                   
	char symbuf[80];

	switch (sym)
	{
	MSTR(PAMS__0, "0");
	MSTR(PAMS__SUCCESS, "NORMAL");

	MSTR(PAMS__DISC_SUCCESS, "DISC_SUCCESS");
	MSTR(PAMS__DISCL_SUCCESS, "DISCL_SUCCESS");
	MSTR(PAMS__DLJ_SUCCESS, "DLJ_SUCCESS");
	MSTR(PAMS__DLQ_SUCCESS, "DLQ_SUCCESS");
	MSTR(PAMS__RTS_SUCCESS, "RTS_SUCCESS");
	MSTR(PAMS__SAF_SUCCESS, "SAF_SUCCESS");
	MSTR(PAMS__NOMOREMSG, "NOMOREMSG");
	MSTR(PAMS__NOSEND, "NOSEND");
	MSTR(PAMS__WAKEFAIL, "WAKEFAIL");
	MSTR(PAMS__TIMERACT, "TIMERACT");
	MSTR(PAMS__MSGACT, "MSGACT");
	MSTR(PAMS__NO_UMA, "NO_UMA");
	MSTR(PAMS__UMA_NA, "UMA_NA");
	MSTR(PAMS__TRACEBACK, "TRACEBACK");
	MSTR(PAMS__STORED, "STORED");
	MSTR(PAMS__ENQUEUED, "ENQUEUED");
	MSTR(PAMS__UNATTACHEDQ, "UNATTACHEDQ");
	MSTR(PAMS__CONFIRMREQ, "CONFIRMREQ");
	MSTR(PAMS__PROPAGATE, "PROPAGATE");
	MSTR(PAMS__ABORT, "ABORT");
	MSTR(PAMS__BADDECLARE, "BADDECLARE");
	MSTR(PAMS__BADFREE, "BADFREE");
	MSTR(PAMS__TIMEOUT, "TIMEOUT");
	MSTR(PAMS__ACKTMO, "ACKTMO");
	MSTR(PAMS__MSGUNDEL, "MSGUNDEL");
	MSTR(PAMS__EX_Q_LEN, "EX_Q_LEN");
	MSTR(PAMS__POSSDUPL, "POSSDUPL");
	MSTR(PAMS__STUB, "STUB");
	MSTR(PAMS__SENDER_TMO_EXPIRED, "SENDER_TMO_EXPIRED");
	MSTR(PAMS__MRQTBLFULL, "MRQTBLFULL");
	MSTR(PAMS__NOTALLOCATE, "NOTALLOCATE");
	MSTR(PAMS__PNUMNOEXIST, "PNUMNOEXIST");
	MSTR(PAMS__BIGBLKSIZE, "BIGBLKSIZE");
	MSTR(PAMS__MSGTOBIG, "MSGTOBIG");
	MSTR(PAMS__INVALIDID, "INVALIDID");
	MSTR(PAMS__INVFORMAT, "INVFORMAT");
	MSTR(PAMS__INVBUFFPTR, "INVBUFFPTR");
	MSTR(PAMS__INVALIDNUM, "INVALIDNUM");
	MSTR(PAMS__BIGMSG, "BIGMSG");
	MSTR(PAMS__MSGTOSMALL, "MSGTOSMALL");
	MSTR(PAMS__AREATOSMALL, "AREATOSMALL");
	MSTR(PAMS__NOCANSEND, "NOCANSEND");
	MSTR(PAMS__NOTACTIVE, "NOTACTIVE");
	MSTR(PAMS__EXCEEDQUOTA, "EXCEEDQUOTA");
	MSTR(PAMS__BADPRIORITY, "BADPRIORITY");
	MSTR(PAMS__BADDELIVERY, "BADDELIVERY");
	MSTR(PAMS__BADJOURNAL, "BADJOURNAL");
	MSTR(PAMS__BADPROCNUM, "BADPROCNUM");
	MSTR(PAMS__BADTMPPROC, "BADTMPPROC");
	MSTR(PAMS__BADSYNCHNUM, "BADSYNCHNUM");
	MSTR(PAMS__BADTMPSYNCH, "BADTMPSYNCH");
	MSTR(PAMS__BADRECEIVE, "BADRECEIVE");
	MSTR(PAMS__BADTIME, "BADTIME");
	MSTR(PAMS__NOTDCL, "NOTDCL");
	MSTR(PAMS__STATECHANGE, "STATECHANGE");
	MSTR(PAMS__INVUCBCNTRL, "INVUCBCNTRL");
	MSTR(PAMS__NOLINK, "NOLINK");
	MSTR(PAMS__CIRACT, "CIRACT");
	MSTR(PAMS__PROTOCOL, "PROTOCOL");
	MSTR(PAMS__COMMERR, "COMMERR");
	MSTR(PAMS__BADSELIDX, "BADSELIDX");
	MSTR(PAMS__IDXTBLFULL, "IDXTBLFULL");
	MSTR(PAMS__BADPARAM, "BADPARAM");
	MSTR(PAMS__NOMRS, "NOMRS");
	MSTR(PAMS__DISC_FAILED, "DISC_FAILED");
	MSTR(PAMS__DISCL_FAILED, "DISCL_FAILED");
	MSTR(PAMS__DLJ_FAILED, "DLJ_FAILED");
	MSTR(PAMS__DLQ_FAILED, "DLQ_FAILED");
	MSTR(PAMS__DQF_DEVICE_FAIL, "DQF_DEVICE_FAIL");
	MSTR(PAMS__INVUMA, "INVUMA");
	MSTR(PAMS__DQF_FULL, "DQF_FULL");
	MSTR(PAMS__INVJH, "INVJH");
	MSTR(PAMS__LINK_DOWN, "LINK_DOWN");
	MSTR(PAMS__BADSEQ, "BADSEQ");
	MSTR(PAMS__NOTJRN, "NOTJRN");
	MSTR(PAMS__MRS_RES_EXH, "MRS_RES_EXH");
	MSTR(PAMS__NOMOREJH, "NOMOREJH");
	MSTR(PAMS__REJECTED, "REJECTED");
	MSTR(PAMS__NOSUCHPCJ, "NOSUCHPCJ");
	MSTR(PAMS__UCBERROR, "UCBERROR");
	MSTR(PAMS__BADUMA, "BADUMA");
	MSTR(PAMS__BADRESPQ, "BADRESPQ");
	MSTR(PAMS__BADARGLIST, "BADARGLIST");
	MSTR(PAMS__NO_DQF, "NO_DQF");
	MSTR(PAMS__NO_SAF, "NO_SAF");
	MSTR(PAMS__RTS_FAILED, "RTS_FAILED");
	MSTR(PAMS__SAF_DEVICE_FAIL, "SAF_DEVICE_FAIL");
	MSTR(PAMS__SAF_FAILED, "SAF_FAILED");
	MSTR(PAMS__BADLOGIC, "BADLOGIC");
	MSTR(PAMS__SELRCVACT, "SELRCVACT");
	MSTR(PAMS__NOMRQRESRC, "NOMRQRESRC");
	MSTR(PAMS__BADASSIGN, "BADASSIGN");
	MSTR(PAMS__LOGNAME, "LOGNAME");
	MSTR(PAMS__LOGNAME2, "LOGNAME2");
	MSTR(PAMS__LOGNAME3, "LOGNAME3");
	MSTR(PAMS__LOGNAME4, "LOGNAME4");
	MSTR(PAMS__LOGNAME5, "LOGNAME5");
	MSTR(PAMS__NOOPEN, "NOOPEN");
	MSTR(PAMS__BADSCRIPT, "BADSCRIPT");
	MSTR(PAMS__DECLARED, "DECLARED");
	MSTR(PAMS__EXHAUSTBLKS, "EXHAUSTBLKS");
	MSTR(PAMS__INTERNAL, "INTERNAL");
	MSTR(PAMS__QUECORRUPT, "QUECORRUPT");
	MSTR(PAMS__REMQUEFAIL, "REMQUEFAIL");
	MSTR(PAMS__INSQUEFAIL, "INSQUEFAIL");
	MSTR(PAMS__CREATEFAIL, "CREATEFAIL");
	MSTR(PAMS__DCLTMPFAIL, "DCLTMPFAIL");
	MSTR(PAMS__PAMSDOWN, "PAMSDOWN");
	MSTR(PAMS__BADASTPARM, "BADASTPARM");
	MSTR(PAMS__NOQUOTA, "NOQUOTA");
	MSTR(PAMS__NOTPRIMARYQ, "NOTPRIMARYQ");
             
	default:
	   sprintf(symbuf, "UNEXPECTED - %d", sym );
	   mstrc = symbuf;
	}                 

	return (mstrc);
}



char *get_uma_str( uma )
char uma;
{
	char *mstrc;
	char symbuf[80];
                        
	switch (uma)
	{
	MSTR(PDEL_UMA_DISC,  "DISC");
	MSTR(PDEL_UMA_DISCL, "DISCL");
 	MSTR(PDEL_UMA_DLQ,   "DLQ");
	MSTR(PDEL_UMA_DLJ,   "DLJ");
	MSTR(PDEL_UMA_RTS,   "RTS");
	MSTR(PDEL_UMA_SAF,   "SAF");
	MSTR(PDEL_UMA_0,     "0");

	default:
	   sprintf(symbuf, "UNKNOWN_UMA - %d", uma );
	   mstrc = symbuf;
	}                 

	return (mstrc);
}




char *get_del_str( del )
char del;
{
	char *mstrc;
	char symbuf[80];

	switch ( del )
	{
	MSTR(PDEL_MODE_WF_MEM, "WF_MEM");
	MSTR(PDEL_MODE_AK_MEM, "AK_MEM");
	MSTR(PDEL_MODE_NN_MEM, "NN_MEM");
	MSTR(PDEL_MODE_WF_DQF, "WF_DQF");
	MSTR(PDEL_MODE_AK_DQF, "AK_DQF");
	MSTR(PDEL_MODE_NN_DQF, "NN_DQF");
	MSTR(PDEL_MODE_WF_SAF, "WF_SAF");
	MSTR(PDEL_MODE_AK_SAF, "AK_SAF");
	MSTR(PDEL_MODE_NN_SAF, "NN_SAF");
	MSTR(PDEL_MODE_DG,     "DG");
	MSTR(PDEL_MODE_DG_LOG, "DG_LOG");
	MSTR(PDEL_MODE_RTS,    "RTS");
	MSTR(PDEL_MODE_WFQ,    "WFQ");

	default:     
	   sprintf(symbuf, "UNKNOWN_MODE - %d", del );
	   mstrc = symbuf;
	}

	return (mstrc);
}



/* ???????????????????????????????????????????????????????????? */
/* modify this to send back the entire PSB so it can be checked */
/* by the sender...                                             */
/* ???????????????????????????????????????????????????????????? */

long int send_status( tc )
TEST_DESCRIP *tc;
{

   PAMS_ADDR            target;
   PAMS_ADDR            resp;
   char			prio;
   short int		class;
   short int 	   	type;
   char		   	delivery;
   short int		size;
   long  int		timeout;
   struct PAMS_PSB	mypsb;
   char			uma;

   long int istat;   
       
	target.au.group   = sender_group;
	target.au.process = PAMS_VALID_S;
        size              = TEST_DESC_SIZE;
	prio              = 0;
        class             = MSG_CLAS_TEST_CONTROL;
        type              = MSG_TYPE_TEST_STATUS;
        delivery          = PDEL_MODE_WF_MEM;
 	timeout           = 600;

	uma               = PDEL_UMA_DISC;
        resp.all          = reqpamsid.all;      /* full id fails */

	istat = pams_put_msg(
			(char *) tc, 
			&prio, 
			&target.all, 
    			&class, 
       			&type, 
			&delivery, 
			&size,
			&timeout, 
			(struct psb *) &mypsb,
			&uma,	
	       		(long *) &resp.all,
		        0L, 0L, 0L );

	return ( istat );
}



static void status_out( tag, istat, del, uma, mypsb )
char *tag;
long int istat;
char del;
char uma;
struct PAMS_PSB *mypsb;
{ 
	printf(
	   "%-3s: %-18s, %-6s:%-18s, %-5s:%-18s\n",
	   tag,
	   get_sym_str(istat),
	   get_del_str(del), get_sym_str(mypsb->del_status),
	   get_uma_str(uma), get_sym_str(mypsb->uma_status ) );
}
       



static long signon_test()
{
   TEST_DESCRIP *tc;
                   
   PAMS_ADDR            target;
   PAMS_ADDR            resp;
   char			prio;
   short int		class;
   short int 	   	type;
   char		   	delivery;
   short int		size;
   long  int		timeout;
   struct PAMS_PSB	mypsb;
   char			uma;

   long int istat;
   
    extern long         get_ack(long);
    
                    
 	tc = (TEST_DESCRIP *) msg_area;
	tc->test_number   =  1;

	target.au.group   = receiver_group;
	target.au.process = PAMS_VALID_R;
        size              = TEST_DESC_SIZE;
	prio              = 0;
        class             = MSG_CLAS_TEST_CONTROL;

	type 		  = MSG_TYPE_TEST_SIGNON;
        delivery          = PDEL_MODE_WF_MEM;
 	timeout           = 300;
                            
	uma               = PDEL_UMA_DISC;
        resp.all          = reqpamsid.all;      /* full id fails */

	partner_active = 1;

	while (loop )
        {

	   istat = pams_put_msg(
			(char *) tc, 
			&prio, 
			&target.all, 
    			&class, 
       			&type, 
			&delivery, 
			&size,
			&timeout, 
			(struct psb *) &mypsb,
			&uma,	
	       		(long *) &resp.all,
			0L, 0L, 0L );
                  
	   switch ( istat )
	   {
	   case PAMS__NOTACTIVE:
	      printf("Partner not active...\n");

	      fflush (stdin);
	      printf("0 = exit, 1 = wait, 2 = continue w/out partner : ");
#ifdef AS400
	      printf("\n");
#endif
	      cmd_line_buf[0] = 0;

	      if (gets(cmd_line_buf) == '\0') exit(1);
	      partner_active = (short) atoi(cmd_line_buf);

	      switch ( partner_active )
              {
	      case 0:
                 return ( istat );

              case 1:
#ifdef VMS
	         sleep( 10 );
#endif
	         continue;

	      case 2:
	         partner_active = 0;
                 return (long) ( PAMS__SUCCESS );

	      default:
                 continue;
              }

	   case PAMS__REMQUEFAIL:
	      fflush (stdin);
              printf("Remquefail on small buffers prevents signon...\n");
	      printf("0 = exit, 1 = wait, 2 = continue w/out partner: ");
#ifdef AS400
	      printf("\n");
#endif

	      if (gets(cmd_line_buf) == '\0') exit(1);
	      partner_active = (short) atoi(cmd_line_buf);

	      switch ( partner_active )
              {
	      case 0:
                 return ( istat );

              case 1:
#ifdef VMS
	         sleep( 10 );
#endif
	         continue;

	      case 2:
	         partner_active = 0;
                 return (long) ( PAMS__SUCCESS );

	      default:
                 continue;
              }

	   case PAMS__SUCCESS:
	      istat = get_ack( 1 );
              return ( istat );

	   default:
              printf("%s\n", get_sym_str( istat ) );
	      return ( istat );
	   }

	}
    return (long) PAMS__SUCCESS;
}



static long not_supported()
{
   int                local_int;
   PAMS_ADDR          target;
   PAMS_ADDR		resp;
   char			prio;
   short int		class;
   short int 	   	type;
   char		   	delivery;
   short int		size;
   long  int		timeout;
   struct PAMS_PSB	mypsb;
   char			uma;
   long int 		istat;


   printf("Checking unsupported delivery modes and uma's...\n");

   for ( local_int = 1; local_int < NUM_TEST_ELEMENTS; local_int++ )
   {
	if ( (valid[local_int] > 0 ) && (valid[local_int] < 127)) continue;

	target.au.group   = receiver_group;
	target.au.process = PAMS_VALID_S;
        size              = 0;       
	prio              = 0;
        class             = MSG_CLAS_TEST_DATA;
        type              = MSG_TYPE_TEST_DATA;
        delivery          = tcary[local_int].delivery;
 	timeout           = 1800;

	uma               = tcary[local_int].uma;
        resp.all          = reqpamsid.all;      /* full id fails */

	istat = pams_put_msg(
			msg_area, 
			&prio, 
			&target.all, 
    			&class, 
       			&type, 
			&delivery, 
			&size,
			&timeout, 
			(struct psb *) &mypsb,
			&uma,	
	       		(long *) &resp.all , 
	                0L, 0L, 0L);

	if (istat == PAMS__TIMEOUT)
		printf("?TIMEOUT - pams_put_msg failed\n");

	status_out( "Put", istat, delivery, uma, (struct PAMS_PSB *) &mypsb );
   }

   return (long) SUCCESS;
        
}



static long set_test( tno )
long int tno;
{
   TEST_DESCRIP *tc;
                   
   PAMS_ADDR            target;
   PAMS_ADDR            resp;
   char			prio;
   short int		class;
   short int 	   	type;
   char		   	delivery;
   short int		size;
   long  int		timeout;
   struct PAMS_PSB	mypsb;
   char			uma;

   long int istat;
    extern long         get_ack(long);


	if (partner_active != 1 ) return (long) PAMS__SUCCESS;

 	tc = (TEST_DESCRIP *)msg_area;
	tc->test_number   = tno;

	target.au.group   = receiver_group;
	target.au.process = PAMS_VALID_R;
        size              = TEST_DESC_SIZE;
	prio              = 0;
        class             = MSG_CLAS_TEST_CONTROL;
        type              = MSG_TYPE_TEST_REQ;
        delivery          = PDEL_MODE_WF_MEM;
 	timeout           = 300;

	uma               = PDEL_UMA_DISC;
        resp.all          = reqpamsid.all;      /* full id fails */

	istat = pams_put_msg(
			(char *) tc, 
			&prio, 
			&target.all, 
    			&class, 
       			&type, 
			&delivery, 
			&size,
			&timeout, 
			(struct psb *) &mypsb,
			&uma,	
	       		(long *) &resp.all,
	                0L, 0L, 0L );

	if ( istat != PAMS__SUCCESS ) return (long) ( istat );

	istat = get_ack( tno );
	return (long) ( istat );
}



long int send_ack( test_num )
long int test_num;
{

   PAMS_ADDR            target;
   PAMS_ADDR            resp;
   char			prio;
   short int		class;
   short int 	   	type;
   char		   	delivery;
   short int		size;
   long  int		timeout;
   struct PAMS_PSB	mypsb;
   char			uma;

   long int istat;

   TEST_DESCRIP *tc;

 	tc = (TEST_DESCRIP *)msg_area;
	tc->test_number   = test_num;
       
	target.au.group   = sender_group;
	target.au.process = PAMS_VALID_S;
        size              = TEST_DESC_SIZE;
	prio              = 0;
        class             = MSG_CLAS_TEST_CONTROL;
        type              = MSG_TYPE_TEST_ACK;
        delivery          = PDEL_MODE_WF_MEM;
 	timeout           = 600;

	uma               = PDEL_UMA_DISC;
        resp.all          = reqpamsid.all;      /* full id fails */

	istat = pams_put_msg(
			(char *) tc, 
			&prio, 
			&target.all, 
    			&class, 
       			&type, 
			&delivery, 
			&size,
			&timeout, 
			(struct psb *) &mypsb,
			&uma,	
	       		(long *) &resp.all,
			0L, 0L, 0L );

	return ( istat );
}



static long wf_test()
{
    int local_int;
    extern long send_data(int, int);
    
	printf("Testing WF delivery modes ...\n");
        
	for (local_int=1; local_int < NUM_TEST_ELEMENTS; local_int++)
	{
	   if (( valid[local_int] <= 0 ) || (valid[local_int] > 127)) continue;

	   if  ( ( tcary[local_int].delivery == PDEL_MODE_WF_MEM )
	   ||    ( tcary[local_int].delivery == PDEL_MODE_WF_DQF )
	   ||	 ( tcary[local_int].delivery == PDEL_MODE_WF_SAF )
	   ||	 ( tcary[local_int].delivery == PDEL_MODE_WFQ    ) )
	   {                                         
              test_no = local_int;

	      (void) set_test( (long) local_int );

	      (void) send_data( local_int, WF );	
           }
	}

	return (long) PAMS__SUCCESS;
}



static long ak_test()
{
    int local_int;
    extern long send_data(int, int);

	printf("Testing AK delivery modes ...\n");

	for (local_int=1; local_int<NUM_TEST_ELEMENTS; local_int++)
	{
	   if (( valid[local_int] <= 0 ) || (valid[local_int] > 127)) continue;

	   if  ( ( tcary[local_int].delivery == PDEL_MODE_AK_MEM )
	   ||    ( tcary[local_int].delivery == PDEL_MODE_AK_DQF )
	   ||	 ( tcary[local_int].delivery == PDEL_MODE_AK_SAF ) )
	   {                                         
              test_no = local_int;

	      (void) set_test( (long) local_int );

	      (void) send_data( local_int, AK );	
           }
	}

	return ( PAMS__SUCCESS );
}
                                                  


static long nn_test()
{
    int local_int;
    extern long send_data(int, int);

	printf("Testing NN delivery modes ...\n");


	for (local_int=1; local_int<NUM_TEST_ELEMENTS; local_int++)
	{
	   if (( valid[local_int] <= 0 ) || (valid[local_int] > 127)) continue;

	   if  ( ( tcary[local_int].delivery == PDEL_MODE_NN_MEM )
	   ||    ( tcary[local_int].delivery == PDEL_MODE_NN_DQF )
	   ||	 ( tcary[local_int].delivery == PDEL_MODE_NN_SAF )
	   ||	 ( tcary[local_int].delivery == PDEL_MODE_DG     ) 
	   ||	 ( tcary[local_int].delivery == PDEL_MODE_DG_LOG )
	   ||	 ( tcary[local_int].delivery == PDEL_MODE_RTS    ) )
	   {                                         
              test_no = local_int;

	      (void) set_test( (long) local_int );

	      (void) send_data( local_int, NN );	
           }
	}

	return ( PAMS__SUCCESS );
}



static long signoff_test()
{
   TEST_DESCRIP *tc;
                   
   PAMS_ADDR            target;
   PAMS_ADDR            resp;
   char			prio;
   short int		class;
   short int 	   	type;
   char		   	delivery;
   short int		size;
   long  int		timeout;
   struct PAMS_PSB	mypsb;
   char			uma;


   long int istat;
   
 	tc = (TEST_DESCRIP *)msg_area;
	tc->test_number   =  0;

	target.au.group   = receiver_group;
	target.au.process = PAMS_VALID_R;
        size              = TEST_DESC_SIZE;
	prio              = 0;
        class             = MSG_CLAS_TEST_CONTROL;

	type 		  = MSG_TYPE_TEST_SIGNOFF;
        delivery          = PDEL_MODE_WF_MEM;
 	timeout           = 300;
                            
	uma               = PDEL_UMA_DISC;
        resp.all          = reqpamsid.all;      /* full id fails */
                                   
	istat = pams_put_msg(
			(char *) tc, 
			&prio, 
			&target.all, 
    			&class, 
       			&type, 
			&delivery, 
			&size,
			&timeout, 
			(struct psb *) &mypsb,
			&uma,	
	       		(long *) &resp.all,
			0L, 0L, 0L );

	return ( istat );

}



static void do_sender()
{
	fflush (stdin);
	printf("\nDECmessageQ group where receiver will run? : ");
#ifdef AS400
	      printf("\n");
#endif

        if (gets(cmd_line_buf) == '\0') exit(1);
	receiver_group = (short) atoi(cmd_line_buf);

	reqpamsid.all = 0;
	reqpamsid.au.process = PAMS_VALID_S;
    	test_status = pams_dcl_process( 
	   &reqpamsid.all, &mypamsid.all, 0L,0L,0L,0L );
        if (test_status != PAMS__SUCCESS )
	{
	   printf("PAMS_DCL_PROCESS: status = %d, %s\n", 
	      test_status, get_sym_str( test_status ) );
           goto OUT;
	}
	else
	    printf("\nReady...\n\n");

	if (! receiver_group ) receiver_group = mypamsid.au.group;

	test_status = signon_test(); 
        if ( test_status != PAMS__SUCCESS ) goto OUT;

	(void) not_supported();	/* Test unsupported arguments */

	(void) wf_test();		/* test WF delivery modes     */

	(void) ak_test();		/* test AK delivery modes     */

	(void) nn_test();		/* test NN delivery modes     */

	if (partner_active == 1 )
	{
	   test_status = signoff_test();  /* Tell VALID_R to exit */
	   if (test_status != SUCCESS )
	   {
	      printf("status = %d, %s\n",
	         test_status, get_sym_str ( test_status ) );
              goto OUT;
	   }
        }
	goto OUT;

OUT:	
	pams_exit();
}



static void do_receiver()
{
   TEST_DESCRIP curtest;
   TEST_DESCRIP 	*tc     = (TEST_DESCRIP *)msg_area;
   short int		size    = TEST_DESC_SIZE;
   short int   		bufsiz  = sizeof(msg_area );
   long 		tag     = 1;
   char			force_j = PDEL_NO_JRN;

   long int 		istat;

   PAMS_ADDR            source;
   char			prio;
   short int		class;
   short int 	   	type;
   long  int		timeout;
   struct PAMS_PSB	mypsb;
   long int		filter;
   

       	reqpamsid.all = 0;
       	reqpamsid.au.process = PAMS_VALID_R;
    	test_status = pams_dcl_process( 
	   &reqpamsid.all, &mypamsid.all, 0,0,0,0 );
	if (test_status != PAMS__SUCCESS )
	{                  
	   printf("%s\n", get_sym_str( test_status ) );
	   goto OUT;
	}

 	timeout           = 1800;
        filter            = 0;

	while (loop)
	{
	   prio = 0;

	   istat = pams_get_msgw(
	      msg_area, 
	      &prio,                 
              &source.all,
	      &class,
	      &type,
	      &bufsiz,
              &size,
	      &timeout,
	      &filter,
              (struct psb *) &mypsb,
	      (char *) 0, (long *) 0,
	      (char *) 0, 0L, 0L );

	   if ( istat  == PAMS__TIMEOUT           )
		printf("?Timeout occured waiting for message\n");

	   if ( istat  != PAMS__SUCCESS              )
	   {
              printf("%s\n", get_sym_str( test_status ) );
              goto OUT;
           }

	   if ( ( mypsb.del_status == PAMS__CONFIRMREQ ) 
	   ||   ( mypsb.del_status == PAMS__POSSDUPL   ) )
	   {
	      pams_confirm_msg( mypsb.seq, &tag, &force_j );
           }

	   switch ( type )
	   {
	   case MSG_TYPE_TEST_SIGNON:
	      if ( source.au.process != PAMS_VALID_S ) continue;
	      sender_group = source.au.group;

	      (void) send_ack( tc->test_number );
	      break;

           case MSG_TYPE_TEST_REQ:
	      if ( source.au.process != PAMS_VALID_S  ) continue;
	      if ( size   != TEST_DESC_SIZE  ) continue;

              test_no     = tc->test_number;
	      curtest     = *tc;
	      (void) send_ack( test_no );

              break;

	   case MSG_TYPE_TEST_DATA:
	      if ( source.au.process != PAMS_VALID_S  ) continue;

              test_status = send_status( &curtest );
              break;

	   case MSG_TYPE_TEST_SIGNOFF:
	      goto OUT;

	   default:
	      continue;

	   }
	}

	goto OUT;

OUT:
        pams_exit();
}



main(argc,argv)
int argc;
char **argv;
{
   int ians;

#ifdef DMQ__QADAPTER

   /* Let the qadapter use foreign command input        */
   /* to set DMQA__host, DMQA__port, and DMQA__debug    */

   DMQA__argc = argc;
   DMQA__argv = argv;
#endif

   while (loop)
   {
      fflush (stdin);
      printf("\nDeclare: ? ( 1 = sender, 2 = receiver, 3 = exit ) :");
#ifdef AS400
	      printf("\n");
#endif

      if (gets(cmd_line_buf) == '\0') exit(1);
      ians = atoi(cmd_line_buf);

      switch(ians)
      {

      case 1:
#ifndef AS400
	 printf("\n%Sender will only wait 3 minutes for receiver\n");
#endif
         do_sender();
         break;

      case 2:
	 printf("\n%Receiver will only wait 3 minutes for message\n");
         do_receiver();
         break;

      case 3:
         return;

      default:
        break;

      }
   }
}



static long send_data( tno, mode )
int tno;
int mode;
{
                   
   TEST_DESCRIP 	*tc;
   PAMS_ADDR		target;
   PAMS_ADDR            resp;
   PAMS_ADDR            source;

   char			prio;
   short int		class;
   short int 	   	type;
   char		   	delivery;
   short int		size;
   long  int		timeout;
   struct PAMS_PSB	mypsb;
   char			uma;
   long int		filter;

   short int bufsiz = sizeof( msg_area );
                        
   long int istat;   
   long int seq[2];

   int ack_recd, status_recd;

	tc 		  = (TEST_DESCRIP *)msg_area;
	*tc		  = tcary[tno];

	target.au.group   = receiver_group;
	target.au.process = PAMS_VALID_R;
        size              = TEST_DESC_SIZE;
	prio              = 0;
        class             = MSG_CLAS_TEST_DATA;
        type              = MSG_TYPE_TEST_DATA;
        delivery          = tcary[tno].delivery;
 	timeout           = 300;			/* 30 second timeout */

	uma               = tcary[tno].uma;
        resp.all          = reqpamsid.all;      /* full id fails */

	istat = pams_put_msg(
			(char *) tc, 
			&prio, 
			&target.all, 
    			&class, 
       			&type, 
			&delivery, 
			&size,
			&timeout, 
			(struct psb *) &mypsb,
			&uma,	
	       		(long *) &resp.all,
		        0L, 0L, 0L );

	status_out("Put",istat, delivery, uma, (struct PAMS_PSB *) &mypsb);

	if ( istat != PAMS__SUCCESS          ) return ( istat );

	if ( ( ! partner_active ) && ( mode != AK ) ) return ( istat );

	seq[0] = mypsb.seq[0];
	seq[1] = mypsb.seq[1];

	timeout = 300;			/* 30 seconds */
        filter  = 0;

	ack_recd    = 0;
	status_recd = 0;

	while (loop)
	{
	   prio  = 0;
          
	   istat = pams_get_msgw(
	      msg_area, 
	      &prio,                 
              &source.all,
	      &class,
	      &type,  
	      &bufsiz,
              &size,
	      &timeout,
	      &filter,
              (struct psb *) &mypsb,
	      0, 0, (char *) 0, 0L, 0L );

	   switch ( istat )
	   {
           case PAMS__TIMEOUT:
              return ( istat );

           case PAMS__MSGUNDEL:
              continue;			/* A RTS message sneaked in... */

	   case PAMS__SUCCESS:
              break;

           default:
              return ( istat );
           }

	   switch ( class )
	   {
	   case MSG_CLAS_MRS:
	      if ( type              != MSG_TYPE_MRS_ACK ) continue;

              if ( ( seq[0] != mypsb.seq[0] ) 
	      ||   ( seq[1] != mypsb.seq[1] ) )  continue;

	      status_out( "Ack", istat, delivery, uma,
		(struct PAMS_PSB *) &mypsb );

	      ack_recd = 1;
              break;

           case MSG_CLAS_TEST_CONTROL:
	      if ( type       != MSG_TYPE_TEST_STATUS   ) continue;
	      if ( source.all != target.all             ) continue;

	      if ( size       != TEST_DESC_SIZE		) continue;
	      if ( tc->test_number != tno               ) continue;

	      status_recd = 1;
	      break;
	   }

	   /*************************************************/
	   /* Message was either a MRS_ACK or a TEST_STATUS */
	   /* Determine if we want to exit the receive loop */
	   /*************************************************/
	   if ( mode == AK )
           {
	      if ( partner_active )
	      {
                 if ( ( ack_recd) && ( status_recd ) ) break;
              }
	      else
	      {
                 if ( ack_recd ) break;
	      }    
	   }
           else
	      break;
	}  

	return (long) ( PAMS__SUCCESS );
}
                       


static long get_ack( tno )
long int tno;     
{
   TEST_DESCRIP *tc;
                   
   struct PAMS_PSB	mypsb;
   PAMS_ADDR            source;
   PAMS_ADDR            target;
   char			prio;
   short int		class;
   short int 	   	type;
   short int		size;
   long  int		timeout;
   short int   		bufsiz;
   long int             istat;
   long int		filter;


        if ( partner_active != 1 ) return (long) ( PAMS__SUCCESS );
                   
 	tc = (TEST_DESCRIP *) msg_area;

	target.au.group   = receiver_group;
	target.au.process = PAMS_VALID_R;

	/****************************************************/
	/* Get a MSG_TYPE_TEST_ACK back from PAMS_VALID_R */
	/****************************************************/
	bufsiz  = sizeof(msg_area);
	filter  = target.all;
        timeout = 300;

	while (loop)
	{
           prio    = 0;
	   istat = pams_get_msgw(
	      msg_area, 
	      &prio,                 
              &source.all,
	      &class,
	      &type,  
	      &bufsiz,
              &size,
	      &timeout,
	      &filter,
              (struct psb *) &mypsb,
	      0, 0, (char *) 0, 0L, 0L );

	   if ( istat      != PAMS__SUCCESS          ) return (istat);

           if ( class      != MSG_CLAS_TEST_CONTROL )
	   {
		printf("?class not correct\n");
		continue;
	   }

	   if ( type       != MSG_TYPE_TEST_ACK     )
	   {
		printf("?type not correct\n");
		continue;
	   }

	   if ( source.all != target.all            )
	   {
		printf("?source not correct\n");
		continue;
	   }

#ifndef AS400
	   if ( tc->test_number != tno      	    )
	   {
		printf("?test_number not correct (%d <> %d)\n",
		    tc -> test_number, tno);
		continue;
	   }
#endif

	   if ( size       != TEST_DESC_SIZE )
	   {
		printf("?size not correct\n");
		continue;
	   }

           return (long) ( PAMS__SUCCESS );
       	}

       return (long) ( PAMS__SUCCESS );
}
