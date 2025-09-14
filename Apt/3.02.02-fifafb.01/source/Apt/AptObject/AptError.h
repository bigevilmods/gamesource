/*H************************************************************************************************/
/*!

    \File
        AptError.h

    \Description
        This file defines a the actionscript "Error" object handlers.

    \Notes

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version 1.0        07/19/05 (DC) Moved This class into it's own file.

*/
/************************************************************************************************H*/

#ifndef __APTERROR_H__
#define __APTERROR_H__

#include "AptObject/AptObject.h"

// Implements Native Error Class in Actionscript.
// (DC) Added for Flash Player 7 / AS 2.0 support. (Release 17.0)
class AptError : public AptObject
{
public:
    APT_VALUE_GC_NEW_DELETE_OPERATORS
        // DC - Added Metric defines
        APT_DECLARE_OBJECT_METRICS
        AptNativeString msMessage;
    AptNativeString msName;

    AptError() : AptObject(AptVFT_Error), msMessage("Error"), msName("Error")
    {
    }

    AptError(AptNativeString sMessage) : AptObject(AptVFT_Error), msMessage(sMessage), msName("Error")
    {
    }

    NATIVE_MEMBER_FUNCTION_DECL(toString);

    virtual AptValue *   objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;
    virtual bool         objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue);

    static void CleanNativeFunctions();

protected:
    virtual ~AptError() ;

};


#endif  //  __APTERROR_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

