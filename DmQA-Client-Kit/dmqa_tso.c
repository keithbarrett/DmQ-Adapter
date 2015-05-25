/*  Module dmqa_tso.c	DmQA V1.0           DmQA TCP/IP Socket functions    */
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
/*                                                                          */
/*  DECmessageQ Expertise Center                                            */
/*                                                                          */
/*                                                                          */
/*  These routines contain the system specific logic that will create and   */
/*  use a datalink between 2 systems. These routines will return one of the */
/*  following errors:							    */
/*									    */
/*	PAMS__SUCCESS	    Successful completion			    */
/*	PAMS__BADLOGIC	    An internal error took place within this module */
/*	PAMS__INTERNAL	    An internal error was returned by the remote    */
/*			    server					    */
/*	PAMS__NOLINK	    A link cannot be established to the remote	    */
/*			    system due some condition there.		    */
/*	PAMS__LINK_DOWN	    A link cannot be established to the remote	    */
/*			    system (or the existing one has come down) due  */
/*			    to a system situation.			    */
/*	PAMS__COMMERR	    A communications or internal protocol error has */
/*			    occurred. The data link may or may not be still */
/*			    available, depending on condition.		    */
/*									    */
/*  Any situation that returned error may also provid the resulting	    */
/*  communications or internal status in the DMQA__errno global variable.   */
/*									    */
/*									    */
/*  IDENT HISTORY:                                                          */
/*                                                                          */
/*  Version     Date    Who     Description                                 */
/*                                                                          */
/*  V1.0FT  08-Mar-1991 kgb     Initial release; Keith Barrett              */
/*  V1.0FT  23-Apr-1991 kgb	changed arg processing for server           */
/*  V1.0FT  25-Apr-1991 kgb	Added offspring logic.			    */
/*  V1.0FT  29-Apr-1991 kgb	Moved argument processing logic		    */
/*  V1.0FT  30-Apr-1991 kgb	Added $GET_SYMBOL call and removed $SPAWN   */
/*  V1.0FT  09-May-1991	kgb	Handle packets below headersize		    */
/*  V1.0FT  15-May-1991	kgb	Minor improvement to argc handling	    */
/*  V1.0FT  22-May-1991	kgb	Moved local_buffer because of Ultrix	    */
/*  V1.0FT  23-May-1991	kgb	Added DMQA__link_check function		    */
/*  V1.0FT  24-May-1991	kgb	Lockout offspring slot if job create failed */
/*  V1.0FT  04-Jun-1991 kgb	Corrected file_struct in offspring creation */
/*  V1.0FT  11-Jul-1991 kgb	Minor changes for HP system                 */
/*  V1.0    19-Jul-1991 kgb     Added load_params routine                   */
/*                                                                          */



        /* Include standard "C" and system definition files. */

#include <errno.h>		/* Error number values */
#include <stdio.h>		/* Standard I/O definitions */
#include <stddef.h>
#include <string.h>		/* String manipulations and functions */
#include <stdlib.h>

#ifdef VMS
#include <types.h>
#pragma nostandard
#include <socket.h>
#include <in.h>
#include <netdb.h>		/* change hostent to comply with BSD 4.3 */
#include <inet.h>
#include <ucx$inetdef.h>	/* INET Symbol definitions */
#include <unixio.h>
#include <time.h>

#include pams_c_return_status_def
#pragma standard                /* We return PAMS status values */

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
/* include <arpa/inet.h> */

#include "p_return.h"           /* We return PAMS status values */

#endif


#define CONST
#define DMQA__INTERNAL  TRUE
#define DMQA__TCPIP     TRUE
#include "p_dmqa.h"

#ifdef VMS
#if DMQA__OFFSPRING
#include <descrip.h>
#include <lib$routines.h>
#include <libdef.h>
#include <prcdef.h>
#include <ssdef.h>
#endif
#endif

#ifndef DMQA__SENDMAX
#define DMQA__SENDMAX DMQA__SIZMAX
#endif

#ifndef ECONNRESET
#define ECONNRESET 0
#endif

#ifndef EADDRINUSE
#define EADDRINUSE 0
#endif

#ifndef ECONNABORTED
#define ECONNABORTED 0
#endif

#ifndef ETIMEDOUT
#define ETIMEDOUT 0
#endif

#ifndef MSG_PEEK
#define MSG_PEEK 2
#endif

static int                  DMQA__link_active	    = FALSE;
static int		    socket_created	    = FALSE;
static int		    sockets[DMQA__TCPIP_BACKLOG+1];
static int		    num_sockets		    = 0;
static short		    send_packet_size	    = 0;
static int		    packet_reduced	    = FALSE;

char		            local_buffer[DMQA__HEADERSIZE];
char                        hostparam[80];
    
static struct sockaddr_in   socket_struct;          /* Struct for socket    */
static struct hostent       hostentstruct;          /* hostent data.        */
static struct hostent       *hostentptr;            /* Ptr to hostent data. */

unsigned long	    	    DMQA__rcount  	    = 0;
unsigned long	    	    DMQA__scount  	    = 0;

int		            DMQA__argc              = 0;
char	    	            **DMQA__argv            = 0;
int                         DMQA__offspring_flag    = 0;
char	    	    	    *DMQA__host		    = 0;
char	    	    	    *DMQA__secondary_host   = 0;
unsigned short      	    DMQA__port		    = 0;
unsigned short      	    DMQA__secondary_port    = 0;
unsigned short      	    DMQA__highest_port	    = 0;
/* TCP/IP server paramters */

extern long	    	    DMQA__errno;
extern int		    DMQA__debug;
extern int		    DMQA__program_id;
/* General parameters */
        
#ifdef VMS
#if DMQA__OFFSPRING
static char                 *DMQA__efc_name         = 0;    /* EF Cluster    */
static long                 DMQA__efn               = 0;    /* Our EFN in it */
static int		    DMQA_efn_set	    = FALSE;
static unsigned long        DMQA__efn_map;                  /* EFN bitmap    */
static char		    exe_symbol[128];
struct dsc$descriptor_s	    exe_struct;
static char		    dir_symbol[80];
struct dsc$descriptor_s	    dir_struct;
static char		    bus_symbol[128];
struct dsc$descriptor_s	    bus_struct;
static char		    dbg_symbol[80];
struct dsc$descriptor_s	    dbg_struct;
static char		    log_symbol[128];
struct dsc$descriptor_s	    log_struct;
static char		    efc_name_buf[128];

#define DMQA__TIMER_EFN         (DMQA__OFFSPRING + 64)
#define DMQA__TIMER_INTERVAL    "0 :01:00"
#define DMQA__EXE_SYMBOL	"DMQA_EXE"
#define DMQA__DIR_SYMBOL	"DMQA_DIR"
#define DMQA__BUS_SYMBOL	"DMQA_BUSGROUP"
#define DMQA__DBG_SYMBOL	"DMQA_DEBUG"
#define	DMQA__LOG_SYMBOL	"DMQA_LOG"
#define DMQA__TIMESTAMP		"$ write sys$output \"DmQA Server Offspring started at ''f$time()'\"\n"

#endif
#endif



/****************************************************************************/
/* Module:    DMQA__load_params                                             */
/*                                                                          */
/* Function: This routine loads the global variables from a configuration   */
/*           file (defined in p_dmqa.h). This will only happen if the       */
/*           variables were not previously set.                             */
/*                                                                          */
/****************************************************************************/

