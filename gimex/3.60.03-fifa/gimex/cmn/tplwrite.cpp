// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "tplgimex.h"
#include "pshgimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

static unsigned short TPLpixel8To5a3(const ARGB *aRgb8)
{
    unsigned int a;
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int argb;

    if (aRgb8->a<0xe0) // 3444 - only 3 bits of alpha significant
    {
        a = (int)aRgb8->a>>(8-3);
        r = (int)aRgb8->r>>(8-4);
        g = (int)aRgb8->g>>(8-4);
        b = (int)aRgb8->b>>(8-4);
        argb = (0<<15)|(a<<12)|(r<<8)|(g<<4)|b;
    }
    else    // 555
    {
        r = (int)aRgb8->r>>(8-5);
        g = (int)aRgb8->g>>(8-5);
        b = (int)aRgb8->b>>(8-5);

        argb = (1<<15)|(r<<10)|(g<<5)|b;
    }
    return((unsigned short) argb);
}

static unsigned short TPLpixel8To565(const ARGB *aRgb8)
{
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int rgb;

    r = (int)aRgb8->r>>(8-5);
    g = (int)aRgb8->g>>(8-6);
    b = (int)aRgb8->b>>(8-5);

    rgb = (r<<11)|(g<<5)|b;
    return((unsigned short) rgb);
}

static int TPLgetpixeloffset(int x, int y, int width, int bpp)
{
    int offset=0;

    switch (bpp)
    {
        case 4: // 8x8
            offset = 8*8/2*(width/8)*(y/8) + 8*8/2*(x/8) + 8/2*(y&7) + (x&7)/2;
            break;


        case 8: // 8x4
            offset = 8*4*1*(width/8)*(y/4) + 8*4*1*(x/8) + 1*8*(y&3) + 1*(x&7);
            break;

        default:
        case 16: // 4x4
            offset = 4*4*2*(width/4)*(y/4) + 4*4*2*(x/4) + 2*4*(y&3) + 2*(x&3);
            break;

        case 32: // 2 blocks of 4x4.  gives offset to AR. +4*4*2 for GB
            offset = 4*4*4*(width/4)*(y/4) + 4*4*4*(x/4) + 2*4*(y&3) + 2*(x&3);
            break;

    }
    return(offset);
}

int GCALL TPL_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;
    unsigned char tplheader[12];

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        *gxptr = gx;

        // interm header.. patch up on close
        gputm(tplheader+0,0x009ac12f,4);    // id
        gputm(tplheader+4,0,4);             // framecount
        gputm(tplheader+4,0,4);             // offset to descriptors

        ok = gwrite(gx->gstream, (char *) &tplheader, 12);   /* write file header id/flags */
    }
    return(ok);
}

