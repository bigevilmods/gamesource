// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Xbox2 specific classes for the resource descriptors and resources.

#ifndef RW_RESOURCEXBOX2_HPP
#define RW_RESOURCEXBOX2_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>
#include <rw/core/debug/exception.h>
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

/**
 \enum BASERESOURCE_MAINMEMORY
 \enum BASERESOURCE_DISPOSABLE
 \enum BASERESOURCE_PHYSICAL
 \enum BASERESOURCE_UNINITIALIZED
 \enum BASERESOURCE_DISPOSABLE_UNINITIALIZED
 \enum BASERESOURCE_NUMBEROFBASERESOURCETYPES
 */
enum {BASERESOURCE_MAINMEMORY = 0,
      BASERESOURCE_DISPOSABLE,
      BASERESOURCE_PHYSICAL,
      BASERESOURCE_UNINITIALIZED,
      BASERESOURCE_DISPOSABLE_UNINITIALIZED,
      BASERESOURCE_NUMBEROFBASERESOURCETYPES};
const RwChar* const BASERESOURCE_BASERESOURCETYPENAMES[] = {"main memory",
                                                            "disposable",
                                                            "physical",
                                                            "uninitialized",
                                                            "disposable uninitialized"};

typedef BaseResourceDescriptorType<BASERESOURCE_MAINMEMORY>  MemoryResourceDescriptor;
typedef BaseResourceDescriptorType<BASERESOURCE_DISPOSABLE>  DisposableResourceDescriptor;
typedef BaseResourceDescriptorType<BASERESOURCE_PHYSICAL>    Xbox2PhysicalResourceDescriptor;
typedef BaseResourceDescriptorType<BASERESOURCE_UNINITIALIZED> UninitializedResourceDescriptor;
typedef BaseResourceDescriptorType<BASERESOURCE_DISPOSABLE_UNINITIALIZED>
                                                             DisposableUninitializedResourceDescriptor;

typedef BaseResource MemoryResource;
typedef BaseResource DisposableResource;
typedef BaseResource Xbox2PhysicalResource;
typedef BaseResource UninitializedResource;
typedef BaseResource DisposableUninitializedResource;

// ***********************************************************************************************************
// Global Variables

// ***********************************************************************************************************
// External Functions Prototypes

// ***********************************************************************************************************
// Structs + Unions + Classes

/**
 class ResourceDescriptor
 */
class ResourceDescriptor : public BaseResourceDescriptors<BASERESOURCE_NUMBEROFBASERESOURCETYPES>
{
public:
    /**
     Constructor taking a single main memory resource descriptors.
     */
    ResourceDescriptor(const MemoryResourceDescriptor& mainMemory = MemoryResourceDescriptor())
    {
        rwFUNCTION();
        GetBaseResourceDescriptor(BASERESOURCE_MAINMEMORY) = mainMemory;
        rwRETURNVOID();
    }

    /**
     Create a Resource Descriptor from a main memory size and alignment
     */
    ResourceDescriptor(uint32_t size, uint32_t alignment)
    {
        rwFUNCTION();
        GetBaseResourceDescriptor(BASERESOURCE_MAINMEMORY) = MemoryResourceDescriptor(size, alignment);
        rwRETURNVOID();
    }

    // default copy constructor, assignment operator and destructors are okay
    // ResourceDescriptor(const ResourceDescriptor& other);
    // ResourceDescriptor operator=(const ResourceDescriptor& other);
    // ~ResourceDescriptor(const ResourceDescriptor& other);


    /**
     \return a const reference to the main memory resource descriptor.
     */
    const MemoryResourceDescriptor& GetMemoryResourceDescriptor() const
    {
        rwFUNCTION();
        rwRETURN(static_cast<const MemoryResourceDescriptor&>
                 (GetBaseResourceDescriptor(BASERESOURCE_MAINMEMORY)));
    }

