// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"
#include "EAAssert/eaassert.h"
#include "EAStdC/EASprintf.h"
#include "EAStdC/EAString.h"

#include "rw/core/filesys/filecopier.h"
#include "rw/core/filesys/async.h"
#include "rw/core/filesys/directory.h"
#include "rw/core/filesys/loadsave.h"
#include "rw/core/filesys/path.h"
#include "private_rwdevice.h"
#include "private_rwhandle.h"

using namespace EA::StdC;

#ifdef CreateDirectory
#undef CreateDirectory
#endif

namespace rw {
namespace core {
namespace filesys {

void FileCopierOptions::ErrorDefaultCallback(const char *, const char *, rw::core::filesys::FileCopierError )
{
}

bool FileCopierOptions::TestDefaultCallback(const char *, const char *)
{
    return true;
}

void FileCopierOptions::UpdateDefaultCallback(const char *, const char *, uint64_t, uint64_t)
{
}

void FileCopierOptions::FileDefaultCallback(const char *, const char *)
{
}

void FileCopierOptions::DoneDefaultCallback()
{
}

void *FileCopier::operator new(size_t size)
{
    EA_ASSERT(sizeof(FileCopier) == size);
    return RWFILESYSTEM_ALLOCATE(size);
}


void FileCopier::operator delete(void *p)
{
    if( p ) Manager::GetInstance()->Free( p );
}


FileCopier::FileCopier(const FileCopierOptions &options)
    : mOptions(options)
    , mReadBuffer(0)
    , mWriteBuffer(0)
    , mCopiedBytes(0)
    , mThreadPause(false)
    , mThreadTerminate(false)
    , mThreadIsTerminating(false)
    , mSrcFilenames(NULL)
    , mSrcPath(NULL)
    , mDstPath(NULL)
    , mDoneWithThis(false)
{
    mReadBuffer = RWFILESYSTEM_ALLOCATE( mOptions.bufferSize );
    mWriteBuffer = RWFILESYSTEM_ALLOCATE( mOptions.bufferSize );
}


FileCopier::~FileCopier()
{
    ThreadTerminate();
    Manager::GetInstance()->Free( mReadBuffer );
    Manager::GetInstance()->Free( mWriteBuffer );
    mReadBuffer = NULL;
    mWriteBuffer = NULL;
    mSrcFilenames = NULL;
    mSrcPath = NULL;
    mDstPath = NULL;
}

void FileCopier::Copy( const char **srcFilenames, const char *srcPath, const char *dstPath )
{
    EA_ASSERT(srcFilenames != NULL);
    EA_ASSERT(dstPath != NULL);

    mDoneWithThis = false;
    mCopiedBytes = 0;
    mSrcFilenames = srcFilenames;
    mSrcPath = srcPath != NULL ? srcPath : ".";
    mDstPath = dstPath;
    mThreadPause = false;
    mThreadTerminate = false;

    EA::Thread::ThreadParameters threadParameters;
    threadParameters.mnPriority = mOptions.threadPriority;
    threadParameters.mpName = "FileCopier::Thread";

    EA::Thread::ThreadId threadId = mThread.Begin( FileCopier::ThreadEntry,
        this, &threadParameters );
    (void)threadId;
    EA_ASSERT( threadId != EA::Thread::kThreadIdInvalid );
}

void FileCopier::Wait()
{
    while( mSrcFilenames != NULL || mDstPath != NULL )
    {
        mComplete.Wait(EA::Thread::kTimeoutNone);
    }
}

void FileCopier::Start()
{
    if( mThreadPause == false )
    {
        return;
    }

    mThreadPause = false;
    mThreadPauseSemaphore.Post();
}

void FileCopier::Stop()
{
    if( mThreadPause == true )
    {
        return;
    }

    mThreadPause = true;
}

void FileCopier::Terminate(bool waitForEnd)
{
    ThreadTerminate(waitForEnd);
}

void FileCopier::ThreadTerminate(bool waitForEnd)
{
    if ( mThread.GetStatus() != EA::Thread::Thread::kStatusRunning)
        return;

    mThreadTerminate = true;
    mThreadIsTerminating = true;

    // Make sure it is running, cause otherwise it can be paused
    Start();

    if (waitForEnd && !mDoneWithThis)
        mThread.WaitForEnd();
}

intptr_t FileCopier::ThreadEntry( void *param )
{
    static_cast<FileCopier *>(param)->Execute();

    return 0;
}

void FileCopier::Execute()
{
    char fullSrcFilename[FILESYS_MAX_PATH];
    char dstFilename[FILESYS_MAX_PATH];
    const char **srcFilename = mSrcFilenames;

    while (*srcFilename != NULL  && !mThreadTerminate)
    {
        EA::StdC::Snprintf( fullSrcFilename, FILESYS_MAX_PATH, "%s/%s", mSrcPath, *srcFilename );
        EA::StdC::Snprintf( dstFilename, FILESYS_MAX_PATH, "%s/%s", mDstPath, *srcFilename );

        // Wait for enough idle time on the src & dst device
        char fullSrcPath[FILESYS_MAX_PATH];
        char fullDstPath[FILESYS_MAX_PATH];
        const Device* device = Device::GetInstance(fullSrcFilename, fullSrcPath);
        WaitForDeviceIdle(device, mOptions.srcIdleTime);
        device = Device::GetInstance(dstFilename, fullDstPath);
        WaitForDeviceIdle(device, mOptions.dstIdleTime);

        // Check whether we need to copy files
        bool ret = mOptions.testCallback(fullSrcFilename, dstFilename );
        EA_ASSERT(EA::StdC::Strncmp(fullSrcPath, fullDstPath, EAArrayCount(fullSrcPath)) != 0);

        if( ret )
        {
            ret = Copy( fullSrcFilename, dstFilename );

            if(ret)
            {
                if(mOptions.fileCallback)
                    mOptions.fileCallback(fullSrcFilename, dstFilename );
            }
        }

        ++srcFilename;
    }

    mSrcFilenames = NULL;
    mDstPath = NULL;

    mComplete.Post();

    if(mThreadTerminate == false)
    {
        mDoneWithThis = true;

        if(mOptions.doneCallback)
            mOptions.doneCallback();
    }
    else
    {
        mThreadIsTerminating = false;
    }
}

enum FileCopierState
{
    FileCopierStateCreateDirectory,
    FileCopierStateOpenSourceFile,
    FileCopierStateOpenDestFile,
    FileCopierStateInitialRead,
    FileCopierStateReadWrite,
    FileCopierStateLastWrite,
    FileCopierStateCloseFiles,
    FileCopierStateRename,
    FileCopierStateDone,
    FileCopierStateError
};

void FileCopier::CreateTempFilename(const char *srcFilename, char *dstFilename, size_t dstFilenameSize)
{
    EA_ASSERT(dstFilenameSize >= EA::StdC::Strlen(srcFilename) + EA::StdC::Strlen(".tmp") + 1);
    (void)dstFilenameSize;
    
    EA_STRLCPY_VERIFY(dstFilename, srcFilename, dstFilenameSize);
    // Append .tmp to the destination
    EA_STRLCAT_VERIFY(dstFilename, ".tmp", dstFilenameSize);
}

void FileCopier::WaitForDeviceIdle(const Device * device, unsigned int idleMilliSeconds) const
{
    if (!idleMilliSeconds) return;

    unsigned int idleTime = (unsigned int)device->GetIdleTime();
    while(idleTime < idleMilliSeconds)
    {
        EA::Thread::ThreadSleep(idleMilliSeconds - idleTime);
        idleTime = (unsigned int) device->GetIdleTime();
    }
}

bool FileCopier::Copy( const char *srcFilename, const char *dstFilename )
{
    // -----
    // Check the file name length and if it is too long call error handler
#if defined EA_PLATFORM_XENON
    size_t maxFilenameLength = 40;
#else
    size_t maxFilenameLength = FILESYS_MAX_PATH - 1;
#endif

    const char* srcFile = FindFileName(srcFilename);
    const char* dstFile = FindFileName(dstFilename);
    size_t srcFilenameLength = EA::StdC::Strlen(srcFile);
    size_t dstFilenameLength = EA::StdC::Strlen(dstFile);

    if(srcFilenameLength > maxFilenameLength || dstFilenameLength > maxFilenameLength)
    {
        if(mOptions.errorCallback)
            mOptions.errorCallback(srcFilename, dstFilename, FileCopierError_InvalidFileName);
        else
            EA_FAIL_MSG("An error has occurred, but no error handler was set");

        return false;
    }
    // -----

    char tmpFilename[FILESYS_MAX_PATH];
    FileCopier::CreateTempFilename(dstFilename, tmpFilename, FILESYS_MAX_PATH);
    FileCopierState currState = FileCopierStateCreateDirectory;

    AsyncOp aor, aow;
    Handle *srcHandle = NULL;
    Handle *dstHandle = NULL;
    uint64_t read = 0;
    uint64_t wrote = 0;
    uint64_t offset = 0;
    char parentDirectory[FILESYS_MAX_PATH];

    // Wait for the dst device to become idle and
    // make sure that both the temp & destination files are not present by deleting them.
    char fullPath[FILESYS_MAX_PATH];
    const Device* device = Device::GetInstance(dstFilename, fullPath);
    WaitForDeviceIdle(device, mOptions.dstIdleTime);
    Delete(dstFilename);
    Delete(tmpFilename);
    size_t readSize = mOptions.bufferSize;
    const uint64_t eccSize = 32 * 1024;
    uint64_t readLocation;

    // We're doing this as a state machine to keep the pause & error handling
    // as simple as possible.
    while ((currState != FileCopierStateDone) &&
        (currState != FileCopierStateError) &&
        !mThreadTerminate)
    {
        switch (currState)
        {
        case FileCopierStateCreateDirectory:
            // if the directory can't be created, fail
            device = Device::GetInstance(tmpFilename, fullPath);
            WaitForDeviceIdle(device, mOptions.dstIdleTime);

            // Get the directory name of tmpFilename
            GetPathDirectory(tmpFilename, parentDirectory, FILESYS_MAX_PATH);

            if ( false == CreateDirectory(parentDirectory, CREATEDIRECTORY_FLAG_RECURSIVE) )
            {
                if(mOptions.errorCallback)
                    mOptions.errorCallback(srcFilename, dstFilename, FileCopierError_NotEnoughSpace);
                else
                    EA_FAIL_MSG("An error has occurred, but no error handler was set");

                currState = FileCopierStateError;
            }
            else
            {
                currState = FileCopierStateOpenSourceFile;
            }
            break;

        case FileCopierStateOpenSourceFile:
            device = Device::GetInstance(srcFilename, fullPath);
            WaitForDeviceIdle(device, mOptions.srcIdleTime);

            aor.SetTrackDeviceIdleTime(false);
            aor.Open( fullPath, AsyncOp::FOPEN_READONLY,
                NULL, NULL, mOptions.copyPriority );
            //aor.SetTrackDeviceIdleTime(true);
            srcHandle = aor.GetResultHandle();
            if( srcHandle != NULL )
            {
                currState = FileCopierStateOpenDestFile;
            }
            else
            {
                if(mOptions.errorCallback)
                    mOptions.errorCallback(srcFilename, dstFilename, FileCopierError_MissingFile);
                else
                    EA_FAIL_MSG("An error has occurred, but no error handler was set");

                currState = FileCopierStateError;
            }
            break;

        case FileCopierStateOpenDestFile:
            device = Device::GetInstance(tmpFilename, fullPath);
            WaitForDeviceIdle(device, mOptions.dstIdleTime);

            aow.SetTrackDeviceIdleTime(false);
            aow.Open( tmpFilename, AsyncOp::FOPEN_OVERWRITE | AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_READWRITE,
                NULL, NULL, mOptions.copyPriority );
            //aow.SetTrackDeviceIdleTime(true);

            dstHandle = aow.GetResultHandle();
            if ( dstHandle != NULL )
            {
                currState = FileCopierStateInitialRead;
            }
            else
            {
                if(mOptions.errorCallback)
                    mOptions.errorCallback(srcFilename, dstFilename, FileCopierError_NotEnoughSpace );
                else
                    EA_FAIL_MSG("An error has occurred, but no error handler was set");

                currState = FileCopierStateError;
            }
            break;

        case FileCopierStateInitialRead:
            WaitForDeviceIdle(srcHandle->mDevice, mOptions.srcIdleTime);
            aor.SetTrackDeviceIdleTime(false);
            readLocation = rw::core::filesys::GetLocation(srcHandle) + offset;
            if((readLocation % eccSize) != 0)
            {
                uint64_t maxReadAlignedSize = eccSize - (readLocation % eccSize);
                if(static_cast<unsigned int>(readSize) > maxReadAlignedSize)
                {
                    readSize = static_cast<unsigned int>(maxReadAlignedSize);
                }
            }
            aor.Read( srcHandle, mReadBuffer, offset, readSize, NULL, NULL, mOptions.copyPriority);
            //aor.SetTrackDeviceIdleTime(true);
            read = aor.GetResultSize();
            BufferSwap();
            if (read < readSize)
            {
                if (read == 0)
                {
                    if (GetSize(srcHandle) == 0)
                    {
                        // the filesize is 0. Nothing to do
                        currState = FileCopierStateCloseFiles;
                    }
                    else
                    {
                        EA_FAIL_MSG("rw::core::filesys::FileCopier::Copy -> Read failed!\n");
                        currState = FileCopierStateError;
                    }
                }
                else
                {
                    currState = FileCopierStateLastWrite;
                }
            }
            else
            {
                currState = FileCopierStateReadWrite;
            }
            break;

        case FileCopierStateReadWrite:
            WaitForDeviceIdle(dstHandle->mDevice, mOptions.dstIdleTime);
            aow.SetTrackDeviceIdleTime(false);
            aow.Write( dstHandle, mWriteBuffer, offset, read, NULL, NULL, mOptions.copyPriority );
            //aow.SetTrackDeviceIdleTime(true);

            // wrote is the last wrote value
            mCopiedBytes += wrote;
            if (mOptions.updateCallback)
                mOptions.updateCallback(srcFilename, dstFilename, wrote, mCopiedBytes);

            // update the offset ptr and read the new block
            offset += read;

            WaitForDeviceIdle(srcHandle->mDevice, mOptions.srcIdleTime);
            aor.SetTrackDeviceIdleTime(false);
            aor.Read( srcHandle, mReadBuffer, offset, mOptions.bufferSize, NULL, NULL, mOptions.copyPriority);
            //aor.SetTrackDeviceIdleTime(true);

            wrote = aow.GetResultSize();
            read = aor.GetResultSize();

            BufferSwap();

            if (wrote != mOptions.bufferSize)
            {
                if (wrote != readSize)
                {
                    // this codepath can only do full writes or the first read to align the data to 32 KB
                    // if we don't do a full write, then we failed
                    EA_FAIL_MSG("rw::core::filesys::FileCopier::Copy -> Write failed!\n");
                    currState = FileCopierStateError;
                }
            }

            if (read < mOptions.bufferSize)
            {
                // We either reached the end of file, or we had a read failure.
                if (read == 0)
                {
                    if (GetSize(srcHandle) == offset)
                    {
                        // the filesize is a multiple of the copy buffer size. No more bytes to write
                        currState = FileCopierStateCloseFiles;
                    }
                    else
                    {
                        EA_FAIL_MSG("rw::core::filesys::FileCopier::Copy -> Read failed!\n");
                        currState = FileCopierStateError;
                    }
                }
                currState = FileCopierStateLastWrite;
            }
            break;

        case FileCopierStateLastWrite:
            WaitForDeviceIdle(dstHandle->mDevice, mOptions.dstIdleTime);
            aow.SetTrackDeviceIdleTime(false);
            aow.Write( dstHandle, mWriteBuffer, offset, read, NULL, NULL, mOptions.copyPriority);
            //aow.SetTrackDeviceIdleTime(true);
            wrote = aow.GetResultSize();

            // Check to see if all data has been written
            if( read != wrote )
            {
                if(mOptions.errorCallback)
                    mOptions.errorCallback(srcFilename, dstFilename, FileCopierError_NotEnoughSpace );
                else
                    EA_FAIL_MSG("An error has occurred, but no error handler was set");

                currState = FileCopierStateError;
            }
            else
            {
                mCopiedBytes += wrote;

                if (mOptions.updateCallback)
                    mOptions.updateCallback(srcFilename, dstFilename, wrote, mCopiedBytes);

                currState = FileCopierStateCloseFiles;
            }

            break;

        case FileCopierStateCloseFiles:
            WaitForDeviceIdle(srcHandle->mDevice, mOptions.srcIdleTime);
            aor.SetTrackDeviceIdleTime(false);
            aor.Close( srcHandle );
            //aor.SetTrackDeviceIdleTime(true);

            WaitForDeviceIdle(dstHandle->mDevice, mOptions.dstIdleTime);
            aow.SetTrackDeviceIdleTime(false);
            aow.Close( dstHandle );
            //aow.SetTrackDeviceIdleTime(true);
            aor.Wait();
            aow.Wait();
            srcHandle = NULL;
            dstHandle = NULL;
            currState = FileCopierStateRename;
            break;

        case FileCopierStateRename:
            device = Device::GetInstance(tmpFilename, fullPath);
            WaitForDeviceIdle(device, mOptions.dstIdleTime);
            if ( Move( tmpFilename, dstFilename ) )
            {
                currState = FileCopierStateDone;
            }
            else
            {
                EA_FAIL_FORMATTED( ("rw::core::filesys::FileCopier::Copy -> Failed to rename file %s -> %s", tmpFilename, dstFilename ) );
                currState = FileCopierStateError;
            }
            break;

        case FileCopierStateDone:
        case FileCopierStateError:
            break;
        }

        if(mThreadPause)
        {
            mThreadPauseSemaphore.Wait();
        }
    }

    if (currState != FileCopierStateDone)
    {
        // an error occurred, release any resources owned by
        // this function.
        if (srcHandle != NULL)
        {
            aor.Close(srcHandle);
            aor.Wait();
            srcHandle = NULL;
        }

        if (dstHandle != NULL)
        {
            aow.Close(dstHandle);
            aow.Wait();
            dstHandle = NULL;
        }

        Delete( tmpFilename );

        return false;
    }

    // The copy succeeded
    return true;
}

}}}

