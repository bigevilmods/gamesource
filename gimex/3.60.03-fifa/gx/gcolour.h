// (c) 1994-2004 Electronic Arts Inc.

#ifndef __GCOLOURH
#define __GCOLOURH 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"

#if CRAPI_VERSION < 150
#error gx requires crapi 1.50 or later
#endif

/****************************************************************/
/*  Colour Remapping Functions                                  */
/****************************************************************/

extern ARGB *gfindpal;
extern int gmaxcolours;
extern CRINVERSETABLE *ginversetbl;

void gxfreeinversetable(void);
int gxgetcolour(ARGB *rgbval);
void gxinitinversetable(ARGB *pal);

GINFO *info8to32(GINFO *gin, char *source, int rowbytes, int bpp);
int filter8to32(GINFO *gin, char *source, int rowbytes, GINFO *ginfo, char *dest, int destrowbytes, int bpp);

GINFO *infodither32(GINFO *gin, char *source, int rowbytes, int alphabits, int redbits, int greenbits, int bluebits);
int filterdither32(GINFO *gin, char *source, int rowbytes, GINFO *ginfo, char *dest, int destrowbytes, int dither);

GINFO *info32to8(GINFO *gin, int numcolors, char *source, int rowbytes, int bpp, int pass,int OPTglobalpalette);
int filter32to8(GINFO *gin, char *source, int rowbytes, GINFO *ginfo, char *dest, int destrowbytes, int bpp, unsigned char *colourmask);

GINFO *infohflip(GINFO *gin, char *source, int rowbytes);
int filterhflip(GINFO *gin, char *source, int rowbytes, GINFO *ginfo, char *dest, int destrowbytes);

void CR_reset(void);

/****************************************************************/
/*  NEW GColour Functions                                       */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif

int imagetransparent(GBITMAP *gbitmap, ARGB *chromacolour, int *chromaflag, int *transparentcount);
void indexhistogram(GBITMAP *gbitmap, int *indexhistogram);

#ifdef __cplusplus
}
#endif
#endif

