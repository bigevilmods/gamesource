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
 * $Id: rascod.h,v 1.3 2000/12/01 07:51:44 mdadams Exp $
 *
 * $Log: rascod.h,v $
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

#ifndef	RASCOD_H
#define	RASCOD_H

/******************************************************************************\
* Includes.
\******************************************************************************/

#include "stream.h"
#include "image.h"

/******************************************************************************\
* Constants.
\******************************************************************************/

#define	RAS_MAGIC	0x59a66a95

#define	RAS_TYPE_OLD	0
#define	RAS_TYPE_STD	1
#define	RAS_TYPE_RLE	2

#define	RAS_MT_NONE	0
#define	RAS_MT_EQUALRGB	1
#define	RAS_MT_RAW	2

/******************************************************************************\
* Types.
\******************************************************************************/

typedef struct {
	int magic;	/* signature */
	int width;	/* width of image (in pixels) */
	int height;	/* height of image (in pixels) */
	int depth;	/* number of bits per pixel */
	int length;	/* length of image data (in bytes) */
	int type;	/* format of image data */
	int maptype;	/* colormap type */
	int maplength;	/* length of colormap data (in bytes) */
} ras_hdr_t;

typedef struct {
	int len;
	int data[256];
} ras_cmap_t;

/******************************************************************************\
* Macros.
\******************************************************************************/

#define RAS_GETBLUE(x)	(((x) >> 16) & 0xff)
#define	RAS_GETGREEN(x)	(((x) >> 8) & 0xff)
#define RAS_GETRED(x)	((x) & 0xff)

#define RAS_BLUE(x)	(((x) & 0xff) << 16)
#define RAS_GREEN(x)	(((x) & 0xff) << 8)
#define	RAS_RED(x)	((x) & 0xff)

#define RAS_ROWSIZE(hdr) \
	((((hdr)->width * (hdr)->depth + 15) / 16) * 2)
#define RAS_ISRGB(hdr) \
	((hdr)->depth == 24 || (hdr)->depth == 32)

#define	RAS_ONES(n) \
	(((n) == 32) ? 0xffffffffU : ((1 << (n)) - 1))

#endif







