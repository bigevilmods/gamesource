/*H************************************************************************************************/
/*!

    \File
        AptDisplayList.cpp

    \Description
        Defines data structures and methods for operating on the display list

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0     03/07/04    (ONA)  First Version
    \Version    1.2     11/01/04    (DC)   Changed to use allocation Macro to remove #if defines
                                           everywhere. There was a minor logic change in that
                                           previously we put an empty string as the default text
                                           of each text field, now we just use a NULL pointer to
                                           signify there is no default value.
    \Version    1.3     12/15/05    (LEO)  Added 3D support
    \Version    1.4     09/05/07    (RB)   Added back bugfixes that were removed during lockless
                                           implementation. They were wrapped in #ifdef APT_DECOUPLED_RENDERING.
    \Version    1.5     01/31/08    (RB)   Made _drawCharacterInstOpt static methods of AptDisplayList.
                                           Avoids warning with rev-mw compiler about missing prototype.
    \Version    1.6     03/05/08    (RB)   Made instantiateCharacter signature match header file.
                                           Removed unused variable in validate function.
    \Version    1.7     04/02/08    (RB)   Eliminate unused variable warning when APT_3D is disabled.
    \Version    1.8     04/29/08    (RB)   Refactored some methods to handle PlaceObject3
                                           Basically this involved duplicating the AptPseudoData struct
                                           so that now there is a AptPseudoData2 for PlaceObject2 and
                                           AptPseudoData3 for PlaceObject3.
                                           Then, AptDisplayList::PlaceObject had to be changed to handle
                                           the data from a PlaceObject3 tag. This involved duplicating the code
                                           that handled PlaceObject2 and adding logic to handle blendmodes and filters.
                                           It looks like a lot of code changed, but it is really just refactoring, and
                                           in many cases the only difference is renaming, or whitespace changes.
                                           When running SWF 7 data, there is absolutely no change in behavior from
                                           the previous version of Apt.
    \Version    1.9        07/29/08 (RB)   EATech 114985: Track max new movieclips via AptHighwaterMemoryMetrics
    \Version    1.10       10/28/08 (JP)   Calling the AptMath new function for 3d rotation instead of having the code here
    \Version    1.11       11/05/08 (JP)   Passing the 3d rotation data to the ClipTransform structure
    \Version    1.12       11/11/08 (JP)   Pass the 3d parameters to the next transformation
    \Version    1.13       07/10/09 (DG)   Add leading handling in AptDisplayList::instantiateCharacter
    \Version    1.14       07/10/09 (RB)   Made getState inline
    \Version    1.15       09/28/09 (MZ)   Added tracking handling in AptDisplayList::instantiateCharacter
*/
/************************************************************************************************H*/

#include "Display/AptDisplayList.h"
#include "Display/AptDisplayListState.h"
#include "Display/AptRenderingContext.h"
#include "_Apt.h"
#include "AptTarget.h"
#include "AptAnimationTarget.h"
#include "AptStd/AptMath.h"
#include "AptGlobal.h"
#include "AptValue/AptValueVector.h"
#include "AptObject/AptFilter.h"
#include "AptMemoryMetricsImpl.h"
#include "TextFormat.h"

#include "MainInline.h"

#include "AptProfile.h"

APT_DEFINE_CUSTOM_METRICS(AptDisplayListObjects, NULL);
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptPseudoData2T, &AptDisplayListObjects);
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptPseudoData3T, &AptDisplayListObjects);
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptDisplayListState, &AptDisplayListObjects);
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptPseudoCIH_t, &AptDisplayListObjects);
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptPseudoDisplayList, &AptDisplayListObjects);


/*F************************************************************************************************/
/*!
    \Function    AptPseudoData2T::AptPseudoData2T

    \Description
        Contructor

    Copies the data from a PlaceObject2 tag into our own internal structure

    \Input      *pControl         - A PlaceObject2 control tag
    \Input      nFrame            - Frame number
    \Input      *pNewCharacter    - AptCharacter associated with this PlaceObject2 tag's data

    \Notes
        None.

    \Version    1.0        09/01/04 (LEO) First Version
    \Version    1.01       04/29/08 (RB)  Split into PlaceObject2 and PlaceObject3 versions

*/
/************************************************************************************************F*/
AptPseudoData2T::AptPseudoData2T(AptControl *pControl, int32_t nFrame, AptCharacter *pNewCharacter)
{
    AptControlPlaceObject2 *pPlaceObject2 = &pControl->placeObject2;
    nFrameCreated = nFrame;
    eFlags = pPlaceObject2->eFlags;
    pCharacter = pNewCharacter;
    nClipDepth = pControl->placeObject2.nClipDepth;
    if(pPlaceObject2->eFlags & AptPlaceObjectFlag_Matrix)
    {
        matrix = &pPlaceObject2->matrix;
    }
    else
    {
        matrix = NULL;
    }
    ncxform = (pPlaceObject2->eFlags & AptPlaceObjectFlag_CXForm) ? &pPlaceObject2->ncxform : NULL;
    pActions = (pPlaceObject2->eFlags & AptPlaceObjectFlag_Actions) ? pPlaceObject2->pActions : NULL;
    fRatio = (pPlaceObject2->eFlags & AptPlaceObjectFlag_Ratio) ? pPlaceObject2->fRatio : 0.f;
}


/*F************************************************************************************************/
/*!
    \Function    AptPseudoData3T::AptPseudoData3T

    \Description
        Contructor

    Copies the data from a PlaceObject3 tag into our own internal structure

    \Input      *pControl         - A PlaceObject3 control tag
    \Input      nFrame            - Frame number
    \Input      *pNewCharacter    - AptCharacter associated with this PlaceObject3 tag's data

    \Notes
        None.

    \Version    1.0        04/29/08 (RB)  First version.
                                          Mostly copied from AptPseudoData2T except for filters/blendmode logic.

*/
/************************************************************************************************F*/
AptPseudoData3T::AptPseudoData3T(AptControl *pControl, int32_t nFrame, AptCharacter *pNewCharacter)
{
    AptControlPlaceObject3 *pPlaceObject3 = &pControl->placeObject3;
    nFrameCreated = nFrame;
    eFlags = pPlaceObject3->eFlags;
    pCharacter = pNewCharacter;
    nClipDepth = pControl->placeObject3.nClipDepth;
    if(pPlaceObject3->eFlags & AptPlaceObjectFlag_Matrix)
    {
        matrix = &pPlaceObject3->matrix;
    }
    else
    {
        matrix = NULL;
    }
    ncxform = (pPlaceObject3->eFlags & AptPlaceObjectFlag_CXForm) ? &pPlaceObject3->ncxform : NULL;
    pActions = (pPlaceObject3->eFlags & AptPlaceObjectFlag_Actions) ? pPlaceObject3->pActions : NULL;
    fRatio = (pPlaceObject3->eFlags & AptPlaceObjectFlag_Ratio) ? pPlaceObject3->fRatio : 0.f;
    nBlendMode = pPlaceObject3->nBlendMode;
    nNumFilters = pPlaceObject3->nNumFilters;
    ppFilters = pPlaceObject3->ppFilters;
}


/*F************************************************************************************************/
/*!
    \Function    AptPseudoCIHT::AptPseudoCIHT

    \Description
        AptSudoDisplayList cotr

    \Input      *pNewControl      -
    \Input      nFrame            -
    \Input      nDpth             -
    \Input      *pNewCharacter    -
    \Input      *pParent          -

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        08/31/04 (LEO) First Version
    \Version    1.1        04/30/08 (RB)  Handle PlaceObject3 too.

*/
/************************************************************************************************F*/
AptPseudoCIH_t::AptPseudoCIH_t(AptControl *pNewControl, int32_t nFrame, int32_t nDpth, AptCharacter *pNewCharacter)
{
    pControlInfo2 = NULL;
    pControlInfo3 = NULL;
    pControl = pNewControl;
    nDepth = nDpth;
    if(pNewControl)
    {
        if( pNewControl->eType == AptControlType_PlaceObject2 )
        {
            pControlInfo2 = new AptPseudoData2T(pNewControl, nFrame, pNewCharacter);
        }
        else if ( pNewControl->eType == AptControlType_PlaceObject3 )
        {
            pControlInfo3 = new AptPseudoData3T(pNewControl, nFrame, pNewCharacter);
        }
    }
    pNext = NULL;
    pPrev = NULL;
}


/*F************************************************************************************************/
/*!
    \Function    AptPseudoCIH_t::~AptPseudoCIH_t

    \Description
        AptPseudoCIH_t dtor

    \Version    1.0        04/29/08 (RB)  First Version. Moved from header file.
                                          Do NULL-checking. Works with DefineMorph and DefineMorph2.

*/
/************************************************************************************************F*/
AptPseudoCIH_t::~AptPseudoCIH_t()
{
    pNext = NULL;
    pPrev = NULL;
    APT_ASSERT( pControl || (!pControl && !pControlInfo2 && !pControlInfo3) );
    if ( pControl )
    {
        if(pControl->eType == AptControlType_PlaceObject2 && pControlInfo2 != NULL)
        {
            delete pControlInfo2;
            pControlInfo2 = NULL;
        }
        else if(pControl->eType == AptControlType_PlaceObject3 && pControlInfo3 != NULL)
        {
            delete pControlInfo3;
            pControlInfo3 = NULL;
        }
    }
    pControl = NULL;
}

/*F************************************************************************************************/
/*!
    \Function    AptPseudoDisplayList::ClearList

    \Description
        Deletes everything in the display list

    Insert your long description here.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptPseudoDisplayList::ClearList()
{
    AptPseudoCIHT *pTmpNext = pHead->GetDisplayListNext(), *pTmp = NULL;
    while(pTmpNext != NULL)
    {
        pTmp = pTmpNext->GetDisplayListNext();
        delete pTmpNext;
        pTmpNext = pTmp;
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptPseudoDisplayList::FindInst

    \Description
        Insert function's purpose here

    Insert your long description here.

    \Input      nDepth            - Depth for insertion
    \Input      **ppPrev          - previous item to the item at nDepth
    \Input      **ppItem          - the item at the depth, if found

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/31/04 (LEO) First Version
     \Version   1.1        11/21/07 (JP) Removing double assignments
*/
/************************************************************************************************F*/
void AptPseudoDisplayList::FindInst(int32_t nDepth, AptPseudoCIHT **ppPrev, AptPseudoCIHT **ppItem)
{
    APT_ASSERT(ppPrev);
    APT_ASSERT(ppItem);
    APT_ASSERT(nDepth >= 0);

    AptPseudoCIHT *pCur = pHead->GetDisplayListNext();
    AptPseudoCIHT *pLast = pHead;

    while(pCur && pCur->nDepth < nDepth)
    {
        pLast = pCur;
        pCur = pCur->GetDisplayListNext();
    }

    if(pCur && pCur->nDepth == nDepth)
    {
        *ppItem = pCur;
    }
    else
    {
        *ppItem = NULL;
    }
    *ppPrev = pLast;
}

