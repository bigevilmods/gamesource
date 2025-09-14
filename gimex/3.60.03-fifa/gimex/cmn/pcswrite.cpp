// (c) 1994-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                          pcs Gimex                               */
/*                                                                  */
/*                    by Laurent Ancessi, EAC                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 941101 pcs read write                                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Compatible with Panoramix PlugIn                                 */
/* Reentrant                                                        */
/* Files:                                                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* canexport in every resolution                                       */
/*                                                                  */
/*------------------------------------------------------------------*/

#include "pcsgimex.h"

/****************************************************************/
/*  Write functions                                             */
/****************************************************************/

int SaveToDisk(Handle TheData,ResType TheType,short ID)
{
    AddResource(TheData,TheType,ID,"\p");
    if (!ResError())
    {
        WriteResource(TheData);
        if (!ResError())
        {
            ReleaseResource(TheData);
            if (!ResError()) return(1);
        }
    }
    else DisposeHandle(TheData);
    return(0);
}


void WriteLine(char* src,char* dst,int width,int bpp)
{
    int offset;

    switch (bpp)
    {
        case 1:
            for (offset = 0; offset < (width+7)/8; offset++)
            {
                unsigned char dstPixel = 0;
                dstPixel |= (*(src + offset*8 + 0)) << 7;
                dstPixel |= (*(src + offset*8 + 1)) << 6;
                dstPixel |= (*(src + offset*8 + 2)) << 5;
                dstPixel |= (*(src + offset*8 + 3)) << 4;
                dstPixel |= (*(src + offset*8 + 4)) << 3;
                dstPixel |= (*(src + offset*8 + 5)) << 2;
                dstPixel |= (*(src + offset*8 + 6)) << 1;
                dstPixel |= (*(src + offset*8 + 7)) << 0;
                *(dst + offset) = dstPixel;
            }
            break;
        case 2:
            for (offset = 0; offset < (width+3)/4; offset++)
            {
                unsigned char dstPixel = 0;
                dstPixel |= (*(src + offset*4 + 0)) << 6;
                dstPixel |= (*(src + offset*4 + 1)) << 4;
                dstPixel |= (*(src + offset*4 + 2)) << 2;
                dstPixel |= (*(src + offset*4 + 3)) << 0;
                *(dst + offset) = dstPixel;
            }
            break;
        case 4:
            for (offset = 0; offset < (width+1)/2; offset++)
            {
                unsigned char dstPixel = 0;
                dstPixel |= (*(src + offset*2 + 0)) << 4;
                dstPixel |= (*(src + offset*2 + 1)) << 0;
                *(dst + offset) = dstPixel;
            }
            break;
        case 8:
            for (offset = 0; offset < width; offset++) *(dst + offset) = *(src + offset);
            break;
        case 16:
            for (offset = 0; offset < width; offset++)
            {
                ARGB  srcPixel = *((ARGB*)src + offset);
                unsigned short dstPixel = 0;
                dstPixel |= ((unsigned short)(srcPixel.r >> 3)) << 10;
                dstPixel |= ((unsigned short)(srcPixel.g >> 3)) << 5;
                dstPixel |= ((unsigned short)(srcPixel.b >> 3)) << 0;
                *(unsigned short*)(dst + offset*2) = dstPixel;
            }
            break;
        case 32:
            for (offset = 0; offset < width; offset++) *((ARGB*)dst + offset) = *((ARGB*)src + offset);
            break;
    }
}


short GetMacBpp(short Bpp)
{
    if (Bpp>16) return(32);
    else
    {
        if (Bpp>8) return(16);
        else
        {
            if (Bpp>4) return(8);
            else
            {
                if (Bpp>2) return(4);
                else
                {
                    if (Bpp>1) return(2);
                    else return(1);
                }
            }
        }
    }
}


/****************************************************************/
/*  Main functions                                              */
/****************************************************************/

/* Export Functions */

int GCALL PCS_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes)
{
    PicsDataPtr gPics=0L;
    GINSTANCE*  Result=0L;

    gPics=(PicsDataPtr) galloc(sizeof(PicsData));
    if (gPics!=nil)
    {
        gPics->MaxSize=0;
        gPics->MaxDepth=0;
        memset(gPics,0,sizeof(PicsData));
        Result=(GINSTANCE*)galloc(sizeof(GINSTANCE));
        if (Result)
        {
            memset(Result,0,sizeof(GINSTANCE));
            Result->frames=numframes;
            Result->gstream=g;
            Result->gref=(char*)gPics;
        }
    }
    *gx=Result;
    if (Result) return(1);
    else return(0);
}


int GCALL PCS_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    PicsDataPtr gPics=0L;
    PicHandle   ThePicH;
    CTabHandle  TheColorTable=0L;
    Rect        TheBounds;
    BOFF*       TheBOff;
    short       DestBpp;
    int         Answer=0;
    char*       DestPixel;
    int        DestRowBytes;
    int         i;

    gPics=(PicsDataPtr)gx->gref;
    TheColorTable=GetGInfoPalette(ginfo);
    DestBpp=ginfo->originalbpp;
    DestBpp=GetMacBpp(DestBpp);
    TheBOff=NewBOff(ginfo->width,ginfo->height,DestBpp,TheColorTable);
    if (TheBOff)
    {
        StartToUseBOff(TheBOff);
        {
            DestPixel=(char*)GetBOffBasePtr(TheBOff,0,0);
            if (DestPixel)
            {
                DestRowBytes=GetBOffRowBytes(TheBOff);
                for (i=0; i<ginfo->height; i++)
                {
                    WriteLine(source,DestPixel,ginfo->width,DestBpp);
                    DestPixel+=DestRowBytes;
                    source+=rowbytes;
                }
            }
        }
        StopToUseBOff(TheBOff);
        ThePicH=GetBOffPicHandle(TheBOff);
        if (ThePicH)
        {
            if (GetHandleSize((Handle)ThePicH)>gPics->MaxSize) gPics->MaxSize=GetHandleSize((Handle)ThePicH);
            if (DestBpp>gPics->MaxDepth) gPics->MaxDepth=DestBpp;
            Answer=SaveToDisk((Handle)ThePicH,'PICT',(short)(128+ginfo->framenum));
        }
        DisposeBOff(TheBOff);
    }
    if (TheColorTable) DisposeHandle((Handle)TheColorTable);
    return(Answer);
}


int GCALL PCS_wclose(GINSTANCE *gx)
{
    PicsDataPtr     gPics=0L;
    PicsInfoHdle    TheInfo;
    int             Answer=0;

    if (gx)
    {
        gPics=(PicsDataPtr)gx->gref;
        TheInfo=(PicsInfoHdle)NewHandle(sizeof(PicsInfo));
        if (TheInfo)
        {
            if (gPics->MaxDepth==1) (**TheInfo).BWColor=0;
            else (**TheInfo).BWColor=1;
            (**TheInfo).MaxDepth=gPics->MaxDepth;
            (**TheInfo).Speed=30;
            (**TheInfo).Version=0;
            (**TheInfo).Creator='Pano';
            (**TheInfo).MaxSize=gPics->MaxSize;
            Answer=SaveToDisk((Handle)TheInfo,'INFO',128);
        }
        gfree((char*)gPics);
        gfree((void*)gx);
    }
    return(Answer);
}
