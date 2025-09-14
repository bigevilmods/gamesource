// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Implementation of Xbox2 physically contiguous allocator.

#include "rwsharedheaders.hpp"
#include "rw/core/memory/memoryalignedfunctions.h"
#include "rw/core/resource/xbox2physicallycontiguousallocator.h"

using namespace rw;

namespace
{
    const uint32_t rwg_Xbox2BitToFlag[32] =
    {
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 31 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 30 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 29 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 28 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 27 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 26 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 25 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 24 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 23 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 22 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 21 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 20 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 19 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 18 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 17 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 16 is set
        XALLOC_PHYSICAL_ALIGNMENT_32K,  // bit 15 is set
        XALLOC_PHYSICAL_ALIGNMENT_16K,  // bit 14 is set
        XALLOC_PHYSICAL_ALIGNMENT_8K,   // bit 13 is set
        XALLOC_PHYSICAL_ALIGNMENT_4K,   // bit 12 is set
        XALLOC_PHYSICAL_ALIGNMENT_2K,   // bit 11 is set
        XALLOC_PHYSICAL_ALIGNMENT_1K,   // bit 10 is set
        XALLOC_PHYSICAL_ALIGNMENT_512,  // bit 9 is set
        XALLOC_PHYSICAL_ALIGNMENT_256,  // bit 8 is set
        XALLOC_PHYSICAL_ALIGNMENT_128,  // bit 7 is set
        XALLOC_PHYSICAL_ALIGNMENT_64,   // bit 6 is set
        XALLOC_PHYSICAL_ALIGNMENT_32,   // bit 5 is set
        XALLOC_PHYSICAL_ALIGNMENT_16,   // bit 4 is set
        XALLOC_PHYSICAL_ALIGNMENT_8,    // bit 3 is set
        XALLOC_PHYSICAL_ALIGNMENT_4,    // bit 2 is set
        XALLOC_PHYSICAL_ALIGNMENT_4,    // bit 1 is set
        XALLOC_PHYSICAL_ALIGNMENT_4     // zero alignment?
    };
};

BaseResource
Xbox2PhysicallyContiguousAllocator::Allocate(const BaseResourceDescriptor& resourceDescriptor,
                                             PageSize pageSize)
{
    rwFUNCTION();

    const uint32_t alignment = resourceDescriptor.GetAlignment();

    const uint32_t physicalAllocFlag = rwg_Xbox2BitToFlag[_CountLeadingZeros((long)alignment)];

    uint32_t memprotectFlag = 0;

    if (pageSize == PAGESIZE_4K)
    {
        memprotectFlag = XALLOC_MEMPROTECT_WRITECOMBINE;
    }
    else
    {
        memprotectFlag = XALLOC_MEMPROTECT_WRITECOMBINE_LARGE_PAGES;
    }

    BaseResource resource = XMemAlloc(resourceDescriptor.GetSize(),
                                      MAKE_XALLOC_ATTRIBUTES(0, 0, 0, 0,
                                                             eXALLOCAllocatorId_MiddlewareReservedMin,
                                                             physicalAllocFlag,
                                                             memprotectFlag,
                                                             0,
                                                             XALLOC_MEMTYPE_PHYSICAL));
    rwRETURN(resource);
}

void
Xbox2PhysicallyContiguousAllocator::Free(const BaseResource& resource)
{
    rwFUNCTION();

    if (resource)
    {
        XMemFree(resource,
                 MAKE_XALLOC_ATTRIBUTES(0, 0, 0, 0,
                                        eXALLOCAllocatorId_MiddlewareReservedMin,
                                        0,
                                        XALLOC_MEMPROTECT_READWRITE |
                                        XALLOC_MEMPROTECT_WRITECOMBINE_LARGE_PAGES,
                                        0,
                                        XALLOC_MEMTYPE_PHYSICAL));
    }

    rwRETURNVOID();
}
