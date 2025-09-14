// (c) Electronic Arts. All Rights Reserved.

#include "private_rwplatformfilehandlecache.h"

namespace rw {
namespace core {
namespace filesys {

#if defined(EA_PLATFORM_PS3)

PlatformFileHandleCache::PlatformFileHandleCache()
    : mFreeList(),
      mUsedHandles(),
      mFutex()
{
    EA_ASSERT(mFreeList.begin() == mFreeList.end());
    EA_ASSERT(mUsedHandles.begin() == mUsedHandles.end());
    for (int i = 0; i < HANDLE_CACHE_SIZE; ++i)
        mFreeList.push_back(mHandles[i]);
    mUsedHandles.clear();
}


PlatformFileHandleCacheEntry *PlatformFileHandleCache::AllocateCacheEntry()
{
    if (mFreeList.begin() != mFreeList.end())
    {
        PlatformFileHandleCacheEntry *entry = &mFreeList.back();
        mFreeList.pop_back();
        mUsedHandles.push_front(*entry);

        return entry;
    }

    // If no free file descriptors are available then close the least-recently
    // used handle. It then moves to the head of the list used handle list and
    // its file descriptor is set to invalid.
    PlatformFileHandleCacheEntry *entry = &mUsedHandles.back();
    MoveToUsedListFront(entry);

    CloseHandle(entry->mContext, entry->mFileDescriptor);

    entry->Invalidate();

    return entry;
}


void PlatformFileHandleCache::MoveToUsedListFront(PlatformFileHandleCacheEntry *entry)
{
    eastl::intrusive_list<PlatformFileHandleCacheEntry>::remove(*entry);
    mUsedHandles.push_front(*entry);
}


PlatformFileHandleCacheEntry *PlatformFileHandleCache::RefreshHandle(PlatformFileHandleCacheEntry *entry, DeviceFileHandle *handle)
{
    EA::Thread::AutoFutex autoFutex(mFutex);

    // Ensure that the file descriptor hasn't been re-purposed for another handle.
    if (!entry || handle != entry->mAssociatedHandle)
    {
        entry = AllocateCacheEntry();
        entry->mAssociatedHandle = handle;

        EA_ASSERT(!entry->IsValid());
    }
    else
    {
        MoveToUsedListFront(entry);
    }

    return entry;
}


void PlatformFileHandleCache::ReleaseCacheEntry(PlatformFileHandleCacheEntry *entry, DeviceFileHandle *handle)
{
    EA::Thread::AutoFutex autoFutex(mFutex);

    if (handle != entry->mAssociatedHandle)
        return;

    if (entry->IsValid())
    {
        CloseHandle(entry->mContext, entry->mFileDescriptor);
        entry->Invalidate();
    }

    eastl::intrusive_list<PlatformFileHandleCacheEntry>::remove(*entry);
    mFreeList.push_front(*entry);
}

#endif

}}}
