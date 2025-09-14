// (c) Electronic Arts. All Rights Reserved.

/// \file
/// A linear memory allocator.

/*************************************************************************************************************
 Includes
 */
#include "rwsharedheaders.hpp"

#include "rw/core/linearalloc.h"

/*************************************************************************************************************
 Defines
 */

/*************************************************************************************************************
 Enums
 */

/*************************************************************************************************************
 Typedef Enums
 */

/*************************************************************************************************************
 Typedef Structs + Unions
 */

/*************************************************************************************************************
 Typedef Function Pointers
 */

/*************************************************************************************************************
 Structs + Unions
 */

/*************************************************************************************************************
 Static Variables
 */
static RwLinearAllocHeap *rw_Heap = NULL;

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Function prototypes
 */

/*************************************************************************************************************
 Functions
 */

// RwLinearAllocInitializeHeap (deprecated)
RwLinearAllocHeap *
RwLinearAllocInitializeHeap(RwLinearAllocHeap *heap, void *mem, size_t size)
{
    rwFUNCTION();
    rwDEPRECATED();
    rwASSERT(heap);
    rwASSERT(mem);

    heap->memory = mem;
    heap->numAllocations = 0;
    heap->numAlignmentBytes = 0;
    heap->currentUsage = 0;
    heap->heapSize = size;

    rwRETURN(heap);
}

// RwLinearAllocSetCurrentHeap (deprecated)
RwLinearAllocHeap *
RwLinearAllocSetCurrentHeap(RwLinearAllocHeap *heap)
{
    RwLinearAllocHeap *oldHeap;

    rwFUNCTION();
    rwDEPRECATED();

    oldHeap = rw_Heap;

    rw_Heap = heap;

    rwRETURN(oldHeap);
}

// RwLinearAllocGetCurrentHeap (deprecated)
RwLinearAllocHeap *
RwLinearAllocGetCurrentHeap(void)
{
    rwFUNCTION();
    rwDEPRECATED();

    rwRETURN(rw_Heap);
}

/// RwLinearAllocAlloc (deprecated)
void *
RwLinearAllocAlloc(size_t size, size_t alignment)
{
    void        *mem;
    size_t     padding;

    rwFUNCTION();
    rwDEPRECATED();
    rwASSERT(rw_Heap);

    /* Calculate alignment padding */
    if (alignment > 0)
    {
        void* currentPtr = rw::MemAddOffset(rw_Heap->memory, rw_Heap->currentUsage);
        padding = static_cast<size_t>(reinterpret_cast<rw::HostPtr>(rw::MemAlign(currentPtr, static_cast<uint32_t>(alignment))) -
                                        reinterpret_cast<rw::HostPtr>(currentPtr));
    }
    else
    {
        padding = 0;
    }

    /* Check there is enough space */
    if ((rw_Heap->currentUsage + padding + size) > rw_Heap->heapSize)
    {
        rwRETURN((void *)0);
    }

    mem = (void *)((uint8_t *)rw_Heap->memory + rw_Heap->currentUsage + padding);

    rw_Heap->numAllocations++;
    rw_Heap->numAlignmentBytes += padding;
    rw_Heap->currentUsage += (padding + size);

    rwRETURN(mem);
}

// RwLinearAllocGetCurrentHeapRestorePoint (deprecated)
void
RwLinearAllocGetCurrentHeapRestorePoint(RwLinearAllocHeapRestorePoint *restorePoint)
{
    rwFUNCTION();
    rwDEPRECATED();
    rwASSERT(rw_Heap != NULL);
    rwASSERT(restorePoint != NULL);

    restorePoint->numAllocations = rw_Heap->numAllocations;
    restorePoint->numAlignmentBytes = rw_Heap->numAlignmentBytes;
    restorePoint->currentUsage = rw_Heap->currentUsage;

    rwRETURNVOID();
}

/// RwLinearAllocSetCurrentHeapRestorePoint (deprecated)
void
RwLinearAllocSetCurrentHeapRestorePoint(const RwLinearAllocHeapRestorePoint *restorePoint)
{
    rwFUNCTION();
    rwDEPRECATED();
    rwASSERT(rw_Heap != NULL);
    rwASSERT(restorePoint != NULL);
    rwASSERT(rw_Heap->currentUsage >= restorePoint->currentUsage);

    rw_Heap->numAllocations = restorePoint->numAllocations;
    rw_Heap->numAlignmentBytes = restorePoint->numAlignmentBytes;
    rw_Heap->currentUsage = restorePoint->currentUsage;

    rwRETURNVOID();
}
