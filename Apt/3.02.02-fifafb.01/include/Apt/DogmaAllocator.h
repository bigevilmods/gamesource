/*H************************************************************************************************/
/*!

    \File    DogmaAllocator.h

    \Description

      Summary:

        Dogma is short for Dynamically Organized General-purpose Memory Allocator.

        It is technically an allocator in and of itself, but more realistically it is a buffering
        system for the actual system allocator. It just takes medium sized memory chucks and
        partitions them among the hopeful threads and relatives based on allocation size.
        Internally it maintains link lists of free memory for each allocatable size (in multiples of 4).

        Dogma is optimized for speed and to not waste memory when possible. All in all it is very
        efficient in what it does. At the same time it requires little in the way of configuration.
        It is better to set the main pool size so as to minimize use of overflow pools but it is
        not required.

      Details:

        The Pool Manager manages the pools that are allocated, for performance reasons there are
        two types of pools, a single main pool, and a number of overflow pools. If a given request
        cannot be fulfilled by an existing pool an overflow pool is created. The idea is to have a
        larger main pool and then as more memory is requested overflow pools are created as needed.

        Internally to a pool, memory is dished out sequentially. So as memory is allocated from the
        pool, the start of free space is incremented accordingly, and the size of the free space at
        the end of the pool is decreased. Once allocated, when the memory is released, the block is
        added to a linked list of free blocks of that size. Each allocatable size has a head pointer
        for it's linked list, and the linked list itself is maintained within the free memory.

        So with that said, when an allocation is made we first check the linked list head for the
        requested size to see if a block is available. Typically after an initialization period
        this becomes the most common case. If the free list for that size is empty, we try to take
        it from the free space at the end of the pool. Should the rest of the pool be unable to
        fulfill the request an overflow pool is created it's free space is consumed.

      Restrictions:

        This allocator does have a limitation however: All deallocations must have a size. This
        allocator is unable to calculate the size of a block when it is deallocated. This means that
        it is probably unsuitable for a number of purposes. Fortunately For C++ objects the size
        of the object is always know at the time of deletion and is maintained by the compiler,
        which makes this allocator ideal for C++ new and delete operators. On the other hand,
        other users will need to maintain the size of the allocation so that it can be furnished
        at the time of deletion. This although it sounds bad is actually not too bad a of a request
        because most places that allocate a block of memory store the size of that block anyway
        for the purpose of making sure the buffer is not overrun. so in practice this limitation is
        usually not a major problem, but mileage may vary based on the design of the code using it.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        10/22/04 (DC) Bringin' it to ya
    \Version    1.1        01/23/08 ($senzee) Added GetExternalBytesUsed() function and associated machinery
    \Version    1.2        08/07/08 (ASK) Split mnBytesAllocated into 2 parts mnBytesAllocatedInDogma and
                                        mnBytesAllocatedOutsideDogma and are now always tracked instead of just for
                                        DOGMA_EXTRA_MEMORY_COUNTERS and mnBytesInFreeLists, mnItemsFreed are now
                                        conditionally tracked as they are not that important as compared to
                                        number of bytes.
    \Version    1.3        09/16/08 (RB) Add new metrics methods GetNumOverflowPools and GetOverflowPoolSize.
*/
/************************************************************************************************H*/


#ifndef _DogmaAllocator_h
#define _DogmaAllocator_h


/*** Include files ********************************************************************************/
#include "string.h"
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
#include "Module.h"
// FIFA_END
/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/
#define DOGMA_ALLOCATION_GRANULARITY EA_PLATFORM_WORD_SIZE
/*** Type Definitions *****************************************************************************/

/*** Functions ************************************************************************************/


