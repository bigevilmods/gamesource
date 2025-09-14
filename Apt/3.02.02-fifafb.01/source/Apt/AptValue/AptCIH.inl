/*H************************************************************************************************/
/*!

    \File
        AptCIH.inl

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        04/13/05 (DC) First Version
        1.1        06/09/09 (DG) FogBug 1233: AptNativeHash change from Tiger10
        1.2        07/10/09 (RB)  Make GetFirstChild inline 
*/
/************************************************************************************************H*/


//-------------------------------------------------------------------------------------------------------------
// OK, Tons of inlined functions provided to give callers type safety. Function comments seem like a waste...

AptCharacterSpriteInst * AptCIH::GetSpriteInst()
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetSpriteInst();
}

AptCharacterSpriteInstBase * AptCIH::GetSpriteInstBase()
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetSpriteInstBase();
}

AptCharacterTextInst * AptCIH::GetDynamicTextInst()
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetDynamicTextInst();
}

AptCharacterStaticTextInst * AptCIH::GetStaticTextInst()
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetStaticTextInst();
}

AptCharacterMorphInst * AptCIH::GetMorphInst()
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetMorphInst();
}

AptCharacterButtonInst * AptCIH::GetButtonInst()
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetButtonInst();  // If there is no button compiled in we will always return false, but why use #defs...  :)
}

AptCharacterAnimationInst * AptCIH::GetAnimationInst()
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetAnimationInst();
}

AptCharacterShapeInst * AptCIH::GetShapeInst()
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetShapeInst();
}

// FIFA_BEGIN | DIVERGENCE | nmeyer | 2020-02-03 | Add support for returning image instances.
AptCharacterImageInst * AptCIH::GetImageInst()
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetImageInst();
}
// FIFA_END

AptCharacterInst * AptCIH::GetCharacterInst()
{
    return mpCharacterInst;
}

//-------------------------------------------------------------------------------------------------------------
// Const versions of the above get functions. 

const AptCharacterSpriteInst * AptCIH::GetSpriteInst() const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetSpriteInst();
}

const AptCharacterSpriteInstBase * AptCIH::GetSpriteInstBase() const 
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetSpriteInstBase();
}

const AptCharacterTextInst * AptCIH::GetDynamicTextInst() const 
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetDynamicTextInst();
}

const AptCharacterStaticTextInst * AptCIH::GetStaticTextInst() const 
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetStaticTextInst();
}

const AptCharacterMorphInst * AptCIH::GetMorphInst() const 
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetMorphInst();
}

const AptCharacterButtonInst * AptCIH::GetButtonInst() const 
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetButtonInst();
}

const AptCharacterAnimationInst * AptCIH::GetAnimationInst() const 
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetAnimationInst();
}

const AptCharacterShapeInst * AptCIH::GetShapeInst() const 
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetShapeInst();
}

// FIFA_BEGIN | DIVERGENCE | nmeyer | 2020-02-03 | Add support for returning image instances.
const AptCharacterImageInst * AptCIH::GetImageInst() const 
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetImageInst();
}
// FIFA_END

const AptCharacterInst * AptCIH::GetCharacterInst() const 
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetCharacterInst();
}

//-------------------------------------------------------------------------------------------------------------
// each function returns true if the character inst is the specified type.


bool                AptCIH::IsSpriteInst()      const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->IsSpriteInst();
}

bool                AptCIH::IsSpriteInstBase()  const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->IsSpriteInstBase();
}

bool                AptCIH::IsButtonInst()      const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->IsButtonInst();
}

bool                AptCIH::IsShapeInst()       const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->IsShapeInst();
}

bool                AptCIH::IsDynamicTextInst() const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->IsDynamicTextInst();
}

bool                AptCIH::IsStaticTextInst()  const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->IsStaticTextInst();
}

bool                AptCIH::IsMorphInst()       const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->IsMorphInst();
}

bool                AptCIH::IsAnimationInst()   const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->IsAnimationInst();
}

bool                AptCIH::IsLevelInst()       const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->IsLevelInst();
}

bool                AptCIH::IsImageInst() const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->IsImageInst();
}


bool AptCIH::IsNone() const    // (LEO) added for the new gpUndefinedCIH object
{
    APT_ASSERT(this);
    return getVtblIndex() == AptVFT_CIHNone;
}

bool AptCIH::IsCustomControlInst(void) const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->IsCustomControlInst();
}


// added new functions for UI effects
bool AptCIH::GetHasBlendMode()   const
{
    return mbHasBlendMode;
}

void AptCIH::SetHasBlendMode(uint32_t nBlendMode)
{
    mbHasBlendMode = nBlendMode;
}

bool AptCIH::GetHasFilterEffects()   const
{
    return mbHasFilters;
}

void AptCIH::SetHasFilterEffects(uint32_t nUIEffect)
{
    mbHasFilters = nUIEffect;
}

bool AptCIH::GetHasUIEffects() const
{
    return (mbHasBlendMode || mbHasFilters);
}

