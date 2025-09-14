/*H************************************************************************************************/
/*!

    \File    AptGlobalObject.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        04/02/04 (JSH) First Version
    \Version    1.1        11/11/09 (RB)  Handle built-in global members in objectMemberLookup
*/
/************************************************************************************************H*/


/*** Include files ********************************************************************************/

#include "_Apt.h"
#include "AptObject/AptGlobalObject.h"
#include "AptObject/AptGlobalExtensionObject.h"
#include "MainInline.h"
#include "AptObjects.h"

#if !defined(APT_ENABLE_INLINE)
    #include "AptObject\AptGlobalObject.inl"
#endif

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Function Prototypes **************************************************************************/

/*** Variables ************************************************************************************/
APT_DEFINE_OBJECT_METRICS(AptGlobal, AptObject);

// Private variables

// Public variables

AptGlobal *gpGlobalGlobalObject = NULL;

/*** Private Functions ****************************************************************************/

/*** Public Functions *****************************************************************************/

/*F************************************************************************************************/
/*!
    objectMemberLookup - Looks up a global member

    \Input pContext    - not used
    \Input pName       - member name

    \Output
            AptValue   - member, if found; otherwise NULL

    \Notes
        None.

    \Version    1.0        04/03/04 (JSH) First Version
    \Version    1.0        11/11/09 (RB)  Handle built-in global members

*/
/************************************************************************************************F*/
AptValue *AptGlobal::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    APT_ASSERT(this == gpGlobalGlobalObject);

    AptValue* pValue = NULL;

    // look up built-in global members before looking in hashtable
    Objects* pObjects = ObjectIndex::in_word_set(pName->c_str(), pName->Size());
    if ( pObjects )
    {
        pValue = Lookup( pObjects->nIndex );
    }

    if ( !pValue )
    {
        pValue = gpGlobalExtensionObject->Lookup(pName);

        if (pValue && (pValue->isUndefined() == false))
        {
            // ONA 2004/4/9: Found something interesting return it
            return  (pValue);
        }
        pValue = gpGlobalGlobalObject->Lookup(pName);
    }
    return  (pValue);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input pContext    -
    \Input pName    -
    \Input pValue    -

    \Output
            bool     -

    \Notes
        None.

    \Version    1.0        04/03/04 (JSH) First Version

*/
/************************************************************************************************F*/
bool AptGlobal::objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue)
{
    APT_ASSERT(this == gpGlobalGlobalObject);

    if (gpGlobalExtensionObject->Lookup(pName))
    {
        //  The member exists in the extension object, we don't want to override it
        //  So we do only a lookup
        return  (true);
    }

    Set(pName, pValue);
    return  (true);
}


/*F************************************************************************************************/
/*!
    GetGperfIndex      - This function exists because we can only #include the generated header
                         AptObjects.h once, so we couldn't call ObjectIndex::in_word_set() from more
                         than one .cpp file. We need to lookup gperf index by name from both AptGlobal
                         and AptValue::findChild, therefore this function was created.

    \Input name        - name to lookup

    \Output
            int        - gperf index if name is in objects.gperf. -1 otherwise

    \Version    1.0    11/11/09 (RB) First Version
*/
/************************************************************************************************F*/
int AptGlobal::GetGperfIndex(const AptNativeString * const name) const
{
    int idx = -1;
    Objects* pObjects = ObjectIndex::in_word_set(name->c_str(), name->Size());
    if ( pObjects )
    {
        idx = pObjects->nIndex;
    }
    return idx;
}
