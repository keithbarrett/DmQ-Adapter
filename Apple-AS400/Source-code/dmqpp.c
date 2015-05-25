/*  Module  dmqpp.c  DmQ 1.0        Push/pop datatypes into/from buffer     */
/*                                                                          */
/*                                                                          */
/*  Copyright (c) 1990, 1991                                                */
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

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*	DMQA__push* and DMQA__pop*      Push and pop datatypes into/from    */
/*					a buffer area                       */
/*                                                                          */
/*  These functions exist to help provide a platform independent method of  */
/*  storing and retrieving binary information (longwords, words, etc) into  */
/*  and from a multi-vendor messaging system. By limiting such operations   */
/*  to these routines, overall program code can be made more portable.      */
/*                                                                          */
/*  These functions provide a means of using a buffer area like a stack     */
/*  (except that it is FIFO instead of LIFO) - the data items (longwords,   */
/*  words,etc ) can be "pushed" (transferred) into the buffer before trans- */
/*  mission to a remote system, then "poped" off for use. In both cases,    */
/*  buffer pointer will automatically get incrememted to the next appro-    */
/*  priate location. These function use the DMQA__ntoh* and DMQA__hton*     */
/*  functions (found in the DMQIN.C file) to fully provide for building     */
/*  platform independent messages.                         	            */
/*                                                                          */
/*  In general, the "push" functions require 2 parameters: a pointer to a   */
/*  pointer (char **) to a message/buffer area that will receive the data,  */
/*  and a pointer (cast into a "char *") to the data item to use. The       */
/*  message area pointer will automatically get incremented by the number   */
/*  of bytes placed into the area. If the data item pointer contains a      */
/*  zero, then a 0 of the appropriate datatype will be placed in the area.  */
/*                                                                          */
/*  In general, the "pop" functions require one parameter - a pointer to a  */
/*  pointer (char **) of the message area/buffer location that contains the */
/*  first byte of the desired item to remove. The function returns an       */
/*  unsigned datatype (which can be cast into a signed type of desired),    */
/*  and increments the buffer pointer to the next location.                 */
/*                                                                          */
/*  Except for the names of variables and functions, and for the provision  */
/*  of including the "DMQA__push_psb" and "DMQA__pop_psb" functions (used   */
/*  by some DECmessageQ product(s)), these functions are generic in nature  */
/*  and have no product specific code references.                           */
/*                                                                          */
/*                                                                          */
/*  Revisions:                                                              */
/*                                                                          */
/*  07-Mar-1991	kgb	Inital release; Keith Barrett                       */
/*  22-Apr-1991 kgb	Some apperance changes.                             */
/*                                                                          */
/*                                                                          */


#ifdef VMS
#ifndef __USE_ARGS
#define __USE_ARGS  TRUE
#endif
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

#ifdef __SAA__
#ifndef __USE_ARGS
#define __USE_ARGS TRUE
#endif
#endif
/* IBM */

#ifdef __SAA_L2__
#ifndef __USE_ARGS
#define __USE_ARGS TRUE
#endif
#endif
/* IBM */

#ifdef __EXTENDED__
#ifndef __USE_ARGS
#define __USE_ARGS TRUE
#endif
#endif
/* IBM */

#ifndef VOID
#define VOID void
#endif

#ifndef CONST
#ifdef AS400
#define CONST const
#else
#define CONST
#endif
#endif
/* Note: On AS/400 systems, this module must be compiled with a definition */
/* for the symbol "AS400", otherwise prototyping errors will result.       */

#include <stddef.h>
#include <string.h>


#ifdef __USE_ARGS
extern unsigned long    DMQA__htonl(unsigned long);
extern unsigned short   DMQA__htons(unsigned short);
extern unsigned long    DMQA__ntohl(unsigned long);
extern unsigned short   DMQA__ntohs(unsigned short);
extern void		*memcpy(VOID *, CONST VOID *, size_t);
#else
extern unsigned long    DMQA__htonl();
extern unsigned short   DMQA__htons();
extern unsigned long    DMQA__ntohl();
extern unsigned short   DMQA__ntohs();
extern void		*memcpy();
#endif

