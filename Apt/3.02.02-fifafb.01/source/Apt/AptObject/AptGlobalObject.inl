/*H************************************************************************************************/
/*!

    \File    AptGlobalObject.inl

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        04/02/04 (JSH) First Version

*/
/************************************************************************************************H*/


#ifndef _AptGlobalObject_inl
#define _AptGlobalObject_inl

/*** Include files ********************************************************************************/

#include "_Apt.h"
#include "AptObject\AptGlobalObject.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

void AptGlobal::Set(const AptNativeString * const pKey, AptValue * const pValue)
{
    mNativeHash.Set(pKey, pValue);
}

AptValue *AptGlobal::Lookup(const AptNativeString * const pKey) const
{
    return (mNativeHash.Lookup(pKey));
}

AptValue * AptGlobal::Lookup( int idx ) const
{
    // Handle built-in _global members.
    // Right now only setInterval and clearInterval are handled;
    // the other members have their code spread all over the Apt
    // codebase. Eventually we want to encapsulate all _global 
    // members here.
    switch(idx)
    {
        case AptGlobal::setIntervalIdx:
        {
            return gpCBsetInterval;
        }
        case AptGlobal::clearIntervalIdx:
        {
            return gpCBclearInterval;
        }
        default:
        {
            break;
        }
    }
    
    return NULL;
}

#endif // _AptGlobalObject_inl
