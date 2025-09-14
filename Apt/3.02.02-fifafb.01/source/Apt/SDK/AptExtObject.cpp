/*H************************************************************************************************/
/*!

    \File    AptExtObject.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        03/29/04 (JSH) First Version
    \Version    1.1        03/18/08 (RB)  Replaced mutex calls with AptUpdateLock() and AptUpdateUnlock().

*/
/************************************************************************************************H*/

/*!
\addtogroup g_referenceguide        Reference Guides
*/
//@{
/*!
\Module AptSDK      Apt Extension SDK
*/
//@}

/*** Include files ********************************************************************************/

#include "_Apt.h"
#include "MainInline.h"
#include "AptExtObject.h"

#include APT_INC_THREAD_H
#include APT_INC_THREAD_MUTEX_H

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Function Prototypes **************************************************************************/

/*** Variables ************************************************************************************/
APT_DEFINE_OBJECT_METRICS(AptExtObject, AptValueGC);

// Private variables


// Public variables

extern void AptUpdateLock();
extern void AptUpdateUnlock();

/*** Private Functions ****************************************************************************/

void * AptExtObject::operator new(size_t size)
{
    AptExtObject * retValue = 0;
    AptUpdateLock();
    retValue = (AptExtObject *) APT_GC_NEW( size );
    retValue->mnObjectSize = static_cast<uint32_t>(size); // Fill this in here. It should be safe.
    AptUpdateUnlock();
    return retValue;
}

/*F************************************************************************************************/
/*!
    Sets a function pointer by name in the extension object.  This is used by the
    APTEXT_FUNCTION_INIT macro to register the user's AptNativeFunction in the extension
    object's hash table.

    \Input *pKey        - the name of the function
    \Input *pFunction   - the AptNative function pointer

    \Output
            None.

    \Notes
        None.

    \Version    1.0        04/03/04 (JSH) First Version

*/
/************************************************************************************************F*/
void AptExtObject::SetFunction(const char *pKey, AptNativeFunction *pFunction)
{
    AptUpdateLock();
    AptNativeString pTmpStr(pKey);
    mpNativeHash->Set(&pTmpStr, (AptValue *)pFunction);
    AptUpdateUnlock();
}

/*** Public Functions *****************************************************************************/

/*F************************************************************************************************/
/*!
    Constructs an AptExtObject with the specified initial hash table size.  The hash table
    <i>can</i> grow as necessary, but it is recommended that you initialize it to the
    expected number of elements it will contain.  At the very least, it should be set to
    the number of C++ functions belonging to the object.

    \Input iNumMembers      - the initial size of the hash table

    \Output
        None.

    \Notes
        We explicitly do NOT initialize mnObjectSize, it was initialized in the new operator!

    \Version    1.0        04/03/04 (JSH) First Version

*/
/************************************************************************************************F*/
AptExtObject::AptExtObject(const int32_t iNumMembers)
 :  AptValueGC(AptVFT_Extension)
{
    //lint -esym(1402, AptExtObject::mnObjectSize)
    //lint -esym(1401, AptExtObject::mnObjectSize)
    APT_ASSERT(mnObjectSize != 0x00000000);
    APT_ASSERT(mnObjectSize != 0xcdcdcdcd); // Dogma initializes allocated memory to cd's.
                                            // This assert typically will fire if AptExtObject is using
                                            // multiple inheritance (bad) or if an AptExtObject Based
                                            // class defines it's own new delete operators (also bad).
                                            // Feel free to contact the Apt team for clarification on this.

    AptUpdateLock();
    mpNativeHash = new AptNativeHash(iNumMembers);

    // DC - don't let Ext objects be placed on the deferred vector. Not sure what thread they are deleted in.
    SetAllowDelayedDeletion(false);
    AptUpdateUnlock();
}

/*F************************************************************************************************/
/*!
    Destructs the extension object and its hash table.

    \Input None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/03/04 (JSH) First Version

*/
/************************************************************************************************F*/
AptExtObject::~AptExtObject()
{
    AptUpdateLock();
    delete mpNativeHash;
    AptUpdateUnlock();
}

