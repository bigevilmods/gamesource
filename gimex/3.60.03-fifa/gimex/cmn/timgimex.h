// (c) 1994-2007 Electronic Arts Inc.

#ifndef __TIMGIMEX
#define __TIMGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  TIM Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL TIM_about(void);
int      GCALL TIM_is(GSTREAM *g);

/* Import Functions */

int      GCALL TIM_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL TIM_info(GINSTANCE *gx, int framenum);
int      GCALL TIM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL TIM_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL TIM_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL TIM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL TIM_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

typedef struct
{
    /* PSX TEXURE IMAGE FILE HEADER structure 18 bytes */

    char id[4];                 /* id field must be 0x00000010) */
    char flags[4];              /* flags (see defines) */

} TIMHeaderStruct;

/* flags for flags TIMHeaderStruct.flags field */

#define TIM_CLUT_PRESENT  0x08  /* specifies CLUT is in file                                      */
#define TIM_16_COLORS     0x00  /* pixels are 16 color indexes (4 bit) pixels are nibble reversed */
#define TIM_256_COLORS    0x01  /* pixles are 256 color indexes (8 bit)                           */
#define TIM_15_BIT_RGB    0x02  /* pixels are 1-5-5-5 PSX direct colors (no CLUT required)        */
#define TIM_24_BIT_RGB    0x03  /* pixels are 8-8-8 PSX direct colors (no CLUT required)          */
#define TIM_MIXED_MODE    0x04  /* ?????                                                          */
#define TIM_MODE_MASK     0x03  /* mask for determining pixel type                                */

typedef struct
{
    unsigned char   length[4];     /* length of struct tim_clut */
    unsigned char   x_pos[2];      /* intended uper left corner in VRAM */
    unsigned char   y_pos[2];      /* intended uper left corner in VRAM */
    unsigned char   width[2];      /* width in VRAM pixels of CLUT (colors only) */
    unsigned char   height[2];     /* height in VRAM pixels of CLUT (colors only) */

  /*unsigned short  colors[]; \\ actual BGR data */
                              /* number of colors is ((length - 0x0c) \ 2) */
} TIMClutStruct;

typedef struct
{
    unsigned char   length[4]; /* length of struct tim_pixels */
    unsigned char   x_pos[2];  /* intended uper left corner in VRAM */
    unsigned char   y_pos[2];  /* intended uper left corner in VRAM */
    unsigned char   width[2];  /* width in VRAM pixels of texture */
    unsigned char   height[2]; /* height in VRAM pixels of texture */

    /* pixels seem to be written as little endian shorts despite their size */
    /* unsigned char   pixels[]; \\ actual pixels (note size may vary) */
                          /* size of all the pixels is (length - 0x0c) */
                          /* to find the number of pixels divide by the pixel size */
                          /* as determined by the tim_header.flags field */
} TIMPixelStruct;


#define TIMGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define TIMPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif


