/*  Module dmqa_ipi.c c DmQA V1.0        DmQA internal program interface    */
/*									    */
/*									    */
/*  Copyright (c) 1990, 1991 by						    */
/*  Digital Equipment Corporation, Maynard, Mass.			    */
/*									    */
/*  This software is furnished under a license and may be used and copied   */
/*  only  in  accordance  with  the  terms  of such  license and with the   */
/*  inclusion of the above copyright notice. This software or  any  other   */
/*  copies thereof may not be provided or otherwise made available to any   */
/*  other person. No title to and ownership of  the  software  is  hereby   */
/*  transferred.							    */
/*									    */
/*  The information in this software is subject to change without  notice   */
/*  and  should  not be  construed  as  a commitment by Digital Equipment   */
/*  Corporation.							    */
/*									    */
/*  Digital assumes no responsibility for the use or  reliability  of its   */
/*  software on equipment which is not supplied by Digital.		    */
/*									    */
/*									    */
/*  DECmessageQ Expertise Center					    */
/*									    */
/*									    */
/*  This program is part of the Server end to the DECmessageQ Queue Adapter */
/*  Although it has conditional compiles for VMS and non-VMS systems,	    */
/*  it currently will only compile and function correctly under VMS.	    */
/*									    */
/*  The general idea here is to perform the PAMS function call, passing	    */
/*  information extracted from the buffer. All PAMS parameters that	    */
/*  are not string descriptors and are of access "read", we should be	    */
/*  able to just point directly into our buffer. Other types we'll	    */
/*  have to build temporary structures and pass pointers to them	    */
/*  instead.								    */
/*									    */
/*  PAMS parameters are stored in the message in reverse order. This	    */
/*  is to help make parameter extraction easier by making "strlen"	    */
/*  (string length) information  available before the string itself	    */
/*  is encountered. Numerical types longer than a byte (long, short,	    */
/*  etc) are mapped to a char string and stored in array element order.	    */
/*									    */
/*									    */
/*  IDENT HISTORY:							    */
/*									    */
/*  Version     Date    Who     Description				    */
/*									    */
/*  V1.0FT  08-Mar-1991 kgb     Initial Release; Keith Barrett		    */
/*  V1.0FT  09-Apr-1991	kgb	missing prototypes in process_client func.  */
/*  V1.0FT  16-Apr-1991 kgb	Added user_tag to pams_get_msgx		    */
/*  V1.0FT  25-Apr-1991 kgb     Changed several arguments (mostly queue     */
/*                              addresses) from longwords into 2 shortwords */
/*				Changed EBCDIC use of q_name in attach_q    */
/*				Enhanced test message display		    */
/*  V1.0FT  01-May-1991	kgb	Changed EBCDIC tbls and added AS/400 one    */
/*  V1.0FT  08-May-1991	kgb	Fixed PAMS_DCL_PROCESS			    */
/*  V1.0FT  09-May-1991	kgb	Added a special ascebc table for as400s	    */
/*  V1.0FT  22-May-1991	kgb	Added use of DMQA__force_bracket	    */
/*  V1.0FT  23-May-1991	kgb	Added DMQA__link_check reference	    */
/*  V1.0FT  29-May-1991	kgb	Changed SHOW_BUFFER_LEN processing	    */
/*  V1.0FT  04-Jun-1991	kgb	Switched "]" and "|" in AS/400 EBCDIC tbl   */
/*									    */


        /* Include standard "C" and system definition files. */

#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef VMS
#include <types.h>
#include <ssdef.h>
#include <descrip.h>
#include <lib$routines.h>
#pragma nostandard
#include pams_c_entry_point
#include pams_c_return_status_def
#pragma standard
#else
#ifndef AS400
#include <sys/types.h>
#define CONST
#endif
#include "p_entry.h"
#include "p_return.h"
#endif

#define DMQA__INTERNAL 1
#include    "p_dmqa.h"          /* DmQA defintions */

#define DMQA__MIN_PROTOCOL DMQA__PROTOCOL


#ifdef __USE_ARGS
extern unsigned long	DMQA__htonl(unsigned long);
extern unsigned short	DMQA__htons(unsigned short);
extern unsigned long	DMQA__ntohl(unsigned long);
extern unsigned short	DMQA__ntohs(unsigned short);
#else
extern unsigned long	DMQA__htonl();
extern unsigned short	DMQA__htons();
extern unsigned long	DMQA__ntohl();
extern unsigned short	DMQA__ntohs();
#endif

extern	char	DMQA__long_order[4];	/* defined in dmqin.c */
extern	char	DMQA__short_order[2];	/* defined in dmqin.c */

int		DMQA__os_aligns_data	= FALSE;
int		DMQA__os_swaps_bytes	= FALSE;
int		DMQA__os_is_EBCDIC	= DMQA__EBCDIC;
int		DMQA__trans_to_EBCDIC	= FALSE;    /* Not used by DMQA_IPI */
int		DMQA__trans_from_EBCDIC	= FALSE;    /* Not used by DMQA_IPI */
int	    	DMQA__msg_is_in_EBCDIC  = FALSE;    /* Not used by DMQA_IPI */
int		DMQA__debug		= 0;        /* debug mode           */
unsigned long   DMQA__task_id           = 0;        /* task-id              */
long    	DMQA__errno;			    /* Secondary status	    */
int		DMQA__program_id	= DMQA__SERVER_ID;
int		DMQA__force_bracket	= 0;	    /* Do not force "[]"    */
/* General parameters */



#ifdef VMS
unsigned char ascebc_table[256] = {
			    0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F,
			    0x16, 0x05, 0x15, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
			    0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26,
			    0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
			    0x40, 0x4F, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D,
			    0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
			    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
			    0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
			    0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
			    0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
			    0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,
			    0xE7, 0xE8, 0xE9, 0x4A, 0xE0, 0x5A, 0x5F, 0x6D,
			    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
			    0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
			    0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
			    0xA7, 0xA8, 0xA9, 0xC0, 0x6A, 0xD0, 0xA1, 0x07,

			    0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F,
			    0x16, 0x05, 0x15, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
			    0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26,
			    0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
			    0x40, 0x4F, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D,
			    0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
			    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
			    0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
			    0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
			    0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
			    0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,
			    0xE7, 0xE8, 0xE9, 0x4A, 0xE0, 0x5A, 0x5F, 0x6D,
			    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
			    0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
			    0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
			    0xA7, 0xA8, 0xA9, 0xC0, 0x6A, 0xD0, 0xA1, 0x07 };
/* ASCII to EBCDIC table */


unsigned char ascebc_as400[256] = {
			    0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F,
			    0x16, 0x05, 0x15, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
			    0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26,
			    0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
			    0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D,
			    0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
			    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
			    0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
			    0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
			    0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
			    0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,
			    0xE7, 0xE8, 0xE9, 0x4A, 0xE0, 0x4F, 0x5F, 0x6D,
			    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
			    0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
			    0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
			    0xA7, 0xA8, 0xA9, 0xC0, 0x6A, 0xD0, 0xA1, 0x07,

			    0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F,
			    0x16, 0x05, 0x15, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
			    0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26,
			    0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
			    0x40, 0x5F, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D,
			    0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
			    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
			    0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
			    0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
			    0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
			    0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,
			    0xE7, 0xE8, 0xE9, 0x4A, 0xE0, 0x6A, 0x5F, 0x6D,
			    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
			    0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
			    0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
			    0xA7, 0xA8, 0xA9, 0xC0, 0x6A, 0xD0, 0xA1, 0x07 };
/* ASCII to EBCDIC table for AS400 */


unsigned char ebcasc_table[256] = {
			    0x00, 0x01, 0x02, 0x03, 0xFF, 0x09, 0xFF, 0x7F,
			    0xFF, 0xFF, 0xFF, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
			    0x10, 0x11, 0x12, 0x13, 0xFF, 0x0A, 0x08, 0xFF,
			    0x18, 0x19, 0xFF, 0xFF, 0x1C, 0x1D, 0x1E, 0x1F,
			    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, 0x17, 0x1B,
			    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x05, 0x06, 0x07,
			    0xFF, 0xFF, 0x16, 0xFF, 0xFF, 0xFF, 0xFF, 0x04,
			    0xFF, 0xFF, 0xFF, 0xFF, 0x14, 0x15, 0xFF, 0x1A,
			    0x20, 0x26, 0x61, 0x00, 0x6B, 0x10, 0x1F, 0x62,
			    0x2B, 0x0B, 0x5B, 0x2E, 0x3C, 0x28, 0x2B, 0x21,
			    0x26, 0x29, 0x2A, 0x1C, 0x5B, 0x25, 0x19, 0x63,
			    0x28, 0x1E, 0x5D, 0x24, 0x2A, 0x29, 0x3B, 0x5E,
			    0x2D, 0x2F, 0x5F, 0x5C, 0x1A, 0x5D, 0x5E, 0x18,
			    0x1D, 0x2C, 0x7C, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
			    0x57, 0x08, 0x14, 0x30, 0x31, 0x32, 0x7C, 0x56,
			    0x7B, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,

			    0x78, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
			    0x68, 0x69, 0x16, 0x24, 0x73, 0x2F, 0x23, 0x45,
			    0x0C, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,
			    0x71, 0x72, 0x17, 0x07, 0x4E, 0x13, 0x71, 0x7E,
			    0x48, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
			    0x79, 0x7A, 0x6F, 0x40, 0x5A, 0x5B, 0x79, 0x79,
			    0x35, 0x36, 0x7D, 0x37, 0x38, 0x39, 0x66, 0x3B,
			    0x3C, 0x3D, 0x0D, 0x59, 0x3F, 0x5D, 0x58, 0x44,
			    0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
			    0x48, 0x49, 0x4B, 0x4A, 0x3E, 0x68, 0x6C, 0x6D,
			    0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
			    0x51, 0x52, 0x21, 0x2D, 0x75, 0x74, 0x23, 0x0F,
			    0x5C, 0x67, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
			    0x59, 0x5A, 0x20, 0x05, 0x0E, 0x69, 0x64, 0x51,
			    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
			    0x38, 0x39, 0xB3, 0xF7, 0xF0, 0xFA, 0xA7, 0xFF };
/* EBCDIC to ASCII table */


unsigned char ebcasc_as400[256] = {
			    0x00, 0x01, 0x02, 0x03, 0xFF, 0x09, 0xFF, 0x7F,
			    0xFF, 0xFF, 0xFF, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
			    0x10, 0x11, 0x12, 0x13, 0xFF, 0x0A, 0x08, 0xFF,
			    0x18, 0x19, 0xFF, 0xFF, 0x1C, 0x1D, 0x1E, 0x1F,
			    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, 0x17, 0x1B,
			    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x05, 0x06, 0x07,
			    0xFF, 0xFF, 0x16, 0xFF, 0xFF, 0xFF, 0xFF, 0x04,
			    0xFF, 0xFF, 0xFF, 0xFF, 0x14, 0x15, 0xFF, 0x1A,
			    0x20, 0xA6, 0xE1, 0x80, 0xEB, 0x90, 0x9F, 0xE2,
			    0xAB, 0x8B, 0x5B, 0x2E, 0x3C, 0x28, 0x2B, 0x5D,
			    0x26, 0xA9, 0xAA, 0x9C, 0xDB, 0xA5, 0x99, 0xE3,
			    0xA8, 0x9E, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0x5E,
			    0x2D, 0x2F, 0xDF, 0xDC, 0x9A, 0xDD, 0xDE, 0x98,
			    0x9D, 0xAC, 0x7C, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
			    0xD7, 0x88, 0x94, 0xB0, 0xB1, 0xB2, 0xFC, 0xD6,
			    0xFB, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,

			    0xF8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
			    0x68, 0x69, 0x96, 0xA4, 0xF3, 0xAF, 0xA3, 0xC5,
			    0x8C, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,
			    0x71, 0x72, 0x97, 0x87, 0xCE, 0x93, 0xF1, 0xFE,
			    0xC8, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
			    0x79, 0x7A, 0xEF, 0xC0, 0xDA, 0x5B, 0xF9, 0xF9,
			    0xB5, 0xB6, 0xFD, 0xB7, 0xB8, 0xB9, 0xE6, 0xBB,
			    0xBC, 0xBD, 0x8D, 0xD9, 0xBF, 0x5D, 0xD8, 0xC4,
			    0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
			    0x48, 0x49, 0xCB, 0xCA, 0xBE, 0xE8, 0xEC, 0xED,
			    0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
			    0x51, 0x52, 0xA1, 0xAD, 0xF5, 0xF4, 0xA3, 0x8F,
			    0x5C, 0xE7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
			    0x59, 0x5A, 0xA0, 0x85, 0x8E, 0xE9, 0xE4, 0xD1,
			    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
			    0x38, 0x39, 0xB3, 0xF7, 0xF0, 0xFA, 0xA7, 0xFF };
/* EBCDIC to ASCII table for the AS/400 (note: the "!" and "]" are inverse ) */

#endif


