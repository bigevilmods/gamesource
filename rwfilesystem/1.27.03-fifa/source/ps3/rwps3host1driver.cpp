// (c) Electronic Arts. All Rights Reserved.

#include "EAAssert/eaassert.h"

#include "../private_rwdevice.h"
#include "../private_rwprofiler.h"
#include "../private_rwplatformfilehandlecache.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/directory.h"
#include "rw/core/filesys/device.h"
#include "rw/core/filesys/driverimpl.h"
#include "rw/core/filesys/path.h"

#include <cell/fs/cell_fs_file_api.h>
#include <cell/fs/cell_fs_errno.h>
#include <np/drm.h>
#include <sys/process.h>
#include <sys/paths.h>
#include <sys/timer.h>
#include <sysutil/sysutil_common.h>

#include "EAStdC/EAMemory.h"
#include "EAStdC/EASprintf.h"
#include "EAStdC/EAString.h"

using namespace EA::StdC;

namespace rw {
namespace core {
namespace filesys {

#define PS3_SIMULATE_ERROR()    (Manager::GetInstance()->GetErrorInjectionFunction()(this, h->GetName()) == false)


static PlatformFileHandleCache gPlatformFileHandleCache;


void PlatformFileHandleCache::CloseHandle(void *, PlatformFileHandle fileHandle)
{
    int ret = cellFsClose(static_cast<int>(fileHandle));
    EA_ASSERT(ret == CELL_FS_SUCCEEDED);
    EA_UNUSED(ret);
}


class DeviceFileHandle
{
public:
    void *operator new(size_t size) { return RWFILESYSTEM_ALLOCATE(size); }
    void operator delete(void *p) { Manager::Free(p); }

    DeviceFileHandle(const char *fileName, int flags, const SceNpDrmKey &drmKey);
    ~DeviceFileHandle();

    int GetFileDescriptor();

    inline void MoveFilePointer(uint64_t amount) { mCurrentPosition += amount; }
    inline void SetFilePointer(uint64_t position) { mCurrentPosition = position; }
    inline uint64_t GetFilePointer() const { return mCurrentPosition; }
    inline const char *GetName() const { return mFileName; }
    inline bool HasValidFileDescriptor() { return GetFileDescriptor() >= 0; }
    inline CellFsErrno GetLastOpenError() { return mLastOpenError; }

private:
    void OpenFile();

private:
    uint64_t mCurrentPosition;
    char mFileName[FILESYS_MAX_PATH];
    int mOpenFlags;
    bool mIsSdata;

    void *mDrmFlag;
    size_t mDrmFlagSize;
    SceNpDrmOpenArg mDrmArg;
    SceNpDrmKey mDrmKey;