/*F************************************************************************************************/
/*!
    \Function    AptPseudoDisplayList::Insert

    \Description
        Inserts the given item at its depth

    This function will remove any item that is at the depth we want to insert at.

    \Input      *pItem            - Item to insert

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/31/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptPseudoDisplayList::Insert(AptPseudoCIHT *pItem)
{
    AptPseudoCIHT *pPrev, *pOldItem;
    FindInst(pItem->nDepth, &pPrev, &pOldItem);
    if(pOldItem != NULL)
    {
        Remove(pOldItem);
        FindInst(pItem->nDepth, &pPrev, &pOldItem);
        Insert(pPrev, pItem);
    }
    else
    {
        Insert(pPrev, pItem);
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptPseudoDisplayList::Insert

    \Description
        Inserts item based on the pPrev pointer

    This function saves time if the pPrev and pOldItem are already given to us, other wise we need to find the item at the given depth and then do our work,
    which is what the function above does.

    \Input      *pNewItem         -
    \Input      *pPrev            -
    \Input      *pOldItem         -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptPseudoDisplayList::Insert(AptPseudoCIHT *pNewItem, AptPseudoCIHT *pPrev, AptPseudoCIHT *pOldItem)
{
    if(pOldItem != NULL)
    {
        Remove(pOldItem);
        Insert(pPrev, pNewItem);
    }
    else
    {
        Insert(pPrev, pNewItem);
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptPseudoDisplayList::Insert

    \Description
        This function finally inserts the item into the display list

    Insert your long description here.

    \Input      *pPrev            -
    \Input      *pNewItem         -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/31/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptPseudoDisplayList::Insert(AptPseudoCIHT *pPrev, AptPseudoCIHT *pNewItem)
{
    pNewItem->SetDisplayListNext(pPrev->GetDisplayListNext());
    pNewItem->SetDisplayListPrevious(pPrev);

    if(pNewItem->GetDisplayListNext())
    {
        pNewItem->GetDisplayListNext()->SetDisplayListPrevious(pNewItem);
    }
    pNewItem->GetDisplayListPrevious()->SetDisplayListNext(pNewItem);
}

/*F************************************************************************************************/
/*!
    \Function    AptPseudoDisplayList::Remove

    \Description
        Removes an item at the given depth

    Insert your long description here.

    \Input      nDepth            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/31/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptPseudoDisplayList::Remove(int32_t nDepth)
{
    AptPseudoCIHT *pPrev, *pItem;
    FindInst(nDepth, &pPrev, &pItem);
    Remove(pItem);
}

/*F************************************************************************************************/
/*!
    \Function    AptPseudoDisplayList::Remove

    \Description
        Finds the item in the display list and deletes it

    Insert your long description here.

    \Input      *pItem            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/31/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptPseudoDisplayList::Remove(AptPseudoCIHT *pItem)
{
    APT_ASSERT(pItem != NULL);

    if(pItem->GetDisplayListPrevious())
    {
        APT_ASSERT(pItem->GetDisplayListPrevious()->GetDisplayListNext() == pItem);
        pItem->GetDisplayListPrevious()->SetDisplayListNext( pItem->GetDisplayListNext() );
    }
    if(pItem->GetDisplayListNext())
    {
        APT_ASSERT(pItem->GetDisplayListNext()->GetDisplayListPrevious() == pItem);
        pItem->GetDisplayListNext()->SetDisplayListPrevious( pItem->GetDisplayListPrevious() );
    }
    delete pItem;
}

// fusionchange - modifying the implementation so that it lines up with the header, otherwise we get a link error
// with the snc linker.
void AptDisplayListState::findInst(int nDepth, const AptNativeString * pName, AptCIH **ppPrev, AptCIH **ppItem)
{
    APT_ASSERT(ppPrev);
    APT_ASSERT(ppItem);
    APT_ASSERT(nDepth >= 0);

    AptCIH *pCur = GetFirstItem();
    AptCIH *pLast = NULL;

    if(pCur == NULL)
    {
        *ppItem = pCur;
        *ppPrev = pLast;
        return;
    }

    if (pName)
    {
        while (pCur)
        {
            if (pCur->isUndefined() && (*pName == pCur->GetInstanceName()))
            {
                *ppItem = pCur;
                *ppPrev = pLast;
                return;
            }

            pLast = pCur;
            pCur = pCur->GetDisplayListNext();
        }
    }

    pCur = GetFirstItem();
    pLast = NULL;

    while (pCur && pCur->GetDepth() < nDepth)
    {
        pLast = pCur;
        pCur = pCur->GetDisplayListNext();
    }

    if (pCur && pCur->GetDepth() == nDepth)
    {
        *ppItem = pCur;
    }
    else
    {
        *ppItem = NULL;
    }

    *ppPrev = pLast;
}

void AptDisplayListState::findInst(int nDepth, const AptNativeString * pName, AptCIH **ppItem)
{
    AptCIH * pPrevious = NULL; // Not used.
    findInst(nDepth, pName, &pPrevious, ppItem);
}

int AptDisplayListState::getLength()
{
    int nLength = 0 ;
    AptCIH * pTemp =  pHead ;
    while (pTemp)
    {
        nLength ++ ;
        pTemp = pTemp->GetDisplayListNext() ;
    }
    return nLength ;
}

AptCIH * AptDisplayListState::getValue(int nIndex)
{
    AptCIH * pTemp = pHead ;
    // 11/21/07 (JP) Simplifying the code
    while (pTemp && (nIndex > 0))
    {
        pTemp =pTemp->GetDisplayListNext() ;
        nIndex--;
    }

    return pTemp;
}


void AptDisplayListState::RegisterReferences(const AptValue * pFrom)
{
    AptCIH * pTemp =  pHead ;
    while (pTemp)
    {
        AptCIH *pNextTmp = pTemp->GetDisplayListNext();
        APT_REGISTER_REFERENCE_FROM(pFrom, pTemp,     "AptDisplayListState::DisplayListItem", APT_REFREG_IS_DISPLAYLIST);
        pTemp = pNextTmp;
    }
    return ;
}


/*F************************************************************************************************/
/*!
    \Function    AptDisplayListState::swapDepths

    \Description
        swaps the depths of the passed objects, and moves them in the list to match that depth.

    \Input      *p0               -
    \Input      *p1               -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/20/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptDisplayListState::swapDepths(AptCIH *p0, AptCIH *p1)
{
    // This is a tricky one!
    // In Flash, you cannot change the parent of a clip to some other clip.
    // In Apt, you can *kind of* do this by calling swapDepths and swapping a clip with another
    // clip with a different parent.  This works, visually, but there are bugs with it such as
    // attempting to call attachMovie on the reparented clip.
    // Since there are bugs with it, we have this potential assert here.  However, really the assert
    // should be in the places there are bugs (such as calling attachMovie on a clip that has been
    // reparented) and not here.  Unfortunately, we don't have time to test out the attachMovie
    // stuff and figure out how to add an assert there; furthermore, Madden relies on this behavior
    // in a couple places to pull items out of lists and in front of scrims.
    // It's kind of a pickle determining whether we want this assert or not, and we really don't want
    // to pollute our build scripts with yet another build flag for such a small thing. So we'll
    // leave it commented out.
    //APT_ASSERTM((p0->GetDisplayListParent() == p1->GetDisplayListParent()), "Trying to Swap Depths between two movieclips with different parents.");

    AptCIH *p0Next = p0->GetDisplayListNext();
    AptCIH *p0Prev = p0->GetDisplayListPrevious();
    AptCIH *p1Next = p1->GetDisplayListNext();
    AptCIH *p1Prev = p1->GetDisplayListPrevious();

    if(p0Next == p1)
    {
        // If p1 is next item to p0
        p0->SetDisplayListNext(p1Next);
        if(p1Next)
            p1Next->SetDisplayListPrevious(p0);

        p1->SetDisplayListNext(p0);
        p0->SetDisplayListPrevious(p1);

        p1->SetDisplayListPrevious(p0Prev);
        if(p0Prev)
            p0Prev->SetDisplayListNext(p1);
    }
    else if(p0Prev == p1)
    {
        // If p1 is previous item to p0
        p1->SetDisplayListNext(p0Next);
        if(p0Next)
            p0Next->SetDisplayListPrevious(p1);

        p0->SetDisplayListNext(p1);
        p1->SetDisplayListPrevious(p0);

        p0->SetDisplayListPrevious(p1Prev);
        if(p1Prev)
            p1Prev->SetDisplayListNext(p0);
    }
    else
    {
        // p0 and p1 are not next to eachother
        p0->SetDisplayListNext(p1Next);
        if(p1Next)
            p1Next->SetDisplayListPrevious(p0);
        p0->SetDisplayListPrevious(p1Prev);
        if(p1Prev)
            p1Prev->SetDisplayListNext(p0);

        p1->SetDisplayListNext(p0Next);
        if(p0Next)
            p0Next->SetDisplayListPrevious(p1);
        p1->SetDisplayListPrevious(p0Prev);
        if(p0Prev)
            p0Prev->SetDisplayListNext(p1);
    }

    int nTemp = p0->GetDepth();
    p0->SetDepth( p1->GetDepth() );
    p1->SetDepth( nTemp );

#if defined(EA_DEBUG)
    if (p0->GetDisplayListPrevious())
    {
        APT_ASSERT(p0->GetDisplayListPrevious()->GetDisplayListNext() == p0);
    }
    if (p0->GetDisplayListNext())
    {
        APT_ASSERT(p0->GetDisplayListNext()->GetDisplayListPrevious() == p0);
    }

    if (p1->GetDisplayListPrevious())
    {
        APT_ASSERT(p1->GetDisplayListPrevious()->GetDisplayListNext() == p1);
    }
    if (p1->GetDisplayListNext())
    {
        APT_ASSERT(p1->GetDisplayListNext()->GetDisplayListPrevious() == p1);
    }
#endif

   if(pHead == p0 || pHead == p1)
    {
        // If either p0 or p1 where the head item, update the parents first child
        if(pHead == p0)
            pHead = p1;
        else if(pHead == p1)
            pHead = p0;
    }

    p0->SetIsInserted();
    p1->SetIsInserted();
    return;
}

/*F************************************************************************************************/
/*!
    \Function    *AptDisplayListState::insert

    \Description
        inserts the passed item after the passed previous item.

        if pPrev is NULL then it is assumed that the intent is to insert at the head.

    \Input      *pPrev            -
    \Input      *pNewItem         -

    \Output     AptCIH            -

    \Notes
        None.

    \Version    1.0        04/20/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptCIH *AptDisplayListState::insert(AptCIH *pPrev, AptCIH *pNewItem)
{
    APT_ASSERT(pNewItem->isCIH() && (pPrev == NULL ||pPrev->isCIH()) );
    APT_ASSERT(pPrev == NULL || (pNewItem->GetDisplayListParent() == pPrev->GetDisplayListParent()));
    APT_ASSERT(pNewItem->GetCharacterInst() != NULL);

    if(pPrev)
    {
        // Not the head item. Simple insertion path.

        AptCIH * pNext = pPrev->GetDisplayListNext();

        pNewItem->SetDisplayListNext(     pNext );
        pNewItem->SetDisplayListPrevious( pPrev );

        pPrev->SetDisplayListNext( pNewItem );
        if(pNext)
        {
            pNext->SetDisplayListPrevious( pNewItem );
        }
    }
    else
    {
        // Is the new head.
        pNewItem->SetDisplayListPrevious( NULL );

        if(pHead)
        {
            // Is replacing another head.
            pNewItem->SetDisplayListNext( pHead );
            pHead->SetDisplayListPrevious( pNewItem );
        }
        else
        {
            pNewItem->SetDisplayListNext( NULL );
        }
        pHead = pNewItem;
    }

    APT_INC(pNewItem);
    pNewItem->SetIsInserted();

#if defined(EA_DEBUG)
    AptCIH * pTemp = pHead;
    bool bMyObjectFound = false;

    while(pTemp)
    {
        if(pTemp == pNewItem)
            bMyObjectFound = true;

        if (pTemp->GetDisplayListPrevious())
        {
            APT_ASSERT(pTemp->GetDisplayListPrevious()->GetDisplayListNext() == pTemp);
        }
        if (pTemp->GetDisplayListNext())
        {
            APT_ASSERT(pTemp->GetDisplayListNext()->GetDisplayListPrevious() == pTemp);
        }
        pTemp = pTemp->GetDisplayListNext();
    }
    APT_ASSERT(bMyObjectFound);
#endif

    return pNewItem;
}

AptCIH *AptDisplayListState::insert(int nDepth, AptCharacter *pCharacter, AptCIH *pParent, AptCIH *pPrev, AptCIH *pItemAtDepth)
{
    AptCIH *pNewItem = NULL;

    pNewItem = new AptCIH(pCharacter, pParent);

    APT_ASSERT(pItemAtDepth == NULL || pItemAtDepth->isUndefined());

    pNewItem->SetDepth(nDepth);

    return insert(pPrev, pNewItem);
}

AptCIH *AptDisplayListState::insert(int nDepth, AptCharacter *pCharacter, AptCIH *pParent)
{
    AptCIH *pPrev, *pItem;
    AptCIH *pNewItem = NULL;

    pNewItem = new AptCIH(pCharacter, pParent);

    findInst(nDepth, NULL, &pPrev, &pItem);
    APT_ASSERT(pItem == NULL || pItem->isUndefined());

    pNewItem->SetDepth(nDepth);

    return insert(pPrev, pNewItem);
}

AptCIH *AptDisplayListState::insert(int nDepth, AptCIH *pItem)
{
    AptCIH *pPrev, *pOldItem;

    findInst(nDepth, NULL, &pPrev, &pOldItem);
    APT_ASSERT(pOldItem == NULL || pOldItem->isUndefined());

    AptCIH *pNewItem = insert(pPrev, pItem);
    pNewItem->SetDepth(nDepth);
    return pNewItem;
}

/*F************************************************************************************************/
/*!
    \Function    *AptDisplayListState::ChangeDepth

    \Description
        Updates the depth of the passed in AptCIH

    \Output     AptCIH            -

    \Notes
        None.

    \Version    1.0        11/02/05 (LEO) Bringin' it to ya

*/
/************************************************************************************************F*/
AptCIH *AptDisplayListState::ChangeDepth(int nDepth, AptCIH *pItem)
{
    AptCIH *pPrev, *pOldItem;

    APT_ASSERT(pItem != NULL);

    AptCIH * pParent = pItem->GetDisplayListParent();
    AptCIH * pNext   = pItem->GetDisplayListNext();
    AptCIH * pPrev1   = pItem->GetDisplayListPrevious();

    // It removes the lotion from the basket
    if (pPrev1)
    {
        APT_ASSERT(pPrev1->GetDisplayListNext() == pItem);
        pPrev1->SetDisplayListNext( pNext );
    }
    if (pItem->GetDisplayListNext())
    {
        APT_ASSERT(pNext->GetDisplayListPrevious() == pItem);
        pNext->SetDisplayListPrevious( pPrev1 );
    }

    // If we remove the first item, be sure to update the head.
    if (pHead == pItem)
    {
        pHead = pNext;
    }

    pItem->SetDisplayListPrevious(NULL);
    pItem->SetDisplayListNext(NULL);

    // It places the lotion in the basket
    findInst(nDepth, NULL, &pPrev, &pOldItem);
    APT_ASSERT(pOldItem == NULL || pOldItem->isUndefined());


    if(pPrev)
    {
        // Not the head item. Simple insertion path.
        AptCIH * pNext = pPrev->GetDisplayListNext();
        pItem->SetDisplayListNext(      pNext );
        pItem->SetDisplayListPrevious(  pPrev );
        pPrev->SetDisplayListNext(      pItem );
        if(pNext)
        {
            pNext->SetDisplayListPrevious( pItem );
        }
    }
    else
    {
        // Is the new head.
        pItem->SetDisplayListPrevious( NULL );

        if(pHead)
        {
            // Is replacing another head.
            pItem->SetDisplayListNext( pHead );
            pHead->SetDisplayListPrevious( pItem );
        }
        else
        {
            pItem->SetDisplayListNext( NULL );
        }
        pHead = pItem;
    }

    // Update RI states
    if (!pPrev1 && !pParent)
    {
        AptCharacterInst::SetRootItem(pNext);
    }

    pItem->SetDepth( nDepth );

    pItem->SetIsInserted();
    return pItem;
}

AptCIH *AptDisplayListState::insert(int nDepth, AptCIH *pParent, AptCIH *pItem, AptCIH *pPrev, AptCIH *pItemAtDepth)
{
    APT_ASSERT(pItemAtDepth == NULL || pItemAtDepth->isUndefined());

    AptCIH *pNewItem = insert(pPrev, pItem);
    pNewItem->SetDepth(nDepth) ;
    return pNewItem;
}

AptCIH *AptDisplayListState::remove(int nDepth)
{
    AptCIH *pPrev, *pItem;
    findInst(nDepth, NULL, &pPrev, &pItem);
    return remove(pItem);
}

AptCIH *AptDisplayListState::removeItem(AptCIH *pItem)
{
    APT_ASSERT(pItem != NULL);
    AptCIH * pParent = pItem->GetDisplayListParent();
    AptCIH * pNext   = pItem->GetDisplayListNext();
    AptCIH * pPrev   = pItem->GetDisplayListPrevious();

    if (pPrev)
    {
        APT_ASSERT(pPrev->GetDisplayListNext() == pItem);
        pPrev->SetDisplayListNext( pNext );
    }
    if (pItem->GetDisplayListNext())
    {
        APT_ASSERT(pNext->GetDisplayListPrevious() == pItem);
        pNext->SetDisplayListPrevious( pPrev );
    }

    // If we remove the first item, be sure to update the head.
    if (pHead == pItem)
    {
        pHead = pNext;
    }

    if (!pPrev && !pParent)
    {
        AptCharacterInst::SetRootItem(pNext);
    }

    pItem->SetDisplayListPrevious(NULL);
    pItem->SetDisplayListNext(NULL);

    return pItem;
}

AptCIH *AptDisplayListState::remove(AptCIH *pItem)
{
    AptCIH * pParent = pItem->GetDisplayListParent();
    AptDisplayListState * pState = pParent ? pParent->GetDisplayListState() : GetTargetSim()->GetAnimationTarget()->GetDisplayList()->pState;

    //  11/21/07 (JP) Checking if pState is NULL
    APT_ASSERT(pState);
    return pState? pState->removeItem(pItem) : NULL;
}

