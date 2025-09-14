// (c) 1994-2007 Electronic Arts Inc.

#ifndef __JPGGIMEX
#define __JPGGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  JPG Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL JPG_about(void);
int      GCALL JPG_is(GSTREAM *g);

/* Import Functions */

int      GCALL JPG_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL JPG_info(GINSTANCE *gx, int framenum);
int      GCALL JPG_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL JPG_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL JPG_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL JPG_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL JPG_wclose(GINSTANCE *gx);


/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#define JPGARGB  0x1
#define JPGNODHT 0x2  /* Whether the file contains (read) or should contain (write) the DHT. */

#define JPGGIMEXMARKERNUMBER JPEG_APP0+13
#define JPGGIMEXMARKER "GIMEXARGB"
#define JPGGIMEXMARKERSIZE 9

#define JPGGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define JPGPUTFIELD(field,data) gputi((void *)&field,(unsigned long)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

