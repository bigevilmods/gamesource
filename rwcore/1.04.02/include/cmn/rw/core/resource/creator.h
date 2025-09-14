// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Creator template definitions

#ifndef RW_CREATOR_HPP
#define RW_CREATOR_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>
#include <rw/core/debug/exception.h>
#include <rw/core/resource/baseresource.h>
#include <rw/core/resource/resource.h>
#include <rw/core/resource/resourcetarget.h>
#include <rw/core/resource/resourceallocatorregistry.h>

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
//                                                AllocatedResource Definition
// ***********************************************************************************************************

/**
 Implements a AllocatedResource pointer object.

 ValueType has no restrictions.
*/
template <typename ValueType>
class AllocatedResource
{
public:
    /**
     Construct a null AllocatedResource pointer.
    */
    AllocatedResource()
      : m_ptr(0),
        m_resource(),
        m_resourceDescriptor(),
        m_allocator(&GetDefaultAllocator())
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Construct a AllocatedResource pointer with a pointer.
     This function is guaranteed to not throw.
     \param ptr A pointer (must be convertable to a pointer to type ValueType)
     \param resource The Resource containing the object
     \param descriptor The ResourceDescriptor describing the memory requirements of the object
    */
    template <typename OtherValueType>
    AllocatedResource(OtherValueType* ptr, Resource& resource, const ResourceDescriptor& descriptor)
      : m_ptr(ptr),
        m_resource(resource),
        m_resourceDescriptor(descriptor),
        m_allocator(&GetDefaultAllocator())
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Construct a AllocatedResource pointer with a pointer.
     This function is guaranteed to not throw.
     \param ptr A pointer (must be convertable to a pointer to type ValueType)
     \param resource The Resource containing the object
     \param descriptor The ResourceDescriptor describing the memory requirements of the object
     \param allocator The allocator used to allocate the resource
    */
    template <typename OtherValueType, typename AllocatorType>
    AllocatedResource(OtherValueType* ptr,
                      Resource& resource, const ResourceDescriptor& descriptor, AllocatorType& allocator)
      : m_ptr(ptr),
        m_resource(resource),
        m_resourceDescriptor(descriptor),
        m_allocator(&allocator)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Construct a AllocatedResource pointer with from another.
     \param other A AllocatedResource pointer
    */
    AllocatedResource(const AllocatedResource& other)
      : m_ptr(other.m_ptr),
        m_resource(other.m_resource),
        m_resourceDescriptor(other.m_resourceDescriptor),
        m_allocator(other.m_allocator)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Construct a AllocatedResource pointer with from another.
     \param other A AllocatedResource pointer to type OtherValueType
      (must be convertable to a pointer to type ValueType)
    */
    template <typename OtherValueType>
    AllocatedResource(const AllocatedResource<OtherValueType>& other)
      : m_ptr(other.m_ptr),
        m_resource(other.m_resource),
        m_resourceDescriptor(other.m_resourceDescriptor),
        m_allocator(other.m_allocator)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Destruct a AllocatedResource pointer.
    */
    ~AllocatedResource()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Assign another AllocatedResource pointer to this one.
     This function is guaranteed to not throw.
     \param other A AllocatedResource pointer.
    */
    AllocatedResource&
    operator=(const AllocatedResource& other)
    {
        rwFUNCTION();
        m_ptr                = other.m_ptr;
        m_resource           = other.m_resource;
        m_resourceDescriptor = other.m_resourceDescriptor;
        m_allocator          = other.m_allocator;
        rwRETURN(*this);
    }



    /**
     Assign another AllocatedResource pointer to this one.
     This function is guaranteed to not throw.
     \param other A AllocatedResource pointer to type OtherValueType
      (must be convertable to a pointer to type ValueType)
    */
    template <typename OtherValueType>
    AllocatedResource&
    operator=(const AllocatedResource<OtherValueType>& other)
    {
        rwFUNCTION();
        m_ptr                = other.m_ptr;
        m_resource           = other.m_resource;
        m_resourceDescriptor = other.m_resourceDescriptor;
        m_allocator          = other.m_allocator;
        rwRETURN(*this);
    }


