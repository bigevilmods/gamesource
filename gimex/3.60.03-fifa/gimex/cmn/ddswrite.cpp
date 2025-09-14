// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "ddsgimex.h"
#include "codex.h"
#include "dxtcodex.h"
#include "s3tccodex.h"
#include <assert.h>
#include "eadxt.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static void DXTreorderblocks(int width, int height, int rowbytes, unsigned char *source, unsigned char *dest)
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

        s = source + i*rowbytes;
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
                memcpy(d, s + k*rowbytes, copywidth * 4);

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

// UncompressedCalcSize mimics SetupUnencodedWrite logic
static unsigned int UncompressedCalcSize(const GINFO *ginfo)
{
    int bpp=0;

    if(ginfo->alphabits == 8) //8888 format
    {
        bpp = 32;
    }
    else if(ginfo->redbits == 8) // 888 format
    {
        bpp = 24;
    }
    else if(ginfo->alphabits == 4) // 4444 format.
    {
        bpp = 16;
    }
    else if(ginfo->greenbits == 6) // 565 format
    {
        bpp = 16;
    }
    else // 1555 format
    {
        bpp = 16;
    }
    return ginfo->width*bpp/8*ginfo->height;
}

static void makeUnencodedHeader(const GINFO *ginfo, DDSHeaderStruct *header)
{

	if (ginfo->alphabits == 8) //8888 format
	{
		DDSPUTFIELD(header->ddsPixelFormat.dwFlags, GDDPF_RGBA);
		DDSPUTFIELD(header->ddsPixelFormat.dwRGBBitCount, 32);     // this 32 bpp
		DDSPUTFIELD(header->ddsPixelFormat.dwRGBAlphaBitMask, 0xFF000000); // setup RGBA masks.
		DDSPUTFIELD(header->ddsPixelFormat.dwRBitMask, 0x00FF0000);
		DDSPUTFIELD(header->ddsPixelFormat.dwGBitMask, 0x0000FF00);
		DDSPUTFIELD(header->ddsPixelFormat.dwBBitMask, 0x000000FF);
	}
	else if (ginfo->redbits == 8) // 888 format
	{
		DDSPUTFIELD(header->ddsPixelFormat.dwFlags, GDDPF_RGB);
		DDSPUTFIELD(header->ddsPixelFormat.dwRGBBitCount, 24);
		DDSPUTFIELD(header->ddsPixelFormat.dwRGBAlphaBitMask, 0x00000000);
		DDSPUTFIELD(header->ddsPixelFormat.dwRBitMask, 0x00FF0000);
		DDSPUTFIELD(header->ddsPixelFormat.dwGBitMask, 0x0000FF00);
		DDSPUTFIELD(header->ddsPixelFormat.dwBBitMask, 0x000000FF);
	}
	else if (ginfo->alphabits == 4) // 4444 format.
	{
		DDSPUTFIELD(header->ddsPixelFormat.dwFlags, GDDPF_RGBA);
		DDSPUTFIELD(header->ddsPixelFormat.dwRGBBitCount, 16);
		DDSPUTFIELD(header->ddsPixelFormat.dwRGBAlphaBitMask, GA4444BITMASK);
		DDSPUTFIELD(header->ddsPixelFormat.dwRBitMask, GR4444BITMASK);
		DDSPUTFIELD(header->ddsPixelFormat.dwGBitMask, GG4444BITMASK);
		DDSPUTFIELD(header->ddsPixelFormat.dwBBitMask, GB4444BITMASK);
		
	}
	else if (ginfo->greenbits == 6) // 565 format
	{
		DDSPUTFIELD(header->ddsPixelFormat.dwFlags, GDDPF_RGB);
		DDSPUTFIELD(header->ddsPixelFormat.dwRGBBitCount, 16);
		DDSPUTFIELD(header->ddsPixelFormat.dwRGBAlphaBitMask, 0);
		DDSPUTFIELD(header->ddsPixelFormat.dwRBitMask, GR565BITMASK);
		DDSPUTFIELD(header->ddsPixelFormat.dwGBitMask, GG565BITMASK);
		DDSPUTFIELD(header->ddsPixelFormat.dwBBitMask, GB565BITMASK);
	}
	else // 1555 format
	{
		DDSPUTFIELD(header->ddsPixelFormat.dwFlags, GDDPF_RGBA);
		DDSPUTFIELD(header->ddsPixelFormat.dwRGBBitCount, 16);
		DDSPUTFIELD(header->ddsPixelFormat.dwRGBAlphaBitMask, GA555BITMASK);
		DDSPUTFIELD(header->ddsPixelFormat.dwRBitMask, GR555BITMASK);
		DDSPUTFIELD(header->ddsPixelFormat.dwGBitMask, GG555BITMASK);
		DDSPUTFIELD(header->ddsPixelFormat.dwBBitMask, GB555BITMASK);
	}

	int dwFlags = DDSGETFIELD(header->dwFlags);
	dwFlags |= GDDSD_LINEARSIZE;
	DDSPUTFIELD(header->dwFlags, dwFlags);
	DDSPUTFIELD(header->dwPitchOrLinearSize, ginfo->width*ginfo->height);


}

