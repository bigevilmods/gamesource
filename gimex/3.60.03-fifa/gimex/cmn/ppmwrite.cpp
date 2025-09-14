// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include <math.h>
#include "gimex.h"
#include "ppmgimex.h"

#if defined(__APPLE__)
inline float powf(float b, float e)
{
	return (float) pow((double) b, (double) e);
}
#endif

/* PPM is a unix file format, so newline is 10 (linefeed) */

#define NEWLINE ((char)10)

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static int PPMwriteval(GSTREAM *g, int val, int asciimode)
{
    unsigned char buf[8];
    int ok;
    int i=0;

    if (asciimode)
    {
        if (val>=10000)
        {
            buf[i++] = (unsigned char) (val/10000+'0');
            val = val % 10000;
        }
        if (val>=1000 || i)
        {
            buf[i++] = (unsigned char) (val/1000+'0');
            val = val % 1000;
        }
        if (val>=100 || i)
        {
            buf[i++] = (unsigned char) (val/100+'0');
            val = val % 100;
        }
        if (val>=10 || i)
        {
            buf[i++] = (unsigned char) (val/10+'0');
            val = val % 10;
        }
        buf[i++] = (unsigned char) (val+'0');
        buf[i++] = (asciimode>1) ? NEWLINE : ' ';
    }
    else
    {
        buf[i++] = (unsigned char) val;
    }

    ok = gwrite(g,buf,i)==i;
    return(ok);
}

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL PPM_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

int GCALL PPM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int  nl=0;
    int  x;
    int  y;
    int  ok=0;
    int  asciimode;
    int  ppmtype;
    ARGB *s;
    char buf[4];

    if (ginfo->bpp>8)
    {
        asciimode = ginfo->subtype?1:0;
        ppmtype = (asciimode) ? 3:6;
        if (ginfo->subtype==2)
            ppmtype = 7;


        buf[0] = 'P';
        buf[1] = (char)('0'+ppmtype);
        buf[2] = NEWLINE;

        if (gwrite(gx->gstream,buf,3))
        {
            buf[0] = NEWLINE;
            ok =  PPMwriteval(gx->gstream,ginfo->width ,1);
            ok &= PPMwriteval(gx->gstream,ginfo->height,2);
            ok &= PPMwriteval(gx->gstream,255          ,2);

            for (y=0; y<ginfo->height && ok; ++y)
            {
                s = (ARGB *) (source+rowbytes*y);

                for (x=0; x<ginfo->width; ++x)
                {
                    /* newline every 8 pixels (8 pix * 3 chan/pix * 4 byte\chan == 96 bytes per line) */

                    if (asciimode)
                    {
                        nl = 0;
                        if ((((x+1)%8)==0)) // || (x==(ginfo->width-1)))
                            nl = 1;
                    }

                    if (ppmtype==7)
                    {
                        int Y0,Cb,Cr,Y1,Cb1,Cr1;
                        int Red,Green,Blue;
#define GAMMA 1.0f/1.7f
                        Red =   (int)(powf((float)s->r,GAMMA)*255/pow((float)255,GAMMA));
                        Green = (int)(powf((float)s->g,GAMMA)*255/pow((float)255,GAMMA));
                        Blue =  (int)(powf((float)s->b,GAMMA)*255/pow((float)255,GAMMA));

                        Y0  = (int)( 0.2570*Red+ 0.5040*Green+ 0.0980*Blue);
                        Cb  = (int)(-0.1480*Red+ -0.2910*Green+ 0.4390*Blue);
                        Cr  = (int)( 0.4390*Red-0.3680*Green-0.0710*Blue);
                        ++x;
                        ++s;

                        Red =   (int)(powf((float)s->r,GAMMA)*255/pow((float)255,GAMMA));
                        Green = (int)(powf((float)s->g,GAMMA)*255/pow((float)255,GAMMA));
                        Blue =  (int)(powf((float)s->b,GAMMA)*255/pow((float)255,GAMMA));

                        Y1  = (int)( 0.2570*Red+ 0.5040*Green+ 0.0980*Blue);
                        Cb1 = (int)(-0.1480*Red+-0.2910*Green+ 0.4390*Blue);
                        Cr1 = (int)( 0.4390*Red-0.3680*Green-0.0710*Blue);
#undef GAMMA

                        Y0 = gmin(255-16,gmax(16,((Y0*(255-32))+128)/255+16));
                        Y1 = gmin(255-16,gmax(16,((Y1*(255-32))+128)/255+16));
                        Cb  = (Cb + Cb1)/2;
                        Cr  = (Cr + Cr1)/2;

                        ok =  PPMwriteval(gx->gstream,Y0,asciimode);
                        ok &= PPMwriteval(gx->gstream,Cb+128,asciimode);
                        ok &= PPMwriteval(gx->gstream,Y1,asciimode);
                        ok &= PPMwriteval(gx->gstream,Cr+128,asciimode+nl);
                    }
                    else
                    {
                        ok =  PPMwriteval(gx->gstream,s->r,asciimode);
                        ok &= PPMwriteval(gx->gstream,s->g,asciimode);
                        ok &= PPMwriteval(gx->gstream,s->b,asciimode+nl);
                    }
                    ++s;
                }
            }
        }
    }
    return(ok);
}

int GCALL PPM_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

