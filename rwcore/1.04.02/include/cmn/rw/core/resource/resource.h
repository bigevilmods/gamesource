// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Platform independent resource classes for the resource descriptors and resources.

#ifndef RW_RESOURCE_HPP
#define RW_RESOURCE_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>
#include <rw/core/debug/exception.h>
#include <rw/core/resource/baseresource.h>

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


/**
 The base class for the platform dependent resource descriptors.
 */
template <uint32_t NumberOfBaseResourceTypes>
class BaseResourceDescriptors
{
public:
    typedef BaseResourceDescriptor* const_iterator;
    typedef BaseResourceDescriptor* iterator;

    /**
     Constructor taking main memory and disposable memory requirements.
     */
    BaseResourceDescriptors()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }


    // default copy constructor, assignment operator and destructors are okay
    // BaseResourceDescriptors(const BaseResourceDescriptors& other);
    // BaseResourceDescriptors operator=(const BaseResourceDescriptors& other);
    // ~BaseResourceDescriptors(const BaseResourceDescriptors& other);


    /**
     \return a reference to the ith memory resource descriptor.
     */
    const BaseResourceDescriptor& GetBaseResourceDescriptor(uint32_t i) const
    {
        rwFUNCTION();
        rwRETURN(m_baseResourceDescriptors[i]);
    }

    /**
     \return a reference to the ith memory resource descriptor.
     */
    BaseResourceDescriptor& GetBaseResourceDescriptor(uint32_t i)
    {
        rwFUNCTION();
        rwRETURN(m_baseResourceDescriptors[i]);
    }


    /**
     \return the number of base resource types.
     */
    static uint32_t GetNumberOfBaseResourceTypes()
    {
        rwFUNCTION();
        rwRETURN(NumberOfBaseResourceTypes);
    }


    /**
     \return an iterator to the first memory resource descriptor
     */
    const_iterator begin() const
    {
        rwFUNCTION();
        rwRETURN(m_baseResourceDescriptors);
    }

    /**
     \return an iterator to the first memory resource descriptor
     */
    iterator begin()
    {
        rwFUNCTION();
        rwRETURN(m_baseResourceDescriptors);
    }


    /**
     \return an iterator to just passed the last memory resource descriptor
     */
    const_iterator end() const
    {
        rwFUNCTION();
        rwRETURN(m_baseResourceDescriptors + NumberOfBaseResourceTypes);
    }

    /**
     \return an iterator to just passed the last memory resource descriptor
     */
    iterator end()
    {
        rwFUNCTION();
        rwRETURN(m_baseResourceDescriptors + NumberOfBaseResourceTypes);
    }


    /**
     Endian swap the components of the BaseResourceDescriptors if required for the target.
     */
    void
    EndianSwap()
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
        {
            m_baseResourceDescriptors[i].EndianSwap();
        }
        rwRETURNVOID();
    }

    /**
     Align the ResourceDescriptor to the alignment requirements of the passed in ResourceDescriptor.
     Each memory resource is aligned to the requirements of its respective memory resource descriptor.
     \param alignmentDescriptor The ResourceDescriptor dictating the alignment requirements.
     */
    void Align(const BaseResourceDescriptors& alignmentDescriptor)
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
        {
            m_baseResourceDescriptors[i].Align(alignmentDescriptor);
        }
        rwRETURNVOID();
    }

    /**
     Flatten the ResourceDescriptor so that all of the memory requirements can be allocated in main memory.
     The main memory base resource descriptor will be the sum
     of the memory requirements (allowing for padding for alignment), with an alignment equal to the maximum
     base resource descriptor alignment.
     */
    void Flatten()
    {
        rwFUNCTION();
        BaseResourceDescriptor totalMemoryResDesc;
        for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
        {
            totalMemoryResDesc += GetBaseResourceDescriptor(i);
            GetBaseResourceDescriptor(i) = BaseResourceDescriptor();
        }
        GetBaseResourceDescriptor(0) = totalMemoryResDesc;
        rwRETURNVOID();
    }

    /**
    Add a BaseResourceDescriptors to this one.
    Each internal BaseResourceDescriptor is added with the respective memory resource descriptor from rhs.
    \param rhs the BaseResourceDescriptors to add to this one.
    */
    void operator+=(const BaseResourceDescriptors& rhs)
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
        {
            m_baseResourceDescriptors[i] += rhs.m_baseResourceDescriptors[i];
        }
        rwRETURNVOID();
    }

    /**
    Subtract a BaseResourceDescriptors to this one. The result is a
    \param rhs the BaseResourceDescriptors to subtrac from this one.
    */
    void operator-=(const BaseResourceDescriptors& rhs)
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
        {
            m_baseResourceDescriptors[i] -= rhs.m_baseResourceDescriptors[i];
        }
        rwRETURNVOID();
    }

