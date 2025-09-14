// (c) 2006-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "xprgimex.h"
#include "codex.h"
#include "dxtcodex.h"
#include "s3tccodex.h"

#define TOC_ENTRYSIZE (16+0x34+XPR_MAXFRAMENAME)

// 16 for resource tags
// 0x34 for d3dtexture
// XPR_MAXFRAMENAME (64) for names
/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static void XPRDXTreorderblocks(int width, int height, int sourcerowbytes, unsigned char *source, unsigned char *dest)
{
    int i;
    int j;
    int k;
    unsigned char *s;
    unsigned char *d = dest;
    int copywidth;
    int clearwidth;
    int copyheight;
    int clearheight;

    for (i=0; i<height; i+=4)
    {

        /* If height is not a multiple of 4 and we're on the last row of blocks */
        if (i+4 > height)
        {
            copyheight = height-i;
            clearheight = 4-copyheight;
        }
        else
        {
            copyheight = 4;
            clearheight = 0;
        }

        s = source + i*sourcerowbytes;
        for (j=0; j<width; j+=4)
        {
            /* If width is not a multiple of 4 and we are on the last block of the line */
            if (j+4 > width)
            {
                copywidth = width-j;
                clearwidth = 4-copywidth;
            }
            else
            {
                copywidth = 4;
                clearwidth = 0;
            }

            /* Copy only what exists in the source */
            for (k=0; k<copyheight; k++)
            {

                /* Copy only what exists in the source */
                memcpy(d, s + k*sourcerowbytes, copywidth * 4);

                /* Clear the rest */
                if (clearwidth)
                {
                    memset(d + copywidth * 4, 0, clearwidth * 4);
                }
                d += 4 * 4;
            }

            /* Clear the rest */
            for (k=0; k<clearheight; k++)
            {
                memset(d, 0, 4 * 4);
                d += 4 * 4;
            }

            s += 4 * 4;
        }
    }
}

#ifndef TYPE3PR
// dxt needs all shorts swapped first
static void ShortSwap(unsigned char *framebuf, int framesize)
{
    for (int i=0; i<framesize; i+=2)
    {
        gputi(framebuf, ggetm(framebuf,2), 2);
        framebuf += 2;
    }
}
#endif

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL XPR_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        int headersize = 12+(((TOC_ENTRYSIZE*numframes)+2047)&~2047);
        char *xprheader = (char *)galloc(headersize);
        memset(xprheader,0,headersize);

#ifdef TYPE3PR
        gputm(xprheader+0,0x33505232   ,4); // id 3PR2
#else
        gputm(xprheader+0,0x58505232   ,4); // id XPR2
#endif
		gputm(xprheader+4,headersize-12,4); // header
        gputm(xprheader+8,0            ,4); // texture size
        gputm(xprheader+12,numframes   ,4); // numframes

        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        gx->gref = xprheader;
        gwrite(g,xprheader,headersize);
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

static int XPRtexturetype(const GINFO *ginfo)
{
    int texturetype = GPUTEXTUREFORMAT_8_8_8_8;

         if (ginfo->packed>2 && ginfo->alphabits>1 && ginfo->alphabits!=3)                          texturetype = GPUTEXTUREFORMAT_DXT2_3;
	else if (ginfo->packed>2 && ginfo->alphabits==3)                                                texturetype = GPUTEXTUREFORMAT_DXT4_5;
	else if (ginfo->packed>2)                                                                       texturetype = GPUTEXTUREFORMAT_DXT1;
	else if (ginfo->greenbits==0 && ginfo->bluebits==0)                                             texturetype = GPUTEXTUREFORMAT_8_8;
	else if (ginfo->alphabits<=1 && ginfo->redbits<=5 && ginfo->greenbits<=5 && ginfo->bluebits<=5) texturetype = GPUTEXTUREFORMAT_1_5_5_5;
    else if (ginfo->alphabits==0 && ginfo->redbits<=5 && ginfo->greenbits<=6 && ginfo->bluebits<=5) texturetype = GPUTEXTUREFORMAT_5_6_5;
    else if (ginfo->alphabits<=4 && ginfo->redbits<=4 && ginfo->greenbits<=4 && ginfo->bluebits<=4) texturetype = GPUTEXTUREFORMAT_4_4_4_4;

	return texturetype;
}


