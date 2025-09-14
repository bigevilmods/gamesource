// (c) 1994-2007 Electronic Arts Inc.


/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  SIM - Soft Image PIC (SIM) Gimex                */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940719 FB based on tinypict, Erik's RAW format, tga2shp  */
/*    1.01 940806 FB mac version alpha channel fix                  */
/*    1.02 941027 FB bump plane size for canexport for memory fault */
/*    1.03 941028 FB write float, don't gclose, free memory         */
/*    1.04 950324 FB read raw channel fix                           */
/*    1.05 950516 FB faster reads, ginfo packed                     */
/*    1.06 950529 FB ginstance signature                            */
/*    1.07 951109 FB port to MrC; check galloc returns; comment     */
/*    1.08 960801 FB MrC if = warning fix & watcom local warning    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit only                                                      */
/* Reentrant                                                        */
/* Line buffered                                                    */
/* Files: simread.cpp simwrite.cpp simgimex.h                       */
/* Written by: Laurent Lauzon                                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* float format  seeeeeee_efffffff_ffffffff_ffffffff                */
/*                                                                  */
/* value is 1.f<<(e-127)                                            */
/*        - 1.f<<(e-127) if s=1                                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "simgimex.h"

static void DK_channelsReadRaw(GSTREAM *, char *, int, int, char **);
static void DK_channelsReadPure(GSTREAM *, char *, int, int, char **);
static void DK_channelsReadMixed(GSTREAM *, char *, int, int, char **);

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

int ggetc(GSTREAM *g)
{
    unsigned char c[4];
    int v=-1;

    if (gread(g,(void *) &c,1))
        v = (int) c[0];
    return(v);
}


DK_Picture *DK_pictureAllocate(void)
{
    DK_Picture *pic;

    pic = (DK_Picture *) galloc(sizeof(DK_Picture));
    if (pic)
    {
        memset(pic,0,sizeof(DK_Picture));
        SIMPUTFIELD(pic->width,0);
        SIMPUTFIELD(pic->height,0);
        SIMPUTFIELD(pic->ratio,0x3f800000);   /* 1.0 in float */
        SIMPUTFIELD(pic->fields,DK_NO_FIELD);
        pic->red    =  pic->ptrRed   = NULL;
        pic->green  =  pic->ptrGreen = NULL;
        pic->blue   =  pic->ptrBlue  = NULL;
        pic->alpha  =  pic->ptrAlpha = NULL;
    }
    return(pic);
}

DK_Boolean DK_pictureDispose(DK_Picture **pic)
{
    if (*pic)
    {
        if ((*pic)->red)    gfree((*pic)->red);
        if ((*pic)->green)  gfree((*pic)->green);
        if ((*pic)->blue)   gfree((*pic)->blue);
        if ((*pic)->alpha)  gfree((*pic)->alpha);

        gfree(*pic);
    }

    return(TRUE);
}

#if 0
unsigned int DK_headerReadBinary(GSTREAM *a)
{
    unsigned int id;
    char buf[4];

    gseek(a,sizeof(DK_BinaryHeader));   /* skip header */
    gread(a,buf,4);     /* 'PICT' */
    id = ggetm(buf,4);
    return(id);
}
#endif

GSTREAM *DK_pictureReadFileHeader(GSTREAM * file, DK_Picture *pic,
                  DK_channelPacket **pck)
{
    DK_channelPacket    *ptr;
    char chained;
    int ok;

    gseek(file,(GPOS)0);
    ok = gread(file,(void *) pic, 4+4+80+4+2+2+4+2+2);   /* read PIC header */
    if (SIMGETFIELD(pic->type) != GMAKEID('P','I','C','T'))
        ok = 0;

    if (!ok)
    {
        file = (GSTREAM *)0;
    }
    else
    {
        ptr = NULL;
        do
        {
            if (ptr == NULL)
                ptr = *pck = (DK_channelPacket *) galloc(sizeof(DK_channelPacket));
            else
            {
                ptr = ptr->next  = (DK_channelPacket *) galloc(sizeof(DK_channelPacket));
            }
            if (ptr)
            {
                ptr->next     = NULL;

                chained       = (char)ggetc(file);
                ptr->size     = (char)ggetc(file);
                ptr->type     = (char)ggetc(file);
                ptr->channels = (char)ggetc(file);
            }
            else
            {
                file = (GSTREAM *) 0;
                break;
            }

        } while (chained != 0);
    }
    return(file);
}

