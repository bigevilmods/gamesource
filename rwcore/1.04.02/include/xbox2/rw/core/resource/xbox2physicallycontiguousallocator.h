// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Xbox2 classes for the resource descriptors and resources for physicially-contiguous memory

#ifndef RW_XBOX2PHYSICALLYCONTIGUOUSALLOCATOR_HPP
#define RW_XBOX2PHYSICALLYCONTIGUOUSALLOCATOR_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/resource/baseresource.h>
#include <rw/core/resource/resource.h>

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
//                                                Xbox2SystemAllocator Class Definition
// ***********************************************************************************************************

/**
class Xbox2PhysicallyContiguousAllocator
*/
class Xbox2PhysicallyContiguousAllocator
{
public:

    enum PageSize
    {
        PAGESIZE_4K = 0,
        PAGESIZE_64K,
        PAGESIZE_16M
    };

    /**
    Constructor.
    */
    Xbox2PhysicallyContiguousAllocator()
    {
    }

    /**
    Destructor.
    */
    ~Xbox2PhysicallyContiguousAllocator()
    {
    }

    /**
    Allocate a resource based on the resource descriptor.
    */
    BaseResource Allocate(const BaseResourceDescriptor& resourceDescriptor,
                          PageSize pageSize = PAGESIZE_64K);

    /**
    Free a resource.
    */
    void Free(const BaseResource& resource);

private:
    // not implemented
    Xbox2PhysicallyContiguousAllocator(const Xbox2PhysicallyContiguousAllocator& other);
    Xbox2PhysicallyContiguousAllocator operator=(const Xbox2PhysicallyContiguousAllocator& other);
};

typedef Xbox2PhysicallyContiguousAllocator PhysicallyContiguousAllocator;

} //namespace rw

#endif // RW_XBOX2PHYSICALLYCONTiguousALLOCATOR_HPP