    /**
     \return a reference to the main memory resource descriptor.
     */
    MemoryResourceDescriptor& GetMemoryResourceDescriptor()
    {
        rwFUNCTION();
        rwRETURN(static_cast<MemoryResourceDescriptor&>
                 (GetBaseResourceDescriptor(BASERESOURCE_MAINMEMORY)));
    }


    /**
     \return a const reference the uninitialized memory resource descriptor.
     */
    const UninitializedResourceDescriptor& GetUninitializedResourceDescriptor() const
    {
        rwFUNCTION();
        rwRETURN(static_cast<const UninitializedResourceDescriptor&>
                 (GetBaseResourceDescriptor(BASERESOURCE_UNINITIALIZED)));
    }

    /**
     \return a reference to the uninitialized memory resource descriptor.
     */
    UninitializedResourceDescriptor& GetUninitializedResourceDescriptor()
    {
        rwFUNCTION();
        rwRETURN(static_cast<UninitializedResourceDescriptor&>
                 (GetBaseResourceDescriptor(BASERESOURCE_UNINITIALIZED)));
    }


    /**
     \return a const reference the disposable memory resource descriptor.
     */
    const DisposableResourceDescriptor& GetDisposableResourceDescriptor() const
    {
        rwFUNCTION();
        rwRETURN(static_cast<const DisposableResourceDescriptor&>
                 (GetBaseResourceDescriptor(BASERESOURCE_DISPOSABLE)));
    }

    /**
     \return a reference to the disposable memory resource descriptor.
     */
    DisposableResourceDescriptor& GetDisposableResourceDescriptor()
    {
        rwFUNCTION();
        rwRETURN(static_cast<DisposableResourceDescriptor&>
                 (GetBaseResourceDescriptor(BASERESOURCE_DISPOSABLE)));
    }


    /**
     \return a const reference the disposable uninitialized memory resource descriptor.
     */
    const DisposableUninitializedResourceDescriptor& GetDisposableUninitializedResourceDescriptor() const
    {
        rwFUNCTION();
        rwRETURN(static_cast<const DisposableUninitializedResourceDescriptor&>
                 (GetBaseResourceDescriptor(BASERESOURCE_DISPOSABLE_UNINITIALIZED)));
    }

    /**
     \return a reference to the disposable uninitialized memory resource descriptor.
     */
    DisposableUninitializedResourceDescriptor& GetDisposableUninitializedResourceDescriptor()
    {
        rwFUNCTION();
        rwRETURN(static_cast<DisposableUninitializedResourceDescriptor&>
                 (GetBaseResourceDescriptor(BASERESOURCE_DISPOSABLE_UNINITIALIZED)));
    }


    /**
     \return a const reference the physically contiguous memory resource descriptor.
     */
    const Xbox2PhysicalResourceDescriptor& Xbox2GetPhysicalResourceDescriptor() const
    {
        rwFUNCTION();
        rwRETURN(static_cast<const Xbox2PhysicalResourceDescriptor&>
                 (GetBaseResourceDescriptor(BASERESOURCE_PHYSICAL)));
    }

    /**
     \return a reference to the physically contiguous memory resource descriptor.
     */
    Xbox2PhysicalResourceDescriptor& Xbox2GetPhysicalResourceDescriptor()
    {
        rwFUNCTION();
        rwRETURN(static_cast<Xbox2PhysicalResourceDescriptor&>
                 (GetBaseResourceDescriptor(BASERESOURCE_PHYSICAL)));
    }

    /**
     \deprecated This is deprecated functionality

     \return a const reference the physically contiguous memory resource descriptor.
     */
    Xbox2PhysicalResourceDescriptor GetPhysicalResourceDescriptor() const
    {
        rwFUNCTION();
        rwDEPRECATEDM("Use Resource::Xbox2GetPhysicalResourceDescriptor()");
        rwRETURN(Xbox2GetPhysicalResourceDescriptor());
    }

