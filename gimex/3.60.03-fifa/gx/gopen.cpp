// (c) 1994-2004 Electronic Arts Inc.

/*
; ABSTRACT
;
; gopen.cpp - Header file defining the GIMEX DLL interface.
;
; SOURCE
;@
*/

/*------------------------------------------------------------------*/
/*                                                                  */
/*                   Gimex DLL Open Interface v1.07                 */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                     Code Module - Jul 29, 2002                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.03 xxxxxx FB C++ extern "C"                                 */
/*    1.04 011106 FB GIMEXabout in header                           */
/*    1.05 020702 FB TTFinfo                                        */
/*    1.06 020718 FB GIMEX_wopen check frame count and return bool  */
/*    1.07 020729 FB GIMEX_wopen allow any non zero return for ok   */
/*                                                                  */
/*------------------------------------------------------------------*/

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"
#include <stdlib.h>
#include "gimex.h"
#include "gopen.h"
#include <stdio.h>

//#pragma optimize("",off)

typedef struct
{
    GSTREAM * (GCALL *gopen)(const char *pathname);
    GSTREAM * (GCALL *gwopen)(const char *pathname);
    bool     (GCALL *gclose)(GSTREAM *g);
    int      (GCALL *gread)(GSTREAM *g, void *buf, int size);
    int      (GCALL *gwrite)(GSTREAM *g, const void *buf, int size);
    bool     (GCALL *gseek)(GSTREAM *g, GPOS offset);
    GPOS     (GCALL *glen)(GSTREAM *g);
    GPOS     (GCALL *gtell)(GSTREAM *g);
    void   * (GCALL *galloc)(int size);
    bool     (GCALL *gfree)(void *memptr);

} GIMEXCALLBACKS;

void GCALL GIMEX_installcallbacks(GIMEXCALLBACKS *callbacks);

struct GIMEXABOUT;
typedef struct GIMEXABOUT GIMEXABOUT;
GIMEXABOUT *GIMEXabout=0;

struct TTFINFO;
typedef struct TTFINFO TTFINFO;
TTFINFO *TTFinfo=0;


