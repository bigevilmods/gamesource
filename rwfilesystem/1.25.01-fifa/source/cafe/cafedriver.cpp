// (c) Electronic Arts. All Rights Reserved.

#include <cafe/fs.h>
#include "EAStdC/EAString.h"
#include "EAStdC/EASprintf.h"
#include "EASTL/type_traits.h"
#include "private_rwdevice.h"
#include "private_rwplatformfilehandlecache.h"
#include "rw/core/filesys/device.h"
#include "rw/core/filesys/directory.h"
#include "rw/core/filesys/driverimpl.h"
#include "rw/core/filesys/path.h"

#ifndef MAX
#define MAX(a, b) ((a)>(b))?(a):(b)
#endif

#ifndef MIN
#define MIN(a, b) ((a)<(b))?(a):(b)
#endif

namespace rw {
namespace core {
namespace filesys {

#define CAFE_SIMULATE_ERROR(HANDLE) (Manager::GetInstance()->GetErrorInjectionFunction()(this, HANDLE->GetName()) == false)


static PlatformFileHandleCache gPlatformFileHandleCache;
static const uint32_t CAFE_READWRITE_BUFFER_SIZE = 128 * 1024;


class FindHandle
{
    FSDirHandle mDirHandle;
    char mPattern[FILESYS_MAX_PATH];
    char mDirName[FILESYS_MAX_PATH];

public:
    void *operator new(size_t size)
    {
        return RWFILESYSTEM_ALLOCATE(size);
    }

    void operator delete(void *p)
    {
        Manager::Free(p);
    }

    FindHandle(FSDirHandle dirHandle, const char *pattern, const char *dirName)
        : mDirHandle(dirHandle)
    {
        EA::StdC::Strlcpy(mPattern, pattern, sizeof mPattern);
        EA::StdC::Strlcpy(mDirName, dirName, sizeof mDirName);
    }

    FindHandle(const FindHandle &findHandle)
        : mDirHandle(findHandle.mDirHandle)
    {
        EA::StdC::Strlcpy(mPattern, findHandle.mPattern, sizeof mPattern);
        EA::StdC::Strlcpy(mDirName, findHandle.mDirName, sizeof mDirName);
    }

    FSDirHandle GetDirHandle() const { return mDirHandle; }
    const char *GetFilePattern() const { return mPattern; }
    const char *GetDirectory() const { return mDirName; }
};


void PlatformFileHandleCache::CloseHandle(void *context, PlatformFileHandle fileHandle)
{
    FSCmdBlock commandBlock;
    FSInitCmdBlock(&commandBlock);

    FSStatus status = FSCloseFile(eastl::union_cast<FSClient *>(context), &commandBlock, static_cast<FSFileHandle>(fileHandle), FS_RET_ALL_ERROR);
    EA_ASSERT(status == FS_STATUS_OK);
    EA_UNUSED(status);
}


class DeviceFileHandle
{
    uint64_t mCurrentPosition;
    char mFileName[FILESYS_MAX_PATH];
    const char *mOpenMode;
    PlatformFileHandleCacheEntry *mCacheEntry;
    FSStatus mLastOpenError;
    FSClient *mClientHandle;
    bool mValid;

    void OpenFile();

public:
    void *operator new(size_t size);
    void operator delete(void *ptr);

    DeviceFileHandle(FSClient *clientHandle, const char *fileName, int flags);
    ~DeviceFileHandle();

    FSFileHandle GetFileDescriptor();
    void TruncateFile();

