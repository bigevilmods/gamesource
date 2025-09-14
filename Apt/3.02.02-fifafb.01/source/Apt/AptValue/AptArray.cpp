/*H************************************************************************************************/
/*!

    \File   AptArray.cpp

    \Description
        Implements AptArray functions

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        01/04/05 (MCB) First Version
    \Version    1.1        08/14/08 ($senzee) Added find functionality in support of AptUtil functions.
    \Version    1.2        10/01/08 (ASK) Added objectFindComparator function in support of AptUtil functions.
    \Version    1.3        02/03/09 (RB) Fixing bugs in propery comparison; EATech 117137.
    \Version    1.4        10/27/09 (RB)  Moved body of new and delete operators into this file.
*/
/************************************************************************************************H*/

#include "_Apt.h"
#include "AptArrayMembers.h"
#include "AptValueHelper.h"

#include "MainInline.h"


static const int AptArray_length = 1;
#if APT_USE_ARRAY_METHODS
static const int AptArray_concat = 2;
static const int AptArray_join = 3;
static const int AptArray_pop = 4;
static const int AptArray_push = 5;
static const int AptArray_shift = 6;
static const int AptArray_unshift = 7;
static const int AptArray_reverse = 8;
static const int AptArray_sort = 9;
static const int AptArray_splice = 10;
static const int AptArray_slice = 11;
static const int AptArray_sortOn = 12;
static const int AptArray_toString = 13;
#endif // APT_USE_ARRAY_METHODS

#define APTARRAY_START_SIZE 8

APT_DEFINE_OBJECT_METRICS(AptArray, AptObject);

/*F************************************************************************************************/
/*!
    \Function    operator new

    \Description class-specific operator new

    \Input      size - size of object in bytes; passed by compiler

    \Returns    newly-created object allocated via garbage-collecting Dogma allocator

    \Version    1.0        10/27/09 (RB)  Moved from public header
*/
/************************************************************************************************F*/
void *AptArray::operator new(size_t size)
{
    return APT_GC_NEW(size);
}

