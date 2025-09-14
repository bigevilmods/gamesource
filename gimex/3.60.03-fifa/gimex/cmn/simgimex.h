// (c) 1994-2007 Electronic Arts Inc.

#ifndef __SIMGIMEX
#define __SIMGIMEX 1

#include "gimex.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Soft Image Gimex                                            */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL SIM_about(void);
int      GCALL SIM_is(GSTREAM *g);

/* Import Functions */

int      GCALL SIM_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL SIM_info(GINSTANCE *gx, int framenum);
int      GCALL SIM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL SIM_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL SIM_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL SIM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL SIM_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef int                    DK_Boolean;

#ifndef FALSE
#define FALSE       0
#endif  /* ifndef FALSE */

#ifndef TRUE
#define TRUE        1
#endif  /* ifndef TRUE */

#ifndef NULL
#define NULL        0L
#endif  /* ifndef NULL */

#define DK_ASCII_GSTREAM_HEADER "Softimage 4D Creative Environment"
#define DK_BINARY_MAGIC_NUMBER  0x5380f634   /* 'S' + HEX( 845 1636 ) */

/* Header found in every SOFTIMAGE binary file */

#if 0
typedef struct
{
    char    ID[4];
    char    version[4];   /* float     version;  version of format (= Softimage release) */
    char    comment[80];
} DK_BinaryHeader;
#endif

/* value for the fields field */
#define  DK_NO_FIELD    0
#define  DK_ODD_FIELD   1
#define  DK_EVEN_FIELD  2
#define  DK_FULL_FRAME  3

typedef struct DK_Picture
{
    /* file header */

    char         ID[4];         /* */
    char         version[4];    /* float     version;    version of format (= Softimage release) */
    char         comment[80];
    char         type[4];       /* 'PICT' */
    char         width[2];
    char         height[2];
    char         ratio[4];      /* sgi float */
    char         fields[2];     /* ntsc-style fields (non, odd, even, full) */
    char         pad[2];

    unsigned char   *red;
    unsigned char   *green;
    unsigned char   *blue;
    unsigned char   *alpha;
    unsigned char   *shadow;    /* for future expansion */
    float       *depth;     /* for future expansion */
    unsigned char   *auxiliary1;    /* for future expansion */
    unsigned char   *auxiliary2;    /* for future expansion */

    /* DK_Pointer to current scanline (used by to read and write scanline) */
    unsigned char   *ptrRed;
    unsigned char   *ptrGreen;
    unsigned char   *ptrBlue;
    unsigned char   *ptrAlpha;
    unsigned char   *ptrShadow; /* for future expansion */
    float       *ptrDepth;  /* for future expansion */
    unsigned char   *ptrAuxiliary1; /* for future expansion */
    unsigned char   *ptrAuxiliary2; /* for future expansion */
} DK_Picture;

/* 1st nibble of type fields */
#define DK_UNSIGNED_INTEGER 0x00
#define DK_SIGNED_INTEGER   0x10    /* for future expansion */
#define DK_SIGNED_FLOAT     0x20    /* for future expansion */

/* 2nd nibble of type fields */
#define DK_UNCOMPRESSED     0x00
#define DK_PURE_RUN_LENGTH  0x01
#define DK_MIXED_RUN_LENGTH 0x02

/* value for channels fields (can be ORed) */
#define DK_RED_CHANNEL      0x80
#define DK_GREEN_CHANNEL    0x40
#define DK_BLUE_CHANNEL     0x20
#define DK_ALPHA_CHANNEL    0x10
#define DK_SHADOW_CHANNEL   0x08    /* for future expansion */
#define DK_DEPTH_CHANNEL    0x04    /* for future expansion */
#define DK_AUXILIARY_1_CHANNEL  0x02    /* for future expansion */
#define DK_AUXILIARY_2_CHANNEL  0x01    /* for future expansion */

/* structure describing the way it is code in file... */

typedef struct DK_channelPacket
{
    char             size;      /* nb of bits by channel (8) */
    char             type;
    char             channels;
    struct DK_channelPacket *next;
} DK_channelPacket;

/* Global DK_Picture prototypes */

#define SIMGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define SIMPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