    void MoveFilePointer(uint64_t amount) { mCurrentPosition += amount; EA_ASSERT(mCurrentPosition < (1ULL << 32ULL)); }
    void SetFilePointer(uint64_t position) { mCurrentPosition = position; EA_ASSERT(mCurrentPosition < (1ULL << 32ULL)); }
    uint64_t GetFilePointer() const { return mCurrentPosition; }
    const char *GetName() const { return mFileName; }
    bool HasValidFileDescriptor() { return mOpenMode != NULL && GetFileDescriptor() >= 0; }
    FSStatus GetLastOpenError() const { return mLastOpenError; }; 
    const char *GetOpenMode() const { return mOpenMode; }
};


void *DeviceFileHandle::operator new(size_t size)
{
    return RWFILESYSTEM_ALLOCATE(size);
}


void DeviceFileHandle::operator delete(void *ptr)
{
    Manager::Free(ptr);
}


DeviceFileHandle::DeviceFileHandle(FSClient *clientHandle, const char *fileName, int flags)
    : mCurrentPosition(0),
      mOpenMode(NULL),
      mCacheEntry(NULL),
      mLastOpenError(FS_STATUS_OK),
      mClientHandle(clientHandle)
{
    FSStat stat;
    FSCmdBlock commandBlock;
    FSInitCmdBlock(&commandBlock);
    FSStatus status = FSGetStat(clientHandle, &commandBlock, fileName, &stat, FS_RET_ALL_ERROR | FS_RET_NOT_FOUND);

    // The only error condition that this ctor works around is if the file does
    // not exist. Any others and we do not complete the initialization process.
    if (status < 0 && status != FS_STATUS_NOT_FOUND)
        return;

    EA::StdC::Strlcpy(mFileName, fileName, sizeof mFileName);

    const int openBehaviorMask = AsyncOp::FOPEN_READWRITE
        | AsyncOp::FOPEN_CREATE
        | AsyncOp::FOPEN_OVERWRITE
        | AsyncOp::FOPEN_READONLY;

    bool fileExists = status == 0;

    switch (flags & openBehaviorMask)
    {
        case AsyncOp::FOPEN_READONLY:
            mOpenMode = "r";
            break;
        case AsyncOp::FOPEN_OVERWRITE: // Files cannot be overwritten without the FOPEN_READWRITE flag.
            break;
        case AsyncOp::FOPEN_CREATE:
            if(!fileExists)
            {
                TruncateFile();

                // We use "r" here because we want the FSOpenFile to have a valid flag and "r" is the least intrusive. The file is already truncated at 
                // this point so there is no access violations.
                mOpenMode = "r";
            }
            break;
        case AsyncOp::FOPEN_READWRITE | AsyncOp::FOPEN_CREATE:
            if (!fileExists)
            {         
                TruncateFile();
                mOpenMode = "r+";
            }
            break;
        case AsyncOp::FOPEN_READWRITE:
            mOpenMode = "r+";
            break;
        case AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE:
            TruncateFile();

            // We use "r" here because we want the FSOpenFile to have a valid flag and "r" is the least intrusive. The file is already truncated at 
            // this point so there is no access violations.
            mOpenMode = "r";
            break;
        case AsyncOp::FOPEN_READWRITE | AsyncOp::FOPEN_OVERWRITE:
            if(fileExists)
            {         
                TruncateFile();
                mOpenMode = "r+";
            }
            break;
        case AsyncOp::FOPEN_READWRITE | AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE:
            TruncateFile();
            mOpenMode = "r+";
            break;
    }

    mValid = mOpenMode != NULL;
}

void DeviceFileHandle::TruncateFile()
{
    // Refresh a file handle cache entry so that we are guaranteed to be able to open
    // a particular file.
    PlatformFileHandleCacheEntry *entry = gPlatformFileHandleCache.RefreshHandle(NULL, this);

    // The file is truncated by opening it for write and immediately closing it.
    FSFileHandle handle = FS_INVALID_HANDLE_VALUE;
    FSCmdBlock commandBlock;
    FSInitCmdBlock(&commandBlock);
    FSStatus status = FSOpenFile(mClientHandle, &commandBlock, mFileName, "w", &handle, FS_RET_ALL_ERROR);

    if (status == FS_STATUS_OK)
    {
        FSCloseFile(mClientHandle, &commandBlock, handle, FS_RET_ALL_ERROR);
    }

    gPlatformFileHandleCache.ReleaseCacheEntry(entry, this);
}

DeviceFileHandle::~DeviceFileHandle()
{
    if (mCacheEntry != NULL)
    {
        gPlatformFileHandleCache.ReleaseCacheEntry(mCacheEntry, this);
    }
    else
    {
        // The only time a cache entry would be non-null is if the user specified an invalid open
        // mode and we were unable to construct a valid device file handle.
        EA_ASSERT(!mValid);
    }
}


void DeviceFileHandle::OpenFile()
{
    FSFileHandle fileHandle;
    FSCmdBlock commandBlock;
    FSInitCmdBlock(&commandBlock);
    mLastOpenError = FSOpenFile(mClientHandle, &commandBlock, mFileName, mOpenMode, &fileHandle, FS_RET_ALL_ERROR);
    mCacheEntry->mFileDescriptor = static_cast<PlatformFileHandle>(fileHandle);
    mCacheEntry->mContext = eastl::union_cast<void *>(mClientHandle);

    if (mLastOpenError != FS_STATUS_OK)
    {
        mCacheEntry->Invalidate();
        return;
    }

    if (mCurrentPosition != 0)
    {
        FSStatus seekStatus = FSSetPosFile(mClientHandle, &commandBlock, fileHandle, static_cast<u32>(mCurrentPosition), FS_RET_ALL_ERROR);
        if (seekStatus != FS_STATUS_OK)
        {
            FSCloseFile(mClientHandle, &commandBlock, fileHandle, FS_RET_ALL_ERROR);
            mCacheEntry->Invalidate();
            return;
        }
    }
}


FSFileHandle DeviceFileHandle::GetFileDescriptor()
{
    mCacheEntry = gPlatformFileHandleCache.RefreshHandle(mCacheEntry, this);
    if (!mCacheEntry->IsValid())
        OpenFile();

    return mCacheEntry->mFileDescriptor;
}


CafeDvdFileDeviceDriver::CafeDvdFileDeviceDriver()
    : mClient(NULL)
{
}


CafeDvdFileDeviceDriver::~CafeDvdFileDeviceDriver()
{
    Restore();
}


void CafeDvdFileDeviceDriver::Restore()
{
    EA::Allocator::ICoreAllocator *allocator = Manager::GetAllocator();

    if (mReadWriteBuffer)
    {
        allocator->Free(mReadWriteBuffer);
        mReadWriteBuffer = NULL;
    }

    DeviceDriver::Restore();
}


void CafeDvdFileDeviceDriver::CafeDvdFileDeviceDriver::SetClientHandle(FSClient *ch)
{
    mClient = ch;
}


FSClient *CafeDvdFileDeviceDriver::GetClientHandle() const
{
    return mClient;
}

bool CafeDvdFileDeviceDriver::IsValidPath(char* path)
{
    const char* illegalCharacters = "\" * , ; < > ? ¥ |";
    if(EA::StdC::Strpbrk(path, illegalCharacters) == NULL)
    {
        return true;
    }
    return false;
}

bool CafeDvdFileDeviceDriver::Init()
{
    EA::Allocator::ICoreAllocator *allocator = Manager::GetAllocator();
    mReadWriteBuffer = allocator->Alloc(
            CAFE_READWRITE_BUFFER_SIZE, 
            "Cafe file read/write buffer",
            0, 
            FS_IO_BUFFER_ALIGN);

    return true;
}


static void ConvertToAbsolutePath(char *nameBuffer, size_t nameBufferSize, const char *name)
{
    // If the name starts with a relative sequence (ie: "./foo"), advance past that.
    if (name[0] == '.' && name[1] == '/')
        name += 2;

    // Absolute paths will either contain a colon or they will start with a slash. If the path
    // is not absolute then concatenate the file name with the current working directory.
    if (name[0] != '/' && !EA::StdC::Strchr(name, ':'))
        EA::StdC::Snprintf(nameBuffer, nameBufferSize, "%s/%s", Manager::GetInstance()->GetCWD(), name);
    else
        EA::StdC::Strncpy(nameBuffer, name, nameBufferSize);
}


void *CafeDvdFileDeviceDriver::Open(const char *name, unsigned int oflags, Handle **)
{
    char fileName[FILESYS_MAX_PATH] = {};
    ConvertToAbsolutePath(fileName, FILESYS_MAX_PATH, name);
    PathSimplify(fileName, fileName, FILESYS_MAX_PATH);

    if(IsValidPath(fileName))
    {
        DeviceFileHandle *handle = new DeviceFileHandle(mClient, fileName, oflags);
        if (handle->HasValidFileDescriptor())
            return handle;

        Manager::GetInstance()->GetErrorHandler()(handle->GetLastOpenError(), Manager::FILEERROR_OPEN, this, name);
        delete handle;
    }

    return NULL;
}


void CafeDvdFileDeviceDriver::Close(void *h)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);
    delete handle;
}


unsigned int CafeDvdFileDeviceDriver::Read(void *fh, void *buffer, unsigned int readBufferSize, DeviceDriver *, void *)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(fh);
    unsigned int bytesRemaining = readBufferSize;
    bool simulateError = CAFE_SIMULATE_ERROR(handle);
    FSFileHandle h = handle->GetFileDescriptor();

