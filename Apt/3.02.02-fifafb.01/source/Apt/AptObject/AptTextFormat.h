/*H************************************************************************************************/
/*!

    \File
        AptTextFormat.h

    \Description
        This file defines a the actionscript "TextFormat" object handlers.

    \Notes

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version 1.0        07/19/05 (DC) Moved This class into it's own file.

*/
/************************************************************************************************H*/

#ifndef __APTTEXTFORMAT_H__
#define __APTTEXTFORMAT_H__

#include "AptObject/AptObject.h"
#include "Textformat.h"

class AptTextFormat : public AptObject
{
public:
    APT_VALUE_GC_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    AptTextFormat(AptValue *pFName, float fFonstSize, unsigned int nFontColor, int isBold, int isItalic, int isUnderline, int nUrl, int nTarget, AptValue *pStringAlignment, int nLMargin, int nRMargin, int nIndentation, int nLeading, int nTracking)
        : AptObject(AptVFT_TextFormat),
          mTextFormat(pFName, fFonstSize, nFontColor, isBold, isItalic, isUnderline, nUrl, nTarget, pStringAlignment, nLMargin, nRMargin, nIndentation, nLeading, nTracking)
    {
        // Do nothing...
    }
    AptTextFormat(const TextFormat *pNewTextObj)
        : AptObject(AptVFT_TextFormat),
          mTextFormat(pNewTextObj)
    {
        // Do nothing...
    }

    virtual AptValue *  objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;
    virtual bool        objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue);

    TextFormat * GetTextFormat()
    {
        return &mTextFormat;
    }

protected:
    APT_INLINE
    virtual ~AptTextFormat()
    {
        //  Do nothing...
    }

    TextFormat mTextFormat;
};

#endif  //  __APTTEXTFORMAT_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

