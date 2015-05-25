/*  Module sbsmsgde.h   DmQA V1.0                                           */
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

#ifndef SBSMSGDE_H
#define SBSMSGDE_H


#ifndef PAMS_ADDRESS_DEFINED
typedef union
        {
	    unsigned long all;
	    struct
            {
	        unsigned short process;
	        unsigned short group;
	    } au;
        } PAMS_ADDRESS;

#define PAMS_ADDRESS_DEFINED YES
#endif


/********************************************************************/
/*      FORMAT OF SBS REGISTRATION    #BYTES   	NOTES               */
/********************************************************************/
/*	TARGET = PAMS_SBS_SERVER                                    */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_SBS_REG                                   */
/*	MSG    =                                                    */
/*    ==============================                                */
/*            version 			2  	always decimal 20   */
/*    ------------------------------                                */
/*            MOT_Q	  		2	MOT address         */
/*    ------------------------------                                */
/*            distribution_Q 		4       dist PAMS Q         */
/*    ------------------------------                                */
/*            data offset   		2       start data offset   */
/*    ------------------------------            (1 based) (if zero  */
/*                                              no matching done)   */
/*            data operator 		1       data operator       */
/*    ------------------------------                                */
/*            length      		2       length of operand   */
/*    ------------------------------            (if zero no match-  */
/*                                              ing done)           */
/*            operand			4       constant value      */
/*    ------------------------------                                */
/*            request acknowledgement 	1	request ack=1       */
/*    ------------------------------                                */
/*   request sequence gap notification  1	request seqgap=1    */
/*    ------------------------------                                */
/*      request auto dereg on exit      1	request autodereg=1 */
/*    ------------------------------                                */
/*	        EOM                                                 */

        /* PSEL types defined in pams_c_symbol_def  */

struct  SBS_REG
        {
	    short 	    version;  	    /* always = 20 */
	    unsigned short  MOT_Q;	    /* MOT address */
	    PAMS_ADDRESS    distribution_Q; /* PAMS dist. Q */
	    short	    offset;	    /* data offset */
	    char            operator;	    /* data operator  */
	    char 	    length[2];	    /* length of operand */  
	    unsigned char   operand[4];     /* data operand */
	    char	    req_ack;        /* 1 if ACK requested */
	    char	    req_seqgap;     /* 1 if SEQGAP requested */
	    char	    req_autodereg;  /* 1 if AUTO DEREG requested */
        };



/********************************************************************/
/*      FORMAT OF SBS REGISTRATION_EZ #BYTES   	NOTES               */
/********************************************************************/
/*	TARGET = PAMS_SBS_SERVER                                    */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_SBS_REG_EZ                                */
/*	MSG    =                                                    */
/*    ==============================                                */
/*            version 			2  	always decimal 20   */
/*    ------------------------------                                */
/*            MOT_Q	  		2	MOT address         */
/*    ------------------------------                                */
/*            distribution_Q 		4       dist PAMS Q         */
/*    ------------------------------                                */
/*	        EOM                                                 */


struct  SBS_REG_EZ
        {
	    short 	 version;		/* always = 20 */
	    short	 MOT_Q;                 /* starting MOT address */
	    PAMS_ADDRESS distribution_Q;  	/* PAMS dist. */
        };  



/********************************************************************/
/*     FORMAT OF SBS REG(_EZ) REPLY  #BYTES   	NOTES               */
/********************************************************************/
/*	TARGET = source of registrant                               */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_SBS_REG_REPLY                             */
/*	MSG    =                                                    */
/*    ==============================                                */
/*              status  		2  	status (1=ok,-n=ng) */  
/*    ------------------------------                                */
/*      	reg_id  		2  	returned reg id     */
/*    ------------------------------                                */
/*             number_reg               2  	number currently    */
/*    ------------------------------            registered for this */
/*	          EOM                           mot or target       */


struct  SBS_REG_REPLY
        {
	    short	    status;	/* returned status (1=ok, -n = ng */
	    unsigned short  reg_id;	/* returned registration id */
	    short	    number_reg; /* # of registrants left on this mot */
        };                  


/********************************************************************/
/*     FORMAT OF MOT DEREGISTRATION  #BYTES   	NOTES               */
/********************************************************************/
/*	TARGET = PAMS_SBS_SERVER                                    */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_MOT_DEREG                                 */
/*	MSG    =                                                    */
/*    ==============================                                */
/*             version 			2    always decimal 10      */
/*    ------------------------------                                */
/*          registration id		2     reg_id from REG_REPLY */
/*    ------------------------------                                */
/*       request acknowledgement 	1	                    */
/*    ------------------------------                                */
/*	        EOM                                                 */
                                      
