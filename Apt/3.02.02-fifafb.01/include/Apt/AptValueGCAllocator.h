/*H************************************************************************************************/
/*!

    \File    AptValueGCAllocator.h

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

*/
/************************************************************************************************H*/


#ifndef _AptValueGCAllocator_h
#define _AptValueGCAllocator_h

/*** Include files ********************************************************************************/
#include "AptDefine.h"

#include "DogmaAllocator.h"
#include "Apt.h"
#include "AptValue\AptValue.h"
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
#include "Module.h"
// FIFA_END

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/
class AptValueGC; // forward Declaration

/*** Functions ************************************************************************************/


/*H************************************************************************************************/
/*!

\struct    AptValueGC_PoolManager

\Description
Extension to the DOGMA_PoolManager which is designed for and aware of AptValueGC

The benefit of making this class aware of both objects is that we can give it enough knowledge
to be able to walk the pools of memory for garbage collection purposes. Since we reserve a bit in the
free memory that is also reserved in AptValue we can start at the begining of each pool and see if
the pointer is allocated and what size it is (to get to the next pointer).

\Notes
User must call AptValueGC_PoolManager::StaticInitialize prior to creating an instance of the class.
This sets up all static variables needed for execution.

\Copyright
(c) 2004 Electronic Arts Inc.

\Version    1.0        10/22/04 (DC) Bringin' it to ya

*/
/************************************************************************************************H*/
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
//class AptValueGC_PoolManager: public DOGMA_PoolManager
class FB_APT_API AptValueGC_PoolManager : public DOGMA_PoolManager
// FIFA_END
{
public:
    static void *operator new(size_t size)
    {
        return gAptFuncs.pfnMemAlloc( APT_ALLOC_PARAMS(size) );
    }
    static void operator delete(void *p, size_t size)
    {
        gAptFuncs.pfnMemFreeSize( p, size);
    }
    static void *operator new[](size_t size)
    {
        return gAptFuncs.pfnMemAlloc( APT_ALLOC_PARAMS(size) ) ;
    }
    static void operator delete[](void *p)
    {
        gAptFuncs.pfnMemFree(p);
    }

    AptValueGC_PoolManager(uint32_t mainPoolSizeBytes, uint32_t overflowPoolSizeBytes) :
      DOGMA_PoolManager(mainPoolSizeBytes,
          overflowPoolSizeBytes,
          snMinAllocation, snMaxAllocation, snOffsetToStoreNext,
          true, snOffsetToStoreSize,
          true  /* Be sure to track outside allocations */ )
    {

    }


    AptValueGC *AllocateAptValueGC(  size_t nAllocatedSize);
    void        DeallocateAptValueGC(AptValueGC *pNowFree, size_t nAllocatedSize);

#if defined(APT_ALLOCATION_TRACKING)
    AptValueGC *TrackedAllocateAptValueGC(  size_t nAllocatedSize, const char * szFile, uint32_t nLine);
    void        TrackedDeallocateAptValueGC(AptValueGC *pNowFree, size_t nAllocatedSize);
#endif

    AptValue*   GetNextAptValue(const AptValue *pPrevious);
    AptValue*   GetFirstAptValue();
    void        VerifyList();

    AptValue **  GetAllAllocatedAptValues();
    void        ReleaseAllocatedAptValuesArray(AptValue ** pAptValuesArray, int32_t nItemsInArray);

    static void StaticInitialize();

protected:
    // Variables setup by StaticInitialize
    static uint8_t    snOffsetToStoreNext;
    static uint8_t    snOffsetToStoreSize;
    static uint8_t    snMinAllocation;
    static uint32_t   snMaxAllocation;
private:
    // Here are a few operators not allowed on this object, they are put in private scope to slap people who try. (They are not implemented)
    AptValueGC_PoolManager& operator=(const AptValueGC_PoolManager& r);
};

