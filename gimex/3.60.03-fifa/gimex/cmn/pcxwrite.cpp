// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "pcxgimex.h"


/* RLE encodes 'n' bytes from source to dest.  Returns number of bytes output to source */
/* skip says how many bytes of the source should be skipped for each pixel */

static int PCX_encoderow(unsigned char *dest, unsigned char *source, int skip, int n, int packed)
{
    int i;
    int count=0;

    if (packed)
    {
        unsigned char pixel;
        int runcount=1;
        int outputrun;

        pixel = *source;
        source += skip;

        for (i=1; i<n; ++i)
        {
            outputrun = 0;

            /* If the pixel is part of a run increase the runcount */
            if (*source == pixel)
            {
                runcount++;

                /* If we have exceeded the run count, output it, ignoring the current pixel. */
                if (runcount == 64)
                {
                    runcount = 63;
                    outputrun = 1;
                }
            }
            else
            {
                /* If there is a run of more than one pixel or the 1 pixel must be in a run */
                if ( (runcount > 1) || ((pixel & 0xC0)==0xC0) )
                {
                    outputrun = 1;
                }
                else
                {
                    /* Output the last pixel with no codes */
                    dest[count++] = pixel;
                    runcount = 1;
                    pixel = *source;
                }
            }

            if (outputrun)
            {
                dest[count++] = (unsigned char) (runcount | 0xC0);
                dest[count++] = pixel;
                runcount = 1;
                pixel = *source;
            }

            /* Move on to the next pixel */
            source+=skip;
        }

        /* Output the last run */
        if ( (runcount > 1) || ((pixel & 0xC0)==0xC0) )
        {
            dest[count++] = (unsigned char) (runcount | 0xC0);
            dest[count++] = pixel;
        }
        else
        {
            /* Output the last pixel with no codes */
            dest[count++] = pixel;
        }
    }
    else
    {
        for (i=0; i<n; ++i)
        {
            if (*source & 0xc1)
                dest[count++] = 0xC1;
            dest[count++] = *source;
            source += skip;
        }
    }

    return (count);
}

/****************************************************************/
/*  export Functions                                            */
/****************************************************************/

int GCALL PCX_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    unsigned char *dcxheader;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        gx->frames = numframes;
        dcxheader = (unsigned char *) galloc(4+numframes*4+4);
        if (dcxheader)
        {
            ok = 1;
            gx->gref = dcxheader;
            if (numframes>1)
            {
                memset(dcxheader,0,4+numframes*4+4);
                gputi(dcxheader+0,0x3ade68b1,4);
                gputi(dcxheader+4,4+numframes*4+4,4);
                ok = gwrite(gx->gstream, dcxheader, 4+numframes*4+4);
            }
            *gxptr = gx;
        }
    }
    return(ok);
}

