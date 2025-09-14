// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Abstract base class for ResourceAllocators.

#ifndef RW_RESOURCEALLOCATOR_HPP
#define RW_RESOURCEALLOCATOR_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>
#include <rw/core/debug/exception.h>
#include <rw/core/resource/baseresource.h>
#include <rw/core/resource/resource.h>
#include <rw/core/resource/resourcetarget.h>
#include <coreallocator/icoreallocator_interface.h>

// ***********************************************************************************************************
// Includes

namespace rw
{

// ***********************************************************************************************************
// Defines

/**
 This macro should be used around any text descriptions
 passed to the IResourceAllocator so the descriptions
 are compiled out in the release configuration.
*/
#ifdef EA_DEBUG
#define RW_RESOURCE_ALLOCATION_DESC(x) (x)
#else
#define RW_RESOURCE_ALLOCATION_DESC(x) NULL
#endif

#define RW_DEFAULT_NAMESTR RW_RESOURCE_ALLOCATION_DESC(RW_CURRENT_FUNCTION)

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
//                                                IResourceAllocator Class Definition
// ***********************************************************************************************************


/**
 An abstract base class for resource allocators.
 Derived classes must override Allocate and Free
*/
class IResourceAllocator : public EA::Allocator::ICoreAllocator
{
public:
    /**
     Virtual destructor.
    */
    virtual ~IResourceAllocator()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Allocate a resource based on the resource descriptor.
     This function calls the virtual member function DoAllocate.
     \param resourceDescriptor a ResourceDescriptor specifying at least one non-zero-sized base resource.
     \param name for allocation block tracking
     \return the newly allocated Resource, returns a default constructed Resource on failure.
    */
    Resource Allocate(const ResourceDescriptor &resourceDescriptor, const char8_t *name = NULL)
    {
        rwFUNCTION();
#ifdef EA_DEBUG
        rwASSERTM(!(resourceDescriptor==ResourceDescriptor()),
                  ("ResourceDescriptor must specify at least one non-zero base-resource"));
        Resource checkedResource(DoAllocate(resourceDescriptor, name));
        // Make sure that each null size resulted in a null base resource ptr
        for (unsigned int i=0; i<ResourceDescriptor::GetNumberOfBaseResourceTypes(); ++i)
        {
            if ( (0 == resourceDescriptor.GetBaseResourceDescriptor(i).GetSize())
                 && checkedResource.GetBaseResource(i))
            {
                rwMESSAGE(("0 size specified for %s base resource, and non-zero ptr returned by allocator",
                          Resource::GetBaseResourceTypeName(i)));
                rwMESSAGE(("*** this message will be replaced with an rwASSERT in a future release ***"));
            }
        }
        rwRETURN(checkedResource);
#else
        rwRETURN(DoAllocate(resourceDescriptor, name));
#endif
    }

    // ICoreAllocator implementation
    virtual void *Alloc(size_t size, const char *name, unsigned int RWUNUSED(flags))
    {
        return AllocateMemoryResource(size, 1, name);
    }

    // ICoreAllocator implementation
    virtual void *Alloc(size_t size, const char *name, unsigned int RWUNUSED(flags),
        unsigned int align, unsigned int RWUNUSED_RELEASE(alignOffset) = 0)
    {
        rwASSERTM(alignOffset == 0, ("IResourceAllocator alignment offset not supported, while trying to allocate: '%s'", name));
        return AllocateMemoryResource(size, align, name);
    }

    // ICoreAllocator implementation
    virtual void Free(void *block, size_t RWUNUSED(size))
    {
        return Free(block);
    }

    /**
     Free a Resource.
     This function calls the virtual member function DoFree.
     This function will not fail if passed a default constructed Resource.
    */
    void Free(const Resource& resource)
    {
        rwFUNCTION();
        DoFree(resource);
        rwRETURNVOID();
    }

    /**
     Free a bare ptr
     This function calls the virtual member function DoFree.
     This function will not fail if passed a default allocated Resource.
    */
    void Free(void *ptr)
    {
        rwFUNCTION();
        DoFree(Resource(ptr));
        rwRETURNVOID();
    }


    /**
     Free the disposable memory resources of a Resource.
     This function calls the virtual member function DoFreeDisposable.
     This function will not fail if passed a default allocated Resource.
     \param the resource. This is modified so that the disposable members are set to 0 after
     they have been freed
     */
    void FreeDisposable(Resource& resource)
    {
        rwFUNCTION();
        DoFreeDisposable(resource);
        rwRETURNVOID();
    }


