// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 IJL - Intel Jpeg Library Gimex                   */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 991029 FB JPeg using intel library                       */
/*    1.01 991030 FB BGR & use memory buffer                        */
/*    1.02 991103 FB dynamic dll loader                             */
/*    1.03 991206 FB quality 100 if packed=0, ijl read line buffer  */
/*    1.04 991209 FB 24->32 inplace right to left                   */
/*    1.05 991210 FB 24->32 use int's                               */
/*    1.06 000107 FB ijl11                                          */
/*    1.07 000124 FB CMYK use IJL_RGBA_FPX (intel suggestion)       */
/*    1.08 000201 FB KDC import                                     */
/*    1.09 000828 FB about disable if ijl10/11 not there            */
/*    1.10 000908 FB disable about tristate                         */
/*    1.11 001031 FB dpi & comment                                  */
/*    1.12 010116 FB removed kdc, which confuses with tiff luv      */
/*    1.13 010331 FB write * 1.10 to avoid int overflow             */
/*    1.15 020108 FB ijl 1.51 bug work around (intel recommended)   */
/*    1.16 020318 AE use subsampling 4:2:2 instead of 4:1:1         */
/*    1.17 020911 FB READ return code unreliable - ignored          */
/*    1.18 020919 FB write switch to 411 (not 422) if quality<75    */
/*    1.19 020925 FB IJL_ANN tried/commented for performance        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit only                                                      */
/* Reentrant                                                        */
/* Line buffered                                                    */
/* Files: ijlread.cpp ijlwrite.cpp ijlgimex.h ijl15.dll             */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* Standard DCT based JPeg                                          */
/*                                                                  */
/* Kodak KDC files use JPeg starting at 15680                       */
/* image05.kdc                                                      */
/* 00000: 4d4d002a 00000008 002200fe 00040000  MM.*....."......     */
/* 00010: 00010000 00010100 00030000 00010060  ...............`     */
/* 00020: 00000101 00030000 00010040 00000102  ...........@....     */
/* 00030: 00030000 00030000 01a60103 00030000  .........&......     */
/* 00040: 00010001 00000106 00030000 00010002  ................     */
/* 00050: 0000010e 00020000 00080000 01ac010f  .............,..     */
/* 00060: 00020000 00160000 01b40110 00020000  .........4......     */
/* 00070: 00270000 01ca0111 00040000 00010000  .'...J..........     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "windows.h"
#include "gimex.h"
#include "ijlgimex.h"
#include "ijl.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

HMODULE dllhandle=0;
IJLERR (WINAPI *ijlInitPtr)( JPEG_CORE_PROPERTIES* jcprops );
IJLERR (WINAPI *ijlReadPtr)( JPEG_CORE_PROPERTIES* jcprops, IJLIOTYPE iotype );
IJLERR (WINAPI *ijlWritePtr)( JPEG_CORE_PROPERTIES* jcprops, IJLIOTYPE iotype );
IJLERR (WINAPI *ijlFreePtr)( JPEG_CORE_PROPERTIES* jcprops );

static void IJL_freedll(void)
{
    if (dllhandle)
    {
        FreeLibrary(dllhandle);
        dllhandle = 0;
    }
}

int IJL_loaddll(void)
{
    int ok=1;

    if (!dllhandle)
    {
        ok = 0;
        dllhandle = LoadLibrary("ijl15.dll");
        if (!dllhandle)
        {
            dllhandle = LoadLibrary("ijl11.dll");
            if (!dllhandle)
            {
                dllhandle = LoadLibrary("ijl10.dll");
            }
        }
        if (dllhandle)
        {
            *(void **)&ijlInitPtr = (void *) GetProcAddress(dllhandle,"ijlInit");
            *(void **)&ijlReadPtr = (void *) GetProcAddress(dllhandle,"ijlRead");
            *(void **)&ijlWritePtr = (void *) GetProcAddress(dllhandle,"ijlWrite");
            *(void **)&ijlFreePtr = (void *) GetProcAddress(dllhandle,"ijlFree");
            if (ijlInitPtr && ijlReadPtr && ijlFreePtr)
                ok = 1;
            else
                IJL_freedll();
        }
    }
    return(ok);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL IJL_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('I','J','P','G');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 80;   /* default pack quality */
        info->mactype[0]      = GMAKEID('.','j','p','g');   /* mac file system types used */
        strcpy(info->extensions[0], ".jpg");                /* extensions */
        strcpy(info->extensions[1], ".ijpg");               /* extensions */
        strcpy(info->extensions[2], ".ijl");               /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.19");                /* version # */
        strcpy(info->shorttypestr,  "ijl");                 /* type */
        strcpy(info->wordtypestr,   "iJPeg");               /* wordtype */
        strcpy(info->longtypestr,   "Intel JPeg");               /* longtype */

        {
            static int plugintested = 0;

            if (!plugintested)
            {
                plugintested = 1;
                if (!IJL_loaddll())
                {
                    plugintested = -1;
                }
            }
            if (plugintested<0)
            {
                info->canimport       = 0;    /* supports importing */
                info->importpacked    = 0;    /* supports importing packed */
                info->import8         = 0;    /* supports importing 8 bit indexed */
                info->import32        = 0;    /* supports importing 32 bit direct rgb */
                info->canexport       = 0;    /* supports exporting */
                info->exportpacked    = 0;    /* supports exporting packed */
                info->export8         = 0;    /* supports exporting 8 bit indexed */
                info->export32        = 0;    /* supports exporting 32 bit direct rgb */
            }
        }
    }
    return(info);
}

