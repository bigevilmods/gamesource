// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                   PCS - Macintosh PICS Gimex                     */
/*                                                                  */
/*                    by Laurent Ancessi, EAC                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940706 pcs read write                                    */
/*    1.01 950529 FB ginstance signature                            */
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
/* import in every resolution                                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "pcsgimex.h"
#include "boff.cpp"

/****************************************************************/
/*  Get info on palette                                         */
/****************************************************************/

CTabHandle GetGInfoPalette(GINFO *theGINFO)
{
    CTabHandle  TheColorTable=0L;
    int         i;
    int        theSize;
    int        ANewCTSeed;
    ARGB        TheColor;

    if (theGINFO->numcolours)
    {
        theSize=sizeof(int)+(sizeof(short)*2)+(theGINFO->numcolours*sizeof(ColorSpec));
        TheColorTable=(CTabHandle) NewHandle(theSize);
        if (TheColorTable)
        {
            ANewCTSeed=GetCTSeed();
            (**TheColorTable).ctSeed=ANewCTSeed;
            (**TheColorTable).ctFlags=0;
            (**TheColorTable).ctSize=theGINFO->numcolours-1;
            for (i=0; i<theGINFO->numcolours; i++)
            {
                TheColor=theGINFO->colourtbl[i];
                (**TheColorTable).ctTable[i].rgb.red =(TheColor.r<<8)|TheColor.r;
                (**TheColorTable).ctTable[i].rgb.green=(TheColor.g<<8)|TheColor.g;
                (**TheColorTable).ctTable[i].rgb.blue=(TheColor.b<<8)|TheColor.b;
                (**TheColorTable).ctTable[i].value=(short) i;
            }
        }
    }
    return(TheColorTable);
}


short GetMacColors(short Bpp)
{
    if (Bpp>16) return(0);
    else
    {
        if (Bpp>8) return(0);
        else
        {
            if (Bpp>4) return(256);
            else
            {
                if (Bpp>2) return(16);
                else
                {
                    if (Bpp>1) return(4);
                    else return(2);
                }
            }
        }
    }
}


/****************************************************************/
/*  Main functions                                              */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL PCS_about(void)
{
    GABOUT* info;

    info=(GABOUT *)galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info,0,sizeof(GABOUT));
        info->signature         = PICSSIGN;
        info->size              = sizeof(GABOUT);
        info->version           = GIMEX_VERSION;
        info->canimport            = 1;    /* supports importing */
        info->canexport            = 1;    /* supports exporting */
        info->importpacked      = 1;    /* supports importing packed */
        info->exportpacked      = 1;    /* supports exporting packed */
        info->import8           = 1;    /* supports importing 8 bit indexed */
        info->export8           = 1;    /* supports exporting 8 bit indexed */
        info->import32          = 1;    /* supports importing 32 bit direct rgb */
        info->export32          = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe        = 1;    /* supports multiple frames */
        info->multifile         = 0;    /* format requires additional files or resource fork */
        info->multisize         = 1;    /* supports each frame different size */
        info->framebuffer       = 0;    /* module requires memory to buffer entire frame */
        info->external          = 0;    /* module uses external resources */
        info->usesfile          = 1;    /* module is file based */
        info->maxframename      = 0;    /* max frame name character */
        info->defaultquality 100;  /* default pack quality */
        strcpy(info->authorstr,     "Laurent Ancessi");     /* author */
        info->mactype[0]        = PICSSIGN;                 /* mac file system types used */
        strcpy(info->extensions[0], ".pcs");                /* extensions */
        strcpy(info->versionstr,    "1.01");                /* version # */
        strcpy(info->shorttypestr,  "PICS");                /* type */
        strcpy(info->wordtypestr,   "Mac PICS");            /* type */
        strcpy(info->longtypestr,   "Mac PICS File");       /* longtype */
    }
    return(info);
}


int GCALL PCS_is(GSTREAM *g)
{
    int     IsPics=0;

    SetResLoad(false); /* do not need resource, just info */
    if (Get1Resource('PICT',128)) /* There is at least one frame */
    {
        IsPics=90;
        if (Get1Resource('INFO',128)) IsPics+=10; /* There the info */
    }
    SetResLoad(true); /* Restore configuration */
    return(IsPics);
}


/* Import Functions */

int GCALL PCS_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE*      Result=0L;
    int             numframes=0;

    numframes=Count1Resources('PICT'); /* The number of resources PICT is the number of frames */
    if (numframes)
    {
        Result=(GINSTANCE*)galloc(sizeof(GINSTANCE));
        if (Result)
        {
            memset(Result,0,sizeof(GINSTANCE));
            Result->frames=numframes;
            Result->gstream=g;
            Result->gref=0L; /* Don't need globals */
        }
    }
    *gx=Result;
    return(numframes);
}


