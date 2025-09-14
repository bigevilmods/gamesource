// (c) 1994-2007 Electronic Arts Inc.

#ifndef __MOVGIMEX
#define __MOVGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  MOV Raw Gimex                                               */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL MOV_about(void);
int      GCALL MOV_is(GSTREAM *g);

/* Import Functions */

int      GCALL MOV_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL MOV_info(GINSTANCE *gx, int framenum);
int      GCALL MOV_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL MOV_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL MOV_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL MOV_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL MOV_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif

typedef struct
{
    /* MVHD Movie Header CHUNK structure */

        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char CreationTime[4];
        char ModificationTime[4];
        char TimeScale[4];
        char Duration[4];
        char PreferredRate[4];
        char PreferredVolume[2];
        char Reserved[10];
        char Matrix[9][4];
        char PreviewTime[4];
        char PreviewDuration[4];
        char PosterTime[4];
        char SelectionTime[4];
        char SelectionDuration[4];
        char CurrentTime[4];
        char NextTrackID[4];

} MOVMVHDStruct;


/*
 * Flag field values for the 'tkhd'
 */
#define MOV_TRACK_ENABLED    0x0001
#define MOV_TRACK_IN_MOVIE   0x0002
#define MOV_TRACK_IN_PREVIEW 0x0004
#define MOV_TRACK_IN_POSTER  0x0008

typedef struct
{
    /* TKHD Track Header CHUNK structure */

        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char CreationTime[4];
        char ModificationTime[4];
        char TrackID[4];
        char Reserved[4];
        char Duration[4];
        char Reserved2[8];
        char Layer[2];
        char AlternateGroup[2];
        char Volume[2];
        char Reserved3[2];
        char Matrix[9][4];
        char TrackWidth[4];
        char TrackHeight[4];

} MOVTKHDStruct;


typedef struct
{
    /* ELST Edit List CHUNK structure */

        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char NumEntries[4];
        char TrackDuration[4];
        char MediaTime[4];
        char MediaRate[4];

} MOVELSTStruct;


typedef struct
{
    /* MDHD Media Header CHUNK structure */

        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char CreationTime[4];
        char ModificationTime[4];
        char TimeScale[4];
        char Duration[4];
        char Language[2];
        char Quality[2];

} MOVMDHDStruct;



typedef struct
{
    /* HDLR Media Handler CHUNK structure */

        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char ComponentType[4];
        char ComponentSubType[4];
        char ComponentManufacturer[4];
        char ComponentFlags[4];
        char ComponentFlagsMask[4];
        char ComponentName[26];

} MOVHDLRStruct;



typedef struct
{
    /* VMHD Video Media Header CHUNK structure */

        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char GraphicsMode[2];
        char Opcolor[6];

} MOVVMHDStruct;


typedef struct
{
    /* HDLR Data Handler CHUNK structure */

        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char ComponentType[4];
        char ComponentSubType[4];
        char ComponentManufacturer[4];
        char ComponentFlags[4];
        char ComponentFlagsMask[4];
        char ComponentName[25];

} MOVDHDLRStruct;

typedef struct
{
    /* DINF/DREF/ALIS Data Information CHUNK structures */

        char Size[4];
        char Type[4];
    struct
    {
        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char NumberOfEntries[4];
        struct
        {
                char Size[4];
                char Type[4];
                char Version[1];
                char Flags[3];
        } ALIS;
    } DREF;

} MOVDINFStruct;

/* See Inside Quicktime 3-173 */

typedef struct
{
    /* STSD Sample Description CHUNK structure */

        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char SampleSize[4];
    struct
    {
        char IdSize[4];
        char CodecType[4];
        char Reserved1[4];
        char Reserved2[2];
        char DataRefIndex[2];
        char Version[2];
        char RevisionLevel[2];
        char Vendor[4];
        char TemporalQuality[4];
        char SpatialQuality[4];
        char Width[2];
        char Height[2];
        char HRes[4];
        char VRes[4];
        char DataSize[4];
        char FrameCount[2];
        char Name[32];
        char Depth[2];
        char ClutID[2];
    } STSD;

} MOVSTSDStruct;

