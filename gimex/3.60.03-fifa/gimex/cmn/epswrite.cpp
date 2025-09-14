// (c) 1994-2007 Electronic Arts Inc.

#include <stdio.h>
#include "gimex.h"
#include "epsgimex.h"

#include <string.h>
#include <stdarg.h>

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

static int EPSprintf(GSTREAM *outf, char *format,...)
{
    va_list arglist;
    char tempstr[256];
    int ok=0;

    va_start(arglist, format);
    vsprintf(tempstr, format, arglist);
    ok = gwrite(outf,tempstr,strlen(tempstr));
    va_end(arglist);
    return(ok);
}

#define MAXLINEWIDTH 64

static int EPSwriteline(GSTREAM *g, void *src, int count, int bpp, const ARGB *pal, int bin, int isgrey)
{
    unsigned char *s= (unsigned char *) src;
    int i           = 0;
    int x           = 0;
    unsigned int hx = 0;
    unsigned int lx = 0;
    int ok          = 1;
    char nibbletbl[16];
    char buf[MAXLINEWIDTH+2];
    int channels;
    int channel;

    for (i=0; i<16; ++i)
        nibbletbl[i]= (char)(i<10?((char)(i+'0')):((char)(i-10+'a')));

    channels = isgrey?1:3;

    lx = 0;
    for (channel=0; channel<channels; ++channel)
    {
        if (bpp==32)
        {
            for (x=0; x<count && ok; ++x)
            {
                if (isgrey)
                    hx = (((ARGB *) s)->r*299+((ARGB *) s)->g*587+((ARGB *) s)->b*114)/1000;
                switch (channel)
                {
                    case 0: hx = ((ARGB *) s)->r; break;
                    case 1: hx = ((ARGB *) s)->g; break;
                    case 2: hx = ((ARGB *) s)->b; break;
                }
                if (bin)
                {
                    buf[lx++] = (char)hx;
                }
                else
                {
                    buf[lx++] = nibbletbl[hx>>4];
                    buf[lx++] = nibbletbl[hx&15];
                }
                s += 4;

                if (lx>=MAXLINEWIDTH)
                {
                    if (!bin)
                    {
                        buf[lx++] = '\n';
                    }
                    ok = gwrite(g,buf,lx);
                    lx = 0;
                }
            }
        }
        else if (bpp==8)
        {
            for (x=0; x<count && ok; ++x)
            {
                hx = *s++;

                if (isgrey)
                    hx = (pal[hx].r*299+pal[hx].g*587+pal[hx].b*114)/1000;
                switch (channel)
                {
                    case 0: hx = pal[hx].r; break;
                    case 1: hx = pal[hx].g; break;
                    case 2: hx = pal[hx].b; break;
                }

                if (bin)
                {
                    buf[lx++] = (char)hx;
                }
                else
                {
                    buf[lx++] = nibbletbl[hx>>4];
                    buf[lx++] = nibbletbl[hx&15];
                }

                if (lx>=MAXLINEWIDTH)
                {
                    if (!bin)
                    {
                        buf[lx++] = '\n';
                    }
                    ok = gwrite(g,buf,lx);
                    lx = 0;
                }
            }
        }
        if (ok)
        {
            if (!bin)
            {
                buf[lx++] = '\n';
            }
            if (lx)
                ok = gwrite(g,buf,lx);
        }
    }
    return(ok);
}

int GCALL EPS_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

#if 0
static int isgreyscale(int colours, ARGB *pal, char *xlat)
{
    int i;
    int ok=1;

    for (i=0; i<colours; ++i)
    {
        if (pal[i].r!=pal[i].g || pal[i].r!=pal[i].b)
            ok = 0;
        xlat[i] = pal[i].g;
    }
    if (!ok)
        for (i=0; i<colours; ++i)
            xlat[i] = i;

    return(ok);
}
#endif

int GCALL EPS_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int width;
    int height;
    int bpp;
    int colours;
    int y;
    int ok=0;
    int bin;
    int isgrey;
    char *s;
    GSTREAM *g;

    colours = ginfo->numcolours;
    width = ginfo->width;
    height = ginfo->height;
    bpp = ginfo->bpp;
    bin = (ginfo->subtype==1 && ginfo->signature==GMAKEID('.','E','P','S'));


    isgrey = 1;
#if 0
    isgrey = 0;
    if (bpp<=8)
    {
        isgrey = isgreyscale(colours, ginfo->colourtbl, xlat);
    }
#endif

    g = gx->gstream;
    ok =  EPSprintf(g,"%%!PS-Adobe-3.0 EPSF-3.0\n");
    ok &= EPSprintf(g,"%%%%BoundingBox: 0 0 %d %d\n",width,height);
    ok &= EPSprintf(g,"%%%%EndComments\n");
    ok &= EPSprintf(g,"%d %d translate\n",72*85/10/2-width/2,72*110/10/2-height/2);
    ok &= EPSprintf(g,"%d %d scale\n",width,height);

    ok &= EPSprintf(g,"/picstr0 %d string def\n",width);
    if (!isgrey)
    {
        ok &= EPSprintf(g,"/picstr1 %d string def\n",width);
        ok &= EPSprintf(g,"/picstr1 %d string def\n",width);
    }
    ok &= EPSprintf(g,"%d %d 8\n",width,height);
    ok &= EPSprintf(g,"[%d 0 0 -%d 0 %d]\n",width,height,height);
    if (isgrey)
    {
        if (bin)
            ok &= EPSprintf(g,"{currentfile picstr0 readstring pop}\n");
        else
            ok &= EPSprintf(g,"{currentfile picstr0 readhexstring pop}\n");

        ok &= EPSprintf(g,"image\n");
    }
    else
    {
        if (bin)
        {
            ok &= EPSprintf(g,"{currentfile picstr0 readstring pop}\n");
            ok &= EPSprintf(g,"{currentfile picstr1 readstring pop}\n");
            ok &= EPSprintf(g,"{currentfile picstr2 readstring pop}\n");
        }
        else
        {
            ok &= EPSprintf(g,"{currentfile picstr0 readhexstring pop}\n");
            ok &= EPSprintf(g,"{currentfile picstr1 readhexstring pop}\n");
            ok &= EPSprintf(g,"{currentfile picstr2 readhexstring pop}\n");
        }

        ok &= EPSprintf(g,"true 3 colorimage\n");
    }

    for (y=0; y<height && ok; ++y)
    {
        s = source+y*rowbytes;
        ok = EPSwriteline(g, s, width, bpp, ginfo->colourtbl, bin, isgrey);
    }
    ok = EPSprintf(g,"showpage\n");

    return(ok);
}

int GCALL EPS_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


