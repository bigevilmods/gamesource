#include "_Apt.h"
#include "AptTextFormat.h"
#include "AptTextFormatMembers.h"

#include "MainInline.h"
APT_DEFINE_OBJECT_METRICS(AptTextFormat, AptObject);

static const int AptTextFormat_align = 1;
static const int AptTextFormat_blockIndent = 2;
static const int AptTextFormat_bold = 3;
static const int AptTextFormat_bullet = 4;
static const int AptTextFormat_color = 5;
static const int AptTextFormat_font = 6;
static const int AptTextFormat_indent = 7;
static const int AptTextFormat_italic = 8;
static const int AptTextFormat_leading = 9;
static const int AptTextFormat_leftMargin = 10;
static const int AptTextFormat_rightMargin = 11;
static const int AptTextFormat_tabStops = 12;
static const int AptTextFormat_target = 13;
static const int AptTextFormat_size = 14;
static const int AptTextFormat_underline = 15;
static const int AptTextFormat_url = 16;
static const int AptTextFormat_tracking     = 17;


/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      pContext          -
    \Input      pName             -

    \Output     AptValue *        -

    \Notes
        None.

    \Version
        1.0        03/26/04 (ONA) First Version
        1.1        03/24/05 (LEO) $bug887 setTextFormat() with RGB(255, 255, 255) does not work in 18.06.
        1.2        07/09/09 (DG)  Added case to handle leading property.
        1.3        09/28/09 (MZ)  Added case to handle tracking property.
*/
/************************************************************************************************F*/
AptValue *    AptTextFormat::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    TextFormatMembers *pProp = pContext ? TextFormatMembersIndex::in_word_set(pName->c_str(), pName->Size()) : NULL;
    if (pProp)
    {
        switch(pProp->nIndex)
        {
            case(AptTextFormat_align):
            {
                AptString *pTemp = AptString::Create();
                    switch(mTextFormat.eAlignment)
                    {
                        case AptStringAlignment_Left:
                        {
                            pTemp->cpy(StringPool::GetString(SC_left));
                        }
                        break;
                        case AptStringAlignment_Right:

                        {
                            pTemp->cpy(StringPool::GetString(SC_right));
                        }
                        break;
                        case AptStringAlignment_Center:

                        {
                            pTemp->cpy(StringPool::GetString(SC_center));
                        }
                        break;
                        case AptStringAlignment_None:

                        {
                            return gpUndefinedValue;
                        }
                        break;
                        default:
                        {
                            return gpUndefinedValue;
                        }
                    }
                    return pTemp;
            }
            // $FR#598 CSB - Added case for handling bold text 15/36
            break;
            case AptTextFormat_bold:

            {
                if ((mTextFormat.nFontStyle & AptFontStyle_isBoldSet) != AptFontStyle_isBoldSet)
                {
                    return gpUndefinedValue;
                }
                bool bIsBold = false;
                if ((mTextFormat.nFontStyle & AptFontStyle_Bold) == AptFontStyle_Bold)
                {
                       bIsBold = true;
                }
                return AptBoolean::Create(bIsBold);
            }
            break;
            case AptTextFormat_color:

            {
                if(mTextFormat.nColor == -1)
                {
                    return gpUndefinedValue;
                }
                return AptInteger::Create(mTextFormat.nColor & 0xffffff);
            }
            break;
            case AptTextFormat_font:

            {
                // DC - If no font present return undefined, not an empty string.
                if(mTextFormat.pFontName.IsEmpty())
                    return gpUndefinedValue;

                AptString *pString = AptString::Create();
                pString->cpy(mTextFormat.pFontName);
                return pString;
            }
            // $FR#598 CSB - Added cases for italic text, indent, leftMargin, rightMargin 16/36
            break;
            case AptTextFormat_indent:

            {
                if (mTextFormat.nIndent == -1)
                {
                    return gpUndefinedValue;
                }
                return AptInteger::Create(mTextFormat.nIndent);
            }

            break;
            case AptTextFormat_italic:

            {
                if ((mTextFormat.nFontStyle & AptFontStyle_isItalicSet) != AptFontStyle_isItalicSet)
                {
                    return gpUndefinedValue;
                }
                bool bIsItalic = false;
                if ((mTextFormat.nFontStyle & AptFontStyle_Italic) == AptFontStyle_Italic)
                {
                    bIsItalic = true;
                }
                return AptBoolean::Create(bIsItalic);
            }

            break;
            case AptTextFormat_leftMargin:

            {
                if (mTextFormat.nLeftMargin == -1)
                {
                    return gpUndefinedValue;
                }
                return AptInteger::Create(mTextFormat.nLeftMargin);
            }

            break;
            case AptTextFormat_rightMargin:

            {
                if (mTextFormat.nRightMargin == -1)
                {
                    return gpUndefinedValue;
                }
                return AptInteger::Create(mTextFormat.nRightMargin);
            }
            break;
            case AptTextFormat_leading:

            {
                if (mTextFormat.nLeading == TextFormat::UNDEFINED_LEADING_VALUE)
                {
                    return gpUndefinedValue;
                }
                return AptInteger::Create(mTextFormat.nLeading);
            }
            break;
            case AptTextFormat_tracking:

            {
                if (mTextFormat.nTracking == TextFormat::UNDEFINED_TRACKING_VALUE)
                {
                    return gpUndefinedValue;
                }
                return AptInteger::Create(mTextFormat.nTracking);
            }
            break;
            case AptTextFormat_size:

            {
                if(mTextFormat.fSize == -1.f)
                {
                    return gpUndefinedValue;
                }
                return AptFloat::Create(mTextFormat.fSize);
            }
            // $FR#598 CSB - Added case for underline text 17/36
            break;
            case AptTextFormat_underline:

            {
                if ((mTextFormat.nFontStyle & AptFontStyle_isUnderlineSet) != AptFontStyle_isUnderlineSet)
                {
                    return gpUndefinedValue;
                }
                bool bIsUnderline = false;
                if ((mTextFormat.nFontStyle & AptFontStyle_Underline) == AptFontStyle_Underline)
                {
                    bIsUnderline = true;
                }
                return AptBoolean::Create(bIsUnderline);
            }
        }
    }

