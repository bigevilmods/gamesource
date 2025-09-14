// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  TGA - Truevision Targa Gimex                    */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940414 FB First gimex prototype module                   */
/*    1.10 940425 FB Reimplimented reentrantly                      */
/*    1.11 940431 FB Gimex 1.03 spec                                */
/*    1.12 940614 FB updated to Gimex 1.5                           */
/*    1.13 940629 FB TPIC for mac type, Truevision Targa for name   */
/*    1.14 940629 FB canexport code based on bmp                    */
/*    1.20 940630 FB scratch rewrite of canimport based on bmp      */
/*    1.21 940704 FB 16 bit compressed                              */
/*    1.22 940704 FB 15 bit pad, 32 bit palettes                    */
/*    1.23 940705 FB 15,24,32 bit palettes, descriptor, -defaulty   */
/*    1.24 940708 FB 3DO and MPW compiler tweeks                    */
/*    1.25 941225 FB write grey scale properly                      */
/*    1.26 504065 FB alpha bits calc                                */
/*    1.27 950529 FB ginstance signature                            */
/*    1.28 950621 FB canexport compress all depths                  */
/*    1.29 950802 FB compressed grey scale canexport set image type */
/*    1.30 950911 FB start colour, frame size supported             */
/*    1.31 950912 FB rle tga canimport use ARGB                     */
/*    1.32 951003 FB istga fully parse header,16/24 packer fix      */
/*    1.33 951107 FB infotga sign extend defaultx/y                 */
/*    1.34 960608 FB canimport 32 bit palette attrib as alpha       */
/*    1.35 971214 FB writer use gputi to write words                */
/*    1.36 990809 FB 1555 support alpha vs 0555                     */
/*    1.37 990830 FB rle encoder use next run prediction            */
/*    1.38 020306 FB export grey only if opaque                     */
/*    1.39 031125 FB grey use 0800 channel bits and work on 32 bit  */
/*    1.40 060628 FB memory based rle decode for speed              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,15,24,32 bit import/export                                     */
/* compresses/decompresses                                          */
/* Reentrant                                                        */
/* Files: tgaread.cpp tgawrite.cpp tgagimex.h                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* TGA files support 8,15,24,32 bit depths.                         */
/* 8 bit can be grey scale                                          */
/* Runlength compression                                            */
/* Additional Footer information optional                           */
/* x/y origin relative to lower left with origin in lower left      */
/* descriptor: 00vhaaaa                                             */
/*   h horizontal flip                                              */
/*   v vertical flip                                                */
/*   a alpha bits                                                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "tgagimex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

#define gmemread(g,dest,len) memcpy(dest,*g,len); *g += len;

static __inline int TGAreadline(char **g,unsigned char *d,unsigned char *linebuf,GINFO *ginfo)
{
    int width;
    int bpp;
    int packed;
    int rowbytes;
    int ok=1;
    int x=0;
    int i;
    int count;
    int bytesread=0;
    int alphabits;
    unsigned char *s=linebuf;

    width     = ginfo->width;
    bpp       = ginfo->originalbpp;
    packed    = ginfo->packed;
    alphabits = ginfo->alphabits;

    if (!packed)        /* uncompressed images */
    {
        count = width;
        rowbytes = ((count*((bpp+7)&~7)))>>3;
        gmemread(g, s, rowbytes); bytesread+=rowbytes;

        switch (bpp)
        {
            case 8:     /* uncompressed 8 bit indexed image */
                for (i=0; i<count; ++i)
                {
                    *d++ = *s++;
                }
                break;

            case 15:    /* uncompressed 15 bit direct image */
            case 16:    /* uncompressed 16 bit direct image */
                for (i=0; i<count; ++i)
                {
                    if (alphabits)
                        ((ARGB *) d)->a = (unsigned char)(((*(s+1)>>7)&1)*255);
                    else
                        ((ARGB *) d)->a = (unsigned char)(255);

                    ((ARGB *) d)->r = (unsigned char)(((*(s+1)>>2)&31)*255/31);
                    ((ARGB *) d)->g = (unsigned char)((((*(s+1)&3)<<3) | ((*(s+0)>>5)&7))*255/31);
                    ((ARGB *) d)->b = (unsigned char)((*s&31)*255/31);
                    d += 4;
                    s += 2;
                }
                break;

            case 24:    /* uncompressed 24 bit direct image */
                for (i=0; i<count; ++i)
                {
                    ((ARGB *) d)->a = (unsigned char)(255);
                    ((ARGB *) d)->r = s[2];
                    ((ARGB *) d)->g = s[1];
                    ((ARGB *) d)->b = s[0];
                    d += 4;
                    s += 3;
                }
                break;

            case 32:    /* uncompressed 32 bit direct image */
                for (i=0; i<count; ++i)
                {
                    ((ARGB *) d)->a = s[3];
                    ((ARGB *) d)->r = s[2];
                    ((ARGB *) d)->g = s[1];
                    ((ARGB *) d)->b = s[0];
                    d += 4;
                    s += 4;
                }
                break;
        }
    }
    else                /* run length compressed images */
    {
        do
        {
            gmemread(g, s, 1); bytesread+=1;
            count = (int) *s;
            s = linebuf;

            if (count&0x80)         /* repeating colours */
            {
                count = (count&0x7f)+1;
                x += count;

                switch (bpp)
                {
                    case 8:     /* compressed 8 bit indexed image */
                        gmemread(g, s, 1); bytesread+=1;
                        for (i=0; i<count; ++i)
                        {
                            *d++ = *s;
                        }
                        break;

                    case 15:    /* compressed 15 bit direct image */
                    case 16:    /* compressed 16 bit direct image */
                        gmemread(g, s, 2); bytesread+=2;
                        for (i=0; i<count; ++i)
                        {
                            if (alphabits)
                                ((ARGB *) d)->a = (unsigned char)(((*(s+1)>>7)&1)*255);
                            else
                                ((ARGB *) d)->a = (unsigned char)(255);

                            ((ARGB *) d)->r = (unsigned char)(((*(s+1)>>2)&31)*255/31);
                            ((ARGB *) d)->g = (unsigned char)((((*(s+1)&3)<<3) | ((*(s+0)>>5)&7))*255/31);
                            ((ARGB *) d)->b = (unsigned char)((*s&31)*255/31);
                            d += 4;
                        }
                        break;

                    case 24:    /* compressed 24 bit direct image */
                        gmemread(g, s, 3); bytesread+=3;
                        for (i=0; i<count; ++i)
                        {
                            ((ARGB *) d)->a = (unsigned char)(255);
                            ((ARGB *) d)->r = s[2];
                            ((ARGB *) d)->g = s[1];
                            ((ARGB *) d)->b = s[0];
                            d += 4;
                        }
                        break;

                    case 32:    /* compressed 32 bit direct image */
                        gmemread(g, s, 4); bytesread+=4;
                        for (i=0; i<count; ++i)
                        {
                            ((ARGB *) d)->a = s[3];
                            ((ARGB *) d)->r = s[2];
                            ((ARGB *) d)->g = s[1];
                            ((ARGB *) d)->b = s[0];
                            d += 4;
                        }
                        break;
                }
            }
            else                /* literal colours */
            {
                count = (count&0x7f)+1;

                x += count;
                rowbytes = ((count*((bpp+7)&~7)))>>3;

                gmemread(g, s, rowbytes); bytesread+=rowbytes;

                switch (bpp)
                {
                    case 8:     /* literal 8 bit indexed image */
                        for (i=0; i<count; ++i)
                        {
                            *d++ = *s++;
                        }
                        break;

                    case 15:    /* literal 15 bit direct image */
                    case 16:    /* literal 16 bit direct image */
                        for (i=0; i<count; ++i)
                        {
                            if (alphabits)
                                ((ARGB *) d)->a = (unsigned char)(((*(s+1)>>7)&1)*255);
                            else
                                ((ARGB *) d)->a = (unsigned char)(255);

                            ((ARGB *) d)->r = (unsigned char)(((*(s+1)>>2)&31)*255/31);
                            ((ARGB *) d)->g = (unsigned char)((((*(s+1)&3)<<3) | ((*(s+0)>>5)&7))*255/31);
                            ((ARGB *) d)->b = (unsigned char)((*s&31)*255/31);
                            d += 4;
                            s += 2;
                        }
                        break;

                    case 24:    /* literal 24 bit direct image */
                        for (i=0; i<count; ++i)
                        {
                            ((ARGB *) d)->a = (unsigned char)(255);
                            ((ARGB *) d)->r = s[2];
                            ((ARGB *) d)->g = s[1];
                            ((ARGB *) d)->b = s[0];
                            d += 4;
                            s += 3;
                        }
                        break;

                    case 32:    /* literal 32 bit direct image */
                        for (i=0; i<count; ++i)
                        {
                            ((ARGB *) d)->a = s[3];
                            ((ARGB *) d)->r = s[2];
                            ((ARGB *) d)->g = s[1];
                            ((ARGB *) d)->b = s[0];
                            d += 4;
                            s += 4;
                        }
                        break;
                }

            }
       } while (x<width);
    }
    return(ok);
}

