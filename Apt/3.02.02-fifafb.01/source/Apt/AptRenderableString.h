/*H************************************************************************************************/
/*!

    \file    Apt/AptRenderableString.h

    \brief         Simple description of main purpose of file.


        (c) 2005 Electronic Arts Inc.

    \version    1.0        06/17/07 ($senzee-lockless) First Version

*/
/************************************************************************************************H*/

#ifndef _Apt_AptRenderableString_h
#define _Apt_AptRenderableString_h

#include "AptRenderable.h"
#include "AptRefCounted.h"

struct AptRenderInfo;

class AptRenderableString : public AptRefCounted<IAptRenderable>
{
public:
    APT_DECLARE_OBJECT_METRICS
    APT_RI_NEW_DELETE_OPERATORS

    AptAssetString mString;

    AptRenderableString(const AptAssetString& str) : mString(str) { nRenderableStrings++; }

	// FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change: Add RenderPrepare
    void           RenderPrepare();
	// FIFA_END
    void           Render(const AptRenderInfo &info, AptRenderableGeometry *pGeometry);
    const AptFile *GetFile() const { return 0; }

protected:

    ~AptRenderableString();
};

inline AptAssetString GetZID(const AptRenderableString *pText)
{
    return pText ? pText->mString : 0;
}

#endif // _Apt_AptRenderableString_h