/*H************************************************************************************************/
/*!

\struct    DOGMA_MemPool

\Description
Manages data associated with a pool, all required maintenance variables are placed within the pool.


\Notes
None.

\Copyright
(c) 2004 Electronic Arts Inc.

\Version    1.0        10/22/04 (DC) Bringin' it to ya

*/
/************************************************************************************************H*/
typedef struct _DOGMA_MemPool{


    /*F************************************************************************************************/
    /*!
        \Function    SetupPool

        \Description
            This setups the given block of memory as a pool to be allocated from.

        Since this is actually a large block of memory we are making into a Mem pool, we don't have a
        constructor, This serves the purpose of initializing the pseudo object.



        \Input      pNextPool         -
        \Input      nSize             -

        \Output
            None.

        \Notes
            None.

        \Version    1.0        10/22/04 (DC) Bringin' it to ya

    */
    /************************************************************************************************F*/
    APT_INLINE void SetupPool(struct _DOGMA_MemPool * pNextPool, size_t nSize)
    {
        mnPoolFree = mnPoolSize = (nSize-sizeof(struct _DOGMA_MemPool)+1);
        mpNextPool = pNextPool;
    }

    struct _DOGMA_MemPool * GetNextPool() const;

    /*F************************************************************************************************/
    /*!
        \Function    CanFitBytes

        \Description
            returns true if the number of bytes passed will fit in the unused portion of this pool.

        \Input      nBytes            - Number of bytes we will use.

        \Output     bool   - true = will fit.

        \Notes

        \Version    1.0        10/22/04 (DC) Bringin' it to ya

    */
    /************************************************************************************************F*/
    APT_INLINE bool CanFitBytes(size_t nBytes) const
    {
        return (mnPoolFree >= nBytes);
    }

    /*F************************************************************************************************/
    /*!
        \Function    *ConsumeBytes

        \Description
            consumes the passed number of bytes from the unused portion of the pool.

        \Input      nBytes            - Number of bytes to use.

        \Output void* - pointer to allocated chunk of memory.

        \Notes
            None.

        \Version    1.0        10/22/04 (DC) Bringin' it to ya

    */
    /************************************************************************************************F*/
    APT_INLINE void *ConsumeBytes(size_t nBytes)
    {
        size_t mOldFree = mnPoolFree;
        mnPoolFree -= nBytes;
        return &(&mPoolStart)[mnPoolSize - mOldFree];
    }

    /*F************************************************************************************************/
    /*!
        \Function    GetFirstItem

        \Description
            Return a pointer to the first byte of allocatable pool memory.

        Warning: this does not know if that chunk is allocated or not it is up to the caller to know
        what they are going to do with it and be sure they do not corrupt memory.

        \Input
            None.

        \Output  void* - pointer to first byte of allocatable pool memory.

        \Notes
            None.

        \Version    1.0        10/22/04 (DC) Bringin' it to ya

    */
    /************************************************************************************************F*/
    APT_INLINE void* GetFirstItem()
    {
        return &mPoolStart;
    }

    /*F************************************************************************************************/
    /*!
        \Function    PtrIsInThisPool

        \Description
            returns true if the pointer passed is within this pools allocatable region.

        This can be used to find out which pool a pointer belongs to.

        \Input
            None.

        \Output     bool              -

        \Notes
            None.

        \Version    1.0        10/22/04 (DC) Bringin' it to ya

    */
    /************************************************************************************************F*/
    APT_INLINE bool PtrIsInThisPool(const void* ptr) const
    {
        if(ptr >= &mPoolStart && ptr < &((&mPoolStart)[mnPoolSize-mnPoolFree]))
            return true;
        else
            return false;
    }

    /*F************************************************************************************************/
    /*!
        \Function    GetBytesUsed

        \Description
            Returns the number of bytes that are used in this pool.

        This does not include pool management overhead or unused space.

        \Input
            None.

        \Output     APT_INLINE size_t  -

        \Notes
            None.

        \Version    1.0        10/22/04 (DC) Bringin' it to ya

    */
    /************************************************************************************************F*/
    APT_INLINE size_t GetBytesUsed()
    {
        return mnPoolSize-mnPoolFree;
    }

protected:

    struct _DOGMA_MemPool * mpNextPool;  // Pointer to the next pool.
    size_t                mnPoolSize;  // Size in Bytes of the allocatable portion of the pool
    size_t                mnPoolFree;  // Number of bytes left free in the pool
    EA_ALIGNED(uint8_t, mPoolStart, DOGMA_ALLOCATION_GRANULARITY);  // Placeholder, used for its address as the base of the allocatable pool memory.
}DOGMA_MemPool;


typedef struct _AllocationDataPerSize {
    uint32_t nAllocationCount;
    size_t nCurrentlyAllocated;
    size_t nMaxAllocatedAtAnyOneTime;
}AllocationDataPerSize ;

typedef struct _TrackedAllocation {
    void  *      pAllocated;
    uint32_t     nLineNumber;
    const char * szFileName;
    size_t     nAllocSize;
}TrackedAllocation;