    /**
     Dereference a AllocatedResource pointer.
     The AllocatedResource pointer must point to a valid object.
     \return a reference to the object pointed to by the AllocatedResource pointer.
    */
    const ValueType& operator*() const
    {
        rwFUNCTION();
        rwENFORCE(m_ptr, ("AllocatedResource::operator* called on null AllocatedResource."));
        rwRETURN(*m_ptr);
    }

    /**
     Dereference a AllocatedResource pointer.
     The AllocatedResource pointer must point to a valid object.
     \return A reference to the object pointed to by the AllocatedResource pointer.
    */
    ValueType& operator*()
    {
        rwFUNCTION();
        rwENFORCE(m_ptr, ("AllocatedResource::operator* called on null AllocatedResource."));
        rwRETURN(*m_ptr);
    }

    /**
     Dereference a AllocatedResource pointer.
     The AllocatedResource pointer must point to a valid object.
     \return A bare pointer to the object pointed to by the AllocatedResource pointer.
    */
    const ValueType* operator->() const
    {
        rwFUNCTION();
        rwENFORCE(m_ptr, ("AllocatedResource::operator-> called on null AllocatedResource."));
        rwRETURN(m_ptr);
    }

    /**
     Dereference a AllocatedResource pointer.
     \return The bare pointer
    */
    ValueType* operator->()
    {
        rwFUNCTION();
        rwENFORCE(m_ptr, ("AllocatedResource::operator-> called on null AllocatedResource."));
        rwRETURN(m_ptr);
    }

    /**
     \return the bare pointer
    */
    ValueType* Get() const
    {
        rwFUNCTION();
        rwRETURN(m_ptr);
    }

    /**
     \return the bare pointer
     */
    operator ValueType*() const
    {
        rwFUNCTION();
        rwRETURN(m_ptr);
    }

    /**
     \return the bare pointer
    */
    operator ValueType*()
    {
        rwFUNCTION();
        rwRETURN(m_ptr);
    }

    /**
     \return true if the pointer is null.
    */
    RwBool operator!() const
    {
        rwFUNCTION();
        rwRETURN(static_cast<RwBool>(m_ptr == 0));
    }

    /**
     Get the resource for this AllocatedResource.
     The AllocatedResource pointer must point to a valid object.
     \return The resource descriptor
    */
    const Resource& GetResource() const
    {
        rwFUNCTION();
        rwRETURN(m_resource);
    }


    /**
     Get the resource descriptor for this AllocatedResource.
     The AllocatedResource pointer must point to a valid object.
     \return The resource descriptor
    */
    const ResourceDescriptor& GetResourceDescriptor() const
    {
        rwFUNCTION();
        rwRETURN(m_resourceDescriptor);
    }


    /**
     Get the resource descriptor for this AllocatedResource.
     The AllocatedResource pointer must point to a valid object.
     \return The resource descriptor
    */
    IResourceAllocator& GetAllocator() const
    {
        rwFUNCTION();
        rwENFORCE(m_allocator, ("AllocatedResource::GetAllocator allocator was null."));
        rwRETURN(*m_allocator);
    }


private:
    ValueType          *m_ptr;
    Resource            m_resource;
    ResourceDescriptor  m_resourceDescriptor;
    IResourceAllocator *m_allocator;
};


// ***********************************************************************************************************
//                                                Type Traits Definition
// ***********************************************************************************************************

template <typename ValueType>
struct TypeTraits
{
    static const char8_t * GetName()
    {
        rwFUNCTION();
        rwRETURN("Creator<UNKNOWN TYPE>::Allocate()");
    }
};