typedef struct
{
    /* STTS Time to Sample CHUNK structure */

        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char NumberOfEntries[4];
        char SampleCount[4];
        char SampleDuration[4];

} MOVSTTSStruct;

typedef struct
{
    /* STSS Sync Sample (keyframe) atom */
        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char NumberOfEntries[4];
        char Table[1][4];

} MOVSTSSStruct;

typedef struct
{
    /* Sample to Chunk table */
        char FirstChunk[4];
        char SamplesPerChunk[4];
        char SampleDescription[4];

} MOVSTCTStruct;

typedef struct
{
    /* STSC Sample to Chunk CHUNK structure */

        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char NumberOfEntries[4];
        MOVSTCTStruct Table[1];

} MOVSTSCStruct;

typedef struct
{
    /* STSZ Sample Size CHUNK structure */

        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char SampleSize[4];
        char NumberOfEntries[4];
        char Table[1][4];

} MOVSTSZStruct;

typedef struct
{
    /* STCO Chunk Offset CHUNK structure */

        char Size[4];
        char Type[4];
        char Version[1];
        char Flags[3];
        char NumberOfEntries[4];
        char Table[1][4];

} MOVSTCOStruct;

typedef struct
{
    /* UDTA User Data CHUNK structure */

        char Size[4];
        char Type[4];
        char NumberOfEntries[4];
        char ID[4];
        char Field1[4];
        char Field2[4];

} MOVUDTAStruct;

typedef struct
{
    /* CTAB Colour table */
        char Size[4];
        char Type[4];
        char Seed[4];
        char Flags[2];
        char NumberOfEntries[2];
        char Table[1][64];

} MOVCTABStruct;

/* Windows types */
typedef unsigned short MOVWORD;
typedef unsigned int  MOVDWORD;
typedef int            MOVLONG;
typedef unsigned char  MOVBYTE;

typedef struct /* BITMAPINFOHEADER */
{
        MOVDWORD Size;
        MOVLONG Width;
        MOVLONG Height;
        MOVWORD Planes;
        MOVWORD BitCount;
        MOVDWORD Compression;
        MOVDWORD ImageSize;
        MOVLONG XpelsMeter;
        MOVLONG YpelsMeter;
        MOVDWORD NumberOfColours;
        MOVDWORD ImportantColours;

} MOVBMIStruct;

typedef struct /* RGBQUAD */
{
        MOVBYTE    rgbBlue;
        MOVBYTE    rgbGreen;
        MOVBYTE    rgbRed;
        MOVBYTE    rgbReserved;

} MOVRGBStruct;

typedef struct /* BITMAPINFO */
{
        MOVBMIStruct    bmiHeader;
        MOVRGBStruct    bmiColours[256];

} MOVBMIHStruct;


typedef struct MOVINFO
{
    int   width;
    int   height;
    int   bpp;
    int   fps;
    int   subtype;
    int   compression;
    int  mdatpos;
    char  *movheader;
    MOVSTCOStruct *chunkoffsettbl;
    MOVSTSCStruct *sampletochunk;
    MOVSTSZStruct *samplesizetbl;
    MOVBMIHStruct *bitmapinfo;
    MOVSTSSStruct *keyframes;
    int  numcolours;
    ARGB *colourtbl;
    int valid;          /* set in the open so the read will fail but the open and info still work */

    /* The following are for windows gimex only */
    void *movie;
    void *moviefile;
    void *library;

} MOVINFO;


#define MOVRAWFCC GMAKEID('r','a','w',' ')
#define MOVLIBRARYFILENAME "quicktime"

#define MOVGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define MOVPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))


#ifdef __cplusplus
}
#endif
#endif

