// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \brief        PS3 host1 driver implementation interface file.

#ifndef RW_CORE_FILESYS_DETAIL_PRIVATE_HOST1_DEVICE_H
#define RW_CORE_FILESYS_DETAIL_PRIVATE_HOST1_DEVICE_H

#include "eathread/eathread_futex.h"
#include "rw/core/filesys/device.h"

namespace rw {
namespace core {
namespace filesys {

/*! \brief PSP2 rwfilesystem DeviceDriver implementation
*/
class PSP2FileDeviceDriver : public DeviceDriver
{
public:
    PSP2FileDeviceDriver();

    virtual ~PSP2FileDeviceDriver();

    virtual void*           Open(const char *name, unsigned int oflags, Handle **pParentFileHandle);
    virtual void            Close(void * h);
    virtual unsigned int    Read(void *h, void *buf, unsigned int bufsize, DeviceDriver *ddParent, void * ddFileHandle);
    virtual unsigned int    Write(void *h, const void *buf, unsigned int bufsize, DeviceDriver *dd, void *dfh);
    virtual uint64_t        Seek(void *h, uint64_t offset, int whence, DeviceDriver *ddParent, void *ddFileHandle);
    virtual uint64_t        GetSize(void *h);
    virtual bool            Resize(void *h, uint64_t newsize);
    virtual bool            Delete(const char *filePath);
    virtual bool            Move(const char *currentName, const char *newName);
    virtual void *          FindBegin(const char *filePattern, FindData *findData);
    virtual bool            FindNext(void *h, FindData *findData);
    virtual bool            FindEnd(void *h);
    virtual bool            DirectoryCreate(const char *directory);
    virtual bool            DirectoryRemove(const char *directory);
    virtual bool            GetNameFromHandle(void *h, char *name, size_t nameBufferSize);
};

extern PSP2FileDeviceDriver gDeviceDriverImpl;

}}}

#endif
