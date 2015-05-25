/*  Module p_symbol.h    DmQA V1.0        PAMS Selective receive symbols    */
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

#ifndef P_SYMBOL_H
#define P_SYMBOL_H


/************************************************************************** */
/*      define the Selective Receive symbols                                */
/************************************************************************** */
/*                                                                          */

#define PSEL_PRI_P0     1
#define PSEL_PRI_P1     2
#define PSEL_PRI_ANY    3
#define PSEL_SOURCE    -1
#define PSEL_CLASS     -2
#define PSEL_TYPE      -3

/*                                                                          */
/************************************************************************** */
/*    define the Select-by-Queue macros                                     */
/************************************************************************** */
/*                                                                          */

#define PSEL_BY_MASK    -1
#define PSEL_PQ         -2
#define PSEL_AQ         -3
#define PSEL_PQ_AQ      -4
#define PSEL_AQ_PQ      -5
#define PSEL_PQ_TYPE    -6
#define PSEL_PQ_CLASS   -7
#define PSEL_PQ_PRI     -8
#define PSEL_UCB        -9
#define PSEL_TQ_PQ      -10
#define PSEL_TQ_PQ_AQ   -11

/*                                                                          */
/************************************************************************** */
/*    define the Selective Operators                                        */
/************************************************************************** */
/*                                                                          */

#define PSEL_OPER_ANY   0
#define PSEL_OPER_EQ    1
#define PSEL_OPER_NEQ   2
#define PSEL_OPER_GTR   3
#define PSEL_OPER_LT    4
#define PSEL_OPER_GTRE  5
#define PSEL_OPER_LTE   6

/*                                                                          */
/************************************************************************** */
/*    define the Selective Order                                            */
/************************************************************************** */
/*                                                                          */

#define PSEL_ORDER_FIFO 0
#define PSEL_ORDER_MIN  11
#define PSEL_ORDER_MAX  12

/*                                                                          */
/************************************************************************** */
/*     define obsolete non-MRS Delivery Mode symbols                        */
/************************************************************************** */
/*                                                                          */

#define PDEL_MODE_DG_LOG    0
#define PDEL_MODE_DG        1
#define PDEL_MODE_RTS       10
#define PDEL_MODE_WFQ       20

/*                                                                          */
/************************************************************************** */
/*     define Delivery Mode symbols                                         */
/************************************************************************** */
/*                                                                          */

#define PDEL_MODE_WF_SAF    25
#define PDEL_MODE_WF_DQF    26
#define PDEL_MODE_WF_NET    27
#define PDEL_MODE_WF_RCM    28
#define PDEL_MODE_WF_MEM    29
#define PDEL_MODE_AK_SAF    30
#define PDEL_MODE_AK_DQF    31
#define PDEL_MODE_AK_NET    32
#define PDEL_MODE_AK_RCM    33
#define PDEL_MODE_AK_MEM    34
#define PDEL_MODE_NN_SAF    35
#define PDEL_MODE_NN_DQF    36
#define PDEL_MODE_NN_NET    37
#define PDEL_MODE_NN_RCM    38
#define PDEL_MODE_NN_MEM    39

/*                                                                          */
/*   obsolete MRS recovery mode                                             */
#define PDEL_MODE_ST_RECOVER 22

/*                                                                          */
/************************************************************************** */
/*     define Undeliverable Message Action symbols                          */
/************************************************************************** */
/*                                                                          */

#define PDEL_UMA_RTS    1
#define PDEL_UMA_DLJ    2
#define PDEL_UMA_DLQ    3
#define PDEL_UMA_SAF    4
#define PDEL_UMA_DISC   5
#define PDEL_UMA_DISCL  6

/*                                                                          */
/************************************************************************** */
/*     define PAMS_CONFIRM_MSG force journal symbols                        */
/************************************************************************** */
/*                                                                          */

#define PDEL_DEFAULT_JRN    0
#define PDEL_FORCE_JRN      1
#define PDEL_NO_JRN         2

/*                                                                          */
/************************************************************************** */
/*     define PAMS_LOCATE_Q symbols                                         */
/************************************************************************** */
/*                                                                          */

#define PSEL_TBL_PROC       -50
#define PSEL_TBL_GRP        -51
#define PSEL_TBL_DNS_CACHE  -52
#define PSEL_TBL_DNS_LOW    -53
#define PSEL_TBL_DNS_MED    -54
#define PSEL_TBL_DNS_HIGH   -55
#define PSYM_WF_RESP        -70
#define PSYM_AK_RESP        -71

/*                                                                          */
/************************************************************************** */
/*     define PAMS_ATTACH_Q symbols                                         */
/************************************************************************** */
/*                                                                          */

#define PSYM_ATTACH_PQ          -200
#define PSYM_ATTACH_SQ          -201
#define PSYM_ATTACH_BY_NAME     -210
#define PSYM_ATTACH_BY_NUMBER   -211
#define PSYM_ATTACH_TEMPORARY   -212

/*                                                                          */
/************************************************************************** */
/*     define PAMS_DCL_PROCESS symbols                                      */
/************************************************************************** */
/*                                                                          */

#define PSYM_DCL_PQ         -200
#define PSYM_DCL_SQ         -201
#define PSYM_SCOPE_LOCAL    -202
#define PSYM_SCOPE_GLOBAL   -203

/*                                                                          */
/************************************************************************** */
/*     define PAMS_WAKE symbols                                             */
/************************************************************************** */
/*                                                                          */

#define PSYM_IGNORE         -230
#define PSYM_SET_UCB_PRI_P0 -231
#define PSYM_SET_UCB_PRI_P1 -232
#define PSYM_SET_UCB_A      -240
#define PSYM_SET_UCB_B      -241
#define PSYM_SET_UCB_C      -242
#define PSYM_SET_UCB_D      -243
#define PSYM_SET_UCB_E      -244
#define PSYM_SET_UCB_F      -245
#define PSYM_SET_UCB_G      -246
#define PSYM_SET_UCB_H      -247
#define PSYM_SET_UCB_I      -248
#define PSYM_SET_UCB_J      -249

/*                                                                          */
/* --------------- End of PAMS symbol file ------------------------------   */

#endif
