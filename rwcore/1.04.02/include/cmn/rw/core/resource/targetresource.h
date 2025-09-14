// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Framework classes for a resource which matches the size, alignment and
/// layout of a resource on the target platform.

#ifndef RW_TARGETRESOURCE_HPP
#define RW_TARGETRESOURCE_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>
#include <rw/core/debug/exception.h>
#include <rw/core/resource/baseresource.h>
#include <rw/core/resource/resource.h>
#include <rw/core/resource/resourcetarget.h>

// ***********************************************************************************************************
// Includes

namespace rw
{

// ***********************************************************************************************************
// Defines

// ***********************************************************************************************************
// Forward Declarations

// ***********************************************************************************************************
// defs

// ***********************************************************************************************************
// Enums + Consts

// ***********************************************************************************************************
// Global Variables

// ***********************************************************************************************************
// External Functions Prototypes

// ***********************************************************************************************************
// Structs + Unions + Classes

// ***********************************************************************************************************
//                                                TargetResourceDescriptor Class Definition
// ***********************************************************************************************************

typedef RWPTR(void) TargetBaseResource;

/**
A templated object a pointer to a single memory block.
This is templated so we can instantiate a different object type for each memory type.
*/
class TargetResource
{
public:
    /**
    Construct a default TargetResource.
    */
    TargetResource()
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < ResourceDescriptor::GetNumberOfBaseResourceTypes(); ++i)
        {
            m_baseResources[i] = TargetBaseResource((void*)0);
        }
        rwRETURNVOID();
    }

    /**
    Construct a TargetResource from a Resource.
    */
    TargetResource(const Resource& resource)
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < ResourceDescriptor::GetNumberOfBaseResourceTypes(); ++i)
        {
            m_baseResources[i] = resource.GetBaseResource(i);
        }
        rwRETURNVOID();
    }

    /**
    Destruct a TargetResource.
    */
    ~TargetResource()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    // default copy constructor and assignment operators are sufficient
    // TargetResource(const TargetResource& other);
    // TargetResource operator=(const TargetResource& other);

    /**
    Get i-th TargetBaseResource
    */
    TargetBaseResource &
    GetTargetBaseResource(uint32_t i)
    {
        rwFUNCTION();
        rwRETURN(m_baseResources[i]);
    }

    /**
    Convert from a TargetResource to a Resource
    */
    Resource
    ToResource() const
    {
        rwFUNCTION();
        Resource resource;
        for (uint32_t i = 0; i < ResourceDescriptor::GetNumberOfBaseResourceTypes(); ++i)
        {
            resource.GetBaseResource(i) = RwPtrToHostPtr(m_baseResources[i]);
        }
        rwRETURN(resource);
    }

    /**
    Convert from a Resource to a TargetResource
    */
    void
    FromResource(const Resource& resource)
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < ResourceDescriptor::GetNumberOfBaseResourceTypes(); ++i)
        {
            m_baseResources[i] = resource.GetBaseResource(i);
        }
        rwRETURNVOID();
    }

    /**
    Endian Swap
    */
    void
    EndianSwap()
    {
        rwFUNCTION();
        for (uint32_t i = 0; i < ResourceDescriptor::GetNumberOfBaseResourceTypes(); ++i)
        {
            rw::EndianSwap(m_baseResources[i]);
        }
        rwRETURNVOID();
    }


private:
    TargetBaseResource m_baseResources[BASERESOURCE_NUMBEROFBASERESOURCETYPES];
};

} //namespace rw

#endif // RW_TARGETRESOURCE_HPP
