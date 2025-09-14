// (c) 1994-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                    Offscreen Bitmap module                       */
/*                                                                  */
/*                       by Gerry Shaw, EAC                         */
/*                 based on code by Laurent Ancessi                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940725 GS Initial macintosh version                      */
/*                                                                  */
/*------------------------------------------------------------------*/

#include "BOff.h"

#if DEBUG
    #define Assert(c, s)    if (!(c)) DebugStr(s)
#else
    #define Assert(c, s)
#endif


/*------------------------------------------------------------------*/
/* public functions                                                 */
/*------------------------------------------------------------------*/

BOFF* NewBOff(short width, short height, short depth, CTabHandle clut)
{
    BOFF* boff;

    Assert(width  > 0, "bad width");
    Assert(height > 0, "bad width");
    Assert(depth == 1 || depth == 2 || depth == 4 || depth == 8 ||
        depth == 16 || depth == 32, "depth must be 1, 2, 4, 8, 16 or 32 bpp");

    boff = (BOFF*)NewPtr(sizeof(BOFF));
    if (boff)
    {
        OSErr err;
        Rect bounds = {0, 0, 0, 0};
        bounds.bottom = height;
        bounds.right = width;

        #if DEBUG
            boff->fTag = 'BOFF';
        #endif

        boff->fLockLevel  = 0;
        boff->fGrafArea   = NULL;
        boff->fGrafDevice = NULL;

        err = NewGWorld(&(boff->fGWorld), depth, &bounds, clut, NULL, 0);
        if (err)
        {
            DisposePtr((Ptr)boff);
            boff = NULL;
            Assert(false, "ERROR: could not allocate a BOFF");

        }
    }
    return boff;
} /* NewBOff */


void DisposeBOff(BOFF* boff)
{
    Assert(boff != NULL, "boff has not been allocated");
    Assert(boff->fLockLevel == 0, "ERROR: boff is still locked");

    DisposeGWorld(boff->fGWorld);
    DisposePtr((Ptr)boff);
} /* DisposeBOff */


void StartToUseBOff(BOFF* boff)
{
    Assert(boff != NULL, "boff has not been allocated");

    if (boff->fLockLevel == 0)
    {
        /* only save if not locked */
        GetGWorld(&(boff->fGrafArea), &(boff->fGrafDevice));

        if (!LockPixels(GetGWorldPixMap(boff->fGWorld)))
        {
            OSErr err = QDError();
            Assert(false, "ERROR: could not lock pixels");
        }
    }
    boff->fLockLevel++;
} /* StartToUse */


void StopToUseBOff(BOFF* boff)
{
    Assert(boff != NULL, "boff has not been allocated");

    boff->fLockLevel--;
    if (boff->fLockLevel == 0)
    {
        /* only restore if no more locks */
        UnlockPixels(GetGWorldPixMap(boff->fGWorld));

        if (boff->fGrafArea && boff->fGrafDevice)
            SetGWorld(boff->fGrafArea, boff->fGrafDevice);

        boff->fGrafDevice = NULL;
        boff->fGrafArea   = NULL;
    }

    Assert(boff->fLockLevel >= 0, "ERROR: lock level < 0, StopToUse called more than StartToUse");
} /* StopToUse */


void ResetBOff(BOFF* boff)
{
    RGBColor resetColor = {0xffff, 0xffff, 0xffff};
    Rect bounds;

    Assert(boff != NULL, "boff has not been allocated");

    StartToUseBOff(boff);
    SetBOff(boff);

    RGBBackColor(&resetColor);
    GetBOffBounds(boff, &bounds);
    EraseRect(&bounds);

    StopToUseBOff(boff);
} /* ResetBOff */


void SetBOff(BOFF* boff)
{
    Assert(boff != NULL, "boff has not been allocated");
    SetGWorld(boff->fGWorld, NULL);
} /* SetBOff */


void GetBOffBounds(BOFF* boff, Rect* bounds)
{
    Assert(boff != NULL, "boff has not been allocated");
    *bounds = (**(GetGWorldPixMap(boff->fGWorld))).bounds;
} /* GetBOffBounds */


Ptr GetBOffBasePtr(BOFF* boff, int x, int y)
{
    Ptr base;
    Assert(boff != NULL, "boff has not been allocated");

    if (boff->fLockLevel == 0)
    {
        Assert(false, "boff has not been locked, call StartToUseBOff first");
        return NULL;
    }

    base = GetPixBaseAddr(GetGWorldPixMap(boff->fGWorld));
    if (base)
    {
        /* WARNING: x/y offset in GetBaseAddr has not been tested */
        int rowOffset    = y * GetBOffRowBytes(boff);
        int columnOffset = (x * GetBOffPixelSize(boff) + 7)/8;
        base += rowOffset + columnOffset;
    }
    return base;
} /* GetBOffBasePtr */


int GetBOffRowBytes(BOFF* boff)
{
    Assert(boff != NULL, "boff has not been allocated");

    /* must mask off the upper two bits to get row bytes */
    return ((**(GetGWorldPixMap(boff->fGWorld))).rowBytes) & 0x3FFF;
} /* GetBOffRowBytes */


CTabHandle GetBOffClut(BOFF* boff)
{
    Assert(boff != NULL, "boff has not been allocated");
    return (**(GetGWorldPixMap(boff->fGWorld))).pmTable;
} /* GetBOffClut */


short GetBOffPixelSize(BOFF* boff)
{
    Assert(boff != NULL, "boff has not been allocated");
    return (**(GetGWorldPixMap(boff->fGWorld))).pixelSize;
} /* GetBOffPixelSize */


PicHandle GetBOffPicHandle(BOFF* boff)
{
    PicHandle pict;

    Assert(boff != NULL, "boff has not been allocated");
    StartToUseBOff(boff);
    {
        Rect bounds;
        GetBOffBounds(boff, &bounds);

        SetBOff(boff);
        pict = OpenPicture(&bounds);
        CopyBits(
            (BitMap*) *(GetGWorldPixMap(boff->fGWorld)),
            (BitMap*) *(GetGWorldPixMap(boff->fGWorld)),
            &bounds, &bounds, srcCopy, NULL);
        ClosePicture();
    }
    StopToUseBOff(boff);

    return pict;
} /* GetBOffPicHandle */