void DMQA__load_params()
{
    extern   FILE *fopen();

    FILE     *fptr;
    char     cnfg_buffer[80];
    char     *buf_ptr = 0;
    int      loop     = TRUE;
    int      found    = FALSE;
    char     *tmp_ptr = 0;
    int      length;

#ifdef __USE_ARGS
    extern  int	   atoi(char *);
    extern  int	   strcmp(CONST char *, CONST char *);
/*  extern  FILE   *fopen(CONST char *, CONST char *);  */
    extern  FILE   *fopen();
    extern  char   *fgets(char *, int, FILE *);
    extern  char   *strstr(CONST char *, CONST char *);
    extern  size_t strlen(CONST char *);
    extern  int    fclose(FILE *);
#else
    extern  int	   atoi();
    extern  int	   strcmp();
    extern  FILE   *fopen();
    extern  char   *fgets();
    extern  char   *strstr();
    extern  size_t strlen();
    extern  int    fclose();
#endif


    if (DMQA__debug)
	printf("%s* Entering DMQA__load_params routine *\n", DMQA__DBG_PREFIX);

    if (fptr = fopen(DMQA__CONFIG_FILE, "r"))
    {
        if (DMQA__debug)
            printf("%s Config file is %s\n",
                DMQA__DBG_PREFIX, DMQA__CONFIG_FILE);

        while (loop)
        {
            if ((buf_ptr = fgets(    cnfg_buffer,
                                     (int) sizeof(cnfg_buffer) - 1,
                                     fptr)) == NULL)
            {
                loop = FALSE;
            }
            else
            {
                length = (int) strlen((CONST char *) buf_ptr);
                cnfg_buffer[length - 1] = 0;
                /* wipe out newline */

                if ((*buf_ptr != '#') && (length > 1))
                {
                    if (DMQA__debug)
                        printf("%s Command line read: %s\n",
                            DMQA__DBG_PREFIX, buf_ptr);

                    if ((tmp_ptr = strstr((CONST char *) buf_ptr,
                                          (CONST char *) "=")) == 0)
                    {
                        if (DMQA__debug)
                            printf("%s missing equal sign; skipping line\n",
                                DMQA__DBG_PREFIX);
                    }
                    else
                    {
                        *tmp_ptr++ = 0;
                        found = FALSE;

                        if ((strcmp((CONST char *) "DEBUG",
                                    (CONST char *) buf_ptr) == 0) ||
                            (strcmp((CONST char *) "debug",
                                    (CONST char *) buf_ptr) == 0))
                        {
                            found = TRUE;
                            if (DMQA__debug == 0)
                            {
                                DMQA__debug = atoi(tmp_ptr);
                                if (DMQA__debug)
                                    printf("%s DMQA__debug set to %d\n",
                                        DMQA__DBG_PREFIX, DMQA__debug);
                            }
                        }

                        if ((strcmp((CONST char *) "HOST",
                                    (CONST char *) buf_ptr) == 0) ||
                            (strcmp((CONST char *) "host",
                                    (CONST char *) buf_ptr) == 0))
                        {
                            found = TRUE;
                            if (DMQA__host == 0)
                            {
                                (void) memcpy((VOID *) hostparam,
                                              (CONST VOID *) tmp_ptr,
                                              (size_t) sizeof(hostparam) - 1);

                                DMQA__host = hostparam;
                                if (DMQA__debug)
                                    printf("%s DMQA__host set to %s\n",
                                        DMQA__DBG_PREFIX, DMQA__host);
                            }
                        }

                        if ((strcmp((CONST char *) "PORT",
                                    (CONST char *) buf_ptr) == 0) ||
                            (strcmp((CONST char *) "port",
                                    (CONST char *) buf_ptr) == 0))
                        {
                            found = TRUE;
                            if (DMQA__port == 0)
                            {
                                DMQA__port = atoi(tmp_ptr);
                                if (DMQA__debug)
                                    printf("%s DMQA__port set to %u\n",
                                        DMQA__DBG_PREFIX, DMQA__port);
                            }
                        }

                        if (found == FALSE)
                        {
                            if (DMQA__debug)
                                printf("%s keyword %s not found; skipping\n",
                                    DMQA__DBG_PREFIX, buf_ptr);
                        }
                    }
                }
            }
        }

        (void) fclose(fptr);
    }
    else
    {
        if (DMQA__debug)
            printf("%s Config file %s not found; skipping\n",
                DMQA__DBG_PREFIX, DMQA__CONFIG_FILE);
    }
}



/****************************************************************************/
/* Module:    DMQA__process_args                                            */
/*                                                                          */
/* Function: This routine is the logic for parsing the program command 	    */
/* 	     line, which may contain values for our required global	    */
/*	     variables (especially true if the main program is the server   */
/*	     program rather than a user client application). We need these  */
/*	     variables before we can start processing communications. This  */
/*	     logic is located within this file because different com-	    */
/*	     munications platforms may require different variables  .       */
/*                                                                          */
/* Note:     Program startup procedures have a limitation that no more than */
/*	     3 global variables (plus DMQA__debug) can be set by command    */
/*	     line parameters in the server process.			    */
/*                                                                          */
/* Passed:								    */
/*	    **DMQA__argv and	copy of **argv and *argc. If set, and the   */
/*	    DMQA__argc		first parameter is "DMQA", then the next    */
/*				parameters will be used to fill in all of   */
/*				the above variables (in order) - over-      */
/*                              riding any previous values.                 */
/*                                                                          */
/****************************************************************************/

void DMQA__process_args()
{
    int		local_index;
    char	**local_ptr;
    int		case_index;
    int		use_args = FALSE;
#ifdef __USE_ARGS
    extern  int	atoi(char *);
    extern  int	strcmp(CONST char *, CONST char *);
#else
    extern  int	atoi();
    extern  int	strcmp();
#endif


    if (DMQA__debug)
	printf("%s * Entering DMQA__process_args routine *\n",
	    DMQA__DBG_PREFIX);

    if ((DMQA__argc > 2) && (DMQA__argv != 0))
    {
	if (DMQA__debug)
	    printf("%s arg variables appear correct -- %d specified\n",
		DMQA__DBG_PREFIX, DMQA__argc);

	for (	local_index = 1, local_ptr = DMQA__argv, case_index = 1;
		local_index < DMQA__argc;
		++local_index, case_index++)
	{
	    local_ptr++;
	    if (DMQA__debug) printf("%s Parsing argument %d\n",
		DMQA__DBG_PREFIX, local_index);

	    if (*local_ptr)
	    {
		if ((case_index == 2) &&
		    (DMQA__program_id == DMQA__SERVER_ID))
			case_index++;
		/* Server has no HOST specifier */

		if ((case_index == 5) &&
		    (DMQA__program_id == DMQA__SERVER_ID))
			case_index += 2;
		/* Server has no secondary connections either */

		switch (case_index)
		{
		    case 1:     /* prefix */
			if ((strcmp((CONST char *) *local_ptr,
				(CONST char *) "DMQA") == 0) ||
			    (strcmp((CONST char *) *local_ptr,
				(CONST char *) "dmqa") == 0))
			{
			    use_args = TRUE;
			    if (DMQA__debug)
				printf("%s ARGs will be processed!\n",
				    DMQA__DBG_PREFIX);
			}

			if ((strcmp((CONST char *) *local_ptr,
				(CONST char *) "DMQA_O") == 0) ||
			    (strcmp((CONST char *) *local_ptr,
				(CONST char *) "dmqa_o") == 0))
			{
			    use_args = TRUE;
			    if (DMQA__debug)
				printf("%s setting use_args AND OFFSPRING!\n",
				    DMQA__DBG_PREFIX);
			    DMQA__offspring_flag = TRUE;
			}

			break;

		    case 2:     /* host */
			if (use_args)
			{
			    if (**local_ptr != 0)
			    {
				if (DMQA__debug) printf(
				    "%s setting DMQA__host to arg %d (%s)\n",
					DMQA__DBG_PREFIX, local_index,
					*local_ptr);
				DMQA__host = *local_ptr;
			    }
			}
			break;

		    case 3:     /* port */
			if (use_args)
			{
			    DMQA__port = (unsigned short) atoi(*local_ptr);
			    if (DMQA__debug)
				printf("%s setting DMQA__port to arg %d (%u)\n",
				    DMQA__DBG_PREFIX, local_index, DMQA__port);
			}
			break;

		    case 4:     /* debug */
			if (use_args)
			{
			    DMQA__debug = (int) atoi(*local_ptr);
			    if (DMQA__debug)
				printf(
                                "%s setting DMQA__debug to arg %d (%d)\n",
				    DMQA__DBG_PREFIX, local_index, DMQA__debug);
			}
			break;

		    case 5:     /* secondary host */
			if (use_args)
			{
			    if (**local_ptr != 0)
			    {
				if (DMQA__debug) printf(
				    "%s setting DMQA__secondary_host to arg %d (%s)\n",
					DMQA__DBG_PREFIX, local_index,
					*local_ptr);
				DMQA__secondary_host = *local_ptr;
			    }
			}
			break;

		    case 6:     /* secondary port */
			if (use_args)
			{
			    DMQA__secondary_port =
				(unsigned short) atoi(*local_ptr);
			    if (DMQA__debug)
				printf(
				"%s setting DMQA__secondary_port to arg %d (%d)\n",
				    DMQA__DBG_PREFIX, local_index,
					DMQA__secondary_port);
			}
			break;

#ifdef VMS
#if DMQA__OFFSPRING
		    case 7:     /* Common EFC name (for offspring) */
			if (use_args != 0)
			{
			    if (**local_ptr != 0)
			    {
                            	DMQA__efc_name = *local_ptr;
			    	if (DMQA__debug)
				    printf(
				    "%s setting EFC name to arg %d (%s)\n",
				        DMQA__DBG_PREFIX, local_index,
					    DMQA__efc_name);
			    }
			}
			break;


		    case 8:     /* Our EFN (for offspring) */
			if ((use_args != 0) && (DMQA__offspring_flag != 0))
			{
			    DMQA__efn = (long) atoi(*local_ptr);
			    if (DMQA__debug)
				printf(
 				"%s setting DMQA__efn to arg %d (%d)\n",
				DMQA__DBG_PREFIX, local_index, DMQA__efn);
			}
			break;
#endif
#endif

		    default:
			break;
		}
	    }
	}
    }
    DMQA__argc = 0;
    DMQA__argv = 0;
    /* Use DMQA__argc and DMQA__argv to over-ride the host and  */
    /* port specifications if every thing appears to be perfect */
}


