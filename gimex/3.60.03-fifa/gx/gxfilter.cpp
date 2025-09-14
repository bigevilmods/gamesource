// (c) 1994-2004 Electronic Arts Inc.

#define GFILTER_STATIC 1

// for thread priority
#if defined(_MSC_VER)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"
#endif

#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "fopen.h"
#include "gfilter.h"
#include "gcolour.h"
#include "gxfilter.h"
#include "fblend.h"
#include "expr.h"
#include "gxcram.h"
#include "gxinfo.h"
// ranger is part of filter lib
#include "ranger.h"
#include "shpgimex.h"

#include "wildname.h"
//#include "remap.c"

extern "C"
{
    extern int FUverbose;
}

#if defined(_MSC_VER)
/* gxfilter.c(5725) : warning C4505: 'getnextpaletteframe' : unreferenced local function has been removed */
#pragma warning(disable: 4505)
#endif


#include <math.h>

extern unsigned char GIMEX_destcolourmask[256];

int EXPRminx;
int EXPRmaxx;
int EXPRminy;
int EXPRmaxy;
int EXPRmaxwidth;
int EXPRmaxheight;
int ORIGmaxwidth;
int ORIGmaxheight;

void ORIGUpdate(GINFO *ginfo);
static void UpdateExpr(GINFO *ginfo,GBITMAP *gm,GBITMAP *gaux,int frame);
static void GX_choptrailingwhitespace(char * str);
//GINFO DropShadowCommonClut;


/****************************************************************/
/*  Do stuff to the image                                       */
/****************************************************************/

static int composite_pass=-1;
static int composite_maxframe=0; // max
static int composite_currentframe=0; // current

static char *composite_f=0;
static GINFO *composite_ginfo=0;
static int composite_rowbytes=0;

static int auxillary_pass=-1;
static int auxillary_maxframe=0; // max
static int auxillary_currentframe=0; // current

static char *auxillary_f=0;
static GINFO *auxillary_ginfo=0;
static int auxillary_rowbytes=0;

// for creating facial texture maps.
// have seperate variables just in case
// user tries to do a flatten at the
// same time.
static int facemap_pass=-1;
static int facemap_maxframe=0; // max
static int facemap_currentframe=0; // current

static char *facemap_f=0;
static GINFO *facemap_ginfo=0;
static int facemap_rowbytes=0;


extern int errorlevel;






#define GXFILTERMAKEFOURCC(A,B,C,D) ((int)(((A)<<24)&0xFF000000)|(int)(((B)<<16)&0x00FF0000)|(int)(((C)<<8)&0x0000FF00)|(int)(((D)<<0)&0x000000FF))
typedef struct
{
    GINFO ginfo;
    int   blendmethod;
    int   visibleflag;
    int   opacitylevel;

// PAULDING START
    int canvasw;
    int canvash;
// PAULDING END
}GINFOPSD;


//----------------------------------------------------------------
// translate from PSD fourcc blendmethod codes to gx codes.
int GetPSDBlendMethod(int blendmethod)
{
    int blendcode = 0;          // set to default alpha blend.
    if(GXFILTERMAKEFOURCC('d','a','r','k') == blendmethod)
        blendcode = 2;          // set to sub mode.
    if(GXFILTERMAKEFOURCC('l','i','t','e') == blendmethod)
        blendcode = 3;          // set to additive mode.
    if(GXFILTERMAKEFOURCC('d','i','f','f') == blendmethod)
        blendcode = 4;          // set to remove mode.
    if(GXFILTERMAKEFOURCC('m','u','l',' ') == blendmethod)
        blendcode = 7;          // set to multiply mode.
    if(GXFILTERMAKEFOURCC('s','c','r','n') == blendmethod)
        blendcode = 8;          // set to screen mode.
    if(GXFILTERMAKEFOURCC('o','v','e','r') == blendmethod)
        blendcode = 9;          // set to overlay mode.
    return(blendcode);
}

//----------------------------------------------------------------
// return true if the destination file format is a PS2 shape file
bool GX_isSony(GABOUT * wabout)
{
       return (
                SignatureCompare(wabout->signature,GMAKEID('S','H','P','S')) ||
                SignatureCompare(wabout->signature,GMAKEID('S','h','p','S'))
              );
}

// ------------------------------------------------------------
bool IsGstFourCC(unsigned int packed)
{
    if((packed&0xFFFFFF00) == 0x47535400) // if the fourCC == 'GST '
        return true;
    return false;
}

/****************************************************************/
/*  Get Next Frame in Composite File                            */
/****************************************************************/

static int compositeindex=0;

static GBITMAP *getnextcompositeframe(void)
{
    GBITMAP *gm=0;

    if (OPTcompositefile[0])
    {
        if (OPTverbose>=4)
            gprintf("Loading composite file '%s', frame %d\n",OPTcompositefile,compositeindex);

        gm = GIMEX_loadframe(OPTcompositefile,compositeindex);
        if (!gm && compositeindex)
        {
            compositeindex=0;
            gm = GIMEX_loadframe(OPTcompositefile,compositeindex);
        }
        if (gm)
            ++compositeindex;
        else
        {
            if (OPTverbose)
            {
                gprintf("GX - ERROR READING COMPOSITE FILE %s\n",OPTcompositefile);
            }
        }

    }
    return(gm);
}

/****************************************************************/
/*  Get Next Frame in Auxillary File                            */
/****************************************************************/

static int auxillaryindex=0;

static GBITMAP *getnextauxillaryframe(void)
{
    GBITMAP *gaux=0;

    if (OPTauxillaryfile[0])
    {
        if (OPTverbose>=5)
            gprintf("loading auxillary file '%s'\n",OPTauxillaryfile);

        gaux = GIMEX_loadframe(OPTauxillaryfile,auxillaryindex);
        if (!gaux && auxillaryindex)
        {
            auxillaryindex=0;
            gaux = GIMEX_loadframe(OPTauxillaryfile,auxillaryindex);
        }
        else
            ++auxillaryindex;
    }
    return(gaux);
}


/****************************************************************/
/*  Get Next Frame in Palette File                              */
/****************************************************************/

static int paletteindex=0;
static GBITMAP *getnextpaletteframe(void)
{
    GBITMAP *gm=0;

    if (OPTpalettefile[0])
    {
        if (OPTverbose>=5)
            gprintf("loading palette file '%s'\n",OPTpalettefile);

        gm = GIMEX_loadframe(OPTpalettefile,paletteindex);
        if (!gm && paletteindex)
        {
            paletteindex=0;
            gm = GIMEX_loadframe(OPTpalettefile,paletteindex);
        }
        else
            ++paletteindex;
    }
    return(gm);
}

/****************************************************************/
/*  Load a Palette from a File                                  */
/****************************************************************/

void GXLoadPalette(int *depth,char *palfile, char *framename)
{
    GBITMAP *gm = 0;
    GINFO *ginfo;
    GINSTANCE     *gx = 0;
    GSTREAM       *g = 0;
    int numframes = 0;
    int notfound = 1;
    void *f=0;
    int framenumber = 0;

    g = GIMEX_gopen(palfile, NULL);
    numframes = GIMEX_open(&gx, g, palfile, 1);
    if (numframes)
    {
        int i = 0;
        framenumber = atoi(framename);
        // if it was a string (ie actual framename) and it wasnt '0'
        if (!framenumber && (strcmp(framename, "0") != 0))
        {
            while ((i < numframes) && notfound )
            {
                ginfo = GIMEX_info(gx, i);
                if (!strcmp(ginfo->framename, framename))
                {
                    f = (char *) galloc(ginfo->width*ginfo->height*ginfo->bpp/8);
                    GIMEX_read(gx,ginfo,(char *)f,ginfo->width*ginfo->bpp/8);
                    gm = (GBITMAP *) galloc(sizeof(GBITMAP));
                    if (gm)
                    {
                        gm->ginfo = ginfo;
                        gm->image = (char *)f;
                        gm->rowbytes = ginfo->width*ginfo->bpp/8;
                    }
                    notfound = 0;
                }
                else if (ginfo) gfree(ginfo);

                i++;
            }
        }
        // it really was a number
        if (notfound)
        {
            gm = GIMEX_loadframe(palfile,framenumber);
        }

        if (gx)      GIMEX_close(gx);
        if (g)       gclose(g);

    }

    if (gm)
    {
        ginfo=gm->ginfo;
        if (ginfo->bpp>8)
        {
            if (OPTverbose)
                gprintf("gx - palette file <%s> is not 8 bit\n",palfile);
        }
        else
        {
            memset(OPTpalfilecolourtbl,0,sizeof(OPTpalfilecolourtbl));
            memcpy(OPTpalfilecolourtbl,ginfo->colourtbl,ginfo->numcolours*sizeof(ARGB));
            indexhistogram(gm, OPTpalfilehistogram);
            if (ginfo->numcolours < 256)
                memset(&OPTpalfilehistogram[ginfo->numcolours],0,(256 - ginfo->numcolours)*sizeof(ARGB));

            if (!*depth)
                *depth = ginfo->originalbpp;
        }
        GIMEX_freebitmap(gm);
   }
}

/****************************************************************/
/*  Palette Copy                                                */
/****************************************************************/

typedef enum { GXPALFUNC_COPY, GXPALFUNC_MOVE, GXPALFUNC_SWAP } PALFUNC;

void GX_palfunc(PALFUNC palfunc,char *OPTpalcopydest,char *OPTpalcopysource, GINFO *ginfo,GBITMAP *gm,GBITMAP *gaux)
{
    RANGER *r;
    double fnext;
    ARGB *d;
    ARGB *s;
    ARGB t;
    int di;
    d = &ginfo->colourtbl[0];
    s = &ginfo->colourtbl[0];

    if (OPTauxillaryfile[0])
        s = &gaux->ginfo->colourtbl[0];

    di = (int) EXPR_get(expr,OPTpalcopydest);
    r = RANGER_open(expr, OPTpalcopysource, 0.0, 256.0);
    while (RANGER_next(r,&fnext))
    {
        int si;
        si = (int) (fnext+0.5);
        switch (palfunc)
        {
            case GXPALFUNC_COPY:
                d[di] = s[si];
                if (OPTverbose>=4)
                    gprintf("copy %d to %d\n",si,di);
                break;

            case GXPALFUNC_MOVE:
                d[di] = s[si];
                s[si].a = s[si].r = s[si].g = s[si].b = (unsigned char) 0;
                if (OPTverbose>=4)
                    gprintf("move %d to %d\n",si,di);
                break;

            case GXPALFUNC_SWAP:
                t = d[di];
                d[di] = s[si];
                s[si] = t;
                if (OPTverbose>=4)
                    gprintf("swap %d to %d\n",si,di);
                break;
        }

        ++di;
    }
    if (r)
        RANGER_close(r);
}








/****************************************************************/
/*  Palette Blend                                               */
/****************************************************************/

static int ColourAvoid(int i, int transparentindex, int allowtrans)
{
    int avoid = 0;

    if (OPTavoid[0])        avoid  = RANGE_match(OPTavoid,(char *)0,i);
    if (!allowtrans)
    if (OPTtransparent[0])  avoid |= RANGE_match(OPTtransparent,(char *)0,i);
    if (OPTtranslucient[0]) avoid |= RANGE_match(OPTtranslucient,(char *)0,i);
    if (OPTopaque[0])       avoid |= RANGE_match(OPTopaque,(char *)0,i);
    if (OPTsetcolour[0])    avoid |= RANGE_match(OPTsetcolour,(char *)0,i);
    if (OPTblack[0])        avoid |= RANGE_match(OPTblack,(char *)0,i);
    if (i==transparentindex) avoid = 1;

    return(avoid);
}

static int ColourStatic(int i)
{
    int avoid = 0;

    if (OPTtransparent[0])  avoid  = RANGE_match(OPTtransparent,(char *)0,i);
    if (OPTtranslucient[0]) avoid |= RANGE_match(OPTtranslucient,(char *)0,i);
    if (OPTopaque[0])       avoid |= RANGE_match(OPTopaque,(char *)0,i);
    if (OPTsetcolour[0])    avoid |= RANGE_match(OPTsetcolour,(char *)0,i);
    if (OPTblack[0])        avoid |= RANGE_match(OPTblack,(char *)0,i);

    return(avoid);
}


static int PaletteCountUsed(ARGB *pal,int *hist,int colours)
{
    int i;
    int numused=0;

    for (i=0; i<colours; ++i)
    {
        if (pal[i].a || pal[i].r || pal[i].g || pal[i].b || hist[i]>1 || ColourStatic(i))
            ++numused;
    }
    return(numused);
}

static int PaletteCountUnavoided(int colours)
{
    int i;
    int avoid;
    int numunavoided=0;

    for (i=0; i<colours; ++i)
    {
        avoid = 0;
        if (OPTavoid[0])        avoid  = RANGE_match(OPTavoid,(char *)0,i);
        if (!avoid)
            ++numunavoided;
    }
    return(numunavoided);
}



static int ColourAvail(ARGB *pal,int *hist,int i,int transparentindex, int allowtrans)
{
    int avoid;
    int avail;

    /* includes values where the histogram is == 1 so .act files can be used to remap images */
    avail = (!pal[i].a && !pal[i].r && !pal[i].g && !pal[i].b) && !hist[i];
//    avail = (!pal[i].a && !pal[i].r && !pal[i].g && !pal[i].b) || hist[i]<1;
    avoid = ColourAvoid(i, transparentindex, allowtrans);

    return(avail && !avoid);
}

// compare signatures (fourcc codes).
// signature1 last 3 letters will be mapped to uppercase
// signature2 can have low byte 0 to ignore that byte

bool SignatureCompare(unsigned int signature1, unsigned int signature2)
{
    bool sameSig=false;

    if (signature1==GMAKEID('S','H','P','I'))   // Match SHPI to ShpF
        signature1 = GMAKEID('S','H','P','F');

    if (signature2 & 0xff)
    {
        // and with 0xdf converts lower to upper case. 'ShpF'&0xdfdfdfdf -> 'SHPF'
        if ((signature1 & 0xffdfdfdf) == (signature2 & 0xffdfdfdf))
            sameSig=true;
    }
    else if ((signature1 & 0xffdfdf00) == (signature2 & 0xffdfdf00))
        sameSig=true;
    return sameSig;
}


static int besttransparentindex(ARGB *staticpal,int *hist,char *OPTtransparent,char *OPTavoid,int colours,GABOUT *wabout)
{
    int transparentindex=-1;
    int i;
    int v;
    int best=0;
    int preferredindex=0;

    if (colours==256
    && (SignatureCompare(wabout->signature,GMAKEID('S','H','P','I')) // FSH
     || SignatureCompare(wabout->signature,GMAKEID('T','G','V','i')))) // TGV
    {
        preferredindex = 255;
    }

    for (i=0; i<colours; ++i)
    {
        v = 0;
        if (!staticpal[i].a && !staticpal[i].r && !staticpal[i].g && !staticpal[i].b)
            v += 16;
        if (ColourAvail(staticpal,hist,i,-1,1))
            v += 16;
        if (OPTtransparent[0])
            if (RANGE_match(OPTtransparent,(char *)0,i))
                v += 8;
        if (OPTavoid[0])
            if (!RANGE_match(OPTavoid,(char *)0,i))
                v += 4;
        if (OPTblack[0])
            if (!RANGE_match(OPTblack,(char *)0,i))
                v += 4;
        if (OPTtranslucient[0])
            if (!RANGE_match(OPTtranslucient,(char *)0,i))
                v += 4;
        if (OPTopaque[0])
            if (!RANGE_match(OPTopaque,(char *)0,i))
                v += 4;
        if (OPTsetcolour[0])
        {
            /* If user explicitly sets alpha to 0 then this is probably
               the best transparent index. If user isn't setting this
               colour then just add 4. */
            if (!RANGE_match(OPTsetcolour,(char *)0,i))
                v += 4;
            else if ((OPTsetcolourmask & 0xff000000) && !(OPTsetcolourvalue & 0xff000000))
                v += 32;
        }
        if (i==preferredindex)
            v += 2;
        if (i==0 || i==15 || i==16 || i==255)
            v += 1;

        if (v>best)
        {
            best = v;
            transparentindex = i;
        }
    }
    return(transparentindex);
}

#define SETCOL(c,v,m,s) (unsigned char)((((unsigned int)m>>s)&~(((unsigned int)m)>>s)) | ((unsigned int)(v&m)>>s))

static int MergePalettes(ARGB *palout, unsigned char *maskout, ARGB *staticpal, int *hist, ARGB *dynamicpal, int transparentindex, ARGB chromacolour)
{
    ARGB pal[256];
    int i=0;
    int j=0;
    int last=0;

    memcpy(pal,staticpal,sizeof(pal));
    memset(maskout, 0, 256);

    for (j=0; j<256; ++j)
    {
        if (!ColourAvail(pal,hist,j,transparentindex,0) && j<256)
        {
            if (OPTsetcolour[0])
                if (RANGE_match(OPTsetcolour,(char *)0,j))
                {
                    pal[j].a = SETCOL(pal[j].a,OPTsetcolourvalue,OPTsetcolourmask,24);
                    pal[j].r = SETCOL(pal[j].r,OPTsetcolourvalue,OPTsetcolourmask,16);
                    pal[j].g = SETCOL(pal[j].g,OPTsetcolourvalue,OPTsetcolourmask,8);
                    pal[j].b = SETCOL(pal[j].b,OPTsetcolourvalue,OPTsetcolourmask,0);
                }
            if (OPTopaque[0])
                if (RANGE_match(OPTopaque,(char *)0,j))
                    pal[j].a = 255;
            if (OPTblack[0])
                if (RANGE_match(OPTblack,(char *)0,j))
                {
                    pal[j].a = 255;
                    pal[j].r = 0;
                    pal[j].g = 0;
                    pal[j].b = 0;
                }
            if (j==transparentindex)
                pal[j] = chromacolour;
            if (OPTtransparent[0])
                if (RANGE_match(OPTtransparent,(char *)0,j))
                {
                    pal[j] = chromacolour;
                }

            if (OPTtranslucient[0])
                if (RANGE_match(OPTtranslucient,(char *)0,j))
                    pal[j].a = 128;

            if (!OPTavoid[0] || !RANGE_match(OPTavoid,(char *)0,j))
            {
                /* Allow the remap to use this colour */
                maskout[j] = 1;
                last = j;
            }
        }
        else /* free spot to put next dynamic colour */
        {
            /* find next entry to put into pal */
            while (!(dynamicpal[i].a || dynamicpal[i].r || dynamicpal[i].g || dynamicpal[i].b) && i<256)
            {
                ++i;
            }

            if (i<256)
            {
                /* Allow the remap to use this colour */
                maskout[j] = 1;
                last = j;
                pal[j] = dynamicpal[i++];
            }
        }
    }

    memcpy(palout,pal,sizeof(pal));
    return(last+1);
}

/****************************************************************/
/*  Misc                                                        */
/****************************************************************/

#if 0
// works, but GX needs portibility more than performance
int intlog2(int n)
{
    int t;
    *(float *)&t = (float) n;
    return((t>>23)-127);        // the exponent of a float is log2 of the number
}
#endif

// note floors to int.  To bump to next power of 2, use MAKEPOWEROF2()

int GXintlog2(unsigned int l)
{
    if(l != 0)                  // don't allow someone to do a log of zero some machines can't handle the floating point divide by zero.
        return((int)(log((double)l)/log(2.0f)));
    else
        return(0);
}

#define MAKEPOWEROF2(x) (1<<(GXintlog2((x)-1)+1))


double GXexprmin=-4294967296.0;
const double GXexprmax=4294967296.0;

double GX_constrain(const char *exprstr, double v)
{
    RANGER *r;

    EXPR_set(expr,"powerof2",MAKEPOWEROF2((int)v));

    r = RANGER_open(expr, exprstr, GXexprmin, GXexprmax);
    v = RANGER_closest(r, v, RANGER_ROUND_CLOSEST);
    RANGER_close(r);
    return(v);
}

double GX_constrain0(const char *exprstr, double v)
{
    RANGER *r;

    EXPR_set(expr,"powerof2",MAKEPOWEROF2((int)v));

    r = RANGER_open(expr, exprstr, 0.0, GXexprmax);
    v = RANGER_closest(r, v, RANGER_ROUND_CLOSEST);
    RANGER_close(r);
    return(v);
}


#if 0
static double GXEXPR_getcolour(int count, const double *data)
{
    int argb=0;

    if (count>=1)
        argb = (int) data[0];

    return((double)argb);

}
#endif


static unsigned int GXFramenameToIndex(char *framename, int framenum)
{
    unsigned int index;

    if (!strncmp(framename,"chr",3))                    /* chrA */
        index = (unsigned char)framename[3];
    else if (framename[0]=='\'' && framename[2]=='\'')  /* 'A' */
        index = (unsigned char)framename[1];
    else if (framename[0]=='$')                         /* $0040 */
        sscanf(framename+1, "%x", &index);
    else if (framename[0]=='0' && framename[1]=='x')    /* 0x0040 */
        sscanf(framename+2, "%x", &index);
    else if (isalnum(framename[0])                      /* 0000 */
          && isalnum(framename[1])
          && isalnum(framename[2])
          && isalnum(framename[3])
          && (framename[4]<=' '))
    {
        sscanf(framename, "%x", &index);
    }
    else if (framename[0])                              /* 65 */
        sscanf(framename, "%d", &index);
    else
        index = 32 + framenum;                          /* default */

    return(index);
}

char *  GX_getopt2(int *optvalptr,const char *optptr,const char *optname, int optmin, int optmax, int optdefault)
{
    const char *d=optptr;
    if (*optptr=='?')
    {
        if (OPTverbose)
        {
            gprintf("Option %s. Valid range is %ld..%ld.\nDefault value %ld.\n", optname, optmin, optmax, *optvalptr);
            SPIThelpgiven = 1;
        }
        return((char *)d+1);
    }
    else if (*optptr<=' ')
        *optvalptr = optdefault;
    else
    {
        double v;
        int optval;
        RANGER *r;
        char OPTexpr[SPITMAXLINE];

        d = SPIT_getfield( OPTexpr,optptr,1,0);
        v = (double) *optvalptr;

        r = RANGER_open(expr, OPTexpr, (double)optmin, (double)optmax);
        v = RANGER_closest(r, v, RANGER_ROUND_CLOSEST);
        RANGER_close(r);

        if (v<0.0)
            optval = -(int)(-v+0.5);
        else
            optval =  (int)(unsigned int)( v+0.5); // allow 0x80000000..0xffffffff

        if (optmin!=0x80000000 && optval < optmin)
        {
            optval = optmin;
            if (OPTverbose)
            {
                gprintf("Value too small for %s option. Valid range is %ld..%ld.  Using %ld.\n", optname, optmin, optmax, optval);
                SPIThelpgiven = 1;
            }
        }
        if (optmax!=0x7fffffff && optval > optmax)
        {
            optval = optmax;
            if (OPTverbose)
            {
                gprintf("Value too large for %s option. Valid range is %ld..%ld.  Using %ld.\n", optname, optmin, optmax, optval);
                SPIThelpgiven = 1;
            }
        }

        *optvalptr = optval;
    }
    return((char *)d);
}

char *GX_getopt(int *optvalptr,const char *optptr,const char *optname, int optmin, int optmax)
{
    return(GX_getopt2(optvalptr,optptr,optname,optmin,optmax,gmin(optmax,*optvalptr+1)));
}