/******************************************************************************/
/*!
    \brief  Placement new operator

    \param  t       Size
    \param  p       Location

    \return         - An AptArray
*/
/******************************************************************************/
void * AptArray::operator new(size_t t, void * p)
{
    return p;
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
void AptArray::operator delete(void *p, size_t size)
{
    AptValueGC::VerifyAptValueGC();
    APT_GC_DELETE(p, size);
}


AptArray::AptArray(int nElements, AptValue ** pAptValue)         // DUG
:
AptObject(AptVFT_Array, nElements)
{
    mnCapacity = 0;
    mpValues = NULL;
    mnLength = nElements;

    _reserve(nElements);

    for (int i = 0; i < mnLength; i++)
    {
        SetAt(i, pAptValue[i]);
    }

}

AptArray::AptArray()
:
AptObject(AptVFT_Array)
{
    mnCapacity = 0;
    mpValues = NULL;
    mnLength = 0;
}

// DC - Warning, NEVER DEcrement GC'd objects in the destructor, That is for the DestroyGCPointers call
AptArray::~AptArray()
{
    APT_ASSERT(mpValues   == NULL && "DestroyGCPointers Was not called on this object before deletion!");
    APT_ASSERT(mnLength   == 0    && "DestroyGCPointers Was not called on this object before deletion!");
    APT_ASSERT(mnCapacity == 0    && "DestroyGCPointers Was not called on this object before deletion!");
}

/*F************************************************************************************************/
/*!
    Releases the AptNativeScript functions.

    \Input      None.

    \Output     None.

    \Notes
        This function needs to be called at least during AptShutdown.

    \Version
        1.0        04/05/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptArray::CleanNativeFunctions()
{
#if APT_USE_ARRAY_METHODS
    NATIVE_MEMBER_FUNCTION_DESTROY(concat);
    NATIVE_MEMBER_FUNCTION_DESTROY(join);
    NATIVE_MEMBER_FUNCTION_DESTROY(pop);
    NATIVE_MEMBER_FUNCTION_DESTROY(push);
    NATIVE_MEMBER_FUNCTION_DESTROY(shift);
    NATIVE_MEMBER_FUNCTION_DESTROY(unshift);
    NATIVE_MEMBER_FUNCTION_DESTROY(reverse);
    NATIVE_MEMBER_FUNCTION_DESTROY(sort);
    NATIVE_MEMBER_FUNCTION_DESTROY(splice);
    NATIVE_MEMBER_FUNCTION_DESTROY(slice);
    NATIVE_MEMBER_FUNCTION_DESTROY(sortOn);
#endif
}

/*F************************************************************************************************/
/*!
    Release all the contained GC pointers.

    \Input      None.

    \Output     None.

    \Notes
        This function is called when the object needs to be released and just before the destruction.
        There is a separation between GC objects and non-GC objects because the Garbage Collector
        will take care of the GC objects. We don't want to destruct them also in the destructor of this class.

    \Version
        1.0        02/19/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptArray::DestroyGCPointers()
{
    AptObject::DestroyGCPointers();
    APT_ASSERT(mpValues != NULL || mnLength == 0 );
    APT_ASSERT(mnLength <= mnCapacity);

    for (int i = 0; i < mnLength; i++)
    {
        APT_ASSERT(mpValues != NULL);
        AptValue * pValue = mpValues[i];
        if(pValue)
        {
            APT_DEC(pValue);
            mpValues[i] = NULL;
        }
    }

    if(mpValues)
        APT_FREE_ARRAY(mpValues, AptValue*, mnCapacity);

    mpValues   = NULL;
    mnLength   = 0;
    mnCapacity = 0;
    return;
}


/*F************************************************************************************************/
/*!
    \Function    GetAt

    \Description
        Returns the value at a specified index

    \Input      nIndex            -

    \Output     AptValue *        -

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptValue * AptArray::GetAt(const int32_t nIndex) const
{
    APT_ASSERT(nIndex < mnLength);
    if(nIndex>= mnLength)
        return gpUndefinedValue;

    APT_ASSERT(mpValues != NULL);
    return  mpValues[nIndex];
}

/*F************************************************************************************************/
/*!
    \Function    SetAt

    \Description
        Sets the value at a specified index.

    \Input      nIndex            -
    \Input      pNewValue         -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptArray::SetAt(const int32_t nIndex, AptValue * const pNewValue)
{
    APT_ASSERT(pNewValue != NULL);
    APT_ASSERT(nIndex < mnLength);
    APT_ASSERT(mpValues != NULL);

    AptValue *  pOldValue = mpValues[nIndex];

    APT_INC(    pNewValue);
    APT_DECSAFE(pOldValue);

    mpValues[nIndex] = pNewValue;
}


/*F************************************************************************************************/
/*!
    \Function    AptArray::RegisterReferences

    \Description
        This calls APT_REGISTER_REFERENCE_SAFE on all our references

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptArray::RegisterReferences()
{
    if( APT_REFERENCES_REGISTERED(this) )
        return;

    AptObject::RegisterReferences();

    // Do our own thing.
    for( int i = 0; i < mnLength; i++)
    {
        //pTmp = GetAt(i);
        //APT_REGISTER_REFERENCE_SAFE( pTmp, "Element");
        APT_ASSERT(mpValues);
        APT_REGISTER_REFERENCE_SAFE( mpValues[i], "Element", APT_REFREG_IS_APTVALUE);
    }

    return;
}



/*F************************************************************************************************/
/*!
    \Function    AptArray::_reserve

    \Description
        USed to provide the anticipated number of items in the Array.

    Insert your long description here.

    \Input      nSize             - Anticipated Size

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptArray::_reserve(int nSize)
{
    int nCapSize = 0;

    if (mnCapacity >= nSize)
        return;

    nSize--;
    while (nSize)
    {
        nCapSize++;
        nSize >>= 1;
    }
    nCapSize = 1 << (nCapSize);
    nCapSize = nCapSize < APTARRAY_START_SIZE ? APTARRAY_START_SIZE : nCapSize;


    AptValue **_aArray = APT_MALLOC_ARRAY(AptValue*, nCapSize);

    if (mpValues && _aArray != NULL)
    {
        memcpy(_aArray, mpValues, sizeof(AptValue *) * mnCapacity);
        APT_FREE_ARRAY(mpValues, AptValue*, mnCapacity);
    }

    // $bug 965 - pNewValue != NULL assert slicing arrays that have been resized. - added code to initialize array with
    // gpUndefinedValue. -- DC Only write gpundefined value out to memory that is not init'd with the memcpy.
    for (int32_t i = mnCapacity; i < nCapSize && _aArray != NULL; i++ )
    {
        _aArray[i] = gpUndefinedValue ;
    } // end of fix

    mpValues = _aArray;
    mnCapacity = nCapSize;
}

/*F************************************************************************************************/
/*!
    \Function    AptArray::set

    \Description
        Sets the given item into the given index

    \Input      nIndex            - index to set
    \Input      *pValue           - Value to put there

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptArray::set(int nIndex, AptValue *pValue)
{
    if (nIndex < 0)
    {
        return;
    }

    _reserve(nIndex + 1);
    APT_ASSERT(nIndex < mnCapacity);

    mnLength = (nIndex + 1) > mnLength ? (nIndex + 1) : mnLength;

    SetAt(nIndex, pValue);
}

/*F************************************************************************************************/
/*!
    \Function    *AptArray::get

    \Description
        returns the item at the given index

    \Input      nIndex            -

    \Output     AptValue          -

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptValue *AptArray::get(int nIndex)
{
    if (nIndex < 0 || nIndex >= mnLength)
    {
        return gpUndefinedValue;
    }

    AptValue *  pValue = GetAt(nIndex);
    if (pValue == NULL)
    {
        pValue = gpUndefinedValue;
    }
    return  (pValue);
}

/*F************************************************************************************************/
/*!
    \Function    AptArray::toString

    \Description
        Concatenates the array contents into a single string using the given separator

    Insert your long description here.

    \Input      *szBuf            - Buffer to place the result
    \Input      *szSeparator      - char to use as separator

    \Output
        None.

    \Notes
        WARNING! Buffer overflow much? These strings can be huge...  Buyer beware!
        I Highly recommend using the version that takes a NativeString instead.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptArray::toString(char *szBuf, const char *szSeparator)
{
    APT_ASSERT(szSeparator);
    AptNativeString sBuf;
    toString(sBuf, szSeparator);
    strcpy(szBuf, sBuf.ConstRawPtr());
}

/*F************************************************************************************************/
/*!
    \Function    AptArray::toString

    \Description
        Concatenates the array contents into a single string using the given separator

    Insert your long description here.

    \Input      &sBuf             - Buffer to place the result
    \Input      *szSeparator      - char to use as separator

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptArray::toString(AptNativeString &sBuf, const char *szSeparator)
{
    APT_ASSERT(szSeparator);
    sBuf = "";

    // $senzee $TODO optimization: calculate total length and reserve() it before appending to sBuf

    for (int i=0; i<mnLength; i++)
    {
        AptValue *  pValue = GetAt( i );
        if (pValue != NULL)
        {
            TO_STRING(pValue, psVal);
            sBuf += *psVal;
        }
        if (i < mnLength - 1)
        {
            sBuf += szSeparator;
        }
    }
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
        1.1        11/21/07 (JP) Checking that pContext is not NULL

*/
/************************************************************************************************F*/
AptValue *    AptArray::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    ArrayMembers *pProp = pContext ? ArrayMembersIndex::in_word_set(pName->c_str(), pName->Size()) : NULL;
    if (pProp)
    {
        switch(pProp->nIndex)
        {
            case(AptArray_length):
            {
                return AptInteger::Create(mnLength);
            }
#if APT_USE_ARRAY_METHODS
            break;
            case AptArray_concat:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(concat);
            }
            break;
            case AptArray_join:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(join);
            }
            break;
            case AptArray_toString:

            {
                // DC 2004/04/08 -- No need for it's own function. toString()
                //    functions the same as join().
                NATIVE_MEMBER_FUNCTION_DISPATCH(join);
            }
            break;
            case AptArray_pop:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(pop);
            }
            break;
            case AptArray_push:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(push);
            }
            break;
            case AptArray_shift:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(shift);
            }
            break;
            case AptArray_unshift:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(unshift);
            }
            break;
            case AptArray_reverse:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(reverse);
            }
            break;
            case AptArray_sort:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(sort);
            }
            break;
            case AptArray_splice:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(splice);
            }
            break;
            case AptArray_slice:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(slice);
            }
            break;
            case AptArray_sortOn:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(sortOn);
            }
