/*H************************************************************************************************/
/*!

    \File
        AptStringObject.h

    \Description
        This file defines a wrapper around AptString that is based on AptObject.

    \Notes

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version 1.0        07/19/05 (DC) Moved This class into it's own file.

*/
/************************************************************************************************H*/

#ifndef __APTSTRINGOBJECT_H__
#define __APTSTRINGOBJECT_H__

#include "AptValue/AptValue.h"
#include "AptValue/AptString.h"
#include "AptObject/AptObject.h"

// DC - Added String Object as a wrapper around Apt String (based off of AptValue)
// This allows us to create script objects based off the string class.
class AptStringObject : public AptObject
{
public:
    APT_VALUE_GC_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    AptStringObject() : AptObject( AptVFT_StringObject), mpStringObject(NULL)
    {
    }
    AptStringObject(AptString * pString) : AptObject( AptVFT_StringObject), mpStringObject(NULL)
    {
        setString(pString);
    }

    ~AptStringObject();
    void setString(AptString*pString);

    virtual AptValue *  objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;
    AptString *mpStringObject;
};

#endif  //  __APTSTRINGOBJECT_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

