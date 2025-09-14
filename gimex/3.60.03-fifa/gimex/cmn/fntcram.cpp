// (c) 2000-2007 Electronic Arts Inc.
#include "cram.h"

#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#endif

static CRAMRECT* AllocRects(const FONTWRITEINFO* pFontInfo)
{
	CRAMRECT*	rects;	// array of cram rects
	int			numrects;	// number of characters to process
	int			i;		// misc loop counter

	numrects = FNTGETFIELD(pFontInfo->header->num);
	rects = galloc(numrects * sizeof(CRAMRECT));
	if (rects != NULL)
	{
		for (i = 0; i < numrects; ++i)
		{
			// allocate rects to have an extra pixel on width/height so there is a blank
			// pixel between all characters
			// see the comment below about padding an extra pixel around characters

			rects[i].width	= FNTGETFIELD(pFontInfo->chartbl[i].width)  + 1;
			rects[i].height = FNTGETFIELD(pFontInfo->chartbl[i].height) + 1;
			rects[i].bpp    = pFontInfo->bpp;
			rects[i].x      = 0;
			rects[i].y      = 0;
			rects[i].rotate = 0;
			rects[i].done   = 0;
		}
	}
	return rects;
}

static void ResetRects(CRAMRECT* rects, int numrects)
{
	int i;

	ASSERT(rects != NULL);

	for (i = 0; i < numrects; ++i)
	{
		rects[i].x      = 0;
		rects[i].y      = 0;
		rects[i].rotate = 0;
		rects[i].done   = 0;
	}
}

static void UpdateUVFields(FONTWRITEINFO* pFontInfo, int* shapeWidth, int* shapeHeight)
{
	CRAM* cram;

	ASSERT(pFontInfo   != NULL);
	ASSERT(shapeWidth  != NULL);
	ASSERT(shapeHeight != NULL);

	*shapeWidth  = 0;
	*shapeHeight = 0;
		
	cram = CRAM_init();
	if (cram != NULL)
	{
		CRAMRECT* rects;

		rects = AllocRects(pFontInfo);
		if (rects != NULL)
		{
			int i;
			int numrects;

			// setup arrays for width and height for each platform
			// try each rectangle in the array until we find a fit

#if FNTTYPE == FNTTYPE_PFN
			int numattempts = 14;
			int widths[]    = { 256,256,256,256, 256,256,256,256, 256,256,256,256, 256, 256 };
			int heights[]   = {  32, 64, 96,128, 160,196,224,256, 320,384,448,512, 768,1024 };
#else
			int numattempts = 12;
			int widths[]    = {  16,32,64,128, 256,256,256, 256,  256, 256, 256, 256 };
			int heights[]   = {  16,32,64,128, 256,512,768,1024, 1280,1536,1792,2048 };
#endif

			numrects = FNTGETFIELD(pFontInfo->header->num);

			// NOTES ABOUT PADDING AN EXTRA PIXEL AROUND CHARACTERS
			// when cramming, make the rectangle one smaller on the right and bottom edges
			// when positioning add a pixel so that the left,top edges have the blank
			// NOTE: AllocRects pads each character with an extra pixel on the right,bottom

			for (i = 0; i < numattempts; ++i)
			{
				CRAM_setstate(cram, CRAMSTATE_TEXTUREPAGE_WIDTH,	256);
				CRAM_setstate(cram, CRAMSTATE_TEXTUREPAGE_HEIGHT,	256);
				CRAM_setstate(cram, CRAMSTATE_CHECK_TEXTUREPAGE,	CRAM_CHECK_TEXTUREPAGE);
				CRAM_setstate(cram, CRAMSTATE_ALLOW_ROTATE,			0);
				
				CRAM_setrect(cram, 0,0, widths[i]-1,heights[i]-1);

				if (CRAM_process(cram, numrects, rects) != 0)
				{
					*shapeWidth  = widths[i];
					*shapeHeight = heights[i];
					break;
				}
				ResetRects(rects, numrects);
			}

			for (i = 0; i < numrects; ++i)
			{
				if (rects[i].done != 0)
				{
					int x = rects[i].x + 1; // see the comment above about padding an extra pixel around characters
					int y = rects[i].y + 1;

					ASSERT(rects[i].rotate == 0);
					ASSERT(rects[i].done   != 0);
					ASSERT(rects[i].width  + x <= *shapeWidth);
					ASSERT(rects[i].height + y <= *shapeHeight);

					FNTPUTFIELD(pFontInfo->chartbl[i].u, x);
					FNTPUTFIELD(pFontInfo->chartbl[i].v, y);
				}
			}

			gfree(rects);
		}

		CRAM_close(cram);
	}
}
