// (c) Electronic Arts. All Rights Reserved.

/// \file
/// PS2 implementation of rw::core::debug::detail::DebugCriticalSection class

#include "rwsharedheaders.hpp"
#include <eekernel.h>
#include <new>  // for placement new

#include "rw/core/debug/detail/debugcriticalsection.h"

namespace rw {
    namespace core {
        namespace debug {
            namespace detail {

                struct Ps2RecursiveMutex
                {
                    Ps2RecursiveMutex(int sema) :
                        mSema(sema), mThreadId(0), mLockCount(0)
                    { }

                    int mSema;
                    int mThreadId;
                    int mLockCount;
                };

                void DebugCriticalSection::Create()
                {
                    struct SemaParam sp;
                    sp.maxCount = 1;
                    sp.initCount  = 1;

                    int semaId = CreateSema(&sp);
                    new (mData) Ps2RecursiveMutex(semaId);
                    mValid = true;
                }

                void DebugCriticalSection::Destroy()
                {
                    if (mValid)
                    {
                        DeleteSema(reinterpret_cast<Ps2RecursiveMutex *>(mData)->mSema);
                        mValid = false;
                    }
                }

                void DebugCriticalSection::Enter()
                {
                    Ps2RecursiveMutex *m = reinterpret_cast<Ps2RecursiveMutex *>(mData);

                    if (mValid)
                    {
                        if ((m->mLockCount > 0) && (m->mThreadId == ::GetThreadId()))
                        {
                            ++m->mLockCount;
                            rwRETURNVOID();
                        }

                        WaitSema(m->mSema);
                        m->mThreadId = ::GetThreadId();
                        ++m->mLockCount;
                    }
                }

                void DebugCriticalSection::Leave()
                {
                    Ps2RecursiveMutex *m = reinterpret_cast<Ps2RecursiveMutex *>(mData);

                    if (mValid)
                    {
                        if (--m->mLockCount == 0)
                        {
                            m->mThreadId = 0;
                            SignalSema(m->mSema);
                        }
                    }
                }

            }
        }
    }
}
