// (c) 1999-2007 Electronic Arts Inc.
/*
 * Columbia, Canada.  All rights reserved.
 *
 * The copyright holders acknowledge and thank Michael David Adams, the
 * original author of the Image Power JPEG-2000 JasPer Software (the
 * "JasPer Software"), for his immense contribution.
 *
 * The complete licensing terms for this software can be found in the file
 * named "LICENSE" in this software distribution.
 */

/*
 * Sun Rasterfile Library
 *
 * $Id: rasdec.h,v 1.3 2000/12/01 07:51:44 mdadams Exp $
 *
 * $Log: rasdec.h,v $
 * Revision 1.3  2000/12/01 07:51:44  mdadams
 * Added new copyright notice.
 *
 * Revision 1.2  2000/12/01 05:16:50  mdadams
 * *** empty log message ***
 *
 * Revision 1.1  2000/10/02 23:24:53  mdadams
 * Initial revision.
 */

/******************************************************************************\
* Sun Rasterfile
\******************************************************************************/

#ifndef	RASDEC_H
#define	RASDEC_H

/******************************************************************************\
* Includes.
\******************************************************************************/

#include "rascod.h"

/******************************************************************************\
* Functions.
\******************************************************************************/

image_t *image_import_ras(stream_t *in, void *opts);
int ras_validate(stream_t *in);

#endif