#endif
        }
    }

#if defined(EA_DEBUG)
    if (
        (pName->EqualNoCase("length"))
#if APT_USE_ARRAY_METHODS
        || (pName->EqualNoCase("concat")) ||
        (pName->EqualNoCase("join")) ||
        (pName->EqualNoCase("pop")) ||
        (pName->EqualNoCase("push")) ||
        (pName->EqualNoCase("shift")) ||
        (pName->EqualNoCase("unshift")) ||
        (pName->EqualNoCase("reverse")) ||
        (pName->EqualNoCase("sort")) ||
        (pName->EqualNoCase("splice")) ||
        (pName->EqualNoCase("slice")) ||
        (pName->EqualNoCase("sortOn")) ||
        (pName->EqualNoCase("toString"))
#endif
       )
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "AptArray: Incorrect case for '%s'.\n", pName->c_str());
        APT_ASSERT(0);
    }
#endif

    char *pEnd = 0;
    int nRet = strtol(pName->c_str(), &pEnd, 10);
    int nLen = pName->Size();
    if (pContext && nLen > 0 && (pEnd == pName->c_str() + nLen)) // if it ended at the null termination, and was non-empty, it was a number as a string
    {
        return pContext->c_array()->get(nRet);
    }

    // DC -- Fixed function to allow NULL return when string is not a member
    return Lookup(pName);
}