/****************************************************************************/
/* Module:  DMQA__push_long                                                 */
/*                                                                          */
/* Desc:    This function will put a longword into the buffer at the        */
/*	    current pointer location, and increments the pointer. If the    */
/*	    item pointer is zero, the function will put a long zero in the  */
/*	    buffer instead. Byte order is determined by the DMQA__htonl     */
/*	    function.                                                       */
/*                                                                          */
/*                                                                          */
/* Passed:  msg_ptr	pointer to the pointer to the buffer to fill in.    */
/*	    item_ptr	pointer to the data to place in buffer.             */
/*                                                                          */
/****************************************************************************/

void DMQA__push_long(msg_ptr, item_ptr)

char	**msg_ptr;
char	*item_ptr;
{
	    unsigned long   *local_longptr  = 0;
            unsigned long   local_long      = 0;
	    char	    *local_item_ptr;

    if (*msg_ptr)
    {
	local_item_ptr  = (char *) item_ptr;
        local_longptr   = (unsigned long *) local_item_ptr;

	if (local_item_ptr == 0)
	    local_long  = 0;
	else
            local_long      = *local_longptr;
	/* Figure out value to deal with */

	local_long = DMQA__htonl(local_long);
	/* Convert it to appropriate format */

	(void) memcpy(	(VOID *) *msg_ptr,
			(CONST VOID *) &local_long,
			(size_t) sizeof(long));

	*msg_ptr    += sizeof(long);
    }
}


/****************************************************************************/
/* Module:   DMQA__push_short                                               */
/*                                                                          */
/* Desc:    This function will put a shortword into the buffer at the       */
/*	    current pointer location, and increments the pointer. If the    */
/*	    item pointer is zero, the function will put a short zero in the */
/*	    buffer instead. Byte order is determined by the DMQA__htonl     */
/*	    function.                                                       */
/*                                                                          */
/*                                                                          */
/* Passed:  msg_ptr	pointer to the pointer to the buffer to fill in.    */
/*	    item_ptr	pointer to the data to place in buffer.             */
/*                                                                          */
/****************************************************************************/

void DMQA__push_short(msg_ptr, item_ptr)

char    **msg_ptr;
char	*item_ptr;
{
	    unsigned short  local_short;
	    unsigned short  *short_ptr;
	    char	    *local_item_ptr;

    if (*msg_ptr)
    {
	local_item_ptr  = (char *) item_ptr;
	short_ptr       = (unsigned short *) item_ptr;

	if (local_item_ptr == 0)
	    local_short  = 0;
	else
	    local_short  = *short_ptr;
	/* Figure out value to deal with */

	short_ptr	= &local_short;
	local_short = DMQA__htons(local_short);
	/* Convert it to appropriate format */

	(void) memcpy(	(VOID *) *msg_ptr,
			(CONST VOID *) short_ptr,
			(size_t) sizeof(short));

	*msg_ptr    += sizeof(short);
    }
}


/****************************************************************************/
/* Module:   DMQA__pop_long                                                 */
/*                                                                          */
/* Desc:    This function will take a longword from the buffer at the       */
/*	    current pointer location, and increments the pointer.           */
/*                                                                          */
/*                                                                          */
/* Passed:   msg_ptr	pointer to the pointer to the buffer to use.        */
/*                                                                          */
/* Returned: DMQA__pop_long                                                 */
/*			Value taken from buffer.                            */
/*                                                                          */
/****************************************************************************/

unsigned long DMQA__pop_long(msg_ptr)

char	**msg_ptr;
{
            unsigned long   local_ulong = 0;

    if (*msg_ptr)
    {
	(void) memcpy(	(VOID *) &local_ulong,
			(CONST VOID *) *msg_ptr,
			(size_t) sizeof(long));

	*msg_ptr    += sizeof(long);
    }

    return DMQA__ntohl(local_ulong);
}