/* Function pointers for to the proper gimex functions */
typedef struct
{
    bool        (GCALL *GIMEX_init)(void);
    void        (GCALL *GIMEX_restore)(void);
    GABOUT *    (GCALL *GIMEX_about)(void);
    int         (GCALL *GIMEX_is)(GSTREAM *g);
    int         (GCALL *GIMEX_open)(GINSTANCE **gx, GSTREAM *g, const char *pathname, bool framecountflag);
    GINFO *     (GCALL *GIMEX_info)(GINSTANCE *gx, int framenum);
    bool        (GCALL *GIMEX_read)(GINSTANCE *gx, const GINFO *ginfo, void *dest, int rowbytes);
    bool        (GCALL *GIMEX_close)(GINSTANCE *gx);
    int         (GCALL *GIMEX_wopen)(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
    bool        (GCALL *GIMEX_write)(GINSTANCE *gx, const GINFO *ginfo, const void *source, int rowbytes);
    bool        (GCALL *GIMEX_wclose)(GINSTANCE *gx);
    bool        (GCALL *GIMEX_gopenstream)(const char *pathname, GSTREAM *g);
    GSTREAM *   (GCALL *GIMEX_gopen)(const char *pathname, char *returnname);
    void        (GCALL *GIMEX_set)(int typenum);
    int         (GCALL *GIMEX_get)(void);
    int         (GCALL *GIMEX_find)(const char *name, int *subtype);
    int         (GCALL *GIMEX_count)(void);
    void        (GCALL *GIMEX_freebitmap)(GBITMAP *gm);
    GBITMAP *   (GCALL *GIMEX_loadframe)(const char *filename,int framenum);
    void        (GCALL *GIMEX_installcallbacks)(GIMEXCALLBACKS *callbacks);
    int         (GCALL *OLDGIMEX_open)(GINSTANCE **gx, GSTREAM *g, const char *pathname);

} GIMEXROUTINES;

/* Pointers to gimex routines in the library */
static GIMEXROUTINES GimexFuncs;

/* Handle of dynamic library */
static HINSTANCE     HLib;

#define setfpointer(fpointer, procaddress) *(void (**)(void))&fpointer = (void (*)(void)) procaddress

/* gimex dll name */
#ifdef _DEBUG
#define GIMEXDLLFILENAME "gimex"
#else
#define GIMEXDLLFILENAME "gimex"
#endif
char *getgimexlibraryname(void)
{
    char *forcedriver;
    char *name=GIMEXDLLFILENAME;

	forcedriver = getenv("GIMEX_DRIVERNAME");

	if (forcedriver)
	    name = forcedriver;

    return(name);
}

/*
 * Load the library, setting the library routined in funcs.
 */

static HINSTANCE loadgimexlibrary(GIMEXROUTINES *funcs)
{
    HINSTANCE hlib;

    hlib = LoadLibrary(getgimexlibraryname());
    if (hlib != NULL)
    {
        setfpointer(funcs->GIMEX_init,                GetProcAddress(hlib, "_GIMEX_init@0"));
        setfpointer(funcs->GIMEX_restore,             GetProcAddress(hlib, "_GIMEX_restore@0"));
        setfpointer(funcs->GIMEX_about,               GetProcAddress(hlib, "_GIMEX_about@0"));
        setfpointer(funcs->GIMEX_is,                  GetProcAddress(hlib, "_GIMEX_is@4"));
        setfpointer(funcs->GIMEX_open,                GetProcAddress(hlib, "_GIMEX_open@16"));
        setfpointer(funcs->GIMEX_info,                GetProcAddress(hlib, "_GIMEX_info@8"));
        setfpointer(funcs->GIMEX_read,                GetProcAddress(hlib, "_GIMEX_read@16"));
        setfpointer(funcs->GIMEX_close,               GetProcAddress(hlib, "_GIMEX_close@4"));
        setfpointer(funcs->GIMEX_wopen,               GetProcAddress(hlib, "_GIMEX_wopen@16"));
        setfpointer(funcs->GIMEX_write,               GetProcAddress(hlib, "_GIMEX_write@16"));
        setfpointer(funcs->GIMEX_wclose,              GetProcAddress(hlib, "_GIMEX_wclose@4"));
        setfpointer(funcs->GIMEX_gopenstream,         GetProcAddress(hlib, "_GIMEX_gopenstream@8"));
        setfpointer(funcs->GIMEX_gopen,               GetProcAddress(hlib, "_GIMEX_gopen@8"));
        setfpointer(funcs->GIMEX_set,                 GetProcAddress(hlib, "_GIMEX_set@4"));
        setfpointer(funcs->GIMEX_get,                 GetProcAddress(hlib, "_GIMEX_get@0"));
        setfpointer(funcs->GIMEX_find,                GetProcAddress(hlib, "_GIMEX_find@8"));
        setfpointer(funcs->GIMEX_count,               GetProcAddress(hlib, "_GIMEX_count@0"));
        setfpointer(funcs->GIMEX_freebitmap,          GetProcAddress(hlib, "_GIMEX_freebitmap@4"));
        setfpointer(funcs->GIMEX_loadframe,           GetProcAddress(hlib, "_GIMEX_loadframe@8"));
        setfpointer(funcs->GIMEX_installcallbacks,    GetProcAddress(hlib, "_GIMEX_installcallbacks64@4"));
        setfpointer(funcs->OLDGIMEX_open,             GetProcAddress(hlib, "_GIMEX_open@12"));
        setfpointer(GIMEXabout,                       GetProcAddress(hlib, "GIMEXabout"));
        setfpointer(TTFinfo,                          GetProcAddress(hlib, "TTFinfo"));
    }

    return(hlib);
}


/*
 * Free the library.
 */
static int freegimexlibrary(HINSTANCE hlib)
{
    return((int)FreeLibrary(hlib));
}


/*
 * Called before using gimex
 */
bool GCALL GIMEX_init(void)
{
    GIMEXCALLBACKS callbacks;
    bool ok=false;

    HLib = loadgimexlibrary(&GimexFuncs);

    if (HLib)
    {
        callbacks.gopen = gopen;
        callbacks.gclose = gclose;
        callbacks.gwopen = gwopen;
        callbacks.gread = gread;
        callbacks.gwrite = gwrite;
        callbacks.gseek = gseek;
        callbacks.gtell = gtell;
        callbacks.glen = glen;
        callbacks.galloc = galloc;
        callbacks.gfree= gfree;

        GIMEX_installcallbacks(&callbacks);

        if (GimexFuncs.GIMEX_init)
            ok = GimexFuncs.GIMEX_init();
    }

    return(ok);
}


/*
 * Called when done using gimex
 */
void GCALL GIMEX_restore(void)
{
    if (GimexFuncs.GIMEX_restore)
        GimexFuncs.GIMEX_restore();

    freegimexlibrary(HLib);
}


/********************************************
 * Public functions map to functions in DLL *
 ********************************************/

GABOUT * GCALL GIMEX_about(void)
{
    if (GimexFuncs.GIMEX_about)
        return(GimexFuncs.GIMEX_about());

    return((GABOUT*)0);
}

int GCALL GIMEX_is(GSTREAM *g)
{
    if (GimexFuncs.GIMEX_is)
        return(GimexFuncs.GIMEX_is(g));

    return 0;
}

int GCALL GIMEX_open(GINSTANCE **gx, GSTREAM *g, const char *pathname, bool framecountflag)
{
    if (GimexFuncs.GIMEX_open)
        return(GimexFuncs.GIMEX_open(gx, g, pathname,framecountflag));
    else if (GimexFuncs.OLDGIMEX_open)
        return(GimexFuncs.OLDGIMEX_open(gx, g, pathname));

    return 0;
}

GINFO * GCALL GIMEX_info(GINSTANCE *gx, int framenum)
{
    if (GimexFuncs.GIMEX_info)
        return(GimexFuncs.GIMEX_info(gx, framenum));

    return((GINFO*)0);
}

bool GCALL GIMEX_read(GINSTANCE *gx, const GINFO *ginfo, void *dest, int rowbytes)
{
    if (GimexFuncs.GIMEX_read)
        return(GimexFuncs.GIMEX_read(gx, ginfo, dest, rowbytes));

    return false;
}

bool GCALL GIMEX_close(GINSTANCE *gx)
{
    if (GimexFuncs.GIMEX_close)
        return(GimexFuncs.GIMEX_close(gx));

    return false;
}

bool GCALL GIMEX_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes)
{
    int ok=0;
    if (GimexFuncs.GIMEX_wopen)
        ok = GimexFuncs.GIMEX_wopen(gx, g, pathname, numframes);

    return ok?true:false;
}