/* check for reasonable header: */
/* id ffd8ffe1 or ffd8ffe0 */

int GCALL IJL_is(GSTREAM *g)
{
    unsigned char jpgheader[4];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,jpgheader,4))
    {
        if (ggetm(jpgheader,4)==0xffd8ffe0  // Jpeg
         || ggetm(jpgheader,4)==0xffd8ffe1
         || ggetm(jpgheader,4)==0xffd8ffed)
//         || ggetm(jpgheader,4)==0x4d4d002a) // KDC
        {
            if (IJL_loaddll())
            {
//                if (ggetm(jpgheader,4)==0x4d4d002a) // KDC
//                    ok = 90;    /* its a KDC */
//                else
                    ok = 99;    /* its a jpeg */
            }
        }
    }
    return(ok);
}

#if 0
/* check if jpeg library can read the file */

int GCALL IJL_is(GSTREAM *g)
{
    int ok=0;
    JPEG_CORE_PROPERTIES image;
    ZeroMemory( &image, sizeof( JPEG_CORE_PROPERTIES ) );

    if ( ijlInitPtr( &image ) == IJL_OK )
    {
        image.JPGFile = const_cast<char*>(lpszPathName);
        if( ijlReadPtr( &image, IJL_JBUFF_READPARAMS ) == IJL_OK )
        {
            ok = 100;
        }
        if( ijlFreePtr( &image ) != IJL_OK )
        {
            ok = 0;
        }
    }
    return(ok);
}
#endif

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL IJL_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;
    int flen;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        flen = (int) glen(g);
        gseek(g,(GPOS)0);

        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('I','J','P','G');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        gx->gref = (void *)galloc(flen);
        if (gx->gref)
        {
            if (gread(g,gx->gref,flen))
            {
                *gxptr = gx;
                if (IJL_loaddll())
                    ok = 1;
                else
                    IJL_freedll();
            }
            else
                gfree(gx->gref);
        }
    }
    return(ok);
}

GINFO * GCALL IJL_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    int ok=0;
    int width=0;
    int height=0;
    int quality=0;
    int subtype=0;
    int density=0;
    const char *comment="";
    JPEG_CORE_PROPERTIES image;

    memset( &image, 0, sizeof( JPEG_CORE_PROPERTIES ) );

    if ( ijlInitPtr( &image ) == IJL_OK )
    {
#if 0
        if (ggetm(gx->gref,4)==0x4d4d002a) // KDC
        {
            image.JPGBytes = (unsigned char *) gx->gref + 15680;
            image.JPGSizeBytes = glen(gx->gstream) - 15680;
        }
        else
#endif
        {
            image.JPGBytes = (unsigned char *) gx->gref;
            image.JPGSizeBytes = (int) glen(gx->gstream);
        }


        if( ijlReadPtr( &image, IJL_JBUFF_READPARAMS ) == IJL_OK )
        {
            width  = image.JPGWidth;  /* Width of bitmap in pixels */
            height = image.JPGHeight; /* Height of bitmap in pixels */
            quality = image.jquality;
            subtype = image.JPGChannels;

            if (image.jprops.jfif_app0_detected)
            {
                if (image.jprops.jfif_app0_units)
                    density = image.jprops.jfif_app0_Xdensity;
            }
            if (image.jprops.jpeg_comment_size)
                comment = image.jprops.jpeg_comment;

            ok = 100;
        }

        if( ijlFreePtr( &image ) != IJL_OK )
        {
            ok = 0;
        }
    }
    if (ok)
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo,0,sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('I','J','P','G');   /* Gimex type */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;    /* Current frame */
            ginfo->width            = width;  /* Width of bitmap in pixels */
            ginfo->height           = height; /* Height of bitmap in pixels */
            ginfo->bpp              = 32;   /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = 24;   /* Bits per pixel in original image (1 to 32) */
            ginfo->numcolours       = 0;    /* Number of colours in original indexed palette */
            ginfo->subtype          = subtype; /* Internal format sub-type 0-default */
            ginfo->packed           = 1;    /* Type of packing on original image. 0 none, 1 run, n other */
            ginfo->quality          = quality;   /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = 0;    /* Number of bits in alpha channel */
            ginfo->redbits          = 8;    /* Number of bits in red channel */
            ginfo->greenbits        = 8;    /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;    /* Number of bits in blue channel */
            ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;    /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */
            ginfo->dpi              = (float) density;
            strcpy(ginfo->comment,comment);
        }
    }

    return(ginfo);
}

