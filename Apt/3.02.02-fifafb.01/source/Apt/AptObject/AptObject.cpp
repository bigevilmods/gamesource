/*H************************************************************************************************/
/*!

    \File
        AptObject.cpp

    \Description
        Implements AptObject methods.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/07/04 (ONA) First Version
        1.1        10/04/07 (ASK) Added Clone method, and cleaned uo some comments.
        1.2        04/29/08 (RB)  Fixed bug in Clone related to string copying
        1.3        10/27/09 (RB)  Moved body of new and delete operators into this file.
        1.4        11/02/09 (MZ)  Made Set/GetImplementedObjects() conditionally available.
*/
/************************************************************************************************H*/

#include "_Apt.h"
#include "AptObject\AptObject.h"
// psp-sn release build was giving compiler errors. so added next line. 0.17.00
#include "MainInline.h"


#if !defined(APT_ENABLE_INLINE)
#include "AptObject/AptObject.inl"
#endif

APT_DEFINE_OBJECT_METRICS(AptValueWithHash, AptValueGC);
APT_DEFINE_OBJECT_METRICS(AptPrototype, AptValueWithHash);
APT_DEFINE_OBJECT_METRICS(AptObject, AptValueWithHash);

/*F************************************************************************************************/
/*!
\Function    AptValueWithHash::GetNumAptValues

\Description
Enumerator. Gets the number of elements stored in the hash table.

\Input
None.

\Output
returns the size of the native hash table

\Notes
None.

\Version    1.0        10/28/09 (AK) Added the function.

*/
/************************************************************************************************F*/
unsigned int AptValueWithHash::GetNumAptValues()
{
    unsigned int uNumKeys = 0;

    for (AptHashItem *pInitItem = mNativeHash.GetFirstItem(); pInitItem; pInitItem = mNativeHash.GetNextItem(pInitItem))
    {
        uNumKeys++;
    }

    return uNumKeys;
}

/*F************************************************************************************************/
/*!
\Function    AptValueWithHash::GetKeyValuePair

\Description
Enumerator. Gets the value of keyIndex element in the hash table.

\Input
keyIndex - the index into the hash table at which you want to extract the (key, value) pair

\Output
keyName - the key name in (key, value)
AptValue* - a pointer to the value in (key, value)

\Notes
None.

\Version    1.0        10/28/09 (AK) Added the function.

*/
/************************************************************************************************F*/
const AptValue* AptValueWithHash::GetKeyValuePair(unsigned int uKeyIndex, AptNativeString& keyName)
{
    APT_ASSERT(uKeyIndex < GetNumAptValues());

    unsigned int uCurrentIndex = 0;
    const AptValue* pAptValue = NULL;
    keyName = "";

    for (AptHashItem *pInitItem = mNativeHash.GetFirstItem(); pInitItem; pInitItem = mNativeHash.GetNextItem(pInitItem))
    {
        if(uKeyIndex == uCurrentIndex)
        {
            keyName = pInitItem->Key;
            pAptValue = pInitItem->mValue;
            break;
        }
        uCurrentIndex++;
    }

    return pAptValue;
}


/*F************************************************************************************************/
/*!
    \Function    operator new

    \Description class-specific operator new

    \Input      size - size of object in bytes; passed by compiler

    \Returns    newly-created object allocated via garbage-collecting Dogma allocator

    \Version    1.0        10/27/09 (RB)  Moved from public header
*/
/************************************************************************************************F*/
void *AptPrototype::operator new(size_t size)
{
    return APT_GC_NEW(size);
}

/*F************************************************************************************************/
/*!
    \Function    operator delete

    \Description class-specific operator delete

    \Input      p    - pointer to object to delete; passed by compiler
    \Input      size - size of object in bytes; passed by compiler

    \Returns    void

    \Version    1.0        10/27/09 (RB)  Moved from public header
*/
/************************************************************************************************F*/
void AptPrototype::operator delete(void *p, size_t size)
{
    AptValueGC::VerifyAptValueGC();
    APT_GC_DELETE(p, size);
}