/*H************************************************************************************************/
/*!

    \class    DOGMA_MemPool

    \Description
    Quickly Manages allocation pools of variably sized objects.


    \Notes
    None.

    \Copyright
    (c) 2004 Electronic Arts Inc.

    \Version    1.0        10/22/04 (DC) Bringin' it to ya

*/
/************************************************************************************************H*/
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
//class DOGMA_PoolManager
class FB_APT_API DOGMA_PoolManager
// FIFA_END
{
public:
    static void *operator new(size_t size);
    static void operator delete(void *p, size_t size);
    static void *operator new[](size_t size);
    static void operator delete[](void *p);

    DOGMA_PoolManager(  size_t mainPoolSizeBytes,
        size_t overflowPoolSizeBytes,
        size_t minSizeAllocation,
        size_t maxSizeAllocation,
        uint8_t  nOffsetToStoreNextInFreeItem,
        bool bStoreFreeBlockSize, uint8_t nOffsetToStoreSizeInFreeItem,
        bool bTrackOusideAllocations);

    ~DOGMA_PoolManager();

#if defined(APT_ALLOCATION_TRACKING)
    void  *Allocate(size_t nAllocatedSize, const char * szAllocFile, uint32_t nLineNumber);
#else
    void  *Allocate(size_t nAllocatedSize);
#endif
    bool   Deallocate(void *pNowFree, size_t nAllocatedSize);

    size_t GetTotalBytesUsed();
    size_t GetExternalBytesUsed();
    int32_t GetNumOverflowPools();

    void * GetFirstOutsideAllocation();
    void * GetNextOutsideAllocation(const void * pCurrent);

    size_t GetOverflowPoolSize() const;

#if defined(APT_ENABLE_DEBUG_REPORT)
    size_t GetMaxAllocatedInBucket(int bucketNumber) const;
#endif

protected:

    void  *ConsumeFreeBlockBySize(size_t nSize);
    void   AddFreeBlockBySize(void * pNowFree, size_t nSize);
    size_t ToNextValidSize( size_t nSize);

    uintptr_t      ** mpaFirstFreeBySize;         // Base of Array where each sizes' first free pointers is stored.

    DOGMA_MemPool   * mpFirstPool;                // First Mem Pool.

    const size_t     mnOverflowPoolSize;          // Pools of this size will be allocated whenever allocation is made that will not fit in the main pool.
                                                  // The overflow pool should be small and if things are setup right then we should need them very often.

    const size_t     mnMaxSizeAllocation;         // Maximum size this allocator will handle.


    uint32_t    mnOffsetToStoreNext;            // Offset in free memory blocks to store the pointer to the next free block of the same size.
    uint32_t    mnOffsetToStoreSize;            // Offset in free memory blocks to store the size of the free block (only used if mbStoreFreeBlockSize is true)
    uint32_t    mnMinimumAllocationSize;        // minimum allocation size possible with this configuration
    uint32_t    mbStoreFreeBlockSize;           // 1 = store sizeof free block within the block, There is no memory overhead for this.
    uint32_t    mbTrackOutsideAllocations;      // 1 = Keep track of all allocations that go directly to the heap.


    struct OutsideAllocationT
    {
    public:

        OutsideAllocationT *pNext;
        OutsideAllocationT *pPrev;
        uint32_t            nSize;

        APT_INLINE void  *GetReturnedPointer()
        {
            return &returnedMemory[0];
        }

        APT_INLINE static size_t GetStructOverHead()
        {
            return 2 * sizeof(OutsideAllocationT *) + sizeof(uint32_t);
        }

        APT_INLINE static OutsideAllocationT * GetStructPointerFromReturnedPointer(const void * pReturned)
        {
            return (OutsideAllocationT *) ( ((uint8_t*)pReturned) - GetStructOverHead() );
        }

    private:
        // Gives us something to get the pointer from... Should not be messed with.
        uint8_t returnedMemory[1];
    };


    OutsideAllocationT *mpFirstOutSideAllocation;

public:

    uint32_t    mnBytesAllocatedInDogma;
    uint32_t    mnBytesAllocatedOutsideDogma;
    uint32_t    mnItemsAllocated;           // Count of Allocated Items

#if defined(DOGMA_EXTRA_MEMORY_COUNTERS)
    uint32_t    mnBytesInFreeLists;
    uint32_t    mnItemsFreed;               // Count of Allocated Items
    AllocationDataPerSize * mpaAllocDataPerSize;
#endif

#if defined(APT_VERIFY_NON_GC_ALLOC_SIZES)
    void *SizeVerifyAllocate( size_t nAllocatedSize );
    bool  SizeVerifyDeallocate(void *pNowFree, size_t nAllocatedSize);
#endif

#if defined(APT_ALLOCATION_TRACKING)
    void *TrackedAllocate(size_t nAllocatedSize, const char * szAllocFile, uint32_t nLineNumber);
    bool  TrackedDeallocate(void *pNowFree, size_t nAllocatedSize);
    void  RegisterTrackedAllocations( void (*pRegFunc)(TrackedAllocation *pTracker) );

    size_t              mnTrackThisSize;
    static const size_t TRACK_SIZE_ALL  = (size_t)-1;
    static const size_t TRACK_SIZE_NONE =  0;

    TrackedAllocation * mpTrackers;
    uint32_t            mnTrackers;
#endif

#if defined(XBOX_FASTCAP)
    void WriteDynamicMemoryUsageFile(void);
#endif

    private:
        // Here are a few operators not allowed on this object, they are put in private scope to slap people who try. (They are not implemented)
        DOGMA_PoolManager& operator=(const DOGMA_PoolManager& r);
};

#endif // _DogmaAllocator_h
