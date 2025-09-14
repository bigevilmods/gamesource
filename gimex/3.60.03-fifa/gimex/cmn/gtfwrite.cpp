// (c) 2006-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "gtfgimex.h"
#include "codex.h"
#include "dxtcodex.h"
#include "s3tccodex.h"

#define TOC_ENTRYSIZE (36)

// 16 for attrib tags
// 0x34 for d3dtexture
// GTF_MAXFRAMENAME (64) for names
/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static void GTFDXTreorderblocks(int width, int height, int sourcerowbytes, unsigned char *source, unsigned char *dest)
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

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL GTF_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
		int headersize = ((12+TOC_ENTRYSIZE*numframes)+127)&~127;
        char *gtfheader = (char *)galloc(headersize);
        memset(gtfheader,0,headersize);

        gputm(gtfheader+0,0x00090701   ,4); // version
		gputm(gtfheader+4,0            ,4); // vramsize
        gputm(gtfheader+8,numframes    ,4); // numframes

        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        gx->gref = gtfheader;
        gwrite(g,gtfheader,headersize);
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

static int GTFtexturetype(const GINFO *ginfo)
{
    int texturetype = CELL_GCM_TEXTURE_A8R8G8B8; // default

         if (ginfo->packed>2 && ginfo->alphabits>1 && ginfo->alphabits!=3)                          texturetype = CELL_GCM_TEXTURE_COMPRESSED_DXT23;
	else if (ginfo->packed>2 && ginfo->alphabits==3)                                                texturetype = CELL_GCM_TEXTURE_COMPRESSED_DXT45;
	else if (ginfo->packed>2)                                                                       texturetype = CELL_GCM_TEXTURE_COMPRESSED_DXT1;
	else if (ginfo->greenbits==0 && ginfo->bluebits==0)                                             texturetype = CELL_GCM_TEXTURE_G8B8;
	else if (ginfo->alphabits==0 && ginfo->redbits<=5 && ginfo->greenbits<=5 && ginfo->bluebits<=5) texturetype = CELL_GCM_TEXTURE_D1R5G5B5;
	else if (ginfo->alphabits<=1 && ginfo->redbits<=5 && ginfo->greenbits<=5 && ginfo->bluebits<=5) texturetype = CELL_GCM_TEXTURE_A1R5G5B5;
    else if (ginfo->alphabits==0 && ginfo->redbits<=5 && ginfo->greenbits<=6 && ginfo->bluebits<=5) texturetype = CELL_GCM_TEXTURE_R5G6B5;
    else if (ginfo->alphabits<=4 && ginfo->redbits<=4 && ginfo->greenbits<=4 && ginfo->bluebits<=4) texturetype = CELL_GCM_TEXTURE_A4R4G4B4;

	return texturetype;
}

#if 0
typedef struct {
	uint8_t format;
	uint8_t mipmap;
	uint8_t dimension;
	uint8_t cubemap;
	uint32_t remap;
	uint16_t width;
	uint16_t height;
	uint16_t depth;
	uint8_t location;
	uint8_t _padding;
	uint32_t pitch;
	uint32_t offset;
} CellGcmTexture;

typedef struct {
	uint32_t Id;
	uint32_t OffsetToTex;
	uint32_t TextureSize;
	CellGcmTexture tex;
} CellGtfTextureAttribute;
#endif

