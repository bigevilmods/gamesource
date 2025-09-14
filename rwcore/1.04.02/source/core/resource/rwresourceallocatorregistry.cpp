// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Implementation of the Resource Allocator Registry

#include "rwsharedheaders.hpp"
#include "rw/core/memory/memoryalignedfunctions.h"
#include "rw/core/resource/resourceallocatortarget.h"
#if defined(RWCROSS)
#include "rw/core/resource/zeroingresourceallocatortarget.h"
#endif
#include "rw/core/resource/resourceallocatorregistry.h"

namespace rw
{

IResourceAllocator *ResourceAllocatorRegistry::s_defaultAllocator = 0;

struct DefaultSystemAllocatorInitializer
{
public:
    DefaultSystemAllocatorInitializer();
};

DefaultSystemAllocatorInitializer::DefaultSystemAllocatorInitializer()
{
    rwFUNCTION();
#if defined(RWCROSS)
        static ZeroingSystemAllocator defaultSystemAllocator;
        if(!ResourceAllocatorRegistry::s_defaultAllocator)
        {
            ResourceAllocatorRegistry::SetDefaultAllocator(&defaultSystemAllocator);
        }
#else
        static SystemAllocator defaultSystemAllocator;
        if(!ResourceAllocatorRegistry::s_defaultAllocator)
        {
            ResourceAllocatorRegistry::SetDefaultAllocator(&defaultSystemAllocator);
        }
#endif
    rwRETURNVOID();
}

// return default allocator
IResourceAllocator&
ResourceAllocatorRegistry::GetDefaultAllocator()
{
    static DefaultSystemAllocatorInitializer initializer;

    rwFUNCTION();
    rwRETURN(*s_defaultAllocator);
}

// sets the default allocator
void
ResourceAllocatorRegistry::SetDefaultAllocator(IResourceAllocator *allocator)
{
    rwFUNCTION();
    s_defaultAllocator = allocator;
    rwRETURNVOID();
}

}
