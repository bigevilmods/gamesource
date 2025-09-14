// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implementation of the rw::RwPtr class for a 32-bit host
platform which is targetting a 64-bit platform.
*/
// NB: this has to be outside the include guard
// since ostypes.h include ptrnative.h
#include <rw/core/base/ostypes.h>
#ifndef RW_RWPTRCROSS32TO64_HPP
#define RW_RWPTRCROSS32TO64_HPP

// ***********************************************************************************************************
// Includes

#if defined(rwUSERWPTRCROSS32TO64)

// ***********************************************************************************************************
// Defines

namespace rw
{

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
//                                                Class Definition
// ***********************************************************************************************************

template <typename ValueType>
class RwPtrBase
{
public:
    // Construction & Destruction
    inline RwPtrBase(ValueType* ptr = 0);
    inline ~RwPtrBase();

    // assignment
    template <typename OtherType>
    inline const RwPtrBase& operator=(const RwPtrBase<OtherType>& rhs);

    template <typename OtherType>
    inline const RwPtrBase& operator=(OtherType* ptr);

    // functions for transforming between RwPtrs and offsets
    inline rw::TargetPtr AsOffset() const;

    inline operator ValueType*() const;
    inline RwBool operator!() const;

    // increment and decrement
    inline RwPtrBase& operator++();
    inline RwPtrBase  operator++(int i);
    inline RwPtrBase& operator--();
    inline RwPtrBase  operator--(int i);

    // unary arithmetic
    inline void operator+=(uint32_t offset);
    inline void operator-=(uint32_t offset);

    inline void EndianSwap();
    inline void ForcedEndianSwap();

    inline RwBool IsMemAligned(uint32_t align) const;

// protected:
#if rwTARGET_ENDIAN == rwBIENDIAN
    uint32_t m_padding;
    ValueType* m_ptr;
#else // if rwTARGET_ENDIAN == rwLIENDIAN
    ValueType* m_ptr;
    uint32_t m_padding;
#endif
};

template <typename ValueType>
class RWCLASSPREALIGN(rwTARGET_POINTERALIGNMENT) RwPtr : public RwPtrBase<ValueType>
{
public:
    // Construction & Destruction
    inline RwPtr();

    template <typename OtherType>
    inline RwPtr(OtherType* ptr);

    inline RwPtr(const RwPtr& rhs);

    template <typename OtherType>
    inline RwPtr(const RwPtrBase<OtherType>& rhs);

    // pointer operators
    inline ValueType& operator*() const;
    inline ValueType& operator*();
    inline ValueType* const operator->() const;
    inline ValueType* operator->();
    // inline const ValueType& operator[](uint32_t index) const;
    // inline ValueType& operator[](uint32_t index);

    // functions for transforming between RwPtrs and offsets
    static inline RwPtr FromOffset(rw::TargetPtr offset);

    inline RwPtr MemAlign(uint32_t align) const;
    inline RwPtr MemAddOffset(rw::TargetPtr offset) const;
} RWCLASSPOSTALIGN(rwTARGET_POINTERALIGNMENT);

// specialization for void ptr to disallow dereferencing
template <>
class RWCLASSPREALIGN(rwTARGET_POINTERALIGNMENT) RwPtr<void> : public RwPtrBase<void>
{
public:
    // Construction & Destruction
    inline RwPtr();

    template <typename OtherType>
    inline RwPtr(OtherType* ptr);

    template <typename OtherType>
    inline RwPtr(const RwPtr<OtherType>& rhs);

    // functions for transforming between RwPtrs and offsets
    static inline RwPtr FromOffset(rw::TargetPtr offset);

    inline RwPtr MemAlign(uint32_t align) const;
    inline RwPtr MemAddOffset(rw::TargetPtr offset) const;
} RWCLASSPOSTALIGN(rwTARGET_POINTERALIGNMENT);

// specialization for const void ptr to disallow dereferencing
template <>
class RWCLASSPREALIGN(rwTARGET_POINTERALIGNMENT) RwPtr<const void> : public RwPtrBase<const void>
{
public:
    // Construction & Destruction
    inline RwPtr();

    template <typename OtherType>
    inline RwPtr(OtherType* ptr);

