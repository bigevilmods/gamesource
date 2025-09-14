// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \brief        PS3 host1 driver implementation interface file.

#ifndef RW_CORE_FILESYS_DETAIL_PRIVATE_HOST1_DEVICE_H
#define RW_CORE_FILESYS_DETAIL_PRIVATE_HOST1_DEVICE_H

#include "eathread/eathread_futex.h"
#include "rw/core/filesys/device.h"

#include <np/drm.h>     // SceNpDrmOpenArg

namespace rw {
namespace core {
namespace filesys {

/*! \brief PS3 rwfilesystem DeviceDriver implementation

    Client code may use this as a base class or declare
    additional instances of it.  On the PS3, there are
    two physical devices - the BluRay drive and the
    Hard Disk.  Each has a single driver, but the hard
    disk has two roots (/dev_hdd0 and /dev_hdd1).
*/
class PS3FileDeviceDriver : public DeviceDriver
{
public:
    PS3FileDeviceDriver();

    virtual ~PS3FileDeviceDriver();

    void SetThroughputLimit(unsigned int KBperSec);

    void                UseDRM(bool enable, const SceNpDrmKey *key,
        bool (*TryDrmFile)(const char *name));
	void                AppendEdatForDrmFlaggedFiles(bool enable);
	void                AppendSdatForDrmFlaggedFiles(bool enable);

protected:
    virtual void *      Open(const char *name, unsigned int oflags, Handle **);

private:
    virtual bool        Init();
    using DeviceDriver::Close;
    virtual void        Close(void * h, DeviceDriver *, void *);
    virtual uint32_t    Read(void * h, void *buf, uint32_t bufsize, DeviceDriver *, void *);
    virtual uint32_t    Write(void * h, const void *buf, uint32_t bufsize, DeviceDriver *, void *);
    virtual uint64_t    Seek(void * h, uint64_t offset, int whence, DeviceDriver *, void *);
    using DeviceDriver::GetSize;
    virtual uint64_t    GetSize(void *h, DeviceDriver *, void *);
    virtual bool        Delete(const char *name);
    virtual bool        Move(const char *, const char *);
    virtual void *      FindBegin(const char *filePattern, FindData *findData);
    virtual bool        FindNext(void *h, FindData *findData);
    virtual bool        FindEnd(void *h);
    virtual bool        DirectoryCreate(const char *directory);
    virtual bool        DirectoryRemove(const char *directory);
    virtual bool        GetNameFromHandle(void *handle, char *name, size_t nameBufferSize);
    virtual unsigned int GetMaxReadSize() { return 512*1024u; }

    static bool         DefaultFileErrorCallback(int, unsigned int, DeviceDriver *, const char *);
    bool                DiscErrorHandler(int errorNumber, unsigned int FileErrorFunctionNum, const char *);

    static bool         DefaultTryDrmFile(const char *);
    void                ThrottleRead();

    // For throughput-throttling
    unsigned int            mThroughputLimit;
    unsigned int            mLastReadSize;
    int64_t                 mLastReadTime;

    SceNpDrmKey             mDrmKey;
    bool                    mIsDrmEnabled;
    bool                  (*mTryDrmFile)(const char *name);

   	bool                    mAutoAppendEdat;
	bool                    mAutoAppendSdat;
};

extern PS3FileDeviceDriver gDeviceDriverImpl;
extern PS3FileDeviceDriver gHddDriverImpl;

}}}

#endif