#define DEFINE_TYPETRAIT_NAME(TYPENAME)               \
inline template<>                                     \
const char8_t * TypeTraits<TYPENAME>::GetName()       \
{                                                     \
    rwFUNCTION();                                     \
    rwRETURN( "Creator<" #TYPENAME ">::Allocate()" ); \
}



// ***********************************************************************************************************
//                                                Creator Definition
// ***********************************************************************************************************

#ifdef _MSC_VER
// Disables 'assignment operator could not be generated' warning
#pragma warning(push)
#pragma warning(disable:4512)
#endif

/**
 A factory to create an instance of ValueType using the default allocator.
 ValueType must implement GetResourceDescriptor() and Initialize(resource) which returns a resource descriptor
 and initializes the class being instantiated respectively.
    */
template <typename ValueType>
class Creator
{
public:
    /**
     constructor
    */
    Creator(IResourceAllocator& allocator = GetDefaultAllocator())
      : m_allocator(allocator)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    destructor
    */
    ~Creator()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Create an instance of ValueType.
     \return a AllocatedResource pointing to the new ValueType instance.
    */
    AllocatedResource<ValueType> New()
    {
        rwFUNCTION();
        ResourceDescriptor resourceDescriptor = ValueType::GetResourceDescriptor();
        Resource resource = m_allocator.Allocate(resourceDescriptor,
               RW_RESOURCE_ALLOCATION_DESC(TypeTraits<ValueType>::GetName()));
        AllocatedResource<ValueType> object(ValueType::Initialize(resource),
                                            resource, resourceDescriptor, m_allocator);
        if (!object)
        {
            m_allocator.Free(resource);
        }
        rwRETURN(object);
    }

    /**
     Create an instance of ValueType.
     \return a AllocatedResource pointing to the new ValueType instance.
    */
    template <typename ARG1_TYPE>
    AllocatedResource<ValueType> New(const ARG1_TYPE& arg1)
    {
        rwFUNCTION();
        ResourceDescriptor resourceDescriptor = ValueType::GetResourceDescriptor(arg1);
        Resource resource = m_allocator.Allocate(resourceDescriptor,
            RW_RESOURCE_ALLOCATION_DESC(TypeTraits<ValueType>::GetName()));
        AllocatedResource<ValueType> object(ValueType::Initialize(resource, arg1),
                                            resource, resourceDescriptor, m_allocator);
        if (!object)
        {
            m_allocator.Free(resource);
        }
        rwRETURN(object);
    }

    /**
     Create an instance of ValueType.
     \return a AllocatedResource pointing to the new ValueType instance.
    */
    template <typename ARG1_TYPE, typename ARG2_TYPE>
    AllocatedResource<ValueType> New(const ARG1_TYPE& arg1,
                                     const ARG2_TYPE& arg2)
    {
        rwFUNCTION();
        ResourceDescriptor resourceDescriptor = ValueType::GetResourceDescriptor(arg1, arg2);
        Resource resource = m_allocator.Allocate(resourceDescriptor,
            RW_RESOURCE_ALLOCATION_DESC(TypeTraits<ValueType>::GetName()));
        AllocatedResource<ValueType> object(ValueType::Initialize(resource, arg1, arg2),
                                            resource, resourceDescriptor, m_allocator);
        if (!object)
        {
            m_allocator.Free(resource);
        }
        rwRETURN(object);
    }

    /**
     Create an instance of ValueType.
     \return a AllocatedResource pointing to the new ValueType instance.
    */
    template <typename ARG1_TYPE, typename ARG2_TYPE, typename ARG3_TYPE>
    AllocatedResource<ValueType> New(const ARG1_TYPE& arg1,
                                     const ARG2_TYPE& arg2,
                                     const ARG3_TYPE& arg3)
    {
        rwFUNCTION();
        ResourceDescriptor resourceDescriptor = ValueType::GetResourceDescriptor(arg1, arg2, arg3);
        Resource resource = m_allocator.Allocate(resourceDescriptor,
            RW_RESOURCE_ALLOCATION_DESC(TypeTraits<ValueType>::GetName()));
        AllocatedResource<ValueType> object(ValueType::Initialize(resource, arg1, arg2, arg3),
                                            resource, resourceDescriptor, m_allocator);
        if (!object)
        {
            m_allocator.Free(resource);
        }
        rwRETURN(object);
    }

    /**
     Create an instance of ValueType.
     \return a AllocatedResource pointing to the new ValueType instance.
    */
    template <typename ARG1_TYPE,
              typename ARG2_TYPE,
              typename ARG3_TYPE,
              typename ARG4_TYPE>
    AllocatedResource<ValueType> New(const ARG1_TYPE& arg1,
                                     const ARG2_TYPE& arg2,
                                     const ARG3_TYPE& arg3,
                                     const ARG4_TYPE& arg4)
    {
        rwFUNCTION();
        ResourceDescriptor resourceDescriptor =
            ValueType::GetResourceDescriptor(arg1, arg2, arg3, arg4);
        Resource resource = m_allocator.Allocate(resourceDescriptor,
            RW_RESOURCE_ALLOCATION_DESC(TypeTraits<ValueType>::GetName()));
        AllocatedResource<ValueType> object(ValueType::Initialize(resource, arg1, arg2, arg3, arg4),
                                            resource, resourceDescriptor, m_allocator);
        if (!object)
        {
            m_allocator.Free(resource);
        }
        rwRETURN(object);
    }

    /**
     Create an instance of ValueType.
     \return a AllocatedResource pointing to the new ValueType instance.
    */
    template <typename ARG1_TYPE,
              typename ARG2_TYPE,
              typename ARG3_TYPE,
              typename ARG4_TYPE,
              typename ARG5_TYPE>
    AllocatedResource<ValueType> New(const ARG1_TYPE& arg1,
                                     const ARG2_TYPE& arg2,
                                     const ARG3_TYPE& arg3,
                                     const ARG4_TYPE& arg4,
                                     const ARG5_TYPE& arg5)
    {
        rwFUNCTION();
        ResourceDescriptor resourceDescriptor =
            ValueType::GetResourceDescriptor(arg1, arg2, arg3, arg4, arg5);
        Resource resource = m_allocator.Allocate(resourceDescriptor,
            RW_RESOURCE_ALLOCATION_DESC(TypeTraits<ValueType>::GetName()));
        AllocatedResource<ValueType> object(ValueType::Initialize(resource, arg1, arg2, arg3, arg4, arg5),
                                            resource, resourceDescriptor, m_allocator);
        if (!object)
        {
            m_allocator.Free(resource);
        }
        rwRETURN(object);
    }

    /**
     Create an instance of ValueType.
     \return a AllocatedResource pointing to the new ValueType instance.
    */
    template <typename ARG1_TYPE,
              typename ARG2_TYPE,
              typename ARG3_TYPE,
              typename ARG4_TYPE,
              typename ARG5_TYPE,
              typename ARG6_TYPE>
    AllocatedResource<ValueType> New(const ARG1_TYPE& arg1,
                                     const ARG2_TYPE& arg2,
                                     const ARG3_TYPE& arg3,
                                     const ARG4_TYPE& arg4,
                                     const ARG5_TYPE& arg5,
                                     const ARG6_TYPE& arg6)
    {
        rwFUNCTION();
        ResourceDescriptor resourceDescriptor =
            ValueType::GetResourceDescriptor(arg1, arg2, arg3, arg4, arg5, arg6);
        Resource resource = m_allocator.Allocate(resourceDescriptor,
            RW_RESOURCE_ALLOCATION_DESC(TypeTraits<ValueType>::GetName()));
        AllocatedResource<ValueType> object(ValueType::Initialize(resource,
                                                                  arg1, arg2, arg3, arg4, arg5, arg6),
                                            resource, resourceDescriptor, m_allocator);
        if (!object)
        {
            m_allocator.Free(resource);
        }
        rwRETURN(object);
    }

    /**
     Create an instance of ValueType.
     \return a AllocatedResource pointing to the new ValueType instance.
    */
    template <typename ARG1_TYPE,
              typename ARG2_TYPE,
              typename ARG3_TYPE,
              typename ARG4_TYPE,
              typename ARG5_TYPE,
              typename ARG6_TYPE,
              typename ARG7_TYPE>
    AllocatedResource<ValueType> New(const ARG1_TYPE& arg1,
                                     const ARG2_TYPE& arg2,
                                     const ARG3_TYPE& arg3,
                                     const ARG4_TYPE& arg4,
                                     const ARG5_TYPE& arg5,
                                     const ARG6_TYPE& arg6,
                                     const ARG7_TYPE& arg7)
    {
        rwFUNCTION();
        ResourceDescriptor resourceDescriptor =
            ValueType::GetResourceDescriptor(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
        Resource resource = m_allocator.Allocate(resourceDescriptor,
            RW_RESOURCE_ALLOCATION_DESC(TypeTraits<ValueType>::GetName()));
        AllocatedResource<ValueType> object(ValueType::Initialize(resource,
                                                                  arg1, arg2, arg3, arg4, arg5, arg6, arg7),
                                            resource, resourceDescriptor, m_allocator);
        if (!object)
        {
            m_allocator.Free(resource);
        }
        rwRETURN(object);
    }

private:
    IResourceAllocator& m_allocator;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Inline Functions

}

#endif // RW_CREATOR_HPP
