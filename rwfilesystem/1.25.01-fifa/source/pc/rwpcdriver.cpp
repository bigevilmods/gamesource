// (c) Electronic Arts. All Rights Reserved.

#include "EAAssert/eaassert.h"

#include "private_rwprofiler.h"
#include "private_rwhandle.h"
#include "private_rwdevice.h"
#include "rw/core/filesys/driverimpl.h"
#include "rw/core/filesys/directory.h"

#include "platform_sdk.h"

#include <string.h>
#include <new>  // placement new

#if !defined(RWFILESYSTEM_CODEPAGE)
// rwfilesystem now assumes paths are encoded in UTF8 by default instead of ANSI (CP_ACP)
// in order to provide a vector for users to encode wide-char paths, such as those for
// languages like Russian.
#define RWFILESYSTEM_CODEPAGE CP_UTF8
#endif

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

    #if EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)
    explicit DeviceFileHandle(HANDLE FileHandle, const char *filePath, const WCHAR *wFilePath, DWORD AccessMode, DWORD CreateMode, DWORD Attributes)
        : mFileHandle(FileHandle), mCurrPos(0), mCreateMode(CreateMode), mAccessMode(AccessMode), mAttributes(Attributes)
    {
        strcpy(mNativeFileName, filePath);
        wcscpy(mUniFileName, wFilePath);
    }
    #else
    explicit DeviceFileHandle(HANDLE FileHandle, const char *filePath, const WCHAR *wFilePath, DWORD AccessMode, DWORD CreateMode, CREATEFILE2_EXTENDED_PARAMETERS * Params)
        : mFileHandle(FileHandle), mCurrPos(0), mCreateMode(CreateMode), mAccessMode(AccessMode)
    {
        strcpy(mNativeFileName, filePath);
        wcscpy(mUniFileName, wFilePath);
        if (Params)
        {
            mExtendedParams = *Params;
        }
    }
    #endif

    HANDLE GetHandle() const
    {
        return mFileHandle;
    }

    void SetHandle(HANDLE hHandle)
    {
        mFileHandle = hHandle;
    }


    DWORD GetCreateMode() const
    { 
        return mCreateMode; 
    }

    #if EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)
    DWORD GetAttributes() const
    {
        return mAttributes;
    }
    #else
    const LPCREATEFILE2_EXTENDED_PARAMETERS GetCreateParams() const
    {
        return (LPCREATEFILE2_EXTENDED_PARAMETERS)(&mExtendedParams);
    }
    #endif


    DWORD GetAccessMode() const
    { 
        return mAccessMode; 
    }

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

    const char *GetFilePath() const
    { 
        return mNativeFileName; 
    }

    const WCHAR *GetFilePathW() const
    { 
        return mUniFileName; 
    }


private:
    HANDLE mFileHandle;
    uint64_t mCurrPos;
    DWORD  mCreateMode;
    DWORD  mAccessMode;
    char mNativeFileName[FILESYS_MAX_PATH];
    WCHAR mUniFileName[MAX_PATH];
    #if EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)
    DWORD mAttributes;
    #else
    CREATEFILE2_EXTENDED_PARAMETERS mExtendedParams;
    #endif
};

