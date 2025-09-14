// © 1996-2003 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*             Font - Draw text onto a given bitmap                 */
/*                                                                  */
/*                     by Ian Davids, EAC                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 021003 ID initial version uses windows GDI               */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Filter Functions:                                                */
/* ----------------                                                 */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "ffont.h"
#include "dib.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#if defined(_MSC_VER) || defined(__R5900)
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include "Windows.h"

static void TTF_strcpyatou(unsigned short *d, const char *s)
{
    while (*s)
    {
        *d++ = (unsigned short)(unsigned char)*s;
        ++s;
    }
    *d = 0;
}


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutfont(void)
{
    FABOUT* about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Ian Davids");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Font");
        strcpy(about->longnamestr,  "Draw Text");
        about->numoptions =         7;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           0xffffffff;
        about->option[0].def.i =           1;
        strcpy(about->option[0].name,      "X");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0;
        about->option[1].max.i =           0xffffffff;
        about->option[1].def.i =           1;
        strcpy(about->option[1].name,      "Y");

        about->option[2].type =            FOP_PTR;
        about->option[2].min.p =           0;
        about->option[2].max.p =           (const char *)0xffffffff;
        about->option[2].def.p =           0;
        strcpy(about->option[2].name,      "Text");

        about->option[3].type =            FOP_PTR;
        about->option[3].min.p =           0;
        about->option[3].max.p =           (const char *)0xffffffff;
        about->option[3].def.p =           0;
        strcpy(about->option[3].name,      "Font");

        about->option[4].type =            FOP_INT;
        about->option[4].min.i =           2;
        about->option[4].max.i =           1000;
        about->option[4].def.i =           10;
        strcpy(about->option[4].name,      "Point Size");

        about->option[5].type =            FOP_INT|FOP_ENUM;
        about->option[5].min.i =           0;
        about->option[5].max.i =           8;
        about->option[5].def.i =           0;
        strcpy(about->option[5].name,      "Mode");
        strcpy(about->option[5].enumlist,  "Normal,Bold,Italic,Underline,BoldItalic,BoldUnderline,ItalicUnderline,BoldItalicUnderline");

        about->option[6].type =            FOP_INT|FOP_TRUE_COLOR;
        about->option[6].min.i =           0;
        about->option[6].max.i =           0xffffffff;
        about->option[6].def.i =           0xffffffff;
        strcpy(about->option[6].name,      "Text Colour");
    }
    return about;
}

FPUBLIC int openfont(FINSTANCE** filterInstance, int numsources, GBITMAP* sources, int numoptions, FOP* options)
{
    int ok = 0;
    FINSTANCE* fx;

    fx = (FINSTANCE*)galloc(sizeof(FINSTANCE));
    if (fx)
    {
        memset(fx, 0, sizeof(FINSTANCE));
        fx->frames      = 1;
        fx->numsources  = numsources;
        fx->source      = sources;
        fx->numoptions  = numoptions;
        fx->option      = options;
        ok = 1;
    }
    *filterInstance = fx;
    return ok;
}

FPUBLIC GINFO* infofont(FINSTANCE* fx, int framenum)
{
    GINFO* ginfo;
    ginfo = (GINFO*)galloc(fx->source[0].ginfo->size);

    if (ginfo)
    {
        /* copy ginfo to destination */
        memcpy(ginfo, fx->source[0].ginfo, (size_t)(fx->source[0].ginfo->size));
        ginfo->framenum = framenum;
    }
    return ginfo;
}

// REAL2INT
static int FONT_REAL2INT(float x)
{
    return ((int)(((float)(x))+((((float)(x))>=0.0f)?0.5f:-0.5f)));
}

