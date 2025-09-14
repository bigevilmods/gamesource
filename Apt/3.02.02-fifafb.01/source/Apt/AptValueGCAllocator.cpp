/*H************************************************************************************************/
/*!

    \File    AptValueGCAllocator.cpp

    \Description

      Summary:

        The Apt Value GC Allocator takes advantage of the dogma allocator architecture to be able
        to provide traceability to AptValue Derived Objects. This traceability is needed for
        garbage collection purposes and used for other debug / informational purposes. Basically
        this provides the ability to walk the list of allocated AptValueGC objects at any time,
        without having to maintain a linked list of them (which was previously using 8 bytes of
        memory for each object). To achieve this the this class has been slightly coupled with
        AptValue Class and as such certain modifications to the AptValue Class can break this.
        It is recommended that anyone looking to modify either this pool manager or AptValueGC
        be aware of this coupling.

      Details:

        Basically This pool manager starts at the base of the first pool and looks at the
        first 4 bytes of it's memory. These 4 bytes may be allocated to apt with an AptValue,
        or could be deallocated and inside of one of the free lists. In order to discern which, we
        have created a bit in AptValue that is reserved for the allocator's purposes. This bit is
        never modified by the Apt Value itself. Thus using this bit the pool manager can tell if the
        block of memory is allocated or not just by reading those four bytes. If the memory is allocated
        we call AptGetSizeOfAptValue to return the size of the AptValue Object (which gives us the
        location of the next object in memory to repeat the process). If the memory is free, we
        retrieve the size of the free block from the free block itself (it is stored when the object
        is deallocated.) In either case we are able to a: detect if it is allocated, and b: Detect
        where the next block of memory starts to repeat the check. This process it used to return each
        and every Allocated value to the garbage collector (or any one else that want to see what is
        out there).

        The other benefit of this system (above the removal of the traceable object linked list)
        is that we use the CPU's cache very efficiently. I.e. walking the blocks in the pools
        travels up memory linearly, how CPU caches work best. Because of this the allocator
        can be much faster then the previous implementation that trashed the cache by
        wondering randomly through memory.


    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        10/22/04 (DC) Bringin' it to ya
    \Version    1.1        04/04/05 (DC) $bug857 added ability to track and report outside
                            allocations. This was needed in response to the fix for large AptExt
                            derived objects.
    \Version    1.2        06/01/07 (RB) Daniel Craig's preventative fix from CentralFootball, in
                                         DeallocateAptValueGC.


*/
/************************************************************************************************H*/


/*** Include files ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "_Apt.h"    // DC -- Include after stdio and stdlib.
#include "Display/AptDisplayListState.h"
#include "AptFastStack.h"
#include "AptStd/AptMath.h"
#include "AptValue/AptValueVector.h"
#include "AptExtObject.h"
#include "AptFrameStack.h"
#include "MainInline.h"
#include "DogmaAllocator.h"
#include "AptValueGCAllocator.h"

/*** Defines **************************************************************************************/

/*** Variables*************************************************************************************/
uint8_t    AptValueGC_PoolManager::snOffsetToStoreNext;
uint8_t    AptValueGC_PoolManager::snOffsetToStoreSize;
uint8_t    AptValueGC_PoolManager::snMinAllocation;
uint32_t   AptValueGC_PoolManager::snMaxAllocation;

/*** Type Definitions *****************************************************************************/


/*** Functions ************************************************************************************/