    /**
     \deprecated This is deprecated functionality

     \return a reference to the physically contiguous memory resource descriptor.
     */
    Xbox2PhysicalResourceDescriptor& GetPhysicalResourceDescriptor()
    {
        rwFUNCTION();
        rwDEPRECATEDM("Use Resource::Xbox2GetPhysicalResourceDescriptor()");
        rwRETURN(Xbox2GetPhysicalResourceDescriptor());
    }


    /**
     \deprecated This is deprecated functionality
 Use GetMemoryResourceDescriptor().GetSize()
     \return the memory size requirement from the main memory resource descriptor
     */
    uint32_t GetSize() const
    {
        rwFUNCTION();
        rwRETURN(GetMemoryResourceDescriptor().GetSize());
    }

    /**
     \deprecated This is deprecated functionality

     \return the memory size requirement from the main memory resource descriptor
     */
    uint32_t GetAlignment() const
    {
        rwFUNCTION();
        rwRETURN(GetMemoryResourceDescriptor().GetAlignment());
    }
};

/**
 operator+
 */
inline ResourceDescriptor operator+(const ResourceDescriptor& lhs, const ResourceDescriptor& rhs)
{
    rwFUNCTION();
    ResourceDescriptor sum = lhs;
    sum += rhs;
    rwRETURN(sum);
}

/**
 operator-
 */
inline ResourceDescriptor operator-(const ResourceDescriptor& lhs, const ResourceDescriptor& rhs)
{
    rwFUNCTION();
    ResourceDescriptor diff = lhs;
    diff -= rhs;
    rwRETURN(diff);
}



/**
 class Resource
 */
class Resource : public BaseResources<BASERESOURCE_NUMBEROFBASERESOURCETYPES>
{
public:
    /**
     Constructor taking a single main memory resource.
     */
    Resource(const MemoryResource& mainMemory = MemoryResource())
    {
        rwFUNCTION();
        GetBaseResource(BASERESOURCE_MAINMEMORY) = mainMemory;
        rwRETURNVOID();
    }

    // default copy constructor, assignment operator and destructors are okay
    // Resource(const Resource& other);
    // Resource operator=(const Resource& other);
    // ~Resource();

    /**
     \return the main memory resource
     */
    MemoryResource GetMemoryResource() const
    {
        rwFUNCTION();
        rwRETURN(GetBaseResource(BASERESOURCE_MAINMEMORY));
    }

    /**
     \return a reference the main memory resource
     */
    MemoryResource& GetMemoryResource()
    {
        rwFUNCTION();
        rwRETURN(GetBaseResource(BASERESOURCE_MAINMEMORY));
    }


    /**
     \return the uninitialized memory resource
     */
    UninitializedResource GetUninitializedResource() const
    {
        rwFUNCTION();
        rwRETURN(GetBaseResource(BASERESOURCE_UNINITIALIZED));
    }

    /**
     \return a reference to the uninitialized memory resource
     */
    UninitializedResource& GetUninitializedResource()
    {
        rwFUNCTION();
        rwRETURN(GetBaseResource(BASERESOURCE_UNINITIALIZED));
    }


    /**
     \return the disposable memory resource
     */
    DisposableResource GetDisposableResource() const
    {
        rwFUNCTION();
        rwRETURN(GetBaseResource(BASERESOURCE_DISPOSABLE));
    }

    /**
     \return a reference to the disposable memory resource
     */
    DisposableResource& GetDisposableResource()
    {
        rwFUNCTION();
        rwRETURN(GetBaseResource(BASERESOURCE_DISPOSABLE));
    }


    /**
     \return the disposable uninitialized memory resource
     */
    DisposableUninitializedResource GetDisposableUninitializedResource() const
    {
        rwFUNCTION();
        rwRETURN(GetBaseResource(BASERESOURCE_DISPOSABLE_UNINITIALIZED));
    }