/*F************************************************************************************************/
/*!
    \Function    AptPrototype::DestroyGCPointers

    \Description
        Cleans up all references.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptPrototype::DestroyGCPointers()
{
    APT_DECSAFE(mp__constructor__);
    mp__constructor__ = NULL;
    AptValueWithHash::DestroyGCPointers();
}



/*F************************************************************************************************/
/*!
    \Function    AptPrototype::RegisterReferences

    \Description
        registers all references.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptPrototype::RegisterReferences()
{
    if( APT_REFERENCES_REGISTERED(this) )
        return;

    AptValueWithHash::RegisterReferences();

    APT_REGISTER_REFERENCE_SAFE( mp__constructor__ , "SuperConstructor", APT_REFREG_IS_APTVALUE);
    return;
}

/*F************************************************************************************************/
/*!
    \Function    AptPrototype::objectMemberLookup

    \Description
        $feature396 - (MCB) - Overloaded objectMemberSet to handle setting the super constructor

    \Input      pContext          - Not used here (just part of vitual function prototype
    \Input      pName             - Name of the variable to look for

    \Output     AptValue *        - AptValue for the name.

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptValue *  AptPrototype::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    if(pName->Equal("__constructor__"))
    {
        return GetSuperConstructor();
    }
    else
    {
        return AptValueWithHash::objectMemberLookup(pContext,pName);
    }
}

//
/*F************************************************************************************************/
/*!
    \Function    AptPrototype::objectMemberSet

    \Description
        $feature396 - (MCB) - Overloaded objectMemberSet to handle setting the super constructor

    \Input      pContext          - not used here directly
    \Input      pName             - name of the property to be set
    \Input      pValue            - AptValue to be set for the property

    \Output     bool              - true if property is set inside object

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
bool        AptPrototype::objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue)
{
    if(pName->Equal("__constructor__"))
    {
        SetSuperConstructor(pValue);
        return(true);
    }
    else
    {
        return AptValueWithHash::objectMemberSet(pContext, pName, pValue);
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptPrototype::SetSuperConstructor

    \Description
        Sets a new Super Constructor

    \Input      pNewSuperConstructor-  pointer to new Super Constructor

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptPrototype::SetSuperConstructor(AptValue * pNewSuperConstructor )
{
    AptValue * pOldSuperConstructor = mp__constructor__;
    mp__constructor__ = pNewSuperConstructor;

    APT_INCSAFE(pNewSuperConstructor);
    APT_DECSAFE(pOldSuperConstructor);
}

/*F************************************************************************************************/
/*!
    \Function    operator new

    \Description class-specific operator new

    \Input      size - size of object in bytes; passed by compiler

    \Returns    newly-created object allocated via garbage-collecting Dogma allocator

    \Version    1.0        10/27/09 (RB)  Moved from public header
*/
/************************************************************************************************F*/
void *AptObject::operator new(size_t size)
{
    return APT_GC_NEW(size);
}

/*F************************************************************************************************/
/*!
    \Function    operator delete

    \Description class-specific operator delete

    \Input      p    - pointer to object to delete; passed by compiler
    \Input      size - size of object in bytes; passed by compiler

    \Returns    void

    \Version    1.0        10/27/09 (RB)  Moved from public header
*/
/************************************************************************************************F*/
void AptObject::operator delete(void *p, size_t size)
{
    AptValueGC::VerifyAptValueGC();
    APT_GC_DELETE(p, size);
}