char *SPIT_getcolouropt(const char *optptr, int *optvalptr, const char *optname, int optmin, int optmax, int last)
{
    static struct
    {
        char *name;
        unsigned int colour;
    } colournametable[]=
    {
        "BLACK",        (unsigned int)((0xff<<24)+(0x00<<16)+(0x00<<8)+(0x00>>0)),
        "BLUE",         (unsigned int)((0xff<<24)+(0x00<<16)+(0x00<<8)+(0xa8>>0)),
        "GREEN",        (unsigned int)((0xff<<24)+(0x00<<16)+(0xa8<<8)+(0x00>>0)),
        "CYAN",         (unsigned int)((0xff<<24)+(0x00<<16)+(0xa8<<8)+(0xa8>>0)),
        "RED",          (unsigned int)((0xff<<24)+(0xa8<<16)+(0x00<<8)+(0x00>>0)),
        "MAGENTA",      (unsigned int)((0xff<<24)+(0xa8<<16)+(0x00<<8)+(0xa8>>0)),
        "BROWN",        (unsigned int)((0xff<<24)+(0xa8<<16)+(0x50<<8)+(0x00>>0)),
        "LIGHTGREY",    (unsigned int)((0xff<<24)+(0xa8<<16)+(0xa8<<8)+(0xa8>>0)),
        "DARKGREY",     (unsigned int)((0xff<<24)+(0x50<<16)+(0x50<<8)+(0x50>>0)),
        "LIGHTBLUE",    (unsigned int)((0xff<<24)+(0x50<<16)+(0x50<<8)+(0xff>>0)),
        "LIGHTGREEN",   (unsigned int)((0xff<<24)+(0x50<<16)+(0xff<<8)+(0x50>>0)),
        "LIGHTCYAN",    (unsigned int)((0xff<<24)+(0x50<<16)+(0xff<<8)+(0xff>>0)),
        "LIGHTRED",     (unsigned int)((0xff<<24)+(0xff<<16)+(0x50<<8)+(0x50>>0)),
        "LIGHTMAGENTA", (unsigned int)((0xff<<24)+(0xff<<16)+(0x50<<8)+(0xff>>0)),
        "YELLOW",       (unsigned int)((0xff<<24)+(0xff<<16)+(0xff<<8)+(0x50>>0)),
        "WHITE",        (unsigned int)((0xff<<24)+(0xff<<16)+(0xff<<8)+(0xff>>0)),
        "GREY",         (unsigned int)((0xff<<24)+(0x80<<16)+(0x80<<8)+(0x80>>0)),
        "TRANSPARENT",  (unsigned int)((0x00<<24)+(0x00<<16)+(0x00<<8)+(0x00>>0)),
        0,0,
    };
    int i;
    char colourbuf[SPITMAXLINE];
    char *colourptr;

    if (*optptr=='?')
    {
        gprintf("Colour Option %s.\n",optname);
        gprintf("There are 3 basic ways you can specify an ARGB colour.\n");
            gprintf("1. (R,G,B) or (A,R,G,B)\n");
        gprintf("2. A single value, typically in hex form 0xAARRGGBB.\n");
        gprintf("3. Using a predefined colour.  The predefined colours are:\n");
        gprintf("   BLACK,BLUE,GREEN,CYAN,RED,MAGENTA,BROWN,LIGHTGREY,DARKGREY,LIGHTBLUE,\n");
        gprintf("   LIGHTGREEN,LIGHTCYAN,LIGHTRED,LIGHTMAGENTA,YELLOW,WHITE,GREY,TRANSPARENT\n");
        return((char *)optptr+1);
    }
    // for compatibility with REAL colour system
    if (!strnicmp(optptr,"C_",2))
        optptr += 2;
    else if (!strnicmp(optptr,"CA_",3))
        optptr += 3;
    for (i=0; colournametable[i].name; ++i)
    {
        if (!strnicmp(colournametable[i].name,optptr,strlen(colournametable[i].name)))
        {
            optptr += strlen(colournametable[i].name);
            *optvalptr = (int) colournametable[i].colour;
            return((char *)optptr);
        }
    }
    if (*optptr=='(')
    {
        int a=255;
        int r=0;
        int g=0;
        int b=0;
        optptr = SPIT_getfield(colourbuf,optptr,1,0);
        colourptr = colourbuf;

                             colourptr = GX_getopt( &a,colourptr+0, "Alpha", 0,255);
        if (*colourptr==',') colourptr = GX_getopt( &r,colourptr+1, "Red",   0,255);
        if (*colourptr==',') colourptr = GX_getopt( &g,colourptr+1, "Green", 0,255);
        if (*colourptr==',') colourptr = GX_getopt( &b,colourptr+1, "Blue",  0,255);
        else
        {
            b = g;  /* only 3 values specified */
            g = r;
            r = a;
            a = 255;
        }
        *optvalptr = (int) (a<<24)|(r<<16)|(g<<8)|(b<<0);
        return((char *)optptr);
    }

    return(GX_getopt( optvalptr,optptr, optname, optmin, optmax));
}

char *SPIT_getpositionopt(const char *optptr, char *optvalptr, const char *optname, int optmin, int optmax)
{
    return(SPIT_getfield( optvalptr, optptr, true, false));
}

static int GXsetposition(int oldval,const char *optionnewval,int maxval)
{
    int newval=oldval;

    EXPR_set(expr,"old",oldval);
    EXPR_set(expr,"mid",oldval/2);
    EXPR_set(expr,"max",maxval-1);

    newval = (int) GX_constrain(optionnewval, (double) oldval);

    return(newval);
}

double GX_sizeratio(const char *sizepix, int pixels)
{
    double v;

    v = GX_constrain0(sizepix, (double) pixels);
    return(sqrt(v)/sqrt((double)pixels));
}

double GXscalesetHighPrecision(int width, const char *OPTscalexstr,int OPTscalexby, double sizeratio)
{
    double x;
    double OPTscalex=0.0;

    if (OPTscalexstr[0])
        OPTscalex = GX_constrain0(OPTscalexstr, (double) width);
//        OPTscalex = EXPR_get(expr,OPTscalexstr);

    switch (OPTscalexby)
    {
        default:
        case 0:
            x = OPTscalex;
            break;

        case OPTSCALEPERCENT:
            x = width*OPTscalex/100;
            break;

        case OPTSCALEPIXELS:
            x = (int) (width*sizeratio);
            break;

        case OPTSCALEBY:
            x = width-OPTscalex*2;
            break;

        case OPTSCALEBY+OPTSCALEPERCENT:
            x = width*(OPTscalex+100)/100;
            break;

        case OPTSCALETO:
            x = width;
            if (OPTscalex>0)
                x = (width+(int)(OPTscalex)-1)/(int)(OPTscalex)*(int)(OPTscalex);
            else if (OPTscalex<0)
                x = width/-(int)(OPTscalex)*-(int)(OPTscalex);
            break;

        case OPTSCALEMAX:
            x = width;
            if (x>OPTscalex)
                x = OPTscalex;
            break;
    }
    return(x);
}


int GXscaleset(int width, const char *OPTscalexstr,int OPTscalexby, double sizeratio)
{
    return ((int)GXscalesetHighPrecision(width,OPTscalexstr,OPTscalexby,sizeratio));
}

int GXscaledefault(int y,int height,int x,int width)
{
    if (!y)
    {
        if (x)
            y = height*x/width;
        else
            y = height;
    }
    return(y);
}

bool GXisnumerical(const char *s)
{
    bool isnum = false;
    if (*s)
    {
        isnum = true;
        do
        {
            if (*s<'0' || *s>'9')
            {
                isnum = false;
                break;
            }
            ++s;
        } while (*s);
    }
    return isnum;
}

char *GXgetscaleopt(const char *optptr, char *OPTscalex, int *optxbyptr, char *OPTscaley, int *optybyptr, const char *optname, int enableby, int optmin, int optmax)
{
//    int OPTscalex=*optxptr;
//    int OPTscaley=*optyptr;
    int OPTscalexby=*optxbyptr;
    int OPTscaleyby=*optybyptr;
    const char *d=optptr;
//    char namebuf[SPITMAXLINE];
    int l;

    OPTscalexby = 0;
    OPTscaleyby = 0;
    if (enableby)
    {
        if (!strnicmp(d,"by",2))
        {
            OPTscalexby = OPTSCALEBY;
            OPTscaleyby = OPTSCALEBY;
            d += 2;
        }
        else if (!strnicmp(d,"to",2))
        {
            OPTscalexby = OPTSCALETO;
            OPTscaleyby = OPTSCALETO;
            d += 2;
        }
        else if (!strnicmp(d,"max",3) && d[3]!='(')
        {
            OPTscalexby = OPTSCALEMAX;
            OPTscaleyby = OPTSCALEMAX;
            d += 3;
        }
        else if (!strnicmp(d,"pix",3))
        {
    //        OPTscalexby = OPTSCALEPIXELS;
    //        OPTscaleyby = OPTSCALEPIXELS;
    //        optmin = 0x80000000;
    //        optmax = 0x7fffffff;
            d += 3;
        }
    }
//    sprintf(namebuf,"%s x",optname);
    d = SPIT_getfield( OPTscalex, d, 1,0);

    char *x = strchr(OPTscalex,'x');
    if (x)
    {
        *x = 0;
        if (GXisnumerical(OPTscalex) && GXisnumerical(x+1))
        {
            d = optptr+strlen(OPTscalex);
        }
        else
        {
            *x = 'x';
        }
    }

//    d = SPIT_getrangeopt( OPTscalex,d, namebuf, optmin, optmax);
    l = strlen(OPTscalex);
    if (l && OPTscalex[l-1]=='%')
    {
        OPTscalex[l-1] = 0;
        OPTscalexby |= OPTSCALEPERCENT;
    }
    if (*d==',' || *d=='x')
    {
//        sprintf(namebuf,"%s y",optname);
        d = SPIT_getfield( OPTscaley, d+1, 1,0);
//        d = SPIT_getrangeopt( OPTscaley,d+1, namebuf, optmin, optmax);
        l = strlen(OPTscaley);
        if (l && OPTscaley[l-1]=='%')
        {
            OPTscaley[l-1] = 0;
            OPTscaleyby |= OPTSCALEPERCENT;
        }
    }
    else if (!OPTscalexby)
    {
        OPTscalexby |= OPTSCALEPIXELS;
        OPTscaleyby |= OPTSCALEPIXELS;
    }

//    *optxptr   = OPTscalex;
//    *optyptr   = OPTscaley;
    if (enableby)
    {
        *optxbyptr = OPTscalexby;
        *optybyptr = OPTscaleyby;
    }
    return((char *)d);
}



/****************************************************************/
/*  Filter Functions                                            */
/****************************************************************/


bool DoFilter(FINSTANCE *fx,GINFO * ginfo0, char * f0,GINFO ** ginfo, char ** f,int drowbytes, GBITMAP *gm, GBITMAP *gaux)
{
    // actually do the filtering.
    bool ok = FILTER_filter(fx, ginfo0, f0, drowbytes)?true:false;

    // free up the original image buffers.
    gfree((*f));
    gfree((*ginfo));

    // setup pointers to the new data
    *f = f0;
    *ginfo = ginfo0;

    // cleanup and update the expression parser.
    FILTER_close(fx);
    UpdateExpr(*ginfo,gm,gaux,composite_currentframe);
    return ok;
}


char * SetupFilter(char * fname,FINSTANCE **fx,FOP * fop, int numfop, GBITMAP * source,int numsources, int * drowbytes,GINFO ** ginfo0)
{
    char * f0 = 0;
    int ok    = 0;
    // find the filter in question.
    ok = FILTER_find(fname);
    if(ok)
    {
        ok = FILTER_open(fx, numsources, source, numfop, fop);
        if(ok)
        {
            // get the info and figure out the rowbytes.
            (*ginfo0)    = FILTER_info((*fx), 0);
            if(*ginfo0)
            {
                (*drowbytes) = (*ginfo0)->width * (*ginfo0)->bpp/8;

                // make my destination image buffer.
                f0 = (char *) galloc((*drowbytes) * (*ginfo0)->height);

                // return the destination image buffer.
                return f0;
            }
        }
    }
    return 0;
}



/* applyzoom zooms an image and returns pointer to new info and image */

static void    applyzoom(GINFO *ginfo, char *f,
                  const char *zoomx, int zoomxby,
                  const char *zoomy, int zoomyby,
                  int zoomsharp,
                  GINFO **pginfoout, char **pfout,GABOUT *about)
{
    FINSTANCE *fx;
    GBITMAP source[1];
    FOP fop[9];
    GINFO *ginfo0;
    int drowbytes;
    char *f0;
    int x;
    int y;

    source[0].ginfo = ginfo;
    source[0].image = f;
    source[0].rowbytes = ginfo->width*ginfo->bpp/8;

    ORIGUpdate(ginfo);
    x = GXscaleset(ginfo->width,zoomx,zoomxby,GX_sizeratio(zoomx,ginfo->width*ginfo->height));
    y = GXscaleset(ginfo->height,zoomy,zoomyby,GX_sizeratio(zoomx,ginfo->width*ginfo->height));
    y = GXscaledefault(y,ginfo->height,x,ginfo->width);
    x = GXscaledefault(x,ginfo->width,y,ginfo->height);

    fop[0].i = x;
    fop[1].i = y;
    fop[2].i = 1;
    fop[3].i = 0;
    fop[4].f = zoomsharp?zoomsharp/100.0:1.0; /* kernel width */
    fop[5].f = 0.0;
    fop[6].i = ORIGmaxwidth;
    fop[7].i = ORIGmaxheight;
    fop[8].i = OPTlayered > 0;        // is this a pages type? (crippled)

    f0 = SetupFilter("Zoom",&fx,fop,9,source,1,&drowbytes,&ginfo0);

    FILTER_filter(fx, ginfo0, f0, drowbytes);

    *pfout = f0;
    *pginfoout = ginfo0;

    FILTER_close(fx);
}

/* applysize sizes an image and returns pointer to new info and image */

static void    applysize(GINFO *ginfo, char *f,
                  const char *sizex, int sizexby,
                  const char *sizey, int sizeyby,
                  int sizealpha,
                  int sizechromakey,
                  int sizemethod,
                  int sizesharpx,
                  int sizesharpy,
                  GINFO **pginfoout, char **pfout,GABOUT *about)
{
    FINSTANCE *fx;
    GBITMAP source[1];
    FOP fop[10];
    GINFO *ginfo0;
    int drowbytes;
    char *f0;
    int x;
    int y;
    double nx = 0;
    double ny = 0;

    source[0].ginfo = ginfo;
    source[0].image = f;
    source[0].rowbytes = ginfo->width*ginfo->bpp/8;

    ORIGUpdate(ginfo);
    x = GXscaleset(ginfo->width,sizex,sizexby,GX_sizeratio(sizex,ginfo->width*ginfo->height));
    y = GXscaleset(ginfo->height,sizey,sizeyby,GX_sizeratio(sizex,ginfo->width*ginfo->height));
    y = GXscaledefault(y,ginfo->height,x,ginfo->width);
    x = GXscaledefault(x,ginfo->width,y,ginfo->height);

    nx = GXscalesetHighPrecision(ginfo->width,sizex,sizexby,GX_sizeratio(sizex,ginfo->width*ginfo->height));
    ny = GXscalesetHighPrecision(ginfo->height,sizey,sizeyby,GX_sizeratio(sizex,ginfo->width*ginfo->height));
    fop[0].i = x;                                    /* int width           Width of resulting image */
    fop[1].i = y;                                    /* int height          Height of resulting image */
    fop[2].i = sizemethod;                           /* int method          Filter function (see below) */
    fop[3].i = sizealpha;                            /* int respect_alpha   0=default,1=keep sharp transparent edges */
    fop[4].i = sizechromakey;                        /* int chromakey       0=default */
    fop[5].f = sizesharpx?sizesharpx/100.0:1.0;      /* double xsharpness   <1=sharp >1=blury x<=0=crash (1.0=default) */
    fop[6].f = sizesharpy?sizesharpy/100.0:fop[5].f; /* double ysharpness   <1=sharp >1=blury x<=0=crash (1.0=default) */
    fop[7].f = nx/ginfo->width;                      /* scale ratio to try to preserve some of original sig figs of expression */
    fop[8].f = ny/ginfo->height;                     /* scale ratio to try to preserve some of original sig figs of expression */

    f0 = SetupFilter("Size",&fx,fop,9,source,1,&drowbytes,&ginfo0);

    FILTER_filter(fx, ginfo0, f0, drowbytes);

    *pfout = f0;
    *pginfoout = ginfo0;

    FILTER_close(fx);
}

static void    applyscale(GINFO *ginfo, char *f,
                  const char *sizex, int sizexby,
                  const char *sizey, int sizeyby,
                  GINFO **pginfoout, char **pfout,GABOUT *about)
{
    FINSTANCE *fx;
    GBITMAP source[1];
    FOP fop[2];
    GINFO *ginfo0;
    int drowbytes;
    char *f0;
    int x;
    int y;

    source[0].ginfo = ginfo;
    source[0].image = f;
    source[0].rowbytes = ginfo->width*ginfo->bpp/8;

    ORIGUpdate(ginfo);
    x = GXscaleset(ginfo->width,sizex,sizexby,GX_sizeratio(sizex,ginfo->width*ginfo->height));
    y = GXscaleset(ginfo->height,sizey,sizeyby,GX_sizeratio(sizex,ginfo->width*ginfo->height));
    y = GXscaledefault(y,ginfo->height,x,ginfo->width);
    x = GXscaledefault(x,ginfo->width,y,ginfo->height);

    fop[0].i = x;                                    /* int width           Width of resulting image */
    fop[1].i = y;                                    /* int height          Height of resulting image */

    f0 = SetupFilter("Scale",&fx,fop,2,source,1,&drowbytes,&ginfo0);

    FILTER_filter(fx, ginfo0, f0, drowbytes);

    *pfout = f0;
    *pginfoout = ginfo0;

    FILTER_close(fx);
}


/* applythumb thumbs an image and returns pointer to new info and image */

static void    applythumb(GINFO *ginfo, char *f,
                  const char *thumbx, int thumbxby,
                  const char *thumby, int thumbyby,
                  GINFO **pginfoout, char **pfout,GABOUT *about)
{
    FINSTANCE *fx;
    GBITMAP source[1];
    FOP fop[10];
    int drowbytes;
    GINFO *ginfo0;
    char *f0;
    GINFO *ginfo1;
    char *f1;
    int x;
    int y;
    int thumbxi;
    int thumbyi;

    source[0].ginfo = ginfo;
    source[0].image = f;
    source[0].rowbytes = ginfo->width*ginfo->bpp/8;

    ORIGUpdate(ginfo);
    thumbxi = GXscaleset(ginfo->width,thumbx,thumbxby,GX_sizeratio(thumbx,ginfo->width*ginfo->height));
    thumbyi = GXscaleset(ginfo->height,thumby,thumbyby,GX_sizeratio(thumbx,ginfo->width*ginfo->height));
    thumbyi = GXscaledefault(thumbyi,ginfo->height,thumbxi,ginfo->width);
    thumbxi = GXscaledefault(thumbxi,ginfo->width,thumbyi,ginfo->height);

    x = thumbxi;
    y = GXscaledefault(0,ginfo->height,thumbxi,ginfo->width);

    if (y <= thumbyi)
    {
        x = GXscaledefault(0,ginfo->width,thumbyi,ginfo->height);
        y = thumbyi;
    }


    /* size image */

    fop[0].i = x;                                    /* int width           Width of resulting image */
    fop[1].i = y;                                    /* int height          Height of resulting image */
    fop[2].i = 0;                                    /* int method          Filter function (see below) */
    fop[3].i = 0;                                    /* int respect_alpha   0=default,1=keep sharp transparent edges */
    fop[4].i = 0;                                    /* int chromakey       0=default */
    fop[5].f = 1.0;                                  /* double xsharpness   <1=sharp >1=blury x<=0=crash (1.0=default) */
    fop[6].f = 1.0;                                  /* double ysharpness   <1=sharp >1=blury x<=0=crash (1.0=default) */
    fop[7].i = ORIGmaxwidth;
    fop[8].i = ORIGmaxheight;
    fop[9].i = 0;

    f0 = SetupFilter("Size",&fx,fop,10,source,1,&drowbytes,&ginfo0);

    FILTER_filter(fx, ginfo0, f0, drowbytes);

    FILTER_close(fx);

    /* crop image */

    source[0].ginfo = ginfo0;
    source[0].image = f0;
    source[0].rowbytes = drowbytes;

    x = (ginfo0->width-thumbxi)/2;
    y = (ginfo0->height-thumbyi)/2;
    fop[0].i = x;
    fop[1].i = y;
    fop[2].i = thumbxi;
    fop[3].i = thumbyi;
    fop[4].i = 0;
    fop[5].i = 0;
    fop[6].i = 0;
    fop[7].i = 0;

    f1 = SetupFilter("Crop",&fx,fop,8,source,1,&drowbytes,&ginfo1);

    FILTER_filter(fx, ginfo1, f1, drowbytes);

    gfree(f0);
    gfree(ginfo0);

    FILTER_close(fx);

    *pfout = f1;
    *pginfoout = ginfo1;
}


static void GXdrawrect(char *f,int bpp, int rowbytes, int x, int y, int width, int height, int colour)
{
    FINSTANCE *fx;
    GBITMAP source[1];
    FOP fop[5];
    GINFO ginfo;
    GINFO *ginfo0;

    memset(&ginfo,0,sizeof(ginfo));
    ginfo.size = sizeof(ginfo);
    ginfo.width = width;
    ginfo.height = height;
    ginfo.bpp = ginfo.originalbpp = bpp;

    source[0].ginfo = &ginfo;
    source[0].image = f;
    source[0].rowbytes = rowbytes;

    fop[0].i = x;
    fop[1].i = y;
    fop[2].i = width;
    fop[3].i = height;
    fop[4].i = colour;

    FILTER_find("Rect");
    FILTER_open(&fx, 1, source, 5, fop);

    ginfo0 = FILTER_info(fx, 0);

    FILTER_filter(fx, ginfo0, f, source[0].rowbytes);

    gfree(ginfo0);

    FILTER_close(fx);
}

static void GXdrawoutline(char *f,int bpp, int rowbytes, int x, int y, int width, int height, int colour)
{
    FINSTANCE *fx;
    GBITMAP source[1];
    FOP fop[5];
    GINFO ginfo;
    GINFO *ginfo0;

    memset(&ginfo,0,sizeof(ginfo));
    ginfo.size = sizeof(ginfo);
    ginfo.width = width;
    ginfo.height = height;
    ginfo.bpp = ginfo.originalbpp = bpp;

    source[0].ginfo = &ginfo;
    source[0].image = f;
    source[0].rowbytes = rowbytes;

    fop[0].i = x;
    fop[1].i = y;
    fop[2].i = width;
    fop[3].i = height;
    fop[4].i = colour;

    FILTER_find("Outline");
    FILTER_open(&fx, 1, source, 5, fop);

    ginfo0 = FILTER_info(fx, 0);

    FILTER_filter(fx, ginfo0, f, source[0].rowbytes);

    gfree(ginfo0);

    FILTER_close(fx);
}




