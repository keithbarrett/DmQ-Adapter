/*  Module p_proces.h    DmQA V1.0                                          */
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


#ifndef P_PROCES_H
#define P_PROCES_H

/*                         Rcv Msg    UCB   Q      Q     Perm    Name */
/*    Process Name     Num  Quota    Send  Type  Owner  Active  Scope */
/*-------------------  ---  -----    ----  ----  -----  ------  ----- */
/*                                                                    */

                            /*   Sample Queues */

#define PAMS_QUEUE_1     1
#define PAMS_QUEUE_2     2
#define PAMS_QUEUE_3     3
#define PAMS_QUEUE_4     4
#define PAMS_QUEUE_5     5
#define PAMS_QUEUE_6     6
#define PAMS_QUEUE_7     7
#define PAMS_QUEUE_8     8
#define PAMS_QUEUE_9     9
#define PAMS_QUEUE_10   10


                /* The Ethernet circuits to test ETH_TEST program */

#define PAMS_ETH_CIRCUIT_1          11
#define PAMS_ETH_CIRCUIT_2          12
#define PAMS_ETH_CIRCUIT_3          13
#define PAMS_ETH_CIRCUIT_4          14
#define PAMS_ETH_CIRCUIT_5          15


/*  SBS_EXAMPLE.EXE uses the following UCB numbers for Optimized Delivery */

#define PAMS_SBS_ETH_CONTROL        74
#define PAMS_SBS_ETH_CHAN1          75
#define PAMS_SBS_ETH_CHAN2          76


        /*---------------------------------------------------------*/
        /*         The following are used for the PAMS Demo        */
        /*---------------------------------------------------------*/

#define PAMS_DEMO_IO_SERVER_1	    101
#define PAMS_DEMO_IO_SERVER_2	    102
#define PAMS_DEMO_IO_SERVER_3	    103
#define PAMS_DEMO_IO_SERVER_4	    104
#define PAMS_DEMO_IO_SERVER_MRQ	    105
#define PAMS_DEMO_ARCHIVE           106
#define PAMS_DEMO_IO_BROADCAST_1   5101
#define PAMS_DEMO_IO_BROADCAST_2   5102


      /*  Processes 90-100 & 150-199 are reserved for PAMS utilities */

#define PAMS_SCREEN_PROCESS         0
#define PAMS_TEMPORARY_Q            0
#define PAMS_SPARE1                 90
#define PAMS_ALL_UCBS               91
#define PAMS_SPARE2                 92
#define PAMS_TIMER_QUEUE            92
#define PAMS_NULL                   93
#define PAMS_INTERNAL1              94
#define PAMS_QTRANSFER_SERVER       95
#define PAMS_INTERNAL2              96
#define PAMS_DEAD_LETTER_QUEUE      96
#define PAMS_NAME_SERVER            97
#define PAMS_MRS_SERVER             98
#define PAMS_AVAIL_SERVER           99
#define PAMS_SBS_SERVER             99
#define PAMS_DECLARE_SERVER         100
#define PAMS_CONNECT_SERVER         100
#define PAMS_QUEUE_SERVER           100
#define PAMS_PAMS_TRANSPORT         100
#define PAMS_PAMS_LOADER            150
#define PAMS_DCL_BY_Q_NAME          151

#endif