/****************************************************************************/
/* Module:    DMQA__link_shutdown                                           */
/*                                                                          */
/* Function: This routine disconnects and shuts down the TCP/IP link to the */
/*	     remote system.                                                 */
/*                                                                          */
/****************************************************************************/

void DMQA__link_shutdown(flag, nullarg)

int	flag;			/* if set, closes ALL sockets */
long	nullarg;		/* future use */
{
#ifdef __USE_ARGS
    extern int	shutdown(int, int);
    extern int  close(int);
#else
    extern int	shutdown();
    extern int  close();
#endif

    int		local_socket;

#ifdef VMS
#if DMQA__OFFSPRING
    long        local_status;
    extern long sys$clef();
    extern long sys$dacefc();
#endif
#endif


    if (DMQA__debug)
	printf("%s * Entering DMQA__link_shutdown routine *\n",
            DMQA__DBG_PREFIX);

    while (num_sockets)
    {
	if (DMQA__debug)
	    printf("%s closing socket #%d\n", DMQA__DBG_PREFIX, num_sockets);
	local_socket = sockets[--num_sockets];
	(void) shutdown(local_socket, (int) 2);
	(void) close(local_socket);

	if (flag == 0) break;
    }
    /* Server processes may have morethan 1 socket in effect (the       */
    /* remote socket being used, and the local one being used to listen */
    /* on.                                                              */

    if (num_sockets == 0)
    {
	DMQA__link_active   = FALSE;
	socket_created	    = FALSE;

#ifdef VMS
#if DMQA__OFFSPRING
        if (	(DMQA__offspring_flag != 0) &&
		(DMQA__efn != 0) &&
		(DMQA_efn_set == TRUE))
        {
	    printf("Clearing EFN %d and disconnecting from EFC %s\n",
                DMQA__efn, DMQA__efc_name);
                    
            local_status    = sys$clref(DMQA__efn);
            (void) sys$dacefc(DMQA__efn);
	    DMQA_efn_set = FALSE;
        }
#endif
#endif
    }
    else
	if (DMQA__debug)
	    printf("%s (%d socket(s) still active)\n",
		DMQA__DBG_PREFIX, num_sockets);
    /* If all sockets are closed, then disable the link active flag */
    
    if (DMQA__debug)
	printf("%s * Exiting DMQA__link_shutdown *\n", DMQA__DBG_PREFIX);
}


/****************************************************************************/
/* Module:    DMQA__link_create                                             */
/*                                                                          */
/* Function: Create the TCP/IP link to the remote system. This function     */
/*	     does not return an error if the link has already been created, */
/*           but will display a message about it if DMQA__debug is enabled. */
/*                                                                          */
/*                                                                          */
/* Passed:   *DMQA__host	Name of host system. (not applicable to     */
/*				server process).                            */
/*                                                                          */
/*	    DMQA__port		Port number.                                */
/*                                                                          */
/*	    DMQA__debug		If set, causes debugging messages to be     */
/*                              displayed                                   */
/*                                                                          */
/*	    *DMQA__secondary_host                                           */
/*				Name of secondary host system (for          */
/*                              failover) -- NOT CURRENTLY IMPLEMENTED --   */
/*                              Applies to clients only.                    */
/*                                                                          */
/*	    DMQA__secondary_port                                            */
/*				Port number on secondary host system (for   */
/*				failover) -- NOT CURRENTLY IMPLEMENTED --   */
/*                              Applies to clients only.                    */
/*                                                                          */
/*          DMQA__efc_name      Name of Common event flag cluster. Applies  */
/*                              to only server programs running on VMS      */
/*                              systems.                                    */
/*                                                                          */
/*          DMQA__efn           EFN withinn the cluster that applies to us  */
/*                              Applies to only server programs running on  */
/*                              VMS systems.                                */
/*                                                                          */
/*	    DMQA__program_id	Determines if client or server type of con- */
/*				nection is needed.                          */
/*                                                                          */
/****************************************************************************/

long DMQA__link_create(nullarg)