/*H************************************************************************************************/
/*!

\struct    AptValueGC_MemItem

\Description
Utility object used to make AptValueGC_PoolManager more readable.

This basically just gives anonymous pool memory an interface. Basically the trick to getting this
all going was to reserve a bit in the "size" field of the free memory block that matches up with a
bit reserved in AptValue in order to ensure

\Notes
None.

\Copyright
(c) 2004 Electronic Arts Inc.

\Version    1.0        10/22/04 (DC) Bringin' it to ya

*/
/************************************************************************************************H*/
typedef struct _AptValueGC_MemItem
{
    union {

        struct {
            struct _AptValueGC_MemItem * pNextItem;
            uintptr_t bIsAllocated:1;
#if EA_PLATFORM_PTR_SIZE == 4
            uintptr_t nSize:31;
#elif EA_PLATFORM_PTR_SIZE == 8
            uintptr_t nSize:63;
#else
#error "Don't know what to do here"
#endif
            struct _AptValueGC_MemItem * pPrevItem;
        }Type1;

        struct {
            uintptr_t bIsAllocated:1;
#if EA_PLATFORM_PTR_SIZE == 4
            uintptr_t nSize:31;
#elif EA_PLATFORM_PTR_SIZE == 8
            uintptr_t nSize:63;
#else
#error "Don't know what to do here"
#endif
            struct _AptValueGC_MemItem * pNextItem;
            struct _AptValueGC_MemItem * pPrevItem;
        }Type2;
    };

    /*F************************************************************************************************/
    /*!
    \Function    IsAllocated

    \Description
    Looks at the bit agreed (shared with AptValue) that tells us if the memory block we are
    looking at is allocated.

    Insert your long description here.

    \Input      nSizeOffset       - how many bytes into the block should we look?

    \Output     bool              - true = it is allocated and occupied by an aptvalue.

    \Notes
    None.

    \Version    1.0        10/22/04 (DC) Bringin' it to ya

    */
    /************************************************************************************************F*/
    APT_INLINE bool IsAllocated(uint32_t nSizeOffset)
    {
        if(nSizeOffset == sizeof(void*))
        {
            return Type1.bIsAllocated ? true : false;
        }
        else if(nSizeOffset == 0)
        {
            return Type2.bIsAllocated ? true : false;
        }
        else
        {
            APT_FAIL("bad nSizeOffset");
            return false;
        }
    }

    /*F************************************************************************************************/
    /*!
    \Function    SetIsAllocated

    \Description
    modify the AptValue approved "Allocated" bit to the given new value.

    \Input      nSizeOffset       - Offset into the block to look for our bit.
    \Input      newValue          - value to put there. true = allocated

    \Output
    None.

    \Notes
    None.

    \Version    1.0        10/22/04 (DC) Bringin' it to ya

    */
    /************************************************************************************************F*/
    void SetIsAllocated(uint32_t nSizeOffset, bool newValue)
    {
        if(nSizeOffset == sizeof(void*))
        {
            Type1.bIsAllocated = (uint32_t)(newValue ? 1 : 0);
        }
        else if(nSizeOffset == 0)
        {
            Type2.bIsAllocated = (uint32_t)(newValue ? 1 : 0);
        }
        else
        {
            APT_FAIL("bad nSizeOffset");
        }
    }

    /*F************************************************************************************************/
    /*!
    \Function

    \Description
    return the size of a free block.

    [idiot disclaimer] The pointer must be to an free block of memory

    \Input      nSizeOffset       -

    \Output     uint32_t GetSize  -

    \Notes
    None.

    \Version    1.0        10/22/04 (DC) Bringin' it to ya

    */
    /************************************************************************************************F*/
    APT_INLINE uintptr_t GetSize (uint32_t nSizeOffset)
    {
        APT_ASSERT(!IsAllocated(nSizeOffset));

        if(nSizeOffset == sizeof(void*))
        {
// DC - Kind of a hack, Big endian systems put the size at the other end of the word. Deal with it.
#if defined (EA_SYSTEM_BIG_ENDIAN)
            return Type1.nSize;
#else
            return Type1.nSize<<1;
#endif
        }
        else if(nSizeOffset == 0)
        {
// DC - Kind of a hack, Big endian systems put the size at the other end of the word. Deal with it.
#if defined (EA_SYSTEM_BIG_ENDIAN)
            return Type2.nSize;
#else
            return Type2.nSize<<1;
#endif
        }
        else
        {
            APT_FAIL("bad nSizeOffset");
            return 0;
        }
    }
}AptValueGC_MemItem;

#endif // _AptValueGCAllocator_h
