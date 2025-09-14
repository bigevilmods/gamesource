// (c) 1994-2007 Electronic Arts Inc.

#ifndef __TGAWRITE
#define __TGAWRITE 1

#include <string.h>
#include "gimex.h"
#include "tgagimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

#define TGAS(X) ggeti((char *)s+(X)*pixsize,pixsize)

static __inline int TGArlecount(void *s, int maxcount, int pixsize)
{
    int ct=0;

    if (maxcount > 128)
        maxcount = 128;

    while (ct<maxcount && TGAS(0)==TGAS(ct))
        ++ct;
    return(ct);
}

static __inline int TGAwriteline(void *src, void *dest, int count, int bpp, int sbpp, char *xlat, int pack, int alphabits)
{
    unsigned char *s=(unsigned char *)src;
    unsigned char *d=(unsigned char *)dest;
    int x;

    if (bpp==15)
        bpp=16;
    if (!pack)          /* uncompressed images */
    {
        for (x=0; x<count; ++x)
        {
            if (bpp==32)
            {
                d[0] = ((ARGB *) s)->b;
                d[1] = ((ARGB *) s)->g;
                d[2] = ((ARGB *) s)->r;
                d[3] = ((ARGB *) s)->a;
                d += 4;
                s += 4;
            }
            else if (bpp==24)
            {
                d[0] = ((ARGB *) s)->b;
                d[1] = ((ARGB *) s)->g;
                d[2] = ((ARGB *) s)->r;
                d += 3;
                s += 4;
            }
            else if (bpp==16)
            {
                unsigned int a=255;
                unsigned int r;
                unsigned int g;
                unsigned int b;
                unsigned int rgb;

                if (alphabits)
                    a = ((ARGB *) s)->a >> 7;
                r = ((ARGB *) s)->r >> 3;
                g = ((ARGB *) s)->g >> 3;
                b = ((ARGB *) s)->b >> 3;
                rgb = (a<<15)|(r<<10)|(g<<5)|b;
                gputi(d,rgb,2);
/*                d[0] = rgb&255; */
/*                d[1] = rgb>>8; */
                d += 2;
                s += 4;
            }
            else if (bpp==8)
            {
				if (sbpp==8)
	                *d++ = xlat[*s++];  /* translate grey scale */
				else
				{
	                *d++ = ((ARGB *) s)->r;
					s += 4;
				}
            }
        }
    }
    else                /* run length compressed images */
    {
        int ct;
        int ct2;
        int pixsize=bpp==8?1:4;

        while (count > 0)
        {
            /* run must be 3 of more, or 2 and followed by another run */

            ct  = TGArlecount(s, count, pixsize);
            ct2 = TGArlecount(s+ct*pixsize, count-ct, pixsize); // next count (could be 0)

            if (ct>=3 || (ct>=2 && (ct2>=2 || bpp>8)))
            {
                /* write out count */
                *d++ = (unsigned char)(0x80|ct-1);

                /* write out value */

                if (bpp==32)
                {
                    d[0] = ((ARGB *) s)->b;
                    d[1] = ((ARGB *) s)->g;
                    d[2] = ((ARGB *) s)->r;
                    d[3] = ((ARGB *) s)->a;
                    d += 4;
                    s += 4*ct;
                }
                else if (bpp==24)
                {
                    d[0] = ((ARGB *) s)->b;
                    d[1] = ((ARGB *) s)->g;
                    d[2] = ((ARGB *) s)->r;
                    d += 3;
                    s += 4*ct;
                }
                else if (bpp==16)
                {
                    unsigned int a=255;
                    unsigned int r;
                    unsigned int g;
                    unsigned int b;
                    unsigned int rgb;

                    if (alphabits)
                        a = ((ARGB *) s)->a >> 7;
                    r = ((ARGB *) s)->r >> 3;
                    g = ((ARGB *) s)->g >> 3;
                    b = ((ARGB *) s)->b >> 3;
                    rgb = (a<<15)|(r<<10)|(g<<5)|b;
                    gputi(d,rgb,2);
                    d += 2;
                    s += 4*ct;
                }
                else if (bpp==8)
                {
					if (sbpp==8)
					{
	                    *d++ = xlat[*s];  /* translate grey scale */
	                    s += ct;
					}
					else
					{
						*d++ = ((ARGB *) s)->r;
						s += 4*ct;
					}
                }

                count -= ct;
            }
            else
            {
                for (ct=1; ct<count && ct<128; ++ct)
                {
                    ct2 = TGArlecount(s+ct*pixsize, count-ct, pixsize);        // rle from here

                    if (ct2>=3 || (ct2>=2 && bpp>8))
                        break;
                }
                if (ct>=(count-3))
                    ct = count;
                if (ct>128)
                    ct = 128;

                /* write out count */
                *d++ = (unsigned char)(ct-1);

                /* copy string of pixels */

                while (ct--)
                {
                    --count;
                    if (bpp==32)
                    {
                        d[3] = ((ARGB *) s)->a;
                        d[2] = ((ARGB *) s)->r;
                        d[1] = ((ARGB *) s)->g;
                        d[0] = ((ARGB *) s)->b;
                        d += 4;
                        s += 4;
                    }
                    else if (bpp==24)
                    {
                        d[2] = ((ARGB *) s)->r;
                        d[1] = ((ARGB *) s)->g;
                        d[0] = ((ARGB *) s)->b;
                        d += 3;
                        s += 4;
                    }
                    else if (bpp==16)
                    {
                        unsigned int a=255;
                        unsigned int r;
                        unsigned int g;
                        unsigned int b;
                        unsigned int rgb;

                        if (alphabits)
                            a = ((ARGB *) s)->a >> 7;
                        r = ((ARGB *) s)->r >> 3;
                        g = ((ARGB *) s)->g >> 3;
                        b = ((ARGB *) s)->b >> 3;
                        rgb = (a<<15)|(r<<10)|(g<<5)|b;
                        gputi(d,rgb,2);
                        d += 2;
                        s += 4;
                    }
                    else if (bpp==8)
                    {
						if (sbpp==8)
						{
							*d++ = xlat[*s];  /* translate grey scale */
							s += 1;
						}
						else
						{
							*d++ = ((ARGB *) s)->r;
							s += 4;
						}
                    }
                }
            }
        }
    }
    return(d-(unsigned char *)dest);
}