long	nullarg;
{
    extern  int		    DMQA__program_id;
    int			    retval;
    char		    host_buffer[256];
    int			    bind_flag	= FALSE;
    int			    local_socket;
    int			    local_index;
    unsigned short          temp_port_no    = 0;
    long                    local_status;
    short                   msg_length      = 0;
    DMQA__HEADER            DMQA_header_rec;
    int			    connect_count   = 0;
    int			    loop_flag	    = TRUE;

#ifdef VMS
#if DMQA__OFFSPRING
    struct  dsc$descriptor_s 	efc_struct;

    extern  size_t          strlen();
    extern  long            sys$ascefc();
    extern  long            DMQA_get_symbol();
    extern  long	    sys$setef();
#endif
#endif

#ifdef __USE_ARGS
    extern  int             socket(int, int, int);
    extern  int		    bind(int, struct sockaddr *, int);
    extern  int		    connect(int, struct sockaddr *, int);
    extern  int		    gethostname(char *, int);
    extern  struct hostent  *gethostbyname(char *);
#ifdef AS400
    extern  unsigned short  htons(unsigned short);
#endif
    extern  long	    DMQA__link_receive(char *, short *, int, int, long);
    extern  unsigned short  DMQA__ntohs(unsigned short);
    extern  unsigned long   DMQA__ntohl(unsigned long);
#else
    extern  int             socket();
    extern  int		    bind();
    extern  int		    connect();
    extern  int		    gethostname();
    extern  struct hostent  *gethostbyname();
#ifdef AS400
    extern  unsigned short  htons();
#endif
    extern  long	    DMQA__link_receive();
    extern  unsigned short  DMQA__ntohs();
    extern  unsigned long   DMQA__ntohl();
#endif


    if (DMQA__debug)
    {
        printf("%s * Entering DMQA__link_create routine, CS flag = %d *\n",
            DMQA__DBG_PREFIX, DMQA__program_id);

	if (	(DMQA__program_id == DMQA__SERVER_ID) &&
		(DMQA__offspring_flag != 0))
	    printf("%s This server supports offspring creation\n",
		DMQA__DBG_PREFIX);
    }


    if (DMQA__argc) DMQA__process_args();
    /* Process command line for arguments if present */

    if (    (DMQA__port == 0) ||
	    ((DMQA__program_id == DMQA__CLIENT_ID) && (DMQA__host == 0)))
        DMQA__load_params();
    /* load them from file if not set already */

    if (DMQA__link_active)
    {
        if (DMQA__debug)
	    printf("%s %Link is already active\n", DMQA__DBG_PREFIX);
	return (long) PAMS__SUCCESS;
    }
    /* Link was already active -- ignore call and just use it */


    if (DMQA__port == 0)
    {
	if (DMQA__debug)
	{
	    printf("%s No port specified\n", DMQA__DBG_PREFIX);
	    DMQA__DBG_EXIT(PAMS__COMMERR);
	}
	return (long) PAMS__COMMERR;
    }
    /* After all that, the user still didn't tell us what port to use */


    /* If this is a client, then the remote host needs to be specified  */
    /* If it's the server, then the local system itself is the host     */
    
    if (DMQA__program_id == DMQA__CLIENT_ID)
    {
	if (DMQA__host == 0)
	{
	    if (DMQA__debug)
	    {
		printf("%s ?No host name specified\n", DMQA__DBG_PREFIX);
		DMQA__DBG_EXIT(PAMS__COMMERR);
	    }
	    return (long) PAMS__COMMERR;
	}
    }
    else
    {
	printf("Server process will use port %u\n", DMQA__port);

    	DMQA__highest_port = DMQA__port + DMQA__OFFSPRING;

	if (DMQA__debug)
 	{
	    printf("%s getting local host name...\n", DMQA__DBG_PREFIX);
	    printf("%s setting highest port to %u\n",
		DMQA__DBG_PREFIX, DMQA__highest_port);
 	}

        retval = gethostname(host_buffer, sizeof host_buffer);
	/* Get local host name */

	if (retval)
	{
	    DMQA__errno = (long) errno;

	    if ((DMQA__debug != 0) || (DMQA__program_id == DMQA__SERVER_ID))
		perror("?gethostname");

	    if (DMQA__debug) DMQA__DBG_EXIT(PAMS__COMMERR);

	    return (long) PAMS__COMMERR;
	}

	DMQA__host = host_buffer;


#ifdef VMS
#if DMQA__OFFSPRING

	if (DMQA__efc_name == 0)
	{
	    if (DMQA__offspring_flag == 0)
	    {
		(void) sprintf(efc_name_buf, "DMQA_%d_EFC", DMQA__port);
		DMQA__efc_name = efc_name_buf;
	    }
	    else
	    {
		if (DMQA__debug)
		{
		    printf("%s ?No EFC name specified!\n",
			DMQA__DBG_PREFIX);
		    DMQA__DBG_EXIT(PAMS__COMMERR);
		}
		return (long) PAMS__COMMERR;
	    }
	}

	if (DMQA__debug)
	    printf("%s Server EFC name is %s\n",
		DMQA__DBG_PREFIX, DMQA__efc_name);
    
	if (DMQA__offspring_flag)
	{
	    if (DMQA__efn == 0)
	    {
		if (DMQA__debug)
		{
		    printf(
		    "%s ?This server is an offspring and no EFN was specified\n",
			DMQA__DBG_PREFIX);
		    DMQA__DBG_EXIT(PAMS__COMMERR);
		}
		return (long) PAMS__COMMERR;
	    }
	    else
	    {
		if (DMQA__debug)
		    printf("%s This program is a server offspring, EFN = %d\n",
			DMQA__DBG_PREFIX, DMQA__efn);
	    }
	}
	else
	{
            if ((local_status = DMQA_get_symbol(&dir_struct, dir_symbol,
                (int) sizeof(dir_symbol), DMQA__DIR_SYMBOL,
		(int) sizeof(DMQA__DIR_SYMBOL) - 1)) != PAMS__SUCCESS)
		    return local_status;

            if ((local_status = DMQA_get_symbol(&bus_struct, bus_symbol,
                (int) sizeof(bus_symbol), DMQA__BUS_SYMBOL,
		(int) sizeof(DMQA__BUS_SYMBOL) - 1)) != PAMS__SUCCESS)
		    return local_status;

            if ((local_status = DMQA_get_symbol(&exe_struct, exe_symbol,
                (int) sizeof(exe_symbol), DMQA__EXE_SYMBOL,
		(int) sizeof(DMQA__EXE_SYMBOL) - 1)) != PAMS__SUCCESS)
		    return local_status;

            if ((local_status = DMQA_get_symbol(&dbg_struct, dbg_symbol,
                (int) sizeof(dbg_symbol), DMQA__DBG_SYMBOL,
		(int) sizeof(DMQA__DBG_SYMBOL) - 1)) != PAMS__SUCCESS)
		    return local_status;
	}
#endif
#endif

    }

    if (DMQA__debug)
    {
	printf("%s --> Remote node = %s, port = %u <--\n",
	    DMQA__DBG_PREFIX, DMQA__host, DMQA__port);

	printf("%s searching network database for %s\n",
	    DMQA__DBG_PREFIX, DMQA__host);
    }

    if ((hostentptr = gethostbyname(DMQA__host)) == NULL)
    /* search for requested name */
    {
        DMQA__errno = (long) errno;
            
    	if ((DMQA__debug != 0) || (DMQA__program_id == DMQA__SERVER_ID))
	    perror("?gethostbyname");

	if (DMQA__debug) DMQA__DBG_EXIT(PAMS__COMMERR);

        DMQA__link_shutdown(-1, 0L);
	return (long) PAMS__COMMERR;
    }
    /* Get pointer to network data structure for socket 2 (remote host). */


    if (DMQA__debug)
	printf("%s Host address of %s is %d.%d.%d.%d\n",
	    DMQA__DBG_PREFIX,
	    hostentptr -> h_name,
	    hostentptr -> h_addr[0],
	    hostentptr -> h_addr[1],
	    hostentptr -> h_addr[2],
	    hostentptr -> h_addr[3]);
    /* Display the internet address found */
		

    while (loop_flag)
    {
	loop_flag = FALSE;

	if (socket_created == FALSE)    /* create the local socket if needed */
	{
	    connect_count++;

	    if (DMQA__debug)
		printf("%s Creating new socket...\n", DMQA__DBG_PREFIX);

	    if ((local_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	    {
		DMQA__errno = (long) errno;

		if ((DMQA__debug != 0) || (DMQA__program_id == DMQA__SERVER_ID))
		    perror("?socket");

		if (DMQA__debug) DMQA__DBG_EXIT(PAMS__COMMERR);

		return (long) PAMS__COMMERR;
	    }
	    socket_created		= TRUE;	
	    sockets[num_sockets++]	= local_socket;
	}
	else
	    if (DMQA__debug)
		printf("%s %socket already created - skipping socket call\n",
		    DMQA__DBG_PREFIX);
	/* Open socket 1: AF_INET, SOCK_STREAM. */


	hostentstruct               = *hostentptr;
	/* copy pointer to safe storage */

	socket_struct.sin_family    = hostentstruct.h_addrtype;
	socket_struct.sin_addr      =
	    * ((struct in_addr *) hostentstruct.h_addr);
	if (connect_count == 1)
	    socket_struct.sin_port  = htons(DMQA__port);
	else
	    socket_struct.sin_port  = htons(temp_port_no);
	/* Fill in the name & address structure for socket 2 */


	if (DMQA__program_id == DMQA__CLIENT_ID)
	/* OK, if this is the client, then attempt a connection */
	{
	    if (DMQA__debug)
		printf("%s connecting to server...\n", DMQA__DBG_PREFIX);

	    if ((retval = connect(  sockets[num_sockets - 1],
				    (struct sockaddr *) &socket_struct,
				    (int) sizeof(socket_struct))))
	    {
		DMQA__errno = (long) errno;

		if ((DMQA__debug != 0) || (DMQA__program_id == DMQA__SERVER_ID))
		    perror("?connect");

		if (DMQA__debug) DMQA__DBG_EXIT(PAMS__LINK_DOWN);

		DMQA__link_shutdown(-1, 0L);
		return (long) PAMS__LINK_DOWN;
	    }
        
	    DMQA__link_active	= TRUE;     /* We've got a link */

	    if (DMQA__debug)
		printf(
		"%s Attempting to receive server's desired port number\n",
		    DMQA__DBG_PREFIX);
                
	    local_status = DMQA__link_receive(  local_buffer,
						&msg_length,
						0, 0, 0L);
	    /* Get port number from server */
                
	    if (    (local_status != PAMS__SUCCESS) ||
		    (msg_length != DMQA__HEADERSIZE))
	    {
		if (DMQA__debug) DMQA__DBG_EXIT(PAMS__COMMERR);
            
		DMQA__link_shutdown(-1, 0L);
		return (long) PAMS__COMMERR;
	    }
        
	    (void) memcpy(  (VOID *) &DMQA_header_rec,
			    (CONST VOID *) local_buffer,
			    (size_t) sizeof(local_buffer));
	    /* copy header into structure */
        
	    temp_port_no = DMQA__ntohs(DMQA_header_rec.offspring_link);

	    if ((local_status =
		(long) DMQA__ntohl(DMQA_header_rec.pams_status))
		    != PAMS__SUCCESS)
	    {
		DMQA__errno = DMQA__ntohl(DMQA_header_rec.error);
            
		if (DMQA__debug)
		{
		    printf(
		    "%s Server returned port %u, error %d, errno %d\n",
			DMQA__DBG_PREFIX,
			temp_port_no,
			local_status,
			DMQA__errno);

		    if (local_status == PAMS__NOOBJECT)
			printf(
			"%s NO OFFSPRING SLOTS AVAILABLE IN SERVER\n",
			    DMQA__DBG_PREFIX);

		    DMQA__DBG_EXIT(PAMS__LINKDOWN);
		}
            
		DMQA__link_shutdown(-1, 0L);
		DMQA__errno = local_status;
		return (long) PAMS__NOLINK;
	    }


	    if (temp_port_no == 0)
	    {
		if (DMQA__debug)
		{
		    printf("%s server returned a port number of zero!\n",
			DMQA__DBG_PREFIX);
		    DMQA__DBG_EXIT(PAMS__INTERNAL);
		}
            
		DMQA__link_shutdown(-1, 0L);
		return (long) PAMS__INTERNAL;
	    }
        

	    if (temp_port_no == DMQA__port)
	    {
		if (DMQA__debug)
			printf(
			"%s Server returned SAME port - using current connection!\n",
			    DMQA__DBG_PREFIX);
	    }
	    else
	    {
		if (DMQA__debug)
		    printf("%s Server returned port number of %d\n",
			DMQA__DBG_PREFIX, temp_port_no);

		if (connect_count == 1)
		{
		    if (DMQA__debug)
			printf("%s shutting down existing connection\n",
			    DMQA__DBG_PREFIX);
                
		    DMQA__link_shutdown(-1, 0L);
		    loop_flag = TRUE;
		}
	    }
	}
	else    /* Otherwise, we're the server so just bind us a socket */
	{

#ifdef VMS
#if DMQA__OFFSPRING

	    efc_struct.dsc$w_length     =
		(unsigned short) strlen((CONST char *) DMQA__efc_name);
	    efc_struct.dsc$b_dtype      = DSC$K_DTYPE_T;
	    efc_struct.dsc$b_class      = DSC$K_CLASS_S;
	    efc_struct.dsc$a_pointer    = DMQA__efc_name;

	    if (DMQA__debug)
		printf("%s calling SYS$ASCEFC with EFC name %s, EFN %d\n",
		    DMQA__DBG_PREFIX, DMQA__efc_name, DMQA__TIMER_EFN);

	    if ((local_status = sys$ascefc( DMQA__TIMER_EFN,
					    &efc_struct,
					    0,
					    0)) != SS$_NORMAL)
	    {
		DMQA__errno = local_status;
        
		if (DMQA__debug)
		{
		    printf("%s sys$ascefc call returned error %d\n",
			DMQA__DBG_PREFIX, local_status);
		    DMQA__DBG_EXIT(PAMS__INTERNAL);
		}
        
		DMQA__link_shutdown(-1, 0L);
		return (long) PAMS__INTERNAL;
	    }
#endif
#endif
	    if ((retval = bind( sockets[num_sockets - 1],
				(struct sockaddr *) &socket_struct,
				(int) sizeof(socket_struct))))
	    {
		DMQA__errno = (long) errno;
		perror("?bind");

#ifdef VMS
#if DMQA__OFFSPRING
		if ((DMQA__errno == EADDRINUSE) &&
		    (DMQA__offspring_flag != 0))
		{
		    printf("Forcing TIMEOUT on parent process\n");
		    (void) sys$setef(DMQA__TIMER_EFN);
		}
		/* Just to help wake up the parent quicker */
#endif
#endif
		if (DMQA__debug) DMQA__DBG_EXIT(PAMS__NOLINK);

		DMQA__link_shutdown(-1, 0L);
		return (long) PAMS__NOLINK;
	    }

	    DMQA__link_active	= TRUE;     /* We've got a link */
	}


	/* send_packet_size is the maximum number of bytes we can send in   */
	/* a single message packet until breakup is needed. This value      */
	/* starts off at a hard-coded constant, but may decrease if the     */
	/* remote system supports a lower value. Eventually, this value     */
	/* be the largest that both systems support together.               */
                
	send_packet_size	= DMQA__SENDMAX;
	if (DMQA__debug)
	    printf("%s setting packetsize to %d\n",
		    DMQA__DBG_PREFIX, send_packet_size);

	packet_reduced = FALSE;
    }

    if (DMQA__debug) DMQA__DBG_EXIT(PAMS__SUCCESS);
    return (long) PAMS__SUCCESS;
 }



/****************************************************************************/
/* Module:    DMQA__link_send                                               */
/*                                                                          */
/* Function: Send a message buffer to the remote system via TCP/IP. This    */
/*           function will also create the link if needed (and specified).  */
/*                                                                          */
/****************************************************************************/

long DMQA__link_send(init_allowed, msg_ptr, length, flag, link_no, nullarg)

int     init_allowed;           /* if set, link will be created if necessary */
char    *msg_ptr;		/* ptr to message buffer to send */
short   length;                 /* length of message to send */
int     flag;                   /* flags */
int	link_no;		/* reserved for future use */
long    nullarg;                /* reserved */
{
    int			    retval;	    /* for socket returned status */
    long		    status;
    extern unsigned long    DMQA__scount;
    char                    *local_msg_ptr;
    short                   local_length;
    short                   send_length;
    short                   packets;
    DMQA__HEADER	    DMQA_header_rec;

#ifdef __USE_ARGS
#ifdef AS400
    extern unsigned short   DMQA__htons(unsigned short);
#endif
    extern long		    DMQA__link_create(long);
#else
#ifdef AS400
    extern unsigned short   DMQA__htons();
#endif
    extern long		    DMQA__link_create();
#endif


    if (DMQA__debug)			/* debugging info */
	printf("%s * Entering DMQA__link_send routine *\n", DMQA__DBG_PREFIX);
    
    if (msg_ptr == 0)
    {
        if (DMQA__debug)
        {
            printf("%s msg_ptr is null!\n", DMQA__DBG_PREFIX);
            DMQA__DBG_EXIT(PAMS__BADLOGIC);
        }
        return (long) PAMS__BADLOGIC;
    }
                
        
    if ((init_allowed) && (DMQA__link_active == FALSE))
    {
        status = DMQA__link_create(0L);

        if (DMQA__debug)
            printf("%s DMQA__link_create routine returned %d\n",
                DMQA__DBG_PREFIX, status);
                
        if ((status & 1) == 0)
        {
            if (DMQA__debug) DMQA__DBG_EXIT(status);
            return status;
        }
    }
    /* Create link if non exists and this PAMS command allows the   */
    /* creation of a link.                                          */


    if (DMQA__link_active == FALSE)
    {
	if (DMQA__debug)
	{
	    printf("%s ?Cannot send; no link is active\n", DMQA__DBG_PREFIX);
	    DMQA__DBG_EXIT(PAMS__NOTDCL);
	}
	return (long) PAMS__NOTDCL;
    }
    /* Hey - no link is active and the command doesn't allow for one to */
    /* be created. This returns an error!                               */
    

    if (DMQA__debug)
	printf("%s filling in packetsize and message type...\n",
	    DMQA__DBG_PREFIX);

    (void) memcpy(  (VOID *) &DMQA_header_rec,
		    (CONST VOID *) msg_ptr,
		    (size_t) DMQA__HEADERSIZE);
    /* copy header so we can fill in our 2 fields */

    DMQA_header_rec.max_packetsize =
	(short) DMQA__htons((unsigned short) send_packet_size);
    DMQA_header_rec.type = 1;
    /* fill in fields */

    (void) memcpy(  (VOID *) msg_ptr,
		    (CONST VOID *) &DMQA_header_rec,
		    (size_t) DMQA__HEADERSIZE);
    /* copy header back into message */

    for (   local_length = length, local_msg_ptr = msg_ptr, packets = 0;
            local_length > 0;
            local_length -= send_length, local_msg_ptr += send_length,
                ++packets)
    {
        if (local_length > send_packet_size)
            send_length = send_packet_size;
        else
            send_length = local_length;
        /* The most we can send at one time is in send_packet_size  */
            
        if (DMQA__debug)
            printf("%s sending packet of %d bytes\n",
                DMQA__DBG_PREFIX, send_length);
            
        if ((retval = send(  sockets[num_sockets - 1],
    		            local_msg_ptr,
		            (int) send_length,
		            flag)) != send_length)
        {
	    DMQA__errno = (long) errno;

	    if ((DMQA__debug != 0) || (DMQA__program_id == DMQA__SERVER_ID))
	        perror("?send");

            if (    (packets != 0) ||
		    (errno == ECONNRESET) ||
		    (errno == ECONNABORTED))
	    {
		DMQA__link_shutdown(0, 0L);
		return (long) PAMS__LINK_DOWN;
	    }
	    else
		return (long) PAMS__COMMERR;
        }
        else
	{
	    if (DMQA__scount == 4294967295L) DMQA__scount = 0L;
	    ++DMQA__scount;
	}
    }
    /* Send message and increment counter */

    if (DMQA__debug)
	DMQA__DBG_EXIT(PAMS__SUCCESS);
    return (long) PAMS__SUCCESS;
}

/****************************************************************************/
/* Module:    DMQA__link_receive                                            */
/*                                                                          */
/* Function: This routine receives TCPIP packets and returns the            */
/*	     reconstructed message buffer.                                  */
/*                                                                          */
/****************************************************************************/

long    DMQA__link_receive(msg_ptr, ret_length, flag, link_no, nullarg)

char	*msg_ptr;			    /* ptr to receive buffer */
short	*ret_length;
int	flag;				    /* TCP/IP recv flags field */
int	link_no;			    /* reserved for future use */
long	nullarg;			    /* Reserved for "inhibit timer" */
{
    int	    	    retval;			/* socket return status       */
    short	    current_byte_cnt	= 0;	/* current # of bytes recv'd  */
    short	    packets_received	= 0;	/* cur. # of packets recv'd   */
    short	    bytes_expected      = DMQA__HEADERSIZE;
    short	    packet_size;		/* size of this packet        */
    short	    awaiting_header	= TRUE;	/* flag: awaiting 1st packet? */
    char	    *local_ptr;			/* ptr to parts of the buffer */
    DMQA__HEADER    DMQA_header_rec;
    int		    bufsiz = DMQA__SIZMAX;
    short	    temp_send_packet_size = 0;

    extern unsigned long    DMQA__rcount;

#ifdef __USE_ARGS
    extern unsigned short   DMQA__ntohs(unsigned short);
#else
    extern unsigned short   DMQA__ntohs();
#endif


    if (DMQA__debug)
	printf("%s * Entering DMQA__link_receive, flag = %d *\n",
	    DMQA__DBG_PREFIX, flag);

    local_ptr = msg_ptr;            /* save the pointer */

    if (local_ptr == 0)
    {
	if (DMQA__debug)
	{
	    printf("%s ?message pointer is zero!\n",DMQA__DBG_PREFIX);
	    DMQA__DBG_EXIT(PAMS__BADLOGIC);
	}
	return (long) PAMS__BADLOGIC;
    }
	

    if (DMQA__link_active == FALSE)
    {
	if (DMQA__debug)
	{
	    printf("%s ?Cannot recv; no link is active\n", DMQA__DBG_PREFIX);
	    DMQA__DBG_EXIT(PAMS__NOTDCL);
	}
	return (long) PAMS__NOTDCL;
    }


    do
    {
	retval = recv( sockets[num_sockets - 1],
			    local_ptr,
			    bufsiz,
			    flag);
    	/* Receive msg from socket; flag maybe 0 or MSG_OOB or MSG_PEEK */

	DMQA__errno = (long) errno;

	if (flag)
	{
	    if (DMQA__debug)
		printf(
		"%s Exiting with PAMS__SUCCESS because FLAG was non-zero\n",
		    DMQA__DBG_PREFIX);

	    return (long) PAMS__SUCCESS;
	}
	/* Using the Async flags will always return success */
	/* The "real" status is the DMQA__errno field */

	if (retval == -1)
	{
	    if (DMQA__debug) perror("_DMQA: ?recv");

	    if ((errno == ECONNRESET) || (errno == ECONNABORTED))
	    {
		DMQA__link_shutdown(0, 0L);
		return (long) PAMS__LINK_DOWN;
	    }
	    else
		return (long) PAMS__COMMERR;
	}

    	packet_size = retval;

	if (DMQA__debug)
	    printf("%s received msg; packetsize = %d\n",
		DMQA__DBG_PREFIX, packet_size);

	if (awaiting_header)			/* Is this the first packet? */
	{

	    if (current_byte_cnt + packet_size < DMQA__HEADERSIZE)
	    {
		if (DMQA__debug)
		    printf(
		    "%s Warning - msg too short for valid header; continuing\n",
	    		DMQA__DBG_PREFIX);
	    }
	    /* Message is too short to deal with */

	    else
	    {

	    (void) memcpy(	(VOID *) &DMQA_header_rec,
				(CONST VOID *) msg_ptr,
				(size_t) DMQA__HEADERSIZE);
	    /* load header record from beginning portion of message */

	    bytes_expected  = ((short)
		DMQA__ntohs((unsigned short) DMQA_header_rec.msg_length));

	    if (DMQA__debug)
		printf("%s (number of bytes expected = %d)\n",
			DMQA__DBG_PREFIX, bytes_expected);

	    if (bytes_expected > DMQA__SIZMAX)
	    {
		if (DMQA__debug)
		{
		    printf("%s ?msg too big to receive; exceeds maximum (%d)\n",
	    		DMQA__DBG_PREFIX, DMQA__SIZMAX);
		    DMQA__DBG_EXIT(DMQA__BUFFEROVF);
		}
		return (long) DMQA__BUFFEROVF;
	    }

	    temp_send_packet_size = (short)
		DMQA__ntohs((unsigned short) DMQA_header_rec.max_packetsize);
	    /* extract his packet size */

	    if ((temp_send_packet_size < send_packet_size) &&
                (temp_send_packet_size > 0))
	    {
		if (DMQA__debug)
		    printf("%s setting packetsize to %d\n",
			DMQA__DBG_PREFIX, temp_send_packet_size);
		send_packet_size = temp_send_packet_size;
	    }

            awaiting_header	= FALSE;	    /* We've got the header */
	    }
	}

	current_byte_cnt	+= packet_size;
	local_ptr		+= packet_size;
	++packets_received;

    } while (current_byte_cnt < bytes_expected);
    /* Go back for more packets if we're expecting more */

    *ret_length = local_ptr - msg_ptr;

    if (DMQA__rcount == 4294967295L) DMQA__rcount = 0L;
    ++DMQA__rcount;

    if (DMQA__debug)
	printf("%s Received message #%u, %d bytes\n",
	    DMQA__DBG_PREFIX, DMQA__rcount, *ret_length);

    return (long) PAMS__SUCCESS;
}



/****************************************************************************/
/* Module:    DMQA__link_check                                              */
/*                                                                          */
/* Function: This routine is used by the server to check the link's status  */
/*                                                                          */
/****************************************************************************/

long    DMQA__link_check(nullarg)

long	nullarg;
{
    long	    returned_status = PAMS__SUCCESS;
    long	    local_status    = PAMS__SUCCESS;

#ifdef VMS
    int		    local_socket;
    int		    rfields = -1;
    int		    wfields = -1;
    int		    efields = -1;
    struct timeval  timeout;
	
    extern int	    select();
#endif

    if (DMQA__debug)
        printf("%s * Entering DMQA__link_check routine\n", DMQA__DBG_PREFIX);

#ifdef VMS
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;
    local_socket = sockets[num_sockets - 1];

    local_status = (long) select(32, &rfields, &wfields, &efields, &timeout);

    if (local_status == -1)
	perror("_DmQA: ?select");

    wfields &= (1 << local_socket);
    rfields &= (1 << local_socket);
    efields &= (1 << local_socket);

    if (local_status != 0)
    {
	if ((wfields == 0) || (rfields != 0) || (efields != 0))
		returned_status = PAMS__COMMERR;
    }
#endif

    DMQA__errno = local_status;

    if (DMQA__debug)
    {
#ifdef VMS
	printf("%s select returned w=%d, r=%d, e=%d\n",
	    DMQA__DBG_PREFIX, rfields, wfields, efields);
#endif
	printf("%s -- Exiting DMQA__link_check with status %d errno %d --\n",
	    DMQA__DBG_PREFIX, returned_status, DMQA__errno);
    }

    return returned_status;
}

/****************************************************************************/
/* Module:    DMQA__link_listen                                             */
/*                                                                          */
/* Function: This routine is used by the server to await connect requests   */
/*	     on the first link.                                             */
/*                                                                          */
/****************************************************************************/
long    DMQA__link_listen(nullarg)

long	nullarg;
{
    int		local_status;
    long	local_long;

#ifdef __USE_ARGS
    extern int	listen(int, int);
#else
    extern int	listen();
#endif

#ifdef VMS
#if DMQA__OFFSPRING
    extern long	sys$setef();
#endif
#endif


    if (DMQA__debug)
	printf("\n%s * Entering DMQA__link_listen *\n", DMQA__DBG_PREFIX);

    if ((local_status = listen(	sockets[0], (int) DMQA__TCPIP_BACKLOG)) == -1)
    {
	DMQA__errno = (long) errno;

    	if ((DMQA__debug != 0) || (DMQA__program_id == DMQA__SERVER_ID))
        {
	    perror("?listen");
            printf("?Socket number was %d; total sockets = %d\n",
		sockets[0], num_sockets);
 	}

	if (DMQA__debug) DMQA__DBG_EXIT(PAMS__COMMERR);

	DMQA__link_shutdown(-1, 0L);
	return (long) PAMS__COMMERR;
    }

#ifdef VMS
#if DMQA__OFFSPRING

    /* This is for the offspring. It only needs to set the EF intended	*/
    /* for us.								*/
    
    if (DMQA__offspring_flag)   /* We is offspring - we need tell server */
    {
	printf("Setting event flag %d to notify parent\n", DMQA__efn);
                
        if ((local_long  = (long) sys$setef(DMQA__efn)) != SS$_WASCLR)
        {
            DMQA__errno = local_long;
            
            if (DMQA__debug)
            {
                printf("%s ?SYS$SETEF call returned %d\n",
                    DMQA__DBG_PREFIX, local_long);

                DMQA__DBG_EXIT(PAMS__INTERNAL);
            }
            
            DMQA__link_shutdown(-1, 0L);
            return (long) PAMS__INTERNAL;
        }

	DMQA_efn_set = TRUE;
    }
#endif
#endif

    if (DMQA__debug) DMQA__DBG_EXIT(PAMS__SUCCESS);
    return (long) PAMS__SUCCESS;
}


/****************************************************************************/
/* Module:    DMQA__link_accept                                             */
/*                                                                          */
/* Function: This routine is used by the server to accept connect requests  */
/*	     on the link, or to reject them (by returning an error and      */
/*           breaking the link).                                            */
/*                                                                          */
/****************************************************************************/

long    DMQA__link_accept(nullarg)

long	nullarg;
{
#ifdef VMS
    extern int	accept();   /* VMS has bad definition */
#else
#ifdef __USE_ARGS
    extern int	accept(int, struct sockaddr *, int *);
#else
    extern int	accept();
#endif
#endif

#ifdef __USE_ARGS
#ifdef AS400
    extern unsigned long    DMQA__htonl(unsigned long);
    extern unsigned short   DMQA__htons(unsigned short);
#endif
    extern long             DMQA__link_send(int, char *, short, int, int, long);
    extern void             DMQA__link_shutdown(int, long);
#else
#ifdef AS400
    extern unsigned long    DMQA__htonl();
    extern unsigned short   DMQA__htons();
#endif
    extern long             DMQA__link_send();
    extern void             DMQA__link_shutdown();
#endif

    int		    local_size;
    int		    local_socket;
    DMQA__HEADER    DMQA_header_rec;
    long            local_long;
    unsigned short  local_ushort;
    unsigned short  offspring_port;
    unsigned long   local_ulong;
    long            port_status = PAMS__SUCCESS;
    
    
#ifdef VMS
#if DMQA__OFFSPRING
    long                    ffc_arg1    = 0L;
    unsigned char           ffc_arg2;
    long                    bit_position;

    extern  long            sys$etef();
    extern  unsigned long   lib$ffc();
    extern  unsigned long   sys$readef();
    extern  long            DMQA_create_offspring(unsigned long,
				long, unsigned short);
#endif
#endif


    if (DMQA__debug)
	printf("%s * Entering DMQA__link_accept *\n", DMQA__DBG_PREFIX);

    local_size = sizeof(socket_struct);


    if ((local_socket = accept(	(int) sockets[0],
				(struct sockaddr *) &socket_struct,
#ifdef VMS
				(int *) &local_size)) == -1)
#else
				&local_size)) == -1)
