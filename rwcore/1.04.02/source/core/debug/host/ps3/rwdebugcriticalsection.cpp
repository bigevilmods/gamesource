// (c) Electronic Arts. All Rights Reserved.

/// \file
/// PS3 implementation of rw::core::debug::detail::DebugCriticalSection class

#include <stdlib.h>
#include <sys/synchronization.h>
#include "rw/core/debug/detail/debugcriticalsection.h"
#include "rw/core/debug/debug.h"

namespace rw {
    namespace core {
        namespace debug {
            namespace detail {

                void DebugCriticalSection::Create()
                {
                    sys_mutex_attribute_t mutexParams;
                    sys_mutex_attribute_initialize(mutexParams);
                    mutexParams.attr_recursive = SYS_SYNC_RECURSIVE;
                    sys_mutex_create(reinterpret_cast<sys_mutex_t *>(mData), &mutexParams);
                    mValid = true;
                }

                void DebugCriticalSection::Destroy()
                {
                    if (mValid)
                    {
                        sys_mutex_destroy(*reinterpret_cast<sys_mutex_t *>(mData));
                        mValid = false;
                    }
                }

                void DebugCriticalSection::Enter()
                {
                    if (mValid)
                        sys_mutex_lock(*reinterpret_cast<sys_mutex_t *>(mData), 0);
                }

                void DebugCriticalSection::Leave()
                {
                    if (mValid)
                        sys_mutex_unlock(*reinterpret_cast<sys_mutex_t *>(mData));
                }

            }
        }
    }
}