FPUBLIC int filterfont(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes)
{
    int    x             = 10;
    int    y             = 10;
    int    PointSize     = 10;
    int    Mode          = 0;
    int    Colour        = 0xffffffff;
    const char   *Text   = "Text";
    const char   *Font   = "Arial";
    char   *source       = (char *)(fx->source[0].image);

    DIB *dib = DIB_create(ginfo->width, ginfo->height, ginfo->bpp);
    HBITMAP hbit = DIB_getbitmap(dib);

    BITMAPINFOHEADER bi;
    memset(&bi,0,sizeof(bi));
    bi.biSize          = sizeof(BITMAPINFOHEADER);
    bi.biWidth         = ginfo->height;
    bi.biHeight        = ginfo->width;
    bi.biPlanes        = 1;
    bi.biBitCount      = 32;
    bi.biCompression   = BI_RGB;
    bi.biSizeImage     = ginfo->width*4*ginfo->height;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed       = 0;
    bi.biClrImportant  = 0;


    // means that my hdc is dependent on my display settings.
    HDC hdc = CreateCompatibleDC(NULL);
    SetDIBits(hdc,hbit,0,ginfo->height,source,(BITMAPINFO *)&bi,DIB_RGB_COLORS);

    BeginPath(hdc);
    SetTextColor(hdc,RGB((Colour>>16)&0xFF,(Colour>>8)&0xFF,Colour&0xFF));
    //SetBkMode(hdc,TRANSPARENT);

    int tok = TextOut(hdc,10,10,"Hello",5);
    printf("textout return %d", tok);

    EndPath(hdc);

    GetDIBits(hdc,hbit,0,ginfo->height,dst,(BITMAPINFO *)&bi,DIB_RGB_COLORS);

    DIB_destroy(dib);
    DeleteDC(hdc);

#if 0
//    LOGFONTW lf;
    if (fx->option)
    {
        if (fx->numoptions>=1) x             = fx->option[0].i;
        if (fx->numoptions>=2) y             = fx->option[1].i;
        if (fx->numoptions>=3) Text          = fx->option[2].p;
        if (fx->numoptions>=4) Font          = fx->option[3].p;
        if (fx->numoptions>=5) PointSize     = (int) fx->option[4].f;
        if (fx->numoptions>=6) Mode          = fx->option[5].i;
        if (fx->numoptions>=7) Colour        = fx->option[6].i;
    }

    HFONT  hfont         = 0;
    memset(&lf,0,sizeof(lf));

    lf.lfHeight           = (LONG) FONT_REAL2INT((float)(-PointSize*96.0f/72.0/*aspect*smallcapsaspect*/));
    lf.lfWidth            = (LONG) 0;
    lf.lfEscapement       = (LONG) 0;
    lf.lfOrientation      = (LONG) 0;
    lf.lfWeight           = (LONG) (Mode==1||Mode==4||Mode==5||Mode==7)?700:400;; // 700=bold
    lf.lfItalic           = (BYTE) (Mode==2||Mode==4||Mode==6||Mode==7)?1:0;
    lf.lfUnderline        = (BYTE) (Mode==3||Mode==5||Mode==6||Mode==7)?1:0;
    lf.lfStrikeOut        = (BYTE) 0;
    lf.lfCharSet          = (BYTE) DEFAULT_CHARSET; //DEFAULT_CHARSET;//HANGUL_CHARSE;
    lf.lfOutPrecision     = (BYTE) 0;
    lf.lfClipPrecision    = (BYTE) 0;
    lf.lfQuality          = (BYTE) ANTIALIASED_QUALITY;
    lf.lfPitchAndFamily   = (BYTE) DEFAULT_PITCH;
    TTF_strcpyatou((unsigned short *)lf.lfFaceName,Font);


    hfont = CreateFontIndirectW(&lf);
    SelectObject(hdc, hfont);
    // get font metrics
    TEXTMETRIC tm;
    GetTextMetrics(hdc,&tm);
    // adjust for aspect ratio
    lf.lfWidth            = (LONG) FONT_REAL2INT((float)tm.tmAveCharWidth/*/aspect*/);
    // reselect font with aspect considered
    DeleteObject(hfont);
    hfont = CreateFontIndirectW(&lf);
    SelectObject(hdc, hfont);


//    SetTextColor(hdc,RGB((Colour>>16)&0xFF,(Colour>>8)&0xFF,Colour&0xFF));
//    SetBkMode(hdc,TRANSPARENT);

    bi.biSize          = sizeof(BITMAPINFOHEADER);
    bi.biWidth         = ginfo->height;
    bi.biHeight        = ginfo->width;
    bi.biPlanes        = 1;
    bi.biBitCount      = 32;
    bi.biCompression   = BI_RGB;
    bi.biSizeImage     = ginfo->width*4*ginfo->height;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed       = 0;
    bi.biClrImportant  = 0;


#endif

//    DeleteObject(hbit);
//    DeleteObject(hfont);

    return 1;
}

FPUBLIC int closefont(FINSTANCE *fx)
{
    return gfree((void*)fx);
}