#endif
    {
	DMQA__errno = (long) errno;

	if ((DMQA__debug != 0) || (DMQA__program_id == DMQA__SERVER_ID))
	    perror("?accept");

	if (DMQA__debug) DMQA__DBG_EXIT(PAMS__COMMERR);

	return (long) PAMS__COMMERR;
    }

    sockets[num_sockets++] = local_socket;

    send_packet_size = DMQA__SENDMAX;
    if (DMQA__debug)
	printf("%s setting packetsize to %d\n",
		DMQA__DBG_PREFIX, send_packet_size);

    offspring_port = DMQA__port;    /* Start off with our current value */

#ifdef VMS
#if DMQA__OFFSPRING

    /* This next section deals with the creation of offspring. If it is */
    /* successful, we will report the new available connection to the   */
    /* client. He will then break the current connection and attempt to */
    /* connect to the child server. That server will either accept the  */
    /* connection or timeout and go away.                               */
    
    /* The server needs to find a free EF. If found, and it's within	*/
    /* range of the number of offspring we're allowed, then start a	*/
    /* timer and create an offspring for that EF, then wait. If we do	*/
    /* not timeout - all is well. In either case, return the result to	*/
    /* the client.							*/

    if (DMQA__offspring_flag == FALSE)
    {
	ffc_arg2    = DMQA__OFFSPRING;		/* for LIB$FFC */
	DMQA__errno = 0;
    
	if (((local_long  = (long) sys$readef(  DMQA__TIMER_EFN,
						&DMQA__efn_map)) & 1) == 0)
	{
	    if (DMQA__debug)
		printf("%s ?SYS$READEF failed with status %d\n",
		    DMQA__DBG_PREFIX, local_long);
                    
	    port_status = PAMS__INTERNAL;
	}
	else
	{
	    local_ulong = lib$ffc(  &ffc_arg1,
				    &ffc_arg2,
				    &DMQA__efn_map,
				    &bit_position);
                                           
	    if (    (local_ulong != SS$_NORMAL) ||
		    (bit_position >= DMQA__OFFSPRING) ||
		    (bit_position + DMQA__port >= DMQA__highest_port))
	    {
		if (DMQA__debug)
		    printf(
		    "%s LIB$FFC returned %u and slot #%d - no free offspring slots assumed\n",
			    DMQA__DBG_PREFIX, local_ulong, bit_position);
                    
		port_status = PAMS__NOOBJECT;
	    }
	    else
	    {
		offspring_port  = DMQA__port + bit_position + 1;
            
		port_status     = DMQA_create_offspring(    bit_position,
							    bit_position + 64,
							    offspring_port);
	    }
	}
    }
