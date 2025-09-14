// (c) 1995-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                    CRapi Wraper Module  v1.09                    */
/*                                                                  */
/*                        by Gerry Shaw, EAC                        */
/*                                                                  */
/*                    Main Module - Sep 24, 2001                    */
/*                                                                  */
/*------------------------------------------------------------------*/

#include "crapi.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "nil.h"
#include "wu.h"
#include "wu555.h"
#include "wu777.h"
#include "exact.h"
#include "alpha.h"
#include "fu.h"
#include "wudata.h"

struct
{
    CRABOUT *    (GCALL * CRAPI_about)(void);
    CRINSTANCE * (GCALL * CRAPI_open)(void);
    int          (GCALL * CRAPI_read)(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
    ARGB *       (GCALL * CRAPI_close)(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);
} crapi[]=
{
    {NILCR_about,  NILCR_open,  NILCR_read,  NILCR_close},
    {WU_about,     WU_open,     WU_read,     WU_close},
    {FU_about,     FU_open,     FU_read,     FU_close},
    {WU777_about,  WU777_open,  WU777_read,  WU777_close},
    {PQ_about,     PQ_open,     PQ_read,     PQ_close},

    {NILCR_about,  NILCR_open,  NILCR_read,  NILCR_close},
    {KOLB_about,   KOLB_open,   KOLB_read,   KOLB_close},

    {EXACT_about,  EXACT_open,  EXACT_read,  EXACT_close},
    {ALPHA_about,  ALPHA_open,  ALPHA_read,  ALPHA_close},
    {WU555_about,  WU555_open,  WU555_read,  WU555_close},
    {WUDA_about,   WUDA_open,   WUDA_read,   WUDA_close }
};

static int currentcrapi = 0;

// for old api
extern "C" {
int crcolours = 256;
}

/****************************************************************/
/*  Crapi functions for DLL export                              */
/****************************************************************/

#if 0
/****************************************************************************************
; ABSTRACT
;
; CRAPI_init - Load CRAPI library.
;
;
; SUMMARY
;
; #include "crapi.h"               Required for structure declarations
; #include "copen.c"               Required for CRAPI DLL
;
; int CRAPI_init();
;
;
; DESCRIPTION
;
; The CRAPI_init function loads the CRAPI library and must be called prior
; to using any CRAPI function.
; If the application is loading the CRAPI from a DLL, "copen.c" should be
; included as it contains the CRAPI_init function for DLL loading.
; "copen.c" is not needed if the application is statically linking the
; CRAPI library.
;
; To unload the CRAPI library, call CRAPI_restore.
;
;
; RETURN VALUE
;
; Non-zero if successful.
;
;
; SEE ALSO
;
; CRAPI_restore
;
;
; EXAMPLE
;
; crtest.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

bool GCALL CRAPI_init(void)
{
    return(true);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; CRAPI_restore - Unload CRAPI library.
;
;
; SUMMARY
;
; #include "crapi.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "copen.c"             Required for CRAPI DLL
;
; void CRAPI_restore();
;
;
; DESCRIPTION
;
; The CRAPI_restore function unloads the CRAPI library after a call to
; CRAPI_init.
;
;
; SEE ALSO
;
; CRAPI_init
;
;
; EXAMPLE
;
; crtest.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

void GCALL CRAPI_restore(void)
{
    return;
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; CRAPI_about - Provide information about CRAPI module.
;
;
; SUMMARY
;
; #include "crapi.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "copen.c"             Required for CRAPI DLL
;
; CRABOUT *CRAPI_about();
;
;
; DESCRIPTION
;
; The CRAPI_about function returns information about the CRAPI module
; such as module name, version, and author.
; It may be called any time after a CRAPI module has been set (see
; CRAPI_set, CRAPI_find).
; The caller is responsible for freeing the memory allocated for the
; CRABOUT structure with the gfree function.
;
;
; RETURN VALUE
;
; Pointer to CRABOUT structure.
;
;
; SEE ALSO
;
; CRABOUT
;
;
; EXAMPLE
;
; crtest.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; CRABOUT - About CRAPI Module Structure.
;
;
; SUMMARY
;
; #include "crapi.h"                Required for structure declarations
;
; typedef struct
; {
;     int signature;                  // crapi module signature
;     int size;                       // size of the CRABOUT structure
;     int  version;                   // version of the CRABOUT structure
;     unsigned int  inversetable :1;  // can return an inversetable if requested
;     unsigned int  external     :1;  // requires external plugin or module
;     unsigned int  pad          :14; // pad to 32 bits
;     unsigned int  pad2         :16; // pad to 32 bits
;     char authorstr[32];             // module author, ie "Gerry Shaw"
;     char versionstr[8];             // version number of module, ie "1.00"
;     char namestr[16];               // single word name of the crapi
;     char longnamestr[32];           // full name of the crapi
; } CRABOUT;
;
;
; DESCRIPTION
;
; The CRABOUT structure describes the characteristics of the CRAPI module such
; as module name, version, and author.  This structure is filled in by the function
; CRAPI_about.
;
;
; SEE ALSO
;
; CRAPI_about
;
;
; EXAMPLE
;
; crtest.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

CRABOUT * GCALL CRAPI_about(void)
{
    return(crapi[currentcrapi].CRAPI_about());
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; CRAPI_open  - Open CRAPI module.
;
;
; SUMMARY
;
; #include "crapi.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "copen.c"             Required for CRAPI DLL
;
; CRINSTANCE * CRAPI_open();
;
;
; DESCRIPTION
;
; The CRAPI_open function loads a CRAPI module for use, returning an
; instance to the module for use in subsequent function calls (CRAPI_read,
; CRAPI_close).  Before calling CRAPI_open, a CRAPI module must be selected
; via CRAPI_set or CRAPI_find.
;
;
; RETURN VALUE
;
; Pointer to CRAPI module instance.  NULL if function call failed.
; The CRAPI module must first be set by a call to CRAPI_set or CRAPI_find.
;
;
; SEE ALSO
;
; CRAPI_set, CRAPI_find, CRAPI_close, CRAPI_read
;
;
; EXAMPLE
;
; colourreduce_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; CRINSTANCE - Data structure for an open CRAPI module.
;
;
; SUMMARY
;
; #include "crapi.h"          // Required for structure declarations
;
; typedef struct
; {
;     int signature;          // crapi module signature
;     int size;               // size of the CRINSTANCE structure
;     int frames;             // number of frames processed so far
;     char *cref;            // reference to additional memory for module
; } CRINSTANCE;
;
;
; DESCRIPTION
;
; The CRINSTANCE data structure is returned by the CRAPI_open functions.
; It is used to identify an opened CRAPI module and hold its data.
;
;
; SEE ALSO
;
; CRAPI_open, CRAPI_read, CRAPI_close
;
;
; EXAMPLE
;
; crtest.c, colourreduce_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

CRINSTANCE * GCALL CRAPI_open(void)
{
    return(crapi[currentcrapi].CRAPI_open());
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; CRAPI_read - Read the image data using the current CRAPI module.
;
;
; SUMMARY
;
; #include "crapi.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "copen.c"             Required for CRAPI DLL
;
; int CRAPI_read (crx, ginfo, source, rowbytes);
; CRINSTANCE *crx       CRAPI module instance from CRAPI_open
; GINFO *ginfo          Image frame information
; const void *source    Source image buffer
; int rowbytes          Bytes of image data per row
;
;
; DESCRIPTION
;
; The CRAPI_read function uses the current CRAPI module to read a frame
; of image data and collect colour information.  The source image must be
; 32-bit ARGB.
; The CRAPI instance is provided by CRAPI_open.
; The GINFO structure, containing the information of the frame to retrieve,
; is provided by the GIMEX_info function.
;
;
; RETURN VALUE
;
; A non-zero value is returned if successful.
;
;
; SEE ALSO
;
; CRAPI_write
;
;
; EXAMPLE
;
; colourreduce_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

int GCALL CRAPI_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes)
{
    return(crapi[currentcrapi].CRAPI_read(crx, ginfo, source, rowbytes));
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; CRAPI_close  - Close a file from reading by CRAPI module.
;
;
; SUMMARY
;
; #include "crapi.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "copen.c"             Required for CRAPI DLL
;
; ARGB* CRAPI_close(crx, numcolours, inversetable, returncolours)
; CRINSTANCE     *crx               CRAPI module instance
; int numcolours                    The number of colours in the new palette
; CRINVERSETABLE **inversetable     The address of the pointer to an empty inversetable
; int *returncolours                Actual number of colours generated (may be less than numcolours)
;
;
; DESCRIPTION
;
; The CRAPI_close function takes a CRAPI instance structure created by the CRAPI_open
; function. The number of colours and the inversetable are used for creating the reduced
; colour palette. The new palette is returned in the form of a table of ARGB values.
;
; Some CRAPI modules create an inversetables and some dont. Please
; refer to each CRAPI module for more information. The inversetables help in remapping the image
; to the new palette.
;
;
; RETURN VALUE
;
; The reduced ARGB colour palette.
;
;
; SEE ALSO
;
; CRAPI_close, CRAPI_info, CRAPI_read
;
;
; EXAMPLE
;
; colourreduce_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif
#if 0
/****************************************************************************************
; ABSTRACT
;
; CRINVERSETABLE - Data structure for storing values for the inversetable.
;
;
; SUMMARY
;
; #include "crapi.h"          // Required for structure declarations
;
; typedef struct CRINVERSETABLE
; {
;   unsigned int  signature; // type of inversetable
;   int           size;      // size of CRINVERSETABLE
;   unsigned char alphabits; // bits per channel stored in table
;   unsigned char redbits;   // 0666 for Wu's inversetable
;   unsigned char greenbits;
;   unsigned char bluebits;
;   unsigned char reserved[4]; // pad to 16 byte header
;   // inversetable data follows header structure
; } CRINVERSETABLE;
;
;
; DESCRIPTION
;
; A pointer to a CRINVERSETABLE structure must be created by the user application and its
; address is passed into the CRAPI_close function. It is used to identify the type of
; inversetable that is created.
;
; If the CRAPI module makes use of the CRINVERSETABLE, it will allocate enough space for
; the struct itself as well as for the inversetable so that they occupy one chunk of
; contiguous space, where the inversetable follows directly after the struct.
;
; The inversetable is useful for remapping the image to the new colour palette.
;
;
; SEE ALSO
;
; CRAPI_close
;
;
; EXAMPLE
;
; colourreduce_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

ARGB * GCALL CRAPI_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours)
{
    return(crapi[currentcrapi].CRAPI_close(crx, numcolours, inversetable, returncolours));
}

extern "C" {

ARGB * GCALL OLDCRAPI_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable)
{
    return(crapi[currentcrapi].CRAPI_close(crx, numcolours, inversetable, NULL));
}

}

/****************************************************************/
/*  Set crapi function pointers based on number                */
/****************************************************************/

#if 0
/****************************************************************************************
; ABSTRACT
;
; CRAPI_set - Set the current CRAPI module using enumerated value.
; CRAPI_get - Get the enumerated value of the current CRAPI module.
;
;
; SUMMARY
;
; #include "crapi.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "copen.c"             Required for CRAPI DLL
;
; void CRAPI_set(typenum)
; int  CRAPI_get()
; int typenum                Enumerated value of CRAPI module
;
;
; DESCRIPTION
;
; The CRAPI_set function sets the current CRAPI module to that specified
; by an enumerated value.  This value should be obtained using the
; CRAPI_get function.  It is also possible to loop through all CRAPI
; modules from 0 (NULL module) to CRAPI_count - 1.
;
; The CRAPI_get function returns the enumerated value of the current
; CRAPI module.  This value should not be assumed to remain constant between
; runtime sessions, particularly when using CRAPI DLL.
;
;
; RETURN VALUE
;
; CRAPI_get - An integer value specifying the current CRAPI module, 0 if failed.
;
;
; SEE ALSO
;
; CRAPI_find, CRAPI_count
;
;
; EXAMPLE
;
; crtest.c, colourreduce_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

void GCALL CRAPI_set(int typenum)
{
    if (typenum >= CRAPI_count() || typenum < 0)
        typenum = 0;

    currentcrapi = typenum;
}

int GCALL CRAPI_get(void)
{
    return(currentcrapi);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; CRAPI_count - Get the number of CRAPI modules.
;
;
; SUMMARY
;
; #include "crapi.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "copen.c"             Required for CRAPI DLL
;
; int CRAPI_count()
;
;
; DESCRIPTION
;
; The CRAPI_count function returns the number of CRAPI modules.
; It is possible to loop through all CRAPI modules from 0 (NULL module)
; to CRAPI_count - 1.
;
;
; RETURN VALUE
;
; The number of available CRAPI modules, including the NULL module.
;
;
; SEE ALSO
;
; CRAPI_set, CRAPI_get, CRAPI_find
;
;
; EXAMPLE
;
; crtest.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

int GCALL CRAPI_count(void)
{
    return(sizeof(crapi)/sizeof(crapi[0]));

    /*
    Changed Oct. 25, 1999 by James Fairweather.  There's no reason we shouldn't make this
    optimization that I can see, so let's do it.

    int num=0;

    while(crapi[num].CRAPI_about != NULL)
    {
        num++;
    }

    return(num);
    */
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; CRAPI_find - Set the current CRAPI module from text description.
;
;
; SUMMARY
;
; #include "crapi.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "copen.c"             Required for CRAPI DLL
;
; int CRAPI_find(name)
; const char *name                Name of CRAPI module
;
;
; DESCRIPTION
;
; The CRAPI_find function sets the current CRAPI module by searching for
; the matching name description from the CRAPI modules' about structure.
; match to a filename extension.  For example, CRAPI_find("4D Wu") would
; use the Wu CRAPI module.
;
;
; RETURN VALUE
;
; The enumerated value of the new CRAPI module, 0 if failed.
;
;
; SEE ALSO
;
; CRAPI_set, CRAPI_get, CRABOUT
;
;
; EXAMPLE
;
; crtest.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

int GCALL CRAPI_find(const char *name)
{
    int besttype=0;
    int i;
    int found=0;

    for (i=0; (i < CRAPI_count()) && !found; i++)
    {
        CRABOUT *about;
        CRAPI_set(i);
        about = CRAPI_about();
        if (strcmp(about->namestr, name) == 0)
        {
            besttype = i;
            found = 1;
        }
        gfree(about);
    }

    CRAPI_set(besttype);
    return(besttype);
}