DK_Boolean DK_pictureReadScanline(GSTREAM *file, DK_Picture *pic,  DK_channelPacket *pck)
{
    DK_channelPacket    *ptr;
    char            *col[4];
    int          nbcol;
    int i;
    int ok=0;
    char *rowbuf;

    for (i=0,ptr = pck; ptr != NULL; ptr = ptr->next,++i)
    {
        nbcol = 0;
        if (ptr->channels & DK_RED_CHANNEL)   col[nbcol++] = (char *) pic->ptrRed;
        if (ptr->channels & DK_GREEN_CHANNEL) col[nbcol++] = (char *) pic->ptrGreen;
        if (ptr->channels & DK_BLUE_CHANNEL)  col[nbcol++] = (char *) pic->ptrBlue;
        if (ptr->channels & DK_ALPHA_CHANNEL) col[nbcol++] = (char *) pic->ptrAlpha;

        rowbuf = (char *) galloc(SIMGETFIELD(pic->width) * nbcol);
        if (rowbuf)
        {
            switch(ptr->type & 0x0f)
            {
                case DK_UNCOMPRESSED:
                    DK_channelsReadRaw(file, rowbuf, SIMGETFIELD(pic->width), nbcol, col);
                    break;

                case  DK_PURE_RUN_LENGTH:
                    DK_channelsReadPure(file, rowbuf, SIMGETFIELD(pic->width), nbcol, col);
                    break;

                case  DK_MIXED_RUN_LENGTH:
                    DK_channelsReadMixed(file, rowbuf, SIMGETFIELD(pic->width), nbcol, col);
                    break;
            }
            gfree(rowbuf);
            ok = 1;
        }
        else
            break;

        if (ptr->channels &  DK_RED_CHANNEL)   pic->ptrRed   += SIMGETFIELD(pic->width);
        if (ptr->channels &  DK_GREEN_CHANNEL) pic->ptrGreen += SIMGETFIELD(pic->width);
        if (ptr->channels &  DK_BLUE_CHANNEL)  pic->ptrBlue  += SIMGETFIELD(pic->width);
        if (ptr->channels & DK_ALPHA_CHANNEL)  pic->ptrAlpha += SIMGETFIELD(pic->width);
    }
    return(ok);
}

void DK_channelPacketDispose(DK_channelPacket **pck)
{
    DK_channelPacket    *ptr, *nxt;

    for (ptr = *pck; ptr != NULL; ptr = nxt)
    {
        nxt = ptr->next;
        gfree(ptr);
    }
}

DK_Picture *DK_pictureReadFile(GSTREAM *file)
{
    DK_Picture       *pic;
    DK_channelPacket *ptr, *pck;
    int i;
    int ok=1;

    pic = DK_pictureAllocate();
    if (pic)
    {
        if (DK_pictureReadFileHeader(file, pic, &pck))
        {
            for (ptr = pck; ptr != NULL && ok; ptr = ptr->next)
            {
                if (ptr->channels & DK_RED_CHANNEL)   pic->ptrRed   = pic->red   = (unsigned char *) galloc(SIMGETFIELD(pic->height) * SIMGETFIELD(pic->width));
                if (ptr->channels & DK_GREEN_CHANNEL) pic->ptrGreen = pic->green = (unsigned char *) galloc(SIMGETFIELD(pic->height) * SIMGETFIELD(pic->width));
                if (ptr->channels & DK_BLUE_CHANNEL)  pic->ptrBlue  = pic->blue  = (unsigned char *) galloc(SIMGETFIELD(pic->height) * SIMGETFIELD(pic->width));
                if (ptr->channels & DK_ALPHA_CHANNEL) pic->ptrAlpha = pic->alpha = (unsigned char *) galloc(SIMGETFIELD(pic->height) * SIMGETFIELD(pic->width));
                if (ptr->channels & DK_RED_CHANNEL   && !pic->ptrRed)   ok = 0;
                if (ptr->channels & DK_GREEN_CHANNEL && !pic->ptrGreen) ok = 0;
                if (ptr->channels & DK_BLUE_CHANNEL  && !pic->ptrBlue ) ok = 0;
                if (ptr->channels & DK_ALPHA_CHANNEL && !pic->ptrAlpha) ok = 0;
            }
            if (!ok)
                pic = 0;
            else
            {
                for (i=0; i<(int)SIMGETFIELD(pic->height); ++i)
                    DK_pictureReadScanline(file, pic, pck);
            }
            DK_channelPacketDispose(&pck);
        }
    }
    return(pic);
}

