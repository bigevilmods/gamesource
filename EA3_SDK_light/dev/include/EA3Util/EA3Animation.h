// (c) 1998-2010 Electronic Arts Inc.

#ifndef EA3Animation_h
#define EA3Animation_h
#include "EA3.h"
#include "XbLinear.h"
#include "EA3names.h"
#include "EA3_Status.h"
#include "EA3Skeleton.h"
#include "EA3AnimUtil.h"
#ifdef EA_PLATFORM_WINDOWS
  	#ifdef EA3_3D_EXPORTS
		#define EA3_3D_EXPORT __declspec(dllexport)
	#else
		#define EA3_3D_EXPORT __declspec(dllimport)
	#endif
#else
  	#define EA3_3D_EXPORT
#endif

class  EA3Channel;

using namespace EA3Util;

class  EA3Animation : public EA3Block {
  public:


    EA3_Status createDiscreteF3Channels(
    	const XbString &name,
	EA3Skeleton *skel,
	XbPArray<EA3Channel *> &clist,
	unsigned int &types )
    {
	return animCreateDiscreteF3Channels( this, name, skel, (XbPArray<EA3Block*> &)clist, types );
    }

    EA3_Status createDiscreteF3Channels(
    	const XbString &name,
	unsigned int what,
	XbPArray<EA3Channel *> &clist,
	unsigned int &types )
    {
	return animCreateDiscreteF3Channels( this, name, what, (XbPArray<EA3Block*> &)clist, types );
    }

    EA3_Status saveKey(
    	XbPArray<EA3Channel *> &clist,
	XbMatrix &mat,
	int frame )
    {
	return animSaveKey( this, (XbPArray<EA3Block*> &)clist, mat, frame );
    }

    EA3_Status saveKey(
    	XbPArray<EA3Channel *> &clist,
	XbMatrix &mat,
	int frame,
	const XbString &name )
    {
	return animSaveKey( this, (XbPArray<EA3Block*> &)clist, mat, frame, name );
    }

    EA3_Status getMatrix(XbPArray<EA3Channel *> &clist, int frame, XbMatrix &mat)
    {
	return animGetMatrix( this, (XbPArray<EA3Block*> &)clist, frame, mat );
    }

    EA3_Status getMatrix(
	XbPArray<EA3Channel *> &clist,
	int frame,
	XbMatrix &mat,
	XbVec3f &trn,
	XbEuler &eul,
	XbVec3f &scl )
    {
	return animGetMatrix( this, (XbPArray<EA3Block*> &)clist, frame, mat, trn, eul, scl );
    }

    EA3_Status getChannels( const XbString &name, XbPArray<EA3Channel *> &clist )
    {
	return animGetChannels( this, name, (XbPArray<EA3Block*> &)clist );
    }


 protected:
    EA3Animation(
    	EA3Block *con,
	EA3Grammar *g,
	int ix,
	const XbIdentifier &t,
	const XbString &nm) : EA3Block(con,g,ix,t,nm)
    {};
};

class  EA3Channel : public EA3Block {
  public:

    EA3_Status saveKey(const XbVec3f &val, int frame)
    {
	return channelSaveKey( this, val, frame );
    }

    EA3_Status saveKey(const XbEuler &val, int frame)
    {
	return channelSaveKey( this, val, frame );
    }

    EA3_Status getNumFrames(int &num)
    {
	return channelGetNumFrames( this, num );
    }

 protected:
    EA3Channel(
    	EA3Block *con,
	EA3Grammar *g,
	int ix,
	const XbIdentifier &t,
	const XbString &nm ) : EA3Block(con,g,ix,t,nm)
    {};
};
#endif