int GCALL XPR_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    char *xprheader = (char *)gx->gref;
    int framenum = gx->framenum;
    int numframes = ggetm(xprheader+12,4);  // total frames that will be in this file
    int texturetype = XPRtexturetype(ginfo);

    /* fill in resource tag (4 dwords) */
    char *resourcetag = xprheader+16+framenum*16;
    gputm(resourcetag+0, 0x54583244,4);
    gputm(resourcetag+4, 4+numframes*16+framenum*0x34,4); //position of d3dtexture
    gputm(resourcetag+8, 0x34,4); // size of d3dtexture
    gputm(resourcetag+12,4+numframes*(16+0x34)+framenum*XPR_MAXFRAMENAME,4); //position of namestring

    /* fill in d3dtexture tag (13 dwords) */
    char *texture = xprheader+16+numframes*16+framenum*0x34;
    gputm(texture+0 ,3,4); // 6 dwords for resource
    gputm(texture+4 ,1,4);
    gputm(texture+8 ,0,4);
    gputm(texture+12,0,4);
    gputm(texture+16,0,4);
    gputm(texture+20,0xffff0000,4);
    gputm(texture+24,0xffff0000,4); // mipflush


    int headersize = ggetm(xprheader+4,4)+12;
    unsigned int BaseAddress = ((int)gtell(gx->gstream)-headersize);

	if (BaseAddress&4095)
	{
		int pad = 4096-(((int)gtell(gx->gstream)-headersize)&4095);
		char buf[4096];
		memset(buf,0,pad);
		BaseAddress += pad;

		gwrite(gx->gstream,buf,pad);
	}

    gputm(texture+28,0x02000002,4); // 6 dwords for gputexture
    gputm(texture+32,0x00000080 | texturetype | (BaseAddress) ,4);
    gputm(texture+36,((ginfo->height-1)<<13)|(ginfo->width-1),4);
    gputm(texture+40,0x00000c14,4);
    gputm(texture+44,0,4);
    gputm(texture+48,0x00000a00,4);

	if (texturetype==GPUTEXTUREFORMAT_8_8) // AL88
	{
	    gputm(texture+32,0x00000040 | texturetype | (BaseAddress) ,4); // endian
	    gputm(texture+40,0x00000400,4); // swizzle
	}
	if (ginfo->packed>2)// texturetype==GPUTEXTUREFORMAT_DXT1 3 or 5
	{
//        if (texturetype==GPUTEXTUREFORMAT_DXT1)
//            gputm(texture+28,0x01000002,4); // 6 dwords for gputexture
        gputm(texture+28,0x04000002,4); // 6 dwords for gputexture
	    gputm(texture+32,0x00000040 | texturetype | (BaseAddress) ,4); // endian
	    gputm(texture+40,0x00000d10,4);
	}


    /* fill in name (64 bytes) */

    char *name = xprheader+16+numframes*(16+0x34)+framenum*XPR_MAXFRAMENAME;
    strcpy(name,ginfo->framename);

    int bpp = texturetype==GPUTEXTUREFORMAT_8_8_8_8?32:16;
#ifdef TYPE3PR
    int bpl = (ginfo->width*bpp/8 + 3)&~3;  // ps3 align to 4 byte boundary?
#else
	int bpl = (ginfo->width*bpp/8 + 255)&~255;// xenon align to 256 byte boundary?