void DMQA__process_client()
{
    int			wait_for_cmd = TRUE;
    long		local_status;
    char		DMQA_buffer[DMQA__SIZMAX];
    char		*DMQA_buffer_ptr;
    DMQA__HEADER	DMQA_header_rec;
    unsigned long	seq_no;
    long		flags;
    short		msg_length;
    char		pams_command;
    char		local_buf[8];
    char		*local_chr_ptr;
    static		initialized = FALSE;
    char		protocol;
    unsigned long	task_id;
    long		local_flags;
    char		id;
    char		os;
    unsigned long	local_ulong = 67305985L;

#ifdef __USE_ARGS
    extern long		DMQA__link_send(int, char *, short, int, int, long);
    extern long		DMQA__link_receive(char *, short *, int, int, long);
    extern long		DMQA__link_check(long);
    extern long		DMQA__test(char *, short, short *, char *, long);
    extern long		DMQA_pams_dcl_process(char *, short *, long);
    extern long		DMQA_pams_attach_q(char *, short *, short, char, long);
    extern long		DMQA_pams_put_msg(char *, short *, short, char, long);
    extern long		DMQA_pams_get_msgw(char *, short *, short, char, long);
    extern long		DMQA_pams_get_msg(char *, short *, short, char, long);
    extern long		DMQA_pams_locate_q(char *, short *, short, char, long);
    extern long		DMQA_pams_confirm_msg(char *, short *, long);
    extern long		DMQA_pams_set_select(char *, short *, long);
    extern long		DMQA_pams_cancel_select(char *, short *, long);
    extern long		DMQA_pams_exit(	char *, short *, long);
#else
    extern long		DMQA__link_send();
    extern long		DMQA__link_receive();
    extern long		DMQA__link_check();
    extern long		DMQA__test();
    extern long		DMQA_pams_dcl_process();
    extern long		DMQA_pams_attach_q();
    extern long		DMQA_pams_put_msg();
    extern long		DMQA_pams_get_msgw();
    extern long		DMQA_pams_get_msg();
    extern long		DMQA_pams_locate_q();
    extern long		DMQA_pams_confirm_msg();
    extern long		DMQA_pams_set_select();
    extern long		DMQA_pams_cancel_select();
    extern long		DMQA_pams_exit();
#endif

#ifdef VMS
    unsigned long	    lib_status;
    unsigned char	    ae4_5b;
    unsigned char	    aet_5b;
    unsigned char	    ae4_5d;
    unsigned char	    aet_5d;
    unsigned char	    ea4_ad;
    unsigned char	    eat_ad;
    unsigned char	    ea4_bd;
    unsigned char	    eat_bd;
#pragma nostandard
    $DESCRIPTOR	    	    (ident_desc, DMQA__IDENT);
#pragma standard
    static char		    EBCDIC_ident[8];
    extern unsigned long    lib$tra_asc_ebc();
    static int		    routine_init = FALSE;
#endif
        
    struct
    {
        char    tmpchr;
        short   tmpshort;
        long    tmplong;
    } test_alignment;
    

    if (DMQA__debug)
	printf("%s *** Entering DMQA__process_client routine ***\n",
	    DMQA__DBG_PREFIX);

#ifdef VMS
    if (routine_init == 0)
    {
	if (DMQA__debug)
            printf("%s Creating an EBCDIC ident variable...\n",
		DMQA__DBG_PREFIX);

    	if ((lib_status = lib$tra_asc_ebc(&ident_desc, &ident_desc)) != SS$_NORMAL)
        {
	    if (DMQA__debug)
	        printf(
	        "%s Warning -- EBCDIC translation of ident failed with error %u\n",
		    DMQA__DBG_PREFIX, lib_status);

	    (void) sprintf(EBCDIC_ident,"%s","Unknown");
        }
        else
	    (void) memcpy(  (VOID *) EBCDIC_ident,
			    (CONST VOID *) ident_desc.dsc$a_pointer,
			    (size_t) sizeof(EBCDIC_ident));

	if (DMQA__force_bracket)
	{
	    if (DMQA__debug)
		printf(
		"%s !!! Setting EBCDIC tables to force '[]' translation !!!\n",
		    DMQA__DBG_PREFIX);

	    ascebc_as400[0x5B] = 0xAD;	    /* Was 4A	*/
	    ascebc_table[0x5B] = 0xAD;	    /* Was 4A	*/
	    ascebc_as400[0x5D] = 0xBD;	    /* Was 5A	*/
	    ascebc_table[0x5D] = 0xBD;	    /* Was 5A	*/

	    ebcasc_as400[0xAD] = 0x5B;	    /* Was DB	*/
	    ebcasc_table[0xAD] = 0x5B;	    /* Was DB	*/
	    ebcasc_as400[0xBD] = 0x5D;	    /* Was DD	*/
	    ebcasc_table[0xBD] = 0x5D;	    /* Was DD	*/

	    /* The above logic forces square backets to get translated into */
	    /* the values that the EBCDIC system uses for successful prog-  */
	    /* ram compiles AT ALL TIMES. This feature has to have been	    */
	    /* forced by the end-user, since using it actually causes mes   */
	    /* sage content to change and file comparisons will fail as a   */
	    /* result.							    */
	}

	ae4_5b = ascebc_as400[0x5B];
	aet_5b = ascebc_table[0x5B];
	ae4_5d = ascebc_as400[0x5D];
	aet_5d = ascebc_table[0x5D];
	ea4_ad = ebcasc_as400[0xAD];
	eat_ad = ebcasc_table[0xAD];
	ea4_bd = ebcasc_as400[0xBD];
	eat_bd = ebcasc_table[0xBD];
	/* Save the 'final' EBCDIC table values for '[]' translation */

	routine_init = TRUE;
    }
#endif

    while (wait_for_cmd)
    {
	local_status = DMQA__link_receive(DMQA_buffer, &msg_length, 0, 0, 0L);
	/* Receive assembled message */

	if ((local_status & 1) == 0)
	{
	    if (DMQA__debug)
		printf(
		"%s *** Exiting; DMQA__link_receive returned error %d, errno %d ***\n",
		    DMQA__DBG_PREFIX, local_status, errno);

	    wait_for_cmd = FALSE;
	    break;
	}

	(void) memcpy(	(VOID *) &DMQA_header_rec,
			(CONST VOID *) DMQA_buffer,
			(size_t) DMQA__HEADERSIZE);
	/* extract message header */

	DMQA_buffer_ptr = DMQA_buffer;
	/* reset pointer */

	msg_length  =
	    (short) DMQA__ntohs((unsigned short) DMQA_header_rec.msg_length);
	seq_no	= DMQA__ntohl(DMQA_header_rec.msg_seq_no);
	flags	=
	    (long) DMQA__ntohl((unsigned long) DMQA_header_rec.flags);
	pams_command    = DMQA_header_rec.cmd;
        id              = DMQA_header_rec.id;
        protocol        = DMQA_header_rec.protocol;
        task_id         = DMQA__ntohl(DMQA_header_rec.task_id);
	os		= DMQA_header_rec.os;
	/* copy header fields into useable variables */


        if (id != DMQA__CLIENT_ID)
        {
            if (DMQA__debug)
                printf("%s invalid client id received (%d)\n",
                    DMQA__DBG_PREFIX, id);
                    
            local_status = PAMS__INTERNAL;
            wait_for_cmd = FALSE;
        }

        if ((pams_command < 1) || (pams_command > 127))
        {
            if (DMQA__debug)
                printf("%s invalid pams command received (%d)\n",
                    DMQA__DBG_PREFIX, pams_command);
                    
            local_status = PAMS__INTERNAL;
            wait_for_cmd = FALSE;
        }
        
        if ((protocol < DMQA__MIN_PROTOCOL) || (protocol > DMQA__PROTOCOL))
        {
            if (DMQA__debug)
                printf("%s received unsupported protocol revision (%d)\n",
                    DMQA__DBG_PREFIX, protocol);
                    
            local_status = PAMS__NOTSUPPORTED;
            wait_for_cmd = FALSE;
        }

#ifdef VMS
	if (flags & DMQA_BIT__FORCE_BRACKET)
	{
	    if (DMQA__debug)
		printf(
		"%s !! User selected EBCDIC translation to force '[]' !!\n",
		    DMQA__DBG_PREFIX);

	    ascebc_as400[0x5B] = 0xAD;	    /* Was 4A	*/
	    ascebc_table[0x5B] = 0xAD;	    /* Was 4A	*/
	    ascebc_as400[0x5D] = 0xBD;	    /* Was 5A	*/
	    ascebc_table[0x5D] = 0xBD;	    /* Was 5A	*/

	    ebcasc_as400[0xAD] = 0x5B;	    /* Was DB	*/
	    ebcasc_table[0xAD] = 0x5B;	    /* Was DB	*/
	    ebcasc_as400[0xBD] = 0x5D;	    /* Was DD	*/
	    ebcasc_table[0xBD] = 0x5D;	    /* Was DD	*/
	}
	else
	{
	    ascebc_as400[0x5B] = ae4_5b;
	    ascebc_table[0x5B] = aet_5b;
	    ascebc_as400[0x5D] = ae4_5d;
	    ascebc_table[0x5D] = aet_5d;
	    ebcasc_as400[0xAD] = ea4_ad;
	    ebcasc_table[0xAD] = eat_ad;
	    ebcasc_as400[0xBD] = ea4_bd;
	    ebcasc_table[0xBD] = eat_bd;
	    /* Put them back to our norm */
	}
#endif

        if (local_status & 1)
        {
    	    if (DMQA__debug)
	        printf(
                    "%s == Command %d received; data is %d bytes long ==\n",
		    DMQA__DBG_PREFIX, pams_command, msg_length);
	    /* All set to go */

	    switch (pams_command)
	    {
	        case DMQA__TEST_MSG :
		    local_status = DMQA__test(DMQA_buffer, 0, &msg_length,
                        (char *) &DMQA_header_rec, 0L);
		    break;


	        case DMQA__PAMS_ATTACH_Q :
		    local_status =
                        DMQA_pams_attach_q( DMQA_buffer, &msg_length,
			    (short) (flags & DMQA_BIT__HEADER_IS_EBCDIC),
			    os, 0L);
		    break;


	        case DMQA__PAMS_LOCATE_Q :
		    local_status =
                        DMQA_pams_locate_q( DMQA_buffer, &msg_length,
			    (short) (flags & DMQA_BIT__HEADER_IS_EBCDIC),
			    os, 0L);
		    break;


	        case DMQA__PAMS_DCL_PROCESS :
		    local_status =
                        DMQA_pams_dcl_process(DMQA_buffer, &msg_length, 0L);
		    break;


	        case DMQA__PAMS_PUT_MSG :
		    local_status =
                        DMQA_pams_put_msg(DMQA_buffer, &msg_length,
			    (short) (flags & DMQA_BIT__TRANS_FROM_EBCDIC),
			    os, 0L);
		    break;


	        case DMQA__PAMS_GET_MSGW :
		    local_status =
                        DMQA_pams_get_msgw(DMQA_buffer, &msg_length,
			    (short) (flags & DMQA_BIT__TRANS_TO_EBCDIC),
			    os, 0L);
		    break;


	        case DMQA__PAMS_GET_MSG :
		    local_status =
                        DMQA_pams_get_msg (DMQA_buffer, &msg_length,
			    (short) (flags & DMQA_BIT__TRANS_TO_EBCDIC),
			    os, 0L);
		    break;


	        case DMQA__PAMS_CONFIRM_MSG :
		    local_status =
                        DMQA_pams_confirm_msg(DMQA_buffer, &msg_length, 0L);
		    break;


	        case DMQA__PAMS_SET_SELECT :
		    local_status =
                        DMQA_pams_set_select(DMQA_buffer, &msg_length, 0L);
		    break;


	        case DMQA__PAMS_CANCEL_SELECT :
		    local_status =
                        DMQA_pams_cancel_select(DMQA_buffer, &msg_length, 0L);
		    break;


	        case DMQA__PAMS_EXIT :
		    local_status =
                        DMQA_pams_exit(DMQA_buffer, &msg_length, 0L);
		    break;


	        default :
		    local_status = PAMS__NOTSUPPORTED;
		    if (DMQA__debug)
		        printf("%s unsupported PAMS cmd %d - skipping\n",
			    DMQA__DBG_PREFIX, pams_command);
		    msg_length = DMQA__HEADERSIZE;
		    break;

	    } /* End Switch statement */
        } /* End if */
        

        if (!initialized)
        {
            local_chr_ptr  = local_buf;
            DMQA__push_long(&local_chr_ptr, (char *) &local_ulong);
            /* test to see how longwords are stored */

            if ((DMQA__long_order[0]  != 1) || (DMQA__long_order[1]  != 2) ||
	        (DMQA__long_order[2]  != 3) || (DMQA__long_order[3]  != 4) ||
	        (DMQA__short_order[0] != 1) || (DMQA__short_order[1] != 2))
	            DMQA__os_swaps_bytes = TRUE;
            /* Does OS swap expected bytes? */
    
            if (sizeof(test_alignment) > 7) DMQA__os_aligns_data = TRUE;
            /* Does OS align datatypes? */

            initialized = TRUE;
        }
    
	local_flags = 0;
        if (flags & DMQA_BIT__TRANS_TO_EBCDIC)
				  local_flags |= DMQA_BIT__MSG_IN_EBCDIC;
	if (flags & DMQA_BIT__HEADER_IS_EBCDIC)
				  local_flags |= DMQA_BIT__HEADER_IS_EBCDIC;
        if (DMQA__os_swaps_bytes) local_flags |= DMQA_BIT__OS_SWAPS_BYTES;
        if (DMQA__os_aligns_data) local_flags |= DMQA_BIT__OS_ALIGNS_DATA;
        /* Build status field */

        DMQA_header_rec.pams_status     =
            (long) DMQA__htonl((unsigned long) local_status);
        DMQA_header_rec.error           = DMQA__errno;
        DMQA_header_rec.task_id         = DMQA__htonl(task_id);
        DMQA_header_rec.msg_seq_no      = DMQA__htonl(seq_no);
        DMQA_header_rec.flags           =
            (long) DMQA__htonl((unsigned long)  local_flags);
        DMQA_header_rec.msg_length      =
            (short) DMQA__htons((unsigned short) msg_length);


#ifdef VMS
	if (flags & DMQA_BIT__HEADER_IS_EBCDIC)
	{
	    if (DMQA__debug)
		printf("%s (using EBCDIC program ident)\n", DMQA__DBG_PREFIX);

		(void) memcpy(  (VOID *) &DMQA_header_rec.ident[0],
				(CONST VOID *) EBCDIC_ident,
				(size_t) sizeof(DMQA_header_rec.ident));
	}
	else
	    (void) memcpy(  (VOID *) &DMQA_header_rec.ident[0],
			    (CONST VOID *) DMQA__IDENT,
			    (size_t) sizeof(DMQA_header_rec.ident));
#else
	(void) memcpy(  (VOID *) &DMQA_header_rec.ident[0],
			(CONST VOID *) DMQA__IDENT,
			(size_t) sizeof(DMQA_header_rec.ident));
#endif
        /* copy ident */

        DMQA_header_rec.id          	= DMQA__SERVER_ID;
        DMQA_header_rec.os          	= DMQA__OS;
        DMQA_header_rec.protocol    	= DMQA__PROTOCOL;
        DMQA_header_rec.cmd         	= pams_command;
        DMQA_header_rec.type        	= 0;	/* filled in by comm logic */
	DMQA_header_rec.max_packetsize	= 0;	/*   "     "  "   "    "   */
        /* Fill in header */
    
        local_chr_ptr          = (char *) &DMQA_header_rec.lbyte_order;
        /* init pointer */
    
        *local_chr_ptr++   = DMQA__long_order[0];
        *local_chr_ptr++   = DMQA__long_order[1];
        *local_chr_ptr++   = DMQA__long_order[2];
        *local_chr_ptr     = DMQA__long_order[3];
        /* fill in long byte order */
    
        local_chr_ptr      = (char *) &DMQA_header_rec.sbyte_order;
        *local_chr_ptr++   = DMQA__short_order[0];
        *local_chr_ptr     = DMQA__short_order[1];
        /* fill in short byte order */
    
        if (DMQA__debug) printf("%s Copying header into message...\n",
            DMQA__DBG_PREFIX);

        (void) memcpy(  (VOID *) DMQA_buffer,
                        (CONST VOID *) &DMQA_header_rec,
                        (size_t) DMQA__HEADERSIZE);
        /* copy header into buffer */

        if (DMQA__debug)
            printf("%s sending response of %d bytes\n",
                DMQA__DBG_PREFIX, msg_length);

        local_status = DMQA__link_send(0, DMQA_buffer, msg_length, 0, 0, 0L);

        if ((local_status & 1) == 0)
        {
            if (DMQA__debug)
                printf("%s DMQA__link_send returned error %d, errno %d\n",
                    DMQA__DBG_PREFIX, local_status, DMQA__errno);

            wait_for_cmd = FALSE;
        }

    } /* End wait_for_cmd loop */

} /* end main function */


