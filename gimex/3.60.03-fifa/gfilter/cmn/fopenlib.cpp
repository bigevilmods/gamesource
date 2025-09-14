// (c) 1995-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                    Filter Wrapper Module v1.06                   */
/*                                                                  */
/*                by Gerry Shaw and FrANK Barchard EAC              */
/*                                                                  */
/*                    Header Module - June 28, 2000                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 950510 GS initial version                                */
/*    1.01 951107 FB added N to 1 and several new filters           */
/*    1.02 980223 KO New naming convention, DLL exportable          */
/*    1.03 980225 KO Implemented FILTER_find                        */
/*    1.04 000221 FB halfsize, assert if find fails                 */
/*    1.05 000316 GS removed EZ filter support                      */
/*    1.06 010628 ID added abstracts to all common API functions    */
/*                                                                  */
/*------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include "fopen.h"

#include "fyi.h"
#include "facenter.h"
#include "facomment.h"
#include "facopy.h"
#include "fblend.h"
#include "fpsnr.h"
#include "fblur.h"
#include "fchanmap.h"
#include "fchroma.h"
#include "fcolbal.h"
#include "fcontrast.h"
#include "fcreep.h"
#include "fcrop.h"
#include "fcsmap.h"
#include "fdeband.h"
#include "fdeint.h"
#include "ftemporalfix.h"
#include "fdilate.h"
#include "fdoubled.h"
#include "fedge.h"
#include "ferode.h"
#include "fequalize.h"
#include "ffill.h"
#include "fhalfsize.h"
#include "foutline.h"
#include "fflip.h"
#include "fgamma.h"
#include "fgrb.h"
#include "fgrey.h"
#include "fgrid.h"
#include "fhsl.h"
#include "fhsv.h"
#include "finvert.h"
#include "ftedge.h"
#include "fline.h"
#include "fmedian.h"
#include "fnil.h"
#include "fpalsort.h"
#include "fpixel.h"
#include "fpreten.h"
//#include "fpsnr.h"
#include "fpsort.h"
#include "fradfill.h"
#include "frect.h"
#include "fremap.h"
#include "frot.h"
#include "frotate.h"
#include "fsafe.h"
#include "fscale.h"
#include "fshade.h"
#include "fsharpen.h"
#include "fshrink.h"
#include "fsize.h"
#include "fsmooth.h"
#include "fstereo.h"
#include "fembm.h"
#include "fswap.h"
#include "ftexture.h"
#include "fwrap.h"
#include "fxcolour.h"
#include "fzoom.h"
#include "fscreen.h"
#include "fmipmont.h"
#include "fwarp.h"
//#include "fdxt.h"
#include "funembm.h"
#include "ftranspose.h"
#include "fdrops.h"
#include "fdot3.h"
#include "fundot3.h"
#include "ftexmap.h"
#include "fmipmap.h"
#if defined(_MSC_VER)
#include "ffont.h"
#include "fphotoshop.h"
#endif

struct FILTER_FUNCTIONS
{
    FABOUT * (*FILTER_about)(void);

    int      (*FILTER_open)(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
    GINFO *  (*FILTER_info)(FINSTANCE *gx, int framenum);
    int      (*FILTER_filter)(FINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
    int      (*FILTER_close)(FINSTANCE *gx);
} filter[]=
{
 {    aboutfnil,    openfnil,    infofnil,    filterfnil,    closefnil       },
 {    aboutfyi,     openfyi,     infofyi,     filterfyi,     closefyi        },
 {    aboutacenter, openacenter, infoacenter, filteracenter, closeacenter    },
 {    aboutalphacomment,openalphacomment,infoalphacomment,filteralphacomment,closealphacomment },
 {    aboutacopy,   openacopy,   infoacopy,   filteracopy,   closeacopy      },
 {    aboutblend,   openblend,   infoblend,   filterblend,   closeblend      },
 {    aboutpsnr,    openpsnr,    infopsnr,    filterpsnr,    closepsnr       },
 {    aboutblur,    openblur,    infoblur,    filterblur,    closeblur       },
 {    aboutchanmap, openchanmap, infochanmap, filterchanmap, closechanmap    },
 {    aboutchroma,  openchroma,  infochroma,  filterchroma,  closechroma     },
 {    aboutcolbal,  opencolbal,  infocolbal,  filtercolbal,  closecolbal     },
 {    aboutcontrast,opencontrast,infocontrast,filtercontrast,closecontrast   },
 {    aboutcreep,   opencreep,   infocreep,   filtercreep,   closecreep      },
 {    aboutcrop,    opencrop,    infocrop,    filtercrop,    closecrop       },
 {    aboutcsmap,   opencsmap,   infocsmap,   filtercsmap,   closecsmap      },
 {    aboutdeband,  opendeband,  infodeband,  filterdeband,  closedeband     },
 {    aboutdeint,   opendeint,   infodeint,   filterdeint,   closedeint      },
 {    abouttemporalfix,   opentemporalfix,   infotemporalfix,   filtertemporalfix,   closetemporalfix      },
 {    aboutdilate,  opendilate,  infodilate,  filterdilate,  closedilate     },
 {    aboutdoubled, opendoubled, infodoubled, filterdoubled, closedoubled    },
 {    aboutedge,    openedge,    infoedge,    filteredge,    closeedge       },
 {    abouterode,   openerode,   infoerode,   filtererode,   closeerode      },
 {    aboutequalize,openequalize,infoequalize,filterequalize,closeequalize   },
 {    aboutfill,    openfill,    infofill,    filterfill,    closefill       },
 {    abouthalfsize,openhalfsize,infohalfsize,filterhalfsize,closehalfsize   },
 {    aboutoutline, openoutline, infooutline, filteroutline, closeoutline    },
 {    aboutflip,    openflip,    infoflip,    filterflip,    closeflip       },
 {    aboutgamma,   opengamma,   infogamma,   filtergamma,   closegamma      },
 {    aboutgrb,     opengrb,     infogrb,     filtergrb,     closegrb        },
 {    aboutgrey,    opengrey,    infogrey,    filtergrey,    closegrey       },
 {    aboutgrid,    opengrid,    infogrid,    filtergrid,    closegrid       },
 {    abouthsl,     openhsl,     infohsl,     filterhsl,     closehsl        },
 {    abouthsv,     openhsv,     infohsv,     filterhsv,     closehsv        },
 {    aboutinvert,  openinvert,  infoinvert,  filterinvert,  closeinvert     },
 {    abouttedge,   opentedge,   infotedge,   filtertedge,   closetedge      },
 {    aboutline,    openline,    infoline,    filterline,    closeline       },
 {    aboutmedian,  openmedian,  infomedian,  filtermedian,  closemedian     },
 {    aboutpalsort, openpalsort, infopalsort, filterpalsort, closepalsort    },
 {    aboutpixel,   openpixel,   infopixel,   filterpixel,   closepixel      },
 {    aboutpreten,  openpreten,  infopreten,  filterpreten,  closepreten     },
 {    aboutpsort,   openpsort,   infopsort,   filterpsort,   closepsort      },
 {    aboutradfill, openradfill, inforadfill, filterradfill, closeradfill    },
 {    aboutrect,    openrect,    inforect,    filterrect,    closerect       },
 {    aboutremap,   openremap,   inforemap,   filterremap,   closeremap      },
 {    aboutrot,     openrot,     inforot,     filterrot,     closerot        },
 {    aboutrotate,  openrotate,  inforotate,  filterrotate,  closerotate     },
 {    aboutsafe,    opensafe,    infosafe,    filtersafe,    closesafe       },
 {    aboutscale,   openscale,   infoscale,   filterscale,   closescale      },
 {    aboutshade,   openshade,   infoshade,   filtershade,   closeshade      },
 {    aboutsharpen, opensharpen, infosharpen, filtersharpen, closesharpen    },
 {    aboutshrink,  openshrink,  infoshrink,  filtershrink,  closeshrink     },
 {    aboutsize,    opensize,    infosize,    filtersize,    closesize       },
 {    aboutsmooth,  opensmooth,  infosmooth,  filtersmooth,  closesmooth     },
 {    aboutstereo,  openstereo,  infostereo,  filterstereo,  closestereo     },
 {    aboutembm,    openembm,    infoembm,    filterembm,    closeembm       },
 {    aboutswap,    openswap,    infoswap,    filterswap,    closeswap       },
 {    abouttexture, opentexture, infotexture, filtertexture, closetexture    },
 {    aboutwrap,    openwrap,    infowrap,    filterwrap,    closewrap       },
 {    aboutxcolour, openxcolour, infoxcolour, filterxcolour, closexcolour    },
 {    aboutzoom,    openzoom,    infozoom,    filterzoom,    closezoom       },
 {    aboutscreen,  openscreen,  infoscreen,  filterscreen,  closescreen     },
 {    aboutmipmont, openmipmont, infomipmont, filtermipmont, closemipmont    },
 {    aboutwarp,    openwarp,    infowarp,    filterwarp,    closewarp       },
//{    aboutdxt,     opendxt,     infodxt,     filterdxt,     closedxt       },
 {    aboutunembm,  openunembm,  infounembm,  filterunembm,  closeunembm     },
 {    abouttrans,   opentrans,   infotrans,   filtertrans,   closetrans      },
 {    aboutdrops,   opendrops,   infodrops,   filterdrops,   closedrops      },
 {    aboutdot3,    opendot3,    infodot3,    filterdot3,    closedot3       },
 {    aboutundot3,  openundot3,  infoundot3,  filterundot3,  closeundot3     },
 {    abouttexmap,  opentexmap,  infotexmap,  filtertexmap,  closetexmap     },
 {    aboutmipmap,  openmipmap,  infomipmap,  filtermipmap,  closemipmap     },
#if defined(_MSC_VER)
 {    aboutfont,    openfont,    infofont,    filterfont,    closefont       },
 {    aboutphotoshop, openphotoshop, infophotoshop, filterphotoshop, closephotoshop    },
#endif

 {    NULL,         NULL,        NULL,        NULL,          NULL            }
};


// borrowed from gopenlib.c
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

/****************************************************************/
/*  Set filter function pointers based on number                */
/****************************************************************/