/*F************************************************************************************************/
/*!
    Returns the AptValue for the context and the member name.

    \Input      pContext          - The context passed in.
    \Input      pName             - The member name.

    \Output     AptValue *        - The AptValue of the member.
                                    gpUndefinedValue if the member is unknown.

    \Notes
        None.

    \Version
        1.0        03/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptValue *    AptObject::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    if (*pName == "registerClass")
    {
        // NATIVE_MEMBER_FUNCTION_DISPATCH(registerClass);
        return gpObjRegistrationFunc ;
    }
    return 0;
}



/*F************************************************************************************************/
/*!
    \Function    AptObject::RegisterReferences

    \Description
        Register all references.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void  AptObject::RegisterReferences()
{
    if( APT_REFERENCES_REGISTERED(this) )
        return;

    AptValueWithHash::RegisterReferences();

    return;
}


/*F************************************************************************************************/
/*!
    \Function    AptObject::DestroyGCPointers

    \Description
        Cleans up all references (in preparation for object deletion)

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void    AptObject::DestroyGCPointers()
{
    AptValueWithHash::DestroyGCPointers();
    return;
}

/*F************************************************************************************************/
/*!
    \Function    AptObject::SetImplementedObjects

    \Description
        This sets the currently implemented interface of this object

    \Input      paImplementedObjects-   Array of objects
    \Input      nNumObjects       -     number of elements in the passed array.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptObject::SetImplementedObjects(AptArray *paImplementedObjects, int nNumObjects)
{
#if APT_TRACK_IMPLEMENTED_INTERFACES
    AptNativeString sTmp("__INTERFACEs__");
    mNativeHash.Set(&sTmp, paImplementedObjects);
    mnImplementedObjects  = nNumObjects;
#endif
}

/*F************************************************************************************************/
/*!
    \Function    AptObject::GetImplementedObjects

    \Description
        Gets a list of the objects this implements.

    \Input      nOutNumObjects    - OUT - Length of array returned.

    \Output     AptValue **       - Array of objects

    \Notes
        The returned array should not be held onto for any length of time.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptArray* AptObject::GetImplementedObjects(int * nOutNumObjects) const
{
#if APT_TRACK_IMPLEMENTED_INTERFACES
    *nOutNumObjects = mnImplementedObjects;
    if(mnImplementedObjects > 0)
    {
        AptNativeString sTmp("__INTERFACES__");
        return mNativeHash.Lookup(&sTmp)->c_array();
    }
    else
#endif
    {
        return NULL;
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptObject::DoesImplementObject

    \Description
        looks through the list of implemented objects and returns true if the passed Object is found.

    \Input      pObject           - Object to look for.

    \Output     bool              - true if Object implements the interface

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya
    \Version    1.1        09/15/04 (DC) Fixed. It wasn't checking the __proto__
    \Version    1.2        09/15/04 (DC) Fixed. It wasn't checking the __proto__  chain.

*/
/************************************************************************************************F*/
bool AptObject::DoesImplementObject(AptValue * pPrototype) const
{
    // $bug726 - DC - 18.01 - Was not looking up the __proro__ chain.
    AptValue *pProto = mNativeHash.Get__Proto__();

    while(pProto)
    {
        if( pProto == pPrototype )
        {
            return true;
        }

        if(pProto->ContainsNativeHashVirtual())
        {
            pProto = pProto->GetNativeHashVirtual()->Get__Proto__();
        }
        else
        {
            break;
        }
    }

#if APT_TRACK_IMPLEMENTED_INTERFACES
    if(mnImplementedObjects > 0)
    {
        AptNativeString sTmp("__INTERFACES__");
        AptArray *pTmp = mNativeHash.Lookup(&sTmp)->c_array();
        APT_ASSERT(pTmp != NULL);
        for(uint32_t i = 0; i < mnImplementedObjects; i++)
        {
            if(pTmp->GetAt(i) == pPrototype)
            {
                return true;
            }
        }
    }
#endif

    return false;
}

