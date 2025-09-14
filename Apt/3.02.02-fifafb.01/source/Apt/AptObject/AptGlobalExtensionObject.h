/*H************************************************************************************************/
/*!

    \File    AptGlobalExtensionObject.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0         04/02/04 (JSH) First Version
                1.1         01/28/05 (DUG) AptGlobalExtensionObject now derives from AptObject
*/
/************************************************************************************************H*/


#ifndef _AptGlobalExtensionObject_h
#define _AptGlobalExtensionObject_h

/*** Include files ********************************************************************************/

#include "AptValue/AptValue.h"
#include "_Apt.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/
// AptGlobalExtensionObject now derives from AptObject instead of AptValueGlobalWithHash

class AptGlobalExtensionObject : public AptObject
{
public:
    APT_VALUE_GC_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    AptGlobalExtensionObject() : AptObject(AptVFT_GlobalExtension, APT_OBJECTHASHSIZE)
    {
        setGCRoot(1);
    }


    APT_INLINE void Set(const AptNativeString * const pKey, AptValue * const pValue);
    APT_INLINE void UnSet(const AptNativeString * const pKey);
    APT_INLINE AptValue *Lookup(const AptNativeString * const pKey);


protected:
    APT_INLINE
    virtual ~AptGlobalExtensionObject()
    {
    }
};

/*** Variables ************************************************************************************/

extern AptGlobalExtensionObject *gpGlobalExtensionObject;

/*** Functions ************************************************************************************/

#endif // _AptGlobalExtensionObject_h
