// (c) 2002-2007 Electronic Arts Inc.

//#define DEBUGGING 1

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    FFS - True Type Font Gimex                    */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 020410 FB initial version based on freetype and HSI      */
/*    1.01 020608 FB info based on ffont filter                     */
/*    1.02 020610 FB frame->unicode->glyph index                    */
/*    1.03 020611 FB signature FNTT, kern, pointsize, cache         */
/*    1.04 020611 FB unicode specify characters (for fontwiz)       */
/*    1.05 020612 FB space ' ' transparent 1x1                      */
/*    1.06 020617 FB antialiasing control                           */
/*    1.07 020618 FB reimplemented using windows (legal concern)    */
/*    1.08 020624 FB info/read first working version with windows   */
/*    1.09 020626 FB space char, italic/bold                        */
/*    1.10 020628 FB gdi only, unicode comment                      */
/*    1.11 020702 FB unicode ranges                                 */
/*    1.12 020703 FB aspect ratio, private fonts, tighten           */
/*    1.13 020711 FB do not encode characters below 32              */
/*    1.14 020715 FB 4 bit flag                                     */
/*    1.15 020728 FB name match length                              */
/*    1.16 020814 FB smallcaps                                      */
/*    1.17 021201 FB tracking adjust spacing by translation value   */
/*    1.18 021204 FB antialias8                                     */
/*    1.19 021218 FB antialias0 fill in 4 bit values                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit import only                                                */
/* Non-Reentrant, Scalable                                          */
/* Files: ffsread.cpp ffswrite.cpp ffsgimex.h                       */
/* FNTT signature choosen to easily match subtypes with FNT modules */
/*                                                                  */
/* Legal page includes notes about FFS and Agfa Fonts               */
/* http://tools.eac.ea.com/gimex/release/introduction/legal.htm     */
/*                                                                  */
/* gref used to specify options before ginfo                        */
/* see ffsgimex.h FFSINFO for public fields                         */
/*                                                                  */
/* subtype, hotspots, center point follows ffn conventions          */
/*                                                                  */
/* enum FontStyle{                                                  */
/*   FontStyleRegular    = 0,                                       */
/*   FontStyleBold       = 1,                                       */
/*   FontStyleItalic     = 2,                                       */
/*   FontStyleBoldItalic = 3,                                       */
/*   FontStyleUnderline  = 4,                                       */
/*   FontStyleStrikeout  = 8                                        */
/* };                                                               */
/*                                                                  */
/*  List of installed fonts                                         */
/*  HKEY_LOCAL_MACHINE,                                             */
/*    SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* GINFO Field          Font/Character Field                        */
/* -----------          --------------------                        */
/* width                width                                       */
/* height               height                                      */
/* defaultx             u                                           */
/* defaulty             v                                           */
/* centerx              -xoffset                                    */
/* centery              -yoffset                                    */
/* hotspot[0].x         advancex                                    */
/* hotspot[0].y         -yoffset (centery)                          */
/* hotspot[1].x         -xoffset (centerx)                          */
/* hotspot[1].y         advancey (\n height)                        */
/* hotspot[2].x         ascent                                      */
/* hotspot[2].y         descent                                     */
/* hotspot[3..].x       kern previous character code (unicode)      */
/* hotspot[3..].y       kern amount                                 */
/*                                                                  */
/* Subtype Values (not supported by REAL)                           */
/* --------------                                                   */
/* 0x00000000 - normal                                              */
/* 0x00000001 - antialiased                                         */
/* 0x00000002 - dropshadow                                          */
/* 0x00000004 - outline                                             */
/* 0x00000008 - spaced extra hardware filtering                     */
/* 0x00020000 - shift-jis encoded                                   */
/* 0x00040000 - char16 format, 2byte advance, kern offset           */
/* Refer to fontfile.doc for more information on this.              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* FFS files true type                                              */
/* They are an outline/scalable font format                         */
/*                                                                  */
/* Pangrams                                                         */
/* http://www.fun-with-words.com/pang_example.html                  */
/*                                                                  */
/* GDI+                                                             */
/* http://msdn.microsoft.com/library/default.asp?url=/library/en-us/gdicpp/cpp_gdi+start_26ic.asp */
/*                                                                  */
/* Reading font names from ffs/otf                                  */
/* http://www.codeguru.com/gdi/fontnamefromfile.html                */
/*                                                                  */
/* Petzoid talking about LOGFONT, aspect ratio                      */
/* http://www.microsoft.com/mspress/books/sampchap/2344a.asp        */
/*                                                                  */
/* CharSets supported by windows                                    */
/* #define ANSI_CHARSET            0                                */
/* #define DEFAULT_CHARSET         1                                */
/* #define SYMBOL_CHARSET          2                                */
/* #define SHIFTJIS_CHARSET        128                              */
/* #define HANGEUL_CHARSET         129                              */
/* #define HANGUL_CHARSET          129     (common asian format)    */
/* #define GB2312_CHARSET          134                              */
/* #define CHINESEBIG5_CHARSET     136                              */
/* #define OEM_CHARSET             255                              */
/* #if(WINVER >= 0x0400)                                            */
/* #define JOHAB_CHARSET           130                              */
/* #define HEBREW_CHARSET          177                              */
/* #define ARABIC_CHARSET          178                              */
/* #define GREEK_CHARSET           161                              */
/* #define TURKISH_CHARSET         162                              */
/* #define VIETNAMESE_CHARSET      163                              */
/* #define THAI_CHARSET            222                              */
/* #define EASTEUROPE_CHARSET      238     (central european)       */
/* #define RUSSIAN_CHARSET         204                              */
/*                                                                  */
/* #define MAC_CHARSET             77                               */
/* #define BALTIC_CHARSET          186                              */
/*                                                                  */
/* File extension Description (From GDI.chm AddFontResource)        */
/* .fon Font resource file.                                         */
/* .fnt  Raw bitmap font file.                                      */
/* .ffs Raw TrueType file.                                          */
/* .ttc East Asian Windows: TrueType font collection.               */
/* .fot TrueType resource file.                                     */
/* .otf PostScript OpenType font.                                   */
/* .mmm multiple master Type1 font resource file. It must be used with .pfm and .pfb files. */
/* .pfb Type 1 font bits file. It is used with a .pfm file.         */
/* .pfm Type 1 font metrics file. It is used with a .pfb file.      */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

