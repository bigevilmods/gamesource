/*H************************************************************************************************/
/*!

    \file    Apt/AptRenderableGeometry.cpp

    \brief         Simple description of main purpose of file.


        (c) 2005 Electronic Arts Inc.

    \version    1.0        06/17/07 ($senzee-lockless) First Version

*/
/************************************************************************************************H*/

#include "_Apt.h"
#include "AptDefine.h"
#include "AptRenderableString.h"
#include "AptCIH.h"

APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptRenderableString,   NULL);

// FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change: Add RenderPrepare
void AptRenderableString::RenderPrepare()
{
    APT_PROFILE_RENDER(AptRenderableString_RenderPrepare);
    APT_ASSERT(mString);
    {APT_PROFILE_AUXILIARY(pfnDrawString);
    gAptFuncs.pfnDrawString(mString, NULL);
    }
}

// FIFA_END
void AptRenderableString::Render(const AptRenderInfo &info, AptRenderableGeometry *pGeometry)
{
    APT_PROFILE_RENDER(AptRenderableString_Render);
    APT_ASSERT(mString);
    {APT_PROFILE_AUXILIARY(pfnDrawString);
    gAptFuncs.pfnDrawString(mString, &info);
    }
}

AptRenderableString::~AptRenderableString()
{
    AptAssetString str = mString;
    mString = 0;
    if (str && str != &AptCIH::sEmptyAssetString)
    {
        APT_PROFILE_AUXILIARY(pfnDeallocateString);
        gAptFuncs.pfnDeallocateString(str, APT_TEXTFIELD_FUPDATE);
    }
    nRenderableStrings--;
}

