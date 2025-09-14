// (c) Electronic Arts. All Rights Reserved.

/// \file
/// A reference counted ptr for memory management. The object pointed to is deleted when the last
/// SharedResource pointer pointing to that object is destroyed.

#ifndef RW_SHAREDRESOURCE_HPP
#define RW_SHAREDRESOURCE_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>
#include <rw/core/debug/exception.h>
#include <rw/core/resource/baseresource.h>
#include <rw/core/resource/resource.h>
#include <rw/core/resource/resourcetarget.h>
#include <rw/core/resource/resourceallocatorregistry.h>
#include <rw/core/resource/creator.h>

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


template <typename ValueType>
class SharedResourceRefCountImpl
{
public:
    /**
    constructor
    Implementation class for the SharedResource pointer reference counter. This class overloads the virtual
    Destruct method to clean up the pointer using the provided Delete function. This class maintains a copy of the
    pointer being held and a delete function so it can correctly dispose of the pointer.
    */
    SharedResourceRefCountImpl()
      : m_count(0),
        m_objectPtr(0),
        m_objectResource(),
        m_objectResourceDescriptor(),
        m_allocator(0)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    ~SharedResourceRefCountImpl()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    Get the resource for the object.
    */
    const Resource& GetObjectResource() const
    {
        rwFUNCTION();
        rwRETURN(m_objectResource);
    }

    /**
    Get the resource for the object.
    */
    Resource& GetObjectResource()
    {
        rwFUNCTION();
        rwRETURN(m_objectResource);
    }

    /**
    Get the resource descriptor for the object.
    */
    const ResourceDescriptor& GetObjectResourceDescriptor() const
    {
        rwFUNCTION();
        rwRETURN(m_objectResourceDescriptor);
    }

    /**
    Get the resource descriptor for the object.
    */
    ResourceDescriptor& GetObjectResourceDescriptor()
    {
        rwFUNCTION();
        rwRETURN(m_objectResourceDescriptor);
    }

    /**
    \return the number of copies of this counter which exist.
    */
    size_t Count() const
    {
        rwFUNCTION();
        rwRETURN(m_count);
    }

    /**
    \return true if count == 1.
    */
    RwBool Unique() const
    {
        rwFUNCTION();
        rwRETURN(static_cast<RwBool>(m_count == 1));
    }

    /**
    Increment the counter.
    */
    void
    Incr()
    {
        rwFUNCTION();
        ++m_count;
        rwRETURNVOID();
    }

    /**
    Decrement the counter.
    If the counter reaches 0, dispose of the object held and the destruct this counter.
    */
    void
    Decr()
    {
        rwFUNCTION();
        --m_count;
        if (m_count == 0)
        {
            Dispose();    // dispose of the object pointed to
            Destruct();   // destruct this reference counter object
        }
        rwRETURNVOID();
    }

    /**
    get the resource descriptor for the object
    */
    static ResourceDescriptor GetResourceDescriptor()
    {
        rwFUNCTION();
        rwRETURN(ResourceDescriptor(sizeof(SharedResourceRefCountImpl<ValueType>), 1));
    }

    /**
    initialize function
    */
    static SharedResourceRefCountImpl* Initialize(const Resource& resource,
                                                  ValueType* ptr,
                                                  const Resource& objectResource,
                                                  const ResourceDescriptor& objectResourceDescriptor,
                                                  IResourceAllocator& allocator)
    {
        rwFUNCTION();
        SharedResourceRefCountImpl* refCountImpl =
            static_cast<SharedResourceRefCountImpl*>(resource.GetMemoryResource());
        refCountImpl->m_count = 1;
        refCountImpl->m_objectPtr = ptr;
        refCountImpl->m_objectResource = objectResource;
        refCountImpl->m_objectResourceDescriptor = objectResourceDescriptor;
        refCountImpl->m_allocator = &allocator;
        rwRETURN(refCountImpl);
    }

    /**
    a no-op function
    */
    void Release()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

private:
    // not implemented
    /**
    constructor
    */
    SharedResourceRefCountImpl(const SharedResourceRefCountImpl& other);
    /**
    constructor
    */
    SharedResourceRefCountImpl& operator=(const SharedResourceRefCountImpl& other);

