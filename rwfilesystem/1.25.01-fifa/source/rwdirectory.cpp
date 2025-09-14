// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::DirectoryEntry.
*/

#include "EAAssert/eaassert.h"

#include "rw/core/filesys/directory.h"
#include "rw/core/filesys/loadsave.h"
#include "rw/core/filesys/path.h"
#include "private_rwdevice.h"
#include <stdio.h>
#include <string.h>

#include "platform_sdk.h"

#if defined(_MSC_VER)
#pragma warning(disable: 4351)  // new behavior: elements of array 'array' will be default initialized
#endif

// winbase.h redefines CreateDirectory and RemoveDirectory to
// other values.  So we undefine them here so RW Core library
// callers use the RW Core CreateDirectory function.
// --> Has to be done in the CPP file as well for unity build
//     purposes.
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
  Default constructor sets up the DirectoryEntry object for
  use.  The caller needs to call OpenDirectory to open a directory
  after the object has been created.

  \importlib rwfilesystem

*/
DirectoryEntry::DirectoryEntry() : mValid(false),
    mDriver(NULL),
    mFindHandle(NULL),
    mDirectory()
{
    memset(&mFindData, 0, sizeof(mFindData));
}

/*!
  This constructor takes a directory for opening parameter.  IsValid
  can be called immediately after the object has been created when this
  constructor is used.

  \importlib rwfilesystem

  \param directory   directory to open (UNC paths supported).  If this
  parameter is NULL, the current working directory is opened.
  \param pattern file pattern to match.  Like the dir shell command,
  '*' will match any pattern, and '?' will match any single character.
*/
DirectoryEntry::DirectoryEntry(
    const char *directory,
    const char *pattern
    ) : mValid(false),
    mDriver(NULL),
    mFindHandle(NULL),
    mDirectory()
{
    memset(&mFindData, 0, sizeof(mFindData));
    OpenDirectory(directory, pattern);
}

//lint -e{1740} pointer member mFindHandle not directly freed or zeroed by destructor.  INVALID_HANDLE_VALUE is -1, so lint is complaining
/*!
  The destructor destroys the object and releases any resources.

  \importlib rwfilesystem
*/
DirectoryEntry::~DirectoryEntry()
{
    if (mFindHandle != NULL)
    {
        mDriver->FindEnd(mFindHandle);
        mFindHandle = NULL;
        mDriver = NULL;
        mValid = false;
    }
}

