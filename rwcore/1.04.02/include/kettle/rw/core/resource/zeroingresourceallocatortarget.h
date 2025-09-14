// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Kettle (generic) resource allocator that zeros out its memory.

#ifndef RW_ZEROINGRESOURCEALLOCATOR_HPP
#define RW_ZEROINGRESOURCEALLOCATOR_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/resource/resourceallocator.h>
#include <rw/core/resource/resourceallocatortarget.h>


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
//                                                ZeroingSystemAllocatorGeneric Class Definition
// ***********************************************************************************************************

/**
 An Allocator which will defer memory allocations and releases to RwMallocAligned and RwFreeAligned.
 */
class ZeroingSystemAllocatorGeneric : public SystemAllocatorGeneric
{
public:
    /**
     Default constructor.
     */
    ZeroingSystemAllocatorGeneric();

    /**
     Destructor.
     */
    virtual ~ZeroingSystemAllocatorGeneric();

    /**
     Get the resource descriptor describing the resource requirements of SystemAllocatorGeneric.
     */
    static ResourceDescriptor GetResourceDescriptor();

    /**
     Construct a SystemAllocatorGeneric into a Resource.
     */
    static ZeroingSystemAllocatorGeneric* Initialize(const Resource& resource);

 protected:
    /**
     Child interface, concrete implementations of IResourceAllocator should implement these
     functions rather than the public non virtual members.
     \return the newly allocated Resource, returns a default allocated Resource on failure.
    */
    virtual Resource DoAllocate(const ResourceDescriptor &resourceDescriptor,
                                const char8_t *name = NULL);

private:
    // not implemented
    ZeroingSystemAllocatorGeneric(const ZeroingSystemAllocatorGeneric& other);
    ZeroingSystemAllocatorGeneric operator=(const ZeroingSystemAllocatorGeneric& other);
};

typedef ZeroingSystemAllocatorGeneric ZeroingSystemAllocator;


} //namespace rw

#endif // RW_ZEROINGRESOURCEALLOCATOR_HPP
