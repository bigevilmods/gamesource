// (c) Electronic Arts. All Rights Reserved.

/// \file
/// A linear memory allocator.

#ifndef RW_LINEARALLOC_H
#define RW_LINEARALLOC_H

#include <rw/core/base/ostypes.h>

/*************************************************************************************************************
 Includes
 */

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

typedef struct RwLinearAllocHeap RwLinearAllocHeap;
typedef struct RwLinearAllocHeapRestorePoint RwLinearAllocHeapRestorePoint;

/*************************************************************************************************************
 Typedef Function Pointers
 */

/*************************************************************************************************************
 Structs + Unions
 */

struct RwLinearAllocHeap
{
    void        *memory;
    uint32_t    numAllocations;
    size_t     numAlignmentBytes;
    size_t     currentUsage;
    size_t     heapSize;
};

struct RwLinearAllocHeapRestorePoint
{
    uint32_t    numAllocations;
    size_t     numAlignmentBytes;
    size_t     currentUsage;
};

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Function prototypes
 */

RwLinearAllocHeap *RwLinearAllocInitializeHeap(RwLinearAllocHeap *heap, void *mem, size_t size);

RwLinearAllocHeap *RwLinearAllocSetCurrentHeap(RwLinearAllocHeap *heap);

RwLinearAllocHeap *RwLinearAllocGetCurrentHeap(void);

void *             RwLinearAllocAlloc(size_t size, size_t alignment);

void               RwLinearAllocGetCurrentHeapRestorePoint(RwLinearAllocHeapRestorePoint *restorePoint);

void               RwLinearAllocSetCurrentHeapRestorePoint(const RwLinearAllocHeapRestorePoint *restorePoint);

#endif /* RW_LINEARALLOC_H */