int GCALL IJL_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok=0;
    char *lineptr;
    int x;
    int y;
    IJLERR jerr;
    ARGB *d;

    JPEG_CORE_PROPERTIES image;

    memset( &image, 0, sizeof( JPEG_CORE_PROPERTIES ) );

    jerr = ijlInitPtr( &image );
    if( jerr == IJL_OK )
    {
        image.JPGBytes = (unsigned char *) gx->gref;
        image.JPGSizeBytes = (int) glen(gx->gstream);

        // ReadParams shouldnt be necessary, but CMYK requires it
        jerr = ijlReadPtr( &image, IJL_JBUFF_READPARAMS );
        if( jerr == IJL_OK )
        {
            image.DIBWidth    = ginfo->width;
            image.DIBHeight   = ginfo->height;
            image.DIBBytes    = (unsigned char *) dest;
            image.DIBPadBytes = 0;

            switch (image.JPGChannels)
            {
                case 1:
                    image.JPGColor    = IJL_G;
                    image.DIBChannels = 3;
                    image.DIBColor    = IJL_BGR;
                    break;

                case 3:
                    image.JPGColor    = IJL_YCBCR;
                    image.DIBChannels = 3;
                    image.DIBColor    = IJL_BGR;
                    break;

                case 4: // CMYK
                    image.JPGColor    = IJL_YCBCRA_FPX;
                    image.DIBChannels = 4;
                    image.DIBColor    = IJL_RGBA_FPX;
                    break;

                default:
                    // This catches everything else, but no
                    // color twist will be performed by the IJL.
                    image.DIBColor = (IJL_COLOR)IJL_OTHER;
                    image.JPGColor = (IJL_COLOR)IJL_OTHER;
                    image.DIBChannels = image.JPGChannels;
                    break;
            }
            image.DIBPadBytes = rowbytes-(ginfo->width*image.DIBChannels);

#define FIX_ BUG
#ifdef FIX_BUG
// 1.51 bug work around
            if (image.JPGFile == NULL)
            {
                image.JPGBytes = image.jprops.state.start_entropy_ptr;
            }
#endif


            jerr = ijlReadPtr( &image, IJL_JBUFF_READWHOLEIMAGE);
//            if( jerr == IJL_OK )  // on playa.jpg image, this fails, but still works anyway
            {
                if (image.DIBChannels==4)   // CMYK
                {
                    for (y=0; y<ginfo->height; ++y)
                    {
                        d = (ARGB *) (dest+rowbytes*y);

                        for (x=0; x<ginfo->width; ++x)
                        {
                            unsigned char a,r,g,b;
                            a = d->a;
                            r = d->b;
                            g = d->g;
                            b = d->r;
                            d->a = (unsigned char) 255;
                            d->r = (unsigned char) ((r * a + 128) >> 8);
                            d->g = (unsigned char) ((g * a + 128) >> 8);
                            d->b = (unsigned char) ((b * a + 128) >> 8);
                            ++d;
                        }
                    }
                }
                else
                {
                    for (y=0; y<ginfo->height; ++y)
                    {
                        lineptr = (dest+rowbytes*y)+(ginfo->width*3)-3;
                        d = (ARGB *) ((dest+rowbytes*y)+(ginfo->width*4)-4);

                        for (x=0; x<ginfo->width; ++x)
                        {
                            *(int *)d = (*(int *) lineptr)|0xff000000;
        //                    d->r = lineptr[2];
        //                    d->g = lineptr[1];
        //                    d->b = lineptr[0];
        //                    d->a = 255;
                            --d;
                            lineptr -= 3;
                        }
                    }
                }

                ok = 100;
            }
        }

        if( ijlFreePtr( &image ) != IJL_OK )
        {
            ok = 0;
        }
    }

    return(ok);
}

int GCALL IJL_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        if (gx->gref)
            gfree(gx->gref);

        ok = gfree((void *) gx);
    }
    IJL_freedll();

    return(ok);
}

