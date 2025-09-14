/*H************************************************************************************************/
/*!

    \File    AptCharacterInst.cpp

    \Description
        This is a placeholder file to implement functions for the AptCharacterInst Class as needed.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        03/30/05 (DC) Bringin' it to ya

*/
/************************************************************************************************H*/


/*** Include files ********************************************************************************/

#include "AptCIH.h"
#include "AptCharacterInst.h"
#include "AptRenderItem.h"
#include "AptObject/AptGlobalObject.h"

#include "MainInline.h"

#if !defined(APT_ENABLE_INLINE)
    #include "AptCharacterInst.inl"
#endif

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Function Prototypes **************************************************************************/

/*** Variables ************************************************************************************/
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptCharacterInst, NULL);
APT_DEFINE_OBJECT_METRICS(AptCharacterLevelInst, AptCharacterInst);
APT_DEFINE_OBJECT_METRICS(AptCharacterSpriteInst, AptCharacterInst);
APT_DEFINE_OBJECT_METRICS(AptCharacterAnimationInst, AptCharacterInst);
APT_DEFINE_OBJECT_METRICS(AptCharacterButtonInst, AptCharacterInst);
APT_DEFINE_OBJECT_METRICS(AptCharacterStaticTextInst, AptCharacterInst);
APT_DEFINE_OBJECT_METRICS(AptCharacterTextInst, AptCharacterInst);
APT_DEFINE_OBJECT_METRICS(AptCharacterMorphInst, AptCharacterInst);
APT_DEFINE_OBJECT_METRICS(AptCharacterShapeInst, AptCharacterInst);
APT_DEFINE_OBJECT_METRICS(AptCharacterImageInst, AptCharacterInst);
// Private variables

// Public variables


/*** Private Functions ****************************************************************************/

/*** Public Functions *****************************************************************************/

#if defined (APT_GATHER_MOVIECLIP_METRICS)
void AptCharacterInst::UpdateMovieClipInfo(AptCharacterType eType, AptMovieclipInformation* pMCInfo, int nChange)
{
    switch(eType)
    {
    case AptCharacterType_Shape :
        pMCInfo->nShapes+= nChange ;
        break;
    case AptCharacterType_Sprite :
        pMCInfo->nMovieClips+= nChange ;
        break;
    case AptCharacterType_Button :
#if defined APT_USE_BUTTONS
        pMCInfo->nButtons+= nChange ;
#endif
        break;
    case AptCharacterType_Text :
        pMCInfo->nDynamicText+= nChange ;
        break;
    case AptCharacterType_StaticText :
        pMCInfo->nStaticText+= nChange ;
        break;
    case AptCharacterType_Morph :
        pMCInfo->nMorph+= nChange ;
        break;
    case AptCharacterType_Animation :
        pMCInfo->nAnimations+= nChange ;
        break;
    case AptCharacterType_CustomControl :
        pMCInfo->nCustomControls+= nChange;
        break;
    default :
        break ;
    }
}
#endif