    PlatformFileHandleCacheEntry *mCacheEntry;
    CellFsErrno mLastOpenError;
};


DeviceFileHandle::DeviceFileHandle(const char *fileName, int flags, const SceNpDrmKey &drmKey)
    : mCurrentPosition(0),
      mOpenFlags(0),
      mDrmFlag(NULL),
      mDrmFlagSize(0),
      mDrmKey(drmKey),
      mCacheEntry(NULL),
      mLastOpenError(CELL_FS_SUCCEEDED)
{
    EA_STRLCPY_VERIFY(mFileName, fileName, EAArrayCount(mFileName));

    mOpenFlags |= ((flags & AsyncOp::FOPEN_READWRITE) != 0 ? CELL_FS_O_RDWR : CELL_FS_O_RDONLY);

    switch (flags & (AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE))
    {
        case AsyncOp::FOPEN_CREATE:
            mOpenFlags |= CELL_FS_O_CREAT | CELL_FS_O_EXCL;
            break;
        case AsyncOp::FOPEN_OVERWRITE:
            mOpenFlags |= CELL_FS_O_TRUNC;
            break;
        case AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE:
            mOpenFlags |= CELL_FS_O_CREAT | CELL_FS_O_TRUNC;
            break;
        default:
            break;
    }

    if (flags & AsyncOp::FOPEN_DRM)
    {
        mDrmArg.flag = SCE_NP_DRM_OPEN_FLAG;
        mDrmFlag = &mDrmArg;
        mDrmFlagSize = sizeof(mDrmArg);
    }
    else
    {
        mDrmArg.flag = 0;
    }

    mIsSdata = (flags & AsyncOp::FOPEN_SDATA) != 0;
}


DeviceFileHandle::~DeviceFileHandle()
{
    if (mCacheEntry)
        gPlatformFileHandleCache.ReleaseCacheEntry(mCacheEntry, this);
}


int DeviceFileHandle::GetFileDescriptor()
{
    mCacheEntry = gPlatformFileHandleCache.RefreshHandle(mCacheEntry, this);

    if (!mCacheEntry->IsValid())
        OpenFile();

    return mCacheEntry->mFileDescriptor;
}


void DeviceFileHandle::OpenFile()
{
    if (mDrmFlag)
    {
        int ret = sceNpDrmIsAvailable(&mDrmKey, mFileName);
        if (ret != CELL_OK)
            return;
    }

    int fileHandle = 0;
    mLastOpenError = mIsSdata 
        ? cellFsSdataOpen(mFileName, mOpenFlags, &fileHandle, mDrmFlag, mDrmFlagSize)
        : cellFsOpen(mFileName, mOpenFlags, &fileHandle, mDrmFlag, mDrmFlagSize);
    mCacheEntry->mFileDescriptor = static_cast<PlatformFileHandle>(fileHandle);

    // After the file is opened clear out any creation/truncation bits so
    // that data isn't lost when re-opening it.
    mOpenFlags &= CELL_FS_O_ACCMODE;

    if (mLastOpenError != CELL_FS_SUCCEEDED)
    {
        mCacheEntry->Invalidate();
        return;
    }

    if (mCurrentPosition == 0)
        return;

    uint64_t currentPosition;
    mLastOpenError = cellFsLseek(mCacheEntry->mFileDescriptor, mCurrentPosition, CELL_FS_SEEK_SET, &currentPosition);

    if (mLastOpenError != CELL_FS_SUCCEEDED)
    {
        cellFsClose(mCacheEntry->mFileDescriptor);
        mCacheEntry->Invalidate();
        return;
    }
}


// Specific to path iteration, holds the file pattern,
// the device handle, and the path
class HostDeviceFilePatternHandle
{
public:
    void *operator new(size_t size)
    {
        return RWFILESYSTEM_ALLOCATE(size);
    }

    void operator delete(void *p)
    {
        return Manager::Free(p);
    }

    explicit HostDeviceFilePatternHandle(int FileHandle,char * filePattern, char * directory)
        : mFileHandle(FileHandle)
    {
        if (filePattern != NULL)
            EA_STRLCPY_VERIFY(mFilePattern, filePattern, EAArrayCount(mFilePattern));
        else
            mFilePattern[0] = '\0';

        if (directory != NULL)
            EA_STRLCPY_VERIFY(mDirectory, directory, EAArrayCount(mDirectory));
        else
            mDirectory[0] = '\0';
    }