static int _isIndex(const char *szName)
{
    int nNum = atoi(szName);
    if (nNum == 0)
    {
        if (szName[0] == '0') return 1;
        return 0;
    }
    return 1;
}

/*F************************************************************************************************/
/*!
    Sets the AptValue for the context and the member name.

    \Input      pContext          - The context passed in.
    \Input      pName             - The member name.
    \Input      pValue            - The value to set.

    \Output     bool              - true if the member was succesfully set.
                                    false otherwise (if not an index).

    \Notes
        For an AptArray, the member should be only an index.

    \Version
        1.0        03/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
bool    AptArray::objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue)
{
    APT_PROFILE_LOOKUP(AptArray_objectMemberSet);
    APT_ASSERT(pContext->isArray());

    if (_isIndex(pName->c_str()))
    {
        int nNum = atoi(pName->c_str());
        pContext->c_array()->set(nNum, pValue == 0 ? gpUndefinedValue : pValue);
        return  (true);
    }
    return  (false);
}

#if APT_USE_ARRAY_METHODS
NATIVE_MEMBER_FUNCTION(AptArray, concat)
{
    if (pThis->isArray())
    {
        AptArray *pArray = pThis->c_array();
        AptArray *pNewArray = new AptArray();
        int i;

        // copy original array
        for (i=0; i<pArray->mnLength; i++)
        {
            pNewArray->set(pNewArray->mnLength, pArray->GetAt(i));
        }
        // flatten concat parameters
        for (i=0; i<nParams; i++)
        {
            AptValue *pValue = gAptActionInterpreter.stackAt(i);

            if (pValue->isArray())
            {
                AptArray *pArray2 = pValue->c_array();
                for (int j=0; j<pArray2->mnLength; j++)
                {
                    pNewArray->set(pNewArray->mnLength, pArray2->GetAt(j));
                }
            }
            else
            {
                pNewArray->set(pNewArray->mnLength, pValue);
            }
        }

        return pNewArray;
    }

    return gpUndefinedValue;
}

NATIVE_MEMBER_FUNCTION(AptArray, join)
{
    if (pThis->isArray())
    {
        AptArray *pArray = pThis->c_array();
        AptNativeString sBuf;
        AptNativeString sSeparator;

        if (nParams > 0)
        {
            gAptActionInterpreter.stackAt(0)->toString(sSeparator);
            pArray->toString(sBuf, sSeparator.ConstRawPtr());
        }
        else
        {
            pArray->toString(sBuf);
        }


        AptString *pString = AptString::Create();
        pString->str = sBuf;
        return pString;
    }

    return gpUndefinedValue;
}

NATIVE_MEMBER_FUNCTION(AptArray, pop)
{
    AptValue *pRet = gpUndefinedValue;

    if (pThis->isArray())
    {
        AptArray *pArray = pThis->c_array();

        if (pArray->mnLength > 0)
        {
            pRet = pArray->get(pArray->mnLength - 1);
// ONA 2004/3/19: We don't decrement here otherwise the object will be destroyed before being added to the stack
//                  The dec will occurs outside this function in the callMethod
//          APT_DEC(pRet);
            pArray->mnLength--;
            pArray->mpValues[pArray->mnLength] = NULL;
        }
    }

    return pRet;
}

NATIVE_MEMBER_FUNCTION(AptArray, push)
{
    if (pThis->isArray())
    {
        AptArray *pArray = pThis->c_array();
        for (int i=0; i<nParams; i++)
        {
            pArray->set(pArray->mnLength, gAptActionInterpreter.stackAt(i));
        }

        return AptInteger::Create(pArray->mnLength);
    }

    return gpUndefinedValue;
}

NATIVE_MEMBER_FUNCTION(AptArray, shift)
{
    AptValue *pRet = gpUndefinedValue;

    if (pThis->isArray())
    {
        AptArray *pArray = pThis->c_array();

        if (pArray->mnLength > 0)
        {
            pRet = pArray->get(0);
            pArray->mnLength--;
            // DC 2004/4/06: We don't decrement here otherwise the object will be destroyed before being added to the stack
            //               The dec will occurs outside this function in the callMethod
            //APT_DEC(pRet);
            if (pArray->mnLength)
            {
                memmove(&pArray->mpValues[0], &pArray->mpValues[1], sizeof(AptValue *) * pArray->mnLength);
            }
            pArray->mpValues[pArray->mnLength] = NULL;
        }
    }
    return pRet;
}

NATIVE_MEMBER_FUNCTION(AptArray, unshift)
{
    if (pThis->isArray())
    {
        AptArray *pArray = pThis->c_array();
        pArray->_reserve(pArray->mnLength + nParams);
        if (nParams)
        {
//          memmove(&pArray->apValues[nParams], &pArray->apValues[0], sizeof(AptValue *) * nParams);
            // bug fix 307 it should shift pArray->mnLength and not nParams number of AptValues.
            memmove(&pArray->mpValues[nParams], &pArray->mpValues[0], sizeof(AptValue *) * pArray->mnLength);
            pArray->mnLength += nParams;
            for (int i=0; i<nParams; i++)
            {
                pArray->mpValues[i] = NULL;
                pArray->set(i, gAptActionInterpreter.stackAt(i));
            }
        }

        return AptInteger::Create(pArray->mnLength);
    }
    return gpUndefinedValue;
}

int AptArray::defaultSortCompareFunc(const void *a, const void *b)
{
    AptValue *pA = *(AptValue **)a;
    AptValue *pB = *(AptValue **)b;

    // DC 2004/04/06 - Flash compares in string form. i.e. 11 < 2 < 'Z' < 'a' etc.
    // compares with regard to content other then string compares (i.e. numbers)
    // must be obtained with a custom compare function.
    TO_STRING(pA, psA);
    TO_STRING(pB, psB);
    return strcmp(*psA, *psB);
}

AptNativeString strFieldName;       // this field name is used in SortOn compare function.

static AptValue *gpfnScriptSortFunctionFunction = 0;
static AptCIH *gpfnScriptSortFunctionContext = 0;

int AptArray::scriptFunctionSortFunc(const void *a, const void *b)
{
    if (!gpfnScriptSortFunctionFunction) return 0;

    AptValue *pA = *(AptValue **)a;
    AptValue *pB = *(AptValue **)b;

    //void * pSavedValue   = gAptActionInterpreter.PrepareForExecution("arraySortScriptFunction");
    APT_DEFINE_ACTION_SETUP(gpfnScriptSortFunctionContext, gpfnScriptSortFunctionFunction,
        "AptArraySortScriptFunction", AptActionType_ArraySortFunction);
    void * pSavedValue   = gAptActionInterpreter.PrepareForExecution(&oActionSetup);

    gAptActionInterpreter.stackPush(pB);
    gAptActionInterpreter.stackPush(pA);
    #if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
        APT_PROF_ENTER("SortOn_Compare");
    #endif
    gAptActionInterpreter.callFunction(gpfnScriptSortFunctionContext, gpfnScriptSortFunctionFunction, 2);
    #if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
        APT_PROF_LEAVE("SortOn_Compare");
    #endif

    int nResult = gAptActionInterpreter.stackAt(0)->toInteger();
    gAptActionInterpreter.stackPop();

    //gAptActionInterpreter.CleanupAfterExecution("arraySortScriptFunction", pSavedValue);
    gAptActionInterpreter.CleanupAfterExecution(pSavedValue, &oActionSetup);  //oActionSetup is defined by the macro

    return  (nResult);
}

NATIVE_MEMBER_FUNCTION(AptArray, sort)
{
    if (pThis->isArray())
    {
        AptArray *pArray = pThis->c_array();
        if (nParams == 0)
        {
            qsort(pArray->mpValues, pArray->mnLength, sizeof(AptValue *), defaultSortCompareFunc);

        }
        else
        {
            gpfnScriptSortFunctionFunction = gAptActionInterpreter.stackAt(0);
            gpfnScriptSortFunctionContext = gpfnScriptSortFunctionFunction->c_scriptfunction()->mpCIH;
            qsort(pArray->mpValues, pArray->mnLength, sizeof(AptValue *), scriptFunctionSortFunc);
        }
    }
    return gpUndefinedValue;
}


/*F************************************************************************************************/
/*!
    \Function    AptArray::objectFindComparator

    \Description  Compares two AptValues.  Can compare one value with another, or compare a value
                  against the properties of the other value.

    \Input
        value     - First value to compare
        equals    - 2nd value to compare (directly with value or with a given property of value)
        equalsStr - If equals is a string, this is the contained string (to avoid converting to a string repeatedly)
        property  - The property name if we are comparing value.property with equals. Null if we are comparing value with equals itself

    \Output    0 if the values are equal, a number different from 0 otherwise

    \Notes     None

    \Version   1.1        11/24/08 (JP) Checking if the object has the property (lookup can return NULL)
    \Version   1.2        02/03/09 (RB) Fixing bugs in propery comparison; EATech 117137.
*/
/************************************************************************************************F*/
int AptArray::objectFindComparator(AptValue *value, AptValue *equals, const AptNativeString *equalsStr, const AptNativeString *property)
{
    if (property)  // What we are comparing is a property of the value, not the value itself of the object
    {
        if( !value->isObject() )
        {
            return 1;  // not an object; comparison fails
        }

        value = ((AptObject *)value)->Lookup(property);
        if ( value==NULL )
        {
            return 1; // no such property; comparison fails
        }
    }

    if (value == equals)
    {
        return 0; // value being compared is the value being compared against.
                  // so, they must be equal!
    }

    AptVirtualFunctionTable_Indices valueType  = value->getVtblIndex();
    AptVirtualFunctionTable_Indices equalsType = equals->getVtblIndex();

    // Booleans
    if (valueType == AptVFT_Boolean && equalsType == AptVFT_Boolean)
    {
        return (int)value->toBool() - (int)equals->toBool();
    }

    // Floats or integers
    if ((valueType  == AptVFT_Float || valueType  == AptVFT_Integer) &&
        (equalsType == AptVFT_Float || equalsType == AptVFT_Integer))
    {
        float diff = AptValueHelper::NumberToFloat(value) - AptValueHelper::NumberToFloat(equals);

        if (diff > APT_EPSILON)
        {
            return 1;
        }
        else if (diff < -APT_EPSILON)
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }

    // Strings
    if (valueType == AptVFT_StringValue && equalsType == AptVFT_StringValue)
    {
        TO_STRING(value, valueStr);
        return strcmp(valueStr->GetBuffer(), equalsStr->GetBuffer());
    }

    // not comparable; comparison fails
    return 1;
}