/*F************************************************************************************************/
/*!
    \Function    AptObject::Clone

    \Description
        Makes a deep copy of the object. It actually creates new AptValues inside this object

        This function will go through the hashtable of the object and make a copy of each and
        every key-value pair into new object

    \Input      bRecursive        - if true goes recursive inside object for deep nested objects

    \Output     AptObject *       - New shallow copied object

    \Notes
        Key strings will not be copied, only reference count will be incremented.

    \Version    1.0        10/04/07 (ASK) First Version
    \Version    1.1        04/29/08 (RB)  Handle arrays, even if bRecursive==true.
                                          Copy string value instead of calling StringPool::GetFromPool
*/
/************************************************************************************************F*/
AptObject * AptObject::Clone(bool bRecursive)   const
{
    AptObject * pNewObject = new AptObject(AptVFT_Object);

    APT_ASSERT(pNewObject);

    // copy internal members
    pNewObject->SetHasClass(this->GetHasClass());
    pNewObject->setInMainInst(this->getInMainInst());

    // have to cast this pointer to AptObject * even though AptObject is derived from AptValueWithHash because function is defined as const
    AptNativeHash *pObjHash = ((AptObject *)this)->GetNativeHashVirtual();
    AptNativeHash *pNewObjHash = pNewObject->GetNativeHashVirtual();

    // copy proto and prototypes
    pNewObjHash->Set__Proto__(pObjHash->Get__Proto__());
    pNewObjHash->SetPrototype(pObjHash->GetPrototype());

    for (AptHashItem *pInitItem = pObjHash->GetFirstItem(); pInitItem; pInitItem = pObjHash->GetNextItem(pInitItem))
    {
        // Right now we only make a copy of int, float, bool, string.
        // $TODO - We need to refactor this.  Each subclass of AptObject (such as AptArray) and probably AptNativeHash could have
        //         its own implementation of Clone. We shouldn't be checking the type in a big list, we could just call pObj->Clone().

        AptValue * pOldValue = pInitItem->mValue;
        if (pOldValue->isInteger())
        {
            pNewObjHash->Set(&pInitItem->Key, AptInteger::Create(pOldValue->c_integer()->GetInt()));
        }
        else if (pOldValue->isFloat())
        {
            pNewObjHash->Set(&pInitItem->Key, AptFloat::Create(pOldValue->c_float()->GetFloat()));
        }
        else if (pOldValue->isBoolean())
        {
            pNewObjHash->Set(&pInitItem->Key, AptBoolean::Create(pOldValue->c_boolean()->GetBool()));
        }
        else if (pOldValue->isString())
        {
            // We must copy the contents of the string instead of calling StringPool::GetFromPool. Otherwise
            // asserts will occur in StringPool::Teardown at shutdown. The reason for the assert isn't immediately clear.
            AptString* pNewString = AptString::Create(pOldValue->c_string()->GetInternalString()->c_str());
            pNewObjHash->Set(&pInitItem->Key, pNewString);
        }
        else if (pOldValue->isArray())
        {
            AptArray* pArray = pOldValue->c_array();
            AptArray* pNewArray = new AptArray;
            int32_t nLen = static_cast<int32_t>( pArray->length() );
            for ( int32_t i=0; i<nLen; ++i )
            {
                AptValue* pVal = pArray->GetAt(i);
                if ( bRecursive )
                {
                    if (pVal->isInteger())
                    {
                        pNewArray->set(i, AptInteger::Create(pVal->c_integer()->GetInt()));
                    }
                    else if (pVal->isFloat())
                    {
                        pNewArray->set(i, AptFloat::Create(pVal->c_float()->GetFloat()));
                    }
                    else
                    {
                        APT_ASSERT( false && "Cannot copy other AptValue types from AptArray" );
                    }
                }
                else
                {
                    pNewArray->set(i, pVal);
                }
            }
            pNewObjHash->Set(&pInitItem->Key, pNewArray);
        }
        else if (pOldValue == gpUndefinedValue)
        {
            pNewObjHash->Set(&pInitItem->Key, gpUndefinedValue);
        }
        else
        {
            APT_ASSERT(false && "Not supporting any other AptValue types for clone operation");
        }
    }
    return pNewObject;
}

/*F************************************************************************************************/
/*!
    \Function    AptPrototype::GetClassName()

    \Description
        Returns the name of the class this AptPrototype represents.

    \Version    1.0        08/14/08 ($senzee) First Version
*/
/************************************************************************************************F*/
const char *AptPrototype::GetClassName() const
{
#if defined(APT_RECORD_SCOPE_INFO) && APT_RECORD_SCOPE_INFO != 0
    return mClassName.GetBuffer();
#else
    return 0;
#endif
}

/*F************************************************************************************************/
/*!
    \Function    AptPrototype::SetClassName()

    \Description
        Sets the name of the class this AptPrototype represents.

    \Version    1.0        08/14/08 ($senzee) First Version
*/
/************************************************************************************************F*/
void AptPrototype::SetClassName(const char *name)
{
#if defined(APT_RECORD_SCOPE_INFO)
    if (mClassName.IsEmpty() && name && *name)
        mClassName = name;
#endif
}

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

