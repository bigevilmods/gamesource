// (c) 1994-2007 Electronic Arts Inc.

#ifndef __ANMGIMEX
#define __ANMGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  ANM Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL ANM_about(void);
int      GCALL ANM_is(GSTREAM *g);

/* Import Functions */

int      GCALL ANM_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL ANM_info(GINSTANCE *gx, int framenum);
int      GCALL ANM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL ANM_close(GINSTANCE *gx);

/* Export Functions (stubbed) */

int      GCALL ANM_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL ANM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL ANM_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

typedef struct
{
    /* ANM FILE HEADER structure 128 bytes */

    char id[4];             /* 4 character ID == "LPF " */
    char maxLps[2];         /* max # largePages allowed. 256 FOR NOW.   */
    char nLps[2];           /* # largePages in this file. */
    char nRecords[4];       /* # records in this file.  65534 is current limit plus */
                            /* one for last-to-first delta for looping the animation */
    char maxRecsPerLp[2];   /* # records permitted in an lp. 256 FOR NOW.   */
    char lpfTableOffset[2]; /* Absolute Seek position of lpfTable.  1280 FOR NOW.
                               The lpf Table is an array of 256 large page structures
                               that is used to facilitate finding records in an anim
                               file without having to seek through all of the Large
                               Pages to find which one a specific record lives in. */
    char contentType[4];    /* 4 character ID == "ANIM" */
    char width[2];          /* Width of screen in pixels. */
    char height[2];         /* Height of screen in pixels. */
    char variant;           /* 0==ANIM. */
    char version;           /* 0==frame rate is multiple of 18 cycles/sec.
                               1==frame rate is multiple of 70 cycles/sec.  */
    char hasLastDelta;      /* 1==Last record is a delta from last-to-first frame. */
    char lastDeltaValid;    /* 0==The last-to-first delta (if present) hasn't been
                               updated to match the current first&last frames, so it
                               should be ignored. */
    char pixelType;         /* 0==256 color. */
    char CompressionType;   /* 1==(RunSkipDump) Only one used FOR NOW. */
    char otherRecsPerFrm;   /* 0 FOR NOW. */
    char bitmaptype;        /* 1==320x200, 256-color.   Only one implemented so far. */
    char recordTypes[32];   /* Not yet implemented. */
    char nFrames[4];        /* In case future version adds other records at end of
                               file, we still know how many actual frames.
                               NOTE: DOES include last-to-first delta when present. */
    char framesPerSecond[2];/* Number of frames to play per second. */
    char pad2[58];          /* 58 bytes of filler to round up to 128 bytes total. */
} lpfileheader;


typedef struct
{
    /* ANM LARGE PAGE structure 6 bytes */

    char baseRecord[2];    /* Number of first record in this large page. */
    char nRecords[2];      /* Number of records in lp.
                              bit 15 of "nRecords" == "has continuation from previous lp".
                              bit 14 of "nRecords" == "final record continues on next lp". */
    char nBytes[2];        /* Total number of bytes of contents, excluding header. */
} lp_descriptor;

#define sizeoflp_descriptor 6

typedef struct
{
    lpfileheader   DAHEADER;          /* file header will be loaded into this structure */
    lp_descriptor  LpArray[256];      /* arrays of large page structs used to find frames */
    unsigned short curlpnum;          /* initialize to an invalid Large page number */
    lp_descriptor  curlp;             /* header of large page currently in memory */
    unsigned char  *thepage;          /* pointer to the buffer where current large page is loaded */
    ARGB           animpalette[256];
    unsigned char  *animwin;          /* anim frames are loaded here */

    /* Next four fields used in writing ANM files only (not used in the reading module) */
    unsigned char  *temp_win;         /* Window used to hold bitmap data temporarily. */
    unsigned char  *temp_data;        /* Space used to hold compressed data temporarily. */
    unsigned char  *first_frame;      /* The data from the first frame is held here, it will be    */
                                      /* used at end of writing to create the last-to-first delta. */
    unsigned short recsizes[256];     /* Sizes of the records in thepage */
} ANMINFO;

#define ANMGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define ANMPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))


#ifdef __cplusplus
}
#endif
#endif

