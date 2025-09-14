// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Implementation of a linear resource allocator

// ***********************************************************************************************************
// Includes
#include "rwsharedheaders.hpp"
#include <new>
#include "rw/core/memory/memoryalignedfunctions.h"
#include "rw/core/resource/linearresourceallocator.h"

using namespace rw;

/**
 Construct a LinearResourceAllocator using provided memory for the internal heap.
 \param heapResource the memory to use for the internal heap.
 \param heapCapacity the size and alignment of the internal heap.
 */
LinearResourceAllocator::LinearResourceAllocator(const Resource& heapResource,
                                                         const ResourceDescriptor& heapCapacity)
  : m_heapResource(heapResource),
    m_heapCapacity(heapCapacity),
    m_currentUsage(),
    m_paddingUsed(),
    m_numAllocations(0)
{
    rwFUNCTION();
    rwASSERT(heapResource);
    rwRETURNVOID();
}

/**
 Destruct the allocator.
 */
LinearResourceAllocator::~LinearResourceAllocator()
{
    rwFUNCTION();
    rwRETURNVOID();
}

ResourceDescriptor
LinearResourceAllocator::GetResourceDescriptor(const ResourceDescriptor& heapCapacityRequired)
{
    rwFUNCTION();
    ResourceDescriptor resourcesRequired = heapCapacityRequired;
    uint32_t allocatorAlignment = heapCapacityRequired.GetAlignment();
    uint32_t allocatorSize = SizeAlign(sizeof(LinearResourceAllocator), allocatorAlignment);
    resourcesRequired += ResourceDescriptor(allocatorSize, allocatorAlignment);
    rwRETURN(resourcesRequired);
}

LinearResourceAllocator*
LinearResourceAllocator::Initialize(const Resource& resource,
                                    const ResourceDescriptor& heapCapacityRequired)
{
    rwFUNCTION();

    void *allocatorPtr = resource.GetMemoryResource();

    // place the internal heap after the allocator in main memory, checking for alignment first
    uint32_t allocatorAlignment = heapCapacityRequired.GetAlignment();
    uint32_t allocatorSize = SizeAlign(sizeof(LinearResourceAllocator), allocatorAlignment);
    Resource heapResource = resource + ResourceDescriptor(allocatorSize, allocatorAlignment);
    rwRETURN(new (allocatorPtr) LinearResourceAllocator(heapResource, heapCapacityRequired));
}

rw::ResourceDescriptor
LinearResourceAllocator::GetResourceDescriptor(const Resource & RWUNUSED(heapResource),
                                               const ResourceDescriptor& RWUNUSED(heapResourceDescriptor))
{
    rwFUNCTION();
    rwRETURN(ResourceDescriptor(sizeof(LinearResourceAllocator), rwTARGET_POINTERALIGNMENT));
}

LinearResourceAllocator*
LinearResourceAllocator::Initialize(const rw::Resource& resource,
                                    const Resource& heapResource,
                                    const ResourceDescriptor& heapResourceDescriptor)
{
    rwFUNCTION();

    void *allocatorPtr = resource.GetMemoryResource();
    rwRETURN(new (allocatorPtr) LinearResourceAllocator(heapResource, heapResourceDescriptor));
}

// This is deprecated functionality
ResourceDescriptor
LinearResourceAllocator::GetResourceDescriptor(uint32_t heapSize, uint32_t heapAlignment)
{
    rwFUNCTION();
    rwDEPRECATEDM("Use GetResourceDescriptor(ResourceDescriptor& heapRequirements)");
    rwRETURN(GetResourceDescriptor(ResourceDescriptor(heapSize, heapAlignment)));
}

// This is deprecated functionality
LinearResourceAllocator*
LinearResourceAllocator::Initialize(const Resource& resource,
                                        uint32_t heapSize, uint32_t heapAlignment)
{
    rwFUNCTION();
    rwDEPRECATEDM("Use Initialize(const Resource& resource, ResourceDescriptor& heapRequirements)");
    rwRETURN(Initialize(resource, ResourceDescriptor(heapSize, heapAlignment)));
}

/**
 Release the LinearResourceAllocator.
 */
void
LinearResourceAllocator::Release()
{
    rwFUNCTION();
    this->~LinearResourceAllocator();
    rwRETURNVOID();
}

/**
 Allocate a resource. Forward the call to the target specific implementation,
 \param allocationRequired The descriptor describing the resource requirements.
 \return The allocated Resource.
 */