/****************************************************************************/
/* Module:   DMQA__pop_short                                                */
/*                                                                          */
/* Desc:    This function will take a shortword from the buffer at the      */
/*	    current pointer location, and increments the pointer.           */
/*                                                                          */
/*                                                                          */
/* Passed:   msg_ptr	pointer to the pointer to the buffer to use.        */
/*                                                                          */
/* Returned: DMQA__pop_short                                                */
/*			Value taken from buffer.                            */
/*                                                                          */
/****************************************************************************/

unsigned short DMQA__pop_short(msg_ptr)

char	**msg_ptr;
{
            unsigned short  local_ushort = 0;

    if (*msg_ptr)
    {
	(void) memcpy(	(VOID *) &local_ushort,
			(CONST VOID *) *msg_ptr,
			(size_t) sizeof(short));

	*msg_ptr    += sizeof(short);
    }

    return DMQA__ntohs(local_ushort);
}


/****************************************************************************/
/* Module:  DMQA__push_char                                                 */
/*                                                                          */
/* Desc:    This function will put a byte into the buffer at the current    */
/*	    pointer location, and increments the pointer. If the item       */
/*	    pointer is zero, the function will put a byte zero in the       */
/*	    buffer instead.                                                 */
/*	                                                                    */
/* Note: No "pop" function is needed; the user can just do a *ptr++.        */
/*	                                                                    */
/*                                                                          */
/* Passed:  msg_ptr	pointer to the pointer to the buffer to fill in.    */
/*	    item_ptr	pointer to the data to place in buffer.             */
/*                                                                          */
/****************************************************************************/

void DMQA__push_char(msg_ptr, item_ptr)

char	**msg_ptr;
char	*item_ptr;
{
    char    *local_ptr = 0;

    if (*msg_ptr)
    {
	if (item_ptr)
	{
	    local_ptr	= (char *) item_ptr;
	    **msg_ptr	= *local_ptr;		/* copy data into buffer */
	}
	else
	    **msg_ptr = 0;			/* copy null into buffer */

	*msg_ptr += sizeof(char);		/* Increment pointer */
    }
}


/****************************************************************************/
/* Module:  DMQA__push_psb                                                  */
/*                                                                          */
/* Desc:    This function will put a PSB into the buffer at the current     */
/*	    pointer location, and increments the pointer. If the item       */
/*	    pointer is zero, the function will put a series of nulls (the   */
/*	    size of the psb) in the buffer instead.                         */
/*                                                                          */
/*                                                                          */
/* Passed:  msg_ptr	pointer to the pointer to the buffer to fill in.    */
/*	    item_ptr	pointer to the data to place in buffer.             */
/*	    psb_size	value of "sizeof(struct psb)"                       */
/*                                                                          */
/****************************************************************************/

static char    DMQA__psb_type1[6]  = {2,2,4,4,4,4};

void DMQA__push_psb(msg_ptr, item_ptr, psb_size)

char	**msg_ptr;
char	*item_ptr;
short	psb_size;
{
    int			    psb_elements;
    int 		    i = 0;
    int                     current_size = 0;
    unsigned short 	    psb_type;
    int			    psb_item_size;
    char		    *local_ptr	= 0;
    short                   *short_ptr  = 0;


    local_ptr	= (char *) item_ptr;
    short_ptr   = (short *) item_ptr;
    
    if (*msg_ptr)
    {
	DMQA__push_short(msg_ptr, (char *) &psb_size);
	psb_type	= (unsigned short) *short_ptr;

	do
	{
	    switch (psb_type)
	    {
                case 0:
		case 1:
		default:
		    psb_item_size = (int) DMQA__psb_type1[i];
		    psb_elements  = sizeof(DMQA__psb_type1);
		    break;
            }

	    switch (psb_item_size)
	    {
		case 0:
		    break;

		case 1:
		default:
		    DMQA__push_char(msg_ptr, (char *) local_ptr);
		    if (local_ptr) local_ptr++;
                    current_size++;
		    break;

		case 2:
		    DMQA__push_short(msg_ptr, (char *) local_ptr);
		    if (local_ptr) local_ptr += sizeof(short);
                    current_size += sizeof(short);
		    break;

		case 4:
		    DMQA__push_long(msg_ptr, (char *) local_ptr);
		    if (local_ptr) local_ptr += sizeof(long);
                    current_size += sizeof(long);
		    break;
	    }
	} while (++i < psb_elements);
        
        while (current_size++ < psb_size)
        {
	    DMQA__push_char(msg_ptr, (char *) local_ptr);
	    if (local_ptr) local_ptr++;
	}
    }
}



