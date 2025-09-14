/*H************************************************************************************************/
/*!

    \file    Apt/AptRenderable.h

    \brief         Simple description of main purpose of file.


        (c) 2005 Electronic Arts Inc.

    \version    1.0        06/17/07 ($senzee-lockless) First Version

*/
/************************************************************************************************H*/

#ifndef _Apt_AptRenderable_h
#define _Apt_AptRenderable_h

struct AptRenderInfo;
struct AptFile;
class  AptRenderableGeometry;

// $senzee temp
extern int nRenderableGeometries,
           nRenderableCustomControls,
           nRenderableStrings;

class IAptRenderable
{
public:
    // not needed for this class as it is a ABC
    //APT_DECLARE_OBJECT_METRICS
    //APT_RI_NEW_DELETE_OPERATORS

	// FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change: Add RenderPrepare
    virtual void           RenderPrepare() = 0;
	// FIFA_END
    virtual void           Render(const AptRenderInfo &info, AptRenderableGeometry *pGeometry)       = 0;
    virtual const AptFile *GetFile()                                                           const = 0;
    virtual void           AddRef()                                                            const = 0;
    virtual void           Release()                                                           const = 0;

protected:

    virtual ~IAptRenderable() {}
};

#endif // _Apt_AptRenderable_h