private:
    BaseResourceDescriptor m_baseResourceDescriptors[NumberOfBaseResourceTypes];
};

/**
Check if a BaseResourceDescriptors is equal to this BaseResourceDescriptors.
\param lhs the BaseResourceDescriptors to compare
\param rhs the BaseResourceDescriptors to compare
\return true if the existing mainMemoryResourceDescriptor is == to that provided by the rhs and
             if the existing disposableResourceDescriptor is == to that provided by the rhs.
*/
template <uint32_t NumberOfBaseResourceTypes>
RwBool
operator==(const BaseResourceDescriptors<NumberOfBaseResourceTypes>& lhs,
           const BaseResourceDescriptors<NumberOfBaseResourceTypes>& rhs)
{
    rwFUNCTION();
    for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
    {
        if (lhs.GetBaseResourceDescriptor(i) != rhs.GetBaseResourceDescriptor(i))
        {
            rwRETURN(FALSE);
        }
    }
    rwRETURN(TRUE);
}

/**
Do a less than comparison between two BaseResources
\param lhs the BaseResourceDescriptors to compare
\param rhs the BaseResourceDescriptors to compare
\return true if all BaseResources in lhs are < than the respective BaseResources in rhs
*/
template <uint32_t NumberOfBaseResourceTypes>
RwBool
operator<(const BaseResourceDescriptors<NumberOfBaseResourceTypes>& lhs,
          const BaseResourceDescriptors<NumberOfBaseResourceTypes>& rhs)
{
    rwFUNCTION();
    for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
    {
        if (rhs.GetBaseResourceDescriptor(i) <= lhs.GetBaseResourceDescriptor(i))
        {
            rwRETURN(FALSE);
        }
    }
    rwRETURN(TRUE);
}

/**
Do a less than or equal to comparison between two BaseResources
\param lhs the BaseResourceDescriptors to compare
\param rhs the BaseResourceDescriptors to compare
\return true if all BaseResources in lhs are < than the respective BaseResources in rhs
*/
template <uint32_t NumberOfBaseResourceTypes>
RwBool
operator<=(const BaseResourceDescriptors<NumberOfBaseResourceTypes>& lhs,
           const BaseResourceDescriptors<NumberOfBaseResourceTypes>& rhs)
{
    rwFUNCTION();
    for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
    {
        if (rhs.GetBaseResourceDescriptor(i) < lhs.GetBaseResourceDescriptor(i))
        {
            rwRETURN(FALSE);
        }
    }
    rwRETURN(TRUE);
}



/**
 The base class for the platform dependent resources.
 */
template <uint32_t NumberOfBaseResourceTypes>
class BaseResources
{
public:
    typedef BaseResource* const_iterator;
    typedef BaseResource* iterator;

