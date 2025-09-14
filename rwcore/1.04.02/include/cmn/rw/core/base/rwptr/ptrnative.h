// (c) Electronic Arts. All Rights Reserved.

/**
\file
Uses the native T* for RWPTR.
*/

// NB: this has to be outside the include guard
// since ostypes.h include ptrnative.h
#include <rw/core/base/ostypes.h>
#ifndef RW_RWPTRNATIVE_HPP
#define RW_RWPTRNATIVE_HPP
// ***********************************************************************************************************
// Includes

#if defined(rwUSERWPTRNATIVE)

// ***********************************************************************************************************
// Defines
#define RWPTR(T) T*
#define RWCONSTPTR(T) const T*
#define RWDECLARE_TYPE_PTRS(T) typedef RWPTR(T) T##Ptr; typedef RWCONSTPTR(T) T##ConstPtr
#define RWFUNCPTR(T) T

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
template <typename T>
inline rw::TargetPtr RwPtrToOffset(const RWPTR(T) ptr)
{
    rwFUNCTION();
    rwRETURN(static_cast<rw::TargetPtr>(reinterpret_cast<uintptr_t>(ptr)));
}

template <typename T>
inline RWPTR(T) RwPtrFromOffset(rw::TargetPtr offset)
{
    rwFUNCTION();
    rwRETURN(reinterpret_cast<T*>(offset));
}

template <typename T>
inline RWPTR(T) RwPtrAddOffset(RWPTR(T) ptr, rw::TargetPtr offset)
{
    rwFUNCTION();
    rwRETURN(RwPtrFromOffset<T>(RwPtrToOffset(ptr) + offset));
}

template <typename T>
inline RWPTR(T) RwPtrSubtractOffset(RWPTR(T) ptr, rw::TargetPtr offset)
{
    rwFUNCTION();
    rwRETURN(RwPtrFromOffset<T>(RwPtrToOffset(ptr) - offset));
}

template <typename ValueType>
RWPTR(ValueType) RwPtrAddBasePtr(RWPTR(ValueType) ptr, RWPTR(const void) base)
{
    rwFUNCTION();
    rwRETURN(RwPtrAddOffset(ptr, RwPtrToOffset(base)));
}

template <typename ValueType>
RWPTR(ValueType) RwPtrSubtractBasePtr(RWPTR(ValueType) ptr, RWPTR(const void) base)
{
    rwFUNCTION();
    rwRETURN(RwPtrSubtractOffset(ptr, RwPtrToOffset(base)));
}

template <typename T>
inline T* RwPtrToHostPtr(RWPTR(T) ptr)
{
    rwFUNCTION();
    rwRETURN(ptr);
}

template <typename T>
class RwPtrReinterpretCast
{
public:
    template <typename U>
    RwPtrReinterpretCast(RWPTR(U) other)
      : m_ptr(reinterpret_cast<T*>(other))
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    operator RWPTR(T)()const
    {
        rwFUNCTION();
        rwRETURN(m_ptr);
    }

private:
    RWPTR(T) m_ptr;
};


// ***********************************************************************************************************
// Structs + Unions + Classes

// Inline Functions

// Public Enums, Consts, Typedefs and Static variables

// Interface Implementations

// Methods

} //namespace rw

#endif // defined(rwUSERWPTRNATIVE)

#endif // RW_RWPTRNATIVE_HPP