static void DK_channelsReadRaw(GSTREAM *file, char *rowbuf, int nbPix, int nbChnl, char **chnl)
{
    int    i,j;


    if (nbChnl==1)
    {
        gread(file,&chnl[0][0],nbPix);
    }
    else
    {
        gread(file,rowbuf,nbPix*nbChnl);
        for (i=0; i<nbPix; ++i)
        {
            for (j=0; j<nbChnl; ++j)
                chnl[j][i] = *rowbuf++;
        }
    }
}

static void DK_channelsReadPure(GSTREAM *file, char *rowbuf, int nbPix, int nbChnl, char **chnl)
{
    unsigned char col[4];
    unsigned char count;
    int     i, j, k, ind;

    ind = 0;
    for (i = nbPix; i > 0;)
    {
        count = (unsigned char) ggetc(file);

        if (count > nbPix)
           count = (unsigned char)nbPix;

        i -= count;

        gread(file,col,nbChnl);

        for (j=0; j<nbChnl; ++j)
           for (k = 0; k < count; k++)
              chnl[j][ind + k] = col[j];

        ind += count;
    }
}

static void DK_channelsReadMixed(GSTREAM *file, char *rowbuf, int nbPix, int nbChnl, char **chnl)
{
    int            pixcount, j, k;
    unsigned int    count;
    char            buf[2];
    char            c;
    char *rowptr;

    for (pixcount = 0; pixcount < nbPix; pixcount += count)
    {
        count = (unsigned char) ggetc(file);
        if (count >= 128)
        {
            /* int run length */
            if (count == 128)
            {
                gread(file,(void *)  &buf, 2);
                count = ggetm(buf,2);
            }
            else
            count -= 127;

            /* see if we hit garbage -> set eof and return */
            if ((int)(pixcount + count) > (int)nbPix)
            {
                gseek(file, 0);
                return;
            }

            for (j=0; j<nbChnl; ++j)
            {
                c = (char) ggetc(file);
                memset(chnl[j], c, count);
                chnl[j] += count;
            }
        }
        else
        {
            /* raw sequence */
            count++;

            /* see if we hit garbage -> set eof and return */
            if ((int)(pixcount + count) > (int)nbPix)
            {
                /* we hit garbage set eof and return */
                gseek(file, 0);
                return;
            }

            gread(file,rowbuf,count*nbChnl);
            rowptr = rowbuf;

            for (k = count; k > 0; k--)
            {
                for (j=0; j<(int)nbChnl; ++j)
                {
                    *(chnl[j]) = *rowptr++;
/*                  *(chnl[j]) = (char) ggetc(file); */
                    chnl[j]++;
                }
            }
        }
    }
}