/*F************************************************************************************************/
/*!
    \Function    AptDisplayListState::AddToDelayReleaseList

    \Description
        This function removes the pItem from the display list, cleans the pItem, and puts it in the
        DelayReleaseList to be removed at the end of the AptUpdate.

    The reason for only calling AddToRemList when the ref count is higher than 2, is because:
    -If ref count is 1, pItem->Remove(bDestroyGC) will delete pItem
    -If ref count is 2, pItem->Remove(bDestroyGC) will decrement to ref to 1, and outer
        ptDisplayList::clear function will dec to 0

    \Input      *pItem            -
    \Input      bDestroyGC        -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/05/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptDisplayListState::AddToDelayReleaseList(AptCIH *pItem, const bool bDestroyGC)
{
    APT_INC(pItem);

    removeItem(pItem);
    pItem->Remove(bDestroyGC);

    if(pItem->getRefCount() > 1)
    {
        GetTargetSim()->GetAnimationTarget()->AddToRemList(pItem);
    }

    APT_DEC(pItem);
}


/*F************************************************************************************************/
/*!
    \Function    AptDisplayList::instantiateCharacter

    \Description
        Insert function's purpose here

    Insert your long description here.

    \Input      nTargetDepth      - Depth of which the new sprite will reside
    \Input      *pCharacter       - The AptCharacter pointer; indicates what the type of AptCIH it will be
    \Input      pName             - The name of the sprite if provided
    \Input      *pParent          - The parent for this new instance
    \Input      bForceNewInstance - This indicates that we want to remove any old instance if there was the same instance one already found
    \Input      nClipDepth        -
        NO END BRACKET

    \Output
        None.

    \Notes
        - (LEO) added code to copy the created on frame number to the AptCIH since during a mergeState the pData (AptCharacter) which originally
        held this value can be NULL.
        - !!ATTENTION!! this is the only place where a AptCIH is added to the display list.

    \Version    1.0        08/20/04 (LEO) First Version
    \Version    1.1        11/11/04 (DC)  changed to use a NULL string instead of an empty string for
                                          text new instances.
    \Version    1.2        07/29/08 (RB)  Track max new movieclips via AptHighwaterMemoryMetrics
    \Version    1.3        07/10/09 (DG)  Set the leading attribute of pTextInst to UNDEFINED_LEADING_VALUE
    \Version    1.4        09/28/09 (MZ)  Set the tracking attribute of pTextInst to UNDEFINED_TRACKING_VALUE
*/
/************************************************************************************************F*/
void AptDisplayList::instantiateCharacter(int nTargetDepth, AptCharacter *pCharacter, const AptNativeString * pName, AptCIH *pParent, int bForceNewInstance, int nClipDepth,
                                          AptCIH **ppCIH, int *pbNeedNewInst)
{
    APT_ASSERT(pParent);
    APT_ASSERT(pState);

    bool bNeedNewInst = false;
    AptCIH *pItemAtDepth, *pCurCIH = 0;

    pState->findInst(nTargetDepth, pName, &pItemAtDepth);

    // Make sure if we add and delete the only object from a file, the files doesn't get deleted.

    if (pItemAtDepth)
    {
        if (bForceNewInstance)
        {
            if(pItemAtDepth)
            {
                //AptDisplayListState::remove(pItemAtDepth);
                removeObject(pItemAtDepth);
                pItemAtDepth = NULL;
            }
            bNeedNewInst = true;
        }
        else if (pItemAtDepth->isUndefined())
        {
            //APT_ASSERT(pItemAtDepth->pMyName);
            if (pName && (*pName == pItemAtDepth->GetInstanceName()))
            {
                pItemAtDepth->setIsDefined(1);
                pCurCIH = pItemAtDepth;
            }
            bNeedNewInst = true;
        }
        else
        {
            pCurCIH = pItemAtDepth;
            bNeedNewInst = false;
        }
    }
    else
    {
        bNeedNewInst = true;
    }

    if (bNeedNewInst)
    {
        // Now we just insert it.
        if (pCurCIH == NULL)
        {
            pCurCIH = pState->insert(nTargetDepth, pCharacter, pParent);
        }
        else
        {
            // reinsert into correct display list depth
            if (nTargetDepth != pCurCIH->GetDepth())
            {
                AptDisplayListState::remove(pCurCIH);
                pState->insert(nTargetDepth, pCurCIH);

                // DC - 1 of 1 - Added in 17.0
                // calling insert will increment the refcount on the object (but remove does not decrement)
                // Thus the remove -> insert above increments the CIH reference count without any new
                // reference being taken. This Decrement counteracts that. Note that the true fix for this
                // would be to have the remove decrement the CIH, but that has it's own problems, and would
                // require much more testing. This will work until we have time to do that.
                APT_DEC(pCurCIH);
            }

            AptCharacterInst* pNewInst = AptCharacterInst::CreateCharacterInst(pCharacter);
            pCurCIH->SetCharacterInst(pNewInst, true);
        }

        if(pParent->IsSpriteInstBase())
        {
            pCurCIH->SetCreatedOnFrame( pParent->GetSpriteInstBase()->mnFrame );
        }
        else
        {
            pCurCIH->SetCreatedOnFrame( -1 );              // if in a button
        }

        // $fr719 - moved block of code up.
        if (pName != NULL)  // if there's a name, add a reference to it to the parent display of its name and point it to the child that we're creating
        {
            pCurCIH->SetInstanceName( *pName );
            if (pName->IsEmpty() == false)
            {
                AptValue *pTmper = pParent->GetCharacterInst()->GetNativeHash()->Lookup(pName);         // (LEO) $bug844 _alpha not updating
                if(pTmper == NULL || !pTmper->isCIH(true))
                {
                    pParent->GetCharacterInst()->GetNativeHash()->Set(pName, pCurCIH);
                }
            }
        }

        if (pCurCIH->IsSpriteInstBase() || pCurCIH->IsButtonInst())
        {
            AptAnimationTarget* pTarget = GetTargetSim()->GetAnimationTarget();
            // pools are allocated at runtime.
            if ( pTarget->GetNewInstSize() >= pTarget->GetMaxNewMovieClips() )
            {
                AptHighwaterMemoryMetrics* pMetrics  = AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_MOVIECLIP_INSTANCES);
                int32_t nPolicy = AptHighwaterMemoryMetrics::sOverflowCallback->OnOverflowDetected( *pMetrics, &pTarget->GetNewInsts(), pCurCIH );
                if ( nPolicy & (int32_t)AptHighwaterMemoryMetrics::POLICY_APT_WILL_ASSERT )
                {
                    APT_ASSERTM( (pTarget->GetNewInstSize() < pTarget->GetMaxNewMovieClips()), pMetrics->mHelpfulHint );
                }
            }
            else
            {
                pTarget->GetNewInsts()[pTarget->IncNewInstSize()] = pCurCIH;
                APT_INC(pCurCIH);
            }

#if defined(EA_DEBUG)
            AptHighwaterMemoryMetrics* pMetrics  = AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_MOVIECLIP_INSTANCES);
            if ( pTarget->GetNewInstSize() > pMetrics->mHighwaterMaxCount )
            {
                pMetrics->mHighwaterMaxCount = pTarget->GetNewInstSize();
            }

#endif // EA_DEBUG
        }
        else if (pCurCIH->IsDynamicTextInst())
        {
            AptCharacterTextInst *  pTextInst = pCurCIH->GetDynamicTextInst();

            if(pTextInst->GetCharacterConst()->text.szInitialText)
                pTextInst->SetTextValue( pTextInst->GetCharacterConst()->text.szInitialText );
            else
                pTextInst->SetTextValue( "" );

            if(pTextInst->GetCharacterConst()->text.szVariable)
                pTextInst->SetVarValue( pTextInst->GetCharacterConst()->text.szVariable );
            else
                pTextInst->SetVarValue( "" );

            pTextInst->SetAlignment(pTextInst->GetCharacterConst()->text.eAlignment);
            pTextInst->SetBoxAlignment(AptStringAlignment_None) ;

            pTextInst->SetLeading(TextFormat::UNDEFINED_LEADING_VALUE);
            pTextInst->SetTracking(TextFormat::UNDEFINED_TRACKING_VALUE);

            //$bug - 102998 - RB - 3/10/2006 - Don't ignore multiline property from Flash
            pTextInst->SetMultiline( pTextInst->GetCharacterConst()->text.bMultiLine > 0 );
            pTextInst->SetWordWrap( pTextInst->GetCharacterConst()->text.bWordWrap > 0 );

            pTextInst->SetText(pParent);
            pTextInst->ClearStateFlags( APT_TEXTFIELD_NONE );
            pTextInst->SetStateFlags(APT_TEXTFIELD_FUPDATE | APT_TEXTFIELD_DIRTY);
        }
    }

//  DEBUGPRINT("putting %p at %d of %p\n", pInst, nDepth, apArray);
    APT_ASSERT(pCurCIH);

    if(bNeedNewInst == false)
    {
        // If we didn't need a new instance, we are re-using an item. make sure that the parent pointer
        // is updated. Also make sure to update the character pointer.
        if(pParent != pCurCIH->GetDisplayListParent())
        {
            APT_INC(pParent);
            APT_DECSAFE(pCurCIH->GetDisplayListParent());
            pCurCIH->SetDisplayListParent( pParent );
        }

        if(pCurCIH->GetCharacterInst()->GetCharacterConst() != pCharacter)
        {
            AptCharacterInst* pNewInst = AptCharacterInst::CreateCharacterInst(pCharacter);
            pCurCIH->SetCharacterInst(pNewInst, true);
        }
    }
    else
    {
        if(pCurCIH->GetCharacterInst()->GetCharacterConst() != pCharacter)
            pCurCIH->GetCharacterInst()->SetCharacter( pCharacter );
    }

    pCurCIH->GetCharacterInst()->SetClipDepth( nClipDepth );


#if defined APT_USE_BUTTONS
    if(pCurCIH->IsButtonInst())                    // This was added for bug 271, basically ticks button to initial state...
        pCurCIH->gotoState(AptCharacterButtonRecordState_Up);
#endif

    *ppCIH = pCurCIH;
    *pbNeedNewInst = bNeedNewInst;

    //APT_INC(pCurCIH);           // (LEO) !!ATTENTION!! THIS IS THE ONLY SPOT WE APT_INC THE CIH WHEN ADDING TO DISPLAYLIST
}

AptCIH *AptDisplayList::placeObjectNCXForm(
    AptCIH *pItem,
    int nTargetDepth,
    AptCharacter *pCharacter,
    const AptNativeString * pName,
    AptCIH *pParent,
    int bForceNewInstance,
    int nClipDepth,
    AptUint32CXForm *pnCXForm,
    AptMatrix *pMatrix,
    AptEventActionSet *pActions,
    float fRatio,
    int32_t nBlendMode,
    uint32_t nNumFilters,
    AptFilter** ppFilters
)
{
    AptCXForm cxform;
    AptCXForm *pCXForm = NULL;

    if (pnCXForm)
    {
        cxform.AptUint32CXFormCopy(pnCXForm);
        pCXForm = &cxform;
    }

    return placeObject(
            pItem,
            nTargetDepth,
            pCharacter,
            pName,
            pParent,
            bForceNewInstance,
            nClipDepth,
            pCXForm,
            pMatrix,
            pActions,
            fRatio,
            NULL,
            nBlendMode,
            nNumFilters,
            ppFilters );
}

void AptDisplayList::_addToSetCaches(AptCIH *pItem, int bQueueClipEvents)
{
    const AptCharacter *pCharacter = pItem->GetCharacterInst()->GetCharacterConst();

    if (pCharacter->eType == AptCharacterType_Button)
    {
#if defined APT_USE_BUTTONS
        if (!GetTargetSim()->GetAnimationTarget()->GetButtonSet()->has(pItem)) GetTargetSim()->GetAnimationTarget()->GetButtonSet()->add(pItem);
#endif
    }
    else if (pCharacter->eType == AptCharacterType_Sprite)
    {
        AptCharacterSpriteInstBase *pSprInst = pItem->GetSpriteInstBase();
        AptEventActionSet *pClipActions = pSprInst->mpClipActions;

        if (pClipActions != NULL)
        {
            bool bInputEventFound = false;
            for (int i=0; i<pClipActions->nEventActions; i++)
            {
                if (pClipActions->aEventActions[i].nTriggers & AptEventActionFlag_AllEvents)
                {
                    pSprInst->SetClipAction(pClipActions->aEventActions[i].nTriggers);
                    #if defined(APT_USE_MOUSE)
                    if(pClipActions->aEventActions[i].nTriggers & (AptEventActionFlag_KeyEvents | AptEventActionFlag_MouseEvents))  // (LEO) added the AptEventActionFlag_MouseEvents to handle mouse events properly
                    #else
                    if(pClipActions->aEventActions[i].nTriggers & (AptEventActionFlag_KeyEvents))
                    #endif
                    {
                        bInputEventFound = true;
                    }
                }
            }
            if(bInputEventFound && !GetTargetSim()->GetAnimationTarget()->GetInputSet()->has(pItem))
            {
                GetTargetSim()->GetAnimationTarget()->GetInputSet()->add(pItem);
            }
            if (bQueueClipEvents)
            {
                pSprInst->SetClipAction(AptEventActionFlag_Initialize | AptEventActionFlag_Construct);
                pItem->queueClipEvents(AptEventActionFlag_Initialize);//, gNullInput, false);
                pItem->queueClipEvents(AptEventActionFlag_Construct);//, gNullInput, false);
                pSprInst->RemoveClipAction(AptEventActionFlag_Initialize | AptEventActionFlag_Construct);
            }
        }
    }
#if defined(APT_USE_MOUSE)
    else if(pCharacter->eType == AptCharacterType_Text && bDefaultMouseWheel == true)      // (LEO) added support for TextMouseWheel   5/
    {
        GetTargetSim()->GetAnimationTarget()->GetMouseListenerSet()->add(pItem);
        GetTargetSim()->GetAnimationTarget()->GetInputSet()->add(pItem);
    }
#endif
}


