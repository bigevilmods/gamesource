// (c) 1994-2004 Electronic Arts Inc.

#ifndef __GOPENH
#define __GOPENH 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GOPEN_VERSION 107

bool GCALL GIMEX_init(void);
void GCALL GIMEX_restore(void);

GABOUT *GCALL GIMEX_about(void);
int     GCALL GIMEX_is(GSTREAM *g);
int     GCALL GIMEX_open(GINSTANCE **gx, GSTREAM *g, const char *pathname, bool framecountflag);
GINFO * GCALL GIMEX_info(GINSTANCE *gx, int framenum);
bool    GCALL GIMEX_read(GINSTANCE *gx, const GINFO *ginfo, void *dest, int rowbytes);
bool    GCALL GIMEX_close(GINSTANCE *gx);
bool    GCALL GIMEX_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
bool    GCALL GIMEX_write(GINSTANCE *gx, const GINFO *ginfo, const void *source, int rowbytes);
bool    GCALL GIMEX_wclose(GINSTANCE *gx);

bool      GCALL GIMEX_gopenstream(const char *pathname, GSTREAM *g); /* open existing stream */
GSTREAM * GCALL GIMEX_gopen(const char *pathname, char *returnname); /* open a file */
void      GCALL GIMEX_set(int typenum);                        /* choose which gimex (GIMEX_find) */
int       GCALL GIMEX_get(void);
int       GCALL GIMEX_find(const char *name, int *subtype);         /* get best type from the name ("bmp") */
int       GCALL GIMEX_count(void);

void     GCALL GIMEX_freebitmap(GBITMAP *gm);
GBITMAP * GCALL GIMEX_loadframe(const char *filename,int framenum);

struct GIMEXABOUT;
extern struct GIMEXABOUT *GIMEXabout;
struct TTFINFO;
extern struct TTFINFO *TTFinfo;

#ifdef __cplusplus
}
#endif
#endif

