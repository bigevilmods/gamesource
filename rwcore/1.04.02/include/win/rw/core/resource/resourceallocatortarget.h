// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Windows (generic) resource allocator

#ifndef RW_RESOURCEALLOCATORGENERIC_HPP
#define RW_RESOURCEALLOCATORGENERIC_HPP
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

// ***********************************************************************************************************
//                                                SystemAllocatorGeneric Class Definition
// ***********************************************************************************************************

/**
 An Allocator which will defer memory allocations and releases to RwMallocAligned and RwFreeAligned.
*/
class SystemAllocatorGeneric : public IResourceAllocator
{
public:
    /**
     Default constructor.
     */
    SystemAllocatorGeneric();

    /**
     Destructor.
     */
    ~SystemAllocatorGeneric();

    /**
     Get the resource descriptor describing the resource requirements of SystemAllocatorGeneric.
     */
    static ResourceDescriptor GetResourceDescriptor();

    /**
     Construct a SystemAllocatorGeneric into a Resource.
     */
    static SystemAllocatorGeneric* Initialize(const Resource& resource);

protected:
    // Implementation routines used by the IResourceAllocator Allocate and
    // Free methods
    Resource
    DoAllocate(const ResourceDescriptor &resourceDescriptor, const char8_t *name);

    void
    DoFree(const Resource &resource);

private:
    // not implemented
    SystemAllocatorGeneric(const SystemAllocatorGeneric& other);
    SystemAllocatorGeneric operator=(const SystemAllocatorGeneric& other);
};

typedef SystemAllocatorGeneric SystemAllocator;

} //namespace rw

#endif // RW_RESOURCEALLOCATORGENERIC_HPP
