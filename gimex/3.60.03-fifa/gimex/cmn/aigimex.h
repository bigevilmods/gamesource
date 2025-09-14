// (c) 1994-2007 Electronic Arts Inc.

#ifndef __AIGIMEX
#define __AIGIMEX 1

#include <stdio.h>
#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  AI Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL AI_about(void);
int      GCALL AI_is(GSTREAM *g);

/* Import Functions */

int      GCALL AI_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL AI_info(GINSTANCE *gx, int framenum);
int      GCALL AI_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL AI_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL AI_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL AI_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL AI_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#define NAMELEN  80

typedef struct document PSDOC;

typedef struct
{
    int  layer;
    char name[NAMELEN+1];
    int begin;
    int end;

} AILAYER;


typedef struct
{
    int     numlayers;
    AILAYER *layerdata;
    PSDOC   *doc;
    FILE    *file;
    void   *handle;

} AIINFO;


#define AIGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define AIPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

