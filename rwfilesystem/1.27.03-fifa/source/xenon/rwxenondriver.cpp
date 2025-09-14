// (c) Electronic Arts. All Rights Reserved.

#include <EAAssert/eaassert.h>
#include <EAStdC/EAString.h>
#include "platform_sdk.h"

#include "private_rwhandle.h"
#include "private_rwdevice.h"
#include "private_rwprofiler.h"
#include "rw/core/filesys/device.h"
#include "rw/core/filesys/driverimpl.h"


namespace rw {
namespace core {
namespace filesys {

// class DeviceFileHandle contains everything that a device driver
// needs to keep track of for the open file.  Each platform will have
// different requirements here.
class DeviceFileHandle
{
public:
    static void *operator new(size_t size)
    {
        return RWFILESYSTEM_ALLOCATE(size);
    }

    static void operator delete(void *p)
    {
        return Manager::Free(p);
    }

    DeviceFileHandle(const char *NativeFileName,
        HANDLE FileHandle, DWORD CreateMode,
        DWORD AccessMode, DWORD Attributes)
        : mFileHandle(FileHandle),
        mCreateMode(CreateMode), mAccessMode(AccessMode),
        mAttributes(Attributes), mCurrPos(0)
    {
        memset(mNativeFileName, 0, FILESYS_MAX_PATH);
        if (NativeFileName != NULL)
        {
            EA_STRLCPY_VERIFY(mNativeFileName ,NativeFileName, FILESYS_MAX_PATH);
        }
    }

    HANDLE GetHandle() const
    {
        return mFileHandle;
    }

    void SetHandle(HANDLE hHandle)
    {
        mFileHandle = hHandle;
    }

    const char *GetName() const
    { return mNativeFileName; }

    DWORD GetCreateMode() const
    { return mCreateMode; }

    DWORD GetAttributes() const
    { return mAttributes; }

    DWORD GetAccessMode() const
    { return mAccessMode; }

    void MovePointer(DWORD amount)
    {
        mCurrPos += amount;
    }

    void SetPos(uint64_t pos)
    {
        mCurrPos = pos;
    }