    int GetHandle() const               {  return mFileHandle; }
    const char * GetFilePattern() const {  return mFilePattern; }
    const char * GetDirectory()   const {  return mDirectory; }

private:
    int  mFileHandle;
    char mFilePattern[128];
    char mDirectory[FILESYS_MAX_PATH];
};

PS3FileDeviceDriver::PS3FileDeviceDriver()
    : DeviceDriver(),
      mThroughputLimit(0),
      mLastReadSize(0),
      mLastReadTime(0)
{
    // Disable DRM by default as it comes with a cost.
    UseDRM( false, 0x0, NULL );
    AppendEdatForDrmFlaggedFiles(true);
    AppendSdatForDrmFlaggedFiles(true);
}

PS3FileDeviceDriver::~PS3FileDeviceDriver()
{
}

void PS3FileDeviceDriver::SetThroughputLimit(unsigned int KBperSec)
{
    mThroughputLimit = KBperSec;

    if (mThroughputLimit == 0)
    {
        mLastReadTime = 0;
        mLastReadSize = 0;
    }
    else
    {
        mLastReadTime = static_cast<int64_t>(EA::Thread::GetThreadTime());
        mLastReadSize = 0;
    }
}

/*! \brief Enable/disable DRM

This function enables the PS3 file device driver to open files that are
part of a downloadable content package.  The .edat extension is appended
automatically to the file name by the driver (e.g. if the file name is
"myfile.txt", the driver will attempt to open "myfile.txt.edat").  If the
file fails to open in secure mode, the open is retried in non-secure mode.

To make the file secure, use the make_edat_npdrm tool in the PS3 SDK.

\param enable Set to true to enable DRM, false to disable it.
\param key The decryption key for your title.
\param tryDrmFile A callback function that may be used to filter
    non-secure files from trying to be opened securely.  If you want to
    try to open the file securely, this function should return true.  If
    you don't want rwfilesystem to attempt to open the file in secure
    mode, have this function return false.  If this parameter is NULL,
    the default function will try to open every file in with security
    enabled.
*/
void PS3FileDeviceDriver::UseDRM(bool enable, const SceNpDrmKey *key,
                                 bool (*tryDrmFile)(const char *name))
{
    mIsDrmEnabled = enable;

    if (key != NULL)
    {
        Memcpy(&mDrmKey, key, sizeof(SceNpDrmKey));
    }

    if (tryDrmFile != NULL)
    {
        mTryDrmFile = tryDrmFile;
    }
    else
    {
        mTryDrmFile = &DefaultTryDrmFile;
    }
}

/*! \brief Enable/disable autoappending of ".edat"

If DRM is activated and the mTryDrmFile callback returns true, this
setting determines if ".edat" should be appended to the input
filename.
".edat" will only be appended if DRM is enabled, the callback function
returns true and the AppendEdatForDrmFlaggedFiles setting is true.

\param enable Set to true to enable automatically append ".edat"
*/
void PS3FileDeviceDriver::AppendEdatForDrmFlaggedFiles(bool enable)
{
    mAutoAppendEdat = enable;
}

/*! \brief Enable/disable autoappending of ".sdat"

If a file is opened with the FOPEN_SDATA flag, this setting 
determines if ".sdat" should be appended to the input filename.

\param enable Set to true to enable automatically append ".sdat"
*/
void PS3FileDeviceDriver::AppendSdatForDrmFlaggedFiles(bool enable)
{
    mAutoAppendSdat = enable;
}


// This function is provided for the game to handle disc errors such as disk ejection or dirty disc
// TRC recommends a message.  This thread defaults to priority 256 (high) so be careful
// this callback is likely to change in the future.
// return value 0=stop retry..fail and exit.  1=continue retrying.  TRC 28.1 says do infinite retries.

// PS button must be respected during eject.  This default
// implementation checks for exit.  If you do your own, make
// sure you do this too.  In addition to this, you must
// continue rendering for this to work.

bool PS3FileDeviceDriver::DefaultFileErrorCallback(int, unsigned int, DeviceDriver *, const char *)
{
    cellSysutilCheckCallback();  // check for PS button->OSD exit request and call callbacks

    // keep trying
    return true;
}

bool PS3FileDeviceDriver::DiscErrorHandler(int errorNumber, unsigned int FileErrorFunctionNum, const char *name)
{
    bool r = Manager::GetInstance()->GetErrorHandler()(errorNumber,
        FileErrorFunctionNum, this, name);

    // The sleep allows lower priority threads (ie the game)
    // to continue and avoids flooding the callback
    sys_timer_usleep(5000);  // 5 ms delay (must be < 1 second for responsiveness TRC)

    return r;
}

bool PS3FileDeviceDriver::Init()
{
    filesys::Manager *m = Manager::GetInstance();

    // Set the default read failure callback if it hasn't already been set.
    if (m->GetErrorHandler() == NULL)
    {
        m->SetErrorHandler(DefaultFileErrorCallback);
    }

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
        EA_STRLCPY_VERIFY(nameBuffer, name, nameBufferSize);
}


void *PS3FileDeviceDriver::Open(const char *name, unsigned int flags, Handle **)
{
    char absolutePath[FILESYS_MAX_PATH] = {},
         fileName[FILESYS_MAX_PATH] = {};

    ConvertToAbsolutePath(absolutePath, FILESYS_MAX_PATH, name);
    if (ConvertHostNameToNativeName(fileName, absolutePath) == NULL)
        return NULL;

    if ((flags & AsyncOp::FOPEN_SDATA) && mAutoAppendSdat)
        EA_STRLCAT_VERIFY(fileName, ".sdat", FILESYS_MAX_PATH);

    if (mIsDrmEnabled && mTryDrmFile(fileName))
    {
        char edatName[FILESYS_MAX_PATH] = {};
        
        EA::StdC::Snprintf(edatName, FILESYS_MAX_PATH, "%s%s", fileName, mAutoAppendEdat ? ".edat" : "");
        flags |= AsyncOp::FOPEN_DRM;

        DeviceFileHandle *handle = new DeviceFileHandle(edatName, flags, mDrmKey);
        if (handle->HasValidFileDescriptor())
            return handle;

        // If the file doesn't exist or isn't available, delete the handle and try
        // to open it as a non-DRM file.
        delete handle;
        flags &= ~AsyncOp::FOPEN_DRM;
    }

    DeviceFileHandle *handle = new DeviceFileHandle(fileName, flags, mDrmKey);
    if (handle->HasValidFileDescriptor())
        return handle;

    DiscErrorHandler(handle->GetLastOpenError(), Manager::FILEERROR_OPEN, fileName);

    delete handle;
    return NULL;
}


void PS3FileDeviceDriver::Close(void *fh, DeviceDriver *, void *)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(fh);
    delete handle;
}


void PS3FileDeviceDriver::ThrottleRead()
{
    if (mThroughputLimit == 0)
        return;

    // calculate the minimum amount of time that needs to pass before
    // the next read can be processed.
    int64_t timeUntilNextRead = mLastReadSize / mThroughputLimit;

    // Get the current system time
    int64_t currTime = static_cast<int64_t>(EA::Thread::GetThreadTime());

    int64_t delayTime = timeUntilNextRead - (currTime - mLastReadTime);

    if (delayTime > 0)
    {
        // put this thread to sleep so the throughput is reduced
        EA::Thread::ThreadSleep(static_cast<EA::Thread::ThreadTime>(delayTime));
    }

    mLastReadTime = static_cast<int64_t>(EA::Thread::GetThreadTime());
    mLastReadSize = 0;
}

uint32_t PS3FileDeviceDriver::Read(void *fh, void *buffer, uint32_t readBufferSize, DeviceDriver *, void *)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);