    u32 bytesRead = 0;

    FSCmdBlock commandBlock;
    FSInitCmdBlock(&commandBlock);
    while (bytesRemaining != 0)
    {
        uint32_t readSize = MIN(CAFE_READWRITE_BUFFER_SIZE, bytesRemaining);
        FSStatus status = FS_STATUS_OK;

        if (!simulateError)
            status = FSReadFileWithPos(
                    mClient,
                    &commandBlock,
                    mReadWriteBuffer,
                    1,
                    readSize,
                    handle->GetFilePointer(),
                    h,
                    0,
                    FS_RET_ALL_ERROR);

        if (status < 0 || simulateError)
        {
            Manager::GetInstance()->GetErrorHandler()(status, Manager::FILEERROR_READ, this, handle->GetName());
            return 0;
        }

        if (status == 0)
            break;

        memcpy(static_cast<char *>(buffer) + bytesRead, mReadWriteBuffer, status);
        bytesRemaining -= status;
        bytesRead += status;
        handle->MoveFilePointer(status);
    }

    return bytesRead;
}


uint32_t CafeDvdFileDeviceDriver::Write(void *fh, const void *buffer, uint32_t writeBufferSize, DeviceDriver *, void *)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(fh);
    u32 bytesRemaining = writeBufferSize;
    FSFileHandle h = handle->GetFileDescriptor();

    u32 bytesWritten = 0;
    
    FSCmdBlock commandBlock;
    FSInitCmdBlock(&commandBlock);
    while (bytesRemaining != 0)
    {
        uint32_t writeSize = MIN(CAFE_READWRITE_BUFFER_SIZE, bytesRemaining);
        memcpy(mReadWriteBuffer, static_cast<const char *>(buffer) + bytesWritten, writeSize);

        FSStatus status = FSWriteFileWithPos(
                mClient,
                &commandBlock,
                mReadWriteBuffer,
                1,
                writeSize,
                handle->GetFilePointer(),
                h,
                0,
                FS_RET_ALL_ERROR);

        if (status < 0)
        {
            Manager::GetInstance()->GetErrorHandler()(status, Manager::FILEERROR_WRITE, this, handle->GetName());
            return 0;
        }

        if (status == 0)
            break;

        // Appending modes ("a", "a+") ignore the file pointer completely and write 
        // only to the end of the file, so our internal file pointer is not updated
        // in this case.
        if (handle->GetOpenMode()[0] != 'a')
            handle->MoveFilePointer(status);
        bytesRemaining -= status;
        bytesWritten += status;
    }

    return bytesWritten;
}


