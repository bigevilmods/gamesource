// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "timgimex.h"
#include "oldpshgimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

static unsigned short TIMpixel8To5(const ARGB *aRgb8)
{
    unsigned int a;
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int aRgb5;

    a = aRgb8->a;
    r = ((int)aRgb8->r*31+128)/255;
    g = ((int)aRgb8->g*31+128)/255;
    b = ((int)aRgb8->b*31+128)/255;

    aRgb5 = (b<<10)|(g<<5)|r;

    if (a<25*255/100)           /* transparent */
    {
        aRgb5 = 0;
    }
    else if (a>=75*255/100)     /* opaque */
    {
        if (!aRgb5)
            aRgb5 = (1<<10);    /* black remap to blue */
    }
    else                        /* translucient */
    {
        aRgb5 |= 0x8000;
    }
    return((unsigned short) aRgb5);
}

static void TIMwriteline(void *src, void *dest, int count, int bpp)
{
    unsigned char *s=(unsigned char *)src;
    unsigned char *d=(unsigned char *)dest;
    int x;

    for (x=0; x<count; ++x)
    {
        if (bpp==4)
        {
            if (x&1)
            {
                *d |= (unsigned char) ((*s&15)<<4);
                ++d;
            }
            else
                *d = (unsigned char) ((*s&15));
            ++s;
        }
        else if (bpp==8)
        {
            *d++ = *s++;  /* translate grey scale */
        }
        else if (bpp==15 || bpp==16)
        {
            unsigned short rgb;

            rgb = TIMpixel8To5((ARGB *)s);
            d[0] = (unsigned char) (rgb&255);
            d[1] = (unsigned char) (rgb>>8);
            d += 2;
            s += 4;
        }
        else if (bpp==24)
        {
            d[2] = ((ARGB *) s)->b;
            d[1] = ((ARGB *) s)->g;
            d[0] = ((ARGB *) s)->r;
            d += 3;
            s += 4;
        }
    }
}

int GCALL TIM_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL TIM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    char *linebuf;
    char *lineptr;
    int  width;
    int  height;
    int  bpp;
    int  bpl;
    int  colours;
    int  x;
    int  y;
    int  w;
    int  h;
    int  ok;
    int  i;
    unsigned int flags=0;
    char *s;
    TIMHeaderStruct timheader;
    TIMClutStruct   timclut;
    TIMPixelStruct  timpixel;
    unsigned char palbuf[256*2];

    colours = ginfo->numcolours;
    width = ginfo->width;
    height = ginfo->height;
    bpp = ginfo->originalbpp;

         if (bpp<=4)  bpp=4;
    else if (bpp<=8)  bpp=8;
    else if (bpp<=16) bpp=16;
    else              bpp=24;

    bpl = ((width*bpp+15)&~15)/8;

    switch (bpp)
    {
        case 4:  flags = TIM_16_COLORS|TIM_CLUT_PRESENT;  break;
        case 8:  flags = TIM_256_COLORS|TIM_CLUT_PRESENT;  break;
        case 16: flags = TIM_15_BIT_RGB;  break;
        case 24: flags = TIM_24_BIT_RGB;  break;
    }

    /* create palette of 16 deep if 4 or 8 bit deep */

    for (i=0; i<colours; ++i)
    {
        unsigned short rgb;

        rgb = (unsigned short) (TIMpixel8To5(&ginfo->colourtbl[i]));

        palbuf[i*2+0] = (unsigned char) (rgb&255);
        palbuf[i*2+1] = (unsigned char) (rgb>>8);
    }

    /* TIM FILE HEADER structure 8 bytes */

    TIMPUTFIELD(timheader.id, 0x00000010);                  /* ID constant */
    TIMPUTFIELD(timheader.flags, flags);                    /* flags (depth) */
    ok = (8 == gwrite(gx->gstream, (char *) &timheader, 8));

    /* TIM CLUT HEADER structure 12+clut bytes */

    if (bpp<=8)
    {
        GINFOOLDSHP *ginfoshp = (GINFOOLDSHP *)ginfo;
        int clutx=0;
        int cluty=240;

        if ((ginfo->signature==GMAKEID('.','T','I','M') || ginfo->signature==GMAKEID('S','H','P','P'))
         && ginfo->size==sizeof(GINFOOLDSHP))
        {
            clutx = ginfoshp->clutx;
            cluty = ginfoshp->cluty;
        }

        TIMPUTFIELD(timclut.length, 12+colours*2);            /* size of clut header and entries */
        TIMPUTFIELD(timclut.x_pos, clutx);
        TIMPUTFIELD(timclut.y_pos, cluty);
        TIMPUTFIELD(timclut.width, colours);         /* number of clut entries for width */
        TIMPUTFIELD(timclut.height, 1);              /* clut is 1 word height */
        ok &= (12 == gwrite(gx->gstream, (char *) &timclut, 12));

        if (bpp<=8)
            ok &= (colours*2 == gwrite(gx->gstream, palbuf, colours*2));
    }

    /* TIM CLUT HEADER structure 12+pixel bytes */

    x = ginfo->defaultx;
    y = ginfo->defaulty;
    w = (ginfo->width*bpp+15)/16;   /* width in words */
    h = ginfo->height;              /* height in pixels */

    if (!x && !y)   /* bad place to put images! */
    {
        x = 640;
        y = 0;
    }

    TIMPUTFIELD(timpixel.length, 12+bpl*height);  /* size of pixel header and entries */
    TIMPUTFIELD(timpixel.x_pos, x);
    TIMPUTFIELD(timpixel.y_pos, y);
    TIMPUTFIELD(timpixel.width, w);   /* width in words */
    TIMPUTFIELD(timpixel.height, h);  /* height in pixels */
    ok &= (12 == gwrite(gx->gstream, (char *) &timpixel, 12));

    linebuf = (char *)galloc(bpl);
    if (linebuf)
    {
        for (y=0; y<height && ok; ++y)
        {
            lineptr = linebuf;
            s = source+y*rowbytes;

            TIMwriteline(s, linebuf, width, bpp);
            ok &= (bpl == gwrite(gx->gstream,linebuf,bpl));
        }

        gfree(linebuf);
    }


    return(ok);
}

int GCALL TIM_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