    /**
     \return a reference to the disposable uninitialized memory resource
     */
    DisposableUninitializedResource& GetDisposableUninitializedResource()
    {
        rwFUNCTION();
        rwRETURN(GetBaseResource(BASERESOURCE_DISPOSABLE_UNINITIALIZED));
    }


    /**
     \return the physically contiguous memory resource
     */
    Xbox2PhysicalResource Xbox2GetPhysicalResource() const
    {
        rwFUNCTION();
        rwRETURN(GetBaseResource(BASERESOURCE_PHYSICAL));
    }

    /**
     \return a reference to the physically contiguous memory resource
     */
    Xbox2PhysicalResource& Xbox2GetPhysicalResource()
    {
        rwFUNCTION();
        rwRETURN(GetBaseResource(BASERESOURCE_PHYSICAL));
    }

    /**
     \deprecated This is deprecated functionality

     \return the physically contiguous memory resource
     */
    Xbox2PhysicalResource GetPhysicalMemoryResource() const
    {
        rwFUNCTION();
        rwDEPRECATEDM("Use Resource::Xbox2GetPhysicalResource()");
        rwRETURN(Xbox2GetPhysicalResource());
    }

    /**
     \deprecated This is deprecated functionality

     \return a reference to the physically contiguous memory resource
     */
    Xbox2PhysicalResource& GetPhysicalMemoryResource()
    {
        rwFUNCTION();
        rwDEPRECATEDM("Use Resource::Xbox2GetPhysicalResource()");
        rwRETURN(GetBaseResource(BASERESOURCE_PHYSICAL));
    }



    /**
     \return the name of a specified base resource
     */
    static const RwChar* GetBaseResourceTypeName(uint32_t index)
    {
        rwFUNCTION();
        rwASSERT(index < BASERESOURCE_NUMBEROFBASERESOURCETYPES);
        rwRETURN(BASERESOURCE_BASERESOURCETYPENAMES[index]);
    }


    /**
     \return true if the BaseResource at a given index does not need to be initialized in an arena
     */
    static bool IsBaseResourceTypeUninitialized(uint32_t index)
    {
        rwFUNCTION();
        rwRETURN((index == BASERESOURCE_UNINITIALIZED) || (index == BASERESOURCE_DISPOSABLE_UNINITIALIZED));
    }


    /**
     \return true if the BaseResource at a given index can be disposed after arena fixup
     */
    static bool IsBaseResourceTypeDisposable(uint32_t index)
    {
        rwFUNCTION();
        rwRETURN((index == BASERESOURCE_DISPOSABLE) || (index == BASERESOURCE_DISPOSABLE_UNINITIALIZED));
    }


    /**
     \return true if the BaseResource refers to physically contiguous memory
     */
    static bool IsBaseResourceTypePhysical(uint32_t index)
    {
        rwFUNCTION();
        rwRETURN(index == BASERESOURCE_PHYSICAL);
    }
};

/**
 \return the result off adding a ResourceDescriptor to a Resource.
 */
inline Resource operator+(const Resource& resource, const ResourceDescriptor& descriptor)
{
    rwFUNCTION();
    Resource sum = resource;
    sum += descriptor;
    rwRETURN(sum);
}

/**
 Subtract one Resource from another.
 \return *this - rhs.
 The size will be the offset in bytes between respective Resources.
 The alignment values will be 1.
 */
inline ResourceDescriptor operator-(const Resource& lhs, const Resource& rhs)
{
    rwFUNCTION();
    ResourceDescriptor diff;
    for (uint32_t i = 0; i < BASERESOURCE_NUMBEROFBASERESOURCETYPES; ++i)
    {
        uint32_t sizeDiff = MemCalculateOffset(lhs.GetBaseResource(i), rhs.GetBaseResource(i));
        diff.GetBaseResourceDescriptor(i) = BaseResourceDescriptor(sizeDiff, 1);
    }
    rwRETURN(diff);
}


// Inline Functions

} //namespace rw

#endif // RW_RESOURCEXBOX2_HPP