/*! \brief Open a directory for finding a pattern of files.

    If the object already contains an open directory, it will be closed
    before the new one is opened.  Calling this method with the same
    directory as is currently open will restart the iterator.  This
    function uses the Manager's search path and opens first directory
    that matches the given directory and pattern.  The Manager's
    current working directory is ignored by OpenDirectory.

    \importlib rwfilesystem

    \param directory Directory to open.  If NULL or relative, the search
    path is used and the first location to return a match to the pattern
    is returned.  The current working directory is ignored.
    \param pattern File pattern to match.  '*' will match any pattern,
    and '?' will match any single character.  The pattern may also contain
    a directory.  If NULL, any files will match, provided the directory
    exists.
    \sa Manager::SetSearchPath
*/
void DirectoryEntry::OpenDirectory(
    const char *directory,
    const char *pattern
    )
{
    if (mFindHandle != NULL)
    {
        mDriver->FindEnd(mFindHandle);
        mFindHandle = NULL;
        mDriver = NULL;
        mValid = false;
    }

    if ((directory == NULL) ||
        (strcmp(directory, ".") == 0))
    {
        // If the user specifies the current directory,
        // FindFirstFile wants the directory to be removed.
        directory = "";
    }

    // skip over any .\ or ./ at the start of the path.
    if (*directory == '.' && ((*(directory+1) == '/') || (*(directory+1) == '\\')))
    {
        directory += 2;
    }

    char FilePattern[FILESYS_MAX_PATH];

    const Device *dev = Device::GetInstance(directory);

    // If the user does not specify a pattern, assume
    // they mean all files in this directory.
    if (pattern == NULL)
        pattern = "*";

    Manager *pMgr = Manager::GetInstance();

    if (dev == pMgr->GetNullDevice())
    {
        // lock the search path futex so that it cannot be modified in
        // another thread while we're iterating over it here.
        EA::Thread::AutoFutex lock(pMgr->GetSearchPathFutex());

        const Manager::SearchPathList& list = pMgr->GetSearchPathList();

        // search for a match using the search path
        for (Manager::SearchPathList::const_iterator info = list.begin(); info != list.end() && (mFindHandle == NULL); ++info)
        {
            const char *sp = info->GetPath();
            dev = info->GetDevice();

#ifdef EA_PLATFORM_WINDOWS
            if (dev == pMgr->GetNullDevice())
            {
                // this is the NULL device.  Assert that the file path
                // is UNC
                EA_ASSERT_MSG((*sp == '\\') && (*(sp+1) == '\\'),
                    "unknown device in search path");

                EA_ASSERT_MSG(dev->mpNext != NULL,
                    "No device exists that can execute this operation.");

                // go to the next registered device.
                dev = static_cast<Device *>(dev->mpNext);
            }
#else
            // make sure the file library knows about this device.
            EA_ASSERT_MSG(dev != pMgr->GetNullDevice(),
                "unknown device in search path");
#endif

            if (*directory != '\0')
            {
                if ((*directory == '/') || (*directory == '\\'))
                {
                    // The directory starts with a dirsep character, so just
                    // use this location's device
#ifdef EA_PLATFORM_WINDOWS
                    if (*(directory+1) == '\\')
                    {
                        // UNC path
                        sprintf(FilePattern, "%s/%s", directory, pattern);
                    }
                    else
#endif
                    {
                        strcpy(FilePattern, sp);
                        sprintf(strchr(FilePattern, ':') + 1, "%s/%s", directory, pattern);
                    }
                }
                else
                {
                    // Append the directory & pattern to the path
                    sprintf(FilePattern, "%s/%s/%s", sp, directory, pattern);
                }
            }
            else
            {
                // No directory; just append the pattern to the path
                sprintf(FilePattern, "%s/%s", sp, pattern);
            }

            mFindHandle = dev->GetDriver()->FindBegin(FilePattern, &mFindData);
        }
    }
    else
    {
        // The directory contains a fully-qualified path already; only one
        // device to check
        strcpy(FilePattern, directory);
        char *lastChar = FilePattern + strlen(FilePattern) - 1;

        // append a dirsep character if FilePattern
        // does not already end in one
        if ((*lastChar != '/') && (*lastChar != '\\'))
        {
            strcat(lastChar, "/");
            ++lastChar;
        }

        strcat(lastChar, pattern);

        mFindHandle = dev->GetDriver()->FindBegin(FilePattern, &mFindData);
    }

    if (mFindHandle != NULL)
    {
        mValid = true;

        mDriver = dev->GetDriver();

        GetDirectory(FilePattern, mDirectory);
    }

#ifdef EA_PLATFORM_XENON
    else if (pattern[0] == '*' && pattern[1] == 0)
    {
        // if there was no search pattern, check if the directory exists.
        GetDirectory(FilePattern, mDirectory);
        ReplaceSlashes(mDirectory, mDirectory);
        if(::GetFileAttributes(mDirectory)== FILE_ATTRIBUTE_DIRECTORY)
        {
            mValid = true;
            mFindData.mFlags = DirectoryEntry::FLAG_DIRECTORY;
            strcpy(mFindData.mName, ".");
        }
    }
#endif
}

/*!
  GetNextFile gets the next file in the directory.  When there are no
  more files, IsValid will return false.  At that point, the
  DirectoryEntry object should be destroyed or a new iteration started
  with OpenDirectory.

  \importlib rwfilesystem
*/
void DirectoryEntry::GetNextFile()
{
    if ((mFindHandle != NULL) && (mDriver != NULL))
    {
        mValid = mDriver->FindNext(mFindHandle, &mFindData);
    }
    else
    {
        mValid = false;
    }
}

