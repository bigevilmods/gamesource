// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                   FLN - SGI Flint Gimex                          */
/*                                                                  */
/*                     by Kaz Okuda EAC                             */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/* 1.00    980208 KO Code copied from flint2mov by Frank Henigman   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Files: flnread.cpp flnwrite.cpp flngimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* Uses Flint API to read images.  Uses libio.h rather than         */
/* standard file io so open uses filename.                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>

#include "gimex.h"
#include "flngimex.h"

/****************************************************************/
/*  Taken from libio.h, part of the Flint API                   */
/*  Parameters and return values are documented in libio.h      */
/****************************************************************/

#include <sys/types.h>   /* time_t */

/*==============================================================================
* Error codes returned by the libio functions.
*
* LIBIO_RETRY
*       The program can loop on a 'ClipReadFrame' or 'ClipWriteFrame' as
*       int as the returned code is LIBIO_RETRY.
*              
*=============================================================================*/

enum {
   LIBIO_STATUS_OK = 0,
   LIBIO_RETRY,
   LIBIO_INVALID_OPEN_MODE,
   LIBIO_CANNOT_GET_FRAMESTORE_PATH,
   LIBIO_CLIP_NOT_OPENED,
   LIBIO_FRAME_INDEX_OUT_OF_RANGE,
   LIBIO_CANNOT_ALLOC_FRAME_TABLE_OBSOLETE,   /* not used, obsolete */
   LIBIO_INVALID_CLIP_PATH_NAME,
   LIBIO_CANNOT_ACCESS_CLIP_OBSOLETE,         /* not used, obsolete */
   LIBIO_PARTITION_FULL_OBSOLETE,             /* not used, obsolete */
   LIBIO_CANNOT_OPEN_VOLUME,
   LIBIO_CANNOT_OPEN_PARTITION,
   LIBIO_CANNOT_OPEN_CLIP_LIBRARY,
   LIBIO_LIBRARY_IS_READ_ONLY_OBSOLETE,       /* not used, obsolete */
   LIBIO_CLIP_IS_READ_ONLY,
   LIBIO_CLIP_NOT_FOUND,
   LIBIO_UNABLE_TO_READ_FRAME,
   LIBIO_UNABLE_TO_WRITE_FRAME,
   LIBIO_SAVED_IN_LIBIO_BIN,
   LIBIO_UNABLE_TO_WRITE_CLIP,
   LIBIO_START_FAILED,
   LIBIO_STOP_FAILED
};


/******************************************************************************/
/* BUFFER FORMAT DEFINITIONS                                                  */
/*                                                                            */
/* The Depth field in the ClipInfoStruct structure used to return 3 for 8 bit */
/* buffers, 8, 128 or 129 for 12 bit buffers. Libio now supports both IrisGL  */
/* and OpenGL buffer formats, hence the need for new enumerations to          */
/* distinguish between these formats.                                         */
/*                                                                            */
/* LIBIO_DEPTH_8_BGR corresponds to an 8 bit per component buffer format      */
/* with the pixel color components ordered in memory BGR BGR BGR...           */
/* This is the format for IrisGL-compatible image buffers as used in Flint on */
/* Indigo2 XZ/Extreme, Flame and Inferno (in 8 bit mode) on RealityEngine2.   */
/*                                                                            */
/* LIBIO_DEPTH_8_RGB corresponds to an 8 bit per component buffer format      */
/* with the pixel color components ordered in memory RGB RGB RGB...           */
/* This is the format for OpenGL-compatible image buffers as used in Flint on */
/* Indigo2 Impact, Flame and Inferno (in 8 bit mode) on InfiniteReality.      */
/*                                                                            */
/* For 12 bit per color component formats ( only supported in Inferno ), the  */
/* 12 data bits are stored in the 12 most significant bits of a 16 bit short. */
/* The 4 least significant bits are ignored.                                  */
/*                                                                            */
/* LIBIO_DEPTH_12_ABGR corresponds to a 12 bit per component buffer format    */
/* with the 16 bit pixel color components ordered in memory ABGR ABGR ABGR... */
/* The extra Alpha component is required for compatibility with IrisGL and is */
/* ignored. This is the format used by Inferno in 12 bit mode on              */
/* RealityEngine2.                                                            */
/*                                                                            */
/* LIBIO_DEPTH_12_RGB corresponds to a 12 bit per component buffer format     */
/* with the 16 bit pixel color components ordered in memory RGB RGB RGB...    */
/* There is no extra Alpha component. This is the format used by Inferno in   */
/* 12 bit mode on InfiniteReality.                                            */
/*                                                                            */
/* COMPATIBILITY NOTES: although in previous versions the Depth field could   */
/* be interpreted as the number of bytes per pixel and thus a buffer could    */
/* be allocated using Width * Height * Depth bytes of memory, this is no      */
/* longer the case: Depth should be interpreted as an arbitrary enum.         */
/* In all cases, the Bytes member of the ClipInfoStruct structure should be   */
/* used as the size of the memory to be allocated.                            */
/*                                                                            */
/* In order to allow third party applications based on older versions of      */
/* libio to continue functionning as much as possible, the Depth field of     */
/* the ClipInfoStruct has not been renamed (although it should really be      */
/* called "Format"), and the enum for 8 bit IrisGL buffers has been defined   */
/* as 3, which is the value that was previously returned.                     */
/* In 12 bits, depending on the internal framestore format, either 8, 128 or  */
/* 129 could be returned, but since most/all Inferno 12 bit customers have    */
/* been using the 36 bit packed framestore format, this would usually return  */
/* 128, so this is the value which has been kept.                             */
/*                                                                            */
/******************************************************************************/
   
