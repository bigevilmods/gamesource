// (c) 1998-2007 Electronic Arts Inc.
#ifndef MADENC_H
#define MADENC_H

#ifdef __cplusplus
extern "C" {
#endif


#include "gimex.h"


typedef struct
{
    int     maxframesize;
    int     maxgroupsize;
    int     groupframes;
    int     extraflag;
    int     motionthreshold;
} ENCODEPARAMS;

typedef struct
{
    char            id[4];
    unsigned int	size;
    unsigned char   reserved[4];	// used to be CRC
    unsigned int	time;
    unsigned short  width;
    unsigned short  height;
    unsigned char   ver; // 0 = original , 1 = 601 style YUV coeficients . 2 = flags present
    unsigned char   quality;
    unsigned char   flags; // 1 = keyframe , 2 = extra frame
	unsigned char   reserved2[1];
} CHUNKHDR;


typedef void (*MADCALLBACKFN) (CHUNKHDR *hdr);

extern MADCALLBACKFN madcallback;

int madinitencode(int width, int height, ENCODEPARAMS *p, GSTREAM *dest);
int madencode(ARGB *rgbframe, int rowbytes, int period);
int madendencode(void);

#ifdef __cplusplus
}
#endif
#endif

