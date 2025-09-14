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
    \Version    1.1        12/15/05 (LEO) Added 3D support
    \Version    1.2        09/20/07 (ASK) Added get/set functions for accessing blendMode property
    \Version    1.3        07/09/09 (DG)  Added AptCharacterTextInst::SetLeading and AptCharacterTextInst::GetLeading methods.
    \Version    1.4        09/28/09 (MZ)  Added AptCharacterTextInst::SetTracking and AptCharacterTextInst::GetTracking methods.
*/
/************************************************************************************************H*/


/*** Include files ********************************************************************************/
#include "AptRenderItem.h" // Needs This
//#include "AptRenderItem.inl" // Needs This
#include "AptBCRenderTree.h"
#include "AptTarget.h"
/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Function Prototypes **************************************************************************/

/*** Variables ************************************************************************************/

// Private variables

// Public variables

/*** Private Functions ****************************************************************************/

/*** Public Functions *****************************************************************************/

//----------------------------------------------------------------------------------------------
// Get Render item const and writable are provided first because other call these and expect them to be inlined.
const AptRenderItem           *AptCharacterInst::GetRenderItem(void)            const 
{ 
    APT_ASSERT(mpRenderItem); 
    return mpRenderItem; 
}

//----------------------------------------------------------------------------------------------
// Give out our character inst type casted
const AptRenderItemSprite     *AptCharacterInst::GetRenderItemSprite(void)      const 
{ 
    APT_ASSERT(IsSpriteInst() || IsAnimationInst());      
    return (const AptRenderItemSprite *)  GetRenderItem(); 
}

const AptRenderItemDynamicText*AptCharacterInst::GetRenderItemDynamicText(void) const 
{ 
    APT_ASSERT(IsDynamicTextInst()); 
    return (const AptRenderItemDynamicText *) GetRenderItem(); 
}

const AptRenderItemStaticText *AptCharacterInst::GetRenderItemStaticText(void)  const 
{ 
    APT_ASSERT(IsStaticTextInst());  
    return (const AptRenderItemStaticText *)this; 
}

const AptRenderItemMorph      *AptCharacterInst::GetRenderItemMorph(void)       const 
{ 
    APT_ASSERT(IsMorphInst());       
    return (const AptRenderItemMorph *) GetRenderItem(); 
}

const AptRenderItemButton     *AptCharacterInst::GetRenderItemButton(void)      const 
{ 
    APT_ASSERT(IsButtonInst());      
    return (const AptRenderItemButton *)  GetRenderItem(); 
}

const AptRenderItemAnimation  *AptCharacterInst::GetRenderItemAnimation(void)   const 
{ 
    APT_ASSERT(IsAnimationInst());   
    return (const AptRenderItemAnimation *) GetRenderItem(); 
}

const AptRenderItemShape      *AptCharacterInst::GetRenderItemShape(void)       const 
{ 
    APT_ASSERT(IsShapeInst());       
    return (const AptRenderItemShape *) GetRenderItem(); 
}

const AptRenderItemCustomControl      *AptCharacterInst::GetRenderItemCustomControl(void)       const 
{ 
    APT_ASSERT(IsCustomControlInst());       
    return (const AptRenderItemCustomControl *) GetRenderItem(); 
}

AptRenderItemSprite     *AptCharacterInst::GetRenderItemSpriteWritable(void)      
{ 
    APT_ASSERT(IsSpriteInst());      
    return (AptRenderItemSprite *) GetRenderItemWritable(); 
}

AptRenderItemDynamicText*AptCharacterInst::GetRenderItemDynamicTextWritable(void) 
{ 
    APT_ASSERT(IsDynamicTextInst()); 
    return (AptRenderItemDynamicText *) GetRenderItemWritable(); 
}

AptRenderItemStaticText *AptCharacterInst::GetRenderItemStaticTextWritable(void)  
{ 
    APT_ASSERT(IsStaticTextInst());  
    return (AptRenderItemStaticText *) GetRenderItemWritable(); 
}

