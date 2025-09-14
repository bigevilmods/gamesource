// (c) Electronic Arts. All Rights Reserved.

/// \file
/// This file is required so that incgen can correctly order the header files needed for rwresource.h.

#include "rwsharedheaders.hpp"
#include <new>
#include "rw/core/memory/memoryalignedfunctions.h"
#include "rw/core/resource/resourceallocatortarget.h"

using namespace rw;

Xbox2CrossSystemAllocator::Xbox2CrossSystemAllocator()
{
    rwFUNCTION();
    rwRETURNVOID();
}

Xbox2CrossSystemAllocator::~Xbox2CrossSystemAllocator()
{
    rwFUNCTION();
    rwRETURNVOID();
}

Resource
Xbox2CrossSystemAllocator::DoAllocate(const ResourceDescriptor& resourceDescriptor, const char8_t *RWUNUSED(name))
{
   rwFUNCTION();

    Resource resource;
    for (uint32_t index = 0; index < Resource::GetNumberOfBaseResourceTypes(); ++index)
    {
        BaseResourceDescriptor resDesc = resourceDescriptor.GetBaseResourceDescriptor(index);
        if (resDesc.GetSize() > 0)
        {
            resource.GetBaseResource(index) = RwMallocAligned(resDesc.GetSize(), resDesc.GetAlignment());
            rwENFORCE(resource.GetBaseResource(index), ("Memory allocation failed."));
        }
    }

    rwRETURN(resource);
}

void
Xbox2CrossSystemAllocator::DoFree(const Resource& resource)
{
    rwFUNCTION();

    for (uint32_t index = 0; index < Resource::GetNumberOfBaseResourceTypes(); ++index)
    {
        RwFreeAligned(resource.GetBaseResource(index));
    }

    rwRETURNVOID();
}


ResourceDescriptor
Xbox2CrossSystemAllocator::GetResourceDescriptor()
{
    rwFUNCTION();
    // give it a non-zero size
    rwRETURN(ResourceDescriptor(1, 1));
}

Xbox2CrossSystemAllocator*
Xbox2CrossSystemAllocator::Initialize(const Resource& resource)
{
    rwFUNCTION();
    MemoryResource memPtr = resource.GetMemoryResource();
    // this must be done using in place new instead of a reinterpret_cast as it is a derived class
    // we can't inline it as it inplace new requires #include <new>
    Xbox2CrossSystemAllocator *newObject = new (memPtr) Xbox2CrossSystemAllocator;
    rwRETURN(newObject);
}
