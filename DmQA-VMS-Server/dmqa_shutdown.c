/*  Module dmqa_shutdown.c	DmQA V1.0   Shutdown program to clear EFNs  */
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
/*  IDENT HISTORY:                                                          */
/*                                                                          */
/*  Version     Date    Who     Description                                 */
/*                                                                          */
/*  V1.0FT  15-May-1991	kgb	Initial Release; Keith Barrett		    */
/*                                                                          */



        /* Include standard "C" and system definition files. */

#include <errno.h>		/* Error number values */
#include <stdio.h>		/* Standard I/O definitions */
#include <stddef.h>
#include <string.h>		/* String manipulations and functions */
#include <stdlib.h>

#include <types.h>
#include <descrip.h>
#include <prcdef.h>
#include <ssdef.h>

char                        *DMQA__efc_name         = 0;    /* EF Cluster    */
long                        DMQA__efn               = 0;    /* Our EFN in it */

extern long sys$ascefc();
extern long sys$dacefc();
extern long sys$clref();
extern long sys$readef();


main (argc, argv)
int	argc;
char	**argv;
{
    struct dsc$descriptor_s	efc_struct;
    long			local_status;
    long			efn_status;
    long			efn_map;


    if ((argc < 3) || (argv == 0))
    {
	printf("\n?Usage:   program  EFC_name  EFN (as a letter)\n");
	exit(1);
    }

    if (++argv)	DMQA__efc_name = *argv;
    if (++argv) DMQA__efn = (long) (**argv) - 1L;

    if (DMQA__efc_name == 0)
    {
	printf("\n?No EFC name specified!\n");
	exit(1);
    }

    if (DMQA__efn < 64)
    {
	printf("\n?Improper EFN specified (%d)\n", DMQA__efn);
	exit(1);
    }

    efc_struct.dsc$w_length	= (unsigned short) strlen(DMQA__efc_name);
    efc_struct.dsc$b_dtype	= DSC$K_DTYPE_T;
    efc_struct.dsc$b_class	= DSC$K_CLASS_S;
    efc_struct.dsc$a_pointer	= DMQA__efc_name;

    if ((local_status = sys$ascefc(DMQA__efn, &efc_struct, 0, 0))
	== SS$_NORMAL)
    {
	efn_status = sys$readef(DMQA__efn, &efn_map);
	local_status = sys$clref(DMQA__efn);
	if (efn_status == SS$_WASSET)
	    printf("    %sEFN %d in %s cleared\n",
		"%", DMQA__efn, DMQA__efc_name);
	(void) sys$dacefc(DMQA__efn);
    }
}
