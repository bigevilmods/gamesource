// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Framework classes for the resource descriptors and resources.

#ifndef RW_BASERESOURCE_HPP
#define RW_BASERESOURCE_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>

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
//                                                BaseResourceDescriptor Class Definition
// ***********************************************************************************************************

/**
An object describing size and alignment requirements of a single memory block.
*/
class BaseResourceDescriptor
{
public:
    /**
    Construct a default BaseResourceDescriptor.
    */
    BaseResourceDescriptor()
      : m_size(0),
        m_alignment(1)
    {
        rwFUNCTION();
        rwASSERT(m_alignment > 0);
        rwRETURNVOID();
    }

    /**
    Construct a BaseResourceDescriptor from a size and alignment.
    \param size the size in bytes.
    \param alignment the alignment in bytes.
    */

    BaseResourceDescriptor(size_t size, uint32_t alignment)
      : m_size(static_cast<unsigned int>(size)),
        m_alignment(alignment)
    {
        rwFUNCTION();
        rwASSERT(m_alignment > 0);
        rwRETURNVOID();
    }

    /**
    Destruct a BaseResourceDescriptor.
    */
    ~BaseResourceDescriptor()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    // default copy constructor and assignment operators are sufficient
    // BaseResourceDescriptor(const BaseResourceDescriptor& other);
    // BaseResourceDescriptor operator=(const BaseResourceDescriptor& other);

    /**
    Endian swap the elements of the BaseResourceDescriptor.
    */
    void
    EndianSwap()
    {
        rwFUNCTION();
        rw::EndianSwap(m_size);
        rw::EndianSwap(m_alignment);
        rwRETURNVOID();
    }

    /**
    Get the size of memory required.
    \return the size in bytes.
    */
    uint32_t GetSize() const
    {
        rwFUNCTION();
        rwRETURN(static_cast<uint32_t>(m_size));
    }

    /**
    Set the size of memory required.
    \param the size in bytes.
    */
    void SetSize(uint32_t RW_W64 size)
    {
        rwFUNCTION();
        m_size = size;
        rwRETURNVOID();
    }

    /**
    Get the memory alignment required.
    \return the alignment in bytes.
    */
    uint32_t GetAlignment() const
    {
        rwFUNCTION();
        rwRETURN(m_alignment);
    }

    /**
    Set the memory alignment required.
    \param the alignment in bytes.
    */
    void SetAlignment(uint32_t alignment)
    {
        rwFUNCTION();
        m_alignment = alignment;
        rwRETURNVOID();
    }

    /**
    Align the size component of a BaseResourceDescriptor to the alignment requirements of the passed in
    BaseResourceDescriptor.
    
    The alignment is updated to the maximum alignment value of this BaseResourceDescriptor and rhs.
    \param rhs the BaseResourceDescriptor used to align this BaseResourceDescriptor.
    */
    void Align(const BaseResourceDescriptor& rhs)
    {
        rwFUNCTION();
        m_size = SizeAlign(m_size, rhs.m_alignment);
        m_alignment = (m_alignment > rhs.m_alignment ? m_alignment : rhs.m_alignment);
        rwRETURNVOID();
    }


    /**
    Add a BaseResourceDescriptor to this one.
    
    The BaseResourceDescriptor is first aligned to the alignment requirements of rhs.
    Then the resulting size is added to the size of the passed in BaseResourceDescriptor.
    The resulting BaseResourceDescriptor has an alignment equal to the maximum of the original alignment
    and the alignment of rhs.
    \param rhs the BaseResourceDescriptor to add to this one.
    */
    void operator+=(const BaseResourceDescriptor& rhs)
    {
        rwFUNCTION();
        Align(rhs);
        m_size += rhs.m_size;
        rwRETURNVOID();
    }

    /**
    Add two BaseResourceDescriptors.
    
    The added BaseResourceDescriptor must have an alignment less than or equal to the lhs alignment.
    The resulting BaseResourceDescriptor has a size equal to the sum of the lhs size rounded up to
    a multiple of the alignment of rhs and the size of rhs.
    The resulting BaseResourceDescriptor has an alignment equal to the maximum of the lhs alignment
    and the alignment of rhs.
    \param rhs the BaseResourceDescriptor to add to this one.
    */
    BaseResourceDescriptor operator+(const BaseResourceDescriptor& rhs) const
    {
        rwFUNCTION();
        BaseResourceDescriptor sum = *this;
        sum += rhs;
        rwRETURN(sum);
    }

    /**
    Subtract a BaseResourceDescriptor from this one.
    
    The size of the subtracted BaseResourceDescriptor must be less than or equal to the original size
    (Enforced).
    The resulting BaseResourceDescriptor has a size equal to the difference between the original size and
    the new size, and an alignment equal to the original alignment.
    \param rhs the BaseResourceDescriptor to add to this one.
    */
    void operator-=(const BaseResourceDescriptor& rhs)
    {
        rwFUNCTION();
        rwASSERT(rhs.m_size <= m_size);
        m_size -= rhs.m_size;
        rwRETURNVOID();
    }