// nt/2k/xp
#define _WIN32_WINNT 0x0500
//#undef WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "gimex.h"
#include "ffsgimex.h"

// global to specify font, charset, pointsize before FFS_open
extern "C" FFSINFO FFSinfo={0,};


enum FontStyle{
  FontStyleRegular    = 0,
  FontStyleBold       = 1,
  FontStyleItalic     = 2,
  FontStyleBoldItalic = 3,
  FontStyleUnderline  = 4,
  FontStyleStrikeout  = 8
};

// REAL2INT
int FFS_REAL2INT(float x)
{
    return ((int)(((float)(x))+((((float)(x))>=0.0f)?0.5f:-0.5f)));
}

// fill in sorted kern table for a character
static int FFS_kernpairs(HDC hdc, int numframes, int *kernpairs, unsigned int nextunicode, float tighten)
{
    int numkern=0;

    int numkerns = GetKerningPairs(hdc,0,NULL);

    if (numkerns)
    {
        KERNINGPAIR *kerningpair = (KERNINGPAIR *) galloc(numkerns*sizeof(KERNINGPAIR));

        GetKerningPairs(hdc,numkerns,kerningpair);

        for (int i=0; i<numkerns; ++i)
        {
            unsigned int prevunicode=(unsigned int) kerningpair[i].wFirst;

        	int kernx = FFS_REAL2INT((float)kerningpair[i].iKernAmount*tighten);	// adjust posx to respect kerning
            if (kernx && numkern<GIMEX_HOTSPOTTBL_SIZE && (unsigned int)kerningpair[i].wSecond==nextunicode)
            {
                kernpairs[numkern*2+0] = prevunicode;
                kernpairs[numkern*2+1] = kernx;
                ++numkern;
            }
    	}


        gfree(kerningpair);
    }
    return numkern;
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL FFS_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = FFSSIGNATURE;
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 0;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 0;    /* supports importing 32 bit direct rgb */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 1;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->globalpalette   = 1;    /* one palette per file */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->movie           = 0;    /* movie file format */
        info->font            = 1;    /* format is a font */
        info->file64          = 0;    /* this format supports 64 bit file sizes */
        info->mactype[0]      = FFSSIGNATURE;   /* mac file system types used */
        strcpy(info->extensions[0], ".ffs");  // Raw TrueType file
        strcpy(info->extensions[1], ".ttc");  // East Asian Windows: TrueType font collection.
        strcpy(info->extensions[2], ".fon");  // Font resource file.
        strcpy(info->extensions[3], ".fnt");  // Raw bitmap font file.
        strcpy(info->extensions[4], ".otf");  // PostScript OpenType font.
        strcpy(info->extensions[5], ".fot");  // TrueType resource file.
        strcpy(info->extensions[6], ".mmm");  // multiple master Type1 font resource file. It must be used with .pfm and .pfb files
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.19");                /* version # */
        strcpy(info->shorttypestr,  "FFS");                 /* type */
        strcpy(info->wordtypestr,   "FFS");                 /* wordtype */
        strcpy(info->longtypestr,   "TrueType Font");       /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
// 00000: 00010000 00170100 00040070 44534947  ...........pDSIG
// 00000: 00010000 00190100 00040090 42415345  ............BASE
// 00000: 00010000 000e0080 00030060 4f532f32  ...........`OS/2
// 00000: 00010000 00120100 00040020 4c545348  ........... LTSH
// 00000: 00010000 00110100 00040010 47535542  ............GSUB
// 00000: 00010000 00120100 00040020 45424454  ........... EBDT

// batang.ttc
// 00000: 74746366 00010000 00000004 0000001c  ttcf............

// smalleg.fon
// 00000: 4d5af500 01000000 04000000 ffff0000  MZu.............
// serifeg.fon
// 00000: 4d5a0501 01000000 04000000 ffff0000  MZ..............

static inline bool ffsistext(char c)
{
    return c>=' ' && c<='z';
}

int GCALL FFS_is(GSTREAM *g)
{
    unsigned char ffsheader[16];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,ffsheader,16))
    {
        if (ggetm(ffsheader,4)==0x00010000
         && ffsistext(ffsheader[12])
         && ffsistext(ffsheader[13])
         && ffsistext(ffsheader[14])
         && ffsistext(ffsheader[15]))
            ok = 100;
        else if (ffsheader[0]=='M'
         && ffsheader[1]=='Z'
         && ffsheader[3]<=1  // 0 or 1
         && ffsheader[12]==0xff
         && ffsheader[13]==0xff)
            ok = 90; // .fon


        if (ggetm(ffsheader,4)==0x74746366) // 1 bit chinese font
            ok = 99;

    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

// from USTR_strcpy
static void FFS_strcpyutoea(char *dst, const unsigned short *src)
{
    while (*src)
    {
        // ie \x1a34
        if (src[0]>=256 || (src[0]==(unsigned short)'\\' && src[1]==(unsigned short)'x'))
        {
            sprintf(dst,"\\x%04x",src[0]);
            dst += 6;
        }
        else
            *dst++ = (char)(unsigned char)*src;
        ++src;
    }
    *dst++ = '\0';
}

// from USTR_strcpy
static void FFS_encode(unsigned short *dst, const unsigned short *src)
{
    while (*src)
    {
        // ie \x1a34
        if (src[0]>=256 || (src[0]==(unsigned short)'\\' && src[1]==(unsigned short)'x'))
        {
            swprintf(dst,L"\\x%04x",src[0]);
            dst += 6;
        }
        else
            *dst++ = *src;
        ++src;
    }
    *dst++ = 0;
}


static void FFS_strcpyu(unsigned short *d, const unsigned short *s)
{
    while (*s)
    {
        *d++ = *s++;
    }
    *d = 0;
}

static void FFS_strcpyatou(unsigned short *d, const char *s)
{
    while (*s)
    {
        *d++ = (unsigned short)(unsigned char)*s;
        ++s;
    }
    *d = 0;
}

static unsigned short * FFS_getname( unsigned short *New,const unsigned short *old)
{
    const unsigned short *nameptr;

    nameptr = old;

    while (*old)
    {
        if (*old==(unsigned short)'/' || *old==(unsigned short)'\\' || *old==(unsigned short)':')
            nameptr = old+1;
        ++old;
    }
    FFS_strcpyu(New, nameptr);                           /* copy to destination */
    return(New);
}


const static int FFS_strcmpu(const unsigned short *str, const unsigned short *substr)
{
    int diff=0;
    while (*str && *substr)
    {
        int c1=(int)*str;
        int c2=(int)*substr;
        diff = c1-c2;
        if (diff)
            break;
        ++str;
        ++substr;
    }
    return diff;
}

const static int FFS_strcmpcountu(const unsigned short *str, const unsigned short *substr)
{
    int diff=0;
    int matchcount=0;
    while (*str && *substr)
    {
        int c1=(int)*str;
        int c2=(int)*substr;
        diff = c1-c2;
        if (diff)
            break;
        ++matchcount;
        ++str;
        ++substr;
    }
    return matchcount;
}

const static int FFS_strcmpau(const char *str, const unsigned short *substr)
{
    int diff=0;
    while (*str && *substr)
    {
        int c1=(int)(unsigned char)*str;
        int c2=(int)*substr;
        if (c2<128)
            diff = c1-c2;
        if (diff)
            break;
        ++str;
        ++substr;
    }
    return diff;
}

const static int FFS_stricmpu(const unsigned short *str, const unsigned short *substr)
{
    int diff=0;
    while (*str && *substr)
    {
        int c1=(int)(unsigned char)*str;
        int c2=(int)*substr;
        if (c1>='A' && c1<='Z') c1 += 'a'-'A';
        if (c2>='A' && c2<='Z') c2 += 'a'-'A';
        diff = c1-c2;
        if (diff)
            break;
        ++str;
        ++substr;
    }
    return diff;
}

const static int FFS_stricmpcountu(const unsigned short *str, const unsigned short *substr)
{
    int diff=0;
    int matchcount=0;
    while (*str && *substr)
    {
        int c1=(int)(unsigned char)*str;
        int c2=(int)*substr;
        if (c1>='A' && c1<='Z') c1 += 'a'-'A';
        if (c2>='A' && c2<='Z') c2 += 'a'-'A';
        diff = c1-c2;
        if (diff)
            break;
        ++matchcount;
        ++str;
        ++substr;
    }
    return matchcount;
}


const static unsigned short *FFS_strstru(const unsigned short *str, const unsigned short *substr)
{
    while (*str)
    {
        if (!FFS_strcmpu(str,substr))
            return str;
        ++str;
    }
    return (unsigned short *) 0;
}

const static int FFS_strstrcountu(const unsigned short *str, const unsigned short *substr)
{
    int bestcount=0;
    while (*str)
    {
        int thiscount = FFS_strcmpcountu(str,substr);
        if (thiscount>bestcount)
            bestcount = thiscount;
        ++str;
    }
    return bestcount;
}

const static int FFS_stristrcountu(const unsigned short *str, const unsigned short *substr)
{
    int bestcount=0;
    while (*str)
    {
        int thiscount = FFS_stricmpcountu(str,substr);
        if (thiscount>bestcount)
            bestcount = thiscount;
        ++str;
    }
    return bestcount;
}



static unsigned short *FFS_strchru(unsigned short *str, unsigned short chr)
{
    while (*str)
    {
        if (*str==chr)
            return str;
        ++str;
    }
    return (unsigned short *) 0;
}

static int FFS_strlenu(const unsigned short *str)
{
    int len=0;
    while (*str)
    {
        ++len;
        ++str;
    }
    return len;
}

static unsigned short regfontname[256]; // font to find
static unsigned short enumfontname[256]; // font found
static int enumpass=0; // 0=exact, 1=best

// find font that is in registry font family name
// ie Courier New Bold Italic (TrueType) -> Courier New

static int CALLBACK FFS_enumerator(
  const ENUMLOGFONTEXW *lpelfe,    // logical-font data
  const NEWTEXTMETRICEXW *lpntme,  // physical-font data
  DWORD FontType,           // type of font
  LPARAM lParam             // application-defined data
)
{
    unsigned short regfontnamebuf[256];
    FFS_strcpyu(regfontnamebuf,regfontname);
    bool foundit=false;

    unsigned short *ttptr = (unsigned short *) FFS_strstru((const unsigned short *)regfontnamebuf,L" (TrueType)");
    if (ttptr) *ttptr = 0;

    int bestmatch;
    int thismatch;

    bestmatch = FFS_stristrcountu(regfontnamebuf,enumfontname);
    thismatch = FFS_stristrcountu(regfontnamebuf,lpelfe->elfFullName);

    int enumlen = FFS_strlenu(lpelfe->elfFullName);
    int reglen = FFS_strlenu(regfontnamebuf);

    if (enumpass==0) // strict exactness
        foundit = (thismatch==enumlen && thismatch==reglen);  // ie courier will not match courier new
    else
        foundit = (thismatch==enumlen); // ie batang will match Batang & BatangChe & Gungsuh & GungsuhChe

    if (foundit || (enumpass>0 && thismatch>=bestmatch))
        FFS_strcpyu(enumfontname,lpelfe->elfFullName);

#ifdef DEBUGGING
    WCHAR fontbuf[256];

    FFS_encode(fontbuf, lpelfe->elfFullName);

    wprintf(L"font %s",fontbuf);

    FFS_encode(fontbuf, regfontnamebuf);

    wprintf(L" == %s  matches %d, len %d,%d, pass %d, foundit %d\n",fontbuf,thismatch,enumlen,reglen,enumpass,foundit);
#endif

    if (foundit)
        return 0; // stop on perfect match, so we dont find bold/italic variations instead

    return 1; // continue
}

// search registry for filename and map it to font/style
// ie cour.ffs -> Courier New (TrueType)

static bool FFS_enumfontfiles(unsigned short *regfontname, int *style, const unsigned short *filename)
{
    int err;
    HKEY hKey;

    /* Try to open the subkey */

    err = RegOpenKey(HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts",
                    &hKey);
    if (err)
    {
        return false;
    }

#ifdef DEBUGGING
    wprintf(L"filename %s\n",filename);
#endif

    int i=0;
    do
    {
        WCHAR regval[128];
        int sizeofregval=256;
        int datatype;
        WCHAR dataval[128];
        int sizeofdataval=256;
        err = RegEnumValueW(hKey,
                            i,
                            regval,
                            (DWORD *)&sizeofregval,
                            0,
                            (DWORD *)&datatype,
                            (LPBYTE) dataval,
                            (DWORD *)&sizeofdataval);
		if (err)
		{
            return false;
		}
        else
        {
#ifdef DEBUGGING
            WCHAR regvalbuf[128];

            FFS_encode(regvalbuf, regval);

            wprintf(L"regval %s     regdata %s\n",regvalbuf,dataval);
#endif

            if (!FFS_stricmpu(filename,dataval))
            {
#ifdef DEBUGGING
                wprintf(L"found it: regval %s     regdata %s\n",regvalbuf,dataval);
#endif

                FFS_strcpyu(regfontname,regval);
                if (FFS_strstru(regfontname,L"Bold Italic ("))
                    *style = FontStyleBoldItalic;
                else if (FFS_strstru(regfontname,L"Bold ("))
                    *style = FontStyleBold;
                else if (FFS_strstru(regfontname,L"Italic ("))
                    *style = FontStyleItalic;
                else
                    *style = FontStyleRegular;


                return true;
            }
        }
        ++i;
    } while (!err);

#ifdef DEBUGGING
    wprintf(L"registry not found it %s\n",filename);
#endif

    return false;
}

// convert font pathname to font name
// ie cour.ffs -> Courier New
// also sets style (bold/italic)

static void FFS_pathtofontname(HDC hdc,unsigned short *fontnamebuf,int *style, const unsigned short *pathname)
{
    unsigned short filename[32];

    // strip folder, but keep extension
    FFS_getname(filename,pathname);

    // look fonts that map to this file
    bool foundit = FFS_enumfontfiles(regfontname, style, filename);
    if (!foundit)
    {
        FFS_strcpyu(regfontname,filename);
        unsigned short *ext= FFS_strchru(regfontname,L'.');
        if (ext)
            *ext = (unsigned short)0;
    }

    // now have string with extra junk in it.  enumerate fonts and find it

#if 0
// enumerate fonts (as opposed to families)
    {
        enumfontname[0] = 0;
        // search for regfontname
        EnumFontsW(hdc, NULL, (FONTENUMPROCW)&FFS_enumerator, NULL);
    }
#endif

    // search for regfontname
    {
        LOGFONTW Logfont; // enum all fonts
        memset(&Logfont,0,sizeof(LOGFONT));
        Logfont.lfCharSet = FFSinfo.charset;//DEFAULT_CHARSET;
        enumfontname[0] = 0;
        enumpass = 0;
        EnumFontFamiliesExW(hdc, &Logfont, (FONTENUMPROCW)&FFS_enumerator, NULL, 0);

        if (!enumfontname[0])
        {
            enumfontname[0] = 0;
            enumpass = 1;
            EnumFontFamiliesExW(hdc, &Logfont, (FONTENUMPROCW)&FFS_enumerator, NULL, 0);
        }

    }

    if (!enumfontname[0]) // not found?
    {
        unsigned short *ext= FFS_strchru(regfontname,' ');    // truncate at first space.
        if (ext)
            *ext = 0;

        FFS_strcpyu(enumfontname,regfontname); // use registry as font name
    }
    FFS_strcpyu(fontnamebuf,enumfontname);
}

static bool FFS_islowercase(unsigned int index)
{
    if (index >= 'a' && index <= 'z')
        return true;

    /* 224..254->192..222 Special case 247 */
    if ( (index >= 224) && (index <= 246) )   // lowercase with accents
    {
        return true;
    }
    if ( (index >= 248) && (index <= 254) )   // lowercase with accents
        return true;
    return false;
}

static int FFS_frameindex(HDC hdc, FFSINFO *ffs)
{
    int frames = 0;
    {
        int glyphsetsize = GetFontUnicodeRanges(hdc,NULL);
        GLYPHSET *glyphset = (GLYPHSET *) galloc(glyphsetsize);
        GetFontUnicodeRanges(hdc,glyphset);

        for (int range=0; range<(int)glyphset->cRanges; ++range)
        {
            for (int unicode=0; unicode<glyphset->ranges[range].cGlyphs; ++unicode)
            {
                int code=glyphset->ranges[range].wcLow+unicode;
                if (code>=32 && (!ffs->ascii || (code<256)))
                {
                    ffs->frametounicode[frames++] = code;
                }
            }
        }
        gfree(glyphset);
    }

    return frames;

}

int GCALL FFS_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
        gx->frames = 1;
        gx->signature=FFSSIGNATURE;
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;

        FFSINFO *ffs = (FFSINFO *) &FFSinfo;//galloc(sizeof(FFSINFO));
        if (ffs)
        {
            gx->gref = ffs;

            HDC hdc = CreateCompatibleDC(NULL);

//            AddFontResource(pathname); // use RemoveFontResource later

            unsigned short wpathname[256];
            FFS_strcpyatou(wpathname,pathname);
            FFS_strcpyu(ffs->pathname,wpathname);

            if (ffs->pathname[0])
            {
                int numadded = AddFontResourceW(ffs->pathname); // use RemoveFontResource later
#ifdef DEBUGGING
                wprintf(L"font %s added %d fonts\n",ffs->pathname,numadded);
#endif
            }

//                AddFontResourceExW(ffs->pathname,FR_PRIVATE,NULL); // use RemoveFontResource later

            if (!ffs->fontname[0])  // fontname not set via FFSINFO
            {
                ffs->charset = DEFAULT_CHARSET;

                unsigned short fontnamebuf[256];

                FFS_pathtofontname(hdc,fontnamebuf,&ffs->style,wpathname);
                FFS_strcpyu(ffs->fontname,fontnamebuf);

#ifdef DEBUG
                unsigned short fontnamebufencoded[256];
                unsigned short wpathnameencoded[256];
                FFS_encode(fontnamebufencoded,fontnamebuf);
                FFS_encode(wpathnameencoded,wpathname);

                wprintf(L"fontname %s <- %s\n",fontnamebufencoded,wpathnameencoded);
#endif
            }

           	float pointsize= 24.0f;
            if (ffs->pointsize)
                pointsize = (float) (ffs->pointsize);
           	float aspect= 1.0f;
            if (ffs->aspect)
                aspect = (float) (ffs->aspect);
           	float tighten= 1.0f;
            if (ffs->tighten)
                tighten = (float) (ffs->tighten);
           	float tracking= 0.0f;
            if (ffs->tracking)
                tracking = (float) (ffs->tracking);
			float smallcaps = 0.0f;
            if (ffs->smallcaps)
                smallcaps = (float) (ffs->smallcaps);
            bool antialias=(ffs->flags&1)?false:true;

        	// Select font

            LOGFONTW lf;

            memset(&lf,0,sizeof(lf));

            lf.lfHeight           = (LONG) FFS_REAL2INT((float)(-pointsize*96.0f/72.0*aspect));
            lf.lfWidth            = (LONG) 0;
            lf.lfEscapement       = (LONG) 0;
            lf.lfOrientation      = (LONG) 0;
            lf.lfWeight           = (LONG) (ffs->style&FontStyleBold)?700:400;; // 700=bold
            lf.lfItalic           = (BYTE) (ffs->style&FontStyleItalic)?1:0;
            lf.lfUnderline        = (BYTE) (ffs->style&FontStyleUnderline)?1:0;
            lf.lfStrikeOut        = (BYTE) (ffs->style&FontStyleStrikeout)?1:0;
            lf.lfCharSet          = (BYTE) ffs->charset; //DEFAULT_CHARSET;//HANGUL_CHARSET;
            lf.lfOutPrecision     = (BYTE) 0; //3=OUT_STROKE_PRECIS
            lf.lfClipPrecision    = (BYTE) 0;
            lf.lfQuality          = (BYTE) (antialias)?ANTIALIASED_QUALITY:NONANTIALIASED_QUALITY;
            lf.lfPitchAndFamily   = (BYTE) DEFAULT_PITCH|FF_SCRIPT;
            FFS_strcpyu(lf.lfFaceName,ffs->fontname);

            HFONT hfont = CreateFontIndirectW(&lf);
            SelectObject(hdc, hfont);

            // get font metrics

            TEXTMETRIC tm;
            GetTextMetrics(hdc,&tm);

            // adjust for aspect ratio
            lf.lfWidth            = (LONG) FFS_REAL2INT((float)tm.tmAveCharWidth/aspect);

            // reselect font with aspect considered
            DeleteObject(hfont);
            hfont = CreateFontIndirectW(&lf);
            SelectObject(hdc, hfont);

            // hash frame to unicode && count frames

            ok = gx->frames = FFS_frameindex(hdc,ffs);

            DeleteObject(hfont);
			DeleteDC(hdc);
        }
        if (gx->gref && !ok)
        {
            gx->gref = 0;
        }
    }


    return(ok);
}