/****************************************************************************/
/*									    */
/*		        DMQA__test function			            */
/*									    */
/****************************************************************************/


long DMQA__test(msg_ptr, flag, length, hdr_ptr, nullarg)

char            *msg_ptr;
short           flag;
short           *length;
char            *hdr_ptr;
long            nullarg;
{
    DMQA__HEADER            *header_ptr;
    unsigned long           task_id;
    unsigned long           seq_no;
    unsigned long           ulong_test  = 67305985L;
    unsigned short          ushort_test = 513;
    long	            flags;
    char	            *local_msg_ptr;
    short	            psb_length;
    short	            int_length;
    short	            msg_size;
    short	            header_size;
    short	            buf_size;
    short	            local_short;
    char                    *char_ptr;
    extern unsigned long    DMQA__rcount;
    extern unsigned long    DMQA__scount;
    extern int              DMQA__os_aligns_data;
    extern int              DMQA__os_is_EBCDIC;

#ifdef __USE_ARGS
    extern unsigned long    DMQA__htonl(unsigned long);
    extern unsigned short   DMQA__htons(unsigned short);
    extern unsigned long    DMQA__ntohl(unsigned long);
    extern unsigned short   DMQA__ntohs(unsigned short);
#else    
    extern unsigned long    DMQA__htonl();
    extern unsigned short   DMQA__htons();
    extern unsigned long    DMQA__ntohl();
    extern unsigned short   DMQA__ntohs();
#endif

#ifdef VMS
    unsigned long	    lib_status;
    extern unsigned long    lib$movtc();
    struct dsc$descriptor_s EBCDIC_struct;
#pragma nostandard
    $DESCRIPTOR		    (EBCDIC_fill, " ");
#pragma standard
    struct dsc$descriptor_s EBCDIC_table;
#endif


    if (DMQA__debug)
	printf("%s *** Entering DMQA_test routine ***\n", DMQA__DBG_PREFIX);

#ifdef VMS
    EBCDIC_table.dsc$w_length	= (unsigned short) sizeof(ebcasc_table);
    EBCDIC_table.dsc$b_dtype	= DSC$K_DTYPE_T;
    EBCDIC_table.dsc$b_class	= DSC$K_CLASS_S;
    EBCDIC_table.dsc$a_pointer	= (char *) ebcasc_table;
#endif
        
    header_ptr      = (DMQA__HEADER *) hdr_ptr;
    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;
    /* init pointers */
    
    if (*length - DMQA__HEADERSIZE != ( sizeof(psb_length)  +
                                        sizeof(int_length)  +
                                        sizeof(msg_size)    +
                                        sizeof(header_size) +
                                        sizeof(buf_size)))
	if (DMQA__debug)
	    printf("%s msg has incorrect length\n", DMQA__DBG_PREFIX);

    task_id = DMQA__ntohl(header_ptr -> task_id);
    /* task id */

    flags = (long) DMQA__ntohl((unsigned long) header_ptr -> flags);
    /* flags */

    seq_no = DMQA__ntohl(header_ptr -> msg_seq_no);
    /* sequence number */

    psb_length = (short) DMQA__pop_short(&local_msg_ptr);
    /* get psb length */

    msg_size = (short) DMQA__pop_short(&local_msg_ptr);
    /* get max msg size */

    buf_size = (short) DMQA__pop_short(&local_msg_ptr);
    /* buffer max size */

    header_size = (short) DMQA__pop_short(&local_msg_ptr);
    /* header size (better be same!) */

    int_length = DMQA__pop_short(&local_msg_ptr);
    /* get int length */


    if (flag | DMQA__debug)
    {
	printf("\n\nServer Configuration:\n");
	printf("       Identification: %s\n", DMQA__IDENT);
	printf("    Protocol Revision: %d\n", DMQA__PROTOCOL);
	printf("         Integer size: %d\n", sizeof(int));
	printf("   Defined Headersize: %d\n", DMQA__HEADERSIZE);
	printf("    Actual Headersize: %d\n", sizeof(DMQA__HEADER));
	printf("    OS Identification: %d\n", DMQA__OS);
	printf("           Buffersize: %d\n", DMQA__SIZMAX);
	printf("    Max PAMS msg size: %d\n", DMQA__MSGMAX);
	printf("          Size of PSB: %d\n", sizeof (struct psb));
	printf("   Current Send Count: %u\n", DMQA__scount);
	printf("   Current Recv Count: %u\n", DMQA__rcount);
	char_ptr = (char *) &ulong_test;
	printf("      LONG byte order: %d %d %d %d\n",	*char_ptr,
							*(char_ptr+1),
							*(char_ptr+2),
							*(char_ptr+3));
	char_ptr = (char *) &ushort_test;
	printf("     SHORT byte order: %d %d\n",	*char_ptr,
							*(char_ptr+1));
	printf("      OS aligns data?: %d\n", DMQA__os_aligns_data);
        printf("       ShowBuffer Max: %d\n", DMQA__SHOWBUFMAX);
        printf("        Name List Max: %d\n", DMQA__NAMELISTMAX);
        printf("       Sel. Array Max: %d\n", DMQA__SELARRAYMAX);
	printf("        DEBUG setting: %d\n", DMQA__debug);
	printf("              Task ID: %u\n", DMQA__task_id);
	printf("           Program ID: %d\n", DMQA__program_id);
	printf("        Offspring max: %d\n", DMQA__OFFSPRING);

	if (((char) 128) > 127)
		printf("         char default: unsigned\n");
 	else
		printf("         char default: unsigned\n");

	printf("\n\n\nClient Configuration:\n");

	if (flags & DMQA_BIT__HEADER_IS_EBCDIC)
	{
#ifdef VMS
	    char_ptr = (char *) header_ptr -> ident;

	    EBCDIC_struct.dsc$w_length	=
		(unsigned short) sizeof(char_ptr);
	    EBCDIC_struct.dsc$b_dtype	= DSC$K_DTYPE_T;
	    EBCDIC_struct.dsc$b_class	= DSC$K_CLASS_S;
	    EBCDIC_struct.dsc$a_pointer	= char_ptr;

	    if (header_ptr -> os == DMQA__AS400_OS)
	    {
		if (DMQA__debug)
		    printf("%s using AS/400 EBCDIC table\n",
			DMQA__DBG_PREFIX);
		EBCDIC_table.dsc$a_pointer = (char *) ebcasc_as400;
	    }
	    /* AS/400 has some special oddities */

	    if ((lib_status = lib$movtc(    &EBCDIC_struct,
					    &EBCDIC_fill,
					    &EBCDIC_table,
					    &EBCDIC_struct)) != SS$_NORMAL)
		printf("       Identification: <in EBCDIC, not displayable>\n");
	    else
		printf("       Identification: %s\n", header_ptr -> ident);
	}
	else
        {
	    printf("       Identification: %s\n", header_ptr -> ident);
#else
            printf("       Identification: <in EBCDIC, not displayable>\n");
#endif
        }

	printf("    Protocol Revision: %d\n", header_ptr -> protocol);
	printf("         Integer size: %d\n", int_length);
	printf("      DMQA Headersize: %d\n", header_size);
	printf("    OS Identification: %d\n", header_ptr -> os);
	printf("           Buffersize: %d\n", buf_size);
	printf("    Max PAMS msg size: %d\n", msg_size);
	printf("          Size of PSB: %d\n", psb_length);
	printf("           Msg Seq No: %u\n",
	    DMQA__ntohl(header_ptr -> msg_seq_no));
	char_ptr = (char *) &(header_ptr -> lbyte_order);
	printf("      LONG byte order: %d %d %d %d\n",	*char_ptr,
							*(char_ptr+1),
							*(char_ptr+2),
							*(char_ptr+3));
	char_ptr = (char *) &(header_ptr -> sbyte_order);
	printf("     SHORT byte order: %d %d\n",	*char_ptr,
							*(char_ptr+1));
	printf("                flags: %d\n", flags);
	printf("     Trans to EBCDIC?: %d\n",
	     (flags & DMQA_BIT__TRANS_TO_EBCDIC));
	printf("   Trans from EBCDIC?: %d\n",
	    (flags & DMQA_BIT__TRANS_FROM_EBCDIC));
	printf("      OS aligns data?: %d\n",
	    (flags & DMQA_BIT__OS_ALIGNS_DATA));

	printf("\n\n");
    }

    local_msg_ptr = msg_ptr + DMQA__HEADERSIZE;

    local_short = sizeof(struct psb);
    DMQA__push_short(&local_msg_ptr, (char *) &local_short);

    local_short = DMQA__MSGMAX;
    DMQA__push_short(&local_msg_ptr, (char *) &local_short);

    local_short = DMQA__SIZMAX;
    DMQA__push_short(&local_msg_ptr, (char *) &local_short);

    local_short = DMQA__HEADERSIZE;
    DMQA__push_short(&local_msg_ptr, (char *) &local_short);

    local_short = sizeof(int);
    DMQA__push_short(&local_msg_ptr, (char *) &local_short);

    *length = local_msg_ptr - msg_ptr;
    return (long) PAMS__SUCCESS;
}

