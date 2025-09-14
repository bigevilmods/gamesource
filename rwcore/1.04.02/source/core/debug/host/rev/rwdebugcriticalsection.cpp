// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Wii implementation of rw::core::debug::detail::DebugCriticalSection class

#include "rw/core/debug/detail/debugcriticalsection.h"

#include <stdlib.h>
#include <revolution/os.h>

namespace rw {
    namespace core {
        namespace debug {
            namespace detail {

                void DebugCriticalSection::Create()
                {
                    OSInitMutex(reinterpret_cast<OSMutex *>(mData));
                    mValid = true;
                }

                void DebugCriticalSection::Destroy()
                {
                    if (mValid)
                    {
                        // There is no destroy system call on Wii.
                        mValid = false;
                    }
                }

                void DebugCriticalSection::Enter()
                {
                    if (mValid)
                        OSLockMutex(reinterpret_cast<OSMutex *>(mData));
                }

                void DebugCriticalSection::Leave()
                {
                    if (mValid)
                        OSUnlockMutex(reinterpret_cast<OSMutex *>(mData));
                }

            }
        }
    }
}
