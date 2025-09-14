// (c) 1994-2007 Electronic Arts Inc.

#ifndef __TGVGIMEX
#define __TGVGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  TGV Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL TGV_about(void);
int      GCALL TGV_is(GSTREAM *g);

/* Import Functions */

int      GCALL TGV_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL TGV_info(GINSTANCE *gx, int framenum);
int      GCALL TGV_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL TGV_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL TGV_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL TGV_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL TGV_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#define TGVSIGN GMAKEID('g','T','G','V')
#define TGVID   GMAKEID(' ','T','G','V')
#define TGVK    GMAKEID('T','G','V','k')
#define TGVF    GMAKEID('T','G','V','f')

#define KLUDGE 0

#define SZ

#define MOTIONBITS 10

typedef struct
{
    /* 'TGVk' FILE HEADER structure 20 bytes */

    char chunkid[4];            /* 'TGVk' */
    char chunksize[4];          /* size of chunk, including header */
    char frameWidth[2];         /* image width in pixels */
    char frameHeight[2];        /* image height in pixels */

    char firstColour[2];        /* first colour map entry in palette */
    char colourCount[2];        /* number of colours in palette */
    char flags[4];              /* flag 1 is raw, 0 if refpacked */

    char colourtbl[256][3];     /* up to 256 palette entries of 8 bits RGB */

} TGVKeyStruct;

typedef struct
{
    /* 'TGVf' FRAME HEADER structure 20 bytes */

    char    chunkid[4];         /* signature is TGVf */
    char    chunksize[4];       /* size of chunk, including header */
    char    numMotion[2];       /* number of motion vector in this frame */
    char    numLiteral16[2];    /* number of literal 16 in this frame */
    char    numLiteral4[2];     /* number of literal 4 in this frame */
    char    indexBits[2];       /* number of bits in index matrix */
    char    pad[4];             /* pad header to 20 bytes */

} TGVFrameStruct;

typedef struct
{
    int x;
    int y;

} TGVMOTION;

/* TGV context */

typedef struct
{
    int chunkSize;             /* size of last chunk read */
    int  frameWidth;            /* image width in pixels */
    int  frameHeight;           /* image height in pixels */
    int  firstColour;           /* first colour map entry in palette */
    int  colourCount;           /* number of colours in palette */
    int  packed;                /* 0=no,keyframe, 1=motion */
    int keyoffset;             /* file offset to last keyframe */
    int keyframe;              /* frame number of last keyframe */
    int  numMotion;             /* number of motion vector in this frame */
    int  numLiteral16;          /* number of literal 16 in this frame */
    int  numLiteral4;           /* number of literal 4 in this frame */
    int  indexBits;             /* number of bits in index matrix */
    int  maxMotion;             /* number of motion vectors allocated */
    int  maxLiteral;            /* number of literals allocated */
    char *current;              /* current frame pointer */
    char *previous;             /* previous frame pointer */
    int  *indexMatrix;          /* index matrix */
    int  *rowtbl;               /* row table */
    char *literal;              /* literal table (blocks of 4x4) */
    TGVMOTION *motion;          /* motion vectors */
    int  *offset;               /* motion offsets */
    ARGB colourtbl[256];        /* palette */

} TGVINFO;


/***********************************************************************************/
/*                       The constant of the Gimex TGV                             */
/***********************************************************************************/

#define     TGVDEBUG                1           /* Should be set to 0 */

#define     kXRes                   4
#define     kYRes                   4
#define     kXResYRes               (kXRes*kYRes)
#define     kDefFocus               20
#define     kMotionVector           1
#define     kFullLiteral            2
#define     kLiteral4               3
#define     kTVisual                13
#define     kReservVector           1
#define     kMaxFullLiteral         2500
#define     kMaxLiteral4            kMaxFullLiteral
#define     kMaxNumError            65536
#define     kInvalidIndex           -1
#define     kLowestMatch            0xFFFFFFFF
#define     kCompressVector         10
#define     kInitSizeVector         60
#define     kSizeColour             3

/***********************************************************************************/
/*                  Structure to compress and decompress an image                  */
/***********************************************************************************/

typedef struct
{
    int                 x;                      /* Save in 10 bits (9+1) */
    int                 y;                      /* Save in 10 bits (9+1) */
} MVector;