/* determines the number of unique colours in the image */
/* returns the number of colours if less than or equal to 256 or else returns 0 */
static int isLE256Colours(unsigned char *base, int height, int width, int rowbytes)
{
    int x, y, i;
    int *pixel;
    int count, found;
    int colour[256];

    count = 0;
    found = 0;

    for (y = 0; y < height; y++)
    {
        pixel = (int *)(base + y*rowbytes);
        for (x = 0; x < width; x++)
        {
            found = 0;

            for (i = 0; i < count; i++)
            {
                if (colour[i]==*pixel)
//              if ((colour[i].a == pixel->a) && (colour[i].r == pixel->r) && (colour[i].g == pixel->g) && (colour[i].b == pixel->b))
                {
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                /* the entire array has been searched and no match in colour - meaning there is a 257th colour */
                if (count==256)
                    return(0);
                colour[count] = *pixel;
                count++;
            }
            pixel++;
        }
    }
    return (count);
}

static GBITMAP *GX8to32(GBITMAP *f8)
{
    GBITMAP *f32=NULL;
    int depth;
    int ok=0;

    f32 = (GBITMAP *) galloc(sizeof(GBITMAP));
    if (f32)
    {
        depth = 32;
        f32->ginfo = info8to32(f8->ginfo,(char *)f8->image,f8->rowbytes,depth);
        if (f32->ginfo)
        {
            f32->rowbytes = f32->ginfo->width*f32->ginfo->bpp/8;
            f32->image = (char *) galloc(f32->ginfo->width*f32->ginfo->height*f32->ginfo->bpp/8);
            if (f32->image)
            {
                if (filter8to32(f8->ginfo,(char *)f8->image,f8->rowbytes,
                    f32->ginfo,(char *)f32->image,f32->rowbytes,depth))
                {
                    ok = 1;
                }
            }
        }
    }
    if (!ok)
        GIMEX_freebitmap(f32);
    return(f32);
}


static double GX_asciitonumber(const char *framename)
{
    double fn=0;
    for (int i=0; i<(int)strlen(framename) && i<8; ++i)
        fn = fn*256+(double)(unsigned char)(framename[i]);
    return fn;
}

// convert ascii to unicode with markups for encoded hex. ie \0x1234

static void GX_atou(unsigned short *utextstr, const char *OPTtextstring)
{
    int i = 0;
    for (i=0; OPTtextstring[i]; ++i)
    {
        if (OPTtextstring[i]=='\\')
        {
            int d=0;
            if (OPTtextstring[i+1]=='0' && OPTtextstring[i+2]=='x') // 0x for hex
            {
                bool moredigits=true;
                int numdigits=0;
                OPTtextstring += 3; // skip \0x
                do
                {
                    int digit=0;
                    if (OPTtextstring[i]>='0' && OPTtextstring[i]<='9')
                        digit= OPTtextstring[i]-'0';
                    else if (OPTtextstring[i]>='a' && OPTtextstring[i]<='f')
                        digit= OPTtextstring[i]-'a'+10;
                    else if (OPTtextstring[i]>='A' && OPTtextstring[i]<='F')
                        digit= OPTtextstring[i]-'A'+10;
                    else
                        moredigits=false;
                    if (moredigits)
                    {
                        d = d*16+digit;
                        ++OPTtextstring;
                        ++numdigits;
                        if (numdigits>=4)
                            moredigits = false;
                    }

                } while (moredigits);
                --OPTtextstring;
            }
            utextstr[i] = (unsigned short)d;
        }
        else
            utextstr[i] = (unsigned short)((unsigned char)OPTtextstring[i]);
    }
    utextstr[i] = (unsigned short)((unsigned char)'\0');
}



static void UpdateExpr(GINFO *ginfo,GBITMAP *gm,GBITMAP *gaux,int frame)
{
    EXPR_set(expr,"frame",frame);
    EXPR_set(expr,"framenum",frame);
    EXPR_set(expr,"framename",GX_asciitonumber(ginfo->framename));
    EXPR_set(expr,"width",ginfo->width);
    EXPR_set(expr,"height",ginfo->height);
    EXPR_set(expr,"bpp",ginfo->originalbpp);
    EXPR_set(expr,"startcolour",ginfo->startcolour);
    EXPR_set(expr,"numcolours",ginfo->numcolours);
    EXPR_set(expr,"col ours",ginfo->numcolours);
    EXPR_set(expr,"subtype",ginfo->subtype);
    EXPR_set(expr,"packed",ginfo->packed);
    EXPR_set(expr,"quality",ginfo->quality);
    EXPR_set(expr,"framesize",ginfo->framesize);
    EXPR_set(expr,"alphabits",ginfo->alphabits);
    EXPR_set(expr,"redbits",ginfo->redbits);
    EXPR_set(expr,"greenbits",ginfo->greenbits);
    EXPR_set(expr,"bluebits",ginfo->bluebits);
    EXPR_set(expr,"centerx",ginfo->centerx);
    EXPR_set(expr,"centery",ginfo->centery);
    EXPR_set(expr,"x",ginfo->defaultx);
    EXPR_set(expr,"y",ginfo->defaulty);
    EXPR_set(expr,"defaultx",ginfo->defaultx);
    EXPR_set(expr,"defaulty",ginfo->defaulty);
    EXPR_set(expr,"fps",ginfo->fps);
    EXPR_set(expr,"dpi",ginfo->dpi);
    EXPR_set(expr,"numhotspots",ginfo->numhotspots);
    EXPR_set(expr,"hot0x",ginfo->hotspottbl[0][0]);
    EXPR_set(expr,"hot0y",ginfo->hotspottbl[0][1]);
    EXPR_set(expr,"hot1x",ginfo->hotspottbl[1][0]);
    EXPR_set(expr,"hot1y",ginfo->hotspottbl[1][1]);
    EXPR_set(expr,"hot2x",ginfo->hotspottbl[2][0]);
    EXPR_set(expr,"hot2y",ginfo->hotspottbl[2][1]);
    EXPR_set(expr,"hot3x",ginfo->hotspottbl[3][0]);
    EXPR_set(expr,"hot3y",ginfo->hotspottbl[3][1]);
    EXPR_set(expr,"hot4x",ginfo->hotspottbl[4][0]);
    EXPR_set(expr,"hot4y",ginfo->hotspottbl[4][1]);
    EXPR_set(expr,"hot5x",ginfo->hotspottbl[5][0]);
    EXPR_set(expr,"hot5y",ginfo->hotspottbl[5][1]);
    EXPR_set(expr,"hot6x",ginfo->hotspottbl[6][0]);
    EXPR_set(expr,"hot6y",ginfo->hotspottbl[6][1]);
    EXPR_set(expr,"hot7x",ginfo->hotspottbl[7][0]);
    EXPR_set(expr,"hot7y",ginfo->hotspottbl[7][1]);
    EXPR_set(expr,"hot8x",ginfo->hotspottbl[8][0]);
    EXPR_set(expr,"hot8y",ginfo->hotspottbl[8][1]);
    EXPR_set(expr,"hot9x",ginfo->hotspottbl[9][0]);
    EXPR_set(expr,"hot9y",ginfo->hotspottbl[9][1]);
    EXPR_set(expr,"hot10x",ginfo->hotspottbl[10][0]);
    EXPR_set(expr,"hot10y",ginfo->hotspottbl[10][1]);
    EXPR_set(expr,"hot11x",ginfo->hotspottbl[11][0]);
    EXPR_set(expr,"hot11y",ginfo->hotspottbl[11][1]);
    EXPR_set(expr,"hot12x",ginfo->hotspottbl[12][0]);
    EXPR_set(expr,"hot12y",ginfo->hotspottbl[12][1]);
    EXPR_set(expr,"hot13x",ginfo->hotspottbl[13][0]);
    EXPR_set(expr,"hot13y",ginfo->hotspottbl[13][1]);
    EXPR_set(expr,"hot14x",ginfo->hotspottbl[14][0]);
    EXPR_set(expr,"hot14y",ginfo->hotspottbl[14][1]);
    EXPR_set(expr,"hot15x",ginfo->hotspottbl[15][0]);
    EXPR_set(expr,"hot15y",ginfo->hotspottbl[15][1]);
    EXPR_set(expr,"hot16x",ginfo->hotspottbl[16][0]);
    EXPR_set(expr,"hot16y",ginfo->hotspottbl[16][1]);
    EXPR_set(expr,"hot17x",ginfo->hotspottbl[17][0]);
    EXPR_set(expr,"hot17y",ginfo->hotspottbl[17][1]);
    EXPR_set(expr,"hot18x",ginfo->hotspottbl[18][0]);
    EXPR_set(expr,"hot18y",ginfo->hotspottbl[18][1]);
    EXPR_set(expr,"hot19x",ginfo->hotspottbl[19][0]);
    EXPR_set(expr,"hot19y",ginfo->hotspottbl[19][1]);
    EXPR_set(expr,"widthp2",MAKEPOWEROF2(ginfo->width));
    EXPR_set(expr,"heightp2",MAKEPOWEROF2(ginfo->height));
    EXPR_set(expr,"powerof2",MAKEPOWEROF2(gmax(ginfo->width,ginfo->height)));

    /* composite file info */
    if (gm)
    {
        GINFO *c_ginfo=gm->ginfo;
        EXPR_set(expr,"c_framename",GX_asciitonumber(c_ginfo->framename));
        EXPR_set(expr,"c_width",c_ginfo->width);
        EXPR_set(expr,"c_height",c_ginfo->height);
        EXPR_set(expr,"c_bpp",c_ginfo->originalbpp);
        EXPR_set(expr,"c_startcolour",c_ginfo->startcolour);
        EXPR_set(expr,"c_numcolours",c_ginfo->numcolours);
        EXPR_set(expr,"c_colours",c_ginfo->numcolours);
        EXPR_set(expr,"c_subtype",c_ginfo->subtype);
        EXPR_set(expr,"c_packed",c_ginfo->packed);
        EXPR_set(expr,"c_quality",c_ginfo->quality);
        EXPR_set(expr,"c_framesize",c_ginfo->framesize);
        EXPR_set(expr,"c_alphabits",c_ginfo->alphabits);
        EXPR_set(expr,"c_redbits",c_ginfo->redbits);
        EXPR_set(expr,"c_greenbits",c_ginfo->greenbits);
        EXPR_set(expr,"c_bluebits",c_ginfo->bluebits);
        EXPR_set(expr,"c_centerx",c_ginfo->centerx);
        EXPR_set(expr,"c_centery",c_ginfo->centery);
        EXPR_set(expr,"c_x",c_ginfo->defaultx);
        EXPR_set(expr,"c_y",c_ginfo->defaulty);
        EXPR_set(expr,"c_defaultx",c_ginfo->defaultx);
        EXPR_set(expr,"c_defaulty",c_ginfo->defaulty);
        EXPR_set(expr,"c_fps",c_ginfo->fps);
        EXPR_set(expr,"c_dpi",c_ginfo->dpi);
        EXPR_set(expr,"c_numhotspots",c_ginfo->numhotspots);
        EXPR_set(expr,"c_hot0x",c_ginfo->hotspottbl[0][0]);
        EXPR_set(expr,"c_hot0y",c_ginfo->hotspottbl[0][1]);
        EXPR_set(expr,"c_hot1x",c_ginfo->hotspottbl[1][0]);
        EXPR_set(expr,"c_hot1y",c_ginfo->hotspottbl[1][1]);
        EXPR_set(expr,"c_hot2x",c_ginfo->hotspottbl[2][0]);
        EXPR_set(expr,"c_hot2y",c_ginfo->hotspottbl[2][1]);
        EXPR_set(expr,"c_hot3x",c_ginfo->hotspottbl[3][0]);
        EXPR_set(expr,"c_hot3y",c_ginfo->hotspottbl[3][1]);
        EXPR_set(expr,"c_hot4x",c_ginfo->hotspottbl[4][0]);
        EXPR_set(expr,"c_hot4y",c_ginfo->hotspottbl[4][1]);
        EXPR_set(expr,"c_hot5x",c_ginfo->hotspottbl[5][0]);
        EXPR_set(expr,"c_hot5y",c_ginfo->hotspottbl[5][1]);
        EXPR_set(expr,"c_hot6x",c_ginfo->hotspottbl[6][0]);
        EXPR_set(expr,"c_hot6y",c_ginfo->hotspottbl[6][1]);
        EXPR_set(expr,"c_hot7x",c_ginfo->hotspottbl[7][0]);
        EXPR_set(expr,"c_hot7y",c_ginfo->hotspottbl[7][1]);
        EXPR_set(expr,"c_hot8x",c_ginfo->hotspottbl[8][0]);
        EXPR_set(expr,"c_hot8y",c_ginfo->hotspottbl[8][1]);
        EXPR_set(expr,"c_hot9x",c_ginfo->hotspottbl[9][0]);
        EXPR_set(expr,"c_hot9y",c_ginfo->hotspottbl[9][1]);
        EXPR_set(expr,"c_hot10x",c_ginfo->hotspottbl[10][0]);
        EXPR_set(expr,"c_hot10y",c_ginfo->hotspottbl[10][1]);
        EXPR_set(expr,"c_hot11x",c_ginfo->hotspottbl[11][0]);
        EXPR_set(expr,"c_hot11y",c_ginfo->hotspottbl[11][1]);
        EXPR_set(expr,"c_hot12x",c_ginfo->hotspottbl[12][0]);
        EXPR_set(expr,"c_hot12y",c_ginfo->hotspottbl[12][1]);
        EXPR_set(expr,"c_hot13x",c_ginfo->hotspottbl[13][0]);
        EXPR_set(expr,"c_hot13y",c_ginfo->hotspottbl[13][1]);
        EXPR_set(expr,"c_hot14x",c_ginfo->hotspottbl[14][0]);
        EXPR_set(expr,"c_hot14y",c_ginfo->hotspottbl[14][1]);
        EXPR_set(expr,"c_hot15x",c_ginfo->hotspottbl[15][0]);
        EXPR_set(expr,"c_hot15y",c_ginfo->hotspottbl[15][1]);
        EXPR_set(expr,"c_hot16x",c_ginfo->hotspottbl[16][0]);
        EXPR_set(expr,"c_hot16y",c_ginfo->hotspottbl[16][1]);
        EXPR_set(expr,"c_hot17x",c_ginfo->hotspottbl[17][0]);
        EXPR_set(expr,"c_hot17y",c_ginfo->hotspottbl[17][1]);
        EXPR_set(expr,"c_hot18x",c_ginfo->hotspottbl[18][0]);
        EXPR_set(expr,"c_hot18y",c_ginfo->hotspottbl[18][1]);
        EXPR_set(expr,"c_hot19x",c_ginfo->hotspottbl[19][0]);
        EXPR_set(expr,"c_hot19y",c_ginfo->hotspottbl[19][1]);
        EXPR_set(expr,"c_widthp2",MAKEPOWEROF2(c_ginfo->width));
        EXPR_set(expr,"c_heightp2",MAKEPOWEROF2(c_ginfo->height));
        EXPR_set(expr,"c_powerof2",MAKEPOWEROF2(gmax(c_ginfo->width,c_ginfo->height)));
    }

    /* auxillary file info */
    if (gaux)
    {
        GINFO *a_ginfo=gaux->ginfo;
        EXPR_set(expr,"a_framename",GX_asciitonumber(a_ginfo->framename));
        EXPR_set(expr,"a_width",a_ginfo->width);
        EXPR_set(expr,"a_height",a_ginfo->height);
        EXPR_set(expr,"a_bpp",a_ginfo->originalbpp);
        EXPR_set(expr,"a_startcolour",a_ginfo->startcolour);
        EXPR_set(expr,"a_numcolours",a_ginfo->numcolours);
        EXPR_set(expr,"a_colours",a_ginfo->numcolours);
        EXPR_set(expr,"a_subtype",a_ginfo->subtype);
        EXPR_set(expr,"a_packed",a_ginfo->packed);
        EXPR_set(expr,"a_quality",a_ginfo->quality);
        EXPR_set(expr,"a_framesize",a_ginfo->framesize);
        EXPR_set(expr,"a_alphabits",a_ginfo->alphabits);
        EXPR_set(expr,"a_redbits",a_ginfo->redbits);
        EXPR_set(expr,"a_greenbits",a_ginfo->greenbits);
        EXPR_set(expr,"a_bluebits",a_ginfo->bluebits);
        EXPR_set(expr,"a_centerx",a_ginfo->centerx);
        EXPR_set(expr,"a_centery",a_ginfo->centery);
        EXPR_set(expr,"a_x",a_ginfo->defaultx);
        EXPR_set(expr,"a_y",a_ginfo->defaulty);
        EXPR_set(expr,"a_defaultx",a_ginfo->defaultx);
        EXPR_set(expr,"a_defaulty",a_ginfo->defaulty);
        EXPR_set(expr,"a_fps",a_ginfo->fps);
        EXPR_set(expr,"a_dpi",a_ginfo->dpi);
        EXPR_set(expr,"a_numhotspots",a_ginfo->numhotspots);
        EXPR_set(expr,"a_hot0x",a_ginfo->hotspottbl[0][0]);
        EXPR_set(expr,"a_hot0y",a_ginfo->hotspottbl[0][1]);
        EXPR_set(expr,"a_hot1x",a_ginfo->hotspottbl[1][0]);
        EXPR_set(expr,"a_hot1y",a_ginfo->hotspottbl[1][1]);
        EXPR_set(expr,"a_hot2x",a_ginfo->hotspottbl[2][0]);
        EXPR_set(expr,"a_hot2y",a_ginfo->hotspottbl[2][1]);
        EXPR_set(expr,"a_hot3x",a_ginfo->hotspottbl[3][0]);
        EXPR_set(expr,"a_hot3y",a_ginfo->hotspottbl[3][1]);
        EXPR_set(expr,"a_hot4x",a_ginfo->hotspottbl[4][0]);
        EXPR_set(expr,"a_hot4y",a_ginfo->hotspottbl[4][1]);
        EXPR_set(expr,"a_hot5x",a_ginfo->hotspottbl[5][0]);
        EXPR_set(expr,"a_hot5y",a_ginfo->hotspottbl[5][1]);
        EXPR_set(expr,"a_hot6x",a_ginfo->hotspottbl[6][0]);
        EXPR_set(expr,"a_hot6y",a_ginfo->hotspottbl[6][1]);
        EXPR_set(expr,"a_hot7x",a_ginfo->hotspottbl[7][0]);
        EXPR_set(expr,"a_hot7y",a_ginfo->hotspottbl[7][1]);
        EXPR_set(expr,"a_hot8x",a_ginfo->hotspottbl[8][0]);
        EXPR_set(expr,"a_hot8y",a_ginfo->hotspottbl[8][1]);
        EXPR_set(expr,"a_hot9x",a_ginfo->hotspottbl[9][0]);
        EXPR_set(expr,"a_hot9y",a_ginfo->hotspottbl[9][1]);
        EXPR_set(expr,"a_hot10x",a_ginfo->hotspottbl[10][0]);
        EXPR_set(expr,"a_hot10y",a_ginfo->hotspottbl[10][1]);
        EXPR_set(expr,"a_hot11x",a_ginfo->hotspottbl[11][0]);
        EXPR_set(expr,"a_hot11y",a_ginfo->hotspottbl[11][1]);
        EXPR_set(expr,"a_hot12x",a_ginfo->hotspottbl[12][0]);
        EXPR_set(expr,"a_hot12y",a_ginfo->hotspottbl[12][1]);
        EXPR_set(expr,"a_hot13x",a_ginfo->hotspottbl[13][0]);
        EXPR_set(expr,"a_hot13y",a_ginfo->hotspottbl[13][1]);
        EXPR_set(expr,"a_hot14x",a_ginfo->hotspottbl[14][0]);
        EXPR_set(expr,"a_hot14y",a_ginfo->hotspottbl[14][1]);
        EXPR_set(expr,"a_hot15x",a_ginfo->hotspottbl[15][0]);
        EXPR_set(expr,"a_hot15y",a_ginfo->hotspottbl[15][1]);
        EXPR_set(expr,"a_hot16x",a_ginfo->hotspottbl[16][0]);
        EXPR_set(expr,"a_hot16y",a_ginfo->hotspottbl[16][1]);
        EXPR_set(expr,"a_hot17x",a_ginfo->hotspottbl[17][0]);
        EXPR_set(expr,"a_hot17y",a_ginfo->hotspottbl[17][1]);
        EXPR_set(expr,"a_hot18x",a_ginfo->hotspottbl[18][0]);
        EXPR_set(expr,"a_hot18y",a_ginfo->hotspottbl[18][1]);
        EXPR_set(expr,"a_hot19x",a_ginfo->hotspottbl[19][0]);
        EXPR_set(expr,"a_hot19y",a_ginfo->hotspottbl[19][1]);
        EXPR_set(expr,"a_widthp2",MAKEPOWEROF2(a_ginfo->width));
        EXPR_set(expr,"a_heightp2",MAKEPOWEROF2(a_ginfo->height));
        EXPR_set(expr,"a_powerof2",MAKEPOWEROF2(gmax(a_ginfo->width,a_ginfo->height)));
    }


    {
        EXPRminx = gmin(ginfo->defaultx,EXPRminx);
        EXPRmaxx = gmax(ginfo->defaultx+ginfo->width,EXPRmaxx);
        EXPRminy = gmin(ginfo->defaulty,EXPRminy);
        EXPRmaxy = gmax(ginfo->defaulty+ginfo->height,EXPRmaxy);
        EXPRmaxwidth  = gmax(ginfo->width,EXPRmaxwidth);
        EXPRmaxheight = gmax(ginfo->height,EXPRmaxheight);
    }

    {
        int mm=0;

        if (ginfo->framename[0]=='m'
         && ginfo->framename[1]=='m'
         && ginfo->framename[2]>='0' && ginfo->framename[2]<='9'
         && ginfo->framename[3]>='0' && ginfo->framename[2]<='9'
         && ginfo->framename[4]=='\0')
        {
            mm=atoi(ginfo->framename+2);
        }

        EXPR_set(expr,"mipmap",(double)mm);
    }

    EXPR_set(expr,"minx",EXPRminx);
    EXPR_set(expr,"maxx",EXPRmaxx);
    EXPR_set(expr,"miny",EXPRminy);
    EXPR_set(expr,"maxy",EXPRmaxy);
    EXPR_set(expr,"maxwidth",EXPRmaxwidth);
    EXPR_set(expr,"maxheight",EXPRmaxheight);
}

GBITMAP **ProcessImage(GBITMAP *gbitmap, GABOUT *about, GABOUT *wabout, int i, int frame, int pass, int *pnout)
{
    int j;
    int width;
    int height;
    int bpp;
    int pack;
    int IOPTpack;
    int quality;
    int IOPTquality;
    int IOPTframesize;
    int IOPTdatarate;
    int alphabits;
    int redbits;
    int greenbits;
    int bluebits;
    int fok;
    int ok=1;
    int depth=0;
    int colours;
    char *f;
    char *f0;
    char **fmany = NULL;
    int nout = 1;
        // currently only for the mipmapsizecolreduce option but it stores whether
        // an upsample has happened and helps us downsample again if need be.
    SavedItems SI;
    static GINFO psdStoredGinfo;

    GINFO *ginfo;
    GINFO *ginfo0;
    GINFO **ginfomany = NULL;
    GBITMAP **gbitmapmany;
    GBITMAP *gm=0;
    GBITMAP *gaux=0;
    static int sampmsgarg=-1;
    static int samp8msgarg=-1;
    CRABOUT *cabout=0;
    double processtime[15];
    extern int galloccount;
    int composite_newpass=0;    // is this a new pass
    int facemap_newpass  =0;    // is this a new pass
    int auxillary_newpass=0;    // is this a new pass

    // structure that remembers the original bitmap
    // settings before we did any upsampling or
    // filtering. This is so we can restore this if
    // need be later.
    SI.SavedOriginalBpp        = gbitmap->ginfo->originalbpp;
    SI.SavedOriginalNumColours = gbitmap->ginfo->numcolours;
    SI.SavedABits              = gbitmap->ginfo->alphabits;
    SI.SavedRBits              = gbitmap->ginfo->redbits;
    SI.SavedGBits              = gbitmap->ginfo->greenbits;
    SI.SavedBBits              = gbitmap->ginfo->bluebits;
    SI.did_upsample            = false;

    if (OPTbreak==2)
    {
#ifdef _MSC_VER
        _asm { int 3 }
#endif
    }
    processtime[0] = SPIT_gettime();

    cabout = CRAPI_about();

    ginfo = (GINFO *) gbitmap->ginfo;
    f =      (char *) gbitmap->image;

    width   = ginfo->width;
    height  = ginfo->height;
    bpp     = ginfo->originalbpp;
    pack    = ginfo->packed;
    quality = ginfo->quality;

    if (composite_pass!=pass)   // start of pass 0 or 1?
    {
        composite_newpass = 1;
        composite_currentframe = 0;
        composite_pass=pass;
    }
    else
    {
        composite_newpass = 0;
        ++composite_currentframe;
        if (composite_currentframe > composite_maxframe)
            composite_maxframe = composite_currentframe; // inclusive last frame
    }

    if (auxillary_pass!=pass)   // start of pass 0 or 1?
    {
        auxillary_newpass = 1;
        auxillary_currentframe = 0;
        auxillary_pass=pass;
    }
    else
    {
        auxillary_newpass = 0;
        ++auxillary_currentframe;
        if (auxillary_currentframe > auxillary_maxframe)
            auxillary_maxframe = auxillary_currentframe; // inclusive last frame
    }

    if (facemap_pass!=pass)   // start of pass 0 or 1?
    {
        facemap_newpass = 1;
        facemap_currentframe = 0;
        facemap_pass=pass;
    }
    else
    {
        facemap_newpass = 0;
        ++facemap_currentframe;
        if (facemap_currentframe > facemap_maxframe)
            facemap_maxframe = facemap_currentframe; // inclusive last frame
    }

    processtime[1] = SPIT_gettime();

#if defined(_MSC_VER)
    if (OPTtime)
        SetThreadPriority(0, THREAD_PRIORITY_ABOVE_NORMAL);
#endif

    if (OPTcompositefile[0])
        gm = getnextcompositeframe();

    if (OPTauxillaryfile[0])
        gaux = getnextauxillaryframe();

continueprocess:

    UpdateExpr(ginfo,gm,gaux,composite_currentframe);

    if (OPTinfo)
    {
        showframeinfo(ginfo,NULL, false, 0,0, OPTinfo>0?OPTinfo:OPTverbose);
    }

    if (OPTwhile[0])
    {
        int ifq = (int) EXPR_get(expr,OPTwhile);
        if (!ifq) goto nowhileprocess;
    }

    if (OPTif[0])
    {
        int ifq = (int) EXPR_get(expr,OPTif);
        errorlevel = ifq;
        if (!ifq) goto noprocess;
    }
    if (OPTfif[0])
    {
        int ifq = (int) EXPR_get(expr,OPTfif);
        if (!ifq)
        {
            nout = 0;
            goto nowhileprocess;
        }
    }

    if (OPTset[0])
    {
        char *x;

        x = strchr(OPTset,'=');
        if (x)
        {
            double exprval;
            *x++ = 0;

            exprval = EXPR_get(expr, x);
            EXPR_set(expr,OPTset,exprval);

            exprval = EXPR_get(expr, OPTset);

            gprintf("expr '%s' = '%s'   %f\n",OPTset,x,exprval);

            exprval = EXPR_get(expr, "a");
            gprintf("expr 'a' %f\n",exprval);

        }
    }
    if (OPTtransparent[0] || OPTtranslucient[0])
        if (!ginfo->alphabits)
            ginfo->alphabits = 1;

    for (j=0; j<256; ++j)
    {
        if (OPTsetcolour[0])
            if (RANGE_match(OPTsetcolour,(char *)0,j))
            {
                ginfo->colourtbl[j].a = SETCOL(ginfo->colourtbl[j].a,OPTsetcolourvalue,OPTsetcolourmask,24);
                ginfo->colourtbl[j].r = SETCOL(ginfo->colourtbl[j].r,OPTsetcolourvalue,OPTsetcolourmask,16);
                ginfo->colourtbl[j].g = SETCOL(ginfo->colourtbl[j].g,OPTsetcolourvalue,OPTsetcolourmask,8);
                ginfo->colourtbl[j].b = SETCOL(ginfo->colourtbl[j].b,OPTsetcolourvalue,OPTsetcolourmask,0);
            }
        if (OPTblack[0])
            if (RANGE_match(OPTblack,(char *)0,j))
            {
                ginfo->colourtbl[j].a = 255;
                ginfo->colourtbl[j].r = 0;
                ginfo->colourtbl[j].g = 0;
                ginfo->colourtbl[j].b = 0;
            }
        if (OPTtransparent[0])
            if (RANGE_match(OPTtransparent,(char *)0,j))
            {
                ginfo->colourtbl[j].a = 0;
                ginfo->colourtbl[j].r = 0;
                ginfo->colourtbl[j].g = 0;
                ginfo->colourtbl[j].b = 0;
            }
        if (OPTtranslucient[0])
            if (RANGE_match(OPTtranslucient,(char *)0,j))
                ginfo->colourtbl[j].a = 128;
        if (OPTopaque[0])
            if (RANGE_match(OPTopaque,(char *)0,j))
                ginfo->colourtbl[j].a = 255;
    }
    // Blit a string of text into the image. 8bpp does a best match.
    // uses a referenced ttf. Tries to do arial by default
    if (OPTtextstring[0])
    {
        FINSTANCE   *fx;
        GBITMAP     source[1];
        GINFO       *ginfo0;
        FOP         fop[8];
        int         drowbytes;
        char        *f0;
        unsigned short utextstr[SPITMAXLINE];
        float pointsize=(float)OPTtextsize;

        GX_atou(utextstr,OPTtextstring);
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        if (!pointsize && OPTpointsize[0])
            pointsize = (float)GX_constrain(OPTpointsize,24.0);

        fop[0].i = (int)(OPTtextx == OPTAUTO?ginfo->width/4:OPTtextx);
        fop[1].i = (int)(OPTtexty == OPTAUTO?ginfo->height-pointsize:OPTtexty);
        fop[2].p = (const char *)utextstr;
        fop[3].p = OPTtextfont;
        fop[4].f = pointsize;
        fop[5].i = OPTtextmode;
        fop[6].i = OPTtextcolour;

        f0 = SetupFilter("Font", &fx, fop, 7, source, 1, &drowbytes, &ginfo0);

        ok = DoFilter(fx, ginfo0, f0, &ginfo, &f, drowbytes, gm, gaux);
        if (!ok)
        {
            static int fontmsg=0;

            if (!fontmsg && OPTverbose)
            {
                gprintf("gx - WARNING FONT UNABLE TO RENDER.  FONT FILE NOT FOUND? %s\n",OPTtextfont);
                fontmsg = 1;
            }
        }
    }

    /* Make image into GreyScale */
    // affects bits per channel, which the user may want to override


    if (OPTgrey)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[1];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTgrey;

        f0 = SetupFilter("Grey",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        if (OPTverbose >= 2)
            gprintf("Resampling to %d levels of grey\n", OPTgrey);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }
    // photoshop colour reduction option. Needs photoshop7.
    // to run, thus it may fail, do not report error on failure
    // just do nothing.
    if (OPTpscolred)
    {
        FINSTANCE *fx = 0;
        GBITMAP source[1];
        FOP fop[1];
        GINFO *ginfo0;
        int drowbytes;
        int rc = 0;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTpscolred;

        f0 = SetupFilter("Photoshop",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        if(f0 && fx)
        {
            rc = DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
            if (OPTverbose >= 2 && rc)
                gprintf("Colour reducing to %d using photoshop\n", OPTpscolred);
        }
        else
            if(OPTverbose >= 2)
                gprintf("Problem attempting to colour reduce, is photoshop 7 installed?\n");
    }


    /* Remap to 32 bpp */

    colours = OPTcolours;
    if (colours)
        depth = gmax(1,GXintlog2(colours-1)+1);
    if (OPTdepth)
        depth = OPTdepth;
    if (!depth)
    {
        depth = ginfo->originalbpp;
    }
    if (!OPTnoremap && depth<=8 && !wabout->export8)
        depth = 32;
    if (!OPTnoremap && depth>8 && !wabout->export32)
        depth = 8;

    if (!colours && depth>=1 && depth<=8)
    {
        if (!colours)
            colours = ginfo->numcolours;
        if (!colours || (colours > (1<<depth)))
            colours =  1<<depth;
    }

    processtime[2] = SPIT_gettime();

    IOPTpack = OPTpack[0] ? (int) GX_constrain(OPTpack, pack) : -2;
    OPTwrite = OPTwritex[0] ? (int) GX_constrain(OPTwritex, OPTwrite) : OPTwrite;

    if (!OPTnoremap && ginfo->bpp<=8 && ((colours<ginfo->numcolours && (cabout->signature!=GMAKEID('X','A','C','T')) && depth<=8) ||
        depth<ginfo->originalbpp || depth>8 || OPTcodeddepth || depth<bpp ||
        OPTzoomx[0] || OPTzoomy[0] || OPTzoomxby || OPTzoomyby ||
        OPTthumbx[0] || OPTthumby[0] || OPTthumbxby || OPTthumbyby ||
        OPTsizex[0] || OPTsizey[0] || OPTsizeyby || OPTsizeyby ||
        OPTtedge || OPToutlinew[0] || OPToutlineh[0] || OPToutlinex[0] || OPToutliney[0] ||
        OPTundot3 || OPTunembm || OPTgshunembm ||  OPTglobalpalette ||
        /*(OPTmipmap && (OPTmipmapmethod!=OPTMIPMAPBOX || OPTmipmapwidth || OPTmipmapheight)) ||*/
        OPTchroma || OPTsharpen || OPTblur[0] || OPTerode[0] || OPTdilate[0] || OPTequalize[0] ||
        OPTblendb!=100 || OPTpalfile[0] || OPTavoid[0] ||
        ((IOPTpack==3||IOPTpack==4) && (SignatureCompare(wabout->signature,GMAKEID('S','H','P',0))
			                         || SignatureCompare(wabout->signature,GMAKEID('.','X','P',0))
			                         || SignatureCompare(wabout->signature,GMAKEID('.','D','D',0)) )) ))
    {
        if (depth<=8)
            depth = 32;

        // this information is for when I'm trying to restore the original bit depth
        // after the other filter operations.

        SI.SavedOriginalBpp        = ginfo->originalbpp;
        SI.SavedOriginalNumColours = ginfo->numcolours;
        SI.SavedABits              = ginfo->alphabits;
        SI.SavedRBits              = ginfo->redbits;
        SI.SavedGBits              = ginfo->greenbits;
        SI.SavedBBits              = ginfo->bluebits;
        SI.did_upsample            = true;

        if (OPTverbose>=2 && sampmsgarg!=i)
        {
            gprintf("Resampling to %d bit\n",depth);
            sampmsgarg = i;
        }
        SPIT_flush();
        ginfo0 = info8to32(ginfo,f,ginfo->width*ginfo->bpp/8,depth);
        if (ginfo0)
        {
            f0 = (char *) galloc(ginfo0->width*ginfo0->height*ginfo0->bpp/8);
            if (f0)
            {
                if (filter8to32(ginfo,f,ginfo->width*ginfo->bpp/8,
                    ginfo0,f0,ginfo0->width*ginfo0->bpp/8,depth))
                {
                    gfree(f);
                    gfree(ginfo);
                    gbitmap->image = 0;
                    gbitmap->ginfo = 0;
                    f = f0;
                    ginfo = ginfo0;
                }
            }
        }
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }

    if ((OPTredbits>=0)
     || (ginfo->bpp==32 && depth>8 && (depth!=ginfo->originalbpp || OPTdepth)))  // not sure why the depth!=ginfo->originalbpp is there but OPTdepth check if
    {                                                                            // the user has tried to change the bit depth so we will default to original depth.
        int dither=OPTdither;

        alphabits = ginfo->alphabits;
        redbits   = ginfo->redbits  ;
        greenbits = ginfo->greenbits;
        bluebits  = ginfo->bluebits ;

        if (ginfo->bpp==32 && depth>8 && (depth!=ginfo->originalbpp || OPTdepth))
        {
            ginfo->originalbpp      = depth;
            redbits          = gmin(8,(depth+1)/3);
            greenbits        = gmin(8,(depth+2)/3);
            bluebits         = gmin(8,(depth+0)/3);
            alphabits        = gmin(8,gmax(0,depth-24));
        }

        if (OPTalphabits>=0) alphabits = OPTalphabits;
        if (OPTredbits>=0)   redbits   = OPTredbits;
        if (OPTgreenbits>=0) greenbits = OPTgreenbits;
        if (OPTbluebits>=0)  bluebits  = OPTbluebits;

        if (dither>0)
        {
            colours = OPTcolours;
            if (colours)
                depth = gmax(1,GXintlog2(colours-1)+1);
            if (OPTdepth)
                depth = OPTdepth;
            if (!depth)
            {
                depth = ginfo->originalbpp;
            }
            if (!OPTnoremap && depth>8 && (!wabout->export32 || OPTpalfile[0] || OPTavoid[0]))
                depth = 8;

            if (!colours && depth>=1 && depth<=8)
            {
                if (OPTdepth)
                    colours = 1<<OPTdepth;
                if (OPTcolours)
                    colours = OPTcolours;
                if (!colours)
                    colours = ginfo->numcolours;
                if (!colours)
                    colours = 1<<depth;
                if (colours > (1<<depth))
                    colours =  1<<depth;
            }

            if (depth>=1 && depth<=8 && wabout->export8 && (colours<ginfo->numcolours || ginfo->bpp>8 || OPTpalfile[0] || OPTavoid[0]))
            {
                dither = 0; // will do dither during colour reduction
            }
        }

        if (dither==-1)
        {
            if (alphabits < ginfo->alphabits
             || redbits < ginfo->redbits
             || greenbits < ginfo->greenbits
             || bluebits < ginfo->bluebits)
                 dither = 1;
            else
                 dither = 0;
        }

        if (OPTverbose>=2)
        {
            static int sampmsgarg=-1;
            if (sampmsgarg!=i)
            {
                if (alphabits>0)
                    gprintf("%s%s to %d%d%d%d bit\n",
                        dither>0?"Dithering":"Setting",
                        (ginfo->originalbpp<=8)?" Clut":"",
                        alphabits,redbits, greenbits, bluebits);
                else
                    gprintf("%s%s to %d%d%d bit\n",
                        dither>0?"Dithering":"Setting",
                        (ginfo->originalbpp<=8)?" Clut":"",
                        redbits, greenbits, bluebits);
                sampmsgarg=i;
            }
        }

        if (ginfo->bpp==32 && dither>0)
        {
            ginfo0 = infodither32(ginfo,f,ginfo->width*ginfo->bpp/8,
              alphabits,redbits, greenbits, bluebits);
            if (ginfo0)
            {
                f0 = (char *) galloc(ginfo0->width*ginfo0->height*ginfo0->bpp/8);
                if (f0)
                {
                    if (filterdither32(ginfo,f,ginfo->width*ginfo->bpp/8,
                        ginfo0,f0,ginfo0->width*ginfo0->bpp/8,OPTdither))
                    {
                        gfree(f);
                        gfree(ginfo);

                        f = f0;
                        ginfo = ginfo0;
                    }
                }
            }
        }
        ginfo->alphabits = alphabits;
        ginfo->redbits   = redbits;
        ginfo->greenbits = greenbits;
        ginfo->bluebits  = bluebits;
        if (ginfo->originalbpp>8)
        {
            ginfo->originalbpp = redbits+greenbits+bluebits;
            if (ginfo->originalbpp>=24 || alphabits>=4)
                ginfo->originalbpp = alphabits+redbits+greenbits+bluebits;
        }
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }

    processtime[3] = SPIT_gettime();

    /* Copy alpha to other channels */

    if (OPTacopy>=0)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[1];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTacopy;  /* method */

        f0 = SetupFilter("Acopy",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* Copy alpha to other channels */

    if (OPTdoubledither)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[4];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = ginfo->alphabits;
        fop[1].i = ginfo->redbits;
        fop[2].i = ginfo->greenbits;
        fop[3].i = ginfo->bluebits;

        f0 = SetupFilter("Doubled",&fx,fop,4,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }


    /* invert ARGBs */

    if (OPTinvert)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[1];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTinvert;  /* channel mask */

        f0 = SetupFilter("invert",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    // embm bump map from height field
    if (OPTembm)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[4];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTembm;
        fop[1].f = OPTembmscale[0]  ? (float)EXPR_get(expr,OPTembmscale) : 0.0f;
        fop[2].i = OPTembmfilter[0] ? (int)EXPR_get(expr,OPTembmfilter) : 0;
        fop[3].i = OPTembmwrap[0]   ? (int)EXPR_get(expr,OPTembmwrap) : 0;


        f0 = SetupFilter("embm",&fx,fop,4,source,1,&drowbytes,&ginfo0);

        if (OPTverbose >= 2)
        {
            gprintf("EMBM BumpMap Generator. Type %d. Scale %f. Kernel %s, %s\n",
                fop[0].i,
                fop[1].f?fop[1].f:600.0f/ginfo->width,
                fop[2].i?"1x1":"3x3",
                fop[3].i?"Wrapping":"Clamping");
        }
        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }




    // dot3 normal map from height field
    if (OPTdot3[0] || OPTdot3filter[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[3];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].f = OPTdot3[0]       ? (float)EXPR_get(expr,OPTdot3) : 0.0f;
        fop[1].i = OPTdot3filter[0] ? (int)EXPR_get(expr,OPTdot3filter) : 0;
        fop[2].i = OPTdot3wrap[0]   ? (int)EXPR_get(expr,OPTdot3wrap) : 0;

        f0 = SetupFilter("Dot3",&fx,fop,3,source,1,&drowbytes,&ginfo0);

        if (OPTverbose >= 2)
        {
            gprintf("Dot3 NormalMap Generator. Scale %f. Kernel %s, %s\n",
                fop[0].f?fop[0].f:600.0f/ginfo->width,
                fop[1].i?"1x1":"3x3",
                fop[2].i?"Wrapping":"Clamping");
        }
        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }


    /* transparent edge extrude */

    if (OPTtedge)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[1];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTtedge;  /* extrude method */

        f0 = SetupFilter("tedge",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }


    /* screen ARGBs */

    if (OPTscreenorientation!=OPTAUTO)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[2];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTscreencolour;
        fop[1].i = OPTscreenorientation;

        f0 = SetupFilter("Screen",&fx,fop,2,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

     /* replace any colour with another */

    if (OPTreplaces!=OPTAUTO)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[5];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTreplaces&OPTreplacesm; /* search */
        fop[1].i = OPTreplacer;  /* replace */
        fop[2].i = OPTreplacesm; /* search mask */
        fop[3].i = OPTreplacerm; /* dest mask */
        fop[4].i = OPTreplacesn; /* search not */

        f0 = SetupFilter("Xcolour",&fx,fop,5,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* pixel expression filter */

    if (OPTpixela[0] || OPTpixelr[0] || OPTpixelg[0] || OPTpixelb[0] || OPTpixeli[0] || OPTpixelx[0] || OPTpixely[0])
    {
        FINSTANCE *fx;
        GBITMAP source[2];
        GINFO *ginfo0;
        FOP fop[8];
        int drowbytes;
        char *f0;
        int numsources=1;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        if (gaux)
        {
            source[1].ginfo = gaux->ginfo;
            source[1].image = gaux->image;
            source[1].rowbytes = gaux->rowbytes;
            numsources=2;
        }


        fop[0].p = (char *) expr;
        fop[1].p = OPTpixela[0]?OPTpixela:"a";
        fop[2].p = OPTpixelr[0]?OPTpixelr:"r";
        fop[3].p = OPTpixelg[0]?OPTpixelg:"g";
        fop[4].p = OPTpixelb[0]?OPTpixelb:"b";
        fop[5].p = OPTpixeli[0]?OPTpixeli:"i";
        fop[6].p = OPTpixelx[0]?OPTpixelx:"x";
        fop[7].p = OPTpixely[0]?OPTpixely:"y";

        f0 = SetupFilter("Pixel",&fx,fop,8,source,numsources,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }


    /* Erode Image */

    if (OPTerode[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[1];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = (int) (EXPR_get(expr,OPTerode)+0.5);

        f0 = SetupFilter("Erode",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* Equalize Image */

    if (OPTequalize[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[1];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = (int) (EXPR_get(expr,OPTequalize));

        f0 = SetupFilter("equalize",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }


    /* Dilate Image */

    if (OPTdilate[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[1];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = (int) (EXPR_get(expr,OPTdilate)+0.5);

        f0 = SetupFilter("Dilate",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* Blur Image */

    if (OPTblur[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[1];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].f = EXPR_get(expr,OPTblur);

        f0 = SetupFilter("Blur",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* NTSC/PAL Safe */

    if (OPTsafe[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[2];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = (int) EXPR_get(expr,OPTsafe);
        fop[1].i = OPTsafe2grey;

        f0 = SetupFilter("Safe",&fx,fop,2,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }




    /* Gamma Correction */

    if (OPTgamma)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[4];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].f = 1.0;
        fop[1].f = OPTgamma;
        fop[2].f = OPTgamma;
        fop[3].f = OPTgamma;

        f0 = SetupFilter("Gamma",&fx,fop,4,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }


    /* Sharpen Image */

    if (OPTsharpen)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[1];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = (int)OPTsharpen;
        if (OPTverbose >= 2 && OPTsharpen == 2)
            gprintf("Sharpening using guassian kernel\n");
        else if (OPTverbose >= 2)
            gprintf("Sharpening image using standard kernel\n");

        f0 = SetupFilter("Sharpen",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* draw rectangle */

    if (OPTrectw[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[6];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTrectx[0] ? (int) EXPR_get(expr,OPTrectx) : 0;
        fop[1].i = OPTrecty[0] ? (int) EXPR_get(expr,OPTrecty) : 0;
        fop[2].i = OPTrectw[0] ? (int) EXPR_get(expr,OPTrectw) : 0;
        fop[3].i = OPTrecth[0] ? (int) EXPR_get(expr,OPTrecth) : 0;
        fop[4].i = OPTrectc ? OPTrectc : OPTbackground;
        fop[5].i = OPTlayered > 0; // is this a pages type

        f0 = SetupFilter("Rect",&fx,fop,6,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
        gbitmap->ginfo = ginfo;
        gbitmap->image = f;
        gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;
    }

    /* FYI adjust YIQ and alpha for ps2/xbox conversion */

    if (OPTFYIYScale[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[6];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].f = OPTFYIYScale[0]  ? EXPR_get(expr,OPTFYIYScale)  : 1.0;
        fop[1].f = OPTFYIYOffset[0] ? EXPR_get(expr,OPTFYIYOffset) : 0.0;
        fop[2].f = OPTFYICScale[0]  ? EXPR_get(expr,OPTFYICScale)  : 1.0;
        fop[3].f = OPTFYICOffset[0] ? EXPR_get(expr,OPTFYICOffset) : 0.0;
        fop[4].f = OPTFYIAScale[0]  ? EXPR_get(expr,OPTFYIAScale)  : 1.0;
        fop[5].f = OPTFYIAOffset[0] ? EXPR_get(expr,OPTFYIAOffset) : 0.0;

        if (OPTverbose>=4)
        {
            printf("fyi %f %f %f %f %f %f\n",
                fop[0].f,
                fop[1].f,
                fop[2].f,
                fop[3].f,
                fop[4].f,
                fop[5].f);
        }

        f0 = SetupFilter("FYI",&fx,fop,6,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
        gbitmap->ginfo = ginfo;
        gbitmap->image = f;
        gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;
    }

    /* draw outline */

    if (OPToutlinew[0] || OPToutlineh[0] || OPToutlinex[0] || OPToutliney[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[6];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPToutlinex[0] ? (int) EXPR_get(expr,OPToutlinex) : 0;
        fop[1].i = OPToutliney[0] ? (int) EXPR_get(expr,OPToutliney) : 0;
        fop[2].i = OPToutlinew[0] ? (int) EXPR_get(expr,OPToutlinew) : 0;
        fop[3].i = OPToutlineh[0] ? (int) EXPR_get(expr,OPToutlineh) : 0;
        fop[4].i = OPToutlinec ? OPToutlinec : OPTbackground;
        fop[5].i = OPTlayered > 0; // is this a pages type

        f0 = SetupFilter("Outline",&fx,fop,6,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
        gbitmap->ginfo = ginfo;
        gbitmap->image = f;
        gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;
    }

    /* draw grid */

    if (OPTgridx[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[6];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTgridx[0] ? (int) EXPR_get(expr,OPTgridx) : 0;
        fop[1].i = OPTgridy[0] ? (int) EXPR_get(expr,OPTgridy) : 0;
        fop[2].i = OPTgridw[0] ? (int) EXPR_get(expr,OPTgridw) : 0;
        fop[3].i = OPTgridh[0] ? (int) EXPR_get(expr,OPTgridh) : 0;
        fop[4].i = OPTgridc ? OPTgridc : OPTbackground;
        fop[5].i = OPTlayered > 0; // is this a pages type

        f0 = SetupFilter("Grid",&fx,fop,6,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
        gbitmap->ginfo = ginfo;
        gbitmap->image = f;
        gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;
    }

    /* draw line */

    if (OPTlinex[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[6];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTlinex[0] ? (int) EXPR_get(expr,OPTlinex) : 0;
        fop[1].i = OPTliney[0] ? (int) EXPR_get(expr,OPTliney) : 0;
        fop[2].i = OPTlinex2[0] ? (int) EXPR_get(expr,OPTlinex2) : 0;
        fop[3].i = OPTliney2[0] ? (int) EXPR_get(expr,OPTliney2) : 0;
        fop[4].i = OPTlinec ? OPTlinec : OPTbackground;
        fop[5].i = OPTlayered > 0; // is this a pages type

        f0 = SetupFilter("Line",&fx,fop,6,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
        gbitmap->ginfo = ginfo;
        gbitmap->image = f;
        gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;

    }


    /* Rotate shape 90 degree */

    if (OPTrot)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[4];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        ORIGUpdate(ginfo);
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;
        fop[0].i = OPTrot;
        fop[1].i = ORIGmaxwidth;                    // original max frame size.
        fop[2].i = ORIGmaxheight;
        fop[3].i = OPTlayered > 0; // is this a pages type

        f0 = SetupFilter("Rot",&fx,fop,4,source,1,&drowbytes,&ginfo0);

        FILTER_filter(fx, ginfo0, f0, drowbytes);

        if(pass == 1)
        {
            gfree(f);
            gfree(ginfo);

            f = f0;
            ginfo = ginfo0;
            gbitmap->ginfo = ginfo;
            gbitmap->image = f;
            gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;

        }
        else
        {
            gfree(f0);
            gfree(ginfo0);
        }

        FILTER_close(fx);
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }



    /* Rotate shape arbitrarily */

    if (OPTrotate)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[7];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        ORIGUpdate(ginfo);
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;
        fop[0].f = OPTrotate*2*3.141593654/360.0;
        fop[1].i = OPTbackground;
        fop[2].i = EXPRmaxwidth;                    // final max framesize.
        fop[3].i = EXPRmaxheight;
        fop[4].i = ORIGmaxwidth;                    // original max frame size.
        fop[5].i = ORIGmaxheight;
        fop[6].i = OPTlayered > 0; // is this a pages type

        f0 = SetupFilter("Rotate",&fx,fop,7,source,1,&drowbytes,&ginfo0);

        FILTER_filter(fx, ginfo0, f0, drowbytes);
       if(pass==1)  // non multiframe images will only execute on pass 1.
        {

              gfree(f);
              gfree(ginfo);
              f = f0;
              ginfo = ginfo0;

              gbitmap->ginfo = ginfo;
              gbitmap->image = f;
              gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;
        }
        else
        {
            gfree(f0);
            gfree(ginfo0);
        }
        FILTER_close(fx);
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }


    /* Zoom shape */

    if (OPTzoomx[0] || OPTzoomy[0])
    {
        GINFO *ginfo0;
        char *f0;
        applyzoom(ginfo, f, OPTzoomx, OPTzoomxby, OPTzoomy, OPTzoomyby, OPTzoomsharp, &ginfo0, &f0,about);
        if(pass == 1)
        {
            gfree(f);
            gfree(ginfo);

            f = f0;
            ginfo = ginfo0;
            gbitmap->ginfo = ginfo;
            gbitmap->image = f;
            gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;

        }
        else
        {
            gfree(f0);
            gfree(ginfo0);
        }
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }

    /* Thumb shape */

    if (OPTthumbx[0] || OPTthumby[0])
    {
        GINFO *ginfo0;
        char *f0;
        applythumb(ginfo, f, OPTthumbx, OPTthumbxby, OPTthumby, OPTthumbyby, &ginfo0, &f0,about);
        if(pass == 1)
        {
            gfree(f);
            gfree(ginfo);

            f = f0;
            ginfo = ginfo0;
            gbitmap->ginfo = ginfo;
            gbitmap->image = f;
            gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;

        }
        else
        {
            gfree(f0);
            gfree(ginfo0);
        }
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }

    /* Scale (point sample) shape */

    if (OPTscalex[0] || OPTscaley[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[5];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        int x;
        int y;
        static int psdFrameWidth = 0;
        static int psdFrameHeight = 0;
        ORIGUpdate(ginfo);
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        x = GXscaleset(ginfo->width,OPTscalex,OPTscalexby,GX_sizeratio(OPTscalex,ginfo->width*ginfo->height));
        y = GXscaleset(ginfo->height,OPTscaley,OPTscaleyby,GX_sizeratio(OPTscalex,ginfo->width*ginfo->height));
        y = GXscaledefault(y,ginfo->height,x,ginfo->width);
        x = GXscaledefault(x,ginfo->width,y,ginfo->height);

        fop[0].i = x;
        fop[1].i = y;
        fop[2].i = ORIGmaxwidth;
        fop[3].i = ORIGmaxheight;
        fop[4].i = OPTlayered > 0; // is this a pages type

        f0 = SetupFilter("Scale",&fx,fop,5,source,1,&drowbytes,&ginfo0);

        FILTER_filter(fx, ginfo0, f0, drowbytes);
        if(pass == 1)
        {
            gfree(f);
            gfree(ginfo);

            f = f0;
            ginfo = ginfo0;
            gbitmap->ginfo = ginfo;
            gbitmap->image = f;
            gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;

        }
        else
        {
            gfree(f0);
            gfree(ginfo0);
        }
        FILTER_close(fx);
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }



    /* Resize shape */

    if (OPTsizex[0] || OPTsizey[0])
    {
        GINFO *ginfo0;
        char *f0;

        applysize(ginfo, f, OPTsizex, OPTsizexby, OPTsizey, OPTsizeyby, OPTsizetrans, OPTsizechromakey, OPTsizemethod, OPTsizesharpx, OPTsizesharpy, &ginfo0, &f0,about);

        if(pass == 1)
        {
            gfree(f);
            gfree(ginfo);

            f = f0;
            ginfo = ginfo0;
            gbitmap->ginfo = ginfo;
            gbitmap->image = f;
            gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;

        }
        else
        {
            gfree(f0);
            gfree(ginfo0);
        }
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }

    /* hsv adjust */

    if (OPThsvh || OPThsvs || OPThsvb || OPThsvv)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[4];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].f = (double)OPThsvh;
        fop[1].f = (double)OPThsvs;
        fop[2].f = (double)OPThsvb;
        fop[3].f = (double)OPThsvv;

        f0 = SetupFilter("HSV",&fx,fop,4,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* hsl adjust */

    if (OPThslh || OPThsls || OPThsll)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[3];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].f = (double)OPThslh;
        fop[1].f = (double)OPThsls;
        fop[2].f = (double)OPThsll;

        f0 = SetupFilter("HSL",&fx,fop,3,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }


    /* contrast adjust */

    if (OPTcontrastc || OPTcontrastb)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[2];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].f = (double)OPTcontrastc;
        fop[1].f = (double)OPTcontrastb;

        f0 = SetupFilter("Contrast",&fx,fop,2,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }


    /* flood fill */

    if (OPTfillx[0] || OPTfillc)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[4];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTfillx[0] ? (int) EXPR_get(expr,OPTfillx) : 0;
        fop[1].i = OPTfilly[0] ? (int) EXPR_get(expr,OPTfilly) : 0;
        fop[2].i = OPTfillc ? OPTfillc : OPTbackground;
        fop[3].i = OPTlayered > 0; // is this a pages type

        f0 = SetupFilter("Fill",&fx,fop,4,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
        gbitmap->ginfo = ginfo;
        gbitmap->image = f;
        gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;

    }


    /* clean up pits and spikes on edge */

    if (OPTedgep)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[3];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTedgec;
        fop[1].i = OPTedgep;
        fop[2].i = OPTedges;

        f0 = SetupFilter("Edge",&fx,fop,3,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }



    /* chroma key */

    if (OPTchroma)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[1];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTchroma;

        f0 = SetupFilter("Chroma",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }



    /* adjust a hotspot */

    if (OPThotn>=0)
    {
        if (OPThotn>=ginfo->numhotspots)
            ginfo->numhotspots = OPThotn+1;

        ginfo->hotspottbl[OPThotn][0] += OPThotx;
        ginfo->hotspottbl[OPThotn][1] += OPThoty;
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }

    /* drop shadow a glyph */
    if (OPTdropshadow)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[34];
        GINFO *ginfo0;
        int drowbytes;
        int numopts = 33;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0 ].f = OPTDSTexPlaneOffset[0] ? (float)EXPR_get(expr,OPTDSTexPlaneOffset) : 1;      // texture plane offset down y axis.
        fop[1 ].f = OPTDSShaPlaneOffset[0] ? (float)EXPR_get(expr,OPTDSShaPlaneOffset) : 0;      // shadow plane offset.
        fop[2 ].f = OPTDSLightLocationx[0] ? (float)EXPR_get(expr,OPTDSLightLocationx) : 0;      // light location.
        fop[3 ].f = OPTDSLightLocationy[0] ? (float)EXPR_get(expr,OPTDSLightLocationy) : 3.0;
        fop[4 ].f = OPTDSLightLocationz[0] ? (float)EXPR_get(expr,OPTDSLightLocationz) : 0.2;
        fop[5 ].f = OPTDSLLookAtx[0] ? (float)EXPR_get(expr,OPTDSLLookAtx) : 0.0;                // camera look at.
        fop[6 ].f = OPTDSLLookAty[0] ? (float)EXPR_get(expr,OPTDSLLookAty) : 0.0;
        fop[7 ].f = OPTDSLLookAtz[0] ? (float)EXPR_get(expr,OPTDSLLookAtz) : 0.0;
        fop[8 ].i = OPTDSLightType;                                                              // light source type.
        fop[9 ].f = OPTDSLightDirectionx[0] ? (float)EXPR_get(expr,OPTDSLightDirectionx) : 0;    // light direction.
        fop[10].f = OPTDSLightDirectiony[0] ? (float)EXPR_get(expr,OPTDSLightDirectiony) : -3.0;
        fop[11].f = OPTDSLightDirectionz[0] ? (float)EXPR_get(expr,OPTDSLightDirectionz) : -0.2;
        fop[12].f = OPTDSRadius[0] ? (float)EXPR_get(expr,OPTDSRadius) : 5;                                          // radius of cylinder light.
        fop[13].f = OPTDSFalloff[0] ? (float)EXPR_get(expr,OPTDSFalloff) : 10;                   // falloff of light.
        fop[14].f = OPTDSFadePower[0] ? (float)EXPR_get(expr,OPTDSFadePower) : 0;                // fadepower of spotlight
        fop[15].f = OPTDSFadeDistance[0] ? (float)EXPR_get(expr,OPTDSFadeDistance) : 0;          // fade distance of spotlight.
        fop[16].f = OPTDSArLightAxis1x[0] ? (float)EXPR_get(expr,OPTDSArLightAxis1x) : 1;        // Axis 1 for my area lights.
        fop[17].f = OPTDSArLightAxis1y[0] ? (float)EXPR_get(expr,OPTDSArLightAxis1y) : 0;        // This specifies the dimensions of the
        fop[18].f = OPTDSArLightAxis1x[0] ? (float)EXPR_get(expr,OPTDSArLightAxis1z) : 0;        // Lightsource.
        fop[19].f = OPTDSArLightAxis2x[0] ? (float)EXPR_get(expr,OPTDSArLightAxis2x) : 0;        // Axis 3 for my area lights.
        fop[20].f = OPTDSArLightAxis2y[0] ? (float)EXPR_get(expr,OPTDSArLightAxis2y) : 0;        // This specifies the dimensions of the
        fop[21].f = OPTDSArLightAxis2z[0] ? (float)EXPR_get(expr,OPTDSArLightAxis2z) : 1;        // Lightsource.
        fop[22].i = OPTDSArSizeW;                                                                // number of lights in Axis1 direction.
        fop[23].i = OPTDSArSizeH;                                                                // number of lights in Axis2 direction.
        fop[24].f = OPTDSLightColourr[0] ? (float)EXPR_get(expr,OPTDSLightColourr) : 0.0;        // the colour of the back plane.
        fop[25].f = OPTDSLightColourg[0] ? (float)EXPR_get(expr,OPTDSLightColourg) : 0.0;
        fop[26].f = OPTDSLightColourb[0] ? (float)EXPR_get(expr,OPTDSLightColourb) : 0.0;
        fop[27].f = OPTDSCamLocationx[0] ? (float)EXPR_get(expr,OPTDSCamLocationx) : 0.0;        // the physical location of point camera
        fop[28].f = OPTDSCamLocationy[0] ? (float)EXPR_get(expr,OPTDSCamLocationy) : 2.0;
        fop[29].f = OPTDSCamLocationz[0] ? (float)EXPR_get(expr,OPTDSCamLocationz) : 0.0;
        fop[30].f = OPTDSLTightness[0] ? (float)EXPR_get(expr,OPTDSLTightness) : 2.0;            // the power of falloff for directional lights.
        fop[31].i = 1;              // number of passes to make over image for GUI systems.
        fop[32].i = OPTDSAntiAlias;
        // if we are going to use a common clut then we need to pass in the clut sofar
        // for all our images.
        //if(OPTDSusecommonclut && frame > 0)
        //{
        //    fop[33].p = (char *)(&DropShadowCommonClut);
        //    numopts = 34;
        //}

        // now actually find the dropshadow filter.
        f0 = SetupFilter("DropS",&fx,fop,numopts,source,1,&drowbytes,&ginfo0);

        // actually do the filtering.
        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* Wrap shape */

    if (OPTwrapx[0] || OPTwrapy[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[2];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTwrapx[0] ? (int) EXPR_get(expr,OPTwrapx) : 0;
        fop[1].i = OPTwrapy[0] ? (int) EXPR_get(expr,OPTwrapy) : 0;

        f0 = SetupFilter("Wrap",&fx,fop,2,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }


    if (OPTwarptype)
    {
        FINSTANCE *fx = NULL;
        GBITMAP source[1];
        FOP *fop = NULL;
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        int I = 0;
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;
        if(OPTwarptype == 1)
        {
            fop = (FOP *) galloc(sizeof(FOP)*8);
            fop[0].i = OPTwarptype;
            fop[1].i = OPTwarpclampx;
            fop[2].i = OPTwarpclampy;
            fop[3].i = OPTwarpc;
            fop[4].i = OPTgen;   // really x1...
            fop[5].i = OPTwarpy1;
            fop[6].i = OPTwarpx2;
            fop[7].i = OPTwarpy2;
            FILTER_find("Warp");
            FILTER_open(&fx, 1, source, 8, fop);
        }
        else if(OPTwarptype == 2)  // wave warp for now.
        {
            fop = (FOP *) galloc(sizeof(FOP)*3);
            fop[0].i = OPTwarptype;
            fop[1].i = OPTwarpclampx;
            fop[2].i = OPTwarpclampy;
            FILTER_find("Warp");
            FILTER_open(&fx, 1, source, 3, fop);
        }
        else if(OPTwarptype == 3 || OPTwarptype == 4)
        {
            int numopts = 6 + OPTgen*8;
            fop = (FOP *) galloc(sizeof(FOP)*numopts);
            fop[0].i = OPTwarptype;
            fop[1].i = OPTwarpclampx;
            fop[2].i = OPTwarpclampy;
            fop[3].i = OPTwarpc;
            fop[4].i = OPTgen;  // holds number of line pairs
            for(I = 0; I < fop[4].i;I++)
            {
                fop[5 + I*8 + 4].i = OPTwarpcontlines[I*8 + 0];
                fop[5 + I*8 + 5].i = OPTwarpcontlines[I*8 + 1];
                fop[5 + I*8 + 6].i = OPTwarpcontlines[I*8 + 2];
                fop[5 + I*8 + 7].i = OPTwarpcontlines[I*8 + 3];
                fop[5 + I*8 + 0].i = OPTwarpcontlines[I*8 + 4];
                fop[5 + I*8 + 1].i = OPTwarpcontlines[I*8 + 5];
                fop[5 + I*8 + 2].i = OPTwarpcontlines[I*8 + 6];
                fop[5 + I*8 + 3].i = OPTwarpcontlines[I*8 + 7];

            }
            FILTER_find("Warp");
            FILTER_open(&fx, 1, source, numopts, fop);
        }


        ginfo0 = FILTER_info(fx, 0);
        drowbytes = ginfo0->width * ginfo0->bpp/8;
        f0 = (char *) galloc(drowbytes * ginfo0->height);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
        gbitmap->ginfo = ginfo;
        gbitmap->image = f;
        gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;

    }
    /* Deband shape */

    if (OPTdebandrgbdist || OPTdebandradius)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[2];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTdebandrgbdist;
        fop[1].i = OPTdebandradius;

        f0 = SetupFilter("Deband",&fx,fop,2,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }


    /* Swap Colours in 8 bit shape */

    if (OPTswap1 || OPTswap2)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[2];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        if (ginfo->bpp==8)
        {
            source[0].ginfo = ginfo;
            source[0].image = f;
            source[0].rowbytes = ginfo->width*ginfo->bpp/8;

            fop[0].i = OPTswap1;
            fop[1].i = OPTswap2;

            f0 = SetupFilter("Swap",&fx,fop,2,source,1,&drowbytes,&ginfo0);

            FILTER_filter(fx, ginfo0, f0, drowbytes);

            gfree(f);
            gfree(ginfo);

            f = f0;
            ginfo = ginfo0;

            FILTER_close(fx);
        }
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }



    /* Make image into stereo gram */

    if (OPTstereofocus)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[2];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTstereofocus;
        fop[1].i = OPTstereorange;

        f0 = SetupFilter("Stereo",&fx,fop,2,source,1,&drowbytes,&ginfo0);

        if (OPTverbose >= 2)
            gprintf("Making into Stereogram\n");

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* flip */

    if (OPThflip || OPTvflip)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[4];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        ORIGUpdate(ginfo);
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = (OPTvflip?2:0)|(OPThflip?1:0);
        fop[1].i = ORIGmaxwidth;
        fop[2].i = ORIGmaxheight;
        fop[3].i = OPTlayered > 0; // is this a pages type

        f0 = SetupFilter("Flip",&fx,fop,4,source,1,&drowbytes,&ginfo0);

        FILTER_filter(fx, ginfo0, f0, drowbytes);

        if(pass == 1)
        {
            gfree(f);
            gfree(ginfo);

            f = f0;
            ginfo = ginfo0;
            gbitmap->ginfo = ginfo;
            gbitmap->image = f;
            gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;

        }
        else
        {
            gfree(f0);
            gfree(ginfo0);
        }
        FILTER_close(fx);
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }

    /* halfsize */

    if (OPThalfsize)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[7];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        bool isdot3 = false;
        if (OPTdot3[0] || OPTdot3filter[0] || (ginfo->subtype&65536 && (SignatureCompare(wabout->signature,GMAKEID('S','H','P',0)))))
            isdot3 = true;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPThalfsize; // direction=3;
        fop[1].i = 0;   // vscale=0;
        fop[2].i = 0;   // hscale=0;
        fop[3].i = 0;   // vstep=0;
        fop[4].i = 0;   // hstep=0;
        fop[5].i = 0;   // edge=0;
        fop[6].i = isdot3?1:0; // dot3=0;

        f0 = SetupFilter("Halfsize",&fx,fop,7,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* palette gradient */

    if (OPTpalgradstartindex>=0)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[6];
        int drowbytes;
        char *f0;
        char bufr[256];
        char bufg[256];
        char bufb[256];
        char bufa[256];

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        if (!OPTpalgradargbq)
        {
            OPTpalgradstartargb = *(int *)&ginfo->colourtbl[OPTpalgradstartindex];
            OPTpalgradendargb =   *(int *)&ginfo->colourtbl[OPTpalgradendindex];
        }
        sprintf(bufa,((OPTpalgradmaskargb>>24)&255)?"(i>=%d && i<=%d) ? (((%d-i)*%d+(i-%d)*%d)/%d) : a":"a",OPTpalgradstartindex,OPTpalgradendindex,OPTpalgradendindex,(OPTpalgradstartargb>>24)&255,OPTpalgradstartindex,(OPTpalgradendargb>>24)&255,OPTpalgradendindex-OPTpalgradstartindex);
        sprintf(bufr,((OPTpalgradmaskargb>>16)&255)?"(i>=%d && i<=%d) ? (((%d-i)*%d+(i-%d)*%d)/%d) : r":"r",OPTpalgradstartindex,OPTpalgradendindex,OPTpalgradendindex,(OPTpalgradstartargb>>16)&255,OPTpalgradstartindex,(OPTpalgradendargb>>16)&255,OPTpalgradendindex-OPTpalgradstartindex);
        sprintf(bufg,((OPTpalgradmaskargb>> 8)&255)?"(i>=%d && i<=%d) ? (((%d-i)*%d+(i-%d)*%d)/%d) : g":"g",OPTpalgradstartindex,OPTpalgradendindex,OPTpalgradendindex,(OPTpalgradstartargb>>8)&255, OPTpalgradstartindex,(OPTpalgradendargb>> 8)&255,OPTpalgradendindex-OPTpalgradstartindex);
        sprintf(bufb,((OPTpalgradmaskargb>> 0)&255)?"(i>=%d && i<=%d) ? (((%d-i)*%d+(i-%d)*%d)/%d) : b":"b",OPTpalgradstartindex,OPTpalgradendindex,OPTpalgradendindex,(OPTpalgradstartargb>>0)&255, OPTpalgradstartindex,(OPTpalgradendargb>> 0)&255,OPTpalgradendindex-OPTpalgradstartindex);

//gprintf("bufr: %s\n",bufr);

        fop[0].p = (char *) expr;
        fop[1].p = bufa;
        fop[2].p = bufr;
        fop[3].p = bufg;
        fop[4].p = bufb;
        fop[5].p = "i";

        f0 = SetupFilter("Pixel",&fx,fop,6,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* Copy palette range */

    if (OPTpalcopydest[0])
    {
        GX_palfunc(GXPALFUNC_COPY,OPTpalcopydest,OPTpalcopysource,ginfo,gm,gaux);
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }
    if (OPTpalmovedest[0])
    {
        GX_palfunc(GXPALFUNC_MOVE,OPTpalmovedest,OPTpalmovesource,ginfo,gm,gaux);
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }
    if (OPTpalswapdest[0])
    {
        GX_palfunc(GXPALFUNC_SWAP,OPTpalswapdest,OPTpalswapsource,ginfo,gm,gaux);
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }


    /* Deinterlace shape */

    if (OPTdeint)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[5];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = 1;
        fop[1].i = 1;
        fop[2].i = 1;
        fop[3].i = 1;
        fop[4].i = 1;;

        f0 = SetupFilter("Halfsize",&fx,fop,5,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* Temporal fix frame - if a frame is grey if was removed by WMV temporal compression so duplicate previous frame */

    if (OPTtemporalfix)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        f0 = SetupFilter("Temporalfix",&fx,NULL,0,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }


    /* grrrrbbb palettize */

    if (OPTgrb>=0)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[1];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTgrb;

        f0 = SetupFilter("Grb",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }


    /* Auto Center Point */

    if (OPTautocenter)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        f0 = SetupFilter("Acenter",&fx,0,0,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    if (OPTfdxt)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        FOP fop[1];
        static int outputCheck = 0;
        if(OPTfdxt == -1)
            fop[0].i = 0;
        else
            fop[0].i = OPTfdxt;
        if(!outputCheck)
        {
            outputCheck = 1;
            if(fop[0].i)
                gprintf("Running Image through DXT%d encoder/decoder...\n",fop[0].i);
            else
                gprintf("Running Image through DXT encoder/decoder...\n");
        }
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        f0 = SetupFilter("FDxt",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* crop image */

    if (OPTcropw[0])
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[8];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        int x;
        int y;
        int width;
        int height;
        int colour;
        int clampx;
        int clampy;
        int pagetype = OPTlayered;
        ORIGUpdate(ginfo);
        if (OPTverbose>=5 && OPTpass>=0)
        {
            gprintf("CROP bounding box %d,%d..%d,%d\n",EXPRminx,EXPRminy,EXPRmaxx,EXPRmaxy);
        }
        width  = GXscaleset(ginfo->width,OPTcropw,OPTcropwby,GX_sizeratio(OPTcropw,ginfo->width*ginfo->height));
        if (OPTcroph[0]) /* && OPTcrophby==OPTSCALEBY) */
            height = GXscaleset(ginfo->height,OPTcroph,OPTcrophby,GX_sizeratio(OPTcropw,ginfo->width*ginfo->height));
        else
            height = GXscaleset(ginfo->height,OPTcropw,OPTcropwby,GX_sizeratio(OPTcropw,ginfo->width*ginfo->height));
        x      = GXscaleset(0,OPTcropx,OPTcropwby,GX_sizeratio(OPTcropw,ginfo->width*ginfo->height));
        y      = GXscaleset(0,OPTcropy,OPTcrophby,GX_sizeratio(OPTcropw,ginfo->width*ginfo->height));
        if (!width || width==OPTAUTO) width = ginfo->width;
        if (!height || height==OPTAUTO) height = ginfo->height;
        if(OPTcropwby != OPTSCALETO)
            x = (OPTcropx[0]) ? (int)EXPR_get(expr,OPTcropx) : ((ginfo->width-width)/2);
        if(OPTcrophby != OPTSCALETO)
            y = (OPTcropy[0]) ? (int)EXPR_get(expr,OPTcropy) : ((ginfo->height-height)/2);

        if (OPTcropx[0] && OPTcropwby==OPTSCALEBY)
        {
            x = (int)EXPR_get(expr,OPTcropx);
            width = ginfo->width-(int)EXPR_get(expr,OPTcropw)-x;
        }
        if (OPTcropy[0] && OPTcrophby==OPTSCALEBY)
        {
            y = (int)EXPR_get(expr,OPTcropy);
            height = ginfo->height-(int)EXPR_get(expr,OPTcroph)-y;
        }


        if((pagetype) && !(OPTcropx[0]))
            x = (ORIGmaxwidth - width)/2;
        if((pagetype) && !(OPTcropy[0]))
            y = (ORIGmaxheight - height)/2;

        colour = (OPTcropc) ? OPTcropc : OPTbackground;
        clampx = (OPTcropclampx==OPTAUTO) ? 0 : OPTcropclampx;
        clampy = (OPTcropclampy==OPTAUTO) ? clampx : OPTcropclampy;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = x;
        fop[1].i = y;
        fop[2].i = width;
        fop[3].i = height;
        fop[4].i = colour;
        fop[5].i = clampx;
        fop[6].i = clampy;
        fop[7].i = pagetype;

        f0 = SetupFilter("Crop",&fx,fop,8,source,1,&drowbytes,&ginfo0);

        FILTER_filter(fx, ginfo0, f0, drowbytes);
//      if((OPTlayered) && pass == 0) || ((!OPTlayered) && pass == 1))   // we force two passes on pages type and so this is necessary.
        {                                                                // to allow flatten to work correctly.
            gfree(f);
            gfree(ginfo);

            f = f0;
            ginfo = ginfo0;
            gbitmap->ginfo = ginfo;
            gbitmap->image = f;
            gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;

        }
#if 0
        else
        {
            gfree(f0);
            gfree(ginfo0);
        }
#endif

        FILTER_close(fx);
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }

    /* Shrink To Fit */

    if (OPTshrinktofit)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[1];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTshrinktofit;

        f0 = SetupFilter("Shrink",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    processtime[4] = SPIT_gettime();

    /* Center point */

    EXPR_set(expr,"frame",frame);
    EXPR_set(expr,"width",ginfo->width);
    EXPR_set(expr,"height",ginfo->height);
    EXPR_set(expr,"bpp",ginfo->originalbpp);
    if (((char *)&ginfo->fps - (char *)ginfo) < ginfo->size)
        EXPR_set(expr,"fps",ginfo->fps);
    else
    {
        static int fpsmsg=0;

        if (!fpsmsg && OPTverbose)
        {
            gprintf("gx - WARNING FPS NOT SET DUE TO OLD GINFO\n");
            fpsmsg = 1;
        }
    }
    if (((char *)&ginfo->dpi - (char *)ginfo) < ginfo->size)
        EXPR_set(expr,"dpi",ginfo->dpi);
    else
    {
        static int dpimsg=0;

        if (!dpimsg && OPTverbose)
        {
            gprintf("gx - WARNING DPI NOT SET DUE TO OLD GINFO\n");
            dpimsg = 1;
        }
    }

    EXPR_set(expr,"defaultx",ginfo->defaultx);
    EXPR_set(expr,"defaulty",ginfo->defaulty);
    EXPR_set(expr,"centerx",ginfo->centerx);
    EXPR_set(expr,"centery",ginfo->centery);
    EXPR_set(expr,"numhotspots",ginfo->numhotspots);
    {
        int i;
        char hotname[32];
        for (i=0; i<GIMEX_HOTSPOTTBL_SIZE; ++i)
        {
            sprintf(hotname,"hot%dx",i);
            EXPR_set(expr,hotname,ginfo->hotspottbl[i][0]);
            sprintf(hotname,"hot%dy",i);
            EXPR_set(expr,hotname,ginfo->hotspottbl[i][1]);
        }
    }

    if (((char *)&ginfo->fps - (char *)ginfo) < ginfo->size)
        if (OPTfps[0]) ginfo->fps = (float)EXPR_get(expr,OPTfps);
    if (((char *)&ginfo->dpi - (char *)ginfo) < ginfo->size)
        if (OPTdpi[0]) ginfo->dpi = (float)EXPR_get(expr,OPTdpi);
    if (OPTdefaultx[0]) ginfo->defaultx = GXsetposition(ginfo->defaultx,OPTdefaultx,ginfo->width);
    if (OPTdefaulty[0]) ginfo->defaulty = GXsetposition(ginfo->defaulty,OPTdefaulty,ginfo->height);
    if (OPTdefaultdx[0]) ginfo->defaultx += GXsetposition(0,OPTdefaultdx,ginfo->width);
    if (OPTdefaultdy[0]) ginfo->defaulty += GXsetposition(0,OPTdefaultdy,ginfo->height);
    if (OPTcenterx[0])  ginfo->centerx =  GXsetposition(ginfo->centerx,OPTcenterx,ginfo->width);
    if (OPTcentery[0])  ginfo->centery =  GXsetposition(ginfo->centery,OPTcentery,ginfo->height);

    UpdateExpr(ginfo,gm,gaux,composite_currentframe);

    {
        int i;
        char *s;
        int firsthot=0;
        char hotstr[SPITMAXLINE*4];

        GX_choptrailingwhitespace(OPThotspots);
        s = OPThotspots;
        i = 0;
        while (*s)
        {
            char ignorefields[SPITMAXLINE*4];

            ++i;
            s = SPIT_getfield(ignorefields,s,1,0);
            if (*s==',') ++s;
        }
        s = OPThotspots;
        if (i&1)    // constrain to numhotspots
        {
            char * hottemp = s;
            s = SPIT_getfield( hotstr, s, 1,0);
            firsthot = GXsetposition(ginfo->numhotspots,hotstr,ginfo->numhotspots);
            if (OPTverbose > 2)
                gprintf("hotspot count uneven, assuming first item is starting offset of: %d\n",firsthot);

            if(firsthot >= GIMEX_HOTSPOTTBL_SIZE)
            {
                if (OPTverbose)
                {
                    gprintf("WARNING - first hotspot marker %d greater than HS table size, setting to zero\n",firsthot);
                    ++errorlevel;
                }
                firsthot = 0;
                s = hottemp;
            }
            else
                if (*s==',') ++s;
        }
        while (*s)
        {
            s = SPIT_getfield( hotstr, s, 1,0);
            ginfo->hotspottbl[firsthot][0] = GXsetposition(ginfo->hotspottbl[firsthot][0],hotstr,ginfo->width);
            if (*s==',')
            {
                s = SPIT_getfield( hotstr, s+1, 1,0);
                ginfo->hotspottbl[firsthot][1] = GXsetposition(ginfo->hotspottbl[firsthot][1],hotstr,ginfo->height);
                if (*s==',') ++s;
            }
            ++firsthot;
        }
        if (firsthot>=ginfo->numhotspots)
            ginfo->numhotspots = firsthot;
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }

    processtime[5] = SPIT_gettime();




    /* Post remap to 32 bpp */

    if (!OPTnoremap && ginfo->bpp<=8 && !wabout->export8 && wabout->export32)
    {
        depth = 32;
        if (OPTverbose>=2 && sampmsgarg!=i)
        {
            gprintf("Resampling to %d bit\n",depth);
            sampmsgarg = i;
        }
        SPIT_flush();
        ginfo0 = info8to32(ginfo,f,ginfo->width*ginfo->bpp/8,depth);
        if (ginfo0)
        {
            f0 = (char *) galloc(ginfo0->width*ginfo0->height*ginfo0->bpp/8);
            if (f0)
            {
                if (filter8to32(ginfo,f,ginfo->width*ginfo->bpp/8,
                    ginfo0,f0,ginfo0->width*ginfo0->bpp/8,depth))
                {
                    gfree(f);
                    gfree(ginfo);

                    f = f0;
                    ginfo = ginfo0;
                }
            }
        }
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }


    {
        static int noremapmsg=0;

        if (OPTnoremap && OPTverbose && !noremapmsg)
        {
            if (depth<=8 && !wabout->export8)
                gprintf("warning - OUTPUT FORMAT DOES NOT SUPPORT 8 BIT, BUT YOU'VE SPECIFIED -NOREMAP\n");
            if (depth>8 && !wabout->export32)
                gprintf("warning - OUTPUT FORMAT DOES NOT SUPPORT 32 BIT, BUT YOU'VE SPECIFIED -NOREMAP\n");
            noremapmsg = 1;
        }
    }

    /* Colour Reduction */
    // we will not do colour reduction if the user is doing
    // a -mipmapc. This is due to the fact that -mipmapc
    // needs the image to be 32 bit AND will do a colour
    // reduction after it is done mipmapping respecting the
    // usual options.

//    if(!OPTmipmap || (OPTmipmap && (OPTmipmapmethod != OPTMIPMAPSIZECOLREDUCE)))
    {
        colours = OPTcolours;
        if (colours)
            depth = gmax(1,GXintlog2(colours-1)+1);
        if (OPTdepth)
            depth = OPTdepth;
        if (!depth)
        {
            depth = ginfo->originalbpp;
        }
        if (!OPTnoremap && depth>8 && (!wabout->export32 || OPTpalfile[0] || OPTavoid[0] || OPTglobalpalette || (!wabout->export32 && wabout->globalpalette)))
            depth = 8;

        if (!colours && depth>=1 && depth<=8)
        {
            if (!colours)
                colours = ginfo->numcolours;
            if (!colours || (colours > (1<<depth)))
                colours =  1<<depth;
        }

        if (depth>=1 && depth<=8 && wabout->export8 && (colours<ginfo->numcolours || ginfo->bpp>8 || OPTpalfile[0] || OPTavoid[0] || OPTglobalpalette))
        {
            GBITMAP source;
            int staticcolours=0;
            int dynamiccolours=0;
            int exactcolours=0;
            int transluciencyflags=0;
            int transparentcolours=0;
            int transparentindex=-1;
            ARGB chromacolour;
            int chromacount;
            unsigned char colourmask[256];
            int quantmethod=OPTquantmethod;
            char *quantstr=NULL;
            CRABOUT *crapiabout=0;

            source.ginfo = ginfo;
            source.image = f;
            source.rowbytes = ginfo->width*ginfo->bpp/8;

            memset(colourmask,0xff,sizeof(colourmask));
            transluciencyflags = imagetransparent(&source,&chromacolour,&chromacount,NULL);
            transparentcolours = transluciencyflags&1;

            if (!wabout->export32 && wabout->globalpalette)
                OPTglobalpalette = 1;

            if (quantmethod==-1)
            {
                /* if <=256 colours use 'exact' crapi */

                exactcolours = isLE256Colours((unsigned char *)f, ginfo->height, ginfo->width, ginfo->width*ginfo->bpp/8);

                // could use 1x1 convention and source fnt as hints too
                bool isfont = (SignatureCompare(wabout->signature,GMAKEID('F','N','T',0)));  /* fonts have alpha */
                bool isgif  = (SignatureCompare(wabout->signature,GMAKEID('.','G','I','F')));
                bool isgloband32 = (OPTglobalpalette && ginfo->alphabits > 1 && ginfo->bpp == 32);

                if (isfont)
                    OPTglobalpalette = 1;

                // exact is perfect when less than 256 colours, but no guarantees when globalpalette, except for fonts
                if (((exactcolours && exactcolours<=colours) && !OPTglobalpalette) || isfont)
                {
                    quantmethod = 7; // Exact
                }
                // general purpose alpha channel reducer
                else if ((ginfo->alphabits>1 && (transluciencyflags&2)) || (chromacount>1) || isfont || isgif || isgloband32)
                {
                    if (OPTquantmethodalpha>=0)
                        quantmethod = OPTquantmethodalpha;  // Fu or Exact alternatives
                    else
                        quantmethod = 8; // 4d Wu
                    quantstr = "4dWU";
                    if (isgif && ginfo->alphabits>1)
                        ginfo->alphabits = 1; // 4d wu respects this
                }
                /* if bits per channel is 555 use Wu5 */
                else if (ginfo->redbits<=5 && ginfo->greenbits<=5 && ginfo->bluebits<=5)
                {
                    quantmethod = 9; // Wu555
                }
                /* default mode is Wu666 */
                else
                {
                    quantmethod = 1; // Wu
                }
                if (OPTglobalpalette)
                    OPTquantmethod = quantmethod;
            }
            if (quantmethod==0)
                quantmethod = 1;

            CRAPI_set(quantmethod);

            {
                FUverbose = OPTverbose>=3;
            }

            crapiabout = CRAPI_about();
            quantstr = crapiabout->namestr;

            // Wu methods are the only ones that need transparent added.  All other methods can handle it.
            if (!OPTtransparent[0] && !(quantmethod==1 || quantmethod==3 || quantmethod==9))
                transparentcolours = 0;

            if (transparentcolours)
            {
                transparentindex = besttransparentindex(OPTpalfilecolourtbl,OPTpalfilehistogram,OPTtransparent,OPTavoid,colours,wabout);
            }

            if (OPTpalfile[0])
            {
                if (OPTcolours)
                    staticcolours = OPTcolours;
                else
                    staticcolours = PaletteCountUsed(OPTpalfilecolourtbl,OPTpalfilehistogram,colours);
                if (staticcolours>colours)
                {
                    colours = staticcolours;
                    transparentcolours = 0;
                }
                dynamiccolours = colours-staticcolours;
                if (dynamiccolours>transparentcolours)
                    dynamiccolours -= transparentcolours;
                else
                    transparentcolours = 0;
            }
            else
            {
                int i;

                /* Count the colours actually available for dynamic selection. */
                dynamiccolours = 0;
                for (i = 0; i < colours; ++i)
                {
                    if (!ColourAvoid(i, transparentindex, 0))
                        ++dynamiccolours;
                }

                memset(OPTpalfilecolourtbl,256*sizeof(ARGB),0);
                memset(OPTpalfilehistogram,256*sizeof(int),0);
                staticcolours = 0;
            }

            if (OPTnoremap)
            {
                if (staticcolours)
                    memcpy(ginfo->colourtbl,OPTpalfilecolourtbl,256*sizeof(ARGB));
            }
            else
            {
                double t;

                if (OPTverbose>=2 && samp8msgarg!=i)
                {
                    if ( (pass == 0) || !OPTglobalpalette )
                    {
                        gprintf("Resampling to");
                        if (staticcolours)
                            gprintf(" %d static(%s)/%d dynamic colour",staticcolours,OPTpalfile,dynamiccolours);
                        else if (OPTcolours)
                            gprintf(" %d colour",dynamiccolours);
                        else
                            gprintf(" %d bit indexed",depth);

                        if (transparentcolours)
                            gprintf(" (+1 transparent)");
                        if (OPTglobalpalette)
                            gprintf(" global palette");
                        if (quantstr)
                            gprintf(" using %s",quantstr);
                        gprintf("\n");
                        samp8msgarg = i;
                    }
                    else if (pass == 1)
                    {
                        gprintf("Remapping to %d bit indexed", depth);
                        if (OPTglobalpalette)
                            gprintf(" global palette");
                        gprintf("\n");
                        samp8msgarg = i;
                    }
                }

                t = SPIT_gettime();

                ginfo0 = info32to8(ginfo,dynamiccolours,f,ginfo->width*ginfo->bpp/8,depth,pass,OPTglobalpalette);
                t = SPIT_gettime()-t;
                if (OPTtime)
                    gprintf("TIME crapi %.2f ms\n",t);

                if (ginfo0)
                {
                    // module doesn't handle alpha so set the bits on the new ginfo
                    // for the remap and handle halfalpha situation.
                    if (!crapiabout->respectalpha)
                    {
                        ginfo0->alphabits = 0;
                        // if we are going to a PS2 shape and halfalpha is turned on then
                        // stomp the clut to 128 in the alpha channel as needed.
                        if (GX_isSony(wabout) && ginfo->subtype&0x4000000) // this should be an enum!
                        {
                            for(int index = 0; index< ginfo0->numcolours; index++)
                            {
                                ginfo0->colourtbl[index].a = 128;
                            }
                        }
                    }
                    if (!OPTcolours && (dynamiccolours != ginfo0->numcolours))
                    {
                        int unusedcolours = dynamiccolours-ginfo0->numcolours;
                        colours -= unusedcolours;
                        dynamiccolours -= unusedcolours;

                        if (OPTverbose>=3)
                            gprintf("reduced to %d colours\n",dynamiccolours);
                    }

                    if (OPTinverse==0 || OPTinverse==2
                     || OPTpalfile[0]
                     || OPTtransparent[0]
                     || OPTtranslucient[0]
                     || OPTopaque[0]
                     || OPTavoid[0]
                     || transluciencyflags
                     || transparentcolours) /* disable inverse table */
// OPTdither)
                    {
                        if (OPTverbose>=5)
                            gprintf("discarding inversetable\n");
                        gxfreeinversetable();
                    }

                    if (OPTpalfile[0] || OPTavoid[0] || OPTtransparent[0] || OPTtranslucient[0] || OPTopaque[0] || OPTsetcolour[0] || OPTblack[0] || (transparentcolours && !OPTglobalpalette))
                    {
                        int newcolours = MergePalettes(ginfo0->colourtbl, colourmask, OPTpalfilecolourtbl,OPTpalfilehistogram,ginfo0->colourtbl,transparentindex, chromacolour);

                        // if we have set -8 or -n we should respect that if we can.
                        if (!OPTcolours)
                            colours = newcolours;

                        if (transparentindex<256)
                            colours = gmax(transparentindex+1,colours);

                        if (transparentcolours && OPTalphabits<0)
                            if (!ginfo0->alphabits)
                                ginfo0->alphabits = 1;
                        if (SignatureCompare(wabout->signature,GMAKEID('S','H','P','I'))  /* FSH */
                         || SignatureCompare(wabout->signature,GMAKEID('T','G','V','i'))) /* TGV */
                        {
                            if (transparentindex!=255)
                                ginfo0->alphabits = 8;
                        }

                        ginfo0->numcolours = colours;
                        ginfo0->startcolour = 0;
                        if(!OPTdepth)
                            depth = gmax(1,GXintlog2(colours-1)+1);
                    }

                    {
                        int i;

                        for (i=0; i<256; ++i)
                        {
                            if (i<colours)
                            {
                                GIMEX_destcolourmask[i] = 1;
                                if (OPTavoid[0])
                                    GIMEX_destcolourmask[i] = (unsigned char)(!RANGE_match(OPTavoid,(char *)0,i));
                            }
                            else
                                GIMEX_destcolourmask[i] = 0;
                        }
                    }


                    if (OPTalphabits>=0)
                         ginfo0->alphabits = OPTalphabits;

                    if ((OPTinverse==2 || (OPTinverse && (OPTpalfile[0] || OPTavoid[0]))) && !transluciencyflags && !OPTtransparent[0])  /* make new inverse table */
                    {

                        if (OPTverbose>=4)
                            gprintf("Building inverse table\n");
                        gmaxcolours = colours;

                        gxinitinversetable(ginfo0->colourtbl);
                    }

                    if (OPTnoremap)
                    {
                        memcpy(ginfo->colourtbl,ginfo0->colourtbl,256*sizeof(ARGB));
                        gfree(ginfo0);
                    }
                    else
                    {
                        f0 = (char *) galloc(ginfo0->width*ginfo0->height*ginfo0->bpp/8);
                        if (f0)
                        {
#if 0
                            CRINVERSETABLE *ginversetbl;
                            gxgetinversetable(&ginversetbl);
                            gprintf("Remapping...\n");
                            fok = CRAPI_remap(f0, ginfo0, ginfo0->width*ginfo0->bpp/8, f, ginfo->width*ginfo->bpp/8, ginfo->bpp, colourmask, ginversetbl, OPTdither);
#endif

                            fok = filter32to8(ginfo,f,ginfo->width*ginfo->bpp/8,ginfo0,f0,ginfo0->width*ginfo0->bpp/8,depth, colourmask);
                            if (fok)
                            {
                                gfree(f);
                                gfree(ginfo);

                                f = f0;
                                ginfo = ginfo0;
                            }
                        }
                    }

                    {
                        int i;

                        for (i=0; i<256; ++i)
                        {
                            GIMEX_destcolourmask[i] = 1;
                        }
                    }

                }
            }
            UpdateExpr(ginfo,gm,gaux,composite_currentframe);
            if (crapiabout)
                gfree(crapiabout);
        }

        if (ginfo->bpp==8 && depth>0 && depth<=8)
        {
            ginfo->originalbpp = depth;
            ginfo->numcolours = colours;
            UpdateExpr(ginfo,gm,gaux,composite_currentframe);
        }
    }

    processtime[6] = SPIT_gettime();

    /* cut alpha or pixel to half intensity */

    if (OPThalfalpha || OPThalfpixel)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[2];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPThalfalpha ? 0x80ffffff : 0x80808080;
        fop[1].i = 0xffffffff;

        f0 = SetupFilter("Shade",&fx,fop,2,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);

        if (SignatureCompare(wabout->signature,GMAKEID('S','H','P',0)))
        {
            if (ginfo->alphabits>1)
                OPTsubtypewant |= SUBHALFALPHA;
        }
    }

    /* double alpha values */

    if (OPTdoublealpha)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[2];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = 0xffffffff;  // scale alpha to 0xff
        fop[1].i = 0x80ffffff;  // from 0x80

        f0 = SetupFilter("Shade",&fx,fop,2,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);

        if (SignatureCompare(wabout->signature,GMAKEID('S','H','P',0)))
        {
            OPTsubtypenowant |= SUBHALFALPHA;
        }
    }


    /* preten ARGBs */

    if (OPTpreten!=OPTAUTO)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[1];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTpreten;  /* xor mask */

        f0 = SetupFilter("Preten",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    /* preten ARGBs */

    if (OPTmipmont)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[1];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = (IsGstFourCC(IOPTpack)?1:0);

        f0 = SetupFilter("mipmont",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        FILTER_filter(fx, ginfo0, f0, drowbytes);

        if (SignatureCompare(wabout->signature,GMAKEID('S','H','P',0)))
        {
            OPTsubtypewant |= SUBMIPMONT;
        }

        gfree(f);
        gfree(ginfo);

        f = f0;
        ginfo = ginfo0;

        FILTER_close(fx);
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }
    if(OPTa8)
    {
        for(int i = 0; i < ginfo->numcolours; i++)
        {
            ginfo->colourtbl[i].a = (unsigned char)(((double)i/(ginfo->numcolours-1))*255.0);
            ginfo->colourtbl[i].r = 255;
            ginfo->colourtbl[i].g = 255;
            ginfo->colourtbl[i].b = 255;
        }
    }

    /* Palette Sort shape */

    if (strcmp(OPTpalsortmethod, ""))
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[2];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        int i;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].p = OPTpalsortmethod;
        fop[1].i = OPTpalsortblack;

        f0 = SetupFilter("PalSort",&fx,fop,2,source,1,&drowbytes,&ginfo0);

        if ((i=FILTER_filter(fx, ginfo0, f0, drowbytes))<0)
        {
            i = -1*i;
            gprintf("Warning: sorted only on the first %d valid characters\n",i);
        }
        else if (i == 0)
            gprintf("Warning: invalid sorting method, sorted using histogram\n");

        gfree(f);
        gfree(ginfo);

        f = f0;
        ginfo = ginfo0;

        FILTER_close(fx);
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }

    if (OPTunembm)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[2];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        int unembmval = 1;
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;
        if(OPTunembm < 2)
            OPTunembm = 50;
        fop[0].i = unembmval;
        fop[1].i = OPTunembm;

        f0 = SetupFilter("UnEmbm",&fx,fop,2,source,1,&drowbytes,&ginfo0);

        if (OPTverbose >= 2)
        {
            gprintf("Making into Height field (base %d)from Embm.\n",OPTunembm);
        }
        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }
    if (OPTundot3)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[1];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].f = ((float)OPTundot3/100.0);   //0->1

        f0 = SetupFilter("UnDot3",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        if (OPTverbose >= 2)
        {
            gprintf("Making into HeightField from Dot3.\n");
        }
        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }


    if (OPTgshunembm)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        FOP fop[2];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        int unembmval = 6;                   // set to gamecube specific unembm value.
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;
        if(OPTunembm < 2)
            OPTunembm = 50;
        fop[0].i = unembmval;
        fop[1].i = OPTunembm;

        f0 = SetupFilter("UnEmbm",&fx,fop,2,source,1,&drowbytes,&ginfo0);

        if (OPTverbose >= 2)
        {
            gprintf("Making into Height field (base %d)from Embm.\n",OPTunembm);
        }
        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    if (OPTtranspose)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        int drowbytes;
        char *f0;
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        f0 = SetupFilter("Trans",&fx,0,0,source,1,&drowbytes,&ginfo0);

        if (OPTverbose >= 2)
            gprintf("Transpose Image\n");

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

#if 0
    if (OPTcomment[0])
        strcpy(ginfo->comment,OPTcomment);
#endif

    if (strcmp(OPTcomment,"%s"))
    {
        char newcomment[GIMEX_COMMENT_SIZE];

        sprintf(newcomment,OPTcomment,ginfo->comment,ginfo->comment,ginfo->comment,ginfo->comment);
        strncpy(ginfo->comment,newcomment,GIMEX_COMMENT_SIZE);
    }

    if (OPTecho[0])
    {
        char formatter[SPITMAXLINE];
        char expr0[SPITMAXLINE];
        char *s;
        double v;
        double v1;
        int vcount=0;

        s = SPIT_getfield(formatter,OPTecho,1,0);
        v = v1 = (double) ginfo->framenum;
        if (*s==',')
        {
            s = SPIT_getfield(expr0,s+1,1,0);
            v = GX_constrain(expr0, v);
            vcount = 1;
            if (*s==',')
            {
                s = SPIT_getfield(expr0,s+1,1,0);
                v1 = GX_constrain(expr0, v1);
                vcount = 2;
            }
        }

        if (!strchr(formatter,'%'))
        {
            if (vcount)
                strcat(formatter," %.2f");
            else
                strcat(formatter," %.0f");
            if (vcount>=2)
                strcat(formatter,",%.2f");
        }

        if (strstr(formatter,"%s"))
            gprintf(formatter,ginfo->framename,v,v1);
        else
            gprintf(formatter,v,v1);
        gprintf("\n");
    }

    if (OPTshell[0])
        system(OPTshell);

    /* imbed a comment string in alpha channel */

    if (OPTalphacomment>=0)
    {
        FINSTANCE *fx;
        GBITMAP source[1];
        GINFO *ginfo0;
        FOP fop[1];
        int drowbytes;
        char *f0;

        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTalphacomment;  /* the ginfo->comment will be written */

        f0 = SetupFilter("alphacomment",&fx,fop,1,source,1,&drowbytes,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,drowbytes,gm,gaux);
    }

    processtime[7] = SPIT_gettime();

    /* Name Frame */

    if (ok)
    {
        int frameindex = frame;
        char *framename = ginfo->framename; // SPITsargv[i];
        char *frameformat = OPTframename;
        char framenamebuf[SPITMAXLINE];

        // if no framename, default is filename without extension
        if (framename[0]<=' ')
        {
            framename = framenamebuf;
            SPIT_getname(framename,SPITsargv[i]); // strip path
//            strcpy(framename,SPITsargv[i]);
//            if (ginfo->framenum)
            {
                char *ext=strrchr(framename,'.');
                if (ext)
                    *ext = '\0';
//                ext = framename+strlen(framename);
//                sprintf(ext,"%d",ginfo->framenum);
            }
        }

        if (OPTcharname || OPTcharmactodos)
        {
            frameindex = GXFramenameToIndex(framename, frameindex);
            if (!frameformat[0])
                frameformat = "%04x '%c' %d";  /* framename "0041 'A' 65" */
        }

        if (OPTframenameexpr[0])
        {
            if (!stricmp(OPTframenameexpr,"a_framename")
             || !stricmp(OPTframenameexpr,"a_name"))
            {
                if (!gaux)
                {
                    if (OPTverbose)
                        gprintf("GX - NO -A AUXILLARY FILE SPECIFIED FOR A_FRAMENAME\n");
                }
                else
                    framename = gaux->ginfo->framename;
            }
            else
                frameindex = (int)GX_constrain(OPTframenameexpr,frameindex);
        }

        char framewildname[SPITMAXLINE];

        wildname(framewildname,frameformat,0,framename,frameindex+OPTstartindex);

        strcpy(ginfo->framename,framewildname);


#if 0
        {
            double t;
            t = SPIT_gettime();

            HASHID_apply(ginfo->framename,framewildname);

            t = SPIT_gettime()-t;
            if (OPTtime && OPTverbose>=4)
                gprintf("TIME hash name %.2f ms %s<-%s\n",t,ginfo->framename,framename);
        }
#endif
        processtime[8] = SPIT_gettime();

        /* Miscellaneous GINFO fields */
        /* Reset subtype, quality, framesize, packed for different file formats */

        {
            int ispal;
            int i;

            ispal = 0;
            if (ginfo->width==256 && ginfo->height==1 && ginfo->bpp==8)
            {
                ispal = 1;
                for (i=0; i<256; ++i)
                    if (f[i]!=i)
                        ispal = 0;
            }
            float fps = ginfo->fps?ginfo->fps:29.97f;

            IOPTpack = OPTpack[0] ? (int) GX_constrain(OPTpack, pack) : -2;
            IOPTquality = OPTquality[0] ? (int) GX_constrain(OPTquality, quality) : -2;
            IOPTframesize = OPTframesize[0] ? (int) GX_constrain(OPTframesize, ginfo->framesize) : -2;
            IOPTdatarate = OPTdatarate[0] ? (int) GX_constrain(OPTdatarate, ginfo->framesize*fps/1024) : -2;
            if (SignatureCompare(about->signature,wabout->signature&0xffffff00) /* same class of module */
              || (SignatureCompare(wabout->signature,GMAKEID('N','U','L','L'))) // or if null, keep for -info
              || (SignatureCompare(about->signature,GMAKEID('.','D','L','S')) && SignatureCompare(about->signature,GMAKEID('S','H','P','X')))) // DDS TO XSH is okay
            {
                if (IOPTpack>=0) pack = IOPTpack;
                ginfo->packed = pack;

                if (IOPTquality>=0) quality = IOPTquality;
                ginfo->quality = quality;

                if (OPTsubtype>=0) ginfo->subtype = OPTsubtype;
                if (OPTsubtypewant) ginfo->subtype |= OPTsubtypewant;
                if (OPTsubtypenowant) ginfo->subtype &= ~OPTsubtypenowant;

                if (IOPTframesize>=0) ginfo->framesize = IOPTframesize;

//            if (!(want&31)) want|=31;
//            OPTsubtype = ((((OPTsubtype>0)?OPTsubtype:0)|want)&~nowant)^31;

            }
            else /* different output type */
            {
                if (IOPTpack>=0) pack = IOPTpack;
                if (IOPTpack==-2) pack = 1;
                ginfo->packed = pack;

                if (IOPTquality>=0) quality = IOPTquality;
                if (IOPTquality==-2)
                {
                    quality = wabout->defaultquality;
                    if (!quality)
                        quality = 80;
                }
                ginfo->quality = quality;

                if (OPTsubtype>=0) ginfo->subtype = OPTsubtype;
                if (OPTsubtype==-2) ginfo->subtype = 0;
                if (OPTsubtypewant) ginfo->subtype |= OPTsubtypewant;
                if (OPTsubtypenowant) ginfo->subtype &= ~OPTsubtypenowant;


                if (IOPTdatarate>=0) ginfo->framesize = (int)(IOPTdatarate*1024/fps);
                else
                {
                    if (IOPTframesize>=0) ginfo->framesize = IOPTframesize;
                    if (IOPTframesize==-2) ginfo->framesize = 0;
                }
            }
            if (SignatureCompare(wabout->signature,GMAKEID('M','A','D','C')))
                ginfo->subtype |= (OPTgopp>1?0x10000:0)|(OPTgopi<<8);
            else
                ginfo->subtype |= (OPTgopi<<16)|(OPTgopp<<8);

            if (OPTsubtype==-2 && ispal && (SignatureCompare(wabout->signature,GMAKEID('S','H','P',0))))
                ginfo->subtype = 2;
        }

        processtime[9] = SPIT_gettime();

        if (OPThotadjust>=0)
        {
            int adjx;
            int adjy;
            if (OPThotadjust==0)    /* adjust from pano abs to ln center rel */
            {
                adjx = -ginfo->centerx;
                adjy = -ginfo->centery;
            }
            else                    /* adjust from ln center rel to pano abs */
            {
                adjx = ginfo->centerx;
                adjy = ginfo->centery;
            }
            for (j=0; j<ginfo->numhotspots; ++j)
            {
                ginfo->hotspottbl[j][0] += adjx;
                ginfo->hotspottbl[j][1] += adjy;
            }
        }

        // set shape alignment
        if (OPTalign[0])
        {
            ginfo0 = (GINFO *) galloc(sizeof(GINFOSHP));
            if (ginfo0)
            {
                memcpy(ginfo0,ginfo,gmin(sizeof(GINFOSHP),ginfo->size));
                gfree(ginfo);
                ginfo = ginfo0;
                ginfo->signature        = GMAKEID('S','H','P','X'); /* Type of file */
                ginfo->size             = sizeof(GINFOSHP);         /* Length of GINFO */
                ((GINFOSHP *)ginfo)->align = (int) EXPR_get(expr,OPTalign);
            }
        }

        // set shape UserSize
        if (OPTUserSize[0])
        {
            ginfo0 = (GINFO *) galloc(sizeof(GINFOSHP));
            if (ginfo0)
            {
                memcpy(ginfo0,ginfo,gmin(sizeof(GINFOSHP),ginfo->size));
                gfree(ginfo);
                ginfo = ginfo0;
                ginfo->signature        = GMAKEID('S','H','P','X'); /* Type of file */
                ginfo->size             = sizeof(GINFOSHP);         /* Length of GINFO */
                ((GINFOSHP *)ginfo)->binsize = (int) EXPR_get(expr,OPTUserSize);
            }
        }

        processtime[10] = SPIT_gettime();

        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
        for (j=0; j<256; ++j)
        {
            if (OPTsetcolour[0])
                if (RANGE_match(OPTsetcolour,(char *)0,j))
                {
                    ginfo->colourtbl[j].a = SETCOL(ginfo->colourtbl[j].a,OPTsetcolourvalue,OPTsetcolourmask,24);
                    ginfo->colourtbl[j].r = SETCOL(ginfo->colourtbl[j].r,OPTsetcolourvalue,OPTsetcolourmask,16);
                    ginfo->colourtbl[j].g = SETCOL(ginfo->colourtbl[j].g,OPTsetcolourvalue,OPTsetcolourmask,8);
                    ginfo->colourtbl[j].b = SETCOL(ginfo->colourtbl[j].b,OPTsetcolourvalue,OPTsetcolourmask,0);
                }
            if (OPTopaque[0])
                if (RANGE_match(OPTopaque,(char *)0,j))
                    ginfo->colourtbl[j].a = 255;
            if (OPTblack[0])
                if (RANGE_match(OPTblack,(char *)0,j))
                {
                    ginfo->colourtbl[j].a = 255;
                    ginfo->colourtbl[j].r = 0;
                    ginfo->colourtbl[j].g = 0;
                    ginfo->colourtbl[j].b = 0;
                }
            if (OPTtransparent[0])
                if (RANGE_match(OPTtransparent,(char *)0,j))
                {
                    ginfo->colourtbl[j].a = 0;
                    ginfo->colourtbl[j].r = 0;
                    ginfo->colourtbl[j].g = 0;
                    ginfo->colourtbl[j].b = 0;
                }
            if (OPTtranslucient[0])
                if (RANGE_match(OPTtranslucient,(char *)0,j))
                    ginfo->colourtbl[j].a = 128;
        }
    }

    processtime[11] = SPIT_gettime();

    /* Composite images (2 sources many to many) */

    if ((OPTcompositefile[0] || OPTblendf!=100))
    {
        FINSTANCE *fx;
        GBITMAP source[2];
        FOP fop[5];
        int method;
        int numsources;
        int blend0;
        int blend1;
        char *f0;
        GINFO *ginfo0;
        int rowbytes0;
        static int compmsg=0;

        if (!compmsg && OPTverbose>=2)
        {
            gprintf("compositing...\n");
            compmsg = 1;
        }

        numsources = 1;
        blend0 = OPTblendf;
        blend1 = OPTblendf;
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        if (gm)
        {
            numsources = 2;
            blend0 = OPTblendb;
            blend1 = OPTblendf;
            source[1].ginfo = gm->ginfo;
            source[1].image = gm->image;
            source[1].rowbytes = gm->rowbytes;

            if (OPTblendbehind)
            {
                GBITMAP sourcetemp;
                sourcetemp = source[0];
                source[0] = source[1];
                source[1] = sourcetemp;
            }
        }

        /* 0 alpha blend (default) */
        /* 1 paste                 */
        /* 2 sub                   */
        /* 3 add                   */
        /* 4 remove                */

        method = OPTblendmethod;
//        if (OPTpsnr)    method = 5;
        if (OPTremove)  method = 4;
        if (OPTsub)     method = 2;
        if (OPTadd)     method = 3;
        if (OPTpaste)   method = 1;

        /* blend(int method,int fillcolour,float percentage)                */

        fop[0].i = method;
        fop[1].i = OPTbackground;
        fop[2].f = (double) blend0;
        fop[3].f = (double) blend1;
        fop[4].i = OPTblendmask;

        f0 = SetupFilter("Blend",&fx,fop,5,source,numsources,&rowbytes0,&ginfo0);

        DoFilter(fx,ginfo0,f0,&ginfo,&f,rowbytes0,gm,gaux);
    }

    if (OPTMkFTexMap)              // make facial texture map
    {
        FINSTANCE *fx;
        GBITMAP source[2];
        int numsources;
        char *f0;
        GINFO *ginfo0;
        int rowbytes0;
        static int mktexmsg = 0;
        static int mktexerrormsg = 0;
        if (!mktexmsg && OPTverbose>=2)
        {
            gprintf("Creating Facial Texture Map.\n");
            mktexmsg = 1;
        }

        if (facemap_newpass)   // start of pass 0 or 1?
        {
            numsources = 1;
            source[0].ginfo = ginfo;
            source[0].image = f;
            source[0].rowbytes = ginfo->width*ginfo->bpp/8;
        }
        else
        {
            numsources = 2;
            source[0].ginfo = ginfo;
            source[0].image = f;
            source[0].rowbytes = ginfo->width*ginfo->bpp/8;

            source[1].ginfo = facemap_ginfo;
            source[1].image = facemap_f;
            source[1].rowbytes = facemap_rowbytes;

        }
        if(!facemap_newpass && (ginfo->width != facemap_ginfo->width || ginfo->height != facemap_ginfo->height))
        {
            if(!mktexerrormsg)
                gprintf("ERROR - Facial texture map requires that all textures be the same dimensions\n");
            mktexerrormsg = 1;
        }
        else
        {
            f0 = SetupFilter("FTxMp",&fx,0,0,source,numsources,&rowbytes0,&ginfo0);

            FILTER_filter(fx, ginfo0, f0, rowbytes0);

            if (facemap_f)     gfree(facemap_f);
            if (facemap_ginfo) gfree(facemap_ginfo);
            facemap_f = f0;
            facemap_ginfo = ginfo0;
            facemap_rowbytes = rowbytes0;

            FILTER_close(fx);

            if (pass==0 && composite_currentframe==0
                || pass==1 && composite_currentframe==facemap_maxframe)
            {
                nout = 1;
                if (pass==1)
                {
                    gfree(f);
                    gfree(ginfo);

                    f = f0;
                    ginfo = ginfo0;
                }
            }
            else
            {
                nout = 0;
            }
        }
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }


    /* Flatten images (composite many to 1) */
    if(OPTcram)
    {
        static GXCRAMHANDLE * gxCramHandle = NULL;
        //static UpdateFlattenDone = 0;
        if(pass == 0)
        {
            if(!GXCRAM_isInit(gxCramHandle))
                gxCramHandle = GXCRAM_Init((about->signature>>8),OPTcramtranspose,OPTcramrotate,(int)(OPTverbose>=3),OPTcramcenterpoint,OPTcramcheckedge,
                                           OPTcramoddodd,OPTcrambppsort,OPTcramclutsort,OPTcramgroupsort,OPTcramgroup,OPTcramtpagewidth,
                                           OPTcramtpageheight,OPTcramtwrapwidth,OPTcramwidthin,OPTcramspace,OPTcramx,OPTcramy,OPTcramw,OPTcramh,OPTcramnumfixed);
            GXCRAM_AddToCramList(gxCramHandle,ginfo);
        }
        if(pass == 1)
        {
            GXCRAM_DoCram(gxCramHandle,&ginfo,&f);
            // seems to be corrupting things and surprisingly things seem to work fine without this.
            /*if(!composite_newpass && !UpdateFlattenDone /*frame == 0 && OPTflatten)               // have to update the flatten information.
            {
                GXCRAM_UpdateFlatten(gxCramHandle,&composite_ginfo,&composite_f,&composite_rowbytes);
                UpdateFlattenDone = 1;
                //UpdateExpr(composite_ginfo,gm,gaux,composite_currentframe);
            }*/
            if(GXCRAM_isLast(gxCramHandle))
            {
                GXCRAM_Close(gxCramHandle);
                gxCramHandle = NULL;
            }
        }
    }
    if (OPTflatten)
    {
        FINSTANCE *fx;
//        GBITMAP *gm=0;
        GBITMAP source[2];
        FOP fop[5];
        int method;
        int numsources;
        char *f0;
        int numoptions;
        GINFO *ginfo0;
        GINFOPSD * ginfopsd = (GINFOPSD *)ginfo;
        int rowbytes0;
        static int flatmsg=0;
        static int HotSpotIndex = 0;            // use this as a means of storeing the x,y,width and height as hotspot information.
        if( OPTnovisiblecheck                                       ||     // should we check for invisible layers?
            ginfo->signature != GXFILTERMAKEFOURCC('8','B','P','S') ||     // not a PSD so go ahead.
           (ginfo->signature == GXFILTERMAKEFOURCC('8','B','P','S') &&     // this is a PSD but using old GIMEX can't get visible flag.
            ginfo->size == sizeof(GINFO)
           )                                                        ||
           (ginfo->signature == GXFILTERMAKEFOURCC('8','B','P','S') &&     // this is a PSD
            ginfo->size == sizeof(GINFOPSD)                         &&     // it has extended information.
            ginfopsd->visibleflag                                          // it is also visible.
           )
          )
        {
            if (!flatmsg && OPTverbose>=2)
            {
                gprintf("Flattening...\n");
                flatmsg = 1;
            }

            if (composite_newpass)   // start of pass 0 or 1?
            {
                numsources = 1;
                source[0].ginfo = ginfo;
                source[0].image = f;
                source[0].rowbytes = ginfo->width*ginfo->bpp/8;
            }
            else
            {
                numsources = 2;
                source[0].ginfo = composite_ginfo;
                source[0].image = composite_f;
                source[0].rowbytes = composite_rowbytes;

                source[1].ginfo = ginfo;
                source[1].image = f;
                source[1].rowbytes = ginfo->width*ginfo->bpp/8;
            }

            /* 0 alpha blend (default) */
            /* 1 paste                 */
            /* 2 sub                   */
            /* 3 add                   */
            /* 4 remove                */
            /* 5 psnr calc             */
            // if this is a PSD file and we have the extra settings in the ginfo and the user doesn't
            // want any specific blend method then use the blend method in the file.
            if(!OPTblendmethod && (ginfo->signature == GXFILTERMAKEFOURCC('8','B','P','S')) && ginfo->size == (sizeof(GINFOPSD)))
            {
                method = GetPSDBlendMethod(ginfopsd->blendmethod);
            }
            else
                method = OPTblendmethod;
            //if (OPTpsnr)    method = 5;
            if (OPTremove)  method = 4;
            if (OPTsub)     method = 2;
            if (OPTadd)     method = 3;
            if (OPTpaste)   method = 1;

            /* blend(int method,int fillcolour,float percentage)                */

            fop[0].i = method;
            fop[1].i = OPTbackground;
            fop[2].f = 100.0;
            fop[3].f = 100.0;
            fop[4].i = OPTblendmask;
// PAULDING START
            numoptions = 5;

            // if this is a PSD w/ entended information, try to use the canvas size when flattening
            if(ginfo->signature == GXFILTERMAKEFOURCC('8','B','P','S') &&
               ginfo->size == sizeof(GINFOPSD))
            {
                fop[5].i = 0;                   // use default palette blending (preserve default behavior)
                fop[6].i = ginfopsd->canvasw;
                fop[7].i = ginfopsd->canvash;

                numoptions = 8;
            }

//            f0 = SetupFilter("Blend",&fx,fop,5,source,numsources,&rowbytes0,&ginfo0);
            f0 = SetupFilter("Blend",&fx,fop,numoptions,source,numsources,&rowbytes0,&ginfo0);
// PAULDING END

            FILTER_filter(fx, ginfo0, f0, rowbytes0);

            if (OPTflatten == 2)        // if we want to save the original image positions
            {                           // as hotspots.
                if(pass == 1)           // only do this on one of the passes either 0 or 1 will do.
                {
                    if (ginfo0->numhotspots<GIMEX_HOTSPOTTBL_SIZE)
                    {
                        if (composite_f)
                            memcpy(&ginfo0->hotspottbl[0][0],&composite_ginfo->hotspottbl[0][0],sizeof(ginfo0->hotspottbl));

                        ginfo0->numhotspots += 2;                               // let the new ginfo struct know the new number of hotspots
                        ginfo0->hotspottbl[HotSpotIndex][0] = ginfo->defaultx;      // store x and y first
                        ginfo0->hotspottbl[HotSpotIndex][1] = ginfo->defaulty;
                        ginfo0->hotspottbl[HotSpotIndex + 1][0] = ginfo->width; // store also the old width and height as they are also useful.
                        ginfo0->hotspottbl[HotSpotIndex + 1][1] = ginfo->height;

                        if ((strlen(ginfo0->comment)+strlen(ginfo->framename)+1)<sizeof(ginfo0->comment))
                        {
                            strcat(ginfo0->comment,",");
                            strcat(ginfo0->comment,ginfo->framename);
                            ginfo0->comment[sizeof(ginfo0->comment)-1] = (char) 0;
                        }
                        HotSpotIndex+=2;                                        // increment my indexer var.
                    }
                }
            }
            if (composite_f)     gfree(composite_f);
            if (composite_ginfo) gfree(composite_ginfo);
            composite_f = f0;
            composite_ginfo = ginfo0;
            composite_rowbytes = rowbytes0;

            FILTER_close(fx);
    //        if (gm)
    //            GIMEX_freebitmap(gm);

            if (pass==0 && composite_currentframe==0
             || pass==1 && composite_currentframe==composite_maxframe)
            {
                nout = 1;
                if (pass==1)
                {
                    gfree(f);
                    gfree(ginfo);

                    f = f0;
                    ginfo = ginfo0;
                }
            }
            else
            {
                nout = 0;
            }

            if (OPTverbose>=5)
                gprintf("flattening frame %d, pass %d to %d\n",composite_currentframe, pass, nout);
        }
        UpdateExpr(ginfo,gm,gaux,composite_currentframe);
    }

    processtime[12] = SPIT_gettime();

    /* Mipmap - generates multiple images from one */

    if (OPTmipmap)
    {
        FINSTANCE * fx     = 0;
        int sizex          = (OPTmipmapwidth  ? OPTmipmapwidth  : ginfo->width);
        int sizey          = (OPTmipmapheight ? OPTmipmapheight : ginfo->height);
        int mip_collapse   = 0;
        GBITMAP source[1];
        FOP fop[9];

        // WARNING CHECKS
         if (ginfo->framename[0]=='m'
         && ginfo->framename[1]=='m'
         && ginfo->framename[2]>='0'
         && ginfo->framename[2]<='9') // ignore 3rd, for backward compatibility to old shape files
        {
            static int mipmappedmsg=0;

            if (!mipmappedmsg && OPTverbose)
                gprintf("gx - WARNING. IMAGE IS ALREADY MIPMAPPED\n");
            strcpy(ginfo->framename,"base"); // force name to not be mm##
        }
        if (!(OPTsubtype>=0) && !(OPTsubtypewant&SUBNOMIPMAPCOLLAPSE))
            mip_collapse = 1;
        // MIPMAP OPTIONS SETUP
        source[0].ginfo = ginfo;
        source[0].image = f;
        source[0].rowbytes = ginfo->width*ginfo->bpp/8;

        fop[0].i = OPTmipmapmethod;
        fop[1].i = OPTmipmap;
        fop[2].i = OPTmipmapmin;
        fop[3].i = sizex;
        fop[4].i = sizey;
        fop[5].i = OPTmipmapclearlast;
        fop[6].i = OPTbackground;
        fop[7].i = OPTmipmaptrans;
        fop[8].i = mip_collapse;
        // setup filter for mipmapping
        FILTER_find("Mipmap");
        int nFrames          = FILTER_open(&fx,1,source,9,fop);
        int ind              = 0;
        int mipmap_error     = 0;
        int ok               = 0;

        if(nFrames > 0)
        {
            ginfomany = (GINFO **)galloc(sizeof(GINFO*)*nFrames);
            fmany     = (char **)galloc(sizeof(char *)*nFrames);
        }
        // loop over all levels and get ginfo's and framebuffers
        for(ind = 0; ind < nFrames; ind++)
        {
            ginfomany[ind]   = FILTER_info(fx,ind);
            if(ginfomany[ind])
            {
                int rb           = ginfomany[ind]->width* ginfomany[ind]->bpp/8;     // calculate the rowbytes for this image
                fmany[ind]       = (char *)galloc(rb*ginfomany[ind]->height);        // create my new framebuffer.
                if(fmany[ind])
                    ok           = FILTER_filter(fx,ginfomany[ind],fmany[ind],rb);   // get the next mipmap
            }
            // if FILTER reported an error or we had memory problems then abort.
            if(!ok || !ginfomany[ind] || !fmany[ind])
            {
                mipmap_error = 1;
                break;
            }
        }
        FILTER_close(fx);
        // if I encountered a problem I better cleanup everything
        if(mipmap_error)
        {
            nout = 0;
            for(;ind >= 0; ind--)
            {
                if(ginfomany[ind])
                    gfree(ginfomany[ind]);
                if(fmany[ind])
                    gfree(fmany[ind]);
                gfree(ginfomany);
                gfree(fmany);
            }
        }
        // otherwise I can tell gx to write out all the
        // bitmaps I just generated.
        else
        {
            if(nFrames != 0)
                nout = nFrames;
        }
    }



noprocess:

    if (OPTwhile[0])
        goto continueprocess;

nowhileprocess:


    /* warning on problematic fully transparent image */

    {
        int transluciencyflags=0;
        int chromacount;
        int transparentcount;
        ARGB chromacolour;
        GBITMAP source;
        int suppresswarning=0;
        static int warntransparent=0;
        static int warnps2alphaoverflow=0;

        source.ginfo = ginfo;
        source.image = f;
        source.rowbytes = ginfo->width*ginfo->bpp/8;
        transluciencyflags = imagetransparent(&source,&chromacolour,&chromacount,&transparentcount);

        if (transparentcount>=ginfo->width * ginfo->height)
        {
            // special case... purposeful place holder images must be 1x1

            if (ginfo->width==1 && ginfo->height==1)
                suppresswarning = 1;

            // suppress warning if user is not outputting the image

            if (SignatureCompare(wabout->signature,GMAKEID('N','U','L','L')))   /* null driver */
                suppresswarning = 1;

            if (!suppresswarning && !warntransparent && OPTverbose)
            {
                gprintf("gx - WARNING.  IMAGE IS ENTIRELY TRANSPARENT. Frame #%d: %s\n",ginfo->framenum,ginfo->framename);
                warntransparent = 1;
            }
        }

        // alpha values 129..255 are problematic on ps2
        if (transluciencyflags&4  // has alpha >128
         && ginfo->alphabits>1    // has alpha channel
         && ginfo->packed<3       // ignore dxt/mpeg alpha channels
         && GX_isSony(wabout))    // PS2
        {
            if (!warnps2alphaoverflow && OPTverbose)
            {
                gprintf("gx - WARNING.  IMAGE HAS ALPHA VALUES GREATER THAN 128\n");
                warnps2alphaoverflow = 1;
            }
        }

    }

    processtime[13] = SPIT_gettime();

    *pnout = nout;

    gbitmapmany = (GBITMAP **) 0;
    if (nout)
    {
        gbitmapmany = (GBITMAP **)galloc(nout*sizeof(GBITMAP *));

        gbitmap->ginfo = ginfo;
        gbitmap->image = f;
        gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;
        gbitmapmany[0] = gbitmap;

        if (ginfomany) // mipmap
        {
            for (j = 0; j < nout; ++j)
            {
                gbitmapmany[j] = (GBITMAP *)galloc(sizeof(GBITMAP));

                if (gbitmapmany[j])
                {
                    gbitmapmany[j]->ginfo = ginfomany[j];
                    gbitmapmany[j]->image = fmany[j];
                    gbitmapmany[j]->rowbytes = ginfomany[j]->width*ginfomany[j]->bpp/8;
                }
            }

            /* Free pointer arrays and original bitmap. */
            gfree(ginfomany);
            gfree(fmany);
            GIMEX_freebitmap(gbitmap);
        }
    }
    else
    {
        if(f)                   // this is a dangerous place here. f  and ginfo could point to gbitmaps
            gfree(f);           // image data OR it could point to some other image data
        if(ginfo)               // so first if it's allocated free it and this will cover the case
            gfree(ginfo);       // where it points to gbitmaps data.. BUT we then have to set these
        gbitmap->ginfo = 0;     // pointers to null so that we dont free them later when we attempt to
        gbitmap->image = 0;     // free the bitmap itself.
        if(gbitmap)
            GIMEX_freebitmap(gbitmap);
    }

    // -return expression deferred as long as possible

    if (OPTreturn[0])
    {
        errorlevel = (int) EXPR_get(expr,OPTreturn);
    }

    if (gm)
        GIMEX_freebitmap(gm);
    if (gaux)
        GIMEX_freebitmap(gaux);

    gfree(cabout);

    processtime[14] = SPIT_gettime();

    if (OPTtime)
    {
        static int timemsgarg=-1;
#if defined(_MSC_VER)
        SetThreadPriority(0, THREAD_PRIORITY_NORMAL);
#endif

        if (OPTverbose>=5)
        {
            int i;
            gprintf("TIME processimage");
            for (i=0; i<14; ++i)
                gprintf(" %.2f",processtime[i+1]-processtime[i]);
            gprintf(" ms\n");
        }
        else if (OPTverbose>=3 || (OPTverbose>=1 && timemsgarg!=i && (processtime[11]-processtime[0])>0.0))
        {
            gprintf("TIME processimage %.2f ms\n",processtime[11]-processtime[0]);
        }
        timemsgarg = i;
    }
    if (OPTverbose>=5)
        gprintf("galloc count %d", galloccount);

    return(gbitmapmany);
}


void GXFILTER_reset(void)
{
    OPTa8           = 0;
    OPTkeepopts = 1;
    OPTopenastype = 0;
    OPTtype = 0;
    OPTcolours = 0;
    OPTstartindex = 0;
    OPTnoremap = 0;
    OPTcharname = 0;
    OPTcharmactodos = 0;
    OPTacopy = -1;
    OPTdoubledither = 0;
    OPTgrey = 0;
    OPTstereofocus = 0;
    OPTstereorange = 0;
    OPTembm = 0;
    OPTembmscale[0] = 0;
    OPTembmfilter[0] = 0;
    OPTembmwrap[0] = 0;
    OPTunembm = 0;
    OPTgshunembm = 0;
    OPTdot3[0] = 0;
    OPTdot3filter[0] = 0;
    OPTdot3wrap[0] = 0;
    OPTundot3 = 0;
    OPTinvert = 0;
    OPTtedge = 0;
    OPTscreencolour = 0;
    OPTscreenorientation = OPTAUTO;
    OPTmipmont = 0;
    OPTpreten = OPTAUTO;
    OPTblendf = 100;
    OPTblendb = 100;
    OPTblendmask = 0xffffffff;
    OPTblendmethod = 0;  // alpha blend
    OPTblendbehind = 0;  // blend composite on top
    OPTgamma = 0.0;
    OPTsharpen = 0;
    OPTshrinktofit = 0;
    OPTautocenter = 0;
    OPTremove = 0;
    OPTsub = 0;
    OPTadd = 0;
    OPTpaste = 0;
    OPTdepth = 0;
    OPTcodeddepth = 0;
    OPToverwrite = 1;
    strcpy(OPTpack,"-2");
    strcpy(OPTquality,"-2");
    OPTsubtype = -2;
    OPTsubtypewant = 0;
    OPTsubtypenowant = 0;
//    OPTdither = -1;
    OPTdither = 0;
    OPTgrb = -1;
    OPTpalgradstartindex = -1;
    OPTpalgradendindex = 255;
    OPTpalgradargbq = 0;
    OPTpalgradstartargb = 0xff000000;
    OPTpalgradendargb = 0xffffffff;
    OPTpalgradmaskargb = 0xffffffff;
    OPThalfsize = 0;
    OPThalfalpha = 0;
    OPThalfpixel = 0;
    OPTdoublealpha = 0;
    OPThflip = 0;
    OPTvflip = 0;
    OPTsorttype = 0;
    OPThotadjust = -1;  /* no adjustment */
    OPTinverse = 1;
    OPTpass = -1;
    OPTquantmethod = 4;
    OPTquantmethodalpha = 4;
    OPTpschannel = -1;
    OPTalphabits = -1;   /* use original values */
    OPTredbits = -1;
    OPTgreenbits = -1;
    OPTbluebits = -1;
    strcpy(OPTframename,"%s");
    OPTframenameexpr[0] = 0;
//    OPTcomment[0] = 0;
    strcpy(OPTcomment,"%s"); // default to not changing comment
    OPTshell[0] = 0;
    OPTecho[0] = 0;
    OPTframes[0] = 0;
    OPTfilterframes[0] = 0;
    OPTopaque[0] = 0;
    OPTblack[0] = 0;
    OPTtransparent[0] = 0;
    OPTtranslucient[0] = 0;
    OPTpalcopydest[0]= 0;
    OPTpalcopysource[0] = 0;
    OPTpalmovedest[0]= 0;
    OPTpalmovesource[0] = 0;
    OPTpalswapdest[0]= 0;
    OPTpalswapsource[0] = 0;
    OPTsetcolour[0] = 0;
    OPTsetcolourvalue = 0;
    OPTsetcolourmask = 0xffffffff;
    OPTpalfile[0] = 0;
    OPTcompositefile[0] = 0;
    OPTauxillaryfile[0] = 0;
    OPTlogfile[0] = 0;
    OPTavoid[0] = 0;
    OPTreturn[0] = 0;
    OPTif[0] = 0;
    OPTwhile[0] = 0;
    OPTfps[0] = 0;
    OPTdpi[0] = 0;
    OPTalign[0] = 0;
    OPTUserSize[0] = 0;
    OPTsafe[0] = 0;
    OPTsafe2grey = 0;
    OPTblur[0] = 0;
    OPTerode[0] = 0;
    OPTequalize[0] = 0;
    OPTdilate[0] = 0;
    OPTdefaultx[0] = 0;
    OPTdefaulty[0] = 0;
    OPTdefaultdx[0] = 0;
    OPTdefaultdy[0] = 0;
    OPTcenterx[0] = 0;
    OPTcentery[0] = 0;
    OPThotspots[0] = 0;
    OPTpixela[0] = 0;
    OPTpixelr[0] = 0;
    OPTpixelg[0] = 0;
    OPTpixelb[0] = 0;
    OPTpixeli[0] = 0;
    OPTpixelx[0] = 0;
    OPTpixely[0] = 0;
    OPTset[0] = 0;
    OPTcropw[0] = 0;
    OPTcroph[0] = 0;
    OPTcropwby = 0;
    OPTcrophby = 0;
    OPTcropx[0] = 0;
    OPTcropy[0] = 0;
    OPTcropc = 0;
    OPTcropclampx = OPTAUTO;
    OPTcropclampy = OPTAUTO;
    OPTcram = 0;
    OPTcramtranspose = 0;
    OPTcramrotate = 0;
    OPTcramcenterpoint = 0;
    OPTcramcheckedge = 0;
    OPTcramoddodd = 0;
    OPTcrambppsort = 0;
    OPTcramclutsort = 0;
    OPTcramgroupsort = 0;
    memset(OPTcramgroup,0,sizeof(GXCRAMGROUP)*20);
    OPTcramtpagewidth  = 256;
    OPTcramtpageheight = 256;
    OPTcramtwrapwidth = 0;
    OPTcramwidthin = 0;
    OPTcramspace = 0;
    OPTcramx = 0;
    OPTcramy = 0;
    OPTcramw = 1024;
    OPTcramh = 256;
    OPTcramnumfixed = 0;
    OPThsvh = 0;
    OPThsvs = 0;
    OPThsvb = 0;
    OPThsvv = 0;
    OPThslh = 0;
    OPThsls = 0;
    OPThsll = 0;
    OPTcontrastc = 0;
    OPTcontrastb = 0;
    OPTreplaces  = OPTAUTO;
    OPTreplacer  = 0x00000000;
    OPTreplacesm = 0xf8f8f8f8;
    OPTreplacerm = 0xffffffff;
    OPTreplacesn = 0;
    OPTgopi = 0;
    OPTgopp = 0;
    OPTrectx[0] = 0;
    OPTrecty[0] = 0;
    OPTrectw[0] = 0;
    OPTrecth[0] = 0;
    OPTrectc = 0;
    OPToutlinex[0] = 0;
    OPToutliney[0] = 0;
    OPToutlinew[0] = 0;
    OPToutlineh[0] = 0;
    OPToutlinec = 0;
    OPTgridx[0] = 0;
    OPTgridy[0] = 0;
    OPTgridw[0] = 0;
    OPTgridh[0] = 0;
    OPTgridc = 0;
    OPTlinex[0] = 0;
    OPTliney[0] = 0;
    OPTlinex2[0] = 0;
    OPTliney2[0] = 0;
    OPTlinec = 0;
    OPTfillx[0] = 0;
    OPTfilly[0] = 0;
    OPTfillc = 0;
    OPTchroma = 0;
    OPTedgec = 0;
    OPTedgep = 0;
    OPTedges = 0;
    OPThotn = -1;
    OPThotx = 0;
    OPThoty = 0;
    OPTpscolred = 0;
//    OPTshadowx1 = 0;
//    OPTshadowx2 = 0;
//    OPTshadowy = 0;
//    OPTshadowdx = 0;
//    OPTshadowdy = 0;
    OPTdropshadow = 0;
//      OPTDSusecommonclut = 0;
    OPTDSAntiAlias = 1;
    strcpy(OPTDSLightLocationx,"0.0");
    strcpy(OPTDSLightLocationy,"3.0");
    strcpy(OPTDSLightLocationz,"-0.2");
    OPTDSLightType = 0;
    strcpy(OPTDSLightDirectionx,"0");
    strcpy(OPTDSLightDirectiony,"-3.0");
    strcpy(OPTDSLightDirectionz,"-0.2");
    strcpy(OPTDSRadius,"5.0");
    strcpy(OPTDSFalloff,"10.0");
    strcpy(OPTDSFadePower,"0");
    strcpy(OPTDSFadeDistance,"0");
    strcpy(OPTDSArLightAxis1x,"1");
    strcpy(OPTDSArLightAxis1y,"0");
    strcpy(OPTDSArLightAxis1z,"0");
    strcpy(OPTDSArLightAxis2x,"0");
    strcpy(OPTDSArLightAxis2y,"0");
    strcpy(OPTDSArLightAxis2z,"1");
    strcpy(OPTDSLTightness,"2.0");
    strcpy(OPTDSLLookAtx,"0");
    strcpy(OPTDSLLookAty,"0");
    strcpy(OPTDSLLookAtz,"0");
    strcpy(OPTDSLightColourr,"0");
    strcpy(OPTDSLightColourg,"0");
    strcpy(OPTDSLightColourb,"0");
    strcpy(OPTDSCamLocationx,"0");
    strcpy(OPTDSCamLocationy,"2.0");
    strcpy(OPTDSCamLocationz,"0");
    strcpy(OPTDSTexPlaneOffset,"1");
    strcpy(OPTDSShaPlaneOffset,"0");
    OPTDSArSizeW = 1;
    OPTDSArSizeH = 1;
    OPTclutx = 0;
    OPTcluty = 0;
    OPTnovisiblecheck = 0;
    strcpy(OPTframesize,"-2");
    OPTzoomx[0] = 0;
    OPTzoomy[0] = 0;
    OPTzoomxby = 0;
    OPTzoomyby = 0;
    OPTzoomsharp = 100;
    OPTthumbx[0] = 0;
    OPTthumby[0] = 0;
    OPTthumbxby = 0;
    OPTthumbyby = 0;
    OPTsizex[0] = 0;
    OPTsizey[0] = 0;
    OPTsizexby = 0;
    OPTsizeyby = 0;
    OPTsizetrans = 0;
    OPTsizechromakey = 0;
    OPTsizemethod = 0;
    OPTsizesharpx = 100;
    OPTsizesharpy = 100;
    OPTscalex[0] = 0;
    OPTscaley[0] = 0;
    OPTscalexby = 0;
    OPTscaleyby = 0;
    OPTmipmap = 0;
    OPTmipmapmethod = OPTMIPMAPBOX;
    OPTmipmapwidth = 0;
    OPTmipmapheight = 0;
    OPTmipmapmin = 0;
    OPTmipmaptrans = 0;
    OPTmipmapchromakey = 0;
    OPTmipmapclearlast = 0;
    OPTwrapx[0] = 0;
    OPTwrapy[0] = 0;
    OPTgen = 0;
    OPTwarpx2 = 0;
    OPTwarpy1 = 0;
    OPTwarpy2 = 0;
    OPTwarpc = 0;
    OPTwarpclampx = 0;
    OPTwarpclampy = 0;
    OPTdebandrgbdist = 0;
    OPTdebandradius = 0;
    OPTswap1 = 0;
    OPTswap2 = 0;
    strcpy(OPTpalsortmethod,"");
    OPTpalsortblack = 0;
    OPTrot = 0;
    OPTrotate = 0;
    OPTtranspose = 0;
    OPTdeint = 0;
    OPTtemporalfix = 0;
    OPTextract = 0;
    OPTsemitrans = 0;
    OPTglobalframe = 0;
    OPTpsflattened = 0;
    OPTflatten = 0;
    OPTfdxt = 0;
    OPTglobalpalette = 0;
    OPTalphacomment = -1;
//    OPTtime = 0;
    OPThashlen = 16;  // longer is ok, but slower
//    strcpy(OPTurl, "http://tools.eac.ea.com/gx/index.htm");
    OPTlayered = 0;
    strcpy(OPTtextstring, "");
    strcpy(OPTtextfont, "c:\\windows\\fonts\\arial.ttf");
    OPTtextx      = OPTAUTO;
    OPTtexty      = OPTAUTO;
    OPTtextsize   = 24;
    OPTtextcolour = 0xFFFFFFFF;
    OPTtextmode   = 0;

    // reset font specific options
    strcpy(OPTpointsize,"");
    strcpy(OPTantialias,"1");
    strcpy(OPTstyle,"0");
    strcpy(OPTcharset,"0");
    strcpy(OPTascii,"0");
    strcpy(OPTtypeface,"");
    strcpy(OPTaspect,"1");
    strcpy(OPTtighten,"1");
    strcpy(OPTtracking,"0");
    strcpy(OPTsmallcaps,"0");
    strcpy(OPTbaseline,"0");
    OPTitalic = 0;
    OPTbold = 0;

    OPTbreak = 0;
    OPTprioritylow = 0;
    OPTinfo = 0;

    OPTFYIYScale[0] = 0;
    OPTFYIYOffset[0] = 0;
    OPTFYICScale[0] = 0;
    OPTFYICOffset[0] = 0;
    OPTFYIAScale[0] = 0;
    OPTFYIAOffset[0] = 0;

    CR_reset();

    compositeindex = 0;
    auxillaryindex = 0;
    paletteindex = 0;
    writemsgarg = -1;

    composite_pass=-1;
    composite_maxframe=0; // max
    composite_currentframe=0; // current

    composite_f=0;
    composite_ginfo=0;
    composite_rowbytes=0;

    auxillary_pass=-1;
    auxillary_maxframe=0; // max
    auxillary_currentframe=0; // current

    auxillary_f=0;
    auxillary_ginfo=0;
    auxillary_rowbytes=0;

    facemap_pass=-1;
    facemap_maxframe=0; // max
    facemap_currentframe=0; // current

    facemap_f=0;
    facemap_ginfo=0;
    facemap_rowbytes=0;

    EXPRminx = EXPRminy = 1<<30;
    EXPRmaxx = EXPRmaxy = EXPRmaxwidth = EXPRmaxheight = -(1<<30);

    OPTlog = 1;
}

void GXFILTER_init(void)
{
    if (!expr)
    {
        expr = EXPR_open();
//        EXPR_addfunc(expr,"getcolour",&GXEXPR_getcolour,1);
    }
}

void GXFILTER_restore(void)
{
    if (expr)
    {
        EXPR_close(expr);
        expr = 0;
    }
}

// calculate a special original max height and widht for the image.
void ORIGUpdate(GINFO *ginfo)
{
    ORIGmaxwidth  = gmax(ginfo->width + ginfo->defaultx,ORIGmaxwidth);
    ORIGmaxheight = gmax(ginfo->height + ginfo->defaulty,ORIGmaxheight);
}

#undef GXFILTERMAKEFOURCC

#define HARDSPACE 28
static void GX_choptrailingwhitespace(char * str)
{
    char * p = str + strlen(str) - 1;
    while(p != str && (*p == ' ' || *p == HARDSPACE))
    {
        *p = '\0';
        p--;
    }
}
#undef HARDSPACE




