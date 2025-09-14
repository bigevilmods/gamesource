// (c) 2003-2007 Electronic Arts Inc.

#ifndef __FPSNR_H
#define __FPSNR_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Psnr Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT *aboutpsnr(void);

FPUBLIC int     openpsnr(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infopsnr(FINSTANCE *fx, int framenum);
FPUBLIC int     filterpsnr(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closepsnr(FINSTANCE *fx);

void PSNR_save(double s[6]);
void PSNR_restore(double s[6]);

// return values
extern double PSNRpsnr;
extern double PSNRpsnrtotal;
extern double PSNRpsnrY;
extern double PSNRpsnrYtotal;
extern double PSNRpsnrMY;
extern double PSNRpsnrMYtotal;

// accumulators for multiple images - zero to reset
extern double PSNRpsnrtotalpixels;
extern double PSNRpsnrtotalsum;
extern double PSNRpsnrYtotalpixels;
extern double PSNRpsnrYtotalsum;
extern double PSNRpsnrMYtotalpixels;
extern double PSNRpsnrMYtotalsum;

#ifdef __cplusplus
}
#endif
#endif