/****************************************************************************/
/*									    */
/*	               DMQA_pams_dcl_process function			    */
/*									    */
/****************************************************************************/

long DMQA_pams_dcl_process(msg_ptr, length, nullarg)

char	*msg_ptr;
short   *length;
long    nullarg;
{
    int             local_int;
    char            *local_msg_ptr;
    long            req_process_num;
    long            process_num;
    long            q_type;
    long            *q_type_ptr     = 0;
    char            q_name[32];
    char            *q_name_ptr     = 0;
    long            q_name_len;
    long            *q_name_len_ptr = 0;
    long            name_scope;
    long            *name_scope_ptr = 0;
    long            local_status;
    short	    short_array[2];


    if (DMQA__debug)
    {
	printf("%s *** Entering DMQA_pams_dcl_process routine ***\n",
            DMQA__DBG_PREFIX);

	printf("%s Processing arguments...\n", DMQA__DBG_PREFIX);
    }


			/** Parse Arguments **/

        
    if (    (*length - DMQA__HEADERSIZE < ( sizeof(name_scope)      +
                                            sizeof(q_name_len)      +
                                            1                       +
                                            sizeof(q_type)          +
                                            sizeof(req_process_num)))

        ||  (*length - DMQA__HEADERSIZE > ( sizeof(name_scope)      +
                                            sizeof(q_name_len)      +
                                            sizeof(q_name)          +
                                            sizeof(q_type)          +
                                            sizeof(req_process_num))))
    {
	if (DMQA__debug)
	{
	    printf("%s msg has incorrect length (%d)\n",
                DMQA__DBG_PREFIX, *length - DMQA__HEADERSIZE);
            DMQA__DBG_EXIT(PAMS__INTERNAL);
	}
        *length = DMQA__HEADERSIZE;
        return (long) PAMS__INTERNAL;
    }


    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;

    for (   local_int = 0;
            local_int < sizeof(q_name);
            ++local_int)
        q_name[local_int] = 0;
    /* init our array */
    
    name_scope  = (long) DMQA__pop_long(&local_msg_ptr);    /* name_scope */
    if (name_scope) name_scope_ptr = &name_scope;
    
    q_name_len  = (long) DMQA__pop_long(&local_msg_ptr);    /* q_name_len */
    if (q_name_len) q_name_len_ptr = &q_name_len;

    if (q_name_len)                                         /* q_name */
    {
        (void) memcpy(  (VOID *) q_name,
                        (CONST VOID *) local_msg_ptr,
                        (size_t) q_name_len);
        local_msg_ptr   += q_name_len;
        q_name_ptr       = q_name;
     }
     
    q_name[q_name_len] = *local_msg_ptr++;
    q_name[31] = 0;
    /* ensure terminating null */
    
    q_type          = (long) DMQA__pop_long(&local_msg_ptr);    /* q_type */
    if (q_type) q_type_ptr = &q_type;
    
    /* req_process_num is one of those arguments that is data typed as a    */
    /* longword, but is really used by applications as a pair of shorts.   */
    
    short_array[0] = (short) DMQA__pop_short(&local_msg_ptr);
    short_array[1] = (short) DMQA__pop_short(&local_msg_ptr);
    
    (void) memcpy(  (VOID *) &req_process_num,
                    (CONST VOID *) &short_array[0],
                    (size_t) sizeof(req_process_num));

    if (DMQA__debug)
    {
        printf("%s calling pams_dcl_process with the following arguments:\n",
            DMQA__DBG_PREFIX);

        printf("%s  req_process_num: %d %d\n",
            DMQA__DBG_PREFIX, short_array[1], short_array[0]);

        printf("%s           q_type: ", DMQA__DBG_PREFIX);
        if (q_type_ptr)
            printf("%d\n", *q_type_ptr);
        else
            printf("null\n");

        printf("%s           q_name: ", DMQA__DBG_PREFIX);
        if (q_name_len_ptr)
        {
            if (q_name_ptr)
                printf("%s\n", q_name_ptr);
            else
                printf("null\n");

        printf("%s       q_name_len: %d\n", DMQA__DBG_PREFIX,
                *q_name_len_ptr);
        }
        else
        {
            printf("null\n");
            printf("%s       q_name_len: null\n", DMQA__DBG_PREFIX);
        }
        
        printf("%s   name_scope_ptr: ", DMQA__DBG_PREFIX);
        if (name_scope_ptr)
            printf("%d\n", *name_scope_ptr);
        else
            printf("null\n");
    }

    local_status    = pams_dcl_process( &req_process_num,
                                        &process_num,
                                        q_type_ptr,
                                        q_name_ptr,
                                        q_name_len_ptr,
                                        name_scope_ptr);
    if (DMQA__debug) printf("\n");                
    /* Perform the pams call */

    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;
    /* reset pointer */

    /* process_num is one of those variables that is datatyped as longword  */
    /* but is really used by applications as a pair of short words.         */

    (void) memcpy(  (VOID *) &short_array[0],
                    (CONST VOID *) &process_num,
                    (size_t) sizeof(process_num));

    DMQA__push_short(&local_msg_ptr, (char *) &short_array[0]);
    DMQA__push_short(&local_msg_ptr, (char *) &short_array[1]);

    if (DMQA__debug) DMQA__DBG_EXIT(local_status);

    *length = local_msg_ptr - msg_ptr;
    return local_status;
}

/****************************************************************************/
/*									    */
/*	               DMQA_pams_attach_q function			    */
/*									    */
/****************************************************************************/

long DMQA_pams_attach_q(msg_ptr, length, eflag, os, nullarg)

char	*msg_ptr;
short   *length;
short	eflag;
char	os;
long    nullarg;
{
    int             local_int;
    char            *local_msg_ptr;
    long            attach_mode;
    long            q_attached;
    long            q_type;
    long            *q_type_ptr             = 0;
    char            q_name[32];
    char            *q_name_ptr             = 0;
    long            q_name_len;
    long            *q_name_len_ptr         = 0;
    long            name_space_list_len;
    long            *name_space_list_len_ptr= 0;
    long            name_space_list[DMQA__NAMELISTMAX];
    long            *name_space_list_ptr    = 0;
    long            local_status;
    long            nullarg1;
    long            *nullarg1_ptr           = 0;
    long            nullarg2;
    long            *nullarg2_ptr           = 0;
    long            nullarg3;
    long            *nullarg3_ptr           = 0;
    short           short_array[2];


#ifdef VMS
    unsigned long	    lib_status;
    extern unsigned long    lib$movtc();
    struct dsc$descriptor_s EBCDIC_struct;
#pragma nostandard
    $DESCRIPTOR		    (EBCDIC_fill," ");
#pragma standard
    struct dsc$descriptor_s EBCDIC_table;
#endif
    

    if (DMQA__debug)
	printf("%s *** Entering DMQA_pams_attach_q routine ***\n",
            DMQA__DBG_PREFIX);
            
#ifdef VMS
    EBCDIC_table.dsc$w_length	= (unsigned short) sizeof(ebcasc_table);
    EBCDIC_table.dsc$b_dtype	= DSC$K_DTYPE_T;
    EBCDIC_table.dsc$b_class	= DSC$K_CLASS_S;
    EBCDIC_table.dsc$a_pointer	= (char *) ebcasc_table;

    if (os == DMQA__AS400_OS)
    {
	if (DMQA__debug)
	    printf("%s using AS/400 EBCDIC table\n",
		DMQA__DBG_PREFIX);
	EBCDIC_table.dsc$a_pointer = (char *) ebcasc_as400;
    }
    /* AS/400 has some special oddities */

#endif
        

			/** Parse Arguments **/


    if (DMQA__debug)
	printf("%s Processing arguments...\n", DMQA__DBG_PREFIX);

    if (*length - DMQA__HEADERSIZE < (  sizeof(nullarg1)            +
                                        sizeof(nullarg2)            +
                                        sizeof(nullarg3)            +
                                        sizeof(name_space_list_len) +
                                        sizeof(q_name_len)          +
                                        sizeof(q_type)              +
                                        sizeof(attach_mode)))
    {
    	if (DMQA__debug)
	{
	    printf("%s msg has incorrect length (%d)\n",
                DMQA__DBG_PREFIX, *length - DMQA__HEADERSIZE);
            DMQA__DBG_EXIT(PAMS__INTERNAL);
	}
        *length = DMQA__HEADERSIZE;
        return (long) PAMS__INTERNAL;
    }

    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;

    for (   local_int = 0;
            local_int < sizeof(q_name);
            ++local_int)
        q_name[local_int] = 0;
    /* init our q_name array */
    
        for (   local_int = 0;
            local_int < DMQA__NAMELISTMAX;
            ++local_int)
        name_space_list[local_int] = 0;
    /* init our name_space_list array */
    
    nullarg3 = (long) DMQA__pop_long(&local_msg_ptr);       /* nullarg */
    if (nullarg3) nullarg3_ptr = &nullarg3;
    
    nullarg2 = (long) DMQA__pop_long(&local_msg_ptr);       /* nullarg */
    if (nullarg2) nullarg2_ptr = &nullarg2;
    
    nullarg1 = (long) DMQA__pop_long(&local_msg_ptr);       /* nullarg */
    if (nullarg1) nullarg1_ptr = &nullarg1;
    
    name_space_list_len =
        (long) DMQA__pop_long(&local_msg_ptr);   /* name_space_list_len */

    if (name_space_list_len)
    {
        if (name_space_list_len > DMQA__NAMELISTMAX)
        {
            if (DMQA__debug)
            {
                printf("%s name_space_list_len exceeds maximum (%d)\n",
                    DMQA__DBG_PREFIX, DMQA__NAMELISTMAX);
                DMQA__DBG_EXIT(PAMS__INTERNAL);
            }
            *length = DMQA__HEADERSIZE;
            return (long) PAMS__INTERNAL;
        }

        name_space_list_len_ptr = &name_space_list_len;
        local_int               = 0;
        
        while (local_int < name_space_list_len)
            name_space_list[local_int++] =
                (long) DMQA__pop_long(&local_msg_ptr);
        /* copy name space */
    }


    q_name_len  = (long) DMQA__pop_long(&local_msg_ptr);    /* q_name_len */

    if (q_name_len)
    {
        q_name_len_ptr = &q_name_len;

        (void) memcpy(  (VOID *) q_name,                    /* q_name */
                        (CONST VOID *) local_msg_ptr,
                        (size_t) q_name_len);
        local_msg_ptr   += q_name_len;
        q_name_ptr       = q_name;

    q_name[q_name_len] = *local_msg_ptr++;
    q_name[31] = 0;
    /* ensure terminating null */
    
#ifdef VMS
	if (eflag != 0)
	{
	    if (DMQA__debug)
		printf("%s Translating EBCDIC q_name...\n", 
		    DMQA__DBG_PREFIX);

	    EBCDIC_struct.dsc$w_length	= (unsigned short) q_name_len;
	    EBCDIC_struct.dsc$b_dtype	= DSC$K_DTYPE_T;
	    EBCDIC_struct.dsc$b_class	= DSC$K_CLASS_S;
	    EBCDIC_struct.dsc$a_pointer	= q_name;

	    if ((lib_status = lib$movtc(&EBCDIC_struct,
					&EBCDIC_fill,
					&EBCDIC_table,
					&EBCDIC_struct)) != SS$_NORMAL)
	    {
		if (DMQA__debug)
		{
		    printf("%s EBCDIC translation failed with LIB$ error %u\n",
			DMQA__DBG_PREFIX, lib_status);
		    DMQA__DBG_EXIT(PAMS__INTERNAL);
		}

		*length = DMQA__HEADERSIZE;
		DMQA__errno = (long) lib_status;
		return (long) PAMS__INTERNAL;
	    }
	}
#endif

     }
     else
	local_msg_ptr++;		/* Remove trailing null */
     

    q_type          = (long) DMQA__pop_long(&local_msg_ptr);    /* q_type */
    if (q_type) q_type_ptr = &q_type;
    
    attach_mode     = (long) DMQA__pop_long(&local_msg_ptr); /* attach_mode */

    if (DMQA__debug)
    {
        printf("%s calling pams_attach_q with the following parameters:\n",
            DMQA__DBG_PREFIX);
        printf("%s     attach_mode: %d\n", DMQA__DBG_PREFIX, attach_mode);

        printf("%s          q_type: ", DMQA__DBG_PREFIX);
        if (q_type_ptr)
            printf("%d\n", *q_type_ptr);
        else
            printf("null\n");

        printf("%s          q_name: ", DMQA__DBG_PREFIX);
        if (q_name_len_ptr)
        {
            if (q_name_ptr)
                printf("%s\n", q_name_ptr);
            else
                printf("null\n");

            printf("%s      q_name_len: %d\n", DMQA__DBG_PREFIX,
                *q_name_len_ptr);
        }
        else
        {
            printf("null\n");
            printf("%s      q_name_len: null\n", DMQA__DBG_PREFIX);
        }
        
        printf("%s  name_space_len: ", DMQA__DBG_PREFIX);
        if (name_space_list_len_ptr)
        {
            printf("%d\n", *name_space_list_len_ptr);

            for (   local_int = 0;
                    local_int < *name_space_list_len_ptr;
                    ++local_int)
		printf("%s   name_space %d: %d\n",
                    DMQA__DBG_PREFIX, local_int, name_space_list[local_int]);
        }
        else
            printf("null\n");
            
        if (nullarg1_ptr)
            printf("%s            nullarg1: Yes\n", DMQA__DBG_PREFIX);
        if (nullarg2_ptr)
            printf("%s            nullarg2: Yes\n", DMQA__DBG_PREFIX);
        if (nullarg3_ptr)
            printf("%s            nullarg3: Yes\n", DMQA__DBG_PREFIX);
    }

    local_status    = pams_attach_q(    &attach_mode,
                                        &q_attached,
                                        q_type_ptr,
                                        q_name_ptr,
                                        q_name_len_ptr,
                                        name_space_list_ptr,
                                        name_space_list_len_ptr,
                                        (ANY *) nullarg1_ptr,
                                        (ANY *) nullarg2_ptr,
                                        (ANY *) nullarg3_ptr);
    if (DMQA__debug) printf("\n");                
    /* Perform the pams call */

    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;
    /* reset pointer */
    

    /* q_attached is one of those variables that is datatyped as a longword */
    /* but is really used by applications as a pair of short words.         */

    (void) memcpy(  (VOID *) &short_array[0],
                    (CONST VOID *) &q_attached,
                    (size_t) sizeof(q_attached));

    DMQA__push_short(&local_msg_ptr, (char *) &short_array[0]);
    DMQA__push_short(&local_msg_ptr, (char *) &short_array[1]);


    *length = local_msg_ptr - msg_ptr;
    if (DMQA__debug) DMQA__DBG_EXIT(local_status);
    return local_status;
}

