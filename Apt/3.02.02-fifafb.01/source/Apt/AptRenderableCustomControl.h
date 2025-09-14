/*H************************************************************************************************/
/*!

    \file    Apt/AptRenderableCustomControl.h

    \brief         Simple description of main purpose of file.


        (c) 2005 Electronic Arts Inc.

    \version    1.0        06/17/07 ($senzee-lockless) First Version
    \version    1.01       09/06/07 (RB)  Added spFreeQueue
    \version    1.02       07/29/08 (RB)  Added MAX_FREE_CUSTCTRLS

*/
/************************************************************************************************H*/

#ifndef _Apt_AptRenderableCustomControl_h
#define _Apt_AptRenderableCustomControl_h

#include "AptRenderable.h"
#include "AptRefCounted.h"

struct AptRenderInfo;
class  AptSafeQueueFixed;

class AptRenderableCustomControl : public AptRefCounted<IAptRenderable>
{
public:
    APT_DECLARE_OBJECT_METRICS
    APT_RI_NEW_DELETE_OPERATORS

    AptAssetCustomControlZId mZid;

    AptRenderableCustomControl(AptAssetCustomControlZId zid = 0) : mZid(zid), mszType(0), mszTarget(0), mszProperties(0) { nRenderableCustomControls++; }

    void           Set(const char *szType, const char *szTarget, const char *szProperties);

	// FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change: Add RenderPrepare
    void           RenderPrepare();
	// FIFA_END
    void           Render(const AptRenderInfo &info, AptRenderableGeometry *pGeometry);
    const AptFile *GetFile() const { return 0; }


    static void    SetFreeQueue( AptSafeQueueFixed* pQueue ) { spFreeQueue = pQueue; }

    static const int32_t MAX_FREE_CUSTCTRLS = 512;

protected:

    ~AptRenderableCustomControl();

private:

    static AptSafeQueueFixed* spFreeQueue;

    char *mszType,
         *mszTarget,
         *mszProperties;

};

inline AptAssetCustomControlZId GetZID(const AptRenderableCustomControl *pCC)
{
    return pCC ? pCC->mZid : 0;
}

#endif // _Apt_AptRenderableCustomControl_h