uint64_t CafeDvdFileDeviceDriver::Seek(void *h, uint64_t offset, int whence, DeviceDriver *, void *)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);
    EA_ASSERT(whence == FSEEK_BEGIN);
    handle->SetFilePointer(offset);
    return offset;
}


uint64_t CafeDvdFileDeviceDriver::GetSize(void *h)
{
    FSStat stat;
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);
    FSCmdBlock commandBlock;
    FSInitCmdBlock(&commandBlock);
    FSGetStatFile(mClient, &commandBlock, handle->GetFileDescriptor(), &stat, FS_RET_ALL_ERROR);
    return stat.size;
}


bool CafeDvdFileDeviceDriver::Delete(const char *name)
{
    char fileName[FILESYS_MAX_PATH];
    PathSimplify(name, fileName, FILESYS_MAX_PATH);

    if(IsValidPath(fileName))
    {

        FSCmdBlock commandBlock;
        FSInitCmdBlock(&commandBlock);
        return FSRemove(mClient, &commandBlock, fileName, FS_RET_ALL_ERROR) == FS_STATUS_OK;
    }

    return false;
}


bool CafeDvdFileDeviceDriver::Move(const char *oldName, const char *newName)
{
    char oldFileName[FILESYS_MAX_PATH], newFileName[FILESYS_MAX_PATH];
    PathSimplify(oldName, oldFileName, FILESYS_MAX_PATH);
    PathSimplify(newName, newFileName, FILESYS_MAX_PATH);

    if(IsValidPath(oldFileName) && IsValidPath(newFileName))
    {
        FSCmdBlock commandBlock;
        FSInitCmdBlock(&commandBlock);
        return FSRename(mClient, &commandBlock, oldFileName, newFileName, FS_RET_ALL_ERROR) == FS_STATUS_OK;
    }

    return false;
}


