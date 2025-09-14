// (c) 1994-2007 Electronic Arts Inc.

/*! 
 * \file  gopenlib.cpp
 * \brief Lib version of the gopen interface
 */
#include "gopen.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// this generates the manifest
//#if defined(_MSC_VER) && _MSC_VER>=1400
//#define ENABLE_OMP 1
//#include <omp.h>
//#endif


extern "C"
{
    int GCALL OLDGIMEX_open(GINSTANCE **gx, GSTREAM *g, const char *pathname);
}

void (*GIMEX_freeram)(void)= NULL;

/* case independent compare */

#define gtolower(a) (((a)>='A'&&(a)<='Z') ? ((a)-'A'+'a') : (a))

static int gstricmp(const char *d, const char *s)
{
    int v=0;

    do
    {
        v=(gtolower(*d)-gtolower(*s));
        if (!*d)
            break;
        ++s;
        ++d;
    } while (!v);
    return(v);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; ARGB - Structure for 32-bit pixels and palette entries.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
;
; // Windows, Linux & PSX:
; typedef struct
; {
;   unsigned char b,g,r,a;
; } ARGB;
;
; // SGI & PS2
; typedef struct
; {
;   unsigned char r,g,b,a;
; } ARGB;
;
; // Mac:
; typedef struct
; {
;   unsigned char a,r,g,b;
; } ARGB;
;
; DESCRIPTION
;
; The ARGB structure describes 32-bit pixels and palette entries.
; You must use this structure to access individual channels of a pixel or
; palette entry. The channels are:
; a - Alpha channel. 0 transparent.. 255 opaque. Unattentuated.
; r - Red channel.   0..255
; g - Green channel. 0..255
; b - Blue channel.  0..255
;
;
; SEE ALSO
;
; GINFO
;
;
; EXAMPLE
;
; saveany_eg
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; GIMEX_gopenstream - Set best GIMEX module from an open file.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"                Required for GIMEX DLL
;
; bool GIMEX_gopenstream(char *pathname, GSTREAM *g)
; const char    *pathname     Pathname of file
; GSTREAM		*g            Handle to opened file
;
;
; DESCRIPTION
;
; The GIMEX_gopenstream function determines the
; best GIMEX module based on the filename extension and the contents
; (see GIMEX_is).  The current GIMEX module is set to this best match.
; The file must already be opened and the file handle passed to this function.
; In most cases, use GIMEX_gopen.
;
; The pathname of the file may be relative to the current directory or
; absolute.
;
;
; RETURN VALUE
;
; true if successful.
;
;
; SEE ALSO
;
; GIMEX_gopen, gopen
;
;
; EXAMPLE
;
; none available yet
;
;
; END ABSTRACT
****************************************************************************************/
#endif

bool GCALL GIMEX_gopenstream(const char *pathname, GSTREAM *g)
{
    GABOUT *about;
    int type=0;
    int ismatch;
    int bestmatch;
    int besttype;
    int plier;
    int found;

    plier = 100;
    found = 1;

/****************************************************************/
/*  Determine best match                                        */
/*  -uses multiplier based on whether file opened directly      */
/****************************************************************/

    bestmatch = 0;
    besttype = 0;
    for (type=0; type<GIMEX_count() && bestmatch<100; ++type)
    {
        about = gfunctions[type].GIMEX_about();
        if (about)
        {
            ismatch = 0;
            if (about->usesfile)
            {
                if (g)
                    ismatch = gfunctions[type].GIMEX_is(g)*plier/100;

                /* reduce ismatch by 10 if extension is not in GABOUT */
                if (ismatch > 10)
                {
                    bool extmatch = false;
                    const char *ext = strrchr(pathname, '.');
                    int i;

                    if (ext)
                    {
                        for (i = 0; i<MAXEXTENSIONS && about->extensions[i]; ++i)
                        {
                            if (!gstricmp(about->extensions[i], ext))
                            {
                                extmatch = true;
                                break;
                            }
                        }
                    }
                    if (!extmatch)
                        ismatch -= 10;
                }
            }
            else
            {
                if (!gstricmp(pathname,about->shorttypestr))
                    ismatch = 50;
                if (!gstricmp(pathname,about->wordtypestr))
                    ismatch = 80;
                if (!gstricmp(pathname,about->longtypestr))
                    ismatch = 95;

                if (!ismatch)
                {
                    const char *ext = strrchr(pathname, '.');

                    if (ext)
                    {
                        if (!gstricmp(about->shorttypestr,ext+1))
                            ismatch = 45;
                    }
                }
            }

            if (ismatch>bestmatch)
            {
                bestmatch = ismatch;
                besttype  = type;
            }
            gfree((char *)about);
        }
    }
    if (!bestmatch)
    {
        besttype = 0;
    }
    GIMEX_set(besttype);
    return(besttype?true:false);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; GIMEX_gopen - Open a file and set best GIMEX module.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
; 
; GSTREAM *GIMEX_gopen(pathname, returnname)
; char  *pathname           Pathname of file
; char  *returnname         Returned pathname of file
;                           (Multifile or resource-fork-based file formats)
; 
;
; DESCRIPTION
;
; The GIMEX_gopen function opens an existing graphics file for reading,
; returning a file stream pointer for subsequent operations (GIMEX_open,
; GIMEX_is, etc).  As the file is opened, this function determines the
; best GIMEX module to use based on the filename extension and the contents
; (see GIMEX_is).  The current GIMEX module is set to this best match.
;
; The pathname of the file may be relative to the current directory or
; absolute.
;
; Returnname:
; If a preallocated returnname buffer is provided, it will be filled in
; with the name of the file opened.  This may be useful if a pathname is
; given without an extension (e.g. myimage instead of myimage.tga), in
; which case the closest matching filename extension is appended.
; If you have no need for the returnname, give a NULL pointer.
;
; The returned GSTREAM must be closed using the gclose function.
;
;
; RETURN VALUE
;
; A pointer to a GSTREAM structure used for subsequent operations such as
; GIMEX_open and GIMEX_is.
; A NULL pointer indicates an error occurred during opening.
;
;
; SEE ALSO
;
; GIMEX_open, GIMEX_is
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

GSTREAM * GCALL GIMEX_gopen(const char *pathname, char *returnname)
{
    GSTREAM *g=0;
    GABOUT *about;
    int type=0;
    int besttype;
    int plier;
    int found;
    int i;
    char fn[1024];

/****************************************************************/
/*  Open the file                                               */
/*  -try known extensions but set probability lower             */
/****************************************************************/

    plier = 100;
    strncpy(fn,pathname,sizeof(fn));
    g = gopen(fn);
    found = 1;
    if (!g)
    {
        found = 0;
        for (type=0; type<GIMEX_count() && !found; ++type)
        {
            plier = 75;
            about = gfunctions[type].GIMEX_about();
            if (about)
            {
                if (about->usesfile && returnname)
                {
                    for (i=0; i<MAXEXTENSIONS && !found; ++i)
                    {
                        if (about->extensions[i][0])
                        {
                            strcpy(fn,pathname);
                            strcat(fn,about->extensions[i]);
                            g = gopen(fn);
                            if(g)
                            {
                                if (gfunctions[type].GIMEX_is(g))
                                {
                                    found = 1;
                                    break;
                                }
                                else
                                {
                                    gclose(g);
                                    g = 0;
                                }
                            }
                        }
                        plier = 40;
                    }
                }
                gfree((char *)about);
            }
        }
    }

/****************************************************************/
/*  Determine best match                                        */
/*  -uses multiplier based on whether file opened directly      */
/****************************************************************/

    besttype = 0;
    if (!found)
        strcpy(fn,pathname);

    besttype = GIMEX_gopenstream(fn, g);
    if (!besttype)
    {
        fn[0] = 0;
        if (g)
            gclose(g);
        g = 0;
    }

    if (returnname)
        strcpy(returnname,fn);

    return(g);
}


/****************************************************************/
/*  Set gimex function pointers based on number                 */
/****************************************************************/

static int currentgimex=0;

#if 0
/****************************************************************************************
; ABSTRACT
; 
; GIMEX_init - Load GIMEX library.
; 
; 
; SUMMARY 
; 
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
;
; bool GIMEX_init();
; 
;
; DESCRIPTION
;
; The GIMEX_init function loads the GIMEX library and must be called prior
; to using any GIMEX function.
; If the application is loading the GIMEX from a DLL, "gopen.c" should be
; included as it contains the GIMEX_init function for DLL loading.
; "gopen.c" is not needed if the application is statically linking the 
; GIMEX library.
;
; To unload the GIMEX library, call GIMEX_restore.
; 
; 
; RETURN VALUE 
; 
; true if successful.
;
;
; SEE ALSO 
;
; GIMEX_restore
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

bool GCALL GIMEX_init(void)
{
    return true;
}

#if 0
/****************************************************************************************
; ABSTRACT
; 
; GIMEX_restore - Unload GIMEX library.
; 
; 
; SUMMARY 
; 
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
; 
; void GIMEX_restore();
; 
; 
; DESCRIPTION
;
; The GIMEX_restore function unloads the GIMEX library after a call to
; GIMEX_init.
; 
; 
; SEE ALSO 
;
; GIMEX_init 
;
;
; EXAMPLE 
;
; any2any_eg.c
; 
;
; END ABSTRACT
****************************************************************************************/
#endif

void GCALL GIMEX_restore(void)
{
    if (GIMEX_freeram)
        GIMEX_freeram();
    return;
}

#if 0
/****************************************************************************************
; ABSTRACT
; 
; GIMEX_about - Provide information about Gimex module and data file 
;               format capabilities.
; 
; 
; SUMMARY 
; 
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
; 
; GABOUT *GIMEX_about();
; 
; 
; DESCRIPTION
;
; The GIMEX_about function returns information about the Gimex module 
; capabilities and requirements, as well the file format specifications. 
; It may be called any time after a GIMEX module has been set (see
; GIMEX_gopen). 
; The caller is responsible for freeing the memory allocated for the 
; GABOUT structure with the gfree function. 
; 
; 
; RETURN VALUE 
; 
; Pointer to GABOUT structure.
;
;
; SEE ALSO
;
; GABOUT
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; GABOUT - About Gimex Module Structure.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
;
; typedef struct
; {
;     int signature;                 // signature of gimex ie 'tga ' (optional)
;     int size;                      // size of GABOUT structure
;     int  version;                   // version number of GABOUT structure (200)
;     unsigned int canimport     :1;  // supports importing
;     unsigned int canexport     :1;  // supports exporting
;     unsigned int importpacked  :2;  // max import packed field 0..3
;     unsigned int exportpacked  :2;  // max export packed field 0..3
;     unsigned int import8       :1;  // supports importing 8 bit indexed
;     unsigned int export8       :1;  // supports exporting 8 bit indexed
;     unsigned int import32      :1;  // supports importing 32 bit direct rgb
;     unsigned int export32      :1;  // supports exporting 32 bit direct rgb
;     unsigned int multiframe    :1;  // supports multiple frames
;     unsigned int multifile     :1;  // format requires additional files or resource fork
;     unsigned int multisize     :1;  // supports different size per frame
;     unsigned int framebuffer   :1;  // module requires memory to buffer entire frame
;     unsigned int external      :1;  // module requires external tool or plugin
;     unsigned int usesfile      :1;  // module is file based vs ads/printer/generator
;     unsigned int globalpalette :1;  // limited to a single palette per file
;     unsigned int greyscale     :1;  // use maxcolours for number of levels
;     unsigned int startcolour   :1;  // supports start colour
;     unsigned int dotsubtype    :1;  // subtype based on extension
;     unsigned int resizable     :1;  // read will respect ginfo width & height
;     unsigned int pad           :11; // pad bitfield to 32 bit boundary for inter compiler compatibility
;     int  maxcolours;                // only use in 8 bit, 0 if module does not care
;     int  maxframename;              // maximum characters in ginfo framename
;     int  defaultquality;            // default pack quality
;     int mactype[MAXMACTYPES];      // mac file system types used
;     char extensions[MAXEXTENSIONS][GIMEX_EXTENSION_SIZE]; // null terminated extensions with '.'
;     char authorstr[GIMEX_AUTHORSTR_SIZE];        // name of gimex module author
;     char versionstr[GIMEX_VERSIONSTR_SIZE];      // version number of gimex module ie 1.00
;     char shorttypestr[GIMEX_SHORTTYPESTR_SIZE];  // 3 or 4 character type string ie TGA
;     char wordtypestr[GIMEX_WORDTYPESTR_SIZE];    // single word type string ie Targa
;     char longtypestr[GIMEX_LONGTYPESTR_SIZE];    // full name of data format ie True Vision Targa
; } GABOUT;
;
;
; DESCRIPTION
;
; The GABOUT structure describes the characteristics of the Gimex module and the file format.  This
; structure is filled in by the function GIMEX_about(), which may be called at any time, as it is not
; based on a particular file.  The caller must free the structure.
;
;
; SEE ALSO
;
; GIMEX_about
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

GABOUT * GCALL GIMEX_about(void)
{
    return(gfunctions[currentgimex].GIMEX_about());
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; GIMEX_is - Identify if the specified data is a format parsable by 
;            current GIMEX module.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
; 
; int GIMEX_is(gstream);
; GSTREAM *gstream;               Graphics file handle.
; 
;
; DESCRIPTION 
;
; The GIMEX_is function returns whether the data from an open file stream is capable 
; of being handled by the current GIMEX module.
;
; NOTE: This function is for information purposes only.  GIMEX_gopen uses this
; function to determine the best GIMEX module to set based on the contents of a
; file.
; Many formats don't contain a signature and require heuristics to determine if the
; file appears to be the given format. If several file formats are be checked, the 
; highest value should be taken as the correct format. 
;
; 
; RETURN VALUE
;
; A value of 0 indicates the file is not parsable by the GIMEX module.
; A value of 100 indicates the file definitely parsable.
; A value in between 0 and 100 represents the probability that the file is 
; of a parsable format.
;
;
; SEE ALSO
;
; GIMEX_about, GIMEX_gopen
;
;
; EXAMPLE 
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

int GCALL GIMEX_is(GSTREAM *g)
{
    return(gfunctions[currentgimex].GIMEX_is(g));
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; GINSTANCE - Data structure of information about an open data stream and
;             corresponding GIMEX module.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
;
; typedef struct
; {
;   int signature;         // signature of gimex ie 'tga ' (optional)
;   int size;              // size of GINSTANCE structure
;   int  frames;            // Number of frames in file
;   int  framenum;          // current frame (optional)
;   GSTREAM *gstream;       // stream pointer for file
;   char *gref;             // gimex reference to additional memory used by module (optional)
; } GINSTANCE;
;
;
; DESCRIPTION
;
; The GINSTANCE data structure is returned by the GIMEX_open and GIMEX_wopen
; functions.  It is used to contain information about an opened file and is required as a
; parameter for GIMEX_info, GIMEX_read, GIMEX_close, GIMEX_write, GIMEX_wclose.
;
;
; SEE ALSO
;
; GIMEX_open, GIMEX_read
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif


#if 0
/****************************************************************************************
; ABSTRACT
;
; GIMEX_open  - Prepare a file for reading by GIMEX module.
; GIMEX_wopen - Prepare a file for writing by GIMEX module.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
;
; int  GIMEX_open (gx, g, pathname, framecountflag)
; bool GIMEX_wopen(gx, g, pathname, numframes)
; GINSTANCE     **gx                Pointer to Gimex module instance
; GSTREAM       *g                  Graphics file handle
; char          *pathname           Pathname of file
; bool          framecountflag      Return a frame count if true
; int           numframes           Number of frames to prepare
;
;
; DESCRIPTION
;
; The GIMEX_open function prepares a file for reading, returning a GIMEX
; instance structure for use by subsequent read operations.
; The GIMEX_wopen function prepares a file for writing, returning a GIMEX
; instance structure for use by subsequent write operations.
;
; Before opening a GIMEX file, the GIMEX module must be set. This can be
; done with GIMEX_set if the module name already known or with
; GIMEX_find if the module is to be determined via the filename extension.
; The GIMEX module must be reset prior to every use of the GIMEX_open or
; GIMEX_wopen, especially when looping through multiple files.
;
; A GIMEX instance must be a preallocated structure that is filled in by
; these functions.  It holds information about a specific graphics file and
; the GIMEX module currently used to parse the file.
;
; The GSTREAM handle points to an open graphics file as returned by the
; GIMEX_gopen or gopen function.
;
; GIMEX_open (or GIMEX_wopen) should be followed by GIMEX_info to target a
; specified frame for reading, then GIMEX_read (of GIMEX_write) to import
; (or export) the frame from the file.
; GIMEX_close (or GIMEX_wclose) should be called once all importing (or
; exporting) from the file is done.
;
; Multifile or resource fork based file formats:
; The pathname specified is required for multifile or resource fork
; based file formats, which will open auxilary files based on the original
; filename.  gopen is used to open these files. The GABOUT structure field
; multifile indicates GIMEX modules that do this.
;
;
; RETURN VALUE
;
; if framecountflag is set, an integer containing the number of
; frames contained in the file.
; if framecountflag is not set, a success flag 0=failure, 1=success
;
; SEE ALSO
;
; GIMEX_close, GIMEX_info, GIMEX_read
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

int GCALL GIMEX_open(GINSTANCE **gx, GSTREAM *g, const char *pathname, bool framecountflag)
{
    return(gfunctions[currentgimex].GIMEX_open(gx, g, pathname, framecountflag));
}

int GCALL OLDGIMEX_open(GINSTANCE **gx, GSTREAM *g, const char *pathname)
{
    return(gfunctions[currentgimex].GIMEX_open(gx, g, pathname, 1));
}

bool GCALL GIMEX_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes)
{
    return(((bool (GCALL *)(GINSTANCE **, GSTREAM *, const char *, int )) gfunctions[currentgimex].GIMEX_wopen)(gx, g, pathname, numframes));
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; GIMEX_info  - Retrieve info about a frame of image data.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
;
; GINFO *GIMEX_info(gx, framenum)
; GINSTANCE *gx                GIMEX module instance
; int framenum                 Frame number from which to retrieve info
;
;
; DESCRIPTION
;
; The GIMEX_info function retrieves information about a specific frame of a
; graphics file.  It requires a GIMEX module instance as returned by the
; GIMEX_open function.  The information about the frame is then used to
; read the frame data using the GIMEX_read function.  The GIMEX_open function
; returns the number of available frames in the image.
;
; The caller is responsible for freeing the memory allocated for the 
; GIMEX_info structure with the gfree function. 
;
; 
; RETURN VALUE
;
; A GINFO structure containing frame information.
; A NULL pointer is returned in the event of an error.
;
;
; SEE ALSO
;
; GIMEX_read
;
;
; EXAMPLE 
;
; any2any_eg.c
; 
; 
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; GINFO - Info about a frame of image data.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
;
; typedef struct
; {
;   int signature;         // signature of gimex ie 'tga ' (optional)
;   int size;              // size of GINFO structure
;   int  version;           // version number of GINFO structure (200)
;   int  framenum;          // current frame
;   int  width;             // width of bitmap in pixels
;   int  height;            // height of bitmap in pixels
;   int  bpp;               // bits per pixel (8 or 32)
;   int  originalbpp;       // bits per pixel in original image (1 to 32)
;   int  startcolour;       // first colour in palette
;   int  numcolours;        // number of colours in original indexed palette
;   ARGB colourtbl[GIMEX_COLOURTBL_SIZE]; // 8 bit palette
;   int  subtype;           // internal format sub-type 0-default
;   int  packed;            // type of packing on original image. 0 none, 1 run, n other
;   int  quality;           // quality of lossy packing 0..100
;   GPOS  framesize;         // size of frame in bytes
;   int  alphabits;         // number of bits in alpha channel
;   int  redbits;           // number of bits in red channel
;   int  greenbits;         // number of bits in green channel
;   int  bluebits;          // number of bits in blue channel
;   int  centerx;           // center point relative to upper left corner
;   int  centery;
;   int  defaultx;          // default coordinate point
;   int  defaulty;
;   int  numhotspots;       // number of hot spots defined
;   char framename[GIMEX_FRAMENAME_SIZE];
;                           // null terminated name of frame/image
;   char comment[GIMEX_COMMENT_SIZE];
;                           // null terminated multiline user comment
;   int  hotspottbl[GIMEX_HOTSPOTTBL_SIZE][GIMEX_HOTSPOTTBL_VALUES];
;                           // up to 256 hot spots, XY pairs relative to upperleft
;   float dpi;				// dots per inch ie 72.0 
;   float fps;              // frame per second (one over duration).  15.0 is typical
;   int  reserved[3];       // reserved for future use - set to zero
; } GINFO;
; 
;
; DESCRIPTION 
;
; The GINFO structure describes the contents of an image or of a single frame of a 
; graphics file, both in original form and in the imported form.  This structure is 
; filled in by the function GIMEX_info(), and must be freed by the caller. The 
; red/green/blue/alpha bits apply to the palette if the image is indexed. 
;
; The dimensions and colour-depth information is necessary for allocating a frame 
; buffer needed by GIMEX_read().  It is also needed by GIMEX_write() to specify the output 
; format.  While filter needs to be used for size and colour depth changes,
; parameters in the file (packed or unpacked, colour channel bits, etc.) can be changed 
; in the GINFO before the call to GIMEX_write() and these changes will be reflected in the 
; destination graphics file.
;
; 
; SEE ALSO
;
; GIMEX_read, GIMEX_write
;
;
; EXAMPLE 
;
; any2any_eg.c
; 
; 
; END ABSTRACT
****************************************************************************************/
#endif

GINFO * GCALL GIMEX_info(GINSTANCE *gx, int framenum)
{
    return(gfunctions[currentgimex].GIMEX_info(gx, framenum));
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; GIMEX_read  - Read the image data to an image buffer.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
;
; bool GIMEX_read(gx, ginfo, dest, rowbytes)
; GINSTANCE *gx                GIMEX module instance
; const GINFO *ginfo           Image frame information
; void *dest                   Destination image buffer
; int rowbytes                 Bytes of image data per row
;
;
; DESCRIPTION
;
; The GIMEX_read function uses the current GIMEX module to read a frame
; of image data into a frame buffer provided by the user.
; The GIMEX instance is provided by GIMEX_open.
; The GINFO structure, containing the information of the frame to retrieve,
; is provided by the GIMEX_info function.
; The destination frame buffer is allocated by the user at a size of
; (GINFO.width * GINFO.height * GINFO.bpp/8)
; The rowbytes argument is (GINFO.width * GINFO.bpp/8)
;
;
; RETURN VALUE
;
; true if successful.
;
; SEE ALSO
;
; GIMEX_write
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

bool GCALL GIMEX_read(GINSTANCE *gx, const GINFO *ginfo, void *dest, int rowbytes)
{
    return(  ((int (GCALL *)(GINSTANCE *, const GINFO *, void *, int )) gfunctions[currentgimex].GIMEX_read) (gx, ginfo, dest, rowbytes)!=0);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; GIMEX_close  - Close a file from reading by GIMEX module.
; GIMEX_wclose - Close a file from writing by GIMEX module.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
;
; bool GIMEX_close(gx)
; bool GIMEX_wclose(gx)
; GINSTANCE     *gx                Gimex module instance
;
;
; DESCRIPTION
;
; The GIMEX_close and GIMEX_wclose functions takes a GIMEX instance
; structure created by the GIMEX_open and GIMEX_wopen functions respectively.
;
; The GSTREAM handle points to an open graphics file as returned by the
; GIMEX_gopen or gopen function.
; GIMEX_close (or GIMEX_wclose) should be called once all importing (or
; exporting) from the file is done.
;
;
; RETURN VALUE
;
; true if successful.
;
; SEE ALSO
;
; GIMEX_open, GIMEX_info
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

bool GCALL GIMEX_close(GINSTANCE *gx)
{
    return(gfunctions[currentgimex].GIMEX_close(gx) != 0);
}

bool GCALL GIMEX_wclose(GINSTANCE *gx)
{
    return(gfunctions[currentgimex].GIMEX_wclose(gx) != 0);
}


#if 0
/****************************************************************************************
; ABSTRACT
;
; GIMEX_write - Write the image data from an image buffer to file.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
;
; bool GIMEX_write(gx, ginfo, source, rowbytes)
; GINSTANCE *gx                GIMEX module instance
; const GINFO *ginfo           Image frame information
; const void *source           Source image buffer
; int rowbytes                 Bytes of image data per row
;
;
; DESCRIPTION
;
; The GIMEX_write function uses the current GIMEX module to write a frame
; of image data from a frame buffer provided by the user.
; The GIMEX instance is provided by GIMEX_wopen.
; The GINFO structure, containing the information of the frame to retrieve,
; may be provided by calling GIMEX_info using an existing source file.  In some
; cases, the input image is converted to an output image which may differ in size
; or in bit depth.  Thus the width, height or bpp fields of the GINFO
; structure needs to be modified accordingly before writing out the output image.
; The destination frame buffer is allocated by the user at a size of
; (GINFO.width * GINFO.height * GINFO.bpp/8)
; The rowbytes argument is (GINFO.width * GINFO.bpp/8)
;
; RETURN VALUE
;
; true if successful.
;
;
; SEE ALSO
;
; GIMEX_read
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

bool GCALL GIMEX_write(GINSTANCE *gx, const GINFO *ginfo, const void *source, int rowbytes)
{
    return ((int (GCALL *)(GINSTANCE *, const GINFO *, const void *, int )) gfunctions[currentgimex].GIMEX_write)(gx, ginfo, source, rowbytes)!=0;
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; GIMEX_set - Set the current GIMEX module using enumerated value.
; GIMEX_get - Get the enumerated value of the current GIMEX module.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
;
; void GIMEX_set(typenum)
; int  GIMEX_get()
; int typenum                Enumerated value of GIMEX module
;
;
; DESCRIPTION
;
; The GIMEX_set function sets the current GIMEX module to that specified
; by an enumerated value.  This value should be obtained using the
; GIMEX_get function.  It is also possible to loop through all GIMEX
; modules from 0 (NULL module) to GIMEX_count - 1.
;
; The GIMEX_get function returns the enumerated value of the current
; GIMEX module.  This value should not be assumed to remain constant between
; runtime sessions, particularly when using GIMEX DLL.
;
;
; RETURN VALUE
;
; GIMEX_get - An integer value specifying the current GIMEX module, 0 if failed.
;
;
; SEE ALSO
;
; GIMEX_find, GIMEX_count
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

void GCALL GIMEX_set(int typenum)
{

    if (typenum>=GIMEX_count() || typenum<0)
    {
        typenum = 0;
    }

    currentgimex = typenum;
}

int GCALL GIMEX_get(void)
{
    return(currentgimex);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; GIMEX_find - Set the current GIMEX module from a filename extension.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
;
; int GIMEX_find(name, subtype)
; const  char *name         Extension string
; int  *subtype             Output. Subtype of image format.
; 
;
; DESCRIPTION 
;
; The GIMEX_find function sets the current GIMEX module using the closest
; match to a filename extension.  For example, GIMEX_find("jpg",0) would
; use the JPEG GIMEX module.
; The subtype parameter is set by the function (if it is non-NULL) to the 
; matching subtype of the GIMEX module.  This is only of use if the GIMEX
; module handles multiple subtypes.  For example, the STR GIMEX module
; handles the STR, XA and DCT formats, each having a unique subtype ID.
;
; 
; RETURN VALUE
;
; The enumerated value of the new GIMEX module, 0 if failed.
;
;
; SEE ALSO
;
; GIMEX_set, GIMEX_get, GIMEX_gopen, GINFO
;
;
; EXAMPLE 
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

int GCALL GIMEX_find(const char *name, int *subtype)
{
    GABOUT *about;
    int i;
    int extnum;
    char optbuf[1024];
    const char *s;
    char *d;
    int bestmatch=0;
    int besttype=0;
    int ismatch;

    s = name;
    d = optbuf;
    if (*s=='-' || *s=='+' || *s=='.') ++s;
    *d++ = '.';

    if ((unsigned char)*s > (unsigned char)' ')
    {
        while (d<optbuf+sizeof(optbuf) && (unsigned char)*s > (unsigned char)' ' && (unsigned char)*s != (unsigned char) '?')
        {
            *d++ = *s++;
        }
        *d++ = 0;

        for (extnum=0; extnum<MAXEXTENSIONS && bestmatch<4; ++extnum)
        {
            for (i=1; i<GIMEX_count() && bestmatch<4; ++i)
            {
                GIMEX_set(i);
                about = GIMEX_about();
                if (about)
                {
                    if (!gstricmp(optbuf,about->extensions[extnum]))
                    {
                        ismatch = extnum?1:2;
                        if (about->canexport)
                            ismatch += 2;
                        if (ismatch > bestmatch)
                        {
                            besttype = i;
                            bestmatch = ismatch;
                            if (subtype) *subtype = extnum;
                        }
                    }
                    gfree((char *)about);
                }
            }
        }
    }

    GIMEX_set(besttype);
    return(besttype);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; GIMEX_count - Get the number of GIMEX modules.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
; 
; int GIMEX_count()
; 
;
; DESCRIPTION 
;
; The GIMEX_count function returns the number of GIMEX modules.
; It is possible to loop through all GIMEX modules from 0 (NULL module)
; to GIMEX_count - 1.
;
; 
; RETURN VALUE
;
; The number of available GIMEX modules, including the NULL module.
;
;
; SEE ALSO
;
; GIMEX_set, GIMEX_get, GIMEX_find
;
;
; EXAMPLE 
;
; none available yet
; 
; 
; END ABSTRACT
****************************************************************************************/
#endif

int GCALL GIMEX_count(void)
{
    int num=0;

    while (gfunctions[num].GIMEX_about != NULL)
    {
        ++num;
    }

    return num;
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; GIMEX_freebitmap - Free memory used by GBITMAP structure.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
; 
; void GIMEX_freebitmap(gbitmap)
; GBITMAP *gbitmap        GBITMAP structure as returned by GIMEX_loadframe
; 
;
; DESCRIPTION 
;
; The GIMEX_freebitmap function frees the memory used by a GBITMAP structure
; as returned by the GIMEX_loadframe function.
;
;
; SEE ALSO
;
; GBITMAP, GIMEX_loadframe
;
;
; EXAMPLE 
;
; none available yet
; 
; 
; END ABSTRACT
****************************************************************************************/
#endif

void GCALL GIMEX_freebitmap(GBITMAP *gm)
{
    int isone;

    if (gm)
    {
        isone = 0;
        if ((char *)gm->ginfo == (char *)gm+sizeof(GBITMAP))
            isone = 1;

        if (!isone)
        {
            if (gm->image)
                gfree(gm->image);
            if (gm->ginfo)
                gfree(gm->ginfo);
        }
        gfree(gm);
    }
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; GIMEX_loadframe - Easy way to load a frame from file.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
; #include "gopen.h"                Required for function declarations
; #include "gopen.cpp"             Required for GIMEX DLL
; 
; GBITMAP *GIMEX_loadframe(filename, framenum)
; const char *filename      Name of graphics file
; int        framenum       Frame to load; 0 is first frame
; 
;
; DESCRIPTION 
;
; The GIMEX_loadframe function is a simple method of loading a frame from
; a file to a buffer.  Internally, it opens the file, determines the best
; GIMEX module to use, uses the GIMEX module to read the frame into a buffer,
; then closes the file.
; It is not recommended you use this function to read multiple frames from
; a file.
;
; 
; RETURN VALUE
;
; A GBITMAP structure containing the image and image information.
; Use the GIMEX_freebitmap function to free the GBITMAP memory.
;
;
; SEE ALSO
;
; GBITMAP, GIMEX_freebitmap, GIMEX_gopen, GIMEX_open, GIMEX_info, GIMEX_read
;
;
; EXAMPLE 
;
; none available yet
; 
; 
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; GBITMAP - Image buffer structure as returned by GIMEX_loadframe.
;
;
; SUMMARY
;
; #include "gimex.h"                Required for structure declarations
;
; typedef struct
; {
;     GINFO *ginfo;             // frame and GIMEX module info
;     char  *image;             // image buffer
;     int   rowbytes;           // bytes per row of image data
; } GBITMAP;
; 
;
; DESCRIPTION 
;
; GBITMAP provides a way to pass around GIMEX image information between functions.
; The GBITMAP structure is returned by the GIMEX_loadframe function and contains
; the raw image, frame and GIMEX module information, and the number of bytes per
; row.
;
; This structure is freed by the GIMEX_freebitmap function.
;
; 
;
; SEE ALSO
;
; GIMEX_loadframe, GIMEX_freebitmap
;
;
; EXAMPLE
;
; none available yet
;
;
; END ABSTRACT
****************************************************************************************/
#endif

GBITMAP * GCALL GIMEX_loadframe(const char *filename,int framenum)
{
    GSTREAM *g=0;
    GINFO *ginfo=0;
    GABOUT *about=0;
    GINSTANCE *gx=0;
    GBITMAP *gm=0;
    void *f=0;
    int oldtype;
    bool ok=false;

    oldtype = GIMEX_get();

    g = GIMEX_gopen(filename,0);

//    if (g)
    {
        ok = GIMEX_open(&gx,g,filename,0) > 0;
        if (ok)
        {
            about = GIMEX_about();
            if (about)
            {
                ginfo = GIMEX_info(gx,framenum);
                if (ginfo)
                {
                    f = galloc(ginfo->width*ginfo->height*ginfo->bpp/8);
                    if (f)
                    {
                        ok = GIMEX_read(gx,ginfo,(char *)f,ginfo->width*ginfo->bpp/8);
                        if (ok)
                        {
                            ok = false;
                            gm = (GBITMAP *) galloc(sizeof(GBITMAP));
                            if (gm)
                            {
                                ok = true;
                                gm->ginfo = ginfo;
                                gm->image = (char *)f;
                                gm->rowbytes = ginfo->width*ginfo->bpp/8;
                            }
                        }
                    }
                }
            }
        }
    }
    if (about)   gfree(about);
    if (gx)      GIMEX_close(gx);
    if (g)       gclose(g);

    if (!ok)
    {
        GIMEX_freebitmap(gm);
        gm = 0;
    }

    GIMEX_set(oldtype);
    return(gm);
}