/*F************************************************************************************************/
/*!
    \Function    *AllocateAptValueGC

    \Description
        Allocate a mem block and condition it to work with our AptValueGC_MemItem logic.

    \Input      nAllocatedSize    - Size of the AptValue
    \Output     AptValueGC*       - Pointer to an AptValue

    \Notes
        None.

    \Version    1.0        10/22/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptValueGC *AptValueGC_PoolManager::AllocateAptValueGC(size_t nAllocatedSize)
{
    AptValueGC* pValue = (AptValueGC *)DOGMA_PoolManager::Allocate(APT_ALLOC_PARAMS(nAllocatedSize));
    AptValueGC_MemItem * pItem  = (AptValueGC_MemItem   *)pValue;

#if APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL
    // Make sure the shared bit is in the right location by toggling it and reading it back.
    pItem->SetIsAllocated(snOffsetToStoreSize, false);     // Clear as free item
    APT_ASSERT(pValue->mValueBitfield.mbIsAllocated == false); // Read as AptValue
#endif

    pItem->SetIsAllocated(snOffsetToStoreSize, true);      // Set Allocated as Free Item
    APT_ASSERT(pValue->mValueBitfield.mbIsAllocated == true);  // Verify as AptValue

    return pValue;
}

/*F************************************************************************************************/
/*!
    \Function    DeallocateAptValueGC

    \Description
        Does some Debug Checking and Calls the DOGMA_PoolManager Deallocate the memory.

    It also clears the AptValueGC_MemItem allocated flag.

    \Input      *pNowFree         - pointer to AptValue Allocated by this pool
    \Input      nAllocatedSize    - size of the object

    \Output
        None.

    \Notes
        None.

    \Version    1.0        10/22/04 (DC) Bringin' it to ya
    \Version    1.01       06/01/07 (RB) Daniel Craig's preventative fix from CentralFootball

*/
/************************************************************************************************F*/
void AptValueGC_PoolManager::DeallocateAptValueGC(AptValueGC *pNowFree, size_t nAllocatedSize)
{
#if defined(EA_DEBUG)
    AptValueGC_MemItem * pItem  = (AptValueGC_MemItem   *)pNowFree;

    // Verify that the Size being deallocated is actually the size of the AptValue object.
    APT_ASSERT(AptGetSizeOfAptValue(pNowFree) == nAllocatedSize && "MemFree was passed the wrong size for this object!");
    APT_ASSERT(pItem->IsAllocated(snOffsetToStoreSize) && "MemFree Was called on a value that was already deallocted!");
#endif // EA_DEBUG

    DOGMA_PoolManager::Deallocate(pNowFree, nAllocatedSize);

    // Code used to do some post processing, but that assumed the value wasn't re-allocated already!
    // This is obviously an error. Besides we were clearing the allocated flag, but since the
    // allocated flag is in the freed size, the bit should automatically be cleared then the size
    // is inserted.
}


#if defined(APT_ALLOCATION_TRACKING)

AptValueGC *AptValueGC_PoolManager::TrackedAllocateAptValueGC(  size_t nAllocatedSize, const char * szFile, uint32_t nLine)
{
    AptValueGC* pValue = (AptValueGC *)DOGMA_PoolManager::TrackedAllocate(nAllocatedSize, szFile, nLine);
    AptValueGC_MemItem * pItem  = (AptValueGC_MemItem   *)pValue;

#if APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL
    // Make sure the shared bit is in the right location by toggling it and reading it back.
    pItem->SetIsAllocated(snOffsetToStoreSize, false);     // Clear as free item
    APT_ASSERT(pValue->mValueBitfield.mbIsAllocated == false); // Read as AptValue
#endif

    pItem->SetIsAllocated(snOffsetToStoreSize, true);      // Set Allocated as Free Item
    APT_ASSERT(pValue->mValueBitfield.mbIsAllocated == true);  // Verify as AptValue

    return pValue;
}

void AptValueGC_PoolManager::TrackedDeallocateAptValueGC(AptValueGC *pNowFree, size_t nAllocatedSize)
{
    AptValueGC_MemItem * pItem  = (AptValueGC_MemItem   *)pNowFree;

    // Verify that the Size being deallocated is actually the size of the AptValue object.
    APT_ASSERT(AptGetSizeOfAptValue(pNowFree) == nAllocatedSize && "MemFree was passed the wrong size for this object!");
    APT_ASSERT(pItem->IsAllocated(snOffsetToStoreSize) && "MemFree Was called on a value that was already deallocted!");

    if(DOGMA_PoolManager::TrackedDeallocate(pNowFree, nAllocatedSize))
    {
        // Must do this after deallocation, because deallocation can over write it.
        pItem->SetIsAllocated(snOffsetToStoreSize, false);

#if APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL
        // Verify that the Pool Manager set the size properly and that the AptValueGC_MemItem can read it out.
        uintptr_t nItemSize = pItem->GetSize(snOffsetToStoreSize);
        APT_ASSERT(nItemSize == nAllocatedSize);
#endif
    }
}

#endif // #if defined(APT_ALLOCATION_TRACKING)