/*!
 * \file  fopenlib.cpp
 * \brief Lib version of the Fopen interface
 */

static int currentfilter = 0;

#if 0
/****************************************************************************************
; ABSTRACT
;
; FILTER_init - Load FILTER library.
;
;
; SUMMARY
;
; #include "gfilter.h"              Required for structure declarations
; #include "fopen.h"                Required for function declarations
; #include "fopen.c"                Required for GFILTER DLL
;
; int FILTER_init();
;
;
; DESCRIPTION
;
; The FILTER_init function loads the FILTER library and must be called prior
; to using any FILTER function.
; If the application is loading the FILTER from a DLL, "fopen.c" should be
; included as it contains the FILTER_init function for DLL loading.
; "fopen.c" is not needed if the application is statically linking the
; FILTER library.
;
; To unload the FILTER library, call FILTER_restore.
;
;
; RETURN VALUE
;
; Non-zero if successful.
; possible causes of failure: cannot connect with FILTER DLL
;
; SEE ALSO
;
; FILTER_restore
;
;
; EXAMPLE
;
; filthe.c
; filthe.h
;
; END ABSTRACT
****************************************************************************************/
#endif

int GCALL FILTER_init(void)
{
    return(1);
}


#if 0
/****************************************************************************************
; ABSTRACT
;
; FILTER_restore - Unload FILTER library.
;
;
; SUMMARY
;
; #include "gfilter.h"           Required for structure declarations
; #include "fopen.h"             Required for function declarations
; #include "fopen.c"             Required for FILTER DLL
;
; void FILTER_restore();
;
;
; DESCRIPTION
;
; The FILTER_restore function unloads the FILTER library after a call to
; FILTER_init.
;
;
; SEE ALSO
;
; FILTER_init
;
;
; EXAMPLE
;
; filthe.c
; filthe.h
;
; END ABSTRACT
****************************************************************************************/
#endif