void *CafeDvdFileDeviceDriver::FindBegin(const char *filePattern, FindData *findData)
{
    char dirName[FILESYS_MAX_PATH];
    char simplifiedPath[FILESYS_MAX_PATH];
    PathSimplify(filePattern, simplifiedPath, FILESYS_MAX_PATH);
    GetPathDirectory(simplifiedPath, dirName, FILESYS_MAX_PATH);
    size_t dirNameLength = EA::StdC::Strlen(dirName);
    
    const char *pattern = GetPathFileName(simplifiedPath);
    if (!pattern)
    {
        pattern = dirName + dirNameLength;
        EA_ASSERT(*pattern == 0);
    }

    FSDirHandle dir;
    FSCmdBlock commandBlock;
    FSInitCmdBlock(&commandBlock);

    if(!IsValidPath(dirName))
    {
        return NULL;
    }

    int result = FSOpenDir(mClient, &commandBlock, dirName, &dir, FS_RET_ALL_ERROR);

    if (result != FS_STATUS_OK)
        return NULL;

    FindHandle findHandle(dir, pattern, dirName);
    if (!FindNext(&findHandle, findData))
    {
        if (EA::StdC::Strcmp(pattern, "*") == 0)
        {
            // FSReadDir returns FS_ERROR_END_OF_DIRECTORY if the directory
            // is empty.  We can't just return NULL from this function because
            // the caller will take that to mean the directory does not exist.
            // Instead, if the first call to FindNext() fails, fill in the
            // findData information with the current directory ('.').  On the
            // second call to FindNext (made by the user), it will fail and the
            // user can delete the directory search object.  This allows the
            // caller to distinguish between an empty directory and one that
            // does not exist.  This applies if there was no pattern to match.
            findData->mFlags = DirectoryEntry::FLAG_DIRECTORY;
            findData->mCreationTime = 0;
            findData->mAccessTime = 0;
            findData->mModificationTime = 0;
            findData->mSize = 0;
            findData->mName[0] = '.';
            findData->mName[1] = '\0';
        }
        else
        {
            // If there was a pattern to match, no files matched it, so the
            // directory search is invalid.
            FSStatus stat = FSCloseDir(mClient, &commandBlock, dir, FS_RET_ALL_ERROR);
            EA_ASSERT(stat == FS_STATUS_OK);
            return NULL;
        }
    }

    return new FindHandle(findHandle);
}