void *  PCFileDeviceDriver::Open(const char *name, unsigned int oflags, Handle **)
{
    DWORD accessmode = GENERIC_READ;
    DWORD createmode = OPEN_EXISTING;
    DWORD attributes = FILE_ATTRIBUTE_NORMAL;
    DWORD fileFlags = 0;

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
        fileFlags |= FILE_FLAG_NO_BUFFERING;

    if ((oflags & AsyncOp::FOPEN_SEQUENTIAL) == AsyncOp::FOPEN_SEQUENTIAL)
        fileFlags |= FILE_FLAG_SEQUENTIAL_SCAN;

    char LocalFileName[FILESYS_MAX_PATH];
    const char *pFileName = name;

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

        if (name[1] != '\\')
        {
            // Only do this if the path is not in UNC format.
            strcpy(LocalFileName, GetName());
            strcat(LocalFileName, name);
            pFileName = LocalFileName;
        }
    }

    WCHAR uniFileName[MAX_PATH];
    if (::MultiByteToWideChar(RWFILESYSTEM_CODEPAGE, 0, pFileName, -1, uniFileName, MAX_PATH) != 0)
    {
        #if EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)

            HANDLE hFile = CreateFileW(uniFileName, accessmode, FILE_SHARE_READ,
                NULL, createmode, (attributes | fileFlags), NULL);
        #else  
            CREATEFILE2_EXTENDED_PARAMETERS temp_CREATEFILE2_EXTENDED_PARAMETERS;
            temp_CREATEFILE2_EXTENDED_PARAMETERS.dwSize = sizeof(temp_CREATEFILE2_EXTENDED_PARAMETERS);
            temp_CREATEFILE2_EXTENDED_PARAMETERS.dwFileAttributes = attributes;
            temp_CREATEFILE2_EXTENDED_PARAMETERS.dwFileFlags = fileFlags;
            temp_CREATEFILE2_EXTENDED_PARAMETERS.dwSecurityQosFlags = SECURITY_ANONYMOUS;
            temp_CREATEFILE2_EXTENDED_PARAMETERS.lpSecurityAttributes = NULL;
            temp_CREATEFILE2_EXTENDED_PARAMETERS.hTemplateFile = NULL;

            HANDLE hFile = CreateFile2(uniFileName, accessmode, FILE_SHARE_READ, 
                createmode, &temp_CREATEFILE2_EXTENDED_PARAMETERS);
        #endif

        if (hFile != INVALID_HANDLE_VALUE)
        {
            #if EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)
            return new DeviceFileHandle(hFile, pFileName, uniFileName, accessmode, createmode, (attributes | fileFlags));
            #else
            return new DeviceFileHandle(hFile, pFileName, uniFileName, accessmode, createmode, &temp_CREATEFILE2_EXTENDED_PARAMETERS);
            #endif
        }
        else
        {
            // Reporting errors on file open can be tricky because people often use 
            // Open to check if a file exists.
            Manager::GetInstance()->GetErrorHandler()(
                static_cast<int>(GetLastError()), Manager::FILEERROR_OPEN, this, pFileName);
        }
    }
    else
    {
        Manager::GetInstance()->GetErrorHandler()(
                static_cast<int>(GetLastError()), Manager::FILEERROR_OPEN, this, pFileName);
    }

    return NULL;
}

void PCFileDeviceDriver::Close(void *fh)
{
    // the input file handle can be NULL if it failed to open.
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);
    CloseHandle(h->GetHandle());
    delete h;
}

void PCFileDeviceDriver::Flush(void *fh)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);
	FlushFileBuffers(h->GetHandle());
}

#pragma warning(disable: 4189)  // local variable is initialized but not referenced.