    /**
    Dispose of the pointer being held.
    */
    void Dispose()
    {
        rwFUNCTION();
        rwENFORCE(m_objectPtr, ("Shared reference counter has null pointer."));
        m_objectPtr->Release();
        m_allocator->Free(m_objectResource);
        rwRETURNVOID();
    }

    /**
    Destruct this reference counter.
    */
    void Destruct()
    {
        rwFUNCTION();
        Release();
        m_allocator->Free(Resource(this));
        rwRETURNVOID();
    }

    size_t             m_count;
    ValueType*          m_objectPtr;
    Resource            m_objectResource;
    ResourceDescriptor  m_objectResourceDescriptor;
    IResourceAllocator* m_allocator;
};

// ***********************************************************************************************************
//                                                SharedResourceRefCount Definition
// ***********************************************************************************************************


template <typename ValueType>
class SharedResourceRefCount
{
private:
    typedef SharedResourceRefCountImpl<ValueType> Impl;

public:
    /**
    constructor with no parameter
    Reference counted SharedResource.
    This SharedResource has functions to copy the reference counter, incrementing the count when it is copied.
    The count is decremented when this SharedResource is destructed.
    */
    SharedResourceRefCount()
      : m_impl(0)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    Construct the reference counter with a pointer to hold and a delete function for disposing of the pointer.
    */
    SharedResourceRefCount(ValueType* ptr,
                           const Resource& resource,
                           const ResourceDescriptor& descriptor,
                           IResourceAllocator& allocator)
      : m_impl(0)
    {
        rwFUNCTION();
        rwENFORCE(ptr, ("Shouldn't construct a shared reference count for null pointer."));
        Resource implResource = allocator.Allocate(Impl::GetResourceDescriptor());
        m_impl = Impl::Initialize(implResource, ptr, resource, descriptor, allocator);
        rwRETURNVOID();
    }

    /**
    Copy a reference counter and increment the count.
    */
    SharedResourceRefCount(const SharedResourceRefCount& other)
      : m_impl(other.m_impl)
    {
        rwFUNCTION();
        if (m_impl)
        {
            m_impl->Incr();
        }
        rwRETURNVOID();
    }

    /**
    Destruct this instance of a count, decrementing the counter.
    This function must not throw an exception.
    */
    ~SharedResourceRefCount()
    {
        rwFUNCTION();
        if (m_impl)
        {
            m_impl->Decr();
        }
        rwRETURNVOID();
    }

    /**
    Copy a reference counter (decrementing the old counter first) and increment the count.
    This function must not throw an exception.
    */
    SharedResourceRefCount& operator=(const SharedResourceRefCount& other)
    {
        rwFUNCTION();
        if (this != &other)
        {
            Impl *temp = other.m_impl;
            if (temp)
            {
                temp->Incr();
            }
            if (m_impl)
            {
                m_impl->Decr();
            }
            m_impl = temp;
        }
        rwRETURN(*this);
    }

    /**
    \return the count size.
    */
    size_t Count() const
    {
        rwFUNCTION();
        if (m_impl)
        {
            rwRETURN(m_impl->Count());
        }
        else
        {
            rwRETURN(0);
        }
    }

    /**
    \return true if count == 1
    */
    RwBool Unique() const
    {
        rwFUNCTION();
        if (m_impl)
        {
            rwRETURN(m_impl->Unique());
        }
        else
        {
            rwRETURN(FALSE);
        }
    }

    /**
    Get the resource descriptor for this object.
    */
    const ResourceDescriptor& GetResourceDescriptor() const
    {
        rwFUNCTION();
        rwENFORCE(m_impl, ("Can't call SharedResource::GetResourceDescriptor on null object."));
        rwRETURN(m_impl->GetObjectResourceDescriptor());
    }

    /**
    Get the resource descriptor for this object.
    */
    ResourceDescriptor& GetResourceDescriptor()
    {
        rwFUNCTION();
        rwENFORCE(m_impl, ("Can't call SharedResource::GetResourceDescriptor on null object."));
        rwRETURN(m_impl->GetObjectResourceDescriptor());
    }

