#ifdef VMS
#pragma nostandard
#module DMQ$ROUNDTRIP  "DMQ V1.1-01" 
#pragma standard
#endif

/*************************/
/* Compile time switches */
/*************************/
#ifndef QADAPTER_TEST
#ifdef VMS
#define QADAPTER_TEST         0
#else
#define QADAPTER_TEST         1
#endif
#endif

/****************************************************************************/
/*  Copyright (c) 1990                                                      */
/*  by DIGITAL Equipment Corporation, Maynard, Mass.                        */
/*                                                                          */
/*  This software is furnished under a license and may be used and  copied  */
/*  only  in  accordance  with  the  terms  of  such  license and with the  */
/*  inclusion of the above copyright notice.  This software or  any  other  */
/*  copies  thereof may not be provided or otherwise made available to any  */
/*  other person.  No title to and ownership of  the  software  is  hereby  */
/*  transferred.                                                            */
/*                                                                          */
/*  The information in this software is subject to change  without  notice  */
/*  and  should  not  be  construed  as  a commitment by DIGITAL Equipment  */
/*  Corporation.                                                            */
/*                                                                          */
/*  DIGITAL assumes no responsibility for the use or  reliability  of  its  */
/*  software on equipment which is not supplied by DIGITAL.                 */
/*                                                                          */
/****************************************************************************/



/***************/
/* ROUNDTRIP.C */
/***************/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

#if QADAPTER_TEST
#include "p_dmqa.h"
#endif

struct PAMS_PSB
{
   short int type;
   short int cd;
   long int del_status;
   long int seq[2];
   long int uma_status;
   long int unused[3];
};

typedef union
{
   struct 
   {
      short process;
      short group;
   } au;
   long all;
} PAMS_ADDRESS;

struct SHOW_BUF_STRUCT
{
   long version;
   long del_status;
   long size;
   long reserved[7];
   PAMS_ADDRESS tgt;
   PAMS_ADDRESS orig_tgt;
   PAMS_ADDRESS src;
   PAMS_ADDRESS orig_src;
   long delmode;
   long prio;
};

PAMS_ADDRESS         req_queue_num;        
PAMS_ADDRESS         new_queue_num;        

const long      loop       = 1;

/*********************************/
/* switches                      */
/*********************************/
long int do_trace          = 0;
long int test_function     = 0;
long int do_recoverable    = 0;

/*********************************/
/* other                         */
/*********************************/
long int test_iterations   = 0;
long int test_count        = 0;
long int test_failed	   = 0;

/*********************************/
/* Terminal input buffer         */
/*********************************/
char answer[256];

#define MAX_SIGNED_INTEGER 0x1fffffff
#define MSG_TYPE_TEST_SHUTDOWN -1



/***************/
/* Message set */
/***************/
#define MSGS_IN_SET 10
unsigned char *msg_set[ MSGS_IN_SET ];
short int msg_set_len[] = {50,100,250,500,1000,2000,4000,8000,16000,32000 };

long int msg_number = 0;

/******************/
/* Test data base */
/******************/
typedef struct
{
   long  msg_number;
   short int msg_size;
   short int duplicates;
} TEST_CONTEXT;

typedef struct 
{
   PAMS_ADDRESS target;
   TEST_CONTEXT *test;         /* pointer to array of TEST_CONTEXT records */
} TARGET_RECORD;

#define MAX_TARGETS 10

TARGET_RECORD target_list[ MAX_TARGETS ];
long int target_count      = 0;


/*************************************************************/
/* Control characters that are the same for ASCII and EBCDIC */
/*************************************************************/
#define NUL 0x00
#define FF  0x0c

/*************************/
/* message header format */
/* 20 bytes              */
/* xxxxx.xxx.9999999999. */
/* 012345678901234567890 */
/*           1         2 */
/*************************/
#define ROUNDTRIP_HDR_LEN 21

#define GRP_FIELD_LEN   5
#define QN_FIELD_LEN    3
#define MN_FIELD_LEN   10

#define GRP_OFFSET      0
#define QN_OFFSET       6
#define MN_OFFSET      10

#define GRP_DELIM       5
#define QN_DELIM        9
#define MN_DELIM       20