    /**
    Subtract a BaseResourceDescriptor from another.
    
    The size of the rhs BaseResourceDescriptor must be less than or equal to the lhs size
    (Enforced).
    The resulting BaseResourceDescriptor has a size equal to the difference between the lhs size and
    the new size, and an alignment equal to the lhs alignment.
    The alignment of the result is the alignment of the lhs.
    \param rhs the BaseResourceDescriptor to subtract this one.
    */
    BaseResourceDescriptor operator-(const BaseResourceDescriptor& rhs) const
    {
        rwFUNCTION();
        BaseResourceDescriptor diff = *this;
        diff -= rhs;
        rwRETURN(diff);
    }

private:

    uint32_t m_size;
    uint32_t m_alignment;
};


// Inline Functions

/**
Check two BaseResourceDescriptor objects for equality.
\return true if the two objects have the same size and alignment.
*/
inline bool operator==(const BaseResourceDescriptor& lhs, const BaseResourceDescriptor& rhs)
{
    rwFUNCTION();
    rwRETURN((lhs.GetSize() == rhs.GetSize()) && (lhs.GetAlignment() == rhs.GetAlignment()));
}

/**
Check two BaseResourceDescriptor objects for inequality.
\return true if the two objects have different size or alignment.
*/
inline bool operator!=(const BaseResourceDescriptor& lhs, const BaseResourceDescriptor& rhs)
{
    rwFUNCTION();
    rwRETURN(!(lhs == rhs));
}

/**
Do a less than comparison between two BaseResourceDescriptors

 Only the size component of the BaseResourceDescriptor is compared. The alignment is ignored.
\param rhs the BaseResourceDescriptor to compare to this one.
\param lhs the BaseResourceDescriptor that the rhs is getting compared to.
\return true if the size is < to that provided by the rhs.
*/
inline bool operator<(const BaseResourceDescriptor& lhs, const BaseResourceDescriptor& rhs)
{
    rwFUNCTION();
    rwRETURN(lhs.GetSize() < rhs.GetSize());
}

/**
Do a less than or equal to comparison between two BaseResourceDescriptors

 Only the size component of the BaseResourceDescriptor is compared. The alignment is ignored.
\param rhs the BaseResourceDescriptor to compare to this one.
\param lhs the BaseResourceDescriptor that the rhs is getting compared to.
\return true if the size is <= to that provided by the rhs.
*/
inline bool operator<=(const BaseResourceDescriptor& lhs, const BaseResourceDescriptor& rhs)
{
    rwFUNCTION();
    rwRETURN(lhs.GetSize() <= rhs.GetSize());
}



/**
A BaseResourceDescriptorType is a templated class to allow unique types to be used for specifying
BaseResourceDescriptors for different base resource types.
It is important that objects of type BaseResourceDescriptorType have the same size as BaseResourceDescriptor
so that we can freely reinterpret cast between the two without slicing. For this reason
BaseResourceDescriptorType may not add any new members or virtual functions to BaseResourceDescriptor.
*/
template <uint32_t typeId>
class BaseResourceDescriptorType : public BaseResourceDescriptor
{
public:
    /**
    Construct a default BaseResourceDescriptorType.
    */
    BaseResourceDescriptorType()
      : BaseResourceDescriptor()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    Construct a BaseResourceDescriptorType from a size and alignment.
    \param size the size in bytes.
    \param alignment the alignment in bytes.
    */
    BaseResourceDescriptorType(size_t size, uint32_t alignment)
      : BaseResourceDescriptor(size, alignment)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    Destruct a BaseResourceDescriptorType.
    */
    ~BaseResourceDescriptorType()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    // default copy constructor and assignment operators are sufficient
    // BaseResourceDescriptorType(const BaseResourceDescriptorType& other);
    // BaseResourceDescriptorType operator=(const BaseResourceDescriptorType& other);
};




typedef void* BaseResource;

/**
A templated object a pointer to a single memory block.
This is templated so we can instantiate a different object type for each memory type.
*/
template <uint32_t typeId>
class BaseResourceType
{
public:
    /**
    Construct a default BaseResourceType.
    */
    BaseResourceType(void* resource = 0)
      : m_resource(resource)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    Destruct a BaseResourceType.
    */
    ~BaseResourceType()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    // default copy constructor and assignment operators are sufficient
    // BaseResourceType(const BaseResourceType& other);
    // BaseResourceType operator=(const BaseResourceType& other);

    operator void*()
    {
        rwFUNCTION();
        rwRETURN(m_resource);
    }

    operator const void*() const
    {
        rwFUNCTION();
        rwRETURN(const_cast<const void*>(m_resource));
    }

private:
    void* m_resource;
};

} //namespace rw

#endif // RW_BASERESOURCE_HPP
