// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_FILESYS_INTERNAL_DEVICE_WII_H
#define RW_CORE_FILESYS_INTERNAL_DEVICE_WII_H

#include <string.h>
#include <revolution.h>

#include "rw/core/filesys/device.h"

namespace rw {
namespace core {
namespace filesys {

/*! \brief Wii rwfilesystem DeviceDriver implementation

    Client code may use this as a base class or declare
    additional instances of it.  Write() is not implemented
    by this driver.
*/
class WiiDvdFileDeviceDriver : public DeviceDriver
{
public:
    WiiDvdFileDeviceDriver() {}

    virtual ~WiiDvdFileDeviceDriver() {}

private:
    virtual bool        Init();

    virtual void *      Open(const char *name, unsigned int oflags, Handle **);
    virtual void        Close(void *h);
    virtual unsigned int Read(void *h, void *buf, unsigned int bufsize, DeviceDriver *, void *);
    virtual uint64_t    Seek(void *h, uint64_t offset, int whence, DeviceDriver *, void *);
    virtual uint64_t    GetSize(void *h);
    virtual void*       FindBegin(const char *, FindData *);
    virtual bool        FindNext(void *, FindData *);
    virtual bool        FindEnd(void *);
    virtual bool        GetNameFromHandle(void *handle, char *name, size_t nameBufferSize);
};

extern WiiDvdFileDeviceDriver gDeviceDriverImpl;

}
}
}

#endif // RW_CORE_FILESYS_INTERNAL_DEVICE_WII_H
