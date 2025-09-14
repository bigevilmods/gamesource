// (c) Electronic Arts. All Rights Reserved.

/**
\file
Private header of rw::core::filesys::Handle, implements the ::detail::ListNodeS class.
*/

#ifndef RW_PRIVATE_RWHANDLE_H
#define RW_PRIVATE_RWHANDLE_H

#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/device.h"

namespace rw {
namespace core {
namespace filesys {

class Device;
class DeviceDriver;

///! \internal
/// Handle class.  Constructor and destructor implemented
/// near the bottom of this source file.
class Handle
{
public:
    Handle(const char *filename, unsigned int modeflags, Device *device);
    ~Handle();

    void *operator new(size_t size) { return RWFILESYSTEM_ALLOCATE(size); }
    static void operator delete(void *ptr) { return Manager::Free(ptr); }

    // parent file handle.  On virtual devices, this points to
    // the parent FILESYSHANDLE.  On non-virtual devices, it
    // points to itself.
    Handle *            mpParentFile;

    // device file handle
    void *              mDevFileHandle;

    // device where this handle was opened on (only physical
    // drivers have devices attached)
    Device *            mDevice;

    // driver for this file handle.  On virtual devices, this
    // points to the virtual driver.  On physical devices, it
    // points to the device's driver.
    DeviceDriver *      mDriver;

    // length of the file
    uint64_t            mLength;

    // current offset in file
    uint64_t            mOffset;

    // Use this function to get the device name.  It actually returns
    // a pointer to the parent device name, in case this file is inside
    // a bigfile.
    const char *GetDeviceName() const
    { return mpParentFile->mDeviceName; }

private:
    // Logical device on which this handle is opened
    char                mDeviceName[MAX_DEVICE_NAME_LENGTH];

    static void         GetRootFromPath(char *root, const char *path);
};

}}}

#endif

