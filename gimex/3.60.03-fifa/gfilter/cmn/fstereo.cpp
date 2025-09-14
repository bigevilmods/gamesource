// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  Stereo - Stereo Scale Filter                    */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951030 FB based on fgrey                                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* stereo(int focus,range)                                          */
/* int focus;    amount of focus range (68 is default)              */
/* int range;    amount of range (10 is default)                    */
/*                                                                  */
/* Converts image to stereo gram                                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fstereo.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

/* Default Latenight Palette */

static const int stereopalette[256]=
{
    0x000000, 0x0000a8,
    0x00a800, 0x00a8a8,
    0xa80000, 0xa800a8,
    0xa85400, 0xa8a8a8,
    0x545454, 0x5454fc,
    0x54fc54, 0x54fcfc,
    0xfc5454, 0xfc54fc,
    0xfcfc54, 0xfcfcfc,
    0xfcfcfc, 0xdcdcdc,
    0xcccccc, 0xc0c0c0,
    0xb0b0b0, 0xa4a4a4,
    0x989898, 0x888888,
    0x7c7c7c, 0x707070,
    0x606060, 0x545454,
    0x484848, 0x383838,
    0x2c2c2c, 0x202020,
    0xfcd8d8, 0xfcb8b8,
    0xfc9c9c, 0xfc7c7c,
    0xfc5c5c, 0xfc4040,
    0xfc2020, 0xfc0000,
    0xe40000, 0xcc0000,
    0xb40000, 0x9c0000,
    0x840000, 0x700000,
    0x580000, 0x400000,
    0xfce8d8, 0xfcdcc0,
    0xfcd4ac, 0xfcc894,
    0xfcc080, 0xfcb868,
    0xfcac54, 0xfca43c,
    0xfc9c28, 0xe08820,
    0xc4781c, 0xa86414,
    0x905410, 0x744008,
    0x583004, 0x402000,
    0xfcfcd8, 0xfcfcb8,
    0xfcfc9c, 0xfcfc7c,
    0xfcf85c, 0xfcf440,
    0xfcf420, 0xfcf400,
    0xe4d800, 0xccc000,
    0xb4a400, 0x9c8c00,
    0x847400, 0x6c5800,
    0x544000, 0x402800,
    0xf8fcd8, 0xf4fcb8,
    0xe8fc9c, 0xe0fc7c,
    0xd0fc5c, 0xc4fc40,
    0xb4fc20, 0xa0fc00,
    0x90e400, 0x80cc00,
    0x74b400, 0x609c00,
    0x508400, 0x447000,
    0x345800, 0x284000,
    0xd8fcd8, 0x9cfc9c,
    0x90ec90, 0x84e084,
    0x78d078, 0x70c46c,
    0x64b864, 0x58a858,
    0x509c4c, 0x449040,
    0x388034, 0x2c742c,
    0x246820, 0x185814,
    0x0c4c08, 0x044000,
    0xd8fcfc, 0xb8fcfc,
    0x9cfcfc, 0x7cfcf8,
    0x5cfcfc, 0x40fcfc,
    0x20fcfc, 0x00fcfc,
    0x00e4e4, 0x00cccc,
    0x00b4b4, 0x009c9c,
    0x008484, 0x007070,
    0x005858, 0x004040,
    0xd8ecfc, 0xb8e0fc,
    0x9cd4fc, 0x7cc8fc,
    0x5cbcfc, 0x40b0fc,
    0x009cfc, 0x008ce4,
    0x0080d0, 0x0074bc,
    0x0064a8, 0x005890,
    0x004c7c, 0x003c68,
    0x003054, 0x002440,
    0xd8d8fc, 0xb8bcfc,
    0x9c9cfc, 0x7c80fc,
    0x5c60fc, 0x4040fc,
    0x0004fc, 0x0000e4,
    0x0000d0, 0x0000bc,
    0x0000a8, 0x000090,
    0x00007c, 0x000068,
    0x000054, 0x000040,
    0xf0d8fc, 0xe4b8fc,
    0xd89cfc, 0xd07cfc,
    0xc85cfc, 0xbc40fc,
    0xb420fc, 0xa800fc,
    0x9800e4, 0x8000cc,
    0x7400b4, 0x60009c,
    0x500084, 0x440070,
    0x340058, 0x280040,
    0xfcd8fc, 0xfcb8fc,
    0xfc9cfc, 0xfc7cfc,
    0xfc5cfc, 0xfc40fc,
    0xfc20fc, 0xe000e4,
    0xcc00cc, 0xb800b8,
    0xa400a4, 0x900090,
    0x7c007c, 0x680068,
    0x540054, 0x400040,
    0xfce8dc, 0xf0d4c4,
    0xe4c4ac, 0xd8b498,
    0xcca080, 0xc0906c,
    0xb48054, 0xac7040,
    0x9c6438, 0x8c5c34,
    0x80542c, 0x704c28,
    0x604020, 0x54381c,
    0x443014, 0x382810,
    0xfcd8cc, 0xf8ccb8,
    0xf4c0a8, 0xf0b494,
    0xeca884, 0xe89c74,
    0xe49464, 0xe08c58,
    0xd8804c, 0xd47840,
    0xc86c34, 0xc0602c,
    0xb45424, 0xa8481c,
    0x9c3c14, 0x94300c,
    0x000000, 0x080808,
    0x101010, 0x181818,
    0x202020, 0x282828,
    0x303030, 0x383838,
    0x404040, 0x484848,
    0x505050, 0x585858,
    0x606060, 0x686868,
    0x707070, 0x787878,
    0x808080, 0x888888,
    0x909090, 0x989898,
    0xa0a0a0, 0xa8a8a8,
    0xb0b0b0, 0xb8b8b8,
    0xc0c0c0, 0xc8c8c8,
    0xd0d0d0, 0xd8d8d8,
    0xe0e0e0, 0xe8e8e8,
    0xf0f0f0, 0xffffff
};

