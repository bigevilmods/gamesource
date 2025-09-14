// (c) Electronic Arts. All Rights Reserved.

/// \file
/// An allocator which remembers what has been allocated and so can automatically free up resources.

#ifndef RW_LINEARRESOURCEALLOCATOR_HPP
#define RW_LINEARRESOURCEALLOCATOR_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>
#include <rw/core/debug/exception.h>
#include <rw/core/resource/baseresource.h>
#include <rw/core/resource/resource.h>
#include <rw/core/resource/resourcetarget.h>
#include <rw/core/resource/resourceallocator.h>

// ***********************************************************************************************************
// Includes

namespace rw
{

// ***********************************************************************************************************
// Defines

// ***********************************************************************************************************
// Forward Declarations

// ***********************************************************************************************************
// Typedefs

// ***********************************************************************************************************
// Enums + Consts

// ***********************************************************************************************************
// Global Variables

// ***********************************************************************************************************
// External Functions Prototypes

// ***********************************************************************************************************
// Structs + Unions + Classes

// ***********************************************************************************************************
//                                                LinearResourceAllocator Class Definition
// ***********************************************************************************************************

/**
 A Linear memory allocator.
 Memory blocks are allocated sequentially on an internal heap until the heap is exhausted.
 Memory release is handled by getting and setting restore points. All memory allocated after getting a
 restore point can be deallocated by clearing to that restore point. Freeing individual blocks is a no-op.
 */
class LinearResourceAllocator : public IResourceAllocator
{
public:
    struct RestorePoint
    {
        ResourceDescriptor currentUsage;
        ResourceDescriptor paddingUsed;
        uint32_t           numAllocations;
    };

    /**
    Create an internal heap
    */
    static rw::ResourceDescriptor GetResourceDescriptor(const ResourceDescriptor& heapCapacityRequired);
    static LinearResourceAllocator* Initialize(const rw::Resource& resource,
                                                   const ResourceDescriptor& heapCapacityRequired);

    /**
    This is for allocating from an external heap passed in by the owner.
    */
    static rw::ResourceDescriptor GetResourceDescriptor(const Resource &heapResource,
                                                        const ResourceDescriptor& heapResourceDescriptor);
    static LinearResourceAllocator* Initialize(const rw::Resource& resource,
                                               const Resource& heapResource,
                                               const ResourceDescriptor& heapResourceDescriptor);

    static rw::ResourceDescriptor GetResourceDescriptor(uint32_t heapSize, uint32_t heapAlignment);
    static LinearResourceAllocator* Initialize(const rw::Resource& resource,
                                                   uint32_t heapSize, uint32_t heapAlignment);
    void Release();

    ResourceDescriptor
    GetCapacity() const;

    ResourceDescriptor
    GetCurrentUsage() const;

    ResourceDescriptor
    GetAvailable() const;

    uint32_t
    GetNumAllocations() const;

    ResourceDescriptor
    GetPaddingUsed() const;

    void
    GetRestorePoint(RestorePoint& restorePoint) const;

    void
    Clear(const RestorePoint& restorePoint);

    void
    Clear();

    const Resource&
    GetLinearHeapBase(void) const;


protected:
    // Implementation routines used by the IResourceAllocator Allocate and Free methods
    Resource DoAllocate(const ResourceDescriptor &resourceDescriptor, const char8_t *name);
    void DoFree(const Resource &resource);


private:
    LinearResourceAllocator(const Resource& heapResource, const ResourceDescriptor& heapCapacity);
    virtual ~LinearResourceAllocator();

    // not implemented
    LinearResourceAllocator(const LinearResourceAllocator& other);
    LinearResourceAllocator operator=(const LinearResourceAllocator& other);

    Resource           m_heapResource;
    ResourceDescriptor m_heapCapacity;
    ResourceDescriptor m_currentUsage;
    ResourceDescriptor m_paddingUsed;
    uint32_t           m_numAllocations;
};

// Inline Functions

} //namespace rw

#endif // RW_LINEARRESOURCEALLOCATOR_HPP
