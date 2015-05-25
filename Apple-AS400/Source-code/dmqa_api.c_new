/*  Module dmqa_api.c   DmQA 1.0    DECmessageQ Queue Adapter DmQ1.0 API    */
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
/****************************************************************************/
/*                                                                          */
/*  DECmessageQ Expertise Center                                            */
/*                                                                          */
/* Note:  AS400 users compiling this module must include a define           */
/*        compiler option for the symbol "AS400".                           */
/*                                                                          */
/*                                                                          */
/*  IDENT HISTORY:                                                          */
/*                                                                          */
/*  Version     Date    Who     Description                                 */
/*                                                                          */
/*  V1.0FT  08-Mar-1991 kgb     Initial release, Keith Barrett              */
/*  V1.0FT  09-Apr-1991 kgb     various extern changes for AS/400           */
/*  V1.0FT  16-Apr-1991 kgb     Added usertag to PAMS_GET_MSGx              */
/*  V1.0FT  24-Apr-1991 kgb     Corrected DNS name space usage in attach_q  */
/*  V1.0FT  25-Apr-1991 kgb     Changed many longword arguments (mostly     */
/*                              queue addresses) into 2 shortwords.         */
/*                              Corrected conditional use of psb. Made      */
/*                              TIMEOUT optional in pams_put_msg.           */
/*  V1.0FT  09-May-1991 kgb     Made PAMS_GET_MSGx suppress receive timer   */
/*  V1.0FT  15-May-1991 kgb     Added call to process_args(), and forced    */
/*                              link shutdown on fatal PAMSDOWN errors      */
/*  V1.0FT  22-May-1991 kgb     Added definition for DMQA__force_bracket    */
/*  V1.0FT  28-May-1991 kgb     Reduced source code so FTP would succeed    */
/*  V1.0FT  29-May-1991 kgb     AS/400 RPG can only call with 13 args       */
/*                                                                          */

#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef VMS
#include <types.h>
#pragma nostandard
#include pams_c_entry_point           /* PAMS function type declarations */
#include pams_c_return_status_def     /* PAMS Func return status defs    */
#pragma standard
#else
#ifndef AS400
#define CONST
#include <sys/types.h>
#endif
#include "p_entry.h"                 /* PAMS function type declarations */
#include "p_return.h"                /* PAMS Func return status defs    */
#endif

#define DMQA__INTERNAL 1
#include "p_dmqa.h"                       /* DECmessageQ Queue Adapter */

extern char    DMQA__long_order[4];              /* defined in dmqin.c */
extern char    DMQA__short_order[2];             /* defined in dmqin.c */

int            DMQA__os_aligns_data    = FALSE;
int            DMQA__os_swaps_bytes    = FALSE;
int            DMQA__os_is_EBCDIC      = DMQA__EBCDIC;

char           DMQA__buffer[DMQA__SIZMAX];      /* Buffer area         */
char           *DMQA__buffer_ptr;               /* Buffer pointer      */

int            DMQA__debug             = 0;     /* debug mode          */
unsigned long  DMQA__task_id           = 0;     /* task-id             */
long           DMQA__errno;
int            DMQA__trans_to_EBCDIC   = -32767;
int            DMQA__trans_from_EBCDIC = -32767;
int            DMQA__msg_is_in_EBCDIC  = -32767;
int            DMQA__force_bracket     = FALSE;
int            DMQA__program_id        = DMQA__CLIENT_ID;

extern int     DMQA__argc;
extern char    **DMQA__argv;

/****************************************************************************/
/*                                                                          */
/*                           DMQA__process_msg                              */
/*                                                                          */
/*   This is a general routine to perform a common piece of logic in all    */
/*   the calls rather than duplicate code. It's main purpose is             */
/*   to send the message, get the response, and re-init the buffer pointer. */
/*                                                                          */
/*   Passed:                                                                */
/*       bufptr          - character pointer to start of buffer             */
/*       length          - length of message to send (including header)     */
/*       expected_retlen - expected len of response (NOT including header). */
/*                         If number is < zero, then value is treated as a  */
/*                         minimum. If number = zero, then expected length  */
/*                         is treated as variable and ignored.              */
/*       pams_cmd        - pams command value.                              */
/*                                                                          */
/*   Returned:                                                              */
/*       DMQA__process_msg                                                  */
/*                       - PAMS status value                                */
/*       *bufptr         - buffer contents destroyed - replaced by response */
/*                                                                          */
/*   NOTE: Access to elements within the buffer or header record need to go */
/*         through the DMQA__ntoh and DMQA__hton functions.                 */
/*                                                                          */
/****************************************************************************/

long DMQA__process_msg(bufptr, length, expected_retlen, pams_cmd)

