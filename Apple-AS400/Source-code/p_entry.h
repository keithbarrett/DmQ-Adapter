/*  Module p_entry.h            DmQA V1.0        PAMS Entry points          */
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

#ifndef P_ENTRY_H
#define P_ENTRY_H

#ifndef	GEN_PAMS_V1
#define	GEN_PAMS_V1 0			/* Generate current structures      */
#endif

#ifndef OLD_PAMS_API
#define OLD_PAMS_API 0			/* Use old portable PAMS prototypes */
#endif

#ifndef DMQ_GENERIC_VERSION		/* For EFG compatibility            */
#define	DMQ_GENERIC_VERSION	"EFG_V10"
#endif

#ifndef FALSE
#define	FALSE	0
#define	TRUE	1
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

#ifndef ANY
#define ANY void		/* Can  be  changed to  "char"  for */
#endif				/* systems without "void *" support */

#ifndef QUADWORD
#define QUADWORD long		/* Treat as an array of longwords   */
#endif



#ifndef VMS
#define	PSB		psb
#endif

                        /* PAMS general return structures */

#ifdef MSDOS
    typedef unsigned long   u_long;
    typedef unsigned short  u_short;
    typedef unsigned char   u_char;
#endif


#if OLD_PAMS_API
    typedef union
    {
       u_long all;
       struct
       {
          u_short process;
          u_short group;
       } au;
    } pams_address;


    struct SHOWMSG
    {
       pams_address		target;
       pams_address		original_target;
       pams_address		original_source;

       u_long    		sequence_number;
       u_char			delivery;
       u_char			priority;
       char			buffer_type;
       char			filler[81];
    } ;
#else
    typedef union
    {
       long all;
       struct
       {
          short process;
          short group;
       } au;
    } pams_address;


    struct SHOWMSG
    {
       pams_address target;
       pams_address original_target;
       pams_address original_source;

       long	    sequence_number;
       char	    delivery;
       char	    priority;
       char	    buffer_type;
       char	    filler[81];
    } ;
#endif


#if GEN_PAMS_V1

    struct PSB
    {
	u_short	type;
	u_short call_depend;
	u_long	status;
	u_long	msg_seq_number[2];
	u_long	uma_status;
	char	filler[12];
    } ;

#else

#if OLD_PAMS_API
    struct PSB
    {
	u_short	type_of_psb;
	u_short call_dependent;
	u_long	del_psb_status;
	u_long	seq_number[2];
	u_long	uma_psb_status;
	char	psb_reserved[12];
    } ;
#else
    struct PSB
    {
	short	type_of_psb;
	short   call_dependent;
	long	del_psb_status;
	long	seq_number[2];
	long	uma_psb_status;
	char	psb_reserved[12];
    } ;
#endif

/*                                                                          */
/*   Define the PAMS Show Buffer structure                                  */
/*   Used to return further information from PAMS_GET_MSG, PAMS_GET_MSGW,   */
/*   and PAMS_GET_MSGA calls.                                               */
/*                                                                          */

struct	show_buffer
	{
	    long int version;
	    long int transfer_status;
	    long int transfer_size;
	    long int reserved[7];
	    long int target;
	    long int original_target;
	    long int source;
	    long int original_source;
	    long int delivery;
	    long int priority;
	} ;

/*                                                                           */
/*   Define the PAMS Selection Array structure                               */
/*   Used to pass an array of selection masks into the PAMS_SET_SELECT call. */
/*                                                                           */

union	fill_0
	{
	    long selection_array [13];
	    struct
	    {
		long queue;
		long priority;
		long key_1_offset;
		long key_1_size;
		long key_1_value;
		long key_1_oper;
		long key_2_offset;
		long key_2_size;
		long key_2_value;
		long key_2_oper;
		long order_offset;
		long order_size;
		long order_order;
	    } fill_1;
	} ;

/*                                                                          */
/*   Define the PAMS Queue List structure                                   */
/*   Used for input into PUTIL_SHOW_PENDING call.                           */
/*                                                                          */

union	fill_2
	{
	    short q_list [1];
	    struct
	    {
		short queue;
	    } fill_3;
	} ;

/*                                                                          */
/*   Define the PAMS Pending List structure                                 */
/*   Used for output from PUTIL_SHOW_PENDING call.                          */
/*                                                                          */

union	fill_4
	{
	    long pending_list [1];
	    struct
	    {
		long count;
	    } fill_5;
	} ;

#endif



struct  DELIVERY_INFO
{
	unsigned int	sn:2;	/* 0 : Sender notification= DM_NN         */
				/* 1 : Sender notification= DM_WF         */
				/* 2 : Sender_notification= DM_AK         */

	unsigned int    rp:3;	/* 0 : Recovery point = DM_RSAF		  */
				/* 1 : Recovery point = DM_DQF            */
				/* 2 : Recovery point = DM_NET            */
				/* 3 : Recovery point = DM_RCM		  */
				/* 4 : Recovery point = DM_NRM		  */

