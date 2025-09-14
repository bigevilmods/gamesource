// (c) 1998-2007 Electronic Arts Inc.
#ifndef MADLAB_H
#define MADLAB_H

#ifdef __cplusplus
extern "C" {
#endif


#include "gimex.h"


typedef struct
{
    short       width;
    short       height;
    signed char *data;
}
FRAMETYPE;

typedef struct
{
    unsigned char   flags;
    signed char     vectorx;
    signed char     vectory;
    signed char     dccorrect[6];
}
MOTIONINFOTYPE;


int  madallocateframe(FRAMETYPE *frame, int width, int height);
void madreleaseframe(FRAMETYPE *frame);
void madcopyframe(FRAMETYPE *src, FRAMETYPE *dest);

void madrgbtoyuv(ARGB *src, int rowbytes, FRAMETYPE *dest);
void madyuvtorgb(FRAMETYPE *src, ARGB *dest, int rowbytes);

void madinitdct(void);
void madsetquality(int quality);
int  madcompressframe(FRAMETYPE *src, unsigned short *dest, int limit);
void maduncompressframe(unsigned short *src, FRAMETYPE *dest);

void madinitmotion(void);
void madsetmotionthreshold(int threshold);
int  madanalyzemotion(FRAMETYPE *src, FRAMETYPE *ref, MOTIONINFOTYPE *motion);
void madapplymotion(FRAMETYPE *src, FRAMETYPE *dest, MOTIONINFOTYPE *motion);

void madsizeframe(unsigned short *rledata, MOTIONINFOTYPE *motiondata, int *key, int *motion);
int  madencodeframe(unsigned short *rledata, MOTIONINFOTYPE *motiondata, unsigned char *dest);


#ifdef __cplusplus
}
#endif
#endif

