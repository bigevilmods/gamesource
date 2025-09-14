// (c) Electronic Arts. All Rights Reserved.

/// \file
/// A reference counted ptr for memory management. The object pointed to is
/// deleted when the last shared pointer pointing to that object is destroyed.

#ifndef PUBLIC_RW_CORE_SHAREDPTR_H
#define PUBLIC_RW_CORE_SHAREDPTR_H

#include <rw/core/stltypes.h>

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

// The default deleter used by the RwSharedPtr class.
// This deleter calls delete on the pointer passed to it.
template <typename PointerType>
class RwDefaultSharedPtrDeleter
{
public:
    /**
    Function call operator implementing Pointer deletion.
    \param ptr pointer to delete.
    */
    void operator()(PointerType ptr) const
    {
        rwFUNCTION();
        delete ptr;
        rwRETURNVOID();
    }
};

// ***********************************************************************************************************
//                                                RwSharedRefCountImplBase Definition
// ***********************************************************************************************************

// Base implementation class for the shared pointer reference counter. This class provides functions to
// increment and decrement the reference count. When the count is decremented to 0 the counter calls the
// virtual Dispose function to clean up the pointer and destructs itself (This class must not be declared
// on the stack as Destruct calls delete this).
class RwSharedRefCountImplBase
{
public:
    RwSharedRefCountImplBase()
      : m_count(1)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    virtual ~RwSharedRefCountImplBase()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    // dispose of the pointer being held
    virtual void Dispose() = 0;

    // destruct this reference counter
    void Destruct()
    {
        rwFUNCTION();
        delete this;
        rwRETURNVOID();
    }

    // Returns the number of copies of this counter which exist.
    size_t Count() const
    {
        rwFUNCTION();
        rwRETURN(m_count);
    }

    // return true if count == 1.
    RwBool Unique() const
    {
        rwFUNCTION();
        rwRETURN((RwBool)(m_count == 1));
    }

    // Increment the counter.
    void
    Incr()
    {
        rwFUNCTION();
        ++m_count;
        rwRETURNVOID();
    }

    // Decrement the counter. If the counter reaches 0, dispose
    // of the object held and the destruct this counter.
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

private:
    // not implemented
    RwSharedRefCountImplBase(const RwSharedRefCountImplBase& other);
    RwSharedRefCountImplBase& operator=(const RwSharedRefCountImplBase& other);

    size_t     m_count;
};

// Implementation class for the shared pointer reference counter.
// \par Template_params
// PointerType type of pointer to implement shared reference counting for.
// DeleterType type of functor used to implement deletion of the pointed to object.
//
// This class overloads the virtual Destruct method to clean up the pointer using the provided Deleter.
// This class maintains a copy of the pointer being held and a deleter so it can correctly dispose of
// the pointer.
template <typename PointerType, typename DeleterType>
class RwSharedRefCountImpl : public RwSharedRefCountImplBase
{
public:
    // Constructs a shared referencing counting pointer.
    // \param ptr Pointer to be reference counted.
    // \param deleter Function implementing pointed to object deletion.
    RwSharedRefCountImpl(PointerType ptr, DeleterType deleter)
      : m_ptr(ptr),
        m_deleter(deleter)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    ~RwSharedRefCountImpl()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    // Dispose of the pointer being held.
    void Dispose()
    {
        rwFUNCTION();
        m_deleter(m_ptr);
        rwRETURNVOID();
    }

private:
    // not implemented
    RwSharedRefCountImpl(const RwSharedRefCountImpl& other);
    RwSharedRefCountImpl& operator=(const RwSharedRefCountImpl& other);

    PointerType m_ptr;
    DeleterType m_deleter;
};

// ***********************************************************************************************************
//                                                RwSharedRefCount Definition
// ***********************************************************************************************************

// Reference counted object.
// This object has functions to copy the reference counter, incrementing the count when it is copied. The count
// is decremented when this object is destructed.
class RwSharedRefCount
{
public:
    RwSharedRefCount()
      : m_impl(0)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    // Construct the reference counter with a pointer to hold and a deleter for disposing of the pointer.
    // \param ptr Pointer to be reference counted.
    // \param deleter Function implementing pointed to object deletion.
    template<typename PointerType, typename DeleterType>
    RwSharedRefCount(PointerType ptr, DeleterType deleter)
      : m_impl(new RwSharedRefCountImpl<PointerType, DeleterType>(ptr, deleter))
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    // Copy a reference counter and increment the count.
    // \param other Reference to other shared pointer to copy.
    RwSharedRefCount(const RwSharedRefCount& other)
      : m_impl(other.m_impl)
    {
        rwFUNCTION();
        if (m_impl)
        {
            m_impl->Incr();
        }
        rwRETURNVOID();
    }

