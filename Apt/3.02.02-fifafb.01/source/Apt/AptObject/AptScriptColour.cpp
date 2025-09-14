/*H************************************************************************************************/
/*!

    \File
        AptScriptColour.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        02/28/04 (ONA) First Version

*/
/************************************************************************************************H*/

#include "AptObject\AptScriptColour.h"

#include "MainInline.h"

#if APT_USE_SCRIPTCOLOUR_OBJECT
APT_DEFINE_OBJECT_METRICS(AptScriptColour, AptObject);

/*F************************************************************************************************/
/*!
    Constructor

    \Input      pMovie            -

    \Notes
        None.

    \Version        1.0        02/28/04 (ONA) First Version
    \Version        1.1        07/01/05 (ONA) $bug928 setRGB crash for Color() object created after loadMovie
    \Version        1.2        03/12/08 (JP) Allow an undefined movie clip

*/
/************************************************************************************************F*/
// fusionchange - modifying the implementation so that it lines up with the header, otherwise we get a link error
// with the snc linker.
AptScriptColour::AptScriptColour(AptValue *const pMovie)
:
AptObject(AptVFT_ScriptColour)
{
    // 2004/05/10 -- DC -- Fixed bug 471: applying a color transform to a text field generated an assert.
    // This just allows text fields to obtain
    if(pMovie != NULL && pMovie->isCIH())
    {
        AptCIH * pCih = pMovie->c_cih();
        if(pCih->IsSpriteInst() || pCih->IsDynamicTextInst() || pCih->IsImageInst())
        {
            pSprite = pCih;
            APT_INC(pSprite);
        }
        else
        {
            pSprite = 0;
        }
    }

    else
    {
        pSprite = 0;
    }
}
#endif
/*F************************************************************************************************/
/*!
    \Function    AptScriptColour::~AptScriptColour

    \Description
        Destructor

    \Input
        None.

    \Output     A                 -

    \Notes
        Do not touch any garbage collected objects over here.

    \Version    1.0        09/29/04 (ASK) First Version

*/
/************************************************************************************************F*/
AptScriptColour::~AptScriptColour()
{
    // commented the dec as we should not be touching any GC objects in destructor
//    APT_DECSAFE(pSprite);
    pSprite = NULL;
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        04/05/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptScriptColour::CleanNativeFunctions()
{
#if APT_USE_SCRIPTCOLOUR_OBJECT
    NATIVE_MEMBER_FUNCTION_DESTROY(setRGB);
    NATIVE_MEMBER_FUNCTION_DESTROY(getRGB);
    NATIVE_MEMBER_FUNCTION_DESTROY(getTransform);
    NATIVE_MEMBER_FUNCTION_DESTROY(setTransform);
#endif
}

#if APT_USE_SCRIPTCOLOUR_OBJECT
/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      pContext          -
    \Input      pName             -

    \Output     AptValue *        -

    \Notes
        None.

    \Version
        1.0        03/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptValue *    AptScriptColour::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    if (pName->CompareNoCase(*StringPool::GetString(SC_setRGB)) == 0)
    {
        NATIVE_MEMBER_FUNCTION_DISPATCH(setRGB);
    }
    else if (pName->CompareNoCase(*StringPool::GetString(SC_getRGB)) == 0)
    {
        NATIVE_MEMBER_FUNCTION_DISPATCH(getRGB) ;
    }
    else if (pName->CompareNoCase(*StringPool::GetString(SC_getTransform)) == 0)
    {
        NATIVE_MEMBER_FUNCTION_DISPATCH(getTransform) ;
    }
    else if (pName->CompareNoCase(*StringPool::GetString(SC_setTransform)) == 0)
    {
        NATIVE_MEMBER_FUNCTION_DISPATCH(setTransform) ;
    }

    return  (NULL);
}

NATIVE_MEMBER_FUNCTION(AptScriptColour, setRGB)
{
    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    AptCIH *pSprite = pThis->c_scriptcolour()->pSprite;
    if (pSprite)
    {
        int nInt = pParam->toInteger();
        AptCXForm *cxform = pSprite->GetColorMatrixWritable() ; // EATech $bug 104740 (CL4629) Acc. to Flash docs, SetRGB does not set alpha transform, see EATech ticket for more details
        cxform->translate.SetValuef(AptColorHelper::Red, (nInt >> 16) & 0xFF );
        cxform->translate.SetValuef(AptColorHelper::Green, (nInt >> 8) & 0xFF );
        cxform->translate.SetValuef(AptColorHelper::Blue, (nInt >> 0) & 0xFF );

        cxform->scale.SetValuef(AptColorHelper::Red, 0.f );
        cxform->scale.SetValuef(AptColorHelper::Green, 0.f );
        cxform->scale.SetValuef(AptColorHelper::Blue, 0.f );
        pSprite->SetASChanged(true);                        // ### BUG337 added to ensure this object retains correct info in displaylist merge
    }
    return gpUndefinedValue;
}

