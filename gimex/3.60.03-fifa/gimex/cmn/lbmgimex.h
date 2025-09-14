// (c) 1994-2007 Electronic Arts Inc.

#ifndef __LBMGIMEX
#define __LBMGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  ILBM Gimex                                                  */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL LBM_about(void);
int      GCALL LBM_is(GSTREAM *g);

/* Import Functions */

int      GCALL LBM_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL LBM_info(GINSTANCE *gx, int framenum);
int      GCALL LBM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL LBM_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL LBM_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL LBM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL LBM_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#define MAXROWBYTES 32*1024

#define RUN             1
#define DUMP            0

// standard lbm,pbm

#define CMAP            GMAKEID('C','M','A','P')
#define ILBM            GMAKEID('I','L','B','M')
#define PBM             GMAKEID('P','B','M',' ')
#define BMHD            GMAKEID('B','M','H','D')
#define BODY            GMAKEID('B','O','D','Y')
#define FORM            GMAKEID('F','O','R','M')
#define CRNG            GMAKEID('C','R','N','G')
#define ANNO            GMAKEID('A','N','N','O')
#define GRAB            GMAKEID('G','R','A','B')

// maya iff

#define FOR4            GMAKEID('F','O','R','4')
#define CIMG            GMAKEID('C','I','M','G')
#define FVER            GMAKEID('F','V','E','R')
#define TBHD            GMAKEID('T','B','H','D')
#define RGBA            GMAKEID('R','G','B','A')

#define MINRUN          3
#define MAXRUN          128
#define MAXDAT          128

#define GByte()         (*source++)
#define PByte(c)        {*dest++ = (c); ++(*putSize);}
#define OutDump(nn)     dest = PutDump(dest, nn, &putSize, buf)
#define OutRun(nn, cc)  dest = PutRun(dest, nn, cc, &putSize)

#define GETBHDR         0
#define DECODEILBM      1

/* shared with SHP */

#ifndef MAXPALANIMS

#define MAXPALANIMS 32

typedef struct
{
    int             dir;                /* animation direction -1 or 1*/
    int             speed;              /* animation speed in hz start*/
    int             start;              /* start colour */
    int             end;                /* end colour */
} PALANIM;

typedef struct
{
    GINFO   ginfo;              /* standard ginfo */
    int     align;              /* 4 char code of creator (app/version ie 'G231') */
    int     creator;            /* 4 char code of creator (app/version ie 'G231') */

    int     clutx;              /* clut position x */
    int     cluty;              /* clut position y */
    int     tpage;
    char    clutid;
    char    code;
    char    pad[2];             /* pad to 4 bytes */

    int     numanims;           /* number of palette anim ranges */
    PALANIM palanim[MAXPALANIMS]; /* palette animation ranges */

} GINFOOLDSHP;


#endif

typedef struct
{
    /* ILBM HEADER structure */

    char w[2];                  /* image width in pixels */
    char h[2];                  /* image height in pixels */
    char x[2];                  /* image x origin */
    char y[2];                  /* image y origin */
    char nPlanes[1];            /* number of bit planes 1..9,24 */
    char masking[1];            /* masking flag */
    char compression[1];        /* compression 0=none, 1=RLE */
    char pad1[1];               /* pad to int */
    char transparentColour[2];  /* transparent rgb value */
    char xAspect[1];            /* horizontal aspect ratio */
    char yAspect[1];            /* vertical aspect ratio */
    char pageWidth[2];          /* Original page width in pixels */
    char pageHeight[2];         /* Original page height in pixels */

} LBMHeaderStruct;

typedef struct
{
    /* CIMG HEADER structure */

    char w[4];                  /* image width in pixels */
    char h[4];                  /* image height in pixels */
    char pad1[4];
    char flags[4];
    char pad2[2];
    char tiles[2];				/* number of tiles in iff image */
    char compression[4];        /* are tiles rle compressed */
    //char pad[6*4];              /* unknown 00010001 00000003 00000036 0 0 0 */
} CIMGHeaderStruct;

#define LBMGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define LBMPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif



