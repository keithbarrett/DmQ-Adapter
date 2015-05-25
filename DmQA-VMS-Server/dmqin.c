/*  Module dmqin.c  DmQ V1.0            Convert to/from known byte order    */
/*                                                                          */
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
/*                                                                          */
/*  DECmessageQ Expertise Center                                            */
/*                                                                          */
/*                                                                          */
/*  DMQA__hton* and DMQA_ntoh*	    Convert datatype storage format         */
/*                                                                          */
/*  These functions are similar to to the unix networking "ntoh" and "hton" */
/*  functions; they take a short or longword (whose byte order is specific  */
/*  to the system being used) and will produce a fixed "known" byte         */
/*  order - independent of the system. They differ, however, in that they   */
/*  automatically "determine" what the byte order of the platform being     */
/*  is. rather than using hard-coded conditions. They also are provided	    */
/*  because not all systems offer the other "ntoh" type functions.          */
/*                                                                          */
/*  There are 3 definitions that can be used to tailor the function-        */
/*  ality of this file beyond it's normal logic of determining byte order.  */
/*  DMQA__USE_VAX_ORDER and DMQA__USE_NETWORK_ORDER determine whether the   */
/*  "byte order" PRODUCED AS A RESULT is to be in VAX byte order, or in     */
/*  Network byte order (little endian or big endian). The default is to use */
/*  VAX order, but this is only to make life easier in debugging DmQA       */
/*  applications. The other definition, DMQA__REVERSE_BITS, will cause the  */
/*  bits within the resulting bytes to be placed in reversed order. There   */
/*  are no current platforms that require this; the default is keep bits    */
/*  intact.                                                                 */
/*                                                                          */
/*  These functions are designed to be generic, and have no product         */
/*  specific logic other than the variable and functions names themselves.  */
/*                                                                          */
/*                                                                          */
/*  IDENT HISTORY:                                                          */
/*                                                                          */
/*  Version     Date    Who     Description                                 */
/*                                                                          */
/*  V1.0    08-Mar-1991 kgb     First release; Keith Barrett                */
/*  V1.0    22-Apr-1991 kgb     Minor appearence changes.                   */
/*                                                                          */
/*                                                                          */


/* Compiler options - determine whether to use full argument prototypes.    */
/* (Note: Cannot use a prototype prefix macro, due to AS/400's inability    */
/* of allowing spaces in a function definition).                            */

#ifdef VMS
#ifndef __USE_ARGS
#define __USE_ARGS TRUE
#endif
#endif

#ifdef ultrix
#ifndef __USE_ARGS
#define __USE_ARGS TRUE
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


#ifndef DMQA__USE_NETWORK_ORDER
#define DMQA__USE_NETWORK_ORDER 0
#endif

#ifndef DMQA__USE_VAX_ORDER
#if DMQA__USE_NETWORK_ORDER
#define DMQA__USE_VAX_ORDER 0
#else
#define DMQA__USE_VAX_ORDER 1
#endif
#endif

/* The above defines determine the byte order used - VAX or network. The */
/* default (per the decision to be as compatible to VMS as possible) is  */
/* VAX byte order, but this can be changed without program impact (as    */
/* long as both communicating programs use the same order.               */


#ifndef DMQA__REVERSE_BITS
#define	DMQA__REVERSE_BITS 0
#endif
/* This flag (when set) will cause the bits within a byte to be reversed */
/* before storing result. This may be needed for some specific platforms */


    static int	need_init		= -1;
    char	DMQA__long_order[4]	= {0,0,0,0};
    char	DMQA__short_order[2]	= {0,0};

#if DMQA__REVERSE_BITS
    static unsigned char    bit_array[16]   = {	0,  8,  4, 12,  2, 10,  6, 14,
						1,  9,  5, 13,  3, 11,  7, 15};
#endif


/* This function initializes the internal array that determines the byte   */
/* order of this platform. It is called automatically if needed. It        */
/* determines the order by placing a known numerical constant into memory, */
/* then examining the bytes to see what landed where.                      */
					    
void DMQA__init_nbo()
{
#if DMQA__USE_VAX_ORDER
    unsigned long    temp_longword  = 67305985L;
    unsigned short   temp_short	    = 513;
#else
    unsigned long    temp_longword  = 16909060L;
    unsigned short   temp_short	    = 258;
#endif
/* The above constants will produce numerical byte values of 1,2,3,4 or */
/* 4,3,2,1; depending on system and constant chosen.                    */

    extern  int	    need_init;
    extern  char    DMQA__long_order[4];
    extern  char    DMQA__short_order[2];
	    char    *char_ptr;

    if (need_init)
    {
	char_ptr	      = (char *) &temp_longword;
	DMQA__long_order[0]   = *char_ptr++;
	DMQA__long_order[1]   = *char_ptr++;
	DMQA__long_order[2]   = *char_ptr++;
	DMQA__long_order[3]   = *char_ptr;
	/* This array determines how our system is storing bytes. We store a */
	/* constant and examine the resulting bytes */

	char_ptr	       = (char *) &temp_short;
	DMQA__short_order[0]   = *char_ptr++;
	DMQA__short_order[1]   = *char_ptr;
	/* Same idea, but for shorts */

	need_init = 0;
	/* We need only do this once */
    }
}


