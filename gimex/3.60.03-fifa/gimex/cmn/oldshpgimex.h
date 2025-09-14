// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  OLDSHP Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL OLDSHP_about(void);
int      GCALL OLDSHP_is(GSTREAM *g);

/* Import Functions */

int      GCALL OLDSHP_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL OLDSHP_info(GINSTANCE *gx, int framenum);
int      GCALL OLDSHP_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL OLDSHP_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL OLDSHP_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL OLDSHP_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL OLDSHP_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef __OLDSHPGIMEX
#define __OLDSHPGIMEX 1


#define OLDSHP_VERSION 359

#define SHAPEFILE 0
#define STREAMINOLDGSHAPE 1




#include <string.h>

/* Colour Oldshp */

typedef struct IteoLdmshapeTbl
{
    char            IDShape[4];         /*  The ID for this item */
    int            OffsetShape;        /*  The Offset for this item */

} IteoLdmshapeTbl;

typedef struct OLDShapeTbl
{
    IteoLdmshapeTbl    ShapeTable[1];      /*  The table that describe the file */

} OLDShapeTbl;

typedef struct OLDSHPFILEHeaderStruct
{
    /* OLDSHP SHAPE FILE HEADER structure 16 bytes */

    char typeFile[4];        /*  'OLDSHPS' for Sony */
    char LengthFile[4];      /*  The length of the file in byte */
    char NuoLdmshape[4];        /*  The Number of Shape in this file */
    char Creator[4];         /*  The creator */

} OLDSHPFILEHeaderStruct;

/* shape */

typedef struct OLDSHPHeaderStruct
{
    char type;               /*  The type for this shape */
    char LinkShape[3];       /*  The Link to the next shape */
    char width[2];           /*  width of the Shape */
    char height[2];          /*  height of the Shape */
    char centerx[2];         /*  center point of the Shape */
    char centery[2];         /*  center point of the Shape */
    char shapex[2];          /*  position of the Shape */
    char shapey[2];          /*  position of the Shape */

} OLDSHPHeaderStruct;

/* clut */

typedef struct OLDClutHeaderStruct
{
    char type;               /*  The type for this shape */
    char LinkShape[3];       /*  The Link to the next shape */
    char width[2];           /*  width of the Shape */
    char height[2];          /*  height of the Shape */
    char centerx[2];         /*  center point of the Shape */
    char centery[2];         /*  center point of the Shape */
    char shapex[2];          /*  position of the Shape */
    char shapey[2];          /*  position of the Shape */
    char Entries[1][2];      /*  The colours */
} OLDClutHeaderStruct;

/* hot spot */

typedef struct OLDHOTHeaderStruct
{
    char type;               /*  The type for this shape */
    char Link[3];            /*  The Link to the next shape */
    char NumHotSpot[4];      /*  The number of HotSpot */
    int  ListHotSpot[1];     /*  The Hot Spot list */

} OLDHOTHeaderStruct;

/* anim */

typedef struct OLDAnimHeaderStruct
{
    char type;               /*  The type for this shape */
    char Link[3];            /*  The Link to the next shape */
    char NumAnim[4];         /*  The number of Anim ranges */
    int  ListAnim[1];        /*  The Anim list */

} OLDAnimHeaderStruct;


typedef struct OldcommEntheaderStruct
{
    char type;               /*  The type for this shape */
    char Link[3];            /*  The Link to the next shape */
    char SizeComment[4];     /*  The size in bytes of this comment following */
    int  ListComment[1];     /*  The Comment list */

} OLDCOMMEntheaderStruct;

typedef struct OldnameheaderStruct
{
    char type;               /*  The type for this shape */
    char Link[3];            /*  The Link to the next shape */
    int  ListName[1];        /*  The Name null terminated */

} OLDnameheaderStruct;

typedef struct OLDTpageHeaderStruct
{
    char type;               /*  The type for this shape */
    char Link[3];            /*  The Link to the next shape */
    char tpage[2];
    char clutid[2];
    char code[2];
    char reserved[2];

} OLDTpageHeaderStruct;

typedef struct OLDMetalHeaderStruct
{
    char type;               /*  The type for this shape */
    char Link[3];            /*  The Link to the next shape */
    char metalbinsize[2];    /*  size of metalbin area in bytes (not including this header) */
    char metal[2];           /*  flags (dot3=1,embm=2,cubemap=4,mipmont=8,metalbin=16) */
    char reserved2[8];       /*  pad header to 16, followed by metalbin */

} OLDMetalHeaderStruct;



