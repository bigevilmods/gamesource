// (c) Electronic Arts. All Rights Reserved.
/**
\file
Implements the mvoe function.
*/

#include "rw/core/filesys/loadsave.h"
#include "private_rwdevice.h"

namespace rw {
namespace core {
namespace filesys {

/*!
  The Move function moves an existing file or a directory, including
  its children.  If the file or directory is moved to the same location,
  it is renamed.

  \param currentName Pointer to a null-terminated string that names an existing file or directory.
  \param newName Pointer to a null-terminated string that specifies the new name of a file or directory. The new name must not already exist. The new file or directory must be on the same drive as the existing file or directory.

  \importlib rwfilesystem

  \return If the file was successfully moved, true is returned.  If
  not, false is returned.
*/
bool Move(const char *currentName, const char *newName)
{
    char fullCurrName[FILESYS_MAX_PATH], fullNewName[FILESYS_MAX_PATH];

    const Device * device = Device::GetInstance(currentName, fullCurrName);
    Device::GetInstance(newName, fullNewName);

    return device->GetDriver()->Move(fullCurrName, fullNewName);
}

}}}
