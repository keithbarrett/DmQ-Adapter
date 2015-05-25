/*** module DMQATEST "V1.0-00"						    */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*  Copyright (c) 1991                                                      */
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
/*                                                                          */
/*  DECmessageQ Expertise Center                                            */
/*                                                                          */
/*                                                                          */
/*  This is a simple test program for the DECmessageQ Queue Adapter. It is  */
/*  basically the SIMPLE_CLIENT program provided with the DECmessageQ       */
/*  product, with some minor modifications for multi-platform portability   */
/*  and use with the queue adapter. Automatic startup of the server process */
/*  has also been removed.                                                  */
/*                                                                          */
/*  Note: For this program to function; the DmQ Queue Adapter Server        */
/*        process must already be running on the host system.               */
/*                                                                          */
/*                                                                          */
/*  IDENT HISTORY:                                                          */
/*                                                                          */
/*  Version     Date    Who     Description                                 */
/*                                                                          */
/*  V1.0    08-Mar-1991 kgb     Initial Release; Keith Barrett              */
/*                                                                          */
/*                                                                          */


/****************************************************************************/
/*                                                                          */
/* This program provides an example of a very simplistic client             */
/*                                                                          */
/*                +---------------------------+                             */
/*                |  Attach program to Bus    |                             */
/*                |  using PAMS_ATTACH_Q      |                             */
/*                +---------------------------+                             */
/*                              |                                           */
/*                +---------------------------+                             */
/*                | Initialize Variables, etc.|                             */
/*                +---------------------------+                             */
/*            +---------------->|                                           */
/*            |   +---------------------------+     +----------+            */
/*            |   |   Wait for user to enter  |     |          |            */
/*            |   |   a message.              |---->|   exit   |            */
/*            |   +---------------------------+     +----------+            */
/*            |                 |                                           */
/*            |   +---------------------------+                             */
/*            |   |   Send user message to    |                             */
/*            |   | server (ourself) using    |                             */
/*            |   |       PAMS_PUT_MSG        |                             */
/*            |   +---------------------------+                             */
/*            |                 |                                           */
/*            |   +---------------------------+                             */
/*            |   |   Wait for a reply from   |                             */
/*            |   | the server (our msg.)     |                             */
/*            |   |       PAMS_GET_MSGW       |                             */
/*            |   +---------------------------+                             */
/*            |                 |                                           */
/*            +---------<-------+                                           */
/*                                                                          */
/****************************************************************************/


        /* Include standard "C" and system definition files. */

#include <stdio.h>                        /* Standard I/O definitions         */
#include <stddef.h>
#include <string.h>

#ifdef VMS
#include <ssdef.h>                      /* VMS System Service Return Codes  */
#include <descrip.h>                    /* Character descriptor macros      */


            /* Include PAMS specific definition files. */

#pragma nostandard
#include pams_c_entry_point             /* PAMS function type declarations  */
#include pams_c_process                 /* Known Queue number definitions.  */
#include pams_c_group                   /* Known Group number definitions   */
#include pams_c_type_class              /* Generic Type/Class definitions   */
#include pams_c_return_status_def       /* PAMS return status definitions   */
#include pams_c_symbol_def              /* Generic PSEL/DSEL definitions    */
#pragma standard
#else
#include "p_entry.h"			/* PAMS function type declarations  */
#include "p_proces.h"                   /* Known Queue number definitions.  */
#include "p_group.h"                    /* Known Group number definitions   */
#include "p_typecl.h"                   /* Generic Type/Class definitions   */
#include "p_return.h"                   /* PAMS return status definitions   */
#include "p_symbol.h"                   /* Generic PSEL/DSEL definitions    */
#endif

#include "p_dmqa.h"			/* QUEUE ADAPTER */

#ifdef DMQ_QADAPTER
#define BUF_SIZE DMQA__MSGMAX
#endif

#ifdef VMS
#ifndef BUF_SIZE
#define BUF_SIZE 32000
#endif
#endif

#ifdef ultrix
#ifndef BUF_SIZE
#define BUF_SIZE 32000
#endif
#endif

#ifndef BUF_SIZE
#define BUF_SIZE 2048
#endif

        /* Define data type for PAMS target/source addresses.  */

typedef union
   {
   long int      all;
   struct
      {
      short int  queue;
      short int  group;
      } au;
   } que_addr;

            /* Declare local variables, pointers, and arrays */

