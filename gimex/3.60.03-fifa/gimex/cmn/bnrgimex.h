// (c) 1994-2007 Electronic Arts Inc.

#ifndef __BNRGIMEX
#define __BNRGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  BNR Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL BNR_about(void);
int      GCALL BNR_is(GSTREAM *g);

/* Import Functions */

int      GCALL BNR_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL BNR_info(GINSTANCE *gx, int framenum);
int      GCALL BNR_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL BNR_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL BNR_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL BNR_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL BNR_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#define DVD_BANNER_WIDTH  96
#define DVD_BANNER_HEIGHT 32

typedef struct
{
    /* BANNER FILE structure 6496 bytes */

    char id[4];                 /* 'BNR1' */
    char padding[7*4];

    char image[2 * DVD_BANNER_WIDTH * DVD_BANNER_HEIGHT]; /* 5a3 image */
    char shortTitle[32];        /* Short game title shown in IPL menu          */
    char shortMaker[32];        /* developer names shown in IPL menu           */
    char longTitle[64];         /* game title shown in IPL game start screen   */
    char longMaker[64];         /* developer name shown in IPL start screen    */
    char comment[128];          /* Game description shown in IPL game start    */
                                /* in two lines. Comment can include a '\n'.   */
} BNRHeaderStruct;

#define BNRGETSIGNEDFIELD(field) ((int)(ggeti((void *)&field, sizeof(field))<<(32-sizeof(field)*8))>>(32-sizeof(field)*8))

#define BNRGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define BNRPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