/*F************************************************************************************************/
/*!
    Gets the AptNativeHash of this instance.

    \Input      None.

    \Output     AptNativeHash *   - The AptNativeHash.

    \Notes
        This method is static. You should call this method instead of the virtual one.
        In fact, you should try to compile with this method, if it doesn't work, use the virtual.

    \Version
        1.0        03/07/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptNativeHash *    AptCIH::GetNativeHash() const
{
    const AptCharacterInst * pData = mpCharacterInst;

    if (pData)
    {
        return  (pData->GetNativeHash());
    }
    return  (NULL);
}

/*F************************************************************************************************/
/*!
    \Function    SetEventHandler
    
    \Description
        Sets the specified bit as being set. 
        
        This function actually has no knowledge of what the bits mean :). So it is actually up to 
        the caller to decide what they are used for. the CIH just stores the bits :)
    
    \Input      nEvent            - 
    
    \Output
        None.
        
    \Notes
    Event Handler Caching API. Allows us to cache whether or not certain event handlers are present without searching.

        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    \Version    1.1        06/09/09 (DG) FogBug 1233: AptNativeHash change from Tiger10
                                         use if instead of APT_ASSERT
*/
/************************************************************************************************F*/
void AptCIH::SetEventHandler(int nEvent)
{
    if(ContainsNativeHashVirtual())
    {
        GetNativeHash()->SetEventHandler(nEvent);
    }
}

