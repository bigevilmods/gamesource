// (c) 1994-2007 Electronic Arts Inc.

#ifndef __MV8GIMEX
#define __MV8GIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  MV8 Cobra Gimex                                             */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL MV8_about(void);
int      GCALL MV8_is(GSTREAM *g);

/* Import Functions */

int      GCALL MV8_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL MV8_info(GINSTANCE *gx, int framenum);
int      GCALL MV8_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL MV8_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL MV8_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL MV8_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL MV8_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

struct AtomHeader
{
    unsigned char       ah_Type[4];
    unsigned char       ah_Size[4];
};

struct OldFrameHeader
{
    struct AtomHeader   ofh_Atom;
    unsigned char       ofh_Type[4];
    unsigned char       ofh_Compression[4];
    unsigned char       ofh_BitBuffSize[4];
    unsigned char       ofh_DCTBuffSize[4];
};

struct FrameHeader
{
    struct AtomHeader   fh_Atom;
    unsigned char       fh_Type[1];
    unsigned char       fh_Compression[1];
    unsigned char       fh_Detail1Size[2];
    unsigned char       fh_Detail2Size[2];
    unsigned char       fh_SmoothSize[2];
    unsigned char       fh_BitBuffSize[2];
    unsigned char       fh_PaletteSize[2];
    unsigned char       fh_DetailSwitch[2];
    unsigned char       fh_Reserved[2];
};


struct AudioHeader
{
    struct AtomHeader   afh_Atom;
    unsigned char       afh_Type[4];
    unsigned char       afh_Channels[4];
};

struct FrameTableEntry
{
    unsigned char       fte_Offset[4];
    unsigned char       fte_Size[4];
    unsigned char       fte_Time[4];
    unsigned char       fte_Duration[4];
};

struct MovieHeader
{
    struct AtomHeader   mh_Atom;
    unsigned char       mh_Width[4];
    unsigned char       mh_Height[4];
    unsigned char       mh_Scale[4];
};

struct FileHeader
{
    unsigned char       ch_fileid[4];
    unsigned char       ch_mode[4];
    unsigned char       ch_fps[2];
    unsigned char       ch_samplerate[2];
    unsigned char       ch_t[4*5];
};

struct MovieContext
{
    unsigned int  *mc_Past;
    unsigned int  *mc_Future;
    unsigned int  *mc_Decode;
    unsigned int  *mc_Display;
    unsigned int  *mc_NextDisp;
    unsigned int  mc_ValidPast;
    unsigned int  mc_MovieWidth;
    unsigned int  mc_MovieHeight;
    unsigned int  mc_MovieOffsetX;
    unsigned int  mc_MovieOffsetY;
    unsigned int  *mc_Frames[4];

    unsigned char  *mc_DCB;
    unsigned char  mc_SCB[256*16*2];

    unsigned char           *AudioBuff;
    unsigned char           *myMovie;
    struct FrameTableEntry  *FrameMovie;
    unsigned int            MAXBUFFSIZE;
    unsigned int            BUFFSIZE;
    unsigned int           palettebuf[192]; /* 192 longs = 256 colours */
    struct MovieHeader      movieh;
    unsigned int           mheaderlen;
    struct FileHeader       fheader;
    struct AudioHeader      aheader;
    int                     currentframe;
    int                     framecount;
    int                    framepos;
    char                    *framebuf;
    int                     oldformat;
};


#define FT_INTRA        0x00
#define FT_INTER        0x01

#define FC_RAW          0x00
#define FC_DCT          0x01
#define FC_MOTION4      0x02
#define FC_MOTION8      0x03
#define FC_CBR           0x04
#define FC_CBR8          0x05


#define FT_AUDIO        0x80

#define AC_NONE         0x00
#define AC_PCM2         0x01
#define AC_PCM26        0x02
#define AC_PCM4         0x03

#define FALSE 0
#define TRUE  1

/* The maximum number of cell's we'll support */
#define MAX_SCREEN_WIDTH    512
#define MAX_SCREEN_HEIGHT   240
#define MIN_BLOCK_SIZE          4               /* Minimum block size */
#define MAX_H_BLOCKS            (MAX_SCREEN_WIDTH / MIN_BLOCK_SIZE)
#define MAX_V_BLOCKS            (MAX_SCREEN_HEIGHT / MIN_BLOCK_SIZE)
#define MAX_CELLS                       (MAX_H_BLOCKS * MAX_V_BLOCKS)

#define DELTA 20

#define MV8GETFIELD(field) ggeti((void *)&field, sizeof(field))
#define MV8PUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

