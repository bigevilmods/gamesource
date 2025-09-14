// (c) 1994-2007 Electronic Arts Inc.

#ifndef __VP6GIMEX
#define __VP6GIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  VP6 Raw Gimex                                               */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL VP6_about(void);
int      GCALL VP6_is(GSTREAM *g);

/* Import Functions */

int      GCALL VP6_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL VP6_info(GINSTANCE *gx, int framenum);
int      GCALL VP6_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL VP6_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL VP6_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL VP6_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL VP6_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

struct ChunkedAviHeader
{
  char   mId[4];                    // 'MVhd' FourCC
  char   mLength[4];                // Length of 'MVhd' chunk
  char   mCompressor[4];            // Compressor FourCC 
  char   mWidth[2];
  char   mHeight[2];
  char   mTotalFrames[4];           // Total number of frames in the file. This should always be accurate
  char   mSuggestedBufferSize[4];   // Recommended decode buffer size 
  char   mFpsNumerator[4];          // FPS = Numerator/Denominator
  char   mFpsDenominator[2];        // Eg. 30000/1001=29.97
  char   mFlags[2];                 // Eg. uses B-frames
};

#define VP6GETFIELD(field) ggeti((void *)&field, sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

