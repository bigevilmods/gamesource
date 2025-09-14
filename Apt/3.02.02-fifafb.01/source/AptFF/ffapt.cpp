/*H************************************************************************************************/
/*!

\File
ffApt.cpp

\Description
This is an implementation of font fusion that is used by Apt Aux to render fonts. The functions here
are admittedly far from optimal, and font fusion has many quirks about it but the system works so we
use it.

\Notes
Font Fusion's error handling mechanism is beyond description.  Managing references to objects after Font Fusion
encounters an error is a black art.  So, the philosophy here is that asserts will catch a lot of stuff, and you
really, really, really don't want to be getting errors in a final product. [Welcome to Font Fusion!]


\Copyright
(c) 2005 Electronic Arts Inc.

\Version
1.0     2004/05/06  (DC)    Comment Creation
1.1     2004/06/18  (rrv:1) mods to correct handling of MaxScroll; calculate the number of
                            lines being rendered, then compared to actual number counted in the loop;
                            also, adjust the scroll value depending on the calculated MaxScroll, and
                            adjust the line offset accordingly.
1.2     2004/07/19  (rrv:1) correction to text and font scaling factor (bugs# 576-578)

*/
/************************************************************************************************H*/
#include "ffapt.h"
#include "Apt.h"

#include "math.h"

   float gAptFFAdditionalScaleFactorX = 1.f;

//
// (1.2:rrv:1/1)
// --- these values may need to be adjusted depending on the font being used
// ---  fonts differences affect the scaling, so a balance needs to be struck
// ---  between the amount of memory needed and the resolution achieved.
float gAptFFScaleFactorX    = 0.7071f;
float gAptFFScaleFactorY    = 0.7071f;
int gAptFFScaleAtThisPtSize = 15;
//

CFontFusion FF;

const int CACHE_SIZE = 32 * 1024;
// $FR#598 CSB - Setting data member Last Font Style 27/36
CFontFusion::CFontFusion() : pCache(NULL), pManager(NULL), pScaler(NULL), bInitialized(0), nFonts(0), m_nLastFont(-1), nLastFontStyle(0xFFFFFFFF) {}