#endif

	int ok = 0;

    char *linebuf = (char *) galloc(bpl);
    if (linebuf)
    {
        memset(linebuf,0,bpl);
        ok = 1;

		if(ginfo->packed > 2) // USE DXT compression
        {
			ok = 0;
            int destsize = ((ginfo->width+3)&~3) * ((ginfo->height+3)&~3) + 8; // +3 to align to block, +8 for dxt header
			unsigned char *dest = (unsigned char *)galloc(destsize);
			if (dest)
			{
				int options[2] = {0,50};
				int size = ((ginfo->width+3)&~3) * ((ginfo->height+3)&~3) * 4;
				unsigned char *newsource = (unsigned char *)galloc(size);
				if(newsource)
				{
					XPRDXTreorderblocks(ginfo->width, ginfo->height, rowbytes, (unsigned char *)source, newsource);
					options[1] = ginfo->quality;

					if (ginfo->alphabits > 1 && ginfo->alphabits != 3) // DXT3
					{
						options[0] = DXT_ENCODE_RGB_FULL |DXT_ENCODE_ALPHA_EXPLICIT;
					}
					else if(ginfo->alphabits == 3)    // DXT5
					{
						options[0] = DXT_ENCODE_RGB_FULL |DXT_ENCODE_ALPHA_INTERPOLATED;
					}
					else if (ginfo->alphabits == 1)   // DXT1
					{
						options[0] = DXT_ENCODE_RGB_ALPHA_COMPARE;
					}
					else // Also DXT1
					{
						options[0] = DXT_ENCODE_RGB_FULL;
					}

					// Encode and write image data.
					//size = DXT_encode(dest, newsource, size, options);

					if (ginfo->packed==3) // S3TC (s3 code)
						size = S3TC_encode(dest, newsource, size, options);
					else    // DXT (ea code)
						size = DXT_encode(dest, newsource, size, options);
#ifndef TYPE3PR
					ShortSwap(dest+sizeof(DXTHEADER), size-sizeof(DXTHEADER));
#endif
					gfree(newsource);

					ok = gwrite(gx->gstream, dest+sizeof(DXTHEADER), size-sizeof(DXTHEADER));
				}
				gfree(dest);
			}
		}
		else
		{
			for (int y=0; y<ginfo->height && ok; ++y)
			{
				char *lineptr = linebuf;
				ARGB *s = (ARGB *) (source+rowbytes*y);

				switch (texturetype)
				{
					case GPUTEXTUREFORMAT_1_5_5_5:
						for (int x=0; x<ginfo->width; ++x)
						{
							unsigned int a = s->a>>7;
							unsigned int r = (s->r*31+128)/255;
							unsigned int g = (s->g*31+128)/255;
							unsigned int b = (s->b*31+128)/255;
							gputm(lineptr,(a<<15)|(r<<10)|(g<<5)|b,2);
							++s;
							lineptr += 2;
						}
						break;

					case GPUTEXTUREFORMAT_4_4_4_4:
						for (int x=0; x<ginfo->width; ++x)
						{
							unsigned int a = (s->a*15+128)/255;
							unsigned int r = (s->r*15+128)/255;
							unsigned int g = (s->g*15+128)/255;
							unsigned int b = (s->b*15+128)/255;
							gputm(lineptr,(a<<12)|(r<<8)|(g<<4)|b,2);
							++s;
							lineptr += 2;
						}
						break;

					case GPUTEXTUREFORMAT_5_6_5:
						for (int x=0; x<ginfo->width; ++x)
						{
							unsigned int r = (s->r*31+128)/255;
							unsigned int g = (s->g*63+128)/255;
							unsigned int b = (s->b*31+128)/255;
							gputm(lineptr,(r<<11)|(g<<5)|b,2);
							++s;
							lineptr += 2;
						}
						break;

					case GPUTEXTUREFORMAT_8_8_8_8:
						for (int x=0; x<ginfo->width; ++x)
						{
							lineptr[0] = s->a;
							lineptr[1] = s->r;
							lineptr[2] = s->g;
							lineptr[3] = s->b;
							++s;
							lineptr += 4;
						}
						break;

					case GPUTEXTUREFORMAT_8_8:
						for (int x=0; x<ginfo->width; ++x)
						{
							lineptr[0] = s->a;
							lineptr[1] = s->r;
							++s;
							lineptr += 2;
						}
						break;
				}

				ok = gwrite(gx->gstream,linebuf,bpl);
			}
		}
		int texturesize = (int)gtell(gx->gstream)-headersize;
		gputm(xprheader+8,texturesize ,4); // texture size

        gfree(linebuf);
	}
    ++gx->framenum;

    return(ok);
}

int GCALL XPR_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        char *xprheader = (char *)gx->gref;
        int headersize = ggetm(xprheader+4,4)+12; // header

        gseek(gx->gstream,0);
        ok = gwrite(gx->gstream,xprheader,headersize);

        gfree(gx->gref);
        ok = gfree((void *) gx);
    }

    return(ok);
}

