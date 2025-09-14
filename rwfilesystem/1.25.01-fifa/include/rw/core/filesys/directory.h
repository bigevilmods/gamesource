// (c) Electronic Arts. All Rights Reserved.

/**
\file
Header file of rw::core::filesys::DirectoryEntry
*/

#ifndef RW_CORE_FILESYS_DIRECTORY_H
#define RW_CORE_FILESYS_DIRECTORY_H

#include "EABase/eabase.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/async.h"
#include "rw/core/filesys/device.h"

// winbase.h redefines CreateDirectory and RemoveDirectory to
// other values.  So we undefine them here so Core library
// callers use our CreateDirectory function.
#ifdef CreateDirectory
    #undef CreateDirectory
#endif

#ifdef RemoveDirectory
    #undef RemoveDirectory
#endif

namespace rw {
namespace core {
namespace filesys {

/*!
    \brief DirectoryEntry is a class for iterating through the set of
    files in a subdirectory.  This functionality is useful during
    title development to check when files are updated and should
    be reloaded.
*/
class RWCORELIB_API DirectoryEntry
{
public:
    DirectoryEntry();
    explicit DirectoryEntry(const char *directory, const char *pattern = NULL);
    ~DirectoryEntry();

    static void *operator new(size_t size) { return RWFILESYSTEM_ALLOCATE(size); }
    static void *operator new(size_t, void *ptr) { return ptr; }
    static void operator delete(void *ptr) { return Manager::Free(ptr); }

    /*!
        \brief DirectoryFlags contains possible return values for GetFlags.
    */
    enum DirectoryFlags
    {
        FLAG_READONLY = 0x01,   ///< The file is readonly.

        FLAG_DIRECTORY = 0x02   ///< This file is a directory.
    };

    void GetNextFile();
    void OpenDirectory(const char *directory, const char *pattern = NULL);

    /*!
        \brief IsValid returns true if the current iterator is still valid.
        Once IsValid returns false, a new directory must be opened
        with OpenDirectory, or the object destroyed and a new one
        constructed.

        \importlib rwfilesystem

        \return true if the current iterator is still valid
    */
    bool            IsValid() const
    { return mValid; }

    unsigned int    GetFlags() const;
    const char *    GetName() const;
    char *          GetPath(char *pathBuffer) const;
    uint64_t        GetSize() const;
    uint64_t        GetCreationTime() const;
    uint64_t        GetAccessTime() const;
    uint64_t        GetModificationTime() const;

private:
    bool mValid;
    DeviceDriver *mDriver;
    void *mFindHandle;
    char mDirectory[FILESYS_MAX_PATH];
    DeviceDriver::FindData mFindData;

    DirectoryEntry(const DirectoryEntry &);
    DirectoryEntry& operator=(const DirectoryEntry &);
};

/*!
    \brief Flags for CreateDirectory
    \sa rw::core::filesys:CreateDirectory
*/
enum CreateDirectoryFlags
{
    /// If the path has multiple directories in it, only the last one is
    // created, and all other directories must exist already.
    CREATEDIRECTORY_FLAG_NONE = 0x0,

    /// Create the tree recursively.  If the path contains multiple
    /// directories, any missing ones will be created (similar to
    /// "mkdir -p".
    CREATEDIRECTORY_FLAG_RECURSIVE = 0x1
};

RWCORELIB_API bool CreateDirectory(const char *path,
                                   unsigned int flags = CREATEDIRECTORY_FLAG_NONE);

/*!
    \brief Flags for RemoveDirectory
    \sa rw::core::filesys:RemoveDirectory
*/
enum RemoveDirectoryFlags
{
    /// Only the last directory in the path is removed, and then only if
    /// that directory is empty.
    REMOVEDIRECTORY_FLAG_NONE = 0x0,

    /// Remove all directories in the path.  If any directories are not
    /// empty, this function fails.  Similar to "rmdir -p"
    REMOVEDIRECTORY_FLAG_RECURSIVE = 0x1,

    /// Remove all directories in the path.  If there are any non-empty
    /// directories, any files in them will also be deleted.
    REMOVEDIRECTORY_FLAG_DELETE_FILES = 0x2
};

RWCORELIB_API bool RemoveDirectory(const char *directory, unsigned int flags = REMOVEDIRECTORY_FLAG_NONE);
RWCORELIB_API bool DirectoryExists( const char *directory );
RWCORELIB_API char *ReplaceSlashes(char *dst, const char *src = NULL);
RWCORELIB_API const char *FindLastSeparator(const char *src);
RWCORELIB_API const char *FindFileName(const char *src);
RWCORELIB_API void GetDirectory(char *path, char *directoryBuffer);

}}}

#endif
