/*H************************************************************************************************/
/*!

    \File
        AptFrameStack.h

    \Description
        This is a wrapper around AptValueWithHash used mainly for scoping variables.

    Technically it just provides functions with better names and a couple of new functions for
    chaining, (i.e. it has a parent scope to chain up to.) This is designed to work the way flash
    does, not necessarily generically.


    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/07/04 (ONA) First Version

*/
/************************************************************************************************H*/

#ifndef __APTFRAMESTACK_H__
#define __APTFRAMESTACK_H__

#include "AptDefine.h"
//#include "AptValue/AptValue.h"
#include "_AptValue.h"
#include "AptNativeHash.h"
#include "AptGC.h"


#define FRAME_STACK_DEFAULT_HASH_SIZE 4

class AptFrameStack : public AptValueWithHash
{
public:
    APT_VALUE_GC_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    APT_INLINE
    explicit AptFrameStack(AptFrameStack * pParentScope) :
        AptValueWithHash(AptVFT_FrameStack, FRAME_STACK_DEFAULT_HASH_SIZE),
        mpParentScope( pParentScope )
    {
        APT_INCSAFE(mpParentScope);
    }

    APT_INLINE
    explicit AptFrameStack(AptFrameStack * pParentScope, int nHashSize) :
        AptValueWithHash(AptVFT_FrameStack, nHashSize),
        mpParentScope( pParentScope )
    {
        APT_INCSAFE(mpParentScope);
    }


    APT_INLINE
    void    Set(const AptNativeString * const pKey, AptValue * const pValue)
    {
        mNativeHash.Set(pKey, pValue);
    }

    APT_INLINE
    AptValue *  Lookup(const AptNativeString * const pKey)
    {
        return  (mNativeHash.Lookup(pKey));
    }

    APT_INLINE
    void ClearScope()
    {
        mNativeHash.ClearDataNoDelete();
    }

    APT_INLINE
    void GetHashSize()
    {
        mNativeHash.GetHashSize() ;
    }



    APT_INLINE
    bool        ExistsInLocalScope(AptNativeString* pVarName)
    {
        return mNativeHash.Lookup(pVarName) != NULL;
    }

    APT_INLINE
    void        SetInLocalScope(AptNativeString* pVarName, AptValue *pValue)
    {
        mNativeHash.Set(pVarName, pValue);
    }

    bool SetWhereExistsInScopeChain(AptNativeString* pVarName, AptValue *pValue)
    {
        AptFrameStack * pF = this;
        while(pF)
        {
            if(pF->mNativeHash.Lookup(pVarName) != NULL)
            {
                pF->mNativeHash.Set(pVarName, pValue);
                return true;
            }
            pF = pF->mpParentScope;
        }

        return false;
    }

    AptValue*   GetInScopeChain(AptNativeString* pVarName)
    {
        AptFrameStack * pF = this;

        while(pF)
        {
            AptValue * pValue = pF->mNativeHash.Lookup(pVarName);
            if( pValue != NULL )
            {
                return pValue;
            }
            pF = pF->mpParentScope;
        }

        return NULL;
    }

    virtual void DestroyGCPointers()
    {
        APT_DECSAFE(mpParentScope);
        AptValueWithHash::DestroyGCPointers();
    }

    virtual void RegisterReferences()
    {
        if( APT_REFERENCES_REGISTERED(this) )
            return;

        APT_REGISTER_REFERENCE_SAFE(mpParentScope, "ParentScope", APT_REFREG_IS_APTVALUE);
        AptValueWithHash::RegisterReferences();
    }
protected:
    virtual ~AptFrameStack()
    {
        //  Do nothing...
    }

    AptFrameStack * mpParentScope;
};

#endif  //  __APTFRAMESTACK_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

