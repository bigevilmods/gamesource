// (c) Electronic Arts. All Rights Reserved.

/// \file
/// PS3 implementation of rw::core::debug::detail::DebugCriticalSection class

#include <stdlib.h>
#include <kernel.h>
#include "rw/core/debug/detail/debugcriticalsection.h"
#include "rw/core/debug/debug.h"

namespace rw {
    namespace core {
        namespace debug {
            namespace detail {

                void DebugCriticalSection::Create()
                {
					rwASSERT(sizeof(ScePthreadMutex)<=HOST_CRITICAL_SECTION_SIZE);
					scePthreadMutexInit(reinterpret_cast<ScePthreadMutex*>(mData),0,0);
                    mValid = true;
                }

                void DebugCriticalSection::Destroy()
                {
                    if (mValid)
                    {
						scePthreadMutexDestroy(reinterpret_cast<ScePthreadMutex*>(mData));
                        mValid = false;
                    }
                }

                void DebugCriticalSection::Enter()
                {
                    if (mValid)
                        scePthreadMutexLock(reinterpret_cast<ScePthreadMutex*>(mData));
                }

                void DebugCriticalSection::Leave()
                {
                    if (mValid)
                        scePthreadMutexUnlock(reinterpret_cast<ScePthreadMutex*>(mData));
                }

            }
        }
    }
}