enum {
  LIBIO_DEPTH_8_BGR  = 3,
  LIBIO_DEPTH_8_RGB  = 130,
  LIBIO_DEPTH_12_ABGR = 128,
  LIBIO_DEPTH_12_RGB = 134
};

#define PATH_MAX 1024

/*============================================================================*/
/* Clip info structure filled by ClipOpen().                                  */
/*============================================================================*/

typedef struct {

   char   openMode;                /* X: not opened, R: read, W: write   */
   int    pmIndex;                 /* UNUSED!!!                          */

   char   clipLibPath[PATH_MAX];   /* clip library path name             */
   char   clipName[PATH_MAX];      /* clip name                          */
   time_t clipDate;                /* clip creation date                 */

   int   Width;                   /* frame width  in pixels             */
   int   Height;                  /* frame height in pixels             */
   int   Depth;                   /* pixel format in memory             */

   int   Bytes;                   /* frame buffer size in bytes         */

   int   Frames;                  /* number of frames in the clip       */
   void*  privateData;             /* the name says it all: hands off!!! */

} ClipInfoStruct;


int ClipStart();
int ClipStop();
int ClipOpen( ClipInfoStruct* clipInfo, const char *clipPath, const char *openMode );
int ClipClose( ClipInfoStruct* clipInfo );
int ClipReadFrame( ClipInfoStruct* clipInfo, int frameIndex, unsigned int *frameBuffer );


/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

/****************************************************************/
/*  Windows specific implementation of Flint for testing        */
/****************************************************************/

#if defined(_MSC_VER) && !defined(_XBOX)

#include <dos.h>

int ClipStart()
{
    return(LIBIO_STATUS_OK);
}

int ClipStop()
{
    return(LIBIO_STATUS_OK);
}

int ClipOpen( ClipInfoStruct* clipInfo, const char *clipPath, const char *openMode )
{
    clipInfo->clipLibPath[0] = 0;      /* clip library path name             */
    clipInfo->clipName[0] = 0;         /* clip name                          */
    clipInfo->clipDate = 0;            /* clip creation date                 */
    clipInfo->Width = 100;                   /* frame width  in pixels             */
    clipInfo->Height = 100;                  /* frame height in pixels             */
    clipInfo->Depth = LIBIO_DEPTH_12_ABGR;    /* pixel format in memory             */
    clipInfo->Bytes = 100*100*8;              /* frame buffer size in bytes         */
    clipInfo->Frames = 10;                  /* number of frames in the clip       */

    return(LIBIO_STATUS_OK);
}

int ClipClose( ClipInfoStruct* clipInfo )
{
    return(LIBIO_STATUS_OK);
}