// note unicode interface will return NULL for undefined codes
//

GINFO * GCALL FFS_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    FFSINFO *ffs = (FFSINFO *) gx->gref;
    int i;

	// Select font

   	float pointsize= 24.0f;
    if (ffs->pointsize)
        pointsize = (float) (ffs->pointsize);
    float aspect= 1.0f;
    if (ffs->aspect)
        aspect = (float) (ffs->aspect);
   	float tighten= 1.0f;
    if (ffs->tighten)
        tighten = (float) (ffs->tighten);
	float tracking= 0.0f;
	if (ffs->tracking)
		tracking = (float) (ffs->tracking);
	float smallcaps = 0.0f;
    if (ffs->smallcaps)
        smallcaps = (float) (ffs->smallcaps);
    bool antialias=(ffs->flags&1)?false:true;
    bool antialias8=(ffs->flags&2)?true:false;

    unsigned int index = (unsigned int) ffs->frametounicode[framenum]; // lookup unicode for our framenum
    if (ffs->unicode)
        index = ffs->unicode;
    int smallcapsindex = index;
    float smallcapsaspect=1.0f;
    if (smallcaps && FFS_islowercase(index))
    {
        smallcapsindex = index - 'a' + 'A';
        smallcapsaspect = smallcaps;
    }

    HDC hdc = CreateCompatibleDC(NULL);
    LOGFONTW lf;

    memset(&lf,0,sizeof(lf));

    lf.lfHeight           = (LONG) FFS_REAL2INT((float)(-pointsize*96.0f/72.0*aspect));
    lf.lfWidth            = (LONG) 0;
    lf.lfEscapement       = (LONG) 0;
    lf.lfOrientation      = (LONG) 0;
    lf.lfWeight           = (LONG) (ffs->style&FontStyleBold)?700:400;; // 700=bold
    lf.lfItalic           = (BYTE) (ffs->style&FontStyleItalic)?1:0;
    lf.lfUnderline        = (BYTE) (ffs->style&FontStyleUnderline)?1:0;
    lf.lfStrikeOut        = (BYTE) (ffs->style&FontStyleStrikeout)?1:0;
    lf.lfCharSet          = (BYTE) ffs->charset; //DEFAULT_CHARSET;//HANGUL_CHARSET;
    lf.lfOutPrecision     = (BYTE) 0;
    lf.lfClipPrecision    = (BYTE) 0;
    lf.lfQuality          = (BYTE) (antialias)?ANTIALIASED_QUALITY:NONANTIALIASED_QUALITY;
    lf.lfPitchAndFamily   = (BYTE) DEFAULT_PITCH|FF_SCRIPT;
    FFS_strcpyu(lf.lfFaceName,ffs->fontname);

    HFONT hfont = CreateFontIndirectW(&lf);
    SelectObject(hdc, hfont);

    // get font metrics

    TEXTMETRIC tm;
    GetTextMetrics(hdc,&tm);

    // adjust for smallcaps
    int advancey = (int)tm.tmHeight;
    int smallcapsascentadjust=0;
    if (smallcaps && FFS_islowercase(index))
    {
        int largecapsascent = tm.tmAscent;
        lf.lfHeight           = (LONG) FFS_REAL2INT((float)(-pointsize*96.0f/72.0*aspect*smallcapsaspect));

        // reselect font with smallcapsaspect considered
        DeleteObject(hfont);
        hfont = CreateFontIndirectW(&lf);
        SelectObject(hdc, hfont);

        // update font metrics

        GetTextMetrics(hdc,&tm);

        int smallcapsascent = tm.tmAscent;
        smallcapsascentadjust = largecapsascent - smallcapsascent;
        if (smallcaps>1.0f)
            advancey = (int)tm.tmHeight;
    }

    // adjust for aspect ratio
	if (aspect!=1.0f)
	{

		lf.lfWidth            = (LONG) FFS_REAL2INT((float)tm.tmAveCharWidth/aspect);

		// reselect font with aspect considered
		DeleteObject(hfont);
		hfont = CreateFontIndirectW(&lf);
		SelectObject(hdc, hfont);
	}

    // update font metrics

    GetTextMetrics(hdc,&tm);

    // get glyph metrics

    GLYPHMETRICS gm; // glyph metrics
	memset(&gm,0,sizeof(gm));
	FIXED fx0 = { 0, 0 }, fx1 = { 0, 1 };
	MAT2 matrix;
	matrix.eM11 = matrix.eM22 = fx1;
	matrix.eM12 = matrix.eM21 = fx0;

    int gco_bitmap=antialias?(antialias8?GGO_GRAY8_BITMAP:GGO_GRAY4_BITMAP):GGO_BITMAP;

    int bmpdatasize = 0;
    if (ffs->ascii)
        bmpdatasize = GetGlyphOutlineA(hdc,(UINT)smallcapsindex,gco_bitmap,&gm,0,NULL,&matrix);
    else
        bmpdatasize = GetGlyphOutlineW(hdc,(UINT)smallcapsindex,gco_bitmap,&gm,0,NULL,&matrix);

	if (bmpdatasize<=0)
	{
		antialias = 0;
		gco_bitmap = GGO_BITMAP;
	    if (ffs->ascii)
	        bmpdatasize = GetGlyphOutlineA(hdc,(UINT)smallcapsindex,gco_bitmap,&gm,0,NULL,&matrix);
	    else
	        bmpdatasize = GetGlyphOutlineW(hdc,(UINT)smallcapsindex,gco_bitmap,&gm,0,NULL,&matrix);
	}
	if (bmpdatasize < 0)
		bmpdatasize = 0;

	// note space (32) will return bmpdatasize 0 but blackboxx/y of 1,1
	BYTE * pBmpBuffer = NULL;
	if (bmpdatasize>0)
    	pBmpBuffer = (BYTE *)galloc(bmpdatasize);	// we know the size now, allocate an array

    if (ffs->ascii)
        GetGlyphOutlineA(hdc,(UINT)smallcapsindex,gco_bitmap,&gm,bmpdatasize,pBmpBuffer,&matrix);
    else
        GetGlyphOutlineW(hdc,(UINT)smallcapsindex,gco_bitmap,&gm,bmpdatasize,pBmpBuffer,&matrix);

    ginfo = (GINFO *) galloc(sizeof(GINFO));

    if (ginfo)
    {
        memset(ginfo,0,sizeof(GINFO));

        /* template for ginfo */
    	int charw = gmax(1,(int)gm.gmBlackBoxX);
    	int charh = gmax(1,(int)gm.gmBlackBoxY);
        int advancex = FFS_REAL2INT((float)gm.gmCellIncX * tighten + tracking);
		int subtype= antialias?9:8; // space + antialias
        int ascent =  (int)tm.tmAscent;
        int descent = (int)tm.tmDescent;
    	int offsetx = gm.gmptGlyphOrigin.x;    // draw origin
    	int offsety = ascent-gm.gmptGlyphOrigin.y+smallcapsascentadjust;

        ginfo->signature        = FFSSIGNATURE;   /* Gimex type */
        ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = framenum;    /* Current frame */
        ginfo->width            = charw;   /* Width of bitmap in pixels */
        ginfo->height           = charh;   /* Height of bitmap in pixels */
        ginfo->bpp              = 8;
        ginfo->originalbpp      = antialias8?8:4;
        ginfo->numcolours       = antialias8?256:16;;  /* Number of colours in original indexed palette */
        ginfo->subtype          = subtype; /* Internal format sub-type 0-default */
        ginfo->packed           = 0;    /* Type of packing on original image. 0 none, 1 run, n other */
        ginfo->quality          = 100;  /* Quality of lossy packing 0..100 */
        ginfo->alphabits        = antialias?8:1;    /* Number of bits in alpha channel */
        ginfo->redbits          = 0;    /* Number of bits in red channel */
        ginfo->greenbits        = 0;    /* Number of bits in grreen channel */
        ginfo->bluebits         = 0;    /* Number of bits in blue channel */
        ginfo->centerx          = -offsetx;    /* Center Point relative to upper left corner */
        ginfo->centery          = -offsety;
        ginfo->defaultx         = 0;    /* Default Coordinate Point */
        ginfo->defaulty         = 0;

        ginfo->numhotspots = 3 + FFS_kernpairs(hdc,gx->frames,&ginfo->hotspottbl[3][0],smallcapsindex, tighten);

        ginfo->hotspottbl[0][0] = advancex;
        ginfo->hotspottbl[0][1] = ginfo->centery;

        ginfo->hotspottbl[1][0] = ginfo->centerx;
        ginfo->hotspottbl[1][1] = advancey; // advance to next row

        ginfo->hotspottbl[2][0] = ascent;
        ginfo->hotspottbl[2][1] = descent;

        /* framename "0041 'A' 65" */
        {
            if (index>=32 && index<=127)
                sprintf(ginfo->framename, "%04x '%c' %d", index, index,index);
            else
                sprintf(ginfo->framename, "%04x %d", index, index);
        }

        if (antialias8)
        {
            for (i=0; i<256; ++i)
            {
                ginfo->colourtbl[i].a = i;
                ginfo->colourtbl[i].r = 255;
                ginfo->colourtbl[i].g = 255;
                ginfo->colourtbl[i].b = 255;
            }
        }
        else
        {
            for (i=0; i<16; ++i)
            {
                ginfo->colourtbl[i].a = (i*255+8)/15;
                ginfo->colourtbl[i].r = 255;
                ginfo->colourtbl[i].g = 255;
                ginfo->colourtbl[i].b = 255;
            }
        }

        // remember font family name as a comment

        FFS_strcpyutoea(ginfo->comment,ffs->fontname);
    }

	if (pBmpBuffer) gfree(pBmpBuffer);
    DeleteObject(hfont);
	DeleteDC(hdc);

    return(ginfo);
}