static char             server;                 /* Answer from server prompt*/
static long int         status;                 /* Completion status code   */

static long int         attach_mode =
                         PSYM_ATTACH_TEMPORARY; /* Temp queue               */
static que_addr         queue_number;           /* Actual queue number      */
static long int         queue_type =
                         PSYM_ATTACH_PQ;        /* Type of queue            */
static char             queue_name[14] =
                         "SIMPLE_CLIENT";       /* Our name                 */
static long int         queue_name_len = 13;    /* Name length              */

static long int         search_list[3] = { PSEL_TBL_PROC,
                                           PSEL_TBL_GRP,
                                           PSEL_TBL_DNS_LOW };

static long int         search_list_len = 3;

struct psb              psb;                    /* PAMS status block        */

#ifdef VMS
$DESCRIPTOR (server_process, "SIMPLE_SERVER");  /* Server process name      */
#endif

        /* Define outbound ("put") message variables and arrays */

static que_addr         source;                 /* Source queue address     */
static char             put_buffer[BUF_SIZE+1]; /* Message buf. +1 null byte*/
static short int        put_class;              /* Message class code       */
static char             put_delivery;           /* Delivery mode            */
static short int        put_priority;           /* Message priority         */
static long int         put_resp_que;           /* Response queue           */
static short int        put_size;               /* Message size             */
static long int         put_timeout;            /* Time-out for blocked msg */
static short int        put_type;               /* Message type code        */
static char             put_uma;                /* Undeliverable msg action */


        /* Define inbound ("get") message variables and arrays */

static que_addr         target;                 /* Target queue address     */
static char             get_buffer[BUF_SIZE+1]; /* Message buf. +1 null byte*/
static short int        get_class;              /* Message class code       */
static short int        get_priority;           /* Message priority         */
static long int         get_select;             /* Message selection mask   */
static short int        get_size;               /* Message size             */
static long int         get_timeout;            /* Time-out for blocked msg */
static short int        get_type;               /* Message type code        */