#undef TGAS

int GCALL TGA_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

int GCALL TGA_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    char *linebuf;
    int width;
    int height;
    int bpp;
    int bpl;
    int colourstart;
    int colours;
    int y;
    int ok;
    int i;
    int colourmaptype;
    int imagetype;
    int alphabits;
    int colourmapbits;
    int headersize;
    int bitmapsize;
    char *s;
    TGAHeaderStruct tgaheader;
    unsigned char palbuf[256*4];
    char xlat[256];

    colourstart = ginfo->startcolour;
    colours = ginfo->numcolours;
    width = ginfo->width;
    height = ginfo->height;
    bpp = ginfo->originalbpp;

    alphabits = ginfo->alphabits;
         if (bpp<=8)  bpp=8;
    else if (ginfo->alphabits<=1
          && ginfo->redbits  <=5
          && ginfo->greenbits<=5
          && ginfo->bluebits <=5) bpp=16;
    else if (ginfo->alphabits==0) bpp=24, alphabits=0;
    else                          bpp=32;

    bpl = (width*bpp)/8;
    headersize = sizeof(TGAHeaderStruct) + (4L * colours);
    bitmapsize = (width * bpp / 8) * height;

    colourmaptype = colours ? 1: 0;
    colourmapbits = 32;
    imagetype = (bpp<=8) ? 1: 2;

	bool isgrey=false;	// 0X00
    if (ginfo->alphabits==0
          && ginfo->redbits >= 1
          && ginfo->greenbits==0
          && ginfo->bluebits ==0)
	{
		isgrey = true;
	}

	for (i=0; i<256; ++i)
		xlat[i] = i;
	if (isgrey)
    {
		imagetype = 3;
        colourmaptype = 0;
		bpp = 8;
		for (i=0; i<256; ++i)
			xlat[i] = ginfo->colourtbl[i].r;
    }
    if (ginfo->packed)
        imagetype |= 8;

    /* create palette of 16,24 or 32 bits deep */

    if (colourmaptype)
    {
        if (ginfo->redbits==5 && ginfo->greenbits==5 && ginfo->bluebits==5 && ginfo->alphabits<=1)
        {
            colourmapbits = 16;
            for (i=0; i<colours; ++i)
            {
                unsigned int a;
                unsigned int r;
                unsigned int g;
                unsigned int b;
                unsigned int rgb;

                a = 255;
                if (alphabits)
                    a = ginfo->colourtbl[colourstart+i].r >> 7;
                r = ginfo->colourtbl[colourstart+i].r >> 3;
                g = ginfo->colourtbl[colourstart+i].g >> 3;
                b = ginfo->colourtbl[colourstart+i].b >> 3;
                rgb = (a<<15)|(r<<10)|(g<<5)|b;
                gputi(palbuf+i*2,rgb,2);
            }
        }
        else if (ginfo->alphabits==8)
        {
            colourmapbits = 32;
            for (i=0; i<colours; ++i)
            {
                palbuf[i*4+0] = ginfo->colourtbl[colourstart+i].b;
                palbuf[i*4+1] = ginfo->colourtbl[colourstart+i].g;
                palbuf[i*4+2] = ginfo->colourtbl[colourstart+i].r;
                palbuf[i*4+3] = ginfo->colourtbl[colourstart+i].a;
            }
        }
        else
        {
            colourmapbits = 24;
            for (i=0; i<colours; ++i)
            {
                palbuf[i*3+0] = ginfo->colourtbl[colourstart+i].b;
                palbuf[i*3+1] = ginfo->colourtbl[colourstart+i].g;
                palbuf[i*3+2] = ginfo->colourtbl[colourstart+i].r;
            }
        }
    }

    /* TARGA FILE HEADER structure 18 bytes */

    TGAPUTFIELD(tgaheader.identsize, 0);                    /* size of ID field that follows 18 byte header (0 usually) */
    TGAPUTFIELD(tgaheader.colourmaptype, colourmaptype);    /* type of colour map 0=none, 1=has palette */
    TGAPUTFIELD(tgaheader.imagetype, imagetype);            /* type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed */

    TGAPUTFIELD(tgaheader.colourmapstart, colourstart);     /* first colour map entry in palette */
    TGAPUTFIELD(tgaheader.colourmaplength, colours);        /* number of colours in palette */
    TGAPUTFIELD(tgaheader.colourmapbits, colourmapbits);    /* number of bits per palette entry 15,16,24,32 */

    TGAPUTFIELD(tgaheader.xstart, ginfo->defaultx);         /* image x origin */
    TGAPUTFIELD(tgaheader.ystart, -ginfo->defaulty);        /* image y origin */
    TGAPUTFIELD(tgaheader.width, width);                    /* image width in pixels */
    TGAPUTFIELD(tgaheader.height, height);                  /* image height in pixels */
    TGAPUTFIELD(tgaheader.bits, bpp);                       /* image bits per pixel 8,16,24,32 */
    TGAPUTFIELD(tgaheader.descriptor, alphabits);           /* image descriptor bits (vh flip bits) */

    ok = gwrite(gx->gstream, (char *) &tgaheader, 18);
    if (ok && !isgrey && colours*colourmapbits/8)
        ok = gwrite(gx->gstream, (char *) palbuf, colours*colourmapbits/8);

    linebuf = (char *)galloc(bpl*3+16);
    if (linebuf)
    {
        for (y=0; y<height && ok; ++y)
        {
            s = (source+(height-y-1)*rowbytes);

            bpl = TGAwriteline(s, linebuf, width, bpp, ginfo->bpp, xlat,ginfo->packed,alphabits);
            ok = gwrite(gx->gstream,linebuf,bpl);
        }

        gfree(linebuf);
    }

    return(ok>0);
}

int GCALL TGA_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

#endif