long int set_pattern(idx, msg_area, mlen )
int idx;
unsigned char *msg_area;
short mlen;
{
   unsigned char ic;
   int i,ilen, j;

   unsigned char pbuf[255];

        memset(pbuf, 0, sizeof(pbuf) );

        /***********************************************************/
        /* Load this machines version of a printable character set */
        /***********************************************************/
        ilen = 0;
        for (i=0; i<sizeof(pbuf)-3; i++ )
        {
           ic = (unsigned char)i;

#ifdef DMQA__EBCDIC
#if DMQA__EBCDIC == 0
	   ic &= 127;
#endif
#else
	   ic &= 127;
#endif
/* If we are NOT on an EBCDIC system, then use values < 128. */
/* (These symbols are created within the DmQ Queue Adapter)	*/

           if ((isprint(ic) != 0) && (ic != '[') && (ic != ']'))
		pbuf[ilen++] = ic;
	   /* We must skip "[]" because their translation is user select-   */
	   /* able, and not predictable within this program		    */
        }

        /**************************************************/
        /* There's enough space for these non print chars */
        /**************************************************/

        pbuf[ilen++] = '\n';
        pbuf[ilen++] = FF; 
        pbuf[ilen++] = NUL; 

        memset(msg_area, 0, ROUNDTRIP_HDR_LEN );

        /**********************************************/
        /* for now, use a simple rotating pattern     */
        /* starting each pattern at a different point */
        /**********************************************/
        j = mlen % 13;

        for (i=ROUNDTRIP_HDR_LEN; i<mlen; i++) 
        {
           msg_area [ i ] = pbuf [ j++];
           if (j > ilen) j = 0;
        }

        return ( 1L );
}



static long int check_ret(stsvalue)
long int stsvalue;
{
   extern char *get_sym_str( long int );

        if (stsvalue & 1) return (1);

        printf ("%s\n", get_sym_str( stsvalue ) );
        return(0);
}

static long int check_psb( lpsb )
struct PAMS_PSB *lpsb;
{
        /****************************************************/
        /* Return 1 iff the message was successfully queued */
        /****************************************************/
        if ( (lpsb->del_status == PAMS__SUCCESS  )
        ||   (lpsb->del_status == PAMS__STORED   )
        ||   (lpsb->del_status == PAMS__ENQUEUED ) )
        {
           return ( 1 );
        }
        else if (lpsb->uma_status == PAMS__SAF_SUCCESS )
        {
           return ( 1 );
        }
        return ( 0 );
}



#define RCV 0L
#define SND 1L

static long int trace_msg( srind, qaddr, ilen, lpsb, mn )
int srind;
long qaddr;
short ilen;
struct PAMS_PSB *lpsb;
long int mn;
{
   char *c1, *c2;
   PAMS_ADDRESS qa;
   extern char *get_sym_str( long int );

        qa.all = qaddr;

        switch (srind)
        {
        case SND:
           c1 = "Send";
           c2 = "to  ";
           break;

        case RCV:
           c1 = "Recv";
           c2 = "from";
           break;

        default:
           return ( 0 );
        }

        printf("%s,Seq:(%8.8x,%-8.8x) %s (%5.5d.%-3.3d) Len:%u mn:%d\n",
                 c1, 
                 lpsb->seq[0],lpsb->seq[1], 
                 c2,
                 qa.au.group, qa.au.process,
                 ilen, mn);

        if ( lpsb->del_status != PAMS__SUCCESS  )
        {
           printf("PSB status: %s\n", get_sym_str( lpsb->del_status ) );

           if ( (lpsb->uma_status != PAMS__UMA_NA )
           &&   (lpsb->uma_status != PAMS__NO_UMA ) )
           {
              printf(" UMA status: %s", get_sym_str( lpsb->uma_status ) );
           }
        }

        return ( 1 );
}




#ifdef VMS
/*************************************************/
/* static main can fit into VMS object libraries */
/* without clashing with other main programs     */
/*************************************************/
static main(argc, argv)
int argc;
char *argv[];

#else

main(argc,argv)
int argc;
char *argv[];

