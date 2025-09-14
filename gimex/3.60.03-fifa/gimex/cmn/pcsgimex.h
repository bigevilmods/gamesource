// (c) 1994-2007 Electronic Arts Inc.

#ifndef __pcsGIMEX
#define __pcsGIMEX 1

#include "gimex.h"
#include "Resources.h"
#include "PictUtil.h"
#include "boff.h"

/****************************************************************/
/*  pcs globals                                                 */
/****************************************************************/

typedef struct {
    short           BWColor;
    short           MaxDepth;
    short           Speed;
    short           Version;
    OSType          Creator;
    unsigned int   MaxSize;
} PicsInfo, *PicsInfoPtr, **PicsInfoHdle;

typedef struct PicsData
{
    int    MaxSize;
    int    MaxDepth;
} PicsData,*PicsDataPtr;

/****************************************************************/
/*  pcs Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT* PCS_about(void);
int         PCS_is(GSTREAM *g);

/* Import Functions */

int         PCS_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO*      PCS_info(GINSTANCE *gx, int framenum);
int         PCS_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int         PCS_close(GINSTANCE *gx);

/* Export Functions */

int         PCS_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int         PCS_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int         PCS_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Com function                                                */
/****************************************************************/

CTabHandle GetGInfoPalette(GINFO *theGINFO);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#define PICSSIGN    GMAKEID('P','I','C','S')
#define PICSID      GMAKEID('p','i','c','s')

#endif
