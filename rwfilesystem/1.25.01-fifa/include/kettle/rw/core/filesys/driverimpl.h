// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_KETTLE_FILESYS_DRIVERIMPL_H
#define RW_CORE_KETTLE_FILESYS_DRIVERIMPL_H

#include "rw/core/filesys/device.h"

namespace rw {
namespace core {
namespace filesys {
    /*! \brief Kettle DeviceDriver implementation
    */
    class KettleFileDeviceDriver : public DeviceDriver
    {
    public:
        KettleFileDeviceDriver();

        virtual ~KettleFileDeviceDriver() {};

        virtual void *          Open(const char *name, unsigned int oflags, Handle **ppdfh);
        virtual void            Close(void * h);
        virtual unsigned int    Read(void * h, void *buf, unsigned int bufsize, DeviceDriver *dd, void *dfh);
        virtual unsigned int    Write(void * h, const void *buf, unsigned int bufsize, DeviceDriver *dd, void *dfh);
        virtual uint64_t        Seek(void * h, uint64_t offset, int whence, DeviceDriver *dd, void *dfh);
        virtual uint64_t        GetSize(void * h);
        virtual bool            Resize(void * h, uint64_t newsize);
        virtual bool            Delete(const char *filePath);
        virtual bool            Move(const char *currentName, const char *newName);
        virtual void *          FindBegin(const char *filePattern, FindData *findData);
        virtual bool            FindNext(void *h, FindData *findData);
        virtual bool            FindEnd(void *h);
        virtual bool            DirectoryCreate(const char *directory);
        virtual bool            DirectoryRemove(const char *directory);
        virtual bool            GetNameFromHandle(void *handle, char *name, size_t nameBufferSize);
    };

    extern KettleFileDeviceDriver gDeviceDriverImpl;
}}}

#endif //RW_CORE_KETTLE_FILESYS_DRIVERIMPL_H
