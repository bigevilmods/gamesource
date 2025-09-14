#include "rw/core/filesys/config.h"
#include "rw/core/filesys/device.h"

namespace rw {
namespace core {
namespace filesys {

/* IHttpDirectoryHandler
 * 
 * This interface allows users to provide a machanism for determining directory information
 * which may be completely dependent on their setup and/or choice of server and 
 */

typedef void* DirHandle;

class RWCORELIB_API IHttpDirectoryHandler
{
public:
    virtual ~IHttpDirectoryHandler() {};

    /*!
        \brief This function searches a directory for a file or subdirectory whose name
        matches the specified file name. It should return a handle to use for
        future GetNextFile/FindNext calls.

        \importlib rwfilesystem

        \return handle for use in future FindNext calls
    */
    virtual DirHandle FindBegin(const char *filePattern, DeviceDriver::FindData *findData) = 0;

    /*!
        \brief This function continues a file search from a previous call to the
        GetFirstFile function.

        \importlib rwfilesystem

        \return false on failure to find any more files
    */
    virtual bool  FindNext(DirHandle h, DeviceDriver::FindData *findData) = 0;

    /*!
        \brief This function closes the specified search handle. The GetFirstFile and
        GetNextFile functions use the search handle to locate files with names
        that match a specified name.

        \importlib rwfilesystem

        \return false on failure
    */
    virtual bool  FindEnd(DirHandle h) = 0;

    /*!
        \brief The DirectoryCreate function creates a new directory on the device.

        \importlib rwfilesystem

        \return false on failure
    */
    virtual bool  DirectoryCreate(const char *directory) = 0;

    /*!
        \brief The DirectoryRemove function deletes a directory on the device.

        \importlib rwfilesystem

        \return false on failure
    */
    virtual bool  DirectoryRemove(const char *directory) = 0;
};

}}} // namespace rw::core::filesys