static void TGAflipline(unsigned char *linebase,int width,int bpp)
{
    int i;

    if (bpp==32)
    {
        int *p1;
        int *p2;
        int swaplong;

        p1 = (int *) (linebase);
        p2 = ((int *) linebase) + width-1;
        for (i=0; i<(width>>1); ++i)
        {
            swaplong = *p1;
            *p1++ = *p2;
            *p2-- = swaplong;
        }
    }
    else
    {
        unsigned char *p1;
        unsigned char *p2;
        unsigned char swapchar;

        p1 = linebase;
        p2 = linebase+width-1;
        for (i=0; i<(width>>1); ++i)
        {
            swapchar = *p1;
            *p1++ = *p2;
            *p2-- = swapchar;
        }
    }
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL TGA_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','T','G','A');
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
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('T','P','I','C');   /* mac file system types used */
        strcpy(info->extensions[0], ".tga");                /* extensions */
        strcpy(info->extensions[1], ".vda");                /* extensions */
        strcpy(info->extensions[2], ".icb");                /* extensions */
        strcpy(info->extensions[3], ".vst");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.40");                /* version # */
        strcpy(info->shorttypestr,  "TGA");                 /* type */
        strcpy(info->wordtypestr,   "Targa");               /* wordtype */
        strcpy(info->longtypestr,   "Targa");               /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* colourmaptype 0 rgb or 1 palette */
/* imagetype 1,2,3 +8 */
/* depth 8,16,24,32 */

int GCALL TGA_is(GSTREAM *g)
{
    TGAHeaderStruct tgaheader;
    GPOS framesize;
    int width;
    int height;
    int defaultx;
    int defaulty;
    int bpp;
    int imagetype;
    int packed;
    int colourmaptype;
    int colourstart;
    int colourlen;
    int colourbits;
    int ok=0;

    gseek(g,(GPOS)0);
    framesize = glen(g);
    ok = gread(g, (char *) &tgaheader, 18);   /* read header */
    if (ok)
    {
        ok = 0;
        width       = TGAGETFIELD(tgaheader.width);                 /* Width of bitmap in pixels */
        height      = TGAGETFIELD(tgaheader.height);                /* Height of bitmap in pixels */
        bpp         = TGAGETFIELD(tgaheader.bits);                  /* Colour bits per pixel */
        imagetype   = TGAGETFIELD(tgaheader.imagetype);             /* type of image */
        packed      = (imagetype&8)?1:0;

        colourmaptype=TGAGETFIELD(tgaheader.colourmaptype);
        colourstart = TGAGETFIELD(tgaheader.colourmapstart);
        colourlen   = TGAGETFIELD(tgaheader.colourmaplength);
        colourbits  = TGAGETFIELD(tgaheader.colourmapbits);
        defaultx    = TGAGETFIELD(tgaheader.xstart);
        defaulty    = TGAGETFIELD(tgaheader.ystart);


        if (colourmaptype>=0 && colourmaptype<=1
         && ((imagetype>=1 && imagetype<=3) || (imagetype>=9 && imagetype<=11))
         && (bpp==8 || bpp==15 || bpp==16 || bpp==24 || bpp==32))
        {
            ok = 60;
            if (width>0 && width<=4096
             && height>0 && height<=2048
             && colourstart>=0 && colourstart<=255
             && colourlen>=0 && colourlen<=256
             && colourbits>=0 && colourbits<=32
             && defaultx>=-2048 && defaultx<=4096
             && defaulty>=-1024 && defaulty<=2048)
                ok = 100;
        }
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL TGA_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','T','G','A');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;

		// read image header into memory
		int len = (int)glen(g);
        gx->gref = galloc(len+4);
		gputi(gx->gref , len, 4); // remember size
	    gseek(g,(GPOS)0);
		int readsize = 18+1024;
		if (len<readsize)
			readsize = len;
		ok = gread(g, (char *) gx->gref+4, readsize)>0?1:0;   /* read header */

        *gxptr = gx;
    }
    return(ok);
}

GINFO * GCALL TGA_info(GINSTANCE *gx, int framenum)
{
    TGAHeaderStruct tgaheader;
    unsigned char pal[256*4];
    GINFO *ginfo=0;
    int bpp;
    int colours;
    int i;
    unsigned int rgb;
    int width;
    int height;
    int packed;
    int imagetype;
    int identsize;
    int colourstart;
    int colourlen;
    int colourbits;
    int ok=0;
    int defaultx;
    int defaulty;
    int alphabits;
    int descriptor;
    GPOS framesize;

    if (gx->gref)
    {
		framesize = ggeti(gx->gref , 4);
		char *imagebuf = (char *)gx->gref + 4;
		memcpy((char *) &tgaheader, imagebuf, 18); /* read header */

		width       = TGAGETFIELD(tgaheader.width);                 /* Width of bitmap in pixels */
        height      = TGAGETFIELD(tgaheader.height);                /* Height of bitmap in pixels */
        bpp         = TGAGETFIELD(tgaheader.bits);                  /* Colour bits per pixel */
        imagetype   = TGAGETFIELD(tgaheader.imagetype);             /* type of image */
        packed      = (imagetype&8)?1:0;

        colourstart = TGAGETFIELD(tgaheader.colourmapstart);
        colourlen   = TGAGETFIELD(tgaheader.colourmaplength);
        colourbits  = TGAGETFIELD(tgaheader.colourmapbits);
        identsize   = TGAGETFIELD(tgaheader.identsize);

        descriptor  = TGAGETFIELD(tgaheader.descriptor);
        defaultx    = TGAGETSIGNEDFIELD(tgaheader.xstart);
        defaulty    = TGAGETSIGNEDFIELD(tgaheader.ystart);

        if (!(descriptor&0x20)) defaulty = -defaulty;
        if (descriptor&0x10)    defaultx = -defaultx;
        alphabits = bpp-24;
        if (bpp==15 || bpp==16 || (bpp==8 && TGAGETFIELD(tgaheader.colourmaptype) && (colourbits==15 || colourbits==16)))
            alphabits = (descriptor&15)==1;
        if (alphabits<=0) alphabits = 0;
        if (alphabits>8)  alphabits = 8;

		char *imagebuf_pal = imagebuf+18+identsize;

        colours = 0;
        if ((imagetype&7)==1)
            colours   = colourlen;
        else if ((imagetype&7)==3)
            colours   = 256;

		bool isgrey = (imagetype&7)==3;

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','T','G','A');   /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;                        /* Current frame num */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */
            ginfo->framesize        = (int)framesize;
            ginfo->subtype          = 0;                        /* subtype */
            ginfo->packed           = packed;
            ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = alphabits;    /* Number of bits in alpha channel */
            ginfo->redbits          = 8;            /* Number of bits in red channel */
            ginfo->greenbits        = isgrey?0:8;            /* Number of bits in grreen channel */
            ginfo->bluebits         = isgrey?0:8;            /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = defaultx;     /* Default Coordinate Point */
            ginfo->defaulty         = defaulty;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */

            /* 15,16,24,32 bpp true colour */
            if (bpp>=15)
            {
                if (bpp==16) bpp=15;
                ginfo->bpp          = 32;       /* Bits per pixel (8 or 32) */
                ginfo->originalbpp  = bpp;
                ginfo->numcolours   = 0;        /* Number of colours in original indexed palette */
                if (bpp<=16)
                    ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 5;
                if (bpp<32)
                    ginfo->alphabits= 0;
            }
            /* 8 grey or colour, with 16 or 24 bit palette */
            else
            {
                if (!bpp)        bpp = 8;
                if (!colours)    colours = (1 << bpp);
                if (colours>256) colours = 256;

                ginfo->bpp          = 8;                /* Bits per pixel (8 or 32) */
                ginfo->originalbpp  = bpp;
                ginfo->numcolours   = colours;          /* Number of colours in original indexed palette */
                ginfo->startcolour  = colourstart;      /* Number of colours in original indexed palette */

                if (TGAGETFIELD(tgaheader.colourmaptype))           /* type of colour map 0=none, 1=has palette */
                {
                    switch (colourbits)
                    {
                        case 32:
                            ginfo->alphabits    = 8;
							memcpy((char *)pal, imagebuf_pal, colours*4);
//                            ok = gread(gx->gstream, (char *) pal, colours*4);
                            for (i=0; i<colourlen; ++i)
                            {
                                ginfo->colourtbl[colourstart+i].a = pal[i*4+3];
                                ginfo->colourtbl[colourstart+i].r = pal[i*4+2];
                                ginfo->colourtbl[colourstart+i].g = pal[i*4+1];
                                ginfo->colourtbl[colourstart+i].b = pal[i*4+0];
                            }
                            break;

                        case 24:
                            ginfo->alphabits    = 0;
  							memcpy((char *)pal, imagebuf_pal, colours*3);
//                          ok = gread(gx->gstream, (char *) pal, colours*3);
                            for (i=0; i<colourlen; ++i)
                            {
                                ginfo->colourtbl[colourstart+i].a = (unsigned char)(255);
                                ginfo->colourtbl[colourstart+i].r = pal[i*3+2];
                                ginfo->colourtbl[colourstart+i].g = pal[i*3+1];
                                ginfo->colourtbl[colourstart+i].b = pal[i*3+0];
                            }
                            break;


                        case 15:
                        case 16:
                            ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 5;
                            ginfo->alphabits    = 0;
							memcpy((char *)pal, imagebuf_pal, colours*2);
//                            ok = gread(gx->gstream, (char *) pal, colours*2);
                            for (i=0; i<colourlen; ++i)
                            {
                                rgb = ggeti(pal+i*2,2);
                                if (alphabits)
                                    ginfo->colourtbl[colourstart+i].a = (unsigned char)(((rgb>>15)&1)*255);
                                else
                                    ginfo->colourtbl[colourstart+i].a = (unsigned char)(255);
                                ginfo->colourtbl[colourstart+i].r = (unsigned char)(((rgb>>10)&31)*255/31);
                                ginfo->colourtbl[colourstart+i].g = (unsigned char)(((rgb>> 5)&31)*255/31);
                                ginfo->colourtbl[colourstart+i].b = (unsigned char)(((rgb>> 0)&31)*255/31);
                            }
                            break;

                        default:
                            break;
                    }
                }
                else    /* grey scale */
                {
                    ginfo->alphabits    = 0;
                    for (i=0; i<256; ++i)
                    {
                        ginfo->colourtbl[i].a = (unsigned char)(255);
                        ginfo->colourtbl[i].r = (unsigned char)(i);
                        ginfo->colourtbl[i].g = (unsigned char)(i);
                        ginfo->colourtbl[i].b = (unsigned char)(i);
                    }
                }
            }
			ok = 1;
        }
        if (ginfo && !ok)
        {
            gfree((char *)ginfo);
            ginfo = 0;
        }
    }

    return(ginfo);
}