    /**
    Get the resource for this object.
    */
    const Resource& GetResource() const
    {
        rwFUNCTION();
        rwENFORCE(m_impl, ("Can't call SharedResource::GetResource on null object."));
        rwRETURN(m_impl->GetObjectResource());
    }

    /**
    Get the resource for this object.
    */
    Resource& GetResource()
    {
        rwFUNCTION();
        rwENFORCE(m_impl, ("Can't call SharedResource::GetResource on null object."));
        rwRETURN(m_impl->GetObjectResource());
    }

    /**
    Swap this reference counter with another. This function
    must not throw and exception.
    */
    void Swap(SharedResourceRefCount& other)
    {
        rwFUNCTION();
        Impl* temp(m_impl);
        m_impl = other.m_impl;
        other.m_impl = temp;
        rwRETURNVOID();
    }

private:
    Impl *m_impl;
};

// ***********************************************************************************************************
//                                                SharedResource Definition
// ***********************************************************************************************************

/**
 Implements a SharedResource pointer object.

 ValueType has no restrictions.
 */
template <typename ValueType>
class SharedResource
{
public:
    /**
     Construct a null SharedResource pointer.
     */
    SharedResource()
      : m_ptr(0),
        m_count()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Construct a SharedResource pointer with a pointer.
     This function is guaranteed to not throw.
     \param ptr A pointer (must be convertable to a pointer to type ValueType)
     \param resource The Resource containing the object
     \param descriptor The ResourceDescriptor describing the memory requirements of the object
     */
    template <typename OtherValueType>
    SharedResource(OtherValueType* ptr, Resource& resource, const ResourceDescriptor& descriptor)
      : m_ptr(ptr),
        m_count()
    {
        rwFUNCTION();
        if (m_ptr)
        {
            SharedResourceRefCount<ValueType> newCount(m_ptr, resource, descriptor,
                                                       GetDefaultAllocator());
            m_count.Swap(newCount);
        }
        rwRETURNVOID();
    }

    /**
     Construct a SharedResource pointer with a pointer.
     This function is guaranteed to not throw.
     \param ptr A pointer (must be convertable to a pointer to type ValueType)
     \param resource The Resource containing the object
     \param descriptor The ResourceDescriptor describing the memory requirements of the object
     \param allocator The allocator used to allocate the resource

     */
    template <typename OtherValueType>
    SharedResource(OtherValueType* ptr,
                   Resource& resource, const ResourceDescriptor& descriptor, IResourceAllocator& allocator)
      : m_ptr(ptr),
        m_count(ptr, resource, descriptor, allocator)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Construct a SharedResource pointer from an allocated resource
     This function is guaranteed to not throw.
     \param allocatedResource allocated resource (pointer must be convertable to a pointer to type ValueType)

     */
    template <typename OtherValueType>
    SharedResource(const AllocatedResource<OtherValueType>& allocatedResource)
      : m_ptr(allocatedResource.Get()),
        m_count()
    {
        rwFUNCTION();
        if (m_ptr)
        {
            SharedResourceRefCount<ValueType> newCount(m_ptr,
                                                       allocatedResource.GetResource(),
                                                       allocatedResource.GetResourceDescriptor(),
                                                       allocatedResource.GetAllocator());
            m_count.Swap(newCount);
        }
        rwRETURNVOID();
    }

    /**
     Construct a SharedResource pointer with from another.
     \param other A SharedResource pointer

     */
    SharedResource(const SharedResource& other)
      : m_ptr(other.m_ptr),
        m_count(other.m_count)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Destruct a SharedResource pointer.
     */
    ~SharedResource()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Assign another SharedResource pointer to this one.
     This function is guaranteed to not throw.
     \param other A SharedResource pointer.
     */
    SharedResource&
    operator=(const SharedResource& other)
    {
        rwFUNCTION();
        m_ptr = other.m_ptr;
        m_count = other.m_count; // this must be written to correctly handle self assignment
        rwRETURN(*this);
    }