    ThrottleRead();
    char *readBuffer = static_cast<char *>(buffer);
    uint32_t bytesToRead = readBufferSize;
    uint64_t totalRead = 0;
    const bool simulateError = PS3_SIMULATE_ERROR();

    int fileDescriptor = h->GetFileDescriptor();

    while (totalRead < readBufferSize)
    {
        uint64_t readSize = 0;
        int ret = CELL_FS_SUCCEEDED;

        if (!simulateError)
            ret = cellFsRead(fileDescriptor, 
                readBuffer + totalRead,
                readBufferSize,
                &readSize);

        if (ret != CELL_FS_SUCCEEDED || simulateError)
        {
            DiscErrorHandler(ret, Manager::FILEERROR_READ, h->GetName());
            return 0;
        }

        if (ret == CELL_FS_SUCCEEDED && readSize == 0)
            break;

        totalRead += readSize;
        bytesToRead -= readSize;
    }

    mLastReadSize = totalRead;
    h->MoveFilePointer(static_cast<size_t>(totalRead));
    return static_cast<uint32_t>(totalRead);
}

uint32_t PS3FileDeviceDriver::Write(void *fh, const void *buf, uint32_t bufsize, DeviceDriver *, void *)
{
    uint64_t writtenSize = 0;

    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);

    int fileDescriptor = h->GetFileDescriptor();
    int ret = cellFsWrite(fileDescriptor, buf, bufsize, &writtenSize);
    EA_ASSERT(writtenSize == bufsize);

    if (ret != CELL_OK)
    {
        // If the write fails, let the user know.  This is not necessarily
        // fatal because either the file was opened with the wrong permissions
        // or the disk is full.  We coul throw an assert if the error is
        // anything other than disk full, but I think it's better to be
        // consistent and let the user halt the system if the error is not
        // recoverable.
        Manager::GetInstance()->GetErrorHandler()(ret, Manager::FILEERROR_WRITE, this, h->GetName());

        // some data may have been written, but we're going to return 0 anyway
        // because the entire write did not succeed.
        return 0;
    }

    h->MoveFilePointer(static_cast<size_t>(writtenSize));
    return static_cast<uint32_t>(writtenSize);
}