/*F************************************************************************************************/
/*!
    \Function    AptArray::find

    \Description  Looks for the element in an array with a given value

    \Input
        comparator - Function used to compare (usually AptArray::objectFindComparator)
        start      - First index to compare in the array
        size       - Array size
        values     - Array of AptValues to look in
        equals     - Second value to compare against (directly with values[i] or with a given property of values[i])
        equalsStr  - If equals is a string, this is the contained string (to avoid converting to a string repeatedly)
        property   - The property name if we are comparing value.property with equals. Null if we are comparing value with equals itself.
        reverse    - True if and only if we are looking in reverse order ([start... 0] decreasing).

    \Output    Index of the element of the array equal (or with property equal) to value.
               Returns -1 if the value is not found in the array.

    \Version   1.1        11/24/08 (JP) Adding the header
    \Version   1.2        02/03/09 (RB) Fixing bugs in propery comparison; EATech 117137.
*/
/************************************************************************************************F*/
static int find(AptArray::fnFindComparator_t comparator, int start, int size, AptValue **values,
                  AptValue *equals, const AptNativeString *equalsStr, const AptNativeString *propertyStr, bool reverse)
{
    if (!reverse)
    {
        if(start < 0)
        {
            start = 0;
        }
        for (int32_t i = start; i < size; i++)
        {
            if (comparator(values[i], equals, equalsStr, propertyStr) == 0)
            {
                return i;
            }
        }
    }
    else
    {
        if ( start >= size )
        {
            start = size-1;
        }
        for (int32_t i = start; i >= 0; --i)
        {
            if (comparator(values[i], equals, equalsStr, propertyStr) == 0)
            {
                return i;
            }
        }
    }
    return -1;
}