NATIVE_MEMBER_FUNCTION(AptScriptColour, getRGB)
{
    AptCIH *pSprite = pThis->c_scriptcolour()->pSprite;
    if (pSprite)
    {
        const AptColorHelperTranslate& vColorAdd4 = pSprite->GetColorMatrixConst()->translate;
        float fSrcR = vColorAdd4.GetValuef( AptColorHelper::Red );
        float fSrcG = vColorAdd4.GetValuef( AptColorHelper::Green );
        float fSrcB = vColorAdd4.GetValuef( AptColorHelper::Blue );

        if ( fSrcR < 0.f ) fSrcR = 0.0f;
        if ( fSrcG < 0.f ) fSrcG = 0.0f;
        if ( fSrcB < 0.f ) fSrcB = 0.0f;

        if ( fSrcR > 255.f ) fSrcR = 255.0f;
        if ( fSrcG > 255.f ) fSrcG = 255.0f;
        if ( fSrcB > 255.f ) fSrcB = 255.0f;

        uint32_t uDst = 0;
        uDst |= ((uint32_t)fSrcR) << 16;
        uDst |= ((uint32_t)fSrcG) << 8;
        uDst |= ((uint32_t)fSrcB) << 0;
        return AptInteger::Create((int)uDst);
    }
    return gpUndefinedValue;
}

NATIVE_MEMBER_FUNCTION(AptScriptColour, getTransform)
{
    if (nParams > 0)
        return gpUndefinedValue;

    AptScriptColour *pTemp = pThis->c_scriptcolour();
    APT_ASSERT(pTemp);
    AptCIH *pSprite = pTemp->pSprite;
    // Issue 113434: Flash admits undefined parameters here

    if (pSprite && pSprite->getIsDefined())
    {
        AptCharacterInst *pCharInst = pSprite->GetCharacterInst();
        AptObject * pObject = new AptObject(AptVFT_Object);

        const AptCXForm *cxform = pCharInst->GetColorMatrixConst();

#if defined(APT_USE_FLASH_COLOR_RANGE)
        AptValue* pRA = AptFloat::Create((cxform->scale.GetValuef(AptColorHelper::Red)));
        AptValue* pGA = AptFloat::Create((cxform->scale.GetValuef(AptColorHelper::Green)));
        AptValue* pBA = AptFloat::Create((cxform->scale.GetValuef(AptColorHelper::Blue)));
        AptValue* pAA = AptFloat::Create((cxform->scale.GetValuef(AptColorHelper::Alpha)));
        AptValue* pRB = AptFloat::Create((cxform->translate.GetValuef(AptColorHelper::Red)));
        AptValue* pGB = AptFloat::Create((cxform->translate.GetValuef(AptColorHelper::Green)));
        AptValue* pBB = AptFloat::Create((cxform->translate.GetValuef(AptColorHelper::Blue)));
        AptValue* pAB = AptFloat::Create((cxform->translate.GetValuef(AptColorHelper::Alpha)));
#else
        AptValue* pRA = AptInteger::Create((cxform->scale.GetValue(AptColorHelper::Red)));
        AptValue* pGA = AptInteger::Create((cxform->scale.GetValue(AptColorHelper::Green)));
        AptValue* pBA = AptInteger::Create((cxform->scale.GetValue(AptColorHelper::Blue)));
        AptValue* pAA = AptInteger::Create((cxform->scale.GetValue(AptColorHelper::Alpha)));
        AptValue* pRB = AptInteger::Create((cxform->translate.GetValue(AptColorHelper::Red)));
        AptValue* pGB = AptInteger::Create((cxform->translate.GetValue(AptColorHelper::Green)));
        AptValue* pBB = AptInteger::Create((cxform->translate.GetValue(AptColorHelper::Blue)));
        AptValue* pAB = AptInteger::Create((cxform->translate.GetValue(AptColorHelper::Alpha)));
#endif
        pObject->Set(StringPool::GetString(SC_ra), pRA);
        pObject->Set(StringPool::GetString(SC_ga), pGA);
        pObject->Set(StringPool::GetString(SC_ba), pBA);
        pObject->Set(StringPool::GetString(SC_aa), pAA);
        pObject->Set(StringPool::GetString(SC_rb), pRB);
        pObject->Set(StringPool::GetString(SC_gb), pGB);
        pObject->Set(StringPool::GetString(SC_bb), pBB);
        pObject->Set(StringPool::GetString(SC_ab), pAB);

        return pObject ;
    }
    return gpUndefinedValue;
}