Resource
LinearResourceAllocator::DoAllocate(const ResourceDescriptor &allocationRequired, const char8_t *RWUNUSED(name))
{
    rwFUNCTION();

    Resource preAlignedResource = m_heapResource + m_currentUsage;
    Resource alignedResource = preAlignedResource;
    alignedResource.Align(allocationRequired);

    ResourceDescriptor paddingRequired = alignedResource - preAlignedResource;
    rwENFORCE(m_currentUsage + paddingRequired + allocationRequired <= m_heapCapacity,
              ("Not enough capacity remaining in LinearResourceAllocator for allocation."));

    if (m_currentUsage + paddingRequired + allocationRequired <= m_heapCapacity)
    {
        // zero out base resources that weren't specified in ResourceDescriptor
        for (uint32_t i = 0; i < ResourceDescriptor::GetNumberOfBaseResourceTypes(); ++i)
        {
            if (0 == allocationRequired.GetBaseResourceDescriptor(i).GetSize())
            {
                alignedResource.GetBaseResource(i) = (void *)0;
            }
        }

        // record the allocation
        m_currentUsage += paddingRequired;
        m_currentUsage += allocationRequired;
        // reset the alignments of m_currentUsage to 1
        for (uint32_t i = 0; i < ResourceDescriptor::GetNumberOfBaseResourceTypes(); ++i)
        {
            m_currentUsage.GetBaseResourceDescriptor(i).SetAlignment(1);
        }
        m_paddingUsed  += paddingRequired;
        ++m_numAllocations;
    }
    else
    {
        // not enough capacity to fulfill the request; set all base resources to NULL
        uint32_t i;
        for (i = 0; i < ResourceDescriptor::GetNumberOfBaseResourceTypes(); ++i)
        {
            alignedResource.GetBaseResource(i) = NULL;
        }
        if (allocationRequired.GetBaseResourceDescriptor(i).GetSize() == 0)
        {
            alignedResource.GetBaseResource(i) = (void *)0;
        }
    }

    rwRETURN(alignedResource);
}


/**
 Free a resource. This does nothing for a linear allocator.
 \param resource The resource to be freed.
 */
void
LinearResourceAllocator::DoFree(const Resource &RWUNUSED(resource))
{
    rwFUNCTION();
    rwRETURNVOID();
}


/**
 \return the total memory available for allocation by the LinearResourceAllocator.
  This total includes memory already allocated by the LinearResourceAllocator.
 */
ResourceDescriptor
LinearResourceAllocator::GetCapacity() const
{
    rwFUNCTION();
    rwRETURN(m_heapCapacity);
}

/**
 \return the amount of space currently allocated by the linear allocator.
 */
ResourceDescriptor
LinearResourceAllocator::GetCurrentUsage() const
{
    rwFUNCTION();
    rwRETURN(m_currentUsage);
}

/**
 \return the amount of space currently available to the linear allocator.
 */
ResourceDescriptor
LinearResourceAllocator::GetAvailable() const
{
    rwFUNCTION();
    rwRETURN(m_heapCapacity - m_currentUsage);
}

/**
 \return the number of allocations which have been made by the LinearResourceAllocator.
 */
uint32_t
LinearResourceAllocator::GetNumAllocations() const
{
    rwFUNCTION();
    rwRETURN(m_numAllocations);
}

/**
 \return the number of bytes wasted by alignment padding.
 */
ResourceDescriptor
LinearResourceAllocator::GetPaddingUsed() const
{
    rwFUNCTION();
    rwRETURN(m_paddingUsed);
}

/**
 Get a restore point. Memory allocations after this restore point can be released by passing this restore
 point to clear.
 \param a reference to the restorePoint which will be modified to store the current state
 \see Clear
 */
void
LinearResourceAllocator::GetRestorePoint(LinearResourceAllocator::RestorePoint& restorePoint) const
{
    rwFUNCTION();
    rwASSERT(m_heapResource);
    restorePoint.currentUsage   = m_currentUsage;
    restorePoint.paddingUsed    = m_paddingUsed;
    restorePoint.numAllocations = m_numAllocations;
    rwRETURNVOID();
}


/**
 Clear the heap to a restore point. All memory allocated since that restore point was acquires is made
 available for future allocations.
 \param the restorePoint which will be used to restore the allocator to a previous state.
 \see GetRestorePoint
 */
void
LinearResourceAllocator::Clear(const LinearResourceAllocator::RestorePoint& restorePoint)
{
    rwFUNCTION();
    rwASSERT(m_heapResource);
    rwASSERT(restorePoint.currentUsage   <= m_currentUsage);
    rwASSERT(restorePoint.paddingUsed    <= m_paddingUsed);
    rwASSERT(restorePoint.numAllocations <= m_numAllocations);
    m_currentUsage   = restorePoint.currentUsage;
    m_numAllocations = restorePoint.numAllocations;
    m_paddingUsed    = restorePoint.paddingUsed;
    rwRETURNVOID();
}


/**
 Clear the heap completely, making all memory in the LinearResourceAllocator available for future
 allocations.
 */
void
LinearResourceAllocator::Clear()
{
    rwFUNCTION();
    m_currentUsage   = ResourceDescriptor(0, m_heapCapacity.GetAlignment());
    m_paddingUsed    = ResourceDescriptor(0, 1);
    m_numAllocations = 0;
    rwRETURNVOID();
}

const Resource&
LinearResourceAllocator::GetLinearHeapBase(void) const
{
    rwRETURN(m_heapResource);
}
