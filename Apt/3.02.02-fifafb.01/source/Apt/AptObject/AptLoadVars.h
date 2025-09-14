/*H************************************************************************************************/
/*!

    \File
        AptLoadVars.h

    \Description
        This file defines a the actionscript "LoadVars" object handlers.

    \Notes

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version 1.0        07/19/05 (DC) Moved This class into it's own file.

*/
/************************************************************************************************H*/

#ifndef __APTLOADVARS_H__
#define __APTLOADVARS_H__

#include "AptObject/AptObject.h"

/// This structure represents loadVars object in Flash. it is derived from AptObject, so that it can
/// store variables in its hash table. This is not specific to any movieClip or level.
/// The movieclip that creates this object owns this object.
// added 12/19/03
class AptLoadVars : public AptObject
{
public:
#if APT_USE_LOADVARS_OBJECT
    APT_VALUE_GC_NEW_DELETE_OPERATORS
        // DC - Added Metric defines
        APT_DECLARE_OBJECT_METRICS
        int iIsLoaded ;

    AptLoadVars() : AptObject(AptVFT_LoadVars)
    {
        iIsLoaded = 0 ;
    }
    NATIVE_MEMBER_FUNCTION_DECL(load);
    NATIVE_MEMBER_FUNCTION_DECL(send) ;
    NATIVE_MEMBER_FUNCTION_DECL(sendAndLoad) ;
    NATIVE_MEMBER_FUNCTION_DECL(getBytesTotal) ;
    NATIVE_MEMBER_FUNCTION_DECL(getBytesLoaded) ;
    NATIVE_MEMBER_FUNCTION_DECL(toString) ;

    virtual AptValue *  objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;
#else
    AptLoadVars() : AptObject(AptVFT_LoadVars)
    {
        APT_ASSERT(0 && "APT-Warning-LoadVars object cannot be used as it is compiled out, make sure to compile with APT_USE_LOADVARS_OBJECT defined as 1");
    }
#endif

    static void CleanNativeFunctions();

protected:
    virtual ~AptLoadVars() ;

};


#endif  //  __APTLOADVARS_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