/*F************************************************************************************************/
/*!
Flush the Font fustion cache (duh!)

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void CFontFusion::flushCache()
{
    APT_ASSERT(pCache);

    int nErr = 0;
    FF_CM_Flush(pCache, &nErr);
    APT_ASSERT(!nErr);
}


/*F************************************************************************************************/
/*!
intialize font fusion.

\Input      nFonts (number of fonts)
            pData  (Array of buffers, one for each font)
            pSize  (Array of buffer sizes, one for each font.)

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void CFontFusion::init(int nFonts, void **pData, int  *pSize)
{
    APT_ASSERT(!bInitialized);

    // Beautiful error mechanism, I wonder why nobody else uses it?  >:-(
    int nError = 0;

    // Create a cache manager
    pCache = FF_CM_New(CACHE_SIZE, &nError);
    APT_ASSERT(pCache);

    // make a font manager
    pManager = FF_FM_New(&nError);
    APT_ASSERT(pManager);

    FF_FM_SetPlatformID(pManager, 3);           // reportedly Microsoft ID
    FF_FM_SetPlatformSpecificID(pManager, 1);   // reportedly Unicode
    FF_FM_SetLanguageID(pManager, 0x0409);      // from MS' documentation
    FF_FM_SetNameID(pManager, 1);               // idem

    APT_ASSERT(!nError);

    APT_ASSERT(nFonts < FF_MAX_FONTS);
    this->nFonts = nFonts;

    for (int i = 0; i < nFonts; i++)
    {
        // Whip up the required data.
        // Font Fusion's error-handling is pure crap (setjmp/longjmp), so there are asserts everywhere here.
        fontData *pFont = &fonts[i];
        int nError = 0;

        pFont->pData = (unsigned char *)pData[i];
        APT_ASSERT(pFont->pData);

        // Get a new font memory handler
        pFont->pMem = tsi_NewMemhandler(&nError);
        APT_ASSERT(!nError);

        // Tell Font Fusion this font is in RAM
        pFont->pStream = New_InputStream3(pFont->pMem, pFont->pData, pSize[i], &nError);
        APT_ASSERT(!nError);

        // Register this crap with the font manager
        FF_FM_AddTypefaceStream(pManager, pFont->pStream, NULL, &nError);
        APT_ASSERT(!nError);

        // Actually create the font in the font manager
        T2K_TRANS_MATRIX    transform;
        transform.t00 = ONE16Dot16 * 12;
        transform.t01 = 0;
        transform.t10 = 0;
        transform.t11 = ONE16Dot16 * 12;
#if defined(FF_230)
        int8 bFlushCache = false; // initialize to false, FF will change if cache stranding can occur. don't think we use this now neways.
#else
        int8 bFlushCache;
#endif
        // I love how we have to get a value by passing a pointer to it to know whether we have to manually
        // flush their cache, even though their "classes" are totally coupled
        pFont->nCode = FF_FM_CreateFont(pManager, static_cast<uint16_t>( i ), &bFlushCache, &transform, NULL, &nError);
        APT_ASSERT(!nError);

        if (bFlushCache)
        {
            FF_CM_Flush(pCache, &nError);
            APT_ASSERT(!nError);
        }

        // We're not using any filters, so set the tag to 0, this way the cache won't get conf00sed
        FF_CM_SetFilter(pCache, 0, NULL, NULL);
    }

    bInitialized = 1;
}


/*F************************************************************************************************/
/*!
deletes all font data.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void CFontFusion::purge()
{
    APT_ASSERT(bInitialized);
    int nError = 0;

    // 2004/05/08 -- LH -- Fixed minor issue where m_nLastFont was not cleared on purge
    m_nLastFont = -1;

    // $FR#598 CSB - Setting Last Font Style 28/36
    nLastFontStyle = 0xFFFFFFFF;

    for (int i = 0; i < nFonts; i++)
    {
        fontData *pFont = &fonts[i];

        FF_FM_DeleteFont(pManager, static_cast<uint16_t>( pFont->nCode ), &nError);
        APT_ASSERT(!nError);

        Delete_InputStream(pFont->pStream, &nError);
        APT_ASSERT(!nError);

        tsi_DeleteMemhandler(pFont->pMem);
        APT_ASSERT(!nError);
    }

    FF_FM_Delete(pManager, &nError);
    APT_ASSERT(!nError);
    pManager = 0;

    FF_CM_Delete(pCache, &nError);
    APT_ASSERT(!nError);
    pCache = 0;

    bInitialized = 0;
}

CFontFusion::~CFontFusion()
{
}


/*F************************************************************************************************/
/*!
This function sets the font scale (makes it bigger or smaller).

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void CFontFusion::setStrokeFontScale(int nFont, float fScale)
{
    APT_ASSERT(nFont >= 0 && nFont < nFonts);
    afScales[nFont] = fScale;
}

/*F************************************************************************************************/
/*!
This function gives you the name of the indexed font.

\Input

\Output

\Notes
for language id, refer to http://support.microsoft.com/?kbid=221435.
English US: 1033
Japanese: 1041

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void CFontFusion::getName(int nFont, unsigned short *szFontName, int nLanguageID)
{
    APT_ASSERT(nFont >= 0 && nFont < nFonts);

    int nError = 0;
    FF_FM_SetPlatformID(pManager, 3);
    FF_FM_SetPlatformSpecificID(pManager, 1);
    FF_FM_SetLanguageID(pManager, static_cast<uint16_t>( nLanguageID ));
    FF_FM_SetNameID(pManager, 1);
    pScaler = FF_FM_SelectFont(pManager, static_cast<uint16_t>( fonts[nFont].nCode ), &nError);
    APT_ASSERT(!nError);
    T2K_SetNameString(pScaler, static_cast<uint16_t>( nLanguageID ), 1, &nError);
    APT_ASSERT(!nError);
    unsigned short *pSrc = pScaler->nameString16;

    if (pSrc)
    {
        for (;;)
        {
            if (*pSrc == 0)
            {
                break;
            }
            *szFontName++ = *pSrc++;
        }
    }
    *szFontName = 0;
}

int HACK_FF_AntialiasMode = T2K_TV_MODE_2 | T2K_SCAN_CONVERT;
int HACK_FF_BitmapMode = GREY_SCALE_BITMAP_HIGH_QUALITY;


/*F************************************************************************************************/
/*!
This function calculates the width in pixels of a string that is passed in. This is useful for
getting the required size of textures and such.

\Input
    int   nFont - Index of desired font in font list
    const unsigned short *text - string to use
    short *xKernValuesInFUnits - kerning values, unused.
    int numChars - number of chars to include in the calculation. This must not exceed strlen of string!

\Output     int - Width in pixels the string will need to render in.

\Notes

\Version
1.0        06/23/04 (DC) Function creation. Added to work around caveats in Font fusion call.

*/
/************************************************************************************************F*/
int CFontFusion::HAK_MeasureTextInX(int nFont, const unsigned short *text, short *xKernValuesInFUnits, int numChars )
{
#if 0 // This uses the old method. Faster, but less accurate
    return  T2K_MeasureTextInX(pScaler, text, xKernValuesInFUnits, numChars);
#else // Tis is the new method
    int nPenLocation = 0;
    int nMaxWidth    = 0;
    int nTempMaxWidth= 0;
    int nError;

    if(numChars < 1)
        return 0;

    // Checks the FontFusion cache for previously created glyph, if it doesn't exist, it will render and cache it.
    FF_CM_RenderGlyph(pCache, static_cast<uint16_t>( fonts[nFont].nCode ), &pScaler, text[0], 0, 0, static_cast<uint8_t>( HACK_FF_BitmapMode ), static_cast<uint16_t>( HACK_FF_AntialiasMode ), &nError);

    APT_ASSERT(!nError);

    if(pScaler->fLeft26Dot6<0)
        nPenLocation -= pScaler->fLeft26Dot6>>6;

    for(int i = 0; i<numChars; i++)
    {
        // Checks the FontFusion cache for previously created glyph, if it doesn't exist, it will render and cache it.
        FF_CM_RenderGlyph(pCache, static_cast<uint16_t>( fonts[nFont].nCode ), &pScaler, text[i], 0, 0, static_cast<uint8_t>( HACK_FF_BitmapMode ), static_cast<uint16_t>( HACK_FF_AntialiasMode ), &nError);

        APT_ASSERT(!nError);

        nPenLocation += pScaler->xAdvanceWidth16Dot16>>16;

        nTempMaxWidth     = nPenLocation + (pScaler->width - ((pScaler->xAdvanceWidth16Dot16>>16) - (pScaler->fLeft26Dot6>>6)));
        if(nTempMaxWidth > nMaxWidth)
        {
            nMaxWidth = nTempMaxWidth;
        }
    }

    return nMaxWidth;
#endif
}