#endif
#endif
                
    DMQA_header_rec.lbyte_order     = 0;
    DMQA_header_rec.task_id         = 0;
    DMQA_header_rec.msg_seq_no      = 0;
    DMQA_header_rec.sbyte_order     = 0;
    DMQA_header_rec.checksum        = 0;
    DMQA_header_rec.max_packetsize  = 0;

    DMQA_header_rec.id              = DMQA__SERVER_ID;
    DMQA_header_rec.os              = DMQA__OS;
    DMQA_header_rec.protocol        = DMQA__PROTOCOL;
    DMQA_header_rec.cmd             = DMQA__INTERNAL_MSG;
    DMQA_header_rec.type            = 1;
    
    (void) memcpy(  (VOID *) &DMQA_header_rec.ident[0],
		    (CONST VOID *) DMQA__IDENT,
		    (size_t) sizeof(DMQA_header_rec.ident));
    /* Copy IDENT into header */

    DMQA_header_rec.pams_status =
        (long) DMQA__htonl((unsigned long) port_status);
    DMQA_header_rec.error =
        (long) DMQA__htonl((unsigned long) DMQA__errno);
        
    local_long  = DMQA_BIT__INTERNAL_MSG;
    DMQA_header_rec.flags =
        (long) DMQA__htonl((unsigned long) local_long);

    local_ushort = offspring_port;
    DMQA_header_rec.offspring_link = DMQA__htons(local_ushort);

    local_ushort = DMQA__HEADERSIZE;
    DMQA_header_rec.msg_length =
        (short) DMQA__htons((unsigned short) local_ushort);
        
    (void) memcpy(  (VOID *) local_buffer,
                    (CONST VOID *) &DMQA_header_rec,
                    (size_t) DMQA__HEADERSIZE);
        
    if (DMQA__debug) printf("%s Sending port %u, and status %d to client\n",
        DMQA__DBG_PREFIX, offspring_port, port_status);
        
    if ((local_long = DMQA__link_send(    0,
                                            local_buffer,
                                            DMQA__HEADERSIZE,
                                            0,
                                            0,
                                            0L)) != PAMS__SUCCESS)
    {
        DMQA__link_shutdown(0, 0L);
        return (long) PAMS__COMMERR;
    }

    return (long) PAMS__SUCCESS;
}