/****************************************************************************/
/*									    */
/*	               DMQA_pams_put_msg function			    */
/*									    */
/****************************************************************************/

long DMQA_pams_put_msg(msg_ptr, length, eflag, os, nullarg)

char	*msg_ptr;
short   *length;
short	eflag;
char	os;
long    nullarg;
{
    char        *local_msg_ptr;
    long        local_status;
    long        nullarg1;
    long        *nullarg1_ptr           = 0;
    long        nullarg2;
    long        *nullarg2_ptr           = 0;
    long        nullarg3;
    long        *nullarg3_ptr           = 0;
    char        priority;
    long        target;
    short       class;
    short       type;
    char        delivery;
    short       msg_size;
    long        timeout;
    long        *timeout_ptr            = 0;
    struct psb  psb;
    char        *uma_ptr                = 0;
    char        uma;
    long        *resp_q_ptr             = 0;
    long        resp_q;
    short       short_array[2];

#ifdef VMS
    unsigned long	    lib_status;
    extern unsigned long    lib$movtc();
    struct dsc$descriptor_s EBCDIC_struct;
#pragma nostandard
    $DESCRIPTOR		    (EBCDIC_fill," ");
#pragma standard
    struct dsc$descriptor_s EBCDIC_table;
#endif

    
    if (DMQA__debug)
	printf("%s *** Entering DMQA_pams_put_msg routine, eflag = %d ***\n",
            DMQA__DBG_PREFIX, eflag);

#ifdef VMS
    EBCDIC_table.dsc$w_length	= (unsigned short) sizeof(ebcasc_table);
    EBCDIC_table.dsc$b_dtype	= DSC$K_DTYPE_T;
    EBCDIC_table.dsc$b_class	= DSC$K_CLASS_S;
    EBCDIC_table.dsc$a_pointer	= (char *) ebcasc_table;

    if (os == DMQA__AS400_OS)
    {
	if (DMQA__debug)
	    printf("%s using AS/400 EBCDIC table\n",
		DMQA__DBG_PREFIX);
	EBCDIC_table.dsc$a_pointer = (char *) ebcasc_as400;
    }
    /* AS/400 has some special oddities */
#endif
        

			/** Parse Arguments **/

        
    if (DMQA__debug)
	printf("%s Processing arguments...\n", DMQA__DBG_PREFIX);

    if (*length - DMQA__HEADERSIZE < (  sizeof(nullarg1)    +
                                        sizeof(nullarg2)    +
                                        sizeof(nullarg3)    +
                                        sizeof(resp_q)      +
                                        sizeof(uma)         +
                                        sizeof(timeout)     +
                                        sizeof(msg_size)    +
                                        sizeof(delivery)    +
                                        sizeof(type)        +
                                        sizeof(class)       +
                                        sizeof(target)      +
                                        sizeof(priority)))
    {
    	if (DMQA__debug)
	{
	    printf("%s msg has incorrect length (%d)\n",
                DMQA__DBG_PREFIX, *length - DMQA__HEADERSIZE);
            DMQA__DBG_EXIT(PAMS__INTERNAL);
	}
        *length = DMQA__HEADERSIZE;
        return (long) PAMS__INTERNAL;
    }

    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;

    nullarg3 = (long) DMQA__pop_long(&local_msg_ptr);       /* nullarg  */
    if (nullarg3) nullarg3_ptr = &nullarg3;
    
    nullarg2 = (long) DMQA__pop_long(&local_msg_ptr);       /* nullarg  */
    if (nullarg2) nullarg2_ptr = &nullarg2;
    
    nullarg1 = (long) DMQA__pop_long(&local_msg_ptr);       /* nullarg  */
    if (nullarg1) nullarg1_ptr = &nullarg1;
    

    /* resp_q is one of those arguments that is data typed as a longword    */
    /* but is really used by applications as a pair of short words.         */
    
    short_array[0] = (short) DMQA__pop_short(&local_msg_ptr);
    short_array[1] = (short) DMQA__pop_short(&local_msg_ptr);
    
    (void) memcpy(  (VOID *) &resp_q,
                    (CONST VOID *) &short_array[0],
                    (size_t) sizeof(resp_q));
                    

    if (resp_q) resp_q_ptr = &resp_q;

    uma = *local_msg_ptr++;                                 /* uma      */
    if (uma) uma_ptr = &uma;
        
    timeout = (long) DMQA__pop_long(&local_msg_ptr);        /* timeout  */
    if (timeout) timeout_ptr = &timeout;

    msg_size = (short) DMQA__pop_short(&local_msg_ptr);     /* msg_size */
    delivery = *local_msg_ptr++;                            /* delivery */    
    type     = (short) DMQA__pop_short(&local_msg_ptr);     /* type     */
    class    = (short) DMQA__pop_short(&local_msg_ptr);     /* class    */


    /* target is one of those arguments that is data typed as a longword    */
    /* but is really used by applications as a pair of short words.         */
    
    short_array[0] = (short) DMQA__pop_short(&local_msg_ptr);
    short_array[1] = (short) DMQA__pop_short(&local_msg_ptr);
    
    (void) memcpy(  (VOID *) &target,
                    (CONST VOID *) &short_array[0],
                    (size_t) sizeof(target));
                    

    priority = *local_msg_ptr++;                            /* priority */

    if (msg_size)                                           /* msg_area */
    {
        if (msg_size > DMQA__MSGMAX)
        {
            if (DMQA__debug)
            {
                printf("%d msg_size (%d) exceeds maximum(%d)\n",
                    DMQA__DBG_PREFIX, msg_size, DMQA__MSGMAX);
                DMQA__DBG_EXIT(PAMS__MSGTOBIG);
            }
            *length = DMQA__HEADERSIZE;
            return (long) PAMS__MSGTOBIG;
        }
    }

#ifdef VMS
    if (eflag != 0)
    {
	if (DMQA__debug)
	    printf("%s translating EBCDIC message...\n", DMQA__DBG_PREFIX);

	EBCDIC_struct.dsc$w_length	= (unsigned short) msg_size;
	EBCDIC_struct.dsc$b_dtype	= DSC$K_DTYPE_T;
	EBCDIC_struct.dsc$b_class	= DSC$K_CLASS_S;
	EBCDIC_struct.dsc$a_pointer	= local_msg_ptr;

	if ((lib_status = lib$movtc(&EBCDIC_struct,
				    &EBCDIC_fill,
				    &EBCDIC_table,
				    &EBCDIC_struct)) != SS$_NORMAL)
	{
	    if (DMQA__debug)
	    {
		printf("%s EBCDIC translation failed with LIB$ error %u\n",
		    DMQA__DBG_PREFIX, lib_status);
		DMQA__DBG_EXIT(PAMS__INTERNAL);
	    }

	    *length = DMQA__HEADERSIZE;
	    DMQA__errno = (long) lib_status;
	    return (long) PAMS__INTERNAL;
	}
    }
#endif
                            
    if (DMQA__debug)
    {
        printf("%s calling pams_put_msg with the following parameters:\n",
            DMQA__DBG_PREFIX);
        printf("%s        msg_size: %d\n", DMQA__DBG_PREFIX, msg_size);
        printf("%s        priority: %d\n", DMQA__DBG_PREFIX, priority);
        printf("%s          target: %d %d\n", DMQA__DBG_PREFIX,
            short_array[1], short_array[0]);
        printf("%s           class: %d\n", DMQA__DBG_PREFIX, class);
        printf("%s            type: %d\n", DMQA__DBG_PREFIX, type);
        printf("%s        delivery: %d\n", DMQA__DBG_PREFIX, delivery);

        printf("%s         timeout: ", DMQA__DBG_PREFIX);
        if (timeout_ptr)
            printf("%d\n", *timeout_ptr);
        else
            printf("null\n");

        printf("%s             uma: ", DMQA__DBG_PREFIX);
        if (uma_ptr)
            printf("%d\n", *uma_ptr);
        else
            printf("null\n");

        printf("%s          resp_q: ", DMQA__DBG_PREFIX);
        if (resp_q_ptr)
        {
            (void) memcpy(  (VOID *) &short_array[0],
                            (CONST VOID *) resp_q_ptr,
                            (size_t) sizeof(*resp_q_ptr));
                            
            printf("%d %d\n", short_array[1], short_array[0]);
        }
        else
            printf("null\n");

        if (nullarg1_ptr)
            printf("%s        nullarg1: Yes\n", DMQA__DBG_PREFIX);
        if (nullarg2_ptr)
            printf("%s        nullarg2: Yes\n", DMQA__DBG_PREFIX);
        if (nullarg3_ptr)
            printf("%s        nullarg3: Yes\n", DMQA__DBG_PREFIX);
    }

    local_status    = pams_put_msg( (ANY *) local_msg_ptr,
                                    &priority,
                                    &target,
                                    &class,
                                    &type,
                                    &delivery,
                                    &msg_size,
                                    timeout_ptr,
                                    &psb,
                                    uma_ptr,
                                    resp_q_ptr,
                                    (ANY *) nullarg1_ptr,
                                    (ANY *) nullarg2_ptr,
                                    (ANY *) nullarg3_ptr);
    if (DMQA__debug) printf("\n");                
    /* Perform the pams call */

    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;
    /* reset pointer */

    DMQA__push_psb(&local_msg_ptr, (char *) &psb, sizeof(struct psb));
    /* return psb */
    
    if (DMQA__debug) DMQA__DBG_EXIT(local_status);

    *length = local_msg_ptr - msg_ptr;
    return local_status;
}

/****************************************************************************/
/*									    */
/*	               DMQA_pams_get_msgw function			    */
/*									    */
/****************************************************************************/

long DMQA_pams_get_msgw(msg_ptr, length, eflag, os, nullarg)