unsigned int PCFileDeviceDriver::Read(void *fh, void *buf, unsigned int bufsize, DeviceDriver *, void *)
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

    BOOL success = FALSE;

    if (Manager::GetInstance()->GetErrorInjectionFunction()(this, h->GetFilePath()))
    {
        success = ReadFile(h->GetHandle(), buf, bufsize, &NumberOfBytesRead, NULL);

        (void)success;
    }

    if (success == FALSE)
    {
        hadError = true;
        int iError = Manager::FILEERROR_READ;

errorRetryReadOpen:
        if(Manager::GetInstance()->GetErrorHandler()(static_cast<int>(GetLastError()), iError, this, h->GetFilePath()))
        {
            if (h->GetHandle() != INVALID_HANDLE_VALUE)
                CloseHandle(h->GetHandle());

            // wait a 10 ms before retrying.  The failure may have occurred
            // due to a heavy load on the HDD and waiting for some time to
            // pass before retrying will allow the read to succeed.
            EA::Thread::ThreadSleep(10);

            #if EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)
            HANDLE hFile = CreateFileW(h->GetFilePathW(), h->GetAccessMode(), FILE_SHARE_READ, NULL, h->GetCreateMode(), h->GetAttributes(), NULL);
            #else  
            HANDLE hFile = CreateFile2(h->GetFilePathW(), h->GetAccessMode(), FILE_SHARE_READ, h->GetCreateMode(), h->GetCreateParams());
            #endif

            if (hFile == INVALID_HANDLE_VALUE)
            {
                iError = Manager::FILEERROR_OPEN;
	            h->SetHandle(INVALID_HANDLE_VALUE);
                goto errorRetryReadOpen;
            }

            h->SetHandle(hFile);

            // seek to correct pos
            uint64_t fpos = h->GetPos();
            if (!SetFilePointerEx(h->GetHandle(), *reinterpret_cast<LARGE_INTEGER *>(&fpos), NULL, FILE_BEGIN))
            {
                iError = Manager::FILEERROR_SEEK;
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
        Manager::GetInstance()->GetErrorHandler()(ERROR_SUCCESS, Manager::FILEERROR_READ, this, h->GetFilePath());
    }

    h->MovePointer(NumberOfBytesRead);
    return NumberOfBytesRead;
}

unsigned int    PCFileDeviceDriver::Write(void *fh, const void *buf, unsigned int bufsize, DeviceDriver *, void *)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);
    DWORD NumberOfBytesWritten;
    BOOL success = WriteFile(h->GetHandle(), buf, bufsize, &NumberOfBytesWritten, NULL);

    if (success != TRUE)
    {
        // If the write fails, let the user know.  This is not necessarily
        // fatal because either the file was opened with the wrong permissions
        // or the disk is full.  We coul throw an assert if the error is
        // anything other than disk full, but I think it's better to be
        // consistent and let the user halt the system if the error is not
        // recoverable.
#if defined(EA_FINAL)
        Manager::GetInstance()->GetErrorHandler()(
            static_cast<int>(GetLastError()), Manager::FILEERROR_WRITE, this, NULL);
#else
        Manager::GetInstance()->GetErrorHandler()(
            static_cast<int>(GetLastError()), Manager::FILEERROR_WRITE, this, h->GetFilePath());
#endif
        return 0;
    }

    h->MovePointer(NumberOfBytesWritten);
    return NumberOfBytesWritten;
}

uint64_t    PCFileDeviceDriver::Seek(void *fh, uint64_t offset, int whence, DeviceDriver *, void *)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);

    // the driver only needs to handle seeks from the origin becuase the caller
    // (the Async module) keeps track of the file position
    EA_ASSERT(whence == FSEEK_BEGIN);

    // don't bother doing anything if we're
    // already at the correct position.
    if (offset == h->GetPos())
        return offset;

    LARGE_INTEGER newFilePointer;

    if (!SetFilePointerEx(h->GetHandle(), *reinterpret_cast<LARGE_INTEGER *>(&offset),
        &newFilePointer, FILE_BEGIN))
    {
        #if EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)
            // if we failed to set the file pointer using SetFilePointerEx, then
            // get the current file pointer position
            newFilePointer.QuadPart = SetFilePointer(h->GetHandle(), 0, NULL, FILE_CURRENT);
        #else
            // On some platforms there is no SetFilePointer() function we can fall back to
            EA_FAIL_FORMATTED(("Seek failed for file %s", h->GetFilePath()));
            return h->GetPos();
        #endif
    }

    // This is a hack.  If another root is added to a device,
    // if a file is opened on that device with a non-primary
    // root, the wrong device name will be called.  I can live
    // with this though because on the PC, it will be very rare
    // for a user to add another root to a library device because
    // each drive letter has its own execution thread, even if that
    // maps onto the same physical device as another letter.  We
    // let the O/S sort them out.
    LogSeekEvent(GetName(), 
            static_cast<int>(reinterpret_cast<intptr_t>(h->GetHandle())),
            whence,
            h->GetPos(),
            static_cast<uint64_t>(newFilePointer.QuadPart));

    h->SetPos(static_cast<uint64_t>(newFilePointer.QuadPart));

    return h->GetPos();
}