struct  SMC_DEREG
        {
	    short	    version;		/* always = 10 */
	    unsigned short  reg_id;		/* registration id */
	    char	    req_ack;		/* = 1 if ACK requested */
        };


/********************************************************************/
/*     FORMAT OF SBS DEREGISTRATION  #BYTES   	NOTES               */
/********************************************************************/
/*	TARGET = PAMS_SBS_SERVER                                    */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_SBS_DEREG                                 */
/*	MSG    =                                                    */
/*    ==============================                                */
/*            version 			2  	always decimal 20   */
/*    ------------------------------                                */
/*             MOT_Q       		2	MOT_address         */
/*    ------------------------------                                */
/*          distribution_Q		4	distribution Q      */
/*    ------------------------------                                */
/*       request acknowledgement 	1	                    */
/*    ------------------------------                                */
/*	       EOM                                                  */
                                                               
struct  SBS_DEREG
        {
	    short	    version;        /* always = 20 */
	    unsigned short  MOT_Q;          /* MOT_address */
	    PAMS_ADDRESS    distribution_Q; /* distribution Q address */
	    char	    req_ack;	    /* = 1 if ACK requested */
        };


/********************************************************************/
/*      FORMAT OF SBS/MOT DEREG ACK    #BYTES   NOTES               */
/********************************************************************/
/*	TARGET = PAMS_SBS_SERVER                                    */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_SBS_DEREG_ACK                             */
/*	MSG    =                                                    */
/*    ==============================                                */
/*              status  	   	2  	status (1=ok,-n=ng) */  
/*    ------------------------------                                */
/*          number reg/mot              2  	# of registrants    */
/*    ------------------------------            left on this mot    */
/*	        EOM                                                 */

struct  SBS_DEREG_ACK
        {
	    short   status;	/* returned status (1=ok, -n = ng */
	    short   number_reg; /* # of registrants left on this mot/target */
        };                  


/********************************************************************/
/*      FORMAT OF SBS PURGE MESSAGE    #BYTES   NOTES               */
/********************************************************************/
/*	TARGET = PAMS_SBS_SERVER                                    */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_SBS_PURGE                                 */
/*	MSG    =                                                    */
/*    ==============================                                */
/*            version 			2  	always decimal 10   */
/*    ------------------------------                                */
/*       request acknowledgement 	1	                    */
/*    ------------------------------                                */

struct  SBS_PURGE
        {
	    short   version;	/* always = 10 */
	    char    req_ack;	/* set to 1 if ACK requested */
        };

/********************************************************************/
/*      FORMAT OF SBS PURGE ACK          #BYTES   NOTES             */
/********************************************************************/
/*	TARGET = PAMS_SBS_SERVER                                    */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_SBS_PURGE_ACK                             */
/*	MSG    =                                                    */
/*    ==============================                                */
/*     	    status  			2  	status (1=ok,-n=ng) */
/*    ------------------------------                                */
/*	    EOM                                                     */

struct  SBS_PURGE_ACK
        {
	    short	 status;	/* returned status (1=ok, -n = ng */
        };                  


/********************************************************************/
/*     FORMAT OF SBS_BS_SEQGAP      #BYTES   	NOTES               */
/********************************************************************/
/*	TARGET = registrant                                         */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_SBS_BS_SEQGAP                             */
/*	MSG    =                                                    */
/*    ==============================                                */
/*            number of msgs            4                           */
/*    ------------------------------                                */
/*            sender's group 	        2                           */         
/*    ------------------------------                                */
/*            mot number                2                           */
/*    ------------------------------                                */
/*            channel number            2                           */
/*    ------------------------------                                */
/*	        EOM                                                 */
                                      
struct  SBS_BS_SEQGAP
        {
	    long	    num_msgs_missing;
	    unsigned short  sender_group;
	    unsigned short  mot;
	    unsigned short  channel;
        };


/********************************************************************/
/*      FORMAT OF MSG_TYPE_SBS_INIT  #BYTES   	NOTES               */
/********************************************************************/
/*	TARGET = PAMS_SBS_SERVER                                    */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_SBS_INIT                                  */
/*	MSG    =                                                    */
/*    ==============================                                */
/*            version 			2  	always decimal 30   */
/*    ------------------------------                                */
/*            group	  		2	short               */
/*    ------------------------------                                */
/*         first_temp_queue 		2       short               */
/*    ------------------------------                                */
/*	        EOM                                                 */

struct  SBS_INIT
        {
	    short   version;		/* always = 30 */
	    short   group;              /* this PAMS group number */
	    short   first_temp_queue;   /* first temporary queue for group */
        };  
                          

#endif
