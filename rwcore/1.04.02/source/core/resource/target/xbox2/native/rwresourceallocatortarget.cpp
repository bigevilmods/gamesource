// (c) Electronic Arts. All Rights Reserved.

/// \file
/// This file is required so that incgen can correctly order the header files needed for rwresource.h.

#include "rwsharedheaders.hpp"
#include <new>
#include "rw/core/memory/memoryalignedfunctions.h"
#include "rw/core/resource/resourceallocatortarget.h"
#include "rw/core/resource/xbox2physicallycontiguousallocator.h"

using namespace rw;

Xbox2NativeSystemAllocator::Xbox2NativeSystemAllocator()
{
    rwFUNCTION();
    rwRETURNVOID();
}

Xbox2NativeSystemAllocator::~Xbox2NativeSystemAllocator()
{
    rwFUNCTION();
    rwRETURNVOID();
}


ResourceDescriptor
Xbox2NativeSystemAllocator::GetResourceDescriptor()
{
    rwFUNCTION();
    // give it a non-zero size
    rwRETURN(ResourceDescriptor(1, 1));
}

Xbox2NativeSystemAllocator*
Xbox2NativeSystemAllocator::Initialize(const Resource& resource)
{
    rwFUNCTION();
    MemoryResource memPtr = resource.GetMemoryResource();
    // this must be done using in place new instead of a reinterpret_cast as it is a derived class
    // we can't inline it as it inplace new requires #include <new>
    Xbox2NativeSystemAllocator *newObject = new (memPtr) Xbox2NativeSystemAllocator;
    rwRETURN(newObject);
}

uint32_t
Xbox2NativeSystemAllocator::Xbox2GetPhysicalPageSize()
{
    rwFUNCTION();
    rwRETURN(64 * 1024);
}

Resource
Xbox2NativeSystemAllocator::DoAllocate(const ResourceDescriptor& resourceDescriptor, const char8_t *RWUNUSED(name))
{
    rwFUNCTION();

    Resource resource;
    for (uint32_t index = 0; index < Resource::GetNumberOfBaseResourceTypes(); ++index)
    {
        BaseResourceDescriptor resDesc = resourceDescriptor.GetBaseResourceDescriptor(index);
        if (resDesc.GetSize() > 0)
        {
            if (Resource::IsBaseResourceTypePhysical(index))
            {
                resource.GetBaseResource(index) = Xbox2PhysicallyContiguousAllocator().Allocate(resDesc,
                                                            Xbox2PhysicallyContiguousAllocator::PAGESIZE_64K);
            }
            else
            {
                resource.GetBaseResource(index) = RwMallocAligned(resDesc.GetSize(), resDesc.GetAlignment());
            }
            rwENFORCE(resource.GetBaseResource(index), ("Memory allocation failed."));
        }
    }

    rwRETURN(resource);
}

void
Xbox2NativeSystemAllocator::DoFree(const Resource& resource)
{
    rwFUNCTION();

    for (uint32_t index = 0; index < Resource::GetNumberOfBaseResourceTypes(); ++index)
    {
        if (Resource::IsBaseResourceTypePhysical(index))
        {
            Xbox2PhysicallyContiguousAllocator().Free(resource.GetBaseResource(index));
        }
        else
        {
            RwFreeAligned(resource.GetBaseResource(index));
        }
    }

    rwRETURNVOID();
}
