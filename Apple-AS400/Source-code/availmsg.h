/*  Module availmsg.h DmQA V1.0                                             */
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
/*                                                                          */


#ifndef AVAILMSG_H
#define AVAILMSG_H

#ifndef	GEN_PAMS_V1
#define	GEN_PAMS_V1 0			/* Generate current structures      */
#endif

#if GEN_PAMS_V1
struct  PAMS_ADDR
        {
	    unsigned short process;
	    unsigned short group;
        };

struct  AVAIL_REG
        {
            short            version;                /* always = 20 */
            struct PAMS_ADDR target_PQ;
            struct PAMS_ADDR distribution_Q;
        };

#else

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


struct  AVAIL_REG
        {
            short        version;                /* always = 20 */
            PAMS_ADDRESS target_PQ;
            PAMS_ADDRESS distribution_Q;
        };
#endif



/********************************************************************/
/*     FORMAT OF   AVAIL REG         #BYTES   	NOTES               */
/********************************************************************/
/*	TARGET = registrant                                         */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_AVAIL_REG                                 */
/*	MSG    =                                                    */
/*    ==============================                                */
/*              version                     2    always decimal 20  */
/*    ------------------------------                                */
/*         target process  PQ               4                       */
/*    ------------------------------                                */
/*            distribution Q                4                       */   
/*    ------------------------------                                */
/*	         EOM                                                */


/********************************************************************/
/*     FORMAT OF   AVAIL  DEREG        #BYTES   	NOTES       */
/********************************************************************/
/*	TARGET = registrant                                         */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_AVAIL_DEREG                               */
/*	MSG    =                                                    */
/*    ==============================                                */
/*              version                   2     always decimal 20   */
/*    ------------------------------                                */
/*          target process PQ             4                         */
/*    ------------------------------                                */
/*            distribution Q              4                         */
/*    ------------------------------                                */
/*        request acknowledgement 	  1 	                    */
/*    ------------------------------                                */
/*	         EOM                                                */


struct  AVAIL_DEREG
        {  
	    short	 version;		/* always = 20 */
	    PAMS_ADDRESS target_PQ;
    	    PAMS_ADDRESS distribution_Q;
	    char	 req_ack;		/* = 1 if ACK requested */
        };


/************************************************************************/
/*     FORMAT OF AVAIL REG REPLY      #BYTES   	NOTES                   */
/************************************************************************/
/*	TARGET = source of registrant                                   */
/*	CLASS  = MSG_CLAS_PAMS                                          */
/*	TYPE   = MSG_TYPE_AVAIL_REG_REPLY                               */
/*	MSG    =                                                        */
/*    ==============================                                    */
/*      	status  		 2  	status (1=ok,-n=ng)     */
/*    ------------------------------                                    */
/*      	reg_id  		 2  	returned reg id         */
/*    ------------------------------                                    */
/*     	      number_reg                 2      # currently registered  */
/*    ------------------------------            for this mot or target  */
/*	         EOM                                                    */
         
struct  AVAIL_REG_REPLY
        {
            short	    status;	/* returned status (1=ok, -n = ng */
	    unsigned short  reg_id;	/* returned registration id */
	    short	    number_reg; /* # of registrants left on this mot */
        };                  


/********************************************************************/
/*     FORMAT OF   AVAIL            #BYTES   	NOTES               */
/********************************************************************/
/*	TARGET = distribution address                               */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_AVAIL                                     */
/*	MSG    =                                                    */
/*    ==============================                                */
/*         target process  PQ          4                            */
/*    ------------------------------                                */
/*	        EOM                                                 */

struct AVAIL
       {  
            PAMS_ADDRESS target_PQ;
       };                

/********************************************************************/
/*     FORMAT OF Q_UNAVAIL            #BYTES   	NOTES               */
/********************************************************************/
/*	TARGET = distribution address                               */
/*	CLASS  = MSG_CLAS_PAMS                                      */
/*	TYPE   = MSG_TYPE_UNAVAIL                                   */
/*	MSG    =                                                    */
/*    ==============================                                */
/*          target process PQ           4                           */
/*    ------------------------------                                */
/*	        EOM                                                 */

struct  UNAVAIL
        {  
	    PAMS_ADDRESS target_PQ;
        };


#endif
