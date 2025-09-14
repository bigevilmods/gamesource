// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Xbox2 resource allocator that zeros out its memory.

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
//                                                ZeroingXbox2CrossSystemAllocator Class Definition
// ***********************************************************************************************************

/**
 An Allocator which will defer memory allocations and releases to RwMallocAligned and RwFreeAligned.
 */
class ZeroingXbox2CrossSystemAllocator : public Xbox2CrossSystemAllocator
{
public:
    /**
     Default constructor.
     */
    ZeroingXbox2CrossSystemAllocator();

    /**
     Destructor.
     */
    virtual ~ZeroingXbox2CrossSystemAllocator();

    /**
     Get the resource descriptor describing the resource requirements of Xbox2CrossSystemAllocator.
     */
    static ResourceDescriptor GetResourceDescriptor();

    /**
     Construct a Xbox2CrossSystemAllocator into a Resource.
     */
    static ZeroingXbox2CrossSystemAllocator* Initialize(const Resource& resource);

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
    ZeroingXbox2CrossSystemAllocator(const ZeroingXbox2CrossSystemAllocator& other);
    ZeroingXbox2CrossSystemAllocator operator=(const ZeroingXbox2CrossSystemAllocator& other);
};

typedef ZeroingXbox2CrossSystemAllocator ZeroingSystemAllocator;


} //namespace rw

#endif // RW_ZEROINGRESOURCEALLOCATOR_HPP