void makeEncodedHeader(const GINFO *ginfo, DDSHeaderStruct* header, int* dxt_flags)
{

	if (ginfo->alphabits > 1 && ginfo->alphabits != 3) // DXT3
	{
		DDSPUTFIELD(header->ddsPixelFormat.dwFourCC, 0x33545844);
		*dxt_flags = DXT_ENCODE_RGB_FULL | DXT_ENCODE_ALPHA_EXPLICIT;
	}
	else if (ginfo->alphabits == 3)    // DXT5
	{
		DDSPUTFIELD(header->ddsPixelFormat.dwFourCC, 0x35545844);
		*dxt_flags = DXT_ENCODE_RGB_FULL | DXT_ENCODE_ALPHA_INTERPOLATED;
	}
	else if (ginfo->alphabits == 1)   // DXT1
	{
		DDSPUTFIELD(header->ddsPixelFormat.dwFourCC, 0x31545844);
		*dxt_flags = DXT_ENCODE_RGB_ALPHA_COMPARE;
	}
	else // Also DXT1
	{
		DDSPUTFIELD(header->ddsPixelFormat.dwFourCC, 0x31545844);
		*dxt_flags = DXT_ENCODE_RGB_FULL;
	}

	DDSPUTFIELD(header->ddsPixelFormat.dwFlags, GDDPF_FOURCC);
	DDSPUTFIELD(header->dwPitchOrLinearSize, 0);
}



static int SetupUnencodedWrite(const GINFO *ginfo, char *source, unsigned long size, char *dest)
{
    unsigned int size2 = (ginfo->width*ginfo->height);  // size/4 ie. number of pixels to copy.
    unsigned int i = 0;
    char *d = dest;
    const char *s = source;
    int T;
    //size2 = (unsigned int)((double)size/(double)4);
	memset(d,0,size);
    if(ginfo->alphabits == 8) //8888 format
    {
       for(i = 0; i < size2; i++) // just do a verbatim copy.
       {

            ((ARGB *)d)->a = ((ARGB*)s)->a;
            ((ARGB *)d)->r = ((ARGB*)s)->r;
            ((ARGB *)d)->g = ((ARGB*)s)->g;
            ((ARGB *)d)->b = ((ARGB*)s)->b;
            d+=4;
            s+=4;
       }
    }
    else if(ginfo->redbits == 8) // 888 format
    {
       for(i = 0; i < size2; i++)
       {
          *d++ = ((ARGB *)s)->b;
          *d++ = ((ARGB *)s)->g;
          *d++ = ((ARGB *)s)->r;
		  //d+=3;
          s+=4;
       }
    }
    else if(ginfo->alphabits == 4) // 4444 format.
    {
       for(i = 0; i < size2; i++)
       {
          T = 0;
          T |=  (((int)((ARGB *)s)->a)<<8)&0xF000;
          T |=  (((int)((ARGB *)s)->r)<<4)&0x0F00;
          T |=  (((int)((ARGB *)s)->g)<<0)&0x00F0;
          T |=  (((int)((ARGB *)s)->b)>>4)&0x000F;
          gputi((void *)d,(unsigned int)T,2);
          s+=4;
          d+=2;
       }
    }
    else if(ginfo->greenbits == 6) // 565 format
    {
       for(i = 0; i < (size2-1); i++)
       {
          T = 0;
          T |=  (((int)((ARGB *)s)->r)<<8)&0xF800;
          T |=  (((int)((ARGB *)s)->g)<<3)&0x07E0;
          T |=  (((int)((ARGB *)s)->b)>>3)&0x001F;
          gputi((void *)d,(unsigned int)T,2);
          d+=2;
          s+=4;
       }
    }
    else // 1555 format
    {
       for(i = 0; i < (size2-1); i++)
       {
          T = 0;//[0] = T[1] = 0;
          T |=  (int)(1<<15);//(((ARGB *)s)->a)&0x8000;
          T |=  (((int)((ARGB *)s)->r)<<7)&0x7C00;
          T |=  (((int)((ARGB *)s)->g)<<2)&0x03E0;
          T |=  (((int)((ARGB *)s)->b)>>3)&0x001F;
          gputi((void *)d,(unsigned int)T,2);
          s+=4;
          d+=2;
       }
    }

    return 1;
}

