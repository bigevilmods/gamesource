// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                   RLA - Wave Front RLA Gimex                     */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940810 FB Based on TGA and rla_read code from wavefront  */
/*    1.01 940811 FB RLA Write (compressed)                         */
/*    1.02 940811 FB Mac friendly                                   */
/*    1.03 941001 FB SGI friendly                                   */
/*    1.04 950529 FB ginstance signature                            */
/*    1.05 951109 FB support comment (description)                  */
/*    1.06 970124 FB ok to rows with 0 bytes                        */
/*    1.07 000315 FB 24 bit support (no matte)                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit only                                                      */
/* decompresses/compresses                                          */
/* Reentrant                                                        */
/* Files: rlaread.cpp rlawrite.cpp rlagimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* RLA code from wave front only supports 32 bit, but the format    */
/* seems to support any true colour or grey scale bit depth.        */
/*                                                                  */
/* 3DS Max can export 24 bit without the matte                      */
/*                                                                  */
/* A 740 byte header is followed by an offset table to rows.        */
/* Images are stored upside down, (SGI style) and in big endian.    */
/*                                                                  */
/* strip.rla dump                                                   */
/* --------------                                                   */
/* 00000: 0000027b 000001e3 0000027b 000001e3  .......c.......c     */
/* 00010: 00000000 00030001 00000000 20322e32  ............ 2.2     */
/* 00020: 30303000 00000000 00000000 20302e36  000......... 0.6     */
/* 00030: 37303020 20302e33 33303000 00000000  700  0.3300.....     */
/* 00040: 00000000 20302e32 31303020 20302e37  .... 0.2100  0.7     */
/* 00050: 31303000 00000000 00000000 20302e31  100......... 0.1     */
/* 00060: 34303020 20302e30 38303000 00000000  400  0.0800.....     */
/* 00070: 00000000 20302e33 31303020 20302e33  .... 0.3100  0.3     */
/* 00080: 31363000 00000000 00000000 00000000  160.............     */
/* 00090: 73747269 70000000 00000000 00000000  strip...........     */
/* 000a0: 00000000 00000000 00000000 00000000  ................     */
/* 000b0: 00000000 00000000 00000000 00000000  ................     */
/* 000c0: 00000000 00000000 00000000 00000000  ................     */
/* 000d0: 00000000 00000000 00000000 00000000  ................     */
/* 000e0: 00000000 00000000 00000000 00000000  ................     */
/* 000f0: 00000000 00000000 00000000 00000000  ................     */
/* 00100: 00000000 00000000 00000000 00000000  ................     */
/* 00110: 67726164 6174696f 6e206672 6f6d2067  gradation from g     */
/* 00120: 72616465 2e707600 00000000 00000000  rade.pv.........     */
/* 00130: 00000000 00000000 00000000 00000000  ................     */
/* 00140: 00000000 00000000 00000000 00000000  ................     */
/* 00150: 00000000 00000000 00000000 00000000  ................     */
/* 00160: 00000000 00000000 00000000 00000000  ................     */
/* 00170: 00000000 00000000 00000000 00000000  ................     */
/* 00180: 00000000 00000000 00000000 00000000  ................     */
/* 00190: 696d665f 67726164 65000000 00000000  imf_grade.......     */
/* 001a0: 00000000 00000000 00000000 00000000  ................     */
/* 001b0: 00000000 00000000 00000000 00000000  ................     */
/* 001c0: 00000000 00000000 00000000 00000000  ................     */
/* 001d0: 73686172 6b000000 00000000 00000000  shark...........     */
/* 001e0: 00000000 00000000 00000000 00000000  ................     */
/* 001f0: 6d61726b 00000000 00000000 00000000  mark............     */
/* 00200: 00000000 00000000 00000000 00000068  ...............h     */
/* 00210: 41707220 33302031 333a3237 20313938  Apr 30 13:27 198     */
/* 00220: 36000000 69726973 5f6e7473 63000000  6...iris_ntsc...     */
/* 00230: 00000000 00000000 00000000 00000000  ................     */
/* 00240: 00000000 72676200 00000000 00000000  ....rgb.........     */
/* 00250: 00000000 00000000 00000000 00000000  ................     */
/* 00260: 00000000 0001ee56 00000005 73747269  ......nV....stri     */
/* 00270: 702e726c 61000000 00000631 00000000  p.rla......1....     */
/* 00280: 00026b5f 00027fbe 1fffd8a8 1fffd89c  ..k_...>..X(..X.     */
/* 00290: 0001cf6c 00026b5f 1fffde94 00000000  ..Ol..k_..^.....     */
/* 002a0: 00000000 00000000 0001b1b6 00000000  ..........16....     */
/* 002b0: 0000012a 00000074 0001b360 00026a99  ...*...t..3`..j.     */
/* 002c0: 00000000 00020964 0001b306 00026a99  .......d..3...j.     */
/* 002d0: 1fffd8e0 0001cf6c 00000000 00000000  ..X`..Ol........     */
/* 002e0: 00000000 00000a74 00000aa4 00000ad4  .......t...$...T     */
/* 002f0: 00000b04 00000b34 00000b64 00000b94  .......4...d....     */
/* 00300: 00000bc4 00000bf4 00000c24 00000c54  ...D...t...$...T     */
/* 00310: 00000c84 00000cb4 00000ce4 00000d14  .......4...d....     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "rlagimex.h"

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

/* DECODE - decodes run length encoded pixels */