char    *bufptr;
short   length;
short   expected_retlen;
char    pams_cmd;
{
    long                   retval;
    char                   *local_char_ptr = 0;
    char                   local_buf[8];
    unsigned long          local_ulong     = 67305985L;
    static int             initialized     = 0;
    static unsigned long   message_seq_no  = 1;
    long                   flags           = 0;
    DMQA__HEADER           DMQA_header;
    short                  local_length;
    int                    init_allowed    = FALSE;
    long                   local_status;
    long                   local_error;
    long                   local_flags;
    extern int             DMQA__os_aligns_data;
    extern int             DMQA__os_swaps_bytes;
    extern int             DMQA__os_is_EBCDIC;

#ifdef __USE_ARGS
    extern unsigned long   DMQA__htonl(unsigned long);
    extern unsigned short  DMQA__htons(unsigned short);
    extern unsigned long   DMQA__ntohl(unsigned long);
    extern unsigned short  DMQA__ntohs(unsigned short);
#ifndef AS400              /* This next definition has problems on AS/400 */
    extern int             atoi(char *);
#endif
    extern int             strcmp(CONST char *, CONST char *);
    extern long            DMQA__link_send(int, char *, short, int, int, long);
    extern long            DMQA__link_receive(char *, short *, int, int, long);
    extern void            DMQA__link_shutdown(int, long);
#ifdef AS400
    extern void            DMQA__process_args(void);
#else
    extern void            DMQA__process_args();
#endif
#else
    extern unsigned long   DMQA__htonl();
    extern unsigned short  DMQA__htons();
    extern unsigned long   DMQA__ntohl();
    extern unsigned short  DMQA__ntohs();
    extern int             atoi();
    extern int             strcmp();
    extern long            DMQA__link_send();
    extern long            DMQA__link_receive();
    extern void            DMQA__link_shutdown();
    extern void            DMQA__process_args();
#endif

    struct
    {
        char    tmpchr;
        short   tmpshort;
        long    tmplong;
    } test_alignment;


    if (DMQA__debug)
        printf("%s ** Entering process_msg **\n", DMQA__DBG_PREFIX);

    if (DMQA__argc) DMQA__process_args();     /* Set params from arg list */

    if (bufptr) local_char_ptr = bufptr;
    
    if (local_char_ptr == 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?no bufptr!\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADLOGIC);
        }
        return (long) PAMS__BADLOGIC;
    }

    if ((pams_cmd < 1) || (pams_cmd > 127))
    {
        if (DMQA__debug)
        {
            printf("%s ?Bad pams_cmd!\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADLOGIC);
        }
        return (long) PAMS__BADLOGIC;
    }

    if (abs(expected_retlen) > DMQA__MSGMAX)
    {
        if (DMQA__debug)
        {
            printf(
            "%s ?Expected Return Len (%d) > max (%d)!\n",
                DMQA__DBG_PREFIX, abs(expected_retlen), DMQA__MSGMAX);
            DMQA__DBG_EXIT(PAMS__BADLOGIC);
        }    
        return (long) PAMS__BADLOGIC;
    }
    
    if (!initialized)
    {
        if (DMQA__debug) printf("%s Initializing...\n", DMQA__DBG_PREFIX);

        local_char_ptr  = local_buf;
        DMQA__push_long(&local_char_ptr, (char *) &local_ulong);

        if ((DMQA__long_order[0]  != 1) || (DMQA__long_order[1]  != 2) ||
            (DMQA__long_order[2]  != 3) || (DMQA__long_order[3]  != 4) ||
            (DMQA__short_order[0] != 1) || (DMQA__short_order[1] != 2))
                DMQA__os_swaps_bytes = TRUE;        /* OS swaps bytes */

        if (sizeof(test_alignment) > 7) DMQA__os_aligns_data = TRUE;
        /* OS aligns datatypes */
        
#if DMQA__EBCDIC
        if (DMQA__trans_to_EBCDIC   == -32767) DMQA__trans_to_EBCDIC   = TRUE;
        if (DMQA__trans_from_EBCDIC == -32767) DMQA__trans_from_EBCDIC = TRUE;
        if (DMQA__msg_is_in_EBCDIC  == -32767) DMQA__msg_is_in_EBCDIC  = TRUE;
#endif

        if (DMQA__trans_to_EBCDIC   == -32767) DMQA__trans_to_EBCDIC   = FALSE;
        if (DMQA__trans_from_EBCDIC == -32767) DMQA__trans_from_EBCDIC = FALSE;
        if (DMQA__msg_is_in_EBCDIC  == -32767) DMQA__msg_is_in_EBCDIC  = FALSE;

        initialized = TRUE;
    }
    
    if (DMQA__debug) printf("%s Filling header\n", DMQA__DBG_PREFIX);

    if (message_seq_no == 4294967295L) message_seq_no = 0;
    /* sequence number wrapped around */

    if (DMQA__trans_to_EBCDIC)      flags |= DMQA_BIT__TRANS_TO_EBCDIC;
    if (DMQA__trans_from_EBCDIC)    flags |= DMQA_BIT__TRANS_FROM_EBCDIC;
    if (DMQA__os_swaps_bytes)       flags |= DMQA_BIT__OS_SWAPS_BYTES;
    if (DMQA__os_aligns_data)       flags |= DMQA_BIT__OS_ALIGNS_DATA;
    if (DMQA__msg_is_in_EBCDIC)     flags |= DMQA_BIT__MSG_IN_EBCDIC;
    if (DMQA__force_bracket)        flags |= DMQA_BIT__FORCE_BRACKET;
#if DMQA__EBCDIC
                                    flags |= DMQA_BIT__HEADER_IS_EBCDIC;    
#endif

    DMQA_header.lbyte_order     = 0;
    DMQA_header.pams_status     = 0;
    DMQA_header.error           = 0;
    DMQA_header.task_id         = DMQA__htonl(DMQA__task_id);
    DMQA_header.msg_seq_no      = DMQA__htonl(++message_seq_no);
    DMQA_header.flags           = (long)  DMQA__htonl((unsigned long)  flags);
    DMQA_header.msg_length      = (short) DMQA__htons((unsigned short) length);
    DMQA_header.sbyte_order     = 0;
    DMQA_header.offspring_link  = 0;

    (void) memcpy(  (VOID *) &DMQA_header.ident[0],
                    (CONST VOID *) DMQA__IDENT,
                    (size_t) sizeof(DMQA_header.ident));

    DMQA_header.id              = DMQA__CLIENT_ID;
    DMQA_header.os              = DMQA__OS;
    DMQA_header.protocol        = DMQA__PROTOCOL;
    DMQA_header.cmd             = pams_cmd;
    DMQA_header.type            = 0;        /* filled in by link routines */
    DMQA_header.max_packetsize  = 0;        /* filled in by link routines */
    /* Fill in header */
    
    local_char_ptr      = (char *) &DMQA_header.lbyte_order;
    *local_char_ptr++   = DMQA__long_order[0];
    *local_char_ptr++   = DMQA__long_order[1];
    *local_char_ptr++   = DMQA__long_order[2];
    *local_char_ptr     = DMQA__long_order[3];
    /* fill in long byte order */
    
    local_char_ptr      = (char *) &DMQA_header.sbyte_order;
    *local_char_ptr++   = DMQA__short_order[0];
    *local_char_ptr     = DMQA__short_order[1];
    /* fill in short byte order */
    
    (void) memcpy(  (VOID *) bufptr,
                    (CONST VOID *) &DMQA_header,
                    (size_t) DMQA__HEADERSIZE);
    /* copy header into buffer */
    
    local_length = length;         /* set total length of message */
    
    if (    (pams_cmd == DMQA__PAMS_ATTACH_Q)    ||
            (pams_cmd == DMQA__PAMS_DCL_PROCESS) ||
            (pams_cmd == DMQA__TEST_MSG))  init_allowed = TRUE;
    /* These cmds are allowed to create non-existing links */
                
    if (DMQA__debug)
        printf("%s Calling DMQA_link_send, %d bytes, seq #%u, init = %d\n",
            DMQA__DBG_PREFIX, local_length, message_seq_no, init_allowed);

    retval = DMQA__link_send(init_allowed, bufptr, local_length, 0, 0, 0L);
    /* Send cmd */

    if ((retval & 1) == 0)
    {
        if (DMQA__debug)
        {
            printf("%s DMQA__link_send returned error %d, errno %d\n",
                DMQA__DBG_PREFIX, retval, DMQA__errno);
            DMQA__DBG_EXIT(retval);
        }
        DMQA__link_shutdown((int) -1, 0L);
        return retval;
    }

    if ((DMQA_header.cmd == DMQA__PAMS_GET_MSG) ||
        (DMQA_header.cmd == DMQA__PAMS_GET_MSGW))
    {
        if (DMQA__debug)
            printf("%s (Calling DMQA__link_receive w/o a timer)\n",
               DMQA__DBG_PREFIX);
        retval = DMQA__link_receive(bufptr, &local_length, 0, 0, -1L);
    }
    else
        retval = DMQA__link_receive(bufptr, &local_length, 0, 0, 0L);

    if ((retval & 1) == 0)            /* Success? */
    {
        if (DMQA__debug)
        {
            printf("%s ?DMQA__link_receive returned error %d, errno %d\n",
                DMQA__DBG_PREFIX, retval, DMQA__errno);
            DMQA__DBG_EXIT(retval);
        }
        DMQA__link_shutdown((int) -1, 0L);
        return retval;
    }

    if (DMQA__debug)
        printf("%s DMQA__link_receive returned %d bytes\n",
            DMQA__DBG_PREFIX, local_length);

    (void) memcpy(  (VOID *) &DMQA_header,
                    (CONST VOID *) bufptr,
                    (size_t) DMQA__HEADERSIZE);
    /* copy header from buffer */
    
    if (DMQA_header.id != DMQA__SERVER_ID)
    {
        if (DMQA__debug)
        {
            printf("%s ?Response contained bad id (%d)\n",
                DMQA__DBG_PREFIX, DMQA_header.id);
            DMQA__DBG_EXIT(PAMS__INTERNAL);
        }
        DMQA__link_shutdown((int) -1, 0L);
        return (long) PAMS__INTERNAL;
    }   /* Make sure message was from a server */

    if (DMQA__ntohl(DMQA_header.task_id) != DMQA__task_id)
    {
        if (DMQA__debug)
        {
            printf("%s ?Returned task id (%u) does not match (%u)!\n",
                DMQA__DBG_PREFIX, DMQA__ntohl(DMQA_header.task_id),
                DMQA__task_id);
            DMQA__DBG_EXIT(PAMS__INTERNAL);
        }
        DMQA__link_shutdown((int) -1, 0L);
        return (long) PAMS__INTERNAL;
    }
    /* Make sure that the msg was for us; Task ID must match the one we sent */
    
    if (DMQA__ntohl(DMQA_header.msg_seq_no) != message_seq_no)
    {
        if (DMQA__debug)
        {
            printf("%s ?Returned seq_no (%u) does not match (%u)!\n",
                DMQA__DBG_PREFIX, DMQA__ntohl(DMQA_header.msg_seq_no),
                message_seq_no);
            DMQA__DBG_EXIT(PAMS__INTERNAL);
        }
        DMQA__link_shutdown((int) -1, 0L);
        return (long) PAMS__INTERNAL;
    }
    /* Make sure response was for our msg; Seq no. must match the one we sent */
    
    local_flags = (long) DMQA__ntohl((unsigned long) DMQA_header.flags);

    if (DMQA__debug)
        printf("%s (Internal DMQA_header.flags = %d)\n",
            DMQA__DBG_PREFIX, DMQA_header.flags);
    
#if DMQA__EBCDIC
    if ((local_flags & DMQA_BIT__HEADER_IS_EBCDIC) == 0)
    {
        if (DMQA__debug)
            printf(
            "%s WARNING -- Response header is not in EBCDIC (flags = %d)\n",
            DMQA__DBG_PREFIX, local_flags);
    }
    /* It's only our ident field; so this is just a warning */
#endif

    if (DMQA_header.cmd != pams_cmd)
    {
        if (DMQA__debug)
        {
            printf("%s Response contained incorrect PAMS cmd (%d)\n",
                DMQA__DBG_PREFIX, DMQA_header.cmd);
            DMQA__DBG_EXIT(PAMS__INTERNAL);
        }
        DMQA__link_shutdown((int) -1, 0L);
        return (long) PAMS__INTERNAL;
    }   /* Consistancy check; PAMS cmd should match ours */
    
    local_status = (long) DMQA__ntohl((unsigned long) DMQA_header.pams_status);
    local_error  = (long) DMQA__ntohl((unsigned long) DMQA_header.error);

    if (DMQA__debug)
        printf("%s Server returned PAMS status %d and errno %d\n",
            DMQA__DBG_PREFIX, local_status, local_error);

    if ((local_status & 1) == 0)
    {
        if ((local_status == PAMS__INTERNAL)  ||
            (local_status == PAMS__COMMERR)   ||
            (local_status == PAMS__LINK_DOWN) ||
            (local_status == PAMS__NOLINK)    ||
            (local_status == PAMS__PAMSDOWN))
                DMQA__link_shutdown((int) -1, 0L);

        if (DMQA__debug) DMQA__DBG_EXIT(local_status);
        return local_status;
    }   /* These errors are fatal to the communications link */

    local_length =
        ((short) DMQA__ntohs((unsigned short) DMQA_header.msg_length)) - 
        DMQA__HEADERSIZE;

    if (expected_retlen)
    {
        if (   ((expected_retlen < 0) && (local_length <  abs(expected_retlen)))
            || ((expected_retlen > 0) && (local_length != expected_retlen)))
        {
            if (DMQA__debug)
            {
                printf("%s ?Returned len (%d) isn't what was expected (%d)\n",
                    DMQA__DBG_PREFIX, local_length, expected_retlen);
                DMQA__DBG_EXIT(PAMS__BADLOGIC);
            }
            DMQA__link_shutdown((int) -1, 0L);
            return (long) PAMS__BADLOGIC;
        }
    }   /* expected length was not >= what we expected */

    if (DMQA__debug) DMQA__DBG_EXIT(local_status);
    return local_status;
}

