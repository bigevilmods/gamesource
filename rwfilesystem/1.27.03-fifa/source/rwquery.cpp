// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements query functionsl like Exists, Getsize and GetLocations.
*/

#include "EAAssert/eaassert.h"
#include "private_rwhandle.h"
#include "private_rwdevice.h"

namespace rw {
namespace core {
namespace filesys {

/*!
    Exists checks for the existence of a file.  It uses the same
    search algorithm as AsyncOp::Open, it runs on the current
    thread, rather than scheduling an operation to be serviced by
    a worker thread.
    \param FilePath a device specific path to the file in question
    \importlib rwfilesystem

    \return If the file was found, true is returned.  If not, false
    is returned.
    \sa AsyncOp::Open, GetSize, GetLocationAndSize
*/
bool Exists(const char *FilePath)
{
    EA_ASSERT(FilePath != NULL);

    Handle FileHandle(FilePath, 0, Device::GetInstance(FilePath));

    return FileHandle.mDevFileHandle != NULL;
}

/*!
    GetSize(const char *) returns the file size in bytes.  Use
    this version if you don't have a file handle and you don't plan
    on opening one.  If you already have a file handle, use
    GetSize(Handle *) instead of GetSize(const char *).
    \param FilePath a device specific path to the file in question
    \return the size of the file in bytes.  0 if the file was not
    found.
    \importlib rwfilesystem
    \sa AsyncOp::Open, Exists, GetSize,
    GetLocationAndSize
*/
uint64_t GetSize(const char *FilePath)
{
    EA_ASSERT(FilePath != NULL);

    Handle FileHandle(FilePath, 0, Device::GetInstance(FilePath));

    uint64_t retval = 0;

    if (FileHandle.mDevFileHandle != NULL)
    {
        retval = FileHandle.mLength;
    }

    return retval;
}

/*!
    GetLocationAndSize opens the file, and returns its physical
    location and size in bytes.  Use this function when you don't
    have an open file handle but you want to get both the size
    and the physical location of the file on the disc.  Refer to
    GetLocation for a full description of why this function is
    useful.

    \param FilePath Path to file.
    \param location Pointer to a uint64_t to receive the location of the file.
    \param size Pointer to a uint64_t to receive the size of the file.
    \importlib rwfilesystem

    \return returns false on failure
    \sa AsyncOp::Open, Exists, GetSize,
    GetLocation
*/
bool GetLocationAndSize(const char *FilePath, uint64_t *location, uint64_t *size)
{
    EA_ASSERT(FilePath != NULL);

    Handle FileHandle(FilePath, 0, Device::GetInstance(FilePath));

    bool retval = false;

    if (FileHandle.mDevFileHandle != NULL)
    {
        if (location != NULL)
        {
            *location = FileHandle.mDriver->
                QueryLocation(FileHandle.mDevFileHandle);
        }

        if (size != NULL)
        {
            *size = FileHandle.mLength;
        }

        retval = true;
    }

    return retval;
}

/*!
    GetSize(const Handle *) and returns the file size in bytes.  Use
    this function when you already have a valid file handle.  If
    you don't have a valid file handle and don't plan on opening
    the file, use GetSize(const char *) to get the file size.
    \param FileHandle a valid open file handle
    \importlib rwfilesystem
    \return The size of the file in bytes.
    \sa AsyncOp::Open, Exists, GetLocation, GetSize,
*/
uint64_t GetSize(const Handle *FileHandle)
{
    EA_ASSERT(FileHandle != NULL);
    
    return FileHandle->mDriver->GetSize(FileHandle->mDevFileHandle, FileHandle->mDevice->GetDriver(), FileHandle->mpParentFile->mDevFileHandle);
}

/*!
    GetLocation returns a physical file location.  Use this
    function when you already have a valid file handle.  If you
    don't have a valid file handle and don't plan on opening the
    file, use GetLocationAndSize get the file location.

    On platforms where the seek time is long compared to the actual
    transfer time, loading files in the same order in which they
    are placed on the disc will help keep overall load times as
    small as possible.  The Xbox DVD has an average seek time of
    130ms, and a transfer rate of 2.7 to 6.9 MB/sec.  In the time
    it takes to do an average seek, between ~350K and ~900K could
    have been loaded.  So keeping the number of seeks (and the
    distance between seeks) to a minimum is important for reducing
    load times.

    GetLocation can help asset loaders by allowing the asset loader
    to submit its load requests in the same order the assets are
    located on the media.

    \param FileHandle a valid open file handle
    \importlib rwfilesystem
    \return The physical location of the file.
    \sa AsyncOp::Open, Exists, GetSize,
    GetSize, GetLocationAndSize
*/
uint64_t GetLocation(const Handle *FileHandle)
{
    EA_ASSERT(FileHandle != NULL);

    return FileHandle->mDriver->QueryLocation(FileHandle->mDevFileHandle);
}

/*!
    GetNameFromHandle returns a file name from a handle.

    \param FileHandle a valid open file handle
    \param name Buffer into which the file name will be copied.
    \param nameBufferSize Size of the name buffer.

    \importlib rwfilesystem
    \return true if the driver implements the lookup function; false if
    the name is unavailable.
*/
bool GetNameFromHandle(const Handle *FileHandle, char *name, size_t nameBufferSize)
{
    EA_ASSERT(FileHandle->mDriver != NULL);
    EA_ASSERT(FileHandle->mDevFileHandle != NULL);

    return FileHandle->mDriver->GetNameFromHandle(FileHandle->mDevFileHandle, name, nameBufferSize);
}

/*!
    GetDevice returns the device associated with the handle, if valid.
*/
DeviceDriver * GetDriver(const Handle *FileHandle)
{
    if( FileHandle )
    {
        return FileHandle->mDriver;
    }
    return NULL;
}

}}}