void SIMCopyPlane(GINFO *ginfo, unsigned char *dest, unsigned char *s,int val, int rowbytes)
{
    int y;
    unsigned char *d;
    unsigned char *dend;

    if (s)
    {
        for (y=0; y<ginfo->height; ++y)
        {
            d = (dest+rowbytes*y);
            dend = d+ginfo->width*sizeof(ARGB);

            do
            {
                *d = *s;
                s += 1;
                d += 4;
            } while (d<dend);
        }
    }
    else
    {
        for (y=0; y<ginfo->height; ++y)
        {
            d = (dest+rowbytes*y);
            dend = d+ginfo->width*sizeof(ARGB);

            do
            {
                *d = (unsigned char)val;
                d += 4;
            } while (d<dend);
        }
    }
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL SIM_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','S','I','M');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('.','S','I','M');   /* mac file system types used */
        info->mactype[0]      = GMAKEID('U','N','K','N');   /* mac file system types used */
        strcpy(info->extensions[0], ".pic");                /* extensions */
        strcpy(info->extensions[1], ".sim");
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.08");                /* version # */
        strcpy(info->shorttypestr,  "Sim");                 /* type */
        strcpy(info->wordtypestr,   "SoftImage");           /* wordtype */
        strcpy(info->longtypestr,   "SoftImage PIC");      /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* id = 0x5380f634 */

/* header from team_sel.pic for soccer 3do                      */
/* 00000: 5380f634 4027ae14 446f6e65 20776974  S.v4@'..Done wit */
/* 00010: 68207468 6520534f 4654494d 41474527  h the SOFTIMAGE' */
/* 00020: 73207265 6e646572 65720000 00000000  s renderer...... */
/* 00030: 00000000 00000000 00000000 00000000  ................ */
/* 00040: 00000000 00000000 00000000 00000000  ................ */
/* 00050: 00000000 00000000 50494354 014000f0  ........PICT.@.p */
/* 00060: 3f800000 00030043 010802e0 00080210  ?......C...`.... */

int GCALL SIM_is(GSTREAM *g)
{
    unsigned char simheader[4];
    int ok=0;

    gseek(g,(GPOS)0);
	memset(simheader,0,sizeof(simheader));
	if (gread(g,(void *) simheader,4))
    {
        if (ggetm(simheader,4)==0x5380f634)
            ok = 100; /* 100% odds its a soft image pic */
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL SIM_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','S','I','M');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL SIM_info(GINSTANCE *gx, int framenum)
{
    unsigned char simheader[4];
    GINFO *ginfo        = 0;
    DK_Picture          *pic;
    DK_channelPacket    *pck;
    DK_channelPacket    *ptr;
    int  bpp            = 0;
    int  alphabits      = 0;
    int  packed         = 0;

    gseek(gx->gstream,(GPOS)0);
    if (gread(gx->gstream,(void *) simheader,4))
    {
        pic = DK_pictureAllocate();
        if (pic)
        {
            if (DK_pictureReadFileHeader(gx->gstream, pic, &pck))
            {
                bpp = 0;
                alphabits = 0;
                for (ptr = pck; ptr != NULL; ptr = ptr->next)
                {
                    packed = ((ptr->type & 0x0f)==DK_UNCOMPRESSED) ? 0 : 1;
                    if (ptr->channels & DK_RED_CHANNEL)   bpp += 8;
                    if (ptr->channels & DK_GREEN_CHANNEL) bpp += 8;
                    if (ptr->channels & DK_BLUE_CHANNEL)  bpp += 8;
                    if (ptr->channels & DK_ALPHA_CHANNEL)
                    {
                        bpp += 8;
                        alphabits = 8;
                    }
                }

                DK_channelPacketDispose(&pck);


                ginfo = (GINFO *) galloc(sizeof(GINFO));
                if (ginfo)
                {
                    memset(ginfo,0,sizeof(GINFO));

                    /* template for ginfo */

                    ginfo->signature        = GMAKEID('.','S','I','M');   /* Gimex type */
                    ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
                    ginfo->version          = GIMEX_VERSION;
                    ginfo->framenum         = 0;    /* Current frame */
                    ginfo->bpp              = 32;   /* Bits per pixel (8 or 32) */
                    ginfo->originalbpp      = bpp;  /* Bits per pixel in original image (1 to 32) */
                    ginfo->numcolours       = 0;    /* Number of colours in original indexed palette */
                    ginfo->subtype          = 0;    /* Internal format sub-type 0-default */
                    ginfo->packed           = packed; /* Type of packing on original image. 0 none, 1 run, n other */
                    ginfo->quality          = 100;    /* Quality of lossy packing 0..100 */
                    ginfo->alphabits        = alphabits;    /* Number of bits in alpha channel */
                    ginfo->redbits          = 8;    /* Number of bits in red channel */
                    ginfo->greenbits        = 8;    /* Number of bits in grreen channel */
                    ginfo->bluebits         = 8;    /* Number of bits in blue channel */
                    ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
                    ginfo->centery          = 0;
                    ginfo->defaultx         = 0;    /* Default Coordinate Point */
                    ginfo->defaulty         = 0;
                    ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */
                    ginfo->width  = SIMGETFIELD(pic->width);   /* Width of bitmap in pixels */
                    ginfo->height = SIMGETFIELD(pic->height);  /* Height of bitmap in pixels */
                    strcpy(ginfo->comment,pic->comment);       /* Null terminated user comment */
                }
            }
            DK_pictureDispose(&pic);
        }
    }
    return(ginfo);
}

int GCALL SIM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok=0;
    ARGB *d;
    DK_Picture *pic;

    pic = DK_pictureReadFile(gx->gstream);
    if (pic)
    {
        d = (ARGB *) dest;
        if (ginfo->width)
        {
            SIMCopyPlane(ginfo, &d->a, pic->alpha, 255, rowbytes);
            SIMCopyPlane(ginfo, &d->r, pic->red,   0,   rowbytes);
            SIMCopyPlane(ginfo, &d->g, pic->green, 0,   rowbytes);
            SIMCopyPlane(ginfo, &d->b, pic->blue,  0,   rowbytes);
            ok = 1;
        }
        DK_pictureDispose(&pic);
    }
    return(ok);
}

int GCALL SIM_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