uint64_t    PCFileDeviceDriver::GetSize(void *fh)
{
    const DeviceFileHandle *h = static_cast<const DeviceFileHandle *>(fh);
    
    #if EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)
        LARGE_INTEGER fileSize;

        if (GetFileSizeEx(h->GetHandle(), &fileSize) == TRUE)
        {
            return static_cast<uint64_t>(fileSize.QuadPart);
        }
    #else
        // Save the current file pointer position
        LARGE_INTEGER currentPos; 
        currentPos.QuadPart = 0;
        const BOOL bResult = SetFilePointerEx(h->GetHandle(), currentPos, &currentPos, FILE_CURRENT);

        if (bResult)
        {
            // Move to the end of the file
            LARGE_INTEGER endPos; 
            endPos.QuadPart = 0;
            const BOOL bResult2 = SetFilePointerEx(h->GetHandle(), endPos, &endPos, FILE_END);

            // Set back to the original position
            const BOOL bResult3 = SetFilePointerEx(h->GetHandle(), currentPos, NULL, FILE_BEGIN);
            // If this fails, it means we could not restore the file pointer to the original position
            // This could trigger an issue later in the calling code
            EA_ASSERT(bResult3);

            if (bResult2)
            {
                return static_cast<uint64_t>(endPos.QuadPart);
            }
        }
    #endif

    return 0;
}

bool        PCFileDeviceDriver::Resize(void *fh, uint64_t newsize)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);

    LARGE_INTEGER liNewSize;
    liNewSize.QuadPart = static_cast<LONGLONG>(newsize);

    SetFilePointerEx(h->GetHandle(), liNewSize, NULL, FILE_BEGIN);

    bool retval = SetEndOfFile(h->GetHandle()) == TRUE;

    LARGE_INTEGER liOrigPos;
    liOrigPos.QuadPart = static_cast<LONGLONG>(h->GetPos());

    SetFilePointerEx(h->GetHandle(), liOrigPos, NULL, FILE_BEGIN);

    return retval;
}

bool PCFileDeviceDriver::Delete(const char *filePath)
{
    WCHAR uniFileName[MAX_PATH];
    if (::MultiByteToWideChar(RWFILESYSTEM_CODEPAGE, 0, filePath, -1,
        uniFileName, MAX_PATH) != 0)
    {
        return ::DeleteFileW(uniFileName) == TRUE;
    }

    return false;
}

bool PCFileDeviceDriver::Move(const char *currentName, const char *newName)
{
    WCHAR uniCurrentName[MAX_PATH];
    WCHAR uniNewName[MAX_PATH];

    if ((::MultiByteToWideChar(RWFILESYSTEM_CODEPAGE, 0, currentName, -1, uniCurrentName, MAX_PATH) != 0) &&
        (::MultiByteToWideChar(RWFILESYSTEM_CODEPAGE, 0, newName, -1, uniNewName, MAX_PATH) != 0))
    {
        #if EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)
            return ::MoveFileW(uniCurrentName, uniNewName) == TRUE;
        #else
            return ::MoveFileExW(uniCurrentName, uniNewName, 0) == TRUE;
        #endif
    }

    return false;
}