    uint64_t GetPos() const
    {
        return mCurrPos;
    }

private:
    HANDLE mFileHandle;
    char   mNativeFileName[FILESYS_MAX_PATH];
    DWORD  mCreateMode;
    DWORD  mAccessMode;
    DWORD  mAttributes;
    uint64_t mCurrPos;
};

XenonFileDeviceDriver::XenonFileDeviceDriver()
: mThroughputLimit(0), mLastReadSize(0), mLastReadTime(0), mQueryLocationEnabled(false)
{
    // empty
}

void XenonFileDeviceDriver::SetThroughputLimit(unsigned int KBperSec)
{
    mThroughputLimit = KBperSec;

    if (mThroughputLimit == 0)
    {
        mLastReadTime = 0;
        mLastReadSize = 0;
    }
    else
    {
        mLastReadTime = EA::Thread::GetThreadTime();
        mLastReadSize = 0;
    }
}

void*  XenonFileDeviceDriver::Open(const char *name, unsigned int oflags, Handle **)
{
    DWORD accessmode = GENERIC_READ;
    DWORD createmode = OPEN_EXISTING;
    DWORD attributes = FILE_ATTRIBUTE_NORMAL;

    // choose access, sharing, creation, and attributes
    if ((oflags & AsyncOp::FOPEN_READWRITE) == AsyncOp::FOPEN_READWRITE)
    {
        accessmode |= GENERIC_WRITE;
    }

    switch(oflags & (AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE))
    {
    case 0:
        /* createmode is already correct */ break;

    case AsyncOp::FOPEN_CREATE:
        createmode = CREATE_NEW; break;

    case AsyncOp::FOPEN_OVERWRITE:
        createmode = TRUNCATE_EXISTING; break;

    case AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE:
        createmode = CREATE_ALWAYS; break;
    }

    if ((oflags & AsyncOp::FOPEN_NOBUFFERING) == AsyncOp::FOPEN_NOBUFFERING)
        attributes |= FILE_FLAG_NO_BUFFERING;

    if ((oflags & AsyncOp::FOPEN_SEQUENTIAL) == AsyncOp::FOPEN_SEQUENTIAL)
        attributes |= FILE_FLAG_SEQUENTIAL_SCAN;

    char LocalFileName[FILESYS_MAX_PATH];

    if ((name[0] == '/') || (name[0] == '\\'))
    {
        // if the path name does not include a device specifer but
        // is fully-qualified, (begins with a directory-separator
        // character), the device name must be prepended to the
        // path.  If this is not done, CreateFile will assume you
        // are trying to open a file in the root directory of the
        // device where the current working directory is, when what
        // you really want to do is open a file in the root
        // directory of the first device in the search path.
        EA_STRLCPY_VERIFY(LocalFileName, GetName(), EAArrayCount(LocalFileName));
        EA_STRLCAT_VERIFY(LocalFileName, name, EAArrayCount(LocalFileName));
    }
    else
    {
        if (name[0]=='.' && (name[1]=='/' || name[1]=='\\'))
        {
            EA_STRLCPY_VERIFY(LocalFileName, Manager::GetInstance()->GetCWD(), EAArrayCount(LocalFileName));
            EA_STRLCAT_VERIFY(LocalFileName, "\\", EAArrayCount(LocalFileName));
            EA_STRLCAT_VERIFY(LocalFileName, name + 2, EAArrayCount(LocalFileName));
        }
        else
        {
            EA_STRLCPY_VERIFY(LocalFileName, name, EAArrayCount(LocalFileName));
        }
    }

    char nativeFileName[FILESYS_MAX_PATH];
    ReplaceSlashes(nativeFileName, LocalFileName);
    HANDLE hFile = CreateFile(nativeFileName, accessmode,
        FILE_SHARE_READ, NULL, createmode, attributes, NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        return new DeviceFileHandle(nativeFileName, hFile,
            createmode, accessmode, attributes);
    }
    else
    {
        Manager::GetInstance()->GetErrorHandler()(
            static_cast<int>(GetLastError()), Manager::FILEERROR_OPEN, this, nativeFileName);
    }

    return NULL;
}

void        XenonFileDeviceDriver::Close(void *fh)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);
    if (h->GetHandle() != INVALID_HANDLE_VALUE)
        CloseHandle(h->GetHandle());
    delete h;
}

#pragma warning(disable: 4189)  // local variable is initialized but not referenced.

unsigned int    XenonFileDeviceDriver::Read(void *fh, void *buf, unsigned int bufsize, DeviceDriver *, void *)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);
    DWORD NumberOfBytesRead = 0;
    bool hadError = false;

errorRetryRead:

    // return 0 bytes read if the handle was previously closed
    // (because of a read error)
    if (h->GetHandle() == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    // touch the first & last bytes of the buffer before calling
    // ReadFile().  This allows virtual memory to be committed via
    // a structured exception handler.  The performance impact
    // should be negligible, so we won't have a configuration
    // option to enable/disable it.
    *((volatile char *)buf+0) = *((volatile char *)buf+0);
    *((volatile char *)buf+bufsize-1) = *((volatile char *)buf+bufsize-1);

    if (mThroughputLimit != 0)
    {
        // calculate the minimum amount of time that needs to pass before
        // the next read can be processed.
        int64_t timeUntilNextRead = mLastReadSize / mThroughputLimit;

        // Get the current system time
        EA::Thread::ThreadTime currTime = EA::Thread::GetThreadTime();

        int64_t delayTime = timeUntilNextRead - (currTime - mLastReadTime);

        if (delayTime > 0)
        {
            // put this thread to sleep so the throughput is reduced
            EA::Thread::ThreadSleep(static_cast<EA::Thread::ThreadTime>(delayTime));
        }

        mLastReadTime = EA::Thread::GetThreadTime();
        mLastReadSize = 0;
    }

    BOOL success = FALSE;

    if (Manager::GetInstance()->GetErrorInjectionFunction()(this, h->GetName()))
    {
        success = ReadFile(h->GetHandle(), buf, bufsize, &NumberOfBytesRead, NULL);

        mLastReadSize = NumberOfBytesRead;
    }

    if (success == FALSE)
    {
        hadError = true;

errorRetryReadOpen:
        if(Manager::GetInstance()->GetErrorHandler()(
            static_cast<int>(GetLastError()), Manager::FILEERROR_READ, this, h->GetName()))
        {
            CloseHandle(h->GetHandle());

            HANDLE hFile = CreateFile(h->GetName(), h->GetAccessMode(),
                FILE_SHARE_READ, NULL, h->GetCreateMode(), h->GetAttributes(), NULL);

            if (hFile == INVALID_HANDLE_VALUE)
                goto errorRetryReadOpen;

            h->SetHandle(hFile);

            // seek to correct pos
            uint64_t fpos = h->GetPos();
            if (!SetFilePointerEx(h->GetHandle(),
                *reinterpret_cast<LARGE_INTEGER *>(&fpos), NULL, FILE_BEGIN))
            {
                goto errorRetryReadOpen;
            }

            goto errorRetryRead;
        }

        // The user has chosen not to retry the read.  The library will
        // return any data that was read up to the point at which the
        // failure ocurred.  It is up to the application to provide a
        // fallback method of getting the rest of the data.

        // hadError will remain set, so the user will get a final error
        // callback stating success.  I hope that's not too misleading.
        // The "success" is that we are no longer blocking waiting on
        // data - we've given up and are now out of the wait loop.
    }

    if(hadError)
    {
        Manager::GetInstance()->GetErrorHandler()(ERROR_SUCCESS, Manager::FILEERROR_READ, this, h->GetName());
    }

    h->MovePointer(NumberOfBytesRead);

    return NumberOfBytesRead;
}