#if defined(EA_DEBUG)
        // EATech#104894
        APT_ASSERT(!pName->EqualNoCase("blockIndent") && "TextFormat: blockIndent is unimplemented property.\n");
        APT_ASSERT(!pName->EqualNoCase("bullet") && "TextFormat: bullet is unimplemented property.\n");
        APT_ASSERT(!pName->EqualNoCase("leading") && "TextFormat: leading is unimplemented property.\n");
        APT_ASSERT(!pName->EqualNoCase("tabStops") && "TextFormat: tabStops is unimplemented property.\n");
        APT_ASSERT(!pName->EqualNoCase("target") && "TextFormat: target is unimplemented property.\n");
        APT_ASSERT(!pName->EqualNoCase("url") && "TextFormat: url is unimplemented property.\n");
#endif

    return 0;
}


/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      pContext          -
    \Input      szName            -
    \Input      pValue            -

    \Output     bool              -

    \Notes
        None.

    \Version
        1.0        03/26/04 (ONA) First Version
        1.1        07/09/09 (DG)  Added case to handle leading property.
        1.2        09/28/09 (MZ)  Added case to handle tracking property.
*/
/************************************************************************************************F*/
bool    AptTextFormat::objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue)
{
    TextFormatMembers *pProp = pContext ? TextFormatMembersIndex::in_word_set(pName->c_str(), pName->Size()) : NULL;
    if (pProp)
    {
        switch(pProp->nIndex)
        {
            case(AptTextFormat_align) :
            {
                    AptNativeString szBuf;
                    pValue->toString(szBuf);
                    if(szBuf == "left" || szBuf == "true")
                    {
                        mTextFormat.eAlignment = AptStringAlignment_Left;
                    }
                    else if(szBuf == "center")
                    {
                        mTextFormat.eAlignment = AptStringAlignment_Center;
                    }
                    else if(szBuf == "right")
                    {
                        mTextFormat.eAlignment = AptStringAlignment_Right;
                    }
                    else if(szBuf == "false" || szBuf == "none")
                    {
                        mTextFormat.eAlignment = AptStringAlignment_None;
                    }
                    return  (true);
            }
            case(AptTextFormat_color) :
            {
                mTextFormat.nColor = (pValue->toInteger() & 0xffffff);
                return  (true);
            }
            case(AptTextFormat_font) :
            {
                pValue->toString(mTextFormat.pFontName);
                return  (true);
            }
            case(AptTextFormat_blockIndent) :
            {
                return (true);
            }
            case(AptTextFormat_bold) :
            {
                 // $FR#598 CSB - Added case for handling bold text 18/36
                AptNativeString szBuf;
                pValue->toString(szBuf);
                if (szBuf == "true")
                {
                    mTextFormat.nFontStyle |= AptFontStyle_isBoldSet;
                    mTextFormat.nFontStyle |= AptFontStyle_Bold;
                }

                if (szBuf == "false")
                {
                    mTextFormat.nFontStyle |= AptFontStyle_isBoldSet;
                    if ((mTextFormat.nFontStyle & AptFontStyle_Bold) == AptFontStyle_Bold)
                    {
                        mTextFormat.nFontStyle ^= AptFontStyle_Bold;
                    }
                }
                return (true);
            }
            case(AptTextFormat_bullet) :
            {
                return (true);
            }
            case(AptTextFormat_indent) :
            {
                // $FR#598 CSB - Added case for handling indent 19/36
                mTextFormat.nIndent = pValue->toInteger();
                return (true);
            }
            case(AptTextFormat_italic) :
            {
                // $FR#598 CSB - Added case for handling italic 20/36
                AptNativeString szBuf;
                pValue->toString(szBuf);

                if (szBuf == "true")
                {
                    mTextFormat.nFontStyle |= AptFontStyle_isItalicSet;
                    mTextFormat.nFontStyle |= AptFontStyle_Italic;
                }

                else if (szBuf == "false")
                {
                    mTextFormat.nFontStyle |= AptFontStyle_isItalicSet;
                    if ((mTextFormat.nFontStyle & AptFontStyle_Italic) == AptFontStyle_Italic)
                    {
                        mTextFormat.nFontStyle ^= AptFontStyle_Italic;
                    }
                }
                else                // $BUG853 DUG To handle passing of anything other than true and false, for instance undefined. Check testcase Undefined.fla
                {
                    mTextFormat.nFontStyle |= AptFontStyle_Italic;
                    mTextFormat.nFontStyle ^= AptFontStyle_Italic;
                }
                return (true);
            }
            case(AptTextFormat_leading) :
            {
                if (pValue->getIsDefined())
                {
                    mTextFormat.nLeading = pValue->toInteger();
                }
                else
                {
                    mTextFormat.nLeading = TextFormat::UNDEFINED_LEADING_VALUE;
                }
                return (true);
            }
            case(AptTextFormat_tracking) :
            {
                if (pValue->getIsDefined())
                {
                    mTextFormat.nTracking = pValue->toInteger();
                }
                else
                {
                    mTextFormat.nTracking = TextFormat::UNDEFINED_TRACKING_VALUE;
                }
                return (true);
            }
            case(AptTextFormat_leftMargin) :
            {
                // $FR#598 CSB - Added cases for handling leftMargin and rightMargin 21/36
                mTextFormat.nLeftMargin = pValue->toInteger();
                return (true);
            }
            case(AptTextFormat_rightMargin) :
            {
                mTextFormat.nRightMargin = pValue->toInteger();
                return (true);
            }
            case(AptTextFormat_tabStops) :
            case(AptTextFormat_target) :
            {
                return (true);
            }
            case(AptTextFormat_underline) :
            {
                // $FR#598 CSB - Added case for handling underline text 22/36
                AptNativeString szBuf;
                pValue->toString(szBuf);
                if (szBuf == "true")
                {
                    mTextFormat.nFontStyle |= AptFontStyle_isUnderlineSet;
                    mTextFormat.nFontStyle |= AptFontStyle_Underline;
                }

                else if (szBuf == "false")
                {
                    mTextFormat.nFontStyle |= AptFontStyle_isUnderlineSet;
                    if ((mTextFormat.nFontStyle & AptFontStyle_Underline) == AptFontStyle_Underline)
                    {
                        mTextFormat.nFontStyle ^= AptFontStyle_Underline;
                    }

                }
                else                // $BUG853 DUG To handle passing of anything other than true and false, for instance undefined. Check testcase Undefined.fla
                {
                    mTextFormat.nFontStyle |= AptFontStyle_isUnderlineSet;
                    mTextFormat.nFontStyle ^= AptFontStyle_Underline;
                }
                return (true);
            }
            case(AptTextFormat_url) :
            {
                return  (true);
            }
            case(AptTextFormat_size) :
            {
                mTextFormat.fSize = pValue->toFloat();
                return  (true);
            }
        }
    }

#if defined(EA_DEBUG)
    if (
        //(pName->EqualNoCase("align")) ||
        (pName->EqualNoCase("blockIndent")) ||
        //(pName->EqualNoCase("bold")) ||
        (pName->EqualNoCase("bullet")) ||
        //(pName->EqualNoCase("color")) ||
        //(pName->EqualNoCase("font")) ||
        //(pName->EqualNoCase("indent")) ||
        //(pName->EqualNoCase("italic")) ||
        (pName->EqualNoCase("leading")) ||
        //(pName->EqualNoCase("leftMargin")) ||
        //(pName->EqualNoCase("rightMargin")) ||
        (pName->EqualNoCase("tabStops")) ||
        (pName->EqualNoCase("target")) ||
        //(pName->EqualNoCase("size")) ||
        //(pName->EqualNoCase("underline")) ||
        (pName->EqualNoCase("url"))
    )
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "TextFormat: Incorrect case or unimplemented property '%s'.\n", pName->c_str());
        APT_ASSERT(0);
    }
#endif

    return  (false);
}