/*!
  GetFlags returns a bit mask showing what flags are set on the current
  file.

  Note: Because of a known issue with current PS3 SDKs, GetFlags reports
  all files as writable on  PS3 at the moment. This is expected to be
  fixed in next SDK releases.

  \importlib rwfilesystem

  \return A bitwise combination of DirectoryFlags.
*/
unsigned int    DirectoryEntry::GetFlags() const
{
    if (!mValid)
        return 0;

    return mFindData.mFlags;
}

/*! \brief Get current file/directory name.

    GetName returns the current files' name (without the directory).  If
    there is no current file, NULL is returned.  If you want a full path
    to the file, use GetPath.
    \return The name of the current file or directory
    \sa GetPath
    \importlib rwfilesystem
*/
const char *    DirectoryEntry::GetName() const
{
    if (!mValid)
        return NULL;

    return mFindData.mName;
}

/*! \brief Get a fully-qualified path to the file, including the device
    and directory.

    \param pathBuffer Pointer to a string buffer to receive the full path
    to the file.  Cannot be NULL, should be FILESYS_MAX_PATH characters.
    \return A pointer to the pathBuffer, allowing the result of this function
    to be passed to another function.
    \sa AsyncOp::Open, GetName
    \importlib rwfilesystem
*/
char *    DirectoryEntry::GetPath(char *pathBuffer) const
{
    if (!mValid)
        return NULL;

    EA_ASSERT_MSG(pathBuffer != NULL, "pathBuffer cannot be NULL");

    // copy the directory & file name to the pathBuffer
    strcat(strcpy(pathBuffer, mDirectory), mFindData.mName);

    return pathBuffer;
}

/*!
  GetSize returns the current files' size in bytes.  If there is no
  current file, or the current file is actually a directory, 0 is
  returned.

  \importlib rwfilesystem

  \return the current files' size in bytes
*/
uint64_t DirectoryEntry::GetSize() const
{
    if (!mValid)
        return 0;

    return mFindData.mSize;
}

/*!
  GetCreationTime returns the time the current file or directory was
  created.  The time is a 64-bit value representing the number of 100-
  nanosecond intervals since January 1, 1601 (UTC).

  \importlib rwfilesystem

  \return time the current file or directory was created
*/
uint64_t DirectoryEntry::GetCreationTime() const
{
    if (!mValid)
        return 0;

    return mFindData.mCreationTime;
}

/*!
  For files, GetAccessTime returns the time the current file was last
  read from or written to. For a directory, GetAccessTime returns
  when the directory was created. For both files and directories, the
  specified date will be correct, but the time of day will always be set
  to midnight.  The time is a 64-bit value representing the number of
  100-nanosecond intervals since January 1, 1601 (UTC).

  \importlib rwfilesystem

  \return time the file was read from or written to, for directories
          returns the time of creation.
*/
uint64_t DirectoryEntry::GetAccessTime() const
{
    if (!mValid)
        return 0;

    return mFindData.mAccessTime;
}

/*!
  For files, GetModificationTime return the time the current file was
  last written to. For a directory, GetModificationTime returns when the
  directory was created.  The time is a 64-bit value representing the
  number of 100-nanosecond intervals since January 1, 1601 (UTC).

  \importlib rwfilesystem

  \return time file was last written to, for directories, returns
          the time the directory was created
*/
uint64_t DirectoryEntry::GetModificationTime() const
{
    if (!mValid)
        return 0;

    return mFindData.mModificationTime;
}