void GCALL FILTER_restore(void)
{
}


#if 0
/****************************************************************************************
; ABSTRACT
;
; FILTER_about - Provide information about Filter module and
;                options type, purpose, and valid ranges.
;
;
; SUMMARY
;
; #include "gfilter.h"           Required for structure declarations
; #include "fopen.h"             Required for function declarations
; #include "fopen.c"             Required for FILTER DLL
;
; FABOUT *FILTER_about();
;
;
; DESCRIPTION
;
; The FILTER_about function returns information about the Filter module
; capabilities and requirements, as well as options information.
; it may be called any time after the FILTER module has been set
; (see FILTER_open and FILTER_find).
; The caller is responsible for freeing the memory allocated for the
; FABOUT structure with the gfree function.
; The FABOUT structure also contains information on each option.
; Each option is listed with the following information:
;    - its type (FOP_FLOAT, FOP_INT, FOP_PTR, FOP_ARGB)
;    - its min value
;    - its max value
;    - its recommended default value.
;    - a short character string describing the option in question.
; This way a user can query the abilities of the module.
;
;
; RETURN VALUE
;
; Pointer to FABOUT structure.
; NULL on failure.
;
; SEE ALSO
;
; FABOUT
;
;
; EXAMPLE
;
; fenum.c
; fenum.h
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; FABOUT - About Filter Module Structure.
;
;
; SUMMARY
;
; #include "gfilter.h"                Required for structure declarations
;
; typedef struct
; {
;     int signature;                  // gfilter module signature
;     int size;                       // size of the FABOUT structure
;     int version;                    // version of the FABOUT structure ie 104
;     unsigned int filter8    :1;     // supports processing 8-bit images
;     unsigned int filter32   :1;     // supports processing 32-bit images
;     unsigned int multisize  :1;     // supports different size source images
;     unsigned int pad        :13;    // pad to 32 bits
;     unsigned int pad2       :16;    // pad to 32 bits
;     int  minbitmaps;                // min # of sources gfilter requires
;     int  maxbitmaps;                // max # of sources gfilter requires
;     int  canoverwritesource;        // supports passing dst buffer same as src buffer
;     int  filterclass;               // kind of gfilter (0 general purpose)
;     char authorstr[32];             // gfilter module author
;     char versionstr[8];             // version number of gfilter module ie 1.00
;     char namestr[16];               // single word name of the gfilter
;     char longnamestr[32];           // full name of the gfilter
;     int  numoptions;                // maximum number of options gfilter can take
;     FOPTION option[FMAXOPTIONS];    // gfilter arguments
; } FABOUT;
;
;
; DESCRIPTION
;
; The FABOUT structure describes the characteristics of the Filter module and options that
; filter in question takes. The structure is filled out by the FILTER_about(), which may be called at any time,
; as it is not based on a particular file. The caller must free the structure useing the gfree().
;
;
; SEE ALSO
;
; FILTER_about
;
;
; EXAMPLE
;
; fenum.c
; fenum.h
;
; END ABSTRACT
****************************************************************************************/
#endif