    /**
     Release this reference to a SharedResource pointer.
     */
    void Reset()
    {
        rwFUNCTION();
        SharedResource<ValueType>().Swap(*this);
        rwRETURNVOID();
    }

    /**
     Replace this reference to a pointer with a reference to another pointer.
     \param other A pointer to a SharedResource of type OtherValueType
      (must be convertable to a pointer to type ValueType)
     \param resource The Resource containing the object
     \param descriptor The ResourceDescriptor describing the memory requirements of the object
     */
    template <typename OtherValueType>
    void Reset(OtherValueType* ptr, Resource& resource, const ResourceDescriptor& descriptor)
    {
        rwFUNCTION();
        rwASSERT(!ptr || (ptr != m_ptr));
        SharedResource<ValueType>(ptr, resource, descriptor, GetDefaultAllocator()).Swap(*this);
        rwRETURNVOID();
    }

    /**
     Replace this reference to a pointer with a reference to another pointer.
     \param other A pointer to an object of type OtherValueType
      (must be convertable to a pointer to type ValueType)
     \param resource The Resource containing the object
     \param descriptor The ResourceDescriptor describing the memory requirements of the object
     \param allocator The allocator used to allocate the resource
     */
    template <typename OtherValueType>
    void Reset(OtherValueType* ptr,
               Resource& resource, const ResourceDescriptor& descriptor, IResourceAllocator& allocator)
    {
        rwFUNCTION();
        rwASSERT(!ptr || (ptr != m_ptr));
        SharedResource<ValueType>(ptr, resource, descriptor, allocator).Swap(*this);
        rwRETURNVOID();
    }

    /**
     Replace this reference to a pointer with a reference to another pointer.
     \param other A pointer to an object of type OtherValueType
      (must be convertable to a pointer to type ValueType)
     \param resource The Resource containing the object
     \param descriptor The ResourceDescriptor describing the memory requirements of the object
     \param allocator The allocator used to allocate the resource
     */
    template <typename OtherValueType>
    void Reset(const AllocatedResource<OtherValueType>& allocatedResource)
    {
        rwFUNCTION();
        rwASSERT(!allocatedResource.Get() || (allocatedResource.Get() != m_ptr));
        SharedResource<ValueType>(allocatedResource).Swap(*this);
        rwRETURNVOID();
    }

    /**
     Dereference a SharedResource pointer.
     The SharedResource pointer must point to a valid object.
     \return A reference to the object pointed to by the SharedResource pointer.
     */
    const ValueType& operator*() const
    {
        rwFUNCTION();
        rwASSERT(m_ptr);
        rwRETURN(*m_ptr);
    }

    /**
     Dereference a SharedResource pointer.
     The SharedResource pointer must point to a valid object.
     \return A reference to the object pointed to by the SharedResource pointer.
     */
    ValueType& operator*()
    {
        rwFUNCTION();
        rwASSERT(m_ptr);
        rwRETURN(*m_ptr);
    }

    /**
     Dereference a SharedResource pointer.
     The SharedResource pointer must point to a valid object.
     \return A bare pointer to the object pointed to by the SharedResource pointer.
     */
    const ValueType* operator->() const
    {
        rwFUNCTION();
        rwASSERT(m_ptr);
        rwRETURN(m_ptr);
    }

    /**
     Dereference a SharedResource pointer.
     The object pointer must point to a valid object.
     \return A bare pointer to the object pointed to by the SharedResource pointer.
     */
    ValueType* operator->()
    {
        rwFUNCTION();
        rwASSERT(m_ptr);
        rwRETURN(m_ptr);
    }

    /**
     Get the pointer without affecting the reference counter.
     The object pointed to remains owned by this SharedResource.
     \return A copy of bare pointer
     */
    const ValueType* Get() const
    {
        rwFUNCTION();
        rwRETURN(m_ptr);
    }

    /**
     Get the pointer without affecting the reference counter.
     The object pointed to remains owned by this SharedResource.
     \return A copy of bare pointer
     */
    ValueType* Get()
    {
        rwFUNCTION();
        rwRETURN(m_ptr);
    }