int GCALL FFS_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok=1;
    FFSINFO *ffs = (FFSINFO *) gx->gref;
    int framenum=ginfo->framenum;

	// Select font

   	float pointsize= 24.0f;
    if (ffs->pointsize)
        pointsize = (float) (ffs->pointsize);
    float aspect= 1.0f;
    if (ffs->aspect)
        aspect = (float) (ffs->aspect);
    float smallcaps = 0.0f;
    if (ffs->smallcaps)
        smallcaps = (float) (ffs->smallcaps);
    bool antialias=(ffs->flags&1)?false:true;
    bool antialias8=(ffs->flags&2)?true:false;

    unsigned int index = (unsigned int) ffs->frametounicode[framenum]; // lookup unicode for our framenum
    if (ffs->unicode)
        index = ffs->unicode;
    int smallcapsindex = index;
    float smallcapsaspect=1.0f;
    if (smallcaps && FFS_islowercase(index))
    {
        smallcapsindex = index - 'a' + 'A';
        smallcapsaspect = smallcaps;
    }


    HDC hdc = CreateCompatibleDC(NULL);
    LOGFONTW lf;

    memset(&lf,0,sizeof(lf));

    lf.lfHeight           = (LONG) FFS_REAL2INT((float)(-pointsize*96.0f/72.0*aspect*smallcapsaspect));
    lf.lfWidth            = (LONG) 0;
    lf.lfEscapement       = (LONG) 0;
    lf.lfOrientation      = (LONG) 0;
    lf.lfWeight           = (LONG) (ffs->style&FontStyleBold)?700:400;; // 700=bold
    lf.lfItalic           = (BYTE) (ffs->style&FontStyleItalic)?1:0;
    lf.lfUnderline        = (BYTE) (ffs->style&FontStyleUnderline)?1:0;
    lf.lfStrikeOut        = (BYTE) (ffs->style&FontStyleStrikeout)?1:0;
    lf.lfCharSet          = (BYTE) ffs->charset; //DEFAULT_CHARSET;//HANGUL_CHARSE;
    lf.lfOutPrecision     = (BYTE) 0;
    lf.lfClipPrecision    = (BYTE) 0;
    lf.lfQuality          = (BYTE) (antialias)?ANTIALIASED_QUALITY:NONANTIALIASED_QUALITY;
    lf.lfPitchAndFamily   = (BYTE) DEFAULT_PITCH|FF_SCRIPT;
    FFS_strcpyu(lf.lfFaceName,ffs->fontname);

    HFONT hfont = CreateFontIndirectW(&lf);
    SelectObject(hdc, hfont);

    // get font metrics

    TEXTMETRIC tm;
    GetTextMetrics(hdc,&tm);

	if (aspect!=1.0f)
	{
		// adjust for aspect ratio
		lf.lfWidth            = (LONG) FFS_REAL2INT((float)tm.tmAveCharWidth/aspect);

		// reselect font with aspect considered
		DeleteObject(hfont);
		hfont = CreateFontIndirectW(&lf);
		SelectObject(hdc, hfont);
	}

    // get glyph metrics

    GLYPHMETRICS gm; // glyph metrics
	FIXED fx0 = { 0, 0 }, fx1 = { 0, 1 };
	MAT2 matrix;
	matrix.eM11 = matrix.eM22 = fx1;
	matrix.eM12 = matrix.eM21 = fx0;

    int gco_bitmap=antialias?(antialias8?GGO_GRAY8_BITMAP:GGO_GRAY4_BITMAP):GGO_BITMAP;

    int bmpdatasize = 0;
    if (ffs->ascii)
        bmpdatasize = GetGlyphOutlineA(hdc,(UINT)smallcapsindex,gco_bitmap,&gm,0,NULL,&matrix);
    else
        bmpdatasize = GetGlyphOutlineW(hdc,(UINT)smallcapsindex,gco_bitmap,&gm,0,NULL,&matrix);

	if (bmpdatasize<=0)
	{
		antialias = 0;
		gco_bitmap = GGO_BITMAP;
	    if (ffs->ascii)
	        bmpdatasize = GetGlyphOutlineA(hdc,(UINT)smallcapsindex,gco_bitmap,&gm,0,NULL,&matrix);
	    else
	        bmpdatasize = GetGlyphOutlineW(hdc,(UINT)smallcapsindex,gco_bitmap,&gm,0,NULL,&matrix);
	}
	if (bmpdatasize < 0)
		bmpdatasize = 0;

	BYTE * pBmpBuffer = NULL;
	if (bmpdatasize>0)
    	pBmpBuffer = (BYTE *)galloc(bmpdatasize);	// we know the size now, allocate an array

    if (ffs->ascii)
        GetGlyphOutlineW(hdc,(UINT)smallcapsindex,gco_bitmap,&gm,bmpdatasize,pBmpBuffer,&matrix);
    else
        GetGlyphOutlineW(hdc,(UINT)smallcapsindex,gco_bitmap,&gm,bmpdatasize,pBmpBuffer,&matrix);

    int charw = (int)gm.gmBlackBoxX;
	int charh = (int)gm.gmBlackBoxY;

    if (ginfo->width==1 && ginfo->height==1)
        *dest = 0; // handle 0x0->1x1 for space character needing initialization

	if (pBmpBuffer)
	{
    	int bpp=antialias?8:1;
		int srowbytes=((charw*bpp+31)&~31)>>3;

		for (int y=0; y < charh ; ++y)
		{
			unsigned char *d =	(unsigned char *) dest + y*rowbytes;
			unsigned char *fontbuf = (unsigned char *)pBmpBuffer + y*srowbytes;

			for (int x = 0; x < charw ; ++x)
			{
				int a=0;

				if (bpp==1)
				{
					a = (*fontbuf>>(~x&7)&1)*15;

					if ((x&7)==7)
						++fontbuf;

				}
                else if (antialias8)
				{
					a = *fontbuf*255/64;  // GREY8 is 0..64
					++fontbuf;
				}
				else
                {
					a = (*fontbuf*15+8)/16;  // GREY4 is 0..16
					++fontbuf;
				}
				*d++ = a;
			}
		}
	}
//fonterrorexit:
	if (pBmpBuffer) gfree(pBmpBuffer);
    DeleteObject(hfont);
	DeleteDC(hdc);

    return(ok);
}

int GCALL FFS_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        FFSINFO *ffs= (FFSINFO *) gx->gref;
        if (ffs)
        {
            if (ffs->pathname[0])
                RemoveFontResourceW(ffs->pathname);
//                RemoveFontResourceExW(ffs->pathname,FR_PRIVATE,NULL); // use RemoveFontResource later
            memset(ffs,0,sizeof(FFSINFO));
        }


        ok = gfree((void *) gx);

    }
    return(ok);
}

