/******************************************************************************/
/*!
    Copyright 2012 Electronic Arts Inc.

    \file       AptRenderableImage.h

    \brief      Renderable wrapper for direct-loaded images
*/
/******************************************************************************/

/*** Include guard ************************************************************/

#ifndef _APT_APTRENDERABLEIMAGE_H
#define _APT_APTRENDERABLEIMAGE_H

/*** Includes *****************************************************************/

#include "Apt.h"
#include "AptRenderable.h"
#include "AptRefCounted.h"

/*** Forward Declarations *****************************************************/

struct AptRenderInfo;

/*** Macros *******************************************************************/

/*** Namespaces ***************************************************************/

/*** Interfaces ***************************************************************/

/*** Implementation ***********************************************************/

class AptRenderableImage : public AptRefCounted<IAptRenderable>
{
public:
    APT_DECLARE_OBJECT_METRICS
    APT_RI_NEW_DELETE_OPERATORS

    AptRenderableImage(AptAssetTexture texture, AptFilePtr file);

	// FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change: Add RenderPrepare
    void RenderPrepare();
	// FIFA_END
    void Render(const AptRenderInfo &info, AptRenderableGeometry *geometry);
    const AptFile *GetFile() const { return mFile.Get(); }

protected:
    virtual ~AptRenderableImage();

private:
    AptAssetTexture mTexture;
    AptFilePtr mFile;
};

/******************************************************************************/

#endif // _APT_APTRENDERABLEIMAGE_H