    /**
     Get the resource descriptor for this SharedResource.
     The SharedResource pointer must point to a valid object.
     \return The resource descriptor
     */
    const ResourceDescriptor& GetResourceDescriptor() const
    {
        rwFUNCTION();
        rwRETURN(m_count.GetResourceDescriptor());
    }

    /**
     Get the resource descriptor for this SharedResource.
     The SharedResource pointer must point to a valid object.
     \return The resource descriptor
     */
    ResourceDescriptor& GetResourceDescriptor()
    {
        rwFUNCTION();
        rwRETURN(m_count.GetResourceDescriptor());
    }

    /**
     Get the resource for this SharedResource.
     The SharedResource pointer must point to a valid object.
     \return The resource descriptor
     */
    const Resource& GetResource() const
    {
        rwFUNCTION();
        rwRETURN(m_count.GetResource());
    }
    /**
     Get the resource for this SharedResource.
     The SharedResource pointer must point to a valid object.
     \return The resource descriptor
     */
    Resource& GetResource()
    {
        rwFUNCTION();
        rwRETURN(m_count.GetResource());
    }

    /**
     \return True if the pointer is not null.
     */
    operator RwBool() const
    {
        rwFUNCTION();
        rwRETURN(static_cast<RwBool>(m_ptr != 0));
    }

    /**
     \return True if the pointer is null.
     */
    RwBool operator!() const
    {
        rwFUNCTION();
        rwRETURN(static_cast<RwBool>(m_ptr == 0));
    }

    /**
     \return The copies of this SharedResource pointer which currently exist.
     */
    size_t Count() const
    {
        rwFUNCTION();
        rwRETURN(m_count.Count());
    }

    /**
     \return True if this is the only copy of this SharedResource pointer.
     */
    RwBool Unique() const
    {
        rwFUNCTION();
        rwRETURN(m_count.Unique());
    }

    /**
     Swap this SharedResource pointer for another.
     */
    void Swap(SharedResource<ValueType>& other)
    {
        rwFUNCTION();
        ValueType* tempPtr = m_ptr;
        m_ptr = other.m_ptr;
        other.m_ptr = tempPtr;
        m_count.Swap(other.m_count);
        rwRETURNVOID();
    }

private:

    ValueType                         *m_ptr;
    SharedResourceRefCount<ValueType>  m_count;
};

/**
 Swap this SharedResource pointer for another.
 This function is guaranteed to not throw.
 */
template <typename ValueType>
inline void RwSwap(SharedResource<ValueType>& lhs, SharedResource<ValueType>& rhs)
{
    rwFUNCTION();
    lhs.Swap(rhs);
    rwRETURNVOID();
}

/**
 \return true if two SharedResource pointers point to the same object.
 */
template <typename LhsValueType, typename RhsValueType>
inline RwBool operator==(const SharedResource<LhsValueType>&lhs, const SharedResource<RhsValueType>& rhs)
{
    rwFUNCTION();
    rwRETURN(static_cast<RwBool>(lhs.Get() == rhs.Get()));
}

/**
 \return true if two SharedResource pointers do not point to the same object.
 */
template <typename LhsValueType, typename RhsValueType>
inline RwBool operator!=(const SharedResource<LhsValueType>& lhs, const SharedResource<RhsValueType>& rhs)
{
    rwFUNCTION();
    rwRETURN(static_cast<RwBool>(lhs.Get() != rhs.Get()));
}

/**
 An ordering function so we can place the SharedResource pointers into ordered containers
 \param the two SharedResource pointers (both pointers must not be null)
 \return true if the lhs pointer < rhs pointer
 */
template <typename LhsValueType, typename RhsValueType>
inline RwBool operator<(const SharedResource<LhsValueType>& lhs, const SharedResource<RhsValueType>& rhs)
{
    rwFUNCTION();
    rwASSERT(lhs != 0);
    rwASSERT(rhs != 0);
    rwRETURN(static_cast<RwBool>(lhs.Get() < rhs.Get()));
}

// Inline Functions

}

#endif // RW_SHAREDRESOURCE_HPP