#ifdef VMS
#if DMQA__OFFSPRING

/****************************************************************************/
/* Module:    DMQA_create_offspring					    */
/*                                                                          */
/* Function: Internal routine to create the offspring process if we are the */
/*	     DECmessageQ server process.				    */
/*                                                                          */
/****************************************************************************/

static long DMQA_create_offspring(bit, efn, port)

unsigned long	bit;
long            efn;
unsigned short  port;

{
    long                    wait_mask;
    static short            quadword[4];
    char                    timebuf[15];
    long                    local_status;
    unsigned long	    local_ulong;
    static int              init_flag = FALSE;
    static unsigned long    spawn_mask;
    long                    efn_mask;
    unsigned long	    stsflg = PRC$M_DETACH | PRC$M_NOPASSWORD;
#pragma nostandard
    $DESCRIPTOR             (time_struct, DMQA__TIMER_INTERVAL);
    $DESCRIPTOR             (lio_struct, "SYS$SYSTEM:LOGINOUT.EXE");
#pragma standard
    struct dsc$descriptor_s file_struct;
    struct dsc$descriptor_s process_struct;
    char		    log_symbol_name[80];
    char                    file_name[80];
    char		    process_name[16];
    int			    log_name_len;
    int                     local_int;
    FILE                    *fp;
    char                    cmd_line[80];
    
    extern  long            sys$setimr();
    extern  long            sys$wflor();
    extern  long            sys$bintim();
    extern  long	    sys$creprc();
    extern  long	    sys$setef();
    extern  long            sys$cantim();    
    extern  unsigned long   sys$readef();
    extern  unsigned long   lib$delete_file();
    extern  long   	    DMQA_get_symbol();
    extern  int		    atoi();
    extern  size_t	    strlen();
    extern  char            *strcpy();
    extern  FILE            *fopen();
    extern  int             fclose();
    extern  size_t          fwrite();
    

    if (DMQA__debug)
        printf("%s << Entering DMQA_create_offspring routine, efn = %d >>\n",
            DMQA__DBG_PREFIX, efn);

    if (init_flag == FALSE);
    {
        if (DMQA__debug)            
            printf("%s Building timer string and spawn mask...\n",
                DMQA__DBG_PREFIX);
            
        if ((local_status = sys$bintim(&time_struct, quadword)) != SS$_NORMAL)
        {
            DMQA__errno = local_status;
            
            if (DMQA__debug)
                printf("%s SYS$BINTIM returned %d - exiting\n",
                    DMQA__DBG_PREFIX);

            return (long) PAMS__INTERNAL;
        }

        spawn_mask  = 1 | 2 | 8;
        init_flag   = TRUE;
    }

    if (DMQA__debug)
	printf("%s Building LOGFILE symbol...\n", DMQA__DBG_PREFIX);

    log_name_len = sprintf(log_symbol_name, "%s%d",
	DMQA__LOG_SYMBOL, bit + 1);

    if ((local_status = DMQA_get_symbol( &log_struct, log_symbol,
	(int) sizeof(log_symbol), log_symbol_name, (int) log_name_len))
	    != PAMS__SUCCESS) return local_status;
        
    if (DMQA__debug)
	printf("%s LOGFILE name is %s\n", DMQA__DBG_PREFIX, log_symbol);

    (void) sprintf( file_name, (CONST char *) "%sOFFSPRING_%u%c.TMP",
	dir_symbol, DMQA__port, bit+65);
    
    if (DMQA__debug)
        printf("%s Command file is %s\n", DMQA__DBG_PREFIX, file_name);

    file_struct.dsc$w_length	    = (unsigned short) strlen(file_name);
    file_struct.dsc$a_pointer	    = file_name;
    file_struct.dsc$b_dtype	    = DSC$K_DTYPE_T;
    file_struct.dsc$b_class	    = DSC$K_CLASS_S;

    local_ulong = lib$delete_file(&file_struct, 0,0,0,0,0,0,0,0);
    if ((local_ulong != PAMS__SUCCESS) && (DMQA__debug != 0))
	printf("%s note: lib$delete returned status %u\n",
	    DMQA__DBG_PREFIX, local_ulong);
    /* We really don't care if this fails - VMS will still create it */            

    fp = fopen( (CONST char *) file_name, (CONST char *) "w");

    if (fp == (FILE *) NULL)
    {
        if (DMQA__debug)
            printf("%s ?fopen() returned error - exiting!\n", DMQA__DBG_PREFIX);
        return (long) PAMS__INTERNAL;
    }
    /* OPEN failed - abort */

    (void) fwrite(  (VOID *) "$ SET NOVERIFY\n",
                    (size_t) sizeof("$ SET NOVERIFY\n") - 1,
                    (size_t) 1,
                    fp);
    /* verify */

    (void) fwrite(  (VOID *) "$ SET NOON\n",
                    (size_t) sizeof("$ SET NOON\n") - 1,
                    (size_t) 1,
                    fp);

    (void) fwrite(  (VOID *) DMQA__TIMESTAMP,
		    (size_t) sizeof(DMQA__TIMESTAMP) - 1,
                    (size_t) 1,
                    fp);
    /* timestamp */

    (void) fwrite(  (VOID *) "$ SET PROC/PRIV=ALL\n",
		    (size_t) sizeof("$ SET PROC/PRIV=ALL\n") - 1,
                    (size_t) 1,
                    fp);
    /* Enable Privs */

    *(bus_symbol + bus_struct.dsc$w_length) = '\n';
    (void) fwrite(  (VOID *) bus_symbol,
                    (size_t) bus_struct.dsc$w_length + 1,
                    (size_t) 1,
                    fp);
    /* DmQ switch to bus and group */
    
    (void) fwrite(  (VOID *) "$ SET VERIFY\n",
                    (size_t) sizeof("$ SET VERIFY\n") - 1,
                    (size_t) 1,
                    fp);
    /* verify */

    *(dbg_symbol + dbg_struct.dsc$w_length) = '\n';
    (void) fwrite(  (VOID *) dbg_symbol,
                    (size_t) dbg_struct.dsc$w_length + 1,
                    (size_t) 1,
                    fp);
    /* DMQ$DEBUG */
    
    *(exe_symbol + exe_struct.dsc$w_length) = '\n';
    (void) fwrite(  (VOID *) exe_symbol,
                    (size_t) exe_struct.dsc$w_length + 1,
                    (size_t) 1,
                    fp);
    /* Define local program symbol */
    
    local_int = (int) sprintf(cmd_line, "$ server dmqa_o %u %d \"%s\" %d\n",
        port, DMQA__debug, DMQA__efc_name, efn);

    (void) fwrite(  (VOID *) cmd_line, (size_t) local_int, (size_t) 1, fp);
    /* execute server */
    
    (void) fwrite(  (VOID *) "$ LOGOUT\n",
                    (size_t) sizeof("$ LOGOUT\n") - 1,
                    (size_t) 1,
                    fp);
    /* verify */
    
    if ((local_int = fclose(fp)) != 0)
    {
        if (DMQA__debug)
            printf("%s ?fclose() returned error - exiting!\n",
                DMQA__DBG_PREFIX);
                
        return (long) PAMS__INTERNAL;
    }

    local_int = (int) sprintf( process_name, "%s%d%c",
	"DmQA_O_", DMQA__port, bit + 65);

    process_struct.dsc$w_length     = (unsigned short) local_int;
    process_struct.dsc$a_pointer    = process_name;
    process_struct.dsc$b_dtype      = DSC$K_DTYPE_T;
    process_struct.dsc$b_class      = DSC$K_CLASS_S;

    efn_mask                    = DMQA_power(efn - 64L) |
				  DMQA_power(DMQA__TIMER_EFN - 64L);
    
    (void) sys$cantim();
    
    if ((local_status = sys$setimr( DMQA__TIMER_EFN,
                                    quadword,
                                    0, 0L, 0L)) != SS$_NORMAL)
    {
        DMQA__errno = local_status;
        
        if (DMQA__debug)
            printf("%s SYS$SETIMR returned %d - exiting\n",
                DMQA__DBG_PREFIX);

        return (long) PAMS__INTERNAL;
    }
    
    printf("Creating offspring process %s, port %u, EFN %d\n",
        process_name, port, efn);

    local_status = sys$creprc(	&local_ulong,           /* for debugging */
				&lio_struct,            /* LOGINOUT      */
				&file_struct,		/* Our tmp file  */
				&log_struct,		/* Logfile       */
				0, 0, 0,		/* N/A		 */
				&process_struct,	/* Process name  */
				4L,			/* Priority      */
				0, 0,			/* N/A		 */
				stsflg);		/* DETACH        */
				
    if (DMQA__debug)
	printf("%s Waiting for event (mask=%u)...\n",
		DMQA__DBG_PREFIX, efn_mask);
    (void) sys$wflor(efn, efn_mask);

    (void) sys$cantim();
    (void) sys$readef(DMQA__TIMER_EFN, &DMQA__efn_map);
    
    if (DMQA__efn_map & DMQA_power(DMQA__TIMER_EFN - 64L))
    {
	printf("TIMEOUT - offspring creation failed for port %u, efn %d\n",
	    port, efn);

	(void) sys$setef(efn);		    /* lockout this slot */
	printf("          This offspring slot is now locked out\n");

	if (DMQA__debug) DMQA__DBG_EXIT(PAMS__NOLINK);
                
        return (long) PAMS__NOLINK;
    }
    
    if (DMQA__debug) DMQA__DBG_EXIT(PAMS__SUCCESS);
    return (long) PAMS__SUCCESS;
}    