#endif
{
   long int status;

   /**************************/
   /* pams_put_msg variables */
   /**************************/
   PAMS_ADDRESS send_target;
   short send_length   = 0;
   char  send_delivery = 0;
   long  send_timeout  = 0;
   char  send_uma      = 0;

   /****************************************/
   /* pams get msg variables               */
   /****************************************/
   struct SHOW_BUF_STRUCT show_buf;
   long int show_buf_len = sizeof(struct SHOW_BUF_STRUCT);

   struct PAMS_PSB lpsb;
   unsigned char    msg_area[32000]; 

   long int        timeout           = 0;
   PAMS_ADDRESS    source;
   char            prio              = 0;
   short           class             = 0;
   short           type              = 0;
   short int       length            = 0;

   short int       max_len = sizeof(msg_area);
   long  int       sel_addr = 0;

   int i,j;

   PAMS_ADDRESS itgt;

   long int confval = 0;
   char     forcej  = PDEL_DEFAULT_JRN;
   long int msgno;

   long int msgs_outstanding = 0;

   TEST_CONTEXT *tc;

   /*********************************/
   /* Functions used by this module */
   /*********************************/
   extern long int check_ret( long int );
   extern long int get_number( char *, long , long );
   extern long int stamp_message( long int, long int, unsigned char *);
   extern long int decode_message( unsigned char *, long int, long int *);
   extern int      get_target_index( long int );
   extern int      get_test_index(int, long int);
   extern long int check_pattern( int, int, unsigned char *, long int, long );



#ifdef DMQ__QADAPTER
        /**************************************************/
        /* Let the qadapter use command line input        */
        /* to set DMQA__host, DMQA__port, and DMQA__debug */
        /**************************************************/
        DMQA__argc = argc;
        DMQA__argv = argv;
#endif


        do_trace = 1;     /* We like tracing on all the time in this version */

        /*********************************/
        /* Attach to the DECmessageQ bus */
        /*********************************/
        while ( loop )
        {
           req_queue_num.all = 0;
           req_queue_num.au.process = get_number(
           "declare as queue number?  0 = assign temp queue :", 0, 999 );

           status = pams_dcl_process(	&req_queue_num.all, 
					&new_queue_num.all,
					0L, 0L, 0L, 0L);

           if (!check_ret(status)) continue;        /* TRY AGAIN */
           break;
        }

        printf ("Declared as (%d.%d)\n", 
           new_queue_num.au.group, new_queue_num.au.process );


        /**************************************/
        /* Find out if we kick off or receive */
        /**************************************/
        test_function = get_number(
        "1 = sender, 2 = receiver, 3 = shutdown          :", 1, 3 );
        switch (test_function)
        {
        case 1:
           goto DO_SENDER;
        case 2:
           goto DO_MIRROR;
        case 3:
           goto DO_SHUTDOWN;
        }



DO_SENDER:
        /***************************/
        /* Drain the primary queue */
        /***************************/
        while (loop)
        {
           status = pams_get_msg(   msg_area,
				    &prio,
				    &source.all,
				    &class,
				    &type, 
				    &max_len,
				    &length,
				    &sel_addr,
				    (struct psb *) &lpsb, 
				    (char *) &show_buf,
				    &show_buf_len, 
				    (char *)0, 0L, 0L);

           if (status == PAMS__NOMOREMSG) break;

           if (!check_ret(status))
	   {
		(void) pams_exit();
		return(status);
	   }

           if ( ( lpsb.del_status == PAMS__CONFIRMREQ )
           ||   ( lpsb.del_status == PAMS__POSSDUPL   ) )
           {
              status = pams_confirm_msg(lpsb.seq, &confval, &forcej );
           }
        }

        /****************************************/
        /* How many times to execute this test? */
        /****************************************/
        test_iterations = get_number(
	    "Test iterations? 0 = loop forever               :", 
           0, MAX_SIGNED_INTEGER );
        if (test_iterations == 0) test_iterations = MAX_SIGNED_INTEGER;

        /*******************************/
        /* Use "guaranteed" delivery ? */
        /*******************************/
        do_recoverable = get_number(
	"0 = Datagram messages, 1 = recoverable messages :", 0, 1 );

        if (do_recoverable )
        {
           send_delivery = PDEL_MODE_WF_DQF;
           send_uma      = PDEL_UMA_SAF;
        }
        else
        {
           send_delivery = 0;
           send_uma      = 0;
        }        

        /**************************************/
        /* Allocate space for the message set */
        /**************************************/
        for (i=0; i<MSGS_IN_SET; i++)
        {
           msg_set[i] = (unsigned char *) malloc(msg_set_len[i]);
           if (!msg_set[i] ) 
           {
              printf("Cannot allocate %d bytes for %dth buffer\n",
                    msg_set_len[i], i );
              return ( 0 );
           }
        }

        /****************************************/
        /* Initialize message set data patterns */
        /****************************************/
        for (i=0; i<MSGS_IN_SET; i++) 
        {
           (void) set_pattern(i, msg_set[i], msg_set_len[i] );
        }

        /*************************/
        /* Get a list of targets */
        /*************************/
        target_count = get_number( 
           "Number of target queues?                        :", 
	    0,           MAX_TARGETS );

	if (target_count == 0)
	{
	    (void) pams_exit();
	    exit(1);
	}

        for (i=0; i<target_count; i++)
        {
           itgt.au.group = get_number(
           "Target group number ?                           :", 0, 32000 );

           if ( itgt.au.group == 0 ) itgt.au.group = new_queue_num.au.group;

           while (loop)
           {
              itgt.au.process = get_number(
               "Target queue number ?                           :", 1, 999 );

              /***************************************/
              /* Dont allow us to queue to ourselves */
              /***************************************/
              if (itgt.all == new_queue_num.all) continue;
              break;
           }
           target_list[i].target.all = itgt.all;
        }

        /*************************************************************/
        /* Allocate memory to keep track of the test for each target */
        /*************************************************************/
        for (i=0; i<target_count; i++)
        {
           target_list [i].test = malloc( MSGS_IN_SET * sizeof(TEST_CONTEXT));
           if (!target_list[i].test )
           {
              printf("Cannot allocate memory for %dth target\n", i);
              return ( 0 );
           }
           memset( target_list[i].test, 0, MSGS_IN_SET * sizeof(TEST_CONTEXT));

        }

        /************************************/
        /* Wait till user tells us to start */
        /************************************/
        printf("<CR> to start:");
#ifdef AS400
	printf("\n");
#endif
        gets ( answer );


        prio = timeout = 0;

        test_count = 0;
        while (loop)
        {
           test_count++;
           if (test_iterations)
           {
              if (test_count > test_iterations )
	      {
		(void) pams_exit();
		break;
	      }
           }

           msgs_outstanding = 0;

           /***************************************/
           /* For each of the messages in the set */
           /***************************************/
           for (j=0; j<MSGS_IN_SET; j++)
           {
              /*******************************/
              /* For each target in the list */
              /*******************************/
              for (i=0; i<target_count; i++)           
              {
                 /******************************/
                 /* Uniquely stamp the message */
                 /******************************/
                 msg_number++;
                 stamp_message( new_queue_num.all, msg_number, msg_set[j] );

                 msgs_outstanding++;

                 /*******************************/
                 /* Load the test context entry */
                 /*******************************/
                 tc = target_list[i].test + j;

                 tc->duplicates   = 0;
                 tc->msg_size     = msg_set_len[j];
                 tc->msg_number   = msg_number;

                 send_length = msg_set_len [j];

                 /********************/
                 /* Send the message */
                 /********************/
                 status = pams_put_msg(	msg_set[j],
					&prio, 
					&target_list[i].target.all,
					&class, 
					&type, 
					&send_delivery, 
					&send_length,
					&send_timeout, 
					(struct psb *)&lpsb, 
					&send_uma, 
					0L, 0L, 0L, 0L ); 

                 if (!check_ret(status))
		 {
		    (void) pams_exit();
		    exit(status);
		 }

                 if (!check_psb (&lpsb) )
                 {
                    status = lpsb.del_status;

                    trace_msg( SND, target_list[i].target.all, 
                                send_length, &lpsb, 
                                msg_number );

		     (void) pams_exit();
                     return(status);
                 }

                 if (do_trace)
                 {
                    trace_msg( SND, target_list[i].target.all, 
                                send_length, &lpsb, 
                                msg_number );
                 }
              }
           }

           /*********************************************/
           /* Wait for all the messages to be sent back */
           /*********************************************/
           while (msgs_outstanding > 0 )
           {
              prio    = 0;
              timeout = 0;

              /**********************/
              /* Wait for a message */
              /**********************/
              status = pams_get_msgw(	msg_area,
					&prio,
					&source.all,
					&class,
					&type, 
					&max_len,
					&length,
					&timeout,
					&sel_addr,
					(struct psb *) &lpsb, 
					(char *) &show_buf,
					&show_buf_len, 
					(char *)0, 0L, 0L);
              
              if (!check_ret(status))
	      {
		(void) pams_exit();
		exit(status);
	      }

              if (type == MSG_TYPE_TEST_SHUTDOWN )
	      {
		(void) pams_exit();
		break;
	      }

              /*******************************************/
              /* Extract message number from the message */
              /*******************************************/
              decode_message( msg_area, length, &msgno );

              if (do_trace) 
              {
                 trace_msg( RCV, source.all, length, &lpsb, msgno );
              }

              /**************************************************/
              /* Release the message if it was sent recoverably */
              /**************************************************/
              if ( (lpsb.del_status == PAMS__POSSDUPL   ) 
              ||   (lpsb.del_status == PAMS__CONFIRMREQ ) )
              {
                 status = pams_confirm_msg(lpsb.seq, &confval, &forcej );
              }

              /********************************************************/
              /* Search the test data base for a matching test number */
              /********************************************************/
              i = get_target_index( source.all );
              if (i >= 0 )
              {
                 j = get_test_index( i, msgno);
              }
              else
              {
                 j = -1;
              }

              if (j < 0 )
              {
                 printf("Unexpected msg from (%d.%d), length:%d, no:%d\n",
                    source.au.group, source.au.process, length, msgno );
              }
              else
              {
                 /*******************************************************/
                 /* This message matches an entry in the test data base */
                 /*******************************************************/
                 status = check_pattern( i,j,msg_area, length, 0L);
                 if (status != 1 ) test_failed = -1;
		 msgs_outstanding--;
              }

              /**************************/
              /* Clear the input buffer */
              /**************************/
              memset( msg_area, 0, length );        
           }

	   if (test_failed)
	   {
	       printf("\nTest %d FAILED\n", test_count );
	       test_iterations = 1;
	       test_count = test_iterations;
	   }
	   else
	   {
	       printf("\nTest %d completed\n", test_count );
	   }

        }

        printf ("\nTESTS COMPLETE\n" );
	(void) pams_exit();
        return ( 1 );


DO_MIRROR:
        while (loop)
        {
           prio = timeout = 0;

           /**********************/
           /* Wait for a message */
           /**********************/
           status = pams_get_msgw(  msg_area,
				    &prio,
				    &source.all,
				    &class,
				    &type, 
				    &max_len,
				    &length,
				    &timeout,
				    &sel_addr,
				    (struct psb *) &lpsb, 
				    (char *) &show_buf,
				    &show_buf_len, 
				    (char *)0, 0L, 0L);
              
           if (!check_ret(status))
	   {
		(void) pams_exit();
		exit(status);
	   }

           if (type == MSG_TYPE_TEST_SHUTDOWN )
	   {
		(void) pams_exit();
		break;
	   }

           source.all = show_buf.orig_src.all;

           if (do_trace) 
           {
              decode_message( msg_area, length, &msgno );
              trace_msg( RCV, source.all, length, &lpsb, msgno);
           }

           send_delivery = 0;
           send_uma      = 0;
           send_length   = length;

           /**************************************************/
           /* Release the message if it was sent recoverably */
           /**************************************************/
           if ( (lpsb.del_status == PAMS__POSSDUPL   ) 
           ||   (lpsb.del_status == PAMS__CONFIRMREQ ) )
           {
              status = pams_confirm_msg(lpsb.seq, &confval, &forcej );

              send_delivery = PDEL_MODE_WF_DQF;
              send_uma      = PDEL_UMA_SAF;
           }
           
           /*****************************************/
           /* Bounce the message back to the sender */
           /*****************************************/
           status = pams_put_msg(   msg_area,
				    &prio, 
				    &source.all,
				    &class, 
				    &type, 
				    &send_delivery, 
				    &length,
				    &send_timeout, 
				    (struct psb *)&lpsb, 
				    &send_uma, 
				    0L, 0L, 0L, 0L ); 

           if (!check_ret(status))
	   {
		(void) pams_exit();
		exit(status);
	   }

           if (!check_psb(&lpsb) )
           {
              status = lpsb.del_status;

              trace_msg( SND, source.all, send_length, &lpsb, msgno );

	      (void) pams_exit();
              return (status);
           }

           if (do_trace) 
              trace_msg( SND, source.all, send_length, &lpsb, msgno );
        }          

	 (void) pams_exit();
         return ( 1 );


DO_SHUTDOWN:
        while (loop)
        {
           test_function = get_number(
              "Shutdown a test program? 0 = no, 1 = yes    :", 0,1);    

           if (!test_function)
	   {
		(void) pams_exit();
		break;
	   }

           send_target.au.group = get_number(
              "group number?                               :", 0, 32000 );

           send_target.au.process = get_number(
              "queue number?  0 = done                     :", 1, 999 );

           type = MSG_TYPE_TEST_SHUTDOWN;
           send_delivery = 0;
           send_uma      = 0;

           status = pams_put_msg(   msg_area,
				    &prio,
				    (long *) &send_target.all,
				    &class,
				    &type, 
				    &send_delivery, 
				    &length,
				    &send_timeout, 
				    (struct psb *) &lpsb, 
				    &send_uma, 
				    0L, 0L, 0L, 0L ); 
           if (!check_ret(status))
	   {
		(void) pams_exit();
		return(status);
	   }
        } 

	(void) pams_exit();
        return ( 1 );
}





