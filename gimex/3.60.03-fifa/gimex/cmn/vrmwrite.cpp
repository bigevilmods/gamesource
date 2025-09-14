// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "vrmgimex.h"

#define TGA_wopen TGA_vrmwopen
#define TGA_write TGA_vrmwrite
#define TGA_wclose TGA_vrmwclose

#include "tgawrite.cpp"

/****************************************************************/
/*  Internal Functions                                           */
/****************************************************************/

static void VRMdrawrect(ARGB *frame,int sx,int sy,int w,int h,int colour)
{
    int *d;
    int x;
    int y;


    for (y=0; y<h; ++y)
    {
        d = (int *)frame+(y+sy)*1024+sx;
        for (x=0; x<w; ++x)
        {
            *d++ = colour;
        }
    }
}

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL VRM_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    int ok;
    GINSTANCE *gx;
    ARGB *frame;

    ok = TGA_wopen(&gx,g,pathname, 1);
    if (ok)
    {
        frame = (ARGB *) galloc(1024*512*4);
        if (frame)
        {
            gx->gref = (char *) frame;
            gx->framenum = 0;
            memset(frame,0,1024*512*4);
            *gxptr = gx;

            /* draw where screens normally go for 320x240 */

            VRMdrawrect(frame,0,0,320,240,0x00000040);
            VRMdrawrect(frame,0,256,320,240,0x00000040);
            VRMdrawrect(frame,320,0,320,240,0x00000020);
            VRMdrawrect(frame,320,256,320,240,0x00000020);

        }
        else
        {
            gfree((char *)gx);
            ok = 0;
        }
    }
    return(ok);
}

int GCALL VRM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int x;
    int y;
    int sx;
    int sy;
    int w;
    int h;
    int bpp;
    int si;
    ARGB *frame;
    ARGB *d;
    char *s;
    int ok=0;
    int clip;

    frame = (ARGB *) gx->gref;
    if (frame)
    {
        sx = ginfo->defaultx;
        sy = ginfo->defaulty;
        w = ginfo->width;
        h = ginfo->height;

        bpp=4;
        if (ginfo->originalbpp>4) bpp=8;
        if (ginfo->originalbpp>8) bpp=16;
        if (ginfo->originalbpp>16) bpp=24;

        w = (w*bpp+15)/16;  /* convert to psx pixels */


        clip = 0-sx;
        if (clip>0)
        {
            sx += clip;
            w -= clip;
        }

        clip = (sx+w)-1024;
        if (clip>0)
        {
            w -= clip;
        }

        clip = 0-sy;
        if (clip>0)
        {
            sy += clip;
            h -= clip;
        }

        clip = (sy+h)-512;
        if (clip>0)
        {
            h -= clip;
        }

        if (h>0 && w>0)
        {
            for (y=0; y<h; ++y)
            {
                d = frame+(y+sy)*1024+sx;
                s = source+y*rowbytes;
                for (x=0; x<w; ++x)
                {
                    si = x*16/bpp;  /* calculate source index for point sampling */

                    if (ginfo->subtype)
                    {
                        if (*(int *)d)
                        {
                            d->r += 0xb8;
                            d->g += 0x48;
                            d->b -= 0x18;
                        }
                        d->r |= (gx->framenum+1)*33;
                        d->g |= 255-(gx->framenum+1)*33;
                        d->b |= (gx->framenum+1)*0xb8;
                        ++d;
                    }
                    else
                    {
                        if (ginfo->bpp==8)
                        {
                            if (*(int *)d)
                            {
                                d->r += 0xb8;
                                d->g += 0x48;
                                d->b -= 0x18;
                            }
                            else
                            {
                                if (bpp==4)
                                {
                                    d->r = (ginfo->colourtbl[*(s+si)].r + ginfo->colourtbl[*(s+si+1)].r + ginfo->colourtbl[*(s+si+2)].r + ginfo->colourtbl[*(s+si+3)].r)/4;
                                    d->g = (ginfo->colourtbl[*(s+si)].g + ginfo->colourtbl[*(s+si+1)].g + ginfo->colourtbl[*(s+si+2)].g + ginfo->colourtbl[*(s+si+3)].g)/4;
                                    d->b = (ginfo->colourtbl[*(s+si)].b + ginfo->colourtbl[*(s+si+1)].b + ginfo->colourtbl[*(s+si+2)].b + ginfo->colourtbl[*(s+si+3)].b)/4;
                                }
                                else
                                {
                                    d->r = (ginfo->colourtbl[*(s+si)].r + ginfo->colourtbl[*(s+si+1)].r)/2;
                                    d->g = (ginfo->colourtbl[*(s+si)].g + ginfo->colourtbl[*(s+si+1)].g)/2;
                                    d->b = (ginfo->colourtbl[*(s+si)].b + ginfo->colourtbl[*(s+si+1)].b)/2;
                                }
                                ++d;
                            }
                        }
                        else
                        {
                            if (*(int *)d)
                            {
                                d->r += 0xb8;
                                d->g += 0x48;
                                d->b -= 0x18;
                            }
                            else
                            {
                                *d++ = *((ARGB *)s + si);
                            }
                        }
                    }
                }
            }
        }
        ok = 1;
    }

    ++gx->framenum;
    return(ok);
}

int GCALL VRM_wclose(GINSTANCE *gx)
{
    int ok=0;
    GINFO *ginfo;
    ARGB *frame;

    frame = (ARGB *) gx->gref;
    if (frame)
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo,0,sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('V','R','A','M');   /* Gimex type */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = 200;
            ginfo->framenum         = 0;    /* Current frame */
            ginfo->width            = 1024; /* Width of bitmap in pixels */
            ginfo->height           = 512;  /* Height of bitmap in pixels */
            ginfo->bpp              = 32;   /* Bits per pixel (8 or 32) */
            /*ginfo->indexed          = 0;    \* 0 no, 1 yes (indexed meaning it uses a palette) *\ */
            ginfo->originalbpp      = 16;   /* Bits per pixel in original image (1 to 32) */
            /*ginfo->originalindexed  = 0;    \* 0 no, 1 yes (indexed meaning it used a palette) *\ */
            ginfo->numcolours  = 0;    /* Number of colours in original indexed palette */
            ginfo->subtype          = 0;    /* Internal format sub-type 0-default */
            ginfo->packed           = 0;    /* Type of packing on original image. 0 none, 1 run, n other */
            ginfo->quality          = 100;  /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = 0;    /* Number of bits in alpha channel */
            ginfo->redbits          = 8;    /* Number of bits in red channel */
            ginfo->greenbits        = 8;    /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;    /* Number of bits in blue channel */
            ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;    /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */

            ok = TGA_write(gx,ginfo,(char *)frame,1024*4);
            ok &= TGA_wclose(gx);

            gfree((char *)ginfo);			
        }
		gfree((char *)frame);
    }
    return(ok);
}

#undef TGA_wopen
#undef TGA_write
#undef TGA_wclose