unsigned int    XenonFileDeviceDriver::Write(void *fh, const void *buf, unsigned int bufsize, DeviceDriver *, void *)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);
    DWORD NumberOfBytesWritten = 0;

    BOOL success = FALSE;

    while(success == FALSE)
    {
        success = WriteFile(h->GetHandle(), buf, bufsize, &NumberOfBytesWritten, NULL);

        if (success != TRUE)
        {
            // If the write fails, let the user know and give the game a chance to
            // repeat the operation as many times as they wish.  From Bob Dawe,
            // in response to a question about whether a second attempt to
            // perform a write ever succeeds when the first attempt failed:
            //
            // That’s an interesting question.  This is mostly speculative but
            // I think its possible on certain xbox hardware when it’s under
            // heavy load.  The problem reports we saw were from a small
            // percentage (<5%) of xbox NHL 2009 owners on the EA forums who
            // encountered the original problem we (believe was) fixed in the
            // patch.  We never managed to reliably reproduce the issue across
            // all our QA testing.  I say heavy load because the problem
            // reports were mainly when the game was doing a lot of disc + hdd
            // access – concurrently caching to the hdd, streaming music off
            // disc and reading data moving between FE screens from both disc
            // and hdd.
            //
            // So I would say it’s possible but cannot say with any certainty
            // that its worth doing a retry.  This doesn’t really give you an
            // answer for the implementation though.  I don’t think it can do
            // any harm, as it really should be the edge case, and if it does
            // fail again the caching code seems to handle failing and moving
            // onto the next file.

            if (Manager::GetInstance()->GetErrorHandler()(
                static_cast<int>(GetLastError()), Manager::FILEERROR_WRITE, this, h->GetName()))
            {
                // wait a 10 ms before retrying.  The failure may have occurred
                // due to a heavy load on the HDD and waiting for some time to
                // pass before retrying will allow the write to succeed.
                EA::Thread::ThreadSleep(10);
            }
            else
            {
                // do not retry any more.  This should not be fatal as Xbox
                // games must be designed to work without a hard disk.
                NumberOfBytesWritten = 0;
                success = TRUE;
            }
        }
    }

    h->MovePointer(NumberOfBytesWritten);
    return NumberOfBytesWritten;
}