int ClipReadFrame( ClipInfoStruct* clipInfo, int frameIndex, unsigned int *frameBuffer )
{
    int i;
    int j;
    char *fb;

    fb = (char*)frameBuffer;

    for (i=0;i<100;i++)
    {
        switch(clipInfo->Depth)
        {
            case LIBIO_DEPTH_8_BGR:
                for (j=0;j<100;j++)
                {
                    *fb++ = 25; /* B */
                    *fb++ = 75; /* G */
                    *fb++ = 50; /* R */
                }
                break;
            case LIBIO_DEPTH_8_RGB:
                for (j=0;j<100;j++)
                {
                    *fb++ = 50; /* R */
                    *fb++ = 75; /* G */
                    *fb++ = 25; /* B */
                }
                break;
            case LIBIO_DEPTH_12_ABGR:
            {
                unsigned short *sfb;
                sfb = (unsigned short*)fb;

                for (j=0;j<100;j++)
                {
                    *sfb++ = 255<<8; /* A */
                    *sfb++ = 25<<8;  /* B */
                    *sfb++ = 75<<8;  /* G */
                    *sfb++ = 50<<8;  /* R */
                }
                fb = (char*)sfb;
                break;
            }
            case LIBIO_DEPTH_12_RGB:
            {
                unsigned short *sfb;
                sfb = (unsigned short*)fb;

                for (j=0;j<100;j++)
                {
                    *sfb++ = 50<<8; /* R */
                    *sfb++ = 75<<8; /* G */
                    *sfb++ = 25<<8; /* B */
                }
                fb = (char*)sfb;
                break;
            }
        }
    }
    return(LIBIO_STATUS_OK);
}


#endif

/****************************************************************/
/*  End Windows specific implementation                         */
/****************************************************************/

static void FLNflint2gimex(ClipInfoStruct *clipinfo, char *input, char *output, int rowbytes)
{
    char *inp;
    char *outp;
    int x;
    int y;

    inp = input;
    outp = output + rowbytes * ( clipinfo->Height - 1 ); /* start at bottom line */

    for ( y = 0; y < clipinfo->Height; ++y )
    {
        switch ( clipinfo->Depth )
        {
            case LIBIO_DEPTH_8_BGR:
                for ( x = 0; x < clipinfo->Width; ++x )
                {
                    ((ARGB*)outp)->a = 255;
                    ((ARGB*)outp)->b = *inp++;
                    ((ARGB*)outp)->g = *inp++;
                    ((ARGB*)outp)->r = *inp++;
                    outp += sizeof(ARGB);
                }
                break;
            case LIBIO_DEPTH_8_RGB:
                for ( x = 0; x < clipinfo->Width; ++x )
                {
                    ((ARGB*)outp)->a = 255;
                    ((ARGB*)outp)->r = *inp++;
                    ((ARGB*)outp)->g = *inp++;
                    ((ARGB*)outp)->b = *inp++;
                    outp += sizeof(ARGB);
                }
                break;
            case LIBIO_DEPTH_12_ABGR:
                for ( x = 0; x < clipinfo->Width; ++x )
                {
                    ((ARGB*)outp)->a = 255;
                    inp+=2;
                    ((ARGB*)outp)->b = (unsigned char)((*(unsigned short*)inp)>>8);
                    inp+=2;
                    ((ARGB*)outp)->g = (unsigned char)((*(unsigned short*)inp)>>8);
                    inp+=2;
                    ((ARGB*)outp)->r = (unsigned char)((*(unsigned short*)inp)>>8);
                    inp+=2;
                    outp += sizeof(ARGB);
                }
                break;
            case LIBIO_DEPTH_12_RGB:
                for ( x = 0; x < clipinfo->Width; ++x )
                {
                    ((ARGB*)outp)->a = 255;
                    ((ARGB*)outp)->r = (unsigned char)((*(unsigned short*)inp)>>8);
                    inp+=2;
                    ((ARGB*)outp)->g = (unsigned char)((*(unsigned short*)inp)>>8);
                    inp+=2;
                    ((ARGB*)outp)->b = (unsigned char)((*(unsigned short*)inp)>>8);
                    inp+=2;
                    outp += sizeof(ARGB);
                }
                break;
        }
        outp -= 2 * rowbytes;
    }

}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL FLN_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));
        info->signature       = GMAKEID('F','L','N','T');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 0;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 1;    /* module uses external resources */
        info->usesfile        = 0;    /* module is file based */
        info->globalpalette   = 0;    /* limited to a single palette per file */
        info->greyscale       = 0;    /* use maxcolours for number of levels */
        info->startcolour     = 0;    /* supports start colour */
        info->dotsubtype      = 0;    /* subtype based on extension */
        info->resizable       = 0;    /* read will respect ginfo width & height */
        info->maxcolours      = 0;    /* only use in 8 bit, 0 if module does not care */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('F','L','N','T');   /* mac file system types used */
        strcpy(info->authorstr,     "Kaz Okuda");           /* author */
        strcpy(info->versionstr,    "1.00");                /* version # */
        strcpy(info->shorttypestr,  "FLN");                 /* type */
        strcpy(info->wordtypestr,   "Flint");               /* wordtype */
        strcpy(info->longtypestr,   "Flint");               /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