uint64_t PS3FileDeviceDriver::Seek(void *fh, uint64_t offset, int whence, DeviceDriver *, void *)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);

    uint64_t pos = 0;
    int cellFsWhence = -1;

    if (whence == FSEEK_BEGIN)
        cellFsWhence = CELL_FS_SEEK_SET;
    else if (whence == FSEEK_CURRENT)
        cellFsWhence = CELL_FS_SEEK_CUR;
    else
        cellFsWhence = CELL_FS_SEEK_END;
    
    const bool simulateError = PS3_SIMULATE_ERROR();
    int ret = CELL_FS_SUCCEEDED;

    int fileDescriptor = h->GetFileDescriptor();
    if (!simulateError)
        ret = cellFsLseek(fileDescriptor, static_cast<int64_t>(offset), cellFsWhence, &pos);

    if (ret != CELL_FS_SUCCEEDED || simulateError)
        DiscErrorHandler(ret, Manager::FILEERROR_SEEK, h->GetName());
    
    h->SetFilePointer(static_cast<size_t>(pos));
    return pos;
}

uint64_t PS3FileDeviceDriver::GetSize(void * fh, DeviceDriver *, void *)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);

    CellFsStat stat;
    int fileDescriptor = h->GetFileDescriptor();

    int ret = cellFsFstat(fileDescriptor, &stat);
    EA_ASSERT(ret == CELL_FS_SUCCEEDED);
    (void)ret;

    return stat.st_size;
}

bool PS3FileDeviceDriver::Delete(const char *name)
{
    char hostNameBuf[FILESYS_MAX_PATH];
    if (ConvertHostNameToNativeName(hostNameBuf, name) == NULL)
        return false;

    return cellFsUnlink(hostNameBuf) == 0;
}


bool PS3FileDeviceDriver::Move(const char * oldName, const char * newName)
{
    char host1OldNameBuffer[FILESYS_MAX_PATH], host1NewNameBuffer[FILESYS_MAX_PATH];

    if (ConvertHostNameToNativeName(host1OldNameBuffer, oldName) == NULL
            || ConvertHostNameToNativeName(host1NewNameBuffer, newName) == NULL)
        return false;

    int rv = cellFsRename(host1OldNameBuffer, host1NewNameBuffer);
   
    return rv == 0;
}