    /**
     Constructor taking main memory and disposable memory requirements.
     */
    BaseResources()
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
        {
            m_baseResources[i] = BaseResource((void*)0);
        }
        rwRETURNVOID();
    }

    // default copy constructor, assignment operator and destructors are okay
    // BaseResources(const BaseResources& other);
    // BaseResources operator=(const BaseResources& other);
    // ~BaseResources();


    /**
     \return true if the Resource was correctly allocated.
     */
    operator bool() const
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
        {
            if (m_baseResources[i])
            {
                rwRETURN(TRUE);
            }
        }
        rwRETURN(FALSE);
    }


    /**
     \return a reference to the ith memory resource descriptor.
     */
    const BaseResource& GetBaseResource(uint32_t i) const
    {
        rwFUNCTION();
        rwRETURN(m_baseResources[i]);
    }

    /**
     \return a reference to the ith memory resource descriptor.
     */
    BaseResource& GetBaseResource(uint32_t i)
    {
        rwFUNCTION();
        rwRETURN(m_baseResources[i]);
    }


    /**
     \return the number of base resource types.
     */
    static uint32_t GetNumberOfBaseResourceTypes()
    {
        rwFUNCTION();
        rwRETURN(NumberOfBaseResourceTypes);
    }


    /**
     \return an iterator to the first memory resource descriptor
     */
    const_iterator begin() const
    {
        rwFUNCTION();
        rwRETURN(m_baseResources);
    }

    /**
     \return an iterator to the first memory resource descriptor
     */
    iterator begin()
    {
        rwFUNCTION();
        rwRETURN(m_baseResources);
    }


    /**
     \return an iterator to just passed the last memory resource descriptor
     */
    const_iterator end() const
    {
        rwFUNCTION();
        rwRETURN(m_baseResources + NumberOfBaseResourceTypes);
    }

    /**
     \return an iterator to just passed the last memory resource descriptor
     */
    iterator end()
    {
        rwFUNCTION();
        rwRETURN(m_baseResources + NumberOfBaseResourceTypes);
    }


    /**
     Is the Resource aligned to the alignment requirements of the passed in ResourceDescriptor.
     \param alignmentDescriptor The Resource dictating the alignment requirements.
     \return true if the alignment of the base resources matches the respective memory resource descriptor.
     */
    RwBool IsAligned(const BaseResourceDescriptors<NumberOfBaseResourceTypes>& alignmentDescriptor)
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
        {
            if (!IsMemAligned(m_baseResources[i],
                              alignmentDescriptor.GetBaseResourceDescriptor(i).GetAlignment()))
            {
                rwRETURN(FALSE);
            }
        }
        rwRETURN(TRUE);
    }


    /**
     Align the Resource to the alignment requirements of the passed in ResourceDescriptor.
     Each memory resource is aligned to the requirements of its respective memory resource descriptor.
     \param alignmentDescriptor The Resource dictating the alignment requirements.
     */
    void Align(const BaseResourceDescriptors<NumberOfBaseResourceTypes>& alignmentDescriptor)
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
        {
            m_baseResources[i] = MemAlign(m_baseResources[i],
                                          alignmentDescriptor.GetBaseResourceDescriptor(i).GetAlignment());
        }
        rwRETURNVOID();
    }


    /**
     Add a BaseResources to this one.
     Each internal BaseResource is added with the respective memory resource descriptor from rhs.
     \param rhs the BaseResourceDescriptors to add to this ResourceDescriptor.
     */
    void operator+=(const BaseResourceDescriptors<NumberOfBaseResourceTypes>& descriptor)
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
        {
            rwENFORCE(IsMemAligned(m_baseResources[i], descriptor.GetBaseResourceDescriptor(i).GetAlignment()),
                      ("Resource Descriptor %d is not aligned to the correct alignment.", i));
            m_baseResources[i] = MemAddOffset(m_baseResources[i],
                                              descriptor.GetBaseResourceDescriptor(i).GetSize());
        }
        rwRETURNVOID();
    }


    /**
     Construct a Resource from a pointer to a memory block and a ResourceDescriptor.
     \param ptr the start of the main memory base resource. This block of memory must be large enough
     to store all of the base resources as outlined in the resource descriptor.
     \param descriptor the size and alignment requirements of each base resource type.
     */
    void FromMemoryBlock(void* ptr, const BaseResourceDescriptors<NumberOfBaseResourceTypes>& descriptor)
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
        {
            if (descriptor.GetBaseResourceDescriptor(i).GetSize() > 0)
            {
                rwASSERT(IsMemAligned(ptr, descriptor.GetBaseResourceDescriptor(i).GetAlignment()));
                m_baseResources[i] = ptr;
                ptr = MemAddOffset(ptr, descriptor.GetBaseResourceDescriptor(i).GetSize());
            }
            else
            {
                m_baseResources[i] = (void*)0;
            }
        }
        rwRETURNVOID();
    }

private:
    BaseResource m_baseResources[NumberOfBaseResourceTypes];
};


/**
Check if a BaseResources is equal to this BaseResources.
\param lhs the BaseResources to compare
\param rhs the BaseResources to compare
\return true if the existing mainMemoryResource is == to that provided by the rhs and
             if the existing disposableResource is == to that provided by the rhs.
*/
template <uint32_t NumberOfBaseResourceTypes>
RwBool
operator==(const BaseResources<NumberOfBaseResourceTypes>& lhs,
           const BaseResources<NumberOfBaseResourceTypes>& rhs)
{
    rwFUNCTION();
    for (uint32_t i = 0; i < NumberOfBaseResourceTypes; ++i)
    {
        if (lhs.GetBaseResource(i) != rhs.GetBaseResource(i))
        {
            rwRETURN(FALSE);
        }
    }
    rwRETURN(TRUE);
}

/////////////////
// Inline Functions

} //namespace rw

#endif // RW_RESOURCEGENERIC_HPP
