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
#include "AptRenderableGeometry.h"

APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptRenderableGeometry,   NULL);

// FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change: Add RenderPrepare
void AptRenderableGeometry::RenderPrepare()
{
}

// FIFA_END
void AptRenderableGeometry::Render(const AptRenderInfo &info, AptRenderableGeometry *pGeometry)
{
    APT_PROFILE_RENDER(AptRenderableGeometry_Render);
    APT_ASSERT(mRenderUnit);
    {APT_PROFILE_AUXILIARY(pfnDrawRenderingUnit);
    gAptFuncs.pfnDrawRenderingUnit(mRenderUnit, &info);
    }
}

AptRenderableGeometry::~AptRenderableGeometry()
{
    APT_PROFILE_AUXILIARY(pfnFreeRenderingUnit);
    AptAssetRenderingUnit ru = mRenderUnit;
    mRenderUnit = 0;
    gAptFuncs.pfnFreeRenderingUnit(ru);
    nRenderableGeometries--;
    mpFile = NULL;
}

