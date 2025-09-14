// (c) Electronic Arts. All Rights Reserved.

#include "EAAssert/eaassert.h"

#include "rw/core/filesys/detail/installdriver.h"
#include "rw/core/filesys/async.h"
#include "rw/core/filesys/loadsave.h"
#include "rw/core/filesys/directory.h"
#include "EAStdC/EASprintf.h"
#include "EAStdC/EAString.h"

using namespace EA::StdC;

namespace rw {
namespace core {
namespace filesys {
namespace detail {

bool InstallDriver::CreateVersionFile( const char *path, uint32_t version )
{
    char versionCacheFilename[rw::core::filesys::FILESYS_MAX_PATH];
    Snprintf( versionCacheFilename, sizeof(versionCacheFilename), "%s/%d.ver", path, version );

    bool result = true;
    rw::core::filesys::AsyncOp ao;
    rw::core::filesys::Handle *handle = NULL;

    // Open the source file
    ao.Open( versionCacheFilename, rw::core::filesys::AsyncOp::FOPEN_OVERWRITE | rw::core::filesys::AsyncOp::FOPEN_CREATE | rw::core::filesys::AsyncOp::FOPEN_READWRITE );
    ao.Wait();
    handle = ao.GetResultHandle();

    if ( handle == NULL )
    {
        EA_ASSERT_FORMATTED( handle != NULL, ("InstallDriver::CreateVersionFile -> Failed to create %s!\n", versionCacheFilename) );
        return false;
    }

    // Write the version number to the version number file. This needs to
    // be done, because we assume that 0 byte files are corrupt.
    const void* buffer = reinterpret_cast<const void*>(versionCacheFilename);
    uint64_t bufferSize = Strlen(versionCacheFilename);

    ao.Write( handle, buffer, 0, bufferSize);
    ao.Wait();
    uint64_t writeSize = ao.GetResultSize();

    // Check to see if all data has been written
    if( writeSize != bufferSize )
    {
        EA_ASSERT_MSG( writeSize != bufferSize, ("InstallDriver::CreateVersionFile -> Write Failed!\n") );
        result = false;
    }

    ao.Close(handle);
    ao.Wait();

    return result;
}


bool InstallDriver::ValidateVersion( const char *path, uint32_t version )
{
    char versionCacheFilename[rw::core::filesys::FILESYS_MAX_PATH];
    Snprintf( versionCacheFilename, sizeof(versionCacheFilename), "%s/%d.ver", path, version );

    bool bFound = rw::core::filesys::Exists(versionCacheFilename);
    if (bFound)
        return true;

    // else
    // check if there is another .ver file
    // return the opposite
    rw::core::filesys::DirectoryEntry directoryEntry(path, "*.ver");
    return !directoryEntry.IsValid();
}

// Check that a list of files at the path exist and are non-zero in size.
bool InstallDriver::ValidateFiles(const char *path, const char **filenames)
{
    bool result = true;

    char installedFilename[FILESYS_MAX_PATH];
    size_t i = 0;
    while((filenames != NULL) && (filenames[i] != NULL) && result)
    {
        Snprintf( installedFilename, FILESYS_MAX_PATH, "%s/%s", path, filenames[i] );

        rw::core::filesys::AsyncOp op;
        op.Open( installedFilename, rw::core::filesys::AsyncOp::FOPEN_READONLY);
        rw::core::filesys::Handle *fileHandle = op.GetResultHandle();

        if (fileHandle != NULL)
        {
            if (GetSize(fileHandle) == 0)
            {
                result = false;
            }

            op.Close(fileHandle);
            op.Wait();

            if (!result)
            {
                Delete(installedFilename);
            }
        }
        else
        {
            // Not found (installation is invalid)
            result = false;
        }

        ++i;
    }

    return result;
}


}}}}

