// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Private mutex class for debugging system

#ifndef RW_CORE_DEBUG_DETAILS_CRITICALSECTION_H
#define RW_CORE_DEBUG_DETAILS_CRITICALSECTION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <rw/core/base/ostypes.h>   // bool32_t

namespace rw
{
    namespace core
    {
        namespace debug
        {
            namespace detail
            {
                // CRITICAL_SECTION is 24 bytes on Win32 & Wii
                // 28 bytes on Xbox 360, 4 on PS2 & PS3.
                // So we're wasting a bit of space here, but since
                // it's just for debug purposes, it shouldn't
                // matter much.
#if defined(EA_PLATFORM_WINDOWS) && defined(_WIN64) ||defined(EA_PLATFORM_XBOXONE) 
                const int HOST_CRITICAL_SECTION_SIZE = 40;
#else
                const int HOST_CRITICAL_SECTION_SIZE = 32;
#endif

                // Wrapper class for the underlying system's Critical
                // Section/Mutex API.  We don't use the one in the
                // thread module because that would create a circular
                // dependency between the debug and thread modules.
                class DebugCriticalSection
                {
                public:
                    DebugCriticalSection() : mValid(false) {}
                    ~DebugCriticalSection() {}

                    // Do not create in constructor and destroy in destructor because the
                    // channel or device will not necessarily be thread-safe.  If it is,
                    // Create will be need to be called when the channel or device is
                    // created, and Destroy will need to be called when the channel or
                    // device is destroyed.
                    void Create();
                    void Destroy();

                    // Enter and Leave can be called whether the CS is initialized or not.
                    // If the CS is not initialized, they will do nothing.
                    void Enter();
                    void Leave();

                private:

                    bool32_t mValid;    // needs to be 4-byte aligned so that mData is also 4-byte-aligned.

                    // This will be cast internally to the host-specific
                    // Critical Section or Mutex type
                    char mData[HOST_CRITICAL_SECTION_SIZE];
                };
            } // namespace detail
        } // namespace debug
    } // namespace core
} // namespace rw

#endif // RW_CORE_DEBUG_CHANNEL_H