    /**
     Allocate a block of memory based on main memory size and alignment passed in
     \return the newly allocated MemoryResource
    */
    MemoryResource AllocateMemoryResource(size_t size, uint32_t alignment, const char8_t *name = NULL)
    {
        rwFUNCTION();
        rwRETURN(DoAllocate(ResourceDescriptor(size, alignment), name).GetMemoryResource());
    }

    /**
    Free a block of memory memory.
    This function will not fail if passed a null MemoryResource.
    */
    void FreeMemoryResource(MemoryResource memoryResource)
    {
        rwFUNCTION();
        Resource resource;
        resource.GetMemoryResource() = memoryResource;
        rwRETURN(DoFree(resource));
    }


    /**
     Allocate a block of disposable memory based on the size and alignment passed in
     \return the newly allocated DisposableResource
    */
    DisposableResource AllocateDisposable(uint32_t RW_W64 size, uint32_t alignment, const char8_t *name = NULL)
    {
        rwFUNCTION();
        ResourceDescriptor descriptor;
        descriptor.GetDisposableResourceDescriptor() = DisposableResourceDescriptor(size, alignment);
        rwRETURN(DoAllocate(descriptor, name).GetDisposableResource());
    }

    /**
    Free a block of disposable memory.
    This function will not fail if passed a null DisposableResource.
    */
    void FreeDisposableResource(DisposableResource disposableResource)
    {
        rwFUNCTION();
        Resource resource;
        resource.GetDisposableResource() = disposableResource;
        rwRETURN(DoFree(resource));
    }

#if defined(rwTARGET_XBOX2)
    /**
    Allocate a block of physically contiguous memory based on the size and alignment passed in.
    (only available on the Xbox 360, or when building a conditioning pipeline targetting the Xbox
    360.
    */
    Xbox2PhysicalResource Xbox2AllocatePhysicalResource(uint32_t size, uint32_t alignment,
                                                        const char8_t *name = NULL)
    {
        rwFUNCTION();
        ResourceDescriptor descriptor;
        descriptor.Xbox2GetPhysicalResourceDescriptor() = Xbox2PhysicalResourceDescriptor(size, alignment);
        rwRETURN(DoAllocate(descriptor, name).Xbox2GetPhysicalResource());
    }

    /**
    Free a block of physically contiguous memory.
    This function will not fail if passed a null XboxPhysicalResource.
    */
    void Xbox2FreePhysicalResource(Xbox2PhysicalResource physicalResource)
    {
        rwFUNCTION();
        Resource resource;
        resource.Xbox2GetPhysicalResource() = physicalResource;
        rwRETURN(DoFree(resource));
    }
#endif // defined(rwTARGET_XBOX2)

#if defined(rwTARGET_PS3)
#if !defined(RWCROSS)
    /**
    The location of PS3 GraphicsSystem and GraphicsLocal memory can only be determined after the library
    has been initialized.  Only exists on PS3 in native mode.
    */
    virtual void PS3GraphicsSystemMemoryInitialize(void *, uint32_t) { };
    virtual void PS3GraphicsLocalMemoryInitialize(void *, uint32_t) { };
#endif  // !defined(RWCROSS)
#endif  // defined(rwTARGET_PS3)

protected:
    /**
     Child interface, concrete implementations of IResourceAllocator should implement these
     functions rather than the public non virtual members.
     Implementations of DoAllocate will always be provided with a resource descriptor specifying
     at least one non-zero base resource.
     Implementations of DoAllocate must not return a non-zero pointer for a base resource with
     a zero size.
     \return the newly allocated Resource, returns a default allocated Resource on failure.
    */
    virtual Resource DoAllocate(const ResourceDescriptor &resourceDescriptor,
                                const char8_t *name = NULL) = 0;

    /**
     Child interface, concrete implementations of IResourceAllocator should implement these
     functions rather than the public non virtual members.
     Free a Resource.
     This function will not fail if passed a default allocated Resource.
    */
    virtual void DoFree(const Resource& resource) = 0;

    /**
     Child interface, concrete implementations of IResourceAllocator should implement these
     functions rather than the public non virtual members.
     Free the disposable a Resource.
     This function will not fail if passed a default allocated Resource.
     This function should set the disposable memory resource members to 0 after they have been freed.
    */
    virtual void DoFreeDisposable(Resource& resource)
    {
        rwFUNCTION();
        Resource newResource;
        for (uint32_t i = 0; i < ResourceDescriptor::GetNumberOfBaseResourceTypes(); ++i)
        {
            if (Resource::IsBaseResourceTypeDisposable(i))
            {
                newResource.GetBaseResource(i) = resource.GetBaseResource(i);
                resource.GetBaseResource(i) = (void*)0;
            }
        }
        DoFree(newResource);
        rwRETURNVOID();
    }
};


// Inline Functions

} //namespace rw

#endif // RW_RESOURCEALLOCATOR_HPP