AptRenderItemMorph      *AptCharacterInst::GetRenderItemMorphWritable(void)       
{ 
    APT_ASSERT(IsMorphInst());       
    return (AptRenderItemMorph *) GetRenderItemWritable(); 
}

AptRenderItemButton     *AptCharacterInst::GetRenderItemButtonWritable(void)      
{ 
    APT_ASSERT(IsButtonInst());      
    return (AptRenderItemButton *) GetRenderItemWritable(); 
}

AptRenderItemAnimation  *AptCharacterInst::GetRenderItemAnimationWritable(void)   
{ 
    APT_ASSERT(IsAnimationInst());   
    return (AptRenderItemAnimation *) GetRenderItemWritable(); 
}

AptRenderItemShape      *AptCharacterInst::GetRenderItemShapeWritable(void)       
{ 
    APT_ASSERT(IsShapeInst());       
    return (AptRenderItemShape *) GetRenderItemWritable(); 
}

AptRenderItemCustomControl      *AptCharacterInst::GetRenderItemCustomControlWritable(void)       
{ 
    APT_ASSERT(IsCustomControlInst());       
    return (AptRenderItemCustomControl *) GetRenderItemWritable(); 
}

/*F************************************************************************************************/
/*!
    \Function    &AptCharacterInst::GetPositionMatrixConst
    
    \Description
        returns a const reference to the position matrix. 
    
    \Input
        None.
        
    \Output     const AptMatrix      -  
    
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
const AptMatrix     *AptCharacterInst::GetPositionMatrixConst(void) const
{
    return GetRenderItem()->GetPositionMatrixConst();
}

/*F************************************************************************************************/
/*!
    \Function    &AptCharacterInst::GetColorMatrixConst
    
    \Description
        returns a const reference to this object's color transform matrix
    
    \Input
        None.
        
    \Output     const AptCXForm      -  
    
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
const AptCXForm     *AptCharacterInst::GetColorMatrixConst(void)   const
{
    return GetRenderItem()->GetColorMatrixConst();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterInst::GetClipDepth
    
    \Description
        returns the clip depth of this item.
    
    \Input
        None.
        
    \Output     int32_t                  -  
    
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
int32_t                 AptCharacterInst::GetClipDepth(void)          const
{
    return GetRenderItem()->GetClipDepth();
}

#if defined(APT_3D)
float AptCharacterInst::GetZ(void) const
{
	return GetRenderItem()->GetZPosition();
}

void AptCharacterInst::SetZ(float z)
{
	GetRenderItemWritable()->SetZPosition(z);
}

float AptCharacterInst::GetZScale(void) const
{
	return GetRenderItem()->GetZScale();
}

void AptCharacterInst::SetZScale(float zScale)
{
	GetRenderItemWritable()->SetZScale(zScale);
}

float AptCharacterInst::GetYRot(void) const
{
	return GetRenderItem()->GetYRotation();
}

void AptCharacterInst::SetYRot(float yRot)
{
	GetRenderItemWritable()->SetYRotation(yRot);
}

float AptCharacterInst::GetXRot(void) const
{
	return GetRenderItem()->GetXRotation();
}

void AptCharacterInst::SetXRot(float xRot)
{
	GetRenderItemWritable()->SetXRotation(xRot);
}
#endif  // #if defined(APT_3D)

/*F************************************************************************************************/
/*!
    \Function    &AptCharacterInst::GetPositionMatrixWritable
    
    \Description
        returns a writable reference to the position matrix.
    
    \Input
        None.
        
    \Output     AptMatrix           -   
    
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
AptMatrix          *AptCharacterInst::GetPositionMatrixWritable(void)
{
    return GetRenderItemWritable()->GetPositionMatrixWritable();
}

/*F************************************************************************************************/
/*!
    \Function    &AptCharacterInst::GetColorMatrixWritable
    
    \Description
        returns a writable pointer to this object's color transformation matrix
    
    \Input
        None.
        
    \Output     AptCXForm           -   
    
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
AptCXForm          *AptCharacterInst::GetColorMatrixWritable(void)
{
    return GetRenderItemWritable()->GetColorMatrixWritable();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterInst::SetClipDepth
    
    \Description
        sets a new clip depth for the object.
    
    \Input      nDepth            - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void                AptCharacterInst::SetClipDepth(int32_t nDepth)
{
    GetRenderItemWritable()->SetClipDepth(nDepth );
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterInst::OwnerNextSiblingChanged
    
    \Description
        Notifies that the passed object has been moved.
    
    \Input      pItemOwner        - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void AptCharacterInst::ItemMoved( AptCIH * pItemOwner)
{
    GetTargetSim()->GetRenderManager()->Update_ItemMoved( pItemOwner, gnCurrUpdateTick);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterInst::OwnerRemoved
    
    \Description
        Notifies what the passed item was just inserted.
    
    \Input      pItemOwner        - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        04/19/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void AptCharacterInst::ItemInserted(AptCIH * pItemOwner)
{
    GetTargetSim()->GetRenderManager()->Update_ItemInserted( pItemOwner, gnCurrUpdateTick);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterInst::CloneItem
    
    \Description
        request that we clone this character inst.
    
    \Input      pJoeShmoe         - 
    \Input      *pWantsJoesRenderState- 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void AptCharacterInst::CloneItem(         const AptCIH * pJoeShmoe, AptCIH *pWantsJoesRenderState)
{
    GetTargetSim()->GetRenderManager()->Update_CloneItem( pJoeShmoe, pWantsJoesRenderState, gnCurrUpdateTick);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterInst::SetRootItem
    
    \Description
        Notifies the character inst that the root render able object has changed.
    
        This in turn notifies the render tree manager
        
    
    \Input      pRoot             - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void AptCharacterInst::SetRootItem(             AptCIH * pRoot)
{
    GetTargetSim()->GetRenderManager()->Update_SetRootItem( pRoot, gnCurrUpdateTick);
}

/*F************************************************************************************************/
/*!
    \Function    GetDepth
    
    \Description
        returns the depth of the current item.
    
    \Input      newDepth          - 
    \Input      nCurrentUpdateTick- 
    
    \Output
        None.
        
    \Notes
        none.

    \Version    1.0        03/30/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
int32_t AptCharacterInst::GetDepth(void) const
{
    return GetRenderItem()->GetDepth();
}


/*F************************************************************************************************/
/*!
    \Function    GetCharacter
    
    \Description
        returns the Character inside this item.
        
    \Input
        None.
        
    \Output     const AptCharacter*  -  
    
    \Notes
        returns a const pointer.
        
    \Version    1.0        03/30/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
const AptCharacter* AptCharacterInst::GetCharacterConst(void)  const
{
    return GetRenderItem()->GetCharacterConst();
}

/*F************************************************************************************************/
/*!
    \Function    GetCharacter
    
    \Description
        returns the Character inside this item.
        
    \Input
        None.
        
    \Output     const AptCharacter*  -  
    
    \Notes
        returns a const pointer.
        
    \Version    1.0        03/30/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
AptCharacter* AptCharacterInst::GetCharacterWritable(void)
{
    return GetRenderItemWritable()->GetCharacterWritable();
}


/*F************************************************************************************************/
/*!
    \Function    GetIsMask
    
    \Description
        returns true if this item is a mask.
    
    \Input
        None.
        
    \Output     bool                     -  
    
    \Notes
        None.
        
    \Version    1.0        03/30/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
bool AptCharacterInst::GetIsMask(void)         const
{
    return GetRenderItem()->GetIsMask();
}

/*F************************************************************************************************/
/*!
    \Function    GetHasMask
    
    \Description
        returns true if the item has a mask dynamically applied.
    
    \Input
        None.
        
    \Output     bool                     -  
    
    \Notes
        None.
        
    \Version    1.0        03/30/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
bool AptCharacterInst::GetHasMask(void)        const
{
    return GetRenderItem()->GetHasMask();
}

/*F************************************************************************************************/
/*!
    \Function    GetIsVisible
    
    \Description
        returns true if the item is visible.
    
    \Input
        None.
        
    \Output     bool                     -  
    
    \Notes
        None.
        
    \Version    1.0        03/30/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
bool AptCharacterInst::GetIsVisible(void)      const
{
    return GetRenderItem()->GetIsVisible();
}

/*F************************************************************************************************/
/*!
    \Function    SetDepth
    
    \Description
        returns the depth of the current item.
    
    \Input      newDepth          - 
    \Input      nCurrentUpdateTick- 
    
    \Output
        None.
        
    \Notes
        none.

    \Version    1.0        03/30/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void                    AptCharacterInst::SetDepth(int32_t newDepth)
{
    GetRenderItemWritable()->SetDepth(newDepth);
}


/*F************************************************************************************************/
/*!
    \Function    SetCharacter
    
    \Description
        Set a new character for the item. 
    
    \Input      pData             - 
    \Input      nCurrentUpdateTick- 
    
    \Output
        None.
        
    \Notes
        Not sure if this functgion is needed.
        
    \Version    1.0        04/11/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void AptCharacterInst::SetCharacter(AptCharacter * pCharacter)
{
    GetRenderItemWritable()->SetCharacter(pCharacter);
}

/*F************************************************************************************************/
/*!
    \Function    IncIsMask
    
    \Description
        increments the isMask property, when isMask becomes zero the movie is no longer a mask.
        
    \Input      nCurrentUpdateTick- 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        03/30/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void AptCharacterInst::SetIsMask(bool bIsMask, AptMatrix *pMatrix)
{
    GetRenderItemWritable()->SetIsMask(bIsMask, pMatrix);
}
/*F************************************************************************************************/
/*!
    \Function    SetHasMask
    
    \Description
        sets whether the current item has a dynamic mask applied.
    
    \Input      newState          - 
    \Input      nCurrentUpdateTick- 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        03/30/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void AptCharacterInst::SetHasMask(bool bNewState, AptRenderItem *pMask)
{
    GetRenderItemWritable()->SetHasMask(bNewState, pMask);
}
/*F************************************************************************************************/
    /*!
        \Function    SetIsVisible
        
    \Description
        sets whether the item is visible at all.
    
    \Input      newState          - 
    \Input      nCurrentUpdateTick- 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        03/30/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void AptCharacterInst::SetIsVisible(bool bNewState)
{
    GetRenderItemWritable()->SetIsVisible(bNewState);
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterInst::DestroyGCPointers
    
    \Description
        Removes dependencies this object contains it does
        
    \Input
        None.
        
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void AptCharacterInst::DestroyGCPointers(void)
{
    if(mpRenderItem != NULL)
    {
        GetTargetSim()->GetRenderManager()->Update_ItemRemoved( GetRenderItemWritable(), gnCurrUpdateTick );
    }
    
    if (mpNativeHash != NULL)
    {
        mpNativeHash->DestroyGCPointers();
        delete mpNativeHash;
        mpNativeHash = NULL;
    }

}

/*F************************************************************************************************/
/*!
    \Function    &AptCharacterTextInst::GetTextValueConst
    
    \Description
        Returns a const text value for this text instance
    
    \Input
        None.
        
    \Output     const AptNativeString      -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
const AptNativeString     &AptCharacterTextInst::GetTextValueConst(void) const
{
    return GetRenderItemDynamicText()->GetTextValueConst();
}

/*F************************************************************************************************/
/*!
    \Function    &AptCharacterTextInst::GetTextValueWritable
    
    \Description
        Returns a text value for this text instance
        
    \Input
        None.
        
    \Output     AptNativeString            -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
AptNativeString           &AptCharacterTextInst::GetTextValueWritable(void)
{
    return GetRenderItemDynamicTextWritable()->GetTextValueWritable();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetTextValue
    
    \Description
        Sets the text value for this instance
    
    \Input      newValue          - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                       AptCharacterTextInst::SetTextValue(const AptNativeString & newValue)
{
    GetRenderItemDynamicTextWritable()->SetTextValue(newValue);
}

/*F************************************************************************************************/
/*!
    \Function    &AptCharacterTextInst::GetVarValueConst
    
    \Description
        Returns const string of the variable member for this instance
    
    \Input
        None.
        
    \Output     const AptNativeString      -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
const AptNativeString     &AptCharacterTextInst::GetVarValueConst(void) const
{
    return GetRenderItemDynamicText()->GetVarValueConst();
}

/*F************************************************************************************************/
/*!
    \Function    &AptCharacterTextInst::GetVarValueWritable
    
    \Description
        Returns string of the variable member for this instance
    
    \Input
        None.
        
    \Output     AptNativeString            -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
AptNativeString           &AptCharacterTextInst::GetVarValueWritable(void)
{
    return GetRenderItemDynamicTextWritable()->GetVarValueWritable();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetVarValue
    
    \Description
        Sets the variable value for this instance
    
    \Input      newValue          - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                      AptCharacterTextInst::SetVarValue(const AptNativeString & newValue)
{
    GetRenderItemDynamicTextWritable()->SetVarValue(newValue);
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetRenderable()
    
    \Description
        returns the const zid object

    \Input
        None.
        
    \Output     const AptAssetString   -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
AptRenderableString *AptCharacterTextInst::GetRenderable(void)
{
    return const_cast<AptRenderableString *>(GetRenderItemDynamicText()->GetRenderable());
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetZID
    
    \Description
        Set the zID

    \Input      newValue          - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetZID(const AptAssetString& newValue)
{
    GetRenderItemDynamicTextWritable()->SetZID(newValue);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetTextColor
    
    \Description
        Returns the text color
    
    \Input
        None.
        
    \Output     uint32_t               -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
uint32_t              AptCharacterTextInst::GetTextColor(void) const
{
	return GetRenderItemDynamicText()->GetTextColor();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetTextColor
    
    \Description
        Sets the text color

    \Input      newTextColor      - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetTextColor(uint32_t newTextColor)
{
	GetRenderItemDynamicTextWritable()->SetTextColor(newTextColor);
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetMaxScroll
    
    \Description
        Returns the maxScroll

    \Input
        None.
        
    \Output     uint32_t               -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
uint32_t              AptCharacterTextInst::GetMaxScroll(void) const
{
    return mnMaxScroll;
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetMaxScroll
    
    \Description
        Sets maxScroll

    \Input      maxScroll         - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetMaxScroll(uint32_t maxScroll)
{
    mnMaxScroll = (maxScroll);
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetScroll
    
    \Description
        Returns the scroll value

    \Input
        None.
        
    \Output     uint32_t               -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
uint32_t              AptCharacterTextInst::GetScroll(void) const
{
	return GetRenderItemDynamicText()->GetScroll();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetScroll
    
    \Description
        Sets the scroll value

    \Input      newScroll         - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetScroll(uint32_t newScroll)
{
	GetRenderItemDynamicTextWritable()->SetScroll(newScroll);
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetBackgroundColor
    
    \Description
        Returns the background color
    
    \Input
        None.
        
    \Output     uint32_t               -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
uint32_t              AptCharacterTextInst::GetBackgroundColor(void) const
{
	return GetRenderItemDynamicText()->GetBackgroundColor();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetBackgroundColor
    
    \Description
        Sets the background color
        
    \Input      newBackgroundColor- 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetBackgroundColor(uint32_t newBackgroundColor)
{
	GetRenderItemDynamicTextWritable()->SetBackgroundColor(newBackgroundColor);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetBorderColor
    
    \Description
        Returns the border color
        
    \Input
        None.
        
    \Output     uint32_t               -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
uint32_t              AptCharacterTextInst::GetBorderColor(void) const
{
	return GetRenderItemDynamicText()->GetBorderColor();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetBorderColor
    
    \Description
        Sets the border color
        
    \Input      newBackgroundColor- 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetBorderColor(uint32_t newBackgroundColor)
{
	GetRenderItemDynamicTextWritable()->SetBorderColor(newBackgroundColor);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetBoxAlignment
    
    \Description
        Returns the box alignment
    
    \Input
        None.
        
    \Output     AptStringAlignment     -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
AptStringAlignment    AptCharacterTextInst::GetBoxAlignment(void) const
{
	return GetRenderItemDynamicText()->GetBoxAlignment();
}
 
/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetBoxAlignment
    
    \Description
        Sets the box alignment
    
    \Input      newBoxAlignment   - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetBoxAlignment(AptStringAlignment newBoxAlignment)
{
	GetRenderItemDynamicTextWritable()->SetBoxAlignment(newBoxAlignment);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetAlignment
    
    \Description
        Returns the alignment
        
    \Input
        None.
        
    \Output     AptStringAlignment     -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
AptStringAlignment    AptCharacterTextInst::GetAlignment(void) const
{
	return GetRenderItemDynamicText()->GetAlignment();
}
 
/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetAlignment
    
    \Description
        Sets the alignment
    
    \Input      newAlignment      - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetAlignment(AptStringAlignment newAlignment)
{
	GetRenderItemDynamicTextWritable()->SetAlignment(newAlignment);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetTextWidth
    
    \Description
        Returns the textWidth
    
    \Input
        None.
        
    \Output     float_t               - 
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
float_t              AptCharacterTextInst::GetTextWidth(void) const
{
    return mfTextWidth;
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetTextWidth
    
    \Description
        Sets the textWidth
    
    \Input      newTextWidth      - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetTextWidth(float_t newTextWidth)
{
    mfTextWidth = (newTextWidth);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetTextHeight
    
    \Description
        Returns the textHeight
    
    \Input
        None.
        
    \Output     float_t               - 
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
float_t              AptCharacterTextInst::GetTextHeight(void) const
{
    return mfTextHeight;
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetTextHeight
    
    \Description
        Sets the textHeight
    
    \Input      newTextHeight     - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetTextHeight(float_t newTextHeight)
{
    mfTextHeight = (newTextHeight);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetLength
    
    \Description
        Returns the length
   
    \Input
        None.
        
    \Output     float_t               - 
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
float_t              AptCharacterTextInst::GetLength(void) const
{
    return mfLength;
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetLength
    
    \Description
        Sets the length
    
    \Input      newLength         - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetLength(float_t newLength)
{
    mfLength = (newLength);
}

/*F************************************************************************************************/
/*!
    \Function    &AptCharacterTextInst::GetBoundsConst
    
    \Description
        Returns the const bound values
        
    \Input
        None.
        
    \Output     const AptRect         - 
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
const AptRect        &AptCharacterTextInst::GetBoundsConst(void) const
{
	return GetRenderItemDynamicText()->GetBoundsConst();
}

/*F************************************************************************************************/
/*!
    \Function    &AptCharacterTextInst::GetBoundsWritable
    
    \Description
        Returns the bound values
        
    \Input
        None.
        
    \Output     AptRect               - 
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
AptRect              &AptCharacterTextInst::GetBoundsWritable(void)
{
	return GetRenderItemDynamicTextWritable()->GetBoundsWritable();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetBounds
    
    \Description
        Sets the bound values
    
    \Input      &newBounds        - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetBounds(const AptRect &newBounds)
{
	GetRenderItemDynamicTextWritable()->SetBounds(newBounds);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetFontSize
    
    \Description
        Returns the font size
    
    \Input
        None.
        
    \Output     float_t                  -  
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
float_t                 AptCharacterTextInst::GetFontSize(void) const
{
	return GetRenderItemDynamicText()->GetFontSize();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetFontSize
    
    \Description
        Sets the font size
    
    \Input      newFontSize       - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetFontSize(float_t newFontSize)
{
	GetRenderItemDynamicTextWritable()->SetFontSize(newFontSize);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetFontID
    
    \Description
        Returns the font id
    
    \Input
        None.
        
    \Output     int32_t                    -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
int32_t                   AptCharacterTextInst::GetFontID(void) const
{
	return GetRenderItemDynamicText()->GetFontID();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetTextFormatConst
    
    \Description
        Returns the const textFormat
    
    \Input
        None.
        
    \Output     const TextFormat *     -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
const TextFormat *    AptCharacterTextInst::GetTextFormatConst(void) const
{
	return GetRenderItemDynamicText()->GetTextFormatConst();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetTextFormatWritable
    
    \Description
        Returns the textFormat
    
    \Input
        None.
        
    \Output     TextFormat *           -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
TextFormat *          AptCharacterTextInst::GetTextFormatWritable(void)
{
	return GetRenderItemDynamicTextWritable()->GetTextFormatWritable();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetTextFormat
    
    \Description
        Sets the textFormat
    
    \Input      pNewFormat        - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetTextFormat(TextFormat * pNewFormat)
{
	GetRenderItemDynamicTextWritable()->SetTextFormat(pNewFormat);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::ClearStateFlags
    
    \Description
        Clears the flag from the textField state flags
    
    \Input      uFlags            - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::ClearStateFlags(uint32_t uFlags)
{
	GetRenderItemDynamicTextWritable()->ClearStateFlags(uFlags);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetStateFlags
    
    \Description
        Sets the flag in the textField state flags

    \Input      uFlags            - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetStateFlags(uint32_t uFlags)
{
	GetRenderItemDynamicTextWritable()->SetStateFlags(uFlags);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetStateFlags
    
    \Description
        Returns textField state flags
    
    \Input
        None.
        
    \Output     uint32_t               -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
uint32_t              AptCharacterTextInst::GetStateFlags(void) const
{
	return GetRenderItemDynamicText()->GetStateFlags();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetCreatedDynamic
    
    \Description
        Returns flag if this was created dynamically or not

    \Input
        None.
        
    \Output     bool                   -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
bool                  AptCharacterTextInst::GetCreatedDynamic(void) const
{
	return GetRenderItemDynamicText()->GetCreatedDynamic();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetCreatedDynamic
    
    \Description
        Sets if this was created dynamically or not

    \Input      newValue          - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetCreatedDynamic(bool newValue)
{
	GetRenderItemDynamicTextWritable()->SetCreatedDynamic(newValue);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetDrawsBorder
    
    \Description
        Returns is this draws border

    \Input
        None.
        
    \Output     bool                   -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
bool                  AptCharacterTextInst::GetDrawsBorder(void) const
{
	return GetRenderItemDynamicText()->GetDrawsBorder();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetDrawsBorder
    
    \Description
        Sets if this should draw a border

    \Input      bDrawsBorder      - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetDrawsBorder(bool bDrawsBorder)
{
	GetRenderItemDynamicTextWritable()->SetDrawsBorder(bDrawsBorder);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetDrawsBackground
    
    \Description
        Returns if this draws a background color

    \Input
        None.
        
    \Output     bool                   -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
bool                  AptCharacterTextInst::GetDrawsBackground(void) const
{
	return GetRenderItemDynamicText()->GetDrawsBackground();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetDrawsBackground
    
    \Description
        Sets if this should draw a background

    \Input      bDrawsBackground  - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetDrawsBackground(bool bDrawsBackground)
{
	GetRenderItemDynamicTextWritable()->SetDrawsBackground(bDrawsBackground);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetMouseWheelEnabled
    
    \Description
        Returns if this has MouseWheelEnabled

    \Input
        None.
        
    \Output     bool                   -    
    
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
bool                  AptCharacterTextInst::GetMouseWheelEnabled(void) const
{
	return GetRenderItemDynamicText()->GetMouseWheelEnabled();
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetMouseWheelEnabled
    
    \Description
        Sets if this has MouseWheelEnabled
    
    \Input      newValue          - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        06/09/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetMouseWheelEnabled(bool newValue)
{
	GetRenderItemDynamicTextWritable()->SetMouseWheelEnabled(newValue);
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetWordWrap
    
    \Description
        Returns if this has WordWrap enabled

    \Input
        None.
        
    \Output     bool                   -    
    
    \Notes
        None.
        
    \Version    1.0        03/07/06 (RB) First Version
    
*/
/************************************************************************************************F*/
bool                  AptCharacterTextInst::GetWordWrap(void) const
{
	return GetRenderItemDynamicText()->GetWordWrap();
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetWordWrap
    
    \Description
        Sets if this has WordWrap enabled
    
    \Input      newValue          - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        03/07/06 (RB) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetWordWrap(bool newValue)
{
	GetRenderItemDynamicTextWritable()->SetWordWrap(newValue);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetMultiline
    
    \Description
        Returns if this has Multiline enabled

    \Input
        None.
        
    \Output     bool                   -    
    
    \Notes
        None.
        
    \Version    1.0        03/07/06 (RB) First Version
    
*/
/************************************************************************************************F*/
bool                  AptCharacterTextInst::GetMultiline(void) const
{
	return GetRenderItemDynamicText()->GetMultiline();
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetMultiline
    
    \Description
        Sets if this has Multiline enabled
    
    \Input      newValue          - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        03/07/06 (RB) First Version
    
*/
/************************************************************************************************F*/
void                  AptCharacterTextInst::SetMultiline(bool newValue)
{
	GetRenderItemDynamicTextWritable()->SetMultiline(newValue);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetBlendMode

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
void                  AptCharacterTextInst::SetBlendMode(uint32_t nBlendMode)
{
    GetRenderItemDynamicTextWritable()->SetBlendMode(nBlendMode);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetBlendMode

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
uint32_t   AptCharacterTextInst::GetBlendMode() const
{
    return GetRenderItemDynamicText()->GetBlendMode();
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetLeading

    \Description
    Gets leading space value

    \Input      
    None

    \Output
        int - leading space value

    \Notes
    None.

    \Version    1.0        07/09/09 (DG) First Version

    */
/************************************************************************************************F*/
int AptCharacterTextInst::GetLeading() const
{
    return GetRenderItemDynamicText()->GetLeading();
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetLeading

    \Description
    Sets leading space value

    \Input      leading  - leading space value

    \Output
    None.

    \Notes
    None.

    \Version    1.0        07/09/09 (DG) First Version

    */
/************************************************************************************************F*/
void AptCharacterTextInst::SetLeading(const int leading)
{
    GetRenderItemDynamicTextWritable()->SetLeading(leading);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::GetTracking

    \Description
    Gets tracking space value

    \Input      
    None

    \Output
        int - tracking space value

    \Notes
    None.

    \Version    1.0        09/28/09 (MZ) First Version

    */
/************************************************************************************************F*/
int AptCharacterTextInst::GetTracking() const
{
    return GetRenderItemDynamicText()->GetTracking();
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterTextInst::SetTracking

    \Description
    Sets tracking space value

    \Input      tracking  - tracking space value

    \Output
    None.

    \Notes
    None.

    \Version    1.0        07/09/09 (DG) First Version

    */
/************************************************************************************************F*/
void AptCharacterTextInst::SetTracking(const int tracking)
{
    GetRenderItemDynamicTextWritable()->SetTracking(tracking);
}


#if defined APT_USE_BUTTONS
/*F************************************************************************************************/
/*!
    \Function    AptCharacterButtonInst::SetBlendMode

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
void                  AptCharacterButtonInst::SetBlendMode(uint32_t nBlendMode)
{
    GetRenderItemButtonWritable()->SetBlendMode(nBlendMode);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterButtonInst::GetBlendMode

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
uint32_t   AptCharacterButtonInst::GetBlendMode() const
{
    return GetRenderItemButton()->GetBlendMode();
}
#endif
