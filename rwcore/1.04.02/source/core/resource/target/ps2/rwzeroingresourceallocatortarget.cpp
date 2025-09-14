// (c) Electronic Arts. All Rights Reserved.

/// \file
/// This file is required so that incgen can correctly order the header files needed for rwresource.h.

#include "rwsharedheaders.hpp"
#include <new>
#include "rw/core/memory/memoryalignedfunctions.h"
#include "rw/core/resource/resourceallocatortarget.h"
#include "rw/core/resource/resourceallocator.h"
#include "rw/core/resource/resourceallocatortarget.h"
#include "rw/core/resource/zeroingresourceallocatortarget.h"

using namespace rw;

ZeroingSystemAllocatorGeneric::ZeroingSystemAllocatorGeneric()
{
    rwFUNCTION();
    rwRETURNVOID();
}

ZeroingSystemAllocatorGeneric::~ZeroingSystemAllocatorGeneric()
{
    rwFUNCTION();
    rwRETURNVOID();
}


ResourceDescriptor
ZeroingSystemAllocatorGeneric::GetResourceDescriptor()
{
    rwFUNCTION();
    // give it a non-zero size
    rwRETURN(ResourceDescriptor(1, 1));
}

ZeroingSystemAllocatorGeneric*
ZeroingSystemAllocatorGeneric::Initialize(const Resource& resource)
{
    rwFUNCTION();
    MemoryResource memPtr = resource.GetMemoryResource();
    // this must be done using in place new instead of a reinterpret_cast as it is a derived class
    // we can't inline it as it inplace new requires #include <new>
    ZeroingSystemAllocatorGeneric *newObject = new (memPtr) ZeroingSystemAllocatorGeneric;
    rwRETURN(newObject);
}

Resource
ZeroingSystemAllocatorGeneric::DoAllocate(const ResourceDescriptor &resourceDescriptor, const char8_t *name)
{
    rwFUNCTION();

    // Allocate the resources.
    Resource resource = SystemAllocatorGeneric::DoAllocate(resourceDescriptor, name);

    // Now set the memory to Zero.
    for (uint32_t index = 0; index < Resource::GetNumberOfBaseResourceTypes(); ++index)
    {
        BaseResourceDescriptor resDesc = resourceDescriptor.GetBaseResourceDescriptor(index);
        if (resDesc.GetSize() > 0)
        {
            memset(resource.GetBaseResource(index), 0, resDesc.GetSize());
        }
    }

    rwRETURN(resource);
}