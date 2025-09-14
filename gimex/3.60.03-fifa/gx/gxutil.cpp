
#include "gxutil.h"
#include "stdgimex.h"
/****************************************************************************/
/* Internal functions                                                       */
/****************************************************************************/
namespace gxutil {
//------------------------------------------------------------------------
// check if a file exists by attempting to open the file and checking
// success.
static int GX_fileexist(char *nextname)
{
    int ok=0;
    GSTREAM *g;
    g = gopen(nextname);
    if (g)
    {
        ok = 1;
        gclose(g);
    }
    return(ok);
}

#define ISPOWER2(v) (!((v)&((v)-1)))
//------------------------------------------------------------------------
// calculate the correct width and height for any swizzled GSH.
// these come from gimex. We should really find a way of sharing
// the code. Frank and I will have to discuss it.
static int IsValidGSHSwizzleWidthHeight(int w,int h,int bpp)
{
    int realwidth  = w;
    int realheight = h;
    int lowheight  = 4;
    switch (bpp)
    {
        case 4:// size need to be div by 8x8 for 4 bit else can not twiddle
        {
            realwidth  = (w + 7)&~7;
            realheight = (h + 7)&~7;
            lowheight = 8;
            break;
        }
        case 8:// size needs to be div by 8x4 for 8 bit else can not twiddle
        {
            realwidth  = (w + 7)&~7;
            realheight = (h + 3)&~3;
            lowheight  = 8;
            break;
        }
        case 15:
        case 16:
        case 32://size needs to be div by 4x4 for higher bit else can not twiddle
        {
            realwidth  = (w + 3)&~3;
            realheight = (h + 3)&~3;
            break;
        }
    }
    if((realwidth != w || realheight != h) && (h > lowheight || w > lowheight))
        return 0;
    else
        return 1;
}

//------------------------------------------------------------------------
// calculate the correct width and height for any swizzled SSH.
// these come from gimex. We should really find a way of sharing
// the code. Frank and I will have to discuss it.

// ------------------------------------------------------------
static int IsValidSSHSwizzleWidthHeight(int w,int h,int bpp)
{
    int realwidth  = w;
    int realheight = h;
    switch (bpp)
    {
        case 4:// size need to be div by 16x32 for 4 bit else can not swizzle
        {
            realwidth  = (w + 15)&~15; // 32x32, 64x64, 128x128 or multiple of 256 (rectangular okay)
            realheight = (h + 31)&~31;
            break;
        }
        case 8:// size needs to be div by 16x16 for 8 bit else can not swizzle
        {
            realwidth  = (w + 15)&~15;
            realheight = (h + 15)&~15;
            break;
        }
        case 15:
        case 16:// size needs to be div by 8x16 for 15 bit else can not swizzle
        {
            realwidth  = (w + 7)&~7;
            realheight = (h + 15)&~15;
            break;
        }
    }
    if ((realwidth == w && realheight == h)
     || (ISPOWER2(w) && ISPOWER2(h)))
        return 1;
    return 0;
}


// ------------------------------------------------------------
// calculate the correct width and height for any swizzled SSH.
// these come from gimex. We should really find a way of sharing
// the code. Frank and I will have to discuss it.
static int IsValidXSHSwizzleWidthHeight(int w,int h,int bpp)
{
    if(ISPOWER2(w) && ISPOWER2(h))
        return 1;
    else
        return 0;
}

// ------------------------------------------------------------
// calculate the correct width and height for any swizzled SSH.
// these come from gimex. We should really find a way of sharing
// the code. Frank and I will have to discuss it.
static int IsValidFSHSwizzleWidthHeight(int w,int h,int bpp)
{
    if(ISPOWER2(w) && ISPOWER2(h))
        return 1;
    else
        return 0;
}

// ------------------------------------------------------------
static bool IsShape(unsigned int signature)
{
    return
        (SignatureCompare(signature,GMAKEID('S','H','P','X')) ||
         SignatureCompare(signature,GMAKEID('S','h','p','F')) ||
         SignatureCompare(signature,GMAKEID('S','h','p','X')) ||
         SignatureCompare(signature,GMAKEID('S','h','p','G')) ||
         SignatureCompare(signature,GMAKEID('S','h','p','S')) );
}

// ------------------------------------------------------------
static bool IsShape(unsigned int signature, unsigned char EndChar)
{
    return
        (SignatureCompare(signature,GMAKEID('S','H','P',EndChar)) ||
         SignatureCompare(signature,GMAKEID('S','h','p',EndChar)) );
}

// ------------------------------------------------------------
static bool IsGstFourCC(unsigned int packed)
{
  if((packed&0xFFFFFF00) == 0x47535400) // if the fourCC == 'GST '
    return true;
  return false;
}

// ------------------------------------------------------------
static bool IsMovie(GABOUT * about)
{
  return(about->multiframe && about->movie && !about->multisize);
}

// ------------------------------------------------------------
bool GX_ismipmap(char * shape_ID,int len)
{
    // if the name is the old one.
    if (shape_ID[0] == 'm'
     && shape_ID[1] == 'm'
     && shape_ID[2] >= '0' && shape_ID[2] <= '9'
     && ((shape_ID[3] >= '1' && shape_ID[3] <= '9') || shape_ID[2] >= '1'))
        return true;

    // if the name is the new one.
    if (len == 0 || len > 4)
    {
        int    i = 0;
        char * s = shape_ID;

        if (len == 0)
            len = strlen(shape_ID) - 3;

        for (i = 0; i < len; i++)
        {
            if ((s[0] == '_') &&
                (s[1] == 'm') &&
                (s[2] == 'm') &&
                (s[3] >= '0') && (s[3] <= '9'))
                    return true;
            s++;
        }

    }

    // otherwise not a mipmap.
    return false;
}

//------------------------------------------------------------------------
// validate shape width and height for swizzleing.
static void GXVerifyFrame(GINFO * ginfo,GABOUT *wabout)
{
    int validMatch;

    // lets check shapes for valid sizes when we want to do swizzleing.
    {
        static bool movieMipmapDoneOnce = false;
        if(IsMovie(wabout) && GX_ismipmap(ginfo->framename,strlen(ginfo->framename)) && !movieMipmapDoneOnce)
        {
            movieMipmapDoneOnce = true;
            gprintf("WARNING: [%s] exporting mipmaps to a movie can result in corrupt data\n",ginfo->framename);
        }
        else if (ginfo->subtype&SUBSWIZZLE && IsShape(wabout->signature,'X'))
        {
            static int xdoneonce = 0;
            validMatch = IsValidXSHSwizzleWidthHeight(ginfo->width,ginfo->height,ginfo->originalbpp);
            if(OPTverbose && !validMatch && !xdoneonce)
            {
                gprintf("WARNING: [%s] swizzle flag turned on but image width and height\n         are not power of 2. Image data may not be valid.\n",ginfo->framename);
                xdoneonce = 1;
            }
        }
        // GST compression
        else if (IsGstFourCC(ginfo->packed) && IsShape(wabout->signature))
        {
            static int gstdoneonce  = 0;
            static int gstmiponce   = 0;
            int width_align         = 32;
            int height_align        = 16;
            int newwidth            = (ginfo->width +(width_align-1))  & ~(width_align-1);
            int newheight           = (ginfo->height+(height_align-1)) & ~(height_align-1);
            validMatch              = (newwidth == ginfo->width && newheight == ginfo->height);
            if(OPTverbose)
            {
              if(!gstdoneonce)
              {
                // no swizzle for ssh
                if(IsShape(wabout->signature,'S') && !(ginfo->subtype&SUBSWIZZLE))
                {
                  gprintf("WARNING:  [%s] GST compressing SSH without the SWIZZLE flag turned on.\n",ginfo->framename);
                  gstdoneonce = 1;
                }
                // Dimension check
                if(!validMatch)
                {
                  gprintf("WARNING:  [%s] Invalid GST texture size (%dx%d), Image dimensions must be a multiple of %dx%d\n",ginfo->framename,ginfo->width,ginfo->height,width_align,height_align);
                  gstdoneonce = 1;
                }
              }
              // GST with mipmap check
              // Mipmaps have to check the second frame
              // but I don't want my other warnings to
              // mask this warning since it's fairly important
              // thus it gets it's own status var.
              if(!gstmiponce)
              {
                if(GX_ismipmap(ginfo->framename,strlen(ginfo->framename)))
                {
                  gprintf("WARNING:  [%s] Attempt to GST compress mipmaps, GST should only be used with -mipmont.\n",ginfo->framename);
                  gstmiponce = 1;
                }
              }
            }
        }
        else if (ginfo->subtype&SUBSWIZZLE && IsShape(wabout->signature,'S'))
        {
            static int sdoneonce  = 0;
            static int sdoneonce2 = 0;

            validMatch = IsValidSSHSwizzleWidthHeight(ginfo->width,ginfo->height,ginfo->originalbpp);
            if(OPTverbose && !validMatch && !sdoneonce)
            {
                gprintf("WARNING:  [%s] swizzle flag turned on but the image width and height\n         are not the correct dimensions. Image data may not be valid.\n",ginfo->framename);
                sdoneonce = 1;
            }
            if(OPTverbose && ginfo->originalbpp == 24 && !sdoneonce2)
            {
                gprintf("WARNING: [%s] swizzle flag turned on but image is 24bit\n         Image will not be swizzled.",ginfo->framename);
                sdoneonce2 = 1;
            }
        }
        else if (ginfo->subtype&SUBSWIZZLE && IsShape(wabout->signature,'G'))
        {
            static int gdoneonce = 0;
            validMatch = IsValidGSHSwizzleWidthHeight(ginfo->width,ginfo->height,ginfo->originalbpp);
            if(OPTverbose && !validMatch && !gdoneonce)
            {
                gprintf("WARNING: [%s] swizzle flag turned on but image width and height\n         are not proper dimensions. Image data may not be valid.\n",ginfo->framename);
                gdoneonce = 1;
            }
        }
        else if (ginfo->subtype&SUBSWIZZLE && IsShape(wabout->signature,'F'))
        {
            static int fdoneonce = 0;
            validMatch = IsValidFSHSwizzleWidthHeight(ginfo->width,ginfo->height,ginfo->originalbpp);
            if(OPTverbose && !validMatch && !fdoneonce)
            {
                gprintf("WARNING: [%s] swizzle flag turned on but image width and height\n         are not power of 2. Image data may not be valid.\n",ginfo->framename);
                fdoneonce = 1;
            }
        }
        else if (ginfo->packed == 4         &&
                 ginfo->bpp    != 8         &&
                 IsShape(wabout->signature) &&
                 (ginfo->width  != ((ginfo->width+15)&~15) ||
                  ginfo->height != ((ginfo->height+15)&~15)
                  ))
        {
            static int iframedoneonce = 0;
            if(OPTverbose && !iframedoneonce)
            {
                gprintf("WARNING: [%s] attempt to export to an IFrame shape that is not a multiple of 16x16\n         texture will not be compressed\n",ginfo->framename);
                iframedoneonce = 1;
            }
        }
    }
}



//------------------------------------------------------------------------
// if we are working on a windows machine then set console title to
// string.
static void gsettitle(char *title)
{
#if defined(_MSC_VER) || defined(__WATCOMC__)
    if (OPTverbose)
        SetConsoleTitle(title);
#endif
}



/* if user specifies a type, but no extension, this should */
/* force a change of extension.  otherwise, use whats there. */

//------------------------------------------------------------------------
// verify that any given extension is valid with GIMEX. returns non
// zero on success.
static int isvalidextension(GABOUT *about,char *ext,int type)
{
    int exttype;
    int ok=0;

    if (ext)
    {
        exttype = GIMEX_find(ext,0);
        if (!exttype || exttype==type)
            ok = 1;
    }

    return(ok);
}

//------------------------------------------------------------------------
// find and return a pointer to the begining of the extension within
// a string. returns 0 if no extension found.
static char *GXextension(char *filename)
{
    char *ext=filename+(strlen(filename))-1; /* find extension */
    while (ext>filename)
    {
        if (*ext=='.')
            break;
        if (*ext==(char)':' || *ext==(char)'/' || *ext==(char)'\\')
            break;
        --ext;
    }
    if (*ext!='.')
        ext = 0;
    return(ext);
}

static void TTF_pointsize(const char *OPTpointsize, const char *OPTantialias, const char *OPTstyle, int OPTitalic, int OPTbold, const char *OPTcharset, const char *OPTtypeface, const char *OPTaspect, const char *OPTtighten, const char *OPTtracking, const char *OPTsmallcaps, const char *OPTascii, const char *OPTbaseline, int frame)
{
    TTFINFO *ttf = (TTFINFO *) TTFinfo;
    if (TTFinfo)
    {
        EXPR_set(expr,"frame",frame);   // available for font pointsize
        EXPR_set(expr,"framenum",frame);

        // point size

        float pointsize = ttf->pointsize;
        if (OPTpointsize[0])
            pointsize = (float)GX_constrain(OPTpointsize,(double)ttf->pointsize);
        ttf->pointsize = pointsize;

        // bit depth control

        int antialias = 1;
        if (OPTantialias[0])
            antialias  = (int)GX_constrain(OPTantialias,(double)((ttf->flags&1)?0:1));
        if (antialias==0)       ttf->flags = 1;
        else if (antialias==1)  ttf->flags = 0;
        else                    ttf->flags = 2;

        // aspect ratio. ie 0.5 is for 640x240 screens
        float aspect = 1.0f;
        if (OPTaspect[0])
            aspect = (float)GX_constrain(OPTaspect,(double)ttf->aspect);
        ttf->aspect = aspect;

        // adjust spacing.  <1.0 is tight.  >1 is widen
        float tighten = 1.0f;
        if (OPTtighten[0])
            tighten = (float)GX_constrain(OPTtighten,(double)ttf->tighten);
        ttf->tighten = tighten;

        // adjust spacing.  <-0 is tight.  >0 is widen
        float tracking = 0.0f;
        if (OPTtracking[0])
            tracking = (float)GX_constrain(OPTtracking,(double)ttf->tracking);
        ttf->tracking = tracking;

        // small caps feature.  <0 is none. 1.0 is all caps. 0.8 is small caps default>
        float smallcaps = 0.8f;
        if (OPTsmallcaps[0])
            smallcaps = (float)GX_constrain(OPTsmallcaps,(double)ttf->smallcaps);
        ttf->smallcaps = smallcaps;


        // bold, italic, strikethrough
        int style = 0;
        if (OPTstyle[0])
            style = (int)GX_constrain(OPTstyle,(double)ttf->style);
        if (OPTitalic)
            style |= 2;
        if (OPTbold)
            style |= 1;
        ttf->style = style;

        // charset is languages. 129=asian.
        int charset = 0;
        if (OPTcharset[0])
            charset = (int)GX_constrain(OPTcharset,(double)ttf->charset);
        ttf->charset = charset;

        // ascii limit to 32..255
        int ascii = 1;
        if (OPTascii[0])
            ascii = (int)GX_constrain(OPTascii,(double)ttf->ascii);
        ttf->ascii = ascii;

        // baseline flag 0 or 1
        int baseline = 1;
        if (OPTbaseline[0])
            baseline = (int)GX_constrain(OPTbaseline,(double)ttf->baseline);
        ttf->baseline = baseline;

        // name of font
        if (OPTtypeface[0])
            USTR_strcpy(ttf->fontname,OPTtypeface);
    }
}


//---------------------------------------------------------------------
// handle the messy business of setting the tools title bar.
// watch out for the 3 different tools.
static void GX_settooltitle(GABOUT * about,char *fnamebuf,int pass)
{
    if (about)
    {
#if 0

        // set window title

        char titlebuf[SPITMAXLINE];
        if (OPTverbose>=3)
            sprintf(titlebuf,"%s - GX Reading Pass %d",fnamebuf,pass);
        else
            sprintf(titlebuf,"%s - GX Reading",fnamebuf);
        gsettitle(titlebuf);
#endif

        // print out what file I am currently reading.

        if (OPTverbose && (pass==1 || OPTverbose>=4 || OPTglobalpalette || OPTflatten || OPTcram))
            gprintf("%s '%s'\n",about->longtypestr,fnamebuf);

    }
}


//------------------------------------------------------------------------
// check to see if the gimex module supports exporing and verify
// bitdepth. Also checks GIMEX version number.
// Returns FALSE if we have an invalid input and gx should
// goto ErrorExit in this case.
// Also returns a flag (int * w) that specifies if the user
// have been warned or not during function call.
// w can be set to true if user should not be warned of anything.
static bool GX_verifyoutputfile(GABOUT * wabout,int SPITargnum,int * w,int morethanonefile)
{
    int warned = *w;                                                              // don't warn the user twice about anything.
    if (wabout)
    {
        if (OPTwrite)                                                              // only flag things as errors if the user
        {                                                                          // requesting to write out. This is on by default.
            if (!wabout->canexport)                                                // does the module selected have the ability to export?
            {
                if (OPTverbose)
                    gprintf(TOOL_NAME " - CAN NOT EXPORT '%s' FORMAT.\n",wabout->wordtypestr);
                return false;                                                                // not gx should goto it's ErrorExit code on failure.
            }
                                                                                             // can I export a high enough bit depth and the user only
            if (!wabout->export32 && (OPTdepth<=32) && (OPTdepth>8) && ((SPITargnum-2)<=0))  // specified this one file?
            {
                if (OPTverbose)
                    gprintf(TOOL_NAME " - '%s' DOES NOT SUPPORT %d BPP.\n",wabout->wordtypestr, OPTdepth);
                return false;                                                                 // note gx should goto it's ErrorExit code on failure.
            }
                                                                                              // the user wants to export to 32 bit, format does not support it
            if (!wabout->export32 && (OPTdepth<=32) && (OPTdepth>8) && OPTverbose && !warned) // and they exported some other number of files then warn them.
            {
                gprintf(TOOL_NAME " - WARNING '%s' DOES NOT SUPPORT %d BPP.\n",wabout->wordtypestr, OPTdepth);
                warned = 1;
            }
                                                                                              // do the same for 8bpp.
            if (!wabout->export8 && (OPTdepth>=1) && (OPTdepth<=8) && OPTverbose && !warned)
            {
                gprintf(TOOL_NAME " - WARNING '%s' DOES NOT SUPPORT %d BPP.\n",wabout->wordtypestr, OPTdepth);
                warned = 1;
            }
        }
        if (wabout->globalpalette && wabout->export8 && !OPTglobalpalette)         // only if I'm going to export to 8 bit format needs a global palette
        {                                                                          // and it's not already on.
            int willbe8=0;                                                         // not sure if I'm going to 8 bit yet.

            // going to export to 8 bit globalpalette AND we have more than one file OR
            // one file in which the input format doesn't support globalpalettes.
            // the latter is done later once we have loaded the input file.
            if ((!wabout->export32
                || ((OPTdepth>=1) && (OPTdepth<=8))
                || OPTcolours)
                && morethanonefile > 1)
            {
                willbe8 = 1;                                                       // definately will be 8
                OPTglobalpalette = 1;
            }

            if (OPTverbose>=2)                                                     // let the user know what we are doing.
                if (OPTverbose>=(4-willbe8))                                       // spit out the message with the assurance we have
                    gprintf(TOOL_NAME " - NOTE '%s' FORMAT ENABLING GLOBAL PALETTE.\n",wabout->wordtypestr);  // that texture will indeed end up as 8bpp.
        }

        {
            static int warnexportversion=0;                                        // only do this once an iteration.
            if (wabout->version<234 && !warnexportversion && OPTverbose)           // what is the minimum gimex version that I am willing to tolerate nicely.
            {
                gprintf(TOOL_NAME " - WARNING '%s' GIMEX IS VERSION %.2f.  GX WANTS 2.30\n",wabout->wordtypestr,(float)wabout->version/100.0f);
                warnexportversion = 1;
            }
        }
    } // end if wabout
    *w = warned;                                                                   // don't warn the user twice.
    return 1;
}

//------------------------------------------------------------------------
// is this a simple decimal number, not preceeded by zeros (octal or framename)
// used to determine if -f is a named frame or numerical

static bool GX_isdecimal(const char *data)
{
    bool isdigits=false;
    const char *p = data;

    while (*p>='0' && *p<='9')  // search for first non-digit (or end of string)
        ++p;

    if (!*p && (*data!='0' || p!=data)) // all digits and not zero padded?
        isdigits = true;

    return isdigits;
}
}