static unsigned int grandom(unsigned int *seed)
{
    *seed = (*seed * 1103515245 + 12345) % 65536;
    return((unsigned int) *seed>>8);
}

static unsigned char pattern(unsigned int *seed, int y, int height)
{
    return((unsigned char)(((unsigned int)(grandom(seed)&15))+y*100/height+70));
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutstereo(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('S','T','A','R');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Stereo" );
        strcpy(about->longnamestr,  "Stereo Image" );
        about->numoptions =         1;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           256;
        about->option[0].def.i =           68;
        strcpy(about->option[0].name,      "focus");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0;
        about->option[1].max.i =           256;
        about->option[1].def.i =           10;
        strcpy(about->option[1].name,      "range");
     }
    return(about);
}

FPUBLIC int openstereo(FINSTANCE* *filterInstance,  int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;

    fx = (FINSTANCE *)galloc(sizeof(FINSTANCE));
    if (fx)
    {
        memset(fx, 0, sizeof(FINSTANCE));
        fx->frames      = 1;
        fx->numsources  = numsources;
        fx->source      = sources;
        fx->numoptions  = numoptions;
        fx->option      = options;
        ok = 1;
    }
    *filterInstance = fx;
    return(ok);
}

FPUBLIC GINFO *infostereo(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    int focus=68;
    int range=10;
    int i;

    if (fx->option)
    {
        if (fx->numoptions>=1) focus = fx->option[0].i;
        if (fx->numoptions>=2) range = fx->option[1].i;
        if (focus<2) focus=68;
        if (range<2) range=10;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        ginfo->originalbpp = 8;
        ginfo->bpp         = 8;
        ginfo->startcolour = 0;
        ginfo->numcolours  = 256;
        ginfo->alphabits   = 0;
        ginfo->redbits     = 8;
        ginfo->greenbits   = 8;
        ginfo->bluebits    = 8;

        for (i=0; i<256; ++i)
        {
            ginfo->colourtbl[i].a = (unsigned char) 0x255;
            ginfo->colourtbl[i].r = (unsigned char) (stereopalette[i]>>16);
            ginfo->colourtbl[i].g = (unsigned char) (stereopalette[i]>>8);
            ginfo->colourtbl[i].b = (unsigned char) (stereopalette[i]>>0);
        }
    }
    return(ginfo);
}

FPUBLIC int filterstereo(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    int x;
    int y;
    int hx;
    unsigned char z;
    unsigned char *s;
    unsigned char *d;
    unsigned char starezx[256];
    unsigned int seed;
    ARGB *p;
    int  focus=68;
    int  range=10;

#if 0
    if (fx->option)
    {
        if (fx->numoptions>=1) focus = fx->option[0].i;
        if (fx->numoptions>=2) range = fx->option[1].i;
        if (focus<2) focus=68;
        if (range<2) range=10;
    }
#endif

    focus = focus*ginfo->width/640*640/1024;
    range = range*ginfo->width/640*480/768;

    /* build translation table for depth */

    for (i=0; i<256; ++i)
    {
        starezx[i] = (unsigned char) ((255-i)*range/255+focus);
    }

    p = &fx->source[0].ginfo->colourtbl[0];

    seed = 0x12345678;
    for (y=0; y<ginfo->height; ++y)
    {
        s = ((unsigned char *) fx->source[0].image) + (fx->source[0].rowbytes*y);
        d = ((unsigned char *) dest) + (rowbytes*y);
        for (x=0; x<ginfo->width; ++x)
        {
            if (ginfo->bpp==32)
            {
                hx = (((ARGB *) s)->r*299+((ARGB *) s)->g*587+((ARGB *) s)->b*114)/1000;
                s += 4;
            }
            else
            {
                hx = (int) *s;
                hx = (p[hx].r*299+p[hx].g*587+p[hx].b*114)/1000;
                ++s;
            }

            z = starezx[hx];
            if (x-z<0)  *d++ = pattern(&seed,y,ginfo->height);
            else        *d++ = *(d-z);
        }
    }

    return(1);
}

FPUBLIC int closestereo(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

