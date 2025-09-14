// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Registry for accessing an instance of the default resource allocator.

#ifndef RW_RESOURCEALLOCATORREGISTRY_HPP
#define RW_RESOURCEALLOCATORREGISTRY_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>
#include <rw/core/debug/exception.h>
#include <rw/core/resource/baseresource.h>
#include <rw/core/resource/resource.h>
#include <rw/core/resource/resourcetarget.h>
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
//                                                ResourceAllocatorRegistry Class Definition
// ***********************************************************************************************************
/**
class ResourceAllocateRegistry
*/
class ResourceAllocatorRegistry
{
public:
    static IResourceAllocator& GetDefaultAllocator();
    static void SetDefaultAllocator(IResourceAllocator *allocator);

private:
    ResourceAllocatorRegistry();

    static SystemAllocator DefaultSystemAllocator;
    static IResourceAllocator *s_defaultAllocator;

    // DefaultSystemAllocatorInitializer has given access to check if s_defaultAllocator
    // has been initialized or not.  This is to avoid branching in
    // ResourceAllocatorRegistry::GetDefaultAllocator implementation and achieving better performance.
    friend struct DefaultSystemAllocatorInitializer;

};

/**
\return the default allocator
*/
inline IResourceAllocator&
GetDefaultAllocator()
{
    rwFUNCTION();
    rwRETURN(ResourceAllocatorRegistry::GetDefaultAllocator());
}

} //namespace rw

#endif // RW_RESOURCEALLOCATORREGISTRY_HPP
