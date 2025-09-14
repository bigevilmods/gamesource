/*H************************************************************************************************/
/*!

    \File    AptGlobalExtensionObject.inl

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        04/02/04 (JSH) First Version

*/
/************************************************************************************************H*/


#ifndef _AptGlobalExtensionObject_inl
#define _AptGlobalExtensionObject_inl

/*** Include files ********************************************************************************/

#include "_Apt.h"
#include "AptObject/AptGlobalExtensionObject.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

/*F************************************************************************************************/
/*!
    Insert your long description here.
    
    \Input pKey    -
    \Input pValue    -
    
    \Output
            None.
            
    \Notes
        None.
        
    \Version    1.0        04/03/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
void AptGlobalExtensionObject::Set(const AptNativeString * const pKey, AptValue * const pValue)
{
    mNativeHash.Set(pKey, pValue);
}

/*F************************************************************************************************/
/*!
Insert your long description here.

\Input pKey    -
\Input pValue    -

\Output
None.

\Notes
None.

\Version    1.0        03/03/05 (DUG) First Version

*/
/************************************************************************************************F*/
void AptGlobalExtensionObject::UnSet(const AptNativeString * const pKey)
{
    mNativeHash.Unset(pKey);
}
/*F************************************************************************************************/
/*!
    Insert your long description here.
    
    \Input pKey    -
    
    \Output
            AptValue     -
            
    \Notes
        None.
        
    \Version    1.0        04/03/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptValue *AptGlobalExtensionObject::Lookup(const AptNativeString * const pKey)
{
    return (mNativeHash.Lookup(pKey));
}

#endif // _AptGlobalExtensionObject_inl