int AptArray::find(fnFindComparator_t comparator, int start, AptValue *equals, AptValue *property, bool reverse)
{
    TO_STRING(equals, equalsStr);
    if (property && !property->isUndefined())
    {
        TO_STRING(property, propertyStr);
        return ::find(comparator, start, mnLength, mpValues, equals, equalsStr, propertyStr, reverse);
    }
    else
    {
        return ::find(comparator, start, mnLength, mpValues, equals, equalsStr, 0, reverse);
    }
}

int AptArray::defaultSortOnCompareFunc(const void *a, const void *b)
{
    AptValue *pA = *(AptValue **)a;
    AptValue *pB = *(AptValue **)b;

    if (pA->isObject() && pB->isObject())
    {
        AptObject *pAObject = pA->c_object();
        AptObject *pBObject = pB->c_object();

        AptValue * pAValue = NULL ;
        AptValue * pBValue = NULL ;

        pAValue = pAObject->Lookup(&strFieldName) ;
        if (pAValue)
        {
            pBValue = pBObject->Lookup(&strFieldName) ;
            if (pBValue)
            {
                // DC 2004/04/06 - call this function to prevent code duplication
                // See definition for more info.
                return defaultSortCompareFunc(&pAValue, &pBValue);
            }
        }
    }
    else if(pA->isArray() && pB->isArray())
    {
        AptValue * pAValue = NULL ;
        AptValue * pBValue = NULL ;
        pAValue = pA->c_array()->get(atoi(strFieldName));
        pBValue = pB->c_array()->get(atoi(strFieldName));

        // DC 2004/04/06 - call this function to prevent code duplication
        // See definition for more info.
        return defaultSortCompareFunc(&pAValue, &pBValue);
    }
    else
    {
        // dont at all compare, as array items are not objects and they wont have fieldnames in it.
        // help says that if fieldname is not present then function defualts to array.sort, but we will just
        // return false. - so result is undefined. and also we will assert so get to know the error.
        APT_ASSERT(NOT_REACHED);
        return 0;
    }
    // it comes here if fieldname is not present in object. return 0 (compare equal).
    // so there wont be any sorting.
    return 0 ; // DC 2004/04/06 - retVal modified, "false" is not an int.
}



