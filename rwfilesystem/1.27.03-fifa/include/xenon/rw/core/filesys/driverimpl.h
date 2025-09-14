// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_FILESYS_INTERNAL_DEVICE_XENON_H
#define RW_CORE_FILESYS_INTERNAL_DEVICE_XENON_H

#include "rw/core/filesys/directory.h"

namespace rw {
namespace core {
namespace filesys {

/*! \brief Xbox360 rwfilesystem DeviceDriver implementation

    Client code may use this as a base class or declare
    additional instances of it.  On the Xbox, there are
    two physical devices - the DVD drive (D:) and the
    Hard Disk (cache:)
*/
class XenonFileDeviceDriver : public DeviceDriver
{
public:
    XenonFileDeviceDriver();
    virtual ~XenonFileDeviceDriver() {};

    void                    SetThroughputLimit(unsigned int KBperSec);
    void                    SetQueryLocationEnabled(bool bEnable)
    { mQueryLocationEnabled = bEnable; }

protected:
	// Open() and Close() are protected to allow DLC-related subclasses to override.
    virtual void *          Open(const char *name, unsigned int oflags, Handle **ppdfh);
    virtual void            Close(void * h);
    using                   DeviceDriver::Close;

private:
    virtual unsigned int    Read(void * h, void *buf, unsigned int bufsize, DeviceDriver *dd, void *dfh);
    virtual unsigned int    Write(void * h, const void *buf, unsigned int bufsize, DeviceDriver *dd, void *dfh);
    virtual uint64_t        Seek(void * h, uint64_t offset, int whence, DeviceDriver *dd, void *dfh);
    virtual uint64_t        GetSize(void * h);
    using                   DeviceDriver::GetSize;
    virtual bool            Resize(void * h, uint64_t newsize);
    virtual uint64_t        QueryLocation(void * h);
    virtual bool            Delete(const char *filePath);
    virtual bool            Move(const char *currentName, const char *newName);
    virtual void *          FindBegin(const char *filePattern, FindData *findData);
    virtual bool            FindNext(void *h, FindData *findData);
    virtual bool            FindEnd(void *h);
    virtual bool            DirectoryCreate(const char *directory);
    virtual bool            DirectoryRemove(const char *directory);
    virtual bool            GetNameFromHandle(void *handle, char *name, size_t nameBufferSize);

    // For throughput-throttling
    unsigned int            mThroughputLimit;
    unsigned int            mLastReadSize;
    EA::Thread::ThreadTime  mLastReadTime;
    bool                    mQueryLocationEnabled;
};

extern XenonFileDeviceDriver gDeviceDriverImpl;
extern XenonFileDeviceDriver gCacheDriverImpl;

}}}

#endif