/*F************************************************************************************************/
/*!
    \Function    AptCharacterInst::CreateCharacterInst

    \Description
        Creates and returns a AptCharacterInst based on the pCharacter->eType

    \Input      *pCharacter       -

    \Output     AptCharacterInst *  -

    \Notes
        None.

    \Version    1.0        06/08/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptCharacterInst * AptCharacterInst::CreateCharacterInst(AptCharacter *pCharacter)
{
    AptCharacterInst *pInst = NULL;

    if (pCharacter == NULL)             // we will create a level inst for this.
    {
        pInst = new AptCharacterLevelInst(NULL);
        return pInst;
    }
    switch(pCharacter->eType)
    {
        case AptCharacterType_Sprite:           pInst = new AptCharacterSpriteInst(pCharacter);         break;
        case AptCharacterType_Text:             pInst = new AptCharacterTextInst(pCharacter);           break;
        case AptCharacterType_StaticText:       pInst = new AptCharacterStaticTextInst(pCharacter);     break;
        case AptCharacterType_Shape:            pInst = new AptCharacterShapeInst(pCharacter);          break;
        case AptCharacterType_Morph:            pInst = new AptCharacterMorphInst(pCharacter);          break;

        case AptCharacterType_Button:
        {
            #if defined APT_USE_BUTTONS
            {
                pInst = new AptCharacterButtonInst(pCharacter);
            }
            #else
            {
                APT_FAIL("Buttons are not compiled into this version, please recompile with buttons enabled.");
            }
            #endif
            break;
        }

        default:
        {
            APT_FAIL("Uncertain how to make an instance for this type");
            break;
        }
    }
    return pInst;
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterInst::AptCharacterInst

    \Description
        Constructor.

    \Input      pCharacter        -

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        04/14/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptCharacterInst::AptCharacterInst(AptCharacter *pCharacter):
    mpRenderItem(0), mpNativeHash(NULL)
{
    mpRenderItem        = (GetTargetSim() == NULL) ? NULL : GetTargetSim()->GetRenderManager()->Update_CreateItem(pCharacter, gnCurrUpdateTick);
    mnCharInstType      = pCharacter ? pCharacter->eType : AptCharacterType_Level;

#if defined (APT_GATHER_MOVIECLIP_METRICS)
        AptCharacterInst::UpdateMovieClipInfo(mnCharInstType, &gAptMovieclipInformation, 1);
#endif

    if(mpRenderItem != NULL)
        APT_RI_INC(mpRenderItem,this, "CharacterInst");
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterInst::~AptCharacterInst

    \Description
        Destructor

    \Input
        None.

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        04/12/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptCharacterInst::~AptCharacterInst(void)
{
    //DC-- This assert points out that we did not call destroy GC pointers before deleting the object.
    // This will happen however, when the object is garbage collected (swept). It points out minor
    // imperfections in GC handling but nothing to worry about (Perhaps to be addressed in a
    // future major version update.. if we ever run out of real problems to address :).

    APT_ASSERT(mpNativeHash == NULL);
    if (mpNativeHash != NULL)
    {
        delete  mpNativeHash;
        mpNativeHash = NULL;
    }

#if defined (APT_GATHER_MOVIECLIP_METRICS)
    if(mpRenderItem != NULL)
        AptCharacterInst::UpdateMovieClipInfo(mpRenderItem->GetCharacterType(), &gAptMovieclipInformation, -1);
    else
        AptCharacterInst::UpdateMovieClipInfo(mnCharInstType, &gAptMovieclipInformation, -1);
#endif

    //APT_ASSERT(mpRenderItem != NULL);
    if(mpRenderItem != NULL)
        APT_RI_DEC(mpRenderItem,this, "CharacterInst");
    mpRenderItem = NULL;
}

/*F************************************************************************************************/
/*!
    \Function    *AptCharacterInst::GetRenderItemWritable

    \Description
        Gets a writable Render item (creates it if needed).

    \Input
        None.

    \Output     AptRenderItem            -

    \Notes
        None.

    \Version    1.0        07/20/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptRenderItem *AptCharacterInst::GetRenderItemWritable(void)
{
    return const_cast<AptRenderItem *>(mpRenderItem);
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterInst::SetRenderItem

    \Description
        Swaps out the render item with the passed item.

    \Input      pItem             -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/20/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptCharacterInst::SetRenderItem(AptRenderItem * pItem)
{
    APT_ASSERT(mpRenderItem);
    if(pItem != mpRenderItem)
    {
        APT_RI_INC(pItem,       this, "CharacterInst");
        APT_RI_DEC(mpRenderItem,this, "CharacterInst");
        mpRenderItem = pItem;
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterInst::CopyRenderDataFrom

    \Description
        Moves the render data from pModel to this isntance

    \Input      pModel            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/18/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptCharacterInst::MoveRenderDataFrom(const AptCharacterInst * pModel)
{
     CopyRenderDataFrom(pModel);

     SetDepth(pModel->GetDepth());

     if(pModel->mpNativeHash != NULL)
     {
         if(mpNativeHash != NULL)
         {
             mpNativeHash->DestroyGCPointers();
             delete mpNativeHash;
         }

         mpNativeHash = pModel->mpNativeHash;

        AptCharacterInst * pTemp = (AptCharacterInst *) pModel;
        pTemp->mpNativeHash = NULL;
     }
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterInst::CopyRenderDataFrom

    \Description
        Copies render data from pModel to this instance

    \Input      pModel            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/18/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptCharacterInst::CopyRenderDataFrom(const AptCharacterInst * pModel)
{
     /*lint -e(613) */ GetRenderItemWritable()->CopyRenderDataFrom(pModel->GetRenderItem());
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterButtonInst::AptCharacterButtonInst

    \Description
        Constructs a button inst.

    \Input      *pCharacter       -

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        04/14/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
#if defined APT_USE_BUTTONS
/*F************************************************************************************************/
/*!
    \Function    AptCharacterButtonInst::AptCharacterButtonInst

    \Description
        constructor for AptCharacterButtonInst

    \Input      *pCharacter       -

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        06/08/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptCharacterButtonInst::AptCharacterButtonInst(AptCharacter *pCharacter) :
AptCharacterInst(pCharacter),
    mDisplayList()
{
    SetNativeHash(new AptNativeHash(4));
    mnState = AptCharacterButtonRecordState_None;
}
#endif



/*F************************************************************************************************/
/*!
    Constructor for AptCharacterSpriteInstBase

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        02/27/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptCharacterSpriteInstBase::AptCharacterSpriteInstBase(AptCharacter *pCharacter) :
    AptCharacterInst(pCharacter), mDisplayList()
{
    mnFrame         = -1;
    mpClipActions   = 0;
    mnGotoAnded     = 0;
    mbJustLoaded    = 1;
    mbIsPlaying     = 1;
    mnIsCustomControl = (uint32_t)CustomControlState_Unknown;
    mnObjectClipActions = AptEventActionFlag_Invalid;

    SetNativeHash(new AptNativeHash(APT_OBJECTHASHSIZE));

    // by default set the __proto__ to MovieClip.prototype and then overwrite it if
    // class registration is found in AssociateInstToClass
    // This will take care of setting __proto__ for every movieclip derived from
    // AptCharacterSpriteInstBase which could be AptCharacterSpriteInst or
    // could be AptChracterAnimationInst (the very basic movieclip created in AptLinker::Update())
    AptValue * pMovieClip = gpGlobalGlobalObject->Lookup(StringPool::GetString(SC_MovieClip));
    AptValue * pPrototypeMovieClip = pMovieClip->GetNativeHashVirtual()->GetPrototype();
    GetNativeHash()->Set__Proto__(pPrototypeMovieClip);

}

/*F************************************************************************************************/
/*!
    Destructor

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        02/27/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptCharacterSpriteInstBase::~AptCharacterSpriteInstBase(void)
{
    mpClipActions = NULL;
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterSpriteInstBase::GetCreatedDynamic

    \Description
        returns true if the object was created dynamically

    \Input
        None.

    \Output     bool              -

    \Notes
        None.

    \Version    1.0        04/25/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
bool AptCharacterSpriteInstBase::GetCreatedDynamic(void) const
{
    return GetRenderItemSprite()->GetCreatedDynamic();
}
/*F************************************************************************************************/
/*!
    \Function    AptCharacterSpriteInstBase::SetCreatedDynamic

    \Description
        Sets whether of not the object was created dynamically.

    \Input      bNewValue         -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/25/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptCharacterSpriteInstBase::SetCreatedDynamic(bool bNewValue)
{
    GetRenderItemSpriteWritable()->SetCreatedDynamic(bNewValue);
}

/*F************************************************************************************************/
/*!
    Destroys all dependencies this object contains

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        03/17/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptCharacterSpriteInstBase::PreDestroy(void)
{
    mDisplayList.PreDestroy();
}

/*F************************************************************************************************/
/*!
    Sets the text value to the varible member for this text instance.

    \Input      pParent           -

    \Output     None.

    \Notes
        None.

    \Version
        1.0        03/24/04 (ONA) First Version
        1.1        04/21/04 (DC)  changed handling of localized strings. These should still set the
                                  text value to the $value. This forces the renderer to pick up
                                  the $ string.
*/
/************************************************************************************************F*/
void    AptCharacterTextInst::SetText(AptCIH * const pParent)
{
    const AptNativeString &varValue = GetVarValueConst();
    if (varValue.IsEmpty())
    {
        return;
    }

    if (varValue.c_str()[0] == '$')
    {
        SetTextValue(varValue);     // DC 2004/4/21 -- Still need to set mTextValue. (Bug #455)
        return;
    }

    // ONA 2004/3/24: This text is associated with a variable
    AptCIH *        pLocalParent = pParent;

    while (pLocalParent && !pLocalParent->IsSpriteInstBase())
    {
        if (pLocalParent->GetDisplayListParent() == NULL)
        {
            break;
        }
        pLocalParent = pLocalParent->GetDisplayListParent();
    }

    AptValue *  pValue = gAptActionInterpreter.getVariable(pLocalParent, NULL, &varValue);
    if (pValue->isUndefined())
    {
        //  The value is undefined, we assume it's because the variable doesn't exist
        //  But we have the same result if the variable contains undefined
        //  There is no way yet to detect between the two cases, this can be a difference between Flash and Apt

        //  We need to create the variable with the initialized text
        AptString * pString = AptString::Create();
        if(GetCharacterConst()->text.szInitialText != NULL)
        {
            pString->str = GetCharacterConst()->text.szInitialText;
        }
        else
        {
            pString->str = "";
        }

        SetTextValue( pString->str );
        if (gAptActionInterpreter.setVariable(pLocalParent, NULL, &varValue, pString) == false)
        {
            // ONA 2004/3/25: For an unknown reason we couldn't set the variable
            //  The string is allocated but nobody will keep a pointer on it
            //  If we don't delete the string here, the deferred release will catch it
        }
    }
    else
    {
        //  The value is defined, set the accordingly
        pValue->toString( GetTextValueWritable() );
    }
}