static long int get_number( prompt, low, high )
char *prompt;
long int low;
long int high;
{
   long int iout;

        if ( high < low ) high = MAX_SIGNED_INTEGER;

        while ( loop )
        {
           printf( prompt );
#ifdef AS400
	    printf("\n");
#endif
           gets( answer );
           iout = atoi(answer);
           if ( ( iout<low ) || ( iout>high) ) 
           {
              printf("Enter a number from %d to %d ", low, high );
              continue;
           }
           return ( iout );
        }
}



#define MSTR(x,y) case(x): mstrc = y; break;

#ifndef PAMS__SUCCESS
#define PAMS__SUCCESS 1
#endif

#define PAMS__ZERO    0

static char *get_sym_str( sym )
long int sym;
{
	char *mstrc;                   
	char symbuf[80];

	switch (sym)
	{
	MSTR(PAMS__ZERO, "PAMS__ZERO");
	MSTR(PAMS__SUCCESS, "PAMS__SUCCESS");

	MSTR(PAMS__DISC_SUCCESS, "PAMS__DISC_SUCCESS");
	MSTR(PAMS__DISCL_SUCCESS, "PAMS__DISCL_SUCCESS");
	MSTR(PAMS__DLJ_SUCCESS, "PAMS__DLJ_SUCCESS");
	MSTR(PAMS__DLQ_SUCCESS, "PAMS__DLQ_SUCCESS");
	MSTR(PAMS__RTS_SUCCESS, "PAMS__RTS_SUCCESS");
	MSTR(PAMS__SAF_SUCCESS, "PAMS__SAF_SUCCESS");
	MSTR(PAMS__NOMOREMSG, "PAMS__NOMOREMSG");
	MSTR(PAMS__NOSEND, "PAMS__NOSEND");
	MSTR(PAMS__WAKEFAIL, "PAMS__WAKEFAIL");
	MSTR(PAMS__TIMERACT, "PAMS__TIMERACT");
	MSTR(PAMS__MSGACT, "PAMS__MSGACT");
	MSTR(PAMS__NO_UMA, "PAMS__NO_UMA");
	MSTR(PAMS__UMA_NA, "PAMS__UMA_NA");
	MSTR(PAMS__TRACEBACK, "PAMS__TRACEBACK");
	MSTR(PAMS__STORED, "PAMS__STORED");
	MSTR(PAMS__ENQUEUED, "PAMS__ENQUEUED");
	MSTR(PAMS__UNATTACHEDQ, "PAMS__UNATTACHEDQ");
	MSTR(PAMS__CONFIRMREQ, "PAMS__CONFIRMREQ");
	MSTR(PAMS__PROPAGATE, "PAMS__PROPAGATE");
	MSTR(PAMS__ABORT, "PAMS__ABORT");
	MSTR(PAMS__BADDECLARE, "PAMS__BADDECLARE");
	MSTR(PAMS__BADFREE, "PAMS__BADFREE");
	MSTR(PAMS__TIMEOUT, "PAMS__TIMEOUT");
	MSTR(PAMS__ACKTMO, "PAMS__ACKTMO");
	MSTR(PAMS__MSGUNDEL, "PAMS__MSGUNDEL");
	MSTR(PAMS__EX_Q_LEN, "PAMS__EX_Q_LEN");
	MSTR(PAMS__POSSDUPL, "PAMS__POSSDUPL");
	MSTR(PAMS__STUB, "PAMS__STUB");
	MSTR(PAMS__SENDER_TMO_EXPIRED, "PAMS__SENDER_TMO_EXPIRED");
	MSTR(PAMS__MRQTBLFULL, "PAMS__MRQTBLFULL");
	MSTR(PAMS__NOTALLOCATE, "PAMS__NOTALLOCATE");
	MSTR(PAMS__PNUMNOEXIST, "PAMS__PNUMNOEXIST");
	MSTR(PAMS__BIGBLKSIZE, "PAMS__BIGBLKSIZE");
	MSTR(PAMS__MSGTOBIG, "PAMS__MSGTOBIG");
	MSTR(PAMS__INVALIDID, "PAMS__INVALIDID");
	MSTR(PAMS__INVFORMAT, "PAMS__INVFORMAT");
	MSTR(PAMS__INVBUFFPTR, "PAMS__INVBUFFPTR");
	MSTR(PAMS__INVALIDNUM, "PAMS__INVALIDNUM");
	MSTR(PAMS__BIGMSG, "PAMS__BIGMSG");
	MSTR(PAMS__MSGTOSMALL, "PAMS__MSGTOSMALL");
	MSTR(PAMS__AREATOSMALL, "PAMS__AREATOSMALL");
	MSTR(PAMS__NOCANSEND, "PAMS__NOCANSEND");
	MSTR(PAMS__NOTACTIVE, "PAMS__NOTACTIVE");
	MSTR(PAMS__EXCEEDQUOTA, "PAMS__EXCEEDQUOTA");
	MSTR(PAMS__BADPRIORITY, "PAMS__BADPRIORITY");
	MSTR(PAMS__BADDELIVERY, "PAMS__BADDELIVERY");
	MSTR(PAMS__BADJOURNAL, "PAMS__BADJOURNAL");
	MSTR(PAMS__BADPROCNUM, "PAMS__BADPROCNUM");
	MSTR(PAMS__BADTMPPROC, "PAMS__BADTMPPROC");
	MSTR(PAMS__BADSYNCHNUM, "PAMS__BADSYNCHNUM");
	MSTR(PAMS__BADTMPSYNCH, "PAMS__BADTMPSYNCH");
	MSTR(PAMS__BADRECEIVE, "PAMS__BADRECEIVE");
	MSTR(PAMS__BADTIME, "PAMS__BADTIME");
	MSTR(PAMS__NOTDCL, "PAMS__NOTDCL");
	MSTR(PAMS__STATECHANGE, "PAMS__STATECHANGE");
	MSTR(PAMS__INVUCBCNTRL, "PAMS__INVUCBCNTRL");
	MSTR(PAMS__NOLINK, "PAMS__NOLINK");
	MSTR(PAMS__CIRACT, "PAMS__CIRACT");
	MSTR(PAMS__PROTOCOL, "PAMS__PROTOCOL");
	MSTR(PAMS__COMMERR, "PAMS__COMMERR");
	MSTR(PAMS__BADSELIDX, "PAMS__BADSELIDX");
	MSTR(PAMS__IDXTBLFULL, "PAMS__IDXTBLFULL");
	MSTR(PAMS__BADPARAM, "PAMS__BADPARAM");
	MSTR(PAMS__NOMRS, "PAMS__NOMRS");
	MSTR(PAMS__DISC_FAILED, "PAMS__DISC_FAILED");
	MSTR(PAMS__DISCL_FAILED, "PAMS__DISCL_FAILED");
	MSTR(PAMS__DLJ_FAILED, "PAMS__DLJ_FAILED");
	MSTR(PAMS__DLQ_FAILED, "PAMS__DLQ_FAILED");
	MSTR(PAMS__DQF_DEVICE_FAIL, "PAMS__DQF_DEVICE_FAIL");
	MSTR(PAMS__INVUMA, "PAMS__INVUMA");
	MSTR(PAMS__DQF_FULL, "PAMS__DQF_FULL");
	MSTR(PAMS__INVJH, "PAMS__INVJH");
	MSTR(PAMS__LINK_DOWN, "PAMS__LINK_DOWN");
	MSTR(PAMS__BADSEQ, "PAMS__BADSEQ");
	MSTR(PAMS__NOTJRN, "PAMS__NOTJRN");
	MSTR(PAMS__MRS_RES_EXH, "PAMS__MRS_RES_EXH");
	MSTR(PAMS__NOMOREJH, "PAMS__NOMOREJH");
	MSTR(PAMS__REJECTED, "PAMS__REJECTED");
	MSTR(PAMS__NOSUCHPCJ, "PAMS__NOSUCHPCJ");
	MSTR(PAMS__UCBERROR, "PAMS__UCBERROR");
	MSTR(PAMS__BADUMA, "PAMS__BADUMA");
	MSTR(PAMS__BADRESPQ, "PAMS__BADRESPQ");
	MSTR(PAMS__BADARGLIST, "PAMS__BADARGLIST");
	MSTR(PAMS__NO_DQF, "PAMS__NO_DQF");
	MSTR(PAMS__NO_SAF, "PAMS__NO_SAF");
	MSTR(PAMS__RTS_FAILED, "PAMS__RTS_FAILED");
	MSTR(PAMS__SAF_DEVICE_FAIL, "PAMS__SAF_DEVICE_FAIL");
	MSTR(PAMS__SAF_FAILED, "PAMS__SAF_FAILED");
	MSTR(PAMS__BADLOGIC, "PAMS__BADLOGIC");
	MSTR(PAMS__SELRCVACT, "PAMS__SELRCVACT");
	MSTR(PAMS__NOMRQRESRC, "PAMS__NOMRQRESRC");
	MSTR(PAMS__BADASSIGN, "PAMS__BADASSIGN");
	MSTR(PAMS__LOGNAME, "PAMS__LOGNAME");
	MSTR(PAMS__LOGNAME2, "PAMS__LOGNAME2");
	MSTR(PAMS__LOGNAME3, "PAMS__LOGNAME3");
	MSTR(PAMS__LOGNAME4, "PAMS__LOGNAME4");
	MSTR(PAMS__LOGNAME5, "PAMS__LOGNAME5");
	MSTR(PAMS__NOOPEN, "PAMS__NOOPEN");
	MSTR(PAMS__BADSCRIPT, "PAMS__BADSCRIPT");
	MSTR(PAMS__DECLARED, "PAMS__DECLARED");
	MSTR(PAMS__EXHAUSTBLKS, "PAMS__EXHAUSTBLKS");
	MSTR(PAMS__INTERNAL, "PAMS__INTERNAL");
	MSTR(PAMS__QUECORRUPT, "PAMS__QUECORRUPT");
	MSTR(PAMS__REMQUEFAIL, "PAMS__REMQUEFAIL");
	MSTR(PAMS__INSQUEFAIL, "PAMS__INSQUEFAIL");
	MSTR(PAMS__CREATEFAIL, "PAMS__CREATEFAIL");
	MSTR(PAMS__DCLTMPFAIL, "PAMS__DCLTMPFAIL");
	MSTR(PAMS__PAMSDOWN, "PAMS__PAMSDOWN");
	MSTR(PAMS__BADASTPARM, "PAMS__BADASTPARM");
	MSTR(PAMS__NOQUOTA, "PAMS__NOQUOTA");
	MSTR(PAMS__NOTPRIMARYQ, "PAMS__NOTPRIMARYQ");
             
	default:
	   sprintf(symbuf, "PAMS__UNEXPECTED - %d", sym );
	   mstrc = symbuf;
	}                 

	return (mstrc);
}