static void decode(unsigned char *s, unsigned char *d, int len)
{
    int     ct;

    while ( len > 0)
    {
        ct = (int) ((unsigned char) *s++);  /* zero extend to int 0..255 */
        len--;
        if ( ct < 128)
        {
            /* repeat pixel value ct+1 times */

            while ( ct-- >= 0)
                *d++ = *s;
            s++;
            len--;
        }
        else
        {
            /* copy ct unencoded values */

            for ( ct = 256-ct; ct-- > 0; len--)
                *d++ = *s++;
        }
    }
}

static int RLAreadline(GSTREAM *g,unsigned char *linebase,unsigned char *buf,RLAHeaderStruct *rlaheader)
{
    int ok;
    unsigned int len;
    unsigned char *red, *green, *blue, *matte;
    ARGB *d;
    int x;
    int left;
    int alphabits   = RLAGETFIELD(rlaheader->num_matte)?8:0;
    int width       = RLAGETFIELD(rlaheader->window.right)-RLAGETFIELD(rlaheader->window.left)+1;


    red    = buf+width*2;
    green  = buf+width*3;
    blue   = buf+width*4;
    matte  = buf+width*5;

    left = RLAGETFIELD(rlaheader->active_window.left);

    /* read red scanline */

    ok  = gread(g, buf, 2);
    len = ggetm(buf,2);
    gread(g, buf, len);
    decode( buf, red, len);

    /* read green scanline */

    ok = gread(g, buf, 2) && ok;
    len = ggetm(buf,2);
    gread(g, buf, len);
    decode( buf, green, len);

    /* read blue scanline */

    ok = gread(g, buf, 2) && ok;
    len = ggetm(buf,2);
    gread(g, buf, len);
    decode( buf, blue, len);

    /* read matte scanline */

    if (alphabits)
    {
        ok = gread(g, buf, 2) && ok;
        len = ggetm(buf,2);
        gread(g, buf, len);
        decode( buf, matte, len);
    }

    /* write out RGBM for each pixel */

    d = (ARGB *) linebase;
    for (x=RLAGETFIELD(rlaheader->window.left); x <= (int)RLAGETFIELD(rlaheader->window.right); ++x)
    {
        if (( x < (int)RLAGETFIELD(rlaheader->active_window.left)) ||
            ( x > (int)RLAGETFIELD(rlaheader->active_window.right)) )
        {
            d->a = 255;
            d->r = 0;
            d->g = 0;
            d->b = 0;
        }
        else
        {
            d->a = alphabits?matte[x-left]:255;
            d->r = red[x-left];
            d->g = green[x-left];
            d->b = blue[x-left];
        }
        ++d;
    }
    return(ok);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL RLA_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','R','L','A');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
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
        info->mactype[0]      = GMAKEID('.','R','L','A');   /* mac file system types used */
        strcpy(info->extensions[0], ".rla");                /* extensions */
        strcpy(info->extensions[1], ".rlb");
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.07");                /* version # */
        strcpy(info->shorttypestr,  "RLA");                 /* type */
        strcpy(info->wordtypestr,   "WaveFront");           /* wordtype */
        strcpy(info->longtypestr,   "WaveFront RLA");       /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* window values should be left/right top/bottom */
/* number of channels should be 1..4 */

int GCALL RLA_is(GSTREAM *g)
{
    RLAHeaderStruct rlaheader;
    int ok=0;

    gseek(g,(GPOS)0);
    if (glen(g)>=740)
    {
        if (gread(g,(void *)&rlaheader,740))
        {
            if (RLAGETFIELD(rlaheader.window.left) < RLAGETFIELD(rlaheader.window.right)
             && RLAGETFIELD(rlaheader.window.bottom)  < RLAGETFIELD(rlaheader.window.top)
             && RLAGETFIELD(rlaheader.active_window.left) < RLAGETFIELD(rlaheader.active_window.right)
             && RLAGETFIELD(rlaheader.active_window.bottom)  < RLAGETFIELD(rlaheader.active_window.top)
             && RLAGETFIELD(rlaheader.num_chan) >= 1
             && RLAGETFIELD(rlaheader.num_chan) <= 4
             && RLAGETFIELD(rlaheader.num_matte) <= 1)
                ok = 95;    /* 95% odds its a rla */
        }
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL RLA_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','R','L','A');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL RLA_info(GINSTANCE *gx, int framenum)
{
    RLAHeaderStruct rlaheader;
    GINFO *ginfo=0;
    int width;
    int height;
    int defaultx;
    int defaulty;
    int alphabits=0;
    int ok=0;

    gseek(gx->gstream,(GPOS)0);
    if (gread(gx->gstream, (char *) &rlaheader, 740))   /* read header */
    {
        width       = RLAGETFIELD(rlaheader.window.right)-RLAGETFIELD(rlaheader.window.left)+1;
        height      = RLAGETFIELD(rlaheader.window.top)-RLAGETFIELD(rlaheader.window.bottom)+1;
        defaultx    = RLAGETFIELD(rlaheader.window.left)+1;
        defaulty    = -((int)RLAGETFIELD(rlaheader.window.bottom));
        alphabits   = RLAGETFIELD(rlaheader.num_matte)?8:0;

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','R','L','A');   /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;                        /* Frame number */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */
            ginfo->subtype          = 0;                        /* subtype */
            ginfo->packed           = 1;
            ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = alphabits;    /* Number of bits in alpha channel */
            ginfo->redbits          = 8;            /* Number of bits in red channel */
            ginfo->greenbits        = 8;            /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;            /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = defaultx;     /* Default Coordinate Point */
            ginfo->defaulty         = defaulty;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */
            strcpy(ginfo->comment,rlaheader.desc);  /* Null terminated user comment */

            ginfo->bpp              = 32;           /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = 24+alphabits;
            ginfo->numcolours  = 0;                 /* Number of colours in original indexed palette */
            ok = 1;
        }
    }

    return(ginfo);
}

int GCALL RLA_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int y;
    int width  = 0;
    int height = 0;
    int activeheight;
    int chanbits;
    int ok=0;
    int offset;
    char *linebase;
    char *offsettbl=0;
    char *buf=0;
    RLAHeaderStruct rlaheader;

    gseek(gx->gstream,(GPOS)0);
    if (gread(gx->gstream, (char *) &rlaheader, 740))   /* read header */
    {
        width   = ginfo->width;
        height  = ginfo->height;

        activeheight = RLAGETFIELD(rlaheader.active_window.top)-RLAGETFIELD(rlaheader.active_window.bottom)+1;
        chanbits =     (int) RLAGETFIELD(rlaheader.chan_bits);

        /* read scanline offset table */

        offsettbl = (char *) galloc(activeheight*4);
        if (offsettbl)
        {
            if (gread(gx->gstream,offsettbl,activeheight*4))
            {
                buf = (char *) galloc(width * 6);  /* packed plane, and 4 unpacked planes */

                if (buf)
                {
                    ok = 1;
                }
            }
        }
    }
    if (ok)
    {
        for (y=(int)RLAGETFIELD(rlaheader.window.bottom); y <= (int)RLAGETFIELD(rlaheader.window.top) && ok; ++y)
        {
            linebase = dest + rowbytes*(height-1-(y-RLAGETFIELD(rlaheader.window.bottom)));

            /* check for black regions outside active window */

            if ((y < (int)RLAGETFIELD(rlaheader.active_window.bottom)) ||
                (y > (int)RLAGETFIELD(rlaheader.active_window.top)))
            {

               /* write out a blank yline */
               memset(linebase, 0, width*4);
            }
            else
            {
                /* seek to file location */

                offset = ggetm(offsettbl+(y-RLAGETFIELD(rlaheader.active_window.bottom))*4,4);
                ok = gseek(gx->gstream,offset);

                if (ok)
                    ok = RLAreadline(gx->gstream,(unsigned char *)linebase,(unsigned char *)buf,&rlaheader);
            }
        }
    }
    if (buf) gfree(buf);
    if (offsettbl) gfree(offsettbl);
    return(ok);
}


int GCALL RLA_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


