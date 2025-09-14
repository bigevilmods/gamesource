/*H************************************************************************************************/
/*!

    \File
        AptSound.h

    \Description
        This file defines a the actionscript "Sound" object handlers.

    \Notes

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version 1.0        07/19/05 (DC) Moved This class into it's own file.

*/
/************************************************************************************************H*/

#ifndef __APTSOUND_H__
#define __APTSOUND_H__

#include "AptObject/AptObject.h"

class AptSound : public AptObject
{
public:
#if defined(APT_USE_SOUND_OBJECT)
    APT_VALUE_GC_NEW_DELETE_OPERATORS
        // DC - Added Metric defines
        APT_DECLARE_OBJECT_METRICS

        const AptCharacter *pParentAnim;
    AptAssetSound zID;
    const char *  szName ;

    AptSound(AptCIH *pParent);

    NATIVE_MEMBER_FUNCTION_DECL(attachSound);
    NATIVE_MEMBER_FUNCTION_DECL(stop);
    NATIVE_MEMBER_FUNCTION_DECL(start);

    virtual AptValue *  objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;
#else
    AptSound(AptCIH *pParent) : AptObject(AptVFT_Sound)
    {
        APT_ASSERT(0 && "APT-Warning-Sound object cannot be used as it is compiled out, make sure to compile with APT_USE_SOUND_OBJECT defined");
    }
#endif

    static void CleanNativeFunctions();

protected:
    APT_INLINE
        virtual ~AptSound()
    {
        //  Do nothing...
    }

};


#endif  //  __APTSOUND_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