/*F************************************************************************************************/
/*!
    Updates the text value to the varible member for this text instance.

    \Input      pParent           -

    \Output     None.

    \Notes
        None.

    \Version
        1.0        03/24/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptCharacterTextInst::UpdateText(AptCIH * const pParent)
{
    const AptNativeString &varValue = GetVarValueConst();

    if (varValue.IsEmpty())
    {
        return;
    }

    if (varValue.c_str()[0] == '$')
    {
        return;
    }

    // ONA 2004/3/24: This text is associated with a variable
    AptNativeString strNewText;
    AptCIH *        pLocalParent = pParent;

    while (pLocalParent && !pLocalParent->IsSpriteInstBase())
    {
        if (pLocalParent->GetDisplayListParent() == NULL)
        {
            break;
        }
        pLocalParent = pLocalParent->GetDisplayListParent();
    }

    AptValue *  pValue = gAptActionInterpreter.getVariable(pLocalParent, NULL, &varValue);
    APT_ASSERT(pValue != NULL);

    if (pValue->isUndefined())          //  If the value was undefined, we use an empty string
    {
        if(GetCharacterConst()->text.szInitialText)
        {
            strNewText = GetCharacterConst()->text.szInitialText;
        }
        else
        {
            strNewText = "";
        }
    }
    else
    {
        // ONA 2004/3/24: There is an associated variable, use it
        pValue->toString(strNewText);
    }

    if (GetTextValueConst() != strNewText)
    {
        //  The text is different, update it
        SetTextValue( strNewText );

        // Note these can be done in the SetTextValue Functions
        ClearStateFlags(APT_TEXTFIELD_NONE);
        SetStateFlags(APT_TEXTFIELD_FUPDATE);
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::AptCharacterTextInst

    \Description
        Constructor

    \Input
        None.

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        09/13/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptCharacterTextInst::AptCharacterTextInst(AptCharacter *pCharacter) :
    AptCharacterInst(pCharacter)
{
    mnMaxScroll = 0;
    mfTextWidth = 0;
    mfTextHeight = 0;
    mfLength = 0;
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::~AptCharacterTextInst

    \Description
        Constructor

    \Input
        None.

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        09/13/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptCharacterTextInst::~AptCharacterTextInst(void)
{

}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterSpriteInstBase::SetBlendMode

    \Description
    Sets blendmode value

    \Input      newValue          - blend mode value - Flash only supports from 0-13

    \Output
    None.

    \Notes
    None.

    \Version    1.0        09/18/07 (ASK) First Version

*/
/************************************************************************************************F*/
void                  AptCharacterSpriteInstBase::SetBlendMode(uint32_t nBlendMode)
{
    GetRenderItemSpriteWritable()->SetBlendMode(nBlendMode);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterSpriteInstBase::GetBlendMode

    \Description
    Gets blendmode value

    \Input
    None

    \Output
    uint32_t - blend mode value

    \Notes
    None.

    \Version    1.0        09/18/07 (ASK) First Version

    */
/************************************************************************************************F*/
uint32_t   AptCharacterSpriteInstBase::GetBlendMode() const
{
    return GetRenderItemSprite()->GetBlendMode();
}
