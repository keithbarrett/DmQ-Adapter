/*  Module p_typecl.h    DmQA V1.0                                          */
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


#ifndef P_TYPECL_H
#define P_TYPECL_H

 
/****************************************************************/
/**                                                            **/
/**        MRS Class and Type codes (800-899)                  **/
/**                                                            **/
/****************************************************************/
 
#define MSG_CLAS_MRS    28           /* * define MRS class **/

 
                /** define MRS type codes **/

#define MSG_TYPE_MRS_DQF_TRANSFER       -700
#define MSG_TYPE_MRS_DQF_TRANSFER_ACK   -701
#define MSG_TYPE_MRS_DQF_TRANSFER_REP   -702
#define MSG_TYPE_MRS_ACK 	        -801
#define MSG_TYPE_MRS_CNF 	        -840
#define MSG_TYPE_MRS_CNF_ACK 	        -841
#define MSG_TYPE_MRS_CNF_NAK 	        -842
#define MSG_TYPE_MRS_DEBUG_ON	        -850
#define MSG_TYPE_MRS_DEBUG_OFF	        -851
#define MSG_TYPE_MRS_SET_PCJ            -880
#define MSG_TYPE_MRS_SET_PCJ_REP        -881
#define MSG_TYPE_MRS_SET_DLJ	        -882
#define MSG_TYPE_MRS_SET_DLJ_REP        -883
 

    /****************************************************************/
    /**                                                            **/
    /**	            PAMS CLASS AND TYPE CODES( 900-999)	           **/
    /**                                                            **/
    /****************************************************************/
 
#define MSG_CLAS_PAMS 29	  /* define PAMS class */
 

                /** define PAMS type codes **/

#define MSG_TYPE_TIMER_ADD               -960
#define MSG_TYPE_TIMER_CANCEL            -961
#define MSG_TYPE_DEL_STATUS	         -983
#define MSG_TYPE_TIMER_EXPIRED           -900
#define MSG_TYPE_DECLARE_SQ              -980
#define MSG_TYPE_UNDECLARE_SQ            -981
#define MSG_TYPE_ALLOCATED_SQ            -982
#define MSG_TYPE_MSG_STATUS              -983
#define MSG_TYPE_GET_PROC_INFO           -984
#define MSG_TYPE_GET_PROC_INFO_REP       -985
#define MSG_TYPE_ENABLE_NOTIFY           -990
#define MSG_TYPE_DISABLE_NOTIFY          -991  
#define MSG_TYPE_PROCESS_DCL             -992  
#define MSG_TYPE_PROCESS_EXIT            -993  
#define MSG_TYPE_LIST_DCLS               -994  
#define MSG_TYPE_LIST_ALL_ENTRYS         -995
#define MSG_TYPE_LIST_ALL_CONNECTIONS    -996
#define MSG_TYPE_LIST_ALL_GROUPS         -997
#define MSG_TYPE_LINK_LOST               -998
#define MSG_TYPE_LINK_COMPLETE           -999
 

        /**************************************************************/
        /*                                                            */
        /* 	  DEFINE SBS type codes for the SBS_SERVER            */
        /*                                                            */
        /**************************************************************/

#define MSG_TYPE_SBS_REG              -1150  /* reg message long form        */
#define MSG_TYPE_SBS_REG_EZ           -1173  /* reg record short form (v2)   */
#define MSG_TYPE_SBS_REG_REPLY        -1152  /* registration o.k.            */
#define MSG_TYPE_SBS_REG_EZ_REPLY     -1153  /* registration o.k.            */
#define MSG_TYPE_MOT_DEREG            -1154  /* de-register request (v1)     */ 
#define MSG_TYPE_SBS_DEREG            -1174  /* de-register request (v2)     */ 
#define MSG_TYPE_SBS_DEREG_ACK        -1155  /* de-register repsonse o.k.    */
#define MSG_TYPE_SBS_PURGE            -1157  /* purge registration table     */
#define MSG_TYPE_SBS_PURGE_ACK        -1158  /* purge registration response  */
#define MSG_TYPE_SBS_DUMP_TABLES      -1162  /* process, reg, & group tables */
#define MSG_TYPE_SBS_BS_SEQGAP        -1166  /* process, reg, & group tables */


    /**************************************************************/
    /*                                                            */
    /* 	  DEFINE AVAIL type codes for the AVAIL_SERVER            */
    /*                                                            */
    /**************************************************************/

#define MSG_TYPE_AVAIL_REG            -1180  /* avail reg message   */
#define MSG_TYPE_AVAIL_DEREG          -1181  /* avail dereg message */
#define MSG_TYPE_AVAIL_REG_REPLY      -1182  /* registration o.k.   */
#define MSG_TYPE_AVAIL                -1183  /* process available   */
#define MSG_TYPE_UNAVAIL              -1184  /* process unavailable */


    /****************************************************************/
    /**                                                            **/
    /**       NAME_SERVER - class: PAMS                            **/
    /**                    type:  codes (1190-1199)                **/
    /**                                                            **/
    /****************************************************************/
 
