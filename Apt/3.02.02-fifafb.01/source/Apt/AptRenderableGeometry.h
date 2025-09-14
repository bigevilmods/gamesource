/*H************************************************************************************************/
/*!

    \file    Apt/AptRenderableGeometry.h

    \brief         Simple description of main purpose of file.


        (c) 2005 Electronic Arts Inc.

    \version    1.0        06/17/07 ($senzee-lockless) First Version

*/
/************************************************************************************************H*/

#ifndef _Apt_AptRenderableGeometry_h
#define _Apt_AptRenderableGeometry_h

#include "AptRenderable.h"
#include "AptRefCounted.h"

struct AptRenderInfo;

class AptRenderableGeometry : public AptRefCounted<IAptRenderable>
{
public:
    APT_DECLARE_OBJECT_METRICS
    APT_RI_NEW_DELETE_OPERATORS

    AptAssetRenderingUnit  mRenderUnit;

    AptRenderableGeometry(AptAssetRenderingUnit ru, const AptFile *pFile) : mRenderUnit(ru), mpFile(pFile) { nRenderableGeometries++; }

	// FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change: Add RenderPrepare
    void           RenderPrepare();
	// FIFA_END
    void           Render(const AptRenderInfo &info, AptRenderableGeometry *pGeometry);
    const AptFile *GetFile() const { return mpFile; }

protected:

    ~AptRenderableGeometry();

private:

    const AptFile *mpFile;
};

inline AptAssetRenderingUnit GetZID(const AptRenderableGeometry *pGeo)
{
    return pGeo ? pGeo->mRenderUnit : 0;
}

#endif // _Apt_AptRenderableGeometry_h

