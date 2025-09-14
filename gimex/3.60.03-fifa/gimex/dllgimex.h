// (c) 1994-2007 Electronic Arts Inc.

#ifndef __DLLGIMEX_H
#define __DLLGIMEX_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    GSTREAM * (GCALL *gopen)(const char *pathname);
    GSTREAM * (GCALL *gwopen)(const char *pathname);
    bool     (GCALL *gclose)(GSTREAM *g);
    int      (GCALL *gread)(GSTREAM *g, void *buf, int size);
    int      (GCALL *gwrite)(GSTREAM *g, const void *buf, int size);
    bool     (GCALL *gseek)(GSTREAM *g, GPOS seekpos);
    GPOS     (GCALL *glen)(GSTREAM *g);
    GPOS     (GCALL *gtell)(GSTREAM *g);
    void   * (GCALL *galloc)(int size);
    bool     (GCALL *gfree)(void *memptr);

} GIMEXCALLBACKS;

void GCALL GIMEX_installcallbacks64(GIMEXCALLBACKS *callbacks);


/* The following is for backwards compatibility, 32 bit seekposs also supported */

typedef struct
{
    GSTREAM * (GCALL *gopen)(const char *pathname);
    GSTREAM * (GCALL *gwopen)(const char *pathname);
    bool     (GCALL *gclose)(GSTREAM *g);
    int      (GCALL *gread)(GSTREAM *g, void *buf, int size);
    int      (GCALL *gwrite)(GSTREAM *g, const void *buf, int size);
    bool     (GCALL *gseek)(GSTREAM *g, int seekpos);
    int      (GCALL *glen)(GSTREAM *g);
    int      (GCALL *gtell)(GSTREAM *g);
    void   * (GCALL *galloc)(int size);
    bool     (GCALL *gfree)(void *memptr);

} OLDGIMEXCALLBACKS;

void GCALL GIMEX_installcallbacks(OLDGIMEXCALLBACKS *callbacks);


#ifdef __cplusplus
}
#endif
#endif

