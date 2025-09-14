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
 * $Id: rasdec.c,v 1.3 2000/12/01 07:51:44 mdadams Exp $
 *
 * $Log: rasdec.c,v $
 * Revision 1.3  2000/12/01 07:51:44  mdadams
 * Added new copyright notice.
 *
 * Revision 1.2  2000/11/14 17:50:36  mdadams
 * *** empty log message ***
 *
 * Revision 1.1  2000/10/02 23:24:53  mdadams
 * Initial revision.
 */

/******************************************************************************\
* Version information.
\******************************************************************************/

static char rcsid[] = "$Id: rasdec.c,v 1.3 2000/12/01 07:51:44 mdadams Exp $";

/******************************************************************************\
* Includes.
\******************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include "rasdec.h"
#include "memalloc.h"

/******************************************************************************\
* Prototypes.
\******************************************************************************/

static int ras_gethdr(stream_t *in, ras_hdr_t *hdr);
static int ras_getint(stream_t *in, int *val);

static int ras_getdata(stream_t *in, ras_hdr_t *hdr, ras_cmap_t *cmap,
  image_t *image);
static int ras_getdatastd(stream_t *in, ras_hdr_t *hdr, ras_cmap_t *cmap,
  image_t *image);
static int ras_getcmap(stream_t *in, ras_hdr_t *hdr, ras_cmap_t *cmap);

/******************************************************************************\
* Code.
\******************************************************************************/

image_t *image_import_ras(stream_t *in, void *opts)
{
	ras_hdr_t hdr;
	ras_cmap_t cmap;
	image_t *image;
	cmpntparms_t cmpnts[3];
	int colormodel;
	int numcmpnts;
	int i;

	/* Read the header. */
	if (ras_gethdr(in, &hdr)) {
		return 0;
	}

	/* Does the header information look reasonably sane? */
	if (hdr.magic != RAS_MAGIC || hdr.width <= 0 || hdr.height <= 0 ||
	  hdr.depth <= 0 || hdr.depth > 32) {
		return 0;
	}

	/* In the case of the old format, do not rely on the length field
	being properly specified.  Calculate the quantity from scratch. */
	if (hdr.type == RAS_TYPE_OLD) {
		hdr.length = RAS_ROWSIZE(&hdr) * hdr.height;
	}

	/* Calculate some quantities needed for creation of the image
	object. */
	if (RAS_ISRGB(&hdr)) {
		colormodel = CM_RGB;
		numcmpnts = 3;
	} else {
		colormodel = CM_GRAY;
		numcmpnts = 1;
	}
	for (i = 0; i < numcmpnts; ++i) {
		cmpnts[i].width = hdr.width;
		cmpnts[i].height = hdr.height;
		cmpnts[i].prec = RAS_ISRGB(&hdr) ? 8 : hdr.depth;
		cmpnts[i].fmt = 0;
	}
	/* Create the image object. */
	if (!(image = image_create(numcmpnts, cmpnts, colormodel))) {
		return 0;
	}

	/* Read the color map (if there is one). */
	if (ras_getcmap(in, &hdr, &cmap)) {
		image_destroy(image);
		return 0;
	}

	/* Read the pixel data. */
	if (ras_getdata(in, &hdr, &cmap, image)) {
		image_destroy(image);
		return 0;
	}

	return image;
}

int ras_validate(stream_t *in)
{
	int x;
	/* Read the first four characters that constitute the signature. */
	ras_getint(in, &x);
	/* Put these characters back to the stream. */
	if (stream_ungetc(in, x & 0xff) == EOF ||
	  stream_ungetc(in, (x >> 8) & 0xff) == EOF ||
	  stream_ungetc(in, (x >> 16) & 0xff) == EOF ||
	  stream_ungetc(in, (x >> 24) & 0xff)) {
		return -1;
	}
	/* Is the signature correct for the Sun Rasterfile format? */
	if (x == RAS_MAGIC) {
		return 0;
	}
	return -1;
}

static int ras_getdata(stream_t *in, ras_hdr_t *hdr, ras_cmap_t *cmap,
  image_t *image)
{
	int ret;

	switch (hdr->type) {
	case RAS_TYPE_OLD:
	case RAS_TYPE_STD:
		ret = ras_getdatastd(in, hdr, cmap, image);
		break;
	case RAS_TYPE_RLE:
	default:
		ret = -1;
		break;
	}
	return ret;
}

