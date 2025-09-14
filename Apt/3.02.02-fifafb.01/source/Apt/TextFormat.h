/*H************************************************************************************************/
/*!

    \File    TextFormat.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        07/20/05  (DC) Moved to it's own file
    \Version    1.1        07/09/09  (DG) Added nLeading to support ActionScript's TextFormat.leading
    \Version    1.2        09/28/09  (MZ) Added nTracking to support ActionScript's TextFormat.tracking

*/
/************************************************************************************************H*/


#ifndef _AptTextFormat_h
#define _AptTextFormat_h

/*** Include files ********************************************************************************/

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

struct TextFormat
{
    static const int UNDEFINED_LEADING_VALUE = 0x7fffff;
    static const int UNDEFINED_TRACKING_VALUE = 0x7fffff;
    APT_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS        // (LEO) $bug783

    AptNativeString    pFontName;
    float              fSize;
    int                nColor;
    AptStringAlignment eAlignment;
    unsigned int       nFontStyle;      // $FR#598 CSB - Adding extra data member eFontStyle for Font Style, Indent, LeftMargin, RightMargin 6/36
    int                nIndent;
    int                nLeftMargin;
    int                nRightMargin;
    int                nLeading;
    int                nTracking;

    // $FR#598 CSB - Adding extra parameter for Font Style, Indent, LeftMargin, RightMargin 7/36
    TextFormat(AptValue *pFName, float fFonstSize, unsigned int nFontColor, int isBold, int isItalic, int isUnderline, int nUrl, int nTarget,
               AptValue *pStringAlignment, int nLMargin, int nRMargin, int nIndentation, int nLeadingArgument, int nTrackingArgument);
    TextFormat(const TextFormat *pNewTextObj);

    void copyTextFormatObj(const TextFormat *pNewTextObj);
};

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // _AptTextFormat_h
