// (c) Electronic Arts. All Rights Reserved.

/// \file
/// PS3 resource allocator

#ifndef RW_RESOURCEALLOCATORGENERIC_HPP
#define RW_RESOURCEALLOCATORGENERIC_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/resource/resourceallocator.h>
#include <stdio.h> // used for printf (debug)

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

#if !defined(RWCROSS)
    /**
     Set the base address and size of GraphicsSystem memory
     */
    void PS3GraphicsSystemMemoryInitialize(void *, uint32_t size);

    /**
     Set the base address and size of GraphicsLocal memory
     */
    void PS3GraphicsLocalMemoryInitialize(void *, uint32_t size);
#endif

protected:
    // Implementation routines used by the IResourceAllocator Allocate and
    // Free methods
    Resource
    DoAllocate(const ResourceDescriptor &resourceDescriptor, const char8_t *name);

    void
    DoFree(const Resource &resource);

#if !defined(RWCROSS)
    static void      *m_graphicsSystemStart;
    static uint32_t   m_graphicsSystemSize;

    static void      *m_graphicsLocalStart;
    static uint32_t   m_graphicsLocalSize;
#endif

private:
    // not implemented
    SystemAllocatorGeneric(const SystemAllocatorGeneric& other);
    SystemAllocatorGeneric operator=(const SystemAllocatorGeneric& other);
};

typedef SystemAllocatorGeneric SystemAllocator;


#if !defined(RWCROSS)
inline void
SystemAllocatorGeneric::PS3GraphicsSystemMemoryInitialize(void *start, uint32_t size)
{
    rwFUNCTION();
    rwASSERTM(m_graphicsSystemStart == 0, ("SystemAllocatorGeneric GraphicsSystem memory already initialized."));
    rwMESSAGE((" Initializing GraphicsSystem memory.  0x%x bytes at %p\n", size, start));
    m_graphicsSystemStart = start;
    m_graphicsSystemSize = size;
    rwRETURNVOID();
}
#endif

#if !defined(RWCROSS)
inline void
SystemAllocatorGeneric::PS3GraphicsLocalMemoryInitialize(void *start, uint32_t size)
{
    rwFUNCTION();
    rwASSERTM(m_graphicsLocalStart == 0, ("SystemAllocatorGeneric GraphicsLocal memory already initialized."));
    rwMESSAGE((" Initializing RSXLocal memory.  0x%x bytes at %p\n", size, start));
    m_graphicsLocalStart = start;
    m_graphicsLocalSize = size;
    rwRETURNVOID();
}
#endif

} //namespace rw

#endif // RW_RESOURCEALLOCATORGENERIC_HPP