long int stamp_message( qa, msg_number, msg_area )
long int qa;
long int msg_number;
unsigned char *msg_area;
{
   PAMS_ADDRESS sender;
        sender.all = qa;

        sprintf( (char *) msg_area, 
            "%5.5d %3.3d %10.10d", 
            sender.au.group, sender.au.process, msg_number );

        msg_area[ GRP_DELIM ] = 0;
        msg_area[  QN_DELIM ] = 0;
        msg_area[  MN_DELIM ] = 0;

        return ( 1 );
}

long int decode_message( msg_area, length, msgno )
unsigned char *msg_area;
long int length;
long *msgno;
{
        if (length < ROUNDTRIP_HDR_LEN ) return ( 0 );

        if (msg_area[ GRP_DELIM ] != 0 ) return ( 0 );
        if (msg_area[ QN_DELIM  ] != 0 ) return ( 0 );
        if (msg_area[ MN_DELIM  ] != 0 ) return ( 0 );

        *msgno  = atoi( (char *) &msg_area[  MN_OFFSET ] );
        
        return ( 1 );
}




int get_target_index( qa )
long int qa;
{
   PAMS_ADDRESS source;
   int i;
        source.all = qa;
        for (i=0; i<target_count; i++)
        {
           if (target_list[i].target.all == source.all ) return ( i );
        }
        return (-1);
}

