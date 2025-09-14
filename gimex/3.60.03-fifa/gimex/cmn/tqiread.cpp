// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                TQI - Transformed Quantized Image                 */
/*                                                                  */
/*    by Martin Griffiths, Patrick Ratto & FrANK G. Barchard, EAC   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 990424 FB First draft based on tqilib, based on uV       */
/*    1.01 990502 FB Exporter, relaxed istqi function               */
/*    1.02 990723 LC included the decode functions                  */
/*    1.03 010312 FB 444 extension removed... too confusing         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Reentrant                                                        */
/* Files: tqiread.cpp tqiwrite.cpp tqigimex.h tqi_lib.lib               */
/*                                                                  */
/* subtype 0=rgb, 1=yuv                                             */
/*                                                                  */
/*		TQI444 - Transformed Quantized Image Decoder Library        */
/*                                                                  */
/*		Martin Griffiths,                                           */
/*                                                                  */
/*		(c) Electronic Arts / Bullfrog Productions Ltd, 1997-99     */
/*                                                                  */
/*		Last updated : 12th April 1999.                             */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* EAUK texture compression using DCT                               */
/* 32 bpp only                                                      */
/* Litte endian fields                                              */
/*                                                                  */
/* Example Image:                                                   */
/* --------------                                                   */
/* 00000: 4001f000 64ff00ff 06000161 e8001980  @.p.d......ah...     */
/*                                                                  */
/* 00000: 8002e001 e4ff00ff 03d4000a 5c001830  ..`.d....T..\..0     */
/*       width height masks                                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "tqigimex.h"
#include "tqi444decode.h"

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL TQI_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','T','Q','I');
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
        info->defaultquality  = 90;   /* default pack quality */
        info->mactype[0]      = GMAKEID('.','T','Q','I');   /* mac file system types used */
        strcpy(info->extensions[0], ".tqi");                /* extensions */
//        strcpy(info->extensions[1], ".444");                /* extensions */
        strcpy(info->authorstr,     "Martin Griffiths");   /* author */
        strcpy(info->versionstr,    "1.03");                /* version # */
        strcpy(info->shorttypestr,  "TQI");                 /* type */
        strcpy(info->wordtypestr,   "TQI");              /* wordtype */
        strcpy(info->longtypestr,   "TQI Texture Quantized Image"); /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* use 64ff00ff at offset 4. 1 byte flag, 3 byte chromakey */
/* 00000: 4001f000 64ff00ff 06000161 e8001980  @.p.d......ah...     */
/*       wide high flag chrome */
/* */
/* x11x010x */
/* a  y   c */
/* */
/*	output[4] = 100 */
/*   | (((flags & TQI444_ALPHA) ? 1 : 0) << 7)      128 */
/*   | 	((flags & TQI444_COLOURKEY) ? 0 : 1)        1 */
/*	 | (((flags & TQI444_YUV) ? 1 : 0) << 4);       16 */


int GCALL TQI_is(GSTREAM *g)
{
    unsigned char tqiheader[8];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,tqiheader,8))
    {
        int flag = ggetm(tqiheader+4,1);

        if (((flag&0x6e)==0x64)
         && (((flag&1) && ggetm(tqiheader+5,3)==0) || (!(flag&1) && ggetm(tqiheader+5,3)==0xff00ff))
         && ggeti(tqiheader+0,2)>=8 && ggeti(tqiheader+0,2)<=8192 /* width */
         && ggeti(tqiheader+2,2)>=8 && ggeti(tqiheader+2,2)<=8192) /* height */
            ok = 90;    /* likely a tqi */
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL TQI_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','T','Q','I');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL TQI_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    int  width;
    int  height;
    int bpp;
    int alphabits;
	int subtype;
    unsigned char tqiheader[8];

    gseek(gx->gstream,(GPOS)0);
    if (gread(gx->gstream,tqiheader,8))
    {
        width = ggeti(tqiheader+0,2);
        height = ggeti(tqiheader+2,2);
        bpp = 24;
        alphabits = 0;
        if (ggetm(tqiheader+4,4)==0xe4ff00ff)
        {
            bpp = 32;
            alphabits = 8;
        }
		subtype = (ggeti(tqiheader+4,1) >> 4) & 1;

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','T','Q','I'); /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;                        /* Number of frames (if more than 1, openframeblt required) */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */
            ginfo->subtype          = subtype;                  /* subtype = 0 for ARGB, = 1 for YUV */
            ginfo->packed           = 1;
            ginfo->quality          = 90;           /* Quality of lossy packing 0..100 */
            ginfo->bpp              = 32;           /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = 32;
            ginfo->numcolours       = 0;            /* Number of colours in original indexed palette */
            ginfo->alphabits        = 8;            /* Number of bits in alpha channel */
            ginfo->redbits          = 8;            /* Number of bits in red channel */
            ginfo->greenbits        = 8;            /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;            /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;            /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */
        }
    }

    return(ginfo);
}

int GCALL TQI_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int  width;
    int  height;
    GPOS framesize;
    void *compresseddata;
    int  ok=0;

    gseek(gx->gstream,(GPOS)0);
    framesize = glen(gx->gstream);
    compresseddata = galloc(framesize);
    if (compresseddata)
    {
        ok = gread(gx->gstream, compresseddata, framesize);  /* read the entire file */
        if (ok)
        {
            tqi444_set_pixeldecode_format(0x00ff0000,0x0000ff00,0x000000ff,0xff000000);

            tqi444_decode_header((unsigned char *)compresseddata, &width, &height);

            /* tqi444_decode_image decompresses the compressed image data(src) into the destination buffer(dst), */
            /* with the specified line stride, width and height. */

            tqi444_decode_image((unsigned char *)compresseddata,(unsigned char *) dest, rowbytes, width, height);
        }
        gfree(compresseddata);
    }
    return(ok);
}

int GCALL TQI_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

