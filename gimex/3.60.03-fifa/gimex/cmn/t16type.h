// (c) 1994-2007 Electronic Arts Inc.

#ifndef __T16type__
#define __T16type__

/***********************************************************************************/
/*                       The constant of the Gimex TGV 16 bits                     */
/***********************************************************************************/

#define     TGVDEBUG                1           /* Should be set to 0 */

#define     kXRes                   4
#define     kYRes                   4
#define     kXResYRes               (kXRes*kYRes)
#define     kDefFocus               20
#define     kMotionVector           1
#define     kHighLiteral            2
#define     kLowLiteral             3
#define     kTVisual                13
#define     kReservVector           1
#define     kMaxLiteral             2500
#define     kMaxNumError            65536
#define     kInvalidIndex           -1
#define     kLowestMatch            0xFFFFFFFF
#define     kCompressVector         10
#define     kInitSizeVector         60
#define     kFastBackup             75
#define     kSlowBackup             85
#define     kTresLDKFl              10
#define     kTresLDKFq              35
#define     kUseDKF                 0x01

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
    unsigned short      CPalette[4];            /* Colour palette of 4 colours */
    unsigned int       PMatrix;                /* kXRes*kYRes of literal pixel in 2 bits */
} LowLiteralMatrix;

typedef struct
{
    unsigned short      PMatrix[kXResYRes];     /* kXRes*kYRes of literal pixel in 16 bits */
} HighLiteralMatrix;

typedef struct
{
    unsigned int       PMatrix[kXResYRes];     /* kXRes*kYRes of literal pixel */
} LiteralMatrix;

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
    DLiteral*           TheHighLiteral;         /* The dictionnary of literals of 16 colours */
    FrequencyRec*       HLFrequency;            /* The frequency record for the literals of 16 colours */
    DLiteral*           TheLowLiteral;          /* The dictionnary of low res literals */
    FrequencyRec*       LLFrequency;            /* The frequency record for the low res literals */
} T16Enviro;

/***********************************************************************************/
/*                        General structure for this Gimex                         */
/***********************************************************************************/

typedef struct T16GimexData
{
    int                 fWidth;                 /* To control the size */
    int                 fHeight;
    char *              fPrevSource;            /* Build using fWidth and fHeight */
    unsigned int       *fError;                /* To write only */
    unsigned int       *fDKF;                  /* To write only */
    unsigned int       *fTDKF;                 /* To write only */
    int                fRealSizeRate;
    int                fLogicSizeRate;
    int                fSizeKeyFrame;
    int                fNumKeyFrame;
} T16GimexData;


/***********************************************************************************/
/*                 Type read and write (intel/motorolat)                           */
/***********************************************************************************/

#define T16GETFIELD(field) ggeti((void *)&field, sizeof(field))
#define T16PUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifndef GIMEXIDLE
#define GIMEXIDLE
#endif

#endif