int GCALL PCX_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int ok     = 0;
    int i      = 0;
    int y      = 0;
    int bpp    = ginfo->bpp;
    int planes = 0;
    int pal    = 0;
    PCXHeaderStruct pcxheader;
    unsigned char *dcxheader;
    unsigned char *linebuf;
    int count;
    int dpi;
    int extplanes= (ginfo->subtype==1 && ginfo->alphabits)?4:3;

    if (gx->framenum < gx->frames)
    {

        dcxheader = (unsigned char *) gx->gref;
        gputi(dcxheader+4+gx->framenum*4,(unsigned int)gtell(gx->gstream),4);

        if (bpp>8)
        {
            planes = extplanes;
            pal = 2;
        }
        else
        {
            planes = 1;
            pal = 1;
            if (extplanes==4)
                pal = 3;  // EA extension
        }
        dpi = (int) ginfo->dpi;
        if (!dpi)
            dpi = 72;

        memset(&pcxheader,0,sizeof(pcxheader));
        PCXPUTFIELD(pcxheader.id,0x0a);              /* 0x0a for PCX */
        PCXPUTFIELD(pcxheader.version,5);            /* 0..5 */
        PCXPUTFIELD(pcxheader.encoding,1);           /* 0 none, 1 rle */
        PCXPUTFIELD(pcxheader.bpp,8);                /* bits per pixel per plane */
        PCXPUTFIELD(pcxheader.xmin,ginfo->defaultx); /* upper left x coordinate */
        PCXPUTFIELD(pcxheader.ymin,ginfo->defaulty); /* upper left y coordinate */
        PCXPUTFIELD(pcxheader.xmax,ginfo->defaultx+ginfo->width-1);  /* lower right x coordinate */
        PCXPUTFIELD(pcxheader.ymax,ginfo->defaulty+ginfo->height-1); /* lower right y coordinate */
        PCXPUTFIELD(pcxheader.hdotpinch,dpi);        /* horizontal dots per inch when printed */
        PCXPUTFIELD(pcxheader.vdotpinch,dpi);        /* vertical dots per inch when printed */
        PCXPUTFIELD(pcxheader.planes,planes);        /* number of planes 1 or 3 */
        PCXPUTFIELD(pcxheader.bpl,ginfo->width);     /* bytes per line per plane */
        PCXPUTFIELD(pcxheader.palinterp,pal);        /* 1=colour or b&w, 2=grey scale, 3=EA argb palette */
        PCXPUTFIELD(pcxheader.videosizex,0);         /* number of horizontal pixels-1 */
        PCXPUTFIELD(pcxheader.videosizey,0);         /* number of vertical pixels-1 */

        ok = gwrite(gx->gstream, (char *) &pcxheader, 128);

        if (ok)
        {
            ok = 0;

            /* Max RLE codes could be width*2*planes */
            linebuf = (unsigned char *)galloc(ginfo->width*2*planes);

            if (linebuf)
            {
                ok = 1;
                for (y=0; y<ginfo->height && ok; ++y)
                {
                    char *s = source+y*rowbytes;

                    count = 0;

                    if (bpp>8)
                    {
                        count += PCX_encoderow(&linebuf[count], &((ARGB *)s)->r, 4, ginfo->width, ginfo->packed);
                        count += PCX_encoderow(&linebuf[count], &((ARGB *)s)->g, 4, ginfo->width, ginfo->packed);
                        count += PCX_encoderow(&linebuf[count], &((ARGB *)s)->b, 4, ginfo->width, ginfo->packed);
                        if (planes==4)
                            count += PCX_encoderow(&linebuf[count], &((ARGB *)s)->a, 4, ginfo->width, ginfo->packed);
                    }
                    else
                    {
                        count = PCX_encoderow(linebuf, (unsigned char *)s, 1, ginfo->width, ginfo->packed);
                    }

                    ok = gwrite(gx->gstream,linebuf,count);
                }
                gfree(linebuf);


                if ((pal==1 || pal==3) && ok)
                {
                    unsigned char palbuf[1+256*4];
                    palbuf[0] = 0x0C;   /* Palette header. */

                    for (i=0; i<256; ++i)
                    {
                        palbuf[i*extplanes+0+1] = ginfo->colourtbl[i].r;
                        palbuf[i*extplanes+1+1] = ginfo->colourtbl[i].g;
                        palbuf[i*extplanes+2+1] = ginfo->colourtbl[i].b;
                        if (extplanes==4)
                            palbuf[i*extplanes+3+1] = ginfo->colourtbl[i].a;
                    }

                    ok = gwrite(gx->gstream, (char *) palbuf, 1+256*extplanes);
                }
            }
        }
        ++gx->framenum;
    }

    return(ok);
}

int GCALL PCX_wclose(GINSTANCE *gx)
{
    int ok=0;
    void *dcxheader;

    if (gx)
    {
        if (gx->gref)
        {
            if (gx->framenum>1)
            {
                int endpos;

                dcxheader = gx->gref;
                endpos    = (int)gtell(gx->gstream);
                gseek(gx->gstream,(GPOS)0);
                ok        = gwrite(gx->gstream, dcxheader, 4+gx->framenum*4+4);
                gseek(gx->gstream,endpos);
            }
            ok = gfree((void *) gx->gref);
        }
        gfree((void *) gx);
    }
    return(ok);
}