	unsigned int   uma:3;	/* 0 : Undel msg action = DM_DISCL        */
				/* 1 : Undel_msg_action = DM_DISC         */
				/* 2 : Undel_msg_action = DM_DLQ          */
				/* 3 : Undel_msg_action = DM_DLJ          */
                                /* 4 : Undel_msg_action = DM_RTS          */
				/* 5 : Undel_msg_action = DM_SAF	  */

	unsigned int   rtm:1;	/* 0 : Not MRS msg			  */
				/* 1 : MRS msg				  */

	unsigned int   dqc:1;  /* disable quota checking                  */
        unsigned int   fret:1; /* return MSGUNDEL to RCV modules          */

	unsigned int   res:5;	/* pad to fill a 16 bit word              */
};


#define DM_NN		0
#define DM_WF		1
#define DM_AK		2

#define DM_RSAF		0
#define DM_DQF		1
#define DM_NET		2
#define DM_RCM		3
#define DM_NRM		4

#define DM_DISCL	0
#define DM_DISC		1
#define DM_DLQ		2
#define DM_DLJ		3
#define DM_RTS		4
#define DM_SAF		5


                /*** PAMS external call declarations ***/


#ifdef __USE_ARGS                   /* Full prototypes */

extern long  pams_attach_q(     long *, 	    /* attach_mode	    */
				long *,	            /* q_attached	    */
				long *,	            /* q_type		    */
				char *,	            /* q_name		    */
				long *,	            /* q_name_len	    */
				long *,	            /* name_space_list      */
				long *,	            /* name_space_list_len  */
				ANY  *,	            /* nullarg	            */
				ANY  *,	            /* nullarg	            */
				ANY  *);            /* nullarg	            */

extern long  pams_locate_q(     char *,	            /* q_name		    */
                                long *,	            /* q_name_len	    */
				long *,	            /* q_address	    */
				long *,	            /* wait_mode	    */
				long *,	            /* req_id		    */
				long *,	            /* resp_q		    */
				long *,	            /* name_space_list      */
				long *,	            /* name_space_list_len  */
				ANY  *);            /* nullarg	            */

extern long  pams_put_msg(      ANY   *,            /* msg_area	            */
				char  *,            /* priority	            */
				long  *,            /* target		    */
				short *,            /* class		    */
				short *,            /* type		    */
				char  *,            /* delivery	            */
				short *,            /* msg_size	            */
				long  *,            /* timeout	            */
				struct psb *,       /* psb		    */
				char  *,            /* uma                  */
				long  *,            /* resp_q		    */
				ANY   *,            /* nullarg              */
				ANY   *,            /* nullarg	            */
				ANY   *);           /* nullarg	            */

extern long  pams_get_msg(      ANY   *,            /* msg_area             */
				char  *,            /* priority	            */
				long  *,            /* source		    */
				short *,            /* class		    */
				short *,            /* type		    */
				short *,            /* msg_area_len	    */
				short *,            /* len_data	            */
				long  *,            /* sel_filter	    */
				struct psb *,       /* psb		    */
				char  *,            /* show_buffer	    */
				long  *,            /* show_bufflen	    */
				char  *,            /* nullarg	            */
				ANY   *,            /* nullarg	            */
				ANY   *);           /* nullarg	            */

extern long  pams_set_select(   ANY   *,            /* selection_array      */
				short *,            /* num_masks	    */
				long  *);           /* index_handle	    */
				/* available on Qadapter only */

extern long  pams_cancel_select(long *);            /* index_handle	    */
				/* available on Qadapter only */

extern long  pams_confirm_msg(  QUADWORD *,         /* msg_seq_num	    */
				long *,	            /* num		    */
				char *);            /* force_j	            */
				/* available on Qadapter only */

extern long  pams_get_msgw(     ANY   *,            /* msg_area	            */
				char  *,            /* priority	            */
				long  *,            /* source		    */
				short *,            /* class		    */
				short *,            /* type		    */
				short *,            /* msg_area_len	    */
				short *,            /* len_data	            */
				long  *,            /* timeout	            */
				long  *,            /* sel_filter	    */
				struct psb *,       /* psb		    */
				char  *,            /* show_buffer	    */
				long  *,            /* show_bufflen	    */
				char  *,            /* nullarg	            */
				ANY   *,            /* nullarg	            */
				ANY   *);           /* nullarg	            */

extern long  pams_exit(void);


                    /* Not implimented in Qadapter */
                    
extern long  pams_trace_msg(    char **,            /* ptr to ptr to buf    */
				char *);            /* flag 1=Recv, 0=Send  */
			        /* Non-VMS only */

extern long  putil_trace_msg(   char **,            /* ptr to ptr to buf    */
				char *);            /* flag 1=Recv, 0=Send  */
				/* Non-VMS only */