GINFO* PCS_info(GINSTANCE *gx, int framenum)
{
    Handle      TheResPict=0L;
    PicHandle   ThePicH;
    GINFO*      theGINFO=0L;
    ARGB        TheColor;
    int         i,Bpp;
    PictInfo    thePictInfo;
    short       NbEntries;

    TheResPict=Get1Resource('PICT',(short)(128+framenum));
    if (TheResPict)
    {
        HNoPurge(TheResPict);
        HLock(TheResPict);
        ThePicH=(PicHandle)TheResPict;
        if (GetPictInfo(ThePicH,&thePictInfo,0,0,systemMethod,0)==noErr)
        {
            Bpp=(int)thePictInfo.depth;
            if (Bpp==0) Bpp=32; /* Bpp==0 means best depth */
            if (framenum < gx->frames)
                theGINFO=(GINFO*)galloc(sizeof(GINFO));

            if (theGINFO)
            {
                memset(theGINFO,0,sizeof(GINFO));
                theGINFO->signature=PICSID;                                             /* Type of file */
                theGINFO->size=sizeof(GINFO);                                           /* Length of theGINFO */
                theGINFO->framenum=framenum;                                            /* Number of frames (if more than 1, openframeblt required) */
                theGINFO->width=(**ThePicH).picFrame.right-(**ThePicH).picFrame.left;   /* Width of bitmap in pixels */
                theGINFO->height=(**ThePicH).picFrame.bottom-(**ThePicH).picFrame.top;  /* Height of bitmap in pixels */
                theGINFO->originalbpp=Bpp;
                if (theGINFO->originalbpp>8) /* theGINFO->originalbpp==0 means best depth */
                {
                    theGINFO->bpp=32;
                    theGINFO->indexed=0;
                }
                else
                {
                    theGINFO->bpp=8;
                    theGINFO->indexed=1;
                }
                theGINFO->originalindexed=theGINFO->indexed;
                if (theGINFO->indexed)
                {
                    theGINFO->numcolours=0;
                    NbEntries=GetMacColors((short)theGINFO->originalbpp);
                    if (GetPictInfo(ThePicH,&thePictInfo,returnColorTable,NbEntries,systemMethod,0)==noErr)
                    {
                        if (thePictInfo.theColorTable)
                        {
                            theGINFO->numcolours=(**(thePictInfo.theColorTable)).ctSize+1;
                            for (i=0; i<theGINFO->numcolours; i++)
                            {
                                TheColor.a=0;
                                TheColor.r=(unsigned char)((**(thePictInfo.theColorTable)).ctTable[i].rgb.red>>8);
                                TheColor.g=(unsigned char)((**(thePictInfo.theColorTable)).ctTable[i].rgb.green>>8);
                                TheColor.b=(unsigned char)((**(thePictInfo.theColorTable)).ctTable[i].rgb.blue>>8);
                                theGINFO->colourtbl[i]=TheColor;
                            }
                            DisposeHandle((Handle)thePictInfo.theColorTable);
                        }
                    }
                }
                theGINFO->packed=1;         /* The pict is a compressed file format */
                theGINFO->quality=100;      /* Quality of lossy packing 0..100 */
                theGINFO->alphabits=0;      /* Number of bits in alpha channel */
                theGINFO->redbits=8;        /* Number of bits in red channel */
                theGINFO->greenbits=8;      /* Number of bits in grreen channel */
                theGINFO->bluebits=8;       /* Number of bits in blue channel */
            }
        }
        HUnlock(TheResPict);
        HPurge(TheResPict); /* Can be released if it needed */
    }
    return(theGINFO);
}


int GCALL PCS_read(GINSTANCE *gx, GINFO *theGINFO, char *dest, int rowbytes)
{
    Handle      TheResPict=0L;
    PicHandle   ThePicH;
    CTabHandle  TheColorTable=0L;
    Rect        TheBounds;
    BOFF*       TheBOff;
    char*       Pixel;
    int         i;
    int        PixelRowBytes;
    int         Answer=0;

    TheColorTable=GetGInfoPalette(theGINFO);
    TheResPict=Get1Resource('PICT',(short)(128+theGINFO->framenum));
    if (TheResPict)
    {
        HNoPurge(TheResPict);
        HLock(TheResPict);
        ThePicH=(PicHandle)TheResPict;
        TheBOff=NewBOff(theGINFO->width,theGINFO->height,theGINFO->bpp,TheColorTable);
        if (TheBOff)
        {
            StartToUseBOff(TheBOff);
            {
                GetBOffBounds(TheBOff,&TheBounds);
                SetBOff(TheBOff);
                DrawPicture(ThePicH,&TheBounds);
                PixelRowBytes=GetBOffRowBytes(TheBOff);
                if (rowbytes<PixelRowBytes) PixelRowBytes=rowbytes;
                for (i=0; i<theGINFO->height; i++)
                {
                    Pixel=(char*)GetBOffBasePtr(TheBOff,0,i);
                    if (Pixel) memcpy(dest,Pixel,PixelRowBytes);
                    dest+=rowbytes;
                }
                Answer=1;
            }
            StopToUseBOff(TheBOff);
            DisposeBOff(TheBOff);
        }
        HUnlock(TheResPict);
        HPurge(TheResPict); /* Can be released if it needed */
    }
    if (TheColorTable) DisposeHandle((Handle)TheColorTable);
    return(Answer);
}


int GCALL PCS_close(GINSTANCE *gx)
{
    if (gx) gfree((void*)gx);
    return(1);
}