/****************************************************************************/
/*                                                                          */
/*                          pams_dcl_process                                */
/*                                                                          */
/****************************************************************************/

long pams_dcl_process( req_process_num, process_num, q_type, q_name,
                       q_name_len, name_scope)

long    *req_process_num;
long    *process_num;
long    *q_type;
char    *q_name;
long    *q_name_len;
long    *name_scope;
{
    long    status;
    short   length;
    short   expected_retlen = 0;
    short   short_array[2];

    if (DMQA__debug)
    {
        printf("\n%s *** Entering pams_dcl_process ***\n",
            DMQA__DBG_PREFIX);
        printf("\n%s %%% Warning - this is an obsolete call %%%\n",
            DMQA__DBG_PREFIX);
    }

    DMQA__errno = 0;                    /* initialize */

    if ((q_name_len != 0) && (q_name != 0))
        if ((*q_name_len < 0) || (*q_name_len > 31))
        {
            if (DMQA__debug)
            {
               printf("%s ?bad q_name_len passed\n", DMQA__DBG_PREFIX);
               DMQA__DBG_EXIT(PAMS__BADNAME);
            }
            return (long) PAMS__BADNAME;
        }

    if (req_process_num == 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?req_process_num not optional\n",DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPROCNUM);
        }
        return (long) PAMS__BADPROCNUM;
    }

    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;
    DMQA__push_long(&DMQA__buffer_ptr, (char *) name_scope);
    DMQA__push_long(&DMQA__buffer_ptr, (char *) q_name_len);

    if ((q_name != 0) && (q_name_len != 0))
    {
        (void) memcpy((VOID *) DMQA__buffer_ptr,
                      (CONST VOID *) q_name,
                      (size_t) *q_name_len);
        DMQA__buffer_ptr += *q_name_len;
    }

    *DMQA__buffer_ptr++ = 0;
    DMQA__push_long(&DMQA__buffer_ptr, (char *) q_type);

    /* This is one of those params that is data typed as a long,  */
    /* but is really used as a pair of shortwords in applications */
    if (req_process_num)
    {
        (void) memcpy(  (VOID *) &short_array[0],
                        (CONST VOID *) req_process_num,
                        (size_t) sizeof(*req_process_num));

        DMQA__push_short(&DMQA__buffer_ptr, (char *) &short_array[0]);
        DMQA__push_short(&DMQA__buffer_ptr, (char *) &short_array[1]);
    }
    else
        DMQA__push_long(&DMQA__buffer_ptr, (char *) req_process_num);

    length = DMQA__buffer_ptr - DMQA__buffer;
    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;

    status = DMQA__process_msg(DMQA__buffer, length, expected_retlen,
                               (char) DMQA__PAMS_DCL_PROCESS);
                                
    if (status & 1)
    {
        if (process_num != 0)
        {
            /* This is also one of those params */
            short_array[0] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
            short_array[1] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
        
            (void) memcpy(  (VOID *) process_num,
                            (CONST VOID *) &short_array[0],
                            (size_t) sizeof(*process_num));
         }
    }

    if (DMQA__debug) DMQA__DBG_EXIT(status);
    return status;
}

/****************************************************************************/
/*                                                                          */
/*                          pams_attach_q                                   */
/*                                                                          */
/****************************************************************************/

long pams_attach_q( attach_mode, q_attached, q_type, q_name, q_name_len,
                    name_space_list, name_space_list_len,
                    nullarg1, nullarg2, nullarg3)
