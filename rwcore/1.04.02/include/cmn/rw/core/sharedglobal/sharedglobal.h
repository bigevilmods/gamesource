// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Definition of SharedGlobal template.

#ifndef PUBLIC_RW_SHAREDGLOBAL_HPP
#define PUBLIC_RW_SHAREDGLOBAL_HPP

#if defined(RWCROSS) && defined(rwHOST_WIN)
#include <rw/core/base/ostypes.h>
#include "rw/core/memory/memoryfunctions.h"
#include "rw/core/sharedglobal/detail/eaosglobal.h"
#include "rw/core/string.h"
#include "rw/core/stltypes.h"

namespace rw
{

template<class T, uint32_t ID>
class SharedGlobal
{
public:
    T& Data()
    {
        // In cross builds we use a shared global
        static rw::shared_globals::internal::AutoOSGlobalPtr<T, ID> sSharedData;
        return *sSharedData;
    }
};

} // namespace rw

#else // native

namespace rw
{

template<class T, uint32_t ID>
class SharedGlobal
{
public:
    T& Data()
    {
        // In native builds we simply hold the data
        static T sData;
        return sData;
    }
};

} // namespace rw

#endif // defined(RWCROSS) && defined(rwHOST_WIN)

#endif // PUBLIC_RW_SHAREDGLOBALS_HPP