NATIVE_MEMBER_FUNCTION(AptScriptColour, setTransform)
{
    if (nParams <= 0)
        return gpUndefinedValue;

    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    // Issue 113434: Flash admits undefined parameters here
    if(pParam->isUndefined())
    {
        return gpUndefinedValue;
    }
    AptScriptColour *pTemp = pThis->c_scriptcolour();
    APT_ASSERT(pTemp);
    AptCIH *pSprite = pTemp->pSprite;
    APT_ASSERT(pSprite);

    // $bug1011 - Crash in NATIVE_MEMBER_FUNCTION(AptScriptColour, setTransform) - added extra check for null sprite.
    if (pSprite == NULL || pSprite->isUndefined())
        return gpUndefinedValue;

    if (pParam->getIsDefined() && pParam->isObject())
    {
        AptCharacterInst *pCharInst = pSprite->GetCharacterInst();
        AptObject * pObject = pParam->c_object();

        AptCXForm *cxform = pCharInst->GetColorMatrixWritable() ;

        AptValue * pRA = pObject->Lookup(StringPool::GetString(SC_ra));
        if(pRA != NULL)
        {
#if defined(APT_USE_FLASH_COLOR_RANGE)
            cxform->scale.SetValuef(AptColorHelper::Red, pRA->toFloat());
#else
            cxform->scale.SetValue(AptColorHelper::Red, pRA->toInteger());
#endif
        }

        AptValue * pRB = pObject->Lookup(StringPool::GetString(SC_rb));
        if(pRB != NULL)
        {
#if defined(APT_USE_FLASH_COLOR_RANGE)
            cxform->translate.SetValuef(AptColorHelper::Red, pRB->toFloat());
#else
            cxform->translate.SetValue(AptColorHelper::Red, pRB->toInteger());
#endif
        }

        AptValue * pGA = pObject->Lookup(StringPool::GetString(SC_ga));
        if(pGA != NULL)
        {
            float val;
            val = pGA->toFloat();
            val += .0f;

#if defined(APT_USE_FLASH_COLOR_RANGE)
            cxform->scale.SetValuef(AptColorHelper::Green, pGA->toFloat());
#else
            cxform->scale.SetValue(AptColorHelper::Green, pGA->toInteger());
#endif
        }

        AptValue * pGB = pObject->Lookup(StringPool::GetString(SC_gb));
        if(pGB != NULL)
        {
#if defined(APT_USE_FLASH_COLOR_RANGE)
            cxform->translate.SetValuef(AptColorHelper::Green, pGB->toFloat());
#else
            cxform->translate.SetValue(AptColorHelper::Green, pGB->toInteger());
#endif
        }

        AptValue * pBA = pObject->Lookup(StringPool::GetString(SC_ba));
        if(pBA != NULL)
        {
#if defined(APT_USE_FLASH_COLOR_RANGE)
            cxform->scale.SetValuef(AptColorHelper::Blue, pBA->toFloat());
#else
            cxform->scale.SetValue(AptColorHelper::Blue, pBA->toInteger());
#endif
        }

        AptValue * pBB = pObject->Lookup(StringPool::GetString(SC_bb));
        if(pBB != NULL)
        {
#if defined(APT_USE_FLASH_COLOR_RANGE)
            cxform->translate.SetValuef(AptColorHelper::Blue, pBB->toFloat());
#else
            cxform->translate.SetValue(AptColorHelper::Blue, pBB->toInteger());
#endif
        }

        AptValue * pAA = pObject->Lookup(StringPool::GetString(SC_aa));
        if(pAA != NULL)
        {
#if defined(APT_USE_FLASH_COLOR_RANGE)
            cxform->scale.SetValuef(AptColorHelper::Alpha, pAA->toFloat());
#else
            cxform->scale.SetValue(AptColorHelper::Alpha, pAA->toInteger());
#endif
        }

        AptValue * pAB = pObject->Lookup(StringPool::GetString(SC_ab));
        if(pAB != NULL)
        {
#if defined(APT_USE_FLASH_COLOR_RANGE)
            cxform->translate.SetValuef(AptColorHelper::Alpha, pAB->toFloat());
#else
            cxform->translate.SetValue(AptColorHelper::Alpha, pAB->toInteger());
#endif
        }

        if ( pRA || pRB || pGA || pGB || pBA || pBB || pAA || pAB )
            pSprite->SetASChanged(true);
    }

    return gpUndefinedValue;
}


/*F************************************************************************************************/
/*!
    \Function    AptScriptColour::RegisterReferences

    \Description
        Registers all garbage collected references with global macro.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/29/04 (ASK)

*/
/************************************************************************************************F*/
void AptScriptColour::RegisterReferences()
{
    if( APT_REFERENCES_REGISTERED(this) )
        return;

    AptObject::RegisterReferences();

    APT_REGISTER_REFERENCE_SAFE(pSprite,    "CIH", APT_REFREG_IS_APTCIH);
    return;
}

/*F************************************************************************************************/
/*!
    \Function    AptScriptColour::DestroyGCPointers

    \Description
        Does work of removing references to GC objects in script function

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/29/04 (ASK) First Version

*/
/************************************************************************************************F*/
void    AptScriptColour::DestroyGCPointers()
{
    APT_DECSAFE(pSprite);
    pSprite = NULL;
    AptObject::DestroyGCPointers();
}
#endif // #if APT_USE_SCRIPTCOLOUR_OBJECT



/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