int GCALL TPL_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int ok=1;
    int imageoffset=0;
    int clutoffset=0;
    int imagedataoffset;
    int clutdataoffset;
    int width;
    int height;
    int awidth;
    int aheight;
    int bpp;
    int colours;
    int i;
    int x;
    int y;
    unsigned short rgb;
    unsigned char *framebuf;
    unsigned char *descriptors;
    unsigned char *s;
    unsigned char *d;
    unsigned char clutbuf[256*4];
    int clutbpp=16;
    GXTexFmt imageformat;
    GXTlutFmt clutformat;
    TPLClutStruct tplclut;
    TPLImageStruct tplimage;

    memset(&tplimage,0,sizeof(tplimage));
    memset(&tplclut,0,sizeof(tplclut));

    width   = ginfo->width;
    height  = ginfo->height;
    bpp     = ginfo->originalbpp;
    awidth = (width+3)&~3;
    aheight = (height+3)&~3;
    if (bpp<=8)
    {
        if (bpp<=4)
        {
            imageformat = (GXTexFmt) GX_TF_C4;
            bpp = 4;
        }
        else
        {
            imageformat = (GXTexFmt) GX_TF_C8;
            bpp = 8;
        }
        if (ginfo->alphabits>3 && ginfo->subtype)
        {
            clutformat = GX_TL_RGBA8;   // EA extension
            clutbpp = 32;
        }
        else if (ginfo->alphabits)
        {
            clutformat = GX_TL_RGB5A3;
            clutbpp = 16;
        }
        else
        {
            clutformat = GX_TL_RGB565;
            clutbpp = 16;
        }

        colours = ginfo->numcolours;
        clutoffset = (int) gtell(gx->gstream);
        imageoffset = clutoffset + sizeof(tplclut);
        clutdataoffset = (imageoffset + sizeof(tplimage)+31)&~31;
        imagedataoffset = (clutdataoffset+colours*clutbpp/8+31)&~31;

        TPLPUTFIELD(tplclut.NumberOfEntries,colours);   /* clut entries */
        TPLPUTFIELD(tplclut.Pad,0);                 /* reserved */
        TPLPUTFIELD(tplclut.ClutFormat,clutformat); /* clut format */
        TPLPUTFIELD(tplclut.Offset,clutdataoffset); /* offset to clut entries */

        ok = gwrite(gx->gstream, (char *) &tplclut, sizeof(tplclut));

        /* create palette */

        if (clutformat == GX_TL_RGBA8)  // TEV based indirect into 32 bit image to implement clut
        {
            memset(clutbuf,0xfb,sizeof(clutbuf));
            s = (unsigned char *) &ginfo->colourtbl[0];
            for (x=0; x<16; ++x)
            {
                for (y=0; y<16; ++y)         // note bigendian format so backwards.
                {
                    d = &clutbuf[0] + TPLgetpixeloffset(x,y,16,32);

                    d[0] =      ((ARGB *) s)->a;
                    d[1] =      ((ARGB *) s)->r;
                    d[16*2+0] = ((ARGB *) s)->g;
                    d[16*2+1] = ((ARGB *) s)->b;
                    s += 4;
                }
            }
        }
        else
        {
            for (i=0; i<colours; ++i)
            {
                if (clutformat == GX_TL_RGB5A3)
                    rgb = (unsigned short) (TPLpixel8To5a3(&ginfo->colourtbl[i]));
                else
                    rgb = (unsigned short) (TPLpixel8To565(&ginfo->colourtbl[i]));

                gputm(clutbuf+i*2,rgb,2);
            }
        }
    }
    else
    {
        if (ginfo->alphabits<=0 && ginfo->redbits<=5 && ginfo->greenbits<=6 && ginfo->bluebits<=5)
        {
            imageformat = GX_TF_RGB565;
            bpp = 16;
        }
        else if (ginfo->alphabits<=3 && ginfo->redbits<=5 && ginfo->greenbits<=5 && ginfo->bluebits<=5)
        {
            imageformat = GX_TF_RGB5A3;
            bpp = 16;
        }
        else
        {
            imageformat = GX_TF_RGBA8;
            bpp = 32;
        }
        colours = 0;
        clutoffset = 0;
        imageoffset = (int) gtell(gx->gstream);
        clutdataoffset = 0;
        imagedataoffset = (imageoffset+sizeof(tplimage)+31)&~31;
    }

    TPLPUTFIELD(tplimage.Height,height);           /* height of image in pixels */
    TPLPUTFIELD(tplimage.Width,width);             /* width of image in pixels */
    TPLPUTFIELD(tplimage.ImageFormat,imageformat); /* image format */
    TPLPUTFIELD(tplimage.Offset,imagedataoffset);  /* offset to texels */
    TPLPUTFIELD(tplimage.WrapS,0);             /* wrap S */
    TPLPUTFIELD(tplimage.WrapT,0);             /* wrap T */
    TPLPUTFIELD(tplimage.MinFilter,0);         /* minification filtering */
    TPLPUTFIELD(tplimage.MagFilter,0);         /* magnification filtering */
    TPLPUTFIELD(tplimage.LODBias,0);           /* bias lod calculation */
    TPLPUTFIELD(tplimage.EdgeLODEnable,0);     /* avoid LOD on edge texels? */
    TPLPUTFIELD(tplimage.ImageMinLOD,0);       /* minimum LOD level */
    TPLPUTFIELD(tplimage.ImageMaxLOD,0);       /* maximum LOD level */
    TPLPUTFIELD(tplimage.Pad,0);               /* reserved */

    ok = gwrite(gx->gstream, (char *) &tplimage, sizeof(tplimage));

    while (gtell(gx->gstream)&31)   // align to 32 bytes
        gwrite(gx->gstream,&tplimage,1);

    ok = gwrite(gx->gstream, clutbuf, (colours*(clutbpp/8)+31)&~31);

    framebuf = (unsigned char *) galloc(awidth*aheight*bpp/8);
    if (framebuf)
    {
        memset(framebuf,0,awidth*aheight*bpp/8);

        for (y=0; y<height; ++y)
        {
            s = (unsigned char *) source + y*rowbytes;
            for (x=0; x<width; ++x)
            {
                d = framebuf+TPLgetpixeloffset(x,y,awidth,bpp);

                switch (imageformat)
                {
                    case GX_TF_C4: // 4 bit indexed
                    case GX_TF_I4: // 4 bit intensity
                        if (x&1)
                        {
                            *d |= *s;
                            ++s;
                        }
                        else
                            *d = (unsigned char) (*s<<4);
                        break;

//                  case GX_TF_IA8: // intensity/alpha 88

                    case GX_TF_IA4: // intensity/alpha 44
                    case GX_TF_C8: // 8 bit indexed
                    case GX_TF_I8: // 8 bit intensity
                        *d = *s++;
                        break;

                    case GX_TF_RGB565: // 565
                        rgb = TPLpixel8To565((ARGB *)s);
                        gputm(d,rgb,2);
                        s += 4;
                        break;

                    case GX_TF_RGB5A3: // 3444/555
                        rgb = TPLpixel8To5a3((ARGB *)s);
                        gputm(d,rgb,2);
                        s += 4;
                        break;

                    case GX_TF_RGBA8: // 8888
                        d[0] =      ((ARGB *) s)->a;
                        d[1] =      ((ARGB *) s)->r;
                        d[16*2+0] = ((ARGB *) s)->g;
                        d[16*2+1] = ((ARGB *) s)->b;
                        s += 4;
                        break;
                    default:
                        break;
                }
            }
        }

        ok = gwrite(gx->gstream, framebuf, awidth*aheight*bpp/8);
        gfree(framebuf);
    }

    descriptors = (unsigned char *)galloc((gx->framenum+1)*8);
    if (gx->gref)
    {
        memcpy(descriptors,gx->gref,(gx->framenum)*8);
        gfree(gx->gref);
    }
    gx->gref = descriptors;
    gputm((char *) gx->gref + (gx->framenum)*8+0,imageoffset,4);    // image header offset
    gputm((char *) gx->gref + (gx->framenum)*8+4,clutoffset, 4);    // clut header offset
    ++gx->framenum;

    return(ok);
}

int GCALL TPL_wclose(GINSTANCE *gx)
{
    int ok=0;
    int offset;
    unsigned char tplheader[12];

    if (gx)
    {
        offset = (int) gtell(gx->gstream);    // offset to descriptor table

        // write descriptor table
        if (gx->gref)
        {
            gwrite(gx->gstream,gx->gref,gx->framenum*8);  // descriptors
            ok = gfree(gx->gref);
        }

        // interm header.. patch up on close
        gputm(tplheader+0,0x0020af30,4);    // id
        gputm(tplheader+4,gx->framenum,4);  // framecount
        gputm(tplheader+8,offset,4);        // offset to descriptors

        gseek(gx->gstream,(GPOS)0);
        ok = gwrite(gx->gstream, (char *) &tplheader, 12);   /* write file header id/flags */

        ok = gfree((void *) gx);
    }

    return(ok);
}