/****************************************************************************/
/* Module:  DMQA__pop_psb                                                   */
/*                                                                          */
/* Desc:    This function will get a PSB from a buffer at the               */
/*	    current pointer location, and increments the pointer.           */
/*                                                                          */
/*                                                                          */
/* Passed:  msg_ptr	pointer to the pointer to the buffer containing the */
/*			psb. Null pointer skips the function.               */
/*	    item_ptr	pointer to the psb structure to fill in             */
/*	    psb_size	value of "sizeof(struct psb)"                       */
/*	                                                                    */
/****************************************************************************/

void DMQA__pop_psb(msg_ptr, item_ptr, psb_size)

char	**msg_ptr;
char	*item_ptr;
short	psb_size;
{
    int			    psb_elements;
    int			    i		    = 0;
    unsigned short	    local_short;
    unsigned short 	    psb_type;
    short		    psb_stored_size;
    unsigned short	    psb_min_size;
    unsigned short	    current_size    = 0;
    int			    psb_item_size;
    char		    *local_ptr	    = 0;
    char		    *local_msg_ptr  = 0;
    unsigned long	    local_long;


    local_ptr	= (char *) item_ptr;

    if ((*msg_ptr != 0) && (local_ptr != 0) && (psb_size > 0))
    {
	local_msg_ptr = *msg_ptr;

	while (i < psb_size)
	    *(local_ptr + i++) = 0;	    /* init area */

	psb_stored_size = (short) DMQA__pop_short(&local_msg_ptr);
	if (psb_stored_size < 0) psb_stored_size = 0;

	if (psb_stored_size < psb_size)
	    psb_min_size = psb_stored_size;
	else
	    psb_min_size = psb_size;
	/* Our minimum size is based on what's */
	/* smaller, the local or remote psb */

	if (psb_min_size >= sizeof(short))
	{
	    psb_type = DMQA__pop_short(&local_msg_ptr);
	    /* Grab the psb type */

	    (void) memcpy(  (VOID *) local_ptr,
			    (CONST VOID *) &psb_type,
			    (size_t) sizeof(short));
	    local_ptr	    += sizeof(short);
	    current_size    += sizeof(short);
	    /* copy - don't PUSH! We need bytes in right order */

	    i = 1;

	    do
	    {
		switch (psb_type)
		{
		    default:
		    case 1:
			psb_item_size = (int) DMQA__psb_type1[i];
			psb_elements  = sizeof(DMQA__psb_type1);
			break;
		}

		if (current_size + psb_item_size <= psb_min_size)
		{
		    switch (psb_item_size)
		    {
			case 0:
			    break;

			default:
			case 1:
			    *local_ptr++ = (char) *local_msg_ptr++;
			    break;

			case 2:
			    local_short = DMQA__pop_short(&local_msg_ptr);
			    (void) memcpy(  (VOID *) local_ptr,
					    (CONST VOID *) &local_short,
					    (size_t) sizeof(short));
			    local_ptr	    += sizeof(short);
			    break;

			case 4:
			    local_long = DMQA__pop_long(&local_msg_ptr);
			    (void) memcpy(  (VOID *) local_ptr,
					    (CONST VOID *) &local_long,
					    (size_t) sizeof(long));
			    local_ptr	    += sizeof(long);
			    break;
		    }
		}

		current_size    += psb_item_size;

	    } while ((++i < psb_elements) && (current_size < psb_min_size));
            
            while (current_size++ < psb_min_size)
                *local_ptr++ = (char) *local_msg_ptr++;
	}
    }

    *msg_ptr += (psb_stored_size + sizeof(short));
}