/* This function converts an unsigned long  HOST value (local to our system) */
/* into an unsigned longword in the proper byte order for networking usage.  */

unsigned long DMQA__htonl(ulong_value)
unsigned long ulong_value;
/* Convert host order to network order for longwords */
{
#if DMQA__REVERSE_BITS
    extern  unsigned char   bit_array[16];
	    unsigned char   lowbits;	    /* Temp area for storing lowbits */
	    unsigned char   highbits;	    /* temp area for high bits */
#endif
    extern  int		    need_init;
    extern  char	    DMQA__long_order[4];
	    unsigned char   *charptr;	    /* ptr to local variable */
	    unsigned char   *charptr2;	    /* ptr to argument value */
	    unsigned long   local_long;	    /* local longword */
	    int		    i = 0;	    /* index */
	    unsigned char   local_char;	    /* Local byte */


    if (need_init) DMQA__init_nbo();	    /* Init if needed */

    local_long  = 0;                        /* Init variable */

    charptr	= (unsigned char *) &local_long;
    charptr2	= (unsigned char *) &ulong_value;
    /* Set up pointers */

    do
    {
	local_char = *charptr2++;	    /* Grab a byte */

#if DMQA__REVERSE_BITS
	highbits    = bit_array[local_char & 15] << 4;
	lowbits	    = bit_array[(local_char >> 4)];
	local_char  = lowbits | highbits;
	/* Reverse the bits if desired */
#endif
	*(charptr + (DMQA__long_order[i] - 1)) = local_char;
	/* copy the byte to its "correct" location in our local variable */
    } while (++i < 4);

    return local_long;
}



/* This function converts an unsigned short HOST value (local to our system) */
/* into an unsigned short in the proper byte order for networking usage.     */

unsigned short DMQA__htons(ushort_value)
unsigned short ushort_value;
/* Convert host order to network order for shortwords */
{
#if DMQA__REVERSE_BITS
    extern  unsigned char   bit_array[16];
	    unsigned char   lowbits;	    /* Temp area for storing lowbits */
	    unsigned char   highbits;	    /* temp area for high bits */
#endif
    extern  int		    need_init;
	    unsigned char   *charptr;	    /* ptr to local variable */
	    unsigned char   *charptr2;	    /* ptr to argument value */
	    unsigned short  local_short;    /* local shortword */
	    int		    i = 0;	    /* index */
	    unsigned char   local_char;	    /* Local byte */


    if (need_init) DMQA__init_nbo();	    /* init if needed */

    local_short = 0;                        /* Init variable */

    charptr	= (unsigned char *) &local_short;
    charptr2	= (unsigned char *) &ushort_value;
    /* Set up pointers */

    do
    {
	local_char = *charptr2++;	    /* Grab a byte */

#if DMQA__REVERSE_BITS
	highbits    = bit_array[local_char & 15] << 4;
	lowbits	    = bit_array[(local_char >> 4)];
	local_char  = lowbits | highbits;
	/* Reverse the bits if desired */
#endif
	*(charptr + (DMQA__short_order[i] - 1)) = local_char;
	/* Copy byte into its "correct" location in out local variable */
    } while (++i < 2);

    return local_short;
}



/* This function converts an unsigned long in the NETWORK order (produced */
/* by DMQA__htonl) into unsigned long HOST format (local to our system)   */

unsigned long DMQA__ntohl(ulong_value)
unsigned long ulong_value;
/* Convert network order to host order for longwords*/
{
    extern	int		need_init;

    if (need_init) DMQA__init_nbo();
    return DMQA__htonl(ulong_value);
    /* Just call the same function again -- it undoes itself */
}



/* This function converts an unsigned short in the NETWORK order (produced */
/* by DMQA__htonl) into unsigned short HOST format (local to our system)   */

unsigned short DMQA__ntohs(ushort_value)
unsigned short ushort_value;
/* Convert network order to host order for shortwords */
{
    extern	int		need_init;

    if (need_init) DMQA__init_nbo();
    return (unsigned short) DMQA__htons(ushort_value);
    /* Just call the same function again -- it undoes itself */
}

