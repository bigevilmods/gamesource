// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                   BMP - Windows/OS2 BMP Gimex                    */
/*                                                                  */
/*             by FrANK G. Barchard, & Eric Pauker EAC              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940513 EP Based on latenight bmp module                  */
/*    1.01 940513 FB Fixed line alignment                           */
/*    1.02 940525 FB 24 bpp, infobmp, openbmp sets colour table     */
/*    1.03 940526 FB 1 bpp, 3do                                     */
/*    1.04 940531 FB Gimex 1.03 spec                                */
/*    1.05 940603 FB 1&4 bit bug fixs, OS2                          */
/*    1.06 940606 FB unpack                                         */
/*    1.07 940614 FB updated to Gimex 1.5                           */
/*    1.08 940628 FB write matches to closest depth                 */
/*    1.09 941001 FB casting for SGI                                */
/*    1.10 941224 FB infobmp was allocing 2 ginfos                  */
/*    1.11 950529 FB ginstance signature                            */
/*    1.12 961118 FB 16,32 bit image, 32 bit palette                */
/*    1.13 970205 FB 16 set info rgb bits                           */
/*    1.14 970422 FB 15 bpp import                                  */
/*    1.15 970527 FB subtype optional                               */
/*    1.16 970801 KO Fixed 15 bit export.                           */
/*    1.17 970807 KO subtype 2=WIN 95, 3=WIN NT, uses bitmasks      */
/*    1.18 970822 KO use different header if included by clpgimex   */
/*    1.19 970828 KO Puts numcolours in header->biClrUsed           */
/*    1.20 971118 KO 16bit masks use low 16 bits                    */
/*    1.21 971209 KO Subtype 4 = win3.1 565                         */
/*    1.22 990401 KO Any alpha rounded up to 32bpp on export        */
/*    1.23 990813 FB RLE8 export, dpi                               */
/*    1.24 990830 FB export limit rle to 255 long (bug fix)         */
/*    1.25 000901 FB dropped rle extension, which confused utah rle */
/*    1.26 020121 AE auto-detect presence of alpha info in cluts    */
/*    1.27 020930 FB rle/4 bpp set size of BMP correctly in 2 pass  */
/*    1.28 030625 FB set RLE estimation in const and use for memset */
/*    1.29 030704 FB importer set RLE estimation in const too       */
/*    1.30 040110 FB clamp colours to 256 maximum                   */
/*    1.31 040902 FC V5 header and RLE4 line overflow check         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 1,4,8,16,24,32 bit                                               */
/* Reentrant                                                        */
/* read supports unpacking.  write only supports windows unpacked,  */
/* except for 8 bit rle                                             */
/* 16 and 32 unpacked import/export                                 */
/*                                                                  */
/* Subtype of 1 uses OS2 header                                     */
/* Subtype of 2 uses WIN 95 header                                  */
/* Subtype of 3 uses WIN NT header                                  */
/* Subtype of 4 uses WIN 3.1 header but 565 data if 16bit           */
/*      WIN 95 format allows 555,565,888,8888                       */
/*      WIN NT format allows any argb bit values up to 8888         */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* cluts (<=8 bpp) in BMP version >= win3.1 up to win95 always have */
/* 4 entries per colour. One each of R,G,B and the 4th is a filler  */
/* byte, which is skipped by bmp readers. This filler can be used   */
/* as alpha information, (ie. for transparency), if a BMP reader    */
/* can support this. If all alphas are 0, or all are 255, we don't	*/
/* have any alpha information. Otherwise, we have alpha and assign  */
/* 8 alphabits.                                                     */
/*                                                                  */
/* Files: bmpread.cpp bmpwrite.cpp bmpgimex.h                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* BMP files support 1,4,8,16,24,32 bpp depths.                     */
/* 16 and 32 are supported by win95 for desktop wallpaper. (unpack) */
/* Windows and OS2 headers.                                         */
/* Runlength compression (usually for 4 and 8 bit Windows format).  */
/* Resolution is measured in dots per meter.                        */
/* The conversion from inches to meters is 39.370079                */
/*                                                                  */
/* RLE encoding                                                     */
/* n val        run of n val's                                      */
/* 0 n [data]   literal of n bytes of data                          */
/* 0 0          end of line                                         */
/* 0 1          end of file                                         */
/* 0 2 x y      offset position by x,y                              */
/*                                                                  */
/* BPP  Compression field                                           */
/*   1   3                                                          */
/*   4   2                                                          */
/*   8   1                                                          */
/*  24   4                                                          */
/*                                                                  */
/*  http://www.daubnet.com/formats/BMP.html                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "bmpgimex.h"

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

static int BMPmaskoffset(unsigned int mask)
{
    int result=0;

    if (mask)
    {
        while(!(mask&1))
        {
            result++;
            mask = mask>>1;
        }
    }
    return(result);
}

static int BMPreadline(GSTREAM *g,unsigned char *d,int dbytes, unsigned char *linebuf, int bpp, GINFO *ginfo, unsigned int amask, unsigned int rmask, unsigned int gmask, unsigned int bmask)
{
    int width;
    int abpp;
    int packed;
    int rowbytes;
    int ok=0;
    int x=0;
    int i;
    int count;
    int bytesread=0;
    unsigned char *s=linebuf;

    unsigned char *dmax = d + dbytes - 1;   // dmax == the last byte that can be safely written to

    width   = ginfo->width;
    packed  = ginfo->packed;
    abpp = bpp;
    if (abpp==15) abpp = 16;

    if (!packed)        /* uncompressed images */
    {
        count = width;
        rowbytes = ((count*abpp+31)&~31)>>3;
        ok = gread(g, s, rowbytes); bytesread+=rowbytes;

        switch (bpp)
        {
            case 1:     /* uncompressed 1 bit indexed image */
                for (i=0; i<count; ++i)
                {
                    *d++ = (*s>>(7-(i&7)))&1;
                    if ((i&7)==7)
                        ++s;
                }
                break;

            case 4:     /* uncompressed 4 bit indexed image */
                for (i=0; i<count; ++i)
                {
                    if (i&1)
                        *d++ = *s++&15;
                    else
                        *d++ = *s>>4;
                }
                break;

            case 8:     /* uncompressed 8 bit indexed image */
                for (i=0; i<count; ++i)
                {
                    *d++ = *s++;
                }
                break;

            case 24:    /* uncompressed 24 bit direct image */
                for (i=0; i<count; ++i)
                {
                    ((ARGB *) d)->a = 255;
                    ((ARGB *) d)->r = s[2];
                    ((ARGB *) d)->g = s[1];
                    ((ARGB *) d)->b = s[0];
                    d += 4;
                    s += 3;
                }
                break;

            case 15:
            case 16:
            case 32:    /* uncompressed 32 bit direct image */
            {
                int aoffset;
                int roffset;
                int goffset;
                int boffset;
                int acolours = 1<<ginfo->alphabits;
                int rcolours = 1<<ginfo->redbits;
                int gcolours = 1<<ginfo->greenbits;
                int bcolours = 1<<ginfo->bluebits;
                int sinc;

                if (bpp <= 16)
                {
                    /*
                     * Only advance source pointer by 2 bytes and only need 16 bit mask
                     */
                    sinc = 2;
                }
                else
                    sinc = 4;

                aoffset = BMPmaskoffset(amask);
                roffset = BMPmaskoffset(rmask);
                goffset = BMPmaskoffset(gmask);
                boffset = BMPmaskoffset(bmask);

                for (i=0; i<count; ++i)
                {
                    if (amask)
                    {
                        if (acolours != 1)
                            ((ARGB *) d)->a = (unsigned char)( (((*((unsigned int*)s) & amask) >> aoffset) * 255 + (acolours>>1)) / (acolours-1) );
                        else
                            ((ARGB *) d)->a = (unsigned char)( ((*((unsigned int*)s) & amask) >> aoffset) * 255 );
                    }
                    else
                        ((ARGB *) d)->a = 0xff;

                    ((ARGB *) d)->r = (unsigned char)( (((*((unsigned int*)s) & rmask) >> roffset) * 255 + (rcolours>>1)) / (rcolours - 1) );
                    ((ARGB *) d)->g = (unsigned char)( (((*((unsigned int*)s) & gmask) >> goffset) * 255 + (gcolours>>1)) / (gcolours - 1) );
                    ((ARGB *) d)->b = (unsigned char)( (((*((unsigned int*)s) & bmask) >> boffset) * 255 + (bcolours>>1)) / (bcolours - 1) );
                    d += 4;
                    s += sinc;
                }
                break;
            }
        }
    }
    else                /* run length compressed images */
    {
        unsigned int r;
        int maxcount;

        do
        {
            ok = gread(g, s, 1); bytesread+=1;
            count = (int) *s;
            s = linebuf;

            if (count)          /* repeating colours */
            {
                x += count;

                if (d + count - 1 > dmax)
                {
                    maxcount = dmax - d + 1;
                }
                else
                {
                    maxcount = count;
                }

//printf("run %d\n",count);

                ok = gread(g, s, 1); bytesread+=1;
                r = (unsigned int) *s;

                switch (bpp)
                {
                    case 1:     /* compressed 1 bit indexed image */
                        for (i=0; i<maxcount; ++i)
                        {
                            *d++ = ((unsigned char) r>>(7-(i&7)))&1;
                        }
                        break;

                    case 4:     /* compressed 4 bit indexed image */
                        for (i=0; i<maxcount; ++i)
                        {
                            if (i&1)
                                *d++ = (unsigned char) r&15;
                            else
                                *d++ = (unsigned char) r>>4;
                        }
                        break;

                    case 8:     /* compressed 8 bit indexed image */
                        for (i=0; i<maxcount; ++i)
                        {
                            *d++ = (unsigned char) r;
                        }
                        break;

                    case 24:    /* compressed 24 bit direct image */
                        ok = gread(g, s+1, 2); bytesread+=2;
                        r = (unsigned int) (ggeti(s,3)|0xff000000);
                        for (i=0; i<maxcount; ++i)
                        {
                            *((unsigned int *) d) = r;
                            d += 4;
                        }
                        break;
                }
            }
            else                /* literal colours */
            {
                ok = gread(g, s, 1); bytesread+=1;
                count = (int) *s;

                if (count<3)    /* 0=end row, 1=end bitmap, 2 movetoxy */
                {
                    if (x==0)
                    {
//printf("end of row\n",count);
                        continue;
                    }
                    else
                    {
//printf("movexy\n");
                        x = width;
                        break;
#if 0
                        if (bpp<=8)
                            count = width-x;
                        else
                            count = (width-x)*4;
/*                        memset(d,0,count); */
                        x = width;
                        break;
#endif
                    }
                }

//printf("lit %d\n",count);

                x += count;
                rowbytes = ((count*bpp+15)&~15)>>3;
                ok = gread(g, s, rowbytes); bytesread+=rowbytes;
                switch (bpp)
                {
                    case 1:     /* literal 1 bit indexed image */
                        for (i=0; i<count; ++i)
                        {
                            *d++ = (*s>>(7-(i&7)))&1;
                            if ((i&7)==7)
                                ++s;
                        }
                        break;

                    case 4:     /* literal 4 bit indexed image */
                        for (i=0; i<count; ++i)
                        {
                            if (i&1)
                                *d++ = *s++&15;
                            else
                                *d++ = *s>>4;
                        }
                        break;

                    case 8:     /* literal 8 bit indexed image */
                        for (i=0; i<count; ++i)
                        {
                            *d++ = *s++;
                        }
                        break;

                    case 24:    /* literal 24 bit direct image */
                        for (i=0; i<count; ++i)
                        {
                            ((ARGB *) d)->a = 255;
                            ((ARGB *) d)->r = s[2];
                            ((ARGB *) d)->g = s[1];
                            ((ARGB *) d)->b = s[0];
                            d += 4;
                            s += 3;
                        }
                        break;
                }

            }
       } while (x<width);
    }
    if (bytesread&3)
        gread(g,s,(-bytesread)&3);
    return(ok);
}