/*F************************************************************************************************/
/*!
    Looks up a variable in the extension object's hash table

    \Input pContext     - the current context.  it is ignored.
    \Input pName        - the name of the variable to lookup

    \Output
            AptValue *  - a pointer to the variable, or NULL if it was not found

    \Notes
        None.

    \Version    1.0        04/03/04 (JSH) First Version

*/
/************************************************************************************************F*/
AptValue *AptExtObject::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    AptUpdateLock();
    AptValue *pVal = (mpNativeHash->Lookup(pName));
    AptUpdateUnlock();
    return pVal;
}

/*F************************************************************************************************/
/*!
    Sets a variable in the extension object's hash table if it does not already exist.

    \Input pContext     - the current context.  it is ignored.
    \Input pName        - the name of the variable to set
    \Input pValue       - the value to set

    \Output
            bool        - true, because we even if the variable is not set, it is the
                          desired behavior and no further action regarding the set
                          should be taken

    \Notes
        None.

    \Version    1.0        04/03/04 (JSH) First Version

*/
/************************************************************************************************F*/
bool AptExtObject::objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue)
{
    AptUpdateLock();
    mpNativeHash->SetIfNotExists(pName, pValue);
    AptUpdateUnlock();
    return true;
}



/*F************************************************************************************************/
/*!
\Function    AptValueWithHash::GetNativeHashVirtual

\Description
Returns the native hash of this object

\Input
None.

\Output
AptNativeHash *

\Notes
None.

\Version    1.0        09/10/04 (DC) First Version

*/
/************************************************************************************************F*/
AptNativeHash * AptExtObject::GetNativeHashVirtual()
{
    return  (mpNativeHash);
}


/*F************************************************************************************************/
/*!
\Function    AptValueWithHash::ContainsNativeHashVirtual

\Description
Returns the native hash of this object

\Input
None.

\Output
bool  -- true because we have one.

\Notes
None.

\Version    1.0        09/10/04 (DC) First Version

*/
/************************************************************************************************F*/
bool            AptExtObject::ContainsNativeHashVirtual() const
{
    return  (true);
}

/*F************************************************************************************************/
/*!
\Function    AptValueWithHash::RegisterReferences

\Description
This calls our parent version and then tells the native hash to register it's.

\Input
None.

\Output
None.

\Notes
None.

\Version    1.0        09/10/04 (DC) First Version

*/
/************************************************************************************************F*/
void            AptExtObject::RegisterReferences()
{
    AptUpdateLock();
    if(!APT_REFERENCES_REGISTERED(this))
    {
        // Right now This function is not defined up the inheritance tree.
        // So we aren't calling it. if at some point in time the function is implementes at the
        // base class level we need to uncomment this line.
        // AptValueGC::RegisterReferences();

        mpNativeHash->RegisterReferences(this);
    }
    AptUpdateUnlock();
}


/*F************************************************************************************************/
/*!
\Function    AptValueWithHash::DestroyGCPointers

\Description
This calls our parent version and then tells the native hash to destroy it's.

\Input
None.

\Output
None.

\Notes
None.

\Version    1.0        09/10/04 (DC) First Version

*/
/************************************************************************************************F*/
void            AptExtObject::DestroyGCPointers()
{
    AptUpdateLock();
    // Go up the Chain.
    AptValueGC::DestroyGCPointers();

    // Now now what we need to do.
    mpNativeHash->DestroyGCPointers();
    AptUpdateUnlock();
}


AptValue *              AptExtObject::Lookup( const AptNativeString * const pKey) const
{
    AptUpdateLock();
    AptValue *pVal = mpNativeHash->Lookup(pKey);
    AptUpdateUnlock();
    return pVal;
}

void                    AptExtObject::Set(const AptNativeString * const pKey, AptValue * const pValue)
{
    AptUpdateLock();
    mpNativeHash->SetIfNotExists(pKey, pValue);
    AptUpdateUnlock();
}

/*F************************************************************************************************/
/*!
    Apt C++ extension functions get two parameters passed to them.  pThis and nParams.  pThis
    is the function's context, which is usually the AptExtObject containing the function.
    nParams is the number of parameters passed to the function when it is called.  GetParam will
    return an AptValue pointer to the parameter at index iParam.

    \Input iParam       - the index of the parameter to lookup

    \Output
            AptValue *  - pointer to the parameter.

    \Notes
        GetParam retrieves the parameters from the Apt interpreter's stack.  If you call GetParam
        with an invalid index (one greater than nParams), you will get back invalid pointers
        and your code may crash.

    \Version    1.0        04/03/04 (JSH) First Version
    \Version    1.1        02/07/08 (JP)  Allowing undefined parameters

*/
/************************************************************************************************F*/
AptValue *AptExtObject::GetParam(const int32_t iParam)
{
    APT_ASSERT(iParam >= 0);
    AptUpdateAutoLock lock;
    // Undefined is a valid parameter
    return (gAptActionInterpreter.stackAt(iParam));
}