    // Destruct this instance of a count, decrementing the counter.
    // This function must not throw an exception.
    ~RwSharedRefCount()
    {
        rwFUNCTION();
        if (m_impl)
        {
            m_impl->Decr();
        }
        rwRETURNVOID();
    }

    // Copy a reference counter (decrementing the old counter firest) and increment the count.
    // This function must not throw an exception.
    RwSharedRefCount& operator=(const RwSharedRefCount& other)
    {
        rwFUNCTION();
        if (this != &other)
        {
            RwSharedRefCountImplBase *temp = other.m_impl;
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

    // Return the count size.
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

    // Returns true if count == 1.
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

    // Swap this reference counter with another.
    // This function must not throw an exception.
    void Swap(RwSharedRefCount& other)
    {
        rwFUNCTION();
        RwSwap(m_impl, other.m_impl);
        rwRETURNVOID();
    }

private:
    RwSharedRefCountImplBase *m_impl;
};

// ***********************************************************************************************************
//                                                RwSharedPtr Definition
// ***********************************************************************************************************

/**
Implements a shared pointer object.

ValueType has no restrictions.

\member m_ptr Please treat this as private. It is public only because member template friends are not supported by all compilers (eg gcc <= 2.97 and MSVC <= 7.1).
\member m_count Please treat this as private. It is public only because member template friends are not supported by all compilers (eg gcc <= 2.97 and MSVC <= 7.1).
*/
template <typename ValueType>
class RwSharedPtr
{
public:
    // Construction & Destruction
    RwSharedPtr()
      : m_ptr(0),
        m_count()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Construct a shared pointer with a pointer
     Note must not throw
     \param a pointer (must be convertable to a pointer to type ValueType)
    */
    template <typename OtherValueType>
    explicit RwSharedPtr(OtherValueType* ptr)
      : m_ptr(ptr),
        m_count(ptr, RwDefaultSharedPtrDeleter<OtherValueType*>())
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Construct a shared pointer with a pointer and a custom deleter used to dispose the pointer
     \param a pointer to an object of type OtherValueType (must be convertable to a pointer to type ValueType)
     \param a deleter object
    */
    template <typename OtherValueType, typename DeleterType>
    RwSharedPtr(OtherValueType* ptr, DeleterType deleter)
      : m_ptr(ptr),
        m_count(ptr, deleter)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Construct a shared pointer with from another
     \param a shared pointer
    */
    RwSharedPtr(const RwSharedPtr& other)
      : m_ptr(other.m_ptr),
        m_count(other.m_count)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Construct a shared pointer with from another
     \param a shared pointer to type OtherValueType (must be convertable to a pointer to type ValueType)
    */
    template <typename OtherValueType>
    RwSharedPtr(const RwSharedPtr<OtherValueType>& other)
      : m_ptr(other.m_ptr),
        m_count(other.m_count)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Destructor
    */
    ~RwSharedPtr()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
     Assign another shared pointer to this one
     Note must not throw
     \param a shared pointer
    */
    RwSharedPtr&
    operator=(const RwSharedPtr& other)
    {
        rwFUNCTION();
        m_ptr = other.m_ptr;
        m_count = other.m_count; // this must be written to correctly handle self assignment
        rwRETURN(*this);
    }

    /**
     Assign another shared pointer to this one
     Note must not throw
     \param a shared pointer to type OtherValueType (must be convertable to a pointer to type ValueType)
    */
    template <typename OtherValueType>
    RwSharedPtr&
    operator=(const RwSharedPtr<OtherValueType>& other)
    {
        rwFUNCTION();
        m_ptr = other.m_ptr;
        m_count = other.m_count; // this must be written to correctly handle self assignment
        rwRETURN(*this);
    }


    /**
     Release this reference to a shared pointer
     */
    void Reset()
    {
        rwFUNCTION();
        RwSharedPtr<ValueType>().Swap(*this);
        rwRETURNVOID();
    }

    /**
     Replace this reference to a pointer with a reference to another pointer
     \param a pointer to an object of type OtherValueType (must be convertable to a pointer to type ValueType)
     */
    template <typename OtherValueType>
    void Reset(OtherValueType* ptr)
    {
        rwFUNCTION();
        rwASSERT(!ptr || (ptr != m_ptr));
        RwSharedPtr<ValueType>(ptr).Swap(*this);
        rwRETURNVOID();
    }

    /**
     Replace this reference to a pointer with a reference to another pointer with a custom deleter
     \param a pointer to an object of type OtherValueType (must be convertable to a pointer to type ValueType)
     \param a deleter which will be used to dispose of the pointer
     */
    template <typename OtherValueType, typename DeleterType>
    void Reset(OtherValueType* ptr, DeleterType deleter)
    {
        rwFUNCTION();
        RwSharedPtr<ValueType>(ptr, deleter).Swap(*this);
        rwRETURNVOID();
    }

    /**
     Dereference a shared pointer.
     Note the shared pointer must not be null.
     \return a reference to the object pointed to by the shared pointer.
     */
    const ValueType& operator*() const
    {
        rwFUNCTION();
        rwASSERT(m_ptr);
        rwRETURN(*m_ptr);
    }

    /**
     Dereference a shared pointer.
     Note the shared pointer must not be null.
     \return a reference to the object pointed to by the shared pointer.
     */
    ValueType& operator*()
    {
        rwFUNCTION();
        rwASSERT(m_ptr);
        rwRETURN(*m_ptr);
    }

    /**
     Dereference a shared pointer.
     Note the shared pointer must not be null.
     \return a bare pointer to the object pointed to by the shared pointer.
     */
    const ValueType* operator->() const
    {
        rwFUNCTION();
        rwASSERT(m_ptr);
        rwRETURN(m_ptr);
    }

    /**
     Dereference a shared pointer.
     Note the shared pointer must not be null.
     \return a bare pointer to the object pointed to by the shared pointer.
     */
    ValueType* operator->()
    {
        rwFUNCTION();
        rwASSERT(m_ptr);
        rwRETURN(m_ptr);
    }

    /**
     Get the pointer without affecting the reference counter.
     Note the bare pointer is still owned by this object
     \return a copy of bare pointer
     */
    const ValueType* Get() const
    {
        rwFUNCTION();
        rwRETURN(m_ptr);
    }

    /**
     Get the pointer without affecting the reference counter.
     Note the bare pointer is still owned by this object
     \return a copy of bare pointer
     */
    ValueType* Get()
    {
        rwFUNCTION();
        rwRETURN(m_ptr);
    }

    /**
     \return true if the pointer is not null.
     */
    operator RwBool() const
    {
        rwFUNCTION();
        rwRETURN(static_cast<RwBool>(m_ptr != 0));
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
     \return the copies of this shared pointer which currently exist.
     */
    size_t Count() const
    {
        rwFUNCTION();
        rwRETURN(m_count.Count());
    }

    /**
     \return true if this is the only copy of this shared pointer.
     */
    RwBool Unique() const
    {
        rwFUNCTION();
        rwRETURN(m_count.Unique());
    }

    /**
     Swap this shared pointer for another.
     */
    void Swap(RwSharedPtr<ValueType>& other)
    {
        rwFUNCTION();
        RwSwap(m_ptr, other.m_ptr);
        m_count.Swap(other.m_count);
        rwRETURNVOID();
    }

private:
    template<class OtherValueType> friend class RwSharedPtr;

    ValueType       *m_ptr;
    RwSharedRefCount m_count;
};

/**
Swap this shared pointer for another.
Note must not throw
*/
template <typename ValueType>
inline void RwSwap(RwSharedPtr<ValueType>& lhs, RwSharedPtr<ValueType>& rhs)
{
    rwFUNCTION();
    lhs.Swap(rhs);
    rwRETURNVOID();
}

/**
 \return true if two shared pointers point to the same object.
*/
template <typename LhsValueType, typename RhsValueType>
inline RwBool operator==(const RwSharedPtr<LhsValueType>& lhs, const RwSharedPtr<RhsValueType>& rhs)
{
    rwFUNCTION();
    rwRETURN(static_cast<RwBool>(lhs.Get() == rhs.Get()));
}

/**
\return true if two shared pointers do not point to the same object.
*/
template <typename LhsValueType, typename RhsValueType>
inline RwBool operator!=(const RwSharedPtr<LhsValueType>& lhs, const RwSharedPtr<RhsValueType>& rhs)
{
    rwFUNCTION();
    rwRETURN(static_cast<RwBool>(lhs.Get() != rhs.Get()));
}

/**
An ordering function so we
\param the two shared pointers (both pointers must not be null)
\return true if the lhs pointer < rhs pointer
*/
template <typename LhsValueType, typename RhsValueType>
inline RwBool operator<(const RwSharedPtr<LhsValueType>& lhs, const RwSharedPtr<RhsValueType>& rhs)
{
    rwFUNCTION();
    rwASSERT(lhs != 0);
    rwASSERT(rhs != 0);
    rwRETURN(static_cast<RwBool>(lhs.Get() < rhs.Get()));
}

// Inline Functions

#endif // PUBLIC_RW_CORE_SHAREDPTR_H
