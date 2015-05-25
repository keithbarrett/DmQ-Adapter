/*  Module dmqa_server.c DmQA V1.0        DmQA Server			    */
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
/*  This program is the Server end to the DECmessageQ Queue Adapter	    */
/*  Although it has conditional compiles for VMS and non-VMS systems,	    */
/*  it currently will only compile and function correctly under VMS.	    */
/*									    */
/*  This version only support a single TCP/IP connection; it does not	    */
/*  support the creation of offspring. It is used for online debugging	    */
/*  of programs with the DECmessageQ Queue Adapter. It is only a simple	    */
/*  main() routine for calling the functions in the DMQA_IPI.C file.	    */
/*									    */
/*									    */
/*  IDENT HISTORY:							    */
/*									    */
/*  Version     Date    Who     Description				    */
/*									    */
/*  V1.0FT  08-Mar-1991 kgb     Initial Release; Keith Barrett		    */
/*  V.10FT  01-May-1991	kgb	Minor changes for offspring support	    */
/*  V1.0FT  07-May-1991	kgb	Added DMQA__process_args call		    */
/*  V1.0FT  22-May-1991	kgb	Added use of DMQA__force_bracket	    */
/*									    */
/*									    */
/*									    */


        /* Include standard "C" and system definition files. */

#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef VMS
#include <types.h>
#pragma nostandard
#include pams_c_return_status_def
#include pams_c_entry_point
#pragma standard
#else
#include <sys/types.h>
#include "p_entry.h"
#include "p_return.h"
#endif

#include    "p_dmqa.h"          /* DmQA defintions */

#ifdef __USE_ARGS
extern  long		DMQA__link_create(long);
extern  long		DMQA__link_listen(long);
extern  long		DMQA__link_accept(long);
extern  long		DMQA__link_reject(long);
extern  void		DMQA__process_client();
extern  int		atoi(char *);
extern	void		DMQA__link_shutdown(int, long);
#else
extern  long		DMQA__link_create();
extern  long		DMQA__link_listen();
extern  long		DMQA__link_accept();
extern  long		DMQA__link_reject();
extern  void		DMQA__process_client();
extern  int		atoi();
extern	void		DMQA__link_shutdown();
#endif

extern	void		DMQA__process_args();


main(argc,argv)

int	argc;
char	**argv;
{
    long    local_status;
    int	    loop_flag = TRUE;


    if (argc < 3)
        printf("%Warning -- no program parameters specified\n");
    /* Check input parameters */

    DMQA__argv = argv;
    DMQA__argc = argc;

    DMQA__process_args();
    /* Process command line */

    printf("\nCreating link...\n");

    if ((local_status = DMQA__link_create(0L)) != PAMS__SUCCESS)
    {
	printf("?DMQA__link_create failed with error %d, errno %d\n",
	    local_status, DMQA__errno);
	exit(local_status);
    }
    
    if (DMQA__offspring_flag)
	printf("This server is an offspring process\n");
#ifdef VMS
#ifdef DMQA__OFFSPRING
    else
#if DMQA__OFFSPRING
	printf("\nThis server supports offspring creation\n");
#else
	printf("\n(This server does not support offspring creation)\n");
#endif
#endif
#endif



    /************************************************************/
    /*			    MAIN LOOP				*/
    /*  Wait for a connection. When received, complete the	*/
    /*  connection and wait for messages.			*/
    /************************************************************/

    printf("\nAwaiting client connection...\n\n");

    while (loop_flag)
    {
	if ((local_status = DMQA__link_listen(0L)) != PAMS__SUCCESS)
	{
	    printf("?DMQA__link_listen returned error %d, errno %d\n",
		local_status, DMQA__errno);
	    exit(local_status);
	}

	if ((local_status = DMQA__link_accept(0L)) != PAMS__SUCCESS)
	{
	    if (DMQA__debug)
		printf("%s ?DMQA__link_accept returned error %d, errno %d\n",
		    DMQA__DBG_PREFIX, local_status, errno);
	}
	else
	{
	    if (DMQA__debug)
		printf("%s Connected to client\n", DMQA__DBG_PREFIX);

	    DMQA__process_client();

	    if (DMQA__debug)
		printf("%s Disconnecting from client\n", DMQA__DBG_PREFIX);

	    DMQA__link_shutdown(0, 0L);

	    if (DMQA__debug)
		printf("%s Disconnecting from PAMS bus (pams_exit)\n",
		    DMQA__DBG_PREFIX);

	    local_status = pams_exit();

	    if (DMQA__offspring_flag)
	    {
		if (DMQA__debug)
			printf("%s Offspring process - terminating link\n",
				DMQA__DBG_PREFIX);
 		DMQA__link_shutdown(-1, 0L);

		loop_flag = FALSE;
 	    }
	}
    }

    printf("Exiting...\n");

}