int GCALL TGA_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    unsigned char *linebuf;
    unsigned char *linebase;
    int y;
    int width;
    int height;
    int bpp;
    int colours;
    int packed;
    int offset;
    int ok;
    int identsize;
    int colourlen;
    int colourbits;
    int descriptor;
    int vfrowbytes;
    TGAHeaderStruct tgaheader;

    if (gx->gref)
    {
		char *imagebuf = (char *)gx->gref + 4;
		int framesize = ggeti(gx->gref , 4);

	    gseek(gx->gstream,(GPOS)0);
		ok = gread(gx->gstream, (char *) gx->gref+4, framesize)>0?1:0;   /* read image */



		memcpy((char *) &tgaheader, imagebuf, 18); /* read header */

        width   = ginfo->width;
        height  = ginfo->height;
        bpp     = ginfo->originalbpp;
        colours = ginfo->numcolours;
        packed  = ginfo->packed;

        colourlen   = TGAGETFIELD(tgaheader.colourmaplength);
        colourbits  = TGAGETFIELD(tgaheader.colourmapbits);
        identsize   = TGAGETFIELD(tgaheader.identsize);
        descriptor  = TGAGETFIELD(tgaheader.descriptor);

        offset = 18+identsize;
        if (TGAGETFIELD(tgaheader.colourmaptype))           /* type of colour map 0=none, 1=has palette */
            offset += colourlen*((colourbits+7)/8);

		char *imagebuf_pixels = imagebuf+offset;
		
        if (descriptor&0x20)             /* start at top */
        {
            linebase = (unsigned char *) dest;
            vfrowbytes = rowbytes;
        }
        else                            /* start of last row */
        {
            linebase = (unsigned char *) dest + (rowbytes * (height - 1));
            vfrowbytes = -rowbytes;
        }
        linebuf = (unsigned char *) galloc(((width*((bpp+7)&~7)))>>3);
        if (linebuf)
        {
			ok = 1;
            for (y=0; y<height && ok; ++y)
            {
                ok = TGAreadline(&imagebuf_pixels,linebase,linebuf,ginfo);
                if (descriptor&0x10)
                    TGAflipline(linebase,width,ginfo->bpp);
                linebase += vfrowbytes;
            }
            gfree(linebuf);
        }
    }
    return(ok);
}

int GCALL TGA_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
	{
		if (gx->gref) gfree(gx->gref);
		gx->gref = NULL;
        ok = gfree((void *) gx);
	}

    return(ok);
}