/*F************************************************************************************************/
/*!
This function finds the largest character in a line so the difference can be added to the rest of the characters at render time.
This is needed to align all of the characters to their baseline.

\Input
int   nFont - Index of desired font in font list
const unsigned short *text - string to use
int xYBase -
int numChars - number of chars to include in the calculation. This must not exceed strlen of string!

\Output     int - Y Adjustment value.

\Notes

\Version
1.0        11/19/04 (MCB) Function creation. Added to work around caveats when using characters with accents. $bug764

*/
/************************************************************************************************F*/
int CFontFusion::HAK_MeasureTextInY(int nFont, const unsigned short *text, int xYBase, int numChars )
{
    int nMaxYAdjustment = 0;
    int nError;
    int y = 0;

    if(numChars < 1)
        return 0;

    // Checks the FontFusion cache for previously created glyph, if it doesn't exist, it will render and cache it.
    FF_CM_RenderGlyph(pCache, static_cast<uint16_t>( fonts[nFont].nCode ), &pScaler, text[0], 0, 0, static_cast<uint8_t>( HACK_FF_BitmapMode ), static_cast<uint16_t>( HACK_FF_AntialiasMode ), &nError);

    APT_ASSERT(!nError);

    y = (((xYBase >> 10) - pScaler->fTop26Dot6) >> 6);
    if (y < nMaxYAdjustment)
    {
        nMaxYAdjustment = y;
    }

    for(int i = 0; i<numChars; i++)
    {
        // Checks the FontFusion cache for previously created glyph, if it doesn't exist, it will render and cache it.
        FF_CM_RenderGlyph(pCache, static_cast<uint16_t>( fonts[nFont].nCode ), &pScaler, text[i], 0, 0, static_cast<uint8_t>( HACK_FF_BitmapMode ), static_cast<uint16_t>( HACK_FF_AntialiasMode ), &nError);

        APT_ASSERT(!nError);

        y = (((xYBase >> 10) - pScaler->fTop26Dot6) >> 6);
        if (y < nMaxYAdjustment)
        {
            nMaxYAdjustment = y;
        }
    }

    return nMaxYAdjustment;
}

/*F************************************************************************************************/
/*!
This function basically takes in a pointer to the base of a line in a string and returns the number
of characters in the line, and the width of the rendered line. Lines are defined by \r \n or
optionally by a wrap width (more time consuming). This returns the pointer to the base of the next
line in the string. (or a pointer to the null character in the case of the end of a string.)

\Input      const uint16 *wszLineStart - pointer to base of line to be analyzed
            bool  bWrapAtBounding - true = wrap at bounding box (note, can be time consuming)
            int   nWrapWidth - if wrapping, this is width to wrap at.
            int * nOutLineChars - [OUT] - characters in the current line.
            int * nOutTextWidth - [OUT] - width of rendered line.
            const short anKerning[] - [IN/OUT] - kerning values, not sure if input or output...

\Output     const uint16 * - pointer to base of next line.

\Notes
This can be greatly optimized for wrapping case by not calculating the width of the whole string for
every length. To do this I would recommend guessing the length of the string using the wraping width
and the average character width (generally defined by the width of the letter 'x' no jokes!).

\Version
1.0        05/04/04 (DC) Function creation. This was being done in two locations, also made somewhat
                         smarter (and much faster).

*/
/************************************************************************************************F*/
const uint16 * const CFontFusion::AnalyzeLine(const uint16 *wszLineStart,
                                              bool          bWrapAtBounding,
                                              int           nWrapWidth,
                                              int          *nOutLineChars,  /* OUT Parameter */
                                              int          *nOutTextWidth,  /* OUT Parameter */
                                              short         anKerning[],
                                              int           nFont)
{
    const uint16 *pCurrentChar = wszLineStart;
    uint16  wThisChar;
    int     nDeadChars = 0; // chars not rendered at the end of the line.

    // Handle Optimal Case of no Wrapping. (This makes things pretty quick.)
    if(!bWrapAtBounding)
    {
        while ( (wThisChar = *pCurrentChar) != '\0')
        {
            // Handle Explicit Line Separators Here.
            if ( wThisChar == '\r')
            {
                nDeadChars++;
                if (*pCurrentChar == '\n')
                {
                    nDeadChars++;
                }
                break;
            }
            if ( wThisChar == '\n')
            {
                nDeadChars++;
                break;
            }
            pCurrentChar++;
        }
        *nOutLineChars = (pCurrentChar - wszLineStart);
        *nOutTextWidth = HAK_MeasureTextInX(nFont,  wszLineStart, anKerning, *nOutLineChars);
    }

    else // if wrapping, Lots more to do. What we do here is advance one word at a time, until we
    {    // overflow our nWrapWidth, at which time we go back to the last word separator.

        int nCurrentWidth    = 0; // running length as we go.
        int nPrevCharWidth   = 0; // Width of the line at the end of the previous word separator.
        int nPrevWordWidth   = 0; // Width of the line at the end of the previous character.
        const uint16 * pPrevWordSeparator = wszLineStart; // pointer to end of previous word


        while ( (wThisChar = *pCurrentChar) != '\0')
        {
            // Handle Explicit Line Separators Here.
            if ( wThisChar == '\r')
            {
                nDeadChars++;
                if (pCurrentChar[1] == '\n')
                {
                    nDeadChars++;
                }
                break;
            }
            // Handle Explicit Line Separators Here.
            if ( wThisChar == '\n')
            {
                nDeadChars++;
                break;
            }

            // We are wrapping, look for word separators and see when we overflow our box.
            // Add Word Separator characters Here (if any others are desired)
            if ( wThisChar == ' ' || wThisChar == '\t')
            {
                nPrevWordWidth     = nCurrentWidth;
                pPrevWordSeparator = pCurrentChar;
            }

            nPrevCharWidth = nCurrentWidth;
            nCurrentWidth  = HAK_MeasureTextInX(nFont,wszLineStart, anKerning, pCurrentChar-wszLineStart+1);

            // Note: Do NOT equal or exceed wrap width, or else we will overflowing our texture!
            if (nCurrentWidth >= nWrapWidth)
            {
                if(pPrevWordSeparator == wszLineStart)
                {
                    if(wszLineStart == pCurrentChar)
                    {
                        // if one letter doesn't fit just let it go.
                        pCurrentChar++; // Advance to the next char, and move on.
                    }
                    else
                    {
                        // if one word does not fit, wrap in-word. (just subtract off the current char data).
                        nCurrentWidth = nPrevCharWidth;
                    }
                }
                else
                {
                    // To get here we must have seen a char separator. add that to dead chars.
                    nDeadChars++;
                    pCurrentChar   = pPrevWordSeparator;
                    nCurrentWidth  = nPrevWordWidth;
                }
                break;
            }
            pCurrentChar++;
        }
        *nOutLineChars = (pCurrentChar - wszLineStart);
        *nOutTextWidth = nCurrentWidth;
    }

    return wszLineStart + *nOutLineChars + nDeadChars;
}