#define ISJUNK(V) (*(V) == '/' || *(V) == '\\' || *(V) == ':')
void *PS3FileDeviceDriver::FindBegin(const char *filePattern, FindData *findData)
{
    char directory[FILESYS_MAX_PATH];
    if (ConvertHostNameToNativeName(directory, filePattern) == NULL)
        return NULL;

    HostDeviceFilePatternHandle *fileHandle = NULL;
    int len = EA::StdC::Strlen(directory);
    char *pattern = NULL;
    char *patend = directory + len;
    char *end = patend;

    // search for a file pattern, we need just
    // the directory bit.
    while (end && end > directory && !ISJUNK(end))
    {
        end--;
    }

    // We have a file pattern
    if (end && end != patend && (*end == '/' || *end == '\\'))
    {
        pattern = (end+1);
        *end = '\0';
    }

    int fd = 0;
    CellFsErrno ret = cellFsOpendir(directory, &fd);

    if (ret != CELL_FS_SUCCEEDED)
    {
        DiscErrorHandler(ret, Manager::FILEERROR_OPENDIR, directory);
        return NULL;
    }

    fileHandle = new HostDeviceFilePatternHandle(fd,pattern,directory);
    if (!FindNext(fileHandle,findData))
    {
        FindEnd(fileHandle);
        fileHandle = NULL;
    }

    return fileHandle;
}

/*! \internal Default TryDrmFile just returns true, which means,
 try to open the file with DRM.
*/
bool PS3FileDeviceDriver::DefaultTryDrmFile(const char *)
{
    return true;
}

bool PS3FileDeviceDriver::FindNext(void *h, FindData *findData)
{
    CellFsDirectoryEntry dirEntry;
    const HostDeviceFilePatternHandle *fh = static_cast<HostDeviceFilePatternHandle *>(h);
    const char * pattern = fh->GetFilePattern();
    bool  noPattern = pattern[0] == '\0';
    uint32_t nread;

    for (;;)
    {
        CellFsErrno result = cellFsGetDirectoryEntries(fh->GetHandle(), &dirEntry, sizeof(dirEntry), &nread);
        if (result != CELL_FS_SUCCEEDED || !nread)
            break;

        if (noPattern || MatchesWildcardPattern(dirEntry.entry_name.d_name, pattern))
        {
            findData->mFlags  = 0;
            if (dirEntry.entry_name.d_type == CELL_FS_TYPE_DIRECTORY)
                findData->mFlags |= DirectoryEntry::FLAG_DIRECTORY;

            // It appears that bit 7 is set on files that are writable
            // and is clear on Read-only files, although I don't see anything
            // about this in the Sonly library API.
            if ((dirEntry.attribute.st_mode & 0x80) == 0)
                findData->mFlags |= DirectoryEntry::FLAG_READONLY;

            EA_STRLCPY_VERIFY(findData->mName, static_cast<const char *>(dirEntry.entry_name.d_name), EAArrayCount(findData->mName));
            findData->mCreationTime = dirEntry.attribute.st_ctime;
            findData->mAccessTime = dirEntry.attribute.st_atime;
            findData->mModificationTime = dirEntry.attribute.st_mtime;
            findData->mSize = dirEntry.attribute.st_size;

            return true;
        }
    }

    return false;
}

bool PS3FileDeviceDriver::FindEnd(void *h)
{
    HostDeviceFilePatternHandle *fh = static_cast<HostDeviceFilePatternHandle *>(h);
    cellFsClosedir(fh->GetHandle());
    delete fh;

    return true;
}

bool PS3FileDeviceDriver::DirectoryRemove(const char *directory)
{
    char directoryNameBuffer[FILESYS_MAX_PATH];
    if (ConvertHostNameToNativeName(directoryNameBuffer, directory) == NULL)
        return false;

    int ret = cellFsRmdir(directory);
    return ret == CELL_FS_SUCCEEDED;
}

bool PS3FileDeviceDriver::DirectoryCreate(const char *directory)
{
    char directoryNameBuffer[FILESYS_MAX_PATH];
    if (ConvertHostNameToNativeName(directoryNameBuffer, directory) == NULL)
        return false;

    return CELL_FS_SUCCEEDED == cellFsMkdir(directoryNameBuffer,0777);
}

bool PS3FileDeviceDriver::GetNameFromHandle(void *fh, char *name, size_t nameBufferSize)
{
    EA_ASSERT(fh != NULL);
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);

    EA_STRLCPY_VERIFY(name, h->GetName(), nameBufferSize);
    return true;
}

PS3FileDeviceDriver gDeviceDriverImpl;
PS3FileDeviceDriver gHddDriverImpl;

}}}
