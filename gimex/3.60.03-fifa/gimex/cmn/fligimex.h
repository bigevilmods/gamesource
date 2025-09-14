// (c) 1994-2007 Electronic Arts Inc.

#ifndef __FLIGIMEX
#define __FLIGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  FLI Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL FLI_about(void);
int      GCALL FLI_is(GSTREAM *g);

/* Import Functions */

int      GCALL FLI_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL FLI_info(GINSTANCE *gx, int framenum);
int      GCALL FLI_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL FLI_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL FLI_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL FLI_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL FLI_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

typedef struct
{
    /* FLI FILE HEADER structure 128 bytes */

    char size[4];               /* Size of flic including this header. */
    char type[2];               /* Either FLI_TYPE or FLC_TYPE below. */
    char frames[2];             /* Number of frames in flic. */
    char width[2];              /* Flic width in pixels. */
    char height[2];             /* Flic height in pixels. */
    char depth[2];              /* Bits per pixel.  (Always 8 now.) */
    char flags[2];              /* FLI_FINISHED | FLI_LOOPED ideally. */
    char speed[4];              /* Delay between frames. */
    char reserved1[2];          /* Set to zero. */
    char created[4];            /* Date of flic creation. (FLC only.) */
    char creator[4];            /* Serial # of flic creator. (FLC only.) */
    char updated[4];            /* Date of flic update. (FLC only.) */
    char updater[4];            /* Serial # of flic updater. (FLC only.) */
    char aspect_dx[2];          /* Width of square rectangle. (FLC only.) */
    char aspect_dy[2];          /* Height of square rectangle. (FLC only.) */
    char reserved2[38];         /* Set to zero. */
    char oframe1[4];            /* Offset to frame 1. (FLC only.) */
    char oframe2[4];            /* Offset to frame 2. (FLC only.) */
    char reserved3[40];         /* Set to zero. */

} FLIHeaderStruct;

/* FLI context */

typedef struct
{
    FLIHeaderStruct header;     /* raw flic header */
    int  width;                 /* image width in pixels */
    int  height;                /* image height in pixels */
    int  packed;                /* 0=no,keyframe, 1=motion */
    int  colours;               /* number of colours in palette */
    ARGB colourtbl[256];        /* palette */
    char *current;              /* current frame pointer */

} FLIINFO;

#define READBYTE() ggeti(chunk,1); chunk+=1
#define READWORD() ggeti(chunk,2); chunk+=2

#define FLIGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define FLIPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

