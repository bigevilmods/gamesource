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
 * $Id: rasenc.c,v 1.3 2000/12/01 07:51:45 mdadams Exp $
 *
 * $Log: rasenc.c,v $
 * Revision 1.3  2000/12/01 07:51:45  mdadams
 * Added new copyright notice.
 *
 * Revision 1.2  2000/12/01 05:16:50  mdadams
 * *** empty log message ***
 *
 * Revision 1.1  2000/10/02 23:24:53  mdadams
 * Initial revision.
 */

/******************************************************************************\
* Version information.
\******************************************************************************/

static char rcsid[] = "$Id: rasenc.c,v 1.3 2000/12/01 07:51:45 mdadams Exp $";

/******************************************************************************\
* Includes.
\******************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include "rasenc.h"
#include "memalloc.h"

/******************************************************************************\
* Prototypes.
\******************************************************************************/

static int ras_puthdr(stream_t *out, ras_hdr_t *hdr);
static int ras_putint(stream_t *out, int val);

static int ras_putdata(stream_t *out, ras_hdr_t *hdr, image_t *image);
static int ras_putdatastd(stream_t *out, ras_hdr_t *hdr, image_t *image);

/******************************************************************************\
* Code.
\******************************************************************************/

int image_export_ras(image_t *image, stream_t *out, void *gopts)
{
	ras_exportopts_t *opts;
	int i;
	ras_hdr_t hdr;
	int rowsize;

	opts = gopts;

	/* Ensure that the image can be encoded in the desired format. */
	if (image_numcmpnts(image) == 3) {
#if 0
		/* The color model must be RGB. */
		if (image_colormodel(image) != CM_RGB) {
			return -1;
		}
#endif
		/* All three components must have the same subsampling
		factor and a precision of 8 bits. */
		if (image_numcmpnts(image) > 1) {
			for (i = 0; i < image_numcmpnts(image); ++i) {
				if (image_cmpnthsubsamp(image, i) != 1 ||
				  image_cmpntvsubsamp(image, i) != 1 ||
				  image_cmpntprec(image, i) != 8) {
					return -1;
				}
			}
		}
	} else if (image_numcmpnts(image) == 1) {
		/* NOP */
	} else {
		return -1;
	}

	hdr.magic = RAS_MAGIC;
	hdr.width = image_width(image);
	hdr.height = image_height(image);
	hdr.depth = (image_numcmpnts(image) == 3) ? 24 :
	  image_cmpntprec(image, 0);

	rowsize = RAS_ROWSIZE(&hdr);
	hdr.length = rowsize * hdr.height;
	hdr.type = RAS_TYPE_STD;

	hdr.maptype = RAS_MT_NONE;
	hdr.maplength = 0;

	if (ras_puthdr(out, &hdr)) {
		return -1;
	}

	if (ras_putdata(out, &hdr, image)) {
		return -1;
	}

	return 0;
}

static int ras_putdata(stream_t *out, ras_hdr_t *hdr, image_t *image)
{
	int ret;

	switch (hdr->type) {
	case RAS_TYPE_STD:
		ret = ras_putdatastd(out, hdr, image);
		break;
	default:
		ret = -1;
		break;
	}
	return ret;
}

static int ras_putdatastd(stream_t *out, ras_hdr_t *hdr, image_t *image)
{
	int rowsize;
	int pad;
	unsigned int z;
	int nz;
	int c;
	int x;
	int y;
	int v;
	matrix_t *data[3];
	int i;

	for (i = 0; i < image_numcmpnts(image); ++i) {
		data[i] = matrix_create(image_height(image), image_width(image));
		assert(data[i]);
	}

	rowsize = RAS_ROWSIZE(hdr);
	pad = rowsize - (hdr->width * hdr->depth + 7) / 8;

	hdr->length = hdr->height * rowsize;

	for (y = 0; y < hdr->height; y++) {
		for (i = 0; i < image_numcmpnts(image); ++i) {
			image_readcmpnt(image, i, 0, y, image_width(image),
			  1, data[i]);
		}
		z = 0;
		nz = 0;
		for (x = 0; x < hdr->width; x++) {
			z <<= hdr->depth;
			if (RAS_ISRGB(hdr)) {
				v = RAS_RED((matrix_getv(data[0], x))) |
				  RAS_GREEN((matrix_getv(data[1], x))) |
				  RAS_BLUE((matrix_getv(data[2], x)));
			} else {
				v = (matrix_getv(data[0], x));
			}
			z |= v & RAS_ONES(hdr->depth);
			nz += hdr->depth;
			while (nz >= 8) {
				c = (z >> (nz - 8)) & 0xff;
				if (stream_putc(out, c) == EOF) {
					return -1;
				}
				nz -= 8;
				z &= RAS_ONES(nz);
			}
		}
		if (nz > 0) {
			c = (z >> (8 - nz)) & RAS_ONES(nz);
			if (stream_putc(out, c) == EOF) {
				return -1;
			}
		}
		if (pad % 2) {
			if (stream_putc(out, 0) == EOF) {
				return -1;
			}
		}
	}

	for (i = 0; i < image_numcmpnts(image); ++i) {
		matrix_destroy(data[i]);
	}

	return 0;
}

static int ras_puthdr(stream_t *out, ras_hdr_t *hdr)
{
	if (ras_putint(out, RAS_MAGIC) || ras_putint(out, hdr->width) ||
	  ras_putint(out, hdr->height) || ras_putint(out, hdr->depth) ||
	  ras_putint(out, hdr->length) || ras_putint(out, hdr->type) ||
	  ras_putint(out, hdr->maptype) || ras_putint(out, hdr->maplength)) {
		return -1;
	}

	return 0;
}

static int ras_putint(stream_t *out, int val)
{
	int x;
	int i;
	int c;

	x = val;
	for (i = 0; i < 4; i++) {
		c = (x >> 24) & 0xff;
		if (stream_putc(out, c) == EOF) {
			return -1;
		}
		x <<= 8;
	}

	return 0;
}