long    *attach_mode;
long    *q_attached;
long    *q_type;
char    *q_name;
long    *q_name_len;
long    *name_space_list;
long    *name_space_list_len;
ANY     *nullarg1;
ANY     *nullarg2;
ANY     *nullarg3;
{
    long    status;                /* local status variable */
    short   length;                /* Len of used buf area  */
    short   expected_retlen = 0;   /* expected response len */
    int     local_qname_len = 0;
    long    *name_space_ptr;
    long    local_name_space_list_len = 0;
    int     bad_name_space  = FALSE;
    int     total_size;
    short   short_array[2];        /* To remap q_address */


    if (DMQA__debug)
        printf("\n%s *** Entering pams_attach_q ***\n", DMQA__DBG_PREFIX);

    DMQA__errno = 0;
        

                           /* Parse params */


    if (DMQA__debug) printf("%s Parsing args\n", DMQA__DBG_PREFIX);

    if ((attach_mode == 0) || (q_attached == 0))
    {
        if (DMQA__debug)
        {
            printf("%s ?attach_mode and q_attached are not optional\n",
                DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }  /* attach_mode and q_attached are required */
    
    if ((q_name_len != 0) && (q_name != 0))        /* q_name_len?  */
        if ((*q_name_len < 0) || (*q_name_len > 31))
        {                                          /* Yes, size OK?*/
            if (DMQA__debug)
            {
                printf("%s ?bad q_name_len passed\n", DMQA__DBG_PREFIX);
                DMQA__DBG_EXIT(PAMS__BADNAME);
            }
            return (long) PAMS__BADNAME;
        }
        else
            local_qname_len = *q_name_len;
    
    if (name_space_list_len)
    {
        local_name_space_list_len = *name_space_list_len;
        if (local_name_space_list_len < 0) bad_name_space = TRUE;

        if (local_name_space_list_len > DMQA__NAMELISTMAX)
        {
            if (DMQA__debug)
            {
                printf("%s ?name_space_list_len > max (%d)\n",
                    DMQA__DBG_PREFIX, DMQA__NAMELISTMAX);
                DMQA__DBG_EXIT(PAMS__BUFFEROVF);
            }
            return (long) PAMS__BUFFEROVF;
        }
    }           

    if (local_name_space_list_len)
        if (name_space_list == 0) bad_name_space = TRUE;

    if (bad_name_space)
    {
        if (DMQA__debug)
        {
            printf("%s ?Bad name_space_list/name_space_list_len\n",
                DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }   /* Name space args OK? */
    
    total_size =    sizeof(*attach_mode)        +
                    sizeof(long)                +   /* q_type       */
                    local_qname_len             +
                    1                           +   /* null ending  */
                    (local_name_space_list_len * sizeof(long))
                                                +   /* name space   */
                    sizeof(long)                +   /* null arg 1   */
                    sizeof(long)                +   /* null arg 2   */
                    sizeof(long)                +   /* null arg 3   */
                    DMQA__HEADERSIZE;               /* Header       */
    /* calculate total size */
    
    if (total_size> DMQA__SIZMAX)
    {
        if (DMQA__debug)
        {
            printf("%s ?total arg len (%d) > max (%d)\n",
                DMQA__DBG_PREFIX, total_size, DMQA__HEADERSIZE);
            DMQA__DBG_EXIT(DMQA__BUFFEROVF);
        }
        return (long) DMQA__BUFFEROVF;
    }
    /* Make sure the name list doesn't cause our buffer to overflow */

    DMQA__buffer_ptr  = DMQA__buffer + DMQA__SIZMAX - 1;
    *DMQA__buffer_ptr = 0;
    /* Initialize end of buffer -- just as a test to make sure the */
    /* program doesn't blow up later */
    

                          /* Process cmd */


    if (DMQA__debug) printf("%s Processing args\n", DMQA__DBG_PREFIX);

    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;

    DMQA__push_long(&DMQA__buffer_ptr, (char *) nullarg3);
    DMQA__push_long(&DMQA__buffer_ptr, (char *) nullarg2);
    DMQA__push_long(&DMQA__buffer_ptr, (char *) nullarg1);
    DMQA__push_long(&DMQA__buffer_ptr, (char *) name_space_list_len);
        
    name_space_ptr  = name_space_list;
    
    while (local_name_space_list_len--)
        DMQA__push_long(&DMQA__buffer_ptr, (char *) name_space_ptr++);
    /* Copy name space into buffer */
    
    DMQA__push_long(&DMQA__buffer_ptr, (char *) q_name_len);

    if ((q_name != 0) && (q_name_len != 0))
    {
        (void) memcpy((VOID *) DMQA__buffer_ptr,
                      (CONST VOID *) q_name,
                      (size_t) *q_name_len);
        DMQA__buffer_ptr += *q_name_len;
    }   /* Copy q_name into buffer */

    *DMQA__buffer_ptr++ = 0;
    /* make sure q_name ends in a null to prevent accvio in adapter */

    DMQA__push_long(&DMQA__buffer_ptr, (char *) q_type);
    DMQA__push_long(&DMQA__buffer_ptr, (char *) attach_mode);

    length = DMQA__buffer_ptr - DMQA__buffer;

    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;
    /* Initialize pointer for response */

    status = DMQA__process_msg(DMQA__buffer, length, expected_retlen,
                               (char) DMQA__PAMS_ATTACH_Q);

    if (status & 1)
    {
        /* q_attached is one of those few params that are typed as   */
        /* a longword, but are really used by applications as a pair */
        /* of short words                                            */
        
        short_array[0] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
        short_array[1] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
        
        (void) memcpy(  (VOID *) q_attached,
                        (CONST VOID *) &short_array[0],
                        (size_t) sizeof(*q_attached));
    }  /* If success, get q_attached param */
        
    if (DMQA__debug) DMQA__DBG_EXIT(status);
    return status;
}

/****************************************************************************/
/*                                                                          */
/*                              DMQA__test                                  */
/*                                                                          */
/*   This module performs a link test, returns the server's configuration   */
/*   header record, and (optionally) prints the information.                */
/*                                                                          */
/*   NOTE: Access to elements within the buffer or header record need to go */
/*         through the DMQA__ntoh and hton functions.                       */
/*                                                                          */
/****************************************************************************/

long DMQA__test(msg_ptr, flag, msg_len, nullarg1, nullarg2)

char    *msg_ptr;
short   flag;
short   *msg_len;
char    *nullarg1;
long    nullarg2;
{
    long                   status;
    long                   local_long  = 67305985L;
    short                  local_short = 513;
    short                  temp_short;
    char                   *char_ptr   = 0;
    short                  psblen;
    short                  msgsize;
    short                  msg_length;
    short                  bufsize;
    short                  headersize;
    short                  intsize;
    extern unsigned long   DMQA__rcount;
    extern unsigned long   DMQA__scount;
    extern int             DMQA__os_aligns_data;
    extern int             DMQA__os_is_EBCDIC;
    DMQA__HEADER           *DMQA__header_ptr;

#ifdef __USE_ARGS
    extern unsigned long    DMQA__ntohl(unsigned long);
    extern unsigned short   DMQA__ntohs(unsigned short);
#else
    extern unsigned long    DMQA__ntohl();
    extern unsigned short   DMQA__ntohs();
#endif

    
    if (DMQA__debug)
        printf("\n%s *** Entering DMQA__test ***\n", DMQA__DBG_PREFIX);

    DMQA__errno = 0;
    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;
    
    temp_short = sizeof(struct psb);
    DMQA__push_short(&DMQA__buffer_ptr, (char *) &temp_short);
    /* push psb length */

    temp_short = DMQA__MSGMAX;
    DMQA__push_short(&DMQA__buffer_ptr, (char *) &temp_short);
    /* push max message size */

    temp_short = DMQA__SIZMAX;
    DMQA__push_short(&DMQA__buffer_ptr, (char *) &temp_short);
    /* push buffer maximum size */

    temp_short = DMQA__HEADERSIZE;
    DMQA__push_short(&DMQA__buffer_ptr, (char *) &temp_short);
    /* push our headersize */

    temp_short = sizeof(int);
    DMQA__push_short(&DMQA__buffer_ptr, (char *) &temp_short);
    /* push size of integer */

    msg_length = (short) (DMQA__buffer_ptr - DMQA__buffer);

    if (flag | DMQA__debug)
    {
        printf("\n\n\nClient Configuration:\n");
        printf("       Identification: %s\n", DMQA__IDENT);
        printf("    Protocol Revision: %d\n", DMQA__PROTOCOL);
        printf("         Integer size: %d\n", sizeof(int));
        printf("   Defined Headersize: %d\n", DMQA__HEADERSIZE);
        printf("    Actual Headersize: %d\n", sizeof(DMQA__HEADER));
        printf("    OS Identification: %d\n", DMQA__OS);
        printf("           Buffersize: %d\n", DMQA__SIZMAX);
        printf("    Max PAMS msg size: %d\n", DMQA__MSGMAX);
        printf("          Size of PSB: %d\n", sizeof(struct psb));
        printf("   Current Send Count: %u\n", DMQA__scount);
        printf("   Current Recv Count: %u\n", DMQA__rcount);

        char_ptr = (char *) &local_long;
        printf("      LONG byte order: %d %d %d %d\n",
            *char_ptr, *(char_ptr+1), *(char_ptr+2), *(char_ptr+3));

        char_ptr = (char *) &local_short;
        printf("     SHORT byte order: %d %d\n\n", *char_ptr, *(char_ptr+1));

        printf("      OS aligns data?: %d\n", DMQA__os_aligns_data);

        printf("      Data is EBCDIC?: ");

        if (DMQA__os_is_EBCDIC)
            printf("Yes\n");
        else
            printf("No\n");

        printf("       ShowBuffer Max: %d\n", DMQA__SHOWBUFMAX);
        printf("        Name List Max: %d\n", DMQA__NAMELISTMAX);
        printf("       Sel. Array Max: %d\n", DMQA__SELARRAYMAX);
        printf("              Task ID: %u\n", DMQA__task_id);
        printf("           Program ID: %d\n", DMQA__program_id);

        printf("         Default char: ");
        if (((char) 128) > 127)
            printf("unsigned\n");
         else
            printf("signed\n");
    }

    status = DMQA__process_msg(DMQA__buffer, msg_length, 0,
                               (char) DMQA__TEST_MSG);
                                
    if (flag | DMQA__debug)
    {
        printf("     Trans to EBCDIC?: %d\n", DMQA__trans_to_EBCDIC);
        printf("   Trans from EBCDIC?: %d\n", DMQA__trans_from_EBCDIC);
    }

    if ((status & 1) == 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?process_message returned error\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(status);
        }
        return status;
    }


    DMQA__buffer_ptr    = DMQA__buffer + DMQA__HEADERSIZE;

    psblen     = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    msgsize    = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    bufsize    = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    headersize = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    intsize    = (short) DMQA__pop_short(&DMQA__buffer_ptr);

    DMQA__header_ptr = (DMQA__HEADER *) DMQA__buffer;
    DMQA__buffer_ptr = DMQA__buffer;

    if (((short) DMQA__ntohs((unsigned short) DMQA__header_ptr -> msg_length)
        - DMQA__HEADERSIZE) != (    sizeof(psblen)      +
                                    sizeof(msgsize)     +
                                    sizeof(bufsize)     +
                                    sizeof(headersize)  +
                                    sizeof(intsize)))
        if (DMQA__debug)
            printf("%s WARNING -- responsed returned incorrect len!\n",
                DMQA__DBG_PREFIX);


    if (flag | DMQA__debug)
    {
        printf("\n\n\nServer Configuration:\n");

#if DMQA__EBCDIC
        if ((DMQA__header_ptr -> flags) & DMQA_BIT__HEADER_IS_EBCDIC)
#else
        if (((DMQA__header_ptr -> flags) & DMQA_BIT__HEADER_IS_EBCDIC) == 0)
#endif
           printf("       Identification: %s\n", DMQA__header_ptr -> ident);
        else
           printf("       Identification: <not displayable>\n");

        printf("    Protocol Revision: %d\n", DMQA__header_ptr -> protocol);
        printf("         Integer size: %d\n", intsize);
        printf("      DMQA Headersize: %d\n", headersize);
        printf("    OS Identification: %d\n", DMQA__header_ptr -> os);
        printf("           Buffersize: %d\n", bufsize);
        printf("    Max PAMS msg size: %d\n", msgsize);
        printf("          Size of PSB: %d\n", psblen);
        printf("           Msg Seq No: %u\n",
             DMQA__ntohl(DMQA__header_ptr -> msg_seq_no));
        printf("   Current Recv Count: N/A\n");

        char_ptr = (char *) &(DMQA__header_ptr -> lbyte_order);
        printf("      LONG byte order: %d %d %d %d\n",
            *char_ptr, *(char_ptr+1), *(char_ptr+2), *(char_ptr+3));

        char_ptr = (char *) &(DMQA__header_ptr -> sbyte_order);
        printf("     SHORT byte order: %d %d\n", *char_ptr, *(char_ptr+1));

        printf("                flags: %d\n\n\n",
            (long) DMQA__ntohl((unsigned long) DMQA__header_ptr -> flags));
    }

    if (msg_ptr)
        (void) memcpy((VOID *) msg_ptr,
                      (CONST VOID *) DMQA__buffer,
                      (size_t) DMQA__HEADERSIZE);

    *msg_len = DMQA__HEADERSIZE;

    if (DMQA__debug) DMQA__DBG_EXIT(status);
    return status;
}

/****************************************************************************/
/*                                                                          */
/*                               pams_get_msgw                              */
/*                                                                          */
/****************************************************************************/

long pams_get_msgw( msg_area, priority, source, class, type, msg_area_len,
                    len_data, timeout, sel_filter, psb_ptr, show_buffer,
                    show_bufflen, nullarg1, nullarg2, nullarg3)
ANY         *msg_area;
char        *priority;
long        *source;
short       *class;
short       *type;
short       *msg_area_len;
short       *len_data;
long        *timeout;
long        *sel_filter;
struct psb  *psb_ptr;
char        *show_buffer;
long        *show_bufflen;
char        *nullarg1;
ANY         *nullarg2;
ANY         *nullarg3;
{
    short   expected_len;
    long    local_show_bufflen = 0;
    long    status;    
    int     local_int;
    char    *showbuf_ptr;
    short   length;
    char    *local_msg_area;
    long    user_taglen        = 0;
    char    *user_tag;
    long    *temp_ptr;
    long    local_long;
    char    *old_pointer;
    long    small_show_bufflen = 0;
    long    returned_show_bufflen;
    int     longword_count;
    short   short_array[2];
    char    local_psb[DMQA__BIGGEST_PSB];
    ANY     *nullarg3_ptr;
    ANY     *nullarg2_ptr;


    if (DMQA__debug)
        printf("\n%s *** Entering pams_get_msgw ***\n", DMQA__DBG_PREFIX);

    DMQA__errno     = 0;
    local_msg_area  = (char *) msg_area;

#ifdef AS400
    nullarg3_ptr = 0;
    nullarg2_ptr = 0;
    /* AS/400 RPG only supports 13 args, so fill in the last ourself */
#else
    nullarg3_ptr    = nullarg3;
    nullarg2_ptr    = nullarg2;
#endif
        
    temp_ptr        = (long *) nullarg2_ptr;

    if (show_bufflen) local_show_bufflen = *show_bufflen;

    if (local_show_bufflen > DMQA__SHOWBUFMAX)
        local_show_bufflen = DMQA__SHOWBUFMAX;

    if (temp_ptr) user_taglen = *temp_ptr;
    user_tag = (char *) nullarg1;


                        /* Parse params */


    if (DMQA__debug) printf("%s Parsing args\n", DMQA__DBG_PREFIX);

    if ((local_msg_area == 0) || (source       == 0) || (class    == 0) ||
        (type           == 0) || (msg_area_len == 0) || (len_data == 0))
        /* None of these params are optional */
    {
        if (DMQA__debug)
        {
            printf("%s ?Required param omitted\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }

    if ((user_taglen < 0) || ((user_taglen > 0) && (user_tag == 0)))
    {
        if (DMQA__debug)
        {
            printf(
            "%s ?nullargs 1 and/or 2 (usertag and usertaglen) are bad\n",
                 DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }

    if (timeout == 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?bad timeout passed\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADTIME);
        }
        return (long) PAMS__BADTIME;
    }

    if (priority == 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?bad priority passed\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPRIORITY);
        }
        return (long) PAMS__BADPRIORITY;
    }

    if (*msg_area_len < 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?msg_area_len is < 0\n",DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__MSGTOSMALL);
        }
        return (long) PAMS__MSGTOSMALL;
    }   /* Bad msg area */
    
    if ((local_show_bufflen < 0) ||
       ((local_show_bufflen > 0) && (show_buffer == 0)))
    {
        if (DMQA__debug)
        {
            printf("%s ?show_bufflen (%d) is < 0 or show_buffer is null\n",
                DMQA__DBG_PREFIX, local_show_bufflen);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }   /* Bad show_buffer_length */
    
    expected_len = *msg_area_len       +
                   sizeof(*source)     +
                   sizeof(*class)      +
                   sizeof(*type)       +
                   sizeof(*len_data)   +
                   sizeof(struct psb)  +
                   sizeof(short)       +   /* psb len param */
                   (short) user_taglen +
                   (short) local_show_bufflen;
                        
    if ((expected_len + DMQA__HEADERSIZE > DMQA__SIZMAX) ||
        (*msg_area_len > DMQA__MSGMAX))
    {
        if (DMQA__debug)
        {
            printf("%s ?msg_area_len (%d) > %d or total msg (%d) > %d\n",
                DMQA__DBG_PREFIX, *msg_area_len, DMQA__MSGMAX,
                expected_len, DMQA__SIZMAX);
            DMQA__DBG_EXIT(DMQA__BUFFEROVF);
        }
        return (long) DMQA__BUFFEROVF;
    }
    /* total size of message, plus the size of all the necessary params,   */
    /* exceeds the message maximum size allowed for this operating system. */
    /* OR the expected response for this cmd will exceed the maximum       */
    /* size if we allow this message to be sent.                           */
    

                        /* Process cmd */


    if (DMQA__debug) printf("%s Processing args\n", DMQA__DBG_PREFIX);

    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;
    
    DMQA__push_long( &DMQA__buffer_ptr, (char *) nullarg3_ptr);
    DMQA__push_long( &DMQA__buffer_ptr, (char *) &user_taglen);

    if (user_taglen)
    {
        (void) memcpy((VOID *) DMQA__buffer_ptr,
                      (CONST VOID *) user_tag,
                      (size_t) user_taglen);
        DMQA__buffer_ptr += user_taglen;
    }   /* Copy user tag */

    DMQA__push_long( &DMQA__buffer_ptr, (char *) show_bufflen);


    /* sel_filter is one of those params that is data typed as a        */
    /* long, but is really used as a pair of shortwords in applications */

    if (sel_filter)
    {    
        (void) memcpy((VOID *) &short_array[0],
                      (CONST VOID *) sel_filter,
                      (size_t) sizeof(*sel_filter));

        DMQA__push_short(&DMQA__buffer_ptr, (char *) &short_array[0]);
        DMQA__push_short(&DMQA__buffer_ptr, (char *) &short_array[1]);
    }
    else
        DMQA__push_long(&DMQA__buffer_ptr, (char *) sel_filter);
        /* push a zero longword if no pointer */

    DMQA__push_long( &DMQA__buffer_ptr, (char *) timeout);
    DMQA__push_short(&DMQA__buffer_ptr, (char *) msg_area_len);
    DMQA__push_char( &DMQA__buffer_ptr, (char *) priority);
    
    length = DMQA__buffer_ptr - DMQA__buffer;   /* Calculate message length */
    
    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;    /* reset pointer */

    expected_len = 0;            /* Don't bother testing this - wastes time */

    status = DMQA__process_msg(DMQA__buffer, length, expected_len,
                               (char) DMQA__PAMS_GET_MSGW);
                                
    if (psb_ptr)
        DMQA__pop_psb(&DMQA__buffer_ptr, (char *) psb_ptr, sizeof(struct psb));
    else
        DMQA__pop_psb(&DMQA__buffer_ptr, (char *) local_psb,
                      sizeof(struct psb));
    /* psb */
    
    if ((status & 1) == 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?DMQA__process_msg returned error\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(status);
        }
        return status;
    }

    returned_show_bufflen = (long) DMQA__pop_long(&DMQA__buffer_ptr);
    old_pointer           = DMQA__buffer_ptr;

    if ((returned_show_bufflen < local_show_bufflen) &&
        (returned_show_bufflen > 0))
    {
        small_show_bufflen = returned_show_bufflen;

        if (DMQA__debug)
            printf("%s server returned showbufflen %d; min is now %d\n",
            DMQA__DBG_PREFIX, returned_show_bufflen, small_show_bufflen);
    }
    else
        small_show_bufflen = local_show_bufflen;

    if (show_buffer)
    {
        longword_count = small_show_bufflen / sizeof(long);

        for (local_int = 0, showbuf_ptr = show_buffer;
             local_int < longword_count;
             local_int++, showbuf_ptr += sizeof(long))
        {
            if ((local_int >= 10) && (local_int <= 13))
            {
                /* parts of showbuffer are fields that are datatyped as */
                /* longwords, but are really used by applications as a  */
                /* pair of short words.                                 */
                
                short_array[0] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
                short_array[1] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
                
                (void) memcpy(  (VOID *) &local_long,
                                (CONST VOID *) &short_array[0],
                                (size_t) sizeof(local_long));
            }
            else
                local_long = (long) DMQA__pop_long(&DMQA__buffer_ptr);

        (void) memcpy((VOID *) showbuf_ptr,
                      (CONST VOID *) &local_long,
                      (size_t) sizeof(long));
        /* must do it this way to ensure correct byte order */
        }

        for (local_int = (longword_count * sizeof(long));
             local_int < local_show_bufflen;
             local_int++)
        {
            if (local_int < returned_show_bufflen)
                *(showbuf_ptr + local_int) = *(DMQA__buffer_ptr + local_int);
            else
                *(showbuf_ptr + local_int) = 0;
        }
    }   /* copy show buffer */

    DMQA__buffer_ptr = old_pointer + returned_show_bufflen;
    
    *len_data = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    *type = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    *class = (short) DMQA__pop_short(&DMQA__buffer_ptr);


    /* Source is one of those params that is data typed as a long,  */
    /* but is really used as a pair of shortwords in applications   */

    short_array[0] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    short_array[1] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    
    (void) memcpy(  (VOID *) source,
                    (CONST VOID *) &short_array[0],
                    (size_t) sizeof(*source));


    if (*len_data != 0)
    {
        (void) memcpy(  (VOID *) msg_area,
                        (CONST VOID *) DMQA__buffer_ptr,
                        (size_t) *len_data);
        DMQA__buffer_ptr += *len_data;
    }   /* msg_area */

    if (DMQA__debug) DMQA__DBG_EXIT(status);
    return status;
}

/****************************************************************************/
/*                                                                          */
/*                               pams_get_msg                               */
/*                                                                          */
/****************************************************************************/

long pams_get_msg( msg_area, priority, source, class, type, msg_area_len,
                    len_data, sel_filter, psb_ptr, show_buffer,
                    show_bufflen, nullarg1, nullarg2, nullarg3)
ANY         *msg_area;
char        *priority;
long        *source;
short       *class;
short       *type;
short       *msg_area_len;
short       *len_data;
long        *sel_filter;
struct psb  *psb_ptr;
char        *show_buffer;
long        *show_bufflen;
char        *nullarg1;                /* user tag */
ANY         *nullarg2;                /* user taglen */
ANY         *nullarg3;
{
    short   expected_len;
    long    local_show_bufflen = 0;
    long    status;    
    int     local_int;
    char    *showbuf_ptr;
    short   length;
    char    *local_msg_area;
    long    user_taglen        = 0;
    char    *user_tag;
    long    *temp_ptr;
    long    local_long;
    char    *old_pointer;
    long    small_show_bufflen = 0;
    long    returned_show_bufflen;
    int     longword_count;
    short   short_array[2];
    char    local_psb[DMQA__BIGGEST_PSB];
    ANY     *nullarg3_ptr;


    if (DMQA__debug)
        printf("\n%s *** Entering pams_get_msg ***\n", DMQA__DBG_PREFIX);

#ifdef AS400
    nullarg3_ptr = 0;
    /* AS/400 RPG only supports 13 args, so fill in the last ourself */
#else
    nullarg3_ptr    = nullarg3;
#endif

    DMQA__errno     = 0;
    local_msg_area  = (char *) msg_area;
    temp_ptr        = (long *) nullarg2;
        
    if (show_bufflen) local_show_bufflen = *show_bufflen;
    if (local_show_bufflen > DMQA__SHOWBUFMAX)
        local_show_bufflen = DMQA__SHOWBUFMAX;

    if (temp_ptr) user_taglen = *temp_ptr;
    user_tag = (char *) nullarg1;


                        /* Parse params */


    if (DMQA__debug) printf("%s Parsing args\n", DMQA__DBG_PREFIX);

    if ((local_msg_area == 0) || (source       == 0) || (class    == 0) ||
        (type           == 0) || (msg_area_len == 0) || (len_data == 0))
        /* None of these params are optional */
    {
        if (DMQA__debug)
        {
            printf("%s ?Required param omitted\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }

    if ((user_taglen < 0) || ((user_taglen > 0) && (user_tag == 0)))
    {
        if (DMQA__debug)
        {
            printf(
            "%s nullargs 1 and/or 2 (usertag and usertaglen) are bad\n",
                DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }

    if (priority == 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?bad priority passed\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPRIORITY);
        }
        return (long) PAMS__BADPRIORITY;
    }

    if (*msg_area_len < 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?msg_area_len is < 0\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__MSGTOSMALL);
        }
        return (long) PAMS__MSGTOSMALL;
    }  /* msg_area_len cannot be less than zero */
    
    if ((local_show_bufflen < 0) ||
        ((local_show_bufflen > 0) && (show_buffer == 0)))
    {
        if (DMQA__debug)
        {
            printf("%s ?show_bufflen (%d) is < 0 or show_buffer is null\n",
                DMQA__DBG_PREFIX, local_show_bufflen);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }   /* Bad show_buffer_length */
    
    expected_len = *msg_area_len       +
                   sizeof(*source)     +
                   sizeof(*class)      +
                   sizeof(*type)       +
                   sizeof(*len_data)   +
                   sizeof(struct psb)  +
                   sizeof(short)       +   /* psb len param */
                   (short) user_taglen +
                   (short) local_show_bufflen;
                        
    if ((expected_len + DMQA__HEADERSIZE > DMQA__SIZMAX) ||
        (*msg_area_len > DMQA__MSGMAX))
    {
        if (DMQA__debug)
        {
            printf("%s ?msg_area_len (%d) > %d or total msg > %d\n",
                DMQA__DBG_PREFIX, *msg_area_len, DMQA__MSGMAX, DMQA__SIZMAX);
            DMQA__DBG_EXIT(DMQA__BUFFEROVF);
        }
        return (long) DMQA__BUFFEROVF;
    }
    /* total size of message, plus the size of all the necessary params,  */
    /* exceeds the message maximum size allowed for this operating system */
    /* OR the expected response for this cmd will exceed the maximum      */
    /* size if we allow this message to be sent.                          */
    

                        /* Process cmd */


    if (DMQA__debug) printf("%s Processing args\n", DMQA__DBG_PREFIX);

    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;
    
    DMQA__push_long( &DMQA__buffer_ptr, (char *) nullarg3_ptr);
    DMQA__push_long( &DMQA__buffer_ptr, (char *) &user_taglen);

    if (user_taglen)
    {
        (void) memcpy((VOID *) DMQA__buffer_ptr,
                      (CONST VOID *) user_tag,
                      (size_t) user_taglen);
        DMQA__buffer_ptr += user_taglen;
    }   /* Copy user tag */

    DMQA__push_long( &DMQA__buffer_ptr, (char *) show_bufflen);


    /* sel_filter is one of those params that is data typed as a        */
    /* long, but is really used as a pair of shortwords in applications */

    if (sel_filter)
    {
        (void) memcpy((VOID *) &short_array[0],
                      (CONST VOID *) sel_filter,
                      (size_t) sizeof(*sel_filter));

        DMQA__push_short(&DMQA__buffer_ptr, (char *) &short_array[0]);
        DMQA__push_short(&DMQA__buffer_ptr, (char *) &short_array[1]);
    }
    else
        DMQA__push_long(&DMQA__buffer_ptr, (char *) sel_filter);
        /* push a zero longword if no pointer */


    DMQA__push_short(&DMQA__buffer_ptr, (char *) msg_area_len);
    DMQA__push_char( &DMQA__buffer_ptr, (char *) priority);
    
    length = DMQA__buffer_ptr - DMQA__buffer;   /* Calculate message length */
    
    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;    /* reset pointer */

    expected_len = 0;            /* Don't bother testing this - wastes time */

    status = DMQA__process_msg(DMQA__buffer, length, expected_len,
                               (char) DMQA__PAMS_GET_MSG);
                                

    if (psb_ptr)
        DMQA__pop_psb(&DMQA__buffer_ptr, (char *) psb_ptr, sizeof(struct psb));
    else
        DMQA__pop_psb(&DMQA__buffer_ptr, (char *) local_psb, 
            sizeof(struct psb));
    /* psb */
    
    if ((status & 1) == 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?DMQA__process_msg returned error\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(status);
        }
        return status;
    }

    returned_show_bufflen = (long) DMQA__pop_long(&DMQA__buffer_ptr);
    old_pointer           = DMQA__buffer_ptr;

    if (returned_show_bufflen < local_show_bufflen)
        small_show_bufflen = returned_show_bufflen;
    else
        small_show_bufflen = local_show_bufflen;

    if (DMQA__debug)
        printf("%s server returned showbufflen %d; min is now %d\n",
            DMQA__DBG_PREFIX, returned_show_bufflen, small_show_bufflen);

    if (show_buffer)
    {
        longword_count = small_show_bufflen / sizeof(long);

        for (local_int = 0, showbuf_ptr = show_buffer;
             local_int < longword_count;
             local_int++, showbuf_ptr += sizeof(long))
        {
            if ((local_int >= 10) && (local_int <= 13))
            {
                /* parts of showbuffer are fields that are datatyped as */
                /* longwords, but are really used by applications as a  */
                /* pair of short words.                                 */
                
                short_array[0] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
                short_array[1] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
                
                (void) memcpy(  (VOID *) &local_long,
                                (CONST VOID *) &short_array[0],
                                (size_t) sizeof(local_long));
            }
            else
                local_long = (long) DMQA__pop_long(&DMQA__buffer_ptr);

        (void) memcpy((VOID *) showbuf_ptr,
                      (CONST VOID *) &local_long,
                      (size_t) sizeof(long));
        /* must do it this way to ensure correct byte order */
        }

        for (local_int = (longword_count * sizeof(long));
             local_int < local_show_bufflen;
             local_int++)
        {
            if (local_int < returned_show_bufflen)
                *(showbuf_ptr + local_int) = *(DMQA__buffer_ptr + local_int);
            else
                *(showbuf_ptr + local_int) = 0;
        }
    }   /* copy show buffer */


    DMQA__buffer_ptr = old_pointer + returned_show_bufflen;

    *len_data = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    *type     = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    *class    = (short) DMQA__pop_short(&DMQA__buffer_ptr);


    /* Source is one of those params that is data typed as a long, */
    /* but is really used as a pair of shortwords in applications  */

    short_array[0] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    short_array[1] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    
    (void) memcpy(  (VOID *) source,
                    (CONST VOID *) &short_array[0],
                    (size_t) sizeof(*source));


    if (*len_data != 0)
    {
        (void) memcpy(  (VOID *) msg_area,
                        (CONST VOID *) DMQA__buffer_ptr,
                        (size_t) *len_data);
        DMQA__buffer_ptr += *len_data;
    }   /* msg_area */

    if (DMQA__debug) DMQA__DBG_EXIT(status);
    return status;
}

/****************************************************************************/
/*                                                                          */
/*                               pams_put_msg                               */
/*                                                                          */
/****************************************************************************/

long pams_put_msg( msg_area, priority, target, class, type, delivery,
                    msg_size, timeout, psb_ptr, uma, resp_q,
                    nullarg1, nullarg2, nullarg3)
ANY         *msg_area;
char        *priority;
long        *target;
short       *class;
short       *type;
char        *delivery;
short       *msg_size;
long        *timeout;
struct psb  *psb_ptr;
char        *uma;
long        *resp_q;
ANY         *nullarg1;
ANY         *nullarg2;
ANY         *nullarg3;
{
    short   expected_retlen = 0;
    long    status;    
    short   length;
    char    *local_msg_area;
    short   short_array[2];
    char    local_psb[DMQA__BIGGEST_PSB];
    ANY     *nullarg3_ptr;


    if (DMQA__debug)
        printf("\n%s *** Entering pams_put_msg ***\n", DMQA__DBG_PREFIX);

#ifdef AS400
    nullarg3_ptr = 0;
    /* AS/400 RPG only supports 13 args, so fill in the last ourself */
#else
    nullarg3_ptr    = nullarg3;
#endif

    DMQA__errno    = 0;
    local_msg_area = (char *) msg_area;
        

                        /* Parse params */


    if (DMQA__debug) printf("%s Parsing args\n", DMQA__DBG_PREFIX);

    if ((local_msg_area == 0) || (target   == 0) || (class == 0) ||
        (type           == 0) || (msg_size == 0))
        /* None of these params are optional */
    {
        if (DMQA__debug)
        {
            printf("%s ?Required param omitted\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }

    if (delivery == 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?bad delivery passed\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADDELIVERY);
        }
        return (long) PAMS__BADDELIVERY;
    }

    if (priority == 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?bad priority passed\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPRIORITY);
        }
        return (long) PAMS__BADPRIORITY;
    }
    
    if ((*msg_size > DMQA__MSGMAX) || ( *msg_size           +
                                        sizeof(*priority)   +
                                        sizeof(*target)     +
                                        sizeof(*class)      +
                                        sizeof(*type)       +
                                        sizeof(*delivery)   +
                                        sizeof(*msg_size)   +
                                        sizeof(long)        +   /* timeout */
                                        sizeof(struct psb)  +
                                        sizeof(short)       +   /* psb len */
                                        sizeof(char)        +   /* uma */
                                        sizeof(long)        +   /* resp_q */
                                        sizeof(long)        +   /* nullarg1 */
                                        sizeof(long)        +   /* nullarg2 */
                                        sizeof(long) > DMQA__SIZMAX))
    {
        if (DMQA__debug)
        {
            printf("%s ?msg_size (%d) > %d or total msg > %d\n",
                DMQA__DBG_PREFIX, *msg_size, DMQA__MSGMAX, DMQA__SIZMAX);
            DMQA__DBG_EXIT(DMQA__BUFFEROVF);
        }
        return (long) DMQA__BUFFEROVF;
    }
    /* total size of message, plus the size of all the necessary params,  */
    /* exceeds the message maximum size allowed for this operating system */
    /* OR the expected response for this cmd will exceed the maximum      */
    /* size if we allow this message to be sent.                          */
    

                        /* Process cmd */


    if (DMQA__debug) printf("%s Processing args\n", DMQA__DBG_PREFIX);

    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;
    
    DMQA__push_long( &DMQA__buffer_ptr, (char *) nullarg3_ptr);
    DMQA__push_long( &DMQA__buffer_ptr, (char *) nullarg2);
    DMQA__push_long( &DMQA__buffer_ptr, (char *) nullarg1);


    /* resp_q is one of those params that is data typed as a long, */
    /* but is really used as a pair of shortwords in applications  */

    if (resp_q)
    {
        (void) memcpy((VOID *) &short_array[0],
                      (CONST VOID *) resp_q,
                      (size_t) sizeof(*resp_q));

        DMQA__push_short(&DMQA__buffer_ptr, (char *) &short_array[0]);
        DMQA__push_short(&DMQA__buffer_ptr, (char *) &short_array[1]);
    }
    else
        DMQA__push_long(&DMQA__buffer_ptr, (char *) resp_q);
        /* push a zero longword if no pointer */

    DMQA__push_char( &DMQA__buffer_ptr, (char *) uma);
    DMQA__push_long( &DMQA__buffer_ptr, (char *) timeout);
    DMQA__push_short(&DMQA__buffer_ptr, (char *) msg_size);
    DMQA__push_char( &DMQA__buffer_ptr, (char *) delivery);
    DMQA__push_short(&DMQA__buffer_ptr, (char *) type);
    DMQA__push_short(&DMQA__buffer_ptr, (char *) class);
    

    /* target is one of those params that is data typed as a long, */
    /* but is really used as a pair of shortwords in applications  */

    (void) memcpy((VOID *) &short_array[0],
                  (CONST VOID *) target,
                  (size_t) sizeof(*target));

    DMQA__push_short(&DMQA__buffer_ptr, (char *) &short_array[0]);
    DMQA__push_short(&DMQA__buffer_ptr, (char *) &short_array[1]);
    DMQA__push_char( &DMQA__buffer_ptr, (char *) priority);
    
    (void) memcpy((VOID *) DMQA__buffer_ptr,
                  (CONST VOID *) msg_area,
                  (size_t) *msg_size);
    DMQA__buffer_ptr += *msg_size;              /* copy msg */
    
    length = DMQA__buffer_ptr - DMQA__buffer;   /* Calculate message length */
    
    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;

    status = DMQA__process_msg(DMQA__buffer, length, expected_retlen,
                               (char) DMQA__PAMS_PUT_MSG);
                                
    if (psb_ptr)
        DMQA__pop_psb(&DMQA__buffer_ptr, (char *) psb_ptr, sizeof(struct psb));
    else
        DMQA__pop_psb(&DMQA__buffer_ptr, (char *) local_psb, 
            sizeof(struct psb));
    /* psb */
    
    if (DMQA__debug) DMQA__DBG_EXIT(status);
    return status;
}

/****************************************************************************/
/*                                                                          */
/*                               pams_locate_q                              */
/*                                                                          */
/****************************************************************************/

long pams_locate_q( q_name, q_name_len, q_addr, wait_mode, req_id,
                    resp_q, name_space_list, name_space_list_len, nullarg)
char    *q_name;
long    *q_name_len;
long    *q_addr;
long    *wait_mode;
long    *req_id;
long    *resp_q;
long    *name_space_list;
long    *name_space_list_len;
ANY     *nullarg;
{
    short   expected_retlen = 0;
    long    status;    
    short   length;
    long    *name_space_ptr;
    long    local_name_space_list_len = 0;
    int     bad_name_space  = FALSE;
    int     total_size;
    short   short_array[2];


    if (DMQA__debug)
        printf("\n%s *** Entering pams_locate_q ***\n", DMQA__DBG_PREFIX);

    DMQA__errno = 0;
        

                        /* Parse params */


    if (DMQA__debug) printf("%s Parsing args\n", DMQA__DBG_PREFIX);

    if (    (q_name      == 0) || (q_name_len  == 0) || (q_addr      == 0))
    /* None of these params are optional */
    {
        if (DMQA__debug)
        {
            printf("%s ?Required param omitted\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }

    if (name_space_list_len != 0)
        if (name_space_list == 0) bad_name_space = TRUE;
    else
    {
        local_name_space_list_len = *name_space_list_len;
        if ((name_space_list == 0) && (local_name_space_list_len != 0))
            bad_name_space = TRUE;
    }   /* test name space args */
    
    if (bad_name_space)
    {
        if (DMQA__debug)
        {
            printf("%s ?Bad name_space_list/name_space_list_len passed\n",
                DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }   /* Are name space args OK? */
    
    if ((*q_name_len < 0) || (*q_name_len > 31))
    {
        if (DMQA__debug)
        {
            printf("%s ?Bad q_name_len passed\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }
    
    total_size =    sizeof(*q_name_len)         +
                    *q_name_len                 +
                    1                           +   /* null ending  */
                    sizeof(long)                +   /* wait_mode    */
                    sizeof(long)                +   /* req_id       */
                    sizeof(long)                +   /* resp_q       */
                    (local_name_space_list_len * sizeof(long))
                                                +   /* name space   */
                    sizeof(long)                +   /* n_s len      */
                    sizeof(long)                +   /* null arg     */
                    DMQA__HEADERSIZE;               /* Header       */
    /* calculate total size */
    
    if (total_size> DMQA__SIZMAX)
    {
        if (DMQA__debug)
        {
            printf("%s ?total argument size (%d) > buffer max (%d)\n",
                DMQA__DBG_PREFIX, total_size, DMQA__HEADERSIZE);
            DMQA__DBG_EXIT(DMQA__BUFFEROVF);
        }
        return (long) DMQA__BUFFEROVF;
    }
    /* Make sure the name list doesn't cause our buffer to overflow */


                        /* Process cmd */


    if (DMQA__debug) printf("%s Processing args\n", DMQA__DBG_PREFIX);

    DMQA__buffer_ptr    = DMQA__buffer + DMQA__HEADERSIZE;
    
    DMQA__push_long(&DMQA__buffer_ptr, (char *) nullarg);
    DMQA__push_long(&DMQA__buffer_ptr, (char *) name_space_list_len);
        
    name_space_ptr  = name_space_list;
    
    while (local_name_space_list_len--)
        DMQA__push_long(&DMQA__buffer_ptr, (char *) name_space_ptr++);
    /* Copy name space into buffer */


    /* resp_q is one of those params that is data typed as a long, */
    /* but is really used as a pair of shortwords in applications  */

    if (resp_q)
    {
        (void) memcpy((VOID *) &short_array[0],
                      (CONST VOID *) resp_q,
                      (size_t) sizeof(*resp_q));

        DMQA__push_short(&DMQA__buffer_ptr, (char *) &short_array[0]);
        DMQA__push_short(&DMQA__buffer_ptr, (char *) &short_array[1]);
    }
    else
        DMQA__push_long(&DMQA__buffer_ptr, (char *) resp_q);
        /* push a zero longword if no pointer */


    DMQA__push_long(&DMQA__buffer_ptr, (char *) req_id);
    DMQA__push_long(&DMQA__buffer_ptr, (char *) wait_mode);
    DMQA__push_long(&DMQA__buffer_ptr, (char *) q_name_len);

    if (q_name_len)
    {
        (void) memcpy((VOID *) DMQA__buffer_ptr,
                      (CONST VOID *) q_name,
                      (size_t) *q_name_len);
        DMQA__buffer_ptr += *q_name_len;
    }

    *DMQA__buffer_ptr++ = 0;
    /* make sure q_name ends in a null to prevent accvio in adapter */

    length = DMQA__buffer_ptr - DMQA__buffer;    /* Calculate message length */
    
    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;     /* reset pointer */

    status = DMQA__process_msg(DMQA__buffer, length, expected_retlen,
                               (char) DMQA__PAMS_LOCATE_Q);
                                
    if ((status & 1) == 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?DMQA__process_msg returned error\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(status);
        }
        return status;
    }
    

    /* q_addr is one of those params that is data typed as a long, */
    /* but is really used as a pair of shortwords in applications  */

    short_array[0] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    short_array[1] = (short) DMQA__pop_short(&DMQA__buffer_ptr);
    
    (void) memcpy(  (VOID *) q_addr,
                    (CONST VOID *) &short_array[0],
                    (size_t) sizeof(*q_addr));

    
    if (DMQA__debug) DMQA__DBG_EXIT(status);
    return status;
}

/****************************************************************************/
/*                                                                          */
/*                               pams_confirm_msg                           */
/*                                                                          */
/****************************************************************************/

long pams_confirm_msg(msg_seq_num, status, force_j)
QUADWORD    *msg_seq_num;
long        *status;
char        *force_j;
{
    short   expected_retlen = 0;
    long    return_status;    
    short   length;
    long    *local_long_ptr;


    if (DMQA__debug)
        printf("\n%s *** Entering pams_confirm_msg ***\n", DMQA__DBG_PREFIX);
            
    DMQA__errno    = 0;
    local_long_ptr = msg_seq_num;
        

                        /* Parse params */


    if (DMQA__debug) printf("%s Parsing args\n", DMQA__DBG_PREFIX);

    if ((msg_seq_num == 0) || (status == 0) || (force_j == 0))
    /* None of these params are optional */
    {
        if (DMQA__debug)
        {
            printf("%s ?Required param omitted\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }


                        /* Process cmd */


    if (DMQA__debug) printf("%s Processing args\n", DMQA__DBG_PREFIX);

    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;
    
    DMQA__push_char(&DMQA__buffer_ptr, (char *) force_j);
    DMQA__push_long(&DMQA__buffer_ptr, (char *) status);

    DMQA__push_long(&DMQA__buffer_ptr, (char *) local_long_ptr);
    DMQA__push_long(&DMQA__buffer_ptr, (char *) ++local_long_ptr);
    /* msg_seq_num */
    
    length = DMQA__buffer_ptr - DMQA__buffer;   /* Calculate message length */
    
    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;    /* reset pointer */

    return_status = DMQA__process_msg(DMQA__buffer, length, expected_retlen,
                                      (char) DMQA__PAMS_CONFIRM_MSG);
                                
    if (DMQA__debug) DMQA__DBG_EXIT(return_status);
    return return_status;
}

/****************************************************************************/
/*                                                                          */
/*                              pams_set_select                             */
/*                                                                          */
/****************************************************************************/

long pams_set_select(selection_array, num_masks, index_handle)

ANY     *selection_array;
short   *num_masks;
long    *index_handle;
{
    long    status;
    short   length;
    short   expected_retlen;
    int     total_size;
    long    sel_index;
    long    sel_max;
    long    *sel_ptr = 0;


    if (DMQA__debug)
        printf("\n%s *** Entering pams_set_select ***\n", DMQA__DBG_PREFIX);

    DMQA__errno = 0;
    sel_ptr     = (long *) selection_array;
        

                            /* Parse params */


    if (DMQA__debug) printf("%s Parsing args\n", DMQA__DBG_PREFIX);

    if ((sel_ptr == 0) || (num_masks == 0) || (index_handle == 0))
    {
        if (DMQA__debug)
        {
            printf("%s ?no params are optional\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }
    
    if ((*num_masks < 1) || (*num_masks > DMQA__SELARRAYMAX))
    {
        if (DMQA__debug)
        {
            printf("%s num_masks (%d) not within range (1-%d)\n",
                DMQA__DBG_PREFIX, *num_masks, DMQA__SELARRAYMAX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }
    
    total_size =    sizeof(*num_masks)                  +
                    (13 * sizeof(long) * (*num_masks))  +
                    DMQA__HEADERSIZE;
    /* calculate total size */
    
    if (total_size> DMQA__SIZMAX)
    {
        if (DMQA__debug)
        {
            printf("%s ?total arg size (%d) > max (%d)\n",
                DMQA__DBG_PREFIX, total_size, DMQA__HEADERSIZE);
            DMQA__DBG_EXIT(DMQA__BUFFEROVF);
        }
        return (long) DMQA__BUFFEROVF;
    }
    /* Make sure the name list doesn't cause our buffer to overflow */


                            /* Process cmd */


    if (DMQA__debug) printf("%s Processing args\n", DMQA__DBG_PREFIX);

    DMQA__buffer_ptr    = DMQA__buffer + DMQA__HEADERSIZE;

    DMQA__push_short(&DMQA__buffer_ptr, (char *) num_masks);
    
    for (sel_index = 0, sel_max = (13 * (*num_masks));
         sel_index < sel_max;
         ++sel_index, ++sel_ptr)
        DMQA__push_long(&DMQA__buffer_ptr, (char *) sel_ptr);
    /* copy selection masks */

    expected_retlen = sizeof(*index_handle);
    length          = DMQA__buffer_ptr - DMQA__buffer;
    /* Calculate length and return length */

    DMQA__buffer_ptr    = DMQA__buffer + DMQA__HEADERSIZE;

    status = DMQA__process_msg(DMQA__buffer, length, expected_retlen,
                               (char) DMQA__PAMS_SET_SELECT);
                                
    if (status & 1)
        *index_handle = (long) DMQA__pop_long(&DMQA__buffer_ptr);
    /* If success, get index_handle */
        
    if (DMQA__debug) DMQA__DBG_EXIT(status);
    return status;
}

/****************************************************************************/
/*                                                                          */
/*                               pams_cancel_select                         */
/*                                                                          */
/****************************************************************************/

long pams_cancel_select(index_handle)

long    *index_handle;
{
    long    retval = 0;
    short   length = DMQA__HEADERSIZE;
                

    if (DMQA__debug)
        printf("\n%s *** Entering pams_cancel_select ***\n",DMQA__DBG_PREFIX);

    if (index_handle == 0)
    {
        if (DMQA__debug)
        {
            printf("%s ?index_handle is not optional\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADPARAM);
        }
        return (long) PAMS__BADPARAM;
    }

    DMQA__buffer_ptr = DMQA__buffer + DMQA__HEADERSIZE;
    
    DMQA__push_long(&DMQA__buffer_ptr, (char *) index_handle);

    retval = DMQA__process_msg( DMQA__buffer, length, 0,
                                (char) DMQA__PAMS_CANCEL_SELECT);
    
    if (DMQA__debug) DMQA__DBG_EXIT(retval);
    return retval;
}

/****************************************************************************/
/*                                                                          */
/*                               pams_exit                                  */
/*                                                                          */
/****************************************************************************/

long pams_exit()
{
    long     retval = 0;
    short    length = DMQA__HEADERSIZE;

#ifdef __USE_ARGS
    extern void    DMQA__link_shutdown(int, long);
#else
    extern void    DMQA__link_shutdown();
#endif
            
    if (DMQA__debug)
        printf("\n%s *** Entering pams_exit ***\n", DMQA__DBG_PREFIX);

    DMQA__errno = 0;

    retval = DMQA__process_msg(DMQA__buffer, length, 0, (char) DMQA__PAMS_EXIT);

    DMQA__link_shutdown((int) -1, 0L);
    
    if (DMQA__debug) DMQA__DBG_EXIT(retval);
    return retval;
}
