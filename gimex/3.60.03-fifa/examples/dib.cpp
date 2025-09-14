// (c) 1999-2007 Electronic Arts Inc.

#if defined(_MSC_VER)
#pragma warning(disable: 4514)
#endif
#include <math.h> /* for abs */
#include "dib.h"
#include "gimex.h"

#ifndef ASSERT
#include <assert.h>
#define ASSERT  assert
#endif

typedef struct DIB
{
    BITMAPINFO* bmi;        /* pointer to BITMAPINFO struct */
    void*       base;       /* pointer to frame buffer */
    HBITMAP     hbitmap;    /* GDI bitmap object that can be selected */
} DIB;

int DIB_isvalid(const DIB* dib)
{
    if (dib == NULL)
        return 0;

    if (dib->bmi == NULL)
        return 0;

    if (dib->bmi->bmiHeader.biWidth <= 0)
        return 0;

    return 1;
}

static DWORD getcolortablesize(int bpp)
{
    switch (bpp)
    {
        case  1: return   2*sizeof(RGBQUAD);
        case  4: return  16*sizeof(RGBQUAD);
        case  8: return 256*sizeof(RGBQUAD);
        case 16: return   3*sizeof(DWORD);
        case 24: return   0;
        case 32: return   3*sizeof(DWORD);
    }
    ASSERT(0);
    return 0;
}

static int getbmisize(int bpp)
{
    return sizeof(BITMAPINFOHEADER) + getcolortablesize(bpp);
}

static int getcompression(int bpp)
{
    switch (bpp)
    {
        case  1: return BI_RGB;
        case  4: return BI_RGB;
        case  8: return BI_RGB;
        case 16: return BI_BITFIELDS;
        case 24: return BI_RGB;
        case 32: return BI_BITFIELDS;
    }
    ASSERT(0);
    return 0;
}


DIB* DIB_create(int width, int height, int bpp)
{
    DIB* dib = NULL;
    BITMAPINFOHEADER* bih = NULL;

    ASSERT(width  > 0 && width  < 10000);   /* sanity checks for probable garbage upper limit (increase if neccessary) */
    ASSERT(height > 0 && height < 10000);
    ASSERT(bpp == 1 || bpp == 4 || bpp == 8 || bpp == 16 || bpp == 24 || bpp == 32);

    /* allocate DIB */
    dib = (DIB*)galloc(sizeof(DIB));
    if (dib == NULL)
        return NULL;

    /* allocate header */
    dib->bmi = (BITMAPINFO*)galloc(getbmisize(bpp));
    if (dib->bmi == NULL)
    {
        gfree(dib);
        return NULL;
    }

    /* fill in header */
    bih                     = (BITMAPINFOHEADER*)dib->bmi;
    bih->biSize             = sizeof(BITMAPINFOHEADER);
    bih->biWidth            = width;
    bih->biHeight           = -height;
    bih->biPlanes           = 1;
    bih->biBitCount         = (WORD)bpp;
    bih->biCompression      = getcompression(bpp);

    bih->biSizeImage        = 0;
    bih->biXPelsPerMeter    = 0;
    bih->biYPelsPerMeter    = 0;
    bih->biClrImportant     = 0;
    bih->biClrUsed          = 0;    /* will be set when bmiColors are set if need be */
                                    /* zero means bmp will use max number it can (default) */

    /* fill in bmiColors field */
    if (bpp == 1)
    {
        RGBQUAD* rgb = DIB_getcolourtable(dib);
        rgb[0].rgbBlue = rgb[0].rgbGreen = rgb[0].rgbRed = 0;
        rgb[1].rgbBlue = rgb[1].rgbGreen = rgb[1].rgbRed = 255;
        rgb[0].rgbReserved = rgb[1].rgbReserved = 255;
    }
    else if (bpp == 4 || bpp == 8)
    {
        RGBQUAD* rgb = DIB_getcolourtable(dib);
        int num = DIB_getcolourcount(dib);
        int i = 0;
        for (i = 0; i < num; i++)
        {
            rgb->rgbBlue  = (unsigned char)(i*256/(num));
            rgb->rgbGreen = (unsigned char)(i*256/(num));
            rgb->rgbRed   = (unsigned char)(i*256/(num));
            rgb->rgbReserved = 255;
            rgb++;
        }
    }
    else if (bpp == 16) /* 555 */
    {
        DWORD* mask = (DWORD*)DIB_getcolourtable(dib);
        mask[0] = 0x7c00;       /* red   (01111100 00000000) */
        mask[1] = 0x03e0;       /* green (00000011 11100000) */
        mask[2] = 0x001f;       /* blue  (00000000 00011111) */
        bih->biSizeImage = (DWORD)(DIB_getheight(dib) * DIB_getrowbytes(dib));
    }
    else if (bpp == 32) /* note that if bpp == 24 nothing is filled in the bmiColors field */
    {
        DWORD* mask = (DWORD*)DIB_getcolourtable(dib);
        mask[0] = 0x00ff0000;   /* red   (00000000 11111111 00000000 00000000) */
        mask[1] = 0x0000ff00;   /* green (00000000 00000000 11111111 00000000) */
        mask[2] = 0x000000ff;   /* blue  (00000000 00000000 00000000 11111111) */
        bih->biClrUsed = 3;
        bih->biSizeImage = (DWORD)(DIB_getheight(dib) * DIB_getrowbytes(dib));
    }

    /* allocate frame buffer */
    dib->hbitmap = CreateDIBSection(NULL, dib->bmi, DIB_RGB_COLORS, (void**)(&dib->base), NULL, 0);
    if (dib->hbitmap  == NULL || dib->base == NULL)
    {
        gfree(dib->bmi);
        gfree(dib);
        return NULL;
    }

    return dib;
}