/****************************************************************************/
/* Module:    DMQA_power						    */
/*                                                                          */
/* Function: Internal routine to return powers of 2 for EFN processing.	    */
/*                                                                          */
/****************************************************************************/

static long	DMQA_power(value)

long value;
{
/*    int	    local_index	= 0; */
/*    long    local_long	= 1; */

/*    while (local_index++ < value) */
/*	local_long *= 2;	    */

/*     return local_long; */

	return (long) (1 << value);
}


/****************************************************************************/
/* Module:    DMQA_get_symbol						    */
/*                                                                          */
/* Function: Internal routine used if we are the DECmessageQ Server. Sets   */
/*	     up variables by reading their values from DCL symbols.	    */
/*                                                                          */
/****************************************************************************/


static long DMQA_get_symbol(struct_ptr, buffer_ptr, buffer_len, symbol,
			    symbol_len)

struct dsc$descriptor_s *struct_ptr;
char                    *buffer_ptr;
char                    *symbol;
int			buffer_len;
int			symbol_len;
{
    unsigned short          item_len;
    struct dsc$descriptor_s local_struct;
    unsigned long           lib_status;
        
    extern unsigned long    lib$get_symbol();
    

    if (DMQA__debug)
        printf("%s Getting symbol %s...\n", DMQA__DBG_PREFIX, symbol);
            
    struct_ptr -> dsc$w_length   = (unsigned short) buffer_len;
    struct_ptr -> dsc$b_dtype    = DSC$K_DTYPE_T;
    struct_ptr -> dsc$b_class    = DSC$K_CLASS_S;
    struct_ptr -> dsc$a_pointer  = buffer_ptr;
            
    local_struct.dsc$w_length   = (unsigned short) symbol_len;
    local_struct.dsc$b_dtype    = DSC$K_DTYPE_T;
    local_struct.dsc$b_class    = DSC$K_CLASS_S;
    local_struct.dsc$a_pointer  = symbol;
            

    if ((lib_status = lib$get_symbol(   &local_struct,
                                        struct_ptr,
                                        &item_len)) != SS$_NORMAL)
    {
        DMQA__errno = (long) lib_status;
        if (DMQA__debug)
            printf("%s ?LIB$GET_SYMBOL returned %u\n",
                DMQA__DBG_PREFIX, lib_status);
        return (long) PAMS__INTERNAL;
    }

    struct_ptr -> dsc$w_length   = item_len;
    *(buffer_ptr + item_len) = 0;

    if (DMQA__debug)
        printf("%s Symbol returned %s\n",
            DMQA__DBG_PREFIX, buffer_ptr);

    return (long) PAMS__SUCCESS;
}

#endif
#endif