NATIVE_MEMBER_FUNCTION(AptArray, sortOn)
{
    if (pThis->isArray())
    {
        AptArray *pArray = pThis->c_array();
        if (nParams >  0)
        {
            // At this time, Apt does not support other parameters like Array.NUMERIC to this function
            APT_ASSERT(nParams == 1) //  $bug 106675 - Array.NUMERIC and related Array.sortOn options values are undefined at run-time
            gAptActionInterpreter.stackAt(0)->toString(strFieldName);   // get fieldname and store it globally
            qsort(pArray->mpValues, pArray->mnLength, sizeof(AptValue *), defaultSortOnCompareFunc);
            strFieldName.Clear();       //  Clear the temporary string afterward
        }
    }
    return gpUndefinedValue;
}



NATIVE_MEMBER_FUNCTION(AptArray, reverse)
{
    if (pThis->isArray())
    {
        AptArray *pArray = pThis->c_array();
        AptValue *pTemp;

        for (int i=0; i<pArray->mnLength / 2; i++)
        {
            pTemp = pArray->mpValues[i];
            pArray->mpValues[i] = pArray->mpValues[pArray->mnLength - i - 1];
            pArray->mpValues[pArray->mnLength - i - 1] = pTemp;
        }
        return pThis;
    }
    return gpUndefinedValue;
}