uint64_t    XenonFileDeviceDriver::Seek(void *fh, uint64_t offset, int whence, DeviceDriver *, void *)
{
	uint64_t newPos(0);
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);

    // the driver only needs to handle seeks from the origin becuase the caller
    // (the Async module) keeps track of the file position
    EA_ASSERT(whence == static_cast<int>(FSEEK_BEGIN));

    // don't bother doing anything if we're
    // already at the correct position.
    // Also don't do anything if the handle has been closed due to
    // a read error
    if (offset == h->GetPos() || h->GetHandle() == INVALID_HANDLE_VALUE)
    {
		newPos = offset;
	}
	else
	{
		LARGE_INTEGER newFilePointer;

		if (!SetFilePointerEx(h->GetHandle(), *reinterpret_cast<LARGE_INTEGER *>(&offset),
			&newFilePointer, FILE_BEGIN))
		{
			// if we failed to set the file pointer using SetFilePointerEx, then
			// get the current file pointer position
			newFilePointer.QuadPart = SetFilePointer(h->GetHandle(), 0, NULL, FILE_CURRENT);
		}

    	LogSeekEvent(GetName(), reinterpret_cast<int>(h->GetHandle()), whence, h->GetPos(), static_cast<uint64_t>(newFilePointer.QuadPart));

		h->SetPos(static_cast<uint64_t>(newFilePointer.QuadPart));
		newPos = h->GetPos();
	}

    return newPos;
}

uint64_t    XenonFileDeviceDriver::GetSize(void *fh)
{
    const DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);
    LARGE_INTEGER fileSize;

    if (GetFileSizeEx(h->GetHandle(), &fileSize) == TRUE)
    {
        return static_cast<uint64_t>(fileSize.QuadPart);
    }

    return 0;
}

bool XenonFileDeviceDriver::Resize(void *fh, uint64_t newsize)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);
    HANDLE hFile = h->GetHandle();

    LARGE_INTEGER currentFilePosition;
    LARGE_INTEGER startFilePosition;
    startFilePosition.QuadPart = 0;

    BOOL success = SetFilePointerEx(
        hFile,                  // must have GENERIC_READ and/or GENERIC_WRITE
        startFilePosition,      // do not move pointer
        &currentFilePosition,   // get the current file position
        FILE_CURRENT);          // provides offset from current position

    success = success && SetFilePointerEx(hFile,
        *reinterpret_cast<LARGE_INTEGER *>(&newsize), NULL, FILE_BEGIN);
    success = success && SetEndOfFile(hFile);
    success = success && SetFilePointerEx(hFile, currentFilePosition,
        NULL, FILE_BEGIN);

    if (success)
    {
        h->SetPos(static_cast<uint64_t>(currentFilePosition.QuadPart));
    }

    return success == TRUE;
}

uint64_t XenonFileDeviceDriver::QueryLocation(void *fh)
{
    // Only do this check on the DVD device.  All other devices will return
    // an error code and it may cause the current thread to stall (refer to
    // DT#31128)
    if (!mQueryLocationEnabled)
        return 0;

    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);

    uint64_t sector = XGetFilePhysicalSortKey(h->GetHandle());

    if (sector == 0xffffffff)
    {
        // this is a recoverable error, because an application can
        // still work (perhaps just not quite as well) without a
        // working QueryLocation function.
        return 0;
    }

    // multiply the returned sortkey, which is the starting sector
    // number of the file, by 2048.  Even if the volume sector size
    // is less than 2K, it doesn't matter because we are going to be
    // comparing relative locations.  The purpose of the multiplication
    // is to allow finer comparisons (inside zipfiles, which usually
    // won't be multiples of sector offsets).
    return sector * 2048ui64;
}

bool XenonFileDeviceDriver::Delete(const char *filePath)
{
    char drvPath[FILESYS_MAX_PATH];

    return ::DeleteFile(ReplaceSlashes(drvPath, filePath)) == TRUE;
}



//This is a Utility Function
//Xenon's MoveFile requires that there is no trailing seperator (even for directories)
//This function assumes trailing seperator is backslash not forward slash. 
//If needed, call ReplaceSlashes() before calling XenonRemoveTrailingSeparator()
char* XenonRemoveTrailingSeparator(char* str)
{
    size_t strLen = EA::StdC::Strlen(str);
    if(strLen > 0 && str[strLen-1] == '\\')
    {
        str[strLen-1] = '\0';
    }
	return str;
}