    template <typename OtherType>
    inline RwPtr(const RwPtr<OtherType>& rhs);

    // functions for transforming between RwPtrs and offsets
    static inline RwPtr FromOffset(rw::TargetPtr offset);

    inline RwPtr MemAlign(uint32_t align) const;
    inline RwPtr MemAddOffset(rw::TargetPtr offset) const;
} RWCLASSPOSTALIGN(rwTARGET_POINTERALIGNMENT);

#define RWPTR(T) rw::RwPtr< T >                 // the spaces in < T > are needed to for nesting
#define RWCONSTPTR(T) rw::RwPtr< const T >      // the spaces in < const T > are needed to for nesting

#define RWDECLARE_TYPE_PTRS(T) \
    typedef RWPTR(T) RWALIGN(T##Ptr, rwTARGET_POINTERALIGNMENT); \
    typedef RWCONSTPTR(T) RWALIGN(T##ConstPtr, rwTARGET_POINTERALIGNMENT)

template <typename ValueType>
inline rw::TargetPtr
RwPtrToOffset(const RWPTR(ValueType)& ptr)
{
    rwFUNCTION();
    rwRETURN(ptr.AsOffset());
}

template <typename ValueType>
inline RWPTR(ValueType)
RwPtrFromOffset(rw::TargetPtr offset)
{
    rwFUNCTION();
    rwRETURN(RWPTR(ValueType)::FromOffset(offset));
}

template <typename ValueType>
inline RWPTR(ValueType)
RwPtrAddOffset(const RWPTR(ValueType)& ptr, rw::TargetPtr offset)
{
    rwFUNCTION();
    rwRETURN(RwPtrFromOffset<ValueType>(RwPtrToOffset(ptr) + offset));
}

template <typename ValueType>
inline RWPTR(ValueType)
RwPtrSubtractOffset(const RWPTR(ValueType)& ptr, rw::TargetPtr offset)
{
    rwFUNCTION();
    rwRETURN(RwPtrFromOffset<ValueType>(RwPtrToOffset(ptr) - offset));
}

template <typename ValueType>
RWPTR(ValueType) RwPtrAddBasePtr(const RWPTR(ValueType)& ptr, const RWPTR(const void)& base)
{
    rwFUNCTION();
    rwRETURN(RwPtrAddOffset(ptr, RwPtrToOffset(base)));
}

template <typename ValueType>
RWPTR(ValueType) RwPtrSubtractBasePtr(const RWPTR(ValueType)& ptr, const RWPTR(const void)& base)
{
    rwFUNCTION();
    rwRETURN(RwPtrSubtractOffset(ptr, RwPtrToOffset(base)));
}

template <typename ValueType>
inline ValueType*
RwPtrToHostPtr(const RWPTR(ValueType)& ptr)
{
    rwFUNCTION();
    rwRETURN(reinterpret_cast<ValueType*>((uint32_t)(RwPtrToOffset(ptr))));
}

template <typename ValueType>
class RwPtrReinterpretCast
{
public:
    template <typename U>
    RwPtrReinterpretCast(const RWPTR(U)& other)
      : m_ptr(reinterpret_cast<ValueType*>(other.m_ptr))
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    template <typename U>
    RwPtrReinterpretCast(U* ptr)
      : m_ptr(reinterpret_cast<ValueType*>(ptr))
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    operator RWPTR(ValueType)()const
    {
        rwFUNCTION();
        rwRETURN(RwPtrFromOffset<ValueType>(RwPtrToOffset(m_ptr)));
    }

private:
    RWPTR(ValueType) m_ptr;
};

template <typename ValueType>
RWPTR(ValueType) MemAlign(RWPTR(ValueType)& ptr, uint32_t align)
{
    rwFUNCTION();
    rwRETURN(ptr.MemAlign(align));
}

template <typename ValueType>
RWPTR(ValueType) MemAddOffset(RWPTR(ValueType)& ptr, rw::TargetPtr offset)
{
    rwFUNCTION();
    rwRETURN(RwPtrFromOffset<ValueType>(RwPtrToOffset(ptr) + offset));
}

template <typename ValueType>
RWPTR(ValueType) MemAddOffset(const RWPTR(ValueType)& ptr, rw::TargetPtr offset)
{
    rwFUNCTION();
    rwRETURN(RwPtrFromOffset<ValueType>(RwPtrToOffset(ptr) + offset));
}

template <typename ValueType>
inline void
EndianSwap(RWPTR(ValueType)& ptr)
{
    rwFUNCTION();
#if rwNATIVE_ENDIAN_CONVERT_REQUIRED
    ptr.EndianSwap();
#endif // rwNATIVE_ENDIAN_CONVERT_REQUIRED
    rwRETURNVOID();
}

template <typename ValueType>
inline void
ForcedEndianSwap(RWPTR(ValueType)& ptr)
{
    rwFUNCTION();
    ptr.EndianSwap();
    rwRETURNVOID();
}

template <typename T>
inline void
EndianSwapArray(RWPTR(T)& array, uint32_t size)
{
    rwFUNCTION();
    EndianSwapArray(RwPtrToHostPtr(array), size);
    rwRETURNVOID();
}

template <typename ValueType>
inline RwBool
IsMemAligned(RWPTR(ValueType)& ptr, uint32_t align)
{
    rwFUNCTION();
    rwRETURN(ptr.IsMemAligned(align));
}

template <typename FuncType>
class RWCLASSPREALIGN(rwTARGET_POINTERALIGNMENT) RwFuncPtr
{
public:
    // Construction & Destruction
    inline RwFuncPtr();

    template <typename OtherType>
    inline RwFuncPtr(OtherType ptr);

    inline RwFuncPtr(const RwFuncPtr& rhs);

    // assignment
    template <typename OtherFuncType>
    inline const RwFuncPtr& operator=(const RwFuncPtr<OtherFuncType>& rhs);

    template <typename OtherFuncType>
    inline const RwFuncPtr& operator=(OtherFuncType ptr);

    inline FuncType operator*() const;

    inline operator bool() const;
    inline RwBool operator!() const;

private:
#if rwTARGET_ENDIAN == rwBIENDIAN
    uint32_t m_padding;
    FuncType m_function;
#else // if rwTARGET_ENDIAN == rwLIENDIAN
    FuncType m_function;
    uint32_t m_padding;
#endif
} RWCLASSPOSTALIGN(rwTARGET_POINTERALIGNMENT);

#define RWFUNCPTR(T) rw::RwFuncPtr< T >

} //namespace rw


// binary arithmetic
// template <typename ValueType>
// inline rw::RwPtr<ValueType> operator+(rw::RwPtr<ValueType> lhs, rw::TargetPtr offset);
// template <typename ValueType>
// inline rw::RwPtr<ValueType> operator+(rw::TargetPtr offset, rw::RwPtr<ValueType> rhs);
// template <typename ValueType>
// inline rw::RwPtr<ValueType> operator-(rw::RwPtr<ValueType> lhs, rw::TargetPtr rhs);

// template <typename ValueType>
// inline rw::TargetPtr operator-(rw::RwPtr<ValueType> lhs, rw::RwPtr<ValueType> rhs);

// equality and comparison operators

template <typename ValueType>
inline RwBool operator==(const rw::RwPtr<ValueType>& lhs, const rw::RwPtr<ValueType>& rhs);

template <typename ValueType>
inline RwBool operator!=(const rw::RwPtr<ValueType>& lhs, const rw::RwPtr<ValueType>& rhs);

template <typename ValueType>
inline RwBool operator<(const rw::RwPtr<ValueType>& lhs, const rw::RwPtr<ValueType>& rhs);

template <typename ValueType>
inline RwBool operator<=(const rw::RwPtr<ValueType>& lhs, const rw::RwPtr<ValueType>& rhs);

template <typename ValueType>
inline RwBool operator>(const rw::RwPtr<ValueType>& lhs, const rw::RwPtr<ValueType>& rhs);

template <typename ValueType>
inline RwBool operator>=(const rw::RwPtr<ValueType>& lhs, const rw::RwPtr<ValueType>& rhs);

// Inline Functions

template <typename ValueType>
rw::RwPtrBase<ValueType>::RwPtrBase(ValueType* ptr)
#if rwTARGET_ENDIAN == rwBIENDIAN
  : m_padding(0),
    m_ptr(ptr)
#else // if rwTARGET_ENDIAN == rwLIENDIAN
  : m_ptr(ptr),
    m_padding(0)
#endif
{
    rwFUNCTION();
    rwRETURNVOID();
}

template <typename ValueType>
rw::RwPtrBase<ValueType>::~RwPtrBase()
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Assign to an RwPtr from another RwPtr
template <typename ValueType>
template <typename OtherType>
const rw::RwPtrBase<ValueType>&
rw::RwPtrBase<ValueType>::operator=(const RwPtrBase<OtherType>& rhs)
{
    rwFUNCTION();
    m_ptr = rhs.m_ptr;
    m_padding = 0;
    rwFUNCTION();
    rwRETURN(*this);
}

// Assign to an RwPtr from a native pointer
template <typename ValueType>
template <typename OtherType>
const rw::RwPtrBase<ValueType>&
rw::RwPtrBase<ValueType>::operator=(OtherType* ptr)
{
    rwFUNCTION();
    m_ptr = ptr;
    m_padding = 0;
    rwFUNCTION();
    rwRETURN(*this);
}

// Get the pointer as an integer type
// \return a copy of bare pointer.
template <typename ValueType>
rw::TargetPtr
rw::RwPtrBase<ValueType>::AsOffset() const
{
    rwFUNCTION();
    rwRETURN(reinterpret_cast<uint32_t>(m_ptr));
}

// \return true if the pointer is not null
template <typename ValueType>
rw::RwPtrBase<ValueType>::operator ValueType*() const
{
    rwFUNCTION();
    rwRETURN(m_ptr);
}

// return true if the pointer is null
template <typename ValueType>
RwBool
rw::RwPtrBase<ValueType>::operator!() const
{
    rwFUNCTION();
    rwRETURN(!m_ptr);
}

// Pre-increment
template <typename ValueType>
rw::RwPtrBase<ValueType>&
rw::RwPtrBase<ValueType>::operator++()
{
    rwFUNCTION();
    ++m_ptr;
    rwRETURN(*this);
}

// Post-increment
// \param unused For post-increment declaration only.
template <typename ValueType>
rw::RwPtrBase<ValueType>
rw::RwPtrBase<ValueType>::operator++(int)
{
    rwFUNCTION();
    rwRETURN(rw::RwPtrBase<ValueType>(m_ptr++));
}

// Pre-decrement
template <typename ValueType>
rw::RwPtrBase<ValueType>&
rw::RwPtrBase<ValueType>::operator--()
{
    rwFUNCTION();
    --m_ptr;
    rwFUNCTION();
    rwRETURN(*this);
}

// Post-decrement
// \param unused For post-decrement declaration only.
template <typename ValueType>
rw::RwPtrBase<ValueType>
rw::RwPtrBase<ValueType>::operator--(int)
{
    rwFUNCTION();
    rwRETURN(rw::RwPtrBase<ValueType>(m_ptr--));
}

// Add an count objects to this pointer.
template <typename ValueType>
void
rw::RwPtrBase<ValueType>::operator+=(uint32_t count)
{
    rwFUNCTION();
    m_ptr += count;
    rwRETURNVOID();
}

// Subtract a count objects from this pointer.
template <typename ValueType>
void
rw::RwPtrBase<ValueType>::operator-=(uint32_t count)
{
    rwFUNCTION();
    m_ptr -= count;
    rwRETURNVOID();
}

// Endian swap the pointer inplace if required.
template <typename ValueType>
void
rw::RwPtrBase<ValueType>::EndianSwap()
{
    rwFUNCTION();
    rw::EndianSwap(m_ptr);
    rwRETURNVOID();
}

// Endian swap the pointer inplace (forced).
template <typename ValueType>
void
rw::RwPtrBase<ValueType>::ForcedEndianSwap()
{
    rwFUNCTION();
    rw::ForcedEndianSwap(m_ptr);
    rwRETURNVOID();
}

// Return true if the pointer value is at least memory aligned to the provided alignment.
// \param the required alignment. align must be a power of 2 (not enforced).
// \return true if the pointer is aligned to a power of 2 greater or equal to align.
template <typename ValueType>
RwBool
rw::RwPtrBase<ValueType>::IsMemAligned(uint32_t align) const
{
    rwFUNCTION();
    rwRETURN(rw::IsMemAligned(m_ptr, align));
}

// Default Constructor.
template <typename ValueType>
rw::RwPtr<ValueType>::RwPtr()
  : RwPtrBase<ValueType>(0)
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Construct an RwPtr from a ptr.
template <typename ValueType>
template <typename OtherType>
rw::RwPtr<ValueType>::RwPtr(OtherType* ptr)
  : RwPtrBase<ValueType>(ptr)
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Construct an RwPtr from another RwPtr.
template <typename ValueType>
rw::RwPtr<ValueType>::RwPtr(const RwPtr<ValueType>& rhs)
  : RwPtrBase<ValueType>(rhs.m_ptr)
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Construct an RwPtr from another RwPtr.
template <typename ValueType>
template <typename OtherType>
rw::RwPtr<ValueType>::RwPtr(const RwPtrBase<OtherType>& rhs)
  : RwPtrBase<ValueType>((ValueType *)rhs.m_ptr)
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Dereference an RwPtr.
// Note the pointer must not be null.
// \return a reference to the object pointed to.
template <typename ValueType>
ValueType&
rw::RwPtr<ValueType>::operator*() const
{
    // rwENFORCE(this->m_ptr, ("Can't dereference a null pointer."));
    rwFUNCTION();
    rwRETURN(*this->m_ptr);
}

// Dereference an RwPtr.
// Note the pointer must not be null.
// \return a reference to the object pointed to.
template <typename ValueType>
ValueType&
rw::RwPtr<ValueType>::operator*()
{
    // rwENFORCE(this->m_ptr, ("Can't dereference a null pointer."));
    rwFUNCTION();
    rwRETURN(*this->m_ptr);
}

// Dereference an RwPtr.
// Note the pointer must not be null.
// \return a bare pointer to the object.
template <typename ValueType>
ValueType* const
rw::RwPtr<ValueType>::operator->() const
{
    rwFUNCTION();
    rwRETURN(this->m_ptr);
}

// Dereference an RwPtr.
// Note the pointer must not be null.
// \return a bare pointer to the object.
template <typename ValueType>
ValueType*
rw::RwPtr<ValueType>::operator->()
{
    rwFUNCTION();
    rwRETURN(this->m_ptr);
}

// // Treat the pointer as an array. Get the object at the index.
// template <typename ValueType>
// const ValueType&
// rw::RwPtr<ValueType>::operator[](uint32_t index) const
// {
//     // rwENFORCE(this + index, ("Can't dereference a null pointer."));
//     return *(this + index);
// }
//
// // Treat the pointer as an array. Get the object at the index.
// template <typename ValueType>
// ValueType&
// rw::RwPtr<ValueType>::operator[](uint32_t index)
// {
//     // rwENFORCE(this + index, ("Can't dereference a null pointer."));
//     return *(this + index);
// }

// Construct an RwPtr from an offset.
template <typename ValueType>
rw::RwPtr<ValueType>
rw::RwPtr<ValueType>::FromOffset(rw::TargetPtr offset)
{
    rwFUNCTION();
    rwRETURN(rw::RwPtr<void>(reinterpret_cast<void*>((uint32_t)offset)));
}

// Mem align the internal pointer to the provided alignment
template <typename ValueType>
rw::RwPtr<ValueType>
rw::RwPtr<ValueType>::MemAlign(uint32_t align) const
{
    rwFUNCTION();
    rwRETURN(rw::MemAlign(this->m_ptr, align));
}

// Mem align the internal pointer to the provided alignment
template <typename ValueType>
rw::RwPtr<ValueType>
rw::RwPtr<ValueType>::MemAddOffset(rw::TargetPtr offset) const
{
    rwFUNCTION();
    rwRETURN(RwPtrFromOffset<ValueType>(this->AsOffset() + offset));
}

// Default Constructor.
rw::RwPtr<void>::RwPtr()
  : RwPtrBase<void>(0)
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Construct an RwPtr from a ptr.
template <typename OtherType>
rw::RwPtr<void>::RwPtr(OtherType* ptr)
  : RwPtrBase<void>(ptr)
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Construct an RwPtr from another RwPtr.
template <typename OtherType>
rw::RwPtr<void>::RwPtr(const RwPtr<OtherType>& rhs)
  : RwPtrBase<void>(rhs.m_ptr)
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Construct an RwPtr from an offset.
rw::RwPtr<void>
rw::RwPtr<void>::FromOffset(rw::TargetPtr offset)
{
    rwFUNCTION();
    rwRETURN(rw::RwPtr<void>(reinterpret_cast<void*>((uint32_t)offset)));
}

// Mem align the internal pointer to the provided alignment
rw::RwPtr<void>
rw::RwPtr<void>::MemAlign(uint32_t align) const
{
    rwFUNCTION();
    rwRETURN(rw::MemAlign(this->m_ptr, align));
}

// Mem align the internal pointer to the provided alignment
rw::RwPtr<void>
rw::RwPtr<void>::MemAddOffset(rw::TargetPtr offset) const
{
    rwFUNCTION();
    rwRETURN(RwPtrFromOffset<void>(this->AsOffset() + offset));
}

// Default Constructor.
rw::RwPtr<const void>::RwPtr()
  : RwPtrBase<const void>(0)
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Construct an RwPtr from a ptr.
template <typename OtherType>
rw::RwPtr<const void>::RwPtr(OtherType* ptr)
  : RwPtrBase<const void>(ptr)
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Construct an RwPtr from another RwPtr.
template <typename OtherType>
rw::RwPtr<const void>::RwPtr(const RwPtr<OtherType>& rhs)
  : RwPtrBase<const void>(rhs.m_ptr)
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Construct an RwPtr from an offset.
rw::RwPtr<const void>
rw::RwPtr<const void>::FromOffset(rw::TargetPtr offset)
{
    rwFUNCTION();
    rwRETURN(rw::RwPtr<const void>(reinterpret_cast<const void*>((uint32_t)offset)));
}

// Mem align the internal pointer to the provided alignment
rw::RwPtr<const void>
rw::RwPtr<const void>::MemAlign(uint32_t align) const
{
    rwFUNCTION();
    rwRETURN(rw::MemAlign(this->m_ptr, align));
}

// Mem align the internal pointer to the provided alignment
rw::RwPtr<const void>
rw::RwPtr<const void>::MemAddOffset(rw::TargetPtr offset) const
{
    rwFUNCTION();
    rwRETURN(RwPtrFromOffset<const void>(this->AsOffset() + offset));
}


// // Add an offset to the lhs pointer.
// template <typename ValueType>
// rw::RwPtr<ValueType>
// operator+(rw::RwPtr<ValueType> lhs, rw::TargetPtr offset)
// {
//     return rw::RwPtr<ValueType>(lhs.AsOffset() + offset);
// }

// // Add an offset to the rhs pointer.
// template <typename ValueType>
// rw::RwPtr<ValueType>
// operator+(rw::TargetPtr offset, rw::RwPtr<ValueType> rhs)
// {
//     return rw::RwPtr<ValueType>(rhs.AsOffset() + offset);
// }

// // Subtract an offset from a pointer
// template <typename ValueType>
// rw::RwPtr<ValueType>
// operator-(rw::RwPtr<ValueType> lhs, rw::TargetPtr offset)
// {
//     return rw::RwPtr<ValueType>(lhs.AsOffset() - offset);
// }

// // Subtract the pointer rhs from from the pointer lhs.
// template <typename ValueType>
// rw::TargetPtr
// operator-(rw::RwPtr<ValueType> lhs, rw::RwPtr<ValueType> rhs)
// {
//     return rw::TargetPtr(lhs.AsOffset() - rhs.AsOffset());
// }

// \return true if two pointers point to the same object.
template <typename ValueType>
RwBool
operator==(const rw::RwPtr<ValueType>& lhs, const rw::RwPtr<ValueType>& rhs)
{
    rwFUNCTION();
    rwRETURN(lhs.AsOffset() == rhs.AsOffset());  //!!!
}

// \return true if two pointers do not point to the same object.
template <typename ValueType>
RwBool
operator!=(const rw::RwPtr<ValueType>& lhs, const rw::RwPtr<ValueType>& rhs)
{
    rwFUNCTION();
    rwRETURN(!(lhs.AsOffset() == rhs.AsOffset()));  //!!!
}

// \return true if pointers lhs is less than pointer rhs
template <typename ValueType>
RwBool
operator<(const rw::RwPtr<ValueType>& lhs, const rw::RwPtr<ValueType>& rhs)
{
    rwFUNCTION();
    rwRETURN(lhs.AsOffset() < rhs.AsOffset()); //!!!
}

// \return true if pointers lhs is less than or equal to pointer rhs
template <typename ValueType>
RwBool
operator<=(const rw::RwPtr<ValueType>& lhs, const rw::RwPtr<ValueType>& rhs)
{
    rwFUNCTION();
    rwRETURN(lhs.AsOffset() <= rhs.AsOffset());
}

// \return true if pointers lhs is greater than pointer rhs
template <typename ValueType>
RwBool
operator>(const rw::RwPtr<ValueType>& lhs, const rw::RwPtr<ValueType>& rhs)
{
    rwFUNCTION();
    rwRETURN(lhs.AsOffset() > rhs.AsOffset());
}

// \return true if pointers lhs is greater than or equal to pointer rhs
template <typename ValueType>
RwBool
operator>=(const rw::RwPtr<ValueType>& lhs, const rw::RwPtr<ValueType>& rhs)
{
    rwFUNCTION();
    rwRETURN(lhs.AsOffset() >= rhs.AsOffset());  //!!!
}

// Create an empty RwFuncPtr
template <typename FuncType>
rw::RwFuncPtr<FuncType>::RwFuncPtr()
#if rwTARGET_ENDIAN == rwBIENDIAN
  : m_padding(0),
    m_function(0)
#else // if rwTARGET_ENDIAN == rwLIENDIAN
  : m_function(0),
    m_padding(0)
#endif
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Create an RwFuncPtr from a raw function ptr
template <typename FuncType>
template <typename OtherFuncType>
rw::RwFuncPtr<FuncType>::RwFuncPtr(OtherFuncType func)
#if rwTARGET_ENDIAN == rwBIENDIAN
  : m_padding(0),
    m_function(func)
#else // if rwTARGET_ENDIAN == rwLIENDIAN
  : m_function(func),
    m_padding(0)
#endif
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Create an RwFuncPtr from another
template <typename FuncType>
rw::RwFuncPtr<FuncType>::RwFuncPtr(const RwFuncPtr<FuncType>& rhs)
#if rwTARGET_ENDIAN == rwBIENDIAN
  : m_padding(0),
    m_function(rhs.m_function)
#else // if rwTARGET_ENDIAN == rwLIENDIAN
  : m_function(rhs.m_function),
    m_padding(0)
#endif
{
    rwFUNCTION();
    rwRETURNVOID();
}

// Assign to an RwFuncPtr from another RwFuncPtr
template <typename FuncType>
template <typename OtherFuncType>
const rw::RwFuncPtr<FuncType>&
rw::RwFuncPtr<FuncType>::operator=(const RwFuncPtr<OtherFuncType>& rhs)
{
    rwFUNCTION();
    m_function = rhs.m_function;
    m_padding = 0;
    rwRETURN(*this);
}

// Assign to an RwPtr from a native pointer
template <typename FuncType>
template <typename OtherFuncType>
const rw::RwFuncPtr<FuncType>&
rw::RwFuncPtr<FuncType>::operator=(OtherFuncType func)
{
    rwFUNCTION();
    m_function = func;
    m_padding = 0;
    rwRETURN(*this);
}

// Dereference the function pointer.
template <typename FuncType>
FuncType
rw::RwFuncPtr<FuncType>::operator*() const
{
    rwFUNCTION();
    rwRETURN(m_function);
}

// Check if the function pointer is not null.
template <typename FuncType>
rw::RwFuncPtr<FuncType>::operator bool() const
{
    rwFUNCTION();
    rwRETURN(m_function != 0);
}

// Check if the function pointer is null.
template <typename FuncType>
RwBool
rw::RwFuncPtr<FuncType>::operator!() const
{
    rwFUNCTION();
    rwRETURN(m_function == 0);
}

#endif // defined(rwUSERWPTRCROSS32TO64)

#endif // RW_RWPTRCROSS32TO64_HPP