/*!
  The CreateDirectory function creates a new directory.

  If the directory name specifies a relative path
  (e.g. "./NewDir") instead of an absolute path
  ("C:\\MyGame\\NewDir"), the directory will be created
  in the first location in the search path, regardless
  of whether that is the current working directory or not.

  Note that the Windows Platform SDK headers \#define
  CreateDirectory to something else, so if you \#include
  <windows.h> after <rw/core/filesys/directory.h>, you
  will get compile errors.  The best solution is to
  \#include <windows.h> before <rw/core/filesys/directory.h>,
  but you can also \#undef CreateDirectory after the
  <windows.h> include.

  \return If the new directory was successfully created,
  true is returned.  If it could not be created, false is
  returned.

  \importlib rwfilesystem

  \param directory Pointer to a null-terminated string that
  specifies the path of the directory to be created.
  \param flags One or more of the rw::core::filesys::CreateDirectoryFlags.

  \sa RemoveDirectory, DirectoryExists
*/
bool CreateDirectory(const char *path, unsigned int flags)
{
    EA_ASSERT_MSG(path !=  NULL, "directory parameter cannot be NULL");
    char simplifiedPath[FILESYS_MAX_PATH] = {};

    if (DirectoryExists(path))
    {
        return true;
    }
    else
    {
        char qualifiedPath[FILESYS_MAX_PATH] = {};
        const Device *device = Device::GetInstance(path, qualifiedPath);

        // Simplify the qualifiedPath (also normalizes the directory separators)
        PathSimplify(qualifiedPath, simplifiedPath, FILESYS_MAX_PATH);
        
        // Try to create parent directory if recursive flag is set
        if (flags & CREATEDIRECTORY_FLAG_RECURSIVE)
        {
            char parentDirectory[FILESYS_MAX_PATH] = {};
            GetPathDirectory(simplifiedPath, parentDirectory, FILESYS_MAX_PATH);
            CreateDirectory(parentDirectory, flags);
        }

        bool directoryCreated = device->GetDriver()->DirectoryCreate(simplifiedPath);

        if (directoryCreated)
        {
            return true;
        }
        else
        {
            // Remove directory.. it may be corrupt
            RemoveDirectory(simplifiedPath);
            // Retry
            return device->GetDriver()->DirectoryCreate(simplifiedPath);
        }
    }
}

