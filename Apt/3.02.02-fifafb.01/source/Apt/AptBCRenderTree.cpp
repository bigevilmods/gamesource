/*H************************************************************************************************/
/*!

    \File    AptRenderTree.cpp

    \Description
        Hold Implementation for AptRenderTreeManager class.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        03/30/05 (DC) Bringin' it to ya

*/
/************************************************************************************************H*/

/*** Include files ********************************************************************************/
#include "AptCIH.h"
#include "AptBCRenderTree.h"
#include "MainInline.h"

#if !defined(APT_ENABLE_INLINE)
#include "AptBCRenderTree.inl"
#endif


/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Function Prototypes **************************************************************************/

/*** Variables ************************************************************************************/
APT_DEFINE_OBJECT_METRICS_EXPLICIT( AptBCRenderTreeManager, NULL);

// Private variables

// Public variables


/*** Private Functions ****************************************************************************/

/*** Public Functions *****************************************************************************/
/*F************************************************************************************************/
/*!
    \Function    Update_CreateItem

    \Description
        Creates and associated a Render Item object with the pOwner that uses the pData

    should be called from the AptCIH constructor.

    \Input      *pOwner           - the dude that needs a render Item
    \Input      nCurrentUpdateTick- tick to stamp the new render item with.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/31/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptRenderItem *AptBCRenderTreeManager::Update_CreateItem( AptCharacter * pCharacter, int32_t nCurrentUpdateTick)
{
    return AptRenderItem::Manager_CreateItem(pCharacter, nCurrentUpdateTick);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// These functions use AptCIH Next and previous, thus the CIH must be setup before these are called.
// If there is a writable item at this tick count we move it. It there is not, we create a new item
// and insert it at the new location.


/*F************************************************************************************************/
/*!
    \Function    Update_OwnerNextSiblingChanged

    \Description
        Called by AptCIH whenever the it's pNext pointer changes.

    \Input      pItemOwner        - object who's pNext changed.
    \Input      nCurrentUpdateTick- tick to stamp the new render item with.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/31/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptBCRenderTreeManager::Update_ItemMoved( AptCIH * pItemOwner, int32_t nCurrentUpdateTick)
{
    if(pItemOwner->GetCharacterInst() == NULL)
        return;

    if (pItemOwner->GetDisplayListPrevious() == NULL && pItemOwner->GetDisplayListParent() == NULL)
    {
        Update_SetRootItem(pItemOwner, nCurrentUpdateTick);
    }
}

/*F************************************************************************************************/
/*!
    \Function    Update_OwnerRemoved

    \Description
        Called by AptCIH when it is being removed.

    \Input      *pItemOwner       - Object being removed.
    \Input      nCurrentUpdateTick- tick to stamp the remove event with.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/31/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptBCRenderTreeManager::Update_ItemRemoved( AptRenderItem *pItem, int32_t nCurrentUpdateTick)
{
    pItem->Manager_SetDeletionMark(true);
}

/*F************************************************************************************************/
/*!
    \Function    AptRenderTreeManager::Update_OwnerRemoved

    \Description
        Notification that the passed item has been inserted.

    \Input      *pItemOwner       -
    \Input      nCurrentUpdateTick-

    \Output
        None.

    \Notes
        This should be called after the insert (so that we know where it went).

    \Version    1.0        04/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptBCRenderTreeManager::Update_ItemInserted( AptCIH *pItemOwner, int32_t nCurrentUpdateTick)
{
    AptRenderItem * pItem    = pItemOwner->GetCharacterInst()->GetRenderItemWritable();
    pItem->Manager_SetDeletionMark(false);
    Update_ItemMoved(pItemOwner, nCurrentUpdateTick);
}


/*F************************************************************************************************/
/*!
    \Function    Update_CloneItem

    \Description
        Take the Render State from on CIH and clone it for some other CIH

    \Input      *pJoeShmoe        - obvious a really cool object.
    \Input      *pWantsJoesRenderState- what a wannabe...
    \Input      nCurrentUpdateTick- tick to stamp the new render item with.

    \Output
        None.

    \Notes
        Obviously the new item is placed into the tree at the location of the wannabe.

    \Version    1.0        03/31/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptBCRenderTreeManager::Update_CloneItem( const AptCIH *pCIH, AptCIH *pNeedsCIHRenderState, int32_t nCurrentUpdateTick)
{
    // We aren't modifying this, and const correctedness is getting to be a headache. for now typecast the const away.
    AptRenderItem *pItem    = (AptRenderItem *)pCIH->GetCharacterInst()->GetRenderItem();
    AptRenderItem *pNewItem = pItem->Manager_CloneNewItem(nCurrentUpdateTick);

    pNeedsCIHRenderState->GetCharacterInst()->SetRenderItem(pNewItem);
}

/*F************************************************************************************************/
/*!
    \Function    Update_SetRootItem

    \Description
        Sets a new Root

    \Input      *pRoot            -
    \Input      nCurrentUpdateTick-

    \Output     AptRenderItem *   -

    \Notes
        None.

    \Version    1.0        03/31/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptBCRenderTreeManager::Update_SetRootItem(AptCIH *pRoot, int32_t nCurrentUpdateTick)
{
    if (pRoot == NULL)
    {
        return ;
    }
    AptRenderItem * pItem = pRoot->GetCharacterInst()->GetRenderItemWritable();

    if(mpRootListHead == NULL)
    {
        mpRootListHead = AptRenderItemRootList::StartTree(pItem);
    }
    else if(mpRootListHead->mpRoot != pItem)
    {
        // New Root Detected!!!
        mpRootListHead->InsertNewRoot(pItem);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Apt Render Traversal functions

/*F************************************************************************************************/
/*!
    \Function    Render_GetChild

    \Description
        returns the child item valid for the specified render tick.

        This also updates the internal child pointer in the passed item to point to the current
        revision of the child based on the tick.

    \Input      pCurrent          - item to get the current revision for.
    \Input      nCurrentRenderTick- tick the caller wants a pointer to.

    \Output     const AptRenderItem *  -

    \Notes
        None.

    \Version    1.0        03/31/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
const AptRenderItem * AptBCRenderTreeManager::Render_GetChild(const AptRenderItem * pCurrent, int32_t nCurrentRenderTick) const
{
    return NULL ;
}

/*F************************************************************************************************/
/*!
    \Function    Render_GetSibling

    \Description
        returns the next sibling item valid for the specified render tick.

        This also updates the internal sibling pointer in the passed item to point to the current
        revision of the sibling based on the tick.

    \Input      pCurrent          -
    \Input      nCurrentRenderTick-

    \Output     const AptRenderItem *  -

    \Notes
        None.

    \Version    1.0        03/31/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
const AptRenderItem * AptBCRenderTreeManager::Render_GetSibling(const AptRenderItem * pCurrent, int32_t nCurrentRenderTick) const
{
    return NULL ;
}

/*F************************************************************************************************/
/*!
    \Function    AptBCRenderTreeManager::Render_GetMask

    \Description
        returns the mask item valid for the specified render tick.

        This also updates the internal mask pointer in the passed item to point to the current
        revision of the sibling based on the tick.

    \Input      pCurrent          -
    \Input      nCurrentRenderTick-

    \Output     const AptRenderItem *  -

    \Notes
        None.

    \Version    1.0        04/30/05 (LEO) First Version

*/
/************************************************************************************************F*/
const AptRenderItem * AptBCRenderTreeManager::Render_GetMask(const AptRenderItem * pCurrent, int32_t nCurrentRenderTick) const
{
    AptRenderItem * pCurrentWritable = (AptRenderItem *) pCurrent;
    pCurrentWritable->Manager_UpdateMask((AptRenderItem*)pCurrentWritable->GetMask());

    AptRenderItem * pSibling = pCurrentWritable->Manager_GetMask();
    APT_ASSERT( (pSibling == NULL || !pSibling->Manager_IsDeletionMark())  && "ERROR: Trying to render a Deleted object!");
    return pSibling;
}

/*F************************************************************************************************/
/*!
    \Function    AptBCRenderTreeManager::Render_Shutdown

    \Description
        Shutdown function for render tree manager

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        05/16/05 (ASK) First Version

*/
/************************************************************************************************F*/
void AptBCRenderTreeManager::Render_Shutdown()
{
    if (mpRootListHead != NULL)
    {
        mpRootListHead->Shutdown();
        mpRootListHead = NULL;
    }
}