int get_test_index( tgti, msgno )
int tgti;
long int msgno;
{
   int i;
   TEST_CONTEXT *tc;

        tc = target_list[tgti].test;

        for (i=0; i<MSGS_IN_SET; i++)
        {
           if ( tc->msg_number == msgno ) return ( i );
           tc++;
        }
        return ( -1 );
}




long int check_pattern(tgti, tstj, msg_area, mlen, itime )
int tgti;
int tstj;
unsigned char *msg_area;
long int mlen;
long int itime;
{
   long int mn, grp, qn;
   TEST_CONTEXT *tc;
   int i;
   unsigned char *cp;

        if (mlen < ROUNDTRIP_HDR_LEN ) return ( 0 );

        if (msg_area[ GRP_DELIM ] != 0 ) return ( 0 );
        if (msg_area[ QN_DELIM  ] != 0 ) return ( 0 );
        if (msg_area[ MN_DELIM  ] != 0 ) return ( 0 );

        grp = atoi( (char *) &msg_area[ GRP_OFFSET ] );
        qn  = atoi( (char *) &msg_area[  QN_OFFSET ] );
        mn  = atoi( (char *) &msg_area[  MN_OFFSET ] );

        tc = ( target_list[tgti].test ) + tstj;

        /***********************/
        /* Validate the header */
        /***********************/
        if (mn   != tc->msg_number            ) return ( 0 );
        if (grp  != new_queue_num.au.group    ) return ( 0 );
        if (qn   != new_queue_num.au.process  ) return ( 0 );
        if (mlen != tc->msg_size              ) return ( 0 );

        if ( tc ->duplicates ) return ( 0 );
        (tc->duplicates)++;

        /*****************************/
        /* Validate the data pattern */
        /*****************************/
        cp = msg_set[tstj] + ROUNDTRIP_HDR_LEN;
        for (i=ROUNDTRIP_HDR_LEN; i<mlen; i++) 
        {
           if (msg_area[i] != *cp++ ) 
           {
              printf ("Data pattern failure: msgno:%d, len:%d\n",
                      mn, mlen );
	      printf( "character #%d was %c (%u), expected %c (%u)\n",
		i, msg_area[i], msg_area[i], *(cp-1), *(cp-1));
              return ( - 1 );
           }
        }

        return ( 1 );
}