/*!
  The RemoveDirectory function deletes an existing empty
  directory.

  If the directory name specifies a relative path
  (e.g. "./NewDir") instead of an absolute path
  ("C:\\MyGame\\NewDir"), the directory will be removed
  from the first location in the search path, regardless
  of whether that is the current working directory or not.

  Note that the Windows Platform SDK headers \#define
  RemoveDirectory to something else, so if you \#include
  <windows.h> after <rw/core/filesys/directory.h>, you
  will get compile errors.  The best solution is to
  \#include <windows.h> before <rw/core/filesys/directory.h>,
  but you can also \#undef RemoveDirectory after the
  <windows.h> include.

  \return If the new directory was successfully removed,
  true is returned.  If it could not be removed (because
  it is not empty or does not exist), false is
  returned.

  \importlib rwfilesystem

  \param directory Pointer to a null-terminated string
  that specifies the path of the directory to be removed.
  This will fail if the directory to be deleted is not
  empty.
  \param flags One or more of the rw::core::filesys::RemoveDirectoryFlags.

  \sa CreateDirectory, DirectoryExists
*/
bool RemoveDirectory(const char *directory, unsigned int flags)
{
    EA_ASSERT_MSG(directory != NULL, "directory parameter cannot be NULL");
    char fullPath[FILESYS_MAX_PATH];

    if( flags & REMOVEDIRECTORY_FLAG_RECURSIVE )
    {
        
        {   // This is not decoration...
            // it is needed to call the destructor for the directoryEntry,
            // so that the last remove directory actually works.
            rw::core::filesys::DirectoryEntry directoryEntry( directory );
            char directoryBuffer[ rw::core::filesys::FILESYS_MAX_PATH ];

            while( directoryEntry.IsValid() )
            {
                if( directoryEntry.GetFlags() & rw::core::filesys::DirectoryEntry::FLAG_DIRECTORY )
                {
                    const char *name = directoryEntry.GetName();
                    if( ( ( ( name[ 0 ] == '.' ) && ( name[ 1 ] == 0 ) ) ||
                        ( ( name[ 0 ] == '.' ) && ( name[ 1 ] == '.' ) && ( name[ 2 ] == 0 ) ) ) == false )
                    {
                        directoryEntry.GetPath( directoryBuffer );
                        if( RemoveDirectory( directoryBuffer, flags ) == false )
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    if( flags & REMOVEDIRECTORY_FLAG_DELETE_FILES )
                    {
                        directoryEntry.GetPath( directoryBuffer );
                        if( rw::core::filesys::Delete( directoryBuffer ) == false )
                        {
                            return false;
                        }
                    }
                    else
                    {
                        return false;
                    }
                }

                directoryEntry.GetNextFile();
            }
        }
        
        bool result = rw::core::filesys::RemoveDirectory( directory, 0 );
        return result;
    }
    else
    {
        const Device *device = Device::GetInstance(directory, fullPath);
        return device->GetDriver()->DirectoryRemove(fullPath);
    }
}


/*!
  \brief Check whether a directory exists.

  \return If the directory exists, true is returned.
  If the directory did not exist, false is returned.

  \importlib rwfilesystem

  \param directory Pointer to a null-terminated string that
  specifies the path of the directory to be created.

  \sa CreateDirectory, RemoveDirectory
*/
bool DirectoryExists( const char *directory )
{
    EA_ASSERT_MSG(directory != NULL, "directory parameter cannot be NULL");

    DirectoryEntry entry;
    entry.OpenDirectory(directory);
    return entry.IsValid();
}


// scan through the src path and replace any slashes or backslashes with the platform correct slash
// \param dst Buffer to receive the output string.  May also contain
// the input string, if src is NULL.
// \param src Pointer to buffer with the string to be replaced.  If NULL,
// the dst string is scanned in-place and any slashes are replaced with
// backslashes
// \return pointer to the dst string so the return value can be passed to
// Platform SDK functions.
char *ReplaceSlashes(char *dst, const char *src)
{
    char *ret = dst;

    if (src == NULL)
        src = dst;

    while (*src)
    {
        *dst = *src;
#if defined EA_PLATFORM_XENON
        if (*dst == '/') *dst = '\\';
#elif defined EA_PLATFORM_UNIX || defined EA_PLATFORM_APPLE || defined EA_PLATFORM_PLAYBOOK || defined EA_PLATFORM_PS4
        if (*dst == '\\') *dst = '/';
#endif
        ++src;
        ++dst;
    }
    *dst = '\0';

    return ret;
}


/*!
  FindLastSeparator finds the last path separator ('/' or '\'

  \importlib rwfilesystem

  \return Last separator in the path, or NULL if there is none.
*/
const char *FindLastSeparator(const char *src)
{
    const char* srcSlash = strrchr(src, '/');
    const char *srcBackSlash = strrchr(src, '\\');
    const char* srcSeparator = 0;

    if(srcSlash > srcBackSlash)
        srcSeparator = srcSlash;
    else
        srcSeparator = srcBackSlash;

    return srcSeparator;
}

/*!
  FindFileName finds the filename part in the src string.
  For example "cache:\dir\filename.dat" returns a pointer to "filename.dat"

  \importlib rwfilesystem

  \return File name in the path, or NULL if there is none.
*/
const char *FindFileName(const char *src)
{
    const char* srcSeparator = FindLastSeparator(src);

    if(srcSeparator)
        ++srcSeparator;

    return srcSeparator;
}

/*!
  GetDirectory fills the directoryBuffer with with the directory in the path.

  \importlib rwfilesystem

  \return last delimiter in the path, or NULL if there is none.
*/
void GetDirectory(char *path, char *directoryBuffer)
{
    char *lastSlashDelim = strrchr(path, '/');
    char *lastBackSlashDelim = strrchr(path, '\\');
    char *lastDelim = NULL;

    if ((lastSlashDelim != NULL) && (lastBackSlashDelim != NULL))
    {
        // the path has both slashes & backslashes.  Take the last one.
        lastDelim = (lastSlashDelim > lastBackSlashDelim) ?
            lastSlashDelim : lastBackSlashDelim;
    }

    if (lastDelim == NULL)
    {
        // the path contains only slashes or backslashes.
        // Take whichever one isn't NULL.
        lastDelim = lastSlashDelim != NULL ? lastSlashDelim : lastBackSlashDelim;
    }

    // mDirectory will have a trailing dirsep character on it.  The only use-case
    // for it will be to return a full path to the file
    strncpy(directoryBuffer, path, static_cast<size_t>(lastDelim - path + 1));

    directoryBuffer[lastDelim - path + 1] = '\0';
}



}}}
