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

#if !defined(RWCROSS)
// Graphics memory variables
void      *SystemAllocatorGeneric::m_graphicsSystemStart = 0;
uint32_t   SystemAllocatorGeneric::m_graphicsSystemSize  = 0;

void      *SystemAllocatorGeneric::m_graphicsLocalStart  = 0;
uint32_t   SystemAllocatorGeneric::m_graphicsLocalSize   = 0;
#endif

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

#if !defined(RWCROSS)

    /**
    The standard system memory resources are allocated with multiple MallocAligned calls.
    The graphics specific are allocated seperately.
     */

    for (uint32_t index = 0 ;
         index <= BASERESOURCE_DISPOSABLE_UNINITIALIZED ;
         ++index)
    {
        BaseResourceDescriptor resDesc = resourceDescriptor.GetBaseResourceDescriptor(index);
        if (resDesc.GetSize() > 0)
        {
            resource.GetBaseResource(index) = RwMallocAligned(resDesc.GetSize(), resDesc.GetAlignment());
            rwENFORCE(resource.GetBaseResource(index), ("Memory allocation failed."));
        }
    }

    // GraphicsSystem
    BaseResourceDescriptor graphicsSystemResDesc = resourceDescriptor.GetGraphicsSystemResourceDescriptor();
    if (graphicsSystemResDesc.GetSize() != 0)
    {
        rwASSERTM(m_graphicsSystemStart != 0, ("GraphicsSystem memory has not been initialized"));

        rw::HostPtr currentPtr = (rw::HostPtr )m_graphicsSystemStart;
        uint32_t currentSize = m_graphicsSystemSize;

        // Move up to required alignment
        currentPtr = (currentPtr + graphicsSystemResDesc.GetAlignment() - 1)
            & ~(graphicsSystemResDesc.GetAlignment() - 1);
        rw::HostPtr padding = (rw::HostPtr)currentPtr - (rw::HostPtr )m_graphicsSystemStart;
        currentSize -= padding;

        rwASSERTM(graphicsSystemResDesc.GetSize() <= currentSize, ("Cannot allocate %d bytes of GraphicsSystem memory.\n",
                                                            graphicsSystemResDesc.GetSize()));
        m_graphicsSystemStart = (void *)(currentPtr + graphicsSystemResDesc.GetSize());
        m_graphicsSystemSize = currentSize - graphicsSystemResDesc.GetSize();

#if 0
        printf(" Request for 0x%x bytes of GraphicsSystem memory.  Using memory at %p.  Total 0x%x bytes left.\n",
               graphicsSystemResDesc.GetSize(), currentPtr, m_graphicsSystemSize);
#endif

        resource.GetGraphicsSystemResource() = (void *)currentPtr;
    }

    // GraphicsLocal
    BaseResourceDescriptor graphicsLocalResDesc = resourceDescriptor.GetGraphicsLocalResourceDescriptor();
    if (graphicsLocalResDesc.GetSize() != 0)
    {
        rwASSERTM(m_graphicsLocalStart != 0, ("GraphicsLocal memory has not been initialized"));

        rw::HostPtr currentPtr = (rw::HostPtr )m_graphicsLocalStart;
        uint32_t currentSize = m_graphicsLocalSize;

        // Move up to required alignment
        currentPtr = (currentPtr + graphicsLocalResDesc.GetAlignment() - 1)
            & ~(graphicsLocalResDesc.GetAlignment() - 1);
        rw::HostPtr padding = (rw::HostPtr)currentPtr - (rw::HostPtr )m_graphicsLocalStart;
        currentSize -= padding;

        rwASSERTM(graphicsLocalResDesc.GetSize() <= currentSize, ("Cannot allocate %d bytes of GraphicsLocal memory.\n",
                                                                  graphicsLocalResDesc.GetSize()));
        m_graphicsLocalStart = (void *)(currentPtr + graphicsLocalResDesc.GetSize());
        m_graphicsLocalSize = currentSize - graphicsLocalResDesc.GetSize();

#if 0
        printf(" Request for 0x%x bytes of RSXLocal memory.  Using memory at %p.  Total 0x%x bytes left.\n",
               graphicsLocalResDesc.GetSize(), currentPtr, m_graphicsLocalSize);
#endif

        resource.GetGraphicsLocalResource() = (void *)currentPtr;
    }

#else

    for (uint32_t index = 0 ;
         index < BASERESOURCE_NUMBEROFBASERESOURCETYPES ;
         ++index)
    {
        BaseResourceDescriptor resDesc = resourceDescriptor.GetBaseResourceDescriptor(index);
        if (resDesc.GetSize() > 0)
        {
            resource.GetBaseResource(index) = RwMallocAligned(resDesc.GetSize(), resDesc.GetAlignment());
            rwENFORCE(resource.GetBaseResource(index), ("Memory allocation failed."));
        }
    }

#endif

    rwRETURN(resource);
}

void
SystemAllocatorGeneric::DoFree(const Resource& resource)
{
    rwFUNCTION();

#if !defined(RWCROSS)

    for (uint32_t index = 0 ;
         index <= BASERESOURCE_DISPOSABLE_UNINITIALIZED ;
         ++index)
    {
        RwFreeAligned(resource.GetBaseResource(index));
    }

#else

    for (uint32_t index = 0; index < Resource::GetNumberOfBaseResourceTypes(); ++index)
    {
        RwFreeAligned(resource.GetBaseResource(index));
    }

#endif

    rwRETURNVOID();
}

} // end namespace rw
