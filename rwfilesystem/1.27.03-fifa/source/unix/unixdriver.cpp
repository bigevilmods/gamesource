// (c) Electronic Arts. All Rights Reserved.

#include "private_rwhandle.h"
#include "private_rwdevice.h"
#include "private_rwprofiler.h"
#include "rw/core/filesys/device.h"
#include "rw/core/filesys/driverimpl.h"
#include "rw/core/filesys/directory.h"
#include "rw/core/filesys/path.h"

#include "EAAssert/eaassert.h"
#include "EAStdC/EAString.h"
#include "EAStdC/EASprintf.h"
#include <sys/stat.h>
#include <dirent.h> //DIR, opendir, readdir, closedir
#include <errno.h> // errno, EACCES, ENOENT
#include <fcntl.h> // read, close, open, lseek, ftruncate, 'O_RDWR, etc.
#include <unistd.h> //read, close, open, lseek etc. Required only for clang

namespace rw {
namespace core {
namespace filesys {

// class DeviceFileHandle contains everything that a device driver
// needs to keep track of for the open file.  Each platform will have
// different requirements here.
class DeviceFileHandle
{
    void SetFileName(const char *nativeFileName)
    {
        memset(mNativeFileName, 0, FILESYS_MAX_PATH);
        if (nativeFileName != NULL)
            EA_STRLCPY_VERIFY(mNativeFileName, nativeFileName, static_cast<size_t>(FILESYS_MAX_PATH));
    }

public:
    static void *operator new(size_t size)
    {
        return RWFILESYSTEM_ALLOCATE(size);
    }

    static void operator delete(void *p)
    {
        return Manager::Free(p);
    }

    DeviceFileHandle(const char *nativeFileName, int fileHandle, int flags, int mode)
        : mFileHandle(fileHandle),
          mDirectoryHandle(0),
          mFlags(flags),
          mMode(mode),
          mCurrPos(0),
          mPattern(0)
    {
        SetFileName(nativeFileName);
    }

    DeviceFileHandle(const char *nativeFileName, DIR *directoryHandle)
        : mFileHandle(-1),
          mDirectoryHandle(directoryHandle)          
    {
        SetFileName(nativeFileName);
    }