void DIB_destroy(DIB* dib)
{
    if (dib)
    {
        if (dib->bmi)
        {
            gfree(dib->bmi);
        }

        if (dib->hbitmap)
        {
            ASSERT(dib->base != NULL);
            DeleteObject(dib->hbitmap);
        }
        gfree(dib);
    }
}

int DIB_getwidth(const DIB* dib)
{
    ASSERT(DIB_isvalid(dib));
    return dib->bmi->bmiHeader.biWidth;
}

int DIB_getheight(const DIB* dib)
{
    ASSERT(DIB_isvalid(dib));
    return abs(dib->bmi->bmiHeader.biHeight);
}

int DIB_getbpp(const DIB* dib)
{
    ASSERT(DIB_isvalid(dib));
    return dib->bmi->bmiHeader.biBitCount;
}

int DIB_getrowbytes(const DIB* dib)
{
    int bpp   = DIB_getbpp(dib);
    int width = DIB_getwidth(dib);

    /* DIB's are 4 byte aligned */
    return (bpp * width + 31)/32 * 4;
}

void* DIB_getbase(DIB* dib)
{
    ASSERT(DIB_isvalid(dib));
    return dib->base;
}

RGBQUAD* DIB_getcolourtable(DIB* dib)
{
    ASSERT(DIB_isvalid(dib));
    return (RGBQUAD*)((char*)(dib->bmi) + (WORD)(dib->bmi->bmiHeader.biSize));
}

int DIB_getcolourcount(const DIB* dib)
{
    int bpp;

    ASSERT(DIB_isvalid(dib));

    bpp = DIB_getbpp(dib);
    if (bpp <= 0 || bpp > 8)
        return 0;

    if (bpp <= 1)
        return 2;

    if (bpp <= 4)
        return 16;

    /* bpp now has to be <= 8 */
    ASSERT(bpp <= 8);
    return 256;
}

HBITMAP DIB_getbitmap(const DIB* dib)
{
    ASSERT(DIB_isvalid(dib));
    return dib->hbitmap;
}

const BITMAPINFO* DIB_getbitmapinfo(const DIB* dib)
{
    ASSERT(DIB_isvalid(dib));
    return dib->bmi;
}


void DIB_draw(const DIB* dib, HDC hdc, int x, int y)
{
	int w = DIB_getwidth(dib);
	int h = DIB_getheight(dib);

	StretchDIBits(hdc,
		x,y,					/* dst offset */
		w,h,					/* dst size */
		0,0,					/* src offset */
		w,h,					/* src size */
		dib->base,				/* bits */
		DIB_getbitmapinfo(dib),	/* BITMAPINFO */
		DIB_RGB_COLORS, 		/* options */
		SRCCOPY);				/* rop */
}