/*F************************************************************************************************/
/*!
This monstrosity some or all of a string. This will render only what is visible within the bounding
box. We still do some parsing outside of the box inorder to get the total number of lines and such,
but they are not rendered. This is not optimized to say the least but is definitely not as wastefull
as it has been in the past.


\Input      int nFont - Font Index
            uint16 *wszString - string to render (in UCS2 format)
            int nPointSize - Font Size
            int nTextureWidth - (IN) Width of texture created (>= size of text field)
            int nTextureHeight - (IN) Height of texture created (>= size of text field)
            unsigned char *pData - texture Buffer. (i.e. from AptAuxEAGLREAL_CreateFontTexture())
            unsigned int *pnTextWidth, - OUT Text Width, can be used for auto size.
            unsigned int *pnTextHeight - OUT Text Height, can be used for auto size.
            float *pfYAscender - value we ascend each line I think.
            int nBorderSize - Size of Border Drawn
            int nX - starting x coordinate
            int nY - starting y coordinate
            int bWrap - does text wrap.
            int nBoxWidth - Width of bounding box (used for wrap)
            Alignment eAlignment - alignment types
            int nLineOffset - line to start rendering (for scrolling)

\Output     int - number of lines that can be scrolled (Max useable nLineOffset value)

\Notes
    Previous comment states that
    "we don't do kerning because flash doesn't.. not sure if that's a great thing,
     but there you have it"
    I am not sure if this comment is/was correct.
    Also, this function has SO MANY arguments! I'm sure there is a better way!

\Version
1.0     05/04/04 (DC)   Comment creation. Function revamped to function more closely to
                        the flash player and also to not be quite so wasteful.
1.1     06/18/04 (rrv)  mods to correct handling of MaxScroll; calculate the number of
                        lines being rendered, then compared to actual number counted in the loop;
                        also, adjust the scroll value depending on the calculated MaxScroll, and
                        adjust the line offset accordingly.
*/
/************************************************************************************************F*/
int CFontFusion::renderString( int           nFont,
                               const uint16 *wszString,
                               int           nPointSize,
                               int           nTextureWidth,
                               int           nTextureHeight,
                               unsigned char *pData,
                               unsigned int  *pnTextWidth,
                               unsigned int  *pnTextHeight,
                               float         *pfYAscender,
                               int           nBorderSize,
                               int           nX,
                               int           nY,
                               int           bWrap,
                               // $FR#598 CSB - Adding extra parameter for Font Style 29/36
                               unsigned int  nFontStyle,
                               int           nBoxWidth,
                               int           nBoxHeight,
                               Alignment     eAlignment,
                               int           nLineOffset )
{
    APT_ASSERT(bInitialized);
    APT_ASSERT(nFont >= 0 && nFont < nFonts);
    APT_ASSERT(nPointSize > 0);
    const int MAX_STRING = 512;
    short anKerning[MAX_STRING];
    int   nError = 0;
    int nYnegAdjustment = 0; // $bug764 - MCB
    bool bFontStyleChanged = true; // $fr834 - MCB - Apt20 optimization - Only recalculate and set the transform if the font changes

    // $FR#598 CSB - Checking the Last Font Style to decide whether to flush the Font Fusion Cache 30/36

    if (nLastFontStyle != nFontStyle)
    {
        FF_CM_Flush(pCache, &nError);
        nLastFontStyle = nFontStyle;
        bFontStyleChanged = true;
    }
    APT_ASSERT(!nError);

    // added not calling FF_FM_SelectFont if it is same as last time font
    if ((m_nLastFont != nFont) || (pScaler == NULL))
    {
        pScaler = FF_FM_SelectFont(pManager, static_cast<uint16_t>( fonts[nFont].nCode ), &nError);
        m_nLastFont = nFont;       // store current font for next operation.
    }
    APT_ASSERT(!nError);

    float HACK_FF_ScaleX = nPointSize < gAptFFScaleAtThisPtSize ? 1.0f : gAptFFScaleFactorX;
    float HACK_FF_ScaleY = nPointSize < gAptFFScaleAtThisPtSize ? 1.0f : gAptFFScaleFactorY;

    if(bFontStyleChanged)
    {
        T2K_TRANS_MATRIX trans;

        trans.t00 = (F16Dot16)(ONE16Dot16 * nPointSize * HACK_FF_ScaleX * gAptFFAdditionalScaleFactorX);
        // $FR#598 CSB - Setting up Matrix Translation for italic font in Font Fusion 31/36
        //double dItalicAngle = 15.0; // should be passed in, for the time being this is not changeable by the user
        if ((nFontStyle & AptFontStyle_Italic) == AptFontStyle_Italic)
        {
            trans.t01 = (F16Dot16)(ONE16Dot16 * sin(15.0f /*dItalicAngle*/) * nPointSize * HACK_FF_ScaleY);
        }
        else
        {
            trans.t01 = 0;
        }
        trans.t10 = 0;
        trans.t11 = (F16Dot16)(ONE16Dot16 * nPointSize * HACK_FF_ScaleY);
        T2K_NewTransformation(pScaler, 1, 72, 72, &trans, 1, &nError);
        if ( T2K_GetNumAxes( pScaler ) == 1 )
        {
            T2K_SetCoordinate(pScaler, 0, (F16Dot16)(0x10000*.7f));
        }
        APT_ASSERT(!nError);
        bFontStyleChanged = false;
    }

    // $FR#598 CSB - So Far, this is NOT making the text bold. Not sure what Font Fusion means by emboldening, can be removed 32/36
    if((nFontStyle & AptFontStyle_Bold) == AptFontStyle_Bold)
    {
#ifdef ALGORITHMIC_STYLES

        tsiMemObject            *   mem      = NULL;
        InputStream             *   in       = NULL;
        sfntClass               *   font     = NULL;
        short                       fontType = FONT_TYPE_TT_OR_T2K;
        int                         errCode;
        T2K_AlgStyleDescriptor      style;

        style.StyleFunc        = tsi_SHAPET_BOLD_GLYPH;
        style.StyleMetricsFunc = tsi_SHAPET_BOLD_METRICS;
        style.params[0]        = 5L << 14;
        font                   = FF_New_sfntClass( mem, fontType, 0, in, NULL, &style, &errCode );

#endif
    }
    // we put the point size as a filter tag as a hack because the cache doesn't take into account size normally.
    // this lets us not have to flush the cache just because we rendered the same font in a different size
    APT_ASSERT(nPointSize < 256);
    FF_CM_SetFilter(pCache, static_cast<uint8_t>( nPointSize ), NULL, NULL);

    // DC Bug Number 266 Text was being drawn at an incorrect offset, need the line gap here.
    // base values are <<16!
    int nBorderSizeLL16 = nBorderSize << 16;
    // 2X bordersize
    int nBorderSizeLL1  = nBorderSize << 1;

    // base values are << 16!
    int xYBase = pScaler->yAscender + 0x8000 + nBorderSizeLL16  + pScaler->yLineGap + (nY << 16); // base values are << 16!

    // calculate the number of lines that will be rendered based on font size, box height, ascender/descender, gap/gutter/border, etc
    //  this value is used to 'normalize' the scroll and maxscroll values (note the shift 16!)
    int nNumer = ( (nBoxHeight-nBorderSizeLL1) << 16);
    //  and these are already <<16
    int nDenom = (pScaler->yAscender + 0x8000 - pScaler->yDescender + pScaler->yLineGap);

    // calculated number of lines that will be rendered
    int nLinesRenderedCalc = (int)( nNumer / nDenom );

    const uint16 *pLineOffset       = wszString;    // Current Line
    const uint16 *pNextLineOffset   = wszString;    // Next Line Start (after \n's or whatever)
    const uint16 *pHoldOffset       = wszString;    // place holder

    int     nMaxLineWidth           = 0;            // Width of the longest Line.
    int     nLinesRendered          = 0;            // Number of lines actually rendered
    int     nLinesTotal             = 0;            // Total number of lines in the field.
    int     nLineChars              = 0;            // Number of chars in the current line
    int     nLineWidth              = 0;            // Width of the current line.
    bool    bFilledBox              = false;        // true = we have finished rendering what is inthe box.
    int     nMaxScrollCalc          = 0;            // calculated value for MaxScroll

    // calculate the number of lines that will be rendered
    //  intercept the line referenced by the scroll value, and
    //  then, adjust scroll value based on the MaxScroll
    {
        //  line offset pointer array size
        const int   HOLDARRAYSZ = 128;
        const   uint16* apHoldOffset[HOLDARRAYSZ];
        int     nLnIndex        = 0;                // index into hold array

        int     nWrapWidth      = nBoxWidth - nBorderSizeLL1;

        bool    bWrapAtBound    = (bWrap!=0);

        memset( apHoldOffset, 0, sizeof(apHoldOffset) );

        // First, calculate total number of lines in text
        //  to check if the scroll value is consistent with the new text string
        //      and, while we are at it, capture the max line width
        while( pLineOffset[0] != '\0')
        {
            //
            pNextLineOffset = AnalyzeLine(pLineOffset, bWrapAtBound, nWrapWidth, &nLineChars, &nLineWidth, anKerning , nFont);
            if(nLineWidth > nMaxLineWidth)
            {
                nMaxLineWidth = nLineWidth;
            }
            // count the line
            nLinesTotal++;
            // if we are at the target line offset, save its pointer
            if(nLinesTotal==nLineOffset)
            {
                pHoldOffset = pLineOffset;
            }
            // save the pointer for later use, if need be
            apHoldOffset[nLnIndex++] = pLineOffset;
            // array populated in round robin manner, so it always has the last HOLDARRAYSZ entries
            if(nLnIndex==HOLDARRAYSZ)
            {
                nLnIndex    = 0;
            }
            // next line
            pLineOffset = pNextLineOffset;
        }

        // calculate maxscroll value
        nMaxScrollCalc = nLinesTotal - nLinesRenderedCalc + 1;

        // adjust maxscroll if total number of lines less than render count
        if(nMaxScrollCalc<1)
        {
            nMaxScrollCalc = 1;
        }

        // adjust scroll value for consistency with number of rendered lines and total lines
        if(nLineOffset>nMaxScrollCalc)
        {
            // delta between last line and the (calculated) number of lines that are rendered
            // $bug626 - DC - Added to 16.01 / 17.00 Must subtract the lesser of total lines and renderable lines.
            int nuix = nLnIndex - ( (nLinesRenderedCalc<nLinesTotal) ? nLinesRenderedCalc : nLinesTotal) ;

            // if negative index, wrap around the end of the array
            if(nuix<0)
            {
                nuix    += HOLDARRAYSZ;
            }

            // still, be careful in case the number of lines rendered exceed the size of the array!
            if(nuix>=0)
            {
                pHoldOffset = apHoldOffset[nuix];
            }
            nLineOffset = nMaxScrollCalc;
        }

    }   // end of block

    // reposition pointer to scroll position
    pLineOffset = pHoldOffset;

     // $bug764 - MCB - calculate the Y adjustment to align all characters on their baseline for the 1st line, use for all lines
    nYnegAdjustment = HAK_MeasureTextInY(nFont, pLineOffset, xYBase, nLineChars);

    // Now time to start rendering
    while ( pLineOffset[0] != '\0' && !bFilledBox)
    {
        F16Dot16 xAlignmentOffset = 0;

        pNextLineOffset = AnalyzeLine(pLineOffset, bWrap!=0,nBoxWidth-(nBorderSizeLL1), &nLineChars, &nLineWidth, anKerning , nFont);

        if( bWrap && (pNextLineOffset-pLineOffset) == 1 && nLineWidth > (nBoxWidth+(nBorderSizeLL1)+1))
        {
            // couldn't even fit one char, well, don't cry over spilled milk. move along.
            break;
        }

        APT_ASSERT(nLineChars < MAX_STRING);
        APT_ASSERT(!bWrap || (bWrap && nLineWidth <= (nBoxWidth+(nBorderSizeLL1)+1)));

        if (eAlignment == CFontFusion::Right)
        {
            xAlignmentOffset = (nBoxWidth - nBorderSizeLL1 - nLineWidth) << 16;         // ### updated for center fix
        }
        else if (eAlignment == CFontFusion::Centre)
        {
            xAlignmentOffset = (nBoxWidth - nBorderSizeLL1 - nLineWidth) << 15;         // ### updated for center fix
        }
        // else if Left, value is already zero!

        if (xAlignmentOffset < 0)
        {
            xAlignmentOffset = 0;
        }

        int xXBase = nBorderSizeLL16 + (nX * 0x10000) + xAlignmentOffset; // base values are << 16!

        // Figure out how much space this takes and where we should start drawing
        unsigned char *pTexRow, *pTex, *pChar;
        int     nRenderY;
        int     nIndex = 0;

        for (nIndex = 0; nIndex < nLineChars; nIndex++)
        {
            // Checks the FontFusion cache for previously created glyph, if it doesn't exist, it will render and cache it.
            FF_CM_RenderGlyph(pCache, static_cast<uint16_t>( fonts[nFont].nCode ), &pScaler, pLineOffset[nIndex], 0, 0, static_cast<uint8_t>( HACK_FF_BitmapMode ), static_cast<uint16_t>( HACK_FF_AntialiasMode ), &nError);

            APT_ASSERT(!nError);

            int x = (((xXBase >> 10) + pScaler->fLeft26Dot6) >> 6);
            int y = (((xYBase >> 10) - pScaler->fTop26Dot6) >> 6) - nYnegAdjustment;  // $bug764 - MCB - add in adjustment to vertically align all characters on the baseline.
            APT_ASSERT(y >= 0);

            // DC -- According to FF docs, only render if baseAddr is non-NULL. Doing so just wastes those cycles...
            // Typically pScaler->baseAddr will be null for whitespace characters.
            if(pScaler->baseAddr != NULL)
            {
                // Start address for the copy - wack 16.16 and 26.6 fixed-point math
                pTexRow = &pData[y * nTextureWidth + x];
                for (nRenderY = 0; nRenderY < pScaler->height; nRenderY++, pTexRow += nTextureWidth)
                {
                    // This is far from optimal, but only run once in a blue moon anyway
                    int tempx = 0;
                    for (pTex = pTexRow, pChar = &pScaler->baseAddr[nRenderY * pScaler->width];
                        pTex < pTexRow + pScaler->width;
                        pTex++, pChar++, tempx++)
                    {
                        if (x + tempx >= nBoxWidth)
                        {
                            break;
                        }
                        // 2004/06/16 -- DC -- Bug 491 -- Font overlap
                        // Basically in the case that characters render over the previous character you end up with
                        // two problems,
                        // 1) adding the color values causes saturation and overflow. Text is rendered in one color.
                        //    summing those color values results in multiple color values...
                        // 2) Just placing the value from the new char will cause all the empty pixels in the new
                        //    char to overwrite the used pixels in the last chars (parts get chopped off.)
                        //
                        // To rectify these problems, instead of doing an if (pChar != 0) { *pTex = *pChar }
                        // we instead do an |= which since we always render in one color, will only modify the
                        // previous pixel values where the new character is drawing. This works like a charm without
                        // sucking up the processor cycles.
                        *pTex |= *pChar;
                    }
                }
            }

            // Update Base values.
            if (pScaler->horizontalFontMetricsAreValid)
            {
                xXBase += pScaler->xAdvanceWidth16Dot16;
            }
            if (pScaler->verticalFontMetricsAreValid)
            {
                xYBase += pScaler->yAdvanceWidth16Dot16;
            }
        }

        nLinesRendered++;

        // Update y Base to the next line.
        xYBase += (pScaler->yAscender - pScaler->yDescender) + pScaler->yLineGap - nYnegAdjustment;  // $bug764 - MCB

        // Current = next.
        pLineOffset = pNextLineOffset;

        // If the next line takes us out of the box, break. Note that the final line
        // does not need a line gap and should not include the descender.
        // the (pScaler->yAscender>>3) is somewhat of a fudge factor. Unfortunately
        // flash player will squeeze a line in if it's close. This is a way of guessing
        // how close we can be based off of the font size (as opposed to a fixed value).
        // I have no way to prove this is what flash does, but it provides similar
        // results to the flash player.
        // (1.1:rrv:1/1) --- note that the fudge factor has been removed!
        if( ((xYBase + pScaler->yAscender - pScaler->yLineGap )>>16) > (nBoxHeight+nBorderSizeLL1) )
        {
            bFilledBox = true;
            break;
        }

    } // End Loop();

    // Text Height property is updated even when not all lines are rendered!
    xYBase += (nLinesTotal - nLinesRendered) * ((pScaler->yAscender - pScaler->yDescender) + pScaler->yLineGap);
    xYBase += (pScaler->yDescender - pScaler->yLineGap);

    *pnTextWidth = nMaxLineWidth;
    *pnTextHeight = (xYBase - (pScaler->yAscender + pScaler->yLineGap))>>16;

    if (pfYAscender)
    {
        *pfYAscender = (pScaler->yAscender + (nBorderSizeLL16) + 0x8000) / 65536.f;
    }

    // +1 because MaxScroll value is a base-1 not a base-0 number.
    int nReturnMaxScroll = nLinesTotal - nLinesRendered + 1;

#if defined(EA_DEBUG) && !defined(DO_COVERAGE)
    if( nMaxScrollCalc != nReturnMaxScroll )
    {
        gAptFuncs.pfnDebugPrint("<warning msg='calculated rendered lines (%d) differs from count (%d). (%s:%d)'/>\n",
                        nLinesRenderedCalc, nLinesRendered, __FILE__, __LINE__);
    }
#endif

    return nReturnMaxScroll;

}   // end of CFontFusion::renderString