bool CafeDvdFileDeviceDriver::FindNext(void *h, FindData *findData)
{
    FindHandle *handle = static_cast<FindHandle *>(h);
    const char *pattern = handle->GetFilePattern();
    bool noPattern = pattern[0] == 0;

    FSDirEntry dirEntry;
    FSCmdBlock commandBlock;
    FSInitCmdBlock(&commandBlock);
    FSStatus result = FSReadDir(mClient, &commandBlock, handle->GetDirHandle(), &dirEntry, FS_RET_ALL_ERROR);

    for(; result == FS_STATUS_OK; result = FSReadDir(mClient, &commandBlock, handle->GetDirHandle(), &dirEntry, FS_RET_ALL_ERROR))
    {
        if (!noPattern && !MatchesWildcardPattern(dirEntry.name, pattern))
            continue;

        EA::StdC::Strlcpy(findData->mName, dirEntry.name, EAArrayCount(findData->mName));
        findData->mFlags = 0;

        if ((dirEntry.stat.permission & FS_MODE_IWGRP) == 0)
            findData->mFlags = DirectoryEntry::FLAG_READONLY;

        if ((dirEntry.stat.flag & FS_STAT_FLAG_IS_DIRECTORY) != 0)
            findData->mFlags |= DirectoryEntry::FLAG_DIRECTORY;
        else
            findData->mSize = dirEntry.stat.size;

        // it's not clear how Nintendo's ctime & mtime map torwfilesystem's
        // mCreationTime and mModificationTime, so we'll just pass them
        // through verbatim for now.
        // Nintendo's FSA/FS does not have an API for access time, so we'll just
        // leave that at 0.
        findData->mCreationTime = dirEntry.stat.flag & FS_STAT_FLAG_SPRT_CTIME ? dirEntry.stat.ctime : 0;
        findData->mAccessTime = 0;
        findData->mModificationTime = dirEntry.stat.flag & FS_STAT_FLAG_SPRT_MTIME ? dirEntry.stat.mtime : 0;

        return true;
    }

    return false;
}


bool CafeDvdFileDeviceDriver::FindEnd(void *h)
{
    FindHandle *handle = static_cast<FindHandle *>(h);
    FSCmdBlock commandBlock;
    FSInitCmdBlock(&commandBlock);
    FSStatus stat = FSCloseDir(mClient, &commandBlock, handle->GetDirHandle(), FS_RET_ALL_ERROR);
    delete handle;

    return stat == FS_STATUS_OK;
}


bool CafeDvdFileDeviceDriver::DirectoryCreate(const char *rawDirectory)
{
    char directory[FILESYS_MAX_PATH];
    PathSimplify(rawDirectory, directory, FILESYS_MAX_PATH);

    if(IsValidPath(directory))
    {
        FSCmdBlock commandBlock;
        FSInitCmdBlock(&commandBlock);
        return FSMakeDir(mClient, &commandBlock, directory, FS_RET_ALL_ERROR) == FS_STATUS_OK;
    }

    return false;
}


bool CafeDvdFileDeviceDriver::DirectoryRemove(const char *rawDirectory)
{
    char directory[FILESYS_MAX_PATH];
    PathSimplify(rawDirectory, directory, FILESYS_MAX_PATH);

    if(IsValidPath(directory))
    {
        FSCmdBlock commandBlock;
        FSInitCmdBlock(&commandBlock);
        return FSRemove(mClient, &commandBlock, directory, FS_RET_ALL_ERROR) == FS_STATUS_OK;
    }

    return false;
}


bool CafeDvdFileDeviceDriver::GetNameFromHandle(void *handle, char *name, size_t nameBufferSize)
{
    DeviceFileHandle *fileHandle = static_cast<DeviceFileHandle *>(handle);
    EA::StdC::Strlcpy(name, fileHandle->GetName(), nameBufferSize);
    return true;
}

CafeDvdFileDeviceDriver gDeviceDriverImpl;

}}}