/****************************************************************/
#define MAX_MIPS 16
struct MipData
{
	int nummips;
	char* mips[MAX_MIPS];
	size_t sizes[MAX_MIPS];

	GINFO ginfo[MAX_MIPS];
};


int eadxt_encode(char* dest, const char* source, int rowbytes, int blocksize, int width, int height, int flags, int quality)
{
	int error = 0;

	void *compressed = dest;
	const void *uncompressed = source;

	int fmode = 0;
	if (flags & DXT_ENCODE_ALPHA_EXPLICIT)
	{
		fmode = EADXT_MODE_ALPHA_EXPLICIT; // 1 = DXT3
	}
	else if (flags & DXT_ENCODE_ALPHA_INTERPOLATED)
	{
		fmode = EADXT_MODE_ALPHA_INTERPOLATED;
	}

	int numblocks = width / 4;
	for (int y = 0; y < height / 4; ++y)
	{
		const char *rowin = (const char *)uncompressed + rowbytes * y * 4;
		char *rowout = (char *)compressed + y * blocksize * numblocks;

		error += eadxt(rowout, rowin, rowbytes, numblocks, fmode, quality);
	}
	return (error == 0 ? 1 : 0);
}

int getBlockSize(int flags)
{
	int blocksize = ((flags & (DXT_ENCODE_ALPHA_EXPLICIT | DXT_ENCODE_ALPHA_INTERPOLATED /*| DXT_ENCODE_INTERPOLATED_RED*/)) > 0) ? 16 : 8;
	return blocksize;
}

size_t getCompressedSize(int width, int height, int blocksize)
{
	size_t size = ((width + 3) / 4) * ((height + 3) / 4) * blocksize;
	return size;
}

static int dds_file_write(GINSTANCE *gx, const MipData* mipdata)
{
	const GINFO* iminfo = &mipdata->ginfo[0];
	
	DDSHeaderStruct header;
	int width;
	int height;
	int ok = 0;
	int isCubeMap = (iminfo->subtype == GDDS_SUBTYPE_CUBECOMPlEX || iminfo->subtype == GDDS_SUBTYPE_CUBESIMPLE);
	int isComplex = (iminfo->subtype == GDDS_SUBTYPE_CUBECOMPlEX);
	int isCompressed = iminfo->packed > 2 ? 1 : 0;
	
	int mipcount = mipdata->nummips;
	if (iminfo->bpp>8)
	{
		gseek(gx->gstream, 0);

		
		if (isCompressed)    
		{
			// DXT expects to be in 4x4 blocs.
			// Round size up to multiples of 4
			width = (iminfo->width + 3)&~3;
			height = (iminfo->height + 3)&~3;
		}
		else
		{
			width = iminfo->width;
			height = iminfo->height;
		}
		memset(&header, 0, sizeof(DDSHeaderStruct));
		DDSPUTFIELD(header.dwVersion, 0x20534444);
		DDSPUTFIELD(header.dwSize, sizeof(DDSHeaderStruct) - 4 /*size of magic*/); 
		DDSPUTFIELD(header.dwFlags, GDDSD_CAPS | GDDSD_WIDTH | GDDSD_HEIGHT | GDDSD_PIXELFORMAT);//0x00021007);
		DDSPUTFIELD(header.dwHeight, height);
		DDSPUTFIELD(header.dwWidth, width);
		DDSPUTFIELD(header.dwMipMapCount, mipcount);
		DDSPUTFIELD(header.ddsPixelFormat.dwSize, sizeof(header.ddsPixelFormat));
		DDSPUTFIELD(header.ddsPixelFormat.dwRGBBitCount, 0);
																			
		DDSPUTFIELD(header.dwCaps, (GDDCAPS_TEXTURE | (mipcount ? GDDSCAPS_MIPMAP : 0) | (isComplex || mipcount ? GDDSCAPS_COMPLEX : 0) ));
		
		// is it a cubemap?
		DDSPUTFIELD(header.dwCaps2, isCubeMap ? (GDDSCAPS_CUBEMAP_ALL_FACES) : 0);

		// if this is a cubemap I only want to store the height of one face.
		if (isCubeMap)
		{
			DDSPUTFIELD(header.dwHeight, height / 6);
			DDSPUTFIELD(header.dwPitchOrLinearSize, width*height / 12);
		}

		int dxt_flags = 0;
		int quality = 50;
		
		if (isCompressed) // USE DXT compression here.
		{
			quality = iminfo->quality;
			makeEncodedHeader(iminfo, &header, &dxt_flags);
		}
		else
		{
			makeUnencodedHeader(iminfo, &header);
		}

		// Write header
		gwrite(gx->gstream, &header, sizeof(header));

		// Write pixel data
		for (int i = 0; i < mipcount; ++i)
		{
			const GINFO *ginfo = &mipdata->ginfo[i];
			width = ginfo->width;
			height = ginfo->height;
			char *source = mipdata->mips[i];
			int rowbytes = mipdata->sizes[i] / height;

			int blocksize = getBlockSize(dxt_flags);
			
			size_t size = 0;
			if (isCompressed)
				size = getCompressedSize(width, height, blocksize);
			else
				size = UncompressedCalcSize(ginfo);

			char *dest = (char *)galloc(size);
			if (dest)
			{
				if (iminfo->packed > 2) // USE DXT compression here.
				{
					ok += eadxt_encode(dest, source, rowbytes, blocksize, width, height, dxt_flags, quality);
				}
				else
				{
					ok += SetupUnencodedWrite(ginfo, source, size, dest);
				}
				
				// write out processed data to stream
				ok = gwrite(gx->gstream, dest, size);

				gfree(dest);
			}// if dest
		}
	}// if bpp
	return(ok > 0 ? 1 : 0);
}