char	*msg_ptr;
short   *length;
short	eflag;
long    nullarg;
char	os;
{
    char        *local_msg_ptr;
    long        local_status;
    char        *nullarg1_ptr       = 0;	/* nullarg1 is user_tag    */
    long        nullarg2;
    long        *nullarg2_ptr       = 0;	/* nullarg2 is user_taglen */
    long        nullarg3;
    long        *nullarg3_ptr       = 0;
    char        priority;
    long        source              = 0;
    short       class               = 0;
    short       type                = 0;
    short       msg_area_len;
    short       len_data            = 0;
    long        timeout;
    struct psb  psb;
    long        sel_filter;
    long        *sel_filter_ptr     = 0;
    char        show_buffer[DMQA__SHOWBUFMAX];
    char	*show_buffer_ptr    = 0;
    long        *show_bufflen_ptr   = 0;
    long        show_bufflen;
    long        show_buffermax	    = DMQA__SHOWBUFMAX;
    char        *msg_area_ptr;
    int		longword_count	    = 0;
    long	local_long;
    int		local_int;
    short       short_array[2];
    long	check_status;

    long	timer_interval	    = 300;	    /* 30 seconds */
    long	local_timer;
    int		loop_flag	    = TRUE;

    
#ifdef VMS
    unsigned long	    lib_status;
    extern unsigned long    lib$movtc();
    struct dsc$descriptor_s EBCDIC_struct;
#pragma nostandard
    $DESCRIPTOR		    (EBCDIC_fill," ");
#pragma standard
    struct dsc$descriptor_s EBCDIC_table;
#endif

    if (DMQA__debug)
	printf("%s *** Entering DMQA_pams_get_msgw routine ***\n",
            DMQA__DBG_PREFIX);
        
#ifdef VMS
    EBCDIC_table.dsc$w_length	= (unsigned short) sizeof(ascebc_table);
    EBCDIC_table.dsc$b_dtype	= DSC$K_DTYPE_T;
    EBCDIC_table.dsc$b_class	= DSC$K_CLASS_S;
    EBCDIC_table.dsc$a_pointer	= (char *) ascebc_table;

    if (os == DMQA__AS400_OS)
    {
	if (DMQA__debug)
	    printf("%s using AS/400 EBCDIC table\n",
		DMQA__DBG_PREFIX);
	EBCDIC_table.dsc$a_pointer = (char *) ascebc_as400;
    }
    /* AS/400 has some special oddities */

#endif

        
			/** Parse Arguments **/


    if (DMQA__debug)
	printf("%s Processing arguments...\n", DMQA__DBG_PREFIX);

    if (*length - DMQA__HEADERSIZE < (  sizeof(nullarg3)        +
                                        sizeof(nullarg2)        +
                                        sizeof(show_bufflen)    +
                                        sizeof(sel_filter)      +
                                        sizeof(timeout)         +
                                        sizeof(msg_area_len)    +
                                        sizeof(priority)))
    {
    	if (DMQA__debug)
	{
	    printf("%s msg has incorrect length (%d)\n",
                DMQA__DBG_PREFIX, *length - DMQA__HEADERSIZE);
            DMQA__DBG_EXIT(PAMS__INTERNAL);
	}
        *length = DMQA__HEADERSIZE;
        return (long) PAMS__INTERNAL;
    }

    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;

    nullarg3 = (long) DMQA__pop_long(&local_msg_ptr);       /* nullarg  */
    if (nullarg3) nullarg3_ptr = &nullarg3;
    
    nullarg2 = (long) DMQA__pop_long(&local_msg_ptr);
    if (nullarg2)
    {
	nullarg2_ptr	= &nullarg2;		/* nullarg (user_taglen) */

	nullarg1_ptr	= local_msg_ptr;	/* nullarg (user_tag) */
	local_msg_ptr	+= nullarg2;
    }
    
    show_bufflen = (long) DMQA__pop_long(&local_msg_ptr);   /* show_bufflen */

    if (show_bufflen < show_buffermax)
        show_buffermax = show_bufflen;

    if (show_buffermax)
    {
	show_bufflen_ptr    = &show_buffermax;
	show_buffer_ptr	    = show_buffer;
    }
    /* establish what size we can handle as a showbuffer. Forcing  */
    /* the smallest common size possible will help ensure that the */
    /* the showbuffer status will have BUFFEROVF if needed.        */


    /* sel_filter is one of those arguments that is data typed as a     */
    /* longword, but is really used by applications as a pair of shorts */
    
    short_array[0] = (short) DMQA__pop_short(&local_msg_ptr);
    short_array[1] = (short) DMQA__pop_short(&local_msg_ptr);
    
    (void) memcpy(  (VOID *) &sel_filter,
                    (CONST VOID *) &short_array[0],
                    (size_t) sizeof(sel_filter));


    if (sel_filter) sel_filter_ptr = &sel_filter;

    timeout = (long) DMQA__pop_long(&local_msg_ptr);        /* timeout  */
    msg_area_len = (short) DMQA__pop_short(&local_msg_ptr); /* msg_area_len */
    priority = *local_msg_ptr++;                            /* priority */
    
    if (msg_area_len > DMQA__MSGMAX)
    {
        if (DMQA__debug)
        {
            printf("%d msg_area_len (%d) exceeds maximum(%d)\n",
                DMQA__DBG_PREFIX, msg_area_len, DMQA__MSGMAX);
                DMQA__DBG_EXIT(PAMS__MSGTOBIG);
        }
        *length = DMQA__HEADERSIZE;
        return (long) PAMS__MSGTOBIG;
    }
    /* Client's message exceeds our capabilities (but passed his?) */
    
    if (DMQA__debug)
    {
        printf("%s calling pams_get_msgw with the following parameters:\n",
            DMQA__DBG_PREFIX);
        printf("%s     msg_area_len: %d\n", DMQA__DBG_PREFIX, msg_area_len);
        printf("%s         priority: %d\n", DMQA__DBG_PREFIX, priority);
        printf("%s         len_data: %d\n", DMQA__DBG_PREFIX, len_data);
        printf("%s          timeout: %d\n", DMQA__DBG_PREFIX, timeout);

        printf("%s       sel_filter: ", DMQA__DBG_PREFIX);
        if (sel_filter_ptr)
            printf("%d %d\n", short_array[1], short_array[0]);
        else
            printf("null\n");

        printf("%s     show_bufflen: %d\n", DMQA__DBG_PREFIX, show_bufflen);
        printf("%s  show_buffer_max: %d\n", DMQA__DBG_PREFIX, show_buffermax);

        if (nullarg1_ptr)
            printf("%s         nullarg1: Yes\n", DMQA__DBG_PREFIX);
        if (nullarg2_ptr)
            printf("%s         nullarg2: Yes\n", DMQA__DBG_PREFIX);
        if (nullarg3_ptr)
            printf("%s         nullarg3: Yes\n", DMQA__DBG_PREFIX);
    }

    /* Perform the pams call. This routine gets a little tricky here.     */
    /* To save memory (rather than wasting 32K of buffering), we will use */
    /* our existing buffer to receive the message. To make room for our   */
    /* header record and argument list, we will offset the buffer ptr by  */
    /* the amount of space we need.                                       */

    msg_area_ptr =  msg_ptr                 +       /* start of area   */
                    (DMQA__HEADERSIZE       +       /* skip header     */
                    sizeof(show_bufflen)    +       /* skip len field  */
                    show_buffermax          +       /* skip showbuffer */
                    sizeof(struct psb)      +       /* skip psb        */
                    sizeof(short)           +       /* skip psblen     */
                    sizeof(len_data)        +       /* skip lendata    */
                    sizeof(type)            +       /* skip type       */
                    sizeof(class)           +       /* skip class      */
                    sizeof(source));                /* skip source     */
    /* calculate message offset within our buffer area */
                    
    local_timer	    = timeout;

    while (loop_flag)
    {
	if (timeout)
	{
	    if (local_timer < timer_interval)
	    {
		timer_interval = local_timer;
	        local_timer = 0;
	    }
	    else
		local_timer -= timer_interval;
	}

	if (DMQA__debug)
	    printf("%s Calling pams_get_msgw with timeout of %d (%d left)\n",
		DMQA__DBG_PREFIX, timer_interval, local_timer);

	local_status    = pams_get_msgw((ANY *) msg_area_ptr,
					&priority,
					&source,
					&class,
					&type,
					&msg_area_len,
					&len_data,
					&timer_interval,
					sel_filter_ptr,
					&psb,
					show_buffer_ptr,
					show_bufflen_ptr,
					(char *) nullarg1_ptr,
					(ANY *)  nullarg2_ptr,
					(ANY *)  nullarg3_ptr);

	if (DMQA__debug) printf("\n");
	loop_flag = FALSE;

	if (local_status == PAMS__TIMEOUT)
	{
	    if ((local_timer > 0) || (timeout == 0))
	    {
		if ((check_status = DMQA__link_check(0L)) != PAMS__SUCCESS)
		{
		    printf(
		    "%s DMQA__link_check indicates link is down - aborting\n",
			DMQA__DBG_PREFIX);
		}
		else
		    loop_flag = TRUE;
	    }
	}
    }

    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;
    /* reset pointer */

    DMQA__push_psb(&local_msg_ptr, (char *) &psb, sizeof(struct psb));
    /* return psb */
    
    DMQA__push_long(&local_msg_ptr, (char *) &show_buffermax);
    /* return size of show_buffer filled in */

    if (show_buffermax)
    {
	longword_count = show_buffermax / sizeof(long);

	for (	local_int = 0, show_buffer_ptr = show_buffer;
		local_int < longword_count;
		local_int++, show_buffer_ptr += sizeof(long))
	{
            if ((local_int >= 10) && (local_int <=13))
            {
                /* Some elements are really used as a pair of shortwords */
                
	        (void) memcpy(  (VOID *) &short_array[0],
			        (CONST VOID *) show_buffer_ptr,
			        (size_t) sizeof(long));

                DMQA__push_short(&local_msg_ptr, (char *) &short_array[0]);
                DMQA__push_short(&local_msg_ptr, (char *) &short_array[1]);
            }
            else
            {    
	        (void) memcpy(  (VOID *) &local_long,
			        (CONST VOID *) show_buffer_ptr,
			        (size_t) sizeof(long));

	        DMQA__push_long(&local_msg_ptr, (char *) &local_long);
            }
	}

	local_int = show_buffermax - (longword_count * sizeof(long));

	if (local_int)
            (void) memcpy(  (VOID *) local_msg_ptr,
			    (CONST VOID *) show_buffer_ptr,
			    (size_t) local_int);

	local_msg_ptr += local_int;
    }
    /* Copy showbuffer */

    DMQA__push_short(&local_msg_ptr, (char *) &len_data);   /* len_data */
    DMQA__push_short(&local_msg_ptr, (char *) &type);       /* type */
    DMQA__push_short(&local_msg_ptr, (char *) &class);      /* class */


    /* source is one of those arguments that is datatyped as a longword */
    /* but is really used by applications as a pair of short words.     */
    
    (void) memcpy(  (VOID *) &short_array[0],
                    (CONST VOID *) &source,
                    (size_t) sizeof(source));
    
    DMQA__push_short(&local_msg_ptr, (char *) &short_array[0]);
    DMQA__push_short(&local_msg_ptr, (char *) &short_array[1]);

    
    if (local_msg_ptr != msg_area_ptr)
    {
        if (DMQA__debug)
        {
            printf("%s message buffer missed correct position by %d bytes\n",
                DMQA__DBG_PREFIX, local_msg_ptr - msg_area_ptr);
            DMQA__DBG_EXIT(PAMS__INTERNAL);
        }
        *length = DMQA__HEADERSIZE;
        return (long) PAMS__INTERNAL;
	/* For some reason, our calculated offset was incorrect */
    }

#ifdef VMS
    if (eflag != 0)
    {
	if (DMQA__debug)
	    printf("%s Translating msg to EBCDIC...\n", DMQA__DBG_PREFIX);

	EBCDIC_struct.dsc$w_length	= (unsigned short) len_data;
	EBCDIC_struct.dsc$b_dtype	= DSC$K_DTYPE_T;
	EBCDIC_struct.dsc$b_class	= DSC$K_CLASS_S;
	EBCDIC_struct.dsc$a_pointer	= local_msg_ptr;

	if ((lib_status = lib$movtc(&EBCDIC_struct,
				    &EBCDIC_fill,
				    &EBCDIC_table,
				    &EBCDIC_struct)) != SS$_NORMAL)
	{
	    if (DMQA__debug)
	    {
		printf("%s EBCDIC translation failed with LIB$ error %u\n",
		    DMQA__DBG_PREFIX, lib_status);
		DMQA__DBG_EXIT(PAMS__INTERNAL);
	    }

	    *length = DMQA__HEADERSIZE;
	    DMQA__errno = (long) lib_status;
	    return (long) PAMS__INTERNAL;
	}
    }
#endif

    local_msg_ptr += len_data;
    /* move the pointer to reflect the message */

    if (DMQA__debug) DMQA__DBG_EXIT(local_status);

    *length = local_msg_ptr - msg_ptr;
    return local_status;
}


/****************************************************************************/
/*									    */
/*	               DMQA_pams_get_msg function			    */
/*									    */
/****************************************************************************/

long DMQA_pams_get_msg(msg_ptr, length, eflag, os, nullarg)