/*F************************************************************************************************/
/*!
    \Function    *AptDisplayList::placeObject

    \Description
        Last parameter added as AptValue * defaulted to NULL.
        This takes care of initialing/adding new properties to newly created instance.
        This paramter will be valid mainly when called from attachmovie kind of functions

    \Input      *pItem            -
    \Input      nTargetDepth      -
    \Input      *pCharacter       -
        NO END BRACKET

    \Output     AptCIH            -

    \Notes
        None.

    \Version    1.0        06/22/05 (LEO) First Version
    \Version    1.01       06/22/05 (LEO) $bug936 MovieClip constructor executes every frame in a motion tween
    \Version    1.02       04/29/08 (RB)  Handle blendMode and filters from PlaceObject3 tag
    \Version    1.03       06/13/08 (ASK) Removed extra APT_INC on pFilterArray as it was causing objects to stay behind.
    \Version    1.04       09/23/09 (MZ)  Conditionally excluded filter-processing code based on APT_USE_FILTERS macro.
*/
/************************************************************************************************F*/
AptCIH *AptDisplayList::placeObject(AptCIH *pItem, int nTargetDepth, AptCharacter *pCharacter,
                                    const AptNativeString * pName, AptCIH *pParent, int bForceNewInstance,
                                 int nClipDepth, AptCXForm *pCXForm, AptMatrix *pMatrix,
                                 AptEventActionSet *pActions, float fRatio, AptValue * pInitObject,
                                 int32_t nBlendMode, uint32_t nNumFilters, AptFilter** ppFilters )
{
    int bNeedNewInst = 0;

    APT_ASSERT(pCharacter || pItem);

    if (!pItem)
    {
        instantiateCharacter(nTargetDepth, pCharacter, pName, pParent, bForceNewInstance, nClipDepth, &pItem, &bNeedNewInst);
    }

    if (pItem)
    {
        // $bug723 - MCB - Always update cxform and matrix, regardless of whether pItem->pProceduralSettings exists yet or not.
        if (pCXForm)
        {
            //pItem->SetColorMatrix(*pCXForm) ;
            pItem->GetColorMatrixWritable()->AptCXFormCopy(pCXForm);
        }

        if (pMatrix)
        {
            //pItem->SetPositionMatrix(*pMatrix);
            pItem->GetPositionMatrixWritable()->AptMatrixCopy(pMatrix);
        }

        if (pActions)
        {
            APT_ASSERT(pItem->IsSpriteInstBase());

            pItem->GetSpriteInstBase()->mpClipActions = pActions;
        }

        if (pItem->IsMorphInst())
        {
            pItem->GetMorphInst()->mfRatio = fRatio;
        }

        if (bNeedNewInst)
        {
            _addToSetCaches(pItem);
        }

        //
        // Set blendmode
        //
        pItem->SetHasBlendMode( 0 );
        if ( nBlendMode != -1 )
        {
            if (pItem->IsSpriteInstBase() || pItem->IsDynamicTextInst() || pItem->IsButtonInst())
            {
                pItem->GetCharacterInst()->SetBlendMode(nBlendMode);
                pItem->SetHasBlendMode( 1 );
            }
            else
            {
                APT_ASSERT(false && "blendMode supported only for Movieclips, TextTextFields and Buttons");
            }
        }

        //
        // Set filters
        //
        pItem->SetHasFilterEffects( 0 );
#if APT_USE_FILTERS
        if ( nNumFilters > 0 && ppFilters != NULL )
        {
            if (pItem->IsSpriteInstBase() || pItem->IsDynamicTextInst() || pItem->IsButtonInst())
            {
                // create filters array -- only if it won't be copied from pInitObject
                if ( !(bNeedNewInst && pItem->IsSpriteInstBase() && pInitObject && pInitObject->getIsDefined()) )
                {
                    AptArray* pFilterArray = new AptArray();

                    //
                    // create each filter
                    //
                    for ( int32_t j=0; j<(int32_t)nNumFilters; ++j )
                    {
                        AptObject* pFilter = AptFilter::CreateFilterObject(ppFilters[j], pItem );
                        pFilterArray->set(j, pFilter) ;
                    }

                    AptNativeHash* pHash = pItem->GetNativeHashVirtual();
                    if (pHash == NULL && pItem->IsDynamicTextInst())
                    {
                        // dynamic text instances do not have hash table by default, so create a new one only for filters
                        AptCharacterInst* pCharInst = pItem->GetCharacterInst();
                        pCharInst->SetNativeHash(new AptNativeHash(APT_OBJECTHASHSIZE));
                    }
                    pItem->GetNativeHashVirtual()->Set( StringPool::GetString(SC_filters), pFilterArray );
                    pItem->SetHasFilterEffects( 1 );
                }
            }
            else
            {
                APT_ASSERT(false && "filters supported only for Movieclips, TextFields and Buttons");
            }
        }
#endif // APT_USE_FILTERS

        // Leo - was running associateInstToclass even if a new inst wasn't made!
        if (bNeedNewInst && pItem->IsSpriteInstBase())      // to check if it has valid Hash init or not
        {
            if ((pInitObject) && pInitObject->getIsDefined())
            {
                // add properties to newly created pItem.
                AptNativeHash *pObjHash = pInitObject->GetNativeHashVirtual();
                // there could be need to gothru parents of initobject also.

                for (AptHashItem *pInitItem = pObjHash->GetFirstItem(); pInitItem; pInitItem = pObjHash->GetNextItem(pInitItem))
                {
                    // TODO: theoretically, on every hash key there's supposed to be a DontEnum/DontDelete flag; we don't have that so hardcode some dontenum's here..
                    if( (pInitItem->Key.Equal(*StringPool::GetString(SC___proto__))) ||
                        (pInitItem->Key.Equal(*StringPool::GetString(SC_prototype))) )
                    {
                        continue;
                    }
                    gAptActionInterpreter.setVariable(pItem, NULL, &pInitItem->Key, pInitItem->mValue, true) ;
                }
            }

            // this is required to setup relationship between the newly created character
            pItem->AssociateInstToClass() ;

        }   // end of newly added
    }
    return pItem;
}

AptCIH *AptDisplayList::placeObject(AptControlPlaceObject2 *pPlaceObject2, AptCIH *pParent)
{
//  DEBUGPRINT("placeObject %d\n", pPlaceObject2->nDepth);


    if (pPlaceObject2->eFlags & AptPlaceObjectFlag_Character)
    {
        bool bValidIndex = (pPlaceObject2->nCharacterID >= 0 && pPlaceObject2->nCharacterID < pParent->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation.nCharacters);
        // For this case, we have a place object that later got replaced with a place object that has the move flag in the psuedo display list, so we have to create a new one.
        // APT_ASSERT(bValidIndex);
        if(!bValidIndex)
        {
            return NULL;
        }
        AptCharacter *pCharacter = pParent->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation.apCharacters[pPlaceObject2->nCharacterID];
        APT_ASSERT(pCharacter);

        AptNativeString     strName;
        AptNativeString *   pName = NULL;

        if (pPlaceObject2->eFlags & AptPlaceObjectFlag_Name)
        {
            strName = pPlaceObject2->szName;
            pName = &strName;
        }

        return placeObjectNCXForm(
            NULL,
            pPlaceObject2->nDepth,
            pCharacter,
            pName,
            pParent,
            0,
            pPlaceObject2->nClipDepth,
            pPlaceObject2->eFlags & AptPlaceObjectFlag_CXForm ? &pPlaceObject2->ncxform : NULL,
            pPlaceObject2->eFlags & AptPlaceObjectFlag_Matrix ? &pPlaceObject2->matrix : NULL,
            pPlaceObject2->eFlags & AptPlaceObjectFlag_Actions ? pPlaceObject2->pActions : NULL,
            pPlaceObject2->fRatio
            );
    }
    else if (pPlaceObject2->eFlags & AptPlaceObjectFlag_Move)
    {
        AptCIH *pCur;
        pState->findInst(pPlaceObject2->nDepth, NULL, &pCur);
        if(pCur == NULL)
        {
            bool bValidIndex = (pPlaceObject2->nCharacterID >= 0 && pPlaceObject2->nCharacterID < pParent->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation.nCharacters);
            // For this case, we have a place object that later got replaced with a place object that has the move flag in the psuedo display list, so we have to create a new one.
            // APT_ASSERT(bValidIndex);
            if(!bValidIndex)
            {
                return NULL;
            }
            AptCharacter *pCharacter = pParent->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation.apCharacters[pPlaceObject2->nCharacterID];
            APT_ASSERT(pCharacter);

            AptNativeString     strName;
            AptNativeString *   pName = NULL;

            if (pPlaceObject2->eFlags & AptPlaceObjectFlag_Name)
            {
                strName = pPlaceObject2->szName;
                pName = &strName;
            }

            return placeObjectNCXForm(
                NULL,
                pPlaceObject2->nDepth,
                pCharacter,
                pName,
                pParent,
                0,
                pPlaceObject2->nClipDepth,
                pPlaceObject2->eFlags & AptPlaceObjectFlag_CXForm ? &pPlaceObject2->ncxform : NULL,
                pPlaceObject2->eFlags & AptPlaceObjectFlag_Matrix ? &pPlaceObject2->matrix : NULL,
                pPlaceObject2->eFlags & AptPlaceObjectFlag_Actions ? pPlaceObject2->pActions : NULL,
                pPlaceObject2->fRatio
                );
        }
        else if(!pCur->GetASChanged())    // (LEO) $bug871 : AptDisplayList force updates after AS changes property
        {
            return placeObjectNCXForm(
                pCur,
                0,
                NULL,
                NULL,
                pParent,
                0,
                -1,
                pPlaceObject2->eFlags & AptPlaceObjectFlag_CXForm ? &pPlaceObject2->ncxform : NULL,
                pPlaceObject2->eFlags & AptPlaceObjectFlag_Matrix ? &pPlaceObject2->matrix : NULL,
                pPlaceObject2->eFlags & AptPlaceObjectFlag_Actions ? pPlaceObject2->pActions : NULL,
                pPlaceObject2->fRatio
                );
        }
    }
    return NULL;
}

/// Same as placeObject above, but takes AptControlPlaceObject3
AptCIH *AptDisplayList::placeObject(AptControlPlaceObject3 *pPlaceObject3, AptCIH *pParent)
{
    int32_t eFlags = pPlaceObject3->eFlags;
    int32_t nDepth = pPlaceObject3->nDepth;
    int32_t nClipDepth = pPlaceObject3->nClipDepth;
    int32_t nCharacterID = pPlaceObject3->nCharacterID;
    if (eFlags & AptPlaceObjectFlag_Character)
    {
        bool bValidIndex = (nCharacterID >= 0 && nCharacterID < pParent->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation.nCharacters);
        // For this case, we have a place object that later got replaced with a place object that has the move flag in the pseudo display list, so we have to create a new one.
        // APT_ASSERT(bValidIndex);
        if(!bValidIndex)
        {
            return NULL;
        }
        AptCharacter *pCharacter = pParent->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation.apCharacters[nCharacterID];
        APT_ASSERT(pCharacter);

        AptNativeString     strName;
        AptNativeString *   pName = NULL;

        if (eFlags & AptPlaceObjectFlag_Name)
        {
            strName = pPlaceObject3->szName;
            pName = &strName;
        }

        return placeObjectNCXForm(
            NULL,
            nDepth,
            pCharacter,
            pName,
            pParent,
            0,
            nClipDepth,
            eFlags & AptPlaceObjectFlag_CXForm ? &pPlaceObject3->ncxform : NULL,
            eFlags & AptPlaceObjectFlag_Matrix ? &pPlaceObject3->matrix : NULL,
            eFlags & AptPlaceObjectFlag_Actions ? pPlaceObject3->pActions : NULL,
            pPlaceObject3->fRatio,
            pPlaceObject3->nBlendMode,
            pPlaceObject3->nNumFilters,
            pPlaceObject3->ppFilters
            );
    }
    else if (eFlags & AptPlaceObjectFlag_Move)
    {
        AptCIH *pCur;
        pState->findInst(nDepth, NULL, &pCur);
        if(pCur == NULL)
        {
            bool bValidIndex = (nCharacterID >= 0 && nCharacterID < pParent->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation.nCharacters);
            // For this case, we have a place object that later got replaced with a place object that has the move flag in the pseudo display list, so we have to create a new one.
            // APT_ASSERT(bValidIndex);
            if(!bValidIndex)
            {
                return NULL;
            }
            AptCharacter *pCharacter = pParent->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation.apCharacters[nCharacterID];
            APT_ASSERT(pCharacter);

            AptNativeString     strName;
            AptNativeString *   pName = NULL;

            if (eFlags & AptPlaceObjectFlag_Name)
            {
                strName = pPlaceObject3->szName;
                pName = &strName;
            }

            return placeObjectNCXForm(
                NULL,
                nDepth,
                pCharacter,
                pName,
                pParent,
                0,
                nClipDepth,
                eFlags & AptPlaceObjectFlag_CXForm ? &pPlaceObject3->ncxform : NULL,
                eFlags & AptPlaceObjectFlag_Matrix ? &pPlaceObject3->matrix : NULL,
                eFlags & AptPlaceObjectFlag_Actions ? pPlaceObject3->pActions : NULL,
                pPlaceObject3->fRatio,
                pPlaceObject3->nBlendMode,
                pPlaceObject3->nNumFilters,
                pPlaceObject3->ppFilters
                );
        }
        else if(!pCur->GetASChanged())    // (LEO) $bug871 : AptDisplayList force updates after AS changes property
        {
            return placeObjectNCXForm(
                pCur,
                0,
                NULL,
                NULL,
                pParent,
                0,
                -1,
                eFlags & AptPlaceObjectFlag_CXForm ? &pPlaceObject3->ncxform : NULL,
                eFlags & AptPlaceObjectFlag_Matrix ? &pPlaceObject3->matrix : NULL,
                eFlags & AptPlaceObjectFlag_Actions ? pPlaceObject3->pActions : NULL,
                pPlaceObject3->fRatio,
                pPlaceObject3->nBlendMode,
                pPlaceObject3->nNumFilters,
                pPlaceObject3->ppFilters
                );
        }
    }
    return NULL;
}


/*F************************************************************************************************/
/*!
    \Function    *AptDisplayList::placeObject

    \Description
        Takes an AptPsuedoCIHT object and creates a new instance of it in the main display list

    This function was created for the new AptPseudoCIHT object

    \Input      *pNewItem         -
    \Input      *pParentSprite    -

    \Output     AptCIH            -

    \Notes
        None.

    \Version    1.0        09/01/04 (LEO) First Version
                1.2        03/24/05 (LEO) $bug880 Sub movie clip is reloaded when it should not.
                            Using the Pseudo displaylist, use the nFrameCreated as the nCreatedOnFrame since using the
                            parents current frame for the nCreatedOnFrame in not correct.
    \Version    1.3        04/29/08 (RB)  Handle blendMode and filters from PlaceObject3 tag

*/
/************************************************************************************************F*/
AptCIH *AptDisplayList::placeObject(AptPseudoCIHT *pNewItem, AptCIH *pParentSprite)
{
    AptNativeString     strName;
    AptNativeString *   pName = NULL;
    AptCIH *            pCIH = NULL;

    if ( pNewItem->pControl->eType == AptControlType_PlaceObject2 )
    {
        if (pNewItem->pControl->placeObject2.eFlags & AptPlaceObjectFlag_Name)
        {
            strName = pNewItem->pControl->placeObject2.szName;
            pName = &strName;
        }

        pCIH = placeObjectNCXForm(
            NULL,
            pNewItem->nDepth,
            pNewItem->pControlInfo2->pCharacter,
            pName,
            pParentSprite,
            1,
            pNewItem->pControlInfo2->nClipDepth,
            pNewItem->pControlInfo2->ncxform,
            pNewItem->pControlInfo2->matrix,
            pNewItem->pControlInfo2->pActions,
            pNewItem->pControlInfo2->fRatio
            );
        pCIH->SetCreatedOnFrame( pNewItem->pControlInfo2->nFrameCreated );
    }
    else
    {
        APT_ASSERT( pNewItem->pControl->eType == AptControlType_PlaceObject3 );
        if (pNewItem->pControl->placeObject3.eFlags & AptPlaceObjectFlag_Name)
        {
            strName = pNewItem->pControl->placeObject3.szName;
            pName = &strName;
        }

        pCIH = placeObjectNCXForm(
            NULL,
            pNewItem->nDepth,
            pNewItem->pControlInfo3->pCharacter,
            pName,
            pParentSprite,
            1,
            pNewItem->pControlInfo3->nClipDepth,
            pNewItem->pControlInfo3->ncxform,
            pNewItem->pControlInfo3->matrix,
            pNewItem->pControlInfo3->pActions,
            pNewItem->pControlInfo3->fRatio,
            pNewItem->pControlInfo3->nBlendMode,
            pNewItem->pControlInfo3->nNumFilters,
            pNewItem->pControlInfo3->ppFilters
            );
        pCIH->SetCreatedOnFrame( pNewItem->pControlInfo3->nFrameCreated );
    }
    return pCIH;
}

