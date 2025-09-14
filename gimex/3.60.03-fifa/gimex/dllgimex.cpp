// (c) 1994-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*            Gimex DLL File and Memory Interface v1.03             */
/*                                                                  */
/*                       by Kaz Okuda, EAC                          */
/*                                                                  */
/*                     Code Module - Nov 20, 2000                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 970827 KO based on stdgimex.c                            */
/*    1.01 970828 KO added GCALL                                    */
/*    1.02 980309 KO gimexinstallcallbacks->GIMEX_installcallbacks  */
/*    1.03 001120 FB GIMEX_installcallbacks64                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Applications using the dll must install new io/mem functions     */
/* Using gimexinstallcallbacks().                                   */
/* gimexinstallcallbacks is called by initgimex() so not needed if  */
/* initgimex is used                                                */
/*                                                                  */
/*------------------------------------------------------------------*/

#define __NOINLINE__ 1
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "..\include\gimex.h"
#include "dllgimex.h"

/*
 * Include stdgimex.cpp, redefining the declarations in it to the initial callback
 * initializers
 */
#define gopen defaultgopen
#define gwopen defaultgwopen
#define gclose defaultgclose
#define gread defaultgread
#define gwrite defaultgwrite
#define gseek defaultgseek
#define glen defaultglen
#define gtell defaultgtell
#define galloc defaultgalloc
#define gfree defaultgfree

#include "stdgimex.cpp"

#undef gopen
#undef gwopen
#undef gclose
#undef gread
#undef gwrite
#undef gseek
#undef glen
#undef gtell
#undef galloc
#undef gfree


static GIMEXCALLBACKS gimexcallbacks =
{
    defaultgopen,
    defaultgwopen,
    defaultgclose,
    defaultgread,
    defaultgwrite,
    defaultgseek,
    defaultglen,
    defaultgtell,
    defaultgalloc,
    defaultgfree
};

// glue default code
bool GCALL olddefaultgseek(GSTREAM *g, int seekpos)
{
    return(defaultgseek(g,(GPOS) seekpos));
}

int GCALL olddefaultglen(GSTREAM *g)
{
    return((int) defaultglen(g));
}

int GCALL olddefaultgtell(GSTREAM *g)
{
    return((int) defaultgtell(g));
}

// glue structure

static OLDGIMEXCALLBACKS oldgimexcallbacks =
{
    defaultgopen,
    defaultgwopen,
    defaultgclose,
    defaultgread,
    defaultgwrite,
    olddefaultgseek,
    olddefaultglen,
    olddefaultgtell,
    defaultgalloc,
    defaultgfree
};

// glue vectors
bool GCALL gluegseek(GSTREAM *g, GPOS seekpos)
{
    return(oldgimexcallbacks.gseek(g,(int)seekpos));
}

GPOS GCALL glueglen(GSTREAM *g)
{
    return((GPOS)(oldgimexcallbacks.glen(g)));
}

GPOS GCALL gluegtell(GSTREAM *g)
{
    return((GPOS)(oldgimexcallbacks.gtell(g)));
}


// glue entry point
void GCALL GIMEX_installcallbacks(OLDGIMEXCALLBACKS *callbacks)
{
    memcpy(&oldgimexcallbacks, callbacks, sizeof(OLDGIMEXCALLBACKS));
    memcpy(&gimexcallbacks, callbacks, sizeof(GIMEXCALLBACKS)); // same except the following 3 functions
    gimexcallbacks.gseek = &gluegseek;
    gimexcallbacks.glen  = &glueglen;
    gimexcallbacks.gtell = &gluegtell;
}



void GCALL GIMEX_installcallbacks64(GIMEXCALLBACKS *callbacks)
{
    memcpy(&gimexcallbacks, callbacks, sizeof(GIMEXCALLBACKS));
    memset(&oldgimexcallbacks, 0, sizeof(OLDGIMEXCALLBACKS));
}

GSTREAM * GCALL gopen(const char *pathname)
{
    return(gimexcallbacks.gopen(pathname));
}

GSTREAM * GCALL gwopen(const char *pathname)
{
    return(gimexcallbacks.gwopen(pathname));
}

bool GCALL gclose(GSTREAM *g)
{
    return(gimexcallbacks.gclose(g));
}

int GCALL gread(GSTREAM *g, void *buf, int size)
{
    return(gimexcallbacks.gread(g,buf,size));
}

int GCALL gwrite(GSTREAM *g, const void *buf, int size)
{
    return(gimexcallbacks.gwrite(g,buf,size));
}

bool GCALL gseek(GSTREAM *g, GPOS seekpos)
{
    return(gimexcallbacks.gseek(g,seekpos));
}

GPOS GCALL glen(GSTREAM *g)
{
    return(gimexcallbacks.glen(g));
}

GPOS GCALL gtell(GSTREAM *g)
{
    return(gimexcallbacks.gtell(g));
}

void * GCALL galloc(int size)
{
    return(gimexcallbacks.galloc(size));
}

bool GCALL gfree(void *memptr)
{
    return(gimexcallbacks.gfree(memptr));
}

/*
 * Required to make the dll
*/

int APIENTRY LibMain(HANDLE hdll, DWORD  reason, LPVOID reserved )
{
    return( 1 );        /* indicate success */
}