FABOUT* GCALL FILTER_about(void)
{
    if (filter[currentfilter].FILTER_about)
        return(filter[currentfilter].FILTER_about());
    return((FABOUT*)NULL);
}


#if 0
/****************************************************************************************
; ABSTRACT
;
; FILTER_open - Open a filter and setup FXINSTANCE structure for work on current image.
;
;
; SUMMARY
;
; #include "gfilter.h"           Required for structure declarations
; #include "fopen.h"             Required for function declarations
; #include "fopen.c"             Required for GFILTER DLL
;
; int FILTER_open(fx, numsources, sources, numoptions, options)
; FINSTANCE **   fx         Pointer to your FINSTANCE handle. This will be initialized
;                           by the module.
; int            numsources Number of images in your GBITMAP sources array.
; const GBITMAP *sources    Array of GBITMAP structures which contain the image data,
; int            numoptions Number of entries in your options array.
;                           the image ginfo and the rowbytes(pitch) of the image.
; const FOP *    options    Array of FOP unions. Each FOP entry is a union of int
;                           float, char *, or ARGB struct These contain the parameters
;                           for the module.
;
; DESCRIPTION
;
; The FILTER_open function opens a given filter module and initializes
; the passed in FINSTANCE handle for manipulation of the source bitmaps.
; The FINSTANCE handle is used in subsequent calls to gfilter functions
; and contains all options, and source bitmaps in a module specific
; format. Most modules use only one source, although some such as
; the compositor uses several.
;
;
;
; RETURN VALUE
;
; returns: 0 on failure
;          1 on success
; possible causes of failure include: - not being able to allocate enough memory
;
;
; SEE ALSO
;
; FILTER_close, FINSTANCE
;
;
; EXAMPLE
;
; filthe.c
; filthe.h
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; FOP - filter options union.
;
;
; SUMMARY
;
; #include "gfilter.h"                Required for structure declarations
;
; Option types
;
; #define FOP_INT         0x0001      // default num gadget
; #define FOP_FLOAT       0x0002      // default double gadget
; #define FOP_PTR         0x0003      // default is block of memory
;
;
; FOP structure describes gfilter options
;
; typedef union
; {
;     int    i;     // FOP_INT
;     double  f;    // FOP_FLOAT
;     char *  p;    // FOP_PTR
;     ARGB    argb; // FOP_INT | FOP_ARGB
; } FOP;
;
;
; DESCRIPTION
;
; The FOP union is used in the FOPTION structure to allow any type to be used
; for each option.
;
; SEE ALSO
;
; FOPTION, FILTER_open
;
;
; EXAMPLE
;
; fenum.c
; fenum.h
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; FOPTION - filter options structure.
;
;
; SUMMARY
;
; #include "gfilter.h"                Required for structure declarations
;
; Option modifiers
;
; #define FOP_ENUM        0x0200      // option has enumerated types
; #define FOP_FPALETTE    0x0400      // mask with FOP_PTR to make a FPALETTE structure
; #define FOP_TRUE_COLOR  0x0800      // mask with FOP_INT
; #define FOP_INDEX_COLOR 0x1000      // mask with FOP_INT
;
;
; typedef struct
; {
;     int        type;                // type of option (ie: FOP_INT)
;     FOP         min;                // minimum value
;     FOP         max;                // maximum value
;     FOP         def;                // default value
;     char        name[32];           // name of the option
;     char        enumlist[256];      // enumerated list of values starting at 0
; } FOPTION;
;
;
; DESCRIPTION
;
; The FOPTION structure is used for passing in parameters to a FILTER module.
; it descibes each object, it's type and use.
;
; SEE ALSO
;
; FOP, FILTER_open
;
;
; EXAMPLE
;
; fenum.c
; fenum.h
;
; END ABSTRACT
****************************************************************************************/
#endif


