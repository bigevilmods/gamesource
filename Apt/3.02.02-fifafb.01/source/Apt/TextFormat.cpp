/*H************************************************************************************************/
/*!

    \File    TextFormat.cpp

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


/*** Include files ********************************************************************************/
#include "_Apt.h"
#include "TextFormat.h"
#include "Maininline.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

    // $FR#598 CSB - Adding extra parameter for Font Style, Indent, LeftMargin, RightMargin 7/36
TextFormat::TextFormat(AptValue *pFName, float fFonstSize, unsigned int nFontColor, int isBold, int isItalic, int isUnderline, int nUrl, int nTarget,
                       AptValue *pStringAlignment, int nLMargin, int nRMargin, int nIndentation, int nLeadingArgument, int nTrackingArgument)
{
    fSize = fFonstSize;
    nColor = nFontColor;

    // $FR#598 CSB - Setting Font Style, Indent, LeftMargin, RightMargin 8/36
    nFontStyle = AptFontStyle_None;

    if (isBold == 0)
    {
        nFontStyle |= AptFontStyle_isBoldSet;
    }
    if (isBold == 1)
    {
        nFontStyle |= (AptFontStyle_Bold | AptFontStyle_isBoldSet);
    }

    if (isItalic == 0)
    {
        nFontStyle |= AptFontStyle_isItalicSet;
    }
    if (isItalic == 1)
    {
        nFontStyle |= (AptFontStyle_Italic | AptFontStyle_isItalicSet);
    }

    if (isUnderline == 0)
    {
        nFontStyle |= AptFontStyle_isUnderlineSet;
    }
    if (isUnderline == 1)
    {
        nFontStyle |= (AptFontStyle_Underline | AptFontStyle_isUnderlineSet);
    }

    nIndent      = nIndentation;
    nLeftMargin  = nLMargin;
    nRightMargin = nRMargin;
    nLeading     = nLeadingArgument;
    nTracking    = nTrackingArgument;

    if(!pFName->isUndefined())
    {
        pFName->toString(pFontName);
    }

    if(!pStringAlignment->isUndefined())
    {
        AptNativeString szBuf;
        pStringAlignment->toString(szBuf);
        if(szBuf == "left" || szBuf == "true")
        {
            eAlignment = AptStringAlignment_Left;
        }
        else if(szBuf == "center")
        {
            eAlignment = AptStringAlignment_Center;
        }
        else if(szBuf == "right")
        {
            eAlignment = AptStringAlignment_Right;
        }
        // DC - Bug 556. Added Default clause here. Else the alignment won't get init'd
        else // if(szBuf == "false" || szBuf == "none" || szBuf == "Something else!")
        {
            eAlignment = AptStringAlignment_None;
        }
    }
    else
    {
        eAlignment = AptStringAlignment_None;
    }

}

TextFormat::TextFormat(const TextFormat *pNewTextObj) :
            pFontName(""),
            fSize(-1.0f),
            nColor(-1),
            eAlignment(AptStringAlignment_None),
            nFontStyle(AptFontStyle_None),
            nIndent(-1),
            nLeftMargin(-1),
            nRightMargin(-1),
            nLeading(UNDEFINED_LEADING_VALUE),
            nTracking(UNDEFINED_TRACKING_VALUE)
{
    copyTextFormatObj(pNewTextObj);
}

void TextFormat::copyTextFormatObj(const TextFormat *pNewTextObj)
{
    // $bug860 Updated the text format for this bug
    if(pNewTextObj->eAlignment != AptStringAlignment_None)
    {
        eAlignment = pNewTextObj->eAlignment;
    }

    if(pNewTextObj->nColor != -1)
    {
        nColor = pNewTextObj->nColor;
    }

    if(pNewTextObj->pFontName != "")
    {
        pFontName = pNewTextObj->pFontName;
    }

    if(pNewTextObj->fSize != -1)
    {
        fSize= pNewTextObj->fSize;
    }

    // $FR#598 CSB - Setting Font Style, Indent, LeftMargin, RightMargin 9/36
    if(pNewTextObj->nFontStyle != AptFontStyle_None)
    {
        nFontStyle = pNewTextObj->nFontStyle;
    }

    if(pNewTextObj->nIndent != -1)
    {
        nIndent = pNewTextObj->nIndent;
    }

    if(pNewTextObj->nLeftMargin != -1)
    {
        nLeftMargin = pNewTextObj->nLeftMargin;
    }

    if(pNewTextObj->nRightMargin != -1)
    {
        nRightMargin = pNewTextObj->nRightMargin;
    }

    if (pNewTextObj->nLeading != UNDEFINED_LEADING_VALUE)
    {
        nLeading = pNewTextObj->nLeading;
    }

    if (pNewTextObj->nTracking != UNDEFINED_TRACKING_VALUE)
    {
        nTracking = pNewTextObj->nTracking;
    }
}

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/