char	*msg_ptr;
short   *length;
short	eflag;
long    nullarg;
char	os;
{
    char            *local_msg_ptr;
    long            local_status;
    char            *nullarg1_ptr       = 0;
    long            nullarg2;
    long            *nullarg2_ptr       = 0;
    long            nullarg3;
    long            *nullarg3_ptr       = 0;
    char            priority;
    long            source              = 0;
    short           class               = 0;
    short           type                = 0;
    short           msg_area_len;
    short           len_data            = 0;
    struct psb      psb;
    long            sel_filter;
    long            *sel_filter_ptr     = 0;
    char            show_buffer[DMQA__SHOWBUFMAX];
    char	    *show_buffer_ptr    = 0;
    long            *show_bufflen_ptr   = 0;
    long            show_bufflen;
    long            show_buffermax	= DMQA__SHOWBUFMAX;
    char            *msg_area_ptr;
    int		    longword_count	= 0;
    long	    local_long;
    int		    local_int;
    short           short_array[2];


#ifdef VMS
    unsigned long	    lib_status;
    extern unsigned long    lib$movtc();
    struct dsc$descriptor_s EBCDIC_struct;
#pragma nostandard
    $DESCRIPTOR		    (EBCDIC_fill," ");
#pragma standard
    struct dsc$descriptor_s EBCDIC_table;
#endif
    

    if (DMQA__debug)
	printf("%s *** Entering DMQA_pams_get_msg routine ***\n",
            DMQA__DBG_PREFIX);
        
#ifdef VMS
    EBCDIC_table.dsc$w_length	= (unsigned short) sizeof(ascebc_table);
    EBCDIC_table.dsc$b_dtype	= DSC$K_DTYPE_T;
    EBCDIC_table.dsc$b_class	= DSC$K_CLASS_S;
    EBCDIC_table.dsc$a_pointer	= (char *) ascebc_table;

    if (os == DMQA__AS400_OS)
    {
	if (DMQA__debug)
	    printf("%s using AS/400 EBCDIC table\n",
		DMQA__DBG_PREFIX);
	EBCDIC_table.dsc$a_pointer = (char *) ascebc_as400;
    }
    /* AS/400 has some special oddities */

#endif
        

			/** Parse Arguments **/


    if (DMQA__debug)
	printf("%s Processing arguments...\n", DMQA__DBG_PREFIX);

    if (*length - DMQA__HEADERSIZE < (  sizeof(nullarg3)        +
                                        sizeof(nullarg2)        +
                                        sizeof(show_bufflen)    +
                                        sizeof(sel_filter)      +
                                        sizeof(msg_area_len)    +
                                        sizeof(priority)))
    {
    	if (DMQA__debug)
	{
	    printf("%s msg has incorrect length (%d)\n",
                DMQA__DBG_PREFIX, *length - DMQA__HEADERSIZE);
            DMQA__DBG_EXIT(PAMS__INTERNAL);
	}
        *length = DMQA__HEADERSIZE;
        return (long) PAMS__INTERNAL;
    }

    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;

    nullarg3 = (long) DMQA__pop_long(&local_msg_ptr);       /* nullarg  */
    if (nullarg3) nullarg3_ptr = &nullarg3;
    
    nullarg2 = (long) DMQA__pop_long(&local_msg_ptr);
    if (nullarg2)
    {
	nullarg2_ptr	= &nullarg2;		/* nullarg (user_taglen) */

	nullarg1_ptr	= local_msg_ptr;	/* nullarg (user_tag) */
	local_msg_ptr	+= nullarg2;
    }
    
    show_bufflen = (long) DMQA__pop_long(&local_msg_ptr);   /* show_bufflen */

    if (show_bufflen < show_buffermax)
        show_buffermax = show_bufflen;

    if (show_buffermax)
    {
	show_bufflen_ptr    = &show_buffermax;
	show_buffer_ptr	    = show_buffer;
    }
    /* establish what size we can handle as a showbuffer. Forcing  */
    /* the smallest common size possible will help ensure that the */
    /* the showbuffer status will have BUFFEROVF if needed.        */


    /* sel_filter is one of those arguments that is data typed as a     */
    /* longword, but is really used by applications as a pair of shorts */
    
    short_array[0] = (short) DMQA__pop_short(&local_msg_ptr);
    short_array[1] = (short) DMQA__pop_short(&local_msg_ptr);
    
    (void) memcpy(  (VOID *) &sel_filter,
                    (CONST VOID *) &short_array[0],
                    (size_t) sizeof(sel_filter));


    if (sel_filter) sel_filter_ptr = &sel_filter;

    msg_area_len = (short) DMQA__pop_short(&local_msg_ptr); /* msg_area_len */
    priority = *local_msg_ptr++;                            /* priority */
    
    if (msg_area_len > DMQA__MSGMAX)
    {
        if (DMQA__debug)
        {
            printf("%d msg_area_len (%d) exceeds maximum(%d)\n",
                DMQA__DBG_PREFIX, msg_area_len, DMQA__MSGMAX);
                DMQA__DBG_EXIT(PAMS__MSGTOBIG);
        }
        *length = DMQA__HEADERSIZE;
        return (long) PAMS__MSGTOBIG;
    }
    /* Client's message exceeds our capabilities (but passed his?) */
    
    if (DMQA__debug)
    {
        printf("%s calling pams_get_msg with the following parameters:\n",
            DMQA__DBG_PREFIX);
        printf("%s     msg_area_len: %d\n", DMQA__DBG_PREFIX, msg_area_len);
        printf("%s         priority: %d\n", DMQA__DBG_PREFIX, priority);
        printf("%s         len_data: %d\n", DMQA__DBG_PREFIX, len_data);

        printf("%s       sel_filter: ", DMQA__DBG_PREFIX);
        if (sel_filter_ptr)
            printf("%d %d\n", short_array[1], short_array[0]);
        else
            printf("null\n");

        printf("%s     show_bufflen: %d\n", DMQA__DBG_PREFIX, show_bufflen);
        printf("%s  show_buffer_max: %d\n", DMQA__DBG_PREFIX, show_buffermax);

        if (nullarg1_ptr)
            printf("%s         nullarg1: Yes\n", DMQA__DBG_PREFIX);
        if (nullarg2_ptr)
            printf("%s         nullarg2: Yes\n", DMQA__DBG_PREFIX);
        if (nullarg3_ptr)
            printf("%s         nullarg3: Yes\n", DMQA__DBG_PREFIX);
    }

    /* Perform the pams call. This routine gets a little tricky here.     */
    /* To save memory (rather than wasting 32K of buffering), we will use */
    /* our existing buffer to receive the message. To make room for our   */
    /* header record and argument list, we will offset the buffer ptr by  */
    /* the amount of space we need.                                       */

    msg_area_ptr =  msg_ptr                 +       /* start of area   */
                    (DMQA__HEADERSIZE       +       /* skip header     */
                    sizeof(show_bufflen)    +       /* skip len field  */
                    show_buffermax          +       /* skip showbuffer */
                    sizeof(struct psb)      +       /* skip psb        */
                    sizeof(short)           +       /* skip psblen     */
                    sizeof(len_data)        +       /* skip lendata    */
                    sizeof(type)            +       /* skip type       */
                    sizeof(class)           +       /* skip class      */
                    sizeof(source));                /* skip source     */
    /* calculate message offset within our buffer area */
                    
    local_status    = pams_get_msg((ANY *) msg_area_ptr,
                                    &priority,
                                    &source,
                                    &class,
                                    &type,
                                    &msg_area_len,
                                    &len_data,
                                    sel_filter_ptr,
                                    &psb,
                                    show_buffer_ptr,
                                    show_bufflen_ptr,
                                    (char *) nullarg1_ptr,
                                    (ANY *)  nullarg2_ptr,
                                    (ANY *)  nullarg3_ptr);
    if (DMQA__debug) printf("\n");
    /* Do the pams call */

    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;
    /* reset pointer */

    DMQA__push_psb(&local_msg_ptr, (char *) &psb, sizeof(struct psb));
    /* return psb */
    
    DMQA__push_long(&local_msg_ptr, (char *) &show_buffermax);
    /* return size of show_buffer filled in */

    if (show_buffermax)
    {
	longword_count = show_buffermax / sizeof(long);

	for (	local_int = 0, show_buffer_ptr = show_buffer;
		local_int < longword_count;
		local_int++, show_buffer_ptr += sizeof(long))
	{
            if ((local_int >= 10) && (local_int <=13))
            {
                /* Some elements are really used as a pair of shortwords */
                
	        (void) memcpy(  (VOID *) &short_array[0],
			        (CONST VOID *) show_buffer_ptr,
			        (size_t) sizeof(long));

                DMQA__push_short(&local_msg_ptr, (char *) &short_array[0]);
                DMQA__push_short(&local_msg_ptr, (char *) &short_array[1]);
            }
            else
            {    
	        (void) memcpy(  (VOID *) &local_long,
			        (CONST VOID *) show_buffer_ptr,
			        (size_t) sizeof(long));

	        DMQA__push_long(&local_msg_ptr, (char *) &local_long);
            }
	}

	local_int = show_buffermax - (longword_count * sizeof(long));

	if (local_int)
            (void) memcpy(  (VOID *) local_msg_ptr,
			    (CONST VOID *) show_buffer_ptr,
			    (size_t) local_int);

	local_msg_ptr += local_int;
    }
    /* Copy showbuffer */

    DMQA__push_short(&local_msg_ptr, (char *) &len_data);   /* len_data */
    DMQA__push_short(&local_msg_ptr, (char *) &type);       /* type */
    DMQA__push_short(&local_msg_ptr, (char *) &class);      /* class */


    /* source is one of those arguments that is datatyped as a longword */
    /* but is really used by applications as a pair of short words.     */
    
    (void) memcpy(  (VOID *) &short_array[0],
                    (CONST VOID *) &source,
                    (size_t) sizeof(source));
    
    DMQA__push_short(&local_msg_ptr, (char *) &short_array[0]);
    DMQA__push_short(&local_msg_ptr, (char *) &short_array[1]);

    
    if (local_msg_ptr != msg_area_ptr)
    {
        if (DMQA__debug)
        {
            printf("%s message buffer missed correct position by %d bytes\n",
                DMQA__DBG_PREFIX, local_msg_ptr - msg_area_ptr);
            DMQA__DBG_EXIT(PAMS__INTERNAL);
        }
        *length = DMQA__HEADERSIZE;
        return (long) PAMS__INTERNAL;
	/* For some reason, our calculated offset was incorrect */
    }

#ifdef VMS
    if (eflag != 0)
    {
	if (DMQA__debug)
	    printf("%s Translating msg to EBCDIC...\n", DMQA__DBG_PREFIX);

	EBCDIC_struct.dsc$w_length	= (unsigned short) len_data;
	EBCDIC_struct.dsc$b_dtype	= DSC$K_DTYPE_T;
	EBCDIC_struct.dsc$b_class	= DSC$K_CLASS_S;
	EBCDIC_struct.dsc$a_pointer	= local_msg_ptr;

	if ((lib_status = lib$movtc(&EBCDIC_struct,
				    &EBCDIC_fill,
				    &EBCDIC_table,
				    &EBCDIC_struct)) != SS$_NORMAL)
	{
	    if (DMQA__debug)
	    {
		printf("%s EBCDIC translation failed with LIB$ error %u\n",
		    DMQA__DBG_PREFIX, lib_status);
		DMQA__DBG_EXIT(PAMS__INTERNAL);
	    }

	    *length = DMQA__HEADERSIZE;
	    DMQA__errno = (long) lib_status;
	    return (long) PAMS__INTERNAL;
	}
    }
#endif

    local_msg_ptr += len_data;
    /* move the pointer to reflect the message */

    if (DMQA__debug) DMQA__DBG_EXIT(local_status);

    *length = local_msg_ptr - msg_ptr;
    return local_status;
}


/****************************************************************************/
/*									    */
/*			    DMQA_locate_q function			    */
/*									    */
/****************************************************************************/

long DMQA_pams_locate_q(msg_ptr, length, eflag, os, nullarg)