int GCALL FILTER_open(FINSTANCE **fx, int numsources, const GBITMAP *sources, int numoptions, const FOP *options)
{
    if (filter[currentfilter].FILTER_open)
        return(filter[currentfilter].FILTER_open(fx, numsources, (GBITMAP*)(sources), numoptions, (FOP*)(options)));
    return(0);
}
#if 0
/****************************************************************************************
; ABSTRACT
;
; FILTER_info  - Retrieve info about the resultant image of a filter operation
;
;
; SUMMARY
;
; #include "gfilter.h"           Required for structure declarations
; #include "fopen.h"             Required for function declarations
; #include "fopen.c"             Required for FILTER DLL
;
; GINFO *FILTER_info(fx,framenum)
; FINSTANCE *fx                FILTER module instance
; int framenum                 Frame number from which to retrieve info
;
;
; DESCRIPTION
;
; The FILTER_info function creates a new ginfo structure for the resultant
; image of the filter operation. This is called before a call to FILTER_filter()
; but after a call to FILTER_find().
; Typically a user calls FILTER_info then uses the information garnered from
; the returned ginfo structure to build a destination data buffer, where they would
; like the filtered image to be placed. The user is responsible for freeing
; the ginfo structure themselves with a call to gfree().
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
; FILTER_filter
; GINFO
;
; EXAMPLE
;
; filthe.c
; filthe.h
;
; END ABSTRACT
****************************************************************************************/
#endif