/* internal shape list */

typedef struct
{
    char	shape_ID[4];        /* The signature of this shape */
    char *	shape_Data;         /* The basic shape found on the file */
    int		shape_SizeData;     /* The size of this shape (write) */
    char	shape_ClutID[4];    /* The signature of this clut */
    char *	shape_Clut;         /* The Clut link to this shape */
    int		shape_SizeClut;     /* The size of this Clut (write) */
    char *	shape_HotSpot;      /* The HotSpot link to this shape */
    int		shape_SizeHotSpot;  /* The size of this HotSpot (write) */
    char *	shape_Comment;      /* The Comment link to this shape */
    int		shape_SizeComment;  /* The size of this Comment (write) */
    char *	shape_Name;         /* The Name link to this shape */
    int		shape_SizeName;     /* The size of this Name (write) */
    char *	shape_Anim;         /* The Anim link to this shape */
    int		shape_SizeAnim;     /* The size of this Anim (write) */
    char *	shape_Tpage;        /* The Tpage link to this shape */
    int		shape_SizeTpage;    /* The size of this Tpage (write) */
    char *	shape_Metal;        /* The Metal link to this shape */
    int		shape_SizeMetal;    /* The size of this Metal (write) */
    int		subtype;            /* Subtype for shape */
    int		packed;             /* Packed method (0=none,2=refpack,3=dxt,4=mpeg) (read) */
    int		framesize;          /* Total data size (read) */
    int		align;              /* Alignment to write shape data to (write) */
} OLDgshapeInfo;

typedef struct
{
    int        NuoLdmshape;           /* The Number of Shape in this file */
    OLDgshapeInfo ListShape[1];       /* The list of shape for this composite file */
} OLDShapeList, *OLDShapeListPtr;

/* shared with LBM */

#ifndef MAXPALANIMS

#define MAXPALANIMS 32

typedef struct
{
    int dir;                /* animation direction -1 or 1*/
    int speed;              /* animation speed in hz start*/
    int start;              /* start colour */
    int end;                /* end colour */
} PALANIM;

// NOTE GINFOSHP used by SHPGIMEX.h as well

typedef struct
{
    GINFO   ginfo;              /* standard ginfo */
    int     align;              /* 4 char code of creator (app/version ie 'G231') */
	int     creator;            /* 4 char code of creator (app/version ie 'G231') */
	int		binsize;			/* binary data size */

    int     clutx;              /* clut position x */
    int     cluty;              /* clut position y */
    int     tpage;
    char    clutid;
    char    code;
    char    pad[2];             /* pad to 4 bytes */

    int     numanims;           /* number of palette anim ranges */
    PALANIM palanim[MAXPALANIMS]; /* palette animation ranges */

} GINFOOLDSHP;


typedef struct yumoldcoorddef
{
    int x;
    int y;
    int z;
} YUMOLDCOORD;


#include "oldshpcmn.h"

#endif

#define CleanField(Field,type)  Field=ggeti((void*)&(Field),sizeof(type))
#define SetIField(Field,type)   gputi((void*)&(Field),(int)Field,sizeof(type))

#endif

/* cleanup macros */

#ifndef OLDSHPMAKECODE

#undef OLDSHP_about
#undef OLDSHP_is
#undef OLDSHP_open
#undef OLDSHP_info
#undef OLDSHP_read
#undef OLDSHP_close
#undef OLDSHP_wopen
#undef OLDSHP_write
#undef OLDSHP_wclose

#undef OLDSHPSIGNATURE
#undef OLDFNTSIGNATURE
#undef OLDSHPMULTIFRAME
#undef OLDSHPEXTENSION0
#undef OLDSHPEXTENSION1
#undef OLDSHPEXTENSION2
#undef OLDSHPEXTENSION3
#undef OLDSHPMACTYPE0
#undef OLDSHPMACTYPE1
#undef OLDSHPMACTYPE2
#undef OLDSHPMACTYPE3
#undef OLDSHPSHORTTYPE
#undef OLDSHPWORDTYPE
#undef OLDSHPLONGTYPE
#undef OLDSHPDATAALIGNMENT
#undef OLDSHPMIPMAPDATAALIGNMENT
#undef OLDSHPCLUTDATAALIGNMENT
#undef OLDSHPPACKEDDATAALIGNMENT

#undef OLDggetx
#undef OLDgputx
#undef OLDSHPGETFIELD
#undef OLDSHPPUTFIELD

#endif

#ifdef __cplusplus
}
#endif