int GCALL FLN_is(GSTREAM *g)
{
    return(0);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL FLN_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;
    int rc;
    ClipInfoStruct *clipinfo=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('F','L','N','T');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;

        rc = ClipStart();
        if ( rc == LIBIO_STATUS_OK )
        {
            clipinfo = galloc(sizeof(ClipInfoStruct));
            if (clipinfo)
            {
                rc = ClipOpen( clipinfo, pathname, "r" );
                if ( rc == LIBIO_STATUS_OK )
                {
                    gx->gref = (char*)clipinfo;
                    ok = clipinfo->Frames;
                }
            }
        }


        if (!ok)
        {
            if (gx)
            {
                if (gx->gref) gfree(gx->gref);
                gfree(gx);
                gx = 0;
            }
            if (clipinfo) gfree(clipinfo);
        }
    }

    *gxptr = gx;
    return(ok);
}

GINFO * GCALL FLN_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    ClipInfoStruct *clipinfo=0;

    clipinfo = (ClipInfoStruct*)gx->gref;
    if (!clipinfo)
    {
        return((GINFO*)NULL);
    }

    if (framenum < gx->frames)
        ginfo = galloc(sizeof(GINFO));

    if (ginfo)
    {
        memset(ginfo, 0, sizeof(GINFO));
        ginfo->signature    = GMAKEID('F','L','N','T');    /* signature of gimex ie 'tga ' (optional) */
        ginfo->size         = sizeof(GINFO);              /* size of GINFO structure */
        ginfo->version      = GIMEX_VERSION;
        ginfo->framenum     = framenum; /* current frame */
        ginfo->width        = clipinfo->Width;    /* width of bitmap in pixels */
        ginfo->height       = clipinfo->Height;   /* height of bitmap in pixels */
        ginfo->bpp          = 32;       /* bits per pixel (8 or 32) */
        ginfo->startcolour  = 0;        /* first colour in palette */
        ginfo->numcolours   = 0;        /* number of colours in original indexed palette */
        ginfo->framesize    = clipinfo->Bytes;   /* size of frame in bytes */
        ginfo->subtype      = 0;        /* internal format sub-type 0-default */
        ginfo->packed       = 0;        /* type of packing on original image. 0 none, 1 run, n other */
        ginfo->quality      = 100;      /* quality of lossy packing 0..100 */
        ginfo->centerx      = 0;        /* center point relative to upper left corner */
        ginfo->centery      = 0;
        ginfo->defaultx     = 0;        /* default coordinate point */
        ginfo->defaulty     = 0;
        ginfo->numhotspots  = 0;        /* number of hot spots defined */
        ginfo->framename[0] = 0;        /* null terminated name of frame/image */
        ginfo->comment[0]   = 0;        /* null terminated multiline user comment */

        switch( clipinfo->Depth )
        {
            case LIBIO_DEPTH_8_BGR:
                ginfo->originalbpp = 24;
                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 8;
                break;
            case LIBIO_DEPTH_8_RGB:
                ginfo->originalbpp = 24;
                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 8;
                break;
            case LIBIO_DEPTH_12_ABGR:
                ginfo->originalbpp = 64;
                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = ginfo->alphabits = 12;
                break;
            case LIBIO_DEPTH_12_RGB:
                ginfo->originalbpp = 36;
                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 12;
                break;
        }
    }
    return(ginfo);
}

int GCALL FLN_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{

    ClipInfoStruct *clipinfo;
    int rc;
    int ok = 0;
    char *fbuf;

    clipinfo = (ClipInfoStruct*)gx->gref;

    if (clipinfo)
    {
        fbuf = (char *) galloc( clipinfo->Bytes );
        if (fbuf)
        {
            for ( ;; )
            {
                rc = ClipReadFrame( clipinfo, ginfo->framenum + 1, (unsigned int*)fbuf );
                if ( rc == LIBIO_RETRY )
                {
                    sleep( 1 );
                    continue;
                }
                break;
            }
    
            if (rc == LIBIO_STATUS_OK)
            {
                FLNflint2gimex(clipinfo, fbuf, dest, rowbytes);
                ok = 1;
            }
            gfree(fbuf);
        }
    }
    return(ok);
}

int GCALL FLN_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        ClipStop();
        if (gx->gref)
        {
            ClipClose((ClipInfoStruct*)gx->gref);
            gfree(gx->gref);
        }
        ok = gfree((void *) gx);
    }

    return(ok);
}

