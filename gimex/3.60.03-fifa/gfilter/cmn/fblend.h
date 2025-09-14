// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FBLEND_H
#define __FBLEND_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Blend Filter                                                 */
/****************************************************************/

FPUBLIC FABOUT *aboutblend(void);

FPUBLIC int     openblend(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoblend(FINSTANCE *fx, int framenum);
FPUBLIC int     filterblend(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeblend(FINSTANCE *fx);

/* method */
#define BLEND_COMPOSITE 0
#define BLEND_PASTE     1
#define BLEND_SUB       2
#define BLEND_ADD       3
#define BLEND_REMOVE    4
#define BLEND_PSNR      5
#define BLEND_PREALPHA  6
#define BLEND_MULTIPLY  7
#define BLEND_SCREEN    8
#define BLEND_OVERLAY   9
#define BLEND_ALPHATEST 10
#define BLEND_ALPHA     11

/* palmethod */
#define BLEND_PALCOMBINE 0
#define BLEND_PALBACK    1
#define BLEND_PALFORE    2

// return values
extern double BLENDpsnr;
extern double BLENDpsnrtotal;
extern double BLENDpsnrY;
extern double BLENDpsnrYtotal;

// accumulators for multiple images - zero to reset
extern double BLENDpsnrtotalpixels;
extern double BLENDpsnrtotalsum;
extern double BLENDpsnrYtotalpixels;
extern double BLENDpsnrYtotalsum;

#ifdef __cplusplus
}
#endif
#endif

