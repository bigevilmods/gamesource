// (c) 1994-2007 Electronic Arts Inc.

#ifndef __GMXGIMEX
#define __GMXGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  GMX Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL GMX_about(void);
int      GCALL GMX_is(GSTREAM *g);

/* Import Functions */

int      GCALL GMX_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL GMX_info(GINSTANCE *gx, int framenum);
int      GCALL GMX_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL GMX_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL GMX_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL GMX_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL GMX_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#define GMX_VERSION 218

typedef struct
{
    char signature[4];         /* signature of gimex ie 'tga ' (optional) */
    char size[4];              /* size of GINFO structure */
    char version[4];           /* version number of GINFO structure (200) */
    char framenum[4];          /* current frame */
    char width[4];             /* width of bitmap in pixels */
    char height[4];            /* height of bitmap in pixels */
    char bpp[4];               /* bits per pixel (8 or 32) */
    char indexed[4];           /* 0 no, 1 yes (indexed meaning it uses a palette) */
    char originalbpp[4];       /* bits per pixel in original image (1 to 32) */
    char originalindexed[4];   /* 0 no, 1 yes (indexed meaning it used a palette) */
    char originalcolours[4];   /* number of colours in original indexed palette */
    char colourtbl[256][4];    /* 8 bit palette */
    char skiprows[4];          /* number of rows to skip over */
    char subtype[4];           /* internal format sub-type 0-default */
    char packed[4];            /* type of packing on original image. 0 none, 1 run, n other */
    char quality[4];           /* quality of lossy packing 0..100 */
    char alphabits[4];         /* number of bits in alpha channel */
    char redbits[4];           /* number of bits in red channel */
    char greenbits[4];         /* number of bits in green channel */
    char bluebits[4];          /* number of bits in blue channel */
    char centerx[4];           /* center point relative to upper left corner */
    char centery[4];
    char defaultx[4];          /* default coordinate point */
    char defaulty[4];
    char reserved[12];         /* Reserved space because of an error in the original defaulty */
    char numhotspots[4];       /* number of hot spots defined */
    char framename[32];        /* null terminated name of frame/image */
    char hotspottbl[32][2][4]; /* up to 32 hot spots, XY pairs relative to upperleft */
    char comment[256];         /* up to 255 character comment */
} V1GMXHeaderStruct;

typedef struct
{
    char signature[4];         /* signature of gimex ie 'tga ' (optional) */
    char size[4];              /* size of GINFO structure */
    char version[4];           /* version number of GINFO structure (180) */
    char framenum[4];          /* current frame */
    char width[4];             /* width of bitmap in pixels */
    char height[4];            /* height of bitmap in pixels */
    char bpp[4];               /* bits per pixel (8 or 32) */
    char indexed[4];           /* 0 no, 1 yes (indexed meaning it uses a palette) */
    char originalbpp[4];       /* bits per pixel in original image (1 to 32) */
    char originalindexed[4];   /* 0 no, 1 yes (indexed meaning it used a palette) */
    char originalcolours[4];   /* number of colours in original indexed palette */
    char colourtbl[256][4];    /* 8 bit palette */
    char skiprows[4];          /* number of rows to skip over */
    char subtype[4];           /* internal format sub-type 0-default */
    char packed[4];            /* type of packing on original image. 0 none, 1 run, n other */
    char quality[4];           /* quality of lossy packing 0..100 */
    char alphabits[4];         /* number of bits in alpha channel */
    char redbits[4];           /* number of bits in red channel */
    char greenbits[4];         /* number of bits in green channel */
    char bluebits[4];          /* number of bits in blue channel */
    char centerx[4];           /* center point relative to upper left corner */
    char centery[4];
    char defaultx[4];          /* default coordinate point */
    char defaulty[4];
    char numhotspots[4];       /* number of hot spots defined */
    char framename[32];        /* null terminated name of frame/image */
    char hotspottbl[32][2][4]; /* up to 32 hot spots, XY pairs relative to upperleft */
    char comment[256];         /* up to 255 character comment */
} V2GMXHeaderStruct;

typedef struct
{
    char signature[4];         /* signature of gimex ie 'tga ' (optional) */
    char size[4];              /* size of GINFO structure */
    char version[4];           /* version number of GINFO structure (200) */
    char framenum[4];          /* current frame */
    char width[4];             /* width of bitmap in pixels */
    char height[4];            /* height of bitmap in pixels */
    char bpp[4];               /* bits per pixel (8 or 32) */
    char originalbpp[4];       /* bits per pixel in original image (1 to 32) */
    char startcolour[4];       /* first colour in palette */
    char numcolours[4];        /* number of colours in original indexed palette */
    char colourtbl[256][4];    /* 8 bit palette */
    char subtype[4];           /* internal format sub-type 0-default */
    char packed[4];            /* type of packing on original image. 0 none, 1 run, n other */
    char quality[4];           /* quality of lossy packing 0..100 */
    char framesize[4];         /* size of frame in bytes */
    char alphabits[4];         /* number of bits in alpha channel */
    char redbits[4];           /* number of bits in red channel */
    char greenbits[4];         /* number of bits in green channel */
    char bluebits[4];          /* number of bits in blue channel */
    char centerx[4];           /* center point relative to upper left corner */
    char centery[4];
    char defaultx[4];          /* default coordinate point */
    char defaulty[4];
    char numhotspots[4];       /* number of hot spots defined */
    char framename[32];        /* null terminated name of frame/image */
    char comment[1024];       /* null terminated multiline user comment */
    char hotspottbl[1024][2][4]; /* up to 1024 hot spots, XY pairs relative to upperleft */
    char dpi[4];               /* float dots per inch ie 72.0 */
    char fps[4];               /* float frame per second (one over duration).  15.0 is typical */
    char reserved[3][4];       /* reserved for future use - set to zero */
} V3GMXHeaderStruct;


#define kGMXSection     GMAKEID('G','M','X','h')
#define kGMXFrame       GMAKEID('G','M','X','p')
#define kGMXsectionSize 24
#define kGMXheaderSize  12

#define GMXGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define GMXPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