#define MSG_TYPE_NAME_ADD             -1190      /* * add name               */
#define MSG_TYPE_NAME_ADD_REP         -1191      /* * add name reponse       */
#define MSG_TYPE_NAME_DEL             -1192      /* * delete name            */
#define MSG_TYPE_NAME_DEL_REP         -1193      /* * delete name response   */
#define MSG_TYPE_NAME_INQ_NAME        -1194      /* * inquire by name        */
#define MSG_TYPE_NAME_INQ_NAME_REP    -1195      /* * inq by name response   */
#define MSG_TYPE_NAME_INQ_ID          -1196      /* * inq by id (local tbl)  */
#define MSG_TYPE_NAME_INQ_ID_REP      -1197      /* * inquire by id response */
#define MSG_TYPE_NAME_DUMP            -1198      /* * dump tables request    */
#define MSG_TYPE_NAME_INQ_LCL_ID      -1196      /* * inq by id (local tbl)  */
#define MSG_TYPE_NAME_INQ_GBL_ID      -1199      /* * inq by id (global tbl) */

 
    /****************************************************************/
    /**                                                            **/
    /**       ETHERNET   Class and Type codes (1000-1010)          **/
    /**                                                            **/
    /****************************************************************/
 
#define MSG_CLAS_ETHERNET  100      /* * define Ethernet class */
 
                    /** define Ethernet type codes*/

#define MSG_TYPE_E_CONNECT            -1000    /* Initiate connection         */
#define MSG_TYPE_E_DISCONNECT         -1001    /* shutdown circuit            */
#define MSG_TYPE_E_INITIALIZE         -1002    /* set ethernet character.     */
#define MSG_TYPE_E_CONNECT_COMPLETE   -1003    /* circuit established         */
#define MSG_TYPE_E_CONNECT_REJECTED   -1004    /* partner rejected circuit    */
#define MSG_TYPE_E_PARTNER_DISC       -1005    /* partner disconnected cir.   */
#define MSG_TYPE_E_IO_ERROR           -1006    /* bad status from QIO$        */
#define MSG_TYPE_E_PROTOCOL_ERROR     -1007    /* unrecog. msg from partner   */
#define MSG_TYPE_E_LISTEN_TIMEOUT     -1008    /* partner's heartbeat stopped */
#define MSG_TYPE_E_MESSAGES_LOST      -1009    /* detected msg loss           */
#define MSG_TYPE_E_RUNTIME_ERROR      -1010    /* detected RT error in AST    */


            /***************************************************/
            /**       Generic UCB Types and Classes           **/
            /***************************************************/
 
#define MSG_CLAS_UCB                    102
#define MSG_TYPE_UCB_CONNECT            -1200      /* Initiate connection    */
#define MSG_TYPE_UCB_DISCONNECT         -1201      /* shutdown circuit       */
#define MSG_TYPE_UCB_INITIALIZE         -1202      /* set UCB character.     */
#define MSG_TYPE_UCB_CONNECT_COMPLETE   -1203      /* circuit established    */
#define MSG_TYPE_UCB_PARTNER_DISC       -1204      /* partner disc. circuit  */
#define MSG_TYPE_UCB_MESSAGES_LOST      -1205      /* detected msg loss      */
#define MSG_TYPE_UCB_IO_ERROR           -1206      /* bad status from QIO$   */
#define MSG_TYPE_UCB_RCV_DATA           -1207      /* data msg recv'd by UCB */


    /**************************************************************/
    /*                                                            */
    /* 	        DEFINE DEMO type codes for DEMO_IO_SERVER	  */
    /*                                                            */
    /**************************************************************/

#define MSG_TYPE_DEMO_ADD	      -1200  /* add record function       */
#define MSG_TYPE_DEMO_CHG	      -1216  /* chg record function       */
#define MSG_TYPE_DEMO_DEL	      -1232  /* del record function       */
#define MSG_TYPE_DEMO_INQ	      -1248  /* inq record function       */
#define MSG_TYPE_DEMO_LST	      -1280  /* list record function      */
#define MSG_TYPE_DEMO_LST_NEXT	      -1281  /* list next record function */
#define MSG_TYPE_DEMO_CLIENT_UP	      -1282  /* client application up     */
#define MSG_TYPE_DEMO_CLIENT_DOWN     -1283  /* client application down   */


        /** Define VALID test messages classes and types **/
 
#define MSG_CLAS_TEST_DATA      1000
#define MSG_CLAS_TEST_CONTROL   1001
#define MSG_TYPE_TEST_SIGNON    1010
#define MSG_TYPE_TEST_REQ       1020
#define MSG_TYPE_TEST_ACK       1030
#define MSG_TYPE_TEST_DATA      1040
#define MSG_TYPE_TEST_STATUS    1050
#define MSG_TYPE_TEST_SIGNOFF   1060


#endif
