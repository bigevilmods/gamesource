// (c) 1994-2007 Electronic Arts Inc.

#ifndef __FFSGIMEX
#define __FFSGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  FFS Raw Gimex                                               */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL FFS_about(void);
int      GCALL FFS_is(GSTREAM *g);

/* Import Functions */

int      GCALL FFS_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL FFS_info(GINSTANCE *gx, int framenum);
int      GCALL FFS_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL FFS_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL FFS_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL FFS_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL FFS_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#define FFSSIGNATURE GMAKEID('F','N','T','T')

typedef struct FFSINFO
{
    // public
    unsigned short fontname[50];            // font family name
    float pointsize;    // set this to adjust point size per glyph
    unsigned int unicode; // if non-zero, use this unicode value to specify the character instead of framenum (for FFS_info)
    int flags;          // 1=monochrome (not antialiased), 2=4 bit antialiased. default is 8 bit

    int style;          // 0=regular,1=bold,2=italic,3=bold/italic.  default based on font supplied in open
    int charset;        // map charset from logfont ie hebrew, greek...
    int ascii;          // 1=force ascii mapping instead of unicode
    float aspect;       // aspect ratio. 1=1:1.  0.5 = half height
    float tighten;      // tighten horizontal spacing by this scale factor (1=none)
    float smallcaps;    // make lowercase into uppercase with this size factor (.8 typical). 0 disables
    float tracking;     // tighten horizontal spacing by this translation factor after scaling (0=none)
    int baseline;       // 0=baseline,1=upperleft

	// private
    int pad[12];        // reserved for future use
    unsigned short pathname[512];   // filename of font (needed to add/remove resource)
    unsigned short frametounicode[65536];   // hash frame to unicode

} FFSINFO;

#ifdef __cplusplus
}
#endif
#endif