int GCALL GTF_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    char *gtfheader = (char *)gx->gref;
    int framenum = gx->framenum;
    int numframes = ggetm(gtfheader+8,4);  // total frames that will be in this file
    int texturetype = GTFtexturetype(ginfo);

	// calculate location in file
	int headersize = ((12+TOC_ENTRYSIZE*numframes)+127)&~127;
	unsigned int BaseAddress = ((int)gtell(gx->gstream));

	if (BaseAddress&127)
	{
		int pad = 128-(((int)gtell(gx->gstream))&127);
		char buf[128];
		memset(buf,0,pad);
		BaseAddress += pad;

		gwrite(gx->gstream,buf,pad);
	}

	// calculate size of texture
	int bpp = texturetype==CELL_GCM_TEXTURE_A8R8G8B8?32:16;
    int bpl = (ginfo->width*bpp/8 + 3)&~3;  // ps3 align to 4 byte boundary
	int texturesize = ginfo->height*bpl;
	if (texturetype==CELL_GCM_TEXTURE_COMPRESSED_DXT1)
	{
		texturesize = ((ginfo->width+3)&~3)*((ginfo->height+3)&~3)/2;
		bpp = 4;
		bpl = ((ginfo->width+3)/4)*8; // 8 bytes per block
	}
	if (texturetype==CELL_GCM_TEXTURE_COMPRESSED_DXT23 || texturetype==CELL_GCM_TEXTURE_COMPRESSED_DXT45)
	{
		texturesize = ((ginfo->width+3)&~3)*((ginfo->height+3)&~3);
		bpp = 8;
		bpl = ((ginfo->width+3)/4)*16; // 16 bytes per block
	}


    /* fill in texture attributes (36 bytes) */
    char *attribtag = gtfheader+12+framenum*36;
    gputm(attribtag+0, framenum, 4);    // Id
    gputm(attribtag+4, BaseAddress, 4); // OffsetToTex from start of file
    gputm(attribtag+8, texturesize, 4);	// TextureSize

    gputm(attribtag+12, texturetype|0x20,  1);		// format. 0x20 for Linear
    gputm(attribtag+13, 1,            1);		// mipmap
    gputm(attribtag+14, 2,            1);		// dimension
    gputm(attribtag+15, 0,            1);		// cubemap
    gputm(attribtag+16, 0xaae4,       4);		// remap - passthru
    gputm(attribtag+20, ginfo->width, 2);		// width;  
    gputm(attribtag+22, ginfo->height,2);		// height; 
    gputm(attribtag+24, 1,            2);		// depth;  
    gputm(attribtag+26, 0,            1);		// location;
    gputm(attribtag+27, 0,            1);		// _padding;
    gputm(attribtag+28, bpl,          4);		// pitch;  
    gputm(attribtag+32, 0,            4);		// offset; 

	int ok = 0;

	if(ginfo->packed > 2) // USE DXT compression
	{
		int destsize = ((ginfo->width+3)&~3) * ((ginfo->height+3)&~3) + 8; // +3 to align to block, +8 for dxt header
		unsigned char *dest = (unsigned char *)galloc(destsize);
		if (dest)
		{
			int options[2] = {0,50};
			int size = ((ginfo->width+3)&~3) * ((ginfo->height+3)&~3) * 4;
			unsigned char *newsource = (unsigned char *)galloc(size);
			if(newsource)
			{
				GTFDXTreorderblocks(ginfo->width, ginfo->height, rowbytes, (unsigned char *)source, newsource);
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

				if (ginfo->packed==3) // S3TC (s3 code)
					size = S3TC_encode(dest, newsource, size, options);
				else    // DXT (ea code)
					size = DXT_encode(dest, newsource, size, options);

				gfree(newsource);

				ok = gwrite(gx->gstream, dest+sizeof(DXTHEADER), size-sizeof(DXTHEADER));
			}
			gfree(dest);
		}
	}
	else
	{
		char *linebuf = (char *) galloc(bpl);
		if (linebuf)
		{
			memset(linebuf,0,bpl);
			ok = 1;

			for (int y=0; y<ginfo->height && ok; ++y)
			{
				char *lineptr = linebuf;
				ARGB *s = (ARGB *) (source+rowbytes*y);

				switch (texturetype)
				{
					case CELL_GCM_TEXTURE_A1R5G5B5:
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

					case CELL_GCM_TEXTURE_D1R5G5B5:
						for (int x=0; x<ginfo->width; ++x)
						{
							unsigned int a = 0;
							unsigned int r = (s->r*31+128)/255;
							unsigned int g = (s->g*31+128)/255;
							unsigned int b = (s->b*31+128)/255;
							gputm(lineptr,(a<<15)|(r<<10)|(g<<5)|b,2);
							++s;
							lineptr += 2;
						}
						break;

					case CELL_GCM_TEXTURE_A4R4G4B4:
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

					case CELL_GCM_TEXTURE_R5G6B5:
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

					case CELL_GCM_TEXTURE_A8R8G8B8:
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

					case CELL_GCM_TEXTURE_G8B8:
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

			int totalvramsize = (int)gtell(gx->gstream)-headersize;
			gputm(gtfheader+4,totalvramsize ,4); // texture size
			gfree(linebuf);
		}
	}
    ++gx->framenum;

    return(ok);
}

int GCALL GTF_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        char *gtfheader = (char *)gx->gref;
	    int numframes = ggetm(gtfheader+8,4);  // total frames that will be in this file
		int headersize = ((12+TOC_ENTRYSIZE*numframes)+127)&~127;
		
        gseek(gx->gstream,0);
        ok = gwrite(gx->gstream,gtfheader,headersize);

        gfree(gx->gref);
        ok = gfree((void *) gx);
    }

    return(ok);
}

