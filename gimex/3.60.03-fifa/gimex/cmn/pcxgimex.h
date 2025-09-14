// (c) 1994-2007 Electronic Arts Inc.

#ifndef __PCXGIMEX
#define __PCXGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  PCX Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL PCX_about(void);
int      GCALL PCX_is(GSTREAM *g);

/* Import Functions */

int      GCALL PCX_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL PCX_info(GINSTANCE *gx, int framenum);
int      GCALL PCX_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL PCX_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL PCX_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL PCX_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL PCX_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

typedef struct
{
    /* PCX FILE HEADER structure 128 bytes */
    char id[1];                 /* 0x0a for PCX */
    char version[1];            /* 0..5 */
    char encoding[1];           /* 0 none, 1 rle */
    char bpp[1];                /* bits per pixel per plane */
    char xmin[2];               /* upper left x coordinate */
    char ymin[2];               /* upper left y coordinate */
    char xmax[2];               /* lower right x coordinate */
    char ymax[2];               /* lower right y coordinate */
    char hdotpinch[2];          /* horizontal dots per inch when printed */
    char vdotpinch[2];          /* vertical dots per inch when printed */
    char headerpal[16][3];      /* header palette for up to 16 colours */
    char unused[1];             /* unused */
    char planes[1];             /* number of planes 1 or 3 */
    char bpl[2];                /* bytes per line per plane */
    char palinterp[2];          /* 1=colour or b&w, 2=grey scale */
    char videosizex[2];         /* number of horizontal pixels-1 */
    char videosizey[2];         /* number of vertical pixels-1 */
    char pad[54];               /* blanks to end of header (128 byte header) */
} PCXHeaderStruct;


#define PCXGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define PCXPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

