// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_FILESYS_HANDLECACHE_H
#define RW_CORE_FILESYS_HANDLECACHE_H

#include "EABase/eabase.h"
#include "EAAssert/eaassert.h"
#include "EASTL/intrusive_list.h"
#include "eathread/eathread_futex.h"

namespace rw {
namespace core {
namespace filesys {

#if defined(EA_PLATFORM_PS3)

class DeviceFileHandle;
typedef uintptr_t PlatformFileHandle;

struct PlatformFileHandleCacheEntry : public eastl::intrusive_list_node
{
    static const PlatformFileHandle INVALID_FILE_DESCRIPTOR = (PlatformFileHandle)-1;

    PlatformFileHandleCacheEntry()
        : mAssociatedHandle(NULL),
          mFileDescriptor(INVALID_FILE_DESCRIPTOR),
          mContext(NULL)
    {}

    DeviceFileHandle *mAssociatedHandle;

    // A uintptr_t ought to handle the cases of file handles being either integer or 
    // pointer sized. 
    PlatformFileHandle mFileDescriptor;
    void *mContext;

    inline bool IsValid() const { return mFileDescriptor != INVALID_FILE_DESCRIPTOR; }
    inline void Invalidate() { mFileDescriptor = INVALID_FILE_DESCRIPTOR; }
};


class PlatformFileHandleCache
{
public:
    PlatformFileHandleCache();

    PlatformFileHandleCacheEntry *RefreshHandle(PlatformFileHandleCacheEntry *entry, DeviceFileHandle *handle);
    void ReleaseCacheEntry(PlatformFileHandleCacheEntry *entry, DeviceFileHandle *handle);

    static const int HANDLE_CACHE_SIZE = 15;

private:
    PlatformFileHandleCache(const PlatformFileHandleCache &);
    PlatformFileHandleCache &operator =(const PlatformFileHandleCache &);

    PlatformFileHandleCacheEntry *AllocateCacheEntry();
    void MoveToUsedListFront(PlatformFileHandleCacheEntry *entry);

    // This static method is implemented by the platform specific back end as it requires calling SDK APIs
    // and cannot easily be made generic.
    static void CloseHandle(void *context, PlatformFileHandle fileHandle);

    PlatformFileHandleCacheEntry mHandles[HANDLE_CACHE_SIZE];
    eastl::intrusive_list<PlatformFileHandleCacheEntry> mFreeList;
    eastl::intrusive_list<PlatformFileHandleCacheEntry> mUsedHandles;
    EA::Thread::Futex mFutex;
};

#endif

}}}

#endif
