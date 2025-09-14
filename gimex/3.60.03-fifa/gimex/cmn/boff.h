// (c) 1994-2007 Electronic Arts Inc.
/* Written by Gerry Shaw, based on code by Laurent Ancessi */

#ifndef __BOFF_H
#define __BOFF_H

/*#define DEBUG 1 */

#include <QDOffscreen.h>

/*------------------------------------------------------------------*/
/* boff data structure                                              */
/*------------------------------------------------------------------*/

typedef struct  boff_t
{
#if DEBUG
    int        fTag;           /* tag so it can be found using Macsbug (debug only) */
#endif
    GWorldPtr   fGWorld;        /* off screen bitmap */
    short       fLockLevel;     /* current number of locks on this boff */
    GDHandle    fGrafDevice;    /* old GDevice */
    CGrafPtr    fGrafArea;      /* old GrafPort */
} BOFF;

/*------------------------------------------------------------------*/
/* prototypes                                                       */
/*------------------------------------------------------------------*/

BOFF*      NewBOff(short width, short height, short depth, CTabHandle clut);
void       DisposeBOff(BOFF* boff);

void       StartToUseBOff(BOFF* boff);
void       StopToUseBOff(BOFF* boff);
void       ResetBOff(BOFF* boff);
void       SetBOff(BOFF* boff);

void       GetBOffBounds(BOFF* boff, Rect* bounds);
Ptr        GetBOffBasePtr(BOFF* boff, int x, int y);
int       GetBOffRowBytes(BOFF* boff);
CTabHandle GetBOffClut(BOFF* boff);
short      GetBOffPixelSize(BOFF* boff);
PicHandle  GetBOffPicHandle(BOFF* boff);

#endif