    DIR *GetDirectoryHandle() const { return mDirectoryHandle; }
    int GetHandle() const { return mFileHandle; }
    void SetHandle(int hHandle) { mFileHandle = hHandle; }
    const char *GetName() const { return mNativeFileName; }
    int GetFlags() const { return mFlags; }
    int GetMode() const { return mMode; }
    void MovePointer(int amount) { mCurrPos += amount; }
    void SetPos(off_t pos) { mCurrPos = pos; }
    off_t GetPos() const { return mCurrPos; }
    void SetPattern(char const * x)
    {
        const size_t mNativeFileNameLen = EA::StdC::Strlen(mNativeFileName) + 1; //+1 for null char
        mPattern = mNativeFileName + mNativeFileNameLen;
        EA_STRLCPY_VERIFY(mPattern, x, EAArrayCount(mNativeFileName) - mNativeFileNameLen);
    }
    char const * GetPattern() const { return mPattern; }

private:
    int mFileHandle;
    DIR *mDirectoryHandle;
    char mNativeFileName[FILESYS_MAX_PATH];
    int mFlags;
    int mMode;
    off_t mCurrPos;
    char * mPattern;
};

PosixFileDeviceDriver::PosixFileDeviceDriver()
{
    // empty
}

// Bit ( Query, Set, Clr )
#define BQ(x, y) (((x)&(y))==(y))
#define BS(x, y) ((x)|(y))
#define BC(x, y) ((x)&(~y))

void*  PosixFileDeviceDriver::Open(const char *name, unsigned int rwflags, Handle **)
{
    int oflag = 0;

    char nativeFileName[FILESYS_MAX_PATH];

    int mode = 0744;

    ReplaceSlashes(nativeFileName, name);

    // choose access, sharing, creation, and attributes
    if (BQ(rwflags, AsyncOp::FOPEN_READWRITE))
    {
        oflag = O_RDWR;
    }
    else
    {
        oflag = O_RDONLY;
    }
    
    switch(rwflags & (AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE))
    {
    case 0:
        /* createmode is already correct */ break;

    case AsyncOp::FOPEN_CREATE:
        oflag |= O_CREAT | O_EXCL;
        break;

    case AsyncOp::FOPEN_OVERWRITE:
        {
            // rwfilesystem expects FOPEN_OVERWRITE on its own to fail if the access 
            // mode is read-only.
            if (oflag == O_RDONLY)
            {
                Manager::GetInstance()->GetErrorHandler()(EACCES, Manager::FILEERROR_OPEN, this, nativeFileName);
                return NULL;
            }

            struct stat st;

            // The result of using O_TRUNC on an existing file is undefined by the posix
            // standard. We want to be consistent with the other rwfilesystem implementations
            // so we mimic windows and fail if the file does not exist.  ENOENT is the error
            // code given if the file does not exist and O_CREAT is not specified, theoretically
            // this is the error that the posix system _should_ give in this situation.
            if (stat(nativeFileName, &st) == -1)
            {
                Manager::GetInstance()->GetErrorHandler()(ENOENT, Manager::FILEERROR_OPEN, this, nativeFileName);
                return NULL;
            }
        }
        oflag |= O_TRUNC;
        break;

    case AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE:
		{
			oflag |= O_CREAT | O_TRUNC;
		}
        break;
    }

    int h = open(nativeFileName, oflag, mode);
    //printf("open(\"%s\", 0x%x, 0x%x) -> %d\n", nativeFileName, oflag, mode, h);

    if (h != -1) return new DeviceFileHandle(nativeFileName, h, oflag, mode);

    Manager::GetInstance()->GetErrorHandler()(errno, Manager::FILEERROR_OPEN, this, nativeFileName);

    return NULL;
}

void PosixFileDeviceDriver::Close(void *fh)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);
    if (h->GetHandle() != -1)
        close(h->GetHandle());
    delete h;
}

unsigned int PosixFileDeviceDriver::Read(void *fh, void *buf, unsigned int bufsize, DeviceDriver *, void *)
{
    DeviceFileHandle *dfh = static_cast<DeviceFileHandle *>(fh);
    int nbytes = 0;
    bool hadError = false;
    int h;

errorRetryRead:

    // return 0 bytes read if the handle was previously closed
    // (because of a read error)
    h = dfh->GetHandle();
    if (h == -1) return 0;

    // Touch the first & last bytes of the buffer before reading This allows
    // virtual memory to be committed via a structured exception handler.  The
    // performance impact should be negligible, so we won't have a
    // configuration option to enable/disable it.
    *((volatile char *)buf+0) = *((volatile char *)buf+0);
    *((volatile char *)buf+bufsize-1) = *((volatile char *)buf+bufsize-1);

    bool success = false;

    if (Manager::GetInstance()->GetErrorInjectionFunction()(this, dfh->GetName()))
    {
        nbytes = static_cast<int>(read(h, buf, bufsize));
        success = nbytes >= 0;
    }

    if (success == false)
    {
        hadError = true;

errorRetryReadClose:
        close(h);
        dfh->SetHandle(-1);

errorRetryReadOpen:
        if (Manager::GetInstance()->GetErrorHandler()(
            errno, Manager::FILEERROR_READ, this, dfh->GetName()))
        {
            h = open(dfh->GetName(), dfh->GetFlags(), dfh->GetMode());

            if (h == -1)
                goto errorRetryReadOpen;

            dfh->SetHandle(h);

            // seek to correct pos
            if (lseek(h, dfh->GetPos(), SEEK_SET) == -1)
                goto errorRetryReadClose;

            goto errorRetryRead;
        }

        // The user has chosen not to retry the read.  The library will
        // return any data that was read up to the point at which the
        // failure occurred.  It is up to the application to provide a
        // fallback method of getting the rest of the data.

        // hadError will remain set, so the user will get a final error
        // callback stating success.  I hope that's not too misleading.
        // The "success" is that we are no longer blocking waiting on
        // data - we've given up and are now out of the wait loop.
    }

    if (hadError)
    {
        Manager::GetInstance()->GetErrorHandler()(success ? 0 : -1, Manager::FILEERROR_READ, this, dfh->GetName());
    }

    dfh->MovePointer(nbytes);
    return nbytes;
}

unsigned int PosixFileDeviceDriver::Write(void *fh, const void *buf, unsigned int bufsize, DeviceDriver *, void *)
{
    DeviceFileHandle *dfh = static_cast<DeviceFileHandle *>(fh);
    int nbytes = 0;

    bool success = false;

    while (success == false)
    {
        nbytes = static_cast<int>(write(dfh->GetHandle(), buf, bufsize));
        success = nbytes >= 0;

        if (success != true)
        {
            // If the write fails, let the user know and give the game a chance
            // to repeat the operation as many times as they wish.

            if (Manager::GetInstance()->GetErrorHandler()(
                errno, Manager::FILEERROR_WRITE, this, dfh->GetName()))
            {
                // wait a 10 ms before retrying.  The failure may have occurred
                // due to a heavy load on the HDD and waiting for some time to
                // pass before retrying will allow the write to succeed.
                EA::Thread::ThreadSleep(10);
            }
            else
            {
                // do not retry any more.
                nbytes = 0;
                success = true;
            }
        }
    }

    dfh->MovePointer(nbytes);
    return nbytes;
}

uint64_t PosixFileDeviceDriver::Seek(void *fh, uint64_t offset, int whence, DeviceDriver *, void *)
{
    DeviceFileHandle *dfh = static_cast<DeviceFileHandle *>(fh);

    // the driver only needs to handle seeks from the origin because the caller
    // (the Async module) keeps track of the file position
    EA_ASSERT(whence == static_cast<int>(FSEEK_BEGIN));

    // don't bother doing anything if we're
    // already at the correct position.
    // Also don't do anything if the handle has been closed due to
    // a read error
    if (offset == (uint64_t)dfh->GetPos() ||
        dfh->GetHandle() == -1)
    {
        return offset;
    }

    lseek(dfh->GetHandle(), static_cast<off_t>(offset), SEEK_SET);

    LogSeekEvent(GetName(), dfh->GetHandle(), whence, static_cast<uint64_t>(dfh->GetPos()), offset);

    dfh->SetPos((off_t)offset);

    return offset;
}

uint64_t PosixFileDeviceDriver::GetSize(void *fh)
{
    const DeviceFileHandle *dfh = static_cast<DeviceFileHandle *>(fh);

    struct stat s;
    if (fstat(dfh->GetHandle(), &s) != -1)
        return (uint64_t)s.st_size;

    return 0;
}

bool PosixFileDeviceDriver::Resize(void *fh, uint64_t newsize)
{
    DeviceFileHandle *dfh = static_cast<DeviceFileHandle *>(fh);
    int h = dfh->GetHandle();
    if (h == -1) return false;

    off_t cur = dfh->GetPos();
    off_t size = static_cast<off_t>(newsize);

    if (ftruncate(h, size) == -1)
        return false;

    // ftruncate doesn't change file pos
    if (cur > size)
    {
        lseek(h, size, SEEK_SET);
        dfh->SetPos(size);
    }
    return true;
}

bool PosixFileDeviceDriver::Delete(const char *filePath)
{
    char drvPath[FILESYS_MAX_PATH];
    ReplaceSlashes(drvPath, filePath);
    bool ret = remove(drvPath) == 0;
    //printf("\n\nremove \"%s\" returned %d errno=%d\n", drvPath, ret, errno);
    return ret;
}

bool PosixFileDeviceDriver::Move(const char *currentName, const char *newName)
{
    char drvCurrName[FILESYS_MAX_PATH], drvNewName[FILESYS_MAX_PATH];

    return rename(ReplaceSlashes(drvCurrName, currentName),
        ReplaceSlashes(drvNewName, newName)) != -1;
}

// need to globally define _DARWIN_FEATURE_64_BIT_INODE

static void SplitName(char * n, size_t & patternIndex)
{
    patternIndex = 0;
    char *q = n;
    char *p = n;
    while (*p)
    {
        if (*p == '/')
            q = p;
        ++p;
    }
    if (q == n) return;
    *q = 0;
    patternIndex = q-n+1;
}

void * PosixFileDeviceDriver::FindBegin(const char *name, FindData *findData)
{
    if (!findData) return NULL;
    DIR * h;
    DeviceFileHandle *dfh = NULL;

    char drvFileName[FILESYS_MAX_PATH];
    ReplaceSlashes( drvFileName, name );

    size_t patternIndex;
    SplitName(drvFileName, patternIndex);

    h = opendir(drvFileName);
    if (h)
    {
        dfh = new DeviceFileHandle(drvFileName, h);
        dfh->SetPattern(drvFileName + patternIndex);

        if (FindNext(dfh, findData))
            return dfh;
        else
            closedir(h);
        delete dfh;
    }

    return NULL;
}

#define TS_TO_U64_S(x) (uint64_t(x))

bool PosixFileDeviceDriver::FindNext(void *h, FindData *findData)
{
    DeviceFileHandle *dfh = static_cast<DeviceFileHandle *>(h);
    DIR *dir = dfh->GetDirectoryHandle();
    if (!h) return false;
    if (!findData) return false;

    char const * pat = dfh->GetPattern();

    for (;;)
    {
        struct dirent *ent = readdir(dir);
        if (!ent) return false;

        // Test to see if the file has been deleted, which is the case if its file id is 0.
#if defined(EA_PLATFORM_BSD) || defined(EA_PLATFORM_APPLE)
        if(ent->d_fileno != 0) // Linux has d_ino but the kernel doesn't treat it the same.
#endif
        {
            if (NoCaseMatchesWildcardPattern(ent->d_name, pat))
            {
            char buf[FILESYS_MAX_PATH];
            EA::StdC::Snprintf(buf, FILESYS_MAX_PATH, "%s/%s", dfh->GetName(), ent->d_name);
            buf[FILESYS_MAX_PATH-1] = 0;
            EA::StdC::Snprintf(findData->mName, FILESYS_MAX_PATH, "%s", ent->d_name);
            findData->mName[FILESYS_MAX_PATH - 1] = 0;

                struct stat st;
                stat(buf, &st);

                findData->mCreationTime = TS_TO_U64_S(st.st_ctime);
                findData->mAccessTime = TS_TO_U64_S(st.st_atime);
                findData->mModificationTime =  TS_TO_U64_S(st.st_mtime);
                findData->mSize = (uint64_t)st.st_size;

                findData->mFlags = 0;

                if (!(st.st_mode & 0200))
                    findData->mFlags |= DirectoryEntry::FLAG_READONLY;

                /*
                  Was previously

                if (ent->d_type & DT_DIR)
                    findData->mFlags |= DirectoryEntry::FLAG_DIRECTORY;
             
                  but that does not work on Android. I believe that only works on ext*
                  filesystems, but unconfirmed.

                 */

                if (S_ISDIR(st.st_mode))
                    findData->mFlags |= DirectoryEntry::FLAG_DIRECTORY;

                return true;
            }
        }
    }
}

bool PosixFileDeviceDriver::DirectoryCreate(const char *directory)
{
    char localDir[FILESYS_MAX_PATH];
    return mkdir(ReplaceSlashes(localDir, directory), 0755) != -1;
}

bool PosixFileDeviceDriver::DirectoryRemove(const char *directory)
{
    char localDir[FILESYS_MAX_PATH];
    return remove(ReplaceSlashes(localDir, directory)) != -1;
}

bool PosixFileDeviceDriver::FindEnd(void *h)
{
    DeviceFileHandle *dfh = static_cast<DeviceFileHandle *>(h);
    bool result = closedir(dfh->GetDirectoryHandle()) != -1;
    delete dfh;
    return result;
}

bool PosixFileDeviceDriver::GetNameFromHandle(void *fh, char *name, size_t nameBufferSize)
{
    DeviceFileHandle *h = static_cast<DeviceFileHandle *>(fh);

    EA_ASSERT(h != NULL);

    EA_STRLCPY_VERIFY(name, h->GetName(), nameBufferSize);

    return true;
}

PosixFileDeviceDriver gDeviceDriverImpl;

}}}
