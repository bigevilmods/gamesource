// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements the delete function.
*/

#include "rw/core/filesys/loadsave.h"

#include "private_rwdevice.h"

namespace rw {
namespace core {
namespace filesys {

/*!
  The Delete function deletes an existing file.  If the file
  does not exist or is open when DeleteFile is called, this
  function will fail.

  Delete does not use the search path.  If the file path is
  not fully-qualified, only the current working directory is
  checked.

  \importlib rwfilesystem

  \return If the file was successfully deleted, true is returned.  If
  not, false is returned.
  \sa rw::core::filesys::Manager::SetCWD
*/
bool Delete(const char *filePath)
{
    char fullPath[FILESYS_MAX_PATH];
    const Device * device = Device::GetInstance(filePath, fullPath);
    return device->GetDriver()->Delete(fullPath);
}

}}}
