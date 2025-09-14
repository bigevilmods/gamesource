// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Xbox2 implementation of rw::core::debug::detail::DebugCriticalSection class

#include <xtl.h>
#include "rw/core/debug/detail/debugcriticalsection.h"
#include "rw/core/debug/debug.h"

namespace rw
{
    namespace core {
        namespace debug {
            namespace detail {

                void DebugCriticalSection::Create()
                {
                    rwFUNCTION();
                    rwASSERTATCOMPILETIME(sizeof(CRITICAL_SECTION) <= sizeof(mData), CRITICAL_SECTION_unexpected_size);
                    InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(mData));
                    mValid = true;
                    rwRETURNVOID();
                }

                void DebugCriticalSection::Destroy()
                {
                    rwFUNCTION();
                    if (mValid)
                    {
                        DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(mData));
                        mValid = false;
                    }
                    rwRETURNVOID();
                }

                void DebugCriticalSection::Enter()
                {
                    rwFUNCTION();
                    if (mValid)
                        EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(mData));
                    rwRETURNVOID();
                }

                void DebugCriticalSection::Leave()
                {
                    rwFUNCTION();
                    if (mValid)
                        LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(mData));
                    rwRETURNVOID();
                }

            }
        }
    }
}
