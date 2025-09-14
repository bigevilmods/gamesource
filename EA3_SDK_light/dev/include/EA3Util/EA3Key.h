// (c) 1998-2010 Electronic Arts Inc.

#ifndef EA3KEY_H
#define EA3KEY_H

#include "EA3.h"
#include "EA3AnimUtil.h"
#ifndef EA3UTIL_NO_COPY
#define EA3UTIL_NO_COPY(CLASS) private: \
	  CLASS( CLASS const &src );\
	  CLASS const &operator=( CLASS const &src );
#endif

#ifdef EA_PLATFORM_WINDOWS
        #ifdef EA3_3D_EXPORTS
                #define EA3_3D_EXPORT __declspec(dllexport)
        #else
                #define EA3_3D_EXPORT __declspec(dllimport)
        #endif
#else
        #define EA3_3D_EXPORT
#endif
using namespace EA3Util;

class  EA3Key : public EA3Field
{
    public:
	void setTime( float t )		{ keySetTime( this, t ); }
	void setValue( float v )	{ keySetValue( this, v ); }
	float getTime() const		{ float f;keyGetTime( this,f );return f;}
	float getValue() const		{ float f;keyGetValue( this,f );return f;}

    protected:
	EA3Key(
	    const XbIdentifier &dt,
	    int nb,
	    EA3Block *con,
	    EA3Grammar *g,
	    int index,
	    const XbIdentifier &t
	    ,const XbString &nm) : EA3Field(dt,nb,con,g,index,t,nm)
	{};
  EA3UTIL_NO_COPY(EA3Key);
};

class  EA3ConstantKey : public EA3Key
{
    // ABSTRACT //
    // A constant key for use in EAFCurve.
    //
    // SUMMARY //
    // #include <EA3Key.h>
    //
    // DESCRIPTION //
    // A key has a time and value.
    //
    // SEE ALSO //
    // EAFCurve
    ///
    // END ABSTRACT //
    protected:
	EA3ConstantKey(
	    const XbIdentifier &dt,
	    int nb,
	    EA3Block *con,
	    EA3Grammar *g,
	    int index,
	    const XbIdentifier &t,
	    const XbString &nm) : EA3Key(dt,nb,con,g,index,t,nm)
	{};
  EA3UTIL_NO_COPY(EA3ConstantKey);
};

class  EA3LinearKey : public EA3Key
{
    // ABSTRACT //
    // A linear key for use in EAFCurve.
    //
    // SUMMARY //
    // #include <EA3Key.h>
    //
    // DESCRIPTION //
    // A key has a time and value, which is the same as a constant key.
    //
    // SEE ALSO //
    // EAFCurve
    ///
    // END ABSTRACT //
    protected:
	EA3LinearKey(
		const XbIdentifier &dt,
		int nb,
		EA3Block *con,
		EA3Grammar *g,
		int index,
		const XbIdentifier &t,
		const XbString &nm) : EA3Key(dt,nb,con,g,index,t,nm)
	{};
  EA3UTIL_NO_COPY(EA3LinearKey);
};

class  EA3Hermite1DKey : public EA3Key
{
    // ABSTRACT //
    // A hermite key for use in EAFCurves.
    //
    // SUMMARY //
    // #include <EA3Key.h>
    //
    // DESCRIPTION //
    // A key has a time and, value, left slope, and right slope.
    //
    // SEE ALSO //
    // EAFCurve
    ///

    public:

	void setLSlope( float slope )	{ keySetLSlope( this, slope ); }
	void setRSlope( float slope )	{ keySetRSlope( this, slope ); }

	void setSlope( float left, float right )
	{
	    keySetSlope( this, left, right );
	}

	float getLSlope() const		{float f; keyGetLSlope( this,f ); return f; }
	float getRSlope() const		{float f; keyGetRSlope( this,f );  return f;}

	void getSlope( float &left, float &right )
	{
	    keyGetSlope( this, left, right );
	}

    // END ABSTRACT //
    protected:
	EA3Hermite1DKey(
		const XbIdentifier &dt,
		int nb,
		EA3Block *con,
		EA3Grammar *g,
		int index,
		const XbIdentifier &t,
		const XbString &nm) : EA3Key(dt,nb,con,g,index,t,nm)
	{};
  EA3UTIL_NO_COPY(EA3Hermite1DKey);
};

class  EA3SoftHermiteKey : public EA3Hermite1DKey
{
    // ABSTRACT //
    // A Softimage hermite key for use in EAFCurves.
    //
    // SUMMARY //
    // #include <EA3Key.h>
    //
    // DESCRIPTION //
    // A key has a time and, value, left slope, and right slope.
    //
    // SEE ALSO //
    // EAFCurve
    ///
    // END ABSTRACT //
    protected:
	EA3SoftHermiteKey(
		const XbIdentifier &dt,
		int nb,
		EA3Block *con,
		EA3Grammar *g,
		int index,
		const XbIdentifier &t,
		const XbString &nm) : EA3Hermite1DKey(dt,nb,con,g,index,t,nm)
	{};
  EA3UTIL_NO_COPY(EA3SoftHermiteKey);
};

#endif // EA3KEY_H

