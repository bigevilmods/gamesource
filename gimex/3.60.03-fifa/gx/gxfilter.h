// (c) 1994-2004 Electronic Arts Inc.

#include "spit.h"
#include "expr.h"
#include "gopen.h"
#include "gcolour.h"
#include "crapi.h"
#include "copen.h"
#include "pshgimex.h"
#include "madgimex.h"
#include "gfilter.h"
#include "range.h"
#include "gsocket.h"
#include "hashidi.h"
#include "gxcram.h"
#include "gxopt.h"

 
static const int OPTAUTO                = 0x80000001;
static const int OPTPOSITIONMID         = 0x80000002;
static const int OPTPOSITIONMAX         = 0x80000003;
static const int OPTSCALEPERCENT        = 1;
static const int OPTSCALEBY             = 2;
static const int OPTSCALETO             = 4;
static const int OPTSCALEMAX            = 8;
static const int OPTSCALEPIXELS         = 16;
static const int OPTMIPMAPDEFAULT       = 16;   // default number of mipmaps to generate
static const int OPTMIPMAPSCALE         = 0;    // use the scale filter (point sample) to do mipmapping
static const int OPTMIPMAPBOX           = 1;    // use the halfsize filter to do mipmapping
static const int OPTMIPMAPSIZE          = 2;    // use the size filter to do mipmapping
static const int OPTMIPMAPSIZECOLREDUCE = 3;    // same as mipmaps but does global colour reduction to retain original bit depth

/****************************************************************/
/*  Public Functions                                            */
/****************************************************************/

GBITMAP **ProcessImage(GBITMAP *gbitmap, GABOUT *about, GABOUT *wabout, int i, int frame, int pass, int *pnout);
void GXFILTER_init(void);
void GXFILTER_restore(void);
void GXFILTER_reset(void);
void GXLoadPalette(int *depth,char *palfile, char* framename);
char *SPIT_getcolouropt(const char *optptr, int *optvalptr, const char *optname, int optmin, int optmax, int last);
char *  GX_getopt(int *optvalptr,const char *optptr,const char *optname, int optmin, int optmax);
char *  GX_getopt2(int *optvalptr,const char *optptr,const char *optname, int optmin, int optmax, int optdefault);
char *SPIT_getpositionopt(const char *optptr, char *optvalptr, const char *optname, int optmin, int optmax);
int GXintlog2(unsigned int l);
char *GXgetscaleopt(const char *optptr, char *optxptr, int *optxbyptr, char *optyptr, int *optybyptr, const char *optname, int enableby, int optmin, int optmax);
int GXscaleset(int width, const char *OPTscalexstr,int OPTscalexby, double sizeratio);
int GXscaledefault(int y,int height,int x,int width);
double GX_sizeratio(const char *sizepix, int pixels);
double GX_constrain(const char *exprstr, double v);
bool SignatureCompare(unsigned int signature1, unsigned int signature2);

/****************************************************************/
/*  Filter Variables                                            */
/****************************************************************/

typedef struct SavedItems
{
    int  SavedOriginalBpp;
    int  SavedOriginalNumColours;
    char SavedABits;
    char SavedRBits;
    char SavedGBits;
    char SavedBBits;
    bool did_upsample;
} SavedItems;

extern EXPR *expr;
//extern int writemsgarg;

