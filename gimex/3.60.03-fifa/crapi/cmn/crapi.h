// (c) 1994-2007 Electronic Arts Inc.

/*
; ABSTRACT
;
; crapi.h - Primary header file for the CRAPI API.
;
; SOURCE
;@
*/

/*------------------------------------------------------------------*/
/*                                                                  */
/*                     Colour Reduction API v1.69                   */
/*                                                                  */
/*      by Gerry Shaw, Frank Barchard and Laurent Ancessi, EAC      */
/*                                                                  */
/*                   Header Module - Mar 04, 2003                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 950523 GS initial version                                */
/*    1.01 960617 GS crapi 1.20 update                              */
/*    1.02 961118 FB added exact                                    */
/*    1.03 970606 KO added alpha                                    */
/*    1.04 980202 KO Crapi func vectors now functions for DLL       */
/*    1.05 980213 KO Added CRAPI_find(). Removed NUMCRAPI           */
/*    1.06 990703 LC added CRINVERSETABLE struct                    */
/*    1.07 991224 FB oldclose and new                               */
/*    1.08 000104 FB removed stat                                   */
/*    1.62 010427 FB CRAPI_VERSION checks and added this history    */
/*    1.63 010706 FB inversetable internal to remap and fixed       */
/*    1.64 010801 FB const ginfo                                    */
/*    1.65 010924 FB converted to C++ internally                    */
/*    1.66 011002 FB CRAPI_findcolour public                        */
/*    1.67 011015 FB CRAPI_init bool                                */
/*    1.68 011030 FB void cref                                      */
/*    1.69 030304 FB CRAPI_getcolour removed mask and adds flag     */
/*                                                                  */
/*------------------------------------------------------------------*/

#ifndef __CRAPI__H
#define __CRAPI__H 1

#if defined(_MSC_VER)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "gimex.h"

#define CRAPI_VERSION 169

#if defined(_MSC_VER)
/* warning C4514: unreferenced inline function has been removed */
#pragma warning(disable: 4514)
// warning C4996: 'strcpy' was declared deprecated
#pragma warning(disable: 4996)
#endif

/****************************************************************/
/*  Data Types                                                  */
/****************************************************************/

typedef struct
{
    int signature;          /* crapi module signature */
    int size;               /* size of the CRINSTANCE structure */
    int frames;             /* number of frames processed so far */
    void *cref;            /* reference to additional memory for module */
} CRINSTANCE;


/* CRBOUT structure describes crapi module capabilities */

typedef struct
{
    int  signature;                 /* crapi module signature */
    int  size;                      /* size of the CRABOUT structure */
    int  version;                   /* version of the CRABOUT structure */
    unsigned int  inversetable :1;  /* can return an inversetable if requested */
    unsigned int  external     :1;  /* requires external plugin or module */
    unsigned int  respectalpha :1;
    unsigned int  pad          :13; /* pad to 32 bits */
    unsigned int  pad2         :16; /* pad to 32 bits */
    char authorstr[32];             /* module author, ie "Gerry Shaw" */
    char versionstr[8];             /* version number of module, ie "1.00" */
    char namestr[16];               /* single word name of the crapi */
    char longnamestr[32];           /* full name of the crapi */
} CRABOUT;

typedef unsigned char GINVERSETABLE[64][64][64];

typedef struct CRINVERSETABLE
{
    unsigned int  signature; // type of inversetable
    int           size;      // size of CRINVERSETABLE
    unsigned char alphabits; // bits per channel stored in table
    unsigned char redbits;   // 0666 for Wu's inversetable
    unsigned char greenbits;
    unsigned char bluebits;
    unsigned char reserved[4]; // pad to 16 byte header
    // inversetable data follows header structure
} CRINVERSETABLE;

/****************************************************************/
/* CRapi Module Prototypes                                      */
/****************************************************************/

CRABOUT *    GCALL CRAPI_about(void);
CRINSTANCE * GCALL CRAPI_open(void);
int          GCALL CRAPI_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *       GCALL CRAPI_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

/* close returns numcolour ARGB's, pass address of pointer to recieve */
/* galloc'ed inverse table, pass null if not wanted (inversetable may */
/* not be available in all modules, check CRABOUT) */


/*
 * Called before and after using following functions.  Defined in inicrapi.c.
 */
bool GCALL CRAPI_init(void);
void GCALL CRAPI_restore(void);

int  GCALL CRAPI_find(const char *name); /* choose which crapi "Wu" */
void GCALL CRAPI_set(int typenum);
int  GCALL CRAPI_get(void);
int  GCALL CRAPI_count(void);

/****************************************************************/
/* Inverse table lookup macros                                  */
/****************************************************************/

static __inline unsigned char INVERSETABLE_getindex(const CRINVERSETABLE *inversetable, const ARGB *color)
{
    int inr=(color->r>>(8-inversetable->redbits));
    int ing=(color->g>>(8-inversetable->greenbits));
    int inb=(color->b>>(8-inversetable->bluebits));

    int ind= inr*(1<<inversetable->redbits)*(1<<inversetable->redbits) + ing*(1<<inversetable->greenbits) + inb;

    return ((unsigned char*)(inversetable) + sizeof(CRINVERSETABLE))[ind];
}

/****************************************************************/
/* Inverse table prototypes                                     */
/****************************************************************/

CRINVERSETABLE * INVERSETABLE_create(unsigned char a, unsigned char r, unsigned char g, unsigned char b, char* signature);

/****************************************************************/
/* CRAPI remap prototype                                     */
/****************************************************************/

void GCALL CRAPI_remap(void *destpixels, const GINFO *destginfo, int destrowbytes, void *sourcepixels, int sourcerowbytes, int sourcebpp, const unsigned char *colourmask, const CRINVERSETABLE *ginversetbl, int OPTdither);
int CRAPI_findcolour(const ARGB *palette, int numcolours, const unsigned char *mask, const ARGB *color); // old function
int CRAPI_getcolour(ARGB color, int numcolours, const ARGB *palette, int flags=0);

// CRAPI_getcolour flags
#define GC_NONE             0
#define GC_RESPECTALPHA     1





/* Compatability */

#ifdef __cplusplus
}
#endif

#endif

/* END ABSTRACT */
