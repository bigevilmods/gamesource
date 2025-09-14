/*H************************************************************************************************/
/*!

    \File
        AptStage.h

    \Description
        This file defines a the actionscript "Stage" object handlers.

    \Notes

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version 1.0        07/19/05 (DC) Moved This class into it's own file.

*/
/************************************************************************************************H*/

#ifndef __APTSTAGE_H__
#define __APTSTAGE_H__

#include "AptObject/AptObject.h"

/**************************************************************************************************/
//(LEO) 06.02.04 : new AptStage object. FR312
/**************************************************************************************************/
class AptStage : public AptObject
{
public:


#if defined (APT_USE_STAGE_OBJECT)

	APT_VALUE_GC_NEW_DELETE_OPERATORS
	// DC - Added Metric defines
	APT_DECLARE_OBJECT_METRICS

        AptStage() : AptObject(AptVFT_Stage, APT_OBJECTHASHSIZE)
    {
        setGCRoot(1) ;
    }

    NATIVE_MEMBER_FUNCTION_DECL(addListener);
    NATIVE_MEMBER_FUNCTION_DECL(removeListener);
    virtual AptValue *  objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;
    static void CleanNativeFunctions();
#else
    AptStage() : AptObject(AptVFT_Stage, APT_OBJECTHASHSIZE)
    {
        APT_ASSERT(0 && "APT-Warning-Stage object cannot be used as it is compiled out, make sure to compile with APT_USE_STAGE_OBJECT defined in Aptdefine.h") ;
    }

	static void *operator new(size_t size)           { APT_ASSERT(0 && "APT-Warning-Stage object cannot be used as it is compiled out, make sure to compile with APT_USE_STAGE_OBJECT defined in Aptdefine.h") ; return (void*)0x00000000 ; }
	static void operator delete(void *p, size_t size){ APT_ASSERT(0 && "APT-Warning-Stage object cannot be used as it is compiled out, make sure to compile with APT_USE_STAGE_OBJECT defined in Aptdefine.h") ;  }
	static void *operator new[](size_t)              { APT_ASSERT(0 && "APT-Warning-Stage object cannot be used as it is compiled out, make sure to compile with APT_USE_STAGE_OBJECT defined in Aptdefine.h") ; return (void*)0x00000000 ; }
	static void operator delete[](void *)            { APT_ASSERT(0 && "APT-Warning-Stage object cannot be used as it is compiled out, make sure to compile with APT_USE_STAGE_OBJECT defined in Aptdefine.h") ; }


#endif

protected:
    APT_INLINE
        virtual ~AptStage()
    {
        //  Do nothing...
    }

private:

};

#endif  //  __APTSTAGE_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