main (argc,argv)
int argc;
char **argv;
{
#ifndef DMQ__QADAPTER
	long		local_long  = 0;
#endif
	short		local_short = 0;
        

   printf ("\n\nDMQATEST - Simple test program\n");


#ifdef DMQ__QADAPTER
        DMQA__argc = argc;
        DMQA__argv = argv;

        fflush (stdin);
        printf ("\n\nDo you wish to send inquiry message?\n>");

	put_buffer[0] = 0;
        if (gets(put_buffer) == '\0') exit(1);

        if ((put_buffer[0] == 'y') || (put_buffer[0] == 'Y'))
 	{
		status = DMQA__test(get_buffer, -1, &local_short, 0, 0L);
        	printf("DMQA__test returned status %d\n", status);
 	}
#endif


   /* Call PAMS attach Q attach our program a temporary queue */
   /* by name and assign our queue number. */

   status = pams_attach_q (&attach_mode, &queue_number.all, &queue_type,
                           queue_name, &queue_name_len, 0, 0, 0, 0, 0); 

   if (status != PAMS__SUCCESS)
   {
      printf ("\nError returned by PAMS_ATTACH_Q code= '%d'\n", status);
      exit (status);
   }

            /* Attach queue was successful continue */

   printf ("\nAttached to %d.%d", queue_number.au.group, 
                                  queue_number.au.queue);

   /* Ask the user if he/she wishes to start the SIMPLE_CLIENT */
   /* to receive the messages. If not, then send the messages to ourself */

#ifndef DMQ__QADAPTER
   do
   {
      printf ("\nDo your wish to start the server process [Y/N]? ");
      server = toupper(getc(stdin));
   } while (server != 'Y' && server != 'N');

   /* If the user answered "Y"es, then use the system() function */
   /* to spawn a subprocess to run the DCL command procedure to start */
   /* the server.  Else, set the target queue to our own queue. */

   if (server == 'Y')
   {
      system ("@DMQ$EXAMPLES:SIMPLE_RUN_SERVER");
      printf ("\nStarting SIMPLE_SERVER process.");

      /* Use PAMS_LOCATE_Q find the address of our server.  The server */
      /* process must attach to the SPARE1 queue by name. */

       local_long = PSYM_WF_RESP;
       status = pams_locate_q (	"SPARE1",
				&queue_name_len,
				&target.all,
				&local_long,
                                0L,
                                0L,
				search_list,
				&search_list_len,
				0);

      if (status != PAMS__SUCCESS)
      {
         printf ("\nError returned by PAMS_LOCATE_Q code= '%d'\n", status);
         exit (status);
      }
   }
   else
#else
  if (1)
#endif
   {
      target.all = queue_number.all;
#ifndef DMQ__QADAPTER
      printf ("\nServer not started.  Messages will be sent to ourself.");
#endif
   }

   /* Initialize the variables that we will be using for messaging. */

   get_priority = 0;                    /* Receive all messages              */
   get_timeout	= 600;                  /* 60 second time-out on rcv         */
   get_select	= 0;                    /* No special selection mask         */

   put_class	= 1;                    /* Send class 1 message              */
   put_priority = 0;                    /* Send at standard priority         */
   put_resp_que = 0;                    /* Response queue - default          */
   put_type	= -123;                 /* Msg type is user defined          */
   put_timeout	= 0;                    /* Accept standard time-out          */
   put_delivery	= PDEL_MODE_NN_MEM;     /* No notify, memory queuing         */
   put_uma	= PDEL_UMA_DISC;        /* Discard if undeliverable          */
                        
   /* Prompt for a message, send it to the server and wait for  */
   /* a response from the server. */

   while (1==1)
   {
      /* Clear type-ahead buffer, then prompt for and read the message. */
      /* If EOF or EXIT, we are done, break out of the loop. */

      fflush (stdin);
      printf ("\n\nEnter message or enter EXIT to exit\n>");
      if (gets(put_buffer) == '\0') break;
      if (strncmp(put_buffer, "EXIT", 4) == 0) break;
      if (strncmp(put_buffer, "exit", 4) == 0) break;

      /* Set put message size to buffer length (+1 to include null). */

      put_size = strlen(put_buffer)+1;

      /* Send the message to the target queue.  If an error is */
      /* returned, report it and exit, otherwise display class & type. */

      status = pams_put_msg (   put_buffer,
                                (char *) &put_priority,
                                &target.all,
                                &put_class,
                                &put_type,
                                &put_delivery,
                                &put_size,
                                &put_timeout,
                                &psb,
                                &put_uma,
                                &put_resp_que,
                                0L,
                                0L,
                                0L); 
 
      if (status != PAMS__SUCCESS)
      {
         printf ("Error returned by PAMS_PUT_MSG code=%d\n", status);
         exit (status);
      }	

      printf ("\nCLIENT: Sent Msg to %d.%d class='%d', type='%d'\n",
              target.au.group, target.au.queue, 
              put_class, put_type);

      /* Now wait for the server process to send the reply message. */
      /* If time-out statuc, report it and continue.  If any other */
      /* error, report it and exit.  Otherwise, display what was received. */

      local_short = BUF_SIZE;
      status = pams_get_msgw (  get_buffer,
                                (char *) &get_priority,
                                &source.all,
                                &get_class,
                                &get_type,
                                &local_short,
                                &get_size, 
                                &get_timeout,
                                &get_select,
                                &psb,
                                (char *) 0,
                                0,
                                (char *) 0,
                                0L,
                                0L); 

      if (status == PAMS__TIMEOUT)
         printf ("PAMS_GET_MSGW Timeout\n");

      else if (status != PAMS__SUCCESS)
      {
         printf ("\nError returned by PAMS_GET_MSGW code=%d\n", status);
         exit (status);
      }
      else
      {
         printf ("\nCLIENT: Received from %d.%d  class='%d', type='%d'\n",
                 source.au.group, source.au.queue, 
                 get_class, get_type);
         printf ("Message='%s'\n", get_buffer);
      }
   }
   /* Go back and wait for user to enter another message to send. */

   /* Clean-up and exit. */

#ifndef DMQ__QADAPTER
   /* If the server process was started, then use the system() function */
   /* to stop the server process using the DCL STOP command. */
   if (server == 'Y')
   {
      status = SYS$FORCEX (0, &server_process, 0);
      printf ("\nStopping SIMPLE_SERVER process.");
   }
#endif

   /* Detach from message bus. */

   (void) pams_exit();

   /*  Tell the user we're done, then exit. */

   printf ("\n\nProgram finished\n");
   exit (PAMS__SUCCESS);
}

