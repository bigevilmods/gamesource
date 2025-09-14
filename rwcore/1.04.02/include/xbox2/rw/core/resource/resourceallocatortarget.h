// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Xbox2 host/target-specific resource allocator.

#ifndef RW_RESOURCEALLOCATORTARGET_HPP
#define RW_RESOURCEALLOCATORTARGET_HPP
#include <rw/core/base/ostypes.h>
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

#if defined(RWCROSS)

// ***********************************************************************************************************
//                                                Xbox2CrossSystemAllocator Class Definition
// ***********************************************************************************************************

/**
 An Allocator which will defer memory allocations and releases to RwMallocAligned and RwFreeAligned.
*/
class Xbox2CrossSystemAllocator : public IResourceAllocator
{
public:
    /**
     Default constructor.
    */
    Xbox2CrossSystemAllocator();

    /**
     Destructor.
    */
    ~Xbox2CrossSystemAllocator();

    /**
     Get the resource descriptor describing the resource requirements of Xbox2CrossSystemAllocator.
    */
    static ResourceDescriptor GetResourceDescriptor();

    /**
     Construct a Xbox2CrossSystemAllocator into a Resource.
    */
    static Xbox2CrossSystemAllocator* Initialize(const Resource& resource);

protected:
    // Implementation routines used by the IResourceAllocator Allocate and
    // Free methods
    Resource
    DoAllocate(const ResourceDescriptor &resourceDescriptor, const char8_t *name);

    void
    DoFree(const Resource &resource);

private:
    // not implemented
    Xbox2CrossSystemAllocator(const Xbox2CrossSystemAllocator& other);
    Xbox2CrossSystemAllocator operator=(const Xbox2CrossSystemAllocator& other);
};

/**
*/
typedef Xbox2CrossSystemAllocator SystemAllocator;

#else // defined(RWCROSS)

// ***********************************************************************************************************
//                                                Xbox2NativeSystemAllocator Class Definition
// ***********************************************************************************************************

class Xbox2NativeSystemAllocator : public IResourceAllocator
{
public:
    /**
    Constructor.
    */
    Xbox2NativeSystemAllocator();
    /**
    Destructor.
    */
    ~Xbox2NativeSystemAllocator();

    /**
    Get the resource descriptor describing the requirements of Xbox2NativeSystemAllocator.
    */
    static ResourceDescriptor GetResourceDescriptor();

    /**
    Initialise a XboxNativeSystemAllocator into a resource.
    */
    Xbox2NativeSystemAllocator* Initialize(const Resource& resource);


    /**
    Gets the page size in bytes used to allocate physical memory.
    */
    static uint32_t
    Xbox2GetPhysicalPageSize();

protected:
    // Implementation routines used by the IResourceAllocator Allocate and Free methods
    Resource DoAllocate(const ResourceDescriptor &resourceDescriptor, const char8_t *name);
    void DoFree(const Resource &resource);

private:
    // not implemented
    Xbox2NativeSystemAllocator(const Xbox2NativeSystemAllocator& other);
    Xbox2NativeSystemAllocator operator=(const Xbox2NativeSystemAllocator& other);
};

typedef Xbox2NativeSystemAllocator SystemAllocator;

#endif // defined(RWCROSS)

} //namespace rw

#endif // RW_RESOURCEALLOCATORTARGET_HPP