NATIVE_MEMBER_FUNCTION(AptArray, splice)
{
    if (pThis->isArray())
    {
        AptArray *pArray = pThis->c_array();

        if (nParams > 0)
        {
            AptValue* pStackValue = gAptActionInterpreter.stackAt(0);
            if ( pStackValue->isUndefined() )
            {
                return gpUndefinedValue;
            }

            int nStart = pStackValue->toInteger();
            if (nStart < 0)
            {
                // convert to non-negative index
                nStart = pArray->mnLength + nStart;
                if ( nStart < 0 )
                {
                    nStart = 0;
                }
            }
            APT_ASSERT( nStart >= 0 );

            if ( nStart >= pArray->mnLength )
            {
                nStart = pArray->mnLength;
            }

            // check how many items should be deleted

            APT_ASSERT( nStart <= pArray->mnLength );
            int nNumToDelete = pArray->mnLength - nStart;

            if (nParams > 1)
            {
                AptValue* pStackValue = gAptActionInterpreter.stackAt(1);
                if ( pStackValue->isUndefined() )
                {
                    return gpUndefinedValue;
                }
                else
                {
                    nNumToDelete = pStackValue->toInteger();
                    if(nNumToDelete > (pArray->mnLength - nStart))
                    {
                        nNumToDelete = pArray->mnLength - nStart;
                    }
                }
            }

            APT_ASSERT( nStart >= 0 );
            APT_ASSERT( nStart <= pArray->mnLength );
            APT_ASSERT( nNumToDelete <= (pArray->mnLength - nStart) );


            // if deleteCount param was negative, we return undefined
            if ( nNumToDelete >= 0 )
            {
                int i = 0;
                AptArray *pNewArray = new AptArray();

                if ( nNumToDelete > 0 )
                {
                    APT_ASSERT( nStart < pArray->mnLength );

                    for (i = 0; i < nNumToDelete; i++)
                    {
                        AptValue *  pValue = pArray->GetAt(nStart + i);
                        if(pValue == NULL)
                        {
                            pValue = gpUndefinedValue;
                        }
                        // $bug927 - RB - 07/05/2005 - store deleted elements in returned array
                        pNewArray->set(pNewArray->mnLength, pValue);

                        APT_DECSAFE(pValue);
                    }

                    // close the gap left by the deleted items
                    memmove(&pArray->mpValues[nStart], &pArray->mpValues[nStart + nNumToDelete], sizeof(AptValue *) * (pArray->mnLength - (nStart + nNumToDelete)));
                    for (i = 0; i < nNumToDelete; i++)
                    {
                        pArray->mpValues[(pArray->mnLength - nNumToDelete) + i] = NULL;
                    }
                    pArray->mnLength -= nNumToDelete;
                }


                if (nParams > 2)
                {
                    // check how many items should be inserted
                    int nNumToAdd = nParams - 2;
                    APT_ASSERT( nNumToAdd >= 1 );

                    pArray->_reserve(pArray->mnLength + nNumToAdd);

                    APT_ASSERT( nStart >= 0 );
                    if ( pArray->mnLength - nStart > 0 )
                    {
                        // everything to the right of (and including) nStart is shifted to the right, to make room for the inserted items
                        memmove(&pArray->mpValues[nStart + nNumToAdd], &pArray->mpValues[nStart], sizeof(AptValue *) * (pArray->mnLength - nStart));
                    }

                    // set the inserted items in the array
                    pArray->mnLength += nNumToAdd;

                    APT_ASSERT( nStart < pArray->mnLength );
                    APT_ASSERT( (nStart + nNumToAdd) <= pArray->mnLength );
                    for (i = 0; i < nNumToAdd; i++)
                    {
                        pArray->mpValues[nStart + i] = NULL;
                        pArray->set(nStart + i, gAptActionInterpreter.stackAt(i + 2));
                    }
                }

                APT_ASSERT( nNumToDelete >= 0 );
                APT_ASSERT( pNewArray->mnLength == nNumToDelete );
                return pNewArray;
            }
        }
    }

    return gpUndefinedValue;
}

NATIVE_MEMBER_FUNCTION(AptArray, slice)
{
    if (pThis->isArray())
    {
        AptArray *pArray = pThis->c_array();
        AptArray *pNewArray = NULL ;

        int i;

        // DC  2004/4/06  modified to allow slice() [no arguments] and also
        //                to handle negative arguments (all valid in Flash).

        int nStart = 0;             // Default is 0
        int nEnd = pArray->mnLength; // Default is the end of the string.

        if (nParams > 0)
        {
            nStart = gAptActionInterpreter.stackAt(0)->toInteger();
            // Negative offsets go backwards from the end of the array.
            if (nStart < 0)
            {
                nStart = pArray->mnLength + nStart;  // actually subtracts (nStart is negative)
            }
        }
        if (nParams > 1)
        {
            nEnd = gAptActionInterpreter.stackAt(1)->toInteger();
            // Negative offsets go backwards from the end of the array.
            if (nEnd < 0)
            {
                nEnd = pArray->mnLength + nEnd;      // actually subtracts (nEnd is negative)
            }
            else if (nEnd > pArray->mnLength)
            {
                nEnd = pArray->mnLength ;
            }
        }

        // nStart cannot be smaller then nEnd.
        // neither value can be negative (should have already handled that).
        // nStart == nEnd is valid and returns an empty array.
        if (nStart > nEnd || nStart<0 || nEnd<0)
        {
            return gpUndefinedValue;
        }

        // now create a new array.
        pNewArray = new AptArray();

        // copy from original array
        for (i = nStart; i < nEnd; i++)     // do not include last element
        {
            pNewArray->set(pNewArray->mnLength, pArray->GetAt(i));
        }

        return pNewArray;
    }
    return gpUndefinedValue;
}
#endif