void * PCFileDeviceDriver::FindBegin(const char *filePattern, FindData *findData)
{
    WIN32_FIND_DATAW localFindData;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DeviceFileHandle *fileHandle = NULL;

    WCHAR uniFilePattern[MAX_PATH];
    if (::MultiByteToWideChar(RWFILESYSTEM_CODEPAGE, 0, filePattern, -1, uniFilePattern, MAX_PATH) == 0)
        return NULL;

    #if EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)
        hFile = ::FindFirstFileW(uniFilePattern, &localFindData);
    #else
        hFile = ::FindFirstFileExW(uniFilePattern, FindExInfoBasic, &localFindData, FindExSearchNameMatch, NULL, 0);
    #endif

    if (hFile != INVALID_HANDLE_VALUE)
    {
        fileHandle = new DeviceFileHandle(hFile, filePattern, uniFilePattern, 0, 0, NULL);
        if (findData != NULL)
        {
            if (::WideCharToMultiByte(RWFILESYSTEM_CODEPAGE, 0, localFindData.cFileName, -1, findData->mName, FILESYS_MAX_PATH, 0, 0) == 0)
            {
                delete fileHandle;
                return NULL;
            }

            findData->mCreationTime = *reinterpret_cast<const uint64_t *>(&localFindData.ftCreationTime);
            findData->mAccessTime = *reinterpret_cast<const uint64_t *>(&localFindData.ftLastAccessTime);
            findData->mModificationTime = *reinterpret_cast<const uint64_t *>(&localFindData.ftLastWriteTime);
            findData->mSize = (static_cast<uint64_t>(localFindData.nFileSizeHigh) << 32) | localFindData.nFileSizeLow;
            if (::MultiByteToWideChar(RWFILESYSTEM_CODEPAGE, 0, findData->mName, -1, localFindData.cFileName, FILESYS_MAX_PATH) == 0)
            {
                delete fileHandle;
                return NULL;
            }
            findData->mName[FILESYS_MAX_PATH - 1] = 0;

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

bool PCFileDeviceDriver::FindNext(void *h, FindData *findData)
{
    WIN32_FIND_DATAW localFindData;
    const DeviceFileHandle *fh = static_cast<const DeviceFileHandle *>(h);

    bool result = ::FindNextFileW(fh->GetHandle(), &localFindData) == TRUE;
    if (result && (findData != NULL))
    {
        if (::WideCharToMultiByte(RWFILESYSTEM_CODEPAGE, 0, localFindData.cFileName, -1, findData->mName, FILESYS_MAX_PATH, 0, 0) == 0)
        {
            return NULL;
        }

        findData->mCreationTime = *reinterpret_cast<const uint64_t *>(&localFindData.ftCreationTime);
        findData->mAccessTime = *reinterpret_cast<const uint64_t *>(&localFindData.ftLastAccessTime);
        findData->mModificationTime = *reinterpret_cast<const uint64_t *>(&localFindData.ftLastWriteTime);
        findData->mSize = (static_cast<uint64_t>(localFindData.nFileSizeHigh) << 32) | localFindData.nFileSizeLow;
        if (::MultiByteToWideChar(RWFILESYSTEM_CODEPAGE, 0, findData->mName, -1, localFindData.cFileName, FILESYS_MAX_PATH) == 0)
        {
            return NULL;
        }
        findData->mName[FILESYS_MAX_PATH - 1] = 0;

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

bool PCFileDeviceDriver::FindEnd(void *h)
{
    DeviceFileHandle *fh = static_cast<DeviceFileHandle *>(h);
    bool result = ::FindClose(fh->GetHandle()) == TRUE;
    delete fh;
    return result;
}

bool PCFileDeviceDriver::DirectoryCreate(const char *directory)
{
    WCHAR uniDirectory[MAX_PATH];
    if (::MultiByteToWideChar(RWFILESYSTEM_CODEPAGE, 0, directory, -1, uniDirectory, MAX_PATH) != 0)
    {
        return ::CreateDirectoryW(uniDirectory, NULL) == TRUE;
    }

    return false;
}

bool PCFileDeviceDriver::DirectoryRemove(const char *directory)
{
    WCHAR uniDirectory[MAX_PATH];
    if (::MultiByteToWideChar(RWFILESYSTEM_CODEPAGE, 0, directory, -1, uniDirectory, MAX_PATH) != 0)
    {
        return ::RemoveDirectoryW(uniDirectory) == TRUE;
    }

    return false;
}

bool PCFileDeviceDriver::GetNameFromHandle(void *fh, char *name, size_t nameBufferSize)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);

    EA_ASSERT(h != NULL);

    strncpy(name, h->GetFilePath(), nameBufferSize);

    name[nameBufferSize-1] = '\0';

    return true;
}

}}}