/****************************************************************/
/*  export Functions                                            */
/****************************************************************/
int GCALL DDS_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
	MipData* mipdata = 0;

    int ok=0;
    gx = (GINSTANCE *)galloc(sizeof(GINSTANCE));
	
	assert(numframes < MAX_MIPS);
	mipdata = (MipData*)galloc(sizeof(MipData));
	memset(mipdata, 0, sizeof(MipData));

	mipdata->nummips = numframes;
	
    if (gx)
    {
        gx->frames = numframes;
        gx->gstream = g;
		gx->gref = (char*)mipdata;
        *gxptr = gx;
        ok = 1;
    }
    return ok;
}

int GCALL DDS_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
	int ok = 1;
	if (gx->framenum < gx->frames)
	{
		MipData* mipdata = (MipData*)gx->gref;

		size_t sz = rowbytes * ginfo->height;
		mipdata->mips[gx->framenum] = (char*)galloc(sz);
		mipdata->sizes[gx->framenum] = sz;
		
		memcpy(mipdata->mips[gx->framenum], source, sz);
		memcpy(&mipdata->ginfo[gx->framenum], ginfo, sizeof(GINFO));

		++gx->framenum;
	}

    return ok;
}


int GCALL DDS_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
		MipData* mipdata = (MipData*)gx->gref;

		if (mipdata->nummips)
		{
			ok = dds_file_write(gx, mipdata);
		}

		for (int i = 0; i < mipdata->nummips; ++i)
			gfree(mipdata->mips[i]);

		gfree(mipdata); //gx->gref
		gfree(gx);
        ok = 1;
    }
    return ok;
}

#undef GDDSD_VOLUMETEXTURE
#undef GDDS_SUBTYPE_VOLUMETEX
#undef GDDS_SUBTYPE_CUBESIMPLE
#undef GDDS_SUBTYPE_CUBECOMPLEX
#undef GDDSMAKEFOURCC
#undef GDXT1FORMAT
#undef GDXT2FORMAT
#undef GDXT3FORMAT
#undef GDXT4FORMAT
#undef GDXT5FORMAT
#undef GR4444BITMASK
#undef GG4444BITMASK
#undef GB4444BITMASK
#undef GA4444BITMASK
#undef GA555BITMASK
#undef GR555BITMASK
#undef GG555BITMASK
#undef GB555BITMASK
#undef GR565BITMASK
#undef GG565BITMASK
#undef GB565BITMASK
#undef GDDPF_ALPHAPIXELS
#undef GDDPF_RGB
#undef GDDPF_RGBA
#undef GDDPF_FOURCC
#undef GDDSD_HEIGHT
#undef GDDSD_WIDTH
#undef GDDSD_PIXELFORMAT
#undef GDDSD_MIPMAPCOUNT
#undef GDDSD_LINEARSIZE
#undef GDDSD_CAPS
#undef GDDCAPS_TEXTURE
#undef GDDSCAPS2_CUBEMAP
#undef GDDSCAPS_CUBEMAP_POSITIVEX
#undef GDDSCAPS_CUBEMAP_NEGATIVEX
#undef GDDSCAPS_CUBEMAP_POSITIVEY
#undef GDDSCAPS_CUBEMAP_NEGATIVEY
#undef GDDSCAPS_CUBEMAP_POSITIVEZ
#undef GDDSCAPS_CUBEMAP_NEGATIVEZ
#undef GDDSCAPS_COMPLEX
