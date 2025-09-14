// (c) 1994-2007 Electronic Arts Inc.

#ifndef __QMTGIMEX
#define __QMTGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  QMT Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL QMT_about(void);
int      GCALL QMT_is(GSTREAM *g);

/* Import Functions */

int      GCALL QMT_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL QMT_info(GINSTANCE *gx, int framenum);
int      GCALL QMT_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL QMT_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL QMT_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL QMT_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL QMT_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

typedef struct                  /* Vector or Position */
{
    float x;                    /* horizontal */
    float y;                    /* horizontal */
    float z;                    /* vertical */
} vec3_t;

typedef struct
{
    int id;                    /* 0x4F504449 = "IDPO" for IDPOLYGON */
    int version;               /* Version = 6 */
    vec3_t scale;               /* Model scale factors. */
    vec3_t origin;              /* Model origin. */
    float radius;               /* Model bounding radius. */
    vec3_t offsets;             /* Eye position (useless?) */
    int numskins ;             /* the number of skin textures */
    int skinwidth;             /* Width of skin texture, must be multiple of 8 */
    int skinheight;            /* Height of skin texture, must be multiple of 8 */
    int numverts;              /* Number of vertices */
    int numtris;               /* Number of triangles surfaces */
    int numframes;             /* Number of frames */
    int synctype;              /* 0= synchron, 1= random */
    int flags;                 /* 0 (see Alias models) */
    float size;                 /* average size of triangles */
} mdl_t;

#define QMTSIGNATURE                    GMAKEID(0, 'M', 'D', 'L');
#define QMTPUTFIELD(field, data)        gputi((void *) &field, (unsigned int) (data), sizeof(field))
#define QMTGETFIELD(field)              ggeti((void *) &field, sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