typedef struct
{
    unsigned char       PMatrix[kXResYRes];     /* kXRes*kYRes of literal pixel */
} LiteralMatrix;

typedef struct
{
    unsigned char       CPalette[4];            /* Colour palette of 4 colours */
    unsigned char       PMatrix[kXResYRes*2/8]; /* kXRes*kYRes of literal pixel in 2 bits */
} Literal4Matrix;

typedef struct
{
    int                 NbMotionVector;         /* The number of motion vector in the list */
    int                 MaxMotionVector;        /* The maximum number of motion vector in the list */
    MVector             ListMV[1];              /* The list of best vectors */
} DMVector;

typedef struct
{
    int                 NbLiteral;              /* The number of best literal in the list */
    int                 MaxLiteral;             /* The maximum number of literal in the list */
    LiteralMatrix       ListLiteral[1];         /* The list of the best literals */
} DLiteral;

typedef struct
{
    int                 xMatrix;                /* Number of cell in x */
    int                 yMatrix;                /* Number of cell in y */
    int                 MXIndex[1];             /* The index to retrieve an element in a dictionary */
} DIMatrix;

typedef struct
{
    unsigned int       BestMatch;              /* Best match values */
    unsigned int        TypeDictionary;         /* Dictionary used */
} WkMatrix;

/***********************************************************************************/
/*                         Structure to compress an image                          */
/***********************************************************************************/

typedef struct
{
    unsigned int       Frequency[1];           /* The frequency of use */
} FrequencyRec;

typedef struct
{
    DIMatrix*           TheMatrix;              /* xMatrix*yMatrix of index */
    WkMatrix*           WorkMatrix;             /* xMatrix*yMatrix of working elements */
    DMVector*           TheVector;              /* The dictionnary of vectors */
    FrequencyRec*       MVFrequency;            /* The frequency record for the vectors */
    DLiteral*           TheLiteral;             /* The dictionnary of literals of 16 colours */
    FrequencyRec*       LFrequency;             /* The frequency record for the literals of 16 colours */
    DLiteral*           TheLiteral4;            /* The dictionnary of literals using 4 colours */
    FrequencyRec*       LFrequency4;            /* The frequency record for the literals using 4 colours */
} TgvEnviro;

/***********************************************************************************/
/*                        General structure for this Gimex                         */
/***********************************************************************************/

typedef struct TgvGimexData
{
    int                 fWidth;                 /* To control the size */
    int                 fHeight;
    int                 fStartColour;
    int                 fNbColour;
    ARGB                fColourTable[256];
    char *              fPrevSource;            /* Build using fWidth and fHeight */
    unsigned int       *fError;                /* To write only */
} TgvGimexData;

/***********************************************************************************/
/*                 Data used by each instance of this Gimex                        */
/***********************************************************************************/

typedef struct FrameStruct
{
    char                chunkid[4];             /* signature is TGVf */
    char                chunksize[4];           /* size of chunk, including header */
    char                NbMotionVector[2];      /* Number of motion vector in this frame */
    char                NbLiteral16[2];         /* Number of literal 16 in this frame */
    char                NbLiteral4[2];          /* Number of literal 4 in this frame */
    char                MatrixDepth[2];         /* Depth of the matrix */
    char                reserved[4];            /* Reserved */
} FrameStruct;

typedef struct KeyFrameStruct
{
    char                chunkid[4];             /* signature is TGVk */
    char                chunksize[4];           /* size of chunk, including header */
    char                w[2];                   /* width in pixels */
    char                h[2];                   /* height in pixels */
    char                startcolour[2];         /* The first index in the palette */
    char                nbcolour[2];            /* Number of colours in the palette */
    char                flags[4];               /* flag 1 if not refpacked */
} KeyFrameStruct;

typedef struct ChunkHeaderStruct
{
    char                chunkid[4];             /* signature */
    char                chunksize[4];           /* size of chunk, including header */
} ChunkHeaderStruct;

/***********************************************************************************/
/*                 Type read and write (intel/motorola)                            */
/***********************************************************************************/

#define TGVSETADRESS gputi

#ifndef GIMEXIDLE
#define GIMEXIDLE
#endif





#define TGVGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define TGVPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif


