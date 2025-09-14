// (c) Electronic Arts. All Rights Reserved.
/**
\file
Implements the resize function.
*/

#include "private_rwhandle.h"
#include "private_rwdevice.h"
#include "EAAssert/eaassert.h"

namespace rw {
namespace core {
namespace filesys {

/*!
    The Resize function moves the end-of-file (EOF) position for
    specified file to a new location.  It can be used to truncate
    or extend a file.  If the file is extended, the contents of
    the file between the old EOF position and the new position are
    not defined.

    The file handle must be opened with the
    rw::core::filesys::AsyncOp::FOPEN_READWRITE flag in order to
    be resized.

    \importlib rwfilesystem

    \param handle Pointer to a null-terminated string that names an existing file.
    \param size   New size of the file in bytes.
    \return If the file was successfully resized, true is returned.
    If not, false is returned.
*/
bool Resize(Handle *handle, uint64_t size)
{
    EA_ASSERT_MSG(handle != 0, "Invalid file handle");

    bool ret = false;
    ret = handle->mDriver->Resize(handle->mDevFileHandle, size);
    handle->mLength = handle->mDriver->GetSize(handle->mDevFileHandle, handle->mDevice->GetDriver(), handle->mpParentFile->mDevFileHandle);
    return ret;

}

}}}
