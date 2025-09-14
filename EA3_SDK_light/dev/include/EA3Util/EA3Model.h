// (c) 1998-2010 Electronic Arts Inc.

#ifndef EA3Model_h
#define EA3Model_h
#include "EA3.h"
#include "XbLinear.h"
#include "EA3names.h"
#include "EA3Skeleton.h"
#include "EA3Animation.h"
#include "EA3Geometry.h"
#include "EA3_Status.h"
#include "EA3ModelUtil.h"
using namespace EA3Util;
#ifdef EA_PLATFORM_WINDOWS
        #ifdef EA3_3D_EXPORTS
                #define EA3_3D_EXPORT __declspec(dllexport)
        #else
                #define EA3_3D_EXPORT __declspec(dllimport)
        #endif
#else
        #define EA3_3D_EXPORT
#endif

class  EA3Animation;
class  EA3Skeleton;

class  EA3Model : public EA3Block {
  public:

    EA3_Status addAnimation(const XbString &name,EA3Animation *&anim)
    {
	return modelAddAnimation( this, name, (EA3Block *&)anim );
    }

    EA3_Status getAnimation(const XbString &name,EA3Animation *&anim)
    {
	return modelGetAnimation( this, name, (EA3Block *&)anim );
    }

    EA3_Status getAnimation(int ix,EA3Animation *&anim)
    {
	return modelGetAnimation( this, ix, (EA3Block *&)anim );
    }

    EA3_Status getNumAnimations(int &num)
    {
	return modelGetNumAnimations( this, num );
    }

    EA3_Status addSkeleton(const XbString &name,EA3Skeleton *&skel)
    {
	return modelAddSkeleton( this, name, (EA3Block *&)skel );
    }

    EA3_Status getSkeleton(const XbString &name,EA3Skeleton *&skel)
    {
	return modelGetSkeleton( this, name, (EA3Block *&)skel );
    }

    EA3_Status getSkeleton(int ix,EA3Skeleton *&skel)
    {
	return modelGetSkeleton( this, ix, (EA3Block *&)skel );
    }

    EA3_Status getNumSkeletons(int &num)
    {
	return modelGetNumSkeletons( this, num );
    }

 protected:
    EA3Model(
    	EA3Block *con,
	EA3Grammar *g,
	int ix,
	const XbIdentifier &t,
	const XbString &nm) : EA3Block(con,g,ix,t,nm)
    {};
};
#endif

