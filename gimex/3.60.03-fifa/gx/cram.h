// (c) 1996-2004 Electronic Arts Inc.

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef _CRAM_H_
#define _CRAM_H_ 1

typedef struct CRAMRECT
{
    int width;  /* in/out */
    int height; /* in/out */
    int bpp;    /* in     */
	int originalbpp;    /* in     */
	int group;
    int x;      /* in/out */
    int y;      /* in/out */
    bool rotate; /* in/out */
    int  done;   /* in/out */
} CRAMRECT;


/* cram states */
#define CRAMSTATE_INITIALIZED                   0 /* READONLY 0=cram module is initialized, 1=not initialized (call CRAM_init) */
#define CRAMSTATE_CHECK_TEXTUREPAGE             1 /* CRAM_IGNORE_TEXTUREPAGE, CRAM_CHECK_TEXTUREPAGE (default), CRAM_CHECK_TEXTUREPAGE_IF_POSSIBLE */
#define CRAMSTATE_CHECK_EDGES                   2 /* 0=textures can touch right/bottom edges, 1=can't touch (default) */
#define CRAMSTATE_TEXTUREPAGE_WIDTH             3 /* texture page width in pixels (default 256) */
#define CRAMSTATE_TEXTUREPAGE_HEIGHT            4 /* texture page height in pixels (default 256) */
#define CRAMSTATE_TARGET_X                      5 /* target rectangle */
#define CRAMSTATE_TARGET_Y                      6 /* target rectangle */
#define CRAMSTATE_TARGET_WIDTH                  7 /* target rectangle */
#define CRAMSTATE_TARGET_HEIGHT                 8 /* target rectangle */
#define CRAMSTATE_ALLOW_ROTATE                  9 /* 0=don't rotate images (default),1=rotate images 90CW to make a better fit */
#define CRAMSTATE_PLATFORM                     10 /* CRAM_PLATFORM_DEFAULT (default),CRAM_PLATFORM_WIN,CRAM_PLATFORM_PSX */
#define CRAMSTATE_ODD_ODD                      11 /* make the height odd if the odd width and even height */
#define CRAMSTATE_TEXTUREPAGE_ALIGNMENT		   12 /* texture page alignment (usually 64) */
#define CRAMSTATE_WIDTHIN					   13 /* cram fails if the width of the image is bigger than the texture page width */
#define CRAMSTATE_SPACE						   14 /* puts some space between the shapes */

/* cram values */
#define CRAM_IGNORE_TEXTUREPAGE                 0 /* image can cross texture pages */
#define CRAM_CHECK_TEXTUREPAGE                  1 /* force image to respect texture pages */
#define CRAM_CHECK_TEXTUREPAGE_IF_POSSIBLE      2 /* force image to respect texture pages only if image is smaller than a texture page */

#define CRAM_PLATFORM_DEFAULT                   0
#define CRAM_PLATFORM_WIN                       0
#define CRAM_PLATFORM_PSX                       1


/* cram api */

typedef struct CRAM CRAM;

CRAM *CRAM_init(void);
void CRAM_close(CRAM *cram);

int  CRAM_getstate(CRAM *cram,int state);
void CRAM_setstate(CRAM *cram, int state, int value);
void CRAM_settexturepagesize(CRAM *cram, int width, int height);
void CRAM_setrect(CRAM *cram, int x, int y, int width, int height);

int  CRAM_process(CRAM *cram, int numrects, CRAMRECT *rects);

/* obsolete functions */

/* cram flags - use CRAM_setstate() instead */
#define CRAM_CHECKUV                 0x01
#define CRAM_CHECKUV_IF_SMALLER      0x02
#define CRAM_ROTATE                  0x04
void CRAM_setflags(CRAM *cram, unsigned int flags);

#endif

#ifdef __cplusplus
}
#endif

