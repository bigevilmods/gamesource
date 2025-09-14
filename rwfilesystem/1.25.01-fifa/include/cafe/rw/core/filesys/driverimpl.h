// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_FILESYS_INTERNAL_DEVICE_CAFE_H
#define RW_CORE_FILESYS_INTERNAL_DEVICE_CAFE_H

#include <string.h>
#include <cafe/fs.h>

#include "rw/core/filesys/device.h"

namespace rw {
namespace core {
namespace filesys {

/*! \brief Cafe rwfilesystem DeviceDriver implementation

    Client code may use this as a base class or declare
    additional instances of it.  Write() is not implemented
    by this driver.
*/
class CafeDvdFileDeviceDriver : public DeviceDriver
{
public:
    CafeDvdFileDeviceDriver();

    virtual ~CafeDvdFileDeviceDriver();

    // Call SetClientHandle before CafeDvdFileDeviceDriver::Init() is called
    void SetClientHandle(FSClient *c);
    FSClient *GetClientHandle() const;
    bool IsValidPath(char* path);
    virtual void        Restore();

private:
    virtual bool        Init();
    virtual void *      Open(const char *name, unsigned int oflags, Handle **);
    virtual void        Close(void *h);
    virtual unsigned int Read(void *h, void *buf, unsigned int bufsize, DeviceDriver *, void *);
    virtual uint32_t    Write(void * h, const void *buf, uint32_t bufsize, DeviceDriver *, void *);
    virtual uint64_t    Seek(void *h, uint64_t offset, int whence, DeviceDriver *, void *);
    virtual uint64_t    GetSize(void *h);
    virtual bool        Delete(const char *name);
    virtual bool        Move(const char *oldName, const char *newName);
    virtual void*       FindBegin(const char *, FindData *);
    virtual bool        FindNext(void *, FindData *);
    virtual bool        FindEnd(void *);
    virtual bool        DirectoryCreate(const char *directory);
    virtual bool        DirectoryRemove(const char *directory);
    virtual bool        GetNameFromHandle(void *handle, char *name, size_t nameBufferSize);

    FSClient            *mClient;
    void *mReadWriteBuffer;
};

extern CafeDvdFileDeviceDriver gDeviceDriverImpl;

}
}
}

#endif // RW_CORE_FILESYS_INTERNAL_DEVICE_CAFE_H