bool GCALL GIMEX_write(GINSTANCE *gx, const GINFO *ginfo, const void *source, int rowbytes)
{
    if (GimexFuncs.GIMEX_write)
        return(GimexFuncs.GIMEX_write(gx, ginfo, source, rowbytes));

    return false;
}

bool GCALL GIMEX_wclose(GINSTANCE *gx)
{
    if (GimexFuncs.GIMEX_wclose)
        return(GimexFuncs.GIMEX_wclose(gx));

    return false;
}

bool GCALL GIMEX_gopenstream(const char *pathname, GSTREAM *g)
{
    if (GimexFuncs.GIMEX_gopenstream)
        return(GimexFuncs.GIMEX_gopenstream(pathname, g));

    return false;
}

GSTREAM * GCALL GIMEX_gopen(const char *pathname, char *returnname)
{
    if (GimexFuncs.GIMEX_gopen)
        return(GimexFuncs.GIMEX_gopen(pathname, returnname));

    return false;
}

void GCALL GIMEX_set(int typenum)
{
    if (GimexFuncs.GIMEX_set)
        GimexFuncs.GIMEX_set(typenum);
}

int GCALL GIMEX_get(void)
{
    if (GimexFuncs.GIMEX_get)
        return(GimexFuncs.GIMEX_get());

    return 0;
}

int GCALL GIMEX_find(const char *name, int *subtype)
{
    if (GimexFuncs.GIMEX_find)
        return(GimexFuncs.GIMEX_find(name, subtype));

    return 0;
}

int GCALL GIMEX_count(void)
{
    if (GimexFuncs.GIMEX_count)
        return(GimexFuncs.GIMEX_count());

    return 0;
}

void GCALL GIMEX_freebitmap(GBITMAP *gm)
{
    if (GimexFuncs.GIMEX_freebitmap)
        GimexFuncs.GIMEX_freebitmap(gm);
}

GBITMAP * GCALL GIMEX_loadframe(const char *filename,int framenum)
{
    if (GimexFuncs.GIMEX_loadframe)
        return(GimexFuncs.GIMEX_loadframe(filename, framenum));

    return((GBITMAP*)0);
}

void GCALL GIMEX_installcallbacks(GIMEXCALLBACKS *callbacks)
{
    if (GimexFuncs.GIMEX_installcallbacks)
        GimexFuncs.GIMEX_installcallbacks(callbacks);
    else
        printf("error - old gimex dll!\n");
}

/* END ABSTRACT */