/*F************************************************************************************************/
/*!
    \Function    AptDisplayList::removeObject

    \Description
        This removes the AptCIH from the parents display list

    This eventually get called by all removeObject functions.  It will remove the name of the sprite from the parents
    hash, and then remove the sprite from the display list.

    \Input      *pItem            -

    \Output
        None.

    \Notes
        -Added code to actually remove the sprite from the display list..
        -!!ATTENTION!! this should be the only place we remove the sprite from the displaylist!

    \Version    1.0        10/27/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptDisplayList::removeObject(AptCIH *pItem)
{
    if (pItem && !pItem->isUndefined()) // sometimes flash removes things that were never placed
    {
        AptValue *pParent = pItem->GetDisplayListParent();
        if (pParent)
        {
            AptNativeHash *pHash = pParent->GetNativeHashVirtual();
            if (pItem->GetInstanceName().IsEmpty() == false)
            {
                // added extra comparison to check if pHash is valid or not in 0.15.05
                if (pHash && pHash->Lookup(&pItem->GetInstanceName()) == pItem)
                {
                    pHash->Unset(&pItem->GetInstanceName());
                }
            }
        }
        pState->AddToDelayReleaseList(pItem, true);
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptDisplayList::removeObject

    \Description
        Insert function's purpose here

    Insert your long description here.

    \Input      nRemovalDepth     -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        10/27/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptDisplayList::removeObject(int nRemovalDepth)
{
    AptCIH *pCur;
    pState->findInst(nRemovalDepth, NULL, &pCur);
    removeObject(pCur);
}

/*F************************************************************************************************/
/*!
    \Function    AptDisplayList::removeObject

    \Description
        Removal of AptCIH from control tag.

    Insert your long description here.

    \Input      *pRemoveObject2   -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        10/27/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptDisplayList::removeObject(AptControlRemoveObject2 *pRemoveObject2)
{
    removeObject(pRemoveObject2->nDepth);
}

/*F************************************************************************************************/
/*!
    \Function    AptDisplayList::removeClonedObject

    \Description
        This function will go and remove the passed in sprite from the display list

    Insert your long description here.

    \Input      *pObject          -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/04/04 (LEO) First Version
        - Removed check for nDepth since a sprite can become an animation and still be removed.  $BUG253 LEO    42/47

*/
/************************************************************************************************F*/
void AptDisplayList::removeClonedObject(AptCIH *pObject)
{
    AptCIH *pCur = NULL;

    pState->findInst(pObject->GetDepth(), NULL, &pCur);
    removeObject(pCur);
}

AptDisplayList::AptDisplayList()
{
    pState = new AptDisplayListState();
}

AptDisplayList::~AptDisplayList()
{
    clear();
    if (pState != NULL)
    {
        delete  pState;
    }
}


#if defined (EA_PLATFORM_PLAYSTATION2) && !defined(APT_USE_FLASH_COLOR_RANGE)

// PSP - GCC compilers are not doing good job here.
// Even if #ifdef ps2 is written, the compiler tries to compile the assembly code written over here.
// so moved this code to another header file and now psp-gcc compiler is not complaining anymore about the assembly code.
// ASK - Changes made for 0.17.02
#include "Display/AptDrawCharacterInstOptiPs2.h"

#else
void AptDisplayList::_drawCharacterInstOpti( const AptRenderItem *pRI)
{
    AptMath::ClipTransformT            *pOutTransform;
    AptMath::ClipTransformT            *pCurTransform;
    AptMath::Mat44T                     *pMatrix;

    pCurTransform = AptMath::_ClipStackGetTop();
    pOutTransform = AptMath::ClipStackPush();

    // swizzle Apt matrix to MatT
    pMatrix = &pOutTransform->Pos44;
    AptMath::MatConvert(pMatrix,(AptMatrix *)pRI->GetPositionMatrixConst());

#if !defined(APT_3D)
    AptMath::MatMul2d(pMatrix, &pCurTransform->Pos44, &pOutTransform->Pos44);
#else
    // Passing the information about the 3d rotation to the ClipTransform
    // It is used to calculate the mask position
    float xAng = pRI->GetXRotation();
    float yAng = pRI->GetYRotation();
    pOutTransform->xrotation = static_cast<int16_t>(xAng);
    pOutTransform->yrotation = static_cast<int16_t>(yAng);
    pOutTransform->zposition = pRI->GetZPosition();

    AptMath::MatRotate3d(pMatrix, xAng, yAng, pRI->GetZPosition(), pRI->GetZScale());
    AptMath::MatMul3d(pMatrix, &pCurTransform->Pos44, pMatrix);
#endif  // #if defined(APT_3D)

    // extract the color transform data
    if(pRI->GetColorMatrixConst() == &gIdentityCXForm)     // Do not do this if pCXForm is the idenity CXForm
    {
        pOutTransform->vColorMul4.Copy(&pCurTransform->vColorMul4);
        pOutTransform->vColorAdd4.Copy(&pCurTransform->vColorAdd4);
    }
    else
    {
        pOutTransform->vColorMul4.Copy(&pRI->GetColorMatrixConst()->scale);
        pOutTransform->vColorAdd4.Copy(&pRI->GetColorMatrixConst()->translate);

        pOutTransform->vColorMul4.SetValuef(AptColorHelper::Alpha, (pOutTransform->vColorMul4.GetValuef(AptColorHelper::Alpha) * pCurTransform->vColorMul4.GetValuef(AptColorHelper::Alpha)) / AptColorHelperScale::SCALE_FACTOR);
        pOutTransform->vColorMul4.SetValuef(AptColorHelper::Red,   (pOutTransform->vColorMul4.GetValuef(AptColorHelper::Red) * pCurTransform->vColorMul4.GetValuef(AptColorHelper::Red)) / AptColorHelperScale::SCALE_FACTOR);
        pOutTransform->vColorMul4.SetValuef(AptColorHelper::Green, (pOutTransform->vColorMul4.GetValuef(AptColorHelper::Green) * pCurTransform->vColorMul4.GetValuef(AptColorHelper::Green)) / AptColorHelperScale::SCALE_FACTOR);
        pOutTransform->vColorMul4.SetValuef(AptColorHelper::Blue,  (pOutTransform->vColorMul4.GetValuef(AptColorHelper::Blue) * pCurTransform->vColorMul4.GetValuef(AptColorHelper::Blue)) / AptColorHelperScale::SCALE_FACTOR);

        pOutTransform->vColorAdd4.SetValuef(AptColorHelper::Alpha, (pOutTransform->vColorAdd4.GetValuef(AptColorHelper::Alpha) + pCurTransform->vColorAdd4.GetValuef(AptColorHelper::Alpha)) );
        pOutTransform->vColorAdd4.SetValuef(AptColorHelper::Red,   (pOutTransform->vColorAdd4.GetValuef(AptColorHelper::Red) + pCurTransform->vColorAdd4.GetValuef(AptColorHelper::Red)) );
        pOutTransform->vColorAdd4.SetValuef(AptColorHelper::Green, (pOutTransform->vColorAdd4.GetValuef(AptColorHelper::Green) + pCurTransform->vColorAdd4.GetValuef(AptColorHelper::Green)) );
        pOutTransform->vColorAdd4.SetValuef(AptColorHelper::Blue,  (pOutTransform->vColorAdd4.GetValuef(AptColorHelper::Blue) + pCurTransform->vColorAdd4.GetValuef(AptColorHelper::Blue)) );

        // The following code has been commented out because MultiplyColors and AddColors have not been fully tested yet..  but it seems to work
        //pOutTransform->vColorMul4.SetValue(AptColorHelper::Alpha, AptColorHelper::MultiplyColors(pOutTransform->vColorMul4.GetA(), pCurTransform->vColorMul4.GetA()));
        //pOutTransform->vColorMul4.SetValue(AptColorHelper::Red,   AptColorHelper::MultiplyColors(pOutTransform->vColorMul4.GetR(), pCurTransform->vColorMul4.GetR()));
        //pOutTransform->vColorMul4.SetValue(AptColorHelper::Green, AptColorHelper::MultiplyColors(pOutTransform->vColorMul4.GetG(), pCurTransform->vColorMul4.GetG()));
        //pOutTransform->vColorMul4.SetValue(AptColorHelper::Blue,  AptColorHelper::MultiplyColors(pOutTransform->vColorMul4.GetB(), pCurTransform->vColorMul4.GetB()));

        //pOutTransform->vColorAdd4.SetValue(AptColorHelper::Alpha, AptColorHelper::AddColors(pOutTransform->vColorAdd4.GetA(), pCurTransform->vColorAdd4.GetA()));
        //pOutTransform->vColorAdd4.SetValue(AptColorHelper::Red,   AptColorHelper::AddColors(pOutTransform->vColorAdd4.GetR(), pCurTransform->vColorAdd4.GetR()));
        //pOutTransform->vColorAdd4.SetValue(AptColorHelper::Green, AptColorHelper::AddColors(pOutTransform->vColorAdd4.GetG(), pCurTransform->vColorAdd4.GetG()));
        //pOutTransform->vColorAdd4.SetValue(AptColorHelper::Blue,  AptColorHelper::AddColors(pOutTransform->vColorAdd4.GetB(), pCurTransform->vColorAdd4.GetB()));
    }
}

void AptDisplayList::_drawCharacterInstOpti( const AptMatrix *pCurrMatrix, const AptCXForm *pCurrCXForm)
{
    AptMath::ClipTransformT            *pOutTransform;
    AptMath::ClipTransformT            *pCurTransform;
    AptMath::Mat44T                     *pMatrix;

    pCurTransform = AptMath::_ClipStackGetTop();
    pOutTransform = AptMath::ClipStackPush();

    // swizzle Apt matrix to MatT
    pMatrix = &pOutTransform->Pos44;
    AptMath::MatConvert(pMatrix, pCurrMatrix);

    AptMath::MatMul2d(&pOutTransform->Pos44, &pCurTransform->Pos44, &pOutTransform->Pos44);

    // extract the color transform data
    if(pCurrCXForm == &gIdentityCXForm)     // Do not do this if pCXForm is the idenity CXForm
    {
        pOutTransform->vColorMul4.Copy(&pCurTransform->vColorMul4);
        pOutTransform->vColorAdd4.Copy(&pCurTransform->vColorAdd4);
    }
    else
    {
        pOutTransform->vColorMul4.Copy(&pCurrCXForm->scale);
        pOutTransform->vColorAdd4.Copy(&pCurrCXForm->translate);

        pOutTransform->vColorMul4.SetValuef(AptColorHelper::Alpha, (pOutTransform->vColorMul4.GetValuef(AptColorHelper::Alpha) * pCurTransform->vColorMul4.GetValuef(AptColorHelper::Alpha)) / AptColorHelperScale::SCALE_FACTOR);
        pOutTransform->vColorMul4.SetValuef(AptColorHelper::Red,   (pOutTransform->vColorMul4.GetValuef(AptColorHelper::Red) * pCurTransform->vColorMul4.GetValuef(AptColorHelper::Red)) / AptColorHelperScale::SCALE_FACTOR);
        pOutTransform->vColorMul4.SetValuef(AptColorHelper::Green, (pOutTransform->vColorMul4.GetValuef(AptColorHelper::Green) * pCurTransform->vColorMul4.GetValuef(AptColorHelper::Green)) / AptColorHelperScale::SCALE_FACTOR);
        pOutTransform->vColorMul4.SetValuef(AptColorHelper::Blue,  (pOutTransform->vColorMul4.GetValuef(AptColorHelper::Blue) * pCurTransform->vColorMul4.GetValuef(AptColorHelper::Blue)) / AptColorHelperScale::SCALE_FACTOR);

        pOutTransform->vColorAdd4.SetValuef(AptColorHelper::Alpha, (pOutTransform->vColorAdd4.GetValuef(AptColorHelper::Alpha) + pCurTransform->vColorAdd4.GetValuef(AptColorHelper::Alpha)) );
        pOutTransform->vColorAdd4.SetValuef(AptColorHelper::Red,   (pOutTransform->vColorAdd4.GetValuef(AptColorHelper::Red) + pCurTransform->vColorAdd4.GetValuef(AptColorHelper::Red)) );
        pOutTransform->vColorAdd4.SetValuef(AptColorHelper::Green, (pOutTransform->vColorAdd4.GetValuef(AptColorHelper::Green) + pCurTransform->vColorAdd4.GetValuef(AptColorHelper::Green)) );
        pOutTransform->vColorAdd4.SetValuef(AptColorHelper::Blue,  (pOutTransform->vColorAdd4.GetValuef(AptColorHelper::Blue) + pCurTransform->vColorAdd4.GetValuef(AptColorHelper::Blue)) );

        // The following code has been commented out because MultiplyColors and AddColors have not been fully tested yet..  but it seems to work
        //pOutTransform->vColorMul4.SetValue(AptColorHelper::Alpha, AptColorHelper::MultiplyColors(pOutTransform->vColorMul4.GetA(), pCurTransform->vColorMul4.GetA()));
        //pOutTransform->vColorMul4.SetValue(AptColorHelper::Red,   AptColorHelper::MultiplyColors(pOutTransform->vColorMul4.GetR(), pCurTransform->vColorMul4.GetR()));
        //pOutTransform->vColorMul4.SetValue(AptColorHelper::Green, AptColorHelper::MultiplyColors(pOutTransform->vColorMul4.GetG(), pCurTransform->vColorMul4.GetG()));
        //pOutTransform->vColorMul4.SetValue(AptColorHelper::Blue,  AptColorHelper::MultiplyColors(pOutTransform->vColorMul4.GetB(), pCurTransform->vColorMul4.GetB()));

        //pOutTransform->vColorAdd4.SetValue(AptColorHelper::Alpha, AptColorHelper::AddColors(pOutTransform->vColorAdd4.GetA(), pCurTransform->vColorAdd4.GetA()));
        //pOutTransform->vColorAdd4.SetValue(AptColorHelper::Red,   AptColorHelper::AddColors(pOutTransform->vColorAdd4.GetR(), pCurTransform->vColorAdd4.GetR()));
        //pOutTransform->vColorAdd4.SetValue(AptColorHelper::Green, AptColorHelper::AddColors(pOutTransform->vColorAdd4.GetG(), pCurTransform->vColorAdd4.GetG()));
        //pOutTransform->vColorAdd4.SetValue(AptColorHelper::Blue,  AptColorHelper::AddColors(pOutTransform->vColorAdd4.GetB(), pCurTransform->vColorAdd4.GetB()));
    }
}
#endif // EA_PLATFORM_PLAYSTATION2

