/*H************************************************************************************************/
/*!

    \File
        AptNativeFunction.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/07/04 (ONA) First Version

*/
/************************************************************************************************H*/

#ifndef __APTNATIVEFUNCTION_H__
#define __APTNATIVEFUNCTION_H__

#include "AptObject/AptObject.h"

typedef AptValue *(*AptNativeFunctionPointer)(AptValue *pThis, int nParams);
class AptNativeFunction : public AptObject
{
public:
    APT_VALUE_GC_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    AptNativeFunction(AptNativeFunctionPointer _pFunc) : AptObject(AptVFT_NativeFunction), mFunc(_pFunc)
    {
        // DC - Native Functions don't need to be put on the deferred vector.
        SetAllowDelayedDeletion(false);
    }

    virtual AptValue* Call(AptValue* pContext, int nParams)
    {
        return mFunc(pContext, nParams);
    }

protected:
    AptNativeFunctionPointer mFunc;

    virtual ~AptNativeFunction()
    {
        //  Do nothing...
    }
};

#endif  //  __APTNATIVEFUNCTION_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

