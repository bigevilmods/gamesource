// (c) Electronic Arts. All Rights Reserved.

/// \file
/// This file is required so that incgen can correctly order the header files needed for rwresource.h.

#include "rwsharedheaders.hpp"
#include <new>
#include <windows.h>
#include "rw/core/memory/memoryalignedfunctions.h"
#include "rw/core/resource/resourceallocatortarget.h"
#include "rw/core/resource/resourceallocator.h"
#include "rw/core/resource/resourceallocatortarget.h"
#include "rw/core/resource/zeroingresourceallocatortarget.h"

using namespace rw;

ZeroingXbox2CrossSystemAllocator::ZeroingXbox2CrossSystemAllocator()
{
    rwFUNCTION();
    rwRETURNVOID();
}

ZeroingXbox2CrossSystemAllocator::~ZeroingXbox2CrossSystemAllocator()
{
    rwFUNCTION();
    rwRETURNVOID();
}


ResourceDescriptor
ZeroingXbox2CrossSystemAllocator::GetResourceDescriptor()
{
    rwFUNCTION();
    // give it a non-zero size
    rwRETURN(ResourceDescriptor(1, 1));
}

ZeroingXbox2CrossSystemAllocator*
ZeroingXbox2CrossSystemAllocator::Initialize(const Resource& resource)
{
    rwFUNCTION();
    MemoryResource memPtr = resource.GetMemoryResource();
    // this must be done using in place new instead of a reinterpret_cast as it is a derived class
    // we can't inline it as it inplace new requires #include <new>
    ZeroingXbox2CrossSystemAllocator *newObject = new (memPtr) ZeroingXbox2CrossSystemAllocator;
    rwRETURN(newObject);
}

Resource
ZeroingXbox2CrossSystemAllocator::DoAllocate(const ResourceDescriptor &resourceDescriptor, const char8_t *name)
{
    rwFUNCTION();

    // Allocate the resources.
    Resource resource = Xbox2CrossSystemAllocator::DoAllocate(resourceDescriptor, name);

    // Now set the memory to Zero.
    for (uint32_t index = 0; index < Resource::GetNumberOfBaseResourceTypes(); ++index)
    {
        BaseResourceDescriptor resDesc = resourceDescriptor.GetBaseResourceDescriptor(index);
        if (resDesc.GetSize() > 0)
        {
            ZeroMemory(resource.GetBaseResource(index), resDesc.GetSize());
        }
    }

    rwRETURN(resource);
}