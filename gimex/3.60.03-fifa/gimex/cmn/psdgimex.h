// (c) 1994-2007 Electronic Arts Inc.

#ifndef __PSDGIMEX
#define __PSDGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  PSD Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL PSD_about(void);
int      GCALL PSD_is(GSTREAM *g);

/* Import Functions */

int      GCALL PSD_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL PSD_info(GINSTANCE *gx, int framenum);
int      GCALL PSD_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL PSD_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL PSD_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL PSD_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL PSD_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

struct PSDHeaderStruct
{
    /* PHOTOSHOP FILE HEADER structure 26 bytes */

    char signature[4];
    char version[2];
    char reserved[6];
    char channels[2];
    char rows[4];
    char columns[4];
    char depth[2];
    char mode[2];

};

#define PSDGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define PSDPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

struct PSDDpiResource
{
    /* DPI RESOURCE structure 16 bytes */

    char xdpi[4];
    char xmeasure[2];
    char xflag[2];
    char ydpi[4];
    char ymeasure[2];
    char yflag[2];

};

const int LAYER_MAXNAME=255;
struct PSDLAYERINFO
{
    int top;
    int left;
    int bottom;
    int right;
    long long offsets[4];            /* Disk offsets to channels. 0 = red, 1 = green, 2 = blue, 3 = transparency */
    short channels;             /* may be different than PSDINFO->channels */
    char name[LAYER_MAXNAME];   /* Name of layer */
    char *data;
    int packed;                 /* Whether to pack data for this layer or not */
    unsigned char flags;
    unsigned char opacity;
    int blendmethod;
    int layerset;
};

struct PSDINFO
{
    short version;             /* PSD=1; PSB=2*/
    int width;                 /* Width of image */
    int height;                /* Height of image */
    short depth;                /* Depth */
    short channels;             /* # of channels in image */
    short mode;                 /* Mode of image */
    unsigned char pal[256*3];   /* CLUT */
    long long imgoffset;        /* Where the image data starts */
    int dpi;                   /* DPI of image in 16.16 fixed point */
    PSDLAYERINFO * layerinfo;   /* Array of layer info */
    int numhotspots;
    int hotspottbl[GIMEX_HOTSPOTTBL_SIZE][GIMEX_HOTSPOTTBL_VALUES]; /* Copy paths to hotspot table for ginfo */

};


struct GINFOPSD
{
    GINFO ginfo;
    int blendmethod;
    int visibleflag;
    int opacitylevel;

// PAULDING START
    int canvasw;
    int canvash;
// PAULDING END
};


#ifdef __cplusplus
}
#endif
#endif