static int BMPcountbits(unsigned int mask)
{
    int result=0;

    if (mask)
    {
        while(!(mask&1))
            mask = mask>>1;

        while(mask&1)
        {
            mask = mask>>1;
            result++;
        }
    }

    return(result);
}


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL BMP_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','B','M','P');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
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
        info->mactype[0]      = GMAKEID('B','I','N','A');
        info->mactype[1]      = GMAKEID('.','B','M','P');   /* mac file system types used */
        strcpy(info->extensions[0], ".bmp");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.31");                /* version # */
        strcpy(info->shorttypestr,  "BMP");                 /* type */
        strcpy(info->wordtypestr,   "BMP");                 /* wordtype */
        strcpy(info->longtypestr,   "Windows BMP");         /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* 'BM' id */

int GCALL BMP_is(GSTREAM *g)
{
    char bmpbuf[2];
    int ok=0;

    gseek(g, 0);
    if (gread(g, bmpbuf, 2))
    {
        if (bmpbuf[0]=='B' && bmpbuf[1]=='M')
            ok = 100;    /* it's a bitmap. */
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL BMP_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','B','M','P');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL BMP_info(GINSTANCE *gx, int framenum)
{
    BMPHeaderStruct bmpheader;
    unsigned char pal[256*4];
    GINFO *ginfo      = 0;
    int bpp           = 8;
    int colours       = 0;
    int i             = 0;
    int subtype       = 0;
    int width         = 0;
    int height        = 0;
    int packed        = 0;
    int headersize    = 0;
    int ntheadersize  = 0;
    int dpm           = 0;
    int ok            = 0;
    unsigned int biRedMask   = 0;
    unsigned int biGreenMask = 0;
    unsigned int biBlueMask  = 0;
    unsigned int biAlphaMask = 0;

    gseek(gx->gstream,(GPOS)0);
    memset(&bmpheader, 0, sizeof(BMPHeaderStruct));

#ifdef BMPCLIPBOARD
    ok = gread(gx->gstream, (char *) &bmpheader+14, 4);   /* read only biSize, no common header */
#else
    ok = gread(gx->gstream, (char *) &bmpheader, 14+4);   /* read common header + biSize */
#endif

    if (ok)
    {
        headersize = BMPGETFIELD(bmpheader.bmi.win.biSize);
        ntheadersize = 0;

        ok = gread(gx->gstream, (char *) &bmpheader+(14+4), headersize-4);  /* read rest of windows headers */

        if (headersize==12)
        {
            subtype = BMPOS2;    /* os2 */
            width   = BMPGETFIELD(bmpheader.bmi.os2.bcWidth);      /* Width of bitmap in pixels */
            height  = BMPGETFIELD(bmpheader.bmi.os2.bcHeight);     /* Height of bitmap in pixels */
            bpp     = BMPGETFIELD(bmpheader.bmi.os2.bcBitCount);   /* Colour bits per pixel */
            colours = 1<<bpp;                                  /* Number of colours */
            packed = 0;
        }
        else
        {
            subtype = BMPWIN31;    /* windows */
            width   = BMPGETFIELD(bmpheader.bmi.win.biWidth);         /* Width of bitmap in pixels */
            height  = BMPGETFIELD(bmpheader.bmi.win.biHeight);        /* Height of bitmap in pixels */
            bpp     = BMPGETFIELD(bmpheader.bmi.win.biBitCount);      /* Colour bits per pixel */
            colours = BMPGETFIELD(bmpheader.bmi.win.biClrUsed);       /* Number of colours */
            packed  = BMPGETFIELD(bmpheader.bmi.win.biCompression);   /* Compression scheme (0 for none, 3 for nt/95) */
            dpm     = BMPGETFIELD(bmpheader.bmi.win.biXPelsPerMeter); /* Dots per Meter */
            if (packed==3 && headersize==40)
            {
                subtype = BMPWINNT;    /* nt */
                ntheadersize = 12;
                ok = gread(gx->gstream, (char *) &bmpheader+(14+headersize), ntheadersize);  /* read rest of nt header */
            }
            if (headersize==108)
            {
                subtype = BMPWIN95;    /* 95 */
            }

            biRedMask =   BMPGETFIELD(bmpheader.bmi.win.biRedMask);
            biGreenMask = BMPGETFIELD(bmpheader.bmi.win.biGreenMask);
            biBlueMask =  BMPGETFIELD(bmpheader.bmi.win.biBlueMask);
            biAlphaMask = BMPGETFIELD(bmpheader.bmi.win.biAlphaMask);
        }
    }
    if (height < 0)
        height = -height;
    if (ok)
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','B','M','P');   /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;                        /* Number of frames (if more than 1, openframeblt required) */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */
            ginfo->subtype          = subtype;                  /* 0 windows, 1 os2, 2 win 95, 3 win NT */
            ginfo->packed           = packed==1||packed==2;
            ginfo->quality          = 100;      /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = 0;        /* Number of bits in alpha channel */
            ginfo->redbits          = 8;        /* Number of bits in red channel */
            ginfo->greenbits        = 8;        /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;        /* Number of bits in blue channel */
            ginfo->centerx          = 0;        /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;        /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;        /* Number of Hot Spots defined */
            ginfo->dpi              = (float)dpm/39.370079f;
            ginfo->framename[0]     = 0;        /* Null terminated name of frame/image */

            if (bpp>8)
            {
                ginfo->bpp              = 32;   /* Bits per pixel (8 or 32) */
                ginfo->originalbpp      = bpp;
                ginfo->numcolours       = 0;    /* Number of colours in original indexed palette */

                if (subtype<2)
                {
                    if (bpp==16)
                    {
                        ginfo->originalbpp = 15;
                        ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 5;
                    }
                }
                else        /* Use mask */
                {
                    if ( (bpp == 32) && (subtype == BMPWIN95) )
                        ginfo->alphabits = BMPcountbits(biAlphaMask);
                    else
                    ginfo->alphabits = 0;
                    ginfo->redbits   = BMPcountbits(biRedMask);
                    ginfo->greenbits = BMPcountbits(biGreenMask);
                    ginfo->bluebits  = BMPcountbits(biBlueMask);
                }
            }
            else
            {
                if (!bpp)        bpp = 8;
                if (!colours)    colours = (1 << bpp);
                if (colours>256) colours = 256;

                if (colours)
                {
                    if (subtype==BMPOS2)    /* os2 */
                    {
                        if (ok)
                            ok = gread(gx->gstream, (char *) pal, colours*3);
                        for (i=0; i<colours; ++i)
                        {
                            ginfo->colourtbl[i].a = 255;
                            ginfo->colourtbl[i].r = pal[i*3+2];
                            ginfo->colourtbl[i].g = pal[i*3+1];
                            ginfo->colourtbl[i].b = pal[i*3+0];
                        }
                    }
                    else            /* windows */
                    {
                        bool alpha_allzero=true; // use to determine if the alphas are all 0
                        bool alpha_all255=true;  // use to determine if the alphas are all 255
                        if (ok)
                            ok = gread(gx->gstream, (char *) pal, colours*4);
                        for (i=0; i<colours; ++i)
                        {
                            ginfo->colourtbl[i].a = pal[i*4+3];
                            ginfo->colourtbl[i].r = pal[i*4+2];
                            ginfo->colourtbl[i].g = pal[i*4+1];
                            ginfo->colourtbl[i].b = pal[i*4+0];

                            if (ginfo->colourtbl[i].a)
                                alpha_allzero=false;

                            if(ginfo->colourtbl[i].a != 255)
                                alpha_all255=false;

                        }

                        if (alpha_allzero)          // if all alpha bits are zero, means we have
                        {                           // no alpha information, since don't want totally
                            for(i=0;i<colours;i++)  // transparent image. Set alphas to completely opaque
                                ginfo->colourtbl[i].a = 255;

                            ginfo->alphabits    = 0; /* Number of bits in alpha channel */
                        }
                        else if(alpha_all255)       // all alphas are 255, so no alpha information
                        {
                            ginfo->alphabits    = 0;    /* Number of bits in alpha channel */

                        }
                        else        // we have alpha information
                            ginfo->alphabits    = 8;

                    }
                }
                ginfo->bpp              = 8;                        /* Bits per pixel (8 or 32) */
                ginfo->originalbpp      = bpp;
                ginfo->numcolours       = colours;                     /* Number of colours in original indexed palette */
            }
        }
        if (ginfo && !ok)
        {
            gfree((char *)ginfo);
            ginfo = 0;
        }
    }

    return(ginfo);
}

