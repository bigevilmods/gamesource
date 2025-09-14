// (c) Electronic Arts. All Rights Reserved.

/// \file
/// This file is required so that incgen can correctly order the header files needed for rwresource.h.

#include "rwsharedheaders.hpp"
#include <new>
#include "rw/core/memory/memoryalignedfunctions.h"
#include "rw/core/resource/resourceallocatortarget.h"

using namespace rw;
namespace rw
{

SystemAllocatorGeneric::SystemAllocatorGeneric()
{
    rwFUNCTION();
    rwRETURNVOID();
}

SystemAllocatorGeneric::~SystemAllocatorGeneric()
{
    rwFUNCTION();
    rwRETURNVOID();
}


ResourceDescriptor
SystemAllocatorGeneric::GetResourceDescriptor()
{
    rwFUNCTION();
    // give it a non-zero size
    rwRETURN(ResourceDescriptor(1, 1));
}

SystemAllocatorGeneric*
SystemAllocatorGeneric::Initialize(const Resource& resource)
{
    rwFUNCTION();
    MemoryResource memPtr = resource.GetMemoryResource();
    // this must be done using in place new instead of a reinterpret_cast as it is a derived class
    // we can't inline it as it inplace new requires #include <new>
    SystemAllocatorGeneric *newObject = new (memPtr) SystemAllocatorGeneric;
    rwRETURN(newObject);
}


Resource
SystemAllocatorGeneric::DoAllocate(const ResourceDescriptor& resourceDescriptor, const char8_t *RWUNUSED(name))
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
SystemAllocatorGeneric::DoFree(const Resource& resource)
{
    rwFUNCTION();

    for (uint32_t index = 0; index < Resource::GetNumberOfBaseResourceTypes(); ++index)
    {
        RwFreeAligned(resource.GetBaseResource(index));
    }

    rwRETURNVOID();
}

} // end namespace rw