/*F************************************************************************************************/
/*!
returns (via passed int references) the max size of the string when rendered. This can be used to
ensure a texture large enough is created. The size this returns does NOT include any borders!!!
borders are added by the caller.


\Input      int nFont - Font Index
            uint16 *wszString - string to render (in UCS2 format)
            int nPointSize - Font Size
            int nMaxWidth - (OUT) Width of texture created (>= size of text field)
            int nMaxHeight - (OUT) Height of texture created (>= size of text field)
            int nBorderSize - [Not used!!! border sizes are added by caller!]
            int nX - starting x coordinate
            int nY - starting y coordinate
            int bWrap - does text wrap.
            int nBoxWidth - Width of bounding box (used for wrap)
            Alignment eAlignment - alignment types
            int nLineOffset - line to start rendering (for scrolling)

\Output     int nMaxWidth - (OUT) Width of texture created (>= size of text field)
            int nMaxHeight - (OUT) Height of texture created (>= size of text field)

\Notes
this function has SO MANY arguments! I'm sure there is a better way!

\Version
1.0        05/04/04 (DC) Comment creation.
*/
/************************************************************************************************F*/
void CFontFusion::calcRenderStringSize(int       nFont,
                                       uint16   *wszString,
                                       int       nPointSize,
                                       int      &nMaxWidth,     /* OUT Parameter! */
                                       int      &nMaxHeight,    /* OUT Parameter! */
                                       int       nBorderSize,
                                       int       nX,
                                       int       nY,
                                       int       bWrap,
                                       // $FR#598 CSB - Adding extra parameter for Font Style 33/36
                                       unsigned int nFontStyle,
                                       int       nBoxWidth,
                                       Alignment eAlignment,
                                       int       nLineOffset )
{
    APT_ASSERT(bInitialized);
    APT_ASSERT(nFont >= 0 && nFont < nFonts);
    APT_ASSERT(nPointSize > 0);
    nMaxWidth = 0;
    nMaxHeight = 0;
    const unsigned int MAX_STRING = 512;
    short anKerning[MAX_STRING];
    int nError = 0;
    bool bFontStyleChanged = true; // $fr834 - MCB - Apt20 optimization - Only recalculate and set the transform if the font changes

    // $FR#598 CSB - Checking the Last Font Style to decide whether to flush the Font Fusion Cache 34/36
    if (nLastFontStyle != nFontStyle)
    {
        FF_CM_Flush(pCache, &nError);
        nLastFontStyle = nFontStyle;
        bFontStyleChanged = true;
    }
    APT_ASSERT(!nError);

    if ((m_nLastFont != nFont) || (pScaler == NULL))
    {
        pScaler = FF_FM_SelectFont(pManager, static_cast<uint16_t>( fonts[nFont].nCode ), &nError);
        m_nLastFont = nFont;       // store current font for next operation.
    }
    APT_ASSERT(!nError);

    float HACK_FF_ScaleX = nPointSize < gAptFFScaleAtThisPtSize ? 1.0f : gAptFFScaleFactorX;
    float HACK_FF_ScaleY = nPointSize < gAptFFScaleAtThisPtSize ? 1.0f : gAptFFScaleFactorY;

    if(bFontStyleChanged)
    {
        T2K_TRANS_MATRIX trans;

        trans.t00 = (F16Dot16)(ONE16Dot16 * nPointSize * HACK_FF_ScaleX * gAptFFAdditionalScaleFactorX);
        // $FR#598 CSB - Setting up Matrix Translation for italic font in Font Fusion 35/36
        //double dItalicAngle = 15.0; // should be passed in, for the time being this is not changeable by the user
        if ((nFontStyle & AptFontStyle_Italic) == AptFontStyle_Italic)
        {
            trans.t01 = (F16Dot16)(ONE16Dot16 * sin(15.0f /*dItalicAngle*/) * nPointSize * HACK_FF_ScaleY);
        }
        else
        {
            trans.t01 = 0;
        }
        trans.t10 = 0;
        trans.t11 = (F16Dot16)(ONE16Dot16 * nPointSize * HACK_FF_ScaleY);
        T2K_NewTransformation(pScaler, 1, 72, 72, &trans, 1, &nError);
        if ( T2K_GetNumAxes( pScaler ) == 1 )
        {
            T2K_SetCoordinate(pScaler, 0, (F16Dot16)(0x10000*.7f));
        }
        APT_ASSERT(!nError);
        bFontStyleChanged = false;
    }

// $FR#598 CSB - So Far, this is NOT making the text bold. Not sure what Font Fusion means by emboldening, can be removed 36/36
    if((nFontStyle & AptFontStyle_Bold) == AptFontStyle_Bold)
    {
#ifdef ALGORITHMIC_STYLES

        tsiMemObject            *   mem      = NULL;
        InputStream             *   in       = NULL;
        sfntClass               *   font     = NULL;
        short                       fontType = FONT_TYPE_TT_OR_T2K;
        int                         errCode;
        T2K_AlgStyleDescriptor      style;

        style.StyleFunc        = tsi_SHAPET_BOLD_GLYPH;
        style.StyleMetricsFunc = tsi_SHAPET_BOLD_METRICS;
        style.params[0]        = 5L << 14;
        font                   = FF_New_sfntClass( mem, fontType, 0, in, NULL, &style, &errCode );

#endif
    }
    // we put the point size as a filter tag as a hack because the cache doesn't take into account size normally.
    // this lets us not have to flush the cache just because we rendered the same font in a different size
    APT_ASSERT(nPointSize < 256);
    FF_CM_SetFilter(pCache, static_cast<uint8_t>( nPointSize ), NULL, NULL);

    const uint16 *pLineOffset    = wszString;
    const uint16 *pNextLineOffset= wszString;
    int     nMaxLineLength = 0;
    int     nLineChars     = 0;
    int     nLineWidth     = 0;

    // base values are << 16!
    nMaxHeight  = pScaler->yAscender + 0x8000 +  pScaler->yLineGap + (nY << 16);

    while(pLineOffset[0] != '\0')
    {
        pNextLineOffset = AnalyzeLine(pLineOffset, bWrap!=0,nBoxWidth-(nBorderSize*2), &nLineChars, &nLineWidth, anKerning, nFont );
        if(nLineWidth > nMaxLineLength)
            nMaxLineLength = nLineWidth;
        pLineOffset = pNextLineOffset;
        nMaxHeight += (pScaler->yAscender - pScaler->yDescender) + pScaler->yLineGap;
    }

    nMaxHeight += pScaler->yDescender;
    nMaxHeight -= pScaler->yLineGap;
    nMaxHeight >>= 16;
    nMaxWidth   = nMaxLineLength + (nBorderSize*2) + 1 /* Extra Pixel so that border is not drawn on top */;
}

