// (c) Electronic Arts. All Rights Reserved.

/**
\file
Header file defining operations that can be performed on a Handle pointer.
*/

#ifndef RW_CORE_FILESYS_HANDLE_H
#define RW_CORE_FILESYS_HANDLE_H

#include "rw/core/filesys/config.h"

namespace rw {
namespace core {
namespace filesys {

class Handle;
class DeviceDriver;

// These functions are currently only intended for use within DeviceDriver implementations.  They should
// not be used from user code in general.  (Please contact the EATech Core team if you have a need to
// call these functions in a different scenario.)

/*!
    \brief Returns the device driver associated with the handle object.
*/
RWCORELIB_API DeviceDriver*   GetDeviceDriver(const Handle *handle);

/*!
    \brief Returns the device file handle associated with the handle object.
*/
RWCORELIB_API void*           GetDeviceFileHandle(const Handle *handle);


/*!
    \brief Returns the parent file handle associated with the handle object.
*/
RWCORELIB_API Handle*           GetParentFileHandle(const Handle *handle);

}}}

#endif

