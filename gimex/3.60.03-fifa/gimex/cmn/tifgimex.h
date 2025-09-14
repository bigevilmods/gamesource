// (c) 1994-2007 Electronic Arts Inc.

#ifndef __TIFGIMEX
#define __TIFGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  TIF Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL TIF_about(void);
int      GCALL TIF_is(GSTREAM *g);

/* Import Functions */

int      GCALL TIF_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL TIF_info(GINSTANCE *gx, int framenum);
int      GCALL TIF_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL TIF_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL TIF_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL TIF_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL TIF_wclose(GINSTANCE *gx);

#define TIFGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define TIFPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

