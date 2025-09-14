/*H************************************************************************************************/
/*!

    \File    AptGlobalObject.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0         04/02/04 (JSH) First Version
    \Version    1.1         01/28/05 (DUG) AptGlobalObject now derives from AptObject
    \Version    1.2         11/11/09 (RB)  Added GetGperfIndex and Lookup(int)
*/
/************************************************************************************************H*/


#ifndef _AptGlobalObject_h
#define _AptGlobalObject_h

/*** Include files ********************************************************************************/

#include "AptValue/AptValue.h"
#include "_Apt.h"

/*** Defines **************************************************************************************/
#define APTGLOBAL_OBJECTHASHSIZE 11

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

// AptGlobal now derives from AptObject instead of AptValueGlobalWithHash

class AptGlobal : public AptObject
{
public:
    APT_VALUE_GC_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    AptGlobal() : AptObject(AptVFT_Global, APTGLOBAL_OBJECTHASHSIZE)
    {
        setGCRoot(1);
    }


    virtual AptValue *objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;
    virtual bool      objectMemberSet(   AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue);

    APT_INLINE AptValue *Lookup(const AptNativeString * const pKey) const;
    APT_INLINE void      Set(   const AptNativeString * const pKey, AptValue * const pValue);

    /// Lookup a member by name and return its gperf key from objects.gperf
    int GetGperfIndex( const AptNativeString* const name  ) const;

    /// Lookup a member when we already know its gperf index
    APT_INLINE AptValue * Lookup( int idx ) const;

    // These indices match objects.gperf. Unfortunately, support for _global members
    // is spread all over the Apt codebase. Eventually we want them to be encapsulated
    // by the AptGlobal class.
    static const int setIntervalIdx     = 50;
    static const int clearIntervalIdx   = 51;

protected:
    APT_INLINE
    virtual ~AptGlobal()
    {
    }

};

/*** Variables ************************************************************************************/

extern AptGlobal *gpGlobalGlobalObject;

/*** Functions ************************************************************************************/

#endif // _AptGlobalObject_h