bool XenonFileDeviceDriver::Move(const char *currentName, const char *newName)
{
    char drvCurrName[FILESYS_MAX_PATH], drvNewName[FILESYS_MAX_PATH];
    //Xenon cannot have trailing slash in MoveFile arguments
    //http://msdn.microsoft.com/en-us/library/ms128010(v=vs.110).aspx mentions this for their
    //filesystem, but it is strange we don't also see this restriction on other MSFT platforms
    return ::MoveFile(XenonRemoveTrailingSeparator(ReplaceSlashes(drvCurrName, currentName)),
        XenonRemoveTrailingSeparator(ReplaceSlashes(drvNewName, newName))) == TRUE;

}

void * XenonFileDeviceDriver::FindBegin(const char *filePattern, FindData *findData)
{
    WIN32_FIND_DATA localFindData;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DeviceFileHandle *fileHandle = NULL;
    char drvFilePattern[FILESYS_MAX_PATH];

    hFile = ::FindFirstFile(ReplaceSlashes(drvFilePattern, filePattern), &localFindData);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        fileHandle = new DeviceFileHandle(NULL, hFile,
            0, 0, localFindData.dwFileAttributes);
        if (findData != NULL)
        {
            findData->mCreationTime = *reinterpret_cast<const uint64_t *>(&localFindData.ftCreationTime);
            findData->mAccessTime = *reinterpret_cast<const uint64_t *>(&localFindData.ftLastAccessTime);
            findData->mModificationTime = *reinterpret_cast<const uint64_t *>(&localFindData.ftLastWriteTime);
            findData->mSize = (static_cast<uint64_t>(localFindData.nFileSizeHigh) << 32) | localFindData.nFileSizeLow;
            EA_STRLCPY_VERIFY(findData->mName, localFindData.cFileName, FILESYS_MAX_PATH);

            findData->mFlags = 0;

            if ((localFindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
            {
                findData->mFlags |= DirectoryEntry::FLAG_READONLY;
            }

            if ((localFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                findData->mFlags |= DirectoryEntry::FLAG_DIRECTORY;
            }
        }
    }

    return fileHandle;
}

bool XenonFileDeviceDriver::FindNext(void *h, FindData *findData)
{
    WIN32_FIND_DATA localFindData;
    const DeviceFileHandle *fh = static_cast<const DeviceFileHandle *>(h);

    bool result = ::FindNextFile(fh->GetHandle(), &localFindData) == TRUE;
    if (result && (findData != NULL))
    {
        findData->mCreationTime = *reinterpret_cast<const uint64_t *>(&localFindData.ftCreationTime);
        findData->mAccessTime = *reinterpret_cast<const uint64_t *>(&localFindData.ftLastAccessTime);
        findData->mModificationTime = *reinterpret_cast<const uint64_t *>(&localFindData.ftLastWriteTime);
        findData->mSize = (static_cast<uint64_t>(localFindData.nFileSizeHigh) << 32) | localFindData.nFileSizeLow;
        EA_STRLCPY_VERIFY(findData->mName, localFindData.cFileName, FILESYS_MAX_PATH);

        findData->mFlags = 0;

        if ((localFindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
        {
            findData->mFlags |= DirectoryEntry::FLAG_READONLY;
        }

        if ((localFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            findData->mFlags |= DirectoryEntry::FLAG_DIRECTORY;
        }
    }
    return result;
}

bool XenonFileDeviceDriver::DirectoryCreate(const char *directory)
{
    char localDir[FILESYS_MAX_PATH];
    return ::CreateDirectoryA(ReplaceSlashes(localDir, directory), NULL) == TRUE;
}

bool XenonFileDeviceDriver::DirectoryRemove(const char *directory)
{
    char localDir[FILESYS_MAX_PATH];
    return ::RemoveDirectoryA(ReplaceSlashes(localDir, directory)) == TRUE;
}

bool XenonFileDeviceDriver::FindEnd(void *h)
{
    DeviceFileHandle *fh = static_cast<DeviceFileHandle *>(h);
    bool result = ::FindClose(fh->GetHandle()) == TRUE;
    delete fh;
    return result;
}

bool XenonFileDeviceDriver::GetNameFromHandle(void *fh, char *name, size_t nameBufferSize)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);

    EA_ASSERT(h != NULL);

    EA_STRLCPY_VERIFY(name, h->GetName(), nameBufferSize);

    return true;
}

//lint -e{956}
XenonFileDeviceDriver gDeviceDriverImpl;
XenonFileDeviceDriver gCacheDriverImpl;

}}}
