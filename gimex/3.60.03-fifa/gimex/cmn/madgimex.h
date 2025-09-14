// (c) 1994-2007 Electronic Arts Inc.

#ifndef __MADGIMEX
#define __MADGIMEX 1

#include "gimex.h"
#include "idxgimex.h"
#include "maddec.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Mad Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL MAD_about(void);
int      GCALL MAD_is(GSTREAM *g);

/* Import Functions */

int      GCALL MAD_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL MAD_info(GINSTANCE *gx, int framenum);
int      GCALL MAD_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL MAD_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL MAD_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL MAD_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL MAD_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

struct MADAviIndex;

struct MADINFO
{
    /* Used by decoder */
    int lastframe;
    int lastpos;
    YUVPAIR *lastyuv;
    IDXTable *table;
    int maxframesize;
    int keycount;
    int extracount;
	int subtype;

    /* Used by encoder */
    int width;
    int height;
	struct MADAviIndex *MADAviIndex; // used for MAV
	float fps;
};

#define MADGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define MADPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))


#define SUBTYPE_MAV 0x01000000





#ifdef __cplusplus
}
#endif
#endif