static int ras_getdatastd(stream_t *in, ras_hdr_t *hdr, ras_cmap_t *cmap,
  image_t *image)
{
	int pad;
	int nz;
	int z;
	int c;
	int y;
	int x;
	int v;
	int i;
	matrix_t *data[3];

/* Note: This function does not properly handle images with a colormap. */

	for (i = 0; i < image_numcmpnts(image); ++i) {
		data[i] = matrix_create(1, image_width(image));
		assert(data[i]);
	}

	pad = RAS_ROWSIZE(hdr) - (hdr->width * hdr->depth + 7) / 8;

	for (y = 0; y < hdr->height; y++) {
		nz = 0;
		z = 0;
		for (x = 0; x < hdr->width; x++) {
			while (nz < hdr->depth) {
				if ((c = stream_getc(in)) == EOF) {
					return -1;
				}
				z = (z << 8) | c;
				nz += 8;
			}

			v = (z >> (nz - hdr->depth)) & RAS_ONES(hdr->depth);
			z &= RAS_ONES(nz - hdr->depth);
			nz -= hdr->depth;

			if (image_numcmpnts(image) == 3) {
				matrix_setv(data[0], x, (RAS_GETRED(v)));
				matrix_setv(data[1], x, (RAS_GETGREEN(v)));
				matrix_setv(data[2], x, (RAS_GETBLUE(v)));
			} else {
				matrix_setv(data[0], x, (v));
			}
		}
		if (pad) {
			if ((c = stream_getc(in)) == EOF) {
				return -1;
			}
		}
		for (i = 0; i < image_numcmpnts(image); ++i) {
			if (image_writecmpnt(image, i, 0, y, hdr->width, 1,
			  data[i])) {
				return -1;
			}
		}
	}

	for (i = 0; i < image_numcmpnts(image); ++i) {
		matrix_destroy(data[i]);
	}

	return 0;
}

static int ras_getcmap(stream_t *in, ras_hdr_t *hdr, ras_cmap_t *cmap)
{
	int i;
	int j;
	int x;
	int c;
	int numcolors;
	int actualnumcolors;

	switch (hdr->maptype) {
	case RAS_MT_NONE:
		break;
	case RAS_MT_EQUALRGB:
		{
fprintf(stderr, "warning: palettized images not fully supported\n");
		numcolors = 1 << hdr->depth;
		assert(numcolors <= sizeof(cmap->data));
		actualnumcolors = hdr->maplength / 3;
		for (i = 0; i < numcolors; i++) {
			cmap->data[i] = 0;
		}
		if ((hdr->maplength % 3) || hdr->maplength < 0 ||
		  hdr->maplength > 3 * numcolors) {
			return -1;
		}
		for (i = 0; i < 3; i++) {
			for (j = 0; j < actualnumcolors; j++) {
				if ((c = stream_getc(in)) == EOF) {
					return -1;
				}
				x = 0;
				switch (i) {
				case 0:
					x = RAS_RED(c);
					break;
				case 1:
					x = RAS_GREEN(c);
					break;
				case 2:
					x = RAS_BLUE(c);
					break;
				}
				cmap->data[j] |= x;
			}
		}
		}
		break;
	default:
		return -1;
		break;
	}

	return 0;
}

static int ras_gethdr(stream_t *in, ras_hdr_t *hdr)
{
	if (ras_getint(in, &hdr->magic) || ras_getint(in, &hdr->width) ||
	  ras_getint(in, &hdr->height) || ras_getint(in, &hdr->depth) ||
	  ras_getint(in, &hdr->length) || ras_getint(in, &hdr->type) ||
	  ras_getint(in, &hdr->maptype) || ras_getint(in, &hdr->maplength)) {
		return -1;
	}

	if (hdr->magic != RAS_MAGIC) {
		return -1;
	}

#if 0
	fprintf(stderr, "width = %d, height = %d, depth = %d\n", hdr->width,
	  hdr->height, hdr->depth);
#endif

	return 0;
}

static int ras_getint(stream_t *in, int *val)
{
	int x;
	int c;
	int i;

	x = 0;
	for (i = 0; i < 4; i++) {
		if ((c = stream_getc(in)) == EOF) {
			return -1;
		}
		x = (x << 8) | (c & 0xff);
	}

	*val = x;
	return 0;
}