void AptDisplayList::_drawCharacterInstAbsoluteOpti( const AptRenderItem *pRI)
{
#if defined(APT_3D)
    AptMath::ClipTransformT            *pCurTransform = AptMath::_ClipStackGetTop();
#endif
    AptMath::ClipTransformT            *pOutTransform = AptMath::ClipStackPush();

    // swizzle Apt matrix to MatT
    AptMath::MatConvert(&pOutTransform->Pos44, pRI->GetMaskPositionMatrixConst());
#if defined(APT_3D)

    // Apply the 3d transform to the mask with the information that comes in the ClipTransform structure
    // This function is only called for masks. If the mask is not rotated in 3d only the z is assigned in MatRotate3d, as before
    // MatRotate3d is the same function used by other render items
    AptMath::MatRotate3d(&pOutTransform->Pos44,
        static_cast<float>(pCurTransform->xrotation),
        static_cast<float>(pCurTransform->yrotation),
        pCurTransform->zposition);

    // Pass the 3d parameters to the next transformation
    pOutTransform->xrotation = static_cast<int16_t>(pRI->GetXRotation());
    pOutTransform->yrotation = static_cast<int16_t>(pRI->GetYRotation());
    pOutTransform->zposition = pRI->GetZPosition();
#endif
    // extract the color transform data
    pOutTransform->vColorMul4.Copy(&(pRI->GetColorMatrixConst()->scale));
    pOutTransform->vColorAdd4.Copy(&(pRI->GetColorMatrixConst()->translate));
}

/*

// this was only used in non-decoupled mode in older code in function void AptDisplayList::render(...)
// but now that function is stubbed out for #if 0 so we do not need this class.

class SortedArrayMask
{
    AptCIH *aMasks[32];
    int nElements;

public:
    SortedArrayMask() : nElements(0) {}
    ~SortedArrayMask() { APT_ASSERT(nElements == 0); }

    void insert(AptCIH *pMask)
    {
        int i;
        for (i = 0; i < nElements && aMasks[i]->GetCharacterInst()->GetClipDepth() >= pMask->GetCharacterInst()->GetClipDepth(); i++) {}
        APT_ASSERT(i < APT_ARRAYSIZE(aMasks));
        // i is where we want to put it
        for (int j = nElements; j > i; j--)
        {
            aMasks[j] = aMasks[j - 1];
        }
        aMasks[i] = pMask;
//      APT_INC(pMask);
        nElements++;
    }

    void remove(int i)
    {
        APT_ASSERT(i >= 0 && i < nElements);
//      APT_DEC(aMasks[i]);
        for (; i < nElements - 1; i++)
        {
            aMasks[i] = aMasks[i + 1];
        }
        nElements--;
    }

    AptCIH *getElement(int i)
    {
        APT_ASSERT(i >= 0 && i < nElements);
        return aMasks[i];
    }


    // DC - Inline it.
    APT_INLINE int getNumElements()
    {
        return nElements;
    }
};
*/