char	*msg_ptr;
short   *length;
short	eflag;
char	os;
long    nullarg;
{
    char            *local_msg_ptr;
    long            q_name_len;
    long            q_address;
    long            local_status;
    long            wait_mode;
    long            *wait_mode_ptr		= 0;
    long            req_id;
    long            *req_id_ptr			= 0;
    long            resp_q;
    long            *resp_q_ptr			= 0;
    long            nullarg1;
    long            *nullarg1_ptr		= 0;
    char            q_name[32];
    long            name_space_list_len;
    long            *name_space_list_len_ptr	= 0;
    long            name_space_list[DMQA__NAMELISTMAX];
    long            *name_space_list_ptr	= 0;
    int	            local_int;
    short           short_array[2];


#ifdef VMS
    unsigned long	    lib_status;
    extern unsigned long    lib$movtc();
    struct dsc$descriptor_s EBCDIC_struct;
#pragma nostandard
    $DESCRIPTOR		    (EBCDIC_fill," ");
#pragma standard
    struct dsc$descriptor_s EBCDIC_table;
#endif


    if (DMQA__debug)
	printf("%s *** Entering DMQA_pams_locate_q routine ***\n",
            DMQA__DBG_PREFIX);

#ifdef VMS
    EBCDIC_table.dsc$w_length	= (unsigned short) sizeof(ebcasc_table);
    EBCDIC_table.dsc$b_dtype	= DSC$K_DTYPE_T;
    EBCDIC_table.dsc$b_class	= DSC$K_CLASS_S;
    EBCDIC_table.dsc$a_pointer	= (char *) ebcasc_table;

    if (os == DMQA__AS400_OS)
    {
	if (DMQA__debug)
	    printf("%s using AS/400 EBCDIC table\n",
		DMQA__DBG_PREFIX);
	EBCDIC_table.dsc$a_pointer = (char *) ebcasc_as400;
    }
    /* AS/400 has some special oddities */
#endif
        

			/** Parse Arguments **/

        
    if (DMQA__debug)
	printf("%s Processing arguments...\n", DMQA__DBG_PREFIX);

    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;

    for (   local_int = 0;
            local_int < DMQA__NAMELISTMAX;
            ++local_int)
        name_space_list[local_int] = 0;
    /* init our name_space_list array */

    nullarg1 = (long) DMQA__pop_long(&local_msg_ptr);       /* nullarg  */
    if (nullarg1) nullarg1_ptr = &nullarg1;
    
    name_space_list_len = (long) DMQA__pop_long(&local_msg_ptr);
    if (name_space_list_len)
    {
        if (name_space_list_len > DMQA__NAMELISTMAX)
        {
            if (DMQA__debug)
            {
                printf("%s name_space_list_len exceeds maximum (%d)\n",
                    DMQA__DBG_PREFIX, DMQA__NAMELISTMAX);
                DMQA__DBG_EXIT(PAMS__INTERNAL);
            }
            *length = DMQA__HEADERSIZE;
            return (long) PAMS__INTERNAL;
        }

        name_space_list_len_ptr = &name_space_list_len;
        local_int               = 0;
        
        while (local_int < name_space_list_len)
            name_space_list[local_int++] =
                (long) DMQA__pop_long(&local_msg_ptr);
        /* copy name space */
    }


    /* resp_q is one  of those  arguments that is  data typed  as a     */
    /* longword, but is really used by applications as a pair of shorts */
    
    short_array[0] = (short) DMQA__pop_short(&local_msg_ptr);
    short_array[1] = (short) DMQA__pop_short(&local_msg_ptr);
    
    (void) memcpy(  (VOID *) &resp_q,
                    (CONST VOID *) &short_array[0],
                    (size_t) sizeof(resp_q));

    if (resp_q) resp_q_ptr = &resp_q;

    
    req_id = (long) DMQA__pop_long(&local_msg_ptr);         /* req_id */
    if (req_id) req_id_ptr = &req_id;
    
    wait_mode = (long) DMQA__pop_long(&local_msg_ptr);      /* wait_mode */
    if (wait_mode) wait_mode_ptr = &wait_mode;
    
    q_name_len  = (long) DMQA__pop_long(&local_msg_ptr);    /* q_name_len */

    if (q_name_len)
    {
        (void) memcpy(  (VOID *) q_name,                    /* q_name */
                        (CONST VOID *) local_msg_ptr,
                        (size_t) q_name_len);
        local_msg_ptr   += q_name_len;

#ifdef VMS
	if (eflag != 0)
	{
	    if (DMQA__debug)
		printf("%s Translating q_name from EBCDIC...\n",
		    DMQA__DBG_PREFIX);

	    EBCDIC_struct.dsc$w_length	= (unsigned short) q_name_len;
	    EBCDIC_struct.dsc$b_dtype	= DSC$K_DTYPE_T;
	    EBCDIC_struct.dsc$b_class	= DSC$K_CLASS_S;
	    EBCDIC_struct.dsc$a_pointer	= q_name;

	    if ((lib_status = lib$movtc(&EBCDIC_struct,
					&EBCDIC_fill,
					&EBCDIC_table,
					&EBCDIC_struct)) != SS$_NORMAL)
	    {
		if (DMQA__debug)
		{
		    printf("%s EBCDIC translation failed with LIB$ error %u\n",
			DMQA__DBG_PREFIX, lib_status);
		    DMQA__DBG_EXIT(PAMS__INTERNAL);
		}

		*length = DMQA__HEADERSIZE;
		DMQA__errno = (long) lib_status;
		return (long) PAMS__INTERNAL;
	    }
	}
#endif

     }
     
    q_name[q_name_len] = *local_msg_ptr++;
    q_name[31] = 0;
    /* ensure terminating null */
    
    if (*length - DMQA__HEADERSIZE < (  sizeof(nullarg1)            +
                                        sizeof(name_space_list_len) +
					sizeof(resp_q)		    +
					sizeof(req_id)		    +
					sizeof(wait_mode)	    +
					q_name_len		    +
                                        sizeof(q_name_len)))
    {
    	if (DMQA__debug)
	{
	    printf("%s msg has incorrect length (%d)\n",
                DMQA__DBG_PREFIX, *length - DMQA__HEADERSIZE);
            DMQA__DBG_EXIT(PAMS__INTERNAL);
	}
        *length = DMQA__HEADERSIZE;
        return (long) PAMS__INTERNAL;
    }

    if (DMQA__debug)
    {
        printf("%s calling pams_locate_q with the following parameters:\n",
            DMQA__DBG_PREFIX);

        printf("%s           q_name: %s\n", DMQA__DBG_PREFIX, q_name);
        printf("%s       q_name_len: %d\n", DMQA__DBG_PREFIX, q_name_len);

        printf("%s        wait_mode: ", DMQA__DBG_PREFIX);
        if (wait_mode_ptr)
            printf("%d\n",*wait_mode_ptr);
        else
            printf("null\n");

        printf("%s          req_id: ", DMQA__DBG_PREFIX);
        if (req_id_ptr)
            printf("%d\n",*req_id_ptr);
        else
            printf("null\n");

        printf("%s          resp_q: ", DMQA__DBG_PREFIX);
        if (resp_q_ptr)
            printf("%d %d\n", short_array[1], short_array[0]);
        else
            printf("null\n");

        printf("%s  name_space_len: ", DMQA__DBG_PREFIX);
        if (name_space_list_len_ptr)
        {
            printf("%d\n", *name_space_list_len_ptr);

            for (   local_int = 0;
                    local_int < *name_space_list_len_ptr;
                    ++local_int)
		printf("%s    name_space %d: %d\n",
                    DMQA__DBG_PREFIX, local_int, name_space_list[local_int]);
        }
        else
            printf("null\n");
            
        if (nullarg1_ptr)
            printf("%s            nullarg: Yes\n", DMQA__DBG_PREFIX);
    }

    local_status    = pams_locate_q(    q_name,
                                        &q_name_len,
					&q_address,
					wait_mode_ptr,
					req_id_ptr,
					resp_q_ptr,
                                        name_space_list_ptr,
                                        name_space_list_len_ptr,
                                        (ANY *) nullarg1_ptr);
    if (DMQA__debug) printf("\n");                
    /* Perform the pams call */

    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;
    /* reset pointer */


    /* q_address is one  of those variables that is datatyped as a longword */
    /* but is really used by applications as a pair of short words.         */

    (void) memcpy(  (VOID *) &short_array[0],
                    (CONST VOID *) &q_address,
                    (size_t) sizeof(q_address));

    DMQA__push_short(&local_msg_ptr, (char *) &short_array[0]);
    DMQA__push_short(&local_msg_ptr, (char *) &short_array[1]);


    if (DMQA__debug) DMQA__DBG_EXIT(local_status);

    *length = local_msg_ptr - msg_ptr;
    return local_status;
}


/****************************************************************************/
/*									    */
/*			 DMQA_pams_confirm_msg function			    */
/*									    */
/****************************************************************************/

long DMQA_pams_confirm_msg(msg_ptr, length, nullarg)

char	*msg_ptr;
short   *length;
long    nullarg;
{
    char    *local_msg_ptr;
    long    local_status;
    long    msg_seq_no[2];
    long    status;
    char    force_j;


    if (DMQA__debug)
    {
	printf("%s *** Entering DMQA_confirm_msg routine ***\n",
            DMQA__DBG_PREFIX);

	printf("%s Processing arguments...\n", DMQA__DBG_PREFIX);
    }


			/** Parse Arguments **/

        
    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;

    force_j	    = *local_msg_ptr++;				/* force_j */
    status	    = (long) DMQA__pop_long(&local_msg_ptr);	/* status  */
    msg_seq_no[0]   = (long) DMQA__pop_long(&local_msg_ptr);	/* msg_seq_no */
    msg_seq_no[1]   = (long) DMQA__pop_long(&local_msg_ptr);	/* msg_seq_no */
    
    if (DMQA__debug)
    {
        printf("%s calling pams_locate_q with the following parameters:\n",
            DMQA__DBG_PREFIX);

        printf("%s          force_j: %d\n", DMQA__DBG_PREFIX, force_j);
        printf("%s           status: %d\n", DMQA__DBG_PREFIX, status);
        printf("%s    msg_seq_no[0]: %d\n", DMQA__DBG_PREFIX,
	    msg_seq_no[0]);
        printf("%s    msg_seq_no[1]: %d\n", DMQA__DBG_PREFIX,
	    msg_seq_no[1]);
    }

    local_status    = pams_confirm_msg(	(QUADWORD *) msg_seq_no,
                                        &status,
					&force_j);
    /* Perform the pams call */

    if (DMQA__debug)
    {
	printf("\n");                
	DMQA__DBG_EXIT(local_status);
    }

    *length = DMQA__HEADERSIZE;
    return local_status;
}


/****************************************************************************/
/*									    */
/*			 DMQA_pams_set_select function			    */
/*									    */
/****************************************************************************/

long DMQA_pams_set_select(msg_ptr, length, nullarg)

char	*msg_ptr;
short   *length;
long    nullarg;
{
    char    *local_msg_ptr;
    long    local_status;
    long    sel_array[13 * DMQA__SELARRAYMAX];
    short   num_masks;
    long    index_handle;
    long    sel_index;
    long    sel_max;


    if (DMQA__debug)
    {
	printf("%s *** Entering DMQA_set_select routine ***\n",
            DMQA__DBG_PREFIX);

	printf("%s Processing arguments...\n", DMQA__DBG_PREFIX);
    }


			/** Parse Arguments **/

        
    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;

    num_masks	    = (short) DMQA__pop_short(&local_msg_ptr);	/* num_masks */

    if ((num_masks < 1) || (num_masks > DMQA__SELARRAYMAX))
    {
	if (DMQA__debug)
	{
	    printf("%s num_masks is out of range (1-%d)\n",
		DMQA__DBG_PREFIX, DMQA__SELARRAYMAX);
	    DMQA__DBG_EXIT(PAMS__BADPARAM);
	}
	*length = DMQA__HEADERSIZE;
	return (long) PAMS__BADPARAM;
    }

    for (   sel_max = num_masks * 13, sel_index	= 0;
	    sel_index < sel_max;
	    sel_index++)
	sel_array[sel_index] = (long) DMQA__pop_long(&local_msg_ptr);
    /* unload selection array */

    if (DMQA__debug)
        printf(
	"%s calling pams_set_select with a selection_array of %d elements\n",
            DMQA__DBG_PREFIX, num_masks);

    local_status    = pams_set_select(	(ANY *) sel_array,
					&num_masks,
					&index_handle);
    if (DMQA__debug) printf("\n");                
    /* Perform the pams call */

    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;

    DMQA__push_long (&local_msg_ptr, (char *) &index_handle);

    if (DMQA__debug) DMQA__DBG_EXIT(local_status);

    *length = local_msg_ptr - msg_ptr;
    return local_status;
}



/****************************************************************************/
/*									    */
/*			 DMQA_pams_cancel_select function		    */
/*									    */
/****************************************************************************/

long DMQA_pams_cancel_select(msg_ptr, length, nullarg)

char	*msg_ptr;
short   *length;
long    nullarg;
{
    char    *local_msg_ptr;
    long    local_status;
    long    index_handle;


    if (DMQA__debug)
    {
	printf("%s *** Entering DMQA_cancel_select routine ***\n",
            DMQA__DBG_PREFIX);

	printf("%s Processing arguments...\n", DMQA__DBG_PREFIX);
    }


			/** Parse Arguments **/

        
    local_msg_ptr   = msg_ptr + DMQA__HEADERSIZE;

    index_handle    = (long) DMQA__pop_long(&local_msg_ptr);

    if (DMQA__debug)
        printf("%s calling pams_cancel_select\n", DMQA__DBG_PREFIX);

    local_status    = pams_cancel_select(&index_handle);
    /* Perform the pams call */

    if (DMQA__debug)
    {
	printf("\n");                
	DMQA__DBG_EXIT(local_status);
    }

    *length = DMQA__HEADERSIZE;
    return local_status;
}



/****************************************************************************/
/*									    */
/*			 DMQA_pams_exit function			    */
/*									    */
/****************************************************************************/

long DMQA_pams_exit(msg_ptr, length, nullarg)

char	*msg_ptr;
short   *length;
long    nullarg;
{
    long    local_status;


    if (DMQA__debug)
	printf("%s *** Entering DMQA_pams_exit routine ***\n",
            DMQA__DBG_PREFIX);

    local_status    = pams_exit();

    if (DMQA__debug)
    {
	printf("\n");                
	DMQA__DBG_EXIT(local_status);
    }

    *length = DMQA__HEADERSIZE;
    return local_status;
    /* Remote system will disconnect the link */
}

