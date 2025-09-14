// (c) Electronic Arts. All Rights Reserved.

/**
\file
Private header of rw::core::filesys::NullFileDriver, implements the DeviceDriver class.
*/

#ifndef RW_CORE_FILESYS_INTERNAL_NULLDRIVER_H
#define RW_CORE_FILESYS_INTERNAL_NULLDRIVER_H

#include "rw/core/filesys/device.h"

namespace rw {
namespace core {
namespace filesys {

/// \internal
/// This class is used when we cannot tell from the file name what
/// device the operation should be executed on (i.e. the file path
/// is not fully-qualified).
class RWCORELIB_API NullFileDriver : public DeviceDriver
{
public:
    NullFileDriver() : DeviceDriver() {}
    virtual ~NullFileDriver() {}

    virtual void* Open(const char *, unsigned int, Handle **) { return 0; }
    virtual void Close(void *, DeviceDriver *, void *) {}

    virtual unsigned int Read(void *, void *, unsigned int, DeviceDriver *, void *) { return 0; }
    virtual uint64_t Seek(void *, uint64_t, int, DeviceDriver *, void *) { return uint64_t(); }
    virtual uint64_t GetSize(void *, DeviceDriver *, void *) { return uint64_t(); }
};

extern NullFileDriver NullFileDriverInst;

}}}

#endif