void AptDisplayList::DeallocAssetStringRecursive()
{
    AptCIH *pCur = pState->GetFirstItem();

  //  SortedArrayMask masks;

    while (pCur)
    {

        pCur->DeallocAssetStringRecursive();
        pCur = pCur->GetDisplayListNext();
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptDisplayList::render

    \Description
        Goes through all the AptCIH in this displaylist and displays them. Also handles static masking

    \Input      *pRenderingContext- Current rendering context
    \Input      eMaskOperation    - masking operation
    \Input      nMaskDepth        - number of masks that are nested under this function.
    \Input      eAnimLevels       - enum that tells which levels/depths to display
    \Input      bUseDepthCompare  - boolean to decide whether to use eAnimLevels for filtering or not

    \Output
        None.

    \Notes
    The implementation of nested masks assumes that you cannot stack masks statically (i.e. all of
    these "masks" in the array are actually objects that build together to make one big mask depth.
    I have verified that you cannot in flash have one mask layer mask another mask layer in the same
    movie clip.

    \Version    1.0        Original version
                2.0        02/28/05  (ASK) after 0.19.01 - added 2 more parameters that allow selective display of levels.
                            eAnimLevels is passed from AptRender at higher level and bUseDepthCompare helps to
                            decide whether to do the comparison of depth number and eAnimLevels or not.
                            This feature was added so that game teams can selectively render animation levels by calling
                            something like AptRender(AptAnimLevel_0 | AptAnimLevel_5 ), which only renders level0 and level5
                3.0        03/07/05 (DC) Cleaned up and fixed masking problems. (would not subtract masks properly.)
                4.0        03/18/05 (DC) $fr840 - Added support for nested masks (maintained via nMaskDepth param).

*/
/************************************************************************************************F*/
void AptDisplayList::render(AptRenderingContext *pRenderingContext, AptMaskRenderOperation eMaskOperation, int nMaskDepth, AptAnimLevelE eAnimLevels, bool bUseDepthCompare)
{
#if 0
    APT_PROFILE_RENDER(AptDisplayList_render);
    AptCIH *pCur = pState->pHead->GetDisplayListNext();
    SortedArrayMask masks;

    {
        APT_PROFILE_RENDER(AptDisplayList_render_loop);

        // Set a flag when a mask is added, to ensure the first non-mask item rendered increments the mask depth.
        // (Objects are masked by anything in a lower mask depth)
        bool addingMasks = false;

        while (pCur)
        {
            // added new comparison to filter out levels to be displayed.
            // this will be used only at root levels, below root level in tree bUseDepthCompare is set to false
            if (bUseDepthCompare)
            {
                if (!(((int)(1 << pCur->GetDepth())) & eAnimLevels))
                {
                    pCur = pCur->GetDisplayListNext();
                    continue ;
                }
            }

                // First things first, clear any static masks that are done.
                if(masks.getNumElements() > 0)   // This is checked in the loop, but lets just skip the loop unless it is true.
                {
                    // keep track of then a mask is removed, because when the first (and only the first) mask is removed, we need to
                    // be sure to decrease the mask depth, so that the masks are subtracted at the depth they were previously drawn at.
                    bool removingMasks = false;

                    // DC - remove all clipping layers (Masks) that have a clip depth less then the depth of the item we are about to render.
                    APT_PROFILE_RENDER(AptDisplayList_render_loopmasks);
                    while (masks.getNumElements() > 0 && masks.getElement(masks.getNumElements()-1)->GetCharacterInst()->GetClipDepth() < pCur->GetDepth())
                    {
                        AptCIH * pMask = masks.getElement(masks.getNumElements()-1);

                        // make sure the first one decrements the mask depth, so that the masks will be cleared at the level they were drawn at.
                        if(!removingMasks)
                        {
                            // only decrement once.
                            nMaskDepth--;
                            removingMasks = true;
                        }

                        pMask->render(pRenderingContext, NULL, AptMaskRenderOperation_Subtract, nMaskDepth);
                        masks.remove(masks.getNumElements()-1);
                    }
                }

                // Now render what we can.
                if (!pCur->isUndefined() && !pCur->IsLevelInst() && !pCur->isMask() && !pCur->IsSkipEval())
                {
                    // If the object is a static mask, Add it.
                    if (pCur->GetCharacterInst()->nClipDepth >= 0) // this obj is a clipper, not a drawn object
                    {
                        masks.insert(pCur);
                        pCur->render(pRenderingContext, NULL, AptMaskRenderOperation_Add, nMaskDepth);

                        // Set a flag so that we know a mask was added.
                        addingMasks = true;
                    }
                    else
                    {
                        APT_PROFILE_RENDER(AptDisplayList_render_clip);

                        // If a mask was added, be sure to increment the mask depth before rendering the object / objects being masked
                        if(addingMasks == 1)
                        {
                            nMaskDepth++;
                            addingMasks = false;
                        }

                        pCur->render(pRenderingContext, NULL, eMaskOperation, nMaskDepth);
                    }
                }

            pCur = pCur->GetDisplayListNext();
        }

    } // end of APT_PROFILE outside while

    // OK, Done, clear any masks we have may have left.
    if(masks.getNumElements() > 0)
    {
        // Only decrement the mask depth once.
        nMaskDepth--;
        while (masks.getNumElements() > 0)
        {
            masks.getElement(0)->render(pRenderingContext, NULL, AptMaskRenderOperation_Subtract, nMaskDepth);
            masks.remove(0);
        }
    }
#endif
}
void AptDisplayList::GetBoundingRect(AptRenderingContext *pRenderingContext, const AptMatrix * pCurrentTransform, AptRect *pRect) const
{
    AptCIH *pCur = pState->GetFirstItem();

    while (pCur)
    {
        if (!pCur->isUndefined() && !pCur->IsLevelInst() && pCur->GetCharacterInst()->GetClipDepth() < 0)
        {
            pCur->GetBoundingRect(pRenderingContext, pCurrentTransform, pRect);
        }

        pCur = pCur->GetDisplayListNext();
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptDisplayList::tick

    \Description
        Goes through all the AptCIHs in this displaylist and ticks them to next frame.

    \Input      eAnimLevels       - enum that tells which levels/depths to tick
    \Input      bUseDepthCompare  - boolean to decide whether to use eAnimLevels for filtering or not

    \Output
        None.

    \Notes
        None.

    \Version    1.0        Original Version.
                2.0        02/28/05 (ASK)  after 0.19.01 - added 2 parameters that allow selective tick/update of levels.
                            eAnimLevels is passed from AptUpdate() at higher level and bUseDepthCompare helps to
                            decide whether to do the comparison of depth number and eAnimLevels or not.
                            This feature was added so that game teams can selectively update/tick animation levels by calling
                            something like AptUpdate(AptAnimLevel_0 | AptAnimLevel_5 ), which only update level0 and level5
*/
/************************************************************************************************F*/
uint32_t AptDisplayList::tick(AptAnimLevelE eAnimLevels, bool bUseDepthCompare)
{
	// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
    //APT_PROFILE_BEGIN( APT_DL_TICK, "AptDisplayList::tick" );
    //APT_PROFILE(AptDisplayList_tick);
    // FIFA_END
	AptCIH *pCur = pState->GetFirstItem();
    uint32_t bState = 0;

    APT_PREFETCH(0, pCur);

    while (pCur)
    {
        AptCIH *next = pCur->GetDisplayListNext();

        APT_PREFETCH(0, next);

        // added new comparison to filter out levels to be ticked.
        // this will be used only at root levels, below root level in tree bUseDepthCompare is set to false
        if (bUseDepthCompare)
        {
            if (!(((int)(1 << pCur->GetDepth())) & eAnimLevels))
            {
                pCur = next;
                continue ;
            }
        }
        else if(pCur->GetCIHState() == AptCIH::AptCIHState_Unloaded)
        {
            pCur = next;
            continue;
        }

        if (pCur->IsSpriteInstBase() || pCur->IsButtonInst())
        {
            bState |= pCur->tick();
        }
        pCur = next;
    }
	// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
    //APT_PROFILE_END( APT_DL_TICK );
    // FIFA_END
	return bState;
}





/*F************************************************************************************************/
/*!
    \Function    AptDisplayList::GeneralisedProcess

    \Description
        Generalized function processing displaylist recursively.

    \Input      pDL               - pointer to display list that needs to be processed
    \Input      pVoid             - void pointer to pass in any type of data
    \Input      eAnimLevels       - enum to decide which levels to process
    \Input      bUseDepthCompare  - flag to decide whether to use enums or not.

    \Output
        This function will be used for processing all items in all the displaylists.
        So start from GetTargetSim()->GetAnimationTarget()->displaylist and it calls recursively thru AptCIH.
        Before calling this function you can set AptCIH::sCIHProcessCb to a function
        which will get called for every AptCIH in displaylist.

    \Notes
        None.

    \Version    1.0        04/22/05 (ASK) First Version

*/
/************************************************************************************************F*/
uint32_t AptDisplayList::GeneralisedProcess(AptDisplayList * pDL, void * pVoid, AptAnimLevelE eAnimLevels, bool bUseDepthCompare)
{
	// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
    //APT_PROFILE_BEGIN( APT_DL_GENERALISED_PROCESS, "AptDisplayList::GeneralisedProcess" );
    //APT_PROFILE(AptDisplayList_tick);
    // FIFA_END
	AptCIH *pCur = pDL->getState()->GetFirstItem();
    uint32_t bState = 0;

    APT_PREFETCH(0, pCur);

    while (pCur)
    {
        AptCIH *next = pCur->GetDisplayListNext();

        APT_PREFETCH(0, next);

        // added new comparison to filter out levels to be ticked.
        // this will be used only at root levels, below root level in tree bUseDepthCompare is set to false
        if (bUseDepthCompare)
        {
            if (!(((int)(1 << pCur->GetDepth())) & eAnimLevels))
            {
                pCur = next;
                continue ;
            }
        }

        bState |= AptCIH::GeneralisedProcess(pCur, pVoid) ;
        pCur = next;
    }
	// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
    //APT_PROFILE_END( APT_DL_GENERALISED_PROCESS );
    // FIFA_END
	return bState;
}




// Clear this display list, properly dereferencing pool elements and such.
/*F************************************************************************************************/
/*!
    \Function    AptDisplayList::clear

    \Description
        This function removes all items from the current display list.

    Insert your long description here.

    \Input      bClean            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/04/04 (LEO) First Version
        -Added code to go and clean out the AptCIH's in the display list        $BUG253 LEO 41/47
*/
/************************************************************************************************F*/
void AptDisplayList::clear(bool bClean)
{
    if (!pState)
    {
        return ;
    }
    AptCIH *pCur = pState->GetFirstItem();
    AptCIH *pNext;

    while (pCur)
    {
        pNext = pCur->GetDisplayListNext();
        APT_INC(pCur);  // Keep pCur Alive until we are done cleaning it up!
        removeObject(pCur);
        if(bClean)
        {
            pCur->setGCRoot(0);
            pCur->ClearCIH(true);
        }
        if ((gpValuesToRelease->GetNumValues() != 0) && (gAptActionInterpreter.stack.GetSize() == 0))
        {
            //  There are some values to collect, we do it
            //  The same test is done internally but by doing that we avoid to call this function
            //  if not needed
            gpValuesToRelease->ReleaseValues();
        }
        APT_DEC(pCur);
        pCur = pNext;
    }
}

// this will be called only from GetTargetSim()->GetAnimationTarget()->PreDestroy to clear of the display list.
// it should also delete the pState  and set it to null so ~displayList will not try to delete/clear it again.
void AptDisplayList::PreDestroy()
{
    if(pState != NULL)
    {
        APT_ASSERT(pState);
        clear() ;
        delete  pState;
        pState = NULL;

    }
}

// Uses a new state struct.
void AptDisplayList::useState(AptDisplayListState *pNewState)
{
    pState = pNewState;
}

/*F************************************************************************************************/
/*!
    \Function    AptDisplayList::RemoveFromDisplayList

    \Description
        Fully removes an item from the display list

    Removes an item from the display list, removes the object from any listener sets it was in, and
    removes the instance of it in the parent hash.

    \Input      *pHash            -
    \Input      *pTmp             -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/14/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptDisplayList::RemoveFromDisplayList(AptNativeHash *pHash, AptCIH *pTmp)
{
    //pTmp->ClearCIH(true);
    //GetTargetSim()->GetAnimationTarget()->removeActionFor(pTmp);           // remove any actions associated with the original context (since we're replaceing it)
    //GetTargetSim()->GetAnimationTarget()->removeFromBIL(pTmp);
    //GetTargetSim()->GetAnimationTarget()->GetButtonSet()->remove(pTmp);
    //GetTargetSim()->GetAnimationTarget()->GetInputSet()->remove(pTmp);
    //if (GetTargetSim()->GetAnimationTarget()->pFocusButton == pTmp)
    //{
    //    APT_DEC(GetTargetSim()->GetAnimationTarget()->pFocusButton);
    //    GetTargetSim()->GetAnimationTarget()->pFocusButton = 0;
    //}

    //if(pTmp->GetInstanceName().IsEmpty() == false)    // Remove sprite name from parent hash
    //{
    //    pHash->Unset(&pTmp->GetInstanceName());
    //}
    removeObject(pTmp);
}

/*F************************************************************************************************/
/*!
    \Function    AptDisplayList::AddToDisplayList

    \Description
        Adds an item to the display list

    This function creates a new AptCIH out of the AptPseudoCIHT object in the display list

    \Input      *pHash            -
    \Input      *pNewControl      -
    \Input      *pParentCIH       -

    \Output     AptCIH *          -

    \Notes
        -Removed the _addToSetCaches since it gets called in placeObject

    \Version    1.0        09/14/04 (LEO) First Version
    \Version    1.2        04/29/08 (RB)  Refactor to handle PlaceObject3 tag. It looks like
                                          a lot changed but really, it's just a refactoring.
*/
/************************************************************************************************F*/
AptCIH * AptDisplayList::AddToDisplayList(AptNativeHash *pHash, AptPseudoCIHT *pNewControl, AptCIH *pParentCIH)
{
    AptCharacterAnimation *pTmpAnim = &pParentCIH->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation;

    int nCharacterID = -1;
    const AptCharacter* pCharacter = NULL;
    if( pNewControl->pControl->eType == AptControlType_PlaceObject2 )
    {
        nCharacterID = pNewControl->pControl->placeObject2.nCharacterID;
        pCharacter = pNewControl->pControlInfo2->pCharacter;
    }
    else if ( pNewControl->pControl->eType == AptControlType_PlaceObject3 )
    {
        nCharacterID = pNewControl->pControl->placeObject3.nCharacterID;
        pCharacter = pNewControl->pControlInfo3->pCharacter;
    }

    pTmpAnim->ExecuteInitActions(pParentCIH, nCharacterID);

    if(nCharacterID != -1)
    {
        AptCIH *pSprInst = pParentCIH;
        AptCharacterAnimation * pTmpAnim = &pSprInst->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation;

        if(pCharacter->eType != AptCharacterType_Animation && pCharacter->m_pAnimFile.pData == NULL )
        {
            int nImpID = pTmpAnim->IsImport(nCharacterID);
            if(nImpID != -1)
            {
                APT_ASSERT(pTmpAnim->aImports[nImpID].nID == nCharacterID);
                pCharacter->m_pAnimFile = pTmpAnim->aImports[nImpID].file;
            }
            else
            {
                APT_ASSERT(pCharacter->pParentAnim == pSprInst->GetCharacterInst()->GetCharacterConst()->pParentAnim);
                pCharacter->m_pAnimFile = pSprInst->GetCharacterInst()->GetCharacterConst()->m_pAnimFile;
            }
        }

    }

    AptCIH *pTmp = placeObject(pNewControl, pParentCIH);

    if(pTmp->GetInstanceName().IsEmpty() == false)
    {
        pHash->Set(&pTmp->GetInstanceName(), pTmp);
    }

    GetTargetSim()->GetAnimationTarget()->GetNewInsts()[GetTargetSim()->GetAnimationTarget()->GetNewInstSize()] = pTmp;

    APT_INC(GetTargetSim()->GetAnimationTarget()->GetNewInsts()[GetTargetSim()->GetAnimationTarget()->GetNewInstSize()]);
    GetTargetSim()->GetAnimationTarget()->IncNewInstSize();
    //_addToSetCaches(pTmp, 0);
    return pTmp;
}

/*F************************************************************************************************/
/*!
    \Function    AptDisplayList::ReplaceDisplyListItem

    \Description
        Replaces an item in the display list with the AptPseudoCIHT object

    If pCharacter is a valid pointer, this will remove pOriginalItem from the display list and then add
    the new item in its place.

    \Input      *pHash            -
    \Input      *pOriginalItem    -
    \Input      *pNewItem         -
    \Input      *pParent          -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/14/04 (LEO) First Version
    \Version    1.1        06/03/05 (DC)  This ensures file pointer is setup for placed items.
    \Version    1.2        04/29/08 (RB)  Refactor to handle PlaceObject3 tag
*/
/************************************************************************************************F*/
void AptDisplayList::ReplaceDisplyListItem(AptNativeHash *pHash, AptCIH *pOriginalItem, AptPseudoCIHT *pNewItem, AptCIH *pParent)
{
    APT_ASSERT(pNewItem->nDepth == pOriginalItem->GetDepth());

    const AptCharacter* pCharacter = NULL;
    int32_t nCharacterID = -1;
    AptControlType eControlType = pNewItem->pControl->eType;
    if (eControlType == AptControlType_PlaceObject2)
    {
        pCharacter = pNewItem->pControlInfo2->pCharacter;
        nCharacterID = pNewItem->pControl->placeObject2.nCharacterID;
    }
    else if (eControlType == AptControlType_PlaceObject3)
    {
        pCharacter = pNewItem->pControlInfo3->pCharacter;
        nCharacterID = pNewItem->pControl->placeObject3.nCharacterID;
    }

    if(pCharacter != NULL)
    {
        RemoveFromDisplayList(pHash, pOriginalItem);

        if(nCharacterID != -1)
        {
            AptCIH *pSprInst = pParent;
            AptCharacterAnimation * pTmpAnim = &pSprInst->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation;

            if(pCharacter->eType != AptCharacterType_Animation && pCharacter->m_pAnimFile.pData == NULL )
            {
                int nImpID = pTmpAnim->IsImport(nCharacterID);
                if(nImpID != -1)
                {
                    APT_ASSERT(pTmpAnim->aImports[nImpID].nID == nCharacterID);
                    pCharacter->m_pAnimFile = pTmpAnim->aImports[nImpID].file;
                }
                else
                {
                    APT_ASSERT(pCharacter->pParentAnim == pSprInst->GetCharacterInst()->GetCharacterConst()->pParentAnim);
                    pCharacter->m_pAnimFile = pSprInst->GetCharacterInst()->GetCharacterConst()->m_pAnimFile;
                }
            }

        }

        AddToDisplayList(pHash, pNewItem, pParent);
    }
    else    // If pCharacter is NULL, then this is a placeObject control that simply updates the original cih
    {
        int32_t eFlags = 0;
        const AptMatrix* pMatrix = NULL;
        const AptUint32CXForm* pCXForm = NULL;
        if ( eControlType == AptControlType_PlaceObject2 )
        {
            eFlags = pNewItem->pControlInfo2->eFlags;
            pMatrix = pNewItem->pControlInfo2->matrix;
            pCXForm = pNewItem->pControlInfo2->ncxform;
        }
        else if ( eControlType == AptControlType_PlaceObject3 )
        {
            eFlags = pNewItem->pControlInfo3->eFlags;
            pMatrix = pNewItem->pControlInfo3->matrix;
            pCXForm = pNewItem->pControlInfo3->ncxform;
        }

        if(!pOriginalItem->GetASChanged() )
        {
            if((eFlags) & AptPlaceObjectFlag_CXForm)
            {
                pOriginalItem->GetColorMatrixWritable()->AptUint32CXFormCopy(pCXForm);
            }
            if((eFlags) & AptPlaceObjectFlag_Matrix)
            {
                pOriginalItem->GetPositionMatrixWritable()->AptMatrixCopy(pMatrix);
            }
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptDisplayList::mergeState

    \Description
        Merges two display lists

        In the event of a GotoAndX call, Apt will have two separate display lists to merge; one being the original current display list, and the other being the
        new GotoAndX display list.  This function will then merge the two, adding or removing anything so that the final display list will represent the
        frame that was GotoAndX'ed to.

    \Input      *pNewState        -
    \Input      *pOrigObject      -

    \Output
        None.

    \Notes
        - (LEO) Removed code which jump out early if the pData (AptCharacter) was NULL.  The reason was, unloaded movieclips should remain in the display list
        and the pData == NULL check would merge these out.  Now the nCreatedOnFrame is held in the AptCIH to get around the problem of having a pData == NULL.
        This bug was found when fixing bug 670.
        - (LEO) This was updated to use a pseudo list rather then another DisplayList.

    \Version    1.0        09/01/04 (LEO) First Version
    \Version    2.0        09/31/04 (LEO) Second Version
    \Version    2.1        04/29/08 (RB)  Refactor to handle PlaceObject3 tag
*/
/************************************************************************************************F*/
void AptDisplayList::mergeState(AptPseudoDisplayList *pNewState, AptNativeHash *pOrigObject, bool bJumpAhead)
{
    AptPseudoCIHT *pNewControl = pNewState->GetFirstItem();
    APT_ASSERT(pState);
    AptCIH *pCurrentCIH = pState->GetFirstItem();

    while(pCurrentCIH != NULL)
    {
        if (pCurrentCIH->GetDepth() >= AptDisplayList::BASE_MOVIE_DEPTH)
        {
            // If the current pCurrentCIH nDepth is >= 16384, then these are items that were places via actionscript
            //  and should always be kept in the current display list.
            while (pNewControl && pNewControl->nDepth < pCurrentCIH->GetDepth())
            {
                if(pNewControl->pControl->eType == AptControlType_PlaceObject2 ||
                   pNewControl->pControl->eType == AptControlType_PlaceObject3)                 // (LEO) we only need to add the place object control
                {
                    AddToDisplayList(pOrigObject, pNewControl, pNewState->GetParentSprite());
                }
                else    // AptControlType_RemoveObject2
                {
                    // We can ignore the removeObject control here since it was never in the main display list
                }
                pNewControl = pNewControl->GetDisplayListNext();
            }
            APT_ASSERT(pNewControl == NULL);        // there should never be any dupes in the target
            break;
        }

        if(pNewControl && (pNewControl->nDepth == pCurrentCIH->GetDepth()))
        {
            // Here we have a clash of items at the same depth, we need to pick which one we want, the new or the old one
            if(pCurrentCIH->IsSpriteInstBase() && pCurrentCIH->GetSpriteInstBase()->GetCreatedDynamic())
            {
                pCurrentCIH = pCurrentCIH->GetDisplayListNext();
                pNewControl = pNewControl->GetDisplayListNext();
                continue;
            }

            AptCIH *pTmpNext = pCurrentCIH->GetDisplayListNext();

            if(pNewControl->pControl->eType == AptControlType_PlaceObject2 || pNewControl->pControl->eType == AptControlType_PlaceObject3)
            {
                int32_t nFrameCreated = -1;
                const AptCharacter* pCharacter = NULL;
                int32_t eControlFlags = 0;
                const AptMatrix* pMatrix = NULL;
                const AptUint32CXForm* pCXForm = NULL;

                bool bHaveControlInfo = false;

                if(pNewControl->pControl->eType == AptControlType_PlaceObject2 && pNewControl->pControlInfo2 )
                {
                    nFrameCreated = pNewControl->pControlInfo2->nFrameCreated;
                    pCharacter = pNewControl->pControlInfo2->pCharacter;
                    eControlFlags = pNewControl->pControlInfo2->eFlags;
                    bHaveControlInfo = true;
                    pMatrix = pNewControl->pControlInfo2->matrix;
                    pCXForm = pNewControl->pControlInfo2->ncxform;
                }
                else if(pNewControl->pControl->eType == AptControlType_PlaceObject3 && pNewControl->pControlInfo3 )
                {
                    nFrameCreated = pNewControl->pControlInfo3->nFrameCreated;
                    pCharacter = pNewControl->pControlInfo3->pCharacter;
                    eControlFlags = pNewControl->pControlInfo3->eFlags;
                    pMatrix = pNewControl->pControlInfo3->matrix;
                    pCXForm = pNewControl->pControlInfo3->ncxform;
                    bHaveControlInfo = true;
                }

                // Here we have a placeObject2 place control, we need to make a choice as to which to keep
                if( bHaveControlInfo
                    && ((nFrameCreated == pCurrentCIH->GetCreatedOnFrame())
                    || pCurrentCIH->IsAnimationInst())
                    )
                {
                    // $BUG 731 DG Case when pCharacter == NULL, forcibly updating parameters
                    if(pCharacter == NULL)
                    {

                        if( !pCurrentCIH->GetASChanged() )
                        {
                            if(eControlFlags & AptPlaceObjectFlag_CXForm)
                            {
                                pCurrentCIH->GetColorMatrixWritable()->AptUint32CXFormCopy(pCXForm);
                            }

                            if(eControlFlags & AptPlaceObjectFlag_Matrix)
                            {
                                pCurrentCIH->GetPositionMatrixWritable()->AptMatrixCopy(pMatrix);
                            }
                        }
                    }
                    else
                    {
                        APT_ASSERT(bHaveControlInfo);

                        if((   pCurrentCIH->GetCharacterInst()->GetCharacterType() == pCharacter->eType
                            && pCharacter == pCurrentCIH->GetCharacterInst()->GetCharacterConst()
                            && nFrameCreated == pCurrentCIH->GetCreatedOnFrame())
                            || pCurrentCIH->IsAnimationInst())
                        {
                            // (LEO) here we leave the original item and copy over the place object information.
                            if( !pCurrentCIH->GetASChanged() )
                            {
                                if(eControlFlags & AptPlaceObjectFlag_CXForm)
                                {
                                    pCurrentCIH->GetColorMatrixWritable()->AptUint32CXFormCopy(pCXForm);
                                }
                                else
                                {
                                    // $bug 1034 cxform not set back to original in a specific case of mergestate
                                    AptColorHelperScale& vColorMul4 = pCurrentCIH->GetColorMatrixWritable()->scale;
                                    AptColorHelperTranslate& vColorAdd4 = pCurrentCIH->GetColorMatrixWritable()->translate;

                                    vColorMul4.SetValuef(AptColorHelper::Red, 255.f);
                                    vColorMul4.SetValuef(AptColorHelper::Green, 255.f );
                                    vColorMul4.SetValuef(AptColorHelper::Blue, 255.f );
                                    vColorMul4.SetValuef(AptColorHelper::Alpha, 255.f );
                                    vColorAdd4.SetValuef(AptColorHelper::Red, 0.f);
                                    vColorAdd4.SetValuef(AptColorHelper::Green, 0.f);
                                    vColorAdd4.SetValuef(AptColorHelper::Blue, 0.f);
                                    vColorAdd4.SetValuef(AptColorHelper::Alpha, 0.f);
                                }


                                // DC -- Only move Matrix if the flag is set.
                                if(eControlFlags & AptPlaceObjectFlag_Matrix)
                                {
                                    pCurrentCIH->GetPositionMatrixWritable()->AptMatrixCopy(pMatrix);
                                }
                            }
                        }
                        else            // Replace the original item with the new one
                        {
                            //gAptFuncs.pfnDebugPrint("0: Replacing %p \n", pCurrentCIH );
                            ReplaceDisplyListItem(pOrigObject, pCurrentCIH, pNewControl, pNewState->GetParentSprite());
                        }
                    }
                }
                else            // Replace the original item with the new one
                {
                    //gAptFuncs.pfnDebugPrint("1: Replacing %p \n", pCurrentCIH );
                    ReplaceDisplyListItem(pOrigObject, pCurrentCIH, pNewControl, pNewState->GetParentSprite());
                }
            }
            else
            {
                // We have a removeObject2 place control, so simply remove the item from the original display list
                //gAptFuncs.pfnDebugPrint("2: Removing %p\n",pCurrentCIH );
                RemoveFromDisplayList(pOrigObject, pCurrentCIH);
            }
            pNewControl = pNewControl->GetDisplayListNext();
            pCurrentCIH = pTmpNext;
        }
        else if (pNewControl == NULL || (pNewControl->nDepth > pCurrentCIH->GetDepth()))
        {
            // Depending on whether bJumpAhead is true, here we have a new item that is higher then the current CIH,
            //  so we have to catch up the pCurrentCIH pointer to the pNewControl
            AptCIH *pCurrentCIHNext = pCurrentCIH->GetDisplayListNext();
            if(bJumpAhead)
            {
                // Here, since we are starting from the original display list state, we can ignore this case since we need to keep this pCurrentCIH in the display list
                // Do nothing
            }
            else
            {
                // Here we start from frame 0, so if there are any pNewControls higher than the pCurrentCIH, we must remove them from
                //  the original display list since they no longer belong in the original display list in the frame jumped to.
                //if(pCurrentCIH->IsSpriteInstBase() && !pCurrentCIH->GetSpriteInst()->bCreatedDynamic)
                {
                    //gAptFuncs.pfnDebugPrint("3: Removing %p\n", pCurrentCIH );
                    RemoveFromDisplayList(pOrigObject, pCurrentCIH);
                }
            }
            pCurrentCIH = pCurrentCIHNext;
        }
        else    // pNewControl.nDepth < pCurrentCIH.nDepth
        {
            // here we have an item we want to simply add to the main display list
            while (pNewControl)
            {
                if (pNewControl->nDepth >= pCurrentCIH->GetDepth())
                {
                    break;
                }
                if(pNewControl->pControl->eType == AptControlType_PlaceObject2 || pNewControl->pControl->eType == AptControlType_PlaceObject3)
                {
                    //gAptFuncs.pfnDebugPrint("4: Adding %p\n", pNewControl );
                    AddToDisplayList(pOrigObject, pNewControl, pNewState->GetParentSprite());
                }
                else
                {
                    // We have a removeObject2 so we can ignore it
                }
                pNewControl = pNewControl->GetDisplayListNext();
            }
        }
    }

    if (pNewControl)
    {
        // Now, we need add all extra items to the main display list
        while (pNewControl)
        {
            if(pNewControl->pControl->eType == AptControlType_PlaceObject2 || pNewControl->pControl->eType == AptControlType_PlaceObject3)
            {
                //gAptFuncs.pfnDebugPrint("5: Adding %p\n", pNewControl );
                AddToDisplayList(pOrigObject, pNewControl, pNewState->GetParentSprite());
            }
            else
            {
                // Do nothing since we are working solely from the pseudo items
            }
            pNewControl = pNewControl->GetDisplayListNext();
        }
    }
}

void AptDisplayList::validate(AptCIH *pParent)
{
    APT_ASSERT(pState);
    AptCIH *pSourceCur = pState->GetFirstItem();

    while (pSourceCur)
    {
        if (pSourceCur->GetDisplayListParent() != pParent)
        {
            APT_ASSERT(0);
        }

        if (pParent && (pSourceCur->GetInstanceName().IsEmpty() == false) && !pSourceCur->isUndefined())
        {
            AptCIH *pParentInst = pSourceCur->GetDisplayListParent();
            APT_ASSERT(pParentInst->IsSpriteInstBase());
            AptHashItem *pItem = NULL;

            for (pItem = pParentInst->GetCharacterInst()->GetNativeHash()->GetFirstItem(); pItem != NULL; pItem = pParentInst->GetCharacterInst()->GetNativeHash()->GetNextItem(pItem))
            {
                AptValue *  pValue = pItem->mValue;
                if (!pValue->isCIH())
                    continue;

                if (pValue == pSourceCur || (pItem->Key == pSourceCur->GetInstanceName()))
                {
                    break;
                }
            }
        }

        if (pSourceCur->IsSpriteInstBase())
        {
            pSourceCur->GetSpriteInstBase()->mDisplayList.validate(pSourceCur);
        }
#if defined APT_USE_BUTTONS
        if (pSourceCur->IsButtonInst())
        {
            pSourceCur->GetButtonInst()->mDisplayList.validate(pSourceCur) ;
        }
#endif

        pSourceCur = pSourceCur->GetDisplayListNext();
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptDisplayListState::HasRenderData

    \Description
        Goes thru all the nodes of list and calls HasRenderData() on every item.
        Returns true if any AptCIH item in list has a render/displayable data.

    \Input
        None.

    \Output     bool  - Returns true if a AptCIH item in list has a render data.

    \Notes
        None.

    \Version    1.0        09/13/04 (ASK) First Version

*/
/************************************************************************************************F*/
bool  AptDisplayListState::HasRenderData() const
{
    const AptCIH * pTemp =  pHead ;
    while (pTemp)
    {
        if (pTemp->HasRenderData())
        {
            return (true) ;
        }
        pTemp = pTemp->GetDisplayListNext() ;
    }
    return (false);
}


/*F************************************************************************************************/
/*!
    \Function    void   AptDisplayListState::GetMovieclipInfo

    \Description
        This function will go thru displaylist and call GetMovieclipInfo
        on each of the AptCIHs in the list.

    \Input      pMCInfo           - pointer to AptMovieclipInformation structure.

    \Output

    \Notes
        Each call to AptCIH::GetMovieclipInfo will add to the counters.
        So at the end we will have the correct number of movieclips.
        pMCInfo in in-out parameter.

    \Version    1.0        09/14/04 (ASK) First Version

*/
/************************************************************************************************F*/
void    AptDisplayListState::GetMovieclipInfo(AptMovieclipInformation * pMCInfo) const
{
    const AptCIH * pTemp =  pHead ;
    while (pTemp)
    {
        pTemp->GetMovieclipInfo(pMCInfo) ;
        pTemp = pTemp->GetDisplayListNext() ;
    }
    return ;
}

/*F************************************************************************************************/
/*!
    \Function    AptDisplayListState::AptDisplayListState

    \Description
        Insert function's purpose here

    Insert your long description here.

    \Input
        None.

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        09/16/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptDisplayListState::AptDisplayListState():
    pHead(NULL)
{

}


/*F************************************************************************************************/
/*!
    \Function    AptDisplayListState::~AptDisplayListState

    \Description
        Insert function's purpose here

    Insert your long description here.

    \Input
        None.

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        09/16/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptDisplayListState::~AptDisplayListState()
{
    APT_ASSERT(pHead == NULL);
    pHead = NULL;
}


// FIFA_BEGIN | DIVERGENCE | nmeyer | 2020-02-03 | Add support for dumping display lists. Useful for debugging and tracking memory leaks in the UI.
void dumpDisplayListIndent(FILE *file, int depth)
{
#ifndef EA_RETAIL
	for (int i = 0; i < depth; ++i)
		fprintf(file, "\t");
#endif
}

void dumpDisplayListNode(FILE *file, const AptValue *parent, AptNativeString& strBuffer, eastl::vector<const AptValue *> &stack)
{
#ifndef EA_RETAIL
	eastl_size_t stackPos = stack.size();
	stack.push_back(parent);

	while (stack.size() > stackPos)
	{
		const AptValue *&current = stack.back();
		if (current)
		{
			// Calculate the tree depth to properly indent entries in the output file
			int currentDepth = stack.size() - 1;

			// Identify node if it is a CIH object
			if (current->isCIH())
			{
				current->toString(strBuffer);
				dumpDisplayListIndent(file, currentDepth);
				fprintf(file, "+%s (%p)", strBuffer.ConstRawPtr(), current);
			}

			// Determine if this object already exists in the stack to avoid circular logging.
			bool isCircular = false;
			for (int i = (stack.size() - 2); i >= 0; --i)
			{
				if (isCircular = (stack[i] == current))
					break;
			}

			if (isCircular)
			{
				fprintf(file, " {Circular Link}\n");
			}
			else
			{
				fprintf(file, "\n");

				// Output object properties. Requires a CIH or AptObject type.
				AptNativeHash *nativeHash = nullptr;
				if (current->isCIH())
				{
					nativeHash = current->c_cih()->GetNativeHash();
				}
				else if (current->isObject())
				{
					nativeHash = current->c_object()->GetNativeHashVirtual();
				}

				if (nativeHash)
				{
					AptHashItem *hashItem = nativeHash->GetFirstItem();
					if (hashItem)
					{
						dumpDisplayListIndent(file, currentDepth + 1);
						fprintf(file, "[\n");
						while (hashItem)
						{
							dumpDisplayListIndent(file, currentDepth + 2);
							hashItem->mValue->toString(strBuffer);
							fprintf(file, "%s = %s (%p)", hashItem->Key.c_str(), strBuffer.ConstRawPtr(), hashItem->mValue);

							// If this property is an object, then traverse its hierarchy as well
							if (hashItem->mValue->isObject())
							{
								dumpDisplayListNode(file, hashItem->mValue, strBuffer, stack);
							}
							else
							{
								fprintf(file, "\n");
							}

							hashItem = nativeHash->GetNextItem(hashItem);
						}
						dumpDisplayListIndent(file, currentDepth + 1);
						fprintf(file, "]\n");
					}
				}

				// Print additional information regarding CIH objects
				if (current->isCIH())
				{
					if ( current->c_cih()->IsDynamicTextInst() )
					{
						AptCharacterTextInst* dynamicText = current->c_cih()->GetDynamicTextInst();
						if (dynamicText)
						{
							dumpDisplayListIndent(file, currentDepth + 1);
							fprintf(file, "*Text: %s (%p)\n",
									dynamicText->GetTextValueConst().c_str(),
									&dynamicText->GetTextValueConst());
						}
					}

					if ( current->c_cih()->IsAnimationInst() )
					{
						AptCharacterAnimationInst* animation = current->c_cih()->GetAnimationInst();
						if (animation)
						{
							dumpDisplayListIndent(file, currentDepth + 1);
							fprintf(file, "*Animation: %s (%p)\n",
									animation->mpFile ? animation->mpFile->GetName().c_str() : "",
									animation);
						}
					}

					if ( current->c_cih()->IsImageInst() )
					{
						AptCharacterImageInst* image = current->c_cih()->GetImageInst();
						if (image)
						{
							dumpDisplayListIndent(file, currentDepth + 1);
							fprintf(file, "*Image: %s (%p)\n",
									image->mFile ? image->mFile->GetName().c_str() : "",
									image);
						}
					}

					if ( current->c_cih()->IsSpriteInstBase() )
					{
						AptCharacterSpriteInstBase* sprite = current->c_cih()->GetSpriteInstBase();
						if (sprite)
						{
							dumpDisplayListIndent(file, currentDepth + 1);
							fprintf(file, "*Sprite: Frame=%d, JustLoaded=%s, IsPlaying=%s (%p)\n",
									sprite->mnFrame,
									sprite->mbJustLoaded ? "true" : "false",
									sprite->mbIsPlaying ? "true" : "false",
									sprite);
						}
					}

					if ( current->c_cih()->IsButtonInst() )
					{
						AptCharacterButtonInst* button = current->c_cih()->GetButtonInst();
						if (button)
						{
							dumpDisplayListIndent(file, currentDepth + 1);
							fprintf(file, "*Button: State=%s (%p)\n",
									button->mnState == AptCharacterButtonRecordState::AptCharacterButtonRecordState_None ? "None" :
									button->mnState == AptCharacterButtonRecordState::AptCharacterButtonRecordState_Up ? "Up" :
									button->mnState == AptCharacterButtonRecordState::AptCharacterButtonRecordState_Over ? "Over" :
									button->mnState == AptCharacterButtonRecordState::AptCharacterButtonRecordState_Down ? "Down" :
									button->mnState == AptCharacterButtonRecordState::AptCharacterButtonRecordState_HitTest ? "HitTest" :
									"Unknown",
									button);
						}
					}
				}

				// Process children
				if (current->isCIH())
				{
					const AptCIH *currentCIH = current->c_cih();
					const AptCIH *child = currentCIH->GetFirstChild();
					if (child)
					{
						stack.push_back(child);

						// Advance this node for when we return
						current = currentCIH->GetDisplayListNext();
						continue;
					}
				}
			}

			// Move on to next sibling
			if (current->isCIH())
			{
				current = current->c_cih()->GetDisplayListNext();
			}
			else
			{
				current = nullptr;
			}
		}
		else
		{
			stack.pop_back();
		}
	}
#endif
}

void AptDisplayList::dumpDisplayList(const char *szFileName)
{
#ifndef EA_RETAIL
	if (pState && pState->GetFirstItem())
	{
		FILE *file = fopen(szFileName, "wb");
		APT_ASSERT(file);
		if (file)
		{
			// Reusable string buffer to store temp formatted strings
			AptNativeString strBuffer;

			// To minimize recursion and potentially overflowing the stack with deeply nested AS code, push new nodes onto the stack for processing instead.
			eastl::vector<const AptValue *> stack;
			stack.set_capacity(128);

			// Begin dumping the display list tree
			dumpDisplayListNode(file, pState->GetFirstItem(), strBuffer, stack);

			fclose(file);
		}
	}
#endif
}
// FIFA_END



/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/
