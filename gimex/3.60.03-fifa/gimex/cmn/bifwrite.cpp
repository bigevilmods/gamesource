// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "bifgimex.h"


/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

/* text functions stolen from EPSwrite */

static int BIFout(GSTREAM *outf, char *str, int n, int donum)
{
    char tempstr[256];
    char *d;
    int ok=0;
    int base;
    int dig;
    int out=0;
    int len;

    strcpy(tempstr,str);
    d = tempstr+strlen(tempstr);

    if (donum)
    {
        base = 1000000000L;

        if (n<0)
        {
            *d++ = '-';
            n = -n;
        }

        while (base)
        {
            dig = n/base;
            if (dig || base==1)
                out = 1;
            if (out)
                *d++ = (char)(dig+'0');
            n -= dig*base;
            base /= 10;
        }
    }
    *d++ = '\n';
    *d = 0;

    len = strlen(tempstr);
    ok = gwrite(outf,tempstr,len)==len;
    return(ok);
}

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL BIF_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;
    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream  = g;
        gx->framenum = 0;
        gx->gref     = (char *)pathname;
        *gxptr       = gx;
        ok           = 1;
    }
    return(ok);
}

int GCALL BIF_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    char *linebuf    = NULL;
    char *lineptr    = NULL;
    int bpp          = 0;
    int bpl          = 0;
    int colours      = 0;
    int x            = 0;
    int y            = 0;
    int ok           = 0;
    unsigned char *s = NULL;
    GSTREAM *g       = NULL;
    char fname[256];
    char fnamestr[256];
    unsigned char rawheader[4];

    if (ginfo->bpp>8)
    {
        colours = ginfo->numcolours;
        bpp = (ginfo->bpp==8)?8:24;
        bpl = ginfo->width*bpp/8;
        g = gx->gstream;
        strcpy(fname,(const char *) gx->gref);
        if (strchr(fname,'.'))
           *strchr(fname,'.') = 0;
        strcat(fname,".raw");

        strcpy(fnamestr,"filename  ");
        strcat(fnamestr,fname);

        ok =  BIFout(g,"BIF",-1,0);
        ok &= BIFout(g,fnamestr,-1,0);
        ok &= BIFout(g,"width    ",ginfo->width,1);
        ok &= BIFout(g,"height   ",ginfo->height,1);
        ok &= BIFout(g,"header   ",4,1);

        ok &= BIFout(g,"channels ",3,1);
        ok &= BIFout(g,"order    rgb",-1,0);
        ok &= BIFout(g,"leftpadding  ",0,1);
        ok &= BIFout(g,"rightpadding ",0,1);

        if (ok)
        {
            ok = 0;
            g = gwopen(fname);
            if (g)
            {
                gputi(rawheader+0,ginfo->width,2);   /* Width of bitmap in pixels */
                gputi(rawheader+2,ginfo->height,2);  /* Height of bitmap in pixels */
                ok = gwrite(g,rawheader,4);

                bpl = ginfo->width*3;
                linebuf = (char *) galloc(bpl);
                if (linebuf)
                {
                    for (y=0; y<ginfo->height && ok; ++y)
                    {
                        lineptr = linebuf;
                        s = (unsigned char *) (source+rowbytes*y);

                        if (bpp==24)
                        {
                            for (x=0; x<ginfo->width; ++x)
                            {
                                lineptr[0] = ((ARGB *) s)->r;
                                lineptr[1] = ((ARGB *) s)->g;
                                lineptr[2] = ((ARGB *) s)->b;
                                s += 4;
                                lineptr += 3;
                            }
                        }
                        else
                        {
                            for (x=0; x<ginfo->width; ++x)
                            {
                                lineptr[0] = ginfo->colourtbl[(int)*s].r;
                                lineptr[1] = ginfo->colourtbl[(int)*s].g;
                                lineptr[2] = ginfo->colourtbl[(int)*s].b;
                                ++s;
                                lineptr += 3;
                            }
                        }
                        ok = gwrite(g,linebuf,bpl);
                    }
                    gfree(linebuf);
                }
            }
        }
    }
    return(ok);
}

int GCALL BIF_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