extern long pams_print_sts(     char *,	            /* process_name	    */
				char *,	            /* message	            */
				long *);            /* return status	    */

extern long  putil_print_sts(   char *,	            /* process name	    */
				char *,	            /* message	            */
				long *);            /* return status	    */

extern long  putil_print_status(char *,	            /* process name	    */
				char *,	            /* message	            */
				long);	            /* return status	    */

extern unsigned char pams_status();	            /* Return link status   */
extern unsigned char putil_status();		    /* Return link status   */


                        /* Obsolete services */

extern long  pams_dcl_process(  long *,	            /* req_process_num      */
				long *,	            /* process_num	    */
				long *,	            /* q_type		    */
				char *,	            /* q_name		    */
				long *,	            /* q_name_len	    */
				long *);            /* name_scope	    */

extern long  pams_alloc_msg(    short *,            /* size		    */
				ANY **);            /* msg_ptr	            */

extern long  pams_send_msg(     ANY  **,            /* msg_ptr	            */
				char  *,            /* priority	            */
				long  *,            /* target		    */
				short *,            /* class		    */
				short *,            /* type		    */
				char  *,            /* delivery	            */
				long  *,            /* timeout	            */
				struct psb *,       /* psb		    */
				char  *,            /* uma		    */
				long  *);           /* resp_q		    */
				/* Not in DmQ Queue Adapter */

extern long  pams_rcv_msg(      ANY  **,            /* msg_ptr	            */
				char  *,            /* priority	            */
				long  *,            /* source		    */
				short *,            /* class		    */
				short *,            /* type		    */
				short *,            /* length		    */
				long  *,            /* sel_filter	    */
				struct psb *);      /* psb                  */
				/* Not in DmQ Queue Adapter */

extern long  pams_rcv_msgw(     ANY  **,            /* msg_ptr	            */
				char  *,            /* priority	            */
				long  *,            /* source		    */
				short *,            /* class		    */
				short *,            /* type		    */
				short *,            /* length		    */
				long  *,            /* timeout	            */
				long  *,            /* sel_filter	    */
				struct psb *);     /* psb                   */
				/* Not in DmQ Queue Adapter */

extern long  pams_free_msg(     ANY *);             /* msg_ptr	            */
			        /* Not in DmQ Queue Adapter */

extern long  pams_show_msg(     ANY **,	            /* buffer		    */
				struct SHOWMSG *);
				/* Not in DmQ Queue Adapter */


      /*** Not implemented in non-VMS systems or Queue Adapter ***/

extern long  putil_show_pending(long *,
                                ANY  *,
                                ANY  *);

extern long  pams_wake(         long *,
                                long *);

extern long  pams_set_timer(    long *,	            /* timer_id	            */
				char *,	            /* format		    */
				long *,	            /* p_timeout	    */
				QUADWORD *);        /* s_timeout	    */

extern long  pams_cancel_timer( long *);            /* timer_id	            */
extern long  pams_wait_timer();
extern long  pams_dcl_synch(    long *, long *);
extern long  pams_set_synch(    char *);
extern long  pams_cancel_synch();
extern long  pams_cancel_get(   long *);            /* sel_filter	    */

extern long  pams_get_msga(     ANY   *,            /* msg_area	            */
				char  *,            /* priority	            */
				long  *,            /* source_q	            */
				short *,            /* msg_class	    */
				short *,            /* msg_type	            */
				short *,            /* area_length	    */
				short *,            /* msg_length	    */
				long  *,            /* sel_filter	    */
				struct psb *,       /* psb		    */
				char  *,            /* show_buffer	    */
				long  *,            /* show_bufflen	    */
				char  *,            /* nullarg	            */
				long  *,            /* nullarg	            */
				long  *,            /* actrtn		    */
				long  *,            /* actparm	            */
				long  *,            /* flag_id	            */
				long  *);           /* nullarg	            */
#else

extern long pams_dcl_process();
extern long pams_attach_q();
extern long pams_locate_q();
extern long pams_alloc_msg();
extern long pams_send_msg();
extern long pams_put_msg();
extern long pams_rcv_msg();
extern long pams_rcv_msgw();
extern long pams_get_msg();
extern long pams_get_msgw();
extern long pams_free_msg();
extern long pams_show_msg();
extern long pams_exit();
extern long pams_trace_msg();
extern long putil_trace_msg();
extern long pams_print_sts();
extern long putil_print_sts();
extern long putil_print_status();
extern unsigned char pams_status();
extern unsigned char putil_status();

extern long putil_show_pending();
extern long pams_wake();
extern long pams_set_timer();
extern long pams_cancel_timer();
extern long pams_wait_timer();
extern long pams_dcl_synch();
extern long pams_set_synch();
extern long pams_cancel_synch();
extern long pams_cancel_get();
extern long pams_get_msga();
#endif

#endif