GINFO * GCALL FILTER_info(FINSTANCE *fx, int framenum)
{
    if (filter[currentfilter].FILTER_info)
        return(filter[currentfilter].FILTER_info(fx, framenum));
    return((GINFO*)NULL);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; FILTER_filter  - filter the input images and place result into output buffer.
;
;
; SUMMARY
;
; #include "gfilter.h"           Required for structure declarations
; #include "fopen.h"             Required for function declarations
; #include "fopen.c"             Required for FILTER DLL
;
; int FILTER_filter(FINSTANCE *fx, const GINFO *ginfo, void *dest, int rowbytes)
; FINSTANCE *fx                FILTER module instance
; const GINFO *ginfo           Resultant Image frame information
;                               (retrieved from a call to FILTER_info)
; void *dest                   Destination image buffer
; int rowbytes                 Bytes of image data per row (pitch)
;
;
; DESCRIPTION
;
; The FILTER_filter function uses the current FILTER module to filter
; a given frame into a frame buffer provided by the user. The FINSTANCE
; is provided by a call to FILTER_open and contains the source information
; for running the filter. All other parameters are for the destination
; image. The GINFO structure is provided by a call to FILTER_info, allowing
; the user to create the destination buffer (using width, height and bpp) and
; the rowbytes of the new image.
; The destination frame buffer is allocated by the user at a size of
; (GINFO.width * GINFO.height * GINFO.bpp/8)
; The rowbytes argument is (GINFO.width * GINFO.bpp/8)
;
;
; RETURN VALUE
;
; A non-zero value is returned if successful.
; possible causes of failure: options are out of range, not able to allocate
;                             memory, module does not support bit depth.
;
; SEE ALSO
;
; FILTER_info
;
;
; EXAMPLE
;
; filthe.c
; filthe.h
;
; END ABSTRACT
****************************************************************************************/
#endif

int GCALL FILTER_filter(FINSTANCE *fx, const GINFO *ginfo, void *dest, int rowbytes)
{
    if (filter[currentfilter].FILTER_filter)
        return(filter[currentfilter].FILTER_filter(fx, (GINFO*)(ginfo), (char *)dest, rowbytes));
    return(0);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; FILTER_close  - Close a FILTER instance.
;
;
; SUMMARY
;
; #include "gfilter.h"           Required for structure declarations
; #include "fopen.h"             Required for function declarations
; #include "fopen.c"             Required for FILTER DLL
;
; int FILTER_close(gx)
; FINSTANCE     *gx                Filter module instance
;
;
; DESCRIPTION
;
; The FILTER_close function takes a FILTER instance
; structure created by the FILTER_open function.
;
; FILTER_close should be called once all filtering
; of the image is done.
;
;
; RETURN VALUE
;
; A non-zero is returned if successful.
;
;
; SEE ALSO
;
; FILTER_open, FILTER_info
;
;
; EXAMPLE
;
; filthe.c
; filthe.h
;
; END ABSTRACT
****************************************************************************************/
#endif

int GCALL FILTER_close(FINSTANCE *gx)
{
    if (filter[currentfilter].FILTER_close)
        return(filter[currentfilter].FILTER_close(gx));
    return(0);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; FILTER_find - Set the current FILTER module from a filter name.
;
;
; SUMMARY
;
; #include "gfilter.h"           Required for structure declarations
; #include "fopen.h"             Required for function declarations
; #include "fopen.c"             Required for FILTER DLL
;
; int FILTER_find(const char *name)
; const  char *name         name of the module in question.
;
;
; DESCRIPTION
;
; The FILTER_find function sets the current FILTER module using the closest
; match to a filter name.  For example, FILTER_find("drops") would
; use the DropS Filter module.
;
;
; RETURN VALUE
;
; The enumerated value of the new FILTER module, 0 if failed.
; possible causes for failure: no filter module with requested name.
;
; SEE ALSO
;
; FILTER_set, FILTER_get, FILTER_open, GINFO
;
;
; EXAMPLE
;
; filthe.c
; filthe.h
; fenum.c
; fenum.h
;
; END ABSTRACT
****************************************************************************************/
#endif

int GCALL FILTER_find(const char *name)
{
    int besttype=0;
    bool found=false;

    for (int i=0; (i < FILTER_count()) && !found; ++i)
    {
        FILTER_set(i);
        FABOUT *about = FILTER_about();
        if (gstricmp(about->namestr, name) == 0)
        {
            besttype = i;
            found = true;
        }
        gfree(about);
    }
#ifdef DEBUG
    if (!found)
        printf("WARNING: filter not found '%s'\n",name);
#endif

    FILTER_set(besttype);
    return(besttype);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; FILTER_set - Set the current FILTER module using enumerated value.
; FILTER_get - Get the enumerated value of the current FILTER module.
;
;
; SUMMARY
;
; #include "gfilter.h"           Required for structure declarations
; #include "fopen.h"             Required for function declarations
; #include "fopen.c"             Required for FILTER DLL
;
; void FILTER_set(typenum)
; int  FILTER_get()
; int typenum                Enumerated value of FILTER module
;
;
; DESCRIPTION
;
; The FILTER_set function sets the current FILTER module to that specified
; by an enumerated value.  This value should be obtained using the
; FILTER_get function.  It is also possible to loop through all FILTER
; modules from 0 (NULL module) to FILTER_count - 1.
;
; The FILTER_get function returns the enumerated value of the current
; FILTER module.  This value should not be assumed to remain constant between
; runtime sessions, particularly when using FILTER DLL.
;
;
; RETURN VALUE
;
; FILTER_get - An integer value specifying the current FILTER module, 0 if failed.
;
;
; SEE ALSO
;
; FILTER_find, FILTER_count
;
;
; EXAMPLE
;
; fenum.c
; fenum.h
;
; END ABSTRACT
****************************************************************************************/
#endif

void GCALL FILTER_set(int typenum)
{
    if (typenum >= FILTER_count() || typenum < 0)
        typenum = 0;

    currentfilter = typenum;
}

int GCALL FILTER_get(void)
{
    return(currentfilter);
}
#if 0
/****************************************************************************************
; ABSTRACT
;
; FILTER_count - Get the number of FILTER modules.
;
;
; SUMMARY
;
; #include "gfilter.h"           Required for structure declarations
; #include "fopen.h"             Required for function declarations
; #include "fopen.c"             Required for FILTER DLL
;
; int FILTER_count()
;
;
; DESCRIPTION
;
; The FILTER_count function returns the number of FILTER modules.
; It is possible to loop through all FILTER modules from 0 (NULL module)
; to FILTER_count - 1.
;
;
; RETURN VALUE
;
; The number of available FILTER modules, including the NULL module.
;
;
; SEE ALSO
;
; FILTER_set, FILTER_get, FILTER_find
;
;
; EXAMPLE
;
; fenum.c
; fenum.h
;
; END ABSTRACT
****************************************************************************************/
#endif

int GCALL FILTER_count(void)
{
    int num=0;

    while(filter[num].FILTER_about != NULL)
    {
        num++;
    }

    return(num);
}