int GCALL BMP_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    char *linebuf             = 0;
    char *linebase            = 0;
    int y                     = 0;
    int width                 = 0;
    int height                = 0;
    int subtype               = 0;
    int bpp                   = 0;
    int abpp                  = 0;
    int colours               = 0;
    int packed                = 0;
    int offset                = 0;
    int ok                    = 0;
    BMPHeaderStruct bmpheader;
    unsigned int biRedMask    = 0;
    unsigned int biGreenMask  = 0;
    unsigned int biBlueMask   = 0;
    unsigned int biAlphaMask  = 0;
#ifdef BMPCLIPBOARD
    int cmnheadersize = 0;
#else
    int cmnheadersize = 14;
#endif

    colours = ginfo->numcolours;
    subtype = ginfo->subtype;
    packed  = ginfo->packed;
    width   = ginfo->width;
    height  = ginfo->height;

    gseek(gx->gstream,(GPOS)0);
    if (subtype >= BMPWIN95)       /* Win NT or WIN 95 */
    {
        ok = gread(gx->gstream, (char *) &bmpheader+14-cmnheadersize, cmnheadersize + 56);   /* read common header + win + masks */
    }
    else
    {
        ok = gread(gx->gstream, (char *) &bmpheader+14-cmnheadersize, cmnheadersize + 12);   /* read common header + os2 */
    }

    if (ok)
    {

        bpp     = ginfo->originalbpp;

        /*
         * If either NT or 95 format then get bitmask from file
         */
        if (subtype >=2)
        {
            biRedMask   = BMPGETFIELD(bmpheader.bmi.win.biRedMask);
            biGreenMask = BMPGETFIELD(bmpheader.bmi.win.biGreenMask);
            biBlueMask  = BMPGETFIELD(bmpheader.bmi.win.biBlueMask);
            if ( (bpp == 32) && (subtype == BMPWIN95) )
                biAlphaMask = BMPGETFIELD(bmpheader.bmi.win.biAlphaMask);
            else
                biAlphaMask = 0x0000;
        }
        else
        {
            if (bpp == 15)
            {
                biRedMask   = 0x7c00;
                biGreenMask = 0x03e0;
                biBlueMask  = 0x001f;
                biAlphaMask = 0x0000;
            }
            else if (bpp == 32)
            {
                biRedMask   = 0x00ff0000;
                biGreenMask = 0x0000ff00;
                biBlueMask  = 0x000000ff;
                biAlphaMask = 0x00000000;
            }
        }

        if (cmnheadersize>6)
            offset = BMPGETFIELD(bmpheader.bfOffbits);    /* offset to bitmap */

        if (!offset)
        {
            if (subtype==BMPOS2)             /* OS\2 */
            {
                offset = 12 + cmnheadersize;
                if (bpp<=8)
                    offset += colours*3;
            }
            else if (subtype == BMPWIN95)      /* Win 95 */
            {
                offset = 108 + cmnheadersize;
            }
            else if (subtype == BMPWINNT)      /* Win NT */
            {
                offset = 52 + cmnheadersize;
            }
            else                        /* Win 3.1 */
            {
                offset = 40 + cmnheadersize;
                if (bpp<=8)
                    offset += colours*4;
            }
        }
        ok = gseek(gx->gstream, offset);

        abpp = bpp;
        if (abpp==15) abpp = 16;

        const int MAXLINEBUF = (((width*abpp+31)&~31)>>3) + 32;

        linebuf = (char *)galloc(MAXLINEBUF);
        if (linebuf)
        {
            if ((int)(BMPGETFIELD(bmpheader.bmi.win.biHeight)) >= 0)
            {
                linebase = dest + (rowbytes * (height - 1));    /* start of last row */
                for (y=0; y<height; ++y)
                {
                    if (ok)
                        ok = BMPreadline(gx->gstream,(unsigned char *)linebase, rowbytes, (unsigned char *)linebuf, bpp, ginfo, biAlphaMask, biRedMask, biGreenMask, biBlueMask);
                    linebase -= rowbytes;
                }
            }
            else
            {
                linebase = dest;
                for(y = height; y > 0; --y)
                {
                    if (ok)
                        ok = BMPreadline(gx->gstream,(unsigned char *)linebase, rowbytes, (unsigned char *)linebuf, bpp, ginfo, biAlphaMask, biRedMask, biGreenMask, biBlueMask);
                    linebase += rowbytes;
                }
            }
            gfree(linebuf);
        }
    }
    return(ok);
}

int GCALL BMP_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