/*F************************************************************************************************/
/*!
    \Function    GetNextAptValue

    \Description
        Gets the next allocated AptValue from the list of pools.

    uses the size of the object passed and then advances though the pool free items until we
    find the next allocated item.

    \Input      *pPrevious        - Item to start looking after.

    \Output     AptValue*         -

    \Notes
        None.

    \Version    1.0        10/22/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptValue* AptValueGC_PoolManager::GetNextAptValue(const AptValue *pPrevious)
{
    DOGMA_MemPool * pPool = mpFirstPool;

    // Find the pool we are in.
    do {
        if( pPool->PtrIsInThisPool(pPrevious) )
            break;
    } while( (pPool = pPool->GetNextPool()) != NULL);


    // if pPool is null already, then the passed pointer was not allocated internally
    if(pPool == NULL)
    {
        return (AptValue*)GetNextOutsideAllocation(pPrevious);
    }
    else
    {
        // Start looking for the next Allocated Block (But First Advance Past this one.
        AptValueGC_MemItem * pItem     = (AptValueGC_MemItem * )pPrevious;
        uintptr_t            nItemSize;

        if(pItem->IsAllocated(snOffsetToStoreSize))
            nItemSize = AptGetSizeOfAptValue((AptValue*)pItem);
        else
            nItemSize = pItem->GetSize(snOffsetToStoreSize);
        // FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change
        //nItemSize = ToNextValidSize(nItemSize);
        nItemSize = static_cast<uint32_t>(ToNextValidSize(nItemSize));
        // FIFA_END

        uintptr_t temp = ((uintptr_t)pItem) + nItemSize;
        pItem = (AptValueGC_MemItem * )temp;

        if( !pPool->PtrIsInThisPool(pItem) )
        {
            pPool = pPool->GetNextPool();
            if(pPool)
            {
                pItem =  (AptValueGC_MemItem * )pPool->GetFirstItem();
            }
        }

        while(pPool != NULL)
        {
            // Start looking for the next item.
            while( pPool->PtrIsInThisPool(pItem) )
            {

                uintptr_t nItemSize;
                if(pItem->IsAllocated(snOffsetToStoreSize))
                {
                    return (AptValue*)pItem;
                }
                else
                {
                    nItemSize = pItem->GetSize(snOffsetToStoreSize);
                }
				// FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change
				//nItemSize = ToNextValidSize(nItemSize);
                nItemSize = static_cast<uint32_t>(ToNextValidSize(nItemSize));
				// FIFA_END
                uintptr_t temp = ((uintptr_t)pItem) + nItemSize;
                pItem = (AptValueGC_MemItem * )temp;

            }

            pPool = pPool->GetNextPool();
			if (pPool)
			{
				pItem =  (AptValueGC_MemItem * )pPool->GetFirstItem();
			}
        }
    }

    // OK The passed pointer WAS an internal allocation, but it was the last one. Start passing back outside allocations
    return (AptValue*)GetFirstOutsideAllocation();
}

/*F************************************************************************************************/
/*!
    \Function    GetFirstAptValue

    \Description
        Returns the first Allocated Item from the first Pool.

    This just walks the items starting at the begining until it finds an allocated item and returns it

    \Input
        None.

    \Output     AptValue*         - First allocated item.

    \Notes
        None.

    \Version    1.0        10/22/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptValue* AptValueGC_PoolManager::GetFirstAptValue()
{

    DOGMA_MemPool * pPool = mpFirstPool;

    do {

        AptValueGC_MemItem * pItem = (AptValueGC_MemItem * )pPool->GetFirstItem();

        while( pPool->PtrIsInThisPool(pItem) )
        {
            uint32_t nItemSize;
            if(pItem->IsAllocated(snOffsetToStoreSize))
            {
                return (AptValue*)pItem;
            }
            else
            {
                nItemSize = pItem->GetSize(snOffsetToStoreSize);
            }

            uintptr_t temp = ((uintptr_t)pItem) + nItemSize;
            pItem = (AptValueGC_MemItem * )temp;

        }

    } while( (pPool = pPool->GetNextPool()) != NULL);

	// FIFA_BEGIN | SENDBACK | nmeyer | 2019-08-19 | Fix APT GC DOGMA allocator excluding large allocations from the GetFirstAptValue()
	return (AptValue*)GetFirstOutsideAllocation();
	// FIFA_END
}


AptValue ** AptValueGC_PoolManager::GetAllAllocatedAptValues()
{
#if APT_USE_TEMPORARY_ALLOCATORS
    AptValue ** pAptValuesArray = (AptValue **)gAptFuncs.pfnTempAlloc( APT_ALLOC_PARAMS( mnItemsAllocated * DOGMA_ALLOCATION_GRANULARITY) );
#else
    AptValue ** pAptValuesArray = (AptValue **)gAptFuncs.pfnMemAlloc( APT_ALLOC_PARAMS( mnItemsAllocated * DOGMA_ALLOCATION_GRANULARITY) );
#endif
    APT_ASSERT(pAptValuesArray != NULL)

    //lint --e(668) PcLint warning Warning 668: Possibly passing a null pointer to function 'memset(void *, int, unsigned int)', arg. no. 1 [Reference: file C:\MyWork\Perforce\mullet\sw10\code\mainline\ion\Apt\2.06.03-ion_dev\source\Apt\AptValueGCAllocator.cpp: line 354]
    memset((void *) pAptValuesArray, 0, mnItemsAllocated * DOGMA_ALLOCATION_GRANULARITY);
    // now walk thru all the pools and collect all the values in the array.

    int32_t    nArrayIndex = 0;

    DOGMA_MemPool * pPool = mpFirstPool;
    do {

        AptValueGC_MemItem * pItem = (AptValueGC_MemItem * )pPool->GetFirstItem();

        while( pPool->PtrIsInThisPool(pItem) )
        {
            uint32_t nItemSize;
            if(pItem->IsAllocated(snOffsetToStoreSize))
            {
                pAptValuesArray[nArrayIndex++] = (AptValue*)pItem;
                nItemSize = AptGetSizeOfAptValue((AptValue*)pItem);
            }
            else
            {
                nItemSize = pItem->GetSize(snOffsetToStoreSize);
            }
			// FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change
			//nItemSize = ToNextValidSize(nItemSize);
            nItemSize = static_cast<uint32_t>(ToNextValidSize(nItemSize));
			// FIFA_END
            uintptr_t temp = ((uintptr_t)pItem) + nItemSize;
            pItem = (AptValueGC_MemItem * )temp;

        }

    } while( (pPool = pPool->GetNextPool()) != NULL);

    AptValue * pOutsideValues = (AptValue*)GetFirstOutsideAllocation();
    if (pOutsideValues != NULL)
    {
        while(pOutsideValues != NULL)
        {
            pAptValuesArray[nArrayIndex++] = (AptValue *)pOutsideValues;
            pOutsideValues = (AptValue *) GetNextOutsideAllocation(pOutsideValues);
        }
    }

    APT_ASSERT(nArrayIndex == (int32_t) mnItemsAllocated);

    return pAptValuesArray;
}

void AptValueGC_PoolManager::ReleaseAllocatedAptValuesArray(AptValue ** pAptValuesArray, int32_t nItemsInArray)
{
    if (pAptValuesArray)
    {
#if APT_USE_TEMPORARY_ALLOCATORS
        gAptFuncs.pfnTempFreeSize( pAptValuesArray , nItemsInArray * DOGMA_ALLOCATION_GRANULARITY );
#else
        gAptFuncs.pfnMemFreeSize( pAptValuesArray , nItemsInArray * DOGMA_ALLOCATION_GRANULARITY );
#endif
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptValueGC_PoolManager::VerifyList

    \Description
        Walks the List to verify that it is all intact. This is for debugging purposes.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        11/05/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void  AptValueGC_PoolManager::VerifyList()
{
    uint32_t nItemsFound = 0;

    AptValue * pValue = GetFirstAptValue();

    while(pValue != NULL)
    {
        nItemsFound++;
        pValue = GetNextAptValue(pValue);
    }

    APT_ASSERT(nItemsFound == mnItemsAllocated);
    return;
}

/*F************************************************************************************************/
/*!
    \Function    StaticInitialize

    \Description
        Sets up all static variables for this class.

    Must be called prior to instantiating any objects of this type.

    Internally this hides some rather wacky things problems that would otherwise complicate our
    lives. The main problem is that some compilers put the vtable (table of virtual functions) for
    Apt Value at the beginning of the memory region, while other compilers put is at the end...
    This seems benign, other then the fact that we need to share a bit with AptValue (the allocated
    bit). Which means that the bit is in a different location depending on the compiler. To
    re-simplify our lives this function gets the the address of the bit location and stores the offset.
    This offset is then used throughout the process instead of nasty #defines for each compiler
    everywhere.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        10/22/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptValueGC_PoolManager::StaticInitialize( )
{
    extern const uint8_t AptValueSizesByVType[APT_VALUESIZES_ARRAYSIZE];

    AptValue * pTemp = 0;

    // Gotta find out where that darn vtable is. (would use the offsetof macro, but it doesn't seem reliable across platforms...)
    if( /*lint -e(413) */ (uintptr_t)&pTemp->mnValueData - (uintptr_t)pTemp == 0)
    {
        // vtable is at the end, so shared dword is at the beginning
        snOffsetToStoreNext = sizeof(void*);
        snOffsetToStoreSize = 0;
    }
    else if ( /*lint -e(413) */ (uintptr_t)&pTemp->mnValueData - (uintptr_t)pTemp == sizeof(void*))
    {
        // Vtable is at the beginning, if we want to share a dword with the apt value, we'll need to move up a little
        snOffsetToStoreNext = 0;
        snOffsetToStoreSize = sizeof(void*);
    }




    // Dynamically Set the Minimum and Maximum Size.
    uint32_t maxSizeSoFar = 0;
    uint32_t minSizeSoFar = 1000000;

    for(int i = (AptVFT_xxx+1); i < AptVFT_NumVFTs; i++)
    {
        if( AptValueSizesByVType[i] > maxSizeSoFar )
        {
            maxSizeSoFar = AptValueSizesByVType[i];
        }

        if( AptValueSizesByVType[i] < minSizeSoFar )
        {
            minSizeSoFar = AptValueSizesByVType[i];
        }
    }

    snMaxAllocation = maxSizeSoFar;
    APT_ASSERT(minSizeSoFar < 0xFF);
    snMinAllocation = static_cast<uint8_t>(minSizeSoFar);
}
