// (c) 1998-2010 Electronic Arts Inc.

#ifndef EA3Skeleton_h
#define EA3Skeleton_h
#include "EA3.h"
#include "XbLinear.h"
#include "EA3names.h"
#include "EA3_Status.h"
#include "EA3Geometry.h"
#include "EA3SkelUtil.h"

#ifdef EA_PLATFORM_WINDOWS
        #ifdef EA3_3D_EXPORTS
                #define EA3_3D_EXPORT __declspec(dllexport)
        #else
                #define EA3_3D_EXPORT __declspec(dllimport)
        #endif
#else
        #define EA3_3D_EXPORT
#endif

class  EA3Bone;
class  EA3Xform;

using namespace EA3Util;

class  EA3Skeleton : public EA3Block {
  public:
    EA3_Status getNumChildren(int &num)
    {
	return skelGetNumChildren( this, num );
    }

    EA3_Status getChild(int ix, EA3Skeleton *&child)
    {
	return skelGetChild( this, ix, (EA3Block *&)child );
    }

    EA3_Status addChild(const XbString &name,EA3Skeleton *&child)
    {
	return skelAddChild( this, name, (EA3Block *&)child );
    }

    EA3_Status getParent(EA3Skeleton *&par)
    {
	return skelGetParent( this, (EA3Block *&)par );
    }

    EA3_Status getMatrix(XbMatrix &mat,const XbIdentifier &type)
    {
	return skelGetMatrix( this, mat, type );
    }

    EA3_Status setMatrix(const XbMatrix &mat,const XbIdentifier &type)
    {
	return skelSetMatrix( this, mat, type );
    }

    EA3_Status getBoneType(XbIdentifier &type)
    {
	return skelGetBoneType( this, type );
    }

    EA3_Status setBoneType(const XbIdentifier &type)
    {
	return skelSetBoneType( this, type );
    }

    EA3_Status getEuler(XbEuler &eul, const XbIdentifier &type)
    {
	return skelGetEuler( this, eul, type );
    }

    EA3_Status setEuler(const XbEuler &eul, const XbIdentifier &type)
    {
	return skelSetEuler( this, eul, type );
    }

    EA3_Status getTranslation(XbVec3f &trn, const XbIdentifier &type)
    {
	return skelGetTranslation( this, trn, type );
    }

    EA3_Status setTranslation(const XbVec3f &trn, const XbIdentifier &type)
    {
	return skelSetTranslation( this, trn, type );
    }

    EA3_Status getScale(XbVec3f &scl, const XbIdentifier &type)
    {
	return skelGetScale( this, scl, type );
    }

    EA3_Status setScale(const XbVec3f &scl, const XbIdentifier &type)
    {
	return skelSetScale( this, scl, type );
    }

    EA3_Status getGeometry(EA3Geometry *&geom)
    {
        EA3Block *b = NULL;
	EA3_Status s=skelGetGeometry( this, b );
        geom = (EA3Geometry*)b;
        return(s);
    }

 protected:
    EA3Skeleton(EA3Block *con,EA3Grammar *g,int ix,const XbIdentifier &t,const XbString &nm) : EA3Block(con,g,ix,t,nm) {};

  private:
    EA3_Status getXform( EA3Xform *&xfm, const XbIdentifier &type, int create )
    {
        EA3Block *x;
	return skelGetXform( this, x, type, create );
        xfm = (EA3Xform *)x;
    }

    EA3Bone *getBone()
    {
	return (EA3Bone*)skelGetBone( this );
    }

};

class  EA3Xform : public EA3Block {
  public:
    void getMatrix(XbMatrix &mat)	{ xformGetMatrix( this, mat ); }
    void setMatrix(const XbMatrix &mat)	{ xformSetMatrix( this, mat ); }
    void getEuler(XbEuler &eul)		{ xformGetEuler( this, eul ); }
    void setEuler(const XbEuler &eul)	{ xformSetEuler( this, eul ); }
    void getTranslation( XbVec3f &trn )	{ xformGetTranslation( this, trn ); }
    void setTranslation( const XbVec3f &trn )
    {
	xformSetTranslation( this, trn );
    }
    void getScale(XbVec3f &scl)		{ xformGetScale( this, scl ); }
    void setScale(const XbVec3f &scl)	{ xformSetScale( this, scl ); }

  protected:
    EA3Xform(
    	EA3Block *con,
	EA3Grammar *g,
	int ix,
	const XbIdentifier &t,
	const XbString &nm ) : EA3Block(con,g,ix,t,nm)
    {};
};

class  EA3Bone : public EA3Block {
  public:
    void getLocalMatrix(XbMatrix &mat) { boneGetLocalMatrix( this, mat ); }

    void setLocalMatrix(const XbMatrix &mat)
    {
	boneSetLocalMatrix( this, mat );
    }

    void getInitMatrix(XbMatrix &mat)	    { boneGetInitMatrix( this, mat ); }
    void setInitMatrix(const XbMatrix &mat) { boneSetInitMatrix( this, mat ); }
    void setBoneType(const XbIdentifier &t) { boneSetType( this, t ); }
    void getBoneType(XbIdentifier &t)	    { boneGetType( this, t ); }

    EA3Xform *getXform( const XbIdentifier &type, int create = 1 )
    {
	return (EA3Xform *)boneGetXform( this, type, create );
    }

  protected:
    EA3Bone(
    	EA3Block *con,
	EA3Grammar *g,
	int ix,
	const XbIdentifier &t,
	const XbString &nm) : EA3Block(con,g,ix,t,nm)
    {};
};
#endif