/*F************************************************************************************************/
/*!
    \Function    RemoveEventHandler
    
    \Description
        Clears the specified bit as being set. 
        
        This function actually has no knowledge of what the bits mean :). So it is actually up to 
        the caller to decide what they are used for. the CIH just stores the bits :)
    
    \Input      nEvent            - 
    
    \Output
        None.
        
    \Notes
    Event Handler Caching API. Allows us to cache whether or not certain event handlers are present without searching.

        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    \Version    1.1        06/09/09 (DG) FogBug 1233: AptNativeHash change from Tiger10
                                         use if instead of APT_ASSERT    
*/
/************************************************************************************************F*/
void AptCIH::RemoveEventHandler(int nEvent)
{
    if(ContainsNativeHashVirtual())
    {
        GetNativeHash()->RemoveEventHandler(nEvent);
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::HasMouseEvent
    
    \Description
        looks to see if any mouse events are set and returns true if they are.
    
        Just a convenience function.
    \Input
        None.
        
    \Output     int               - 
    
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
int AptCIH::HasMouseEvent()
{
    return (HasEvent(AptEventActionFlag_MouseEvents));
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::SetIsInserted
    
    \Description
        Notifies the render tree manager that this is being added.
    
    \Input
        None.
        
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        04/19/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void                AptCIH::SetIsInserted()
{
    if(mpCharacterInst != NULL)
    {
        AptCharacterInst::ItemInserted(this);
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::SetDisplayListNext
    
    \Description
        Sets the next Display list item pointed to by this object.
    
    Also notifies the character inst. Might not seem right just yet, but no more time to clean at the 
    moment

    \Input      pNew              - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void                AptCIH::SetDisplayListNext(AptCIH* pNew)
{
    APT_ASSERT(mbInRemList == false);
    mpNext = pNew;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::SetDisplayListPrevious
    
    \Description
        Sets the display list pPrev pointer to the new item.
    
    \Input      pNew              - 
    
    \Output
        None.
        
    \Notes
        If this object was moved to the head of a list, we notify that the first child of the parent 
        (which is this) has changed.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void                AptCIH::SetDisplayListPrevious(AptCIH* pNew)
{
    APT_ASSERT(mbInRemList == false);
    mpPrev = pNew;
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::isMask
    
    \Description
        Returns true if CIH is itself a mask.
    
    \Input
        None.
        
    \Output     bool              - true if CIH is itself a mask.
    
    \Notes
        None.
        
    \Version    1.0        04/01/05 (ASK) First Version
    
*/
/************************************************************************************************F*/
bool AptCIH::IsMask() const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetIsMask();
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::hasMask
    
    \Description
        Returns if CIH has mask.
    
    \Input
        None.
        
    \Output     bool              - true if CIH has mask.
    
    \Notes
        None.
        
    \Version    1.0        04/01/05 (ASK) First Version
    
*/
/************************************************************************************************F*/
bool AptCIH::HasMask() const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetHasMask();
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::SetIsMask
    
    \Description
        Insert function's purpose here
        
    Insert your long description here.
    
    \Input      bNewValue         - 
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void AptCIH::SetIsMask(bool bNewValue, AptMatrix *pMatrix)
{
    APT_ASSERT(mpCharacterInst != NULL);
    mpCharacterInst->SetIsMask(bNewValue, pMatrix);
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::SetHasMask
    
    \Description
        Insert function's purpose here
        
    Insert your long description here.
    
    \Input
        None.
        
    \Output     bool              - 
    
    \Notes
        None.
        
    \Version    1.0        04/13/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
void AptCIH::SetHasMask(bool bNewValue, AptRenderItem *pMask)
{
    APT_ASSERT(mpCharacterInst != NULL);
    mpCharacterInst->SetHasMask(bNewValue, pMask);
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetDepth
    
    \Description
        Returns depth of CIH

    \Input
        None.
        
    \Output     int32_t           - Depth
    
    \Notes
        None.
        
    \Version    1.0        04/01/05 (ASK) First Version
    
*/
/************************************************************************************************F*/
int32_t AptCIH::GetDepth() const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetDepth() ;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::SetDepth
    
    \Description
        Sets the depth of movieclip
    
    \Input      nDepth            - Depth to be set.
    
    \Output
        None.
        
    \Notes
        None.
        
    \Version    1.0        04/01/05 (ASK) First Version
    
*/
/************************************************************************************************F*/
void        AptCIH::SetDepth(uint32_t nDepth)
{
    APT_ASSERT(mpCharacterInst != NULL);
    mpCharacterInst->SetDepth(nDepth) ;
}


/*F************************************************************************************************/
/*!
\Function    AptCIH::SetDisplayListParent

\Description
Sets the new Display list parent pointer.

\Input      pNew              - 

\Output
None.

\Notes
None.

\Version    1.0        04/13/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void    AptCIH::SetDisplayListParent(AptCIH* pNew)
{
    mpParent = pNew;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetPositionMatrixWritable
    
    \Description
        Returns a non-const pointer to position matrix.
    
    \Input
        None.
        
    \Output     AptMatrix *          -  position matrix
    
    \Notes
        None.
        
    \Version    1.0        04/01/05 (ASK) First Version
    
*/
/************************************************************************************************F*/
AptMatrix *AptCIH::GetPositionMatrixWritable()
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetPositionMatrixWritable();
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetPositionMatrixConst
    
    \Description
        Returns a const pointer to position matrix 
    
    \Input
        None.
        
    \Output     const AptMatrix *    -  position matrix
    
    \Notes
        None.
        
    \Version    1.0        04/01/05 (ASK) First Version
    
*/
/************************************************************************************************F*/
const AptMatrix *AptCIH::GetPositionMatrixConst() const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetPositionMatrixConst();
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetColorMatrixWritable
    
    \Description
        Returns a pointer to color matrix which can be used
        for modifying the color matrix

    \Input
        None.
        
    \Output     AptCXForm *          -  color matrix 
    
    \Notes
        None.
        
    \Version    1.0        04/01/05 (ASK) First Version
    
*/
/************************************************************************************************F*/
AptCXForm *AptCIH::GetColorMatrixWritable()
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetColorMatrixWritable();
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetColorMatrixConst
    
    \Description
        Returns a const pointer to color matrix 

    \Input
        None.
        
    \Output     const AptCXForm *          -    cxform inside RenderItem.
    
    \Notes
        None.
        
    \Version    1.0        04/01/05 (ASK) First Version
    
*/
/************************************************************************************************F*/
const AptCXForm *AptCIH::GetColorMatrixConst() const
{
    APT_ASSERT(mpCharacterInst != NULL);
    return mpCharacterInst->GetColorMatrixConst() ;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetIsPlaying
    
    \Description
        Returns the playing flag
    
    \Input
        None.
        
    \Output     bool              - 
    
    \Notes
        None.
        
    \Version    1.0        08/23/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
bool AptCIH::GetIsPlaying() const
{
    return GetSpriteInstBase()->mbIsPlaying == 1;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetDirtyState
    
    \Description
        Returns dirty state
    
    \Input
        None.
        
    \Output     bool              - 
    
    \Notes
        None.
        
    \Version    1.0        08/24/05 (LEO) First Version
    
*/
/************************************************************************************************F*/
bool AptCIH::GetDirtyState() const
{
    return mbDirty == 1;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetFirstChild
    
    \Description
        Returns the head item of the child display list (or NULL).
    
    \Input
        
    \Output     AptCIH *             -  pointer to first child AptCIH
    
    \Notes
        None.
        
    \Version    1.0        04/18/05 (DC) Bringin' it to ya
    
*/
/************************************************************************************************F*/
const AptCIH * AptCIH::GetFirstChild() const
{
    if(IsSpriteInstBase())
    {
        const AptCharacterSpriteInstBase *base = GetSpriteInstBase();
        if(base->mDisplayList.getState())
        {
            const AptDisplayListState *state = base->mDisplayList.getState();
            return state->GetFirstItem();
        }
    }
#if defined APT_USE_BUTTONS
    else if (IsButtonInst())
    {
        const AptCharacterButtonInst *pButtonInst = GetButtonInst();
        if (pButtonInst->mDisplayList.getState())
        {
            const AptDisplayListState *state = pButtonInst->mDisplayList.getState();
            return state->GetFirstItem();
        }
    }
#endif
    return NULL;
}

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

