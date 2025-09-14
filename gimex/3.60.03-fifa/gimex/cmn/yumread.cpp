// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    YUM - YU yMe Shape Gimex                      */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 010718 FB adapted to YUV ACOM gimex                      */
/*    1.01 010817 FB YUY2 version                                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit only                                                      */
/* Reentrant                                                        */
/* Frame buffered                                                   */
/* Files: yumread.cpp yumwrite.cpp yumgimex.h                           */
/*                                                                  */
/* A quick YUV shape gimex                                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/*                                                                  */
/* RGB -> YCbCr (with CCIR 601-1 specs)                             */
/* Y=  0.2990*Red+0.5870*Green+0.1140*Blue                          */
/* Cb=-0.1687*Red-0.3313*Green+0.5000*Blue                          */
/* Cr= 0.5000*Red-0.4187*Green-0.0813*Blue                          */
/*                                                                  */
/* YCbCr -> RGB                                                     */
/* Red=  Y+0.0000*Cb+1.4020*Cr                                      */
/* Green=Y-0.3441*Cb-0.7141*Cr                                      */
/* Blue= Y+1.7720*Cb+0.0000*Cr                                      */
/*                                                                  */
/* I assume Y is within the range [0;1], and Cb and Cr are within   */
/* the range  [-0.5;0.5].                                           */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include <stdlib.h>
#include "gimex.h"
#include "yumgimex.h"

/****************************************************************/
/*  YUM to RGB                                                  */
/****************************************************************/

static void yumtorgb(YUMCOORD *yum, YUMCOORD *rgb)
{
    int Y;
    int Cb;
    int Cr;
    int Red;
    int Green;
    int Blue;

    Y =  (yum->x);
    Cb = (yum->y-128);
    Cr = (yum->z-128);

    Red    = (int)gmax(0,gmin(255,Y+0.0000f*Cb+1.4020f*Cr));
    Green  = (int)gmax(0,gmin(255,Y-0.3441f*Cb-0.7141f*Cr));
    Blue   = (int)gmax(0,gmin(255,Y+1.7720f*Cb+0.0000f*Cr));

    rgb->x = Red;
    rgb->y = Green;
    rgb->z = Blue;
}

static void YUMdecode(ARGB *d,unsigned int y,unsigned int cb,unsigned int cr)
{
    YUMCOORD rgb;
    YUMCOORD yum;



    yum.x = y;   /* y */
    yum.y = (int)(unsigned char)cb;  /* cb */
    yum.z = (int)(unsigned char)cr;  /* cr */
    yumtorgb(&yum,&rgb);
    d->a = (unsigned char) 255;
    d->r = (unsigned char) rgb.x;     /* r */
    d->g = (unsigned char) rgb.y;     /* g */
    d->b = (unsigned char) rgb.z;     /* b */
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL YUM_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('Y','U','Y','M');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->dotsubtype      = 1;    /* subtype based on extension */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('Y','U','Y','M');   /* mac file system types used */
        strcpy(info->extensions[0], ".yum");                /* extensions */
        strcpy(info->extensions[1], ".yui");                /* intel version */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.00");                /* version # */
        strcpy(info->shorttypestr,  "YUM");                 /* type */
        strcpy(info->wordtypestr,   "YUYMe");               /* wordtype */
        strcpy(info->longtypestr,   "YUYMe Shape");         /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* shape type and file size match width*height */

int GCALL YUM_is(GSTREAM *g)
{
    unsigned char yumheader[8];
    int width;
    int height;
    int flen;
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,yumheader,8))
    {
        flen = (int) glen(g);

        width = ggetm(yumheader+4,2);
        height = ggetm(yumheader+6,2);
        if (ggetm(yumheader,4)==0x1c000000
          && flen==16+width*height*3/2)
            ok = 100;
        else
        {
            width = ggeti(yumheader+4,2);
            height = ggeti(yumheader+6,2);
            if (ggetm(yumheader,4)==0x6c000000
              && flen==16+width*height*3/2)
                ok = 100;
        }
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL YUM_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('A','Y','U','V');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL YUM_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    int width;
    int height;
    unsigned char yumheader[16];
    int isintel=0;
    int ok=0;

    gseek(gx->gstream,(GPOS)0);
    ok = gread(gx->gstream, (char *) &yumheader, 16);   /* read header */
    if (ok)
    {
        if (yumheader[0]==0x1c)
        {
            width = ggetm(yumheader+4,2);
            height = ggetm(yumheader+6,2);
        }
        else
        {
            width = ggeti(yumheader+4,2);
            height = ggeti(yumheader+6,2);
        }

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo,0,sizeof(GINFO));

            ginfo->signature        = GMAKEID('Y','U','Y','M'); /* Gimex type */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;
            ginfo->width            = width;
            ginfo->height           = height;
            ginfo->bpp              = 32;   /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = 24;   /* Bits per pixel in original image (1 to 32) */
            ginfo->numcolours       = 0;    /* Number of colours in original indexed palette */
            ginfo->subtype          = 0;    /* Internal format sub-type 0-default */
            ginfo->packed           = 0;    /* Type of packing on original image. 0 none, 1 run, n other */
            ginfo->quality          = 100;  /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = 0;    /* Number of bits in alpha channel */
            ginfo->redbits          = 2;    /* Number of bits in red channel */
            ginfo->greenbits        = 8;    /* Number of bits in grreen channel */
            ginfo->bluebits         = 2;    /* Number of bits in blue channel */
            ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;    /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */
        }
    }
    return(ginfo);
}

int GCALL YUM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    int ok=0;
    unsigned char *Yptr;
    unsigned char *Cbptr;
    unsigned char *Crptr;
    unsigned char *yumbuf;

    ARGB *d;

    gseek(gx->gstream,(GPOS)16);
    yumbuf = (unsigned char *)galloc((int)glen(gx->gstream)-16);

    if (yumbuf)
    {
        ok = gread(gx->gstream,yumbuf,(int)glen(gx->gstream)-16);
        if (ok)
        {
            for (y=0; y<ginfo->height; y+=2)
            {
                d = (ARGB *) (dest+rowbytes*y);
                Yptr = yumbuf+ginfo->width*y;
                Cbptr = yumbuf+ginfo->width*ginfo->height    +ginfo->width/2*y/2;
                Crptr = yumbuf+ginfo->width*ginfo->height*5/4+ginfo->width/2*y/2;

                for (x=0; x<ginfo->width; x+=2)
                {
                    YUMdecode(d+0,Yptr[0],Cbptr[0],Crptr[0]);
                    YUMdecode(d+1,Yptr[1],Cbptr[0],Crptr[0]);
                    YUMdecode(d+ginfo->width+0,Yptr[ginfo->width+1],Cbptr[0],Crptr[0]);
                    YUMdecode(d+ginfo->width+1,Yptr[ginfo->width+1],Cbptr[0],Crptr[0]);

                    d+=2;
                    Yptr += 2;
                    Cbptr += 1;
                    Crptr += 1;
                }
            }
        }
        gfree(yumbuf);
    }

    return(ok);
}

int GCALL YUM_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