/*F************************************************************************************************/
/*!
    \Input None.

    \Output
            AptValue *  - a pointer to the global undefined value

    \Notes
        None.

    \Version    1.0        04/03/04 (JSH) First Version

*/
/************************************************************************************************F*/
AptValue *AptExtObject::GetUndefinedValue(void)
{
    return (gpUndefinedValue);
}

/*F************************************************************************************************/
/*!
    \Input None.

    \Output
            AptValue *  - a pointer to a new empty AptObject, with default hash table size

    \Notes
        None.

    \Version    1.0        04/03/04 (JSH) First Version

*/
/************************************************************************************************F*/
AptValue *AptExtObject::CreateNewAptObject(void)
{
    AptUpdateAutoLock lock;
    return (new AptObject(AptVFT_Object));
}

/*F************************************************************************************************/
/*!
    \Input iHashSize    - the initial hash table size of the new object

    \Output
        AptValue *  - a pointer to a new empty AptObject, with hash table size of iHashTable

    \Notes
        None.

    \Version    1.0        04/03/04 (JSH) First Version

*/
/************************************************************************************************F*/
AptValue *AptExtObject::CreateNewAptObject(const int32_t iHashSize)
{
    AptUpdateAutoLock lock;
    return (new AptObject(AptVFT_Object, iHashSize));
}

/*F************************************************************************************************/
/*!
    This is used by the APTEXT_FUNCTION_INIT macro to register the user's AptNativeFunction
    in the extension object's hash table.

    \Input pAptExtFnc       - a pointer to the C++ function called by the AptNativeFunction

    \Output
            AptNativeFunction *     - pointer a new AptNativeFunction object

    \Notes
        None.

    \Version    1.0        04/03/04 (JSH) First Version

*/
/************************************************************************************************F*/
AptNativeFunction *AptExtObject::CreateNewAptFunction(AptExtFunctionPtr pAptExtFnc)
{
    AptUpdateAutoLock lock;
    return (new AptNativeFunction((AptNativeFunctionPointer)pAptExtFnc));
}

/*F************************************************************************************************/
/*!
    Looks up a variable in the specified context, and returns a pointer to that
    object if it is found, or NULL.  The variable can be any AptValue object.

    \Input *pContext    - the scope to use when looking up the variable.
    \Input pVariable    - the name of the variable to lookup

    \Output
            AptValue *  - a pointer to the variable, or NULL if the variable could not
                          found or was undefined

    \Notes
        None.

    \Version    1.0        04/03/04 (JSH) First Version
    \Version    1.1        03/16/09 (RB)  Set getVariable flags to match the intent of this
                                          method, which is to get only global or member variables.

*/
/************************************************************************************************F*/
AptValue *AptExtObject::GetVariable(AptValue *pContext, const AptNativeString * const pVariable)
{
    AptUpdateAutoLock lock;
    AptValue *pValue = gAptActionInterpreter.getVariable(pContext, NULL, pVariable, true, false, true);
    if (pValue && pValue->getIsDefined())
    {
        return pValue;
    }
    return NULL;
}

/*F************************************************************************************************/
/*!
    Sets a variable in the specified context, and returns success or failure.

    \Input *pContext    - the scope to use when setting the variable.
    \Input pVariable    - the name of the variable to set
    \Input pValue       - the value to set

    \Output
            bool        - true if the variable was set.  false if it was not set.

    \Notes
        None.

    \Version    1.0        04/08/04 (JSH) First Version
    \Version    1.1        03/16/09 (RB)  Set setVariable flags to match the intent of this
                                          method, which is to set only global or member variables.
*/
/************************************************************************************************F*/
bool AptExtObject::SetVariable(AptValue *pContext, const AptNativeString * const pName, AptValue * const pValue)
{
    AptUpdateAutoLock lock;
    return (gAptActionInterpreter.setVariable(pContext, NULL, pName, pValue, true, false, true));
}